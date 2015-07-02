////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"

#include <stdio.h>
#include <string.h>

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

nameTable* newNameTable( void ){
  nameTable* ans = LNZmalloc( sizeof( nameTable ) );
  ans->size = 0;
  ans->bufsize = 1;  
  ans->dict = LNZcalloc( sizeof( nameTableNode ) );
  ans->dict->index = 0;
  ans->revdict = LNZmalloc( sizeof( u8* ) );
  return ans;
}


void deleteNodes( nameTableNode* ntn ){
  for( u64 i = 0; i < 256; ++i )
    if( ntn->continuations[ i ] != NULL )
     deleteNodes( ntn->continuations[ i ] );
  LNZfree( ntn );
}

void deleteNameTable( nameTable* nt ){
  for( u64 i = 0; i < nt->size; ++i )
    LNZfree( nt->revdict[ i ] );
  LNZfree( nt->revdict );
  deleteNodes( nt->dict );
  LNZfree( nt );
}
  
	

// Inserting the same name repeatedly is a no-op.
void addNameToTable( nameTable* nt, const u8* name, u64 namelen ){
  ++nt->size;
  if( nt->size > nt->bufsize ){
    nt->bufsize *= 2;
    u8** newrevdict = LNZmalloc( sizeof( u8* ) * nt->bufsize );
    memcpy( newrevdict, nt->revdict, sizeof( u8* ) * ( nt->size - 1 ) );
    LNZfree( nt->revdict );
    nt->revdict = newrevdict;
  }
  nt->revdict[ nt->size - 1 ] = LNZmalloc( namelen + 1 );
  memcpy( nt->revdict[ nt->size - 1 ], name, namelen );
  nt->revdict[ nt->size - 1 ][ namelen ] = '\0';
  
  nameTableNode* cl = nt->dict;
  const u8* s = name;
  while( (u64)( s - name ) < namelen ){
    if( cl->continuations[ *s ] != NULL )
      cl = cl->continuations[ *s ];
    else{
      nameTableNode* nn = LNZcalloc( sizeof( nameTableNode ) );
      cl->continuations[ *s ] = nn;
      cl = nn;
    }
    ++s;
  }
  cl->index = nt->size;
}

    
u64 getName( nameTable* nt, const u8* name, u64 namelen ){
  const u8* n = name;
  nameTableNode* ntn = nt->dict;
  while( (u64)( n - name ) < namelen ){
    if( ntn->continuations[ *n ] != NULL ){
      ntn = ntn->continuations[ *n ];
      ++n;
    } else 
      return 0;
  }
  return ntn->index;
}


void printNameTableRec( u64 prefix, const nameTableNode* ntn ){
  u64 c = 0;
  if( ntn->index ){
    ++c;
    printf( " - index %u", (unsigned int)ntn->index );
  }
  for( u64 i = 0; i < 256; ++i ){
    if( ntn->continuations[ i ] != NULL ){
      if( c ){
	printf( "\n" );
	for( u64 i = 0; i < prefix; ++i )
	  printf( " " );
      }
      printf( "%c", (int)i );
      printNameTableRec( prefix + 1, ntn->continuations[ i ] );
      ++c;
    }
  }
}

void printNameTable( const nameTable* nt ){
  printNameTableRec( 0, nt->dict );
  printf( "\n\n" );
  for( u64 i = 0; i < nt->size; ++i )
    printf( "index %u - %s\n", (int)( i + 1 ), nt->revdict[ i ] );
}
    
int main( int argc, char** argv ){
  (void)argc; (void)argv;

  nameTable* nt = newNameTable();
  
  addNameToTable( nt, (u8*)"foo", 3 );
  addNameToTable( nt, (u8*)"bar", 3 );
  addNameToTable( nt, (u8*)"fol", 3 );
  addNameToTable( nt, (u8*)"too", 3 );
  addNameToTable( nt, (u8*)"fot", 3 );
  addNameToTable( nt, (u8*)"follow", 6 );
  addNameToTable( nt, (u8*)"follower", 8 );
  addNameToTable( nt, (u8*)"following", 9 );

  printNameTable( nt );


  deleteNameTable( nt );
#ifdef DEBUG
  printf( "Malloc count (non-zero is a memory leak): %u\n", (unsigned int)LNZmallocCount() );
#endif
}
