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

// font model code
//

#include "font_pkg.h"
#include "remFileOpen.h"

#include "thread.h"

static int debugMode ( void ) {

int val;
char *envPtr;

  envPtr = getenv( "EDMDEBUGMODE" );
  if ( envPtr ) {
    val = atol(envPtr);
    if ( !val ) val = 1; // if value is non-numeric make it 1
    return val;
  }
  else {
    return 0;
  }

}

static void fixFontSize (
  char *spec
) {

  // On non-english speaking systems, the format of floating points
  // rendered by "sprintf" may use a ',' for the decimal separator,
  // instead of a '.'. If so, change each ',' to '.'.

int i;

  for ( i=0; i<(int) strlen(spec); i++ ) {
     if ( spec[i] == ',' ) spec[i] = '.';
  }

}

static int compare_nodes (
  void *node1,
  void *node2
) {

fontNameListPtr p1, p2;

  p1 = (fontNameListPtr) node1;
  p2 = (fontNameListPtr) node2;

  return strcmp( p1->name, p2->name );

}

static int compare_key (
  void *key,
  void *node
) {

fontNameListPtr p;
char *name;

  p = (fontNameListPtr) node;
  name = (char *) key;

  return strcmp( name, p->name );

}

static int copy_nodes (
  void *node1,
  void *node2
) {

fontNameListPtr p1, p2;

  p1 = (fontNameListPtr) node1;
  p2 = (fontNameListPtr) node2;

  *p1 = *p2;

  return 1;

}

fontInfoClass::fontInfoClass ( void ) {   // constructor

int stat;

  stat = thread_init();

  stat = avl_init_tree( compare_nodes,
   compare_key, copy_nodes, &(this->fontNameListH) );
  if ( !( stat & 1 ) ) this->fontNameListH = (AVL_HANDLE) NULL;

  // create sentinel node
  familyHead = new familyListType;
  familyTail = familyHead;
  familyTail->flink = NULL;

  fontMap = NULL;

  fontListEmpty = 1;

  requireExactMatch = 0;

  strcpy( mediumString, "medium" );
  strcpy( boldString, "bold" );
  strcpy( regularString, "r" );
  strcpy( italicString, "i" );

  lineNum = lastNonCommentLine = 1;

}

fontInfoClass::~fontInfoClass ( void ) {   // destructor

int stat;
fontNameListPtr cur;
familyListPtr curFamily, nextFamily;
sizeListPtr curSize, nextSize;

  stat = avl_get_first( this->fontNameListH, (void **) &cur );
  if ( !( stat & 1 ) ) {
    cur = NULL;
  }

  while ( cur ) {

    stat = avl_delete_node( this->fontNameListH, (void **) &cur );
    if ( stat & 1 ) {

      if ( cur->fontLoaded ) {
        if ( cur->fontStruct ) {
          XFreeFont( this->display, cur->fontStruct );
          cur->fontLoaded = 0;
        }
      }

      if ( cur->name ) {
        delete[] cur->name;
        cur->name = NULL;
      }
      if ( cur->fullName ) {
        delete[] cur->fullName;
        cur->fullName = NULL;
      }
      if ( cur->family ) {
        delete[] cur->family;
        cur->family = NULL;
      }

      delete cur;

    }

    stat = avl_get_first( this->fontNameListH, (void **) &cur );
    if ( !( stat & 1 ) ) {
      cur = NULL;
    }

  }

  stat = avl_destroy( this->fontNameListH );

  curFamily = familyHead->flink;
  while ( curFamily ) {

    nextFamily = curFamily->flink;

    curSize = curFamily->sizeHead->flink;
    while ( curSize ) {
      nextSize = curSize->flink;
      delete curSize;
      curSize = nextSize;
    }
    delete curFamily->sizeHead;

    delete[] curFamily->name;

    delete curFamily;

    curFamily = nextFamily;

  }

  delete familyHead;

  if ( fontMap ) {
    fontMap->erase( fontMap->begin(), fontMap->end() );
    delete fontMap;
  }

}

char *fontInfoClass::getStrFromFile (
  char *str,
  int maxLen,
  FILE *f
) {

char *ctx, *ptr, *tk, stackBuf[255+1];
char *buf;
int tryAgain, bufOnHeap;

  // ignore blank lines and comment lines

  if ( maxLen < 1 ) return (char *) NULL;

  if ( maxLen > 255 ) {
    buf = new char[maxLen+1];
    bufOnHeap = 1;
  }
  else {
    buf = stackBuf;
    bufOnHeap = 0;
  }

  do {

    tryAgain = 0;

    ptr = fgets( str, maxLen, f );
    if ( !ptr ) {
      strcpy( str, "" );
      if ( bufOnHeap ) delete [] buf;
      return (char *) NULL;
    }

    lineNum++;

    strcpy( buf, str );

    ctx = NULL;
    tk = strtok_r( buf, "\n", &ctx );

    if ( tk ) {

      if ( tk[0] == '#' ) tryAgain = 1;

    }
    else {

      tryAgain = 1;

    }

  } while ( tryAgain );

  lastNonCommentLine = lineNum;

  if ( bufOnHeap ) delete [] buf;

  return str;

}

int fontInfoClass::parseFontSpec (
  char *fontSpec,
  char *foundary,
  char *family,
  char *weight,
  char *slant,
  char *pixelSize ) {

static const int GETTING_DASH = 1;
static const int GETTING_STRING = 2;
static const int GETTING_LAST_STRING = 3;
static const int STORE_VALUE = 4;
static const int DONE = -1;
int l, ii, iii, i = 0, first = 0, last = 0, n = 0;
int state = GETTING_DASH;
char value[14][63+1];

  l = strlen( fontSpec );

  while ( state != DONE ) {

    //fprintf( stderr, "s: %-d, n=%-d, i=%-d\n", state, n, i );

    switch ( state ) {

    case GETTING_DASH:

      if ( fontSpec[i] == '-' ) {
        i++;
        if ( i >= l ) return FONTINFO_BADSPEC;
        first = i;
        state = GETTING_STRING;
      }
      else if ( fontSpec[i] == '\t' ) {
        return FONTINFO_BADSPEC;
      }
      else {
        return FONTINFO_BADSPEC;
      }

      break;

    case GETTING_STRING:

      if ( fontSpec[i] == '-' ) {
        last = i - 1;
        state = STORE_VALUE;
      }
      else if ( fontSpec[i] == '\t' ) {
        return FONTINFO_BADSPEC;
      }

      i++;
      if ( i >= l ) return FONTINFO_BADSPEC;

      break;

    case GETTING_LAST_STRING:

      if ( fontSpec[i] == '\t' ) {
        last = i - 1;
        state = STORE_VALUE;
      }

      i++;
      if ( i >= l ) {
        last = i - 1;
        state = STORE_VALUE;
      }

      break;

    case STORE_VALUE:

      if ( last >= first ) {

        for ( ii=first, iii=0; ii<=last; ii++, iii++ ) {
          value[n][iii] = fontSpec[ii];
        }
        value[n][iii] = 0;

        //fprintf( stderr, "value[%-d] = [%s]\n", n, value[n] );

      }
      else {

	strcpy( value[n], "" );
        //fprintf( stderr, "value[%-d] = NULL\n", n );

      }

      first = i;

      n++;

      if ( n < 13 ) {
        state = GETTING_STRING;
      }
      else if ( n == 13 ) {
        state = GETTING_LAST_STRING;
      }
      else {
	state = DONE;
      }

      break;

    }

  }

  strncpy( foundary, value[0], 63 );
  foundary[63] = 0;

  strncpy( family, value[1], 63 );
  family[63] = 0;

  strncpy( weight, value[2], 63 );
  weight[63] = 0;

  strncpy( slant, value[3], 63 );
  slant[63] = 0;

  strncpy( pixelSize, value[7], 63 );
  pixelSize[63] = 0;

  return FONTINFO_SUCCESS;

}

static char **findBestFont(
  Display *d,
  char *fontSpec,
  int *n ) {

char **list;

char *tk, *ctx, spec[127+1], rest[127+1], foundry[63+1], family[63+1],
 weight[31+1], slant[31+1], ftype[31+1], size[31+1], newFont[127+1];

  strncpy( spec, fontSpec, 127 );
  spec[127] = 0;

  ctx = NULL;
  tk = strtok_r( spec, "-", &ctx );
  if ( !tk ) goto err_return;
  strncpy( foundry, tk, 63 );
  foundry[63] = 0;

  tk = strtok_r( NULL, "-", &ctx );
  if ( !tk ) goto err_return;
  strncpy( family, tk, 63 );
  family[63] = 0;

  tk = strtok_r( NULL, "\n", &ctx );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );
  rest[127] = 0;

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, rest, 127 );

  //  fprintf( stderr, "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( spec, rest, 127 );
  spec[127] = 0;

  ctx = NULL;
  tk = strtok_r( spec, "-", &ctx );
  if ( !tk ) goto err_return;
  strncpy( weight, tk, 31 );
  weight[31] = 0;

  tk = strtok_r( NULL, "\n", &ctx );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );
  rest[127] = 0;

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, rest, 127 );

//   fprintf( stderr, "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( spec, rest, 127 );
  spec[127] = 0;

  ctx = NULL;
  tk = strtok_r( spec, "-", &ctx );
  if ( !tk ) goto err_return;
  strncpy( slant, tk, 31 );
  slant[31] = 0;

  tk = strtok_r( NULL, "\n", &ctx );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );
  rest[127] = 0;

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, rest, 127 );

