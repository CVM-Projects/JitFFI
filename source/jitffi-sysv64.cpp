#include "jitffi-sysv64.h"
#include "opcode.h"
#include "jitffi-def.h"

namespace JitFFI
{
	namespace SysV64
	{
		void JitFuncCallerCreaterPlatform::add_int(uint64_t dat) {
			return _add_int([&](unsigned int c) { return OpCode_sysv64::add_int(jfc, dat, c); });
		}
		void JitFuncCallerCreaterPlatform::add_int_uint32(uint32_t dat) {
			return _add_int([&](unsigned int c) { return OpCode_sysv64::add_int_uint32(jfc, dat, c); });
		}
		void JitFuncCallerCreaterPlatform::add_int_rbx() {
			return _add_int([&](unsigned int c) { return OpCode_sysv64::add_int_rbx(jfc, c); });
		}
		void JitFuncCallerCreaterPlatform::add_double(uint64_t dat) {
			return _add_double([&](unsigned int c) { return OpCode_sysv64::add_double(jfc, dat, c); });
		}

		void JitFuncCallerCreaterPlatform::_add_int(const std::function<OpHandler> &handler) {
			assert(addarg_int_count >= 0);
			--addarg_int_count;
			push_count += handler(addarg_int_count);
		}
		void JitFuncCallerCreaterPlatform::_add_double(const std::function<OpHandler> &handler) {
			assert(addarg_double_count >= 0);
			--addarg_double_count;
			push_count += handler(addarg_double_count);
		}

		void JitFuncCallerCreaterPlatform::call() {
			assert(have_init);
			assert(func);
			OpCode::call_func(jfc, func);
		}

		//

		inline bool need_pass_by_memory(size_t n) {
			if (n > 16)
				return true;
			else
				return false;
		}

		void push_int_data(ArgumentList &list, void *t, const ArgTypeUnit &atu);
		void push_float_data(ArgumentList &list, void *t, const ArgTypeUnit &atu);
		void push_memory_data(ArgumentList &list, void *t, const ArgTypeUnit &atu);
		void push_struct_data(ArgumentList &list, void *t, const ArgTypeUnit &atu);

		void push_data(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			switch (atu.type) {
			case AT_Int:
				push_int_data(list, t, atu);
				break;
			case AT_Float:
				push_float_data(list, t, atu);
				break;
			case AT_Memory:
				push_memory_data(list, t, atu);
				break;
			case AT_Struct:
				if (need_pass_by_memory(atu.size)) {
					push_memory_data(list, t, atu);
				}
				else {
					push_struct_data(list, t, atu);
				}
				break;
			default:
				assert(false);
			}
		}

		void push_int_data(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			list.push(atu.type, convert_uint64(t, atu.size));
		}
		void push_float_data(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			push_int_data(list, t, atu);
		}
		void push_memory_data(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			list.push_memory(t, atu.size);
		}

		struct ArgTypeInfo
		{
			ArgType type;
			unsigned int num;
			unsigned int size;
			unsigned int post;
		};
		void create_struct_typelist(ArgTypeUnit::TypeDataList &newtypelist, const ArgTypeUnit::TypeDataList &typedata);
		void get_argtypeinfo_list(std::list<ArgTypeInfo> &list, size_t align, const ArgTypeUnit::TypeDataList &typedata);
		void push_struct_data_base(ArgumentList &list, void *t, const std::list<ArgTypeInfo> &nlist);

		void push_struct_data(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			ArgTypeUnit::TypeDataList newtypelist;
			std::list<ArgTypeInfo> infolist;

			create_struct_typelist(newtypelist, atu.typedata);
			get_argtypeinfo_list(infolist, atu.align, newtypelist);
			push_struct_data_base(list, t, infolist);
		}

		ArgType get_type(ArgType t1, ArgType t2)
		{
			if (t1 == AT_Unknown)
				return t2;
			else if (t2 == AT_Unknown)
				return t1;
			else if (t1 == t2)
				return t1;
			else if (t1 == AT_Memory || t2 == AT_Memory)
				return AT_Memory;
			else if (t1 == AT_Int || t2 == AT_Int)
				return AT_Int;
			else
				return AT_Float;
		}

		void create_struct_typelist(ArgTypeUnit::TypeDataList &newtypelist, const ArgTypeUnit::TypeDataList &typedata) {
			for (auto type : typedata) {
				assert(type->type != AT_Unknown);
				if (type->type == AT_Struct) {
					create_struct_typelist(newtypelist, type->typedata);
				}
				else {
					newtypelist.push_back(type);
				}
			}
		}

		class NewSize
		{
		public:
			bool push(unsigned int size) {
				assert(size == 1 || size % 2 == 0);
				if (_size + size > 8)
					return false;
				if (_size <= 8 - size) {
					_size += size + (_size % size);
				}
				else {
					return false;
				}
				_num++;
				return true;
			}

			void clear() {
				_size = 0;
				_num = 0;
			}

			unsigned int size() const {
				return _size;
			}
			unsigned int num() const {
				return _num;
			}

		private:
			unsigned int _size = 0;
			unsigned int _num = 0;
		};

		void get_argtypeinfo_list(std::list<ArgTypeInfo> &list, size_t align, const ArgTypeUnit::TypeDataList &typedata)
		{
			NewSize ns;
			ArgType c_type = AT_Unknown;

			auto push_and_clear = [&]() {
				list.push_back({ c_type, ns.num(), ns.size() });
				ns.clear();
				c_type = AT_Unknown;
			};

			for (const auto &e : typedata) {
				ArgType type = e->type;
				unsigned int size = e->size;

				assert(size <= 8);

				if (size == 8) {
					if (c_type != AT_Unknown) {
						push_and_clear();
					}
					list.push_back({ type, 1, 8 });
				}
				else {
					if (!ns.push(size)) {
						push_and_clear();
						ns.push(size);
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

			unsigned int i = 0;
			auto get_next_post = get_next_post_f([=, &typedata]() mutable {
				auto data = typedata.at(i);
				i++;
				return data->size;
			}, align);

			unsigned int post = 0;
			for (auto &e : list) {
				unsigned int num = e.num;
				assert(num != 0);
				unsigned int rpost = get_next_post();
				while (--num) {
					post += get_next_post();
				}
				e.post = rpost;
			}
		}

		void push_struct_data_base(ArgumentList &list, void *t, const std::list<ArgTypeInfo> &nlist)
		{
			byte *p = (byte*)t;

			for (const auto &e : nlist) {
				ArgType type = e.type;
				unsigned int num = e.num;
				unsigned int size = e.size;
				assert(num != 0);
				assert(size != 0);
				JitFFI::push_memory<uint64_t>(p + e.post, size, [&](uint64_t v) { list.push(type, v); });
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

		//

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
