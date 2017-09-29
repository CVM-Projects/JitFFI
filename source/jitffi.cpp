#include "jitffi.h"
#include "opcode.h"
#include <cassert>
#include <algorithm>

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


	byte JitFuncCallerCreater::get_sub_offset() {
#if (defined(_WIN64))
		return get_add_offset();
#elif (defined(__x86_64__))
		return 0x8 + ((argn % 2 == 0) ? 0 : 0x8);
#endif
	}

	byte JitFuncCallerCreater::get_add_offset() {
#if (defined(_WIN64))
		int nargn = (argn % 2 == 0) ? argn : argn + 1;
		return 0x28 + std::max<int>(0, nargn - 4) * 0x8;
#elif (defined(__x86_64__))
		return 0x8 + (push_count + 1) / 2 * 0x10;
#endif
	}

	void JitFuncCallerCreater::sub_rsp() {
		OpCode_x64::sub_rsp(jfc, get_sub_offset());
	}

	byte& JitFuncCallerCreater::sub_rsp_unadjusted() {
		OpCode_x64::sub_rsp(jfc, 0);
		return *(jfc.end() - 1);
	}

	void JitFuncCallerCreater::add_rsp() {
		OpCode_x64::add_rsp(jfc, get_add_offset());
	}

	void JitFuncCallerCreater::adjust_sub_rsp(byte &d) {
		d = get_sub_offset();
	}

	void JitFuncCallerCreater::addarg_int(uint64_t dat) {
#if (defined(_WIN64))
		assert(argn - add_count >= 0);
		++add_count;
		push_count += OpCode_win64::add_int(jfc, dat, argn - add_count);
#elif (defined(__x86_64__))
		assert(addarg_double_count >= 0);
		--addarg_int_count;
		push_count += OpCode_sysv64::add_int(jfc, dat, addarg_int_count);
#endif
	}
	void JitFuncCallerCreater::addarg_double(uint64_t dat) {
#if (defined(_WIN64))
		assert(argn - add_count >= 0);
		++add_count;
		push_count += OpCode_win64::add_double(jfc, dat, argn - add_count);
#elif (defined(__x86_64__))
		assert(addarg_double_count >= 0);
		--addarg_double_count;
		push_count += OpCode_sysv64::add_double(jfc, dat, addarg_double_count);
#endif
	}

	void JitFuncCallerCreater::call() {
		OpCode::call_func(jfc, func);
	}

	void JitFuncCallerCreater::ret() {
		OpCode::ret(jfc);
	}

}