//   fprintf( stderr, "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( spec, rest, 127 );
  spec[127] = 0;

  ctx = NULL;
  tk = strtok_r( spec, "-", &ctx );
  if ( !tk ) goto err_return;
  strncpy( ftype, tk, 31 );
  ftype[31] = 0;

  tk = strtok_r( NULL, "-", &ctx );
  if ( !tk ) goto err_return;
  strncpy( size, tk, 31 );
  size[31] = 0;

  tk = strtok_r( NULL, "\n", &ctx );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );
  rest[127] = 0;

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, ftype, 127 );
  Strncat( newFont, "--", 127 );
  Strncat( newFont, size, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-*-*-*-*-*-*", 127 );

//   fprintf( stderr, "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-*-*-*-*--*-*-*-*-*-*-*-*", 127 );

//   fprintf( stderr, "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( newFont, "-*-*-*-*-*--*-*-*-*-*-*-*-*", 127 );

//   fprintf( stderr, "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

err_return:
  *n = 0;
  return (char **) NULL;

}

int fontInfoClass::resolveFont (
  char *fontSpec,
  fontNameListPtr ptr ) {

int n, isize, isScalable;
float fsize;
char **list;
char *tk, *ctx, spec[127+1], name[127+1], family[63+1], weight[31+1],
 slant[31+1], size[31+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    list = findBestFont( this->display, fontSpec, &n );
    if ( n == 0 ) {
      return FONTINFO_NO_FONT;
    }
  }

  strncpy( spec, list[0], 127 );

  if ( debugMode() == 1000 ) fprintf( stderr, "Font Spec: [%s]\n", spec );

  ctx = NULL;
  tk = strtok_r( spec, "-", &ctx );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( family, tk, 63 );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( weight, tk, 31 );

  tk = strtok_r( NULL, "-", &ctx );
  if ( strcmp( tk, "r" ) == 0 )
    strncpy( slant, "r", 31 );
  else
    strncpy( slant, "i", 31 );

  tk = strtok_r( NULL, "-", &ctx );
  tk = strtok_r( NULL, "-", &ctx );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( size, tk, 31 );
  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );
  fsize = atof( size );
  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );
  fixFontSize( size );

  strncpy( name, family, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(family)+1];
  strcpy( ptr->family, family );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}

void fontInfoClass::setMediumString (
  char *str
) {

  strncpy( mediumString, str, 63 );
  mediumString[63] = 0;

}
void fontInfoClass::setBoldString (
  char *str
) {

  strncpy( boldString, str, 63 );
  boldString[63] = 0;

}
void fontInfoClass::setRegularString (
  char *str
) {

  strncpy( regularString, str, 63 );
  regularString[63] = 0;

}
void fontInfoClass::setItalicString (
  char *str
) {

  strncpy( italicString, str, 63 );
  italicString[63] = 0;

}

int fontInfoClass::resolveFont (
  char *fontSpec,
  char *userFontFamilyName,
  fontNameListPtr ptr ) {

int n, isize, isScalable, stat;
float fsize;
char **list;
char spec[127+1], name[127+1], foundary[63+1], family[63+1], weight[63+1],
 slant[63+1], size[63+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    if ( requireExactMatch ) {
      fprintf( stderr, fontInfoClass_str8, fontSpec );
      fprintf( stderr, fontInfoClass_str9, lastNonCommentLine );
      return FONTINFO_NO_FONT;
    }
    else {
      list = findBestFont( this->display, fontSpec, &n );
      if ( n == 0 ) {
        fprintf( stderr, fontInfoClass_str8, fontSpec );
        fprintf( stderr, fontInfoClass_str9, lastNonCommentLine );
        return FONTINFO_NO_FONT;
      }
    }
  }

  strncpy( spec, list[0], 127 );

  if ( debugMode() == 1000 ) fprintf( stderr, "Font Spec: [%s]\n", spec );

  stat = parseFontSpec( spec, foundary, family, weight, slant, size );

  if ( strcmp( weight, mediumString ) == 0 ) {
    strcpy( weight, "medium" );
  }
  else if ( strcmp( weight, boldString ) == 0 ) {
    strcpy( weight, "bold" );
  }
  else {
    strcpy( weight, "medium" );
  }

  if ( strcmp( slant, regularString ) == 0 ) {
    strcpy( slant, "r" );
  }
  else if ( strcmp( slant, italicString ) == 0 ) {
    strcpy( slant, "i" );
  }
  else {
    strcpy( slant, "r" );
  }

  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );
  fsize = atof( size );
  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );
  fixFontSize( size );

  strncpy( name, userFontFamilyName, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  //fprintf( stderr, "name=[%s]\n", name );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(userFontFamilyName)+1];
  strcpy( ptr->family, userFontFamilyName );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::resolveFont (
  char *fontSpec,
  char *userFontFamilyName,
  char *useWeight,
  char *useSlant,
  fontNameListPtr ptr ) {

int n, isize, isScalable, stat;
float fsize;
char **list;
char spec[127+1], name[127+1], foundary[63+1], family[63+1], weight[63+1],
 slant[63+1], size[63+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    if ( requireExactMatch ) {
      fprintf( stderr, fontInfoClass_str8, fontSpec );
      fprintf( stderr, fontInfoClass_str9, lastNonCommentLine );
      return FONTINFO_NO_FONT;
    }
    else {
      list = findBestFont( this->display, fontSpec, &n );
      if ( n == 0 ) {
        fprintf( stderr, fontInfoClass_str8, fontSpec );
        fprintf( stderr, fontInfoClass_str9, lastNonCommentLine );
        return FONTINFO_NO_FONT;
      }
    }
  }

  strncpy( spec, list[0], 127 );

  if ( debugMode() == 1000 ) fprintf( stderr, "Font Spec: [%s]\n", spec );

  stat = parseFontSpec( spec, foundary, family, weight, slant, size );

  if ( strcmp( useWeight, mediumString ) == 0 ) {
    strcpy( weight, "medium" );
  }
  else if ( strcmp( useWeight, boldString ) == 0 ) {
    strcpy( weight, "bold" );
  }
  else {
    strcpy( weight, "medium" );
  }

  if ( strcmp( useSlant, regularString ) == 0 ) {
    strcpy( slant, "r" );
  }
  else if ( strcmp( useSlant, italicString ) == 0 ) {
    strcpy( slant, "i" );
  }
  else {
    strcpy( slant, "r" );
  }

  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );
  fsize = atof( size );
  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );
  fixFontSize( size );

  strncpy( name, userFontFamilyName, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  //fprintf( stderr, "name=[%s]\n", name );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(userFontFamilyName)+1];
  strcpy( ptr->family, userFontFamilyName );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::resolveOneFont (
  char *fontSpec,
  fontNameListPtr ptr ) {

int n, isize, isScalable;
float fsize;
char **list;
char *tk, *ctx, spec[127+1], name[127+1], family[63+1], weight[31+1],
 slant[31+1], size[31+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    return FONTINFO_NO_FONT;
  }

  strncpy( spec, list[0], 127 );

//   fprintf( stderr, "Spec is [%s]\n", spec );

  ctx = NULL;
  tk = strtok_r( spec, "-", &ctx );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( family, tk, 63 );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( weight, tk, 31 );

  tk = strtok_r( NULL, "-", &ctx );
  if ( strcmp( tk, "r" ) == 0 )
    strncpy( slant, "r", 31 );
  else
    strncpy( slant, "i", 31 );

  tk = strtok_r( NULL, "-", &ctx );
  tk = strtok_r( NULL, "-", &ctx );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( size, tk, 31 );
  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );
  fsize = atof( size );
  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );
  fixFontSize( size );

  strncpy( name, family, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(family)+1];
  strcpy( ptr->family, family );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::appendSizeMenu(
  char *family,
  int size,
  float fsize )
{

familyListPtr curFamily;
sizeListPtr curSize;

  curFamily = familyHead->flink;
  while ( curFamily ) {

    if ( strcmp( curFamily->name, family ) == 0 ) {

      curSize = curFamily->sizeHead->flink;
      while ( curSize ) {
        if ( curSize->fsize == fsize ) return FONTINFO_SUCCESS;
        curSize = curSize->flink;
      }

      // append size to list
      curSize = new sizeListType;
      if ( !curSize ) return FONTINFO_NO_MEM;
      curSize->size = size;
      curSize->fsize = fsize;
      curFamily->sizeTail->flink = curSize;
      curFamily->sizeTail = curSize;
      curFamily->sizeTail->flink = NULL;
      
      return FONTINFO_SUCCESS;

    }

    curFamily = curFamily->flink;

  }

  // new family, append to list

  curFamily = new familyListType;
  if ( !curFamily ) return FONTINFO_NO_MEM;

  curFamily->name = new char[strlen(family)+1];
  if ( !curFamily->name ) return FONTINFO_NO_MEM;
  strcpy( curFamily->name, family );
  curFamily->sizeHead = new sizeListType;
  if ( !curFamily->sizeHead ) return FONTINFO_NO_MEM;
  curFamily->sizeTail = curFamily->sizeHead;
  curFamily->sizeTail->flink = NULL;

  familyTail->flink = curFamily;
  familyTail = curFamily;
  familyTail->flink = NULL;

  // append size to new list
  curSize = new sizeListType;
  if ( !curSize ) return FONTINFO_NO_MEM;
  curSize->size = size;
  curSize->fsize = fsize;
  curFamily->sizeTail->flink = curSize;
  curFamily->sizeTail = curSize;
  curFamily->sizeTail->flink = NULL;

  return FONTINFO_SUCCESS;

}

