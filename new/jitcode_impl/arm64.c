#include "arm64.h"
#include <assert.h>

static size_t _get_reg_bits(enum jitcode_register_arm64 r) {
    return 1 << (((r & 0xf00) >> 8) + 5);
}

static void _set_code(uint8_t **data, uint32_t code) {
    *((uint64_t*)(*data)) = code;
    *data += 4;
}

enum lsl_bits {
    lsl_b_0 = 0,
    lsl_b_16 = 1,
    lsl_b_32 = 2,
    lsl_b_48 = 3,
};

static uint32_t _impl_movz_reg(enum jitcode_register_arm64 r, uint16_t imm16, enum lsl_bits lsl_bits) {
    uint32_t code = (((_get_reg_bits(r) == 64) ? 0x694 : 0x294) + lsl_bits) << 16 << 5;
    return code | (imm16 << 5) | (r & 0xff);
}
static uint32_t _impl_movk_reg(enum jitcode_register_arm64 r, uint16_t imm16, enum lsl_bits lsl_bits) {
    uint32_t code = (((_get_reg_bits(r) == 64) ? 0x794 : 0x394) + lsl_bits) << 16 << 5;
    return code | (imm16 << 5) | (r & 0xff);
}

// %r = imm
size_t JITCODE_API(mov_r64_imm64_arm64)(uint8_t *data, enum jitcode_register_arm64 r, uint64_t imm64) {
    assert(_get_reg_bits(r) == 64);
    uint8_t *current = data;
    uint16_t imm_16vec[4] = {
        (imm64 & 0x000000000000ffffull),
        (imm64 & 0x00000000ffff0000ull) >> 16,
        (imm64 & 0x0000ffff00000000ull) >> 32,
        (imm64 & 0xffff000000000000ull) >> 48,
    };
    _set_code(&current, _impl_movz_reg(r, imm_16vec[0], lsl_b_0));
    if (imm_16vec[1] != 0) {
        _set_code(&current, _impl_movk_reg(r, imm_16vec[1], lsl_b_16));
    }
    if (imm_16vec[2] != 0) {
        _set_code(&current, _impl_movk_reg(r, imm_16vec[2], lsl_b_32));
    }
    if (imm_16vec[3] != 0) {
        _set_code(&current, _impl_movk_reg(r, imm_16vec[3], lsl_b_48));
    }
    return current - data;
}

size_t JITCODE_API(mov_r32_imm32_arm64)(uint8_t *data, enum jitcode_register_arm64 r, uint32_t imm32) {
    assert(_get_reg_bits(r) == 32);
    uint8_t *current = data;
    uint16_t imm_16vec[2] = {
        (imm32 & 0x0000fffful),
        (imm32 & 0xffff0000ul) >> 16,
    };
    _set_code(&current, _impl_movz_reg(r, imm_16vec[0], lsl_b_0));
    if (imm_16vec[1] != 0) {
        _set_code(&current, _impl_movk_reg(r, imm_16vec[1], lsl_b_16));
    }
    return current - data;
}

// return
size_t JITCODE_API(return_arm64)(uint8_t *data) {
    *(uint32_t*)data = 0xd65f03c0;
    return 4;
}
