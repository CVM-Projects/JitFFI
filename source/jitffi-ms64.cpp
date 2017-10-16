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
		void push_struct_data_base(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			unsigned int offset = list.push_memory(t, atu.size);
			list.push(AT_Memory, offset);
		}

		void push_struct_data(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			if (need_pass_by_pointer(atu.size)) {
				push_struct_data_base(list, t, atu);
			}
			else {
				list.push(atu.type == AT_Struct ? AT_Int : atu.type, convert_uint64(t, atu.size));
			}
		}

		void push_data(ArgumentList &list, void *t, const ArgTypeUnit &atu) {
			switch (atu.type) {
			case AT_Struct:
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
