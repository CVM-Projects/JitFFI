#include <stdint.h>
#include <assert.h>
#include <printf.h>
#include <string.h>

#include "../jitfuncpool.h"

int add(int x, int y) {
    return x + y;
}

typedef int func(int, int);

int main() {
    size_t size = 8 * sizeof(uint32_t);
    jitfuncpool pool = jitfuncpool_alloc(size, PAM_READWRITE);

    memcpy(pool, &add, size);

    jitfuncpool_set_pool_mode(pool, size, PAM_READ_EXEC);

    func *f = pool;
    assert(f(5, 6) == 11);

    jitfuncpool_free(pool, size);
}
