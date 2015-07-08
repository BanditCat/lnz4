////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// LNZ main header.

#ifndef LNZ_H
#define LNZ_H

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Types.
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;
typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;
typedef float f32;
typedef double f64;

// Types sanity check.
#if CHAR_BIT != 8
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if INT_MAX != 2147483647 || INT_MIN != -2147483648
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if UINT_MAX != 4294967295 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if SHRT_MAX != 32767 || SHRT_MIN != -32768 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if USHRT_MAX != 65535 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if SCHAR_MAX != 127 || SCHAR_MIN != -128
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if UCHAR_MAX != 255 
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if LLONG_MAX != 9223372036854775807 || LLONG_MIN != 9223372036854775808ull
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 
#if ULLONG_MAX != 18446744073709551615ull
#error Alien build envioronment, check vislib.h for correct typedefs.
#endif 

#define pi 3.1415926535897932384626433832795028841971693993751058209749445923078


// The actual programs are stored as 128 bit nodes. The 32 bit pointers are just
// integer offsets into the 128-bit heap. All nodes contain a 32-bit reference count.
//lambda ( 0-32bits ) body(32-bit pointer)
//application (1-32 bits) func(32-bit pointer) arg(32-bit pointer)
//free variable (2-32 bits) lambda(32-bit pointer)
//data (3-32 bits) data(64-bit data) 
//builtin function ( 1024 + function number ) arg1(32-bit pointer) arg2(32-bit pointer)

#define LNZ_LAMBDA_TYPE 0
#define LNZ_APPLICATION_TYPE 1
#define LNZ_FREE_TYPE 2
#define LNZ_STRING_TYPE 3
#define LNZ_INT_TYPE 4
#define LNZ_DATA_TYPE 5
#define LNZ_BUILTIN_START 1024
typedef struct{
  u32 type;
  u32 references;
  u64 data;
} LNZnode;

#include "nameTable.h"
#include "stack.h"

#define LNZ_INITIAL_HEAP_SIZE 1

typedef struct{
  LNZnode* heap;
  u32 heapsize;
  stack* frees;
  nameTable* names;
  nameTable* pointers;
  u64 global;
} LNZprogram;

LNZprogram* newProgram( void );
void deleteProgram( LNZprogram* p );
u32 mallocNode( LNZprogram* p );
void freeNode( LNZprogram* p, u32 node );

// Adds a character c to the end of the string at p->heap[ string ].
void addStringChar( LNZprogram* p, u32 string, u8 c );

// Add a name pointer pair to the tables. A global.
void addNamePointerPair( LNZprogram* p, const u8* name, u64 namelen, u32 pointer );
void popNamePointerPair( LNZprogram* p );
u32 getPointerFromName( LNZprogram* p, const u8* name, u64 namelen );
const u8* getNameFromPointer( LNZprogram* p, u32 pointer, u64* len );

void LNZdie( const char* );

// Memory wrappers.
void* LNZmalloc( u64 size );
void* LNZcalloc( u64 size );
void LNZfree( void* mem );

// These functions return malloc'd resources that need to be free'd.
// The size in bytes is stored in size. The second version exits on failure.
// All data has a complementary nul, so buffer size is actually size + 1.
u8* LNZLoadResource( const char* name, u64* size );
u8* LNZLoadResourceOrDie( const char* name, u64* size );

#ifdef DEBUG
u64 LNZmallocCount( void );
#endif 

#include "parser.h"

#endif //LNZ_H 

