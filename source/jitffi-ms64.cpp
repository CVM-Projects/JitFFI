#include "jitffi.h"
#include "jitffi-def.h"
#include "opcode.h"
#include "platform/argtype-ms64.h"

namespace JitFFI
{
	namespace MS64
	{
#define _define_atu_(name, type, at_type, size) const ArgTypeUnit atu_##name(at_type, size)

		_define_atu_(void, void, AT_Void, 0);
		_define_atu_(bool, bool, AT_Int, 1);

		_define_atu_(char, char, AT_Int, 1);
		_define_atu_(schar, signed char, AT_Int, 1);
		_define_atu_(uchar, unsigned char, AT_Int, 1);

		_define_atu_(int, int, AT_Int, 4);
		_define_atu_(short, short, AT_Int, 2);
		_define_atu_(lint, long int, AT_Int, 4);
		_define_atu_(llint, long long int, AT_Int, 8);

		_define_atu_(uint, unsigned int, AT_Int, 4);
		_define_atu_(ushort, unsigned short int, AT_Int, 2);
		_define_atu_(ulint, unsigned long int, AT_Int, 4);
		_define_atu_(ullint, unsigned long long int, AT_Int, 8);

		_define_atu_(float, float, AT_Float, 4);
		_define_atu_(double, double, AT_Float, 8);
		_define_atu_(ldouble, long double, AT_Float, 8);

		_define_atu_(pointer, void*, AT_Int, 8);

		_define_atu_(size, size_t, AT_Int, 8);

		_define_atu_(int8, int8_t, AT_Int, 1);
		_define_atu_(int16, int16_t, AT_Int, 2);
		_define_atu_(int32, int32_t, AT_Int, 4);
		_define_atu_(int64, int64_t, AT_Int, 8);

		_define_atu_(uint8, int8_t, AT_Int, 1);
		_define_atu_(uint16, int16_t, AT_Int, 2);
		_define_atu_(uint32, int32_t, AT_Int, 4);
		_define_atu_(uint64, int64_t, AT_Int, 8);

#undef _define_atu_
	}
}

namespace JitFFI
{
	namespace MS64
	{
		using namespace OpCode_x64;

		class JitFuncCallerCreaterPlatform : public JitFuncCallerCreater
		{
		public:
			template <typename _FTy>
			explicit JitFuncCallerCreaterPlatform(JitFuncCreater &jfc, _FTy *func = nullptr)
				: JitFuncCallerCreater(jfc, func) {}

			void init_addarg_count(unsigned int int_c, unsigned int dou_c, unsigned int mem_c = 0) {
				argn = int_c + dou_c + mem_c;
				have_init = true;
			}

			void add_int(uint64_t dat);
			void add_void();
			void add_int_uint32(uint32_t dat);
			void add_int_rbx();
			void add_int_prax();
			void add_double(uint64_t dat);
			void add_double_prax();

			void call();

		private:
			void _add_int(const std::function<OpHandler> &handler);
			void _add_double(const std::function<OpHandler> &handler);

			unsigned int argn;
			unsigned int add_count = 0;
		};

		void JitFuncCallerCreaterPlatform::add_int(uint64_t dat) {
			return _add_int([&](unsigned int c) { return OpCode_win64::add_int(jfc, dat, c); });
		}
		void JitFuncCallerCreaterPlatform::add_int_uint32(uint32_t dat) {
			return _add_int([&](unsigned int c) { return OpCode_win64::add_int_uint32(jfc, dat, c); });
		}
		void JitFuncCallerCreaterPlatform::add_int_rbx() {
			return _add_int([&](unsigned int c) { return OpCode_win64::add_int_rbx(jfc, c); });
		}
		void JitFuncCallerCreaterPlatform::add_int_prax() {
			return _add_int([&](unsigned int c) { return OpCode_win64::add_int_prax(jfc, c); });
		}
		void JitFuncCallerCreaterPlatform::add_double(uint64_t dat) {
			return _add_double([&](unsigned int c) { return OpCode_win64::add_double(jfc, dat, c); });
		}
		void JitFuncCallerCreaterPlatform::add_double_prax() {
			return _add_double([&](unsigned int c) { return OpCode_win64::add_double_prax(jfc, c); });
		}

