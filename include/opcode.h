// JitFFI Library
// * opcode.h

#pragma once
#ifndef _JITFFI_OPCODE_H_
#define _JITFFI_OPCODE_H_
#include <cstdint>
#include "creater.h"

namespace JitFFI
{
	namespace OpCode_x86
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
		// RA : [ rax : 0, rcx : 1, rdx : 2, rbx : 3, rsp : 4, rbp : 5, rsi : 6, rdi : 7 ]
		// RB : [ r8  : 0, r9  : 1, r10 : 2, r11 : 3, r12 : 4 ]

		// mov RA, RA   : 0x48 0x89 03** : RA -> RA
		// mov RB, RA   : 0x49 0x89 03** : RA -> RB 
		// mov RA, RB   : 0x4c 0x89 03** : RB -> RA
		// mov RB, RB   : 0x4d 0x89 03** : RB -> RB

		// mov pRA, RA  : 0x48 0x89 00** : RA -> pRA
		// mov pRB, RA  : 0x49 0x89 00** : RA -> pRB
		// mov pRA, RB  : 0x4c 0x89 00** : RB -> pRA
		// mov pRB, RB  : 0x4d 0x89 00** : RB -> pRB

		// mov RA, pRA  : 0x48 0x8b 00** : RA <- pRA
		// mov RA, pRB  : 0x49 0x8b 00** : RA <- pRB
		// mov RB, pRA  : 0x4c 0x8b 00** : RB <- pRA
		// mov RB, pRB  : 0x4d 0x8b 00** : RB <- pRB

		enum Register
		{
			// register
			rax = 0,
			rcx = 1,
			rdx = 2,
			rbx = 3,
			rsp = 4,
			rbp = 5,
			rsi = 6,
			rdi = 7,
			r8 = 10,
			r9 = 11,
			r10 = 12,
			r11 = 13,
			r12 = 14,
			// pointer to register
			prax = 20,
			prcx = 21,
			prdx = 22,
			prbx = 23,
			prsp = 24,
			prbp = 25,
			prsi = 26,
			prdi = 27,
			pr8 = 30,
			pr9 = 31,
			pr10 = 32,
			pr11 = 33,
			pr12 = 34,
		};

		inline byte _get_mov_head(Register r0, Register r1) {
			// A A -> 0x48
			// B A -> 0x49
			// A B -> 0x4c
			// B B -> 0x4d

			byte rr0 = (r0 >= 20) ? r0 - 20 : r0;
			byte rr1 = (r1 >= 20) ? r1 - 20 : r1;
			return 0x48 + ((rr0 < 10) ? 0 : 1) + ((rr1 < 10) ? 0 : 4);
		}
		inline byte _get_mov_rcode(Register r0, Register r1) {
			auto v = (r0 % 10) * 010 + (r1 % 10);
			assert(v < UINT8_MAX);
			return static_cast<byte>(v);
		}

		inline void mov_reg_reg(JitFuncCreater &jfc, Register dst, Register src) {
			assert(dst < 20 && src < 20);
			jfc.push(_get_mov_head(dst, src), 0x89, 0300 + _get_mov_rcode(src, dst));
		}

		inline void mov_preg_reg(JitFuncCreater &jfc, Register dst, Register src) {
			assert(dst >= 20 && src < 20);
			printf("<%d %d>\n", dst, src);
			jfc.push(_get_mov_head(dst, src), 0x89, 0000 + _get_mov_rcode(src, dst));
			if (dst == prsp)
				jfc.push(0x24);
			else if (dst == prbp)
				jfc.push(0x00);
		}
		inline void mov_reg_preg(JitFuncCreater &jfc, Register dst, Register src) {
			assert(dst < 20 && src >= 20);
			jfc.push(_get_mov_head(src, dst), 0x8b);
			if (src == prbp) {
				jfc.push(0100 + _get_mov_rcode(dst, src), 0x00);
			}
			else {
				jfc.push(0000 + _get_mov_rcode(dst, src));
				if (src == prsp)
					jfc.push(0x24);
			}
		}

		inline void mov(JitFuncCreater &jfc, Register dst, Register src) {
			assert(dst < 20 || src < 20);

			if (dst < 20 && src < 20)
				mov_reg_reg(jfc, dst, src);
			else if (dst < 20)
				mov_reg_preg(jfc, dst, src);
			else
				mov_preg_reg(jfc, dst, src);
		}


