#include "jitffi.h"
#include "jitffi-def.h"
#include "opcode.h"

namespace JitFFI
{
	namespace SysV64
	{
		class JitFuncCallerCreaterPlatform : public JitFuncCallerCreater
		{
		public:
			template <typename _FTy>
			explicit JitFuncCallerCreaterPlatform(JitFuncCreater &jfc, _FTy *func = nullptr)
				: JitFuncCallerCreater(jfc, func) {}

			void init_addarg_count(unsigned int int_c, unsigned int dou_c, unsigned int mem_c = 0) {
				addarg_int_count = int_c;
				addarg_double_count = dou_c;
				have_init = true;
			}

			void add_int(uint64_t dat);
			void add_int_uint32(uint32_t dat);
			void add_int_rbx();
			void add_double(uint64_t dat);

			void call();

		private:
			void _add_int(const std::function<OpHandler> &handler);
			void _add_double(const std::function<OpHandler> &handler);

			unsigned int addarg_int_count = 0;
			unsigned int addarg_double_count = 0;
		};

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
			OpCode_x64::call_func(jfc, func);
		}
	}
}

namespace JitFFI
{
	namespace SysV64
	{
		class ArgumentList
		{
			struct Data {
				ArgType type;
				uint64_t data;
			};
			using DataList = std::list<Data>;

		public:
			ArgumentList() = default;

			void push(ArgType type, uint64_t v) {
				switch (type) {
				case AT_Int: _int_count++; break;
				case AT_Float: _float_count++; break;
				case AT_Memory: _memory_count++; break;
				default: assert(false);
				}
				list.push_front({ type, v });
			}

			void push_memory(uint64_t v) {
				list.push_front({ AT_Memory, v });
				_memory_count++;
			}

			void push_memory(const void *dat, size_t size) {
				JitFFI::push_memory<uint64_t>(dat, size, [&](uint64_t v) { push_memory(v); });
			}

			bool get_next(ArgType &type, uint64_t &data) {
				if (list.empty()) {
					return false;
				}
				else {
					auto &e = list.front();
					type = e.type;
					data = e.data;
					list.pop_front();
					return true;
				}
			}

			unsigned int get_int_count() const {
				return _int_count;
			}
			unsigned int get_float_count() const {
				return _float_count;
			}
			unsigned int get_memory_count() const {
				return _memory_count;
			}

		private:
			DataList list;
			unsigned int _int_count = 0;
			unsigned int _float_count = 0;
			unsigned int _memory_count = 0;
		};

		struct ArgStructTypeInfoUnit
		{
			ArgType type;
			unsigned int num;
			unsigned int size;
			unsigned int post;
		};
		using ArgStructTypeInfo = std::list<ArgStructTypeInfoUnit>;

		struct ArgTypeInfo
		{
			enum OP
			{
				op_int,
				op_float,
				op_memory,
				op_struct,
			};
			using Size = unsigned int;
			using Data = std::pair<OP, Size>;

			std::vector<Data> typelist;
			std::vector<ArgStructTypeInfo> structlist;
		};

		//

		inline bool need_pass_by_memory(size_t n) {
			if (n > 16)
				return true;
			else
				return false;
		}

		namespace Struct
		{
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

			void create_struct_typelist_base(ArgTypeUnit::TypeDataList &newtypelist, const ArgTypeUnit::TypeDataList &typedata) {
				for (auto type : typedata) {
					assert(type->type != AT_Unknown);
					if (type->type == AT_Struct) {
						create_struct_typelist_base(newtypelist, type->typedata);
					}
					else {
						newtypelist.push_back(type);
					}
				}
			}
			ArgTypeUnit::TypeDataList create_struct_typelist(const ArgTypeUnit::TypeDataList &typedata) {
				ArgTypeUnit::TypeDataList newtypelist;
				create_struct_typelist_base(newtypelist, typedata);
				return newtypelist;
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

				void clear() { _size = 0; _num = 0; }

				unsigned int size() const { return _size; }
				unsigned int num() const { return _num; }

			private:
				unsigned int _size = 0;
				unsigned int _num = 0;
			};

			ArgStructTypeInfo get_argstructtypeinfo_base(unsigned int align, const ArgTypeUnit::TypeDataList &typedata)
			{
				ArgStructTypeInfo list;
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

				return list;
			}

