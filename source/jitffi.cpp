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


namespace JitFFI
{
	namespace MS64
	{
		void push_struct_data_base(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			unsigned int offset = list.push_memory(t, atu.size);
			list.push(AT_Memory, offset);
		}

		void push_struct_data(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			if (need_pass_by_pointer(atu.size)) {
				push_struct_data_base(list, t, atu);
			}
			else {
				list.push(atu.type, convert_uint64(t, atu.size));
			}
		}

		void push_data(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			switch (atu.type) {
			case AT_Unknown:
				push_struct_data(list, t, atu);
				break;
			case AT_Memory:
				push_struct_data_base(list, t, atu);
				break;
			case AT_Int:
			case AT_Float:
				list.push(atu.type, convert_uint64(t, atu.size));
				break;
			default:
				assert(false);
			}
		}

		void add_argument(JitFuncCallerCreater &jfcc, ArgumentList &list) {
			ArgType type;
			uint64_t data;

			jfcc.init_addarg_count(list.size(), 0, 0);

			while (list.get_next_memory(data)) {
				jfcc.push(data);
			}

			jfcc.mov_rbx_rsp();

			while (list.get_next(type, data)) {
				switch (type) {
				case AT_Int:
					jfcc.add_int(data);
					break;
				case AT_Float:
					jfcc.add_double(data);
					break;
				case AT_Memory:
					jfcc.add_int_rbx();
					assert(data * 8 < UINT32_MAX);
					jfcc.sub_rbx(static_cast<uint32_t>(data * 8));
					break;
				default:
					assert(false);
				}
			}
		}
	}

	namespace SysV64
	{
		
		std::shared_ptr<ArgumentList> init_argumentlist() {
			return std::shared_ptr<ArgumentList>(new ArgumentList());
		}

		void push_struct_data(ArgumentList &list, void *t, const ArgTypeUnit &atu);

		void push_data(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			switch (atu.type) {
			case AT_Unknown:
			case AT_Memory:
				push_struct_data(list, t, atu);
				break;
			case AT_Int:
			case AT_Float:
				list.push(atu.type, convert_uint64(t, atu.size));
				break;
			default:
				assert(false);
			}
		}

		void push_struct_data(ArgumentList &list, void *t, const ArgTypeUnit &atu)
		{
			const ArgType init_type = (atu.type == AT_Memory) ? AT_Memory : AT_Unknown;

			NewStruct ns;

			ArgType c_type = AT_Unknown;

			auto push_and_clear = [&]() {
				list.push(c_type, ns.data());
				ns.clear();
				c_type = AT_Unknown;
			};

			for (auto &e : atu.typedata) {
				ArgType type = (init_type == AT_Unknown) ? e.argtype->type : AT_Memory;
				unsigned int size = e.argtype->size;
				byte *p = (byte*)t + e.post;

				assert(size <= 8);

				if (size >= 8) {
					if (c_type != AT_Unknown) {
						push_and_clear();
					}
					list.push(type, convert_uint64(p, size));
				}
				else {
					if (!ns.push(p, size)) {
						push_and_clear();
						ns.push(p, size);
					}
					switch (type) {
					case AT_Int:
						if (c_type == AT_Float || c_type == AT_Unknown) {
							c_type = AT_Int;
						}
						break;
					case AT_Float:
						if (c_type == AT_Unknown) {
							c_type = AT_Float;
						}
						break;
					case AT_Memory:
						c_type = AT_Memory;
						break;
					default:
						assert(false);
					}
				}
			}

			if (c_type != AT_Unknown) {
				push_and_clear();
			}
		}

