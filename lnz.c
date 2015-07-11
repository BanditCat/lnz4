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

#ifdef DEBUG
u64 LNZmallocCount( void ){
  return lnzMallocCount;
}
#endif  

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
