////////////////////////////////////////////////////////////////////////////////
// This is a public domain implementation of Lamping's optimal beta reduction //
// algorithm written by Jon DuBois in 2015.                                   //
////////////////////////////////////////////////////////////////////////////////


// la.arg points to the free variable.
#define LAMPING_LAMBDA_TYPE 1
// out is the function, la.arg is the argument.
#define LAMPING_APPLICATION_TYPE 2
// la.arg is the lambda containing this variable, out is unused.
#define LAMPING_FREE_TYPE 3

// Only in and out are used.
#define LAMPING_BRACKET_TYPE 4
#define LAMPING_CONDITIONAL_BRACKET_TYPE 5
#define LAMPING_RESTRICTED_BRACKET_TYPE 6

// Only out is used.
#define LAMPING_ROOT_TYPE 7
#define LAMPING_VOID_TYPE 8

// Fan level is type - LAMPING_FAN_START out is the point, in is 0 and la.arg is 1
#define LAMPING_FAN_START 1024


typedef struct{
  u32 type;
  union{
    u32 level;
    u32 arg;
  } la;
  u32 in;
  u32 out;
} LampingNode;


typedef struct{
  LampingNode* heap;
  u32 heapsize;
  u32 frees;
  u32 root;
} LampingGraph;

LampingGraph* newLampingGraph( void );
void deleteLampingGraph( LampingGraph* g );
u32 mallocLampingNode( LampingGraph* g );
void freeLampingNode( LampingGraph* g, u32 node );



typedef struct pathContexti{
  stack* dirs;
  struct pathContexti* next;
  struct pathContexti* closures;
} pathContext;

// Traverses the graph in normal order. func is called with the graph, the index,
// the node in the graph we came from, an
// opaque data pointer and the path context. This is a top down traversal. If func
// returns non zero, this function stops. Returns 0 iff it traversed the whole tree
// without stopping.
int traverseGraph( LampingGraph* g, u32 ind, u32 from, void* data, pathContext** pc,
		    int (*func)( LampingGraph* g, u32 i, u32 f, void* d, 
				 pathContext ** ) );

// Creates a graph from a tree.
LampingGraph* makeGraph( LNZprogram* p, u32 ind );
// Creates a tree from a graph, the nametable is empty except for e which is
// the translated expression.
LNZprogram* makeProgramFromGraph( LampingGraph* g );

void printLampingGraph( const LampingGraph* g );

// Sweeps the graph attempting to apply rule. Returns 1 and sets ind if successful.
int ruleSweep( LampingGraph* g, int (*rule)( LampingGraph*, u32 ), u32* ind );

// Makes the part of ind that points to whch point to new instead.
void repoint( LampingGraph* g, u32 ind, u32 whch, u32 new );

int rulesSweep( LampingGraph* g, u32* ind, u32* rule );
int rulesSweepNormalOrder( LampingGraph* g, u32* ind, u32* rule );
int traceRulesSweep( LampingGraph* g );



void validateGraph( LampingGraph* g );
