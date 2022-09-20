#include <stddef.h>

#if !defined(JITFUNCPOOL_API)
#   define JITFUNCPOOL_API(ident) jitfuncpool_##ident
#endif

typedef void* jitfuncpool;

jitfuncpool JITFUNCPOOL_API(alloc)(size_t size);  /* if error occurs, return NULL */
int JITFUNCPOOL_API(free)(jitfuncpool pool);  /* if no error, return 0 */
void* JITFUNCPOOL_API(get_func)(jitfuncpool pool);
int JITFUNCPOOL_API(set_executable)(jitfuncpool pool);  /* if no error, return 0 */
int JITFUNCPOOL_API(copy_from)(jitfuncpool pool, const void *src, size_t src_size);  /* if no error, return 0 */
