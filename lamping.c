////////////////////////////////////////////////////////////////////////////////
// This is a public domain implementation of Lamping's optimal beta reduction //
// algorithm written by Jon DuBois in 2015.                                   //
////////////////////////////////////////////////////////////////////////////////

#include "lnz.h"


u32 mallocLampingNode( LampingGraph* g ){
  if( g->heap[ g->frees ].out == g->frees ){
    LampingNode* nh = LNZcalloc( sizeof( LampingNode ) * g->heapsize * 2 );
    memcpy( nh, g->heap, sizeof( LampingNode ) * g->heapsize );
    LNZfree( g->heap );
    g->heap = nh;
    for( u64 i = g->heapsize; i < g->heapsize * 2; ++i ){
      g->heap[ i ].out = i + 1;
      if( i + 1 == g->heapsize * 2 )
	g->heap[ i ].out = i;
    }
    g->heap[ g->frees ].out = g->heapsize;
    g->heapsize *= 2;
  }
  u32 ans = g->frees;
  g->frees = g->heap[ g->frees ].out;
  return ans;
}
void freeLampingNode( LampingGraph* g, u32 w ){
  g->heap[ w ].type = 0;
  g->heap[ w ].out = g->frees;
  g->frees = w;
}

LampingGraph* newLampingGraph( void ){
  LampingGraph* ans = LNZmalloc( sizeof( LampingGraph ) );
  ans->heapsize = LNZ_INITIAL_HEAP_SIZE;
  ans->heap = LNZcalloc( sizeof( LNZnode ) * ans->heapsize ); 
  // Mark all addresses free.
  for( u32 i = 0; i < ans->heapsize; ++i ){
    ans->heap[ i ].out = i + 1;
    if( i + 1 == ans->heapsize )
      ans->heap[ i ].out = i;
  }
  ans->frees = 0;

  return ans;
}
void deleteLampingGraph( LampingGraph* g ){
  LNZfree( g->heap );
  LNZfree( g );
}


// Returns 1 iff a node and a graph are equal, during the weak copy stage. This
// functon uses lambdas a nametable to look up free variables. 
int equalsGraphTree( const LampingGraph* g, u32 gind, const LNZprogram* p,
		     u32 pind, LNZprogram* lambdas ){
  if( g->heap[ gind ].type == LAMPING_LAMBDA_TYPE &&
      p->heap[ pind ].type == LNZ_LAMBDA_TYPE ){
    addNamePointerPair( lambdas, (const u8*)( &pind ), sizeof( u32 ), gind );
    int ans = equalsGraphTree( g, g->heap[ gind ].out, 
			       p, p->heap[ pind ].data, lambdas );
    popNamePointerPair( lambdas );
    return ans;
  }else if( g->heap[ gind ].type == LAMPING_APPLICATION_TYPE &&
	    p->heap[ pind ].type == LNZ_APPLICATION_TYPE ){
    int ans = equalsGraphTree( g, g->heap[ gind ].out,
			       p, p->heap[ pind ].data, lambdas );
    if( !ans )
      return 0;
    return equalsGraphTree( g, g->heap[ gind ].la.arg,
			    p, p->heap[ pind ].data >> 32, lambdas );
  }else if( g->heap[ gind ].type == LAMPING_FREE_TYPE &&
	    p->heap[ pind ].type == LNZ_FREE_TYPE ){
    u32 wind = getIndex( lambdas->names, (const u8*) &( p->heap[ pind ].data ), 
			 sizeof( u32 ) );
    if( !wind )
      return 0;
    if( *( (const u32*)( getName( lambdas->pointers, wind, NULL ) ) ) == 
	g->heap[ gind ].la.arg )
      return 1;
    else
      return 0;
  }
  return 0;
}

