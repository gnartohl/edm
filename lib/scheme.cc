//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "scheme.h"

displaySchemeClass::displaySchemeClass ( void )
{

  schemeLoaded = 0;

  bg = 0;
  fg = 0;
  defaultTextFg = 0;
  defaultFg1 = 0;
  defaultFg2 = 0;
  defaultBg = 0;
  topShadow = 0;
  botShadow = 0;
  offset = 0;

  strcpy( defFontTag, "" );
  strcpy( defCtlFontTag, "" );
  strcpy( defBtnFontTag, "" );
  defAlignment = XmALIGNMENT_BEGINNING;
  defCtlAlignment = XmALIGNMENT_BEGINNING;
  defBtnAlignment = XmALIGNMENT_BEGINNING;
  strcpy( defPvType, "" );

}

displaySchemeClass::~displaySchemeClass ( void ) { }

int displaySchemeClass::loadDefault (
  colorInfoClass *ci )
{

char *envPtr;
char fileName[127+1], buf[127+1], *tk;
FILE *f;
int r, g, b, index;
int major, minor, release;

  envPtr = getenv( environment_str5 );
  if ( envPtr ) {
    strncpy( buf, envPtr, 127 );
    tk = strtok( buf, ":" );
    if ( tk ) {
      strncpy( fileName, tk, 127 );
      if ( fileName[strlen(fileName)-1] != '/' )
       strncat( fileName, "/", 127 );
    }
    else {
      strcpy( fileName, "" );
    }
  }
  else {
    strcpy( fileName, "" );
  }

  strncat( fileName, "default.scheme", 127 );

  f = fopen( fileName, "r" );
  if ( !f ) return 0;

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  readStringFromFile( defFontTag, 127, f );

  fscanf( f, "%d\n", &defAlignment );

  readStringFromFile( defCtlFontTag, 127, f );

  fscanf( f, "%d\n", &defCtlAlignment );

  if ( major > 1 ) {

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &fg );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &bg );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &defaultTextFg );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &defaultFg1 );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &defaultFg2 );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &defaultBg );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &topShadow );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &botShadow );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &offset );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &fg );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &bg );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &defaultTextFg );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &defaultFg1 );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &defaultFg2 );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &defaultBg );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &topShadow );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &botShadow );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &offset );

  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( defPvType, 15, f );
  }
  else {
    strcpy( defPvType, "epics" );
  }

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 2 ) ) ) {
    readStringFromFile( defBtnFontTag, 127, f );
    fscanf( f, "%d\n", &defBtnAlignment );
  }
  else {
    strcpy( defBtnFontTag, "" );
    defBtnAlignment = XmALIGNMENT_BEGINNING;
  }

  fclose( f );

  schemeLoaded = 1;

  return 1;

}

int displaySchemeClass::load (
  colorInfoClass *ci,
  char *fileName )
{

FILE *f;
int r, g, b, index;
int major, minor, release;

  f = fopen( fileName, "r" );
  if ( !f ) return 0;

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  readStringFromFile( defFontTag, 127, f );

  fscanf( f, "%d\n", &defAlignment );

  readStringFromFile( defCtlFontTag, 127, f );

  fscanf( f, "%d\n", &defCtlAlignment );

  if ( major > 1 ) {

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &fg );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &bg );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &defaultTextFg );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &defaultFg1 );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &defaultFg2 );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &defaultBg );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &topShadow );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &botShadow );

    fscanf( f, "%d\n", &index );
    ci->setIndex( index, &offset );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &fg );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &bg );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &defaultTextFg );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &defaultFg1 );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &defaultFg2 );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &defaultBg );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &topShadow );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &botShadow );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &offset );

  }

  if ( ( major > 1 ) || ( (major == 1 ) && ( minor > 1 ) ) ) {
    readStringFromFile( defPvType, 15, f );
  }
  else {
    strcpy( defPvType, "epics" );
  }

  if ( ( major > 1 ) || ( (major == 1 ) && ( minor > 2 ) ) ) {
    readStringFromFile( defBtnFontTag, 127, f );
    fscanf( f, "%d\n", &defBtnAlignment );
  }
  else {
    strcpy( defBtnFontTag, "" );
    defBtnAlignment = XmALIGNMENT_BEGINNING;
  }

  fclose( f );

  schemeLoaded = 1;

  return 1;

}

int displaySchemeClass::save (
  colorInfoClass *ci,
  char *fileName )
{

FILE *f;
int index;

  f = fopen( fileName, "w" );
  if ( !f ) return 0;

  fprintf( f, "%-d %-d %-d\n", SCHEME_MAJOR_VERSION, SCHEME_MINOR_VERSION,
   SCHEME_RELEASE );

  writeStringToFile( f, defFontTag );

  fprintf( f, "%-d\n", defAlignment );

  writeStringToFile( f, defCtlFontTag );

  fprintf( f, "%-d\n", defCtlAlignment );

  ci->getIndex( fg, &index );
  fprintf( f, "%-d\n", index );

  ci->getIndex( bg, &index );
  fprintf( f, "%-d\n", index );

  ci->getIndex( defaultTextFg, &index );
  fprintf( f, "%-d\n", index );

  ci->getIndex( defaultFg1, &index );
  fprintf( f, "%-d\n", index );

  ci->getIndex( defaultFg2, &index );
  fprintf( f, "%-d\n", index );

  ci->getIndex( defaultBg, &index );
  fprintf( f, "%-d\n", index );

  ci->getIndex( topShadow, &index );
  fprintf( f, "%-d\n", index );

  ci->getIndex( botShadow, &index );
  fprintf( f, "%-d\n", index );

  ci->getIndex( offset, &index );
  fprintf( f, "%-d\n", index );

  writeStringToFile( f, defPvType );

  writeStringToFile( f, defBtnFontTag );

  fprintf( f, "%-d\n", defBtnAlignment );

  fclose( f );

  return 1;

}


