/**********************************************************************

  regint.h -  Oniguruma (regular expression library)

  Copyright (C) 2002-2004  K.Kosako (kosako@sofnec.co.jp)

**********************************************************************/
#ifndef REGINT_H
#define REGINT_H

/* for debug */
/* #define ONIG_DEBUG_PARSE_TREE */
/* #define ONIG_DEBUG_COMPILE */
/* #define ONIG_DEBUG_SEARCH */
/* #define ONIG_DEBUG_MATCH */
/* #define ONIG_DONT_OPTIMIZE */

/* for byte-code statistical data. */
/* #define ONIG_DEBUG_STATISTICS */

#if defined(ONIG_DEBUG_PARSE_TREE) || defined(ONIG_DEBUG_MATCH) || \
    defined(ONIG_DEBUG_COMPILE) || defined(ONIG_DEBUG_STATISTICS)
#ifndef ONIG_DEBUG
#define ONIG_DEBUG
#endif
#endif

#if defined(__i386) || defined(__i386__) || defined(_M_IX86) || \
    (defined(__ppc__) && defined(__APPLE__)) || \
    defined(__x86_64) || defined(__x86_64__) || \
    defined(__mc68020__)
#define PLATFORM_UNALIGNED_WORD_ACCESS
#endif

/* config */
/* spec. config */
#define USE_NAMED_GROUP
#define USE_SUBEXP_CALL
#define USE_FOLD_MATCH                                  /* ess-tsett etc... */
#define USE_INFINITE_REPEAT_MONOMANIAC_MEM_STATUS_CHECK /* /(?:()|())*\2/ */
#define USE_NEWLINE_AT_END_OF_STRING_HAS_EMPTY_LINE     /* /\n$/ =~ "\n" */
#define USE_WARNING_REDUNDANT_NESTED_REPEAT_OPERATOR
/* internal config */
#define USE_RECYCLE_NODE
#define USE_OP_PUSH_OR_JUMP_EXACT
#define USE_QUALIFIER_PEEK_NEXT

#define INIT_MATCH_STACK_SIZE                     160
#define MATCH_STACK_LIMIT_SIZE                 500000

/* interface to external system */
#ifdef NOT_RUBY      /* gived from Makefile */
#include "config.h"
#define USE_VARIABLE_META_CHARS
#define USE_VARIABLE_SYNTAX
#define USE_WORD_BEGIN_END          /* "\<": word-begin, "\>": word-end */
#define USE_POSIX_REGION_OPTION     /* needed for POSIX API support */
#define THREAD_ATOMIC_START         /* depend on thread system */
#define THREAD_ATOMIC_END           /* depend on thread system */
#define THREAD_PASS                 /* depend on thread system */
#define xmalloc     malloc
#define xrealloc    realloc
#define xfree       free
#else
#include "ruby.h"
#include "version.h"
#include "rubysig.h"      /* for DEFER_INTS, ENABLE_INTS */
#define THREAD_ATOMIC_START          DEFER_INTS
#define THREAD_ATOMIC_END            ENABLE_INTS
#define THREAD_PASS                  rb_thread_schedule()
#define DEFAULT_WARN_FUNCTION        rb_warn
#define DEFAULT_VERB_WARN_FUNCTION   rb_warning

#if defined(RUBY_VERSION_MAJOR)
#if RUBY_VERSION_MAJOR > 1 || \
(RUBY_VERSION_MAJOR == 1 && \
 defined(RUBY_VERSION_MINOR) && RUBY_VERSION_MINOR >= 8)
#define USE_ST_HASH_TABLE
#endif
#endif

#endif /* else NOT_RUBY */

#define THREAD_PASS_LIMIT_COUNT    10
#define xmemset     memset
#define xmemcpy     memcpy
#define xmemmove    memmove
#if defined(_WIN32) && !defined(__CYGWIN__)
#define xalloca     _alloca
#ifdef NOT_RUBY
#define vsnprintf   _vsnprintf
#endif
#else
#define xalloca     alloca
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if defined(HAVE_ALLOCA_H) && !defined(__GNUC__)
#include <alloca.h>
#endif

#ifdef HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

#include <ctype.h>
#include <sys/types.h>

#ifdef ONIG_DEBUG
# include <stdio.h>
#endif

#include "regenc.h"
#include "oniguruma.h"

