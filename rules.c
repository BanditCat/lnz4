////////////////////////////////////////////////////////////////////////////////
// This is a public domain implementation of Lamping's optimal beta reduction //
// algorithm written by Jon DuBois in 2015.                                   //
////////////////////////////////////////////////////////////////////////////////


#include "lnz.h"

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
      g->heap[ ncb ].in = var;
 

      g->heap[ ncb ].out = vc;
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
      
      u32 nl = mallocLampingNode( g );
      u32 nv = mallocLampingNode( g );
      u32 nfo = mallocLampingNode( g );
      g->heap[ nl ].type = LAMPING_LAMBDA_TYPE;
      g->heap[ nv ].type = LAMPING_FREE_TYPE;
      g->heap[ nfo ].type = g->heap[ fi ].type;
      g->heap[ nl ].la.arg = nv;
      g->heap[ nv ].la.arg = nl;
 
      g->heap[ ind ].in = va;
      g->heap[ ind ].out = fi;
      g->heap[ fi ].out = vc;
      repoint( g, vc, ind, fi );
      g->heap[ fi ].in = ind;
      repoint( g, va, fi, ind );

      g->heap[ nl ].out = fi;
      g->heap[ fi ].la.arg = nl;
      repoint( g, vb, fi, nl );
      g->heap[ nl ].in = vb;
      
      u32 vd = g->heap[ var ].in;

      g->heap[ nfo ].out = vd;
      g->heap[ nfo ].in = var;
      g->heap[ var ].in = nfo;
      g->heap[ nfo ].la.arg = nv;
      g->heap[ nv ].in = nfo;
      repoint( g, vd, var, nfo );


      
      return 1;
    }
  }
  return 0;
}
int ruleThreeA( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    u32 b = g->heap[ ind ].in;
    if( g->heap[ b ].type == LAMPING_BRACKET_TYPE &&
	g->heap[ b ].out == ind ){
      g->heap[ b ].type = LAMPING_CONDITIONAL_BRACKET_TYPE;
      return 1;
    }
  }
  return 0;
}
int ruleThreeB( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type == LAMPING_LAMBDA_TYPE ){
    u32 rb = g->heap[ ind ].in;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].in == ind &&
	g->heap[ rb ].la.level == 0 ){
      u32 b = g->heap[ rb ].out;
      if( g->heap[ b ].type == LAMPING_BRACKET_TYPE &&
	  g->heap[ b ].out == rb ){
	u32 va = g->heap[ b ].in;
	freeLampingNode( g, rb );
	freeLampingNode( g, b );
	g->heap[ ind ].in = va;
	repoint( g, va, b, ind );
	return 1;
      }
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
  }
  return 0;
}
int ruleSixB( LampingGraph* g, u32 ind ){
  if( g->heap[ ind ].type >= LAMPING_FAN_START ){
    u32 rb = g->heap[ ind ].out;
    if( g->heap[ rb ].type == LAMPING_RESTRICTED_BRACKET_TYPE &&
	g->heap[ rb ].out == ind &&
	g->heap[ rb ].la.level == 0 ){
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
      t = g->heap[ ind ].out;
      g->heap[ ind ].out = g->heap[ ind ].in;
      g->heap[ ind ].in = t;
      t = g->heap[ rb ].out;
      g->heap[ rb ].out = g->heap[ rb ].in;
      g->heap[ rb ].in = t;

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
      t = g->heap[ ind ].out;
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
