#include "jitffi.h"
#include "jitffi-def.h"
#include "opcode.h"
#include "platform/argtype-sysv64.h"

namespace JitFFI
{
	namespace SysV64
	{
#define _define_atu_(name, type, at_type, size) const ArgTypeUnit atu_##name(at_type, size)

		_define_atu_(void, void, AT_Void, 0);

		_define_atu_(bool, bool, AT_Integer, 1);

		_define_atu_(char, char, AT_Integer, 1);
		_define_atu_(schar, signed char, AT_Integer, 1);
		_define_atu_(uchar, unsigned char, AT_Integer, 1);

		_define_atu_(int, int, AT_Integer, 4);
		_define_atu_(short, short, AT_Integer, 2);
		_define_atu_(lint, long int, AT_Integer, 8);
		_define_atu_(llint, long long int, AT_Integer, 8);

		_define_atu_(uint, unsigned int, AT_Integer, 4);
		_define_atu_(ushort, unsigned short, AT_Integer, 2);
		_define_atu_(ulint, unsigned long int, AT_Integer, 8);
		_define_atu_(ullint, unsigned long long int, AT_Integer, 8);

		_define_atu_(float, float, AT_SSE, 4);
		_define_atu_(double, double, AT_SSE, 8);
		_define_atu_(ldouble, long double, AT_X87, 16);

		_define_atu_(pointer, void*, AT_Integer, 8);

		_define_atu_(size, size_t, AT_Integer, 8);

		_define_atu_(int8, int8_t, AT_Integer, 1);
		_define_atu_(int16, int16_t, AT_Integer, 2);
		_define_atu_(int32, int32_t, AT_Integer, 4);
		_define_atu_(int64, int64_t, AT_Integer, 8);

