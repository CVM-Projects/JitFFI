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
	bool NewStruct::push(byte * dat, unsigned int size) {
		assert(size != 0);
		if (count + size > 8)
			return false;
		if (size == 8) {
			if (count == 0) {
				write_uint64(0, dat);
				count = 8;
			}
			else {
				return false;
			}
		}
		else if (size == 4) {
			if (count == 0) {
				write_uint32(0, dat);
				count = 4;
			}
			else if (count <= 4) {
				write_uint32(1, dat);
				count = 8;
			}
			else {
				return false;
			}
		}
		else if (size == 2) {
			if (count == 0) {
				write_uint16(0, dat);
				count = 2;
			}
			else if (count <= 2) {
				write_uint16(1, dat);
				count = 4;
			}
			else if (count <= 4) {
				write_uint16(2, dat);
				count = 6;
			}
			else if (count <= 6) {
				write_uint16(3, dat);
				count = 8;
			}
			else {
				return false;
			}
		}
		else if (size == 1) {
			write_byte(count, dat);
			count += 1;
		}
		else {
			assert(false);
			return false;
		}
		return true;
	}
}

namespace JitFFI
{
	const ArgTypeUnit atu_bool(AT_Int, sizeof(bool));

	const ArgTypeUnit atu_char(AT_Int, sizeof(char));
	const ArgTypeUnit atu_schar(AT_Int, sizeof(signed char));
	const ArgTypeUnit atu_uchar(AT_Int, sizeof(unsigned char));
	const ArgTypeUnit atu_wchar(AT_Int, sizeof(wchar_t));

	const ArgTypeUnit atu_int(AT_Int, sizeof(int));
	const ArgTypeUnit atu_lint(AT_Int, sizeof(long int));
	const ArgTypeUnit atu_llint(AT_Int, sizeof(long long int));
	const ArgTypeUnit atu_sint(AT_Int, sizeof(short int));

	const ArgTypeUnit atu_uint(AT_Int, sizeof(unsigned int));
	const ArgTypeUnit atu_ulint(AT_Int, sizeof(unsigned long int));
	const ArgTypeUnit atu_ullint(AT_Int, sizeof(unsigned long long int));
	const ArgTypeUnit atu_usint(AT_Int, sizeof(unsigned short int));

	const ArgTypeUnit atu_float(AT_Float, sizeof(float));
	const ArgTypeUnit atu_double(AT_Float, sizeof(double));

#if (defined(_WIN64))
	const ArgTypeUnit atu_ldouble = atu_double;
#elif (defined(__x86_64__))
	const ArgTypeUnit atu_ldouble(AT_Memory, sizeof(long double), { { 0, &atu_double },{ 8, &atu_double } });
#endif

	const ArgTypeUnit atu_pointer(AT_Int, sizeof(void*));

	const ArgTypeUnit atu_size(AT_Int, sizeof(size_t));

	const ArgTypeUnit atu_int8(AT_Int, sizeof(int8_t));
	const ArgTypeUnit atu_int16(AT_Int, sizeof(int16_t));
	const ArgTypeUnit atu_int32(AT_Int, sizeof(int32_t));
	const ArgTypeUnit atu_int64(AT_Int, sizeof(int64_t));

	const ArgTypeUnit atu_uint8(AT_Int, sizeof(int8_t));
	const ArgTypeUnit atu_uint16(AT_Int, sizeof(int16_t));
	const ArgTypeUnit atu_uint32(AT_Int, sizeof(int32_t));
	const ArgTypeUnit atu_uint64(AT_Int, sizeof(int64_t));
}
