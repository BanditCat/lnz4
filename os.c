////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

// OS dependant code goes in this file.
#define WIN32_LEAN_AND_MEAN

#include "lnz.h"

#include <windows.h>


void LNZdie( const char* msg ){
  MessageBoxA( NULL, (char*)msg, "Exiting LNZ...", 0 );
  exit( 0 );
}


u8* LNZLoadResource( const char* name, u64* size ){
  HRSRC rsc;
  HGLOBAL hnd;
  u8* dt,* ans;
  if( ( rsc = FindResource( NULL, name, RT_RCDATA ) ) == NULL )
    return NULL;
  if( ( hnd = LoadResource( NULL, rsc ) ) == NULL )
    return NULL;
  if( ( dt = LockResource( hnd ) ) == NULL )
    return NULL;
  *size = SizeofResource( NULL, rsc );
  ans = LNZmalloc( *size + 1 );
  memcpy( ans, dt, *size );
  ans[ *size ] = '\0';
  return ans;
}

u8* LNZLoadResourceOrDie( const char* name, u64* size ){
  u8* ans = LNZLoadResource( name, size );
  if( ans == NULL ){
    LNZdie( "Failed to load resource." );
  }
  return ans;
}  
