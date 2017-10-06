#pragma once

#include <cstdio>
#include <algorithm>
#include <functional>
#include "jitffi.h"
#include "opcode.h"
using namespace JitFFI;

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

template <typename T> void print(T t) { return print_num(t); }

using uint = unsigned int;

//#define printf new_printf

// AOT Usage:
//    auto f = Compile(XXX, true);
//    Run(f);

// JIT Usage:
//    Call(XXX);

using CallerProcess = void(JitFuncCreater &jfc);

inline void run_objdump(JitFuncCreater &jfc)
{
	FILE *file = fopen("tmp.bin", "wb+");
	fwrite(jfc.begin(), sizeof(byte), jfc.end() - jfc.begin(), file);
	fclose(file);

	system("objdump -D -b binary -m i386:x86-64 tmp.bin");

	printf("\n");
}

inline auto Compile(CallerProcess *handler, bool use_new_memory = false)
{
	static JitFuncPool global_pool(0x1000, JitFuncPool::ReadWrite);
	JitFuncPool *pool;

	if (use_new_memory) {
		pool = new JitFuncPool(0x1000, JitFuncPool::ReadWrite);
	}
	else {
		pool = &global_pool;
	}

	JitFunc jf(*pool);
	JitFuncCreater jfc(jf);

	handler(jfc);

	run_objdump(jfc);

	return jf.func<int(void)>();
}

template <typename _FTy>
inline void Run(_FTy f)
{
	int v = f();
	printf("%d\n", v);
}

inline void Call(CallerProcess *handler)
{
	auto f = Compile(handler);

	Run(f);
}

#if (defined(_WIN64))
inline bool need_pass_by_pointer(size_t n) {
	return (n != 1 && n != 2 && n != 4 && n != 8);
}
inline void push_copy(JitFuncCallerCreater &jfcc, const void *tp, size_t size) {
	unsigned int n = size / 8 + size % 8;
	const uint64_t *p = reinterpret_cast<const uint64_t*>(tp);

	for (const uint64_t *dp = p + n; dp != p; --dp) {
		jfcc.push(*(dp - 1));
	}
}
#elif (defined(__x86_64__))
enum PassType
{
	PT_Integer,
	PT_SSE,
	PT_SSEUP,
	PT_X87,
	PT_Memory,
};
inline bool need_pass_by_memory(size_t n) {
	if (n > 16)
		return true;
	else
		return false;
}
#endif
