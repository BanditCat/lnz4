////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"


u32 mallocNode( LNZprogram* p ){
  if( p->heap[ p->frees ].data == p->frees ){
    LNZnode* nh = LNZcalloc( sizeof( LNZnode ) * p->heapsize * 2 );
    memcpy( nh, p->heap, sizeof( LNZnode ) * p->heapsize );
    LNZfree( p->heap );
    p->heap = nh;
    for( u64 i = p->heapsize; i < p->heapsize * 2; ++i ){
      p->heap[ i ].data = i + 1;
      if( i + 1 == p->heapsize * 2 )
	p->heap[ i ].data = i;
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
    ans->heap[ i ].data = i + 1;
    if( i + 1 == ans->heapsize )
      ans->heap[ i ].data = i;
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
u32 getPointerFromName( const LNZprogram* p, const u8* name, u64 namelen ){
  u64 i = getIndex( p->names, name, namelen );
  if( !i )
    return 0;
  return *( (const u32*)getName( p->pointers, i, NULL ) );
}
const u8* getNameFromPointer( const LNZprogram* p, u32 pointer, u64* len ){
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


void printProgram( const LNZprogram* p, const LNZprogram* ref ){
  for( u64 i = 1; i <= p->names->size; ++i ){
    u64 len;
    const u8* name = getName( p->names, i, &len );
    for( u64 j = 0; j < len; ++j )
      putchar( (int)name[ j ] );
    printf( " = " );
    u32 expr = *( (const u32*)( getName( p->pointers, i, &len  ) ) ); 
    printExpression( p, expr, 0, ref );
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
u32 copyData( LNZprogram* p, const LNZprogram* cp, u32 arg ){
  u32 nn = mallocNode( p );
  p->heap[ nn ].type = cp->heap[ arg ].type;
  p->heap[ nn ].references = 1;
  s64 len = cp->heap[ cp->heap[ arg ].data >> 32 ].references;
  u32 olen = len;
  u32 ds = cp->heap[ arg ].data & (u64)( (u32)-1 );
  s64 stride;
  if( cp->heap[ arg ].type == LNZ_STRING_TYPE )
    stride = 8;
  else
    stride = 1;
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
  p->heap[ nn ].data = first + ( (u64)last << 32 );
  return nn;
} 

// Sets all reference counts to 1. if replace not 0, replace free variables of func 
// with repl
u32 copyExpression( LNZprogram* p, int overwrite, u32 copyto,
		    const LNZprogram* cp, u32 arg,
		    int replace, u32 func, u32 repl ){
  /* if( !overwrite && !replace && cp == p && p->names->size == 1 ){ */
  /*   ++( p->heap[ arg ].references ); */
  /*   return arg; */
  /* }else */{
    u32 nn;
    if( overwrite )
      nn = copyto;
    else
      nn = mallocNode( p );
    
    p->heap[ nn ].type = cp->heap[ arg ].type;
    if( !overwrite )
      p->heap[ nn ].references = 1;
    if( cp->heap[ arg ].type == LNZ_LAMBDA_TYPE ){
      p->heap[ nn ].references = 1;
      u32 bdy = cp->heap[ arg ].data;
      addNamePointerPair( p, (const u8*)( &arg ), sizeof( u32 ), nn );
      u32 se;
      /* if( replace && p->heap[ bdy ].type == LNZ_FREE_TYPE && */
      /* 	  p->heap[ bdy ].data == func ){ */
      /* 	++( p->heap[ repl ].references ); */
      /* 	se = repl; */
      /* }else */
      se = copyExpression( p, 0, 0, cp, bdy, replace, func, repl );
      p->heap[ nn ].data = se;
      popNamePointerPair( p );
    }else if( cp->heap[ arg ].type == LNZ_APPLICATION_TYPE ){
      u32 lo = cp->heap[ arg ].data & (u64)( (u32)-1 );
      u32 hi = cp->heap[ arg ].data >> 32;
      /* if( replace && p->heap[ lo ].type == LNZ_FREE_TYPE && */
      /* 	  p->heap[ lo ].data == func ){ */
      /* 	++( p->heap[ repl ].references ); */
      /* 	lo = repl; */
      /* }else */
	lo = copyExpression( p, 0, 0, cp, lo, replace, func, repl );
      /* if( replace && p->heap[ hi ].type == LNZ_FREE_TYPE && */
      /* 	  p->heap[ hi ].data == func ){ */
      /* 	++( p->heap[ repl ].references ); */
      /* 	hi = repl; */
      /* }else */
	hi = copyExpression( p, 0, 0, cp, hi, replace, func, repl );
      p->heap[ nn ].data = (u64)lo + ( ( (u64)hi ) << 32 );
    }else if( cp->heap[ arg ].type == LNZ_FREE_TYPE ){
      if( replace && cp->heap[ arg ].data == func ){
       	copyExpression( p, 1, nn, cp, repl, 0, 0, 0 ); 
      }else{
	u32 wind = getIndex( p->names, (const u8*)( &( cp->heap[ arg ].data ) ), sizeof( u32 ) );
	u32 ti;
	if( wind )
	  ti = *( (const u32*)p->pointers->revdict[ wind - 1 ] );
	else
	  ti = cp->heap[ arg ].data;
	p->heap[ nn ].data = ti;
      }
    }else if( cp->heap[ arg ].type == LNZ_INT_TYPE || 
	      cp->heap[ arg ].type == LNZ_NEGATIVE_INT_TYPE || 
	      cp->heap[ arg ].type == LNZ_STRING_TYPE ){
      if( overwrite ){
	u32 ans = copyData( p, cp, arg );
	p->heap[ nn ].type = p->heap[ ans ].type;
	p->heap[ nn ].references = 1;
	p->heap[ nn ].data = p->heap[ ans ].data;
	freeNode( p, ans );
      } else{
	freeNode( p, nn );
	if( p == cp ){
	  ++( p->heap[ arg ].references );
	  return arg;
	}else
	  return copyData( p, cp, arg );
      }
    } 
    return nn;
  }
}

/* // Helper functions that makes the high order of data in lambda point upwards to its' parent. */
/* void upLambda( LNZprogram* p, u32 expr ){ */
/*   if( p->heap[ expr ].type == LNZ_LAMBDA_TYPE ){ */
/*     u32 bdy = p->heap[ expr ].data; */
/*     if( p->heap[ bdy ].type == LNZ_LAMBDA_TYPE ) */
/*       p->heap[ bdy ].data += ( ( (u64)expr ) << 32 ); */
/*     upLambda( p, bdy ); */
/*   }else if( p->heap[ expr ].type == LNZ_APPLICATION_TYPE ){ */
/*     u32 bdy = p->heap[ expr ].data; */
/*     if( p->heap[ bdy ].type == LNZ_LAMBDA_TYPE ) */
/*       p->heap[ bdy ].data += ( ( (u64)expr ) << 32 ); */
/*     upLambda( p, bdy ); */
/*     bdy = ( p->heap[ expr ].data >> 32 ); */
/*     if( p->heap[ bdy ].type == LNZ_LAMBDA_TYPE ) */
/*       p->heap[ bdy ].data += ( ( (u64)expr ) << 32 ); */
/*     upLambda( p, bdy ); */
/*   } */
/* } */
// Helper functions that makes the high order of data in lambda point at itself.
void reLambda( LNZprogram* p, u32 expr ){
  if( p->heap[ expr ].type == LNZ_LAMBDA_TYPE ){
    u32 bdy = p->heap[ expr ].data;
    u64 nd = ( (u32)( p->heap[ expr ].data ) );
    nd += ( ( (u64)expr ) << 32 );
    p->heap[ expr ].data = nd;
    reLambda( p, bdy );
  }else if( p->heap[ expr ].type == LNZ_APPLICATION_TYPE ){
    u32 bdy = p->heap[ expr ].data;
    reLambda( p, bdy );
    bdy = ( p->heap[ expr ].data >> 32 );
    reLambda( p, bdy );
  }
}
// Helper functions that makes the high order of data in lambda point at linked list
// of all free varoables for that lambda. The links are stored in the high order
// word of the free varoables data and the last element points at itself.
void downLambda( LNZprogram* p, u32 expr ){
  if( p->heap[ expr ].type == LNZ_LAMBDA_TYPE ){
    u32 bdy = p->heap[ expr ].data;
    downLambda( p, bdy );
  }else if( p->heap[ expr ].type == LNZ_APPLICATION_TYPE ){
    u32 bdy = p->heap[ expr ].data;
    downLambda( p, bdy );
    bdy = ( p->heap[ expr ].data >> 32 );
    downLambda( p, bdy );
  }else if( p->heap[ expr ].type == LNZ_FREE_TYPE ){
    u64 ul = ( (u32)( p->heap[ expr ].data ) );
    u64 dl = p->heap[ ul ].data >> 32;
    if( dl == ul ){
      p->heap[ expr ].data = ul + ( ( (u64)expr ) << 32 );
      p->heap[ ul ].data = ( (u64)( (u32)p->heap[ ul ].data ) )
	+ ( ( (u64)expr ) << 32 );
    }else{
      p->heap[ expr ].data = ul + ( dl << 32 );
      p->heap[ ul ].data = ( (u64)( (u32)p->heap[ ul ].data ) )
	+ ( ( (u64)expr ) << 32 );
    }
  }
}
// This assumes refs is all 0s, it updates to correct values.
void countRefs( LNZprogram* p ){
  for( u64 i = 0; i < p->heapsize; ++i ){
    if( p->heap[ i ].type ){
      if( p->heap[ i ].type == LNZ_APPLICATION_TYPE ){
	u32 lo = p->heap[ i ].data;
	u32 hi = ( p->heap[ i ].data >> 32 );
	++( p->heap[ lo ].references );
	++( p->heap[ hi ].references );
      }else if( p->heap[ i ].type == LNZ_LAMBDA_TYPE ){
	u32 lo = p->heap[ i ].data;
	++( p->heap[ lo ].references );
      }
    }
  }
}

LNZprogram* makeComputable( const LNZprogram* p, u32 expr ){
  LNZprogram* ans = newProgram();
  u32 e = copyExpression( ans, 0, 0, p, expr, 0, 0, 0 );
  addNamePointerPair( ans, (const u8*)"e", 1, e );
  /* if( ans->heap[ e ].type == LNZ_LAMBDA_TYPE ) */
  /*   ans->heap[ e ].data += ( ( (u64)e ) << 32 ); */
  /* //reLambda( ans, e ); */
  /* downLambda( ans, e ); */
  /* countRefs( ans ); */
  /* ++( ans->heap[ e ].references ); */
  return ans;
}

// Recursively decrease reference count.
void decref( LNZprogram* p, u32 arg ){
  if( p->heap[ arg ].type ){
    if( !( --( p->heap[ arg ].references ) ) ){
      if( p->heap[ arg ].type == LNZ_APPLICATION_TYPE ){
	u32 lo = p->heap[ arg ].data;
	u32 hi = ( p->heap[ arg ].data >> 32 );
	freeNode( p, arg );
	decref( p, lo );
	decref( p, hi );
      }else if( p->heap[ arg ].type == LNZ_LAMBDA_TYPE ){
	u32 lo = p->heap[ arg ].data;
	freeNode( p, arg );
	decref( p, lo );
      }else if( p->heap[ arg ].type == LNZ_FREE_TYPE ){
	freeNode( p, arg );
      }else if( p->heap[ arg ].type == LNZ_INT_TYPE || 
		p->heap[ arg ].type == LNZ_NEGATIVE_INT_TYPE || 
		p->heap[ arg ].type == LNZ_STRING_TYPE ){
	s64 len = p->heap[ p->heap[ arg ].data >> 32 ].references;
	u32 ds = p->heap[ arg ].data & (u64)( (u32)-1 );
	s64 stride;
	if( p->heap[ arg ].type == LNZ_STRING_TYPE )
	  stride = 8;
	else
	  stride = 1;
	do{
	  len -= stride;
	  freeNode( p, ds );
	  ds = p->heap[ ds ].references;
	}while( len > 0 );
	freeNode( p, arg );
      }
    }
  }
}

// Checks whether 2 data types are equal.
int dataEqual( const LNZprogram* p1, u32 arg1,
		const LNZprogram* p2, u32 arg2 ){

  if( p1->heap[ arg1 ].type != p2->heap[ arg2 ].type )
    return 0;
  if( p1->heap[ arg1 ].data == p2->heap[ arg2 ].data )
    return 1;
  s64 len = p1->heap[ p1->heap[ arg1 ].data >> 32 ].references;
  if( len != p2->heap[ p2->heap[ arg2 ].data >> 32 ].references )
    return 0;
  
  u32 d1 = p1->heap[ arg1 ].data;
  u32 d2 = p2->heap[ arg2 ].data;
 
  s64 stride;
  if( p1->heap[ arg1 ].type == LNZ_STRING_TYPE )
    stride = 8;
  else
    stride = 1;

  do{
    u64 data1 = p1->heap[ d1 ].data;
    u64 data2 = p2->heap[ d2 ].data;
    if( p1->heap[ arg1 ].type == LNZ_STRING_TYPE && len < 8 ){
      __int128 mask = ( (__int128)1 << (__int128)( len * 8 ) );
      mask -= 1;
      data1 &= mask;
      data2 &= mask;
    }
    if( data1 != data2 )
      return 0;
    
    d1 = p1->heap[ d1 ].references;
    d2 = p2->heap[ d2 ].references;
    len -= stride;
  }while( len > 0 ); 
  return 1;
}

int nodesEqual( const LNZprogram* p1, u32 arg1,
		const LNZprogram* p2, u32 arg2 ){
  if( p1 == p2 && arg1 == arg2 )
    return 1;
  if( p1->heap[ arg1 ].type != p2->heap[ arg2 ].type )
    return 0;
  if( p1->heap[ arg1 ].type == LNZ_LAMBDA_TYPE ){
    addNamePointerPair( (LNZprogram*)p1, (const u8*)( &arg2 ), sizeof( u32 ), arg1 );
    int ans = nodesEqual( p1, p1->heap[ arg1 ].data,
			  p2, p2->heap[ arg2 ].data );
    popNamePointerPair( (LNZprogram*)p1 );
    return ans;
  }else if( p1->heap[ arg1 ].type == LNZ_APPLICATION_TYPE )
    return ( nodesEqual( p1, p1->heap[ arg1 ].data,
			 p2, p2->heap[ arg2 ].data ) &&
	     nodesEqual( p1, p1->heap[ arg1 ].data >> 32,
			 p2, p2->heap[ arg2 ].data >> 32 ) );
  else if( p1->heap[ arg1 ].type == LNZ_FREE_TYPE ){
    u32 wl = p2->heap[ arg2 ].data;
    return getPointerFromName( p1, (const u8*)( &wl ), sizeof( u32 ) ) == 
      p1->heap[ arg1 ].data;
  }else if( p1->heap[ arg1 ].type >= LNZ_DATA_START &&
	    p1->heap[ arg1 ].type <= LNZ_DATA_END ){
    return dataEqual( p1, arg1, p2, arg2 );
  }
  return 0;
}


u64 betaReduce( LNZprogram* p ){
  u64 reds = 0;
  u64 len = p->heapsize;
  for( u64 i = 0; i < len; ++i ){
    if( p->heap[ i ].type ){
      if( p->heap[ i ].type == LNZ_APPLICATION_TYPE ){
	u32 func = p->heap[ i ].data;
	u32 arg = ( p->heap[ i ].data >> 32 );
	if( p->heap[ func ].type == LNZ_LAMBDA_TYPE ){
	  ++reds;
	  u32 bdy = p->heap[ func ].data;
	  copyExpression( p, 1, i, p, bdy, 1, func, arg ); 
	  decref( p, func );
	  decref( p, arg );
	  break;
	}
      }
    }
  }
  return reds;
}

int betaReduceNormalOrder( LNZprogram* p, u32 ind ){
  if( p->heap[ ind ].type == LNZ_APPLICATION_TYPE ){
    u32 func = p->heap[ ind ].data;
    u32 arg = ( p->heap[ ind ].data >> 32 );
    if( p->heap[ func ].type == LNZ_LAMBDA_TYPE ){
      u32 bdy = p->heap[ func ].data;
      copyExpression( p, 1, ind, p, bdy, 1, func, arg ); 
      decref( p, func );
      decref( p, arg );
      return 1;
    }else{
      if( betaReduceNormalOrder( p, func ) )
	return 1;
      return betaReduceNormalOrder( p, arg );
    }
  }else if( p->heap[ ind ].type == LNZ_LAMBDA_TYPE ){
    u32 bdy = p->heap[ ind ].data;
    return betaReduceNormalOrder( p, bdy );
  }
  return 0;
}
