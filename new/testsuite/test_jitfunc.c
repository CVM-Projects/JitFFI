#include <stdint.h>
#include <assert.h>

#include "../jitfunc.h"

int add(int x, int y) {
    return x + y;
}

typedef int func(int, int);

// #include "../jitcode_impl/x86_64.h"
#include "../jitcode_impl/arm64.h"

#include <stdio.h>

int main() {
    size_t size = 20 * sizeof(uint32_t);
    int r = 0;
    jitfunc the_func = jitfunc_alloc(size);
    assert(the_func != NULL);
    r = jitfunc_copy_from(the_func, (const void*)&add, size);
    assert(r == 0);
    r = jitfunc_set_executable(the_func);
    assert(r == 0);

    func *f = (func*)the_func;
    assert(f(5, 6) == 11);

    r = jitfunc_free(the_func);
    assert(r == 0);

    jitfunc new_func = jitfunc_alloc(0x10000);
    assert(new_func != NULL);
    uint8_t *raw = (uint8_t*)new_func;
    size_t s = 0;
    s = jitcode_mov_r64_imm64_arm64(raw, r64_arm64_x0, 0x123456789abcdef0);
    raw += s;
    *(uint32_t*)raw = 0x52824680;
    raw += 4;
    s = jitcode_return_arm64(raw);
    raw += s;
    r = jitfunc_set_executable(the_func);
    assert(r == 0);
    typedef int64_t func_1();
    func_1 *ff = (func_1*)new_func;
    printf("0x%llx\n", ff());

    return 0;
}
