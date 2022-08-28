#include <stddef.h>

enum pool_access_mode {
    PAM_NONE = 0x00,
    PAM_READ = 0x01,
    PAM_WRITE = 0x02,
    PAM_EXEC = 0x04,
    PAM_READWRITE = PAM_READ | PAM_WRITE,
    PAM_READ_EXEC = PAM_READ | PAM_EXEC,
};

typedef void* jitfuncpool;

jitfuncpool jitfuncpool_alloc(size_t size, enum pool_access_mode mode);
void jitfuncpool_set_pool_mode(jitfuncpool pool, size_t size, enum pool_access_mode mode);
void jitfuncpool_free(jitfuncpool pool, size_t size);
