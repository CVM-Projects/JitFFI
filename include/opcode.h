#pragma once
#include "jitffi.h"
using byte = uint8_t;

#define OP_64BIT (SIZE_MAX == UINT64_MAX)
using uint_op = std::conditional_t<OP_64BIT, uint64_t, uint32_t>;

namespace JitFFI
{
	namespace OpCode
	{
		inline void mov_eax(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0xb8);
			jfc.push_uint32(dat);
		}
		inline void mov_ecx(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0xbb);
			jfc.push_uint32(dat);
		}

		inline void push_eax(JitFuncCreater &jfc) {
			jfc.push(0x50);
		}
		inline void push_rbp(JitFuncCreater &jfc) {
			jfc.push(0x55);
		}
		inline void pop_rbp(JitFuncCreater &jfc) {
			jfc.push(0x5d);
		}
		inline void push_ebp(JitFuncCreater &jfc) {
			jfc.push(0x55);
		}
		inline void pop_ebp(JitFuncCreater &jfc) {
			jfc.push(0x5d);
		}

		inline void mov_ebp_esp(JitFuncCreater &jfc) {
			jfc.push(0x8b, 0xec);
		}
		inline void mov_esp_ebp(JitFuncCreater &jfc) {
			jfc.push(0x8b, 0xe5);
		}

		inline void mov_rbp_rsp(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x89, 0xe5); // ???
		}

		inline void sub_esp(JitFuncCreater &jfc, byte dat) {
			jfc.push(0x83, 0xec);
			jfc.push(dat);
		}

		inline void sub_esp(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0x81, 0xec);
			jfc.push_uint32(dat);
		}

		inline void call_eax(JitFuncCreater &jfc) {
			jfc.push(0xff, 0xd0);
		}

		inline void ret(JitFuncCreater &jfc) {
			jfc.push(0xc3);
		}
		inline void ret(JitFuncCreater &jfc, uint16_t dat) {
			jfc.push(0xc6);
			jfc.push_uint16(dat);
		}
		inline void leave(JitFuncCreater &jfc) {
			jfc.push(0xc9);
		}
	}

	namespace OpCode_x86
	{
		inline void push_byte(JitFuncCreater &jfc, byte dat) {
			jfc.push(0x6a);
			jfc.push(dat);
		}
		inline void push_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0x68);
			jfc.push_uint32(dat);
		}
		inline void push_uint64(JitFuncCreater &jfc, uint64_t dat) {
			union {
				struct {
					uint32_t a, b;
				};
				uint64_t v;
			} v;
			v.v = dat;
			push_uint32(jfc, v.b);
			push_uint32(jfc, v.a);
		}
	}

	namespace OpCode_x64
	{
		inline void mov_rax(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0xb8);
			jfc.push_uint64(dat);
		}
		inline void push_rax(JitFuncCreater &jfc) {
			jfc.push(0x50);
		}
		inline void call_rax(JitFuncCreater &jfc) {
			jfc.push(0xff, 0xd0);
		}

		inline void sub_rsp(JitFuncCreater &jfc, byte v) {
			// sub rsp, v
			jfc.push(0x48, 0x83, 0xec, v);
		}
		inline void add_rsp(JitFuncCreater &jfc, byte v) {
			// add rsp, v
			jfc.push(0x48, 0x83, 0xc4, v);
		}

		inline void mov_rax_rsp(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x89, 0xe0);
		}
		inline void mov_rcx_rax(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x89, 0xc1);
		}
		inline void mov_rdi_rax(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x89, 0xc7);
		}

		inline void mov_rax_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0xb8);
			jfc.push_uint64(dat);
		}
		inline void mov_rbx_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0xbb);
			jfc.push_uint64(dat);
		}
		inline void mov_rcx_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0xb9);
			jfc.push_uint64(dat);
		}
		inline void mov_rdx_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0xba);
			jfc.push_uint64(dat);
		}
		inline void mov_rdi_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0xbf);
			jfc.push_uint64(dat);
		}
		inline void mov_rsi_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0xbe);
			jfc.push_uint64(dat);
		}
		inline void mov_r8_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x49, 0xb8);
			jfc.push_uint64(dat);
		}
		inline void mov_r9_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x49, 0xb9);
			jfc.push_uint64(dat);
		}
		inline void mov_prsp_rax(JitFuncCreater &jfc, byte offset) {
			jfc.push(0x48, 0x89, 0x44, 0x24);
			jfc.push(offset);
		}

		inline void movq_xmm0_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0xc0);
		}
		inline void movq_xmm1_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0xc8);
		}
		inline void movq_xmm2_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0xd0);
		}
		inline void movq_xmm3_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0xd8);
		}
		inline void movq_xmm4_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0xe0);
		}
		inline void movq_xmm5_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0xe8);
		}
		inline void movq_xmm6_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0xf0);
		}
		inline void movq_xmm7_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0xf8);
		}
	}

	namespace OpCode
	{
		template <typename _FTy>
		inline void call_func(JitFuncCreater &jfc, _FTy *dat) {
#if OP_64BIT
			OpCode_x64::mov_rax(jfc, reinterpret_cast<uint_op>(dat));
			OpCode_x64::call_rax(jfc);
#else
			mov_eax(jfc, reinterpret_cast<uint_op>(dat));
			call_eax(jfc);
#endif
		}
	}

	namespace OpCode_win64
	{
		inline void add_int0(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rcx_uint64(jfc, dat);
		}
		inline void add_int1(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rdx_uint64(jfc, dat);
		}
		inline void add_int2(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_r8_uint64(jfc, dat);
		}
		inline void add_int3(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_r9_uint64(jfc, dat);
		}
		inline void add_intx(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			// mov rax, dat
			OpCode_x64::mov_rax_uint64(jfc, dat);
			// mov qword ptr [rsp+20h+count*8], rax
			OpCode_x64::mov_prsp_rax(jfc, 0x20 + count * 8);
		}

		// add_double_n: (n : 0 ~ 3)
		//     mov rax, dat    ; dat : convert(value)
		//     mov xmm_n, rax

		inline void add_double0(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm0_rax(jfc);
		}
		inline void add_double1(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm1_rax(jfc);
		}
		inline void add_double2(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm2_rax(jfc);
		}
		inline void add_double3(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm3_rax(jfc);
		}
		inline void add_doublex(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			add_intx(jfc, dat, count);
		}

		inline unsigned int add_int(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			switch (count) {
			case 0:  add_int0(jfc, dat); return 0;
			case 1:  add_int1(jfc, dat); return 0;
			case 2:  add_int2(jfc, dat); return 0;
			case 3:  add_int3(jfc, dat); return 0;
			default: add_intx(jfc, dat, count - 4); return 1;
			}
		}
		inline unsigned int add_double(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			switch (count) {
			case 0:  add_double0(jfc, dat); return 0;
			case 1:  add_double1(jfc, dat); return 0;
			case 2:  add_double2(jfc, dat); return 0;
			case 3:  add_double3(jfc, dat); return 0;
			default: add_doublex(jfc, dat, count - 4); return 1;
			}
		}
	}

	namespace OpCode_sysv64
	{
		inline void add_int0(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rdi_uint64(jfc, dat);
		}
		inline void add_int1(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rsi_uint64(jfc, dat);
		}
		inline void add_int2(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rdx_uint64(jfc, dat);
		}
		inline void add_int3(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rcx_uint64(jfc, dat);
		}
		inline void add_int4(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_r8_uint64(jfc, dat);
		}
		inline void add_int5(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_r9_uint64(jfc, dat);
		}
		inline void add_intx(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			OpCode_x64::mov_rax_uint64(jfc, dat);
			OpCode_x64::push_rax(jfc);
		}

		// add_double_n: (n : 0 ~ 7)
		//     mov rax, dat    ; dat : convert(value)
		//     mov xmm_n, rax

		inline void add_double0(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm0_rax(jfc);
		}
		inline void add_double1(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm1_rax(jfc);
		}
		inline void add_double2(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm2_rax(jfc);
		}
		inline void add_double3(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm3_rax(jfc);
		}
		inline void add_double4(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm4_rax(jfc);
		}
		inline void add_double5(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm5_rax(jfc);
		}
		inline void add_double6(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm6_rax(jfc);
		}
		inline void add_double7(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax(jfc, dat);
			OpCode_x64::movq_xmm7_rax(jfc);
		}
		inline void add_doublex(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			add_intx(jfc, dat, count);
		}

		inline unsigned int add_int(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			switch (count) {
			case 0:  add_int0(jfc, dat); return 0;
			case 1:  add_int1(jfc, dat); return 0;
			case 2:  add_int2(jfc, dat); return 0;
			case 3:  add_int3(jfc, dat); return 0;
			case 4:  add_int4(jfc, dat); return 0;
			case 5:  add_int5(jfc, dat); return 0;
			default: add_intx(jfc, dat, count - 6); return 1;
			}
		}
		inline unsigned int add_double(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			switch (count) {
			case 0:  add_double0(jfc, dat); return 0;
			case 1:  add_double1(jfc, dat); return 0;
			case 2:  add_double2(jfc, dat); return 0;
			case 3:  add_double3(jfc, dat); return 0;
			case 4:  add_double4(jfc, dat); return 0;
			case 5:  add_double5(jfc, dat); return 0;
			case 6:  add_double6(jfc, dat); return 0;
			case 7:  add_double7(jfc, dat); return 0;
			default: add_doublex(jfc, dat, count - 8); return 1;
			}
		}
	}
}
