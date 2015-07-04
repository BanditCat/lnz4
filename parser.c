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

// This returns the parsed expression as a node, or NULL. If NULL is returned, error is made to point at an error message.
u32 parseExpression( const u8* string, u64 length, const char** error, LNZprogram* p ){
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
  // Handle parens.
  if( *s == '[' ){
    if( s[ l - 1 ] != ']' ){
      *error = "Syntax error: mismatched parentheses.";
      return 0;
    }
    return parseExpression( s + 1, l - 2, error, p ); 
  }
  
  // Handle lambdas
  if( *s == '\\' ){
    ++s;
    --l;
    if( !l || !isName( *s ) ){
      *error = "Syntax error: malformed lambda, expected a name.";
      return 0;
    }
    u64 i = 0;
    
    while( i < l && isName( s[ i ] ) )
      ++i;
    
  }

  
  return 1;

}



void printExpression( u32 expression, const LNZprogram* p ){
  printf( "foo%p", (void*)( expression + p ) );
}
