////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#ifndef LNZ_STACK_H
#define LNZ_STACK_H

// A stack of u32s.
typedef struct{
  u64 size;
  u64 bufsize;
  u32* stack;
} stack;

stack* newStack( void );
stack* copyStack( const stack* c );
int equalsStack( const stack* a, const stack* b );
void deleteStack( stack* st );
void push( stack* st, u32 val );
u32 pop( stack* st );
u32 top( stack* st );


#endif //LNZ_STACK_H
