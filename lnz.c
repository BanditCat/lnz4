////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include "lnz.h"

#ifdef DEBUG
static u64 lnzMallocCount = 0;
#endif

void LNZdie( const char* msg ){
  MessageBoxA( NULL, (char*)msg, "Exiting LNZ...", 0 );
  exit( 0 );
}

void* LNZmalloc( u64 size ){
  void* ans = malloc( size );
  if( ans == NULL )
    LNZdie( "Out of memory!" );
#ifdef DEBUG
  ++lnzMallocCount;
#endif
  return ans;
}
void* LNZcalloc( u64 size ){
  void* ans = calloc( 1, size );
  if( ans == NULL )
    LNZdie( "Out of memory!" );
#ifdef DEBUG
  ++lnzMallocCount;
#endif
  return ans;
}

void LNZfree( void* mem ){
  free( mem );
#ifdef DEBUG
  --lnzMallocCount;
#endif
}

LNZprogram* newProgram( void ){
  LNZprogram* ans = LNZmalloc( sizeof( LNZprogram ) );
  ans->heapsize = LNZ_INITIAL_HEAP_SIZE;
  ans->heap = LNZmalloc( sizeof( LNZnode ) * ans->heapsize ); 
  ans->frees = newStack();
  // Mark all addresses free.
  for( u32 i = 0; i < ans->heapsize; ++i )
    push( ans->frees, i );
  ans->names = newNameTable();
  ans->pointers = newNameTable();
    
  return ans;
}
void deleteProgram( LNZprogram* p ){
  deleteStack( p->frees );
  deleteNameTable( p->names );
  deleteNameTable( p->pointers );
  LNZfree( p->heap );
  LNZfree( p );
}
void addNamePointerPair( LNZprogram* p, const u8* name, u64 namelen, u32 pointer ){
  addNameToTable( p->names, name, namelen );
  addNameToTable( p->pointers, (const u8*)( &pointer ), sizeof( u32 ) );
}
void popNamePointerPair( LNZprogram* p ){
  popNameTable( p->names );
  popNameTable( p->pointers );
}
u32 getPointerFromName( LNZprogram* p, const u8* name, u64 namelen ){
  u64 i = getIndex( p->names, name, namelen );
  if( !i )
    return 0;
  return *( (const u32*)getName( p->pointers, i, NULL ) );
}
const u8* getNameFromPointer( LNZprogram* p, const u32 pointer, u64* len ){
  u64 i = getIndex( p->names, (const u8*)( &pointer ), sizeof( u32 ) );
  if( !i )
    return 0;
  return getName( p->names, i, len );
}

#ifdef DEBUG
u64 LNZmallocCount( void ){
  return lnzMallocCount;
}
#endif  

