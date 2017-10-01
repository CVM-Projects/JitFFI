#include "jitffi.h"
#include "opcode.h"
#include <cassert>
#include <algorithm>
#include <functional>

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
		mprotect(dp, size, PROT_READ | PROT_EXEC);
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
		mprotect(dp, size, PROT_WRITE | PROT_EXEC);
#endif
	}

	void JitFuncPool::free(void * dp, size_t size)
	{
#if defined(_WIN32)
		VirtualFreeEx(GetCurrentProcess(), dp, size, MEM_RELEASE);
#else
		munmap(dp, size);
#endif
	}

	// JitFuncCallerCreater


	byte JitFuncCallerCreater::get_offset() {
		return (push_count % 2 == 0) ? 0 : 0x8;
	}

	byte JitFuncCallerCreater::get_add_offset() {
		return 0x8 + get_offset() + push_count * 0x8;
	}

	byte& JitFuncCallerCreater::sub_rsp_unadjusted() {
		OpCode_x64::sub_rsp(jfc, 0x8);
		return *(jfc.end() - 1);
	}

	void JitFuncCallerCreater::add_rsp() {
		OpCode_x64::add_rsp(jfc, get_add_offset());
	}

	void JitFuncCallerCreater::adjust_sub_rsp(byte &d) {
		d += get_offset();
	}

	void JitFuncCallerCreater::add_int(uint64_t dat) {
		return _add_int([&](unsigned int c) { return OpCode_curr::add_int(jfc, dat, c); });
	}
	void JitFuncCallerCreater::add_int_uint32(uint32_t dat) {
		return _add_int([&](unsigned int c) { return OpCode_curr::add_int_uint32(jfc, dat, c); });
	}
	void JitFuncCallerCreater::add_int_rbx() {
		return _add_int([&](unsigned int c) { return OpCode_curr::add_int_rbx(jfc, c); });
	}
	void JitFuncCallerCreater::add_double(uint64_t dat) {
		return _add_double([&](unsigned int c) { return OpCode_curr::add_double(jfc, dat, c); });
	}

	void JitFuncCallerCreater::_add_int(const std::function<OpHandler> &handler) {
#if (defined(_WIN64))
		assert(argn - add_count >= 0);
		++add_count;
		push_count += handler(argn - add_count);
#elif (defined(__x86_64__))
		assert(addarg_int_count >= 0);
		--addarg_int_count;
		push_count += handler(addarg_int_count);
#endif
	}
	void JitFuncCallerCreater::_add_double(const std::function<OpHandler> &handler) {
#if (defined(_WIN64))
		assert(argn - add_count >= 0);
		++add_count;
		push_count += handler(argn - add_count);
#elif (defined(__x86_64__))
		assert(addarg_double_count >= 0);
		--addarg_double_count;
		push_count += handler(addarg_double_count);
#endif
	}

	void JitFuncCallerCreater::push(uint64_t dat) {
		OpCode_x64::mov_rax_uint64(jfc, dat);
		OpCode_x64::push_rax(jfc);
		push_count += 1;
	}

	void JitFuncCallerCreater::call() {
#if (defined(_WIN64))
		OpCode_x64::sub_rsp(jfc, 0x20);
#endif
		OpCode::call_func(jfc, func);
#if (defined(_WIN64))
		OpCode_x64::add_rsp(jfc, 0x20);
#endif
	}

	void JitFuncCallerCreater::ret() {
		OpCode::ret(jfc);
	}

}