// Weakly copies a tree expression into a graph. For all nodes in just points to the 
// node, because there may be a reference counts over 1.
u32 copyTreeToGraph( LampingGraph* g, LNZprogram* p, u32 ind,
		     LNZprogram* lambdas, stack* subexprs ){
  // If this expression already exists in the graph, return it.
  for( u64 i = 0; i < subexprs->size; ++i ){
    if( equalsGraphTree( g, subexprs->stack[ i ], p, ind, lambdas ) ) 
      return subexprs->stack[ i ];
  }
  
  u32 nn = mallocLampingNode( g );
  g->heap[ nn ].in = nn;
  if( p->heap[ ind ].type == LNZ_APPLICATION_TYPE ){
    g->heap[ nn ].type = LAMPING_APPLICATION_TYPE;
    u32 func = copyTreeToGraph( g, p, p->heap[ ind ].data, lambdas, subexprs );
    u32 arg = copyTreeToGraph( g, p, ( p->heap[ ind ].data >> 32 ), lambdas, 
			       subexprs );
    g->heap[ nn ].la.arg = arg;
    g->heap[ nn ].out = func;
  }else if( p->heap[ ind ].type == LNZ_LAMBDA_TYPE ){
    g->heap[ nn ].type = LAMPING_LAMBDA_TYPE;
    g->heap[ nn ].la.arg = nn;
    addNamePointerPair( lambdas, (const u8*)( &ind ), sizeof( u32 ), nn );
    u32 bdy = copyTreeToGraph( g, p, p->heap[ ind ].data, lambdas, subexprs );
    popNamePointerPair( lambdas );
    g->heap[ nn ].out = bdy;
  }else if( p->heap[ ind ].type == LNZ_FREE_TYPE ){
    g->heap[ nn ].type = LAMPING_FREE_TYPE;
    g->heap[ nn ].out = nn;
    u32 lmbda = getPointerFromName( lambdas, (const u8*)( &( p->heap[ ind ].data ) ),
				    sizeof( u32 ) );
    g->heap[ nn ].la.arg = lmbda;
    g->heap[ lmbda ].la.arg = nn;
    push( subexprs, nn );
  }else
    LNZdie( "IMPLEMENT ME!BUGBUG" );
  return nn;
}

void repoint( LampingGraph* g, u32 ind, u32 whch, u32 new ){
  if( g->heap[ ind ].type != LAMPING_ROOT_TYPE &&
      g->heap[ ind ].type != LAMPING_VOID_TYPE ){
    if( g->heap[ ind ].in == whch ){
      g->heap[ ind ].in = new;
      return;
    }
  }
  if( g->heap[ ind ].type != LAMPING_FREE_TYPE ){
    if( g->heap[ ind ].out == whch ){
      g->heap[ ind ].out = new;
      return;
    }
  }
  if( g->heap[ ind ].type == LAMPING_APPLICATION_TYPE ||
      g->heap[ ind ].type >= LAMPING_FAN_START ){
    if( g->heap[ ind ].la.arg == whch ){
      g->heap[ ind ].la.arg = new;
      return;
    }
  }
  LNZdie( "ldnlsog" );
}


// This function points all ins upward at the parent, generating fan-ins where
// necessary.
void fanin( LampingGraph* g ){
  for( u64 i = 0; i < g->heapsize; ++i ){
    if( g->heap[ i ].type ){
      if( g->heap[ i ].type == LAMPING_APPLICATION_TYPE ){
	u32 arg = g->heap[ i ].la.arg;
	if( g->heap[ arg ].in == arg )
	  g->heap[ arg ].in = i;
	else{
	  u32 fi = mallocLampingNode( g );
	  u32 op = g->heap[ arg ].in;
	  g->heap[ fi ].type = LAMPING_FAN_START;
	  g->heap[ fi ].out = arg;
	  g->heap[ arg ].in = fi;
	  g->heap[ i ].la.arg = fi;
	  g->heap[ fi ].la.arg = i;
	  g->heap[ fi ].in = op;
	  repoint( g, op, arg, fi );
	}
	u32 func = g->heap[ i ].out;
	if( g->heap[ func ].in == func )
	  g->heap[ func ].in = i;
	else{
	  u32 fi = mallocLampingNode( g );
	  u32 op = g->heap[ func ].in;
	  g->heap[ fi ].type = LAMPING_FAN_START;
	  g->heap[ fi ].out = func;
	  g->heap[ func ].in = fi;
	  g->heap[ i ].out = fi;
	  g->heap[ fi ].la.arg = i;
	  g->heap[ fi ].in = op;
	  repoint( g, op, func, fi );
	}
      }else if( g->heap[ i ].type == LAMPING_LAMBDA_TYPE ){
	u32 bdy = g->heap[ i ].out;
	if( g->heap[ bdy ].in == bdy )
	  g->heap[ bdy ].in = i;
	else{
	  u32 fi = mallocLampingNode( g );
	  u32 op = g->heap[ bdy ].in;
	  g->heap[ fi ].type = LAMPING_FAN_START;
	  g->heap[ fi ].out = bdy;
	  g->heap[ bdy ].in = fi;
	  g->heap[ i ].out = fi;
	  g->heap[ fi ].la.arg = i;
	  g->heap[ fi ].in = op;
	  repoint( g, op, bdy, fi );
	}
      }else if( g->heap[ i ].type == LAMPING_ROOT_TYPE ){
	u32 bdy = g->heap[ i ].out;
	if( g->heap[ bdy ].in == bdy )
	  g->heap[ bdy ].in = i;
      }     
    }
  }
}
  
