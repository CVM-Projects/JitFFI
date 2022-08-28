#include <assert.h>

#include "jitfuncpool.h"

#if defined(_WIN32)
#	include <Windows.h>
#else
#	include <sys/mman.h>
#endif

#if !defined(MAP_JIT)
#   define MAP_JIT 0
#endif

// If failed, return -1
static int get_mode_flag(int mode) {
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

void* jitfuncpool_alloc(size_t size, enum pool_access_mode mode) {
    int flag = get_mode_flag(mode);
    assert(flag != -1);
#if defined(_WIN32)
    void *pool = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, flag);
    assert(pool != NULL);
#else
    void *pool = mmap(NULL, size, flag, MAP_ANON | MAP_PRIVATE | MAP_JIT, -1, 0);
    assert(pool != NULL && pool != MAP_FAILED);
#endif
    return pool;
}

void jitfuncpool_set_pool_mode(void *pool, size_t size, enum pool_access_mode mode) {
    int flag = get_mode_flag(mode);
    assert(flag != -1);
#if defined(_WIN32)
    DWORD w;
    BOOL v = VirtualProtect(pool, size, flag, &w);
    assert(v);
#else
    int v = mprotect(pool, size, flag);
    assert(v == 0);
#endif
}

void jitfuncpool_free(void *pool, size_t size) {
#if defined(_WIN32)
    BOOL v = VirtualFree(pool, 0, MEM_RELEASE);
    assert(v);
#else
    int v = munmap(pool, size);
    assert(v == 0);
#endif
}
