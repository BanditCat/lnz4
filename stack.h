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
void deleteStack( stack* st );
void push( stack* st, u32 val );
u32 pop( stack* st );


#endif //LNZ_STACK_H
