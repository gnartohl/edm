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

// utility functions

#include "utility.h"
#include <errno.h>

int blank (
  char *string )
{

unsigned int i;

  for ( i=0; i<strlen(string); i++ ) {
    if ( !isspace( (int) string[i] ) ) return 0;
  }

  return 1;

}

void genericProcessAllEvents (
  int sync,
  XtAppContext app,
  Display *d )
{

XEvent Xev;
int result, isXEvent, count;

  count = 1000;

  if ( sync ) {
    XFlush( d );
    XSync( d, False ); /* wait for all X transactions to complete */
  }

  do {
    result = XtAppPending( app );
    if ( result ) {
      isXEvent = XtAppPeekEvent( app, &Xev );
      if ( isXEvent ) {
        if ( Xev.type != Expose ) {
          XtAppProcessEvent( app, result );
	}
        else {
	  // XtAppNextEvent( app, &Xev ); // discard
          XtAppProcessEvent( app, result );
	}
      }
      else { // process all timer or alternate events
        XtAppProcessEvent( app, result );
      }
    }
    count--;
  } while ( result && count );

}

void processAllEventsWithSync (
  XtAppContext app,
  Display *d )
{

  genericProcessAllEvents( 1, app, d );

}

void processAllEvents (
  XtAppContext app,
  Display *d )
{

  genericProcessAllEvents( 0, app, d );

}

static void trimWhiteSpace (
  char *str )
{

char buf[127+1];
int first, last, i, ii, l;

  l = strlen(str);
  if ( l > 126 ) l = 126;

  ii = 0;

  i = 0;
  while ( ( i < l ) && isspace( str[i] ) ) {
    i++;
  }

  first = i;

  i = l-1;
  while ( ( i >= first ) && isspace( str[i] ) ) {
    i--;
  }

  last = i;

  for ( i=first; i<=last; i++ ) {
    buf[ii] = str[i];
    ii++;
  }

  buf[ii] = 0;

  strcpy( str, buf );

}

#define DONE -1

#define SIGN_OR_NUM 1
#define NUM 2

#define SIGN_OR_NUM2 3
#define NUM1 4
#define SIGN_OR_POINT_OR_NUM 5
#define POINT_OR_NUM 6
#define EXP_OR_POINT_OR_NUM 7
#define EXP_OR_NUM 8
#define NUM2 9