		void push_struct_data(ArgumentList &list, void *t, size_t size, const TypeList &typelist)
		{
			NewStruct ns;

			ArgType c_type = AT_Unknown;

			auto push_and_clear = [&]() {
				list.push(c_type, ns.data());
				ns.clear();
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
					list.push(type, convert_uint64(p, size));
				}
				else {
					if (!ns.push(p, size)) {
						push_and_clear();
						ns.push(p, size);
					}
					switch (type) {
					case AT_Int:
						if (c_type == AT_Float || c_type == AT_Unknown) {
							c_type = AT_Int;
						}
						break;
					case AT_Float:
						if (c_type == AT_Unknown) {
							c_type = AT_Float;
						}
						break;
					case AT_Memory:
						c_type = AT_Memory;
						break;
					default:
						assert(false);
					}
				}
			}

			if (c_type != AT_Unknown) {
				push_and_clear();
			}
		}

		void add_argument(JitFuncCallerCreater &jfcc, ArgumentList &list)
		{
			ArgType type;
			uint64_t data;

			jfcc.init_addarg_count(list.get_int_count(), list.get_float_count(), list.get_memory_count());

			while (list.get_next(type, data)) {
				switch (type) {
				case AT_Int:
					jfcc.add_int(data);
					break;
				case AT_Float:
					jfcc.add_double(data);
					break;
				case AT_Memory:
					jfcc.push(data);
					break;
				default:
					assert(false);
				}
			}
		}

		void pass_struct(JitFuncCallerCreater &jfcc, void * t, size_t size, const TypeList & typelist)
		{
			if (need_pass_by_memory(size)) {
				jfcc.init_addarg_count(0, 0, 1);

				for (unsigned int i = typelist.num; i != 0; --i) {
					auto &e = typelist.data[i - 1];
					jfcc.push(convert_uint64((byte*)t + e.post, e.size));
				}
			}
			else {
				ArgumentList list;

				push_struct_data(list, t, size, typelist);

				jfcc.init_addarg_count(list.get_int_count(), list.get_float_count(), list.get_memory_count());

				add_argument(jfcc, list);
			}
		}

	}
}

namespace JitFFI
{
	template <typename ArgumentList, typename JitFuncCallerCreaterPlatform>
	void create_function_caller_base(JitFuncCreater &jfc, ArgumentList &list, void *func)
	{
		JitFuncCallerCreaterPlatform jfcc(jfc, func);
		jfcc.push_rbx();
		byte &v = jfcc.sub_rsp_unadjusted();

		add_argument(jfcc, list);

		jfcc.call();

		jfcc.add_rsp();
		jfcc.adjust_sub_rsp(v);
		jfcc.pop_rbx();
		jfcc.ret();
	}

	template <typename ArgumentList, typename JitFuncCallerCreaterPlatform>
	void create_function_caller_base(JitFuncCreater &jfc, void *func, const ArgDataList &adlist, const ArgTypeList &atlist)
	{
		ArgumentList list;

		assert(adlist.size() == atlist.size());
		assert(adlist.size() < UINT32_MAX);

		size_t count = adlist.size();

		auto ad_iter = adlist.begin();
		auto at_iter = atlist.begin();

		while (count--) {
			push_data(list, *ad_iter, **at_iter);

			++ad_iter;
			++at_iter;
		}

		create_function_caller_base<ArgumentList, JitFuncCallerCreaterPlatform>(jfc, list, func);
	}

	namespace SysV64
	{
		void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, void *func)
		{
			create_function_caller_base<ArgumentList, JitFuncCallerCreaterPlatform>(jfc, list, func);
		}
		void create_function_caller(JitFuncCreater &jfc, void *func, const ArgDataList &adlist, const ArgTypeList &atlist)
		{
			create_function_caller_base<ArgumentList, JitFuncCallerCreaterPlatform>(jfc, func, adlist, atlist);
		}
	}
	namespace MS64
	{
		void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, void *func)
		{
			create_function_caller_base<ArgumentList, JitFuncCallerCreaterPlatform>(jfc, list, func);
		}
		void create_function_caller(JitFuncCreater &jfc, void *func, const ArgDataList &adlist, const ArgTypeList &atlist)
		{
			create_function_caller_base<ArgumentList, JitFuncCallerCreaterPlatform>(jfc, func, adlist, atlist);
		}
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
