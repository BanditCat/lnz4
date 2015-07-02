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
  addNameToTable( nt, (u8*)"bar", 3 );
  addNameToTable( nt, (u8*)"fol", 3 );
  addNameToTable( nt, (u8*)"too", 3 );
  addNameToTable( nt, (u8*)"fot", 3 );
  addNameToTable( nt, (u8*)"follow", 6 );
  addNameToTable( nt, (u8*)"follower", 8 );
  addNameToTable( nt, (u8*)"following", 9 );

  printNameTable( nt );


  printf( "getName( 2 ): %s\n", getName( nt, 2 ) );
  printf( "getIndex( \"fol\", 3 ): %u\n", (unsigned int)getIndex( nt, (u8*)"fol", 3 ) );
  printf( "getIndex( \"foll\", 4 ): %u\n", (unsigned int)getIndex( nt, (u8*)"foll", 4 ) );
  printf( "getIndex( \"follower\", 8 ): %u\n", (unsigned int)getIndex( nt, (u8*)"follower", 8 ) );

  deleteNameTable( nt );
#ifdef DEBUG
  printf( "Malloc count (non-zero is a memory leak): %u\n", (unsigned int)LNZmallocCount() );
#endif
}
