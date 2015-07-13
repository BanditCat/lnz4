////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// Program structure and functions

#ifndef LNZ_PROGRAM_H
#define LNZ_PROGRAM_H

// The actual programs are stored as 128 bit nodes. The 32 bit pointers are just
// integer offsets into the 128-bit heap. All nodes contain a 32-bit reference count.
//lambda ( 0-32bits ) body(32-bit pointer)
//application (1-32 bits) func(32-bit pointer) arg(32-bit pointer)
//free variable (2-32 bits) lambda(32-bit pointer)
//data (3-32 bits) data(64-bit data) 
//builtin function ( 1024 + function number ) arg1(32-bit pointer) arg2(32-bit pointer)
// 0 is reserved to indicate free space. Free space is a linked list.
#define LNZ_LAMBDA_TYPE 1
#define LNZ_APPLICATION_TYPE 2
#define LNZ_FREE_TYPE 3


// Data types.
#define LNZ_DATA_START 4
#define LNZ_STRING_TYPE 4
#define LNZ_INT_TYPE 5
#define LNZ_NEGATIVE_INT_TYPE 6
#define LNZ_DATA_END 6
#define LNZ_DATA_TYPE 7


#define LNZ_BUILTIN_START 1024

typedef struct{
  u32 type;
  u32 references;
  u64 data;
} LNZnode;

#include "nameTable.h"
#include "stack.h"

#define LNZ_INITIAL_HEAP_SIZE 1;

typedef struct{
  LNZnode* heap;
  u32 heapsize;
  u32 frees;
  nameTable* names;
  nameTable* pointers;
  u64 global;
} LNZprogram;


LNZprogram* newProgram( void );
void deleteProgram( LNZprogram* p );
u32 mallocNode( LNZprogram* p );
void freeNode( LNZprogram* p, u32 node );

// Add a name pointer pair to the tables. A global.
void addNamePointerPair( LNZprogram* p, const u8* name, u64 namelen, u32 pointer );
void popNamePointerPair( LNZprogram* p );

void multiplyNumberByInt( LNZprogram* p, u32 arg, u64 x );
void addIntToNumber( LNZprogram* p, u32 arg, u64 x );

u32 getPointerFromName( const LNZprogram* p, const u8* name, u64 namelen );
const u8* getNameFromPointer( const LNZprogram* p, u32 pointer, u64* len );

// Adds a character c to the end of the string at p->heap[ string ].
void addStringChar( LNZprogram* p, u32 string, u8 c );

// Converts an integer into a string. ignores sign.
char* numberToString( const LNZprogram* p, u32 num, u64* len );

// This creates a c new LNZprogram just containing expr with proper reference counts and ready for beta-evalution.
// There is only one name, "e" and it points to the top of the expression.
LNZprogram* makeComputable( const LNZprogram* p, u32 expr );

// If ref is not null, then it used for names hen printing.
void printProgram( const LNZprogram* p, const LNZprogram* ref );

void printHeap( const LNZprogram* p );

// Returns 0 unless arg1 and arg2 point to identical expressions. 
int nodesEqual( const LNZprogram* p1, u32 arg1,
		const LNZprogram* p2, u32 arg2 );


// Returns number of beta reductions performed.
u64 betaReduce( LNZprogram* p );

#endif //LNZ_PROGRAM_H