// This puts a general close variable over free variables in lambda ind.
// Returns 1 iff there was a free variable in ind.
int bracketFreeVars( LampingGraph* g, u32 ind, nameTable* lambdas ){
  if( g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE )
    return bracketFreeVars( g, g->heap[ ind ].in, lambdas );
  else if( g->heap[ ind ].type == LAMPING_BRACKET_TYPE )
    return bracketFreeVars( g, g->heap[ ind ].out, lambdas );
  else if( g->heap[ ind ].type >= LAMPING_FAN_START )
    return bracketFreeVars( g, g->heap[ ind ].out, lambdas );
  else if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    addNameToTable( lambdas, (const u8*)( &ind ), sizeof( u32 ) );
    int ans = bracketFreeVars( g, g->heap[ ind ].out, lambdas );
    if( getIndex( lambdas, (const u8*)( &ind ), sizeof( u32 ) ) )
      popNameTable( lambdas );
    return ans;
  }else if( g->heap[ ind ].type == LAMPING_APPLICATION_TYPE ){
    int ft = bracketFreeVars( g, g->heap[ ind ].out, lambdas );
    return bracketFreeVars( g, g->heap[ ind ].la.arg, lambdas ) || ft;
  }else if( g->heap[ ind ].type == LAMPING_FREE_TYPE ){
    if( getIndex( lambdas, (const u8*)( &( g->heap[ ind ].la.arg ) ), 
		  sizeof( u32 ) ) )
      return 0;
    u32 t = g->heap[ ind ].in;
    u32 nn = mallocLampingNode( g ); 
    g->heap[ nn ].type = LAMPING_BRACKET_TYPE;
    g->heap[ nn ].la.level = 0;
    g->heap[ nn ].in = t;
    g->heap[ nn ].out = ind;
    repoint( g, t, ind, nn );
    g->heap[ ind ].in = nn;
    addNameToTable( lambdas, (const u8*)( &( g->heap[ ind ].la.arg ) ), sizeof( u32 ) );
    return 1;
  }
  return 0;
}
// This is a better version that respects the transparency rule.
int betterBracketFreeVars( LampingGraph* g, u32 ind, u32 from, nameTable* lambdas ){
  if( g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE )
    return betterBracketFreeVars( g, g->heap[ ind ].in, ind, lambdas );
  else if( g->heap[ ind ].type == LAMPING_BRACKET_TYPE )
    return betterBracketFreeVars( g, g->heap[ ind ].out, ind, lambdas );
  else if( g->heap[ ind ].type >= LAMPING_FAN_START ){
    u32 bot = g->heap[ ind ].out;
    while( g->heap[ bot ].type >= LAMPING_FAN_START )
      bot = g->heap[ bot ].out;
    if( g->heap[ bot ].type == LAMPING_FREE_TYPE ){
      if( getIndex( lambdas, (const u8*)( &( g->heap[ bot ].la.arg ) ), 
		    sizeof( u32 ) ) )
	return 0;
      u32 nn = mallocLampingNode( g ); 
      g->heap[ nn ].type = LAMPING_BRACKET_TYPE;
      g->heap[ nn ].la.level = 0;
      g->heap[ nn ].in = from;
      g->heap[ nn ].out = ind;
      repoint( g, from, ind, nn );
      if( from == g->heap[ ind ].in )
	g->heap[ ind ].in = nn;
      else if( from == g->heap[ ind ].la.arg )
	g->heap[ ind ].la.arg = nn;
      else
	LNZdie( "Antipep!" );
      return 1;    
    } else
      return betterBracketFreeVars( g, g->heap[ ind ].out, ind, lambdas );
  }
  else if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    addNameToTable( lambdas, (const u8*)( &ind ), sizeof( u32 ) );
    int ans = betterBracketFreeVars( g, g->heap[ ind ].out, ind, lambdas );
    popNameTable( lambdas );
    return ans;
  }else if( g->heap[ ind ].type == LAMPING_APPLICATION_TYPE ){
    int ft = betterBracketFreeVars( g, g->heap[ ind ].out, ind, lambdas );
    return betterBracketFreeVars( g, g->heap[ ind ].la.arg, ind, lambdas ) || ft;
  }else if( g->heap[ ind ].type == LAMPING_FREE_TYPE ){
    if( getIndex( lambdas, (const u8*)( &( g->heap[ ind ].la.arg ) ), 
		  sizeof( u32 ) ) )
      return 0;
    u32 t = g->heap[ ind ].in;
    u32 nn = mallocLampingNode( g ); 
    g->heap[ nn ].type = LAMPING_BRACKET_TYPE;
    g->heap[ nn ].la.level = 0;
    g->heap[ nn ].in = t;
    g->heap[ nn ].out = ind;
    repoint( g, t, ind, nn );
    g->heap[ ind ].in = nn;
    return 1;
  }
  return 0;
}

