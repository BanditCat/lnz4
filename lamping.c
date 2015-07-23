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

// Makes the part of ind that points to whch point to new instead.
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
}


// This function points all ins upward at the parent, generating fan-ins where
// necessary.
void fanin( LampingGraph* g ){
  for( u64 i = 0; i < g->heapsize; ++i ){
    if( g->heap[ i ].type ){
      if( g->heap[ i ].type == LAMPING_APPLICATION_TYPE ){
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
	  g->heap[ fi ].in = i;
	  g->heap[ fi ].la.arg = op;
	  repoint( g, op, func, fi );
	}
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
	  g->heap[ fi ].in = i;
	  g->heap[ fi ].la.arg = op;
	  repoint( g, op, arg, fi );
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
	  g->heap[ fi ].in = i;
	  g->heap[ fi ].la.arg = op;
	  repoint( g, op, bdy, fi );
	}
      }else if( g->heap[ i ].type == LAMPING_ROOT_TYPE ){
	u32 bdy = g->heap[ i ].out;
	if( g->heap[ bdy ].in == bdy )
	  g->heap[ bdy ].in = i;
	else{
	  LNZdie( "Impossible!" );
	}
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
int betterBracketFreeVars( LampingGraph* g, u32 ind, nameTable* lambdas ){
  if( g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE )
    return betterBracketFreeVars( g, g->heap[ ind ].in, lambdas );
  else if( g->heap[ ind ].type == LAMPING_BRACKET_TYPE )
    return betterBracketFreeVars( g, g->heap[ ind ].out, lambdas );
  else if( g->heap[ ind ].type >= LAMPING_FAN_START ){
    u32 bot = g->heap[ ind ].out;
    while( g->heap[ bot ].type >= LAMPING_FAN_START )
      bot = g->heap[ bot ].out;
    if( g->heap[ bot ].type == LAMPING_FREE_TYPE ){
      if( getIndex( lambdas, (const u8*)( &( g->heap[ bot ].la.arg ) ), 
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
    } else
      return betterBracketFreeVars( g, g->heap[ ind ].out, lambdas );
  }
  else if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    addNameToTable( lambdas, (const u8*)( &ind ), sizeof( u32 ) );
    int ans = betterBracketFreeVars( g, g->heap[ ind ].out, lambdas );
    popNameTable( lambdas );
    return ans;
  }else if( g->heap[ ind ].type == LAMPING_APPLICATION_TYPE ){
    int ft = betterBracketFreeVars( g, g->heap[ ind ].out, lambdas );
    return betterBracketFreeVars( g, g->heap[ ind ].la.arg, lambdas ) || ft;
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
      if( betterBracketFreeVars( g, i, nt ) ){
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
	printf( "fan        , level   %8u, point   %8u, zero    %8u, one     %8u\n",
		g->heap[ i ].type - LAMPING_FAN_START, g->heap[ i ].out,
		g->heap[ i ].in, g->heap[ i ].la.arg );
      else
	printf( "type %5u, level   %8u, in      %8u, out     %8u\n",
		g->heap[ i ].type, g->heap[ i ].la.level, g->heap[ i ].in,
		g->heap[ i ].out );
    }
  }
}


// These all attempt to apply a rule at ind.

int ruleOneA( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    u32 appl = g->heap[ ind ].in;
    if( g->heap[ appl ].type == LAMPING_APPLICATION_TYPE &&
	g->heap[ appl ].out == ind ){
      u32 bdy = g->heap[ ind ].out;
      u32 va = g->heap[ appl ].in;
      repoint( g, bdy, ind, va );
      repoint( g, va, appl, bdy );
      u32 var = g->heap[ ind ].la.arg;
      u32 vc = g->heap[ var ].in;
      u32 vd = g->heap[ appl ].la.arg;
      repoint( g, vd, appl, vc );
      repoint( g, vc, var, vd );
      freeLampingNode( g, var );
      freeLampingNode( g, appl );
      freeLampingNode( g, ind );
      return 1;
    }
  }
  return 0;
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

