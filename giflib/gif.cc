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

void printErrMsg (
  const char *fileName,
  int lineNum,
  const char *msg );

#define __gif_cc 1

#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <setjmp.h>

#include "gif.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

#if GIFLIB_MAJOR > 5 || GIFLIB_MAJOR == 5 && GIFLIB_MINOR >= 1
  #define GIF_CLOSE_FILE(gif) DGifCloseFile(gif, NULL)
  #define GIF_OPEN_FILE(gif) DGifOpenFileName(gif, NULL)
#else
  #define GIF_CLOSE_FILE(gif) DGifCloseFile(gif)
  #define GIF_OPEN_FILE(gif) DGifOpenFileName(gif)
#endif

static jmp_buf g_jump_h;

static void signal_handler (
  int sig
) {

  //fprintf( stderr, "Got signal: sig = %-d\n", sig );
  longjmp( g_jump_h, 1 );

}

static int littleEndian ( void ) {

char c[2];
unsigned short *s;

  c[0] = 1;
  c[1] = 0;

  s = (unsigned short *) c;

  if ( *s == 1 )
    return 1;
  else
    return 0;

}

static void agifc_update (
  XtPointer client,
  XtIntervalId *id )
{

activeGifClass *agifo = (activeGifClass *) client;

  if ( !agifo->timerActive ) return;

  agifo->timer = appAddTimeOut(
   agifo->actWin->appCtx->appContext(),
   agifo->timerValue, agifc_update, client );

  agifo->actWin->appCtx->proc->lock();
  agifo->actWin->addDefExeNode( agifo->aglPtr );
  agifo->actWin->appCtx->proc->unlock();

}

static void agifc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeGifClass *agifo = (activeGifClass *) client;
int status;

  agifo->actWin->setChanged();

  agifo->eraseSelectBoxCorners();
  agifo->erase();

  agifo->x = agifo->bufX;
  agifo->sboxX = agifo->bufX;

  agifo->y = agifo->bufY;
  agifo->sboxY = agifo->bufY;

  strncpy( agifo->gifFileName, agifo->bufGifFileName, 127 );

  agifo->uniformSize = agifo->bufUniformSize;
  agifo->refreshRate = agifo->bufRefreshRate;
  if ( ( agifo->refreshRate > 0 ) && ( agifo->refreshRate < 1000 ) ) {
    agifo->refreshRate = 1000;
  }
  agifo->fastErase = agifo->bufFastErase;
  agifo->noErase = agifo->bufNoErase;

  status = agifo->readGifFile();

  agifo->initSelectBox();

  if ( !( status & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, activeGifClass_str1, agifo->actWin->fileName,
     agifo->gifFileName );
    agifo->actWin->appCtx->postMessage( msg );
  }

}

static void agifc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeGifClass *agifo = (activeGifClass *) client;

  agifc_edit_update ( w, client, call );
  agifo->refresh( agifo );

}

static void agifc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeGifClass *agifo = (activeGifClass *) client;

  agifc_edit_update ( w, client, call );
  agifo->ef.popdown();
  agifo->operationComplete();

}

static void agifc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeGifClass *agifo = (activeGifClass *) client;

  agifo->ef.popdown();
  agifo->operationCancel();

}

static void agifc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeGifClass *agifo = (activeGifClass *) client;

  agifo->erase();
  agifo->deleteRequest = 1;
  agifo->ef.popdown();
  agifo->operationCancel();
  agifo->drawAll();

}