// This is a helper function to set up brackets over free variables and 
// the lambdas containing them. This also creates voids for lambdas with
// no variables.
void bracketFreeLambdas( LampingGraph* g ){ 
  nameTable* nt = newNameTable();
  for( u64 i = 0; i < g->heapsize; ++i ){
    if( g->heap[ i ].type == LAMPING_LAMBDA_TYPE ){
      if( g->heap[ i ].la.arg == i ){
	u32 vd = mallocLampingNode( g );
	u32 vr = mallocLampingNode( g );
	g->heap[ vd ].type = LAMPING_VOID_TYPE;
	g->heap[ vd ].la.arg = vd;
	g->heap[ vd ].in = vd;
	g->heap[ vd ].out = vr;
	g->heap[ vr ].type = LAMPING_FREE_TYPE;
	g->heap[ vr ].in = vd;
	g->heap[ vr ].out = vr;
	g->heap[ vr ].la.arg = i;
	g->heap[ i ].la.arg = vr;
      }
      while( nt->size )
	popNameTable( nt );
      if( betterBracketFreeVars( g, i, i, nt ) ){
	u32 t = g->heap[ i ].in;
	u32 nn = mallocLampingNode( g ); 
	g->heap[ nn ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
	g->heap[ nn ].la.level = 0;
	g->heap[ nn ].in = i;
	g->heap[ nn ].out = t;
	repoint( g, t, i, nn );
	g->heap[ i ].in = nn;
      }
    }
  }
  deleteNameTable( nt );
}

void bracketFanins( LampingGraph* g ){
  for( u64 i = 0; i < g->heapsize; ++i ){
    if( g->heap[ i ].type >= LAMPING_FAN_START ){
      u32 t = g->heap[ i ].in;
      u32 nn = mallocLampingNode( g ); 
      g->heap[ nn ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
      g->heap[ nn ].la.level = 0;
      g->heap[ nn ].in = i;
      g->heap[ nn ].out = t;
      repoint( g, t, i, nn );
      g->heap[ i ].in = nn;

      t = g->heap[ i ].la.arg;
      nn = mallocLampingNode( g ); 
      g->heap[ nn ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
      g->heap[ nn ].la.level = 0;
      g->heap[ nn ].in = i;
      g->heap[ nn ].out = t;
      repoint( g, t, i, nn );
      g->heap[ i ].la.arg = nn;

      t = g->heap[ i ].out;
      nn = mallocLampingNode( g ); 
      g->heap[ nn ].type = LAMPING_BRACKET_TYPE;
      g->heap[ nn ].la.level = 0;
      g->heap[ nn ].in = i;
      g->heap[ nn ].out = t;
      repoint( g, t, i, nn );
      g->heap[ i ].out = nn;
    }
  }
}

LampingGraph* makeGraph( LNZprogram* p, u32 ind ){
  LampingGraph* ans = newLampingGraph();
  LNZprogram* lambdas = newProgram();
  stack* subexprs = newStack();

  u32 bd = copyTreeToGraph( ans, p, ind, lambdas, subexprs );
  u32 rt = mallocLampingNode( ans );
  ans->heap[ rt ].type = LAMPING_ROOT_TYPE;
  ans->heap[ rt ].la.arg = rt;
  ans->heap[ rt ].in = rt;
  ans->heap[ rt ].out = bd;
  

  fanin( ans );
  bracketFreeLambdas( ans );
  bracketFanins( ans );

  ans->root = rt;

  deleteStack( subexprs );
  deleteProgram( lambdas );
  return ans;

}


void printLampingGraph( const LampingGraph* g ){
  for( u64 i = 0; i < g->heapsize; ++i ){
    if( g->heap[ i ].type ){
      printf( "node# %8u, ", (unsigned int)i );
      if( g->heap[ i ].type == LAMPING_APPLICATION_TYPE )
	printf( "application, parent  %8u, func    %8u, arg     %8u\n", 
		g->heap[ i ].in, g->heap[ i ].out, g->heap[ i ].la.arg );
      else if( g->heap[ i ].type == LAMPING_LAMBDA_TYPE )
	printf( "lambda     , parent  %8u, body    %8u, var     %8u\n", 
		g->heap[ i ].in, g->heap[ i ].out, g->heap[ i ].la.arg );
      else if( g->heap[ i ].type == LAMPING_FREE_TYPE )
	printf( "variable   , parent  %8u, lambda  %8u\n", 
		g->heap[ i ].in, g->heap[ i ].la.arg );
      else if( g->heap[ i ].type == LAMPING_BRACKET_TYPE )
	printf( "bracket    , level   %8u, in      %8u, out     %8u\n", 
		g->heap[ i ].la.level, g->heap[ i ].in, g->heap[ i ].out );
      else if( g->heap[ i ].type == LAMPING_RESTRICTED_BRACKET_TYPE )
	printf( "r.bracket  , level   %8u, in      %8u, out     %8u\n", 
		g->heap[ i ].la.level, g->heap[ i ].in, g->heap[ i ].out );
      else if( g->heap[ i ].type == LAMPING_CONDITIONAL_BRACKET_TYPE )
	printf( "c.bracket  , level   %8u, in      %8u, out     %8u\n", 
		g->heap[ i ].la.level, g->heap[ i ].in, g->heap[ i ].out );
      else if( g->heap[ i ].type == LAMPING_ROOT_TYPE )
	printf( "root       , body    %8u\n", 
		g->heap[ i ].out );
      else if( g->heap[ i ].type == LAMPING_VOID_TYPE )
	printf( "void       , body    %8u\n", 
		g->heap[ i ].out );
      else if( g->heap[ i ].type >= LAMPING_FAN_START )
	printf( "fan        , level   %8u, point   %8u, star    %8u, zero    %8u\n",
		g->heap[ i ].type - LAMPING_FAN_START, g->heap[ i ].out,
		g->heap[ i ].in, g->heap[ i ].la.arg );
      else
	printf( "type %5u, level   %8u, in      %8u, out     %8u\n",
		g->heap[ i ].type, g->heap[ i ].la.level, g->heap[ i ].in,
		g->heap[ i ].out );
    }
  }
}



int ruleSweep( LampingGraph* g, int (*rule)( LampingGraph*, u32 ), u32* ind ){
  for( u64 i = 0; i < g->heapsize; ++i ){
    if( g->heap[ i ].type ){
      if( rule( g, i ) ){
	*ind = i;
	return 1;
      }
    }
  }
  return 0;
}
static int (*rules[])( LampingGraph* g, u32 ind ) = {
  ruleOneA,
  ruleOneB,
  ruleTwoA,
  ruleTwoB,
  ruleTwoC,
  ruleThreeA,
  ruleThreeB,
  ruleFourA,
  ruleFourB,
  ruleFourC,
  ruleFourD,
  ruleFourE,
  ruleFiveAB,
  ruleSixA,
  ruleSixB,
  ruleSixCD,
  ruleSixEF,
  ruleSevenA,
  ruleSevenB,
  ruleSevenC,
  ruleSevenD,
  ruleSevenE,
  ruleSevenF,
  ruleSevenG,
  ruleSevenH,
  ruleSevenI,
  ruleSevenJ,
  ruleSevenKL,
  ruleSevenM,
  ruleSevenN,

  ruleEightA,
  ruleEightBCD,
  ruleEightE,
  ruleEightF,
  ruleEightGH,
  ruleEightIJKLMN,
  ruleEightO

};
const char* ruleNames[] = {
  "I.a",
  "I.b",
  "II.a",
  "II.b",
  "II.c",
  "III.a",
  "III.b",
  "IV.a",
  "IV.b",
  "IV.c",
  "IV.d",
  "IV.e",
  "V.ab",
  "VI.a",
  "VI.b",
  "VI.cd",
  "VI.ef",
  "VII.a",
  "VII.b",
  "VII.c",
  "VII.d",
  "VII.e",
  "VII.f",
  "VII.g",
  "VII.h",
  "VII.i",
  "VII.j",
  "VII.kl",
  "VII.m",
  "VII.n",

  "VIII.a",
  "VIII.bcd",
  "VIII.e",
  "VIII.f",
  "VIII.gh",
  "VIII.ijklmn",
  "VIII.o"
};

int rulesSweep( LampingGraph* g, u32* ind, u32* rule ){
  for( u64 i = 0; i < g->heapsize; ++i ){
    if( g->heap[ i ].type ){
      for( u64 j = 0; j < ( sizeof( rules ) / sizeof( rules[ 0 ] ) ); ++j )
	if( (rules[ j ])( g, i ) ){
	  *rule = j;
	  *ind = i;
	  return 1;
	}
    }
  }
  return 0;
}
int traceRulesSweep( LampingGraph* g ){
  u32 w, r;
  if( rulesSweep( g, &w, &r ) ){
    printf( "\nApplied rule %s at %u\n", ruleNames[ r ], w );
    return 1;
  }else{
    printf( "\nNo more simplifications.\n" );
    return 0;
  }
}


typedef struct pathContexti{
  int dir;
  struct pathContexti* next;
  struct pathContexti* closures;
} pathContext;
  

pathContext* newPathContext( void ){
  pathContext* ans = LNZcalloc( sizeof( pathContext ) );
  return ans;
}
pathContext* copyPathContext( const pathContext* cp ){
  pathContext* ans = LNZcalloc( sizeof( pathContext ) );
  ans->dir = cp->dir;
  if( cp->next != NULL )
    ans->next = copyPathContext( cp->next );
  else
    ans->next = NULL;
  if( cp->closures != NULL )
    ans->closures = copyPathContext( cp->closures );
  else
    ans->closures = NULL;
  return ans;
}
void deletePathContext( pathContext* d ){
  if( d->next != NULL )
    deletePathContext( d->next );
  if( d->closures != NULL )
    deletePathContext( d->closures );
  LNZfree( d );
}
void deletePathContextLevel( pathContext* d ){
  if( d->closures != NULL )
    deletePathContextLevel( d->closures );
  LNZfree( d );
}

u32 copyGraphToTree( LNZprogram* p, const LampingGraph* g, u32 ind, u32 from,
		     pathContext** pc ){
  if( g->heap[ ind ].type == LAMPING_ROOT_TYPE )
    return copyGraphToTree( p, g, g->heap[ ind ].out, ind, pc );
  else if( g->heap[ ind ].type == LAMPING_APPLICATION_TYPE ){
    pathContext* npc = copyPathContext( *pc );
    u32 nn = mallocNode( p );
    p->heap[ nn ].type = LNZ_APPLICATION_TYPE;
    u64 lo = copyGraphToTree( p, g, g->heap[ ind ].out, ind, pc );
    u64 hi = copyGraphToTree( p, g, g->heap[ ind ].la.arg, ind, &npc );
    deletePathContext( npc );
    hi <<= 32;
    p->heap[ nn ].data = lo + hi;
    p->heap[ nn ].references = 1;
    return nn;
  }else if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    u32 nn = mallocNode( p );
    p->heap[ nn ].type = LNZ_LAMBDA_TYPE;
    p->heap[ nn ].references = 1;
    addNamePointerPair( p, (const u8*)( &ind ), sizeof( u32 ), nn );
    u32 ans = copyGraphToTree( p, g, g->heap[ ind ].out, ind, pc );
    popNamePointerPair( p );
    p->heap[ nn ].data = ans;
    return nn;
  }else if( g->heap[ ind ].type == LAMPING_FREE_TYPE ){
    u32 nn = mallocNode( p );
    p->heap[ nn ].type = LNZ_FREE_TYPE;
    p->heap[ nn ].references = 1;
    u32 nm = getIndex( p->names, (const u8*)( &( g->heap[ ind ].la.arg ) ), 
		       sizeof( u32 ) );
    if( nm )
      p->heap[ nn ].data = *( (const u32*)( p->pointers->revdict[ nm - 1 ] ) );
    else
      LNZdie( "Huh?" );
    return nn;
  }else if( g->heap[ ind ].type >= LAMPING_FAN_START ){
    u32 lvl = g->heap[ ind ].type - LAMPING_FAN_START;
    pathContext* w = *pc;
    while( lvl-- ){
      if( w->next != NULL )
	w = w->next;
      else
	LNZdie( "What?" );
    }   
   
    if( from == g->heap[ ind ].in ){
      w->dir = 1;
      return copyGraphToTree( p, g, g->heap[ ind ].out, ind, pc );
    }else if( from == g->heap[ ind ].la.arg ){
      w->dir = 0;
      return copyGraphToTree( p, g, g->heap[ ind ].out, ind, pc );
    }else if( from == g->heap[ ind ].out ){
      if( w->dir )
	return copyGraphToTree( p, g, g->heap[ ind ].in, ind, pc );
      else
	return copyGraphToTree( p, g, g->heap[ ind ].la.arg, ind, pc );
    }else
      LNZdie( "Why?" );
  }else if( g->heap[ ind ].type == LAMPING_BRACKET_TYPE ||
	    g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE ){
    u32 lvl = g->heap[ ind ].la.arg;
    pathContext* w = *pc;
    pathContext** pcf = pc;
    while( lvl-- ){
      if( w->next != NULL ){
	pcf = &( w->next );
	w = w->next;
      }else
	LNZdie( "When?" );
    }   
   
    u32 os;
    if( from == g->heap[ ind ].in ){
#ifdef DEBUG
      if( pcf == pc && w->next == NULL )
	LNZdie( "Not possible!" );
#endif
      *pcf = w->next;
      if( w->closures != NULL )
	deletePathContext( w->closures );
      LNZfree( w );
      os = g->heap[ ind ].out;

    }else if( from == g->heap[ ind ].out ){
      *pcf = LNZmalloc( sizeof( pathContext ) );
      (*pcf)->dir = 0;
      (*pcf)->next = w;
      (*pcf)->closures = NULL;
      os = g->heap[ ind ].in;

    }else
      LNZdie( "How?" );
    return copyGraphToTree( p, g, os, ind, pc ); 
  }else if( g->heap[ ind ].type == LAMPING_CONDITIONAL_BRACKET_TYPE ){
    u32 lvl = g->heap[ ind ].la.arg;
    pathContext* w = *pc;
    pathContext** pcf = pc;
    while( lvl-- ){
      if( w->next != NULL ){
	pcf = &( w->next );
	w = w->next;
      }else
	LNZdie( "When?" );
    }   
   
    u32 os;
    if( from == g->heap[ ind ].in ){
#ifdef DEBUG
      if( pcf == pc && w->next == NULL )
	LNZdie( "Not possible2!" );
#endif
      *pcf = w->next;
      pathContext* o = (*pcf)->closures;
      (*pcf)->closures = w;
      w->next = w->closures;
      w->closures = o;
 
      os = g->heap[ ind ].out;
    }else if( from == g->heap[ ind ].out ){
#ifdef DEBUG
      if( w->closures == NULL )
	LNZdie( "No closure!" );
#endif
      *pcf = w->closures;
      pathContext* o = (*pcf)->closures;
      (*pcf)->closures = (*pcf)->next;
      (*pcf)->next = w;
      w->closures = o;
       
      /* w->closures = NULL;     */
      //(*pcf)->next = w;
      //w->closures = w->closures->closures;
      //(*pcf)->closures = NULL;
      os = g->heap[ ind ].in;
      
    }else
      LNZdie( "How2?" );
    return copyGraphToTree( p, g, os, ind, pc ); 
  }

  LNZdie( "Can't get here!" );
  return 0;
}


LNZprogram* makeProgramFromGraph( LampingGraph* g ){
  LNZprogram* ans = newProgram();
  pathContext* pc = newPathContext();
  u32 e = copyGraphToTree( ans, g, g->heap[ g->root ].out, g->root, &pc );
  addNamePointerPair( ans, (const u8*)"e", 1, e );
  deletePathContext( pc );
  return ans;
}