			ArgStructTypeInfo get_argstructtypeinfo(const ArgTypeUnit &atu)
			{
				ArgTypeUnit::TypeDataList newtypelist = create_struct_typelist(atu.typedata);
				return get_argstructtypeinfo_base(atu.align, newtypelist);
			}
		}

		ArgStructTypeInfo get_argstructtypeinfo(const ArgTypeUnit &atu) {
			return Struct::get_argstructtypeinfo(atu);
		}

		//

		ArgTypeInfo::Data get_argtypeinfo_data(const ArgTypeUnit &atu) {
			switch (atu.type) {
			case AT_Int:
				return { ArgTypeInfo::op_int, atu.size };
			case AT_Float:
				return { ArgTypeInfo::op_float, atu.size };
				break;
			case AT_Memory:
				return { ArgTypeInfo::op_memory, atu.size };
				break;
			case AT_Struct:
				if (need_pass_by_memory(atu.size)) {
					return { ArgTypeInfo::op_memory, atu.size };
				}
				else {
					return { ArgTypeInfo::op_struct, atu.size };
				}
				break;
			default:
				assert(false);
			}
			return { ArgTypeInfo::op_int, 0xcccccccc };
		}

		ArgTypeInfo create_argtypeinfo(const ArgTypeList &atlist) {
			ArgTypeInfo ati;
			for (auto &type : atlist) {
				ArgTypeInfo::Data data = get_argtypeinfo_data(*type);
				ati.typelist.push_back(data);
				if (data.first == ArgTypeInfo::op_struct) {
					ati.structlist.push_back(get_argstructtypeinfo(*type));
				}
			}
			return ati;
		}

		void push_struct_data(ArgumentList &list, const void *t, const ArgStructTypeInfo &structinfo) {
			byte *p = (byte*)t;

			for (const auto &e : structinfo) {
				ArgType type = e.type;
				unsigned int num = e.num;
				unsigned int size = e.size;
				assert(num != 0);
				assert(size != 0);
				JitFFI::push_memory<uint64_t>(p + e.post, size, [&](uint64_t v) { list.push(type, v); });
			}
		}
		ArgumentList create_argumentlist(const ArgTypeInfo &ati, const ArgDataList &datalist) {
			ArgumentList list;
			auto iter = datalist.begin();
			auto siter = ati.structlist.begin();
			assert(ati.typelist.size() == datalist.size());
			for (auto &type : ati.typelist) {
				switch (type.first) {
				case ArgTypeInfo::op_int:
					list.push(AT_Int, convert_uint64(*iter, type.second));
					break;
				case ArgTypeInfo::op_float:
					list.push(AT_Float, convert_uint64(*iter, type.second));
					break;
				case ArgTypeInfo::op_memory:
					list.push_memory(*iter, type.second);
					break;
				case ArgTypeInfo::op_struct: {
					assert(siter != ati.structlist.end());
					push_struct_data(list, *iter, *siter);
					++siter;
					break;
				}
				default:
					assert(false);
					break;
				}
				++iter;
			}
			return list;
		}

		void add_argument(JitFuncCallerCreater &jfcc, ArgumentList &list) {
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

		ArgumentInfo get_argumentinfo(const ArgTypeList &atlist) {
			ArgTypeInfo *p_ati = new ArgTypeInfo(create_argtypeinfo(atlist));
			return ArgumentInfo(P_SysV64, p_ati);
		}
		const ArgTypeInfo& get_argtypeinfo(const ArgumentInfo &argumentinfo) {
			return argumentinfo.data<P_SysV64, ArgTypeInfo>();
		}

		void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, void *func)
		{
			JitFuncCallerCreaterPlatform jfcc(jfc, func);
			byte &v = jfcc.sub_rsp_unadjusted();

			add_argument(jfcc, list);

			jfcc.call();

			jfcc.add_rsp();
			jfcc.adjust_sub_rsp(v);
			jfcc.ret();
		}

		void create_function_caller(JitFuncCreater &jfc, void *func, const ArgumentInfo &argumentinfo, const ArgDataList &adlist)
		{
			assert(adlist.size() < UINT32_MAX);

			ArgumentList list = create_argumentlist(get_argtypeinfo(argumentinfo), adlist);

			create_function_caller(jfc, list, func);
		}
	}
}
