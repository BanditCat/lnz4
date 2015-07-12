////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"


u32 mallocNode( LNZprogram* p ){
  if( !p->heap[ p->frees ].data ){
    LNZnode* nh = LNZcalloc( sizeof( LNZnode ) * p->heapsize * 2 );
    memcpy( nh, p->heap, sizeof( LNZnode ) * p->heapsize );
    LNZfree( p->heap );
    p->heap = nh;
    for( u64 i = p->heapsize; i < p->heapsize * 2; ++i ){
      p->heap[ i ].type = 0;
      p->heap[ i ].data = i + 1;
      if( i + 1 == p->heapsize * 2 )
	p->heap[ i ].data = 0;
    }
    p->heap[ p->frees ].data = p->heapsize;
    p->heapsize *= 2;
  }
  u32 ans = p->frees;
  p->frees = p->heap[ p->frees ].data;
  return ans;
}
void freeNode( LNZprogram* p, u32 w ){
  p->heap[ w ].type = 0;
  p->heap[ w ].data = p->frees;
  p->frees = w;
}

LNZprogram* newProgram( void ){
  LNZprogram* ans = LNZmalloc( sizeof( LNZprogram ) );
  ans->heapsize = LNZ_INITIAL_HEAP_SIZE;
  ans->heap = LNZcalloc( sizeof( LNZnode ) * ans->heapsize ); 
  ans->global = 0;
  // Mark all addresses free.
  for( u32 i = 0; i < ans->heapsize; ++i ){
    ans->heap[ i ].type = 0;
    ans->heap[ i ].data = i + 1;
    if( i + 1 == ans->heapsize )
      ans->heap[ i ].data = 0;
  }
  ans->frees = 0;

  ans->names = newNameTable();
  ans->pointers = newNameTable();
    
  return ans;
}
void deleteProgram( LNZprogram* p ){
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


void printProgram( const LNZprogram* p ){
  for( u64 i = 1; i <= p->names->size; ++i ){
    u64 len;
    const u8* name = getName( p->names, i, &len );
    for( u64 j = 0; j < len; ++j )
      putchar( (int)name[ j ] );
    printf( " = " );
    u32 expr = *( (const u32*)( getName( p->pointers, i, &len  ) ) ); 
    printExpression( p, expr, 0 );
    printf( ";\n\n" );
  }
}


void printHeap( const LNZprogram* p ){
  for( u64 i = 0; i < p->heapsize; ++i ){
    if( p->heap[ i ].type ){
      printf( "%u, type %u, references %u, data %u,%u\n", (u32)i, p->heap[ i ].type,  p->heap[ i ].references
	      ,(u32)( p->heap[ i ].data & ( (u64)( (u32)-1 ) ) ), (u32)( p->heap[ i ].data >> 32 ) );
    }
  }
}


// Sets all reference counts to 0.
u32 copyExpression( LNZprogram* p, const LNZprogram* cp, u32 arg ){
  u32 nn = mallocNode( p );
  p->heap[ nn ].type = cp->heap[ arg ].type;
  if( cp->heap[ arg ].type == LNZ_LAMBDA_TYPE ){
    p->heap[ nn ].references = 0;
    addNamePointerPair( p, (const u8*)( &arg ), sizeof( u32 ), nn );
    u32 se = copyExpression( p, cp, cp->heap[ arg ].data );
    p->heap[ nn ].data = se;
    popNamePointerPair( p );
  }else if( cp->heap[ arg ].type == LNZ_APPLICATION_TYPE ){
    u32 lo = cp->heap[ arg ].data & (u64)( (u32)-1 );
    u32 hi = cp->heap[ arg ].data >> 32;
    lo = copyExpression( p, cp, lo );
    hi = copyExpression( p, cp, hi );
    p->heap[ nn ].references = 0;
    p->heap[ nn ].data = (u64)lo + ( ( (u64)hi ) << 32 );
  }else if( cp->heap[ arg ].type == LNZ_FREE_TYPE ){
    p->heap[ nn ].references = 0;
    p->heap[ nn ].data = getPointerFromName( p, (const u8*)( &( cp->heap[ arg ].data ) ), sizeof( u32 ) );
  }else if( cp->heap[ arg ].type == LNZ_INT_TYPE || 
	    cp->heap[ arg ].type == LNZ_NEGATIVE_INT_TYPE || 
	    cp->heap[ arg ].type == LNZ_STRING_TYPE ){
    s64 len = cp->heap[ cp->heap[ arg ].data >> 32 ].references;
    u32 olen = len;
    u32 ds = cp->heap[ arg ].data & (u64)( (u32)-1 );
    s64 stride;
    if( cp->heap[ arg ].type == LNZ_STRING_TYPE )
      stride = 8;
    else
      stride = 1;
    p->heap[ nn ].references = 0;
    u32 narg = mallocNode( p );
    u64 first = narg;
    u64 last = 0;
    do{
      len -= stride;
      u32 next = narg;
      if( len >= stride || ( cp->heap[ arg ].type == LNZ_STRING_TYPE && len > 0 ) )
	next = mallocNode( p );
      else{
	last = next;
	next = olen;
      }
      p->heap[ narg ].type = cp->heap[ ds ].type;
      p->heap[ narg ].data = cp->heap[ ds ].data;
      p->heap[ narg ].references = next;
      narg = next;
      ds = cp->heap[ ds ].references;
    }while( len > 0 );
    p->heap[ nn ].references = 0;
    p->heap[ nn ].data = first + ( (u64)last << 32 );
  } 
  
  return nn;
}

// Helper functions that makes the high order of data in lambda point upwards to its' parent.
void upLambda( LNZprogram* p, u32 expr ){
  if( p->heap[ expr ].type == LNZ_LAMBDA_TYPE ){
    u32 bdy = p->heap[ expr ].data;
    if( p->heap[ bdy ].type == LNZ_LAMBDA_TYPE )
      p->heap[ bdy ].data += ( ( (u64)expr ) << 32 );
    upLambda( p, bdy );
  }else if( p->heap[ expr ].type == LNZ_APPLICATION_TYPE ){
    u32 bdy = p->heap[ expr ].data;
    if( p->heap[ bdy ].type == LNZ_LAMBDA_TYPE )
      p->heap[ bdy ].data += ( ( (u64)expr ) << 32 );
    upLambda( p, bdy );
    bdy = ( p->heap[ expr ].data >> 32 );
    if( p->heap[ bdy ].type == LNZ_LAMBDA_TYPE )
      p->heap[ bdy ].data += ( ( (u64)expr ) << 32 );
    upLambda( p, bdy );
  }
}
// This assumes refs is all 0s, it updates to correct values.
void countRefs( LNZprogram* p ){
  for( u64 i = 0; i < p->heapsize; ++i ){
    if( p->heap[ i ].type ){
      if( p->heap[ i ].type == LNZ_APPLICATION_TYPE ||
	  p->heap[ i ].type == LNZ_LAMBDA_TYPE ){
	u32 lo = p->heap[ i ].data;
	u32 hi = ( p->heap[ i ].data >> 32 );
	++( p->heap[ lo ].references );
	++( p->heap[ hi ].references );
      }else if( p->heap[ i ].type == LNZ_FREE_TYPE ||
	  p->heap[ i ].type == LNZ_LAMBDA_TYPE ){
	u32 lo = p->heap[ i ].data;
	++( p->heap[ lo ].references );
      }
    }
  }
}

LNZprogram* makeComputable( const LNZprogram* p, u32 expr ){
  LNZprogram* ans = newProgram();
  u32 e = copyExpression( ans, p, expr );
  addNamePointerPair( ans, (const u8*)"e", 1, e );
  if( ans->heap[ e ].type == LNZ_LAMBDA_TYPE )
    ans->heap[ e ].data += ( ( (u64)e ) << 32 );
  upLambda( ans, e );
  countRefs( ans );
  return ans;
}


/* void betaReduce( LNZprogram* p ){ */
/*   for( i = 0; i < p->heapsize; ++i ){ */
/*     if( p->heap[ i ].type ){ */
/*       if( p->heap[ i ].type == LNZ_APPLICATION_TYPE ){ */
/* 	u32 func = p->heap[ i ].data; */
/* 	u32 bdy = ( p->heap[ i ].data >> 32 ); */
/* 	if( p->heap[ func ].type == LNZ_LAMBDA_TYPE ){ */
	  
/* 	} */
/*       } */
/*     } */
/*   } */
/* } */