#ifdef MIN
#undef MIN
#endif
#ifdef MAX
#undef MAX
#endif
#define MIN(a,b) (((a)>(b))?(b):(a))
#define MAX(a,b) (((a)<(b))?(b):(a))

#define IS_NULL(p)                    (((void*)(p)) == (void*)0)
#define IS_NOT_NULL(p)                (((void*)(p)) != (void*)0)
#define CHECK_NULL_RETURN(p)          if (IS_NULL(p)) return NULL
#define CHECK_NULL_RETURN_VAL(p,val)  if (IS_NULL(p)) return (val)
#define NULL_UCHARP                   ((UChar* )0)

#ifndef PLATFORM_UNALIGNED_WORD_ACCESS
#define WORD_ALIGNMENT_SIZE       SIZEOF_INT

#define GET_ALIGNMENT_PAD_SIZE(addr,pad_size) do {\
  (pad_size) = WORD_ALIGNMENT_SIZE \
               - ((unsigned int )(addr) % WORD_ALIGNMENT_SIZE);\
  if ((pad_size) == WORD_ALIGNMENT_SIZE) (pad_size) = 0;\
} while (0)

#define ALIGNMENT_RIGHT(addr) do {\
  (addr) += (WORD_ALIGNMENT_SIZE - 1);\
  (addr) -= ((unsigned int )(addr) % WORD_ALIGNMENT_SIZE);\
} while (0)


#define B_SHIFT  8
#define B_MASK   0xff

#define SERIALIZE_2BYTE_INT(i,p) do {\
  *(p)     = ((i) >> B_SHIFT) & B_MASK;\
  *((p)+1) = (i) & B_MASK;\
} while (0)

#define SERIALIZE_4BYTE_INT(i,p) do {\
  *(p)     = ((i) >> B_SHIFT*3) & B_MASK;\
  *((p)+1) = ((i) >> B_SHIFT*2) & B_MASK;\
  *((p)+2) = ((i) >> B_SHIFT  ) & B_MASK;\
  *((p)+3) = (i) & B_MASK;\
} while (0)

#define SERIALIZE_8BYTE_INT(i,p) do {\
  *(p)     = ((i) >> B_SHIFT*7) & B_MASK;\
  *((p)+1) = ((i) >> B_SHIFT*6) & B_MASK;\
  *((p)+2) = ((i) >> B_SHIFT*5) & B_MASK;\
  *((p)+3) = ((i) >> B_SHIFT*4) & B_MASK;\
  *((p)+4) = ((i) >> B_SHIFT*3) & B_MASK;\
  *((p)+5) = ((i) >> B_SHIFT*2) & B_MASK;\
  *((p)+6) = ((i) >> B_SHIFT  ) & B_MASK;\
  *((p)+7) = (i) & B_MASK;\
} while (0)

#define GET_2BYTE_INT_INC(type,i,p) do {\
  (i) = (type )(((unsigned int )(*(p)) << B_SHIFT) | (unsigned int )((p)[1]));\
  (p) += 2;\
} while (0)

#define GET_4BYTE_INT_INC(type,i,p) do {\
  (i) = (type )(((unsigned int )((p)[0]) << B_SHIFT*3) | \
		((unsigned int )((p)[1]) << B_SHIFT*2) | \
		((unsigned int )((p)[2]) << B_SHIFT  ) | \
		((unsigned int )((p)[3])             )); \
  (p) += 4;\
} while (0)

#define GET_8BYTE_INT_INC(type,i,p) do {\
  (i) = (type )(((unsigned long )((p)[0]) << B_SHIFT*7) | \
		((unsigned long )((p)[1]) << B_SHIFT*6) | \
		((unsigned long )((p)[2]) << B_SHIFT*5) | \
		((unsigned long )((p)[3]) << B_SHIFT*4) | \
		((unsigned long )((p)[4]) << B_SHIFT*3) | \
		((unsigned long )((p)[5]) << B_SHIFT*2) | \
		((unsigned long )((p)[6]) << B_SHIFT  ) | \
		((unsigned long )((p)[7])             )); \
  (p) += 8;\
} while (0)

