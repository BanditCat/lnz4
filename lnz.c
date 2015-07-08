////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"

#ifdef DEBUG
static u64 lnzMallocCount = 0;
#endif


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

u32 mallocNode( LNZprogram* p ){
  if( p->frees->size == 0 ){
    LNZnode* nh = LNZmalloc( sizeof( LNZnode ) * p->heapsize * 2 );
    memcpy( nh, p->heap, sizeof( LNZnode ) * p->heapsize );
    LNZfree( p->heap );
    p->heap = nh;
    for( u64 i = 0; i < p->heapsize; ++i )
      push( p->frees, ( p->heapsize * 2 ) - i - 1 );
    p->heapsize *= 2;
  }
  return pop( p->frees );
}
void freeNode( LNZprogram* p, u32 w ){
  push( p->frees, w );
}

LNZprogram* newProgram( void ){
  LNZprogram* ans = LNZmalloc( sizeof( LNZprogram ) );
  ans->heapsize = LNZ_INITIAL_HEAP_SIZE;
  ans->heap = LNZmalloc( sizeof( LNZnode ) * ans->heapsize ); 
  ans->frees = newStack();
  ans->global = 0;
  // Mark all addresses free.
  for( u32 i = 0; i < ans->heapsize; ++i )
    push( ans->frees, ans->heapsize - i - 1 );
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
const u8* getNameFromPointer( LNZprogram* p, u32 pointer, u64* len ){
  u64 i = getIndex( p->names, (const u8*)( &pointer ), sizeof( u32 ) );
  if( !i )
    return NULL;
  return getName( p->names, i, len );
}

#ifdef DEBUG
u64 LNZmallocCount( void ){
  return lnzMallocCount;
}
#endif  

void addStringChar( LNZprogram* p, u32 string, u8 c ){
  u32 back = p->heap[ string ].data >> 32;
  u32 len = p->heap[ back ].references;
  if( len && !( len % 8 ) ){
    u32 nn = mallocNode( p );
    p->heap[ nn ].type = LNZ_DATA_TYPE;
    p->heap[ nn ].references = len + 1; 
    p->heap[ nn ].data = (u64)c;
    p->heap[ back ].references = nn;
    p->heap[ string ].data &= (u64)( (u32)-1 );
    p->heap[ string ].data += (u64)( nn ) << 32;
  }else{
    ++p->heap[ back ].references;
    p->heap[ back ].data += ( (u64)c ) << ( 8 * ( len % 8 ) );
  }
}
