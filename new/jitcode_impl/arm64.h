#pragma once
#ifndef __JITFFI_JITCODE_IMPL_ARM64_H__
#define __JITFFI_JITCODE_IMPL_ARM64_H__
#include "../jitcode.h"

enum jitcode_register_arm64 {
    // 64 bits
    r64_arm64_x0  = 0x100 | 0x00,
    r64_arm64_x1  = 0x100 | 0x01,
    r64_arm64_x2  = 0x100 | 0x02,
    r64_arm64_x3  = 0x100 | 0x03,
    r64_arm64_x4  = 0x100 | 0x04,
    r64_arm64_x5  = 0x100 | 0x05,
    r64_arm64_x6  = 0x100 | 0x06,
    r64_arm64_x7  = 0x100 | 0x07,
    r64_arm64_x8  = 0x100 | 0x08,
    r64_arm64_x9  = 0x100 | 0x09,
    r64_arm64_x10 = 0x100 | 0x0a,
    r64_arm64_x11 = 0x100 | 0x0b,
    r64_arm64_x12 = 0x100 | 0x0c,
    r64_arm64_x13 = 0x100 | 0x0d,
    r64_arm64_x14 = 0x100 | 0x0e,
    r64_arm64_x15 = 0x100 | 0x0f,
    r64_arm64_x16 = 0x100 | 0x10,
    r64_arm64_x17 = 0x100 | 0x11,
    r64_arm64_x18 = 0x100 | 0x12,
    r64_arm64_x19 = 0x100 | 0x13,
    r64_arm64_x20 = 0x100 | 0x14,
    r64_arm64_x21 = 0x100 | 0x15,
    r64_arm64_x22 = 0x100 | 0x16,
    r64_arm64_x23 = 0x100 | 0x17,
    r64_arm64_x24 = 0x100 | 0x18,
    r64_arm64_x25 = 0x100 | 0x19,
    r64_arm64_x26 = 0x100 | 0x1a,
    r64_arm64_x27 = 0x100 | 0x1b,
    r64_arm64_x28 = 0x100 | 0x1c,
    r64_arm64_x29 = 0x100 | 0x1d,
    r64_arm64_x30 = 0x100 | 0x1e,
    r64_arm64_x31 = 0x100 | 0x1f,
    // 32 bits
    r32_arm64_w0  = 0x000 | 0x00,
    r32_arm64_w1  = 0x000 | 0x01,
    r32_arm64_w2  = 0x000 | 0x02,
    r32_arm64_w3  = 0x000 | 0x03,
    r32_arm64_w4  = 0x000 | 0x04,
    r32_arm64_w5  = 0x000 | 0x05,
    r32_arm64_w6  = 0x000 | 0x06,
    r32_arm64_w7  = 0x000 | 0x07,
    r32_arm64_w8  = 0x000 | 0x08,
    r32_arm64_w9  = 0x000 | 0x09,
    r32_arm64_w10 = 0x000 | 0x0a,
    r32_arm64_w11 = 0x000 | 0x0b,
    r32_arm64_w12 = 0x000 | 0x0c,
    r32_arm64_w13 = 0x000 | 0x0d,
    r32_arm64_w14 = 0x000 | 0x0e,
    r32_arm64_w15 = 0x000 | 0x0f,
    r32_arm64_w16 = 0x000 | 0x10,
    r32_arm64_w17 = 0x000 | 0x11,
    r32_arm64_w18 = 0x000 | 0x12,
    r32_arm64_w19 = 0x000 | 0x13,
    r32_arm64_w20 = 0x000 | 0x14,
    r32_arm64_w21 = 0x000 | 0x15,
    r32_arm64_w22 = 0x000 | 0x16,
    r32_arm64_w23 = 0x000 | 0x17,
    r32_arm64_w24 = 0x000 | 0x18,
    r32_arm64_w25 = 0x000 | 0x19,
    r32_arm64_w26 = 0x000 | 0x1a,
    r32_arm64_w27 = 0x000 | 0x1b,
    r32_arm64_w28 = 0x000 | 0x1c,
    r32_arm64_w29 = 0x000 | 0x1d,
    r32_arm64_w30 = 0x000 | 0x1e,
    r32_arm64_w31 = 0x000 | 0x1f,
};

// %r = imm
size_t JITCODE_API(mov_r64_imm64_arm64)(uint8_t *data, enum jitcode_register_arm64 r, uint64_t imm64);
size_t JITCODE_API(mov_r32_imm32_arm64)(uint8_t *data, enum jitcode_register_arm64 r, uint32_t imm32);

// %r1 = %r2
size_t JITCODE_API(mov_r1_r2_arm64)(uint8_t *data, enum jitcode_register_arm64 dst, enum jitcode_register_arm64 src);

// *%r1 = %r2
size_t JITCODE_API(mov_pr1_r2_arm64)(uint8_t *data, enum jitcode_register_arm64 dst, enum jitcode_register_arm64 src);

// %r1 = *%r2
size_t JITCODE_API(mov_r1_pr2_arm64)(uint8_t *data, enum jitcode_register_arm64 dst, enum jitcode_register_arm64 src);

// return
size_t JITCODE_API(return_arm64)(uint8_t *data);

#endif
