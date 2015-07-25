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
int ruleOneB( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    u32 rb = g->heap[ ind ].in;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].la.level == 0 ){
      u32 appl = g->heap[ rb ].out;
      if( g->heap[ appl ].type == LAMPING_APPLICATION_TYPE &&
	  g->heap[ appl ].out == rb ){
	u32 bdy = g->heap[ ind ].out;
	u32 va = g->heap[ appl ].in;
	repoint( g, bdy, ind, rb );
	repoint( g, rb, ind, bdy );
	repoint( g, va, appl, rb );
	repoint( g, rb, appl, va );
	u32 var = g->heap[ ind ].la.arg;
	u32 vd = g->heap[ appl ].la.arg;
	g->heap[ var ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
	g->heap[ var ].la.level = 0;
	g->heap[ var ].out = vd;
	repoint( g, vd, appl, var );
	freeLampingNode( g, appl );
	freeLampingNode( g, ind );
	return 1;
      }
    }
  }
  return 0;
}
int ruleTwoA( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    u32 cb = g->heap[ ind ].in;
    if( g->heap[ cb ].type == LAMPING_CONDITIONAL_BRACKET_TYPE &&
	g->heap[ cb ].out == ind ){
      u32 va = g->heap[ cb ].in;
      u32 vb = g->heap[ ind ].out;
      u32 var = g->heap[ ind ].la.arg;
      u32 vc = g->heap[ var ].in;
      u32 ncb = mallocLampingNode( g );
      g->heap[ ncb ].type = LAMPING_CONDITIONAL_BRACKET_TYPE;
      g->heap[ ncb ].la.level = g->heap[ cb ].la.level;
      g->heap[ ncb ].out = vc;
      g->heap[ ncb ].in = var;
      repoint( g, var, vc, ncb );
      repoint( g, vc, var, ncb );
      g->heap[ ind ].in = va;
      repoint( g, va, cb, ind );
      vb = g->heap[ ind ].out;
      g->heap[ ind ].out = cb;
      g->heap[ cb ].in = ind;
      g->heap[ cb ].out = vb;
      repoint( g, vb, ind, cb );
      return 1;
    }
  }
  return 0;
}
int ruleTwoB( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    u32 rb = g->heap[ ind ].in;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].in == ind ){
      u32 va = g->heap[ rb ].out;
      u32 vb = g->heap[ ind ].out;
      u32 var = g->heap[ ind ].la.arg;
      u32 vc = g->heap[ var ].in;
      u32 nrb = mallocLampingNode( g );
      g->heap[ nrb ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
      g->heap[ nrb ].la.level = g->heap[ rb ].la.level;
      g->heap[ nrb ].out = var;
      g->heap[ nrb ].in = vc;
      repoint( g, var, vc, nrb );
      repoint( g, vc, var, nrb );
      g->heap[ ind ].in = va;
      repoint( g, va, rb, ind );
      vb = g->heap[ ind ].out;
      g->heap[ ind ].out = rb;
      g->heap[ rb ].out = ind;
      g->heap[ rb ].in = vb;
      repoint( g, vb, ind, rb );
      return 1;
    }
  }
  return 0;
}
int ruleTwoC( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    u32 fi = g->heap[ ind ].in;
    if( g->heap[ fi ].type >= LAMPING_FAN_START &&
	g->heap[ fi ].out == ind ){
      u32 va = g->heap[ fi ].in;
      u32 vb = g->heap[ fi ].la.arg;
      u32 vc = g->heap[ ind ].out;
      u32 var = g->heap[ ind ].la.arg;
      u32 vd = g->heap[ var ].in;
      
      u32 nl = mallocLampingNode( g );
      u32 nv = mallocLampingNode( g );
      u32 nfo = mallocLampingNode( g );
      g->heap[ nl ].type = LAMPING_LAMBDA_TYPE;
      g->heap[ nv ].type = LAMPING_FREE_TYPE;
      g->heap[ nfo ].type = g->heap[ fi ].type;
      g->heap[ nl ].la.arg = nv;
      g->heap[ nv ].la.arg = nl;
      
      g->heap[ nfo ].out = vd;
      g->heap[ nfo ].in = var;
      g->heap[ nfo ].la.arg = nv;
      g->heap[ nv ].in = nfo;
      repoint( g, vd, var, nfo );

      g->heap[ nl ].out = fi;
      g->heap[ fi ].la.arg = nl;
      repoint( g, vb, fi, nl );
      g->heap[ nl ].in = vb;

      g->heap[ ind ].in = va;
      g->heap[ ind ].out = fi;
      g->heap[ fi ].out = vc;
      repoint( g, vc, ind, fi );
      g->heap[ fi ].in = ind;
      repoint( g, va, fi, ind );
      
      return 1;
    }
  }
  return 0;
}
int ruleThreeA( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    u32 b = g->heap[ ind ].in;
    if( g->heap[ b ].type == LAMPING_BRACKET_TYPE &&
	g->heap[ b ].out == ind &&
	g->heap[ b ].la.level == 0 ){
      g->heap[ b ].type = LAMPING_CONDITIONAL_BRACKET_TYPE;
      return 1;
    }
  }
  return 0;
} 
int ruleFourA( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_APPLICATION_TYPE ){
    u32 rb = g->heap[ ind ].out;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].in == ind ){
      u32 va = g->heap[ ind ].in;
      u32 vb = g->heap[ rb ].out;
      u32 vc = g->heap[ ind ].la.arg;
      g->heap[ ind ].in = rb;
      g->heap[ rb ].in = va;
      g->heap[ rb ].out = ind;
      repoint( g, va, ind, rb );
      g->heap[ ind ].out = vb;
      repoint( g, vb, rb, ind );
      u32 nrb = mallocLampingNode( g );
      g->heap[ nrb ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
      g->heap[ nrb ].la.level = g->heap[ rb ].la.level;
      g->heap[ nrb ].in = vc;
      repoint( g, vc, ind, nrb );
      g->heap[ nrb ].out = ind;
      g->heap[ ind ].la.arg = nrb;      
      return 1;
    }
  }
  return 0;
}
int ruleFourB( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_APPLICATION_TYPE ){
    u32 cb = g->heap[ ind ].out;
    if( g->heap[ cb ].type == LAMPING_CONDITIONAL_BRACKET_TYPE &&
	g->heap[ cb ].out == ind ){
      u32 va = g->heap[ ind ].in;
      u32 vb = g->heap[ cb ].in;
      u32 vc = g->heap[ ind ].la.arg;
      g->heap[ ind ].in = cb;
      g->heap[ cb ].out = va;
      g->heap[ cb ].in = ind;
      repoint( g, va, ind, cb );
      g->heap[ ind ].out = vb;
      repoint( g, vb, cb, ind );
      u32 ncb = mallocLampingNode( g );
      g->heap[ ncb ].type = LAMPING_CONDITIONAL_BRACKET_TYPE;
      g->heap[ ncb ].la.level = g->heap[ cb ].la.level;
      g->heap[ ncb ].out = vc;
      repoint( g, vc, ind, ncb );
      g->heap[ ncb ].in = ind;
      g->heap[ ind ].la.arg = ncb;      
      return 1;
    }
  }
  return 0;
}
int ruleFourC( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_APPLICATION_TYPE ){
    u32 rb = g->heap[ ind ].in;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].in == ind ){
      u32 va = g->heap[ rb ].out;
      u32 vb = g->heap[ ind ].out;
      u32 vc = g->heap[ ind ].la.arg;
      g->heap[ ind ].in = va;
      repoint( g, va, rb, ind );
      g->heap[ rb ].out = ind;
      g->heap[ rb ].in = vb;
      repoint( g, vb, ind, rb );
      g->heap[ ind ].out = rb;
      u32 nrb = mallocLampingNode( g );
      g->heap[ nrb ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
      g->heap[ nrb ].la.level = g->heap[ rb ].la.level;
      g->heap[ nrb ].in = vc;
      repoint( g, vc, ind, nrb );
      g->heap[ nrb ].out = ind;
      g->heap[ ind ].la.arg = nrb;      
      return 1;
    }
  }
  return 0;
}
int ruleFourD( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_APPLICATION_TYPE ){
    u32 cb = g->heap[ ind ].in;
    if( g->heap[ cb ].type == LAMPING_CONDITIONAL_BRACKET_TYPE &&
	g->heap[ cb ].out == ind ){
      u32 va = g->heap[ cb ].in;
      u32 vb = g->heap[ ind ].out;
      u32 vc = g->heap[ ind ].la.arg;
      g->heap[ ind ].in = va;
      repoint( g, va, cb, ind );
      g->heap[ cb ].in = ind;
      g->heap[ cb ].out = vb;
      repoint( g, vb, ind, cb );
      g->heap[ ind ].out = cb;
      u32 ncb = mallocLampingNode( g );
      g->heap[ ncb ].type = LAMPING_CONDITIONAL_BRACKET_TYPE;
      g->heap[ ncb ].la.level = g->heap[ cb ].la.level;
      g->heap[ ncb ].out = vc;
      repoint( g, vc, ind, ncb );
      g->heap[ ncb ].in = ind;
      g->heap[ ind ].la.arg = ncb;      
      return 1;
    }
  }
  return 0;
}
int ruleFourE( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_APPLICATION_TYPE ){
    u32 fo = g->heap[ ind ].out;
    if( g->heap[ fo ].type >= LAMPING_FAN_START &&
	g->heap[ fo ].out == ind ){
      u32 va = g->heap[ ind ].in;
      u32 vb = g->heap[ fo ].in;
      u32 vc = g->heap[ fo ].la.arg;
      u32 vd = g->heap[ ind ].la.arg;
      u32 nfi = mallocLampingNode( g );
      u32 nappl = mallocLampingNode( g );
      g->heap[ nfi ].type = g->heap[ fo ].type;
      g->heap[ nappl ].type = LAMPING_APPLICATION_TYPE;
      
      g->heap[ nfi ].out = vd;
      repoint( g, vd, ind, nfi );
      g->heap[ nfi ].la.arg = nappl;
      g->heap[ nappl ].la.arg = nfi;
      g->heap[ nfi ].in = ind;
      g->heap[ ind ].la.arg = nfi;
      
      g->heap[ nappl ].in = fo;
      g->heap[ fo ].la.arg = nappl;
      g->heap[ nappl ].out = vc;
      repoint( g, vc, fo, nappl );
      
      g->heap[ fo ].out = va;
      repoint( g, va, ind, fo );
      g->heap[ fo ].in = ind;
      g->heap[ ind ].in = fo;
      
      g->heap[ ind ].out = vb;
      repoint( g, vb, fo, ind );
      
      return 1;
    }
  }
  return 0;
}
int ruleFiveAB( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type >= LAMPING_FAN_START ){
    u32 fo = g->heap[ ind ].out;
    if( g->heap[ fo ].type >= LAMPING_FAN_START &&
	g->heap[ fo ].out == ind ){
      u32 va = g->heap[ ind ].in;
      u32 vb = g->heap[ ind ].la.arg;
      u32 vc = g->heap[ fo ].in;
      u32 vd = g->heap[ fo ].la.arg;
      if( g->heap[ fo ].type == g->heap[ ind ].type ){
	freeLampingNode( g, ind );
	freeLampingNode( g, fo );
	repoint( g, va, ind, vc );
	repoint( g, vc, fo, va );
	repoint( g, vb, ind, vd );
	repoint( g, vd, fo, vb );
      } else{
	u32 nfi = mallocLampingNode( g );
	u32 nfo = mallocLampingNode( g );
	g->heap[ nfi ].type = g->heap[ ind ].type;
	g->heap[ nfo ].type = g->heap[ fo ].type;

	g->heap[ fo ].out = va;
	repoint( g, va, ind, fo );
	g->heap[ fo ].in = ind;
	g->heap[ ind ].in = fo;
	g->heap[ fo ].la.arg = nfi;
	g->heap[ nfi ].in = fo;
	
	g->heap[ ind ].out = vc;
	repoint( g, vc, fo, ind );
	g->heap[ ind ].la.arg = nfo;
	g->heap[ nfo ].in = ind;

	g->heap[ nfo ].out = vb;
	repoint( g, vb, ind, nfo );
	g->heap[ nfo ].la.arg = nfi;
	g->heap[ nfi ].la.arg = nfo;

	g->heap[ nfi ].out = vd;
	repoint( g, vd, fo, nfi );

      }
      return 1;
    }
  }
  return 0;
}
int ruleSixA( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type >= LAMPING_FAN_START ){
    u32 b = g->heap[ ind ].out;
    if( g->heap[ b ].type == LAMPING_BRACKET_TYPE &&
	g->heap[ b ].out == ind ){
    }
    u32 va = g->heap[ b ].in;
    u32 vb = g->heap[ ind ].in;
    u32 vc = g->heap[ ind ].la.arg;
    u32 nb = mallocLampingNode( g );
    g->heap[ nb ].type = LAMPING_BRACKET_TYPE;
    g->heap[ nb ].la.level = 0;
    g->heap[ nb ].out = vc;
    repoint( g, vc, ind, nb );
    g->heap[ nb ].in = ind;
    g->heap[ ind ].la.arg = nb;
   
    g->heap[ ind ].out = va;
    repoint( g, va, b, ind );
    g->heap[ ind ].in = b;
    g->heap[ b ].in = ind;
    g->heap[ ind ].type += 1;

    g->heap[ b ].out = vb;
    repoint( g, vb, ind, b );
    
    return 1;
  }
  return 0;
}
int ruleSixB( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type >= LAMPING_FAN_START ){
    u32 rb = g->heap[ ind ].out;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].out == ind &&
	g->heap[ rb ].la.level == 0 ){
    }
    u32 va = g->heap[ ind ].in;
    u32 vb = g->heap[ ind ].la.arg;
    u32 vc = g->heap[ rb ].in;
    u32 nrb = mallocLampingNode( g );
    g->heap[ nrb ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
    g->heap[ nrb ].la.level = 0;
    g->heap[ nrb ].out = vb;
    repoint( g, vb, ind, nrb );
    g->heap[ nrb ].in = ind;
    g->heap[ ind ].la.arg = nrb;
   
    g->heap[ ind ].out = vc;
    repoint( g, vc, rb, ind );
    g->heap[ ind ].in = rb;
    g->heap[ rb ].in = ind;
    g->heap[ ind ].type += 1;

    g->heap[ rb ].out = va;
    repoint( g, va, ind, rb );
    
    return 1;
  }
  return 0;
}
int ruleSixCD( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type >= LAMPING_FAN_START ){
    u32 rb = g->heap[ ind ].out;
    u32 lvl = g->heap[ ind ].type - LAMPING_FAN_START;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].in == ind &&
	g->heap[ rb ].la.level != lvl ){
    }
    u32 va = g->heap[ ind ].in;
    u32 vb = g->heap[ ind ].la.arg;
    u32 vc = g->heap[ rb ].out;
    u32 nrb = mallocLampingNode( g );
    g->heap[ nrb ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
    g->heap[ nrb ].la.level = g->heap[ rb ].la.level;
    g->heap[ nrb ].in = vb;
    repoint( g, vb, ind, nrb );
    g->heap[ nrb ].out = ind;
    g->heap[ ind ].la.arg = nrb;
   
    g->heap[ ind ].out = vc;
    repoint( g, vc, rb, ind );
    g->heap[ ind ].in = rb;
    g->heap[ rb ].out = ind;
    if( lvl > g->heap[ rb ].la.level )
      g->heap[ ind ].type -= 1;

    g->heap[ rb ].in = va;
    repoint( g, va, ind, rb );
    
    return 1;
  }
  return 0;
}
int ruleSixEF( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type >= LAMPING_FAN_START ){
    u32 cb = g->heap[ ind ].out;
    u32 lvl = g->heap[ ind ].type - LAMPING_FAN_START;
    if( g->heap[ cb ].type == LAMPING_CONDITIONAL_BRACKET_TYPE &&
	g->heap[ cb ].out == ind &&
	g->heap[ cb ].la.level != lvl ){
      u32 va = g->heap[ ind ].in;
      u32 vb = g->heap[ ind ].la.arg;
      u32 vc = g->heap[ cb ].in;
      u32 ncb = mallocLampingNode( g );
      g->heap[ ncb ].type = LAMPING_CONDITIONAL_BRACKET_TYPE;
      g->heap[ ncb ].la.level = g->heap[ cb ].la.level;
      g->heap[ ncb ].out = vb;
      repoint( g, vb, ind, ncb );
      g->heap[ ncb ].in = ind;
      g->heap[ ind ].la.arg = ncb;
      
      g->heap[ ind ].out = vc;
      repoint( g, vc, cb, ind );
      g->heap[ ind ].in = cb;
      g->heap[ cb ].in = ind;
      if( lvl > g->heap[ cb ].la.level )
	g->heap[ ind ].type += 1;
      
      g->heap[ cb ].out = va;
      repoint( g, va, ind, cb );
      
      return 1;
    }
  }
  return 0;
}
int ruleSevenA( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE ){
    u32 rb = g->heap[ ind ].in;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].in == ind &&
	g->heap[ rb ].la.level == g->heap[ ind ].la.level ){
      u32 va = g->heap[ ind ].out;
      u32 vb = g->heap[ rb ].out;
      repoint( g, va, ind, vb );
      repoint( g, vb, rb, va );
      freeLampingNode( g, ind );
      freeLampingNode( g, rb );
      return 1;
    }    
  }
  return 0;
}
int ruleSevenB( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE ){
    u32 b = g->heap[ ind ].in;
    if( g->heap[ b ].type == LAMPING_BRACKET_TYPE &&
	g->heap[ b ].in == ind &&
	g->heap[ b ].la.level == g->heap[ ind ].la.level ){
      u32 va = g->heap[ ind ].out;
      u32 vb = g->heap[ b ].out;
      repoint( g, va, ind, vb );
      repoint( g, vb, b, va );
      freeLampingNode( g, ind );
      freeLampingNode( g, b );
      return 1;
    }    
  }
  return 0;
}
int ruleSevenC( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_CONDITIONAL_BRACKET_TYPE ){
    u32 cb = g->heap[ ind ].out;
    if( g->heap[ cb ].type == LAMPING_CONDITIONAL_BRACKET_TYPE &&
	g->heap[ cb ].out == ind &&
	g->heap[ cb ].la.level == g->heap[ ind ].la.level ){
      u32 va = g->heap[ ind ].in;
      u32 vb = g->heap[ cb ].in;
      repoint( g, va, ind, vb );
      repoint( g, vb, cb, va );
      freeLampingNode( g, ind );
      freeLampingNode( g, cb );
      return 1;
    }    
  }
  return 0;
}
int ruleSevenD( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_CONDITIONAL_BRACKET_TYPE ){
    u32 b = g->heap[ ind ].out;
    if( g->heap[ b ].type == LAMPING_BRACKET_TYPE &&
	g->heap[ b ].in == ind &&
	g->heap[ b ].la.level == g->heap[ ind ].la.level ){
      g->heap[ ind ].type = LAMPING_BRACKET_TYPE;
      return 1;
    }    
  }
  return 0;
}
int ruleSevenE( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_BRACKET_TYPE ){
    u32 rb = g->heap[ ind ].out;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].in == ind ){
      g->heap[ ind ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
      g->heap[ ind ].la.level = g->heap[ rb ].la.level + 1;
      g->heap[ rb ].type = LAMPING_BRACKET_TYPE;
      g->heap[ rb ].la.level = 0;
      return 1;
    }    
  }
  return 0;
}
int ruleSevenF( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_BRACKET_TYPE ){
    u32 cb = g->heap[ ind ].out;
    if( g->heap[ cb ].type == LAMPING_CONDITIONAL_BRACKET_TYPE &&
	g->heap[ cb ].out == ind ){
      g->heap[ ind ].type = LAMPING_CONDITIONAL_BRACKET_TYPE;
      g->heap[ ind ].la.level = g->heap[ cb ].la.level + 1;
      g->heap[ cb ].type = LAMPING_BRACKET_TYPE;
      g->heap[ cb ].la.level = 0;
      u32 t = g->heap[ ind ].out;
      g->heap[ ind ].out = g->heap[ ind ].in;
      g->heap[ ind ].in = t;
      t = g->heap[ cb ].out;
      g->heap[ cb ].out = g->heap[ cb ].in;
      g->heap[ cb ].in = t;
      return 1;
    }    
  }
  return 0;
}
int ruleSevenG( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE ){
    u32 rb = g->heap[ ind ].out;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].in == ind &&
	g->heap[ ind ].la.level == 0 ){
      g->heap[ ind ].la.level = g->heap[ rb ].la.level + 1;
      g->heap[ rb ].la.level = 0;
      return 1;
    }    
  }
  return 0;
}
int ruleSevenH( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE ){
    u32 cb = g->heap[ ind ].out;
    if( g->heap[ cb ].type == LAMPING_CONDITIONAL_BRACKET_TYPE &&
	g->heap[ cb ].out == ind &&
	g->heap[ ind ].la.level == 0 ){
      g->heap[ ind ].type = LAMPING_CONDITIONAL_BRACKET_TYPE;
      g->heap[ ind ].la.level = g->heap[ cb ].la.level + 1;
      g->heap[ cb ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
      g->heap[ cb ].la.level = 0;
      u32 t = g->heap[ ind ].out;
      g->heap[ ind ].out = g->heap[ ind ].in;
      g->heap[ ind ].in = t;
      t = g->heap[ cb ].out;
      g->heap[ cb ].out = g->heap[ cb ].in;
      g->heap[ cb ].in = t;
      return 1;
    }    
  }
  return 0;
}
int ruleSevenI( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE ){
    u32 b = g->heap[ ind ].in;
    if( g->heap[ b ].type == LAMPING_BRACKET_TYPE &&
	g->heap[ b ].in == ind &&
	g->heap[ ind ].la.level > 0 ){
      g->heap[ ind ].type = LAMPING_BRACKET_TYPE;
      g->heap[ b ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
      g->heap[ b ].la.level = g->heap[ ind ].la.level - 1;
      g->heap[ ind ].la.level = 0;
      u32 t = g->heap[ ind ].out;
      g->heap[ ind ].out = g->heap[ ind ].in;
      g->heap[ ind ].in = t;
      t = g->heap[ b ].out;
      g->heap[ b ].out = g->heap[ b ].in;
      g->heap[ b ].in = t;
      return 1;
    }    
  }
  return 0;
}

