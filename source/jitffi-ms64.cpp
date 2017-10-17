#include "jitffi-ms64.h"
#include "opcode.h"
#include "jitffi-def.h"

namespace JitFFI
{
	namespace MS64
	{
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
			OpCode::call_func(jfc, func);
			OpCode_x64::add_rsp_byte(jfc, 0x20);
		}

		//

		inline bool need_pass_by_pointer(size_t n) {
			return (n != 1 && n != 2 && n != 4 && n != 8);
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

		void create_function_caller(JitFuncCreater &jfc, void *func, const ArgDataList &adlist, const ArgTypeList &atlist)
		{
			assert(adlist.size() == atlist.size());
			assert(adlist.size() < UINT32_MAX);

			ArgTypeInfo ati = create_argtypeinfo(atlist);
			ArgumentList list = create_argumentlist(ati, adlist);

			create_function_caller(jfc, list, func);
		}
	}
}
