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

#include "thread.h"


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

  fontListEmpty = 1;

}

fontInfoClass::~fontInfoClass ( void ) {   // destructor

// must free all avl nodes

}

static char **findBestFont(
  Display *d,
  char *fontSpec,
  int *n ) {

char **list;
char *tk, spec[127+1], rest[127+1], foundry[63+1], family[63+1], weight[31+1],
 slant[31+1], ftype[31+1], size[31+1], newFont[127+1];

  strncpy( spec, fontSpec, 127 );

  tk = strtok( spec, "-" );
  if ( !tk ) goto err_return;
  strncpy( foundry, tk, 63 );

  tk = strtok( NULL, "-" );
  if ( !tk ) goto err_return;
  strncpy( family, tk, 63 );

  tk = strtok( NULL, "\n" );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, rest, 127 );

  //  printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( spec, rest, 127 );

  tk = strtok( spec, "-" );
  if ( !tk ) goto err_return;
  strncpy( weight, tk, 63 );

  tk = strtok( NULL, "\n" );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, rest, 127 );

//   printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( spec, rest, 127 );

  tk = strtok( spec, "-" );
  if ( !tk ) goto err_return;
  strncpy( slant, tk, 63 );

  tk = strtok( NULL, "\n" );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, "*-", 127 );
  Strncat( newFont, rest, 127 );

//   printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( spec, rest, 127 );

  tk = strtok( spec, "-" );
  if ( !tk ) goto err_return;
  strncpy( ftype, tk, 63 );

  tk = strtok( NULL, "-" );
  if ( !tk ) goto err_return;
  strncpy( size, tk, 31 );

  tk = strtok( NULL, "\n" );
  if ( !tk ) goto err_return;
  strncpy( rest, tk, 127 );

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

//   printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( newFont, "-", 127 );
  Strncat( newFont, foundry, 127 );
  Strncat( newFont, "-*-*-*-*--*-*-*-*-*-*-*-*", 127 );

//   printf( "new font is %s\n", newFont );

  list = XListFonts( d, newFont, 1, n );
  if ( *n == 1 ) return list;

  strncpy( newFont, "-*-*-*-*-*--*-*-*-*-*-*-*-*", 127 );

//   printf( "new font is %s\n", newFont );

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
char *tk, spec[127+1], name[127+1], family[63+1], weight[31+1],
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

  //  printf( "Spec is [%s]\n", spec );

  tk = strtok( spec, "-" );

  tk = strtok( NULL, "-" );
  strncpy( family, tk, 63 );

  tk = strtok( NULL, "-" );
  strncpy( weight, tk, 31 );

  tk = strtok( NULL, "-" );
  if ( strcmp( tk, "r" ) == 0 )
    strncpy( slant, "r", 31 );
  else
    strncpy( slant, "i", 31 );

  tk = strtok( NULL, "-" );
  tk = strtok( NULL, "-" );

  tk = strtok( NULL, "-" );
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

