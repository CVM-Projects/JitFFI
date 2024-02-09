#include "x86_64.h"
#include <assert.h>

#define append_40_prefix_1(data, r) do { uint8_t _40_prefix = _get_40_prefix_value_1(r); if (_40_prefix) { *(data++) = _40_prefix; } } while (0)
#define append_40_prefix_2(data, r0, r1, rbits, cond_8bits) do { uint8_t _40_prefix = _get_40_prefix_value_2(r0, r1, rbits, cond_8bits); if (_40_prefix) { *(data++) = _40_prefix; } } while (0)


static size_t _get_reg_bits(enum jitcode_register_x86_64 r) {
    return 1 << (((r & 0xf0) >> 4) + 3);
}

static int _is_gpr(enum jitcode_register_x86_64 r) {
    return _get_reg_bits(r) <= 64;
}

static int _is_xmm(enum jitcode_register_x86_64 r) {
    return _get_reg_bits(r) == 128;
}

static uint8_t _get_40_prefix_value_1(enum jitcode_register_x86_64 r) {
    uint8_t base = (r & 0x8) >> 3;
    size_t bits = _get_reg_bits(r);
    if (base == 0 && (bits == 32 || bits == 16 || (bits == 8 && !(r & 0x4)))) {
        return 0;
    }
    return 0x40 + base + (bits == 64 ? 8 : 0);
}

static uint8_t _get_40_prefix_base_2(enum jitcode_register_x86_64 r0, enum jitcode_register_x86_64 r1) {
    return ((r1 & 0x8) >> 1) | ((r0 & 0x8) >> 3);
}

static uint8_t _get_40_prefix_value_2(enum jitcode_register_x86_64 r0, enum jitcode_register_x86_64 r1, enum jitcode_register_x86_64 rbits, int cond_8bits) {
    uint8_t base = _get_40_prefix_base_2(r0, r1);
    size_t bits = _get_reg_bits(rbits);
    if (base == 0 && (bits == 32 || bits == 16 || (bits == 8 && !cond_8bits))) {
        return 0;
    }
    return 0x40 + base + (bits == 64 ? 8 : 0);
}

static uint8_t _get_basecode_2(enum jitcode_register_x86_64 r0, enum jitcode_register_x86_64 r1) {
    return ((r1 & 0x7) << 3) + (r0 & 0x7);
}


static size_t _impl_mov_gpr_imm(uint8_t *data, enum jitcode_register_x86_64 r, uint64_t imm) {
    const uint8_t * const start = data;

    size_t bits = _get_reg_bits(r);

    if (bits == 16) {
        *(data++) = 0x66;
    }
    append_40_prefix_1(data, r);
    *(data++) = 0xb0 + (r & 0x7) + (bits == 8 ? 0 : 8);

    switch (bits) {
    case 64: *(uint64_t*)data = imm; break;
    case 32: *(uint32_t*)data = imm; break;
    case 16: *(uint16_t*)data = imm; break;
    case  8:  *(uint8_t*)data = imm; break;
    }
    data += (bits >> 3);

    return data - start;
}

static size_t _impl_mov_gpr_gpr(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src) {
    const uint8_t * const start = data;

    size_t bits = _get_reg_bits(dst);
    assert(bits == _get_reg_bits(src));

    if (bits == 16) {
        *(data++) = 0x66;
    }
    append_40_prefix_2(data, dst, src, src, ((dst & 0x4) | (src & 0x4)));
    *(data++) = (bits == 8) ? 0x88 : 0x89;
    *(data++) = 0xc0 + _get_basecode_2(dst, src);

    return data - start;
}

static size_t _impl_mov_pgpr_gpr(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src, int src_ptr) {
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
    append_40_prefix_2(data, ptr_r, drt_r, drt_r, drt_r & 0x4);
    *(data++) = (drt_bits == 8) ? (src_ptr ? 0x8a : 0x88) : (src_ptr ? 0x8b : 0x89);
    uint8_t basecode = _get_basecode_2(ptr_r, drt_r);
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

static size_t _impl_mov_xmm_gpr(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src, int src_xmm) {
    const uint8_t * const start = data;

    enum jitcode_register_x86_64 xmm_r = src_xmm ? src : dst;
    enum jitcode_register_x86_64 gpr_r = src_xmm ? dst : src;

    size_t gpr_bits = _get_reg_bits(gpr_r);
    assert(gpr_bits == 64 || gpr_bits == 32);

    *(data++) = 0x66;
    append_40_prefix_2(data, gpr_r, xmm_r, gpr_r, 0);
    *(data++) = 0x0f;
    *(data++) = src_xmm ? 0x7e : 0x6e;
    *(data++) = 0xc0 + _get_basecode_2(gpr_r, xmm_r);

    return data - start;
}


// %r = imm
size_t JITCODE_API(mov_r64_imm64_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint64_t imm64) {
    return _impl_mov_gpr_imm(data, r, imm64);
}
size_t JITCODE_API(mov_r32_imm32_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint32_t imm32) {
    return _impl_mov_gpr_imm(data, r, imm32);
}
size_t JITCODE_API(mov_r16_imm16_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint16_t imm16) {
    return _impl_mov_gpr_imm(data, r, imm16);
}
size_t JITCODE_API(mov_r8_imm8_x86_64)(uint8_t *data, enum jitcode_register_x86_64 r, uint8_t imm8) {
    return _impl_mov_gpr_imm(data, r, imm8);
}

// %r1 = %r2
size_t JITCODE_API(mov_r1_r2_x86_64)(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src) {
    if (_is_gpr(dst) && _is_gpr(src)) {
        return _impl_mov_gpr_gpr(data, dst, src);
    } else if (_is_xmm(dst) && _is_xmm(src)) {
        assert(0);
    } else {
        return _impl_mov_xmm_gpr(data, dst, src, _is_xmm(src));
    }
}

// *%r1 = %r2
size_t JITCODE_API(mov_pr1_r2_x86_64)(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src) {
    return _impl_mov_pgpr_gpr(data, dst, src, 0);
}

// %r1 = *%r2
size_t JITCODE_API(mov_r1_pr2_x86_64)(uint8_t *data, enum jitcode_register_x86_64 dst, enum jitcode_register_x86_64 src) {
    return _impl_mov_pgpr_gpr(data, dst, src, 1);
}
