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

#ifndef __scheme_h
#define __scheme_h 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>

class appContextClass;

#include "color_pkg.h"
#include "utility.h"

#include "scheme.str"
#include "environment.str"

#define SCHEME_MAJOR_VERSION 2
#define SCHEME_MINOR_VERSION 0
#define SCHEME_RELEASE 0

class displaySchemeClass {

private:

appContextClass *appCtx;

int schemeLoaded;

int fg, bg, defaultTextFg, defaultFg1, defaultFg2, defaultBg,
 topShadow, botShadow, offset;

int defAlignment, defCtlAlignment, defBtnAlignment;

char defFontTag[127+1], defCtlFontTag[127+1], defBtnFontTag[127+1],
 defPvType[15+1];

public:

displaySchemeClass ( void );

~displaySchemeClass ( void );

void setAppCtx (
  appContextClass *_appCtx );

int loadDefault (
  colorInfoClass *ci );

int load (
  colorInfoClass *ci,
  char *fileName );

int save (
  colorInfoClass *ci,
  char *fileName );

int getFg ( void )
{
  return fg;
}

int getBg ( void )
{
  return bg;
}

int getDefTextFg ( void )
{
  return defaultTextFg;
}

int getDefFg1 ( void )
{
  return defaultFg1;
}

int getDefFg2 ( void )
{
  return defaultFg2;
}

int getDefBg ( void )
{
  return defaultBg;
}

int getOffset ( void )
{
  return offset;
}

int getTopShadow ( void )
{
  return topShadow;
}

int getBotShadow ( void )
{
  return botShadow;
}

char *getFont ( void )
{
  return defFontTag;
}

int getAlignment ( void )
{
  return defAlignment;
}

char *getCtlFont ( void )
{
  return defCtlFontTag;
}

int getCtlAlignment ( void )
{
  return defCtlAlignment;
}

char *getBtnFont ( void )
{
  return defBtnFontTag;
}

int getBtnAlignment ( void )
{
  return defBtnAlignment;
}

char *getPvType ( void )
{
  return defPvType;
}

void setFg ( int color )
{
  fg = color;
}

void setBg ( int color )
{
  bg = color;
}

void setDefTextFg ( int color )
{
  defaultTextFg = color;
}

void setDefFg1 ( int color )
{
  defaultFg1 = color;
}

void setDefFg2 ( int color )
{
  defaultFg2 = color;
}

void setDefBg ( int color )
{
  defaultBg = color;
}

void setOffset ( int color )
{
  offset = color;
}

void setTopShadow ( int color )
{
  topShadow = color;
}

void setBotShadow ( int color )
{
  botShadow = color;
}

void setFont ( char *font )
{
  strncpy( defFontTag, font, 127 );
}

void setCtlFont ( char *font )
{
  strncpy( defCtlFontTag, font, 127 );
}

void setBtnFont ( char *font )
{
  strncpy( defBtnFontTag, font, 127 );
}

void setAlignment ( int alignment )
{
  defAlignment = alignment;
}

void setCtlAlignment ( int alignment )
{
  defCtlAlignment = alignment;
}

void setBtnAlignment ( int alignment )
{
  defBtnAlignment = alignment;
}

void setPvType ( char *pvType )
{
  strncpy( defPvType, pvType, 15 );
}

int isLoaded ( void )
{
  return schemeLoaded;
}

};

#endif