#if SIZEOF_SHORT == 2
#define GET_SHORT_INC(i,p)     GET_2BYTE_INT_INC(short,i,p)
#define SERIALIZE_SHORT(i,p)   SERIALIZE_2BYTE_INT(i,p)
#elif SIZEOF_SHORT == 4
#define GET_SHORT_INC(i,p)     GET_4BYTE_INT_INC(short,i,p)
#define SERIALIZE_SHORT(i,p)   SERIALIZE_4BYTE_INT(i,p)
#elif SIZEOF_SHORT == 8
#define GET_SHORT_INC(i,p)     GET_8BYTE_INT_INC(short,i,p)
#define SERIALIZE_SHORT(i,p)   SERIALIZE_8BYTE_INT(i,p)
#endif

#if SIZEOF_INT == 2
#define GET_INT_INC(i,p)       GET_2BYTE_INT_INC(int,i,p)
#define GET_UINT_INC(i,p)      GET_2BYTE_INT_INC(unsigned,i,p)
#define SERIALIZE_INT(i,p)     SERIALIZE_2BYTE_INT(i,p)
#define SERIALIZE_UINT(i,p)    SERIALIZE_2BYTE_INT(i,p)
#elif SIZEOF_INT == 4
#define GET_INT_INC(i,p)       GET_4BYTE_INT_INC(int,i,p)
#define GET_UINT_INC(i,p)      GET_4BYTE_INT_INC(unsigned,i,p)
#define SERIALIZE_INT(i,p)     SERIALIZE_4BYTE_INT(i,p)
#define SERIALIZE_UINT(i,p)    SERIALIZE_4BYTE_INT(i,p)
#elif SIZEOF_INT == 8
#define GET_INT_INC(i,p)       GET_8BYTE_INT_INC(int,i,p)
#define GET_UINT_INC(i,p)      GET_8BYTE_INT_INC(unsigned,i,p)
#define SERIALIZE_INT(i,p)     SERIALIZE_8BYTE_INT(i,p)
#define SERIALIZE_UINT(i,p)    SERIALIZE_8BYTE_INT(i,p)
#endif

#endif /* PLATFORM_UNALIGNED_WORD_ACCESS */

/* stack pop level */
#define STACK_POP_LEVEL_FREE        0
#define STACK_POP_LEVEL_MEM_START   1
#define STACK_POP_LEVEL_ALL         2

/* optimize flags */
#define ONIG_OPTIMIZE_NONE              0
#define ONIG_OPTIMIZE_EXACT             1   /* Slow Search */
#define ONIG_OPTIMIZE_EXACT_BM          2   /* Boyer Moore Search */
#define ONIG_OPTIMIZE_EXACT_BM_NOT_REV  3   /* BM   (but not simple match) */
#define ONIG_OPTIMIZE_EXACT_IC          4   /* Slow Search (ignore case) */
#define ONIG_OPTIMIZE_MAP               5   /* char map */

/* bit status */
typedef unsigned int  BitStatusType;

#define BIT_STATUS_BITS_NUM          (sizeof(BitStatusType) * 8)
#define BIT_STATUS_CLEAR(stats)      (stats) = 0
#define BIT_STATUS_ON_ALL(stats)     (stats) = ~((BitStatusType )0)
#define BIT_STATUS_AT(stats,n) \
  ((n) < BIT_STATUS_BITS_NUM  ?  ((stats) & (1 << n)) : ((stats) & 1))

#define BIT_STATUS_ON_AT(stats,n) do {\
  if ((n) < BIT_STATUS_BITS_NUM)\
    (stats) |= (1 << (n));\
  else\
    (stats) |= 1;\
} while (0)

#define BIT_STATUS_ON_AT_SIMPLE(stats,n) do {\
  if ((n) < BIT_STATUS_BITS_NUM)\
    (stats) |= (1 << (n));\
} while (0)


#define INT_MAX_LIMIT           ((1UL << (SIZEOF_INT * 8 - 1)) - 1)

#define DIGITVAL(code)    ((code) - '0')
#define ODIGITVAL(code)   DIGITVAL(code)
#define XDIGITVAL(enc,code) \
  (ONIGENC_IS_CODE_DIGIT(enc,code) ? DIGITVAL(code) \
   : (ONIGENC_IS_CODE_UPPER(enc,code) ? (code) - 'A' + 10 : (code) - 'a' + 10))

