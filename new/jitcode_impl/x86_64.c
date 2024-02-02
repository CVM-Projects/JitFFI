#include "x86_64.h"
#include <assert.h>

#define append_40_prefix(data, r) do { uint8_t _40_prefix = _get_40_prefix_value(r); if (_40_prefix) { *(data++) = _40_prefix; } } while (0)
#define append_40_prefix_2(data, dst, src, cond_8bits) do { uint8_t _40_prefix = _get_40_prefix_value_2(dst, src, cond_8bits); if (_40_prefix) { *(data++) = _40_prefix; } } while (0)


static size_t _get_reg_bits(enum jitcode_register_x86_64 r) {
    return 1 << (((r & 0xf0) >> 4) + 3);
}

static uint8_t _get_40_prefix_value(enum jitcode_register_x86_64 r) {
    uint8_t base = (r & 0x8) >> 3;
    size_t bits = _get_reg_bits(r);
    if (base == 0 && (bits == 32 || bits == 16 || (bits == 8 && !(r & 0x4)))) {
        return 0;
    }
    return 0x40 + base + (bits == 64 ? 8 : 0);
}

static uint8_t _get_40_prefix_value_2(enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src, int cond_8bits) {
    uint8_t base = ((src & 0x8) >> 1) | ((dst & 0x8) >> 3);
    size_t src_bits = _get_reg_bits(src);
    if (base == 0 && (src_bits == 32 || src_bits == 16 || (src_bits == 8 && !cond_8bits))) {
        return 0;
    }
    return 0x40 + base + (src_bits == 64 ? 8 : 0);
}

static size_t _impl_mov_reg_reg(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src) {
    const uint8_t * const start = data;

    size_t bits = _get_reg_bits(dst);
    assert(bits == _get_reg_bits(src));

    if (bits == 16) {
        *(data++) = 0x66;
    }
    append_40_prefix_2(data, dst, src, ((dst & 0x4) | (src & 0x4)));
    *(data++) = (bits == 8) ? 0x88 : 0x89;
    *(data++) = 0xc0 + ((src & 0x7) * 8) + (dst & 0x7);

    return data - start;
}

static size_t _impl_mov_preg_reg(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src, int src_ptr) {
    const uint8_t * const start = data;

    enum jitcode_register_x86_64 ptr_r = src_ptr ? src : dst;
    enum jitcode_register_x86_64 drt_r = src_ptr ? dst : src;

    size_t ptr_bits = _get_reg_bits(ptr_r);
    size_t drt_bits = _get_reg_bits(drt_r);

    assert(ptr_bits == 64 || ptr_bits == 32);

    if (ptr_bits == 32) {
        *(data++) = 0x67;
    }
    if (drt_bits == 16) {
        *(data++) = 0x66;
    }
    append_40_prefix_2(data, ptr_r, drt_r, drt_r & 0x4);
    *(data++) = (drt_bits == 8) ? (src_ptr ? 0x8a : 0x88) : (src_ptr ? 0x8b : 0x89);
    uint8_t basecode = (drt_r & 0x7) * 8 + (ptr_r & 0x7);
    if ((ptr_r & 0x7) == 0x4) {
        *(data++) = basecode;
        *(data++) = 0x24;
    } else if ((ptr_r & 0x7) == 0x5) {
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