		inline void mov_rax_prsp(JitFuncCreater &jfc, byte offset) {
			jfc.push(0x48, 0x8b, 0104, 0x24);
			jfc.push(offset);
		}
		inline void mov_st0_prbx(JitFuncCreater &jfc) {
			// fldt (%rbx)
			jfc.push(0xdb, 0x2b);
		}
		inline void mov_prbx_st0(JitFuncCreater &jfc) {
			// fstpt (%rbx)
			jfc.push(0xdb, 0x3b);
		}
		inline void mov_st0_prsp(JitFuncCreater &jfc) {
			// fldt (%rsp)
			jfc.push(0xdb, 0x2c, 0x24);
		}
		inline void mov_prsp_st0(JitFuncCreater &jfc) {
			// fstpt (%rsp)
			jfc.push(0xdb, 0x3c, 0x24);
		}

		inline void mov_rax_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0270);
			jfc.push_uint64(dat);
		}
		inline void mov_rcx_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0271);
			jfc.push_uint64(dat);
		}
		inline void mov_rdx_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0272);
			jfc.push_uint64(dat);
		}
		inline void mov_rbx_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0273);
			jfc.push_uint64(dat);
		}
		inline void mov_rsi_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0276);
			jfc.push_uint64(dat);
		}
		inline void mov_rdi_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x48, 0277);
			jfc.push_uint64(dat);
		}
		inline void mov_r8_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x49, 0270);
			jfc.push_uint64(dat);
		}
		inline void mov_r9_uint64(JitFuncCreater &jfc, uint64_t dat) {
			jfc.push(0x49, 0271);
			jfc.push_uint64(dat);
		}

		inline void mov_eax_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0270);
			jfc.push_uint32(dat);
		}
		inline void mov_ecx_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0271);
			jfc.push_uint32(dat);
		}
		inline void mov_edx_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0272);
			jfc.push_uint32(dat);
		}
		inline void mov_ebx_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0273);
			jfc.push_uint32(dat);
		}
		inline void mov_esi_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0276);
			jfc.push_uint32(dat);
		}
		inline void mov_edi_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0277);
			jfc.push_uint32(dat);
		}
		inline void mov_r8d_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0x41, 0270);
			jfc.push_uint32(dat);
		}
		inline void mov_r9d_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0x41, 0271);
			jfc.push_uint32(dat);
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

		inline void movq_rax_xmm0(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x7e, 0xc0);
		}
		inline void movq_rax_xmm1(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x7e, 0xc8);
		}
		inline void movq_prax_xmm0(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x0f, 0xd6, 0x00);
		}
		inline void movq_prax_xmm1(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x0f, 0xd6, 0x08);
		}
		//OpCode_x64::mov_(\w+)_p(\w+)\(jfc\)
		//OpCode_x64::mov(jfc, OpCode_x64::$1, OpCode_x64::$2)
		inline void mov_rcx_prax(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x8b, 0010);
		}
		inline void mov_rdx_prax(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x8b, 0020);
		}
		inline void mov_rsi_prax(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x8b, 0060);
		}
		inline void mov_rdi_prax(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x8b, 0070);
		}
		inline void mov_r8_prax(JitFuncCreater &jfc) {
			jfc.push(0x4c, 0x8b, 0000);
		}
		inline void mov_r9_prax(JitFuncCreater &jfc) {
			jfc.push(0x4c, 0x8b, 0010);
		}

		inline void movq_xmm0_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0x00);
		}
		inline void movq_xmm1_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0x08);
		}
		inline void movq_xmm2_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0x10);
		}
		inline void movq_xmm3_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0x18);
		}
		inline void movq_xmm4_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0x20);
		}
		inline void movq_xmm5_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0x28);
		}
		inline void movq_xmm6_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0x30);
		}
		inline void movq_xmm7_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0x38);
		}


		inline void mov_prcx_rax(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x89, 0x01);
		}
		inline void mov_prcx_eax(JitFuncCreater &jfc) {
			jfc.push(0x89, 0x01);
		}
		inline void mov_prcx_ax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x89, 0x01);
		}
		inline void mov_prcx_al(JitFuncCreater &jfc) {
			jfc.push(0x88, 0x01);
		}

		inline void mov_prbx_rax(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x89, 0x03);
		}
		inline void mov_prbx_eax(JitFuncCreater &jfc) {
			jfc.push(0x89, 0x03);
		}
		inline void mov_prbx_ax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x89, 0x03);
		}
		inline void mov_prbx_al(JitFuncCreater &jfc) {
			jfc.push(0x88, 0x03);
		}

		inline void mov_prbx_rdx(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x89, 0x13);
		}
		inline void movq_prbx_xmm0(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x0f, 0xd6, 0x03);
		}
		inline void movq_prbx_xmm1(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x0f, 0xd6, 0x0b);
		}
		inline void movsd_prbx_xmm0(JitFuncCreater &jfc) {
			jfc.push(0xf2, 0x0f, 0x11, 0x03);
		}
		inline void movss_prbx_xmm0(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x11, 0x03);
		}
		inline void movsd_prbx_xmm1(JitFuncCreater &jfc) {
			jfc.push(0xf2, 0x0f, 0x11, 0x0b);
		}
		inline void movss_prbx_xmm1(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x11, 0x0b);
		}

		inline void movq_rbx_xmm0(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x7e, 0xc3);
		}
		inline void movq_rbx_xmm1(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x7e, 0xcb);
		}

		inline void movzbl_eax_prsp(JitFuncCreater &jfc, byte offset) {
			jfc.push(0x0f, 0xb6, 0x44, 0x24, offset);
		}
		inline void mov_prbx_al(JitFuncCreater &jfc, byte offset) {
			jfc.push(0x88, 0x43, offset);
		}

		inline void mov_prsp_rax(JitFuncCreater &jfc, byte offset) {
			jfc.push(0x48, 0x89, 0x44, 0x24);
			jfc.push(offset);
		}
		inline void mov_prsp_uint32(JitFuncCreater &jfc, byte offset, uint32_t dat) {
			jfc.push(0x48, 0xc7, 0x44, 0x24);
			jfc.push(offset);
			jfc.push_uint32(dat);
		}

		inline void mov_rcx_byte(JitFuncCreater &jfc, byte v) {
			// mov cl, v
			jfc.push(0xb1, v);
		}
		inline void mov_rcx_uint32(JitFuncCreater &jfc, uint32_t v) {
			// mov ecx, v
			jfc.push(0xb9);
			jfc.push_uint32(v);
		}
		inline void movsb_prdi_prsi(JitFuncCreater &jfc) {
			// movsb es:[rdi], ds:[rsi]
			jfc.push(0xa4);
		}
		inline void movsb_prdi_prsi_rep(JitFuncCreater &jfc) {
			// rep movsb es:[rdi], ds:[rsi]
			jfc.push(0xf3, 0xa4);
		}

		inline void mov_rax(JitFuncCreater &jfc, uint64_t dat) {
			mov_rax_uint64(jfc, dat);
		}
		inline void push_rax(JitFuncCreater &jfc) {
			jfc.push(0x50);
		}
		inline void push_rdx(JitFuncCreater &jfc) {
			jfc.push(0x52);
		}
		inline void push_rbx(JitFuncCreater &jfc) {
			jfc.push(0x53);
		}
		inline void push_rbp(JitFuncCreater &jfc) {
			jfc.push(0x55);
		}
		inline void push_rsi(JitFuncCreater &jfc) {
			jfc.push(0x56);
		}
		inline void push_rdi(JitFuncCreater &jfc) {
			jfc.push(0x57);
		}
		inline void push_r10(JitFuncCreater &jfc) {
			jfc.push(0x41, 0x52);
		}
		inline void push_r12(JitFuncCreater &jfc) {
			jfc.push(0x41, 0x54);
		}

		inline void pop_rax(JitFuncCreater &jfc) {
			jfc.push(0x58);
		}
		inline void pop_rdx(JitFuncCreater &jfc) {
			jfc.push(0x5a);
		}
		inline void pop_rbx(JitFuncCreater &jfc) {
			jfc.push(0x5b);
		}
		inline void pop_rbp(JitFuncCreater &jfc) {
			jfc.push(0x5d);
		}
		inline void pop_rsi(JitFuncCreater &jfc) {
			jfc.push(0x5e);
		}
		inline void pop_rdi(JitFuncCreater &jfc) {
			jfc.push(0x5f);
		}
		inline void pop_r10(JitFuncCreater &jfc) {
			jfc.push(0x41, 0x5a);
		}
		inline void pop_r12(JitFuncCreater &jfc) {
			jfc.push(0x41, 0x5c);
		}

		inline void push_prax(JitFuncCreater &jfc) {
			jfc.push(0xff, 0x30);
		}
		inline void call_rax(JitFuncCreater &jfc) {
			jfc.push(0xff, 0xd0);
		}

		inline void push_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0x68);
			jfc.push_uint32(dat);
		}

		inline void add_rax_byte(JitFuncCreater &jfc, byte v) {
			jfc.push(0x48, 0x83, 0xc0, v);
		}
		inline void add_rbx_byte(JitFuncCreater &jfc, byte v) {
			jfc.push(0x48, 0x83, 0xc3, v);
		}
		inline void add_rsp_byte(JitFuncCreater &jfc, byte v) {
			jfc.push(0x48, 0x83, 0xc4, v);
		}

		inline void add_rax_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0x48, 0x81, 0xc0);
			jfc.push_uint32(dat);
		}
		inline void add_rbx_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0x48, 0x81, 0xc3);
			jfc.push_uint32(dat);
		}
		inline void add_rsp_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0x48, 0x81, 0xc4);
			jfc.push_uint32(dat);
		}

		inline void sub_rax_byte(JitFuncCreater &jfc, byte v) {
			jfc.push(0x48, 0x83, 0xe8, v);
		}
		inline void sub_rbx_byte(JitFuncCreater &jfc, byte v) {
			jfc.push(0x48, 0x83, 0xeb, v);
		}
		inline void sub_rsp_byte(JitFuncCreater &jfc, byte v) {
			jfc.push(0x48, 0x83, 0xec, v);
		}

		inline void sub_rbx_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0x48, 0x81, 0xeb);
			jfc.push_uint32(dat);
		}

		inline void ret(JitFuncCreater &jfc) {
			jfc.push(0xc3);
		}
	}

	namespace OpCode_x86
	{
		template <typename _FTy>
		inline void call_func(JitFuncCreater &jfc, _FTy *dat) {
			OpCode_x86::mov_eax(jfc, reinterpret_cast<uint32_t>(dat));
			OpCode_x86::call_eax(jfc);
		}
	}
	namespace OpCode_x64
	{
		template <typename _FTy>
		inline void call_func(JitFuncCreater &jfc, _FTy *dat) {
			OpCode_x64::mov_rax(jfc, reinterpret_cast<uint64_t>(dat));
			OpCode_x64::call_rax(jfc);
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
		inline void add_intx(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax_uint64(jfc, dat);
			OpCode_x64::push_rax(jfc);
		}

		inline void add_int0_rbx(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rcx, OpCode_x64::rbx);
		}
		inline void add_int1_rbx(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rdx, OpCode_x64::rbx);
		}
		inline void add_int2_rbx(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::r8, OpCode_x64::rbx);
		}
		inline void add_int3_rbx(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::r9, OpCode_x64::rbx);
		}
		inline void add_intx_rbx(JitFuncCreater &jfc) {
			OpCode_x64::push_rbx(jfc);
		}

		inline void add_int0_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rcx, OpCode_x64::prax);
		}
		inline void add_int1_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rdx, OpCode_x64::prax);
		}
		inline void add_int2_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::r8, OpCode_x64::prax);
		}
		inline void add_int3_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::r9, OpCode_x64::prax);
		}
		inline void add_intx_prax(JitFuncCreater &jfc) {
			OpCode_x64::push_prax(jfc);
		}

		inline void add_int0_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_ecx_uint32(jfc, dat);
		}
		inline void add_int1_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_edx_uint32(jfc, dat);
		}
		inline void add_int2_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_r8d_uint32(jfc, dat);
		}
		inline void add_int3_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_r9d_uint32(jfc, dat);
		}
		inline void add_intx_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::push_uint32(jfc, dat);
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
		inline void add_doublex(JitFuncCreater &jfc, uint64_t dat) {
			add_intx(jfc, dat);
		}

		inline void add_double0_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm0_prax(jfc);
		}
		inline void add_double1_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm1_prax(jfc);
		}
		inline void add_double2_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm2_prax(jfc);
		}
		inline void add_double3_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm3_prax(jfc);
		}
		inline void add_doublex_prax(JitFuncCreater &jfc) {
			add_intx_prax(jfc);
		}

		inline unsigned int add_int(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			switch (count) {
			case 0:  add_int0(jfc, dat); return 0;
			case 1:  add_int1(jfc, dat); return 0;
			case 2:  add_int2(jfc, dat); return 0;
			case 3:  add_int3(jfc, dat); return 0;
			default: add_intx(jfc, dat); return 1;
			}
		}
		inline unsigned int add_int_uint32(JitFuncCreater &jfc, uint32_t dat, unsigned int count) {
			switch (count) {
			case 0:  add_int0_uint32(jfc, dat); return 0;
			case 1:  add_int1_uint32(jfc, dat); return 0;
			case 2:  add_int2_uint32(jfc, dat); return 0;
			case 3:  add_int3_uint32(jfc, dat); return 0;
			default: add_intx_uint32(jfc, dat); return 1;
			}
		}
		inline unsigned int add_int_rbx(JitFuncCreater &jfc, unsigned int count) {
			switch (count) {
			case 0:  add_int0_rbx(jfc); return 0;
			case 1:  add_int1_rbx(jfc); return 0;
			case 2:  add_int2_rbx(jfc); return 0;
			case 3:  add_int3_rbx(jfc); return 0;
			default: add_intx_rbx(jfc); return 1;
			}
		}
		inline unsigned int add_int_prax(JitFuncCreater &jfc, unsigned int count) {
			switch (count) {
			case 0:  add_int0_prax(jfc); return 0;
			case 1:  add_int1_prax(jfc); return 0;
			case 2:  add_int2_prax(jfc); return 0;
			case 3:  add_int3_prax(jfc); return 0;
			default: add_intx_prax(jfc); return 1;
			}
		}
		inline unsigned int add_double(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			switch (count) {
			case 0:  add_double0(jfc, dat); return 0;
			case 1:  add_double1(jfc, dat); return 0;
			case 2:  add_double2(jfc, dat); return 0;
			case 3:  add_double3(jfc, dat); return 0;
			default: add_doublex(jfc, dat); return 1;
			}
		}
		inline unsigned int add_double_prax(JitFuncCreater &jfc, unsigned int count) {
			switch (count) {
			case 0:  add_double0_prax(jfc); return 0;
			case 1:  add_double1_prax(jfc); return 0;
			case 2:  add_double2_prax(jfc); return 0;
			case 3:  add_double3_prax(jfc); return 0;
			default: add_doublex_prax(jfc); return 1;
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
		inline void add_intx(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rax_uint64(jfc, dat);
			OpCode_x64::push_rax(jfc);
		}

		inline void add_int0_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_edi_uint32(jfc, dat);
		}
		inline void add_int1_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_esi_uint32(jfc, dat);
		}
		inline void add_int2_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_edx_uint32(jfc, dat);
		}
		inline void add_int3_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_ecx_uint32(jfc, dat);
		}
		inline void add_int4_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_r8d_uint32(jfc, dat);
		}
		inline void add_int5_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_r9d_uint32(jfc, dat);
		}
		inline void add_intx_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::push_uint32(jfc, dat);
		}

		inline void add_int0_rbx(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rdi, OpCode_x64::rbx);
		}
		inline void add_int1_rbx(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rsi, OpCode_x64::rbx);
		}
		inline void add_int2_rbx(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rdx, OpCode_x64::rbx);
		}
		inline void add_int3_rbx(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rcx, OpCode_x64::rbx);
		}
		inline void add_int4_rbx(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::r8, OpCode_x64::rbx);
		}
		inline void add_int5_rbx(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::r9, OpCode_x64::rbx);
		}
		inline void add_intx_rbx(JitFuncCreater &jfc) {
			OpCode_x64::push_rbx(jfc);
		}

		inline void add_int0_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rdi, OpCode_x64::prax);
		}
		inline void add_int1_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rsi, OpCode_x64::prax);
		}
		inline void add_int2_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov_rdx_prax(jfc);
		}
		inline void add_int3_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov_rcx_prax(jfc);
		}
		inline void add_int4_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov_r8_prax(jfc);
		}
		inline void add_int5_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov_r9_prax(jfc);
		}
		inline void add_intx_prax(JitFuncCreater &jfc) {
			OpCode_x64::push_prax(jfc);
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
		inline void add_doublex(JitFuncCreater &jfc, uint64_t dat) {
			add_intx(jfc, dat);
		}

		inline void add_double0_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm0_prax(jfc);
		}
		inline void add_double1_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm1_prax(jfc);
		}
		inline void add_double2_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm2_prax(jfc);
		}
		inline void add_double3_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm3_prax(jfc);
		}
		inline void add_double4_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm4_prax(jfc);
		}
		inline void add_double5_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm5_prax(jfc);
		}
		inline void add_double6_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm6_prax(jfc);
		}
		inline void add_double7_prax(JitFuncCreater &jfc) {
			OpCode_x64::movq_xmm7_prax(jfc);
		}
		inline void add_doublex_prax(JitFuncCreater &jfc) {
			add_intx_prax(jfc);
		}

		inline unsigned int add_int(JitFuncCreater &jfc, uint64_t dat, unsigned int count) {
			switch (count) {
			case 0:  add_int0(jfc, dat); return 0;
			case 1:  add_int1(jfc, dat); return 0;
			case 2:  add_int2(jfc, dat); return 0;
			case 3:  add_int3(jfc, dat); return 0;
			case 4:  add_int4(jfc, dat); return 0;
			case 5:  add_int5(jfc, dat); return 0;
			default: add_intx(jfc, dat); return 1;
			}
		}
		inline unsigned int add_int_uint32(JitFuncCreater &jfc, uint32_t dat, unsigned int count) {
			switch (count) {
			case 0:  add_int0_uint32(jfc, dat); return 0;
			case 1:  add_int1_uint32(jfc, dat); return 0;
			case 2:  add_int2_uint32(jfc, dat); return 0;
			case 3:  add_int3_uint32(jfc, dat); return 0;
			case 4:  add_int4_uint32(jfc, dat); return 0;
			case 5:  add_int5_uint32(jfc, dat); return 0;
			default: add_intx_uint32(jfc, dat); return 1;
			}
		}
		inline unsigned int add_int_rbx(JitFuncCreater &jfc, unsigned int count) {
			switch (count) {
			case 0:  add_int0_rbx(jfc); return 0;
			case 1:  add_int1_rbx(jfc); return 0;
			case 2:  add_int2_rbx(jfc); return 0;
			case 3:  add_int3_rbx(jfc); return 0;
			case 4:  add_int4_rbx(jfc); return 0;
			case 5:  add_int5_rbx(jfc); return 0;
			default: add_intx_rbx(jfc); return 1;
			}
		}
		inline unsigned int add_int_prax(JitFuncCreater &jfc, unsigned int count) {
			switch (count) {
			case 0:  add_int0_prax(jfc); return 0;
			case 1:  add_int1_prax(jfc); return 0;
			case 2:  add_int2_prax(jfc); return 0;
			case 3:  add_int3_prax(jfc); return 0;
			case 4:  add_int4_prax(jfc); return 0;
			case 5:  add_int5_prax(jfc); return 0;
			default: add_intx_prax(jfc); return 1;
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
			default: add_doublex(jfc, dat); return 1;
			}
		}
		inline unsigned int add_double_prax(JitFuncCreater &jfc, unsigned int count) {
			switch (count) {
			case 0:  add_double0_prax(jfc); return 0;
			case 1:  add_double1_prax(jfc); return 0;
			case 2:  add_double2_prax(jfc); return 0;
			case 3:  add_double3_prax(jfc); return 0;
			case 4:  add_double4_prax(jfc); return 0;
			case 5:  add_double5_prax(jfc); return 0;
			case 6:  add_double6_prax(jfc); return 0;
			case 7:  add_double7_prax(jfc); return 0;
			default: add_doublex_prax(jfc); return 1;
			}
		}
	}
}

#	if (defined(_WIN64))
#		define OpCode_curr OpCode_win64
#	elif (defined(__x86_64__))
#		define OpCode_curr OpCode_sysv64
#	endif
#endif
