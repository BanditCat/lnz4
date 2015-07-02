////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// name tables associating strings with 64 bit integers using tries.

#ifndef NAMETABLE_H
#define NAMETABLE_H

// A trie based name table associating strings with 64 bit integers. 0 is not a valid index; they begin at 1.
// This is fast but massive. It could be improved with embedded strings.
// Doesn't handle NUL or adding the same name twice.
typedef struct node{
  struct node* continuations[ 256 ];
  u64 index;
} nameTableNode;
typedef struct{
  u64 size;
  u64 bufsize;
  nameTableNode* dict;
  u8** revdict;
} nameTable;

nameTable* newNameTable( void );
void deleteNameTable( nameTable* nt );
	
void addNameToTable( nameTable* nt, const u8* name, u64 namelen );
// Returns the index of a string, or 0 if there isn't one.  
u64 getIndex( const nameTable* nt, const u8* name, u64 namelen );
// Returns the string associated with an index, indices are 1 based and there is no error checking.
const u8* getName( const nameTable* nt, u64 index );


void printNameTable( const nameTable* nt );


#endif //NAMETABLE_H 