#define IS_SINGLELINE(option)     ((option) & ONIG_OPTION_SINGLELINE)
#define IS_MULTILINE(option)      ((option) & ONIG_OPTION_MULTILINE)
#define IS_IGNORECASE(option)     ((option) & ONIG_OPTION_IGNORECASE)
#define IS_EXTEND(option)         ((option) & ONIG_OPTION_EXTEND)
#define IS_FIND_LONGEST(option)   ((option) & ONIG_OPTION_FIND_LONGEST)
#define IS_FIND_NOT_EMPTY(option) ((option) & ONIG_OPTION_FIND_NOT_EMPTY)
#define IS_POSIXLINE(option)      (IS_SINGLELINE(option) && IS_MULTILINE(option))
#define IS_FIND_CONDITION(option) ((option) & \
          (ONIG_OPTION_FIND_LONGEST | ONIG_OPTION_FIND_NOT_EMPTY))
#define IS_NOTBOL(option)         ((option) & ONIG_OPTION_NOTBOL)
#define IS_NOTEOL(option)         ((option) & ONIG_OPTION_NOTEOL)
#define IS_POSIX_REGION(option)   ((option) & ONIG_OPTION_POSIX_REGION)

/* OP_SET_OPTION is required for these options.
#define IS_DYNAMIC_OPTION(option) \
  (((option) & (ONIG_OPTION_MULTILINE | ONIG_OPTION_IGNORECASE)) != 0)
*/
/* ignore-case and multibyte status are included in compiled code. */
#define IS_DYNAMIC_OPTION(option)  0


/* bitset */
#define BITS_PER_BYTE      8
#define SINGLE_BYTE_SIZE   (1 << BITS_PER_BYTE)
#define BITS_IN_ROOM       (sizeof(Bits) * BITS_PER_BYTE)
#define BITSET_SIZE        (SINGLE_BYTE_SIZE / BITS_IN_ROOM)

#ifdef PLATFORM_UNALIGNED_WORD_ACCESS
typedef unsigned int   Bits;
#else
typedef unsigned char  Bits;
#endif
typedef Bits           BitSet[BITSET_SIZE];
typedef Bits*          BitSetRef;

#define SIZE_BITSET        sizeof(BitSet)

#define BITSET_CLEAR(bs) do {\
  int i;\
  for (i = 0; i < BITSET_SIZE; i++) { (bs)[i] = 0; }\
} while (0)

#define BS_ROOM(bs,pos)            (bs)[pos / BITS_IN_ROOM]
#define BS_BIT(pos)                (1 << (pos % BITS_IN_ROOM))

#define BITSET_AT(bs, pos)         (BS_ROOM(bs,pos) & BS_BIT(pos))
#define BITSET_SET_BIT(bs, pos)     BS_ROOM(bs,pos) |= BS_BIT(pos)
#define BITSET_CLEAR_BIT(bs, pos)   BS_ROOM(bs,pos) &= ~(BS_BIT(pos))
#define BITSET_INVERT_BIT(bs, pos)  BS_ROOM(bs,pos) ^= BS_BIT(pos)

/* bytes buffer */
typedef struct _BBuf {
  UChar* p;
  unsigned int used;
  unsigned int alloc;
} BBuf;

#define BBUF_INIT(buf,size)    onig_bbuf_init((BBuf* )(buf), (size))

#define BBUF_SIZE_INC(buf,inc) do{\
  (buf)->alloc += (inc);\
  (buf)->p = (UChar* )xrealloc((buf)->p, (buf)->alloc);\
  if (IS_NULL((buf)->p)) return(ONIGERR_MEMORY);\
} while (0)

#define BBUF_EXPAND(buf,low) do{\
  do { (buf)->alloc *= 2; } while ((buf)->alloc < (unsigned int )low);\
  (buf)->p = (UChar* )xrealloc((buf)->p, (buf)->alloc);\
  if (IS_NULL((buf)->p)) return(ONIGERR_MEMORY);\
} while (0)

#define BBUF_ENSURE_SIZE(buf,size) do{\
  unsigned int new_alloc = (buf)->alloc;\
  while (new_alloc < (unsigned int )(size)) { new_alloc *= 2; }\
  if ((buf)->alloc != new_alloc) {\
    (buf)->p = (UChar* )xrealloc((buf)->p, new_alloc);\
    if (IS_NULL((buf)->p)) return(ONIGERR_MEMORY);\
    (buf)->alloc = new_alloc;\
  }\
} while (0)

