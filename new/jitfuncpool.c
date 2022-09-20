#include <assert.h>
#include <string.h>

#include "jitfuncpool.h"

#if defined(_WIN32)
#	include <Windows.h>
#else
#	include <sys/mman.h>
#endif

#if !defined(MAP_JIT)
#   define MAP_JIT 0
#endif

enum pool_access_mode {
    PAM_NONE = 0x00,
    PAM_READ = 0x01,
    PAM_WRITE = 0x02,
    PAM_EXEC = 0x04,
    PAM_READWRITE = PAM_READ | PAM_WRITE,
    PAM_READ_EXEC = PAM_READ | PAM_EXEC,
};

/* If failed, return -1 */
static int get_mode_flag(enum pool_access_mode mode) {
#if defined(_WIN32)
    switch (mode) {
    case PAM_NONE:
        return PAGE_NOACCESS;
    case PAM_READ:
        return PAGE_READONLY;
    case PAM_EXEC:
        return PAGE_EXECUTE;
    case PAM_READWRITE:
        return PAGE_READWRITE;
    case PAM_READ_EXEC:
        return PAGE_EXECUTE_READ;
    default:
        return -1;
    }
#else
    return mode;
#endif
}

/* if error occurs, return NULL */
static void* impl_jitfuncpool_alloc(size_t size, enum pool_access_mode mode) {
    int flag = get_mode_flag(mode);
    assert(flag != -1);
#if defined(_WIN32)
    void *pool = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, flag);
#else
    void *pool = mmap(NULL, size, flag, MAP_ANON | MAP_PRIVATE | MAP_JIT, -1, 0);
    if (pool == MAP_FAILED) {
        return NULL;
    }
#endif
    return pool;
}

/* if no error, return 0 */
static int impl_jitfuncpool_free(void *pool, size_t size) {
#if defined(_WIN32)
    BOOL v = VirtualFree(pool, 0, MEM_RELEASE);
    return !v;
#else
    int v = munmap(pool, size);
    return v;
#endif
}

/* if no error, return 0 */
static int impl_jitfuncpool_set_pool_mode(void *pool, size_t size, enum pool_access_mode mode) {
    int flag = get_mode_flag(mode);
    assert(flag != -1);
#if defined(_WIN32)
    DWORD w;
    BOOL v = VirtualProtect(pool, size, flag, &w);
    return !v;
#else
    int v = mprotect(pool, size, flag);
    return v;
#endif
}

typedef struct impl_jitfuncpool {
    size_t pool_size;
} impl_jitfuncpool;

static size_t impl_get_pool_size(jitfuncpool pool) {
    return ((impl_jitfuncpool*)pool)->pool_size;
}

/* APIs */
jitfuncpool JITFUNCPOOL_API(alloc)(size_t size) {
    impl_jitfuncpool *pool = (impl_jitfuncpool*)impl_jitfuncpool_alloc(sizeof(size_t) + size, PAM_READWRITE);
    if (pool == NULL) {
        return NULL;
    }
    pool->pool_size = size + sizeof(size_t);
    return pool;
}

int JITFUNCPOOL_API(free)(jitfuncpool pool) {
    size_t size = impl_get_pool_size(pool);
    return impl_jitfuncpool_free(pool, size);
}

void* JITFUNCPOOL_API(get_func)(jitfuncpool pool) {
    return ((size_t*)pool) + 1;
}

int JITFUNCPOOL_API(set_executable)(jitfuncpool pool) {
    size_t size = impl_get_pool_size(pool);
    return impl_jitfuncpool_set_pool_mode(pool, size, PAM_READ_EXEC);
}

int JITFUNCPOOL_API(copy_from)(jitfuncpool pool, const void *src, size_t src_size) {
    void *func = JITFUNCPOOL_API(get_func)(pool);
    size_t size = impl_get_pool_size(pool) - sizeof(impl_jitfuncpool);
    if (size < src_size) {
        return -1;
    }
    memcpy(func, src, src_size);
    return 0;
}
