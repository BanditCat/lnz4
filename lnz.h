////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// LNZ main header.

#ifndef LNZ_H
#define LNZ_H

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
typedef struct{
  u32 type;
  u32 references;
  u64 data;
} LNZnode;

#include "nameTable.h"
#include "stack.h"

typedef struct{
  LNZnode* heap;
  u32 heapSize;
  u32 nodeCount;
  nameTable names;

} LNZprogram;



void LNZdie( const char* );

// Memory wrappers.
void* LNZmalloc( u64 size );
void* LNZcalloc( u64 size );
void LNZfree( void* mem );


#ifdef DEBUG
u64 LNZmallocCount( void );
#endif 

#include "parser.h"

#endif //LNZ_H 

