#include "x86_64.h"
#include <assert.h>

#define append_40_prefix(data, r) do { uint8_t _40_prefix = _get_40_prefix_value(r); if (_40_prefix) { *(data++) = _40_prefix; } } while (0)
#define append_40_prefix_2(data, r, v) do { uint8_t _40_prefix = _get_40_prefix_value(r); if (_40_prefix) { *(data++) = _40_prefix + (v); } else if (v) { *(data++) = 0x40 + (v); } } while (0)


static size_t _get_reg_bits(enum jitcode_register_x86_64 r) {
    return 1 << (((r & 0xf0) >> 4) + 3);
}

static uint8_t _get_40_prefix_value(enum jitcode_register_x86_64 r) {
    // r64      -> 48, 49
    // r32, r16 -> _, 41
    // r8       -> _, 40, 41
    size_t bits = _get_reg_bits(r);
    int is_rb = r & 0x8;  // rb: r8 ~ r15
    switch (bits) {
    case 64:
        return is_rb ? 0x49 : 0x48;
    case 32:
    case 16:
        return is_rb ? 0x41 : 0;
    case 8:
        return is_rb ? 0x41 : (r & 0x4 ? 0x40 : 0);
    default:
        return 0;
    }
}

static size_t _impl_mov_reg_reg(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src) {
    const uint8_t * const start = data;

    size_t bits = _get_reg_bits(dst);
    assert(bits == _get_reg_bits(src));

    if (bits == 16) {
        *(data++) = 0x66;
    }
    uint8_t _40_prefix = _get_40_prefix_value(dst);
    if (bits == 8 && _40_prefix == 0 && !(dst < 4 && src < 4)) {
        *(data++) = 0x40 + ((src & 0x8) >> 1);
    } else 
    {
        append_40_prefix_2(data, dst, (src & 0x8) >> 1);
    }
    *(data++) = (bits == 8) ? 0x88 : 0x89;
    *(data++) = 0xc0 + ((src & 0x7) * 8) + (dst & 0x7);

    return data - start;
}

static size_t _impl_mov_preg_reg(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src, int src_ptr) {
    const uint8_t * const start = data;

    size_t dst_bits = _get_reg_bits(dst);
    size_t src_bits = _get_reg_bits(src);

    assert(dst_bits == 64 || dst_bits == 32);
    assert(src_bits <= dst_bits);  // src bits <= dst bits

    if (dst_bits == 32) {
        *(data++) = 0x67;
    }
    if (src_bits == 16) {
        *(data++) = 0x66;
    }
    append_40_prefix_2(data, src, (src & 0x8) >> 1);
    *(data++) = (dst_bits == 8) ? (src_ptr ? 0x8a : 0x88) : (src_ptr ? 0x8b : 0x89);
    uint8_t basecode = (src & 0x7) * 8 + (dst & 0x7);
    if ((dst & 0x7) == 0x4) {
        *(data++) = basecode;
        *(data++) = 0x24;
    } else if ((dst & 0x7) == 0x5) {
        *(data++) = 0x40 + basecode;
        *(data++) = 0x00;
    } else {
        *(data++) = basecode;
    }

    return data - start;
}

#define append_imm(data, type, imm) do { *(type*)data = imm; data += sizeof(type); } while (0)


// %r = imm
size_t JITCODE_API(mov_r64_imm64_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint64_t imm64) {
    const uint8_t * const start = data;

    append_40_prefix(data, r);
    *(data++) = 0xb8 + (r & 0x7);
    append_imm(data, uint64_t, imm64);

    return data - start;
}
size_t JITCODE_API(mov_r32_imm32_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint32_t imm32) {
    const uint8_t * const start = data;

    append_40_prefix(data, r);
    *(data++) = 0xb8 + (r & 0x7);
    append_imm(data, uint32_t, imm32);

    return data - start;
}
size_t JITCODE_API(mov_r16_imm16_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint16_t imm16) {
    const uint8_t * const start = data;

    *(data++) = 0x66;
    append_40_prefix(data, r);
    *(data++) = 0xb8 + (r & 0x7);
    append_imm(data, uint16_t, imm16);

    return data - start;
}
size_t JITCODE_API(mov_r8_imm8_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint8_t imm8) {
    const uint8_t * const start = data;

    append_40_prefix(data, r);
    *(data++) = 0xb0 + (r & 0x7);
    append_imm(data, uint8_t, imm8);

    return data - start;
}

// %r1 = %r2
size_t JITCODE_API(mov_r1_r2_x86_64)(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src) {
    return _impl_mov_reg_reg(data, dst, src);
}

// *%r1 = %r2
size_t JITCODE_API(mov_pr1_r2_x86_64)(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src) {
    return _impl_mov_preg_reg(data, dst, src, 0);
}

// %r1 = *%r2
size_t JITCODE_API(mov_r1_pr2_x86_64)(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src) {
    return _impl_mov_preg_reg(data, dst, src, 1);
}
