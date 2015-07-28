////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"

stack* newStack( void ){
  stack* ans = LNZcalloc( sizeof( stack ) );
  ans->bufsize = 1;
  ans->stack = LNZmalloc( sizeof( u32 ) );
  return ans;
}

stack* copyStack( const stack* c ){
  stack* ans = LNZmalloc( sizeof( stack ) );
  ans->bufsize = c->bufsize;
  ans->size = c->size;
  ans->stack = LNZmalloc( sizeof( u32 ) * c->bufsize );
  for( u64 i = 0; i < ans->size; ++i )
    ans->stack[ i ] = c->stack[ i ];
  return ans;
}

int equalsStack( const stack* a, const stack* b ){
  if( a->size != b->size )
    return 0;
  for( u64 i = 0; i < a->size; ++i )
    if( a->stack[ i ] != b->stack[ i ] )
      return 0;
  return 1;
}
void deleteStack( stack* st ){
  LNZfree( st->stack );
  LNZfree( st );
}

void push( stack* st, u32 val ){
  if( st->size >= st->bufsize ){
    st->bufsize *= 2;
    u32* nvp = LNZmalloc( sizeof( u32 ) * st->bufsize );
    memcpy( nvp, st->stack, sizeof( u32 ) * st->size );
    LNZfree( st->stack );
    st->stack = nvp;
  }
  st->stack[ st->size++ ] = val;
}
u32 pop( stack* st ){
#ifdef DEBUG
  if( !st->size )
    LNZdie( "You can't pop an empty stack!" );
#endif
  return st->stack[ --st->size ];
}
u32 top( stack* st ){
#ifdef DEBUG
  if( !st->size )
    LNZdie( "You can't top an empty stack!" );
#endif
  return st->stack[ st->size - 1 ];
}
