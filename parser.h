////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// LNZ parser header.

#ifndef LNZ_PARSER_H
#define LNZ_PARSER_H

// This function returns 1 if c is a whitespace. NUL is considered whitespace. Returns 0 otherwise.
int isWhitespace( u8 c );

// This function returns 0 unless c is a reserved (punctuation) character.
int isReserved( u8 c );

// This function returns 0 unless c is a valid name character. UTF-8 compatible.
int isName( u8 c );

// Parses an expression or stores an error string in error. Returns the index of the
// node on the heap. On error, the error position is returned.
u32 parseExpression( LNZprogram* p, const u8* string, u64 length, const char** error );

// Returns number of characters parsed, on error this will point to the error.
u64 parseLine( LNZprogram* p, const u8* string, u64 length, const char** error );

// *error will be NULL on success, otherwise you must LNZfree it.
LNZprogram* parseProgram( const u8* string, u64 length, const char** error );

// Call with level set to 0. 
void printExpression( const LNZprogram* p, u32 index, u32 level );
void printProgram( const LNZprogram* p );


// Does at least count beta-reductions, returns the actual number of reductions.
void reduce( LNZprogram* p, u64 count, u32 expression );

#endif //LNZ_PARSER_H