		void JitFuncCallerCreaterPlatform::add_void() {
			assert(argn - add_count >= 0);
			++add_count;
		}
		void JitFuncCallerCreaterPlatform::_add_int(const std::function<OpHandler> &handler) {
			assert(argn - add_count >= 0);
			++add_count;
			push_count += handler(argn - add_count);
		}
		void JitFuncCallerCreaterPlatform::_add_double(const std::function<OpHandler> &handler) {
			assert(argn - add_count >= 0);
			++add_count;
			push_count += handler(argn - add_count);
		}

		void JitFuncCallerCreaterPlatform::call() {
			assert(have_init);
			assert(func);
			OpCode_x64::sub_rsp_byte(jfc, 0x20);
			OpCode_x64::call_func(jfc, func);
			OpCode_x64::add_rsp_byte(jfc, 0x20);
		}
	}
}

namespace JitFFI
{
	namespace MS64
	{
		struct ArgTypeInfo
		{
			enum OP : uint16_t
			{
				op_void,
				op_int,
				op_float,
				op_push,
				op_push_pointer,
			};
			using Size = uint16_t;
			using Data = std::pair<OP, Size>;

			explicit ArgTypeInfo(uint32_t num)
				: num(num), typelist(num) {}

			uint32_t num;
			Data retdata;
			UPtrValVector<Data> typelist;
		};

		constexpr int ix = sizeof(ArgTypeInfo);

		//==============================
		// * Get ArgTypeInfo
		//==============================

		static ArgTypeInfo::Data get_argtypeinfo_pass_data(const ArgTypeUnit &atu);
		static ArgTypeInfo::Data get_argtypeinfo_ret_data(const ArgTypeUnit &atu);

		static ArgTypeInfo create_argtypeinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist) {
			return JitFFI::create_argtypeinfo<ArgTypeInfo, ArgTypeInfo::Data, ArgTypeInfo::Data, get_argtypeinfo_pass_data, get_argtypeinfo_ret_data>(restype, atlist);
		}

		static bool need_pass_by_pointer(size_t n) {
			return (n != 1 && n != 2 && n != 4 && n != 8);
		}
		static bool need_return_by_pointer(size_t n) {
			return need_pass_by_pointer(n);
		}

		static ArgTypeInfo::Data get_argtypeinfo_pass_data(const ArgTypeUnit &atu) {
			switch (atu.type) {
			case AT_Int:
				return { ArgTypeInfo::op_int, atu.size };
			case AT_Float:
				return { ArgTypeInfo::op_float, atu.size };
				break;
			case AT_Memory:
				return { ArgTypeInfo::op_push_pointer, atu.size };
				break;
			case AT_Struct:
				if (need_pass_by_pointer(atu.size)) {
					return { ArgTypeInfo::op_push_pointer, atu.size };
				}
				else {
					return { ArgTypeInfo::op_int, atu.size };
				}
				break;
			default:
				assert(false);
			}
			return { ArgTypeInfo::op_int, 0xcccccccc };
		}

