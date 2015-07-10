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


// Could be asymptotically faster. But KISS.
void multiplyNumberByInt( LNZprogram* p, u32 arg, u64 x ){
  unsigned __int128 carry = 0;
  unsigned __int128 high, low;
  u32 len = p->heap[ p->heap[ arg ].data >> 32 ].references;
  u32 dp = p->heap[ arg ].data & ( (u64)( (u32)-1 ) );
  while( len ){
    unsigned __int128 lng = (unsigned __int128)p->heap[ dp ].data * (unsigned __int128)x;
    lng += carry;
    high = lng >> 64;
    low = lng & (unsigned __int128)( (u64)-1 );
    carry = high;
    p->heap[ dp ].data = (u64)low;
    --len;
    if( !len && carry ){
      u32 olen = p->heap[ dp ].references;
      u32 nn = mallocNode( p );
      p->heap[ dp ].references = nn;
      p->heap[ nn ].type = LNZ_DATA_TYPE;
      p->heap[ nn ].references = olen + 1;
      p->heap[ nn ].data = carry;
      p->heap[ arg ].data &= (u64)( (u32)-1 );
      p->heap[ arg ].data += (u64)( nn ) << 32;
      dp = nn;
    } else
      dp = p->heap[ dp ].references;
  }
}

void addIntToNumber( LNZprogram* p, u32 arg, u64 x ){
  unsigned __int128 acc = 0;
  u32 olen = p->heap[ p->heap[ arg ].data >> 32 ].references;
  u32 len = olen;
  u32 dp = p->heap[ arg ].data & ( (u64)( (u32)-1 ) );
  while( len ){
    acc += (unsigned __int128)p->heap[ dp ].data;
    if( len == olen )
      acc += (unsigned __int128)x;
    p->heap[ dp ].data = acc & (unsigned __int128)( (u64)-1 );
    acc >>= 64;
    --len;
    if( !len && acc ){
      u32 olen = p->heap[ dp ].references;
      u32 nn = mallocNode( p );
      p->heap[ dp ].references = nn;
      p->heap[ nn ].type = LNZ_DATA_TYPE;
      p->heap[ nn ].references = olen + 1;
      p->heap[ nn ].data = acc;
      p->heap[ arg ].data &= (u64)( (u32)-1 );
      p->heap[ arg ].data += (u64)( nn ) << 32;
      dp = nn;
    } else
      dp = p->heap[ dp ].references;
  }
}


void addStringToString( char** string, u64* len, const char* add, u64 addlen ){
  u64 sz = 1;
  char* ans = LNZmalloc( sz );
  u64 l = *len > addlen ? *len : addlen;
  u64 i = 0;
  u64 carry = 0;
  while( carry || i < l ){
    if( i >= sz ){
      sz *= 2;
      char* na = ans;
      ans = LNZmalloc( sz );
      memcpy( ans + sz / 2, na, sz / 2 );
      LNZfree( na );
    }
    u64 a1, a2;
    if( i < *len )
      a1 = (*string)[ *len - 1 - i ] - '0';
    else
      a1 = 0;
    if( i < addlen )
      a2 = add[ addlen - 1 - i ] - '0';
    else
      a2 = 0;
    u64 ad = a1 + a2 + carry;
    ans[ sz - 1 - i ] = ad % 10 + '0';
    if( ad >= 10 )
      carry = 1;
    else 
      carry = 0;

    ++i;
  }

  LNZfree( *string );
  *string = LNZmalloc( i );
  memcpy( *string, ans + ( sz - i ), i ); 
  LNZfree( ans );
  *len = i;
}

char* numberToString( const LNZprogram* p, u32 num, u64* len ){
  char* acc = LNZmalloc( 1 );
  *acc = '0';
  u64 acclen = 1;
  char* add = LNZmalloc( 1 );
  *add = '1';
  u64 addlen = 1;

  u32 nlen = p->heap[ p->heap[ num ].data >> 32 ].references;
  u32 dp = p->heap[ num ].data & (u64)( (u32)-1 );
  u64 sel = 1;
  while( nlen ){
    if( sel & p->heap[ dp ].data )
      addStringToString( &acc, &acclen, add, addlen );
    addStringToString( &add, &addlen, add, addlen );
    sel <<= 1;
    if( !sel ){
      sel = 1;
      dp = p->heap[ dp ].references;
      --nlen;
    }
  }
  
  LNZfree( add );
  *len = acclen;
  return acc;
}
