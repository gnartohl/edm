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

#ifndef __font_pkg_h
#define __font_pkg_h 1

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <Xm/Xm.h>
#include <math.h>

#include "sys_types.h"
#include "thread.h"
#include "avl.h"
#include "font_pkg.str"

#define FONTINFO_SUCCESS 1
#define FONTINFO_EMPTY 100
#define FONTINFO_FAIL 102
#define FONTINFO_NO_FILE 104
#define FONTINFO_NO_FONT 106
#define FONTINFO_NO_MEM 108

void processAllEvents (
  XtAppContext app,
  Display *d );

typedef struct fontNameListTag {
  AVL_FIELDS(fontNameListTag)
  XFontStruct *fontStruct;
  char *fullName;
  char *name;
  char *family;
  int size;
  float fsize;
  char weight;   // m or b
  char slant;    // r or i
  char isScalable;
  char fontLoaded;
} fontNameListType, *fontNameListPtr;

typedef struct sizeListTag {
  struct sizeListTag *flink;
  int size;
  float fsize;
} sizeListType, *sizeListPtr;

typedef struct familyListTag {
  struct familyListTag *flink;
  sizeListPtr sizeHead;
  sizeListPtr sizeTail;
  char *name;
} familyListType, *familyListPtr;

class fontInfoClass {

friend class fontMenuClass;

private:

AVL_HANDLE fontNameListH;

XmFontList fontList;
int fontListEmpty;

familyListPtr familyHead;
familyListPtr familyTail;

Display *display;

char defFontTag[127+1], defSiteFontTag[127+1];

public:

fontInfoClass::fontInfoClass ( void );   // constructor

fontInfoClass::~fontInfoClass ( void );   // destructor

char *fontInfoClass::defaultSiteFont ( void ) { return defSiteFontTag; }

char *fontInfoClass::defaultFont ( void ) { return defFontTag; }

int fontInfoClass::addFont (
  char *name );

int fontInfoClass::initFromFile (
  XtAppContext app,
  Display *d,
  char *fileName );

int fontInfoClass::resolveFont (
  char *fontSpec,
  fontNameListPtr ptr );

int fontInfoClass::resolveOneFont (
  char *fontSpec,
  fontNameListPtr ptr );

int fontInfoClass::show ( void );

int fontInfoClass::getFontName (
  char *fontTag,
  double rotation,
  char *name,
  int len );

XFontStruct *fontInfoClass::getXFontStruct (
  char *name );

XFontStruct *fontInfoClass::getXNativeFontStruct (
  char *name );

int fontInfoClass::loadFontTag (
  char *name );

XmFontList fontInfoClass::getXmFontList ( void );

int fontInfoClass::getTextFontList (
  char *name,
  XmFontList *fontList );

int fontInfoClass::appendSizeMenu(
  char *family,
  int size,
  float fsize );

};

#endif
