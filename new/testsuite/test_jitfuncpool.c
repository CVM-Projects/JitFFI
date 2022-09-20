#include <stdint.h>
#include <assert.h>

#include "../jitfuncpool.h"

int add(int x, int y) {
    return x + y;
}

typedef int func(int, int);

int main() {
    size_t size = 20 * sizeof(uint32_t);
    int r = 0;
    jitfuncpool pool = jitfuncpool_alloc(size);
    assert(pool != NULL);
    r = jitfuncpool_copy_from(pool, (const void*)&add, size);
    assert(r == 0);
    r = jitfuncpool_set_executable(pool);
    assert(r == 0);

    func *f = (func*)jitfuncpool_get_func(pool);
    assert(f(5, 6) == 11);

    r = jitfuncpool_free(pool);
    assert(r == 0);

    return 0;
}
