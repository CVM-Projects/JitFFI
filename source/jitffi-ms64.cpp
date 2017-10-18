#include "jitffi.h"
#include "jitffi-def.h"
#include "opcode.h"

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
				return JitFFI::push_memory<uint64_t>(dat, size, [&](uint64_t v) { push_memory(v); });
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

		private:
			DataList list;
			std::list<uint64_t> memlist;
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
		};

		//

		inline bool need_pass_by_pointer(size_t n) {
			return (n != 1 && n != 2 && n != 4 && n != 8);
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

		ArgTypeInfo create_argtypeinfo(const ArgTypeList &atlist)
		{
			ArgTypeInfo ati;
			for (auto &type : atlist) {
				ati.typelist.push_back(get_argtypeinfo_data(*type));
			}
			return ati;
		}

		ArgumentList create_argumentlist(const ArgTypeInfo &ati, const ArgDataList &datalist) {
			ArgumentList list;
			auto iter = datalist.begin();
			assert(ati.typelist.size() == datalist.size());
			for (auto &type : ati.typelist) {
				switch (type.first) {
				case ArgTypeInfo::op_int:
					list.push(AT_Int, convert_uint64(*iter, type.second));
					break;
				case ArgTypeInfo::op_float:
					list.push(AT_Float, convert_uint64(*iter, type.second));
					break;
				case ArgTypeInfo::op_push:
					list.push(AT_Int, convert_uint64(*iter, type.second));
					break;
				case ArgTypeInfo::op_push_pointer: {
					unsigned int offset = list.push_memory(*iter, type.second);
					list.push(AT_Memory, offset);
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

		ArgumentInfo get_argumentinfo(const ArgTypeList &atlist) {
			ArgTypeInfo *p_ati = new ArgTypeInfo(create_argtypeinfo(atlist));
			return ArgumentInfo(P_MS64, p_ati);
		}
		const ArgTypeInfo& get_argtypeinfo(const ArgumentInfo &argumentinfo) {
			return argumentinfo.data<P_MS64, ArgTypeInfo>();
		}

		void create_function_caller(JitFuncCreater &jfc, ArgumentList &list, void *func)
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

		void create_function_caller(JitFuncCreater &jfc, void *func, const ArgumentInfo &argumentinfo, const ArgDataList &adlist)
		{
			assert(adlist.size() < UINT32_MAX);

			ArgumentList list = create_argumentlist(get_argtypeinfo(argumentinfo), adlist);

			create_function_caller(jfc, list, func);
		}
	}
}