activeGifClass::activeGifClass ( void ) {

  name = new char[strlen("cfcf6c8a_dbeb_11d2_8a97_00104b8742df")+1];
  strcpy( name, "cfcf6c8a_dbeb_11d2_8a97_00104b8742df" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  strcpy( gifFileName, "" );

  activeMode = 0;
  active = 0;
  pixels = NULL;
  imageData = NULL;
  image = NULL;
  noFile = 1;
  w = 5;
  h = 5;
  uniformSize = 0;
  refreshRate = 0;
  fastErase = 0;
  noErase = 0;

}

activeGifClass::~activeGifClass ( void ) {

int i;

//   fprintf( stderr, "In activeGifClass::~activeGifClass\n" );

  if ( name ) delete[] name;

  if ( pixels ) {

    for ( i=0; i<numPixels; i++ ) {
      XFreeColors( actWin->display(), actWin->ci->getColorMap(),
       &pixels[i], 1, 0L );
    }

    delete pixels;

  }

//    if ( imageData ) delete imageData;
  if ( image ) {
    XDestroyImage( image );
    image = NULL;
  }

}

// copy constructor
activeGifClass::activeGifClass
 ( const activeGifClass *source ) {

int status;
activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("cfcf6c8a_dbeb_11d2_8a97_00104b8742df")+1];
  strcpy( name, "cfcf6c8a_dbeb_11d2_8a97_00104b8742df" );

  strncpy( gifFileName, source->gifFileName, 127 );

  activeMode = 0;
  active = 0;
  pixels = NULL;
  imageData = NULL;
  image = NULL;
  noFile = 1;
  w = 5;
  h = 5;
  uniformSize = source->uniformSize;
  refreshRate = source->refreshRate;
  fastErase = source->fastErase;
  noErase = source->noErase;

  doAccSubs( gifFileName, 127 );

  status = readGifFile();

}

int activeGifClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h )
{

  actWin = (activeWindowClass *) aw_obj;
  xOrigin = 0;
  yOrigin = 0;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  activeMode = 0;

  this->editCreate();

  return 1;

}

int activeGifClass::genericEdit ( void )
{

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "cfcf6c8a_dbeb_11d2_8a97_00104b8742df" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeGifClass_str2, 31 );

  Strncat( title, activeGifClass_str3, 31 );

  bufX = x;
  bufY = y;

  strncpy( bufGifFileName, gifFileName, 127 );

  bufUniformSize = uniformSize;
  bufRefreshRate = refreshRate;
  bufFastErase = fastErase;
  bufNoErase = noErase;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeGifClass_str4, 27, &bufX );
  ef.addTextField( activeGifClass_str5, 27, &bufY );
  ef.addTextField( activeGifClass_str6, 27, bufGifFileName, 127 );
  ef.addTextField( activeGifClass_str7, 27, &bufRefreshRate );
  ef.addToggle( activeGifClass_str8, &bufUniformSize );
  ef.addToggle( activeGifClass_str9, &bufFastErase );
  ef.addToggle( activeGifClass_str10, &bufNoErase );

  return 1;

}

