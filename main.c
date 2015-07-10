////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"




    
int main( int argc, char** argv ){
  (void)argc; (void)argv;
  srand( time( NULL ) );

  /* nameTable* nt = newNameTable(); */
  
  /* addNameToTable( nt, (u8*)"foo", 3 ); */
  /* addNameToTable( nt, (u8*)"follow", 6 ); */
  /* addNameToTable( nt, (u8*)"bar", 3 ); */
  /* addNameToTable( nt, (u8*)"fol", 3 ); */
  /* addNameToTable( nt, (u8*)"too", 3 ); */
  /* addNameToTable( nt, (u8*)"foo", 3 ); */
  /* addNameToTable( nt, (u8*)"fot", 3 ); */
  /* addNameToTable( nt, (u8*)"follow", 6 ); */
  /* addNameToTable( nt, (u8*)"follower", 8 ); */
  /* addNameToTable( nt, (u8*)"following", 9 ); */
  /* addNameToTable( nt, (u8*)"foo", 3 ); */
  /* addNameToTable( nt, (u8*)"foo", 3 ); */
  
  /* for( u64 i = 0; i < 3; ++i ){ */
  /*   popNameTable( nt ); */
  /*   printNameTable( nt ); */
  /* } */
  

  /* printf( "getName( 2 ): %s\n", getName( nt, 2, NULL ) ); */
  /* printf( "getIndex( \"fol\", 3 ): %u\n", (unsigned int)getIndex( nt, (u8*)"fol", 3 ) ); */
  /* printf( "getIndex( \"foll\", 4 ): %u\n", (unsigned int)getIndex( nt, (u8*)"foll", 4 ) ); */
  /* printf( "getIndex( \"follower\", 8 ): %u\n", (unsigned int)getIndex( nt, (u8*)"follower", 8 ) ); */

  /* deleteNameTable( nt ); */


  char* foo = LNZmalloc( 1 );
  char* bar = LNZmalloc( 1 );
  u64 flen = 1;
  u64 blen = 1;
  foo[ 0 ] = '0';
  bar[ 0 ] = '1';
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &foo, &flen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &foo, &flen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &bar, &blen, bar, blen );
  addStringToString( &foo, &flen, bar, blen );
  printf( "\n\nfoobar  \n" );
  for( u64 i = 0; i < flen; ++i )
    putchar( foo[ i ] );
  printf( "\n\n" );
  for( u64 i = 0; i < blen; ++i )
    putchar( bar[ i ] );
  printf( "\n\n" );
  LNZfree( foo );
  LNZfree( bar );


  const char* err = NULL;
 
  
  u64 len;
  u8* code = LNZLoadResourceOrDie( "base.lnz", &len );
  LNZprogram* prog = parseProgram( code, len, &err );
  LNZfree( code );
  if( err != NULL ){
    printf( "%s", err );
    LNZfree( (char*)err );
  } else 
    printProgram( prog );
  if( prog != NULL )
    deleteProgram( prog );


#ifdef DEBUG
  printf( "Malloc count (non-zero is a memory leak): %u\n", (unsigned int)LNZmallocCount() );
#endif
}


