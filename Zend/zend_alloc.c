/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2000 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 0.92 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        | 
   | available at through the world-wide-web at                           |
   | http://www.zend.com/license/0_92.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/


#include <stdlib.h>

#include "zend.h"
#include "zend_alloc.h"
#include "zend_globals.h"
#include "zend_fast_cache.h"
#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifndef ZTS
ZEND_API zend_alloc_globals alloc_globals;
#endif


#define ZEND_DISABLE_MEMORY_CACHE 0


#if ZEND_DEBUG
# define END_MAGIC_SIZE sizeof(long)
# define END_ALIGNMENT(size) (((size)%PLATFORM_ALIGNMENT)?(PLATFORM_ALIGNMENT-((size)%PLATFORM_ALIGNMENT)):0)
#else
# define END_MAGIC_SIZE 0
# define END_ALIGNMENT(size) 0
#endif


# if MEMORY_LIMIT
#  if ZEND_DEBUG
#define CHECK_MEMORY_LIMIT(s) _CHECK_MEMORY_LIMIT(s, __zend_filename, __zend_lineno)
#  else
#define CHECK_MEMORY_LIMIT(s)	_CHECK_MEMORY_LIMIT(s,NULL,0)
#  endif

#define _CHECK_MEMORY_LIMIT(s,file,lineno) { AG(allocated_memory) += (s);\
								if (AG(memory_limit)<AG(allocated_memory)) {\
									if (!file) { \
										zend_error(E_ERROR,"Allowed memory size of %d bytes exhausted (tried to allocate %d bytes)", AG(memory_limit),s); \
									} else { \
										zend_error(E_ERROR,"Allowed memory size of %d bytes exhausted at %s:%d (tried to allocate %d bytes)", AG(memory_limit), file, lineno, s); \
									} \
								} \
							}
# endif

#ifndef CHECK_MEMORY_LIMIT
#define CHECK_MEMORY_LIMIT(s)
#endif


#define REMOVE_POINTER_FROM_LIST(p)				\
	if (!p->persistent && p==AG(head)) {		\
		AG(head) = p->pNext;					\
	} else if (p->persistent && p==AG(phead)) {	\
		AG(phead) = p->pNext;					\
	} else {									\
		p->pLast->pNext = p->pNext;				\
	}											\
	if (p->pNext) {								\
		p->pNext->pLast = p->pLast;				\
	}

#define ADD_POINTER_TO_LIST(p)		\
	if (p->persistent) {			\
		p->pNext = AG(phead);		\
		if (AG(phead)) {			\
			AG(phead)->pLast = p;	\
		}							\
		AG(phead) = p;				\
	} else {						\
		p->pNext = AG(head);		\
		if (AG(head)) {				\
			AG(head)->pLast = p;	\
		}							\
		AG(head) = p;				\
	}								\
	p->pLast = (zend_mem_header *) NULL;



ZEND_API void *_emalloc(size_t size ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
	zend_mem_header *p;
	ALS_FETCH();

	if (!ZEND_DISABLE_MEMORY_CACHE && (size < MAX_CACHED_MEMORY) && (AG(cache_count)[size] > 0)) {
		p = AG(cache)[size][--AG(cache_count)[size]];
#if ZEND_DEBUG
		p->filename = __zend_filename;
		p->lineno = __zend_lineno;
		p->orig_filename = __zend_orig_filename;
		p->orig_lineno = __zend_orig_lineno;
		p->magic = MEM_BLOCK_START_MAGIC;
		p->reported = 0;
		AG(cache_stats)[size][1]++;
#endif
		p->persistent = 0;
		p->cached = 0;
		return (void *)((char *)p + sizeof(zend_mem_header) + PLATFORM_PADDING);
	} else {
#if ZEND_DEBUG
		if (size<MAX_CACHED_MEMORY) {
			AG(cache_stats)[size][0]++;
		}
#endif
		p  = (zend_mem_header *) malloc(sizeof(zend_mem_header) + size + PLATFORM_PADDING + END_ALIGNMENT(size) + END_MAGIC_SIZE);
	}

	HANDLE_BLOCK_INTERRUPTIONS();

	if (!p) {
		fprintf(stderr,"FATAL:  emalloc():  Unable to allocate %ld bytes\n", (long) size);
#if ZEND_DEBUG && defined(HAVE_KILL) && defined(HAVE_GETPID)
		kill(getpid(), SIGSEGV);
#else
		exit(1);
#endif
		HANDLE_UNBLOCK_INTERRUPTIONS();
		return (void *)p;
	}
	p->persistent = p->cached = 0;
	ADD_POINTER_TO_LIST(p);
	p->size = size;
#if ZEND_DEBUG
	p->filename = __zend_filename;
	p->lineno = __zend_lineno;
	p->orig_filename = __zend_orig_filename;
	p->orig_lineno = __zend_orig_lineno;
	p->magic = MEM_BLOCK_START_MAGIC;
	p->reported = 0;
	*((long *)(((char *) p) + sizeof(zend_mem_header)+size+PLATFORM_PADDING+END_ALIGNMENT(size))) = MEM_BLOCK_END_MAGIC;
#endif
#if MEMORY_LIMIT
	CHECK_MEMORY_LIMIT(size);
#endif
	HANDLE_UNBLOCK_INTERRUPTIONS();
	return (void *)((char *)p + sizeof(zend_mem_header) + PLATFORM_PADDING);
}


