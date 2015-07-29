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
     //validateGraph( lg );
    
    /* { */
    /*   LNZprogram* tp = makeProgramFromGraph( lg ); */
    /*   printProgram( tp, prog ); */
    /*   deleteProgram( tp ); */
    /* } */
    //  u32 w;
  /*   if( !ruleSweep( lg, ruleOneA, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule I.a at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*  { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleOneA, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule I.a at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleThreeA, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule III.a at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleTwoA, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule II.a at %u\n", w ); */
  /*       validateGraph( lg ); */
  /*  { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleFourB, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule IV.b at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleFourD, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule IV.d at %u graph e\n", w ); */
  /*      validateGraph( lg ); */

  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleFourD, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule IV.d at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleSevenH, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VII.h at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleSevenH, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VII.h at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleSevenF, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VII.f at %u graph f\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleSixEF, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VI.ef at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleTwoA, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule II.a at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleTwoA, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule II.a at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleSevenC, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VII.c at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleSevenC, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VII.c at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleSevenC, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VII.c at %u\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleSevenC, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VII.c at %u graph h\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleTwoC, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule II.c at %u graph i\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /* u32 ic; */

  /*   ic = 27; */
  /*   if( !ruleOneB( lg, ic ) ) */
  /*      LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule I.b at %u graph j\n", ic ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */

  /*   if( !ruleSweep( lg, ruleFourE, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule IV.e at %u graph k\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleSixA, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VI.a at %u graph l\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleFiveAB, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule V.ab at %u graph m\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*      validateGraph( lg ); */
  /*   if( !ruleSweep( lg, ruleSixCD, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VI.cd at %u\n", w ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
  /*   if( !ruleSweep( lg, ruleSixCD, &w ) ) */
  /*     LNZdie( "bloop" ); */
  /*   printf( "\nApplied rule VI.cd at %u graph n\n", w ); */
  /*      validateGraph( lg ); */
  /*   { */
  /*     LNZprogram* tp = makeProgramFromGraph( lg ); */
  /*     printProgram( tp, prog ); */
  /*     deleteProgram( tp ); */
  /*   } */
    

  /*   { */

  /*   LampingGraph* test = newLampingGraph(); */
  /*   u32 v = mallocLampingNode( test ); */
  /*   u32 l = mallocLampingNode( test ); */
  /*   u32 r = mallocLampingNode( test ); */
  /*   u32 rb = mallocLampingNode( test ); */
  /*   u32 bi = mallocLampingNode( test ); */
  /*   u32 bo = mallocLampingNode( test ); */
  /*   u32 bi2 = mallocLampingNode( test ); */
  /*   u32 bo2 = mallocLampingNode( test ); */
  /*   u32 rb2 = mallocLampingNode( test ); */
  /*   test->root = r; */

  /*   test->heap[ v ].type = LAMPING_FREE_TYPE; */
  /*   test->heap[ v ].la.arg = l; */
  /*   test->heap[ v ].in = bo; */

  /*   test->heap[ bo ].type = LAMPING_CONDITIONAL_BRACKET_TYPE; */
  /*   test->heap[ bo ].la.level = 0; */
  /*   test->heap[ bo ].in = v; */
  /*   test->heap[ bo ].out = bo2; */
  /*   test->heap[ bo2 ].type = LAMPING_CONDITIONAL_BRACKET_TYPE; */
  /*   test->heap[ bo2 ].la.level = 0; */
  /*   test->heap[ bo2 ].in = bo; */
  /*   test->heap[ bo2 ].out = bi; */
  /*   test->heap[ bi ].type = LAMPING_CONDITIONAL_BRACKET_TYPE; */
  /*   test->heap[ bi ].la.level = 0; */
  /*   test->heap[ bi ].in = bi2; */
  /*   test->heap[ bi ].out = bo2; */
  /*   test->heap[ bi2 ].type = LAMPING_CONDITIONAL_BRACKET_TYPE; */
  /*   test->heap[ bi2 ].la.level = 0; */
  /*   test->heap[ bi2 ].in = l; */
  /*   test->heap[ bi2 ].out = bi; */

  /*   test->heap[ l ].type = LAMPING_LAMBDA_TYPE; */
  /*   test->heap[ l ].out = bi2; */
  /*   test->heap[ l ].in = rb; */

  /*   test->heap[ rb ].type = LAMPING_RESTRICTED_BRACKET_TYPE; */
  /*   test->heap[ rb ].la.level = 0; */
  /*   test->heap[ rb ].in = l; */
  /*   test->heap[ rb ].out = rb2; */
  /*   test->heap[ rb2 ].type = LAMPING_RESTRICTED_BRACKET_TYPE; */
  /*   test->heap[ rb2 ].la.level = 0; */
  /*   test->heap[ rb2 ].in = rb; */
  /*   test->heap[ rb2 ].out = r; */

  /*   test->heap[ r ].type = LAMPING_ROOT_TYPE; */
  /*   test->heap[ r ].out = rb2; */

  /*   printLampingGraph( test ); */
  /*   LNZprogram* tp = makeProgramFromGraph( test ); */
  /*   printProgram( tp, prog ); */
  /*   deleteProgram( tp ); */
  /*   deleteLampingGraph( test ); */

  /* } */




  printLampingGraph( lg );
       LNZprogram* tp = makeProgramFromGraph( lg );
      printProgram( tp, prog );
      deleteProgram( tp );
    u32 c = 0;
    while( traceRulesSweep( lg ) ){
      printf( "\n%u\n", c++ );
      //validateGraph( lg );
    }
      tp = makeProgramFromGraph( lg );
      printProgram( tp, prog );
      deleteProgram( tp );
    
      //if( c > 95 && c < 100 )
      /* if( c > 82  ) */
      /* 	break; */
      // printLampingGraph( lg );
      //validateGraph( lg );
    

   
     deleteLampingGraph( lg ); 
   }

  if( prog != NULL )
    deleteProgram( prog );


#ifdef DEBUG
  printf( "Malloc count (non-zero is a memory leak): %u\n", (unsigned int)LNZmallocCount() );
#endif
}


