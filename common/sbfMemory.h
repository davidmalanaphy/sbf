#ifndef _SBF_MEMORY_H_
#define _SBF_MEMORY_H_

#include "sbfCommon.h"

SBF_BEGIN_DECLS

void* sbfMemory_malloc (size_t size);
#define xmalloc sbfMemory_malloc
void* sbfMemory_calloc (size_t nmemb, size_t size);
#define xcalloc sbfMemory_calloc
void* sbfMemory_realloc (void* ptr, size_t nmemb, size_t size);
#define xrealloc sbfMemory_realloc
char* sbfMemory_strdup (const char* s);
#define xstrdup sbfMemory_strdup

void sbfMemory_asprintf (char** ret, const char* fmt, ...);
#define xasprintf sbfMemory_asprintf
void sbfMemory_vasprintf (char** ret, const char* fmt, va_list ap);
#define xvasprintf sbfMemory_vasprintf

SBF_END_DECLS

#endif /* _SBF_MEMORY_H_ */
