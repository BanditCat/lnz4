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
  return st->stack[ --st->size ];
}
u32 top( stack* st ){
  return st->stack[ st->size - 1 ];
}
