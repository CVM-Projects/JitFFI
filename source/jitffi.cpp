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

	void MS64::JitFuncCallerCreaterPlatform::add_int(uint64_t dat) {
		return _add_int([&](unsigned int c) { return OpCode_win64::add_int(jfc, dat, c); });
	}
	void MS64::JitFuncCallerCreaterPlatform::add_int_uint32(uint32_t dat) {
		return _add_int([&](unsigned int c) { return OpCode_win64::add_int_uint32(jfc, dat, c); });
	}
	void MS64::JitFuncCallerCreaterPlatform::add_int_rbx() {
		return _add_int([&](unsigned int c) { return OpCode_win64::add_int_rbx(jfc, c); });
	}
	void MS64::JitFuncCallerCreaterPlatform::add_double(uint64_t dat) {
		return _add_double([&](unsigned int c) { return OpCode_win64::add_double(jfc, dat, c); });
	}

	void SysV64::JitFuncCallerCreaterPlatform::add_int(uint64_t dat) {
		return _add_int([&](unsigned int c) { return OpCode_sysv64::add_int(jfc, dat, c); });
	}
	void SysV64::JitFuncCallerCreaterPlatform::add_int_uint32(uint32_t dat) {
		return _add_int([&](unsigned int c) { return OpCode_sysv64::add_int_uint32(jfc, dat, c); });
	}
	void SysV64::JitFuncCallerCreaterPlatform::add_int_rbx() {
		return _add_int([&](unsigned int c) { return OpCode_sysv64::add_int_rbx(jfc, c); });
	}
	void SysV64::JitFuncCallerCreaterPlatform::add_double(uint64_t dat) {
		return _add_double([&](unsigned int c) { return OpCode_sysv64::add_double(jfc, dat, c); });
	}

	void MS64::JitFuncCallerCreaterPlatform::_add_int(const std::function<OpHandler> &handler) {
		assert(argn - add_count >= 0);
		++add_count;
		push_count += handler(argn - add_count);
	}
	void MS64::JitFuncCallerCreaterPlatform::_add_double(const std::function<OpHandler> &handler) {
		assert(argn - add_count >= 0);
		++add_count;
		push_count += handler(argn - add_count);
	}

	void SysV64::JitFuncCallerCreaterPlatform::_add_int(const std::function<OpHandler> &handler) {
		assert(addarg_int_count >= 0);
		--addarg_int_count;
		push_count += handler(addarg_int_count);
	}
	void SysV64::JitFuncCallerCreaterPlatform::_add_double(const std::function<OpHandler> &handler) {
		assert(addarg_double_count >= 0);
		--addarg_double_count;
		push_count += handler(addarg_double_count);
	}

	void JitFuncCallerCreater::push(uint64_t dat) {
		OpCode_x64::mov_rax_uint64(jfc, dat);
		OpCode_x64::push_rax(jfc);
		push_count += 1;
	}

	void JitFuncCallerCreater::push_rbx() {
		OpCode_x64::push_rbx(jfc);
		OpCode_x64::sub_rsp_byte(jfc, 0x8);
	}
	void JitFuncCallerCreater::pop_rbx() {
		OpCode_x64::add_rsp_byte(jfc, 0x8);
		OpCode_x64::pop_rbx(jfc);
	}

	void SysV64::JitFuncCallerCreaterPlatform::call() {
		assert(have_init);
		assert(func);
		OpCode::call_func(jfc, func);
	}

	void MS64::JitFuncCallerCreaterPlatform::call() {
		assert(have_init);
		assert(func);
		OpCode_x64::sub_rsp_byte(jfc, 0x20);
		OpCode::call_func(jfc, func);
		OpCode_x64::add_rsp_byte(jfc, 0x20);
	}

	void JitFuncCallerCreater::ret() {
		OpCode::ret(jfc);
	}
}

namespace JitFFI
{
	bool NewStruct::push(byte * dat, unsigned int size) {
		assert(size != 0);
		if (count + size > 8)
			return false;
		if (size == 4) {
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

void JitFFI::MS64::pass_struct(JitFuncCallerCreater & jfcc, void * t, size_t size, const TypeList & typelist)
{
	if (need_pass_by_pointer(size)) {
		jfcc.init_addarg_count(0, 0, 1);

		push_copy(jfcc, t, size);

		OpCode_x64::mov_rbx_rsp(jfcc.data());

		jfcc.add_int_rbx();
	}
	else {
		jfcc.init_addarg_count(1, 0, 0);

		jfcc.add_int(convert_uint64(t, size));
	}
}

void JitFFI::SysV64::pass_struct(JitFuncCallerCreater & jfcc, void * t, size_t size, const TypeList & typelist)
{
	if (need_pass_by_memory(size)) {
		jfcc.init_addarg_count(0, 0, 1);

		for (unsigned int i = typelist.num; i != 0; --i) {
			auto &e = typelist.data[i - 1];
			jfcc.push(convert_uint64((byte*)t + e.post, e.size));
		}
	}
	else {
		unsigned int int_count = 0;
		unsigned int double_count = 0;
		unsigned int memory_count = 0;

		NewStruct ns;

		ArgType c_type = AT_Unknown;

		std::vector<uint64_t> vec;
		std::vector<ArgType> vectype;

		auto add_count = [&](ArgType type) {
			switch (type) {
			case AT_Int:
				int_count++;
				break;
			case AT_Double:
				double_count++;
				break;
			case AT_Memory:
				memory_count++;
				break;
			default:
				assert(false);
			}
		};

		auto push_and_clear = [&]() {
			vec.push_back(ns.data());
			vectype.push_back(c_type);
			ns.clear();
			add_count(c_type);
			c_type = AT_Unknown;
		};

		for (unsigned int i = 0; i != typelist.num; ++i) {
			auto &e = typelist.data[i];
			ArgType type = e.type;
			unsigned int size = e.size;
			byte *p = (byte*)t + e.post;

			assert(size <= 8);

			if (size >= 8) {
				if (c_type != AT_Unknown) {
					push_and_clear();
				}
				vec.push_back(convert_uint64(p, size));
				vectype.push_back(type);
				add_count(type);
			}
			else {
				if (ns.push(p, size)) {
					switch (type) {
					case AT_Int:
						if (c_type == AT_Double || c_type == AT_Unknown) {
							c_type = AT_Int;
						}
						break;
					case AT_Double:
						if (c_type == AT_Unknown) {
							c_type = AT_Double;
						}
						break;
					case AT_Memory:
						c_type = AT_Memory;
					}
				}
				else {
					push_and_clear();
					i--;
					continue;
				}
			}
		}

		if (c_type != AT_Unknown) {
			push_and_clear();
		}

		jfcc.init_addarg_count(int_count, double_count, memory_count);

		for (unsigned int i = static_cast<unsigned int>(vec.size()); i != 0; --i) {
			auto &e = vec[i - 1];
			switch (vectype[i - 1]) {
			case AT_Int:
				jfcc.add_int(convert_uint64(e));
				break;
			case AT_Double:
				jfcc.add_double(convert_uint64(e));
				break;
			case AT_Memory:
				jfcc.push(convert_uint64(e));
				break;
			default:
				assert(false);
			}
		}
	}
}