ZEND_API void _efree(void *ptr ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
	zend_mem_header *p = (zend_mem_header *) ((char *)ptr - sizeof(zend_mem_header) - PLATFORM_PADDING);
	ALS_FETCH();

#if ZEND_DEBUG
	if (!_mem_block_check(ptr, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC)) {
		return;
	}
	memset(ptr, 0x5a, p->size);
#endif

	if (!ZEND_DISABLE_MEMORY_CACHE 
		&& !p->persistent && (p->size < MAX_CACHED_MEMORY) && (AG(cache_count)[p->size] < MAX_CACHED_ENTRIES)) {
		AG(cache)[p->size][AG(cache_count)[p->size]++] = p;
		p->cached = 1;
#if ZEND_DEBUG
		p->magic = MEM_BLOCK_CACHED_MAGIC;
#endif
		return;
	}
	HANDLE_BLOCK_INTERRUPTIONS();
	REMOVE_POINTER_FROM_LIST(p);

#if MEMORY_LIMIT
	AG(allocated_memory) -= p->size;
#endif
	
	free(p);
	HANDLE_UNBLOCK_INTERRUPTIONS();
}


ZEND_API void *_ecalloc(size_t nmemb, size_t size ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
	void *p;
	int final_size=size*nmemb;

	HANDLE_BLOCK_INTERRUPTIONS();
	p = _emalloc(final_size ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
	if (!p) {
		HANDLE_UNBLOCK_INTERRUPTIONS();
		return (void *) p;
	}
	memset(p,(int)NULL,final_size);
	HANDLE_UNBLOCK_INTERRUPTIONS();
	return p;
}


ZEND_API void *_erealloc(void *ptr, size_t size, int allow_failure ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
	zend_mem_header *p = (zend_mem_header *) ((char *)ptr-sizeof(zend_mem_header)-PLATFORM_PADDING);
	zend_mem_header *orig = p;
	ALS_FETCH();

	if (!ptr) {
		return _emalloc(size ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
	}
	HANDLE_BLOCK_INTERRUPTIONS();
	REMOVE_POINTER_FROM_LIST(p);
	p = (zend_mem_header *) realloc(p,sizeof(zend_mem_header)+size+PLATFORM_PADDING+END_ALIGNMENT(size)+END_MAGIC_SIZE);
	if (!p) {
		if (!allow_failure) {
			fprintf(stderr,"FATAL:  erealloc():  Unable to allocate %ld bytes\n", (long) size);
#if ZEND_DEBUG && HAVE_KILL && HAVE_GETPID
			kill(getpid(), SIGSEGV);
#else
			exit(1);
#endif
		}
		ADD_POINTER_TO_LIST(orig);
		HANDLE_UNBLOCK_INTERRUPTIONS();
		return (void *)NULL;
	}
	ADD_POINTER_TO_LIST(p);
#if ZEND_DEBUG
	p->filename = __zend_filename;
	p->lineno = __zend_lineno;
	p->magic = MEM_BLOCK_START_MAGIC;
	*((long *)(((char *) p) + sizeof(zend_mem_header)+size+PLATFORM_PADDING+END_ALIGNMENT(size))) = MEM_BLOCK_END_MAGIC;
#endif	
#if MEMORY_LIMIT
	CHECK_MEMORY_LIMIT(size - p->size);
#endif
	p->size = size;

	HANDLE_UNBLOCK_INTERRUPTIONS();
	return (void *)((char *)p+sizeof(zend_mem_header)+PLATFORM_PADDING);
}


ZEND_API char *_estrdup(const char *s ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
	int length;
	char *p;

	length = strlen(s)+1;
	HANDLE_BLOCK_INTERRUPTIONS();
	p = (char *) _emalloc(length ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
	if (!p) {
		HANDLE_UNBLOCK_INTERRUPTIONS();
		return (char *)NULL;
	}
	HANDLE_UNBLOCK_INTERRUPTIONS();
	memcpy(p,s,length);
	return p;
}


ZEND_API char *_estrndup(const char *s, uint length ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
	char *p;

	HANDLE_BLOCK_INTERRUPTIONS();
	p = (char *) _emalloc(length+1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
	if (!p) {
		HANDLE_UNBLOCK_INTERRUPTIONS();
		return (char *)NULL;
	}
	HANDLE_UNBLOCK_INTERRUPTIONS();
	memcpy(p,s,length);
	p[length]=0;
	return p;
}


ZEND_API char *zend_strndup(const char *s, uint length)
{
	char *p;

	p = (char *) malloc(length+1);
	if (!p) {
		return (char *)NULL;
	}
	if (length) {
		memcpy(p,s,length);
	}
	p[length]=0;
	return p;
}


ZEND_API int zend_set_memory_limit(unsigned int memory_limit)
{
#if MEMORY_LIMIT
	ALS_FETCH();

	AG(memory_limit) = memory_limit;
	return SUCCESS;
#else
	return FAILURE;
#endif
}


ZEND_API void start_memory_manager(ALS_D)
{
	int i, j;
	void *cached_entries[MAX_CACHED_MEMORY][MAX_CACHED_ENTRIES];

	AG(phead) = AG(head) = NULL;
	
#if MEMORY_LIMIT
	AG(memory_limit)=1<<30;		/* rediculous limit, effectively no limit */
	AG(allocated_memory)=0;
	AG(memory_exhausted)=0;
#endif

#if ZEND_DEBUG
	memset(AG(cache_stats), 0, sizeof(AG(cache_stats)));
	memset(AG(fast_cache_stats), 0, sizeof(AG(fast_cache_stats)));
#endif

	memset(AG(fast_cache_list_head), 0, sizeof(AG(fast_cache_list_head)));
	memset(AG(cache_count),0,MAX_CACHED_MEMORY*sizeof(unsigned char));

	/* Initialize cache, to prevent fragmentation */
	for (i=1; i<MAX_CACHED_MEMORY; i++) {
		for (j=0; j<MAX_CACHED_ENTRIES; j++) {
			cached_entries[i][j] = emalloc(i);
		}
	}
	for (i=1; i<MAX_CACHED_MEMORY; i++) {
		for (j=0; j<MAX_CACHED_ENTRIES; j++) {
			efree(cached_entries[i][j]);
		}
	}
}


ZEND_API void shutdown_memory_manager(int silent, int clean_cache)
{
	zend_mem_header *p, *t;
	int fci;
#if ZEND_DEBUG
	int had_leaks=0;
#endif
	zend_fast_cache_list_entry *fast_cache_list_entry, *next_fast_cache_list_entry;
	ALS_FETCH();

	
	for (fci=0; fci<MAX_FAST_CACHE_TYPES; fci++) {
		fast_cache_list_entry = AG(fast_cache_list_head)[fci];
		while (fast_cache_list_entry) {
			next_fast_cache_list_entry = fast_cache_list_entry->next;
			efree(fast_cache_list_entry);
			fast_cache_list_entry = next_fast_cache_list_entry;
		}
		AG(fast_cache_list_head)[fci] = NULL;
	}

	p=AG(head);
	t=AG(head);
	while (t) {
		if (!t->cached || clean_cache) {
#if ZEND_DEBUG
			if (!t->cached && !t->reported) {
				zend_mem_header *iterator;
				int total_leak=0, total_leak_count=0;

				had_leaks=1;
				if (!silent) {
					zend_message_dispatcher(ZMSG_MEMORY_LEAK_DETECTED, t);
				}
				t->reported = 1;
				for (iterator=t->pNext; iterator; iterator=iterator->pNext) {
					if (!iterator->cached
						&& iterator->filename==t->filename
						&& iterator->lineno==t->lineno) {
						total_leak += iterator->size;
						total_leak_count++;
						iterator->reported = 1;
					}
				}
				if (!silent && total_leak_count>0) {
					zend_message_dispatcher(ZMSG_MEMORY_LEAK_REPEATED, (void *) (long) (total_leak_count));
				}
			}
#endif
			p = t->pNext;
			REMOVE_POINTER_FROM_LIST(t);
			free(t);
			t = p;
		} else {
			t = t->pNext;
		}
	}

#if (ZEND_DEBUG)
	do {
		zval display_memory_cache_stats;
		int i, j;

		if (clean_cache) {
			/* we're shutting down completely, don't even touch the INI subsystem */
			break;
		}
		if (zend_get_ini_entry("display_memory_cache_stats", sizeof("display_memory_cache_stats"), &display_memory_cache_stats)==FAILURE) {
			break;
		}
		if (!atoi(display_memory_cache_stats.value.str.val)) {
			break;
		}
		fprintf(stderr, "Memory cache statistics\n"
						"-----------------------\n\n"
						"[zval, %2ld]\t\t%d / %d (%.2f%%)\n"
						"[hash, %2ld]\t\t%d / %d (%.2f%%)\n",
						(long) sizeof(zval),
						AG(fast_cache_stats)[ZVAL_CACHE_LIST][1], AG(fast_cache_stats)[ZVAL_CACHE_LIST][0]+AG(fast_cache_stats)[ZVAL_CACHE_LIST][1],
						((double) AG(fast_cache_stats)[ZVAL_CACHE_LIST][1] / (AG(fast_cache_stats)[ZVAL_CACHE_LIST][0]+AG(fast_cache_stats)[ZVAL_CACHE_LIST][1]))*100,
						(long) sizeof(HashTable),
						AG(fast_cache_stats)[HASHTABLE_CACHE_LIST][1], AG(fast_cache_stats)[HASHTABLE_CACHE_LIST][0]+AG(fast_cache_stats)[HASHTABLE_CACHE_LIST][1],
						((double) AG(fast_cache_stats)[HASHTABLE_CACHE_LIST][1] / (AG(fast_cache_stats)[HASHTABLE_CACHE_LIST][0]+AG(fast_cache_stats)[HASHTABLE_CACHE_LIST][1]))*100);


		for (i=0; i<MAX_CACHED_MEMORY; i+=2) {
			fprintf(stderr, "[%2d, %2d]\t\t", i, i+1);
			for (j=0; j<2; j++) {
				fprintf(stderr, "%d / %d (%.2f%%)\t\t",
						AG(cache_stats)[i+j][1], AG(cache_stats)[i+j][0]+AG(cache_stats)[i+j][1],
						((double) AG(cache_stats)[i+j][1] / (AG(cache_stats)[i+j][0]+AG(cache_stats)[i+j][1]))*100);
			}
			fprintf(stderr, "\n");
		}
					
	} while (0);
#endif
}


#if ZEND_DEBUG
void zend_debug_alloc_output(char *format, ...)
{
	char output_buf[256];
	va_list args;

	va_start(args, format);
	vsprintf(output_buf, format, args);
	va_end(args);

#ifdef ZEND_WIN32
	OutputDebugString(output_buf);
#else
	fprintf(stderr, output_buf);
#endif
}


ZEND_API int _mem_block_check(void *ptr, int silent ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
	zend_mem_header *p = (zend_mem_header *) ((char *)ptr - sizeof(zend_mem_header) - PLATFORM_PADDING);
	int no_cache_notice=0;
	int valid_beginning=1;
	int had_problems=0;

	if (silent==2) {
		silent=1;
		no_cache_notice=1;
	}
	if (silent==3) {
		silent=0;
		no_cache_notice=1;
	}
	if (!silent) {
		zend_message_dispatcher(ZMSG_LOG_SCRIPT_NAME, NULL);
		zend_debug_alloc_output("---------------------------------------\n");
		zend_debug_alloc_output("%s(%d) : Block 0x%0.8lX status:\n" ZEND_FILE_LINE_RELAY_CC, (long) p);
		if (__zend_orig_filename) {
			zend_debug_alloc_output("%s(%d) : Actual location (location was relayed)\n" ZEND_FILE_LINE_ORIG_RELAY_CC);
		}
		zend_debug_alloc_output("%10s\t","Beginning:  ");
	}

	switch (p->magic) {
		case MEM_BLOCK_START_MAGIC:
			if (!silent) {
				zend_debug_alloc_output("OK (allocated on %s:%d, %d bytes)\n", p->filename, p->lineno, p->size);
			}
			break; /* ok */
		case MEM_BLOCK_FREED_MAGIC:
			if (!silent) {
				zend_debug_alloc_output("Freed\n");
				had_problems=1;
			} else {
				return _mem_block_check(ptr, 0 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
			}
			break;
		case MEM_BLOCK_CACHED_MAGIC:
			if (!silent) {
				if (!no_cache_notice) {
					zend_debug_alloc_output("Cached (allocated on %s:%d, %d bytes)\n", p->filename, p->lineno, p->size);
					had_problems=1;
				}
			} else {
				if (!no_cache_notice) {
					return _mem_block_check(ptr, 0 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
				}
			}
			break;
		default:
			if (!silent) {
				zend_debug_alloc_output("Overrun (magic=0x%0.8lX, expected=0x%0.8lX)\n", p->magic, MEM_BLOCK_START_MAGIC);
			} else {
				return _mem_block_check(ptr, 0 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
			}
			had_problems=1;
			valid_beginning=0;
			break;
	}


	if (valid_beginning
		&& *((long *)(((char *) p)+sizeof(zend_mem_header)+p->size+PLATFORM_PADDING+END_ALIGNMENT(p->size))) != MEM_BLOCK_END_MAGIC) {
		long magic_num = MEM_BLOCK_END_MAGIC;
		char *overflow_ptr, *magic_ptr=(char *) &magic_num;
		int overflows=0;
		int i;

		if (silent) {
			return _mem_block_check(ptr, 0 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
		}
		had_problems=1;
		overflow_ptr = ((char *) p)+sizeof(zend_mem_header)+p->size+PLATFORM_PADDING;

		for (i=0; i<sizeof(long); i++) {
			if (overflow_ptr[i]!=magic_ptr[i]) {
				overflows++;
			}
		}

		zend_debug_alloc_output("%10s\t", "End:");
		zend_debug_alloc_output("Overflown (magic=0x%0.8lX instead of 0x%0.8lX)\n", 
				*((long *)(((char *) p) + sizeof(zend_mem_header)+p->size+PLATFORM_PADDING+END_ALIGNMENT(p->size))), MEM_BLOCK_END_MAGIC);
		zend_debug_alloc_output("%10s\t","");
		if (overflows>=sizeof(long)) {
			zend_debug_alloc_output("At least %d bytes overflown\n", sizeof(long));
		} else {
			zend_debug_alloc_output("%d byte(s) overflown\n", overflows);
		}
	} else if (!silent) {
		zend_debug_alloc_output("%10s\t", "End:");
		if (valid_beginning) {
			zend_debug_alloc_output("OK\n");
		} else {
			zend_debug_alloc_output("Unknown\n");
		}
	}

	if (had_problems) {
		int foo = 5;

		foo+=1;
	}
		
	if (!silent) {
		zend_debug_alloc_output("---------------------------------------\n");
	}
	return ((!had_problems) ? 1 : 0);
}


ZEND_API void _full_mem_check(int silent ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
	zend_mem_header *p;
	int errors=0;
	ALS_FETCH();

	p = AG(head);
	

	zend_debug_alloc_output("------------------------------------------------\n");
	zend_debug_alloc_output("Full Memory Check at %s:%d\n" ZEND_FILE_LINE_RELAY_CC);

	while (p) {
		if (!_mem_block_check((void *)((char *)p + sizeof(zend_mem_header) + PLATFORM_PADDING), (silent?2:3) ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC)) {
			errors++;
		}
		p = p->pNext;
	}
	zend_debug_alloc_output("End of full memory check %s:%d (%d errors)\n" ZEND_FILE_LINE_RELAY_CC, errors);
	zend_debug_alloc_output("------------------------------------------------\n");
}
#endif


ZEND_API int _persist_alloc(void *ptr ZEND_FILE_LINE_DC ZEND_FILE_LINE_ORIG_DC)
{
	zend_mem_header *p = (zend_mem_header *) ((char *)ptr-sizeof(zend_mem_header)-PLATFORM_PADDING);
	ALS_FETCH();

#if ZEND_DEBUG
	_mem_block_check(ptr, 1 ZEND_FILE_LINE_RELAY_CC ZEND_FILE_LINE_ORIG_RELAY_CC);
#endif

	HANDLE_BLOCK_INTERRUPTIONS();

	/* remove the block from the non persistent list */
	REMOVE_POINTER_FROM_LIST(p);

	p->persistent = 1;

	/* add the block to the persistent list */
	ADD_POINTER_TO_LIST(p);
	HANDLE_UNBLOCK_INTERRUPTIONS();
	return p->size+sizeof(zend_mem_header)+PLATFORM_PADDING;
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
