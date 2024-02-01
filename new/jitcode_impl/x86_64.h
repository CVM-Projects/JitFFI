#pragma once
#ifndef __JITFFI_JITCODE_IMPL_X86_64_H__
#define __JITFFI_JITCODE_IMPL_X86_64_H__
#include "../jitcode.h"

enum jitcode_register_x86_64 {
    // 64 bits
    r64_x86_64_rax  = 0x30 | 0x0,
    r64_x86_64_rcx  = 0x30 | 0x1,
    r64_x86_64_rdx  = 0x30 | 0x2,
    r64_x86_64_rbx  = 0x30 | 0x3,
    r64_x86_64_rsp  = 0x30 | 0x4,
    r64_x86_64_rbp  = 0x30 | 0x5,
    r64_x86_64_rsi  = 0x30 | 0x6,
    r64_x86_64_rdi  = 0x30 | 0x7,
    r64_x86_64_r8   = 0x30 | 0x8,
    r64_x86_64_r9   = 0x30 | 0x9,
    r64_x86_64_r10  = 0x30 | 0xa,
    r64_x86_64_r11  = 0x30 | 0xb,
    r64_x86_64_r12  = 0x30 | 0xc,
    r64_x86_64_r13  = 0x30 | 0xd,
    r64_x86_64_r14  = 0x30 | 0xe,
    r64_x86_64_r15  = 0x30 | 0xf,
    // 32 bits
    r32_x86_64_eax  = 0x20 | 0x0,
    r32_x86_64_ecx  = 0x20 | 0x1,
    r32_x86_64_edx  = 0x20 | 0x2,
    r32_x86_64_ebx  = 0x20 | 0x3,
    r32_x86_64_esp  = 0x20 | 0x4,
    r32_x86_64_ebp  = 0x20 | 0x5,
    r32_x86_64_esi  = 0x20 | 0x6,
    r32_x86_64_edi  = 0x20 | 0x7,
    r32_x86_64_r8d  = 0x20 | 0x8,
    r32_x86_64_r9d  = 0x20 | 0x9,
    r32_x86_64_r10d = 0x20 | 0xa,
    r32_x86_64_r11d = 0x20 | 0xb,
    r32_x86_64_r12d = 0x20 | 0xc,
    r32_x86_64_r13d = 0x20 | 0xd,
    r32_x86_64_r14d = 0x20 | 0xe,
    r32_x86_64_r15d = 0x20 | 0xf,
    // 16 bits
    r16_x86_64_ax   = 0x10 | 0x0,
    r16_x86_64_cx   = 0x10 | 0x1,
    r16_x86_64_dx   = 0x10 | 0x2,
    r16_x86_64_bx   = 0x10 | 0x3,
    r16_x86_64_sp   = 0x10 | 0x4,
    r16_x86_64_bp   = 0x10 | 0x5,
    r16_x86_64_si   = 0x10 | 0x6,
    r16_x86_64_di   = 0x10 | 0x7,
    r16_x86_64_r8w  = 0x10 | 0x8,
    r16_x86_64_r9w  = 0x10 | 0x9,
    r16_x86_64_r10w = 0x10 | 0xa,
    r16_x86_64_r11w = 0x10 | 0xb,
    r16_x86_64_r12w = 0x10 | 0xc,
    r16_x86_64_r13w = 0x10 | 0xd,
    r16_x86_64_r14w = 0x10 | 0xe,
    r16_x86_64_r15w = 0x10 | 0xf,
    // 8 bits
    r8_x86_64_al    = 0x00 | 0x0,
    r8_x86_64_cl    = 0x00 | 0x1,
    r8_x86_64_dl    = 0x00 | 0x2,
    r8_x86_64_bl    = 0x00 | 0x3,
    r8_x86_64_spl   = 0x00 | 0x4,
    r8_x86_64_bpl   = 0x00 | 0x5,
    r8_x86_64_sil   = 0x00 | 0x6,
    r8_x86_64_dil   = 0x00 | 0x7,
    r8_x86_64_r8b   = 0x00 | 0x8,
    r8_x86_64_r9b   = 0x00 | 0x9,
    r8_x86_64_r10b  = 0x00 | 0xa,
    r8_x86_64_r11b  = 0x00 | 0xb,
    r8_x86_64_r12b  = 0x00 | 0xc,
    r8_x86_64_r13b  = 0x00 | 0xd,
    r8_x86_64_r14b  = 0x00 | 0xe,
    r8_x86_64_r15b  = 0x00 | 0xf,
};

// %r = imm
size_t JITCODE_API(mov_r64_imm64_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint64_t imm64);
size_t JITCODE_API(mov_r32_imm32_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint32_t imm32);
size_t JITCODE_API(mov_r16_imm16_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint16_t imm16);
size_t JITCODE_API(mov_r8_imm8_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint8_t imm8);

// %r1 = %r2
size_t JITCODE_API(mov_r1_r2_x86_64)(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src);

// *%r1 = %r2
size_t JITCODE_API(mov_pr1_r2_x86_64)(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src);

// %r1 = *%r2
size_t JITCODE_API(mov_r1_pr2_x86_64)(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src);

#endif
