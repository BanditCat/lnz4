////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"
#include "pep.h"

int isWhitespace( u8 c ){
  if( !c || isspace( c ) )
    return 1;
  return 0;
}

int isReserved( u8 c ){
  if( strchr( "[].\\/=;", (int)c ) )
    return 1;
  return 0;
}

int isName( u8 c ){
  if( !isReserved( c ) && !isWhitespace( c ) )
    return 1;
  return 0;
}

int isNumber( u8 c ){
  if( c >= '0' && c <= '9' )
    return 1;
  else
    return 0;
}

// This returns the parsed expression as a node, or NULL. If NULL is returned, error is made to point at an error message. All named indices less than or equal to global 
// are considered global; all other indices should be bound lambdas.
u32 parseExpression( LNZprogram* p, const u8* string, u64 length, const char** error ){
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
    return s - string;
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
      return s - string;
    }
 
    // Eat identifiers, add them to the nametables, and build up an empty lambda at
    // top.
    u64 lambdaCount = 0;
    u32 cur, top;
    do{
      u64 namelen = 0;
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

      // Eat trailing namespace
      while( namelen < l && isWhitespace( s[ namelen ] ) )
	++namelen;

      if( namelen == l || ( s[ namelen ] != '.' && !isName( s[ namelen ] ) ) ){
	*error = "Syntax error: malformed lambda, expected '.' or an identifier.";
	return ( s - string ) + namelen;
      }
      
      s += namelen;
      l -= namelen;
	     
    }while( *s != '.' );
    // Parse the body.
    u32 pe = parseExpression( p, s + 1, l - 1, error );
    if( *error != NULL )
      return ( s - string ) + 1 + pe; 
    p->heap[ cur ].data = pe;
    // Pop scope.
    while( lambdaCount ){
      --lambdaCount;
      popNamePointerPair( p );
    }

    return top;
  }

  // Handle applications,free variables, numbers and strings.

  u64 applicationCount = 0;
  u32 top;
  // Read one expression at a time; must be parenthetical or a name.
  do{
    u32 arg;
    u64 namelen = 0;
    // Handle strings.
    if( s[ namelen ] == '\'' ){
      ++namelen;
      u32 nullstring = mallocNode( p );
      p->heap[ nullstring ].type = LNZ_DATA_TYPE;
      p->heap[ nullstring ].references = 0; // size.
      p->heap[ nullstring ].data = 0;
      arg = mallocNode( p );
      p->heap[ arg ].type = LNZ_STRING_TYPE;
      p->heap[ arg ].references = 1; // size.
      p->heap[ arg ].data = ( (u64)nullstring ) << 32;
      p->heap[ arg ].data += ( (u64)nullstring );
      
      
      u64 bsc = 0;
      while( namelen < l ){
	if( s[ namelen ] == '\'' ){
	  while( bsc >= 2 ){
	    bsc -= 2;
	    addStringChar( p, arg, '\\' );
	  }
	  if( bsc ){
	    addStringChar( p, arg, '\'' );
	    bsc = 0;
	  }else
	    break;
	}else if( s[ namelen ] == '\\' ){
	  ++bsc;
	}else{	  
	  while( bsc >= 2 ){
	    bsc -= 2;
	    addStringChar( p, arg, '\\' );
	  }
	  bsc = 0;
	  addStringChar( p, arg, s[ namelen ] );
	}
	++namelen;
      }
      if( namelen == l || s[ namelen ] != '\'' ){
	*error = "Syntax error: malformed string.";
	return ( s - string ) + namelen;
      }
      ++namelen;

    // Handle parenthetical subexpressions.
    }else if( *s == '[' ){
      namelen = 1;
      u64 f = 1;
      // scan for parentheses.
      u64 bsc = 0;
      u64 qm = 0;
      while( namelen < l ){
	if( s[ namelen ] == '\\' )
	  ++bsc;
	else if( s[ namelen ] == '\'' ){
	  if( !( bsc % 2 ) && qm )
	    qm = 0;
	  else if( !( bsc % 2 ) && !qm )
	    qm = 1;
	  bsc = 0;
	}else{
	  if( !qm && s[ namelen ] == '[' )
	    ++f;
	  if( !qm && s[ namelen ] == ']' )
	    --f;
	  bsc = 0;
	}
	++namelen;
	if( !f )
	  break;
      }
      if( f || namelen < 2 ){
	*error = "Syntax error: unbalanced parentheses.";
	return s - string;
      }
    
      arg = parseExpression( p, s + 1, namelen - 2, error );
      if( *error != NULL )
	return ( s - string ) + 1 + arg;

    }else{
      if( !l || !isName( *s ) ){
	*error = "Syntax error: malformed expression.";
	return s - string;
      }
        
      // Handle identifiers and integers.
      while( namelen < l && isName( s[ namelen ] ) )
	++namelen;    

      u64 isint = 1;
      u64 isminus = 0;
      u64 i = 0;
      if( s[ i ] == '-' ){
	++i;
	isminus = 1;
	if( namelen == 1 )
	  isint = 0;
      }
      for( ; i < namelen; ++i )
	if( !isNumber( s[ i ] ) )
	  isint = 0;

      if( isint ){
	arg = mallocNode( p );
	u32 dn = mallocNode( p );
	p->heap[ arg ].type = LNZ_INT_TYPE + isminus;
	p->heap[ arg ].references = 1;
	p->heap[ arg ].data = ( ( (u64)dn ) << 32 ) + ( (u64)dn );
	p->heap[ dn ].type = LNZ_DATA_TYPE;
	p->heap[ dn ].references = 1;
	p->heap[ dn ].data = 0;
	
	i = 0;
	if( s[ i ] == '-' )
	  ++i;
	
	while( i < namelen && isNumber( s[ i ] ) ){
	  multiplyNumberByInt( p, arg, 10 );
	  addIntToNumber( p, arg, s[ i ] - '0' );
	  ++i; 
	}
	
      }else{ 
	u64 nind = getIndex( p->names, s, namelen );
	if( !nind ){
	  *error = "Unknown identifier.";
	  return s - string;
	}
	u32 pntr = getPointerFromName( p, s, namelen );
	
	// if global or not a lambda, treat as an identifier, otherwise a free variable.
	if( nind <= p->global || p->heap[ pntr ].type != LNZ_LAMBDA_TYPE )
	  arg = *( ( u32* )( getName( p->pointers, nind, NULL ) ) );
	else{
	  arg = mallocNode( p );
	  p->heap[ arg ].type = LNZ_FREE_TYPE;
	  p->heap[ arg ].references = 1;
	  p->heap[ arg ].data = *( ( u32* )( getName( p->pointers, nind, NULL ) ) );
	}
      }
    }
     
    u32 newnode;
    if( applicationCount == 1 )
      newnode = top;
    else{
      newnode = mallocNode( p );
      p->heap[ newnode ].data = 0;
    }

    p->heap[ newnode ].type = LNZ_APPLICATION_TYPE;
    p->heap[ newnode ].references = 1;
    p->heap[ newnode ].data += ( (u64)arg ) << 32;
      
    if( applicationCount > 1 ){
      p->heap[ newnode ].data += top;
    } else if( !applicationCount )
	p->heap[ newnode ].data >>= 32;

    top = newnode;
      
    ++applicationCount;
  
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
    return top;
  }  
}

