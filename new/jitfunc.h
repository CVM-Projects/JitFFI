#include <stddef.h>

#if !defined(JITFUNC_API)
#   define JITFUNC_API(ident) jitfunc_##ident
#endif

#if !defined(JITFUNC_TYPE)
#   define  JITFUNC_TYPE void*
#endif

typedef JITFUNC_TYPE jitfunc;

jitfunc JITFUNC_API(alloc)(size_t size);  /* if error occurs, return NULL */
int JITFUNC_API(free)(jitfunc func);  /* if no error, return 0 */
int JITFUNC_API(set_executable)(jitfunc func);  /* if no error, return 0 */
int JITFUNC_API(copy_from)(jitfunc func, const void *src, size_t src_size);  /* if no error, return 0 */
