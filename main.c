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



  const char* err = NULL;
 
  
  u64 len;
  u8* code = LNZLoadResourceOrDie( "base.lnz", &len );
  LNZprogram* prog = parseProgram( "base.lnz", code, len, &err );
  const u8* eval = (const u8*)"inc";
  LNZprogram* eprog = NULL;
  if( prog != NULL )
    eprog  = makeComputable( prog, getPointerFromName( prog, eval, strlen( (const char*)eval ) ) );
  LNZfree( code );
  if( err != NULL ){
    printf( "%s", err );
    LNZfree( (char*)err );
  } else{
    printProgram( prog, NULL );
 
    u64 reds = 0;
    u64 red = 0;
    do{
      if( !( reds % 10 ) ){
	printf( "\n%u reductions.\n\n", (unsigned int)reds );
	printProgram( eprog, prog );
      }
      //  printHeap( eprog );
       //red = betaReduceNormalOrder( eprog, getPointerFromName( eprog, (const u8*)"e", 1 ) );
       reds += red;
    }while( red );
    printf( "\n%u reductions.\n\n", (unsigned int)reds );
    printProgram( eprog, prog );
  }
  if( eprog != NULL )
    deleteProgram( eprog );

  if( prog != NULL ){
    LampingGraph* lg = makeGraph( prog, getPointerFromName( prog, eval, strlen( (const char*)eval ) ) );
    printLampingGraph( lg );
    deleteLampingGraph( lg );
  }

  if( prog != NULL )
    deleteProgram( prog );


#ifdef DEBUG
  printf( "Malloc count (non-zero is a memory leak): %u\n", (unsigned int)LNZmallocCount() );
#endif
}