		_define_atu_(uint8, int8_t, AT_Integer, 1);
		_define_atu_(uint16, int16_t, AT_Integer, 2);
		_define_atu_(uint32, int32_t, AT_Integer, 4);
		_define_atu_(uint64, int64_t, AT_Integer, 8);

#undef _define_atu_
	}
}

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

			void add_void();
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

		void JitFuncCallerCreaterPlatform::add_void() {
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
		struct ArgStructTypeInfo
		{
			uint16_t size = 0;
			uint8_t list[2] = { 0, 0 };
		};

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
			Data resdata;
			ArgType restype;
			ArgStructTypeInfo resstutype;
		};

		constexpr int i = sizeof(ArgTypeInfo);

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
				case AT_Void: _int_count++; break;
				case AT_Integer: _int_count++; break;
				case AT_SSE: _float_count++; break;
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

			void set_resstutype(const ArgStructTypeInfo *asti) {
				this->asti = asti;
			}
			bool return_by_memory() const {
				return asti == nullptr;
			}
			const ArgStructTypeInfo& resstutype() const {
				assert(asti != nullptr);
				return *asti;
			}
			void set_restype(ArgType at) {
				resat = at;
			}
			ArgType restype() const {
				return resat;
			}

		private:
			DataList list;
			unsigned int _int_count = 0;
			unsigned int _float_count = 0;
			unsigned int _memory_count = 0;
			ArgType resat;
			const ArgStructTypeInfo *asti = nullptr;
		};

		//

		inline bool need_pass_by_memory(size_t n) {
			return n > 16;
		}

		namespace Struct
		{
			struct StructTypeList
			{
				struct Data {
					uint8_t type;
					uint8_t size;
				};

				unsigned int num = 0;
				Data data[16] = { 0 };
			};

			static void print_stl(const StructTypeList &stl) {
				printf("(");
				for (int i = 0; i < 16; ++i) {
					printf("(%d %d) ", stl.data[i].type, stl.data[i].size);
				}
				printf("%d)\n", stl.num);
			}

			class StructTypeListCreater
			{
			public:
				explicit StructTypeListCreater(StructTypeList &stl)
					: data(stl) {}

				void push(ArgType type, unsigned size) {
					return push(static_cast<ArgTypeIndex>(type), size);
				}
				void push(ArgTypeIndex type, unsigned size) {
					assert(type < AT_Over);
					assert(size < 16);
					assert(data.num < 16);

					auto &e = data.data[data.num++];
					e.type = static_cast<uint8_t>(type);
					e.size = static_cast<uint8_t>(size);
				}

			private:
				StructTypeList &data;
			};

			// Create StructTypeList

			static void create_struct_typelist_base(StructTypeListCreater &stl_creater, const ArgTypeUnit::TypeList &typelist);
			static StructTypeList create_struct_typebaselist(const ArgTypeUnit::TypeList &typelist) {
				StructTypeList stl;
				StructTypeListCreater stl_creater(stl);

				create_struct_typelist_base(stl_creater, typelist);

				return stl;
			}
			static void create_struct_typelist_base(StructTypeListCreater &stl_creater, const ArgTypeUnit::TypeList &typelist) {
				for (auto type : typelist) {
					assert(AT_Unknown < type->type && type->type < AT_Over);
					if (type->type == AT_Struct) {
						create_struct_typelist_base(stl_creater, type->typelist);
					}
					else {
						stl_creater.push(static_cast<ArgType>(type->type), type->size);
					}
				}
			}

			// StructTypeList -> StructTypeList (Aligned)

			static StructTypeList get_alignsizelist(const StructTypeList &stl, unsigned int align) {
				StructTypeList astl;
				StructTypeListCreater astl_creater(astl);
				unsigned int count = 0;
				unsigned int record;
				unsigned int post;
				int ci = 0;
				for (int i = 0; i != stl.num; ++i) {
					unsigned int size = stl.data[i].size;
					std::tie(count, post) = get_next_post_base(count, size, align);
					if (post != 0) {
						astl_creater.push(stl.data[ci++].type, post - record);
					}
					record = post;
				}
				astl_creater.push(stl.data[ci].type, count - post);
				return astl;
			}

			// Get ArgStructTypeInfo

			static ArgStructTypeInfo get_argstructtypeinfo_base(uint32_t size, const StructTypeList &stl);

			static ArgStructTypeInfo get_argstructtypeinfo(const ArgTypeUnit &atu) {
				StructTypeList stl = get_alignsizelist(create_struct_typebaselist(atu.typelist), atu.align);

				//print_stl(stl);

				return get_argstructtypeinfo_base(atu.size, stl);
			}

			static bool have_type(ArgType t, ArgType t1, ArgType t2) {
				return t1 == t || t2 == t;
			}

			static ArgType get_struct_type_base(ArgType t1, ArgType t2)
			{
				assert(t1 != AT_Unknown);
				assert(t2 != AT_Unknown);

				if (t1 == t2)
					return t1;
				if (t1 == AT_NoClass)
					return t2;
				if (t2 == AT_NoClass)
					return t1;
				if (have_type(AT_Memory, t1, t2))
					return AT_Memory;
				if (have_type(AT_Integer, t1, t2))
					return AT_Integer;
				if (have_type(AT_X87, t1, t2))
					return AT_Memory;
				if (have_type(AT_X87UP, t1, t2))
					return AT_Memory;
				if (have_type(AT_ComplexX87, t1, t2))
					return AT_Memory;
				return AT_SSE;
			}

			static ArgStructTypeInfo get_argstructtypeinfo_base(uint32_t size, const StructTypeList &stl) {
				ArgStructTypeInfo asti;
				assert(size < UINT16_MAX);
				asti.size = static_cast<uint16_t>(size);

				int count = 0;
				int asti_i = 0;
				int stl_i = 0;
				ArgType type = AT_NoClass;
				for (int stl_i = 0; stl_i < stl.num; ++stl_i) {
					auto &e = stl.data[stl_i];
					count += e.size;
					type = get_struct_type_base(type, static_cast<ArgType>(e.type));

					if (count % 8 == 0) {
						asti.list[asti_i++] = type;
						type = AT_NoClass;
					}
				}
				if (count % 8 != 0) {
					asti.list[asti_i++] = type;
				}

				//printf("(%d (%d %d))\n", asti.size, asti.list[0], asti.list[1]);

				return asti;
			}
		}

		ArgStructTypeInfo get_argstructtypeinfo(const ArgTypeUnit &atu) {
			assert(atu.type == AT_Struct);
			return Struct::get_argstructtypeinfo(atu);
		}

		//

		ArgTypeInfo::Data get_argtypeinfo_data(const ArgTypeUnit &atu) {
			switch (atu.type) {
			case AT_Integer:
				return { ArgTypeInfo::op_int, atu.size };
			case AT_SSE:
				return { ArgTypeInfo::op_float, atu.size };
			case AT_Memory:
				return { ArgTypeInfo::op_memory, atu.size };
			case AT_Struct:
				if (need_pass_by_memory(atu.size)) {
					return { ArgTypeInfo::op_memory, atu.size };
				}
				else {
					return { ArgTypeInfo::op_struct, atu.size };
				}
			case AT_X87:
				return { ArgTypeInfo::op_memory, atu.size };
			default:
				assert(false);
			}
			return { ArgTypeInfo::op_int, 0xcccccccc };
		}

		ArgTypeInfo create_argtypeinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist) {
			ArgTypeInfo ati;
			if (restype.type != AT_Void && need_pass_by_memory(restype.size)) {
				ati.resdata = { ArgTypeInfo::op_memory, restype.size };
			}
			else {
				//ati.resdata = { ArgTypeInfo::op_struct, restype.size };
				//ati.resstutype = get_argstructtypeinfo(restype);
			}
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
			unsigned int size = structinfo.size;
			unsigned int count = size > 8 ? 2 : 1;
			byte *p = (byte*)t;
			for (unsigned int i = 0; i < count; ++i) {
				ArgType type = static_cast<ArgType>(structinfo.list[i]);
				JitFFI::push_memory<uint64_t>(p, size, [&](uint64_t v) { list.push(type, v); });
				p += 8;
				size -= 8;
			}
		}
		ArgumentList create_argumentlist(const ArgTypeInfo &ati, const ArgDataList &datalist) {
			ArgumentList list;
			auto iter = datalist.begin();
			auto siter = ati.structlist.begin();
			assert(ati.typelist.size() == datalist.size());
			if (ati.resdata.first == ArgTypeInfo::op_memory) {
				list.push(AT_Void, 0);
			}
			else {
				list.set_restype(ati.restype);
				list.set_resstutype(&ati.resstutype);
			}
			for (auto &type : ati.typelist) {
				switch (type.first) {
				case ArgTypeInfo::op_int:
					list.push(AT_Integer, convert_uint64(*iter, type.second));
					break;
				case ArgTypeInfo::op_float:
					list.push(AT_SSE, convert_uint64(*iter, type.second));
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

		void create_argument(JitFuncCallerCreater &jfcc, ArgumentList &list) {
			ArgType type;
			uint64_t data;

			jfcc.init_addarg_count(list.get_int_count(), list.get_float_count(), list.get_memory_count());

			while (list.get_next(type, data)) {
				switch (type) {
				case AT_Void:
					jfcc.add_void();
					break;
				case AT_Integer:
					jfcc.add_int(data);
					break;
				case AT_SSE:
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

		void create_return(JitFuncCallerCreater &jfcc, ArgumentList &list) {
			//if (!list.return_by_memory()) {
			//	if (list.)
			//}
			if (list.restype() == AT_X87) {
				OpCode_x64::mov_rbx_rdi(jfcc.data());
				OpCode_x64::mov_st0_prbx(jfcc.data());
				OpCode_x64::mov_prbx_st0(jfcc.data());
			}
		}

		//

		ArgumentInfo get_argumentinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist) {
			ArgTypeInfo *p_ati = new ArgTypeInfo(create_argtypeinfo(restype, atlist));
			return ArgumentInfo(P_SysV64, p_ati);
		}
		const ArgTypeInfo& get_argtypeinfo(const ArgumentInfo &argumentinfo) {
			return argumentinfo.data<P_SysV64, ArgTypeInfo>();
		}

		void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, void *func)
		{
			JitFuncCallerCreaterPlatform jfcc(jfc, func);
			OpCode_x64::push_rbx(jfc);
			OpCode_x64::sub_rsp_byte(jfc, 0x8);
			byte &v = jfcc.sub_rsp_unadjusted();

			OpCode_x64::mov_rbx_rdi(jfc);
			create_argument(jfcc, list);

			jfcc.call();

			OpCode_x64::mov_rdi_rbx(jfc);
			create_return(jfcc, list);

			jfcc.add_rsp();
			jfcc.adjust_sub_rsp(v);
			OpCode_x64::add_rsp_byte(jfc, 0x8);
			OpCode_x64::pop_rbx(jfc);
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
