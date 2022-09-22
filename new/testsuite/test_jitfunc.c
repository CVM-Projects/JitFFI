#include <stdint.h>
#include <assert.h>

#include "../jitfunc.h"

int add(int x, int y) {
    return x + y;
}

typedef int func(int, int);

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

    return 0;
}