u64 parseLine( LNZprogram* p, const u8* string, u64 length, const char **error ){
  const u8* s = string;
  u64 l = length;

  // Eat up white space and parse an lvalue.
  while( l && isWhitespace( *s ) ){
    ++s;
    --l;
  }  
  u64 namelen = 0;
  while( namelen < l && isName( s[ namelen ] ) )
    ++namelen;
  if( !namelen || namelen >= l ){
    *error = "Syntax error: malformed equation";
    return s - string;
  }
  const u8* name = s;
  s += namelen;
  l -= namelen;
  
  // Eat up white space and parse an =.
  while( l && isWhitespace( *s ) ){
    ++s;
    --l;
  }  
  if( !l || *s != '=' ){
    *error = "Syntax error: malformed equation, expected an =.";
    return s - string;
  }
  ++s;
  --l;
  
  // scan for semicolon.
  u64 exprlen = 0;
  while( exprlen < l && s[ exprlen ] != ';' )
    ++exprlen;
  if( !exprlen || exprlen == l ){
    *error = "Syntax error: malformed equation, expected an expression and a ;.";
    return s - string;
  }
    
  u32 expr = parseExpression( p, s, exprlen, error );
  if( *error != NULL )
    return ( s - string ) + expr;
  
  if( getIndex( p->names, name, namelen ) ){
    *error = "Redefinition of an identifier.";
    return name - string;
  }
  addNamePointerPair( p, name, namelen, expr );
  ++p->global;
  
  s += exprlen + 1;
  l -= exprlen + 1;

  while( l && isWhitespace( *s ) ){
    ++s;
    --l;
  }  


  return s - string;
}

