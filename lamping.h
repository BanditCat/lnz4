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


// Creates a graph from a tree.
LampingGraph* makeGraph( LNZprogram* p, u32 ind );

void printLampingGraph( const LampingGraph* g );

// Sweeps the graph attempting to apply rule. Returns 1 and sets ind if successful.
int ruleSweep( LampingGraph* g, int (*rule)( LampingGraph*, u32 ), u32* ind );

int ruleOneA( LampingGraph* g, u32 ind );
int ruleOneB( LampingGraph* g, u32 ind );
int ruleTwoA( LampingGraph* g, u32 ind );
int ruleTwoB( LampingGraph* g, u32 ind );
int ruleTwoC( LampingGraph* g, u32 ind );
int ruleThreeA( LampingGraph* g, u32 ind );
int ruleThreeB( LampingGraph* g, u32 ind );
int ruleFourA( LampingGraph* g, u32 ind );
int ruleFourB( LampingGraph* g, u32 ind );
int ruleFourC( LampingGraph* g, u32 ind );
int ruleFourD( LampingGraph* g, u32 ind );
int ruleFourE( LampingGraph* g, u32 ind );
int ruleFiveAB( LampingGraph* g, u32 ind );
int ruleSixA( LampingGraph* g, u32 ind );
int ruleSixB( LampingGraph* g, u32 ind );
int ruleSixCD( LampingGraph* g, u32 ind );
int ruleSixEF( LampingGraph* g, u32 ind );
