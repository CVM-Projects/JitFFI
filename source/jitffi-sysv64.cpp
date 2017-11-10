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
		using namespace OpCode_x64;

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
			void add_int_prax();
			void add_double(uint64_t dat);
			void add_double_prax();

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
		void JitFuncCallerCreaterPlatform::add_int_prax() {
			return _add_int([&](unsigned int c) { return OpCode_sysv64::add_int_prax(jfc, c); });
		}
		void JitFuncCallerCreaterPlatform::add_double(uint64_t dat) {
			return _add_double([&](unsigned int c) { return OpCode_sysv64::add_double(jfc, dat, c); });
		}
		void JitFuncCallerCreaterPlatform::add_double_prax() {
			return _add_double([&](unsigned int c) { return OpCode_sysv64::add_double_prax(jfc, c); });
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

			explicit ArgTypeInfo(uint32_t num)
				: num(num), typelist(num) {}

			uint32_t num;
			RetData retdata;
			UPtrValVector<PassData> typelist;
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
		struct ArgOPCounter
		{
			void add(ArgTypeInfo::OP type) {
				switch (type) {
				case ArgTypeInfo::op_void: int_count++; break;
				case ArgTypeInfo::op_int: int_count++; break;
				case ArgTypeInfo::op_float: float_count++; break;
				case ArgTypeInfo::op_memory: memory_count++; break;
				default: assert(false);
				}
			}

			unsigned int int_count = 0;
			unsigned int float_count = 0;
			unsigned int memory_count = 0;
		};

		class ArgOPList
		{
			struct Data {
				ArgTypeInfo::OP op;
				unsigned int id;
				unsigned int num;
			};
		public:
			explicit ArgOPList(unsigned int num)
				: _num(num) {}

			void add(unsigned int id, ArgTypeInfo::OP op, unsigned int num = 0) {
				assert(op != ArgTypeInfo::op_struct);
				counter.add(op);
				datalist.add({ op, id, num });
			}
			void add_stu(unsigned int id, const ArgTypeInfo::StructPassData &spd, unsigned int size) {
				if (size > 0)
					add(id, spd.get(0), (size > 8 ? 1 : 0));
				if (size > 8)
					add(id, spd.get(1), (size > 8 ? 1 : 0));
			}
			void add_mem(unsigned int id) {
				add(id, ArgTypeInfo::op_memory, 0);
			}
			void add_mem(unsigned int id, size_t size) {
				unsigned int count = get_value_count(size, 8);
				unsigned int num = count;
				while (count--) {
					add(id, ArgTypeInfo::op_memory, num);
				}
			}

			bool do_next(const std::function<void(ArgTypeInfo::OP, unsigned int num)> &f) {
				Data data;
				bool v = datalist.get_next(data);
				if (v == false)
					return false;
				f(data.op, data.num);
				return true;
			}

			unsigned int get_int_count() const {
				return counter.int_count;
			}
			unsigned int get_float_count() const {
				return counter.float_count;
			}
			unsigned int get_memory_count() const {
				return counter.memory_count;
			}
			unsigned int num() const {
				return _num;
			}

			const ArgTypeInfo::RetData& get_retdata() const {
				return retdata;
			}
			void set_retdata(const ArgTypeInfo::RetData &rd) {
				retdata = rd;
			}

		private:
			ArgOPCounter counter;
			const unsigned int _num;
			StackList<Data> datalist;
			ArgTypeInfo::RetData retdata;
		};

		class ArgumentList
		{
			struct Data {
				ArgTypeInfo::OP op;
				uint64_t data;
			};

		public:
			ArgumentList() = default;

			void push(ArgTypeInfo::OP op, uint64_t v) {
				counter.add(op);
				datalist.add({ op, v });
			}

			void push_memory(uint64_t v) {
				counter.add(ArgTypeInfo::op_memory);
				datalist.add({ ArgTypeInfo::op_memory, v });
			}

			void push_memory(const void *dat, size_t size) {
				JitFFI::push_memory<uint64_t>(dat, size, [&](uint64_t v) { push_memory(v); });
			}

			bool get_next(ArgTypeInfo::OP &op, uint64_t &data) {
				Data d;
				bool v = datalist.get_next(d);
				op = d.op;
				data = d.data;
				return v;
			}

			unsigned int get_int_count() const {
				return counter.int_count;
			}
			unsigned int get_float_count() const {
				return counter.float_count;
			}
			unsigned int get_memory_count() const {
				return counter.memory_count;
			}

			const ArgTypeInfo::RetData& get_retdata() const {
				return retdata;
			}
			void set_retdata(const ArgTypeInfo::RetData &rd) {
				retdata = rd;
			}

		private:
			StackList<Data> datalist;
			ArgOPCounter counter;
			ArgTypeInfo::RetData retdata;
			unsigned int count = 0;
		};

		//==============================
		// * Get ArgOPList
		//==============================

		static ArgOPList create_argoplist(const ArgTypeInfo &ati) {
			ArgOPList list(ati.num);
			unsigned int id = 0;
			//if (ati.retdata.type == AT_Memory) {
			//	list.add(id, ArgTypeInfo::op_void);
			//}
			list.set_retdata(ati.retdata);

			for (unsigned int i = 0; i != ati.num; i++, id++) {
				auto &type = ati.typelist[i];
				switch (type.op) {
				case ArgTypeInfo::op_int:
					list.add(id, ArgTypeInfo::op_int);
					break;
				case ArgTypeInfo::op_float:
					list.add(id, ArgTypeInfo::op_float);
					break;
				case ArgTypeInfo::op_memory:
					list.add_mem(id, type.size);
					break;
				case ArgTypeInfo::op_struct:
					list.add_stu(id, type.getspd(), type.size);
					break;
				default:
					assert(false);
					break;
				}
			}
			return list;
		}

		//==============================
		// * Get ArgumentList
		//==============================

		static void push_struct_data(ArgumentList &list, const void *t, const ArgTypeInfo::StructPassData &structinfo, unsigned int size) {
			assert(size <= 16);
			if (size > 0) {
				list.push(structinfo.get(0), convert_uint64(t, std::min<unsigned>(size, 8)));
			}
			if (size > 8) {
				list.push(structinfo.get(1), convert_uint64((byte*)t + 8, size - 8));
			}
		}
		static ArgumentList create_argumentlist(const ArgTypeInfo &ati, const ArgDataList &datalist) {
			ArgumentList list;
			auto iter = datalist.begin();
			if (ati.retdata.type == AT_Memory) {
				list.push(ArgTypeInfo::op_void, 0);
			}
			list.set_retdata(ati.retdata);

			for (unsigned int i = 0; i != ati.num; ++i) {
				auto &type = ati.typelist[i];
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
		//==============================
		// * Create
		//==============================

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
					printf("%d\n", type);
					assert(false);
				}
			}
		}

		static void create_argument(JitFuncCallerCreater &jfcc, ArgOPList &list) {
			ArgTypeInfo::OP type;
			uint64_t data;

			jfcc.init_addarg_count(list.get_int_count(), list.get_float_count(), list.get_memory_count());

			/* TODO */ OpCode_x64::mov(jfcc.data(), rbx, rsi);

			if (list.get_retdata().type == ArgTypeInfo::op_memory) {
				jfcc.add_void();
			}

			OpCode_x64::add_rbx_uint32(jfcc.data(), (list.num() - 1) * 8);

			bool is_first = true;
			unsigned int rnum = 0;

			while (list.do_next([&](ArgTypeInfo::OP op, unsigned int num) {
				printf("<%d %d>\n", op, num);
				if (rnum == 0) {
					rnum = num + 1;
					OpCode_x64::mov(jfcc.data(), rax, prbx);
					OpCode_x64::sub_rbx_byte(jfcc.data(), 8);
					OpCode_x64::add_rax_uint32(jfcc.data(), num * 8);
				}
				switch (op) {
				case ArgTypeInfo::op_int:
					jfcc.add_int_prax();
					break;
				case ArgTypeInfo::op_float:
					jfcc.add_double_prax();
					break;
				case ArgTypeInfo::op_memory:
					jfcc.push_prax();
					break;
				default:
					printf("%d\n", type);
					assert(false);
				}
				if (rnum != 0) {
					rnum--;
					OpCode_x64::sub_rax_byte(jfcc.data(), 8);
				}
			}));
		}

		static void create_return_base(JitFuncCallerCreater &jfcc, ArgType type, unsigned int size, unsigned int rec[]);
		static void create_return(JitFuncCallerCreater &jfcc, const ArgTypeInfo::RetData &retdata) {
			if (retdata.type != AT_Memory) {
				unsigned int rec[2] = { 0 }; // rax/rdx, xmm0/xmm1
				if (retdata.type == AT_Struct) {
					auto &st = retdata.getstd();
					if (retdata.size > 0) {
						create_return_base(jfcc, st.get(0), std::min<unsigned>(retdata.size, 8), rec);
					}
					if (retdata.size > 8) {
						OpCode_x64::add_rbx_byte(jfcc.data(), 8);
						create_return_base(jfcc, st.get(1), retdata.size - 8, rec);
					}
				}
				else {
					create_return_base(jfcc, static_cast<ArgType>(retdata.type), retdata.size, rec);
				}
			}
		}

		static void create_return_copy(JitFuncCallerCreater &jfcc, unsigned int size) {
			OpCode_x64::mov(jfcc.data(), rsi, rsp);
			OpCode_x64::mov(jfcc.data(), rdi, rbx);
			OpCode_x64::mov_rcx_uint32(jfcc.data(), size);
			OpCode_x64::movsb_prdi_prsi_rep(jfcc.data());
		}

		static void create_return_base(JitFuncCallerCreater &jfcc, ArgType type, unsigned int size, unsigned int rec[]) {
			switch (type) {
			case AT_Void:
				break;
			case AT_Integer:
				if (rec[0] == 0) {
					if (size == 8) {
						OpCode_x64::mov_prbx_rax(jfcc.data());
					}
					else {
						OpCode_x64::push_rax(jfcc.data());
						create_return_copy(jfcc, size);
						OpCode_x64::pop_rax(jfcc.data());
					}
					rec[0] = 1;
				}
				else {
					if (size == 8) {
						OpCode_x64::mov_prbx_rdx(jfcc.data());
					}
					else {
						OpCode_x64::push_rdx(jfcc.data());
						create_return_copy(jfcc, size);
						OpCode_x64::pop_rdx(jfcc.data());
					}
				}
				break;
			case AT_SSE:
			case AT_SSEUP:
				if (rec[1] == 0) {
					if (size == 8) {
						OpCode_x64::movsd_prbx_xmm0(jfcc.data());
					}
					else {
						assert(size == 4);
						OpCode_x64::movss_prbx_xmm0(jfcc.data());
					}
					rec[1] = 1;
				}
				else {
					if (size == 8) {
						OpCode_x64::movsd_prbx_xmm1(jfcc.data());
					}
					else {
						assert(size == 4);
						OpCode_x64::movss_prbx_xmm1(jfcc.data());
					}
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
				OpCode_x64::mov_prbx_st0(jfcc.data());
				break;
			default:
				printf("%d\n", type);
				assert(false);
			}
		}
	}

	namespace SysV64
	{
		//

		ArgumentInfo get_argumentinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist) {
			ArgTypeInfo *p_ati = new ArgTypeInfo(create_argtypeinfo(restype, atlist));
			return ArgumentInfo(P_SysV64, p_ati);
		}
		static const ArgTypeInfo& get_argtypeinfo(const ArgumentInfo &argumentinfo) {
			return argumentinfo.data<P_SysV64, ArgTypeInfo>();
		}

		static void create_function_caller_head(JitFuncCallerCreater &jfcc) {
			OpCode_x64::push_rbx(jfcc.data());
			OpCode_x64::push_r12(jfcc.data());

			//OpCode_x64::sub_rsp_byte(jfcc.data(), 0x8);
			jfcc.sub_rsp();
		}
		static void create_function_caller_foot(JitFuncCallerCreater &jfcc) {
			jfcc.add_rsp();
			//OpCode_x64::add_rsp_byte(jfcc.data(), 0x8);
			OpCode_x64::pop_r12(jfcc.data());
			OpCode_x64::pop_rbx(jfcc.data());
			jfcc.ret();
		}

		static void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, void *func)
		{
			JitFuncCallerCreaterPlatform jfcc(jfc, func);
			create_function_caller_head(jfcc);

			OpCode_x64::mov(jfc, rbx, rdi);
			create_argument(jfcc, list);

			jfcc.call();

			create_return(jfcc, list.get_retdata());

			create_function_caller_foot(jfcc);
		}

		void create_function_caller(JitFuncCreater &jfc, void *func, const ArgumentInfo &argumentinfo)
		{
			ArgOPList aol = create_argoplist(get_argtypeinfo(argumentinfo));


			JitFuncCallerCreaterPlatform jfcc(jfc, func);
			create_function_caller_head(jfcc);

			OpCode_x64::mov(jfc, r12, rdi);
			create_argument(jfcc, aol);

			jfcc.call();

			OpCode_x64::mov(jfc, rbx, r12);
			create_return(jfcc, aol.get_retdata());

			create_function_caller_foot(jfcc);

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
