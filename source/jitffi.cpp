#include "jitffi.h"
#include "opcode.h"
#include <cassert>
#include <algorithm>
#include <functional>
#include <vector>

#if defined(_WIN32)
#	include <Windows.h>
#else
#	include <sys/mman.h>
#endif

namespace JitFFI
{

	// JitFuncPool

	void* JitFuncPool::alloc(size_t size, bool readonly)
	{
#if defined(_WIN32)
		auto flags = readonly ? PAGE_EXECUTE_READ : PAGE_EXECUTE_READWRITE;
		void *dp = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, flags);
#else
		auto flags = (readonly ? PROT_READ : PROT_WRITE) | PROT_EXEC;
		void *dp = mmap(NULL, size, flags, MAP_ANON | MAP_PRIVATE, -1, 0);
#endif
		return dp;
	}

	void JitFuncPool::protect(void * dp, size_t size)
	{
		assert(size != 0);
#if defined(_WIN32)
		DWORD w;
		BOOL v = VirtualProtect(dp, size, PAGE_EXECUTE_READ, &w);
		assert(v);
#else
		int v = mprotect(dp, size, PROT_READ | PROT_EXEC);
		assert(v == 0);
#endif
	}

	void JitFuncPool::unprotect(void * dp, size_t size)
	{
		assert(size != 0);
#if defined(_WIN32)
		DWORD w;
		BOOL v = VirtualProtect(dp, size, PAGE_EXECUTE_READWRITE, &w);
		assert(v);
#else
		int v = mprotect(dp, size, PROT_WRITE | PROT_EXEC);
		assert(v == 0);
#endif
	}

	void JitFuncPool::free(void * dp, size_t size)
	{
#if defined(_WIN32)
		BOOL v = VirtualFree(dp, 0, MEM_RELEASE);
		assert(v);
#else
		int v = munmap(dp, size);
		assert(v == 0);
#endif
	}
}

namespace JitFFI
{
	// JitFuncCallerCreater

	byte JitFuncCallerCreater::get_offset() {
		return (push_count % 2 == 0) ? 0 : 0x8;
	}

	auto JitFuncCallerCreater::get_add_offset() {
		auto push_offset = push_count * 0x8;
		assert(push_offset <= UINT32_MAX - 0x10);
		return 0x8 + get_offset() + push_offset;
	}

	byte& JitFuncCallerCreater::sub_rsp_unadjusted() {
		OpCode_x64::sub_rsp_byte(jfc, 0x8);
		return *(jfc.end() - 1);
	}

	void JitFuncCallerCreater::add_rsp() {
		OpCode_x64::add_rsp_uint32(jfc, get_add_offset());  // !NOTICE! this num may > 1 byte.
	}

	void JitFuncCallerCreater::adjust_sub_rsp(byte &d) {
		d += get_offset();
	}

	void JitFuncCallerCreater::push(uint64_t dat) {
		OpCode_x64::mov_rax_uint64(jfc, dat);
		OpCode_x64::push_rax(jfc);
		push_count += 1;
	}

	void JitFuncCallerCreater::sub_rbx(uint32_t dat) {
		OpCode_x64::sub_rbx_uint32(jfc, dat);
	}
	void JitFuncCallerCreater::mov_rbx_rsp() {
		OpCode_x64::mov_rbx_rsp(jfc);
	}

	void JitFuncCallerCreater::push_rbx() {
		OpCode_x64::push_rbx(jfc);
		OpCode_x64::sub_rsp_byte(jfc, 0x8);
	}
	void JitFuncCallerCreater::pop_rbx() {
		OpCode_x64::add_rsp_byte(jfc, 0x8);
		OpCode_x64::pop_rbx(jfc);
	}

	void JitFuncCallerCreater::ret() {
		OpCode_x64::ret(jfc);
	}
}

namespace JitFFI
{
#define _define_atu_(name, type, at_type) const ArgTypeUnit atu_##name(at_type, sizeof(type), alignof(type))

	_define_atu_(bool, bool, AT_Int);

	_define_atu_(char, char, AT_Int);
	_define_atu_(schar, signed char, AT_Int);
	_define_atu_(uchar, unsigned char, AT_Int);
	_define_atu_(wchar, wchar_t, AT_Int);

	_define_atu_(int, int, AT_Int);
	_define_atu_(lint, long int, AT_Int);
	_define_atu_(llint, long long int, AT_Int);
	_define_atu_(sint, short int, AT_Int);

	_define_atu_(uint, unsigned int, AT_Int);
	_define_atu_(ulint, unsigned long int, AT_Int);
	_define_atu_(ullint, unsigned long long int, AT_Int);
	_define_atu_(usint, unsigned short int, AT_Int);

	_define_atu_(float, float, AT_Float);
	_define_atu_(double, double, AT_Float);

#if (defined(_WIN64))
	const ArgTypeUnit atu_ldouble = atu_double;
#elif (defined(__x86_64__))
	const ArgTypeUnit atu_ldouble(AT_Memory, sizeof(long double), alignof(long double), { &atu_double, &atu_double });
#endif

	_define_atu_(pointer, void*, AT_Int);

	_define_atu_(size, size_t, AT_Int);

	_define_atu_(int8, int8_t, AT_Int);
	_define_atu_(int16, int16_t, AT_Int);
	_define_atu_(int32, int32_t, AT_Int);
	_define_atu_(int64, int64_t, AT_Int);

	_define_atu_(uint8, int8_t, AT_Int);
	_define_atu_(uint16, int16_t, AT_Int);
	_define_atu_(uint32, int32_t, AT_Int);
	_define_atu_(uint64, int64_t, AT_Int);

#undef _define_atu_
}