LNZprogram* parseProgram( const u8* string, u64 length, const char** error ){
  LNZprogram* ans = newProgram();
  const u8* s = string;
  u64 l = length;
  do{
    u64 prsd = parseLine( ans, s, l, error );
    s += prsd;
    l -= prsd;
    if( *error != NULL ){
      u64 line = 1;
      u64 chr = 1;
      const u8* c = string;
      while( c != s ){
	if( *c == '\n' ){
	  chr = 1;
	  ++line;
	}else
	  ++chr;
	++c;
      }
	  

      u64 pep = rand() % ( sizeof( pepTalks ) / sizeof( pepTalks[ 0 ] ) );
      char* err = LNZmalloc( strlen( *error ) + 
			     strlen( pepTalks[ pep ] ) + 256 );
      sprintf( err, "line and character: %u,%u.\n%s\n%s\n\n", (int)line, (int)chr,
	       *error, pepTalks[ pep ] );
      *error = err;
      deleteProgram( ans );
      return NULL;
    }
  }while( l );
  return ans;
}

void printExpression( const LNZprogram* p, u32 expression, u32 level ){
  u64 ind = getIndex( p->pointers, (const u8*)( &expression ), sizeof( u32 ) ); 
  if( level && ind ){
    u64 len;
    const u8* name = getName( p->names, ind, &len );
    for( u64 i = 0; i < len; ++i )
      putchar( (int)name[ i ] );
  }else{
    if( p->heap[ expression ].type == LNZ_LAMBDA_TYPE ){
      printf( "\\" );
      ind = 0;
      while( p->heap[ expression ].type == LNZ_LAMBDA_TYPE && !ind ){
	printf( "l%u", expression );
	expression = p->heap[ expression ].data;
	++level;
	ind = getIndex( p->pointers, (const u8*)( &expression ), sizeof( u32 ) );
	if( p->heap[ expression ].type == LNZ_LAMBDA_TYPE && !ind )
	  printf( " " );
	else{
	  printf( "." );
	  printExpression( p, expression, level + 1 );
	}
      }
    }else if( p->heap[ expression ].type == LNZ_APPLICATION_TYPE ){
      u32 low = p->heap[ expression ].data & (u32)( -1 );
      u32 high = p->heap[ expression ].data >> 32;
      ind = getIndex( p->pointers, (const u8*)( &high ), sizeof( u32 ) );
      u64 lind = getIndex( p->pointers, (const u8*)( &low ), sizeof( u32 ) );
      if( p->heap[ low ].type == LNZ_LAMBDA_TYPE && !lind ){ 
	printf( "[" );
	printExpression( p, low, level + 1 );
	printf( "]" );
      }else
	printExpression( p, low, level + 1 );
      if( ( p->heap[ high ].type == LNZ_APPLICATION_TYPE || 
	  p->heap[ high ].type == LNZ_LAMBDA_TYPE ) && !ind ){

	printf( " [" );
	printExpression( p, high, level + 1 );
	printf( "]" );
      }else{
	printf( " " );
	printExpression( p, high, level + 1 );
      }
    }else if( p->heap[ expression ].type == LNZ_STRING_TYPE ){
      u32 len = p->heap[ p->heap[ expression ].data >> 32 ].references;
      u32 w = p->heap[ expression ].data & (u32)-1;
      u64 shft = 0;
      putchar( '\'' );
      while( len ){
	int pc = (int)( ( p->heap[ w ].data & ( ( (u64)255 ) << shft ) ) >> shft );
	if( pc == '\'' || pc == '\\' )
	  putchar( '\\' );
	putchar( pc );
	shft += 8;
	if( shft >= 64 ){
	  shft = 0;
	  w = p->heap[ w ].references;
	}
	--len;
      }
      putchar( '\'' );

    }else if( p->heap[ expression ].type == LNZ_INT_TYPE ||
	      p->heap[ expression ].type == LNZ_NEGATIVE_INT_TYPE ){
      u32 back = p->heap[ expression ].data >> 32;
      u32 len = p->heap[ back ].references;

      if( p->heap[ expression ].type == LNZ_NEGATIVE_INT_TYPE &&
	  ( len > 1 || p->heap[ back ].data ) )
	putchar( '-' );
      
      u64 numlen = 0;
      char* num = numberToString( p, expression, &numlen );
      for( u64 i = 0; i < numlen; ++i )
	putchar( num[ i ] );
      LNZfree( num );

    }else if( p->heap[ expression ].type == LNZ_FREE_TYPE )
      printf( "l%u", (u32)p->heap[ expression ].data );
  } 
}