int ruleSevenJ( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_CONDITIONAL_BRACKET_TYPE ){
    u32 b = g->heap[ ind ].out;
    if( g->heap[ b ].type == LAMPING_BRACKET_TYPE &&
	g->heap[ b ].in == ind &&
	g->heap[ ind ].la.level > 0 ){
      g->heap[ ind ].type = LAMPING_BRACKET_TYPE;
      g->heap[ b ].type = LAMPING_CONDITIONAL_BRACKET_TYPE;
      g->heap[ b ].la.level = g->heap[ ind ].la.level - 1;
      g->heap[ ind ].la.level = 0;
    }    
  }
  return 0;
}
int ruleSevenKL( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE ){
    u32 cb = g->heap[ ind ].in;
    if( g->heap[ cb ].type == LAMPING_CONDITIONAL_BRACKET_TYPE &&
	g->heap[ cb ].out == ind &&
	g->heap[ ind ].la.level != g->heap[ cb ].la.level ){
      u32 clvl = g->heap[ cb ].la.level;
      u32 rlvl = g->heap[ ind ].la.level; 
      g->heap[ ind ].type = LAMPING_CONDITIONAL_BRACKET_TYPE;
      g->heap[ cb ].type = LAMPING_RESTRICTED_BRACKET_TYPE;
      if( rlvl > clvl ){
	g->heap[ cb ].la.level = rlvl + 1;
	g->heap[ ind ].la.level = clvl;
      }else{
	g->heap[ cb ].la.level = rlvl;
	g->heap[ ind ].la.level = clvl - 1;
      }
    }    
  }
  return 0;
}
int ruleSevenM( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_RESTRICTED_BRACKET_TYPE ){
    u32 rb = g->heap[ ind ].in;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].in == ind &&
	g->heap[ ind ].la.level > g->heap[ rb ].la.level ){
      u32 t = g->heap[ ind ].la.level;
      g->heap[ ind ].la.level = g->heap[ rb ].la.level;
      g->heap[ rb ].la.level = t - 1;
      t = g->heap[ ind ].in;
      g->heap[ ind ].in = g->heap[ rb ].in;
      g->heap[ rb ].in = t;
      t = g->heap[ ind ].out;
      g->heap[ ind ].out = g->heap[ rb ].out;
      g->heap[ rb ].out = t;

      return 1;
    }    
  }
  return 0;
}
int ruleSevenN( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_CONDITIONAL_BRACKET_TYPE ){
    u32 cb = g->heap[ ind ].out;
    if( g->heap[ cb ].type == LAMPING_CONDITIONAL_BRACKET_TYPE &&
	g->heap[ cb ].out == ind &&
	g->heap[ ind ].la.level > g->heap[ cb ].la.level ){
      u32 t = g->heap[ ind ].la.level;
      g->heap[ ind ].la.level = g->heap[ cb ].la.level;
      g->heap[ cb ].la.level = t + 1;
      t = g->heap[ ind ].in;
      g->heap[ ind ].in = g->heap[ cb ].in;
      g->heap[ cb ].in = t;
      t = g->heap[ ind ].out;
      g->heap[ ind ].out = g->heap[ cb ].out;
      g->heap[ cb ].out = t;

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