#define BBUF_WRITE(buf,pos,bytes,n) do{\
  int used = (pos) + (n);\
  if ((buf)->alloc < (unsigned int )used) BBUF_EXPAND((buf),used);\
  xmemcpy((buf)->p + (pos), (bytes), (n));\
  if ((buf)->used < (unsigned int )used) (buf)->used = used;\
} while (0)

#define BBUF_WRITE1(buf,pos,byte) do{\
  int used = (pos) + 1;\
  if ((buf)->alloc < (unsigned int )used) BBUF_EXPAND((buf),used);\
  (buf)->p[(pos)] = (byte);\
  if ((buf)->used < (unsigned int )used) (buf)->used = used;\
} while (0)

#define BBUF_ADD(buf,bytes,n)       BBUF_WRITE((buf),(buf)->used,(bytes),(n))
#define BBUF_ADD1(buf,byte)         BBUF_WRITE1((buf),(buf)->used,(byte))
#define BBUF_GET_ADD_ADDRESS(buf)   ((buf)->p + (buf)->used)
#define BBUF_GET_OFFSET_POS(buf)    ((buf)->used)

/* from < to */
#define BBUF_MOVE_RIGHT(buf,from,to,n) do {\
  if ((unsigned int )((to)+(n)) > (buf)->alloc) BBUF_EXPAND((buf),(to) + (n));\
  xmemmove((buf)->p + (to), (buf)->p + (from), (n));\
  if ((unsigned int )((to)+(n)) > (buf)->used) (buf)->used = (to) + (n);\
} while (0)

/* from > to */
#define BBUF_MOVE_LEFT(buf,from,to,n) do {\
  xmemmove((buf)->p + (to), (buf)->p + (from), (n));\
} while (0)

/* from > to */
#define BBUF_MOVE_LEFT_REDUCE(buf,from,to) do {\
  xmemmove((buf)->p + (to), (buf)->p + (from), (buf)->used - (from));\
  (buf)->used -= (from - to);\
} while (0)

#define BBUF_INSERT(buf,pos,bytes,n) do {\
  if (pos >= (buf)->used) {\
    BBUF_WRITE(buf,pos,bytes,n);\
  }\
  else {\
    BBUF_MOVE_RIGHT((buf),(pos),(pos) + (n),((buf)->used - (pos)));\
    xmemcpy((buf)->p + (pos), (bytes), (n));\
  }\
} while (0)

#define BBUF_GET_BYTE(buf, pos) (buf)->p[(pos)]


#define ANCHOR_BEGIN_BUF        (1<<0)
#define ANCHOR_BEGIN_LINE       (1<<1)
#define ANCHOR_BEGIN_POSITION   (1<<2)
#define ANCHOR_END_BUF          (1<<3)
#define ANCHOR_SEMI_END_BUF     (1<<4)
#define ANCHOR_END_LINE         (1<<5)

#define ANCHOR_WORD_BOUND       (1<<6)
#define ANCHOR_NOT_WORD_BOUND   (1<<7)
#define ANCHOR_WORD_BEGIN       (1<<8)
#define ANCHOR_WORD_END         (1<<9)
#define ANCHOR_PREC_READ        (1<<10)
#define ANCHOR_PREC_READ_NOT    (1<<11)
#define ANCHOR_LOOK_BEHIND      (1<<12)
#define ANCHOR_LOOK_BEHIND_NOT  (1<<13)

#define ANCHOR_ANYCHAR_STAR     (1<<14)   /* ".*" optimize info */
#define ANCHOR_ANYCHAR_STAR_PL  (1<<15)   /* ".*" optimize info (posix-line) */

/* operation code */
enum OpCode {
  OP_FINISH = 0,        /* matching process terminator (no more alternative) */
  OP_END    = 1,        /* pattern code terminator (success end) */

