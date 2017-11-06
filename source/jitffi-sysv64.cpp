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
		struct ArgTypeInfo
		{
			enum OP : uint8_t {
				op_void,
				op_int,
				op_float,
				op_memory,
				op_struct,
				op_over,
			};

			struct StructBaseData {
				StructBaseData() : StructBaseData(0, 0) {}
				StructBaseData(uint8_t t0, uint8_t t1) : type0(t0), type1(t1) {
					assert(t0 < 16);
					assert(t1 < 16);
				}

				uint8_t type0 : 4;
				uint8_t type1 : 4;

				void set_typex(int i, uint8_t type) {
					assert(i == 0 || i == 1);
					assert(type < 16);
					if (i == 0)
						type0 = type;
					else
						type1 = type;
				}
				uint8_t get_typex(int i) const {
					assert(i == 0 || i == 1);
					return (i == 0) ? type0 : type1;
				}
			};

			struct StructTypeData : public StructBaseData {
				StructTypeData() = default;

				StructTypeData(ArgType t0, ArgType t1)
					: StructBaseData(t0, t1) {}

				void set(int i, ArgType type) {
					assert(AT_Unknown < type && type < AT_Over);
					set_typex(i, type);
				}
				ArgType get(int i) const {
					return static_cast<ArgType>(get_typex(i));
				}
			};

			struct StructPassData : public StructBaseData {
				StructPassData() = default;

				StructPassData(OP t0, OP t1)
					: StructBaseData(t0, t1) {}

				void set(int i, OP type) {
					assert(type < OP::op_over);
					set_typex(i, type);
				}
				OP get(int i) const {
					return static_cast<OP>(get_typex(i));
				}
			};

			struct PassData {
				PassData() = default;

				PassData(OP op, uint16_t size)
					: op(op), size(size) {}

				void setspd(const StructPassData &spd) {
					if (spd.get(0) == op_memory || spd.get(1) == op_memory) {
						op = op_memory;
					}
					this->spd = spd;
				}
				const StructPassData& getspd() const {
					return spd;
				}

				uint8_t op = 0;
			private:
				StructPassData spd;
			public:
				uint16_t size = 0;
			};

			struct RetData {
				RetData() = default;

				RetData(ArgTypeIndex type, uint16_t size)
					: type(type), size(size) {
					assert(AT_Unknown < type && type < AT_Over);
				}

				void setstd(const StructTypeData &std) {
					if (std.get(0) == AT_Memory || std.get(1) == AT_Memory) {
						type = AT_Memory;
					}
					this->std = std;
				}
				const StructTypeData& getstd() const {
					return std;
				}

				uint8_t type = 0;
			private:
				StructTypeData std;
			public:
				uint16_t size = 0;
			};

			ArgTypeInfo(uint32_t size)
				: size(size), typelist((PassData*)std::malloc(size * sizeof(PassData))) {}

			struct Deleter { void operator()(PassData *p) { std::free(p); } };

			uint32_t size;
			RetData restype;
			std::unique_ptr<PassData, Deleter> typelist;
		};

		constexpr int ix = sizeof(ArgTypeInfo);

		//==============================
		// * Get ArgTypeInfo
		//==============================

		static ArgTypeInfo::PassData get_argtypeinfo_pass_data(const ArgTypeUnit &atu);
		static ArgTypeInfo::RetData get_argtypeinfo_ret_data(const ArgTypeUnit &atu);

		static ArgTypeInfo create_argtypeinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist) {
			return JitFFI::create_argtypeinfo<ArgTypeInfo, ArgTypeInfo::PassData, ArgTypeInfo::RetData, get_argtypeinfo_pass_data, get_argtypeinfo_ret_data>(restype, atlist);
		}

		static bool need_pass_by_memory(size_t n) {
			return n > 16;
		}

		static ArgTypeInfo::StructTypeData get_argstructtypeinfo(const ArgTypeUnit &atu);
		static ArgTypeInfo::StructPassData convert_to_structpassdata(const ArgTypeInfo::StructTypeData &astd);

		static ArgTypeInfo::PassData get_argtypeinfo_pass_data(const ArgTypeUnit &atu) {
			switch (atu.type) {
			case AT_Integer:
				return ArgTypeInfo::PassData(ArgTypeInfo::op_int, atu.size);
			case AT_SSE:
				return ArgTypeInfo::PassData(ArgTypeInfo::op_float, atu.size);
			case AT_Struct:
				if (need_pass_by_memory(atu.size)) {
					return ArgTypeInfo::PassData(ArgTypeInfo::op_memory, atu.size);
				}
				else {
					ArgTypeInfo::PassData res(ArgTypeInfo::op_struct, atu.size);
					res.setspd(convert_to_structpassdata(get_argstructtypeinfo(atu)));
					return res;
				}
			case AT_Memory:
			case AT_X87:
			case AT_X87UP:
			case AT_ComplexX87:
				return ArgTypeInfo::PassData(ArgTypeInfo::op_memory, atu.size);
			default:
				assert(false);
			}
			return ArgTypeInfo::PassData(ArgTypeInfo::op_int, 0xcccc);
		}

		static ArgTypeInfo::RetData get_argtypeinfo_ret_data(const ArgTypeUnit &atu) {
			ArgTypeInfo::RetData ard(atu.type, atu.size);
			if (atu.type == AT_Struct) {
				if (need_pass_by_memory(atu.size)) {
					ard.type = AT_Memory;
				}
				else {
					ard.setstd(get_argstructtypeinfo(atu));
					if (ard.getstd().get(0) == AT_Memory || ard.getstd().get(1) == AT_Memory) {
						ard.type = AT_Memory;
					}
				}
			}
			return ard;
		}

		static ArgTypeInfo::OP get_pass_type(ArgType type) {
			switch (type) {
			case AT_Unknown:
				return ArgTypeInfo::op_void;
			case AT_Integer:
				return ArgTypeInfo::op_int;
			case AT_SSE:
				return ArgTypeInfo::op_float;
			case AT_Memory:
			case AT_X87:
			case AT_X87UP:
			case AT_ComplexX87:
				return ArgTypeInfo::op_memory;
			default:
				assert(false);
				return ArgTypeInfo::op_void;
			}
		}

		static ArgTypeInfo::StructPassData convert_to_structpassdata(const ArgTypeInfo::StructTypeData &astd) {
			return ArgTypeInfo::StructPassData(get_pass_type(astd.get(0)), get_pass_type(astd.get(1)));
		}

		//==============================
		// * Get ArgStructTypeInfo
		//==============================

		namespace Struct
		{
			struct StructTypeList
			{
				struct Data {
					uint8_t type;
					uint8_t size;
				};

				unsigned int num = 0;
				Data data[16] = { { 0, 0 } };
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

			static ArgTypeInfo::StructTypeData get_argstructtypeinfo_base(uint32_t size, const StructTypeList &stl);

			static ArgTypeInfo::StructTypeData get_argstructtypeinfo(const ArgTypeUnit &atu) {
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

			static ArgTypeInfo::StructTypeData get_argstructtypeinfo_base(uint32_t size, const StructTypeList &stl) {
				ArgTypeInfo::StructTypeData asti;
				assert(size < UINT16_MAX);

				int count = 0;
				int asti_i = 0;
				int stl_i = 0;
				ArgType type = AT_NoClass;
				for (unsigned stl_i = 0; stl_i < stl.num; ++stl_i) {
					auto &e = stl.data[stl_i];
					count += e.size;
					type = get_struct_type_base(type, static_cast<ArgType>(e.type));

					if (count % 8 == 0) {
						asti.set(asti_i++, type);
						type = AT_NoClass;
					}
				}
				if (count % 8 != 0) {
					asti.set(asti_i++, type);
				}

				//printf("(%d (%d %d))\n", asti.size, asti.get(0), asti.get(1));

				return asti;
			}
		}

		static ArgTypeInfo::StructTypeData get_argstructtypeinfo(const ArgTypeUnit &atu) {
			assert(atu.type == AT_Struct);
			assert(!need_pass_by_memory(atu.size));
			return Struct::get_argstructtypeinfo(atu);
		}
	}

	namespace SysV64
	{
		//==============================
		// * Get ArgumentList
		//==============================

		class ArgumentList
		{
			struct Data {
				ArgTypeInfo::OP type;
				uint64_t data;
			};
			using DataList = std::list<Data>;

		public:
			ArgumentList() = default;

			void push(ArgTypeInfo::OP type, uint64_t v) {
				switch (type) {
				case ArgTypeInfo::op_void: _int_count++; break;
				case ArgTypeInfo::op_int: _int_count++; break;
				case ArgTypeInfo::op_float: _float_count++; break;
				case ArgTypeInfo::op_memory: _memory_count++; break;
				default: assert(false);
				}
				list.push_front({ type, v });
			}

			void push_memory(uint64_t v) {
				list.push_front({ ArgTypeInfo::op_memory, v });
				_memory_count++;
			}

			void push_memory(const void *dat, size_t size) {
				JitFFI::push_memory<uint64_t>(dat, size, [&](uint64_t v) { push_memory(v); });
			}

			bool get_next(ArgTypeInfo::OP &type, uint64_t &data) {
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

			void set_resstutype(const ArgTypeInfo::StructTypeData &asti) {
				this->retdata.setstd(asti);
			}
			const ArgTypeInfo::StructTypeData& resstutype() const {
				assert(retdata.type != 0);
				return retdata.getstd();
			}
			void set_restype(ArgType at) {
				retdata.type = static_cast<uint8_t>(at);
			}
			void set_restype(const ArgTypeInfo::RetData &rd) {
				retdata = rd;
			}
			ArgType restype() const {
				return static_cast<ArgType>(retdata.type);
			}
			const ArgTypeInfo::RetData& get_retdata() const {
				return retdata;
			}

			//private:
			DataList list;
			unsigned int _int_count = 0;
			unsigned int _float_count = 0;
			unsigned int _memory_count = 0;
			ArgTypeInfo::RetData retdata;
		};

		void push_struct_data(ArgumentList &list, const void *t, const ArgTypeInfo::StructPassData &structinfo, unsigned int size) {
			unsigned int count = size > 8 ? 2 : 1;
			byte *p = (byte*)t;
			for (unsigned int i = 0; i < count; ++i) {
				ArgTypeInfo::OP type = structinfo.get(i);
				JitFFI::push_memory<uint64_t>(p, ((size > 8) ? 8 : size), [&](uint64_t v) { list.push(type, v); });
				p += 8;
				size -= 8;
			}
		}
		ArgumentList create_argumentlist(const ArgTypeInfo &ati, const ArgDataList &datalist) {
			ArgumentList list;
			auto iter = datalist.begin();
			if (ati.restype.type == AT_Memory) {
				list.push(ArgTypeInfo::op_void, 0);
			}
			list.set_restype(ati.restype);

			for (unsigned int i = 0; i != ati.size; ++i) {
				auto &type = ati.typelist.get()[i];
				switch (type.op) {
				case ArgTypeInfo::op_int:
					list.push(ArgTypeInfo::op_int, convert_uint64(*iter, type.size));
					break;
				case ArgTypeInfo::op_float:
					list.push(ArgTypeInfo::op_float, convert_uint64(*iter, type.size));
					break;
				case ArgTypeInfo::op_memory:
					list.push_memory(*iter, type.size);
					break;
				case ArgTypeInfo::op_struct: {
					push_struct_data(list, *iter, type.getspd(), type.size);
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
	}

	namespace SysV64
	{
		static void create_argument(JitFuncCallerCreater &jfcc, ArgumentList &list) {
			ArgTypeInfo::OP type;
			uint64_t data;

			jfcc.init_addarg_count(list.get_int_count(), list.get_float_count(), list.get_memory_count());

			while (list.get_next(type, data)) {
				switch (type) {
				case ArgTypeInfo::op_void:
					jfcc.add_void();
					break;
				case ArgTypeInfo::op_int:
					jfcc.add_int(data);
					break;
				case ArgTypeInfo::op_float:
					jfcc.add_double(data);
					break;
				case ArgTypeInfo::op_memory:
					jfcc.push(data);
					break;
				default:
					assert(false);
				}
			}
		}

		static void create_return_base(JitFuncCallerCreater &jfcc, ArgType type, unsigned int rec[]) {
			switch (type) {
			case AT_Void:
				break;
			case AT_Integer:
				if (rec[0] == 0) {
					OpCode_x64::mov_prbx_rax(jfcc.data());
					rec[0] = 1;
				}
				else {
					OpCode_x64::mov_prbx_rdx(jfcc.data());
				}
				break;
			case AT_SSE:
			case AT_SSEUP:
				if (rec[1] == 0) {
					OpCode_x64::mov_prbx_xmm0(jfcc.data());
					rec[1] = 1;
				}
				else {
					OpCode_x64::mov_prbx_xmm1(jfcc.data());
				}
				break;
			case AT_X87:
			case AT_X87UP:
				OpCode_x64::mov_prbx_st0(jfcc.data());
				break;
			case AT_ComplexX87:
				assert(false); // TODO
				OpCode_x64::mov_prbx_st0(jfcc.data());
				OpCode_x64::add_rbx_byte(jfcc.data(), 8);
				//OpCode_x64::mov_prbx_st1(jfcc.data());
				break;
			default:
				printf("%d\n", type);
				assert(false);
			}
		}

		static void create_return(JitFuncCallerCreater &jfcc, ArgumentList &list) {
			if (list.restype() != AT_Memory) {
				unsigned int rec[2] = { 0 }; // rax/rdx, xmm0/xmm1
				if (list.restype() == AT_Struct) {
					auto &rd = list.get_retdata();
					auto &st = list.resstutype();
					if (rd.size > 0) {
						create_return_base(jfcc, st.get(0), rec);
					}
					if (rd.size > 8) {
						OpCode_x64::add_rbx_byte(jfcc.data(), 8);
						create_return_base(jfcc, st.get(1), rec);
					}
				}
				else {
					create_return_base(jfcc, list.restype(), rec);
				}
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

			//printf("<");
			//for (auto &e : list.list) {
			//	printf("(%d 0x%llX)\n", e.type, e.data);
			//}
			//printf(">\n");

			create_function_caller(jfc, list, func);
		}
	}
}
