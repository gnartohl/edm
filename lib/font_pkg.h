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

#include <map>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <time.h>
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
#define FONTINFO_SYNTAX 110
#define FONTINFO_BADSPEC 112
#define FONTINFO_TOOMANYSIZES 114
#define FONTINFO_MISSINGBRACE 116
#define FONTINFO_GROUPSYNTAX 118

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

typedef std::map<std::string, std::string> FontMapType, *FontMapPtr;
typedef FontMapType::value_type FontMapEntry;

FontMapPtr fontMap;

AVL_HANDLE fontNameListH;

XmFontList fontList;
int fontListEmpty;

familyListPtr familyHead;
familyListPtr familyTail;

Display *display;

char defFontTag[127+1], defSiteFontTag[127+1];

int requireExactMatch; // all font sizes/types must exist, do no substitution

char mediumString[63+1], boldString[63+1], regularString[63+1],
 italicString[63+1];

int lineNum, lastNonCommentLine;

public:

fontInfoClass ( void );   // constructor

~fontInfoClass ( void );   // destructor

char *getStrFromFile (
  char *str,
  int maxLen,
  FILE *f
);

void setMediumString (
  char *str
);

void setBoldString (
  char *str
);

void setRegularString (
  char *str
);

void setItalicString (
  char *str
);

int parseFontSpec (
  char *fontSpec,
  char *foundary,
  char *family,
  char *weight,
  char *slant,
  char *pixelSize );

char *defaultSiteFont ( void ) { return defSiteFontTag; }

char *defaultFont ( void ) { return defFontTag; }

int addFont (
  char *name );

int flushToBrace (
  FILE *f );

int checkSingleFontSpecGeneric (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int checkBestFont,
  int major,
  int minor,
  int release );

int checkSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *buf,
  int major,
  int minor,
  int release );

int checkBestSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *buf,
  int major,
  int minor,
  int release );

int getSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *buf,
  int major,
  int minor,
  int release );

int processFontGroup (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  FILE *f,
  int major,
  int minor,
  int release );

int readSubstitutions (
  FILE *f
);

int checkSingleFontSpecGenericVer5 (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int checkBestFont,
  int major,
  int minor,
  int release );

int checkSingleFontSpecVer5 (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *buf,
  int major,
  int minor,
  int release );

int checkBestSingleFontSpecVer5 (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *buf,
  int major,
  int minor,
  int release );

int getSingleFontSpecVer5 (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *buf,
  int major,
  int minor,
  int release );

int processFontGroupVer5 (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  FILE *f,
  int major,
  int minor,
  int release );

int initFromFileVer5 (
  XtAppContext app,
  Display *d,
  FILE *f,
  int major,
  int minor,
  int release );

int initFromFileVer4 (
  XtAppContext app,
  Display *d,
  FILE *f,
  int major,
  int minor,
  int release );

int initFromFileVer3 (
  XtAppContext app,
  Display *d,
  FILE *f,
  int major,
  int minor,
  int release );

int initFromFile (
  XtAppContext app,
  Display *d,
  char *fileName );

int resolveFont (
  char *fontSpec,
  fontNameListPtr ptr );

int resolveFont (
  char *fontSpec,
  char *userFontFamilyName,
  fontNameListPtr ptr );

int resolveFont (
  char *fontSpec,
  char *useWeight,
  char *useSlant,
  char *userFontFamilyName,
  fontNameListPtr ptr );

int resolveOneFont (
  char *fontSpec,
  fontNameListPtr ptr );

int resolveFontVer5 (
  char *fontSpec,
  char *sizeLabel,
  fontNameListPtr ptr );

int resolveFontVer5 (
  char *fontSpec,
  char *sizeLabel,
  char *userFontFamilyName,
  fontNameListPtr ptr );

int resolveFontVer5 (
  char *fontSpec,
  char *sizeLabel,
  char *useWeight,
  char *useSlant,
  char *userFontFamilyName,
  fontNameListPtr ptr );

int resolveOneFontVer5 (
  char *fontSpec,
  char *sizeLabel,
  fontNameListPtr ptr );

int getFontName (
  char *fontTag,
  double rotation,
  char *name,
  int len );

XFontStruct *getXFontStruct (
  char *name );

XFontStruct *getXNativeFontStruct (
  char *name );

int loadFontTag (
  char *name );

XmFontList getXmFontList ( void );

int getTextFontList (
  char *name,
  XmFontList *fontList );

int appendSizeMenu(
  char *family,
  int size,
  float fsize );

int getFirstFontMapping (
  char *tag,
  int tagMax,
  char *spec,
  int specMax
);

int getNextFontMapping (
  char *tag,
  int tagMax,
  char *spec,
  int specMax
);

};

#endif
