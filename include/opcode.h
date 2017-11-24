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
		// RA(r0) : [ rax : 0, rcx : 1, rdx : 2, rbx : 3, rsp : 4, rbp : 5, rsi : 6, rdi : 7 ]
		// RB(r1) : [ r8  : 0, r9  : 1, r10 : 2, r11 : 3, r12 : 4, r13 : 5, r14 : 6, r15 : 7 ]

		enum Register
		{
			// register 64 bit : rxq
			rax = 0000,
			rcx = 0001,
			rdx = 0002,
			rbx = 0003,
			rsp = 0004,
			rbp = 0005,
			rsi = 0006,
			rdi = 0007,
			r8 = 0010,
			r9 = 0011,
			r10 = 0012,
			r11 = 0013,
			r12 = 0014,
			r13 = 0015,
			r14 = 0016,
			r15 = 0017,
			// register 32 bit : rxd
			eax = 0020,
			ecx = 0021,
			edx = 0022,
			ebx = 0023,
			esp = 0024,
			ebp = 0025,
			esi = 0026,
			edi = 0027,
			r8d = 0030,
			r9d = 0031,
			r10d = 0032,
			r11d = 0033,
			r12d = 0034,
			r13d = 0035,
			r14d = 0036,
			r15d = 0037,
			// register 16 bit : rxw
			ax = 0040,
			cx = 0041,
			dx = 0042,
			bx = 0043,
			sp = 0044,
			bp = 0045,
			si = 0046,
			di = 0047,
			r8w = 0050,
			r9w = 0051,
			r10w = 0052,
			r11w = 0053,
			r12w = 0054,
			r13w = 0055,
			r14w = 0056,
			r15w = 0057,
			// register 8 bit : rxb
			al = 0060,
			cl = 0061,
			dl = 0062,
			bl = 0063,
			spl = 0064,
			bpl = 0065,
			sil = 0066,
			dil = 0067,
			r8b = 0070,
			r9b = 0071,
			r10b = 0072,
			r11b = 0073,
			r12b = 0074,
			r13b = 0075,
			r14b = 0076,
			r15b = 0077,
			// pointer to register 64 bit : prxq
			prax = 0100,
			prcx = 0101,
			prdx = 0102,
			prbx = 0103,
			prsp = 0104,
			prbp = 0105,
			prsi = 0106,
			prdi = 0107,
			pr8 = 0110,
			pr9 = 0111,
			pr10 = 0112,
			pr11 = 0113,
			pr12 = 0114,
			pr13 = 0115,
			pr14 = 0116,
			pr15 = 0117,
			// pointer to register 32 bit : prxd
			peax = 0120,
			pecx = 0121,
			pedx = 0122,
			pebx = 0123,
			pesp = 0124,
			pebp = 0125,
			pesi = 0126,
			pedi = 0127,
			pr8d = 0130,
			pr9d = 0131,
			pr10d = 0132,
			pr11d = 0133,
			pr12d = 0134,
			pr13d = 0135,
			pr14d = 0136,
			pr15d = 0137,
		};

		inline bool is_rxq0(Register r) { return rax <= r && r <= rdi; }
		inline bool is_rxq1(Register r) { return r8 <= r && r <= r15; }
		inline bool is_rxq(Register r) { return rax <= r && r <= r15; }
		inline bool is_rxd0(Register r) { return eax <= r && r <= edi; }
		inline bool is_rxd1(Register r) { return r8d <= r && r <= r15d; }
		inline bool is_rxd(Register r) { return eax <= r && r <= r15d; }
		inline bool is_rxw0(Register r) { return ax <= r && r <= di; }
		inline bool is_rxw1(Register r) { return r8w <= r && r <= r15w; }
		inline bool is_rxw(Register r) { return ax <= r && r <= r15w; }
		inline bool is_rxb0(Register r) { return al <= r && r <= dil; }
		inline bool is_rxb1(Register r) { return r8b <= r && r <= r15b; }
		inline bool is_rxb(Register r) { return al <= r && r <= r15b; }

		inline bool is_rx(Register r) { return rax <= r && r <= r15b; }

		inline bool is_prxq0(Register r) { return prax <= r && r <= prdi; }
		inline bool is_prxq1(Register r) { return pr8 <= r && r <= pr15; }
		inline bool is_prxq(Register r) { return prax <= r && r <= pr15; }

		inline bool is_prx(Register r) { return prax <= r && r <= pr15; } // TODO

		// rp = r & p
		inline bool is_rpx0(Register r) { return r % 020 < 010; }
		inline bool is_rpx1(Register r) { return r % 020 >= 010; }

		inline bool is_same_qdwb(Register r0, Register r1) { return (r0 / 020) == (r1 / 020); }

		inline byte get_code(Register r) { return r % 010; }

		// mov RAQ, RAQ   : 0x48 0x89 03** : RA -> RA
		// mov RBQ, RAQ   : 0x49 0x89 03** : RA -> RB 
		// mov RAQ, RBQ   : 0x4c 0x89 03** : RB -> RA
		// mov RBQ, RBQ   : 0x4d 0x89 03** : RB -> RB

		// mov pRAQ, RAQ  : 0x48 0x89 00** : RA -> pRA
		// mov pRBQ, RAQ  : 0x49 0x89 00** : RA -> pRB
		// mov pRAQ, RBQ  : 0x4c 0x89 00** : RB -> pRA
		// mov pRBQ, RBQ  : 0x4d 0x89 00** : RB -> pRB

		// mov RAQ, pRAQ  : 0x48 0x8b 00** : RA <- pRA
		// mov RAQ, pRBQ  : 0x49 0x8b 00** : RA <- pRB
		// mov RBQ, pRAQ  : 0x4c 0x8b 00** : RB <- pRA
		// mov RBQ, pRBQ  : 0x4d 0x8b 00** : RB <- pRB

		// mov RAD, RAD   :      0x89 03** : RA -> RA
		// mov RBD, RAD   : 0x41 0x89 03** : RA -> RB 
		// mov RAD, RBD   : 0x44 0x89 03** : RB -> RA
		// mov RBD, RBD   : 0x45 0x89 03** : RB -> RB

		inline byte _get_mov_head(Register r0, Register r1) {
			// A A -> 0
			// B A -> 1
			// A B -> 4
			// B B -> 5
			return (is_rpx0(r0) ? 0 : 1) + (is_rpx0(r1) ? 0 : 4);
		}
		inline byte _get_mov_head_rpxq(Register r0, Register r1) {
			return 0x48 + _get_mov_head(r0, r1);
		}
		inline byte _get_mov_head_rpxd(Register r0, Register r1) {
			return 0x40 + _get_mov_head(r0, r1);
		}
		inline byte _get_mov_head_rpxw(Register r0, Register r1) {
			return 0x40 + _get_mov_head(r0, r1);
		}
		inline byte _get_mov_head_rpxb(Register r0, Register r1) {
			return 0x40 + _get_mov_head(r0, r1);
		}
		inline byte _get_mov_rcode(Register r0, Register r1) {
			auto v = get_code(r0) * 010 + get_code(r1);
			assert(v < UINT8_MAX);
			return static_cast<byte>(v);
		}

		inline void mov_rx_rx_base(JitFuncCreater &jfc, Register dst, Register src) {
			assert(is_same_qdwb(dst, src));
			jfc.push((is_rxb(dst) ? 0x88 : 0x89), 0300 + _get_mov_rcode(src, dst));
		}
		inline void mov_prx_rx_base(JitFuncCreater &jfc, Register dst, Register src) {
			jfc.push((is_rxb(src) ? 0x88 : 0x89));
			if (dst == prbp || dst == pr13) {
				jfc.push(0100 + _get_mov_rcode(src, dst), 0x00);
			}
			else {
				jfc.push(0000 + _get_mov_rcode(src, dst));
				if (dst == prsp || dst == pr12)
					jfc.push(0x24);
			}
		}

		inline void mov_rx_prx_base(JitFuncCreater &jfc, Register dst, Register src) {
			jfc.push((is_rxb(dst) ? 0x8a : 0x8b));
			if (src == prbp || src == pr13) {
				jfc.push(0100 + _get_mov_rcode(dst, src), 0x00);
			}
			else {
				jfc.push(0000 + _get_mov_rcode(dst, src));
				if (src == prsp || src == pr12)
					jfc.push(0x24);
			}
		}

		inline void _add_head(JitFuncCreater &jfc, Register dst, Register src) {
			if (is_rxq(src))
				jfc.push(_get_mov_head_rpxq(dst, src));
			else if (is_rxd(src)) {
				if (_get_mov_head(dst, src) != 0)
					jfc.push(_get_mov_head_rpxd(dst, src));
			}
			else if (is_rxw(src)) {
				jfc.push(0x66);
				if (_get_mov_head(dst, src) != 0)
					jfc.push(_get_mov_head_rpxw(dst, src));
			}
			else if (is_rxb(src)) {
				jfc.push(_get_mov_head_rpxb(dst, src));
			}
			else
				assert(false);
		}

		inline void mov_rx_rx(JitFuncCreater &jfc, Register dst, Register src) {
			assert(is_rx(dst) && is_rx(src));
			_add_head(jfc, dst, src);
			mov_rx_rx_base(jfc, dst, src);
		}

		inline void mov_prx_rx(JitFuncCreater &jfc, Register dst, Register src) {
			assert(is_prx(dst) && is_rx(src));
			_add_head(jfc, dst, src);
			mov_prx_rx_base(jfc, dst, src);
		}

		inline void mov_rx_prx(JitFuncCreater &jfc, Register dst, Register src) {
			assert(is_rx(dst) && is_prx(src));
			_add_head(jfc, src, dst);
			mov_rx_prx_base(jfc, dst, src);
		}

		inline void mov(JitFuncCreater &jfc, Register dst, Register src) {
			if (is_rx(dst) && is_rx(src) && is_same_qdwb(dst, src))
				mov_rx_rx(jfc, dst, src);
			else if (is_prxq(dst) && is_rx(src))
				mov_prx_rx(jfc, dst, src);
			else if (is_rxq(dst) && is_prxq(src))
				mov_rx_prx(jfc, dst, src);
			else if (is_rx(dst) && is_prxq(src))
				mov_rx_prx(jfc, dst, src);
			else
				assert(false);
		}

		template <byte PreCode, byte V1, byte V2>
		inline void _push_pop_base(JitFuncCreater &jfc, Register src) {
			if (is_rxq0(src))
				jfc.push(V1 + get_code(src));
			else if (is_rxq1(src))
				jfc.push(0x41, V1 + get_code(src));
			else if (is_prxq(src)) {
				if (src == prbp || src == pr13) {
					jfc.push(PreCode, V2 + 0100 + get_code(src), 0x00);
				}
				else {
					if (is_prxq0(src))
						jfc.push(PreCode, V2 + get_code(src));
					else
						jfc.push(0x41, PreCode, V2 + get_code(src));
					if (src == prsp || src == pr12)
						jfc.push(0x24);
				}
			}
			else
				assert(false);
		}

		inline void push(JitFuncCreater &jfc, Register src) {
			assert(is_rxq(src) || is_prxq(src));
			_push_pop_base<0xff, 0120, 0060>(jfc, src);
		}

		inline void pop(JitFuncCreater &jfc, Register src) {
			assert(is_rxq(src) || is_prxq(src));
			_push_pop_base<0x8f, 0130, 0000>(jfc, src);
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

		inline void mov_rxq_u64(JitFuncCreater &jfc, Register r, uint64_t dat) {
			assert(is_rxq(r));
			jfc.push(0x48 + (is_rpx0(r) ? 0 : 1));
			jfc.push(0270 + get_code(r));
			jfc.push_uint64(dat);
		}
		inline void mov_rxd_u32(JitFuncCreater &jfc, Register r, uint32_t dat) {
			assert(is_rxd(r));
			if (is_rxd1(r)) jfc.push(0x41);
			jfc.push(0270 + get_code(r));
			jfc.push_uint32(dat);
		}
		inline void mov_rxw_u16(JitFuncCreater &jfc, Register r, uint16_t dat) {
			assert(is_rxw(r));
			jfc.push(0x66);
			if (is_rxd1(r)) jfc.push(0x41);
			jfc.push(0270 + get_code(r));
			jfc.push_uint16(dat);
		}
		inline void mov_rxb_u8(JitFuncCreater &jfc, Register r, uint8_t dat) {
			assert(is_rxb(r));
			jfc.push(0x40 + (is_rpx0(r) ? 0 : 1));
			jfc.push(0260 + get_code(r));
			jfc.push(dat);
		}

		inline void movq_xmm0_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0300);
		}
		inline void movq_xmm1_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0310);
		}
		inline void movq_xmm2_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0320);
		}
		inline void movq_xmm3_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0330);
		}
		inline void movq_xmm4_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0340);
		}
		inline void movq_xmm5_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0350);
		}
		inline void movq_xmm6_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0360);
		}
		inline void movq_xmm7_rax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x6e, 0370);
		}

		inline void movq_rax_xmm0(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x7e, 0300);
		}
		inline void movq_rax_xmm1(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x48, 0x0f, 0x7e, 0310);
		}
		inline void movq_prax_xmm0(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x0f, 0xd6, 0000);
		}
		inline void movq_prax_xmm1(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x0f, 0xd6, 0010);
		}

		inline void movq_xmm0_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0000);
		}
		inline void movq_xmm1_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0010);
		}
		inline void movq_xmm2_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0020);
		}
		inline void movq_xmm3_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0030);
		}
		inline void movq_xmm4_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0040);
		}
		inline void movq_xmm5_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0050);
		}
		inline void movq_xmm6_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0060);
		}
		inline void movq_xmm7_prax(JitFuncCreater &jfc) {
			jfc.push(0xf3, 0x0f, 0x7e, 0070);
		}


		inline void mov_prcx_rax(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x89, 0001);
		}
		inline void mov_prcx_eax(JitFuncCreater &jfc) {
			jfc.push(0x89, 0001);
		}
		inline void mov_prcx_ax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x89, 0001);
		}
		inline void mov_prcx_al(JitFuncCreater &jfc) {
			jfc.push(0x88, 0001);
		}

		inline void mov_prbx_rax(JitFuncCreater &jfc) {
			jfc.push(0x48, 0x89, 0003);
		}
		inline void mov_prbx_eax(JitFuncCreater &jfc) {
			jfc.push(0x89, 0003);
		}
		inline void mov_prbx_ax(JitFuncCreater &jfc) {
			jfc.push(0x66, 0x89, 0003);
		}
		inline void mov_prbx_al(JitFuncCreater &jfc) {
			jfc.push(0x88, 0003);
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
			mov_rxq_u64(jfc, OpCode_x64::rax, dat);
		}

		inline void call_rax(JitFuncCreater &jfc) {
			jfc.push(0xff, 0xd0);
		}

		inline void push_uint32(JitFuncCreater &jfc, uint32_t dat) {
			jfc.push(0x68);
			jfc.push_uint32(dat);
		}

		template <byte V>
		inline void _add_sub_rx_byte(JitFuncCreater &jfc, Register dst, byte v) {
			assert(is_rx(dst));
			jfc.push(0x48 + (is_rpx0(dst) ? 0 : 1), 0x83, V + get_code(dst), v);
		}

		template <byte V>
		inline void _add_sub_rx_u32(JitFuncCreater &jfc, Register dst, uint32_t v) {
			assert(is_rx(dst));
			jfc.push(0x48 + (is_rpx0(dst) ? 0 : 1), 0x81, V + get_code(dst));
			jfc.push_uint32(v);
		}

		inline void add_rx_byte(JitFuncCreater &jfc, Register dst, byte v) {
			_add_sub_rx_byte<0300>(jfc, dst, v);
		}
		inline void add_rx_u32(JitFuncCreater &jfc, Register dst, uint32_t v) {
			_add_sub_rx_u32<0300>(jfc, dst, v);
		}
		inline void sub_rx_byte(JitFuncCreater &jfc, Register dst, byte v) {
			_add_sub_rx_byte<0350>(jfc, dst, v);
		}
		inline void sub_rx_u32(JitFuncCreater &jfc, Register dst, uint32_t v) {
			_add_sub_rx_u32<0350>(jfc, dst, v);
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
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::rcx, dat);
		}
		inline void add_int1(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::rdx, dat);
		}
		inline void add_int2(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::r8, dat);
		}
		inline void add_int3(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::r9, dat);
		}
		inline void add_intx(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::rax, dat);
			OpCode_x64::push(jfc, OpCode_x64::rax);
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
			OpCode_x64::push(jfc, OpCode_x64::rbx);
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
			OpCode_x64::push(jfc, OpCode_x64::prax);
		}

		inline void add_int0_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_rxd_u32(jfc, OpCode_x64::ecx, dat);
		}
		inline void add_int1_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_rxd_u32(jfc, OpCode_x64::edx, dat);
		}
		inline void add_int2_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_rxd_u32(jfc, OpCode_x64::r8d, dat);
		}
		inline void add_int3_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_rxd_u32(jfc, OpCode_x64::r9d, dat);
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
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::rdi, dat);
		}
		inline void add_int1(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::rsi, dat);
		}
		inline void add_int2(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::rdx, dat);
		}
		inline void add_int3(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::rcx, dat);
		}
		inline void add_int4(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::r8, dat);
		}
		inline void add_int5(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::r9, dat);
		}
		inline void add_intx(JitFuncCreater &jfc, uint64_t dat) {
			OpCode_x64::mov_rxq_u64(jfc, OpCode_x64::rax, dat);
			OpCode_x64::push(jfc, OpCode_x64::rax);
		}

		inline void add_int0_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_rxd_u32(jfc, OpCode_x64::edi, dat);
		}
		inline void add_int1_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_rxd_u32(jfc, OpCode_x64::esi, dat);
		}
		inline void add_int2_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_rxd_u32(jfc, OpCode_x64::edx, dat);
		}
		inline void add_int3_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_rxd_u32(jfc, OpCode_x64::ecx, dat);
		}
		inline void add_int4_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_rxd_u32(jfc, OpCode_x64::r8d, dat);
		}
		inline void add_int5_uint32(JitFuncCreater &jfc, uint32_t dat) {
			OpCode_x64::mov_rxd_u32(jfc, OpCode_x64::r9d, dat);
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
			OpCode_x64::push(jfc, OpCode_x64::rbx);
		}

		inline void add_int0_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rdi, OpCode_x64::prax);
		}
		inline void add_int1_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rsi, OpCode_x64::prax);
		}
		inline void add_int2_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rdx, OpCode_x64::prax);
		}
		inline void add_int3_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::rcx, OpCode_x64::prax);
		}
		inline void add_int4_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::r8, OpCode_x64::prax);
		}
		inline void add_int5_prax(JitFuncCreater &jfc) {
			OpCode_x64::mov(jfc, OpCode_x64::r9, OpCode_x64::prax);
		}
		inline void add_intx_prax(JitFuncCreater &jfc) {
			OpCode_x64::push(jfc, OpCode_x64::prax);
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
