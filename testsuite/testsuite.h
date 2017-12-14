#pragma once

#include <cstdio>
#include <algorithm>
#include <functional>
#include <vector>
#include "jitffi.h"
using namespace JitFFI;
using namespace JitFFI::CurrABI;

template <typename... Args>
void new_printf(const char *msg, Args... args) {}

template <typename... Args>
void old_printf(const char *msg, Args... args) {
	printf(msg, args...);
}

void print_num(uint32_t v) { printf("0x%X\n", v); }
void print_num(int32_t v) { printf("0x%X\n", v); }
void print_num(uint64_t v) { printf("0x%llX\n", v); }
void print_num(int64_t v) { printf("0x%llX\n", v); }
void print_num(double v) { printf("%lf\n", v); }
void print_num(float v) { printf("%f\n", v); }
void print_num(long double v) { printf("%Lf\n", v); }

template <typename T> void print(T t) { return print_num(t); }

using uint = unsigned int;

//#define printf new_printf

// AOT Usage:
//    auto f = Compile(XXX, true);
//    Run(f);

// JIT Usage:
//    Call(XXX);

using CallerProcess = std::function<void(JitFuncCreater &jfc)>;

inline void run_objdump(JitFuncCreater &jfc)
{
	FILE *file = fopen("tmp.bin", "wb+");
	fwrite(jfc.begin(), sizeof(byte), jfc.end() - jfc.begin(), file);
	fclose(file);

	system("objdump -D -b binary -m i386:x86-64 tmp.bin");

	printf("\n");
}

template <typename T = void>
inline auto Compile(CallerProcess handler, bool use_new_memory = true, size_t size = 0x1000)
{
	static JitFuncPool global_pool(0x1000, JitFuncPool::ReadWrite);
	JitFuncPool *pool;

	if (use_new_memory) {
		pool = new JitFuncPool(size, JitFuncPool::ReadWrite);
	}
	else {
		pool = &global_pool;
	}

	JitFunc jf(*pool);
	JitFuncCreater jfc(jf);

	handler(jfc);

	run_objdump(jfc);

	return jf.func<T>();
}

using F1 = void(void *dst, void *data);
using F2 = void(void *dst);

F1* Compile(const ArgumentInfo &info, void *func)
{
	return Compile<F1>([&](JitFuncCreater &jfc) {
		CurrABI::create_function_caller(jfc, info, func);
	});
}

ArgumentInfo GetInfo(const ArgTypeUnit &restype, const ArgTypeList &atlist)
{
	return CurrABI::get_argumentinfo(restype, atlist);
}

template <typename _FTy>
F1* Compile(const ArgumentInfo &info, _FTy func)
{
	return Compile(info, (void*)(func));
}

F2* Compile(const ArgumentInfo &info, void *func, const ArgDataList &adl)
{
	return Compile<F2>([&](JitFuncCreater &jfc) {
		CurrABI::create_function_caller(jfc, info, func, adl);
	});
}

template <typename _FTy>
F2* Compile(const ArgumentInfo &info, _FTy func, const ArgDataList &adl)
{
	return Compile(info, (void*)(func), adl);
}

template <typename _FTy>
inline void Run(_FTy f, void *dst = nullptr)
{
	uint64_t v = f(dst);
	printf("[Return:0x%016llX]\n", v);
}

template <typename _FTy>
inline void Run(_FTy f, void* list[], void *dst = nullptr)
{
	uint64_t v = f(dst, list);
	printf("[Return:0x%016llX]\n", v);
}

inline void Call(CallerProcess handler, void *dst = nullptr)
{
	auto f = Compile<uint64_t(void*)>(handler);

	Run(f, dst);
}

inline void Call(CallerProcess handler, void* list[], void *dst = nullptr)
{
	auto f = Compile<uint64_t(void*, void*[])>(handler);

	Run(f, list, dst);
}
