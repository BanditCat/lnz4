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
// node on the heap.
u32 parseExpression( const u8* string, u64 length, const char** error, LNZprogram* p );
void printExpression( u32 index, const LNZprogram* p );


#endif //LNZ_PARSER_H
