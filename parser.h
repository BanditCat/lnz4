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

// returns the index of a command in a nametable or 0 if not a valid command string.
LNZnode* parseExpression( const u8* string, u64 length, const char** error );
void printExpression( const LNZnode* expression );


#endif //LNZ_PARSER_H
