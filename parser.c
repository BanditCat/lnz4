////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"

int isWhitespace( u8 c ){
  if( !c || isspace( c ) )
    return 1;
  return 0;
}

int isReserved( u8 c ){
  if( strchr( "[].\\/", (int)c ) )
    return 1;
  return 0;
}

int isName( u8 c ){
  if( !isReserved( c ) && !isWhitespace( c ) )
    return 1;
  return 0;
}

// This returns the parsed expression as a node, or NULL. If NULL is returned, error is made to point at an error message. All named indices less than or equal to global 
// are considered global; all other indices should be bound lambdas.
u32 parseExpression( LNZprogram* p, const u8* string, u64 length, const char** error, u64 global ){
  *error = NULL;
  const u8* s = string;
  u64 l = length;

  // Eat up whitespace in front and back.
  while( l && isWhitespace( *s ) ){
    --l;
    ++s;
  }
  while( l && isWhitespace( s[ l - 1 ] ) )
    --l;

  if( !l ){
    *error = "Syntax error: null expression.";
    return 0;
  }
  
  // Handle lambdas
  if( *s == '\\' ){
    ++s;
    --l;
    while( l && isWhitespace( *s ) ){
      --l;
      ++s;
    }
    if( !l || !isName( *s ) ){
      *error = "Syntax error: malformed lambda, expected an identifier.";
      return 0;
    }
 
    // Eat identifiers, add them to the nametables, and build up an empty lambda at
    // top.
    u64 namelen = 0;
    u64 lambdaCount = 0;
    u32 cur, top;
    do{
      while( namelen < l && isName( s[ namelen ] ) )
	++namelen;
      
      u32 newnode = mallocNode( p );
      p->heap[ newnode ].type = LNZ_LAMBDA_TYPE;
      p->heap[ newnode ].references = 1;
      
      if( lambdaCount )
	p->heap[ cur ].data = newnode;
      else
	top = newnode;
      
      ++lambdaCount;
      cur = newnode;

      addNamePointerPair( p, s, namelen, newnode );

      while( namelen < l && isWhitespace( s[ namelen ] ) )
	++namelen;

      if( namelen == l || ( s[ namelen ] != '.' && !isName( s[ namelen ] ) ) ){
	*error = "Syntax error: malformed lambda, expected '.' or an identifier.";
	return 0;
      }
      
      s += namelen;
      l -= namelen;
	     
    }while( *s != '.' );
    // Parse the body.
    p->heap[ cur ].data = parseExpression( p, s + 1, l - 1, error, global );
    // Pop scope.
    while( lambdaCount ){
      --lambdaCount;
      popNamePointerPair( p );
    }

    return top;
  }


  // Handle applications and free variables.

  u64 applicationCount = 0;
  u32 top, cur, oc;
  // Read one expression at a time; must be parenthetical or a name.
  do{
    u32 arg;
    u64 namelen = 0;
    // Handle parenthetical subexpressions.
    if( *s == '[' ){
      namelen = 1;
      u64 f = 1;
      
      while( namelen < l ){
	if( s[ namelen ] == '[' )
	  ++f;
	if( s[ namelen ] == ']' )
	  --f;
	++namelen;
	if( !f )
	  break;
      }
      if( f || namelen < 2 ){
	*error = "Syntax error: unbalanced parentheses.";
	return 0;
      }
    
      arg = parseExpression( p, s + 1, namelen - 2, error, global );
      if( error != NULL )
	return 0;

    }else{
      // Handle identifiers.
      if( !l || !isName( *s ) ){
	*error = "Syntax error: malformed expression.";
	return 0;
      }
  
      
      while( namelen < l && isName( s[ namelen ] ) )
	++namelen;    

      u64 nind = getIndex( p->names, s, namelen );
      if( !nind ){
	*error = "Syntax error: unknown identifier.";
	return 0;
      }
      u32 pntr = getPointerFromName( p, s, namelen );
      
      // if global or not a lambda, treat as an identifier, otherwise a free variable.
      if( nind <= global || p->heap[ pntr ].type != LNZ_LAMBDA_TYPE )
	arg = *( ( u32* )( getName( p->pointers, nind, NULL ) ) );
      else{
	arg = mallocNode( p );
	p->heap[ arg ].type = LNZ_FREE_TYPE;
	p->heap[ arg ].references = 1;
	p->heap[ arg ].data = *( ( u32* )( getName( p->pointers, nind, NULL ) ) );
      }
    }

    u32 newnode = mallocNode( p );
    p->heap[ newnode ].type = LNZ_APPLICATION_TYPE;
    p->heap[ newnode ].references = 1;
    p->heap[ newnode ].data = arg;
     
      
    if( applicationCount ) 
      p->heap[ cur ].data += ( ( (u64)newnode ) << 32 );
    else
      top = newnode;
      
    
    ++applicationCount;
    oc = cur;
    cur = newnode;
  
    // Eat up trailing whitespace.
    while( namelen < l && isWhitespace( s[ namelen ] ) )
      ++namelen;

      
    s += namelen;
    l -= namelen;
	     
  }while( l );
  
  if( applicationCount == 1 ){
    u32 ans = p->heap[ top ].data;
    freeNode( p, top );
    return ans;
  }else{
    p->heap[ oc ].data &= ( (u32)-1 );
    p->heap[ oc ].data += p->heap[ cur ].data << 32;
    freeNode( p, cur );
    return top;
  }  

}



void printExpression( const LNZprogram* p, u32 expression ){
  printf( "node #%u\n\n", expression );


  for( u64 i = 0; i < p->heapsize; ++i ){
    printf( "node %u, type %u, references %u, data low %u, data high %u\n",
	    (u32)i,
	    p->heap[ i ].type,
	    p->heap[ i ].references,
	    (u32)( p->heap[ i ].data & (u32)( -1 ) ),
	    (u32)( p->heap[ i ].data >> 32 )
	    );
    (void)expression;
  }
}
