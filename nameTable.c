////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"


nameTable* newNameTable( void ){
  nameTable* ans = LNZmalloc( sizeof( nameTable ) );
  ans->size = 0;
  ans->bufsize = 1;  
  ans->dict = LNZcalloc( sizeof( nameTableNode ) );
  ans->dict->indices = NULL;
  ans->revdict = LNZmalloc( sizeof( u8* ) );
  ans->revdictSizes = LNZmalloc( sizeof( u64 ) );
  return ans;
}

void deleteIndexNodes( nameTableIndexNode* ntin ){
  nameTableIndexNode* w = ntin;
  
  while( w != NULL ){
    w = w->next;
    LNZfree( ntin );
    ntin = w;
  }
}
    

void deleteNodes( nameTableNode* ntn ){
  deleteIndexNodes( ntn->indices );
  for( u64 i = 0; i < 256; ++i ){
    if( ntn->continuations[ i ] != NULL )
     deleteNodes( ntn->continuations[ i ] );
  }
  LNZfree( ntn );
}

void deleteNameTable( nameTable* nt ){
  for( u64 i = 0; i < nt->size; ++i )
    LNZfree( nt->revdict[ i ] );
  LNZfree( nt->revdict );
  LNZfree( nt->revdictSizes );
  deleteNodes( nt->dict );
  LNZfree( nt );
}
  
	

// Inserting the same name repeatedly or a name with NULs will melt your computer.
void addNameToTable( nameTable* nt, const u8* name, u64 namelen ){
  ++nt->size;
  if( nt->size > nt->bufsize ){
    nt->bufsize *= 2;
    u8** newrevdict = LNZmalloc( sizeof( u8* ) * nt->bufsize );
    memcpy( newrevdict, nt->revdict, sizeof( u8* ) * ( nt->size - 1 ) );
    u64* newrevdictsizes = LNZmalloc( sizeof( u64 ) * nt->bufsize );
    memcpy( newrevdictsizes, nt->revdictSizes, sizeof( u64 ) * ( nt->size - 1 ) );
    LNZfree( nt->revdict );
    LNZfree( nt->revdictSizes );
    nt->revdict = newrevdict;
    nt->revdictSizes = newrevdictsizes;
  }
  nt->revdict[ nt->size - 1 ] = LNZmalloc( namelen );
  memcpy( nt->revdict[ nt->size - 1 ], name, namelen );
  nt->revdictSizes[ nt->size - 1 ] = namelen;
  
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
  nameTableIndexNode* w = cl->indices;
  cl->indices = LNZcalloc( sizeof( nameTableIndexNode ) );
  cl->indices->index = nt->size;
  cl->indices->next = w;
}

// Returns the index of a string, or 0 if there isn't one.  
u64 getIndex( const nameTable* nt, const u8* name, u64 namelen ){
  const u8* n = name;
  nameTableNode* ntn = nt->dict;
  while( (u64)( n - name ) < namelen ){
    if( ntn->continuations[ *n ] != NULL ){
      ntn = ntn->continuations[ *n ];
      ++n;
    } else 
      return 0;
  }
  if( ntn->indices == NULL )
    return 0;
  else
    return ntn->indices->index;
}
// Returns the string associated with an index, indices are 1 based and there is no error checking.
const u8* getName( const nameTable* nt, u64 index ){
  return nt->revdict[ index - 1 ];
}


void printNameTableRec( u64 prefix, const nameTableNode* ntn ){
  u64 c = 0;
  if( ntn->indices != NULL ){
    ++c;
    printf( " - indices: %u", (unsigned int)ntn->indices->index );
    nameTableIndexNode* w = ntn->indices->next;
    while( w != NULL ){
      printf( ", %u", (unsigned int)w->index );
      w = w->next;
    }

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
  for( u64 i = 0; i < nt->size; ++i ){
    printf( "index %u - ", (int)( i + 1 ) );
    for( u64 j = 0; j < nt->revdictSizes[ i ]; ++j )
      printf( "%c", (int)nt->revdict[ i ][ j ] );
    printf( "\n" );
  }
  printf( "\n" );
}

// Better make sure that the name is in the dictionary first!
// This returns 1 if nodes were freed and 0 if just an index was cleared.
int removeName( const u8* name, u64 length, nameTableNode* ntn ){
  if( length ){
    if( removeName( name + 1, length - 1, ntn->continuations[ *name ] ) )
      ntn->continuations[ *name ] = NULL;
  }else{
    nameTableIndexNode* w = ntn->indices;
    ntn->indices = ntn->indices->next;
    LNZfree( w );
  }
  int del = 1;
  for( u64 i = 0; i < 256; ++i ){
    if( ntn->continuations[ i ] != NULL )
      del = 0;
  }
  if( ntn->indices != NULL )
    del = 0;
  if( del )
    LNZfree( ntn );
  return del;
}

void popNameTable( nameTable* nt ){
  if( nt->size ){
    --nt->size;
    if( removeName( nt->revdict[ nt->size ], nt->revdictSizes[ nt->size ], nt->dict ) && !nt->size ){
      nt->dict = LNZcalloc( sizeof( nameTableNode ) );
      nt->dict->indices = NULL;
    }
  }
  
  LNZfree( nt->revdict[ nt->size ] );
}