int activeGifClass::edit ( void )
{

  this->genericEdit();
  ef.finished( agifc_edit_ok, agifc_edit_apply, agifc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeGifClass::editCreate ( void )
{

  this->genericEdit();
  ef.finished( agifc_edit_ok, agifc_edit_apply, agifc_edit_cancel_delete,
   this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

void activeGifClass::checkGifFileTime ( void )
{

int status, i;
char name[127+1];
struct stat statBuf;
expStringClass expStr;

  this->actWin->substituteSpecial( 127, gifFileName, name );

  expStr.setRaw( name );
  expStr.expand1st( actWin->numMacros, actWin->macros, actWin->expansions );

  i = 0;
  do {
    this->actWin->appCtx->expandFileName( i, name, expStr.getExpanded(),
     ".gif", 127 );
    status = stat( name, &statBuf );
    i++;
  } while ( ( i < actWin->appCtx->numPaths ) && status );
  if ( status ) {
    fileModTime = 0;
    return;
  }

  fileModTime = statBuf.st_mtime;

}

int activeGifClass::readGifFile ( void )
{

XColor color;
int i, ii, iw, extra, base, offset, pass, index, status;
unsigned char *ptr, mask, lastColor;
int screen_num, row, col, depth;
Visual *visual;
int bgR, bgG, bgB;
char name[127+1];
unsigned long pix;

int pixelsAllocated = 0;
int imageAllocated = 0;
int imageCreated = 0;
int colorsAllocated = 0;
int fileOpened = 0;
struct stat statBuf;
expStringClass expStr;
struct sigaction sa, oldsa, dummysa;

  // Make sure gif is accessable
  status = setjmp( g_jump_h );
  if ( !status ) {

    sa.sa_handler = signal_handler;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = 0;

    status = sigaction( SIGILL, &sa, &oldsa );
    status = sigaction( SIGSEGV, &sa, &dummysa );

  }
  else {

    printErrMsg( __FILE__, __LINE__, "got signal" );
    goto sig_error_return;

  }

  this->actWin->substituteSpecial( 127, gifFileName, name );

  expStr.setRaw( name );
  expStr.expand1st( actWin->numMacros, actWin->macros, actWin->expansions );

  i = 0;
  do {
    this->actWin->appCtx->expandFileName( i, name, expStr.getExpanded(),
     ".gif", 127 );
    gif = GIF_OPEN_FILE( name );
    i++;
  } while ( ( i < actWin->appCtx->numPaths ) && !gif );
  if ( !gif ) {
    goto error_return;
  }
  fileOpened = 1;

  status = stat( name, &statBuf );

  if ( !status ) {
    fileModTime = statBuf.st_mtime;
  }
  else {
    fileModTime = 0;
  }

  screen_num = DefaultScreen( actWin->display() );
  visual = DefaultVisual( actWin->display(), screen_num );
  depth = DefaultDepth( actWin->display(), screen_num );

  if ( pixels ) {

    for ( i=0; i<numPixels; i++ ) {
      XFreeColors( actWin->display(), actWin->ci->getColorMap(),
       &pixels[i], 1, 0L );
    }

    delete pixels;
    pixels = NULL;

  }

//    if ( imageData ) {
//      delete imageData;
//      imageData = NULL;
//    }

  if ( image ) {
    XDestroyImage( image );
    image = NULL;
  }

  w = gif->SWidth;
  h = gif->SHeight;

  if ( ( w == 0 ) || ( h == 0 ) ) {
    goto error_return;
  }

  status = DGifSlurp( gif );
  if ( !status ) {
    goto error_return;
  }

  if ( !gif->SColorMap ) {
    goto error_return;
  }

  lastColor = gif->SColorMap->ColorCount - 1;

  numPixels = 0;
  pixels = new unsigned long[gif->SColorMap->ColorCount+1];
  if ( !pixels ) {
    goto error_return;
  }
  pixelsAllocated = 1;
  numPixels = gif->SColorMap->ColorCount;

  for ( i=0; i<gif->SColorMap->ColorCount; i++ ) pixels[i] = 0;

  // start with 128 colors; we can't possibly get 256
  for (ii=0, mask=0xfe; ii<7; ii++, mask<<=1) {

    for ( i=0; i<gif->SColorMap->ColorCount; i++ ) {

      color.red = ( gif->SColorMap->Colors[i].Red & mask ) * 256;
      color.green = ( gif->SColorMap->Colors[i].Green & mask ) * 256;
      color.blue = ( gif->SColorMap->Colors[i].Blue & mask ) * 256;
      color.flags = DoRed | DoGreen | DoBlue;

      if ( XAllocColor(actWin->display(),actWin->ci->getColorMap(),&color) )
	pixels[i] = color.pixel;
      else
	break;

      if ( gif->SBackGroundColor == i ) {
        bgR = color.red;
        bgG = color.green;
        bgB = color.blue;
      }

    }

    if ( i < gif->SColorMap->ColorCount )
      XFreeColors( actWin->display(), actWin->ci->getColorMap(),
       pixels, i, 0L);
    else
      break;

  }

  if (ii == 8) {
    actWin->appCtx->postMessage(
     "Not enough colors available to display image" );
    goto error_return;
  }

  colorsAllocated = 1;

  switch ( depth ) {

  case 8:

    if ( w % 4 ) {
      iw = w + 4 - ( w % 4 );
      extra = 4 - ( w % 4 );
    }
    else {
      iw = w;
      extra = 0;
    }

    imageData = new char[iw*h];
    if ( !imageData ) {
      goto error_return;
    }
    imageAllocated = 1;

    i=0;
    ii = 0;
    ptr = (unsigned char *) gif->SavedImages->RasterBits;

    if ( !gif->Image.Interlace ) {

      for ( row=0; row<h; row++ ) {

        for ( col=0; col<w; col++ ) {

          if ( ptr[i] == lastColor )
            pix = actWin->drawGc.getBaseBG();
          else
            pix = pixels[ptr[i]];

          imageData[ii++] = pix;
          i++;

        }

        ii += extra;

      }

    }
    else {

      pass = 0;
      base = 0;
      offset = 8;

      for ( row=0; row<h; row++ ) {

        index = base * iw;

        for ( col=0; col<w; col++ ) {

          imageData[index++] = pixels[ptr[i++]];

        }

        base += offset;
        if ( base >= h ) {

          switch ( pass ) {
          case 0:
            base = 4;
            offset = 8;
            break;
          case 1:
            base = 2;
            offset = 4;
            break;
          case 2:
            base = 1;
            offset = 2;
            break;
          default:
            base = 0;
            offset = 0;
          }

          pass++;

        }

      }

    }

    image = XCreateImage( actWin->display(), visual,
     depth, ZPixmap, 0, imageData, w, h, 32, 0 );
    if ( !image ) {
      goto error_return;
    }

    if ( littleEndian() )
      image->byte_order = LSBFirst;
    else
      image->byte_order = MSBFirst;

    imageCreated = 1;

    break;

  case 16:
 
    if ( w % 2 ) {
      iw = w + 2 - ( w % 2 );
      extra = 2 - ( w % 2 );
    }
    else {
      iw = w;
      extra = 0;
    }

    imageData = new char[iw*h*2];
    if ( !imageData ) {
      goto error_return;
    }
    imageAllocated = 1;

    i=0;
    ii = 0;
    ptr = (unsigned char *) gif->SavedImages->RasterBits;

    if ( !gif->Image.Interlace ) {

      for ( row=0; row<h; row++ ) {

        for ( col=0; col<w; col++ ) {

          if ( ptr[i] == lastColor )
            pix = actWin->drawGc.getBaseBG();
          else
            pix = pixels[ptr[i]];

          if ( littleEndian() ) {
            imageData[ii++] = ( pix & 0xff );
            imageData[ii++] = ( pix & 0xff00 ) >> 8;
          }
          else {
            imageData[ii++] = ( pix & 0xff00 ) >> 8;
            imageData[ii++] = ( pix & 0xff );
          }
          i++;

        }

        ii += ( 2 * extra );

      }

    }
    else {

      pass = 0;
      base = 0;
      offset = 8;

      for ( row=0; row<h; row++ ) {

        index = base * 2 * iw;

        for ( col=0; col<w; col++ ) {

          if ( littleEndian() ) {
            imageData[index++] = ( pixels[ptr[i]] & 0xff );
            imageData[index++] = ( pixels[ptr[i]] & 0xff00 ) >> 8;
          }
          else {
            imageData[index++] = ( pixels[ptr[i]] & 0xff00 ) >> 8;
            imageData[index++] = ( pixels[ptr[i]] & 0xff );
          }
          i++;

        }

        base += offset;
        if ( base >= h ) {

          switch ( pass ) {
          case 0:
            base = 4;
            offset = 8;
            break;
          case 1:
            base = 2;
            offset = 4;
            break;
          case 2:
            base = 1;
            offset = 2;
            break;
          default:
            base = 0;
            offset = 0;
          }

          pass++;

        }

      }

    }

    image = XCreateImage( actWin->display(), visual,
     depth, ZPixmap, 0, imageData, w, h, 32, 0 );
    if ( !image ) {
      goto error_return;
    }

    if ( littleEndian() )
      image->byte_order = LSBFirst;
    else
      image->byte_order = MSBFirst;

    imageCreated = 1;

    break;

  case 24:
 
    iw = w;
    extra = 0;

    imageData = new char[iw*h*4];
    if ( !imageData ) {
      goto error_return;
    }
    imageAllocated = 1;

    i=0;
    ii = 0;
    ptr = (unsigned char *) gif->SavedImages->RasterBits;

    if ( !gif->Image.Interlace ) {

      for ( row=0; row<h; row++ ) {

        for ( col=0; col<w; col++ ) {

          if ( ptr[i] == lastColor )
            pix = actWin->drawGc.getBaseBG();
          else
            pix = pixels[ptr[i]];

          if ( littleEndian() ) {
            imageData[ii++] = ( pix & 0xff );
            imageData[ii++] = ( pix & 0xff00 ) >> 8;
            imageData[ii++] = ( pix & 0xff0000 ) >> 16;
            imageData[ii++] = 0;
          }
          else {
            imageData[ii++] = 0;
            imageData[ii++] = ( pix & 0xff0000 ) >> 16;
            imageData[ii++] = ( pix & 0xff00 ) >> 8;
            imageData[ii++] = ( pix & 0xff );
          }
          i++;

        }

        ii += ( 4 * extra );

      }

    }
    else {

      pass = 0;
      base = 0;
      offset = 8;

      for ( row=0; row<h; row++ ) {

        index = base * 4 * iw;

        for ( col=0; col<w; col++ ) {

          if ( littleEndian() ) {
            imageData[index++] = ( pixels[ptr[i]] & 0xff );
            imageData[index++] = ( pixels[ptr[i]] & 0xff00 ) >> 8;
            imageData[index++] = ( pixels[ptr[i]] & 0xff0000 ) >> 16;
            imageData[index++] = 0;
          }
          else {
            imageData[index++] = 0;
            imageData[index++] = ( pixels[ptr[i]] & 0xff0000 ) >> 16;
            imageData[index++] = ( pixels[ptr[i]] & 0xff00 ) >> 8;
            imageData[index++] = ( pixels[ptr[i]] & 0xff );
          }
          i++;

        }

        base += offset;
        if ( base >= h ) {

          switch ( pass ) {
          case 0:
            base = 4;
            offset = 8;
            break;
          case 1:
            base = 2;
            offset = 4;
            break;
          case 2:
            base = 1;
            offset = 2;
            break;
          default:
            base = 0;
            offset = 0;
          }

          pass++;

        }

      }

    }

    image = XCreateImage( actWin->display(), visual,
     depth, ZPixmap, 0, imageData, w, h, 32, 0 );
    if ( !image ) {
      goto error_return;
    }

    if ( littleEndian() )
      image->byte_order = LSBFirst;
    else
      image->byte_order = MSBFirst;

    imageCreated = 1;

    break;

  default:

    goto error_return;

  }

  status = GIF_CLOSE_FILE( gif );

  noFile = 0;

  // restore default sig handler
  status = sigaction( SIGILL, &oldsa, NULL );
  status = sigaction( SIGSEGV, &oldsa, NULL );

  return 1;

error_return:

  if ( fileOpened ) status = GIF_CLOSE_FILE( gif );

sig_error_return:

  // restore default sig handler
  status = sigaction( SIGILL, &oldsa, NULL );
  status = sigaction( SIGSEGV, &oldsa, NULL );

  if ( pixelsAllocated ) {

    if ( colorsAllocated ) {

      for ( i=0; i<numPixels; i++ ) {
        XFreeColors( actWin->display(), actWin->ci->getColorMap(),
         &pixels[i], 1, 0L );
      }

    }

    delete pixels;
    pixels = NULL;
    numPixels = 0;

  }

  if ( imageCreated ) {
    XDestroyImage( image );
    image = NULL;
  }
  else if ( imageAllocated ) {
    delete imageData;
    imageData = NULL;
  }

  noFile = 1;
  return 0;

}

int activeGifClass::save (
 FILE *f )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
static char *emptyStr = "";

  major = AGIFC_MAJOR_VERSION;
  minor = AGIFC_MINOR_VERSION;
  release = AGIFC_RELEASE;

  // read file and process each "object" tag
  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "file", gifFileName, emptyStr );
  tag.loadW( "refreshRate", &refreshRate, &zero );
  tag.loadBoolW( "uniformSize", &uniformSize, &zero );
  tag.loadBoolW( "fastErase", &fastErase, &zero );
  tag.loadBoolW( "noErase", &noErase, &zero );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeGifClass::old_save (
 FILE *f )
{

  fprintf( f, "%-d %-d %-d\n", AGIFC_MAJOR_VERSION, AGIFC_MINOR_VERSION,
   AGIFC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  writeStringToFile( f, gifFileName );

  fprintf( f, "%-d\n", refreshRate );
  fprintf( f, "%-d\n", uniformSize );
  fprintf( f, "%-d\n", fastErase );

  return 1;

}

int activeGifClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat, status;

tagClass tag;

int zero = 0;
static char *emptyStr = "";

  this->actWin = _actWin;

  // read file and process each "object" tag
  tag.init();
  tag.loadR( "beginObjectProperties" );
  tag.loadR( unknownTags );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );
  tag.loadR( "x", &x );
  tag.loadR( "y", &y );
  tag.loadR( "w", &w );
  tag.loadR( "h", &h );
  tag.loadR( "file", 127, gifFileName, emptyStr );
  tag.loadR( "refreshRate", &refreshRate, &zero );
  tag.loadR( "uniformSize", &uniformSize, &zero );
  tag.loadR( "fastErase", &fastErase, &zero );
  tag.loadR( "noErase", &noErase, &zero );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > AGIFC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  status = readGifFile();
  if ( !( status & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, activeGifClass_str1, actWin->fileName,
     gifFileName );
    actWin->appCtx->postMessage( msg );
  }

  return stat;

}

int activeGifClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int status;
int major, minor, release;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > AGIFC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  readStringFromFile( gifFileName, 127+1, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    fscanf( f, "%d\n", &refreshRate );
    fscanf( f, "%d\n", &uniformSize );
  }
  else {
    refreshRate = 0;
    uniformSize = 0;
  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    fscanf( f, "%d\n", &fastErase );
  }
  else {
    fastErase = 0;
  }

  noErase = 0;

  status = readGifFile();
  if ( !( status & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, activeGifClass_str1, actWin->fileName,
     gifFileName );
    actWin->appCtx->postMessage( msg );
  }

  this->initSelectBox();

  return 1;

}

int activeGifClass::erase ( void ) {

  if ( noFile || activeMode || deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeGifClass::eraseActive ( void ) {

  if ( noErase ) return 1;

  if ( !enabled || noFile || !activeMode ) return 1;

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeGifClass::draw ( void ) {

  if ( noFile || activeMode || deleteRequest ) return 1;

  if ( !actWin->appCtx->renderImages() ) {
    actWin->drawGc.setFG( actWin->defaultTextFgColor );
    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );
    return 1;
  }

  if ( image ) {
    XPutImage( actWin->display(), XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), image, 0, 0, x, y, w, h );
  }

  return 1;

}

int activeGifClass::draw (
  int x0,
  int y0,
  int x1,
  int y1 )
{

int curW, curH;

  if ( noFile || activeMode || deleteRequest ) return 1;

  if ( !actWin->appCtx->renderImages() ) {
    actWin->drawGc.setFG( actWin->defaultTextFgColor );
    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );
    return 1;
  }

  if ( x0 > ( x + w ) ) return 1;
  if ( x1 < x ) return 1;
  if ( y0 > ( y + h ) ) return 1;
  if ( y1 < y ) return 1;

  if ( x0 < x ) {
    x0 = x;
  }
  if ( y0 < y ) {
    y0 = y;
  }
  if ( x1 > ( x + w ) ) {
    x1 = x + w;
  }
  if ( y1 > ( y + h ) ) {
    y1 = y + h;
  }

  curW = x1 - x0;
  curH = y1 - y0;

  if ( image ) {
    XPutImage( actWin->display(), XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), image, x0-x, y0-y, x0, y0, curW, curH );
  }

  return 1;

}