int isLegalInteger (
  char *str )
{

char buf[127+1];
int i, l, legal, state;

  strncpy( buf, str, 127 );
  trimWhiteSpace( buf );
  l = strlen(buf);
  if ( l < 1 ) return 0;

  state = SIGN_OR_NUM;
  i = 0;
  legal = 1;
  while ( state != DONE ) {

    if ( i >= l ) state = DONE;

    switch ( state ) {

    case SIGN_OR_NUM:

      if ( buf[i] == '-' ) {
        i++;
        state = NUM;
        continue;
      }
        
      if ( buf[i] == '+' ) {
        i++;
        state = NUM;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        state = NUM;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case NUM:

      if ( isdigit(buf[i]) ) {
        i++;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    }

  }

  return legal;

}

int isLegalFloat (
  char *str )
{

char buf[127+1];
int i, l, legal, state;

  strncpy( buf, str, 127 );
  trimWhiteSpace( buf );
  l = strlen(buf);
  if ( l < 1 ) return 0;

  state = SIGN_OR_POINT_OR_NUM;
  i = 0;
  legal = 1;
  while ( state != DONE ) {

    if ( i >= l ) state = DONE;

    switch ( state ) {

    case SIGN_OR_POINT_OR_NUM:

      if ( buf[i] == '-' ) {
        i++;
        state = POINT_OR_NUM;
        continue;
      }
        
      if ( buf[i] == '+' ) {
        i++;
        state = POINT_OR_NUM;
        continue;
      }
        
      if ( buf[i] == '.' ) {
        i++;
        state = NUM1;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        state = EXP_OR_POINT_OR_NUM;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case NUM1:

      if ( isdigit(buf[i]) ) {
        i++;
        state = EXP_OR_NUM;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case EXP_OR_POINT_OR_NUM:

      if ( buf[i] == 'E' ) {
        i++;
        state = SIGN_OR_NUM2;
        continue;
      }
        
      if ( buf[i] == 'e' ) {
        i++;
        state = SIGN_OR_NUM2;
        continue;
      }
        
      if ( buf[i] == '.' ) {
        i++;
        state = EXP_OR_NUM;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case POINT_OR_NUM:

      if ( buf[i] == '.' ) {
        i++;
        state = EXP_OR_NUM;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        state = EXP_OR_POINT_OR_NUM;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case EXP_OR_NUM:

      if ( buf[i] == 'E' ) {
        i++;
        state = SIGN_OR_NUM2;
        continue;
      }
        
      if ( buf[i] == 'e' ) {
        i++;
        state = SIGN_OR_NUM2;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case SIGN_OR_NUM2:

      if ( buf[i] == '-' ) {
        i++;
        state = NUM2;
        continue;
      }
        
      if ( buf[i] == '+' ) {
        i++;
        state = NUM2;
        continue;
      }
        
      if ( isdigit(buf[i]) ) {
        i++;
        state = NUM2;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    case NUM2:

      if ( isdigit(buf[i]) ) {
        i++;
        continue;
      }

      legal = 0;
      state = DONE;

      break;        

    }

  }

  return legal;

}

int writeStringToFile (
  FILE *f,
  char *str )
{

int stat;

  if ( strcmp( str, "" ) == 0 ) {

    stat = fprintf( f, "<<<empty>>>\n" );
    if ( stat == -1 ) return 0;

  }
  else if ( blank(str) ) {

    {
      char buf[300+1];

      strcpy( buf, "<<<blank>>>" );
      strncat( buf, str, 300 );
      stat = fprintf( f, "%s\n", buf );

    }

  }

  else {

    stat = fprintf( f, "%s\n", str );
    if ( stat == -1 ) return 0;

  }

  return 1;

}

void readStringFromFile (
  char *str,
  int maxChars,
  FILE *f )
{

char *ptr;
int i, ii, l;

  ptr = fgets( str, maxChars, f );
  if ( !ptr ) {
    strcpy( str, "" );
    return;
  }

  l = strlen(str);
  if ( l > maxChars ) l = maxChars;
  if ( l < 1 ) l = 1;
  str[l-1] = 0;

  if ( strcmp( str, "<<<empty>>>" ) == 0 ) {

    strcpy( str, "" );

  }
  else if ( strncmp( str, "<<<blank>>>", 11 ) == 0 ) {

    {
      char buf[300+1];

      strncpy( buf, str, 300 );
      for ( i=0, ii=11; ii<strlen(str); i++, ii++ ) {
        str[i] = buf[ii];
      }

      if ( i > maxChars ) i = maxChars;
      str[i] = 0;

    }

  }

}

int drawText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stringLength, stringWidth, stringX, stringY;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
    stringY = _y + fs->ascent;
  }
  else {
    stringWidth = 0;
    stringY = _y;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  XDrawString( XtDisplay(widget), XtWindow(widget),
   gc->normGC(), stringX, stringY, value, stringLength );

  return 1;

}

int eraseText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stringLength, stringWidth, stringX, stringY;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
    stringY = _y + fs->ascent;
  }
  else {
    stringWidth = 0;
    stringY = _y;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  XDrawString( XtDisplay(widget), XtWindow(widget),
   gc->eraseGC(), stringX, stringY, value, stringLength );

  return 1;

}

int drawImageText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stringLength, stringWidth, stringX, stringY;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
    stringY = _y + fs->ascent;
  }
  else {
    stringWidth = 0;
    stringY = _y;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  XDrawImageString( XtDisplay(widget), XtWindow(widget),
   gc->normGC(), stringX, stringY, value, stringLength );

  return 1;

}

int eraseImageText (
  Widget widget,
  gcClass *gc,
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value ) {

int stringLength, stringWidth, stringX, stringY;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
    stringY = _y + fs->ascent;
  }
  else {
    stringWidth = 0;
    stringY = _y;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  XDrawImageString( XtDisplay(widget), XtWindow(widget),
   gc->eraseGC(), stringX, stringY, value, stringLength );

  return 1;

}

int textBoundaries (
  XFontStruct *fs,
  int _x,
  int _y,
  int _alignment,
  char *value,
  int *x0,
  int *y0,
  int *x1,
  int *y1 ) {

int stringLength, stringWidth, stringX;

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
  }
  else {
    stringWidth = 0;
  }

  stringX = _x;

  if ( _alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( _alignment == XmALIGNMENT_CENTER )
    stringX = _x - stringWidth/2;
  else if ( _alignment == XmALIGNMENT_END )
    stringX = _x - stringWidth;

  *y0 = _y;
  *y1 = _y + fs->ascent + fs->descent;
  *x0 = stringX;
  *x1 = stringX + stringWidth;

  return 1;

}

int fileIsLocked (
  FILE *f )
{

int stat;
int fd = fileno(f);
struct flock l;

  l.l_type = F_WRLCK;
  l.l_start = 0;
  l.l_whence = SEEK_SET;
  l.l_len = 1;
  l.l_pid = 0;

  stat = fcntl( fd, F_GETLK, &l );

  if ( stat < 0 ) {
    return 1; /* failure, assume file is locked */
  }

  if ( l.l_pid == 0 )
    return 0; /* not locked */
  else
    return 1; /* locked */

}

int lockFile (
  FILE *f )
{

int stat;
int fd = fileno(f);
struct flock l;

  l.l_type = F_WRLCK;
  l.l_start = 0;
  l.l_whence = SEEK_SET;
  l.l_len = 1;

  stat = fcntl( fd, F_SETLK, &l );

  if ( stat < 0 )
    return 0; /* even, failure */
  else
    return 1; /* odd, success */

}

int unlockFile (
  FILE *f )
{

int stat;
int fd = fileno(f);
struct flock l;

  l.l_type = F_UNLCK;
  l.l_start = 0;
  l.l_whence = SEEK_SET;
  l.l_len = 1;

  stat = fcntl( fd, F_SETLK, &l );

  if ( stat < 0 )
    return 0; /* even, failure */
  else
    return 1; /* odd, success */

  return 1;

}

void buildFileName (
  char *inName,
  char *prefix,
  char *postfix,
  char *expandedName,
  int maxSize )
{

char *gotOne;

    gotOne = strstr( inName, "/" );

  if ( gotOne ) {
    strncpy( expandedName, inName, maxSize );
  }
  else {
    strncpy( expandedName, prefix, maxSize );
    strncat( expandedName, inName, maxSize );
  }

  if ( strlen(expandedName) > strlen(postfix) ) {
    if ( strcmp( &expandedName[strlen(expandedName)-strlen(postfix)], postfix )
     != 0 ) {
      strncat( expandedName, postfix, maxSize );
    }
  }
  else {
    strncat( expandedName, postfix, maxSize );
  }

}

int getFileName (
  char *name,
  char *fullName,
  int maxSize )
{

int start, end, i, ii, l, ret_stat;

 if ( !fullName || !name ) {
   ret_stat = 0;
   goto err_return;
 }

  l = strlen(fullName);

  start = 0;

  for ( i=l-1; i>=0; i-- ) {

    if ( fullName[i] == '/' ) {
      start = i+1;
      break;

    }

  }

  end = l-1;

  for ( i=l-1; i>=start; i-- ) {

    if ( fullName[i] == '.' ) {
      end = i-1;
      break;

    }

  }

  strcpy( name, "" );
  for ( i=start, ii=0; (i<=end) && (ii<maxSize); i++, ii++ ) {
    name[ii] = fullName[i];
  }

  if ( ii >= maxSize ) ii = maxSize-1;
  name[ii] = 0;

  return 1;

err_return:

  if ( name ) strcpy( name, "" );

  return ret_stat;

}

int getFilePrefix (
  char *prefix,
  char *fullName,
  int maxSize )
{

int start, end, i, ii, l, ret_stat;

 if ( !fullName || !prefix ) {
   ret_stat = 0;
   goto err_return;
 }

  l = strlen(fullName);

  start = 0;
  end = -1;

  for ( i=l-1; i>=0; i-- ) {

    if ( fullName[i] == '/' ) {
      end = i;
      break;

    }

  }

  strcpy( prefix, "" );
  for ( i=start, ii=0; (i<=end) && (ii<maxSize); i++, ii++ ) {
    prefix[ii] = fullName[i];
  }

  if ( ii >= maxSize ) ii = maxSize-1;
  prefix[ii] = 0;

  return 1;

err_return:

  if ( prefix ) strcpy( prefix, "" );

  return ret_stat;

}

int getFilePostfix (
  char *postfix,
  char *fullName,
  int maxSize )
{

int start, end, i, ii, l, ret_stat;

 if ( !fullName || !postfix ) {
   ret_stat = 0;
   goto err_return;
 }

  l = strlen(fullName);

  start = l;
  end = l-1;

  for ( i=l-1; i>=0; i-- ) {

    if ( fullName[i] == '/' ) break;

    if ( fullName[i] == '.' ) {
      start = i;
      break;

    }

  }

  strcpy( postfix, "" );
  for ( i=start, ii=0; (i<=end) && (ii<maxSize); i++, ii++ ) {
    postfix[ii] = fullName[i];
  }

  if ( ii >= maxSize ) ii = maxSize-1;
  postfix[ii] = 0;

  return 1;

err_return:

  if ( postfix ) strcpy( postfix, "" );

  return ret_stat;

}

char *getNextDataString (
  char *str,
  int max,
  FILE *f )
{

int blankOrComment;
char *gotOne, *tk, *context, buf[255+1];

  do {

    gotOne = fgets( str, max, f );
    if ( !gotOne ) return NULL;

    strncpy( buf, str, 255 );

    context = NULL;
    tk = strtok_r( buf, " \t\n", &context );

    blankOrComment = 0;
    if ( tk ) {
      if ( tk[0] == '#' ) blankOrComment = 1;
    }
    else {
      blankOrComment = 1;
    }

  } while ( blankOrComment );

  return str;

}

// these routines consider an ascii 1 to mean new line

void XDrawStrings (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int h,
  char *str,
  int len )
{

char buf[255+1], *tk, *context;

  strncpy( buf, str, 255 );

  context = NULL;
  tk = strtok_r( buf, "\001", &context );

  while ( tk ) {

    XDrawString( d, w, gc, x, y, tk, strlen(tk) );
    
    tk = strtok_r( NULL, "\001", &context );
    y += h;

  }

}

void XDrawImageStrings (
  Display *d,
  Window w,
  GC gc,
  int x,
  int y,
  int h,
  char *str,
  int len )
{

char buf[255+1], *tk, *context;

  strncpy( buf, str, 255 );

  context = NULL;
  tk = strtok_r( buf, "\001", &context );

  while ( tk ) {

    XDrawImageString( d, w, gc, x, y, tk, strlen(tk) );
    
    tk = strtok_r( NULL, "\001", &context );
    y += h;

  }

}

int countSymbolsAndValues (
  char *string,
  int *total,
  int *maxLen
) {

  // string contains s1=v1,s2=v2,s3=v3,...
  // this routine counts the number of symbol,value pairs
  // maxLen equals the longest string found

char *context, *tk, buf[511+1];

  if ( !string ) return 100; // fail

  strncpy( buf, string, 511 );
  buf[511] = 0;

  *maxLen = 0;
  *total = 0;
  context = NULL;
  tk = strtok_r( buf, ",=", &context );

  while ( tk ) {

    tk = strtok_r( NULL, ",=", &context );
    if ( !tk ) return 101; // missing value

    if ( strlen(tk) > (unsigned int) *maxLen ) *maxLen = (int) strlen(tk);
    (*total)++;

    tk = strtok_r( NULL, ",=", &context );

  }

  return 1;

}

int parseSymbolsAndValues (
  char *string,
  int max,
  char *symbols[],
  char *values[],
  int *numFound
) {

  // string contains s1=v1,s2=v2,s3=v3,...
  // this routine puts s1, s2, s3, ... into symbols
  // and v1, v2, v3, ... into values

int l;
char *context, *tk, buf[511+1];

  if ( !string ) return 100; // fail

  strncpy( buf, string, 511 );
  buf[511] = 0;

  *numFound = 0;
  context = NULL;
  tk = strtok_r( buf, ",=", &context );

  while ( tk ) {

    l = strlen(tk) + 1;
    symbols[*numFound] = new (char)[l];
    strcpy( symbols[*numFound], tk );
    trimWhiteSpace( symbols[*numFound] );

    tk = strtok_r( NULL, ",=", &context );
    if ( !tk ) return 101; // missing value

    l = strlen(tk) + 1;
    values[*numFound] = new (char)[l];
    strcpy( values[*numFound], tk );

    (*numFound)++;

    tk = strtok_r( NULL, ",=", &context );

  }

  return 1;

}

int parseLocalSymbolsAndValues (
  char *string,
  int max,
  int maxLen,
  char *symbols[],
  char *values[],
  int *numFound
) {

  // string contains s1=v1,s2=v2,s3=v3,...
  // this routine puts s1, s2, s3, ... into symbols
  // and v1, v2, v3, ... into values

int l;
char *context, *tk, buf[511+1];

  if ( !string ) return 100; // fail

  strncpy( buf, string, 511 );
  buf[511] = 0;

  *numFound = 0;
  context = NULL;
  tk = strtok_r( buf, ",=", &context );

  while ( tk ) {

    l = strlen(tk) + 1;
    strncpy( symbols[*numFound], tk, maxLen );
    symbols[*numFound][maxLen] = 0;
    trimWhiteSpace( symbols[*numFound] );

    tk = strtok_r( NULL, ",=", &context );
    if ( !tk ) return 101; // missing value

    l = strlen(tk) + 1;
    strncpy( values[*numFound], tk, maxLen );
    values[*numFound][maxLen] = 0;

    (*numFound)++;

    tk = strtok_r( NULL, ",=", &context );

  }

  return 1;

}