  OP_EXACT1 = 2,        /* single byte, N = 1 */
  OP_EXACT2,            /* single byte, N = 2 */
  OP_EXACT3,            /* single byte, N = 3 */
  OP_EXACT4,            /* single byte, N = 4 */
  OP_EXACT5,            /* single byte, N = 5 */
  OP_EXACTN,            /* single byte */
  OP_EXACTMB2N1,        /* mb-length = 2 N = 1 */
  OP_EXACTMB2N2,        /* mb-length = 2 N = 2 */
  OP_EXACTMB2N3,        /* mb-length = 2 N = 3 */
  OP_EXACTMB2N,         /* mb-length = 2 */
  OP_EXACTMB3N,         /* mb-length = 3 */
  OP_EXACTMBN,          /* other length */

  OP_EXACT1_IC,         /* single byte, N = 1, ignore case */
  OP_EXACTN_IC,         /* single byte,        ignore case */

  OP_CCLASS,
  OP_CCLASS_MB,
  OP_CCLASS_MIX,
  OP_CCLASS_NOT,
  OP_CCLASS_MB_NOT,
  OP_CCLASS_MIX_NOT,

  OP_ANYCHAR,                 /* "."  */
  OP_ANYCHAR_ML,              /* "."  multi-line */
  OP_ANYCHAR_STAR,            /* ".*" */
  OP_ANYCHAR_ML_STAR,         /* ".*" multi-line */
  OP_ANYCHAR_STAR_PEEK_NEXT,
  OP_ANYCHAR_ML_STAR_PEEK_NEXT,

  OP_WORD,
  OP_NOT_WORD,
  OP_WORD_SB,
  OP_WORD_MB,
  OP_WORD_BOUND,
  OP_NOT_WORD_BOUND,
  OP_WORD_BEGIN,
  OP_WORD_END,

  OP_BEGIN_BUF,
  OP_END_BUF,
  OP_BEGIN_LINE,
  OP_END_LINE,
  OP_SEMI_END_BUF,
  OP_BEGIN_POSITION,

  OP_BACKREF1,
  OP_BACKREF2,
  OP_BACKREF3,
  OP_BACKREFN,
  OP_BACKREFN_IC,
  OP_BACKREF_MULTI,
  OP_BACKREF_MULTI_IC,

  OP_MEMORY_START,
  OP_MEMORY_START_PUSH,   /* push back-tracker to stack */
  OP_MEMORY_END_PUSH,     /* push back-tracker to stack */
  OP_MEMORY_END_PUSH_REC, /* push back-tracker to stack */
  OP_MEMORY_END,
  OP_MEMORY_END_REC,      /* push marker to stack */

  OP_SET_OPTION_PUSH,    /* set option and push recover option */
  OP_SET_OPTION,         /* set option */

  OP_FAIL,               /* pop stack and move */
  OP_JUMP,
  OP_PUSH,
  OP_POP,
  OP_PUSH_OR_JUMP_EXACT1,  /* if match exact then push, else jump. */
  OP_PUSH_IF_PEEK_NEXT,    /* if match exact then push, else none. */
  OP_REPEAT,               /* {n,m} */
  OP_REPEAT_NG,            /* {n,m}? (non greedy) */
  OP_REPEAT_INC,
  OP_REPEAT_INC_NG,        /* non greedy */
  OP_NULL_CHECK_START,     /* null loop checker start */
  OP_NULL_CHECK_END,       /* null loop checker end   */
  OP_NULL_CHECK_END_MEMST, /* null loop checker end (with capture status) */
  OP_NULL_CHECK_END_MEMST_PUSH, /* with capture status and push check-end */

  OP_PUSH_POS,             /* (?=...)  start */
  OP_POP_POS,              /* (?=...)  end   */
  OP_PUSH_POS_NOT,         /* (?!...)  start */
  OP_FAIL_POS,             /* (?!...)  end   */
  OP_PUSH_STOP_BT,         /* (?>...)  start */
  OP_POP_STOP_BT,          /* (?>...)  end   */
  OP_LOOK_BEHIND,          /* (?<=...) start (no needs end opcode) */
  OP_PUSH_LOOK_BEHIND_NOT, /* (?<!...) start */
  OP_FAIL_LOOK_BEHIND_NOT, /* (?<!...) end   */

  OP_CALL,                 /* \g<name> */
  OP_RETURN
};

/* arguments type */
#define ARG_SPECIAL     -1
#define ARG_NON          0
#define ARG_RELADDR      1
#define ARG_ABSADDR      2
#define ARG_LENGTH       3
#define ARG_MEMNUM       4
#define ARG_OPTION       5

