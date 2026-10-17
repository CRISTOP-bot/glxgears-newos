/* Fixed arena for TinyGL. NEWOS currently has no sbrk; this bounded arena
 * makes allocation failures explicit instead of silently using host malloc. */
#include <TGL/gl.h>

#define ARENA_BYTES (8u * 1024u * 1024u)
#define ALIGNMENT 16u

static unsigned char arena[ARENA_BYTES] __attribute__((aligned(ALIGNMENT)));
static unsigned long arena_used;

void *gl_malloc(GLint size)
{
    unsigned long n, start;
    if (size <= 0)
        return 0;
    n = (unsigned long)size;
    start = (arena_used + (ALIGNMENT - 1u)) & ~(ALIGNMENT - 1u);
    if (start > ARENA_BYTES || n > ARENA_BYTES - start)
        return 0;
    arena_used = start + n;
    return &arena[start];
}

void *gl_zalloc(GLint size)
{
    unsigned char *p = gl_malloc(size);
    unsigned long i;
    if (!p)
        return 0;
    for (i = 0; i < (unsigned long)size; ++i)
        p[i] = 0;
    return p;
}

/* TinyGL allocates its long-lived context and buffers before the frame loop.
 * This process exits after its requested frame count, so per-object freeing
 * is intentionally unnecessary; the kernel reclaims the address space. */
void gl_free(void *p)
{
    (void)p;
}
