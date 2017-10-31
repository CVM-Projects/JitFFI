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
			void add_double(uint64_t dat);

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
		void JitFuncCallerCreaterPlatform::add_double(uint64_t dat) {
			return _add_double([&](unsigned int c) { return OpCode_win64::add_double(jfc, dat, c); });
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

			unsigned int size() const {
				assert(list.size() < UINT32_MAX);
				return static_cast<unsigned int>(list.size());
			}
			void set_ressize(unsigned int size) {
				_ressize = size;
			}
			bool return_by_pointer() const {
				return _ressize == 0;
			}
			unsigned int ressize() const {
				return _ressize;
			}

		private:
			DataList list;
			std::list<uint64_t> memlist;
			unsigned int _ressize = 0;
		};

		struct ArgTypeInfo
		{
			enum OP
			{
				op_int,
				op_float,
				op_push,
				op_push_pointer,
			};
			using Size = unsigned int;
			using Data = std::pair<OP, Size>;

			std::vector<Data> typelist;
			Data restype;
		};

		//

		inline bool need_pass_by_pointer(size_t n) {
			return (n != 1 && n != 2 && n != 4 && n != 8);
		}
		inline bool need_return_by_pointer(size_t n) {
			return need_pass_by_pointer(n);
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

		//

		ArgTypeInfo create_argtypeinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist)
		{
			ArgTypeInfo ati;
			if (restype.type != AT_Void && need_return_by_pointer(restype.size)) {
				ati.restype = { ArgTypeInfo::op_push_pointer, restype.size };
			}
			else {
				ati.restype = { ArgTypeInfo::op_int, restype.size };
			}
			for (auto &type : atlist) {
				ati.typelist.push_back(get_argtypeinfo_data(*type));
			}
			return ati;
		}

		void create_argumentlist_base(ArgumentList &list, const ArgTypeInfo::Data &type, const void *data) {
			switch (type.first) {
			case ArgTypeInfo::op_int:
				list.push(AT_Int, convert_uint64(data, type.second));
				break;
			case ArgTypeInfo::op_float:
				list.push(AT_Float, convert_uint64(data, type.second));
				break;
			case ArgTypeInfo::op_push:
				list.push(AT_Int, convert_uint64(data, type.second));
				break;
			case ArgTypeInfo::op_push_pointer: {
				unsigned int offset = list.push_memory(data, type.second);
				list.push(AT_Memory, offset);
				break;
			}
			default:
				assert(false);
				break;
			}
		}

		ArgumentList create_argumentlist(const ArgTypeInfo &ati, const ArgDataList &datalist) {
			ArgumentList list;
			auto iter = datalist.begin();
			assert(ati.typelist.size() == datalist.size());
			if (ati.restype.first == ArgTypeInfo::op_push_pointer) {
				list.push(AT_Void, 0);
				assert(ati.restype.second != 0);
			}
			else {
				list.set_ressize(ati.restype.second);
			}
			for (auto &type : ati.typelist) {
				create_argumentlist_base(list, type, *iter);
				++iter;
			}
			return list;
		}

		void create_argument(JitFuncCallerCreater &jfcc, ArgumentList &list) {
			ArgType type;
			uint64_t data;

			jfcc.init_addarg_count(list.size(), 0, 0);

			while (list.get_next_memory(data)) {
				jfcc.push(data);
			}

			jfcc.mov_rbx_rsp();

			while (list.get_next(type, data)) {
				switch (type) {
				case AT_Void:
					jfcc.add_void();
					break;
				case AT_Int:
					jfcc.add_int(data);
					break;
				case AT_Float:
					jfcc.add_double(data);
					break;
				case AT_Memory:
					jfcc.add_int_rbx();
					assert(data * 8 < UINT32_MAX);
					jfcc.add_rbx(static_cast<uint32_t>(data * 8));
					break;
				default:
					assert(false);
				}
			}
		}

		void create_return(JitFuncCallerCreater &jfcc, ArgumentList &list) {
			if (!list.return_by_pointer()) {
				switch (list.ressize()) {
				case 8: OpCode_x64::mov_prcx_rax(jfcc.data()); break;
				case 4: OpCode_x64::mov_prcx_eax(jfcc.data()); break;
				case 2: OpCode_x64::mov_prcx_ax(jfcc.data());  break;
				case 1: OpCode_x64::mov_prcx_al(jfcc.data());  break;
				default: assert(false);
				}
			}
		}

		ArgumentInfo get_argumentinfo(const ArgTypeUnit &restype, const ArgTypeList &atlist) {
			ArgTypeInfo *p_ati = new ArgTypeInfo(create_argtypeinfo(restype, atlist));
			return ArgumentInfo(P_MS64, p_ati);
		}
		const ArgTypeInfo& get_argtypeinfo(const ArgumentInfo &argumentinfo) {
			return argumentinfo.data<P_MS64, ArgTypeInfo>();
		}

		void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, void *func)
		{
			JitFuncCallerCreaterPlatform jfcc(jfc, func);
			OpCode_x64::push_rbx(jfc);
			OpCode_x64::push_rdi(jfc);
			byte &v = jfcc.sub_rsp_unadjusted();

			OpCode_x64::mov_rdi_rcx(jfc);
			create_argument(jfcc, list);

			jfcc.call();

			OpCode_x64::mov_rcx_rdi(jfc);
			create_return(jfcc, list);

			jfcc.add_rsp();
			jfcc.adjust_sub_rsp(v);
			OpCode_x64::pop_rdi(jfc);
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