typedef short int   RelAddrType;
typedef short int   AbsAddrType;
typedef short int   LengthType;
typedef short int   MemNumType;
typedef int         RepeatNumType;

#define SIZE_OPCODE        1
#define SIZE_RELADDR       sizeof(RelAddrType)
#define SIZE_ABSADDR       sizeof(AbsAddrType)
#define SIZE_LENGTH        sizeof(LengthType)
#define SIZE_MEMNUM        sizeof(MemNumType)
#define SIZE_REPEATNUM     sizeof(RepeatNumType)
#define SIZE_OPTION        sizeof(OnigOptionType)
#define SIZE_CODE_POINT    sizeof(OnigCodePoint)

#ifdef PLATFORM_UNALIGNED_WORD_ACCESS
#define GET_RELADDR_INC(addr,p) do{\
  addr = *((RelAddrType* )(p));\
  (p) += SIZE_RELADDR;\
} while(0)

#define GET_ABSADDR_INC(addr,p) do{\
  addr = *((AbsAddrType* )(p));\
  (p) += SIZE_ABSADDR;\
} while(0)

#define GET_LENGTH_INC(len,p) do{\
  len = *((LengthType* )(p));\
  (p) += SIZE_LENGTH;\
} while(0)

#define GET_MEMNUM_INC(num,p) do{\
  num = *((MemNumType* )(p));\
  (p) += SIZE_MEMNUM;\
} while(0)

#define GET_REPEATNUM_INC(num,p) do{\
  num = *((RepeatNumType* )(p));\
  (p) += SIZE_REPEATNUM;\
} while(0)

#define GET_OPTION_INC(option,p) do{\
  option = *((OnigOptionType* )(p));\
  (p) += SIZE_OPTION;\
} while(0)
#else

#define GET_RELADDR_INC(addr,p)      GET_SHORT_INC(addr,p)
#define GET_ABSADDR_INC(addr,p)      GET_SHORT_INC(addr,p)
#define GET_LENGTH_INC(len,p)        GET_SHORT_INC(len,p)
#define GET_MEMNUM_INC(num,p)        GET_SHORT_INC(num,p)
#define GET_REPEATNUM_INC(num,p)     GET_INT_INC(num,p)
#define GET_OPTION_INC(option,p)     GET_UINT_INC(option,p)

#define SERIALIZE_RELADDR(addr,p)    SERIALIZE_SHORT(addr,p)
#define SERIALIZE_ABSADDR(addr,p)    SERIALIZE_SHORT(addr,p)
#define SERIALIZE_LENGTH(len,p)      SERIALIZE_SHORT(len,p)
#define SERIALIZE_MEMNUM(num,p)      SERIALIZE_SHORT(num,p)
#define SERIALIZE_REPEATNUM(num,p)   SERIALIZE_INT(num,p)
#define SERIALIZE_OPTION(option,p)   SERIALIZE_UINT(option,p)

#define SERIALIZE_BUFSIZE            SIZEOF_INT

#endif  /* PLATFORM_UNALIGNED_WORD_ACCESS */

/* code point's address must be aligned address. */
#define GET_CODE_POINT(code,p)   code = *((OnigCodePoint* )(p))
#define GET_BYTE_INC(byte,p) do{\
  byte = *(p);\
  (p)++;\
} while(0)


