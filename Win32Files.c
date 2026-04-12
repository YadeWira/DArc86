/* External (non-inline) wrappers for helpers used by Win32Files.hs.
 *
 * Two groups of wrappers are needed because the Haskell FFI imports expect
 * real external symbols:
 *
 *   1. __w_* helpers declared as INLINE in Win32Files.h (`static inline`
 *      in HsBase.h → no external symbol).
 *   2. __hscore_seek_* accessors from HsBase.h, same reason.
 *
 * msvcrt (GHC 8.6 Windows i386) exports _fstati64 / _wstati64 /
 * _wfindfirsti64 / _wfindnexti64 directly, so no CRT aliasing wrappers
 * are needed.
 */
#include <HsBase.h>
#include <io.h>
#include <wchar.h>
#include <sys/stat.h>
#include <stdio.h>
#include <setjmp.h>

/* Group 1: __w_* helpers */
HsInt    __w_find_sizeof       ( void )                          { return sizeof(struct _wfinddatai64_t); }
unsigned __w_find_attrib       ( struct _wfinddatai64_t* st )    { return st->attrib;      }
time_t   __w_find_time_create  ( struct _wfinddatai64_t* st )    { return st->time_create; }
time_t   __w_find_time_access  ( struct _wfinddatai64_t* st )    { return st->time_access; }
time_t   __w_find_time_write   ( struct _wfinddatai64_t* st )    { return st->time_write;  }
__int64  __w_find_size         ( struct _wfinddatai64_t* st )    { return st->size;        }
wchar_t* __w_find_name         ( struct _wfinddatai64_t* st )    { return st->name;        }

HsInt          __w_stat_sizeof ( void )                          { return sizeof(struct _stati64); }
unsigned short __w_stat_mode   ( struct _stati64* st )           { return st->st_mode;  }
time_t         __w_stat_ctime  ( struct _stati64* st )           { return st->st_ctime; }
time_t         __w_stat_atime  ( struct _stati64* st )           { return st->st_atime; }
time_t         __w_stat_mtime  ( struct _stati64* st )           { return st->st_mtime; }
__int64        __w_stat_size   ( struct _stati64* st )           { return st->st_size;  }

/* Group 2: SEEK_* accessors (static inline in HsBase.h means no external symbol) */
int __hscore_seek_cur(void) { return SEEK_CUR; }
int __hscore_seek_set(void) { return SEEK_SET; }
int __hscore_seek_end(void) { return SEEK_END; }
