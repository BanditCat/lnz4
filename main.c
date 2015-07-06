////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"

#include <stdio.h>
#include <string.h>


    
int main( int argc, char** argv ){
  (void)argc; (void)argv;

  nameTable* nt = newNameTable();
  
  addNameToTable( nt, (u8*)"foo", 3 );
  addNameToTable( nt, (u8*)"follow", 6 );
  addNameToTable( nt, (u8*)"bar", 3 );
  addNameToTable( nt, (u8*)"fol", 3 );
  addNameToTable( nt, (u8*)"too", 3 );
  addNameToTable( nt, (u8*)"foo", 3 );
  addNameToTable( nt, (u8*)"fot", 3 );
  addNameToTable( nt, (u8*)"follow", 6 );
  addNameToTable( nt, (u8*)"follower", 8 );
  addNameToTable( nt, (u8*)"following", 9 );
  addNameToTable( nt, (u8*)"foo", 3 );
  addNameToTable( nt, (u8*)"foo", 3 );
  
  for( u64 i = 0; i < 3; ++i ){
    popNameTable( nt );
    printNameTable( nt );
  }
  

  printf( "getName( 2 ): %s\n", getName( nt, 2, NULL ) );
  printf( "getIndex( \"fol\", 3 ): %u\n", (unsigned int)getIndex( nt, (u8*)"fol", 3 ) );
  printf( "getIndex( \"foll\", 4 ): %u\n", (unsigned int)getIndex( nt, (u8*)"foll", 4 ) );
  printf( "getIndex( \"follower\", 8 ): %u\n", (unsigned int)getIndex( nt, (u8*)"follower", 8 ) );

  deleteNameTable( nt );


  const char* err = "bazly";
  LNZprogram* prog = newProgram();
  u32 expr;

  const char* code = "\\d.d d";

 
  expr = parseExpression( prog, (const u8*)code, strlen( code ), &err, 0 );
  if( err != NULL )
    printf( "%s\n\n", err );
  else printExpression( prog, expr );

  deleteProgram( prog );


#ifdef DEBUG
  printf( "Malloc count (non-zero is a memory leak): %u\n", (unsigned int)LNZmallocCount() );
#endif
}