		static ArgTypeInfo::Data get_argtypeinfo_ret_data(const ArgTypeUnit &atu) {
			if (atu.type == AT_Void)
				return { ArgTypeInfo::op_void, 0 };
			else
				return get_argtypeinfo_pass_data(atu);
			if (atu.type == AT_Struct && need_return_by_pointer(atu.size)) {
				return { ArgTypeInfo::op_push_pointer, atu.size };
			}
			else {
				return { ArgTypeInfo::op_int, atu.size };
			}
		}
	}

	namespace MS64
	{
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
				list.push_front({ type, v });
			}
			unsigned int push_memory(uint64_t v) {
				memlist.push_front(v);
				return 1;
			}

			unsigned int push_memory(const void *dat, size_t size) {
				std::list<uint64_t> vlist;
				unsigned int r = JitFFI::push_memory<uint64_t>(dat, size, [&](uint64_t v) { vlist.push_front(v); });
				memlist.insert(memlist.end(), vlist.begin(), vlist.end());
				return r;
			}

			bool get_next_memory(uint64_t &data) {
				if (memlist.empty()) {
					return false;
				}
				else {
					data = memlist.front();
					memlist.pop_front();
					return true;
				}
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

			unsigned int size() const {
				assert(list.size() < UINT32_MAX);
				return static_cast<unsigned int>(list.size());
			}
			void set_retdata(const ArgTypeInfo::Data &rd) {
				retdata = rd;
			}
			const ArgTypeInfo::Data& get_retdata() const {
				return retdata;
			}

		private:
			DataList list;
			std::list<uint64_t> memlist;
			ArgTypeInfo::Data retdata;
		};


		static void create_argumentlist_base(ArgumentList &list, const ArgTypeInfo::Data &type, const void *data) {
			switch (type.first) {
			case ArgTypeInfo::op_int:
				list.push(ArgTypeInfo::op_int, convert_uint64(data, type.second));
				break;
			case ArgTypeInfo::op_float:
				list.push(ArgTypeInfo::op_float, convert_uint64(data, type.second));
				break;
			case ArgTypeInfo::op_push:
				list.push(ArgTypeInfo::op_push, convert_uint64(data, type.second));
				break;
			case ArgTypeInfo::op_push_pointer: {
				unsigned int offset = list.push_memory(data, type.second);
				list.push(ArgTypeInfo::op_push_pointer, offset);
				break;
			}
			default:
				assert(false);
				break;
			}
		}

		static ArgumentList create_argumentlist(const ArgTypeInfo &ati, const ArgDataList &datalist) {
			ArgumentList list;
			auto iter = datalist.begin();
			if (ati.retdata.first == ArgTypeInfo::op_push_pointer) {
				list.push(ArgTypeInfo::op_void, 0);
			}
			assert(ati.retdata.first == ArgTypeInfo::op_void || ati.retdata.second != 0);
			list.set_retdata(ati.retdata);
			for (unsigned int i = 0; i != ati.num; ++i) {
				auto &type = ati.typelist[i];
				auto &data = *iter;
				create_argumentlist_base(list, type, data);
				++iter;
			}
			return list;
		}
	}

	namespace MS64
	{
		//==============================
		// * Create
		//==============================

		static void create_argument(JitFuncCallerCreater &jfcc, ArgumentList &list) {
			ArgTypeInfo::OP type;
			uint64_t data;

			jfcc.init_addarg_count(list.size(), 0, 0);

			while (list.get_next_memory(data)) {
				jfcc.push(data);
			}

			jfcc.mov_rbx_rsp();

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
				case ArgTypeInfo::op_push_pointer:
					jfcc.add_int_rbx();
					assert(data * 8 < UINT32_MAX);
					jfcc.add_rbx(static_cast<uint32_t>(data * 8));
					break;
				default:
					assert(false);
				}
			}
		}

		static void create_return(JitFuncCallerCreater &jfcc, const ArgTypeInfo::Data &retdata) {
			if (retdata.first != ArgTypeInfo::op_push_pointer && retdata.first != ArgTypeInfo::op_void) {
				switch (retdata.second) {
				case 8: OpCode_x64::mov_prbx_rax(jfcc.data()); break;
				case 4: OpCode_x64::mov_prbx_eax(jfcc.data()); break;
				case 2: OpCode_x64::mov_prbx_ax(jfcc.data());  break;
				case 1: OpCode_x64::mov_prbx_al(jfcc.data());  break;
				default: printf("%d\n", retdata.first); assert(false);
				}
			}
		}
	}

	namespace MS64
	{
		// 

		ArgumentInfo get_argumentinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist) {
			ArgTypeInfo *p_ati = new ArgTypeInfo(create_argtypeinfo(restype, atlist));
			return ArgumentInfo(P_MS64, p_ati);
		}
		static const ArgTypeInfo& get_argtypeinfo(const ArgumentInfo &argumentinfo) {
			return argumentinfo.data<P_MS64, ArgTypeInfo>();
		}

		static void create_function_caller_head(JitFuncCallerCreater &jfcc) {
			OpCode_x64::push(jfcc.data(), rbx);
			OpCode_x64::push(jfcc.data(), r12);
			jfcc.sub_rsp();
		}
		static void create_function_caller_foot(JitFuncCallerCreater &jfcc) {
			jfcc.add_rsp();
			OpCode_x64::pop(jfcc.data(), r12);
			OpCode_x64::pop(jfcc.data(), rbx);
			jfcc.ret();
		}

		static void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, void *func)
		{
			JitFuncCallerCreaterPlatform jfcc(jfc, func);
			create_function_caller_head(jfcc);

			OpCode_x64::mov(jfc, r12, rcx);
			create_argument(jfcc, list);

			jfcc.call();

			OpCode_x64::mov(jfc, rbx, r12);
			create_return(jfcc, list.get_retdata());

			create_function_caller_foot(jfcc);
		}

		void create_function_caller(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, void *func, const ArgDataList &adlist)
		{
			assert(adlist.size() < UINT32_MAX);

			ArgumentList list = create_argumentlist(get_argtypeinfo(argumentinfo), adlist);

			create_function_caller(jfc, list, func);
		}
	}
}