int fontInfoClass::checkSingleFontSpecGeneric (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int checkBestFont,
  int major,
  int minor,
  int release )
{

char buf[255+1], t1[255+1], t2[255+1], t3[255+1], t4[255+1],
 t5[255+1], t6[255+1], t7[255+1], mod[4][255+1], fontSpec[255+1],
 *tk1, *tk2, *ctx1, *ctx2;
int i, ii, iii, n, pointSize[200], numSizes;
int preload;
char **list;

  strncpy( buf, line, 255 );

  ctx1 = NULL;
  tk1 = strtok_r( buf, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t1, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t2, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get bold and medium indicators

    ctx2 = NULL;
    tk2 = strtok_r( t2, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[0], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[1], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t3, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t4, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get italic and regular indicators

    ctx2 = NULL;
    tk2 = strtok_r( t4, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[2], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[3], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t5, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t6, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get point sizes
    numSizes = 0;
    ctx2 = NULL;
    tk2 = strtok_r( t6, ",", &ctx2 );
    if ( tk2 ) {
      pointSize[numSizes] = atol( tk2 );
      numSizes++;
      if ( numSizes >= 200 ) {
        fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
        return FONTINFO_TOOMANYSIZES;
      }
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    do {

      tk2 = strtok_r( NULL, ",", &ctx2 );
      if ( tk2 ) {
        pointSize[numSizes] = atol( tk2 );
        numSizes++;
        if ( numSizes >= 200 ) {
          fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
          return FONTINFO_TOOMANYSIZES;
        }
      }

    } while ( tk2 );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t7, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  preload = 0;
  requireExactMatch = 0;

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  //fprintf( stderr, "t1 = [%s]\n", t1 );
  //fprintf( stderr, "  mod[0] = [%s]\n", mod[0] );
  //fprintf( stderr, "  mod[1] = [%s]\n", mod[1] );
  //fprintf( stderr, "t3 = [%s]\n", t3 );
  //fprintf( stderr, "  mod[2] = [%s]\n", mod[2] );
  //fprintf( stderr, "  mod[3] = [%s]\n", mod[3] );
  //fprintf( stderr, "t5 = [%s]\n", t5 );

  //for ( i=0; i<numSizes; i++ ) {
  //  fprintf( stderr, "  size[%-d] = %-d\n", i, pointSize[i] );
  //}

  //fprintf( stderr, "t7 = [%s]\n", t7 );

  // Build fontspec

  for ( i=0; i<2; i++ ) {

    for ( ii=2; ii<4; ii++ ) {

      for ( iii=0; iii<numSizes; iii++ ) {

        sprintf( fontSpec, "%s%s%s%s%s%-d%s", t1, mod[i], t3, mod[ii],
         t5, pointSize[iii], t7 );

        //fprintf( stderr, "checkSingleFontSpec : [%s]\n", fontSpec );

        if ( fontMap ) {
          FontMapType::iterator it = fontMap->begin();
          while ( it != fontMap->end() ) {
	    std::string f = it->first;
	    std::string s = it->second;
            if ( strcmp( fontSpec, f.c_str() ) == 0 ) {
              strcpy( fontSpec, s.c_str() );
	      break;
	    }
	    it++;
	  }
        }

        list = XListFonts( display, fontSpec, 1, &n );
        if ( n == 0 ) {
          if ( checkBestFont && !requireExactMatch ) {
            list = findBestFont( this->display, fontSpec, &n );
            if ( n == 0 ) {
              fprintf( stderr, fontInfoClass_str8, fontSpec );
              fprintf( stderr, fontInfoClass_str9, lastNonCommentLine );
              return FONTINFO_NO_FONT;
            }
          }
          else {
            return FONTINFO_NO_FONT;
	  }
        }

        XFreeFontNames( list );

      }

    }

  }

  return FONTINFO_SUCCESS;

}

int fontInfoClass::checkSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int major,
  int minor,
  int release )
{

int checkBest = 0;

  return checkSingleFontSpecGeneric( app, d, userFontFamilyName,
   line, checkBest, major, minor, release );

}

int fontInfoClass::checkBestSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int major,
  int minor,
  int release )
{

int checkBest = 1;

  return checkSingleFontSpecGeneric( app, d, userFontFamilyName,
   line, checkBest, major, minor, release );

}

int fontInfoClass::getSingleFontSpec (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *buf,
  int major,
  int minor,
  int release )
{

char t1[255+1], t2[255+1], t3[255+1], t4[255+1],
 t5[255+1], t6[255+1], t7[255+1], mod[4][255+1], fontSpec[255+1],
 *tk1, *tk2, *ctx1, *ctx2;
int useSubstitution;
int i, ii, iii, pointSize[200], numSizes;
int stat, preload;
int empty = 1;
fontNameListPtr cur;
int dup;
XFontStruct *fs;

  ctx1 = NULL;
  tk1 = strtok_r( buf, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t1, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t2, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get medium and bold modifiers

    ctx2 = NULL;
    tk2 = strtok_r( t2, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[0], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    setMediumString( mod[0] );

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[1], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    setBoldString( mod[1] );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t3, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t4, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get italic and regular indicators

    ctx2 = NULL;
    tk2 = strtok_r( t4, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[2], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    setRegularString( mod[2] );

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[3], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    setItalicString( mod[3] );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t5, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t6, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get point sizes
    numSizes = 0;
    ctx2 = NULL;
    tk2 = strtok_r( t6, ",", &ctx2 );
    if ( tk2 ) {
      pointSize[numSizes] = atol( tk2 );
      numSizes++;
      if ( numSizes >= 200 ) {
        fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
        return FONTINFO_TOOMANYSIZES;
      }
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    do {

      tk2 = strtok_r( NULL, ",", &ctx2 );
      if ( tk2 ) {
        pointSize[numSizes] = atol( tk2 );
        numSizes++;
        if ( numSizes >= 200 ) {
          fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
          return FONTINFO_TOOMANYSIZES;
        }
      }

    } while ( tk2 );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t7, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  preload = 0;
  requireExactMatch = 0;

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  //fprintf( stderr, "preload = %-d\n", preload );
  //fprintf( stderr, "exact = %-d\n", requireExactMatch );

  //fprintf( stderr, "t1 = [%s]\n", t1 );
  //fprintf( stderr, "  mod[0] = [%s]\n", mod[0] );
  //fprintf( stderr, "  mod[1] = [%s]\n", mod[1] );
  //fprintf( stderr, "t3 = [%s]\n", t3 );
  //fprintf( stderr, "  mod[2] = [%s]\n", mod[2] );
  //fprintf( stderr, "  mod[3] = [%s]\n", mod[3] );
  //fprintf( stderr, "t5 = [%s]\n", t5 );

  //for ( i=0; i<numSizes; i++ ) {
  //  fprintf( stderr, "  size[%-d] = %-d\n", i, pointSize[i] );
  //}

  //fprintf( stderr, "t7 = [%s]\n", t7 );

  // Build fontspec

  for ( i=0; i<2; i++ ) {

    for ( ii=2; ii<4; ii++ ) {

      for ( iii=0; iii<numSizes; iii++ ) {

        sprintf( fontSpec, "%s%s%s%s%s%-d%s", t1, mod[i], t3, mod[ii],
         t5, pointSize[iii], t7 );

        //fprintf( stderr, "getSingleFontSpec : [%s]\n", fontSpec );

        useSubstitution = 0;

        if ( fontMap ) {
          FontMapType::iterator it = fontMap->begin();
          while ( it != fontMap->end() ) {
	    std::string f = it->first;
	    std::string s = it->second;
            if ( strcmp( fontSpec, f.c_str() ) == 0 ) {
              strcpy( fontSpec, s.c_str() );
              useSubstitution = 1;
	      break;
	    }
	    it++;
	  }
        }

        cur = new fontNameListType;

	if ( useSubstitution ) {
          stat = this->resolveFont( fontSpec, userFontFamilyName, mod[i], mod[ii], cur );
          if ( !( stat & 1 ) ) {
            delete cur;
            return stat;
          }
	}
	else {
          stat = this->resolveFont( fontSpec, userFontFamilyName, cur );
          if ( !( stat & 1 ) ) {
            delete cur;
            return stat;
          }
	}

        stat = avl_insert_node( this->fontNameListH, (void *) cur,
         &dup );
        if ( !( stat & 1 ) ) {
          fprintf( stderr, fontInfoClass_str11, __LINE__, __FILE__ );
          return stat;
	}
        if ( dup ) {
          fprintf( stderr, fontInfoClass_str13, cur->name, lastNonCommentLine );
	}

        if ( preload ) {
          //fprintf( stderr, "preload %s\n", cur->name );
          fs = getXFontStruct( cur->name );
        }

        stat = appendSizeMenu( cur->family, cur->size, cur->fsize );
        if ( !( stat & 1 ) ) return stat;
         empty = 0;

      }

    }

  }

  return 1;

}

int fontInfoClass::flushToBrace (
  FILE *f )
{

char line[255+1], *tk, *ptr, *ctx;
int foundBrace = 0;

  do {

    ptr = getStrFromFile ( line, 255, f );
    if ( ptr ) {

      ctx = NULL;

      tk = strtok_r( line, " \t\n", &ctx );
      if ( tk ) {
        if ( tk[0] == '}' ) {
          foundBrace = 1;
        }
        else {
          foundBrace = 0;
        }
      }

    }
    else {
      fprintf( stderr, fontInfoClass_str10 );
      return FONTINFO_FAIL;
    }

  } while ( !foundBrace );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::processFontGroup (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  FILE *f,
  int major,
  int minor,
  int release )
{

char line[255+1], buf[255+1], lastLine[255+1], *ptr, *tk1, *ctx1;
int stat;
int foundBrace = 0;

  strcpy( lastLine, "" );
  stat = FONTINFO_GROUPSYNTAX; // in case all lines are blank

  do {

    processAllEvents( app, display );

    ptr = getStrFromFile ( line, 255, f );
    if ( ptr ) {

      ctx1 = NULL;
      strcpy( buf, line );

      tk1 = strtok_r( buf, "\t\n", &ctx1 );
      if ( tk1 ) {
        if ( tk1[0] == '}' ) {
          foundBrace = 1;
	}
	else {
          foundBrace = 0;
	}
      }

      if ( ! foundBrace ) {

        strcpy( lastLine, line );

        stat = checkSingleFontSpec( app, d, userFontFamilyName, line,
         major, minor, release );
        if ( stat & 1 ) {

          if ( debugMode() == 1000 ) fprintf( stderr, "Using font: %s", line );
          stat = getSingleFontSpec( app, d, userFontFamilyName, line,
           major, minor, release );

          flushToBrace( f );

          return stat; // return stat from getSingleFontSpec

        }
	else {
          if ( debugMode() == 1000 ) fprintf( stderr, "Font not found: %s", line );
	}

      }

    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

  } while ( !foundBrace );

  // If we never found a matching font, try to get something that matches,
  // even badly, using the findBestFont function (in this file)
  stat = checkBestSingleFontSpec( app, d, userFontFamilyName, lastLine,
   major, minor, release );
  if ( stat & 1 ) {
    if ( debugMode() == 1000 ) fprintf( stderr, "Using font (with substitutions): %s",
     lastLine );
    stat = getSingleFontSpec( app, d, userFontFamilyName, lastLine,
     major, minor, release );
  }

  if ( stat == FONTINFO_GROUPSYNTAX ) {
    fprintf( stderr, fontInfoClass_str12, lastNonCommentLine );
  }

  return stat; // return last stat from checkSingleFontSpec or GROUPSYNTAX

}

int fontInfoClass::readSubstitutions (
  FILE *f
) {

char line[255+1], buf[255+1], *ptr, *tk1, *tk2, *ctx1;

  ptr = getStrFromFile( line, 255, f );
  if ( !ptr ) {
    return FONTINFO_EMPTY;
  }

  do {

    ctx1 = NULL;
    strcpy( buf, line );

    tk1 = strtok_r( buf, "=\t\n", &ctx1 );
    if ( tk1 ) {

      if ( strcmp( tk1, "}" ) == 0 ) {
        return FONTINFO_SUCCESS;
      }

      tk2 = strtok_r( NULL, "=\t\n", &ctx1 );
      if ( tk2 ) {

	if ( !fontMap ) {
	  fontMap = new FontMapType;
	}

	std::string s1( tk1 );
	std::string s2( tk2 );
	fontMap->insert( FontMapEntry( s1, s2 ) );

      }
      else {

        return FONTINFO_SYNTAX;

      }

    }
    else {
      return FONTINFO_SYNTAX;
    }

    ptr = getStrFromFile( line, 255, f );
    if ( !ptr ) {
      return FONTINFO_EMPTY;
    }

  } while ( 1 );

}

int fontInfoClass::initFromFileVer4 (
  XtAppContext app,
  Display *d,
  FILE *f,
  int major,
  int minor,
  int release )
{

char line[255+1], buf[255+1], userFontFamilyName[63+1], *ptr, *tk1, *ctx1;
int stat;
int empty = 1;

  ptr = getStrFromFile( line, 255, f );
  if ( !ptr ) {
    fclose( f );
    fprintf( stderr, fontInfoClass_str3, lastNonCommentLine );
    return FONTINFO_EMPTY;
  }

  strncpy( defSiteFontTag, line, 127 );
  defSiteFontTag[127] = 0;
  defSiteFontTag[strlen(defSiteFontTag)-1] = 0; // discard \n

  ptr = getStrFromFile( line, 255, f );
  if ( !ptr ) {
    fclose( f );
    fprintf( stderr, fontInfoClass_str4, lastNonCommentLine );
    return FONTINFO_EMPTY;
  }

  strncpy( defFontTag, line, 127 );
  defFontTag[127] = 0;
  defFontTag[strlen(defFontTag)-1] = 0; // discard \n

  do {

    processAllEvents( app, display );

    ptr = getStrFromFile ( line, 255, f );
    if ( ptr ) {

      empty = 0;

      ctx1 = NULL;
      strcpy( buf, line );

      tk1 = strtok_r( buf, "=\t\n(){", &ctx1 );
      if ( tk1 ) {

        if ( strncmp( tk1, "substitutions", 13 ) == 0 ) {

          stat = readSubstitutions( f );
          if ( stat != FONTINFO_SUCCESS ) {
            fclose( f );
            return stat;
          }

	}
	else {

          strncpy( userFontFamilyName, tk1, 63 );
          userFontFamilyName[63] = 0;

          tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
          if ( tk1 ) {

            if ( strcmp( tk1, "{" ) == 0 ) { // font groups

              stat = processFontGroup( app, d, userFontFamilyName, f,
               major, minor, release );
              if ( !( stat & 1 ) ) {
                fclose( f );
                return stat;
              }

            }
            else {

              // tk1 points to first character after "<name>="

              strcpy( buf, line );

              stat = getSingleFontSpec( app, d, userFontFamilyName, tk1,
               major, minor, release );
              if ( !( stat & 1 ) ) {
                fclose( f );
                return stat;
              }

            }

          }
          else {
            fclose( f );
            fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
            return FONTINFO_SYNTAX;
          }

        }

      }
      else {

        fclose( f );
        fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
        return FONTINFO_SYNTAX;

      }

    }

  } while ( ptr );

  fclose( f );

  if ( empty ) {
    fprintf( stderr, fontInfoClass_str6 );
    return FONTINFO_EMPTY;
  }

  return FONTINFO_SUCCESS;

}

int fontInfoClass::initFromFileVer3 (
  XtAppContext app,
  Display *d,
  FILE *f,
  int major,
  int minor,
  int release )
{

char line[255+1], buf[255+1], userFontFamilyName[63+1], *ptr, *tk1, *ctx1;
int stat;
int empty = 1;

  ptr = getStrFromFile( line, 255, f );
  if ( !ptr ) {
    fclose( f );
    fprintf( stderr, fontInfoClass_str3, lastNonCommentLine );
    return FONTINFO_EMPTY;
  }

  strncpy( defSiteFontTag, line, 127 );
  defSiteFontTag[127] = 0;
  defSiteFontTag[strlen(defSiteFontTag)-1] = 0; // discard \n

  ptr = getStrFromFile( line, 255, f );
  if ( !ptr ) {
    fclose( f );
    fprintf( stderr, fontInfoClass_str4, lastNonCommentLine );
    return FONTINFO_EMPTY;
  }

  strncpy( defFontTag, line, 127 );
  defFontTag[127] = 0;
  defFontTag[strlen(defFontTag)-1] = 0; // discard \n

  do {

    processAllEvents( app, display );

    ptr = getStrFromFile ( line, 255, f );
    if ( ptr ) {

      empty = 0;

      ctx1 = NULL;
      strcpy( buf, line );

      tk1 = strtok_r( buf, "=\t\n()", &ctx1 );
      if ( tk1 ) {
        strncpy( userFontFamilyName, tk1, 63 );
        userFontFamilyName[63] = 0;
      }
      else {
        fclose( f );
        fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
        return FONTINFO_SYNTAX;
      }

      tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
      if ( tk1 ) {
        if ( strcmp( tk1, "{" ) == 0 ) { // font groups
          stat = processFontGroup( app, d, userFontFamilyName, f,
           major, minor, release );
          if ( !( stat & 1 ) ) {
            fclose( f );
            return stat;
          }
	}
	else {

          // tk1 points to first character after "<name>="

          strcpy( buf, line );

          stat = getSingleFontSpec( app, d, userFontFamilyName, tk1,
           major, minor, release );
          if ( !( stat & 1 ) ) {
            fclose( f );
            return stat;
          }

	}

      }
      else {
        fclose( f );
        fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
        return FONTINFO_SYNTAX;
      }

    }

  } while ( ptr );

  fclose( f );

  if ( empty ) {
    fprintf( stderr, fontInfoClass_str6 );
    return FONTINFO_EMPTY;
  }

  return FONTINFO_SUCCESS;

}

int fontInfoClass::initFromFile (
  XtAppContext app,
  Display *d,
  char *fileName )
{

// Read font specs from given file, query server, and populate data structure.
// If font does not exist, use some other font.

char line[127+1], *ptr, *fontSpec, *tk, *ctx;
int stat, preload;
FILE *f;
int empty = 1;
fontNameListPtr cur;
int dup;
int major, minor, release;
XFontStruct *fs;

  this->display = d;

  strncpy( defSiteFontTag, "helvetica-medium-r-10.0", 127 );
  strncpy( defFontTag, "helvetica-medium-r-14.0", 127 );

  //f = fopen( fileName, "r" );
  f = fileOpen( fileName, "r" );
  if ( !f ) {
    return FONTINFO_NO_FILE;
  }

  ptr = fgets( line, 127, f );
  if ( !ptr ) {
    fprintf( stderr, fontInfoClass_str6 );
    return FONTINFO_EMPTY;
  }

  sscanf( line, "%d %d %d\n", &major, &minor, &release );

  if ( major == 5 ) {
    stat = initFromFileVer5( app, d, f, major, minor, release );
    return stat;
  }

  if ( major == 4 ) {
    stat = initFromFileVer4( app, d, f, major, minor, release );
    return stat;
  }

  if ( major == 3 ) {
    stat = initFromFileVer3( app, d, f, major, minor, release );
    return stat;
  }

  if ( ( major > 1 ) || ( minor > 0 ) ) {

    ptr = fgets ( defSiteFontTag, 127, f );
    if ( !ptr ) {
      fclose( f );
      return FONTINFO_EMPTY;
    }

    defSiteFontTag[strlen(defSiteFontTag)-1] = 0; // discard \n

  }

  ptr = fgets ( defFontTag, 127, f );
  if ( !ptr ) {
    fclose( f );
    return FONTINFO_EMPTY;
  }

  defFontTag[strlen(defFontTag)-1] = 0; // discard \n

  do {

    processAllEvents( app, display );

    ptr = fgets ( line, 127, f );
    if ( ptr ) { // ignore blank lines

      ctx = NULL;
      fontSpec = strtok_r( line, "\t\n", &ctx );
      if ( fontSpec ) {

        if ( major >= 2 ) {

          tk = strtok_r( NULL, "\t\n", &ctx );
          if ( tk ) {
            if ( strcmp( tk, "preload" ) == 0 ) {
              preload = 1;
            }
            else {
              preload = 0;
            }
          }
          else {
            preload = 0;
          }

        }
        else {

          preload = 0;

        }

        cur = new fontNameListType;

        stat = this->resolveFont( fontSpec, cur );
        if ( !( stat & 1 ) ) {
          delete cur;
          return stat;
        }

        stat = avl_insert_node( this->fontNameListH, (void *) cur, &dup );
        if ( !( stat & 1 ) ) return stat;
//        if ( dup ) fprintf( stderr, "duplicate\n" );

        if ( preload ) {
          //fprintf( stderr, "preload %s\n", cur->name );
          fs = getXFontStruct( cur->name );
        }

        stat = appendSizeMenu( cur->family, cur->size, cur->fsize );
        if ( !( stat & 1 ) ) return stat;

        empty = 0;

      }

    }

  } while ( ptr );

  fclose( f );

  if ( empty ) return FONTINFO_EMPTY;

  return FONTINFO_SUCCESS;

}

int fontInfoClass::addFont (
  char *oneName )
{

int stat, slantLoc;
fontNameListPtr cur;
char *tk, *ctx, spec[127+1], family[63+1], weight[31+1],
 slant[31+1], pixels[31+1], size[31+1];
int dup;

  stat = avl_get_match( this->fontNameListH, (void *) oneName,
   (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( cur ) return FONTINFO_SUCCESS;

  strncpy( spec, oneName, 127 );

  ctx = NULL;
  tk = strtok_r( spec, "-", &ctx );
  if ( !tk ) return FONTINFO_FAIL;
  strncpy( family, tk, 63 );

  tk = strtok_r( NULL, "-", &ctx );
  if ( !tk ) return FONTINFO_FAIL;
  strncpy( weight, tk, 31 );

  tk = strtok_r( NULL, "-", &ctx );
  if ( !tk ) return FONTINFO_FAIL;
  strncpy( slant, tk, 31 );

  tk = strtok_r( NULL, "-", &ctx );
  if ( !tk ) return FONTINFO_FAIL;
  strncpy( pixels, tk, 31 );

  tk = strtok_r( NULL, "-", &ctx );
  if ( !tk ) return FONTINFO_FAIL;
  strncpy( size, tk, 31 );

  // slant here is always 'i' but the actual font spec may use 'o' so
  // build the font spec first with i then with o

  strncpy( spec, "-*-", 127 ); // don't care about foundry
  Strncat( spec, family, 127 );
  Strncat( spec, "-", 127 );
  Strncat( spec, weight, 127 );
  Strncat( spec, "-", 127 );
  slantLoc = strlen(spec);
  Strncat( spec, "?", 127 ); // will hold slant
  Strncat( spec, "-", 127 );
  Strncat( spec, "normal", 127 );
  Strncat( spec, "--", 127 );
  Strncat( spec, pixels, 127 );
  Strncat( spec, "-", 127 );
  Strncat( spec, size, 127 );
  Strncat( spec, "-*-*-*-*-*-*", 127 );

  cur = new fontNameListType;

  if ( strcmp( slant, "i" ) ) {

    spec[slantLoc] = 'i';

//     fprintf( stderr, "trying [%s]\n", spec );
    stat = this->resolveOneFont( spec, cur );
    if ( ( stat & 1 ) ) {
      goto success;
    }

    spec[slantLoc] = 'o';

//     fprintf( stderr, "trying [%s]\n", spec );
    stat = this->resolveOneFont( spec, cur );
    if ( ( stat & 1 ) ) {
      goto success;
    }

//     fprintf( stderr, "last try [%s]\n", spec );
    stat = this->resolveFont( spec, cur );
    if ( ( stat & 1 ) ) {
      goto success;
    }

    delete cur;
    return FONTINFO_NO_FONT;

  }
  else {

    spec[slantLoc] = 'r';

//     fprintf( stderr, "trying [%s]\n", spec );
    stat = this->resolveOneFont( spec, cur );
    if ( ( stat & 1 ) ) {
      goto success;
    }

//     fprintf( stderr, "last try [%s]\n", spec );
    stat = this->resolveFont( spec, cur );
    if ( ( stat & 1 ) ) {
      goto success;
    }

    delete cur;
    return FONTINFO_NO_FONT;

  }

success:

//   fprintf( stderr, "success\n" );

  stat = avl_insert_node( this->fontNameListH, (void *) cur, &dup );
  if ( !( stat & 1 ) ) return stat;
//   if ( dup ) fprintf( stderr, "duplicate\n" );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::getFontName (
  char *fontTag,
  double rotation,
  char *name,
  int len )
{

int stat;
fontNameListPtr cur;
double radians = rotation * 0.017453;
double s, c, pixels;
double term;
char buf[127+1], tmp[31+1], matrix[63+1], sign[2], *tk, *context;

  stat = avl_get_match( this->fontNameListH, (void *) fontTag,
   (void **) &cur );
  if ( !(stat & 1) ) return 0;
  if ( !cur ) return 0;

  if ( rotation == 0.0 ) {
    strncpy( name, cur->fullName, len );
    name[len] = 0;
    return 1;
  }

  s = sin(radians);
  c = cos(radians);

  strncpy( buf, cur->fullName, 127 );

  strncpy( name, "-", len );

  context = NULL;
  tk = strtok_r( buf, "-", &context );		// foundary
  Strncat( name, tk, len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// name
  Strncat( name, tk, len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// weight
  Strncat( name, tk, len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// slant
  Strncat( name, tk, len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// set width
  Strncat( name, tk, len );
  Strncat( name, "-", len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// points
  Strncat( name, "0", len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// pixels

  strncpy( matrix, "[", 63 );

  pixels = atof( tk ) / 10.0;

  if ( c < 0.0 ) {
    strcpy( sign, "~" );
  }
  else {
    strcpy( sign, "+" );
  }

  term = fabs(c) * pixels;
  sprintf( tmp, "%s%-.1f", sign, term );
  fixFontSize( tmp );
  Strncat( matrix, tmp, 63 );

  if ( s < 0.0 ) {
    strcpy( sign, "~" );
  }
  else {
    strcpy( sign, "+" );
  }

  term = fabs(s) * pixels;
  sprintf( tmp, "%s%-.1f", sign, term );
  fixFontSize( tmp );
  Strncat( matrix, tmp, 63 );

  if ( s < 0.0 ) {
    strcpy( sign, "+" );
  }
  else {
    strcpy( sign, "~" );
  }

  term = fabs(s) * pixels;
  sprintf( tmp, "%s%-.1f", sign, term );
  fixFontSize( tmp );
  Strncat( matrix, tmp, 63 );

  if ( c < 0.0 ) {
    strcpy( sign, "~" );
  }
  else {
    strcpy( sign, "+" );
  }

  term = fabs(c) * pixels;
  sprintf( tmp, "%s%-.1f", sign, term );
  fixFontSize( tmp );
  Strncat( matrix, tmp, 63 );

  Strncat( matrix, "]", 63 );

  Strncat( name, matrix, len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// horz res
  Strncat( name, tk, len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// vert res
  Strncat( name, tk, len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// spacing
  Strncat( name, tk, len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// average width
  Strncat( name, "0", len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// char set
  Strncat( name, tk, len );

  tk = strtok_r( NULL, "-", &context );		// char set
  if ( tk ) {
    Strncat( name, "-", len );
    Strncat( name, tk, len );
  }

  tk = strtok_r( NULL, "-", &context );		// char set
  if ( tk ) {
    Strncat( name, "-", len );
    Strncat( name, tk, len );
  }

  name[len] = 0;

  return 1;

}

XFontStruct *fontInfoClass::getXFontStruct (
  char *name )
{

int stat;
fontNameListPtr cur;
XmFontListEntry entry;

  stat = avl_get_match( this->fontNameListH, (void *) name, (void **) &cur );
  if ( !(stat & 1) ) return (XFontStruct *) NULL;

  if ( !cur ) return (XFontStruct *) NULL;

  if ( cur->fontLoaded ) return cur->fontStruct;


  cur->fontStruct = XLoadQueryFont( this->display, cur->fullName );

  entry = XmFontListEntryLoad( this->display, cur->fullName,
   XmFONT_IS_FONT, cur->name );


  if ( entry ) {


    if ( this->fontListEmpty ) {
      this->fontList = XmFontListAppendEntry( NULL, entry );
      this->fontListEmpty = 0;
    }
    else {
      this->fontList = XmFontListAppendEntry( this->fontList, entry );
    }

    XmFontListEntryFree( &entry );


  }

  if ( cur->fontStruct ) {
    cur->fontLoaded = 1;
  }

  return cur->fontStruct;

}

XFontStruct *fontInfoClass::getXNativeFontStruct (
  char *name )
{

XmFontListEntry entry;
XFontStruct *fs;

  fs = XLoadQueryFont( this->display, name );

  entry = XmFontListEntryLoad( this->display, name,
   XmFONT_IS_FONT, name );

  if ( entry ) {

    if ( this->fontListEmpty ) {
      this->fontList = XmFontListAppendEntry( NULL, entry );
      this->fontListEmpty = 0;
    }
    else {
      this->fontList = XmFontListAppendEntry( this->fontList, entry );
    }

    XmFontListEntryFree( &entry );

  }

  return fs;

}

XmFontList fontInfoClass::getXmFontList ( void )
{

  return this->fontList;

}

int fontInfoClass::loadFontTag (
  char *name )
{

int stat;
fontNameListPtr cur;
XmFontListEntry entry;

  stat = avl_get_match( this->fontNameListH, (void *) name, (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( !cur ) return FONTINFO_FAIL;

  if ( cur->fontLoaded ) return FONTINFO_SUCCESS;

  cur->fontStruct = XLoadQueryFont( this->display, cur->fullName );

  entry = XmFontListEntryLoad( this->display, cur->fullName,
   XmFONT_IS_FONT, cur->name );


  if ( entry ) {

    if ( this->fontListEmpty ) {
      this->fontList = XmFontListAppendEntry( NULL, entry );
      this->fontListEmpty = 0;
    }
    else {
      this->fontList = XmFontListAppendEntry( this->fontList, entry );
    }

    XmFontListEntryFree( &entry );


  }

  if ( cur->fontStruct ) {
    cur->fontLoaded = 1;
  }

  return FONTINFO_SUCCESS;

}

int fontInfoClass::getTextFontList (
  char *name,
  XmFontList *oneFontList )
{

int stat;
fontNameListPtr cur;
XmFontListEntry entry;

  stat = avl_get_match( this->fontNameListH, (void *) name, (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( !cur ) return FONTINFO_FAIL;

  if ( !cur->fontLoaded ) {
    stat = this->loadFontTag( name );
    if ( !( stat & 1 ) ) return FONTINFO_FAIL;
  }


  entry = XmFontListEntryLoad( this->display, cur->fullName,
   XmFONT_IS_FONT, cur->name );


  if ( entry ) {

    *oneFontList = XmFontListAppendEntry( NULL, entry );

    XmFontListEntryFree( &entry );


  }
  else {

    return FONTINFO_FAIL;

  }

  return FONTINFO_SUCCESS;

}

int fontInfoClass::getFirstFontMapping (
  char *tag,
  int tagMax,
  char *spec,
  int specMax
) {

int stat;
fontNameListPtr cur;

  stat = avl_get_first( this->fontNameListH, (void **) &cur );
  if ( !( stat & 1 ) || !cur ) {
    return 0;
  }

  strncpy( tag, cur->name, tagMax );
  tag[tagMax] = 0;

  strncpy( spec, cur->fullName, specMax );
  spec[specMax] = 0;

  return 1;

}

int fontInfoClass::getNextFontMapping (
  char *tag,
  int tagMax,
  char *spec,
  int specMax
) {

int stat;
fontNameListPtr cur;

  stat = avl_get_next( this->fontNameListH, (void **) &cur );
  if ( !( stat & 1 ) || !cur ) {
    return 0;
  }

  strncpy( tag, cur->name, tagMax );
  tag[tagMax] = 0;

  strncpy( spec, cur->fullName, specMax );
  spec[specMax] = 0;

  return 1;

}

int fontInfoClass::initFromFileVer5 (
  XtAppContext app,
  Display *d,
  FILE *f,
  int major,
  int minor,
  int release )
{

char line[255+1], buf[255+1], userFontFamilyName[63+1], *ptr, *tk1, *ctx1;
int stat;
int empty = 1;

  ptr = getStrFromFile( line, 255, f );
  if ( !ptr ) {
    fclose( f );
    fprintf( stderr, fontInfoClass_str3, lastNonCommentLine );
    return FONTINFO_EMPTY;
  }

  strncpy( defSiteFontTag, line, 127 );
  defSiteFontTag[127] = 0;
  defSiteFontTag[strlen(defSiteFontTag)-1] = 0; // discard \n

  ptr = getStrFromFile( line, 255, f );
  if ( !ptr ) {
    fclose( f );
    fprintf( stderr, fontInfoClass_str4, lastNonCommentLine );
    return FONTINFO_EMPTY;
  }

  strncpy( defFontTag, line, 127 );
  defFontTag[127] = 0;
  defFontTag[strlen(defFontTag)-1] = 0; // discard \n

  do {

    processAllEvents( app, display );

    ptr = getStrFromFile ( line, 255, f );
    if ( ptr ) {

      empty = 0;

      ctx1 = NULL;
      strcpy( buf, line );

      tk1 = strtok_r( buf, "=\t\n(){", &ctx1 );
      if ( tk1 ) {

        if ( strncmp( tk1, "substitutions", 13 ) == 0 ) {

          stat = readSubstitutions( f );
          if ( stat != FONTINFO_SUCCESS ) {
            fclose( f );
            return stat;
          }

	}
	else {

          strncpy( userFontFamilyName, tk1, 63 );
          userFontFamilyName[63] = 0;

          tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
          if ( tk1 ) {

            if ( strcmp( tk1, "{" ) == 0 ) { // font groups

              stat = processFontGroupVer5( app, d, userFontFamilyName, f,
               major, minor, release );
              if ( !( stat & 1 ) ) {
                fclose( f );
                return stat;
              }

            }
            else {

              // tk1 points to first character after "<name>="

              strcpy( buf, line );

              stat = getSingleFontSpecVer5( app, d, userFontFamilyName, tk1,
               major, minor, release );
              if ( !( stat & 1 ) ) {
                fclose( f );
                return stat;
              }

            }

          }
          else {
            fclose( f );
            fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
            return FONTINFO_SYNTAX;
          }

        }

      }
      else {

        fclose( f );
        fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
        return FONTINFO_SYNTAX;

      }

    }

  } while ( ptr );

  fclose( f );

  if ( empty ) {
    fprintf( stderr, fontInfoClass_str6 );
    return FONTINFO_EMPTY;
  }

  return FONTINFO_SUCCESS;

}

int fontInfoClass::resolveFontVer5 (
  char *fontSpec,
  char *sizeLabel,
  fontNameListPtr ptr ) {

int n, isize, isScalable;
float fsize;
char **list;
char *tk, *ctx, spec[127+1], name[127+1], family[63+1], weight[31+1],
 slant[31+1], size[31+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    list = findBestFont( this->display, fontSpec, &n );
    if ( n == 0 ) {
      return FONTINFO_NO_FONT;
    }
  }

  strncpy( spec, list[0], 127 );

  if ( ( debugMode() == 1000 ) || ( debugMode() == 1001 ) ) fprintf( stderr, "1 Font Spec: [%s]\n", spec );

  ctx = NULL;
  tk = strtok_r( spec, "-", &ctx );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( family, tk, 63 );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( weight, tk, 31 );

  tk = strtok_r( NULL, "-", &ctx );
  if ( strcmp( tk, "r" ) == 0 )
    strncpy( slant, "r", 31 );
  else
    strncpy( slant, "i", 31 );

  tk = strtok_r( NULL, "-", &ctx );
  tk = strtok_r( NULL, "-", &ctx );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( size, tk, 31 );
  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );

  if ( sizeLabel ) {
    fsize = atof( sizeLabel );
  }
  else {
    fsize = atof( size );
  }

  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );
  fixFontSize( size );

  strncpy( name, family, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(family)+1];
  strcpy( ptr->family, family );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::resolveFontVer5 (
  char *fontSpec,
  char *sizeLabel,
  char *userFontFamilyName,
  fontNameListPtr ptr ) {

int n, isize, isScalable, stat;
float fsize;
char **list;
char spec[127+1], name[127+1], foundary[63+1], family[63+1], weight[63+1],
 slant[63+1], size[63+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    if ( requireExactMatch ) {
      fprintf( stderr, fontInfoClass_str8, fontSpec );
      fprintf( stderr, fontInfoClass_str9, lastNonCommentLine );
      return FONTINFO_NO_FONT;
    }
    else {
      list = findBestFont( this->display, fontSpec, &n );
      if ( n == 0 ) {
        fprintf( stderr, fontInfoClass_str8, fontSpec );
        fprintf( stderr, fontInfoClass_str9, lastNonCommentLine );
        return FONTINFO_NO_FONT;
      }
    }
  }

  strncpy( spec, list[0], 127 );

  if ( ( debugMode() == 1000 ) || ( debugMode() == 1001 ) ) fprintf( stderr, "2 Font Spec: [%s]\n", spec );

  stat = parseFontSpec( spec, foundary, family, weight, slant, size );

  if ( strcmp( weight, mediumString ) == 0 ) {
    strcpy( weight, "medium" );
  }
  else if ( strcmp( weight, boldString ) == 0 ) {
    strcpy( weight, "bold" );
  }
  else {
    strcpy( weight, "medium" );
  }

  if ( strcmp( slant, regularString ) == 0 ) {
    strcpy( slant, "r" );
  }
  else if ( strcmp( slant, italicString ) == 0 ) {
    strcpy( slant, "i" );
  }
  else {
    strcpy( slant, "r" );
  }

  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );

  if ( sizeLabel ) {
    fsize = atof( sizeLabel );
  }
  else {
    fsize = atof( size );
  }

  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );
  fixFontSize( size );

  strncpy( name, userFontFamilyName, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  //fprintf( stderr, "name=[%s]\n", name );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(userFontFamilyName)+1];
  strcpy( ptr->family, userFontFamilyName );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::resolveFontVer5 (
  char *fontSpec,
  char *sizeLabel,
  char *userFontFamilyName,
  char *useWeight,
  char *useSlant,
  fontNameListPtr ptr ) {

int n, isize, isScalable, stat;
float fsize;
char **list;
char spec[127+1], name[127+1], foundary[63+1], family[63+1], weight[63+1],
 slant[63+1], size[63+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    if ( requireExactMatch ) {
      fprintf( stderr, fontInfoClass_str8, fontSpec );
      fprintf( stderr, fontInfoClass_str9, lastNonCommentLine );
      return FONTINFO_NO_FONT;
    }
    else {
      list = findBestFont( this->display, fontSpec, &n );
      if ( n == 0 ) {
        fprintf( stderr, fontInfoClass_str8, fontSpec );
        fprintf( stderr, fontInfoClass_str9, lastNonCommentLine );
        return FONTINFO_NO_FONT;
      }
    }
  }

  strncpy( spec, list[0], 127 );

  if ( ( debugMode() == 1000 ) || ( debugMode() == 1001 ) ) fprintf( stderr, "3 Font Spec: [%s]\n", spec );

  stat = parseFontSpec( spec, foundary, family, weight, slant, size );

  if ( strcmp( useWeight, mediumString ) == 0 ) {
    strcpy( weight, "medium" );
  }
  else if ( strcmp( useWeight, boldString ) == 0 ) {
    strcpy( weight, "bold" );
  }
  else {
    strcpy( weight, "medium" );
  }

  if ( strcmp( useSlant, regularString ) == 0 ) {
    strcpy( slant, "r" );
  }
  else if ( strcmp( useSlant, italicString ) == 0 ) {
    strcpy( slant, "i" );
  }
  else {
    strcpy( slant, "r" );
  }

  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );

  if ( sizeLabel ) {
    fsize = atof( sizeLabel );
  }
  else {
    fsize = atof( size );
  }

  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );
  fixFontSize( size );

  strncpy( name, userFontFamilyName, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  //fprintf( stderr, "name=[%s]\n", name );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(userFontFamilyName)+1];
  strcpy( ptr->family, userFontFamilyName );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}

int fontInfoClass::resolveOneFontVer5 (
  char *fontSpec,
  char *sizeLabel,
  fontNameListPtr ptr ) {

int n, isize, isScalable;
float fsize;
char **list;
char *tk, *ctx, spec[127+1], name[127+1], family[63+1], weight[31+1],
 slant[31+1], size[31+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    return FONTINFO_NO_FONT;
  }

  strncpy( spec, list[0], 127 );

  if ( ( debugMode() == 1000 ) || ( debugMode() == 1001 ) ) fprintf( stderr, "resolveOneFont - Spec: [%s]\n", spec );

  ctx = NULL;
  tk = strtok_r( spec, "-", &ctx );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( family, tk, 63 );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( weight, tk, 31 );

  tk = strtok_r( NULL, "-", &ctx );
  if ( strcmp( tk, "r" ) == 0 )
    strncpy( slant, "r", 31 );
  else
    strncpy( slant, "i", 31 );

  tk = strtok_r( NULL, "-", &ctx );
  tk = strtok_r( NULL, "-", &ctx );

  tk = strtok_r( NULL, "-", &ctx );
  strncpy( size, tk, 31 );
  if ( strcmp( size, "0" ) == 0 )
    isScalable = 1;
  else
    isScalable = 0;

  isize = atol( size );

  if ( sizeLabel ) {
    fsize = atof( sizeLabel );
  }
  else {
    fsize = atof( size );
  }

  fsize /= 10;
  ptr->size = isize;
  ptr->fsize = fsize;

  sprintf( size, "%-.1f", fsize );
  fixFontSize( size );

  strncpy( name, family, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, weight, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, slant, 127 );
  Strncat( name, "-", 127 );
  Strncat( name, size, 127 );

  ptr->isScalable = (char) isScalable;

  ptr->fullName = new char[strlen(list[0])+1];
  strcpy( ptr->fullName, list[0] );

  ptr->name = new char[strlen(name)+1];
  strcpy( ptr->name, name );

  ptr->family = new char[strlen(family)+1];
  strcpy( ptr->family, family );

  ptr->weight = weight[0];

  ptr->slant = slant[0];

  XFreeFontNames( list );

  return FONTINFO_SUCCESS;

}


int fontInfoClass::checkSingleFontSpecGenericVer5 (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int checkBestFont,
  int major,
  int minor,
  int release )
{

char buf[255+1], t1[255+1], t2[255+1], t3[255+1], t4[255+1],
 t5[255+1], t6[255+1], t7[255+1], mod[4][255+1], fontSpec[255+1],
 buf2[31+1], *tk1, *tk2, *tk3, *ctx1, *ctx2, *ctx3;
int i, ii, iii, n, numSizes, pointSize[200];
int preload;
char **list;

  strncpy( buf, line, 255 );

  ctx1 = NULL;
  tk1 = strtok_r( buf, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t1, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t2, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get bold and medium indicators

    ctx2 = NULL;
    tk2 = strtok_r( t2, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[0], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[1], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t3, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t4, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get italic and regular indicators

    ctx2 = NULL;
    tk2 = strtok_r( t4, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[2], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[3], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t5, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t6, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get point sizes
    numSizes = 0;
    ctx2 = NULL;
    tk2 = strtok_r( t6, ",", &ctx2 );
    if ( tk2 ) {

      if ( strstr( tk2, "=" ) ) {

        strncpy( buf2, tk2, 31 );
        buf2[31] = 0;

        ctx3 = NULL;
        tk3 = strtok_r( buf2, "=", &ctx3 );
        if ( !tk3 ) {
          fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
          return FONTINFO_SYNTAX;
        }

        tk3 = strtok_r( NULL, "=", &ctx3 );
        if ( !tk3 ) {
          fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
          return FONTINFO_SYNTAX;
        }

        pointSize[numSizes] = atol( tk3 );
        numSizes++;
        if ( numSizes >= 200 ) {
          fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
          return FONTINFO_TOOMANYSIZES;
        }

      }
      else {

        pointSize[numSizes] = atol( tk2 );
        numSizes++;
        if ( numSizes >= 200 ) {
          fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
          return FONTINFO_TOOMANYSIZES;
        }

      }

    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    do {

      tk2 = strtok_r( NULL, ",", &ctx2 );
      if ( tk2 ) {

        if ( strstr( tk2, "=" ) ) {

          strncpy( buf2, tk2, 31 );
          buf2[31] = 0;

          ctx3 = NULL;
          tk3 = strtok_r( buf2, "=", &ctx3 );
          if ( !tk3 ) {
            fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
            return FONTINFO_SYNTAX;
          }

          tk3 = strtok_r( NULL, "=", &ctx3 );
          if ( !tk3 ) {
            fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
            return FONTINFO_SYNTAX;
          }

          pointSize[numSizes] = atol( tk3 );
          numSizes++;
          if ( numSizes >= 200 ) {
            fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
            return FONTINFO_TOOMANYSIZES;
          }

        }
        else {

          pointSize[numSizes] = atol( tk2 );
          numSizes++;
          if ( numSizes >= 200 ) {
            fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
            return FONTINFO_TOOMANYSIZES;
          }

        }

      }

    } while ( tk2 );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t7, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  preload = 0;
  requireExactMatch = 0;

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  //fprintf( stderr, "t1 = [%s]\n", t1 );
  //fprintf( stderr, "  mod[0] = [%s]\n", mod[0] );
  //fprintf( stderr, "  mod[1] = [%s]\n", mod[1] );
  //fprintf( stderr, "t3 = [%s]\n", t3 );
  //fprintf( stderr, "  mod[2] = [%s]\n", mod[2] );
  //fprintf( stderr, "  mod[3] = [%s]\n", mod[3] );
  //fprintf( stderr, "t5 = [%s]\n", t5 );

  //for ( i=0; i<numSizes; i++ ) {
  //  fprintf( stderr, "  size[%-d] = %-d\n", i, pointSize[i] );
  //}

  //fprintf( stderr, "t7 = [%s]\n", t7 );

  // Build fontspec

  for ( i=0; i<2; i++ ) {

    for ( ii=2; ii<4; ii++ ) {

      for ( iii=0; iii<numSizes; iii++ ) {

        sprintf( fontSpec, "%s%s%s%s%s%-d%s", t1, mod[i], t3, mod[ii],
         t5, pointSize[iii], t7 );

        //fprintf( stderr, "checkSingleFontSpec : [%s]\n", fontSpec );

        if ( fontMap ) {
          FontMapType::iterator it = fontMap->begin();
          while ( it != fontMap->end() ) {
	    std::string f = it->first;
	    std::string s = it->second;
            if ( debugMode() == 1001 ) {
              fprintf( stderr, "1 Check [%s] against potential sub [%s]\n", fontSpec, f.c_str() );
            }
            if ( strcmp( fontSpec, f.c_str() ) == 0 ) {
              if ( ( debugMode() == 1000 ) || ( debugMode() == 1001 ) ) {
                fprintf( stderr, "  1 Substitution match - perform font substitution:\n" );
                fprintf( stderr, "    [%s] will be replaced with [%s]\n", fontSpec, s.c_str() );
              }
              strcpy( fontSpec, s.c_str() );
	      break;
	    }
	    it++;
	  }
        }

        list = XListFonts( display, fontSpec, 1, &n );
        if ( n == 0 ) {
          if ( checkBestFont && !requireExactMatch ) {
            list = findBestFont( this->display, fontSpec, &n );
            if ( n == 0 ) {
              fprintf( stderr, fontInfoClass_str8, fontSpec );
              fprintf( stderr, fontInfoClass_str9, lastNonCommentLine );
              return FONTINFO_NO_FONT;
            }
          }
          else {
            return FONTINFO_NO_FONT;
	  }
        }

        XFreeFontNames( list );

      }

    }

  }

  return FONTINFO_SUCCESS;

}

int fontInfoClass::checkSingleFontSpecVer5 (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int major,
  int minor,
  int release )
{

int checkBest = 0;

  return checkSingleFontSpecGenericVer5( app, d, userFontFamilyName,
   line, checkBest, major, minor, release );

}

int fontInfoClass::checkBestSingleFontSpecVer5 (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *line,
  int major,
  int minor,
  int release )
{

int checkBest = 1;

  return checkSingleFontSpecGenericVer5( app, d, userFontFamilyName,
   line, checkBest, major, minor, release );

}

int fontInfoClass::getSingleFontSpecVer5 (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  char *buf,
  int major,
  int minor,
  int release )
{

char t1[255+1], t2[255+1], t3[255+1], t4[255+1],
 t5[255+1], t6[255+1], t7[255+1], mod[4][255+1], fontSpec[255+1],
 buf2[31+1], sizeLabel[200][15+1], *tk1, *tk2, *tk3, *ctx1, *ctx2, *ctx3;
int useSubstitution;
int i, ii, iii, pointSize[200], numSizes;
int stat, preload;
int empty = 1;
fontNameListPtr cur;
int dup;
XFontStruct *fs;

  ctx1 = NULL;
  tk1 = strtok_r( buf, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t1, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t2, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

    // get medium and bold modifiers

    ctx2 = NULL;
    tk2 = strtok_r( t2, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[0], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    setMediumString( mod[0] );

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {
      strcpy( mod[1], tk2 );
    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

    setBoldString( mod[1] );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t3, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t4, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  // get italic and regular indicators

  ctx2 = NULL;
  tk2 = strtok_r( t4, ",", &ctx2 );
  if ( tk2 ) {
    strcpy( mod[2], tk2 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  setRegularString( mod[2] );

  tk2 = strtok_r( NULL, ",", &ctx2 );
  if ( tk2 ) {
    strcpy( mod[3], tk2 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  setItalicString( mod[3] );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t5, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t6, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  // get point sizes
  numSizes = 0;
  ctx2 = NULL;
  tk2 = strtok_r( t6, ",", &ctx2 );
  if ( tk2 ) {

    if ( strstr( tk2, "=" ) ) {

      strncpy( buf2, tk2, 31 );
      buf2[31] = 0;

      ctx3 = NULL;
      tk3 = strtok_r( buf2, "=", &ctx3 );
      if ( !tk3 ) {
        fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
        return FONTINFO_SYNTAX;
      }

      strncpy( sizeLabel[numSizes], tk3, 15 );
      sizeLabel[numSizes][15] = 0;

      tk3 = strtok_r( NULL, "=", &ctx3 );
      if ( !tk3 ) {
        fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
        return FONTINFO_SYNTAX;
      }

      pointSize[numSizes] = atol( tk3 );
      numSizes++;
      if ( numSizes >= 200 ) {
        fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
        return FONTINFO_TOOMANYSIZES;
      }

    }
    else {

      strncpy( sizeLabel[numSizes], tk2, 15 );
      sizeLabel[numSizes][15] = 0;
      pointSize[numSizes] = atol( tk2 );
      numSizes++;
      if ( numSizes >= 200 ) {
        fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
        return FONTINFO_TOOMANYSIZES;
      }

    }

  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  do {

    tk2 = strtok_r( NULL, ",", &ctx2 );
    if ( tk2 ) {

      if ( strstr( tk2, "=" ) ) {

        strncpy( buf2, tk2, 31 );
        buf2[31] = 0;

        ctx3 = NULL;
        tk3 = strtok_r( buf2, "=", &ctx3 );
        if ( !tk3 ) {
          fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
          return FONTINFO_SYNTAX;
        }

        strncpy( sizeLabel[numSizes], tk3, 15 );
        sizeLabel[numSizes][15] = 0;

        tk3 = strtok_r( NULL, "=", &ctx3 );
        if ( !tk3 ) {
          fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
          return FONTINFO_SYNTAX;
        }

        pointSize[numSizes] = atol( tk3 );
        numSizes++;
        if ( numSizes >= 200 ) {
          fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
          return FONTINFO_TOOMANYSIZES;
        }

      }
      else {

        strncpy( sizeLabel[numSizes], tk2, 15 );
        sizeLabel[numSizes][15] = 0;
        pointSize[numSizes] = atol( tk2 );
        numSizes++;
        if ( numSizes >= 200 ) {
          fprintf( stderr, fontInfoClass_str7, lastNonCommentLine );
          return FONTINFO_TOOMANYSIZES;
        }

      }

    }

  } while ( tk2 );

  tk1 = strtok_r( NULL, "\t\n()", &ctx1 );
  if ( tk1 ) {
    strcpy( t7, tk1 );
  }
  else {
    fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
    return FONTINFO_SYNTAX;
  }

  preload = 0;
  requireExactMatch = 0;

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  tk1 = strtok_r( NULL, "\t\n", &ctx1 );
  if ( tk1 ) {
    if ( strcmp( tk1, "preload" ) == 0 ) {
      preload = 1;
    }
    else if ( strcmp( tk1, "exact" ) == 0 ) {
      requireExactMatch = 1;
    }
  }

  //fprintf( stderr, "preload = %-d\n", preload );
  //fprintf( stderr, "exact = %-d\n", requireExactMatch );

  //fprintf( stderr, "t1 = [%s]\n", t1 );
  //fprintf( stderr, "  mod[0] = [%s]\n", mod[0] );
  //fprintf( stderr, "  mod[1] = [%s]\n", mod[1] );
  //fprintf( stderr, "t3 = [%s]\n", t3 );
  //fprintf( stderr, "  mod[2] = [%s]\n", mod[2] );
  //fprintf( stderr, "  mod[3] = [%s]\n", mod[3] );
  //fprintf( stderr, "t5 = [%s]\n", t5 );

  //for ( i=0; i<numSizes; i++ ) {
  //  fprintf( stderr, "  size[%-d] = %-d\n", i, pointSize[i] );
  //}

  //fprintf( stderr, "t7 = [%s]\n", t7 );

  // Build fontspec

  for ( i=0; i<2; i++ ) {

    for ( ii=2; ii<4; ii++ ) {

      for ( iii=0; iii<numSizes; iii++ ) {

        sprintf( fontSpec, "%s%s%s%s%s%-d%s", t1, mod[i], t3, mod[ii],
         t5, pointSize[iii], t7 );

        //fprintf( stderr, "getSingleFontSpec : [%s]\n", fontSpec );

        useSubstitution = 0;

        if ( fontMap ) {
          FontMapType::iterator it = fontMap->begin();
          while ( it != fontMap->end() ) {
	    std::string f = it->first;
	    std::string s = it->second;
            if ( debugMode() == 1001 ) {
              fprintf( stderr, "2 Check [%s] against potential sub [%s]\n", fontSpec, f.c_str() );
            }
            if ( strcmp( fontSpec, f.c_str() ) == 0 ) {
              if ( ( debugMode() == 1000 ) || ( debugMode() == 1001 ) ) {
                fprintf( stderr, "  2 Substitution match - perform font substitution:\n" );
                fprintf( stderr, "    [%s] will be replaced with [%s]\n", fontSpec, s.c_str() );
              }
              strcpy( fontSpec, s.c_str() );
              useSubstitution = 1;
	      break;
	    }
	    it++;
	  }
        }

        cur = new fontNameListType;

	if ( useSubstitution ) {
          stat = this->resolveFontVer5( fontSpec, sizeLabel[iii], userFontFamilyName, mod[i], mod[ii], cur );
          if ( !( stat & 1 ) ) {
            delete cur;
            return stat;
          }
	}
	else {
          stat = this->resolveFontVer5( fontSpec, sizeLabel[iii], userFontFamilyName, cur );
          if ( !( stat & 1 ) ) {
            delete cur;
            return stat;
          }
	}

        stat = avl_insert_node( this->fontNameListH, (void *) cur,
         &dup );
        if ( !( stat & 1 ) ) {
          fprintf( stderr, fontInfoClass_str11, __LINE__, __FILE__ );
          return stat;
	}
        if ( dup ) {
          fprintf( stderr, fontInfoClass_str13, cur->name, lastNonCommentLine );
	}

        if ( preload ) {
          //fprintf( stderr, "preload %s\n", cur->name );
          fs = getXFontStruct( cur->name );
        }

        stat = appendSizeMenu( cur->family, cur->size, cur->fsize );
        if ( !( stat & 1 ) ) return stat;
         empty = 0;

      }

    }

  }

  return 1;

}


int fontInfoClass::processFontGroupVer5 (
  XtAppContext app,
  Display *d,
  char *userFontFamilyName,
  FILE *f,
  int major,
  int minor,
  int release )
{

char line[255+1], buf[255+1], lastLine[255+1], *ptr, *tk1, *ctx1;
int stat;
int foundBrace = 0;

  strcpy( lastLine, "" );
  stat = FONTINFO_GROUPSYNTAX; // in case all lines are blank

  do {

    processAllEvents( app, display );

    ptr = getStrFromFile ( line, 255, f );
    if ( ptr ) {

      ctx1 = NULL;
      strcpy( buf, line );

      tk1 = strtok_r( buf, "\t\n", &ctx1 );
      if ( tk1 ) {
        if ( tk1[0] == '}' ) {
          foundBrace = 1;
	}
	else {
          foundBrace = 0;
	}
      }

      if ( ! foundBrace ) {

        strcpy( lastLine, line );

        stat = checkSingleFontSpecVer5( app, d, userFontFamilyName, line,
         major, minor, release );
        if ( stat & 1 ) {

          if ( ( debugMode() == 1000 ) || ( debugMode() == 1001 ) ) fprintf( stderr, "Using font: %s", line );
          stat = getSingleFontSpecVer5( app, d, userFontFamilyName, line,
           major, minor, release );

          flushToBrace( f );

          return stat; // return stat from getSingleFontSpec

        }
	else {
          if ( ( debugMode() == 1000 ) || ( debugMode() == 1001 ) ) fprintf( stderr, "Font not found: %s", line );
	}

      }

    }
    else {
      fprintf( stderr, fontInfoClass_str5, lastNonCommentLine );
      return FONTINFO_SYNTAX;
    }

  } while ( !foundBrace );

  // If we never found a matching font, try to get something that matches,
  // even badly, using the findBestFont function (in this file)
  stat = checkBestSingleFontSpecVer5( app, d, userFontFamilyName, lastLine,
   major, minor, release );
  if ( stat & 1 ) {
    if ( ( debugMode() == 1000 ) || ( debugMode() == 1001 ) ) fprintf( stderr, "Using font (with substitutions): %s",
     lastLine );
    stat = getSingleFontSpecVer5( app, d, userFontFamilyName, lastLine,
     major, minor, release );
  }

  if ( stat == FONTINFO_GROUPSYNTAX ) {
    fprintf( stderr, fontInfoClass_str12, lastNonCommentLine );
  }

  return stat; // return last stat from checkSingleFontSpec or GROUPSYNTAX

}
