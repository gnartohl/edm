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

#include "color_pkg.h"
#include "utility.h"

#include "scheme.str"
#include "environment.str"

#define SCHEME_MAJOR_VERSION 2
#define SCHEME_MINOR_VERSION 0
#define SCHEME_RELEASE 0

class displaySchemeClass {

private:

int schemeLoaded;

unsigned int fg, bg, defaultTextFg, defaultFg1, defaultFg2, defaultBg,
 topShadow, botShadow, offset;

int defAlignment, defCtlAlignment, defBtnAlignment;

char defFontTag[127+1], defCtlFontTag[127+1], defBtnFontTag[127+1],
 defPvType[15+1];

public:

displaySchemeClass::displaySchemeClass ( void );

displaySchemeClass::~displaySchemeClass ( void );

int displaySchemeClass::loadDefault (
  colorInfoClass *ci );

int displaySchemeClass::load (
  colorInfoClass *ci,
  char *fileName );

int displaySchemeClass::save (
  colorInfoClass *ci,
  char *fileName );

unsigned int displaySchemeClass::getFg ( void )
{
  return fg;
}

unsigned int displaySchemeClass::getBg ( void )
{
  return bg;
}

unsigned int displaySchemeClass::getDefTextFg ( void )
{
  return defaultTextFg;
}

unsigned int displaySchemeClass::getDefFg1 ( void )
{
  return defaultFg1;
}

unsigned int displaySchemeClass::getDefFg2 ( void )
{
  return defaultFg2;
}

unsigned int displaySchemeClass::getDefBg ( void )
{
  return defaultBg;
}

unsigned int displaySchemeClass::getOffset ( void )
{
  return offset;
}

unsigned int displaySchemeClass::getTopShadow ( void )
{
  return topShadow;
}

unsigned int displaySchemeClass::getBotShadow ( void )
{
  return botShadow;
}

char *displaySchemeClass::getFont ( void )
{
  return defFontTag;
}

int displaySchemeClass::getAlignment ( void )
{
  return defAlignment;
}

char *displaySchemeClass::getCtlFont ( void )
{
  return defCtlFontTag;
}

int displaySchemeClass::getCtlAlignment ( void )
{
  return defCtlAlignment;
}

char *displaySchemeClass::getBtnFont ( void )
{
  return defBtnFontTag;
}

int displaySchemeClass::getBtnAlignment ( void )
{
  return defBtnAlignment;
}

char *displaySchemeClass::getPvType ( void )
{
  return defPvType;
}

void displaySchemeClass::setFg ( unsigned int color )
{
  fg = color;
}

void displaySchemeClass::setBg ( unsigned int color )
{
  bg = color;
}

void displaySchemeClass::setDefTextFg ( unsigned int color )
{
  defaultTextFg = color;
}

void displaySchemeClass::setDefFg1 ( unsigned int color )
{
  defaultFg1 = color;
}

void displaySchemeClass::setDefFg2 ( unsigned int color )
{
  defaultFg2 = color;
}

void displaySchemeClass::setDefBg ( unsigned int color )
{
  defaultBg = color;
}

void displaySchemeClass::setOffset ( unsigned int color )
{
  offset = color;
}

void displaySchemeClass::setTopShadow ( unsigned int color )
{
  topShadow = color;
}

void displaySchemeClass::setBotShadow ( unsigned int color )
{
  botShadow = color;
}

void displaySchemeClass::setFont ( char *font )
{
  strncpy( defFontTag, font, 127 );
}

void displaySchemeClass::setCtlFont ( char *font )
{
  strncpy( defCtlFontTag, font, 127 );
}

void displaySchemeClass::setBtnFont ( char *font )
{
  strncpy( defBtnFontTag, font, 127 );
}

void displaySchemeClass::setAlignment ( int alignment )
{
  defAlignment = alignment;
}

void displaySchemeClass::setCtlAlignment ( int alignment )
{
  defCtlAlignment = alignment;
}

void displaySchemeClass::setBtnAlignment ( int alignment )
{
  defBtnAlignment = alignment;
}

void displaySchemeClass::setPvType ( char *pvType )
{
  strncpy( defPvType, pvType, 15 );
}

int isLoaded ( void )
{
  return schemeLoaded;
}

};

#endif
