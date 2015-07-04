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
    
  return ans;
}
void deleteProgram( LNZprogram* p ){
  deleteStack( p->frees );
  
  LNZfree( p->heap );
  LNZfree( p );
}


#ifdef DEBUG
u64 LNZmallocCount( void ){
  return lnzMallocCount;
}
#endif  

