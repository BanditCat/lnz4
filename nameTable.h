////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#ifndef LNZ_NAMETABLE_H
#define LNZ_NAMETABLE_H

// A trie based name table associating strings with 64 bit integers. 0 is not a valid index; they begin at 1.
// This is fast but massive. It could be improved with embedded strings.
// If the same name is added more than once, the latest index added is returned when queried.
typedef struct indexNode{
  u64 index;
  struct indexNode* next;
} nameTableIndexNode;
typedef struct node{
  struct node* continuations[ 256 ];
  nameTableIndexNode* indices;
} nameTableNode;
typedef struct{
  u64 size;
  u64 bufsize;
  nameTableNode* dict;
  u8** revdict;
  u64* revdictSizes;
} nameTable;

nameTable* newNameTable( void );
void deleteNameTable( nameTable* nt );
	
void addNameToTable( nameTable* nt, const u8* name, u64 namelen );
// Returns the 1-based index of a string, or 0 if there isn't one.  
u64 getIndex( const nameTable* nt, const u8* name, u64 namelen );
// Returns the string associated with an index, indices are 1 based and there is no error checking.
const u8* getName( const nameTable* nt, u64 index );
// Pops the last name added off.
void popNameTable( nameTable* nt );


void printNameTable( const nameTable* nt );


#endif //LNZ_NAMETABLE_H 

