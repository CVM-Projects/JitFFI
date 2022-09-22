#include <assert.h>
#include <string.h>

#include "jitfunc.h"

#if defined(_WIN32)
#	include <Windows.h>
#else
#	include <sys/mman.h>
#endif

#if !defined(MAP_JIT)
#   define MAP_JIT 0
#endif

enum access_mode {
    AM_NONE = 0x00,
    AM_READ = 0x01,
    AM_WRITE = 0x02,
    AM_EXEC = 0x04,
    AM_READWRITE = AM_READ | AM_WRITE,
    AM_READ_EXEC = AM_READ | AM_EXEC,
};

/* If failed, return -1 */
static int get_mode_flag(enum access_mode mode) {
#if defined(_WIN32)
    switch (mode) {
    case AM_NONE:
        return PAGE_NOACCESS;
    case AM_READ:
        return PAGE_READONLY;
    case AM_EXEC:
        return PAGE_EXECUTE;
    case AM_READWRITE:
        return PAGE_READWRITE;
    case AM_READ_EXEC:
        return PAGE_EXECUTE_READ;
    default:
        return -1;
    }
#else
    return mode;
#endif
}

/* if error occurs, return NULL */
static void* impl_jitfunc_alloc(size_t size, enum access_mode mode) {
    int flag = get_mode_flag(mode);
    assert(flag != -1);
#if defined(_WIN32)
    void *address = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, flag);
#else
    void *address = mmap(NULL, size, flag, MAP_ANON | MAP_PRIVATE | MAP_JIT, -1, 0);
    if (address == MAP_FAILED) {
        return NULL;
    }
#endif
    return address;
}

/* if no error, return 0 */
static int impl_jitfunc_free(void *address, size_t size) {
#if defined(_WIN32)
    BOOL v = VirtualFree(address, 0, MEM_RELEASE);
    return !v;
#else
    int v = munmap(address, size);
    return v;
#endif
}

/* if no error, return 0 */
static int impl_jitfunc_set_mode(void *address, size_t size, enum access_mode mode) {
    int flag = get_mode_flag(mode);
    assert(flag != -1);
#if defined(_WIN32)
    DWORD w;
    BOOL v = VirtualProtect(address, size, flag, &w);
    return !v;
#else
    int v = mprotect(address, size, flag);
    return v;
#endif
}

typedef struct impl_jitfunc {
    size_t total_size;
} impl_jitfunc;

static impl_jitfunc* impl_get_head(jitfunc func) {
    return (impl_jitfunc*)func - 1;
}

/* APIs */
jitfunc JITFUNC_API(alloc)(size_t size) {
    size_t total_size = sizeof(impl_jitfunc) + size;
    impl_jitfunc *head = (impl_jitfunc*)impl_jitfunc_alloc(total_size, AM_READWRITE);
    if (head == NULL) {
        return NULL;
    }
    head->total_size = total_size;
    return (impl_jitfunc*)head + 1;
}

int JITFUNC_API(free)(jitfunc func) {
    impl_jitfunc *head = impl_get_head(func);
    size_t size = head->total_size;
    return impl_jitfunc_free(head, size);
}

int JITFUNC_API(set_executable)(jitfunc func) {
    impl_jitfunc *head = impl_get_head(func);
    size_t size = head->total_size;
    return impl_jitfunc_set_mode(head, size, AM_READ_EXEC);
}

int JITFUNC_API(copy_from)(jitfunc func, const void *src, size_t src_size) {
    impl_jitfunc *head = impl_get_head(func);
    size_t func_size = head->total_size - sizeof(impl_jitfunc);
    if (func_size < src_size) {
        return -1;
    }
    memcpy(func, src, src_size);
    return 0;
}