int activeGifClass::drawActive ( void ) {

int screen_num, depth, stat;
Visual *visual;
Pixmap pixmap;

  stat = drawActive( x, y, x+w, y+h );
  return stat;

  if ( !enabled || noFile || !activeMode ) return 1;

  if ( !actWin->appCtx->renderImages() ) {
    actWin->executeGc.setFG( actWin->defaultTextFgColor );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h );
    return 1;
  }

  screen_num = DefaultScreen( actWin->display() );
  visual = DefaultVisual( actWin->display(), screen_num );
  depth = DefaultDepth( actWin->display(), screen_num );

  pixmap = XCreatePixmap( actWin->display(),
   XtWindow(actWin->executeWidget), w, h, depth );

  if ( image ) {
    XPutImage( actWin->display(), pixmap,
     actWin->executeGc.normGC(), image, 0, 0, 0, 0, w, h );
  }

  if ( needErase ) {
    needErase = 0;
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), bufX, bufY, bufW, bufH );
    XFillRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), bufX, bufY, bufW, bufH );
  }

  XCopyArea( actWin->display(), pixmap,
   drawable(actWin->executeWidget), actWin->executeGc.normGC(),
   0, 0, w, h, x, y );

  XFreePixmap( actWin->display(), pixmap );

  return 1;

}

