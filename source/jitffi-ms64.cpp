#include "jitffi.h"
#include "jitffi-def.h"
#include "opcode.h"
#include "platform/argtype-ms64.h"
#include <cstdio>

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

		inline unsigned int add_int(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			switch (count) {
			case 0:  movq_u64(jfc, rcx, dat); return 0;
			case 1:  movq_u64(jfc, rdx, dat); return 0;
			case 2:  movq_u64(jfc, r8, dat); return 0;
			case 3:  movq_u64(jfc, r9, dat); return 0;
			default:
				movq_u64(jfc, rax, dat);
				push(jfc, rax);
				return 1;
			}
		}
		inline unsigned int add_int_uint32(JitFuncCreater &jfc, uint32_t dat, unsigned int count) {
			switch (count) {
			case 0:  movd_u32(jfc, ecx, dat); return 0;
			case 1:  movd_u32(jfc, edx, dat); return 0;
			case 2:  movd_u32(jfc, r8d, dat); return 0;
			case 3:  movd_u32(jfc, r9d, dat); return 0;
			default: push_u32(jfc, dat); return 1;
			}
		}
		inline unsigned int add_int_reg(JitFuncCreater &jfc, Register r, unsigned int count) {
			switch (count) {
			case 0:  mov(jfc, rcx, r); return 0;
			case 1:  mov(jfc, rdx, r); return 0;
			case 2:  mov(jfc, r8, r); return 0;
			case 3:  mov(jfc, r9, r); return 0;
			default: push(jfc, r); return 1;
			}
		}

		// add_double_n: (n : 0 ~ 3)
		//     mov rax, dat    ; dat : convert(value)
		//     mov xmm_n, rax

		inline unsigned int add_double_reg(JitFuncCreater &jfc, Register r, unsigned int count) {
			auto f = is_prx(r) ? movsd : mov;
			switch (count) {
			case 0:  f(jfc, xmm0, r); return 0;
			case 1:  f(jfc, xmm1, r); return 0;
			case 2:  f(jfc, xmm2, r); return 0;
			case 3:  f(jfc, xmm3, r); return 0;
			default: push(jfc, r); return 1;
			}
		}
		inline unsigned int add_double(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			movq_u64(jfc, rax, dat);
			return add_double_reg(jfc, rax, count);
		}
	}

	namespace MS64
	{
		using namespace OpCode_x64;

		class JitFuncCallerCreater : public JitFuncCallerCreaterX64
		{
		public:
			explicit JitFuncCallerCreater(JitFuncCreater &jfc)
				: JitFuncCallerCreaterX64(jfc) {}

			void init_addarg_count(unsigned int argn) {
				this->argn = argn;
#ifndef NDEBUG
				have_init = true;
#endif
			}

			void add_void();
			void add_int(uint64_t dat);
			void add_int_uint32(uint32_t dat);
			void add_int_reg(Register r);
			void add_double(uint64_t dat);
			void add_double_reg(Register r);

			void call(void *func);

		private:
			void _add_int(const std::function<OpHandler> &handler);
			void _add_double(const std::function<OpHandler> &handler);

			unsigned int argn;
			unsigned int add_count = 0;
		};

		void JitFuncCallerCreater::add_int(uint64_t dat) {
			return _add_int([&](unsigned int c) { return MS64::add_int(data, dat, c); });
		}
		void JitFuncCallerCreater::add_int_uint32(uint32_t dat) {
			return _add_int([&](unsigned int c) { return MS64::add_int_uint32(data, dat, c); });
		}
		void JitFuncCallerCreater::add_int_reg(Register r) {
			return _add_int([&](unsigned int c) { return MS64::add_int_reg(data, r, c); });
		}
		void JitFuncCallerCreater::add_double(uint64_t dat) {
			return _add_double([&](unsigned int c) { return MS64::add_double(data, dat, c); });
		}
		void JitFuncCallerCreater::add_double_reg(Register r) {
			return _add_double([&](unsigned int c) { return MS64::add_double_reg(data, r, c); });
		}

		void JitFuncCallerCreater::add_void() {
			assert(argn - add_count >= 0);
			++add_count;
		}
		void JitFuncCallerCreater::_add_int(const std::function<OpHandler> &handler) {
			assert(argn - add_count >= 0);
			++add_count;
			push_count += handler(argn - add_count);
		}
		void JitFuncCallerCreater::_add_double(const std::function<OpHandler> &handler) {
			assert(argn - add_count >= 0);
			++add_count;
			push_count += handler(argn - add_count);
		}

		void JitFuncCallerCreater::call(void *func) {
			assert(have_init);
			assert(func);
			sub_rx_byte(data, rsp, 0x20);
			call_func(func);
			add_rx_byte(data, rsp, 0x20);
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
			ArgTypeInfo::Data retdata;
			if (atu.type == AT_Void)
				retdata = { ArgTypeInfo::op_void, 0 };
			else
				return get_argtypeinfo_pass_data(atu);
			if (atu.type == AT_Struct && need_return_by_pointer(atu.size)) {
				retdata = { ArgTypeInfo::op_push_pointer, atu.size };
			}
			else {
				retdata = { ArgTypeInfo::op_int, atu.size };
			}
			assert(retdata.first == ArgTypeInfo::op_void || retdata.second != 0);
			return retdata;
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

		static void create_argumentlist_base(JitFuncCallerCreater &jfcc, const ArgTypeInfo::Data &type);

		static void create_argument(JitFuncCallerCreater &jfcc, const ArgTypeInfo &ati) {
			assert(ati.num < UINT32_MAX);
			for (unsigned int i = 0; i != ati.num; ++i) {
				auto &type = ati.typelist[i];
				if (type.first == ArgTypeInfo::OP::op_push_pointer) {
					mov_offset_u32(jfcc.data, rax, prbx, i * 8);
					unsigned int offset = get_value_count(type.second, 8);
					add_rx_u32(jfcc.data, rax, 8 * (offset - 1));
					while (offset--) {
						jfcc.push_reg(prax);
						sub_rx_byte(jfcc.data, rax, 8);
					}
				}
			}
			mov(jfcc.data, rbp, rsp);
			unsigned int num = ati.num;
			if (ati.retdata.first == ArgTypeInfo::op_push_pointer) {
				num++;
			}
			jfcc.init_addarg_count(num);
			if (ati.num > 1)
				add_rx_u32(jfcc.data, rbx, 8 * (ati.num - 1));
			for (unsigned int i = ati.num - 1; i != unsigned(-1); --i) {
				auto &type = ati.typelist[i];
				create_argumentlist_base(jfcc, type);
				sub_rx_byte(jfcc.data, rbx, 8);
			}
			if (ati.retdata.first == ArgTypeInfo::op_push_pointer) {
				jfcc.add_void();
			}
		}
		static void create_argumentlist_base(JitFuncCallerCreater &jfcc, const ArgTypeInfo::Data &type) {
			switch (type.first) {
			case ArgTypeInfo::op_int:
				mov(jfcc.data, rax, prbx);
				jfcc.add_int_reg(prax);
				break;
			case ArgTypeInfo::op_float:
				mov(jfcc.data, rax, prbx);
				jfcc.add_double_reg(prax);
				break;
			case ArgTypeInfo::op_push:
				mov(jfcc.data, rax, prbx);
				jfcc.push_reg(prax);
				break;
			case ArgTypeInfo::op_push_pointer: {
				jfcc.data.push(0x90);
				jfcc.add_int_reg(rbp);
				unsigned int offset = get_value_count(type.second, 8);
				add_rx_u32(jfcc.data, rbp, 8 * offset);
				jfcc.data.push(0x90);
				break;
			}
			default:
				assert(false);
				break;
			}
		}

		static void create_argument(JitFuncCallerCreater &jfcc, ArgumentList &list) {
			ArgTypeInfo::OP type;
			uint64_t data;

			jfcc.init_addarg_count(list.size());

			while (list.get_next_memory(data)) {
				jfcc.push(data);
			}

			mov(jfcc.data, rbp, rsp);

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
					jfcc.add_int_reg(rbp);
					assert(data * 8 < UINT32_MAX);
					add_rx_u32(jfcc.data, rbp, static_cast<uint32_t>(data * 8));
					break;
				default:
					assert(false);
				}
			}
		}

		static void create_return(JitFuncCallerCreater &jfcc, const ArgTypeInfo::Data &retdata) {
			Register dst = prbx;
			if (retdata.first != ArgTypeInfo::op_push_pointer && retdata.first != ArgTypeInfo::op_void) {
				if (retdata.first == ArgTypeInfo::op_int) {
					switch (retdata.second) {
					case 8: mov(jfcc.data, dst, rax); break;
					case 4: mov(jfcc.data, dst, eax); break;
					case 2: mov(jfcc.data, dst, ax);  break;
					case 1: mov(jfcc.data, dst, al);  break;
					default: printf("%d\n", retdata.first); assert(false);
					}
				}
				else if (retdata.first == ArgTypeInfo::op_float) {
					switch (retdata.second) {
					case 8: movsd(jfcc.data, dst, xmm0); break;
					case 4: movss(jfcc.data, dst, xmm0); break;
					default: printf("%d\n", retdata.first); assert(false);
					}
				}
			}
		}
	}

	namespace MS64
	{
		// 

		ArgumentInfo GetArgInfo(const ArgTypeUnit &restype, const ArgTypeList &atlist) {
			ArgTypeInfo *p_ati = new ArgTypeInfo(create_argtypeinfo(restype, atlist));
			return ArgumentInfo(P_MS64, p_ati);
		}
		static const ArgTypeInfo& get_argtypeinfo(const ArgumentInfo &argumentinfo) {
			return argumentinfo.data<P_MS64, ArgTypeInfo>();
		}

		static void create_function_caller_head(JitFuncCallerCreater &jfcc) {
			push(jfcc.data, rbx);
			push(jfcc.data, rbp);
			push(jfcc.data, r12);
			sub_rx_byte(jfcc.data, rsp, 0x8);
			jfcc.sub_rsp();
		}
		static void create_function_caller_foot(JitFuncCallerCreater &jfcc) {
			jfcc.add_rsp();
			add_rx_byte(jfcc.data, rsp, 0x8);
			pop(jfcc.data, r12);
			pop(jfcc.data, rbp);
			pop(jfcc.data, rbx);
			ret(jfcc.data);
		}

		void CreateCaller(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, void *func)
		{
			const ArgTypeInfo &ati = get_argtypeinfo(argumentinfo);

			JitFuncCallerCreater jfcc(jfc);
			create_function_caller_head(jfcc);

			mov(jfc, r12, rcx);
			mov(jfc, rbx, rdx);
			create_argument(jfcc, ati);

			jfcc.call(func);

			mov(jfc, rbx, r12);
			create_return(jfcc, ati.retdata);

			create_function_caller_foot(jfcc);
		}

		void CreateCaller(JitFuncCreater &jfc, const ArgumentInfo &argumentinfo, void *func, const ArgDataList &adlist)
		{
			assert(adlist.size() < UINT32_MAX);

			ArgumentList list = create_argumentlist(get_argtypeinfo(argumentinfo), adlist);

			JitFuncCallerCreater jfcc(jfc);
			create_function_caller_head(jfcc);

			mov(jfc, r12, rcx);
			create_argument(jfcc, list);

			jfcc.call(func);

			mov(jfc, rbx, r12);
			create_return(jfcc, list.get_retdata());

			create_function_caller_foot(jfcc);
		}
	}
}