/* op-code + arg size */
#define SIZE_OP_ANYCHAR_STAR            SIZE_OPCODE
#define SIZE_OP_ANYCHAR_STAR_PEEK_NEXT (SIZE_OPCODE + 1)
#define SIZE_OP_JUMP                   (SIZE_OPCODE + SIZE_RELADDR)
#define SIZE_OP_PUSH                   (SIZE_OPCODE + SIZE_RELADDR)
#define SIZE_OP_POP                     SIZE_OPCODE
#define SIZE_OP_PUSH_OR_JUMP_EXACT1    (SIZE_OPCODE + SIZE_RELADDR + 1)
#define SIZE_OP_PUSH_IF_PEEK_NEXT      (SIZE_OPCODE + SIZE_RELADDR + 1)
#define SIZE_OP_REPEAT_INC             (SIZE_OPCODE + SIZE_MEMNUM)
#define SIZE_OP_REPEAT_INC_NG          (SIZE_OPCODE + SIZE_MEMNUM)
#define SIZE_OP_PUSH_POS                SIZE_OPCODE
#define SIZE_OP_PUSH_POS_NOT           (SIZE_OPCODE + SIZE_RELADDR)
#define SIZE_OP_POP_POS                 SIZE_OPCODE
#define SIZE_OP_FAIL_POS                SIZE_OPCODE
#define SIZE_OP_SET_OPTION             (SIZE_OPCODE + SIZE_OPTION)
#define SIZE_OP_SET_OPTION_PUSH        (SIZE_OPCODE + SIZE_OPTION)
#define SIZE_OP_FAIL                    SIZE_OPCODE
#define SIZE_OP_MEMORY_START           (SIZE_OPCODE + SIZE_MEMNUM)
#define SIZE_OP_MEMORY_START_PUSH      (SIZE_OPCODE + SIZE_MEMNUM)
#define SIZE_OP_MEMORY_END_PUSH        (SIZE_OPCODE + SIZE_MEMNUM)
#define SIZE_OP_MEMORY_END_PUSH_REC    (SIZE_OPCODE + SIZE_MEMNUM)
#define SIZE_OP_MEMORY_END             (SIZE_OPCODE + SIZE_MEMNUM)
#define SIZE_OP_MEMORY_END_REC         (SIZE_OPCODE + SIZE_MEMNUM)
#define SIZE_OP_PUSH_STOP_BT            SIZE_OPCODE
#define SIZE_OP_POP_STOP_BT             SIZE_OPCODE
#define SIZE_OP_NULL_CHECK_START       (SIZE_OPCODE + SIZE_MEMNUM)
#define SIZE_OP_NULL_CHECK_END         (SIZE_OPCODE + SIZE_MEMNUM)
#define SIZE_OP_LOOK_BEHIND            (SIZE_OPCODE + SIZE_LENGTH)
#define SIZE_OP_PUSH_LOOK_BEHIND_NOT   (SIZE_OPCODE + SIZE_RELADDR + SIZE_LENGTH)
#define SIZE_OP_FAIL_LOOK_BEHIND_NOT    SIZE_OPCODE
#define SIZE_OP_CALL                   (SIZE_OPCODE + SIZE_ABSADDR)
#define SIZE_OP_RETURN                  SIZE_OPCODE


typedef struct {
  UChar esc;
  UChar anychar;
  UChar anytime;
  UChar zero_or_one_time;
  UChar one_or_more_time;
  UChar anychar_anytime;
} OnigMetaCharTableType;

extern OnigMetaCharTableType OnigMetaCharTable;

#define MC_ESC               OnigMetaCharTable.esc
#define MC_ANYCHAR           OnigMetaCharTable.anychar
#define MC_ANYTIME           OnigMetaCharTable.anytime
#define MC_ZERO_OR_ONE_TIME  OnigMetaCharTable.zero_or_one_time
#define MC_ONE_OR_MORE_TIME  OnigMetaCharTable.one_or_more_time
#define MC_ANYCHAR_ANYTIME   OnigMetaCharTable.anychar_anytime


#ifdef ONIG_DEBUG

typedef struct {
  short int opcode;
  char*     name;
  short int arg_type;
} OnigOpInfoType;

extern OnigOpInfoType OnigOpInfo[];

extern void onig_print_compiled_byte_code P_((FILE* f, UChar* bp, UChar** nextp));

#ifdef ONIG_DEBUG_STATISTICS
extern void onig_statistics_init P_((void));
extern void onig_print_statistics P_((FILE* f));
#endif
#endif

extern char* onig_error_code_to_format P_((int code));
extern void  onig_snprintf_with_pattern PV_((char buf[], int bufsize, OnigEncoding enc, char* pat, char* pat_end, char *fmt, ...));
extern UChar* onig_strdup P_((UChar* s, UChar* end));
extern int  onig_bbuf_init P_((BBuf* buf, int size));
extern int  onig_alloc_init P_((regex_t** reg, OnigOptionType option, OnigEncoding enc, OnigSyntaxType* syntax));
extern int  onig_compile P_((regex_t* reg, UChar* pattern, UChar* pattern_end, OnigErrorInfo* einfo));
extern void onig_chain_reduce P_((regex_t* reg));
extern int  onig_is_in_code_range P_((UChar* p, OnigCodePoint code));

#endif /* REGINT_H */