#if 1
int activeGifClass::drawActive (
  int x0,
  int y0,
  int x1,
  int y1 )
{

int curW, curH;

  if ( !enabled || noFile || !activeMode ) return 1;

  if ( !actWin->appCtx->renderImages() ) {
    actWin->executeGc.setFG( actWin->defaultTextFgColor );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h );
    return 1;
  }

  if ( x0 > ( x + w ) ) return 1;
  if ( x1 < x ) return 1;
  if ( y0 > ( y + h ) ) return 1;
  if ( y1 < y ) return 1;

  if ( x0 < x ) {
    x0 = x;
  }
  if ( y0 < y ) {
    y0 = y;
  }
  if ( x1 > ( x + w ) ) {
    x1 = x + w;
  }
  if ( y1 > ( y + h ) ) {
    y1 = y + h;
  }

  curW = x1 - x0;
  curH = y1 - y0;

  if ( image ) {
    XPutImage( actWin->display(), drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), image, x0-x, y0-y, x0, y0, curW, curH );
  }

  return 1;

}
#endif

#if 0
int activeGifClass::drawActive (
  int x0,
  int y0,
  int x1,
  int y1 )
{

int curW, curH;
int screen_num, depth;
Visual *visual;
Pixmap pixmap;

  if ( !enabled || noFile || !activeMode ) return 1;

  if ( !actWin->appCtx->renderImages() ) {
    actWin->executeGc.setFG( actWin->defaultTextFgColor );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h );
    return 1;
  }

  screen_num = DefaultScreen( actWin->display() );
  visual = DefaultVisual( actWin->display(), screen_num );
  depth = DefaultDepth( actWin->display(), screen_num );

  pixmap = XCreatePixmap( actWin->display(),
   XtWindow(actWin->executeWidget), w, h, depth );

  if ( x0 > ( x + w ) ) return 1;
  if ( x1 < x ) return 1;
  if ( y0 > ( y + h ) ) return 1;
  if ( y1 < y ) return 1;

  if ( x0 < x ) {
    x0 = x;
  }
  if ( y0 < y ) {
    y0 = y;
  }
  if ( x1 > ( x + w ) ) {
    x1 = x + w;
  }
  if ( y1 > ( y + h ) ) {
    y1 = y + h;
  }

  curW = x1 - x0;
  curH = y1 - y0;

  if ( image ) {
    XPutImage( actWin->display(), pixmap,
     actWin->executeGc.normGC(), image, 0, 0, 0, 0, w, h );
  }

  XCopyArea( actWin->display(), pixmap,
   drawable(actWin->executeWidget), actWin->executeGc.normGC(),
   0, 0, w, h, x, y );

  XFreePixmap( actWin->display(), pixmap );

  return 1;

}
#endif

