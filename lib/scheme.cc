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
#include "app_pkg.h"

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

  appCtx = NULL;
}

displaySchemeClass::~displaySchemeClass ( void ) { }

void displaySchemeClass::setAppCtx (
  appContextClass *_appCtx )
{

  appCtx = _appCtx;

}

int displaySchemeClass::loadDefault (
  colorInfoClass *ci )
{

char buf[127+1];
FILE *f;
int r, g, b;
int major, minor, release;
unsigned int pixel;

  if ( appCtx ) {

    strncpy( buf, appCtx->colorPath, 127 );
    Strncat( buf, "default.scheme", 127 );
    f = fopen( buf, "r" );
    if ( !f ) return 0;

  }
  else {

    f = fopen( "./default.scheme", "r" );
    if ( !f ) return 0;

  }

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  readStringFromFile( defFontTag, 127, f );

  fscanf( f, "%d\n", &defAlignment );

  readStringFromFile( defCtlFontTag, 127, f );

  fscanf( f, "%d\n", &defCtlAlignment );

  if ( major > 1 ) {

    fscanf( f, "%d\n", &fg );

    fscanf( f, "%d\n", &bg );

    fscanf( f, "%d\n", &defaultTextFg );

    fscanf( f, "%d\n", &defaultFg1 );

    fscanf( f, "%d\n", &defaultFg2 );

    fscanf( f, "%d\n", &defaultBg );

    fscanf( f, "%d\n", &topShadow );

    fscanf( f, "%d\n", &botShadow );

    fscanf( f, "%d\n", &offset );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    fg = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    bg = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultTextFg = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultFg1 = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultFg2 = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultBg = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    topShadow = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    botShadow = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    offset = ci->pixIndex( pixel );

  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( defPvType, 15, f );
  }
  else {
    strcpy( defPvType, "" );
  }

  if ( strcmp( defPvType, "epics" ) == 0 ) {
    strcpy( defPvType, "EPICS" );
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
int r, g, b;
int major, minor, release;
unsigned int pixel;

  f = fopen( fileName, "r" );
  if ( !f ) return 0;

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  readStringFromFile( defFontTag, 127, f );

  fscanf( f, "%d\n", &defAlignment );

  readStringFromFile( defCtlFontTag, 127, f );

  fscanf( f, "%d\n", &defCtlAlignment );

  if ( major > 1 ) {

    fscanf( f, "%d\n", &fg );

    fscanf( f, "%d\n", &bg );

    fscanf( f, "%d\n", &defaultTextFg );

    fscanf( f, "%d\n", &defaultFg1 );

    fscanf( f, "%d\n", &defaultFg2 );

    fscanf( f, "%d\n", &defaultBg );

    fscanf( f, "%d\n", &topShadow );

    fscanf( f, "%d\n", &botShadow );

    fscanf( f, "%d\n", &offset );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    fg = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    bg = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultTextFg = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultFg1 = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultFg2 = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultBg = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    topShadow = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    botShadow = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    offset = ci->pixIndex( pixel );

  }

  if ( ( major > 1 ) || ( (major == 1 ) && ( minor > 1 ) ) ) {
    readStringFromFile( defPvType, 15, f );
  }
  else {
    strcpy( defPvType, "" );
  }

  if ( strcmp( defPvType, "epics" ) == 0 ) {
    strcpy( defPvType, "EPICS" );
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

  index = fg;
  fprintf( f, "%-d\n", index );

  index = bg;
  fprintf( f, "%-d\n", index );

  index = defaultTextFg;
  fprintf( f, "%-d\n", index );

  index = defaultFg1;
  fprintf( f, "%-d\n", index );

  index = defaultFg2;
  fprintf( f, "%-d\n", index );

  index = defaultBg;
  fprintf( f, "%-d\n", index );

  index = topShadow;
  fprintf( f, "%-d\n", index );

  index = botShadow;
  fprintf( f, "%-d\n", index );

  index = offset;
  fprintf( f, "%-d\n", index );

  writeStringToFile( f, defPvType );

  writeStringToFile( f, defBtnFontTag );

  fprintf( f, "%-d\n", defBtnAlignment );

  fclose( f );

  return 1;

}