int fontInfoClass::resolveOneFont (
  char *fontSpec,
  fontNameListPtr ptr ) {

int n, isize, isScalable;
float fsize;
char **list;
char *tk, spec[127+1], name[127+1], family[63+1], weight[31+1],
 slant[31+1], size[31+1];

  ptr->fontLoaded = 0;

  list = XListFonts( this->display, fontSpec, 1, &n );
  if ( n == 0 ) {
    return FONTINFO_NO_FONT;
  }

  strncpy( spec, list[0], 127 );

//   printf( "Spec is [%s]\n", spec );

  tk = strtok( spec, "-" );

  tk = strtok( NULL, "-" );
  strncpy( family, tk, 63 );

  tk = strtok( NULL, "-" );
  strncpy( weight, tk, 31 );

  tk = strtok( NULL, "-" );
  if ( strcmp( tk, "r" ) == 0 )
    strncpy( slant, "r", 31 );
  else
    strncpy( slant, "i", 31 );

  tk = strtok( NULL, "-" );
  tk = strtok( NULL, "-" );

  tk = strtok( NULL, "-" );
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

int fontInfoClass::initFromFile (
  XtAppContext app,
  Display *d,
  char *fileName )
{

// Read font specs from given file, query server, and populate data structure.
// If font does not exist, use some other font.

char line[127+1], *ptr, *fontSpec, *tk;
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

  f = fopen( fileName, "r" );
  if ( !f ) {
    return FONTINFO_NO_FILE;
  }

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

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

      fontSpec = strtok( line, "\t\n" );
      if ( fontSpec ) {

        if ( major >= 2 ) {

          tk = strtok( NULL, "\t\n" );
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
//        if ( dup ) printf( "duplicate\n" );

        if ( preload ) {
          //printf( "preload %s\n", cur->name );
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
char *tk, spec[127+1], family[63+1], weight[31+1],
 slant[31+1], pixels[31+1], size[31+1];
int dup;

  stat = avl_get_match( this->fontNameListH, (void *) oneName,
   (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( cur ) return FONTINFO_SUCCESS;

  strncpy( spec, oneName, 127 );

  tk = strtok( spec, "-" );
  if ( !tk ) return FONTINFO_FAIL;
  strncpy( family, tk, 63 );

  tk = strtok( NULL, "-" );
  if ( !tk ) return FONTINFO_FAIL;
  strncpy( weight, tk, 31 );

  tk = strtok( NULL, "-" );
  if ( !tk ) return FONTINFO_FAIL;
  strncpy( slant, tk, 31 );

  tk = strtok( NULL, "-" );
  if ( !tk ) return FONTINFO_FAIL;
  strncpy( pixels, tk, 31 );

  tk = strtok( NULL, "-" );
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

//     printf( "trying [%s]\n", spec );
    stat = this->resolveOneFont( spec, cur );
    if ( ( stat & 1 ) ) {
      goto success;
    }

    spec[slantLoc] = 'o';

//     printf( "trying [%s]\n", spec );
    stat = this->resolveOneFont( spec, cur );
    if ( ( stat & 1 ) ) {
      goto success;
    }

//     printf( "last try [%s]\n", spec );
    stat = this->resolveFont( spec, cur );
    if ( ( stat & 1 ) ) {
      goto success;
    }

    delete cur;
    return FONTINFO_NO_FONT;

  }
  else {

    spec[slantLoc] = 'r';

//     printf( "trying [%s]\n", spec );
    stat = this->resolveOneFont( spec, cur );
    if ( ( stat & 1 ) ) {
      goto success;
    }

//     printf( "last try [%s]\n", spec );
    stat = this->resolveFont( spec, cur );
    if ( ( stat & 1 ) ) {
      goto success;
    }

    delete cur;
    return FONTINFO_NO_FONT;

  }

success:

//   printf( "success\n" );

  stat = avl_insert_node( this->fontNameListH, (void *) cur, &dup );
  if ( !( stat & 1 ) ) return stat;
//   if ( dup ) printf( "duplicate\n" );

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
int term;
char buf[127+1], tmp[31+1], matrix[63+1], sign[2], *tk, *context;

  stat = avl_get_match( this->fontNameListH, (void *) fontTag,
   (void **) &cur );
  if ( !(stat & 1) ) return 0;

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

  tk = strtok_r( NULL, "-", &context );		// pixels
  Strncat( name, "*", len );
  Strncat( name, "-", len );

  strncpy( matrix, "[", 63 );

  pixels = atof( tk );

  if ( c < 0.0 ) {
    strcpy( sign, "~" );
  }
  else {
    strcpy( sign, "+" );
  }

  term = (int) ( fabs(c) * pixels );
  sprintf( tmp, "%s%-d", sign, term );
  Strncat( matrix, tmp, 63 );

  if ( s < 0.0 ) {
    strcpy( sign, "~" );
  }
  else {
    strcpy( sign, "+" );
  }

  term = (int) ( fabs(s) * pixels );
  sprintf( tmp, "%s%-d", sign, term );
  Strncat( matrix, tmp, 63 );

  if ( s < 0.0 ) {
    strcpy( sign, "+" );
  }
  else {
    strcpy( sign, "~" );
  }

  term = (int) ( fabs(s) * pixels );
  sprintf( tmp, "%s%-d", sign, term );
  Strncat( matrix, tmp, 63 );

  if ( c < 0.0 ) {
    strcpy( sign, "~" );
  }
  else {
    strcpy( sign, "+" );
  }

  term = (int) ( fabs(c) * pixels );
  sprintf( tmp, "%s%-d", sign, term );
  Strncat( matrix, tmp, 63 );

  Strncat( matrix, "]", 63 );

  Strncat( name, matrix, len );
  Strncat( name, "-", len );

  tk = strtok_r( NULL, "-", &context );		// points (discard)

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
  Strncat( name, "*", len );
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

int fontInfoClass::show ( void ) {

int stat;
fontNameListPtr cur;
char c1[2], c2[2];

  stat = avl_get_first( this->fontNameListH, (void **) &cur );
  if ( !( stat & 1 ) ) {
    printf( fontInfoClass_str1 );
    return FONTINFO_FAIL;
  }

  while ( cur ) {

    if ( cur->isScalable )
      strcpy( c1, "Y" );
    else
      strcpy( c1, "N" );

    if ( cur->fontLoaded )
      strcpy( c2, "Y" );
    else
      strcpy( c2, "N" );

    stat = avl_get_next( this->fontNameListH, (void **) &cur );
    if ( !( stat & 1 ) ) {
      printf( fontInfoClass_str2 );
      return FONTINFO_FAIL;
    }

  }

  return FONTINFO_SUCCESS;

}