int activeGifClass::activate (
  int pass,
  void *ptr ) {

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;
    aglPtr = ptr;
    prevFileModTime = fileModTime;
    needErase = 0;

    break;

  case 2:

    if ( !opComplete ) {

      opComplete = 1;
      activeMode = 1;
      active = 1;
      initEnable();

      if ( refreshRate > 0 ) {
        timerValue = refreshRate;
        timer = appAddTimeOut(
         actWin->appCtx->appContext(), timerValue, agifc_update, this );
        timerActive = 1;
      }
      else {
        timerActive = 0;
      }

    }

    break;

  case 3:
  case 4:
  case 5:
  case 6:

    break;

  }

  return 1;

}

int activeGifClass::deactivate (
  int pass
) {

  if ( pass == 1 ) {
    active = 0;
    activeMode = 0;
    if ( timerActive ) {
      timerActive = 0;
      XtRemoveTimeOut( timer );
    }
  }

  return 1;

}

int activeGifClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

  return 0; // no resize allowed

}

int activeGifClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

  return 0; // no resize allowed

}


void activeGifClass::executeDeferred ( void ) {

int status;

  actWin->appCtx->proc->lock();
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  checkGifFileTime();

  if ( fileModTime != prevFileModTime ) {
    prevFileModTime = fileModTime;
    if ( !uniformSize ) {
      if ( fastErase ) {
        bufX = x;
        bufY = y;
        bufH = h;
        bufW = w;
        needErase = 1;
      }
      else {
        eraseActive();
      }
    }
    status = readGifFile();
    drawAllActive();
  }

}

char *activeGifClass::getSearchString (
  int index
) {

  if ( index == 0 ) {
    return gifFileName;
  }
  else {
    return NULL;
  }

}

void activeGifClass::replaceString (
  int i,
  int max,
  char *string
) {

int status;
int l = 127;

  if ( max < l ) l = max;

  if ( i == 0 ) {

    strncpy( gifFileName, string, l );
    gifFileName[l] = 0;

    status = readGifFile();

    initSelectBox();

    if ( !( status & 1 ) ) {
      char msg[255+1];
      snprintf( msg, 255, activeGifClass_str1, actWin->fileName,
       gifFileName );
      actWin->appCtx->postMessage( msg );
    }

  }

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_cfcf6c8a_dbeb_11d2_8a97_00104b8742dfPtr ( void ) {

activeGifClass *ptr;

  ptr = new activeGifClass;
  return (void *) ptr;

}

void *clone_cfcf6c8a_dbeb_11d2_8a97_00104b8742dfPtr (
  void *_srcPtr )
{

activeGifClass *ptr, *srcPtr;

  srcPtr = (activeGifClass *) _srcPtr;

  ptr = new activeGifClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
