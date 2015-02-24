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

#define __png_cc 1

#include "myPng.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"
#include "zlib.h"

#include "remFileOpen.h"

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

static void apngc_update (
  XtPointer client,
  XtIntervalId *id )
{

activePngClass *apngo = (activePngClass *) client;

  if ( !apngo->timerActive ) return;

  apngo->timer = appAddTimeOut(
   apngo->actWin->appCtx->appContext(),
   apngo->timerValue, apngc_update, client );

  apngo->actWin->appCtx->proc->lock();
  apngo->actWin->addDefExeNode( apngo->aglPtr );
  apngo->actWin->appCtx->proc->unlock();

}

static void apngc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePngClass *apngo = (activePngClass *) client;
int status;

  apngo->actWin->setChanged();

  apngo->eraseSelectBoxCorners();
  apngo->erase();

  apngo->x = apngo->bufX;
  apngo->sboxX = apngo->bufX;

  apngo->y = apngo->bufY;
  apngo->sboxY = apngo->bufY;

  strncpy( apngo->pngFileName, apngo->bufPngFileName, 127 );

  apngo->uniformSize = apngo->bufUniformSize;
  apngo->refreshRate = apngo->bufRefreshRate;
  if ( ( apngo->refreshRate > 0 ) && ( apngo->refreshRate < 1000 ) ) {
    apngo->refreshRate = 1000;
  }
  apngo->fastErase = apngo->bufFastErase;
  apngo->noErase = apngo->bufNoErase;

  status = apngo->readPngFile();

  apngo->initSelectBox();

  if ( !( status & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, activePngClass_str1, apngo->actWin->fileName,
     apngo->pngFileName );
    apngo->actWin->appCtx->postMessage( msg );
  }

}

static void apngc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePngClass *apngo = (activePngClass *) client;

  apngc_edit_update ( w, client, call );
  apngo->refresh( apngo );

}

static void apngc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePngClass *apngo = (activePngClass *) client;

  apngc_edit_update ( w, client, call );
  apngo->ef.popdown();
  apngo->operationComplete();

}

static void apngc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePngClass *apngo = (activePngClass *) client;

  apngo->ef.popdown();
  apngo->operationCancel();

}

static void apngc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePngClass *apngo = (activePngClass *) client;

  apngo->erase();
  apngo->deleteRequest = 1;
  apngo->ef.popdown();
  apngo->operationCancel();
  apngo->drawAll();

}

static int compare_nodes_by_pixel (
  void *node1,
  void *node2
) {

colorCachePtr p1, p2;

  p1 = (colorCachePtr) node1;
  p2 = (colorCachePtr) node2;

  if ( p1->pixel > p2->pixel )
      return 1;
  else if ( p1->pixel < p2->pixel )
    return -1;

  return 0;

}

static int compare_key_by_pixel (
  void *key,
  void *node
) {

colorCachePtr p;
unsigned long *onePixel;

  p = (colorCachePtr) node;
  onePixel = (unsigned long *) key;

  if ( *onePixel > p->pixel )
      return 1;
  else if ( *onePixel < p->pixel )
    return -1;

  return 0;

}

static int compare_nodes_by_color (
  void *node1,
  void *node2
) {

int i;
colorCachePtr p1, p2;

  p1 = (colorCachePtr) node1;
  p2 = (colorCachePtr) node2;

  for ( i=0; i<3; i++ ) {
    if ( p1->rgb[i] > p2->rgb[i] )
      return 1;
    else if ( p1->rgb[i] < p2->rgb[i] )
      return -1;
  }

  return 0;

}

static int compare_key_by_color (
  void *key,
  void *node
) {

int i;
colorCachePtr p;
int *oneRgb;

  p = (colorCachePtr) node;
  oneRgb = (int *) key;

  for ( i=0; i<3; i++ ) {
    if ( oneRgb[i] > p->rgb[i] )
      return 1;
    else if ( oneRgb[i] < p->rgb[i] )
      return -1;
  }

  return 0;

}

static int copy_nodes (
  void *node1,
  void *node2
) {

colorCachePtr p1, p2;

  p1 = (colorCachePtr) node1;
  p2 = (colorCachePtr) node2;

  *p1 = *p2;

  return 1;

}

activePngClass::activePngClass ( void ) {

int status;

  name = new char[strlen("activePngClass")+1];
  strcpy( name, "activePngClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  strcpy( pngFileName, "" );

  activeMode = 0;
  active = 0;
  xData = NULL;
  image = NULL;
  pixels = NULL;
  noFile = 1;
  w = 5;
  h = 5;
  uniformSize = 0;
  refreshRate = 0;
  fastErase = 0;
  noErase = 0;

  status = avl_init_tree( compare_nodes_by_color,
   compare_key_by_color, copy_nodes, &(colorCacheByColorH) );
  if ( !( status & 1 ) ) colorCacheByColorH = (AVL_HANDLE) NULL;

  status = avl_init_tree( compare_nodes_by_color,
   compare_key_by_color, copy_nodes, &(allColorsH) );
  if ( !( status & 1 ) ) allColorsH = (AVL_HANDLE) NULL;

  status = avl_init_tree( compare_nodes_by_pixel,
   compare_key_by_pixel, copy_nodes, &(colorCacheByPixelH) );
  if ( !( status & 1 ) ) colorCacheByPixelH = (AVL_HANDLE) NULL;

}

activePngClass::~activePngClass ( void ) {

//   fprintf( stderr, "In activePngClass::~activePngClass\n" );

  if ( name ) delete[] name;

  discardColorList();

  discardPixels();

  if ( image ) {
    XDestroyImage( image );
    image = NULL;
  }

  if ( pixels ) {
    delete[] pixels;
    pixels = NULL;
  }

  if ( allColorsH ) free( allColorsH );
  if ( colorCacheByColorH ) free( colorCacheByColorH );
  if ( colorCacheByPixelH ) free( colorCacheByPixelH );

}

// copy constructor
activePngClass::activePngClass
 ( const activePngClass *source ) {

int status;
activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activePngClass")+1];
  strcpy( name, "activePngClass" );

  strncpy( pngFileName, source->pngFileName, 127 );

  activeMode = 0;
  active = 0;
  xData = NULL;
  image = NULL;
  pixels = NULL;
  noFile = 1;
  uniformSize = source->uniformSize;
  refreshRate = source->refreshRate;
  fastErase = source->fastErase;
  noErase = source->noErase;

  status = avl_init_tree( compare_nodes_by_color,
   compare_key_by_color, copy_nodes, &(colorCacheByColorH) );
  if ( !( status & 1 ) ) colorCacheByColorH = (AVL_HANDLE) NULL;

  status = avl_init_tree( compare_nodes_by_color,
   compare_key_by_color, copy_nodes, &(allColorsH) );
  if ( !( status & 1 ) ) allColorsH = (AVL_HANDLE) NULL;

  status = avl_init_tree( compare_nodes_by_pixel,
   compare_key_by_pixel, copy_nodes, &(colorCacheByPixelH) );
  if ( !( status & 1 ) ) colorCacheByPixelH = (AVL_HANDLE) NULL;

  doAccSubs( pngFileName, 127 );

  status = readPngFile();

}

int activePngClass::addColorToList (
  int red,
  int green,
  int blue
) {

int status, dup;
colorCachePtr cur;

  cur = new colorCacheType;
  if ( !cur ) return 0;

  cur->rgb[0] = red;
  cur->rgb[1] = green;
  cur->rgb[2] = blue;

  cur->pixel = 0;

  status = avl_insert_node( allColorsH, (void *) cur,
   &dup );
  if ( !( status & 1 ) ) {
    delete cur;
    return 0;
  }
  if ( dup ) {
    delete cur;
    return 0;
  }

  return 1;

}

int activePngClass::discardColorList ( void ) {

colorCachePtr cur, nodeToDel;
int status;

  status = avl_get_first( allColorsH, (void **) &cur );
  if ( !( status & 1 ) ) return 0;

  while ( cur ) {

    nodeToDel = cur;

    status = avl_delete_node( allColorsH, (void **) &cur );
    if ( !( status & 1 ) ) return 0;

    delete nodeToDel;

    status = avl_get_first( allColorsH, (void **) &cur );
    if ( !( status & 1 ) ) return 0;

  }

  return 1;

}

int activePngClass::allocColors ( void ) {

  // allocate pixels for all colors in list; if we can't get
  // all colors then apply a strip mask until we can

colorCachePtr cur;
int status, i, colorAllocFailure, num;
unsigned short r, g, b;

  for (i=0, colorStrippingMask=0xff; i<8; i++, colorStrippingMask<<=1) {

    colorAllocFailure = 0;

    num = 0;
    status = avl_get_first( allColorsH, (void **) &cur );
    if ( !( status & 1 ) ) goto error_return;

    while ( cur ) {

      num++;

      r =
       (unsigned short) ( (unsigned char) cur->rgb[0] & colorStrippingMask ) *
       256;
      g =
       (unsigned short) ( (unsigned char) cur->rgb[1] & colorStrippingMask ) *
       256;
      b =
       (unsigned short) ( (unsigned char) cur->rgb[2] & colorStrippingMask ) *
       256;

      status = allocOneColor( r, g, b );
      if ( !( status & 1 ) ) {
        colorAllocFailure = 1;
        discardPixels();
	break;
      }

      status = avl_get_next( allColorsH, (void **) &cur );
      if ( !( status & 1 ) ) goto error_return;

    }

    if ( !colorAllocFailure ) {
      //fprintf( stderr, "num colors = %-d, mask = %-x\n", num,
      // (unsigned int) colorStrippingMask );
      goto success_return;
    }

  }

success_return:

  return 1;

error_return:

  return 0; // failed

}

int activePngClass::discardPixels ( void ) {

colorCachePtr cur, nodeToDel;
int status;

  status = avl_get_first( colorCacheByColorH, (void **) &cur );
  if ( !( status & 1 ) ) return 0;

  while ( cur ) {

    XFreeColors( actWin->display(), actWin->ci->getColorMap(),
     &cur->pixel, 1, 0L );

    nodeToDel = cur;

    status = avl_delete_node( colorCacheByColorH, (void **) &cur );
    if ( !( status & 1 ) ) return 0;

    delete nodeToDel;

    status = avl_get_first( colorCacheByColorH, (void **) &cur );
    if ( !( status & 1 ) ) return 0;

  }

  return 1;

}

int activePngClass::allocOneColor (
  unsigned short r,
  unsigned short g,
  unsigned short b
) {

unsigned long pixel;
int status, dup;
unsigned int rgb[3];
colorCachePtr cur;
XColor color;

  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
  status = avl_get_match( colorCacheByColorH, (void *) rgb,
   (void **) &cur );

  if ( cur ) return 1;

  color.red = r;
  color.green = g;
  color.blue = b;
  color.flags = DoRed | DoGreen | DoBlue;

  if ( XAllocColor(actWin->display(), actWin->ci->getColorMap(), &color) )
    pixel = color.pixel;
  else
    return 0;

  cur = new colorCacheType;
  if ( !cur ) return 0;

  cur->rgb[0] = r;
  cur->rgb[1] = g;
  cur->rgb[2] = b;

  cur->pixel = pixel;

  status = avl_insert_node( colorCacheByColorH, (void *) cur,
   &dup );
  if ( !( status & 1 ) ) {
    XFreeColors( actWin->display(), actWin->ci->getColorMap(),
     &pixel, 1, 0L );
    delete cur;
    return 0;
  }
  if ( dup ) {
    XFreeColors( actWin->display(), actWin->ci->getColorMap(),
     &pixel, 1, 0L );
    delete cur;
    return 0;
  }

  return 1;

}

void activePngClass::fillPixelArray ( void ) {

int i, numColors;
png_color *palette;

  png_get_PLTE( png_ptr, info_ptr, &palette,
   &numColors );

  for ( i=0; i<numColors; i++ ) {
    pixels[i] = getPixel( palette[i].red, palette[i].green, palette[i].blue );
  }

}

unsigned int activePngClass::getPixel (
  unsigned char r,
  unsigned char g,
  unsigned char b
) {

int status;
unsigned int rgb[3];
colorCachePtr cur;

  rgb[0] = ( (unsigned short) r & colorStrippingMask ) * 256;
  rgb[1] = ( (unsigned short) g & colorStrippingMask ) * 256;
  rgb[2] = ( (unsigned short) b & colorStrippingMask ) * 256;
  status = avl_get_match( colorCacheByColorH, (void *) rgb,
   (void **) &cur );

  if ( cur ) 
    return cur->pixel;
  else
    return 0;

}

int activePngClass::createInteractive (
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

int activePngClass::genericEdit ( void )
{

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activePngClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activePngClass_str2, 31 );

  Strncat( title, activePngClass_str3, 31 );

  bufX = x;
  bufY = y;

  strncpy( bufPngFileName, pngFileName, 127 );

  bufUniformSize = uniformSize;
  bufRefreshRate = refreshRate;
  bufFastErase = fastErase;
  bufNoErase = noErase;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activePngClass_str4, 27, &bufX );
  ef.addTextField( activePngClass_str5, 27, &bufY );
  ef.addTextField( activePngClass_str6, 27, bufPngFileName, 127 );
  ef.addTextField( activePngClass_str7, 27, &bufRefreshRate );
  ef.addToggle( activePngClass_str8, &bufUniformSize );
  ef.addToggle( activePngClass_str9, &bufFastErase );
  ef.addToggle( activePngClass_str10, &bufNoErase );

  return 1;

}

int activePngClass::edit ( void )
{

  this->genericEdit();
  ef.finished( apngc_edit_ok, apngc_edit_apply, apngc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activePngClass::editCreate ( void )
{

  this->genericEdit();
  ef.finished( apngc_edit_ok, apngc_edit_apply, apngc_edit_cancel_delete,
   this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

void activePngClass::checkPngFileTime ( void )
{

int status, i;
char name[127+1];
struct stat statBuf;
expStringClass expStr;

  this->actWin->substituteSpecial( 127, pngFileName, name );

  expStr.setRaw( name );
  expStr.expand1st( actWin->numMacros, actWin->macros, actWin->expansions );

  i = 0;
  do {
    this->actWin->appCtx->expandFileName( i, name, expStr.getExpanded(),
     ".png", 127 );
    status = stat( name, &statBuf );
    i++;
  } while ( ( i < actWin->appCtx->numPaths ) && status );
  if ( status ) {
    fileModTime = 0;
    return;
  }

  fileModTime = statBuf.st_mtime;

}

int activePngClass::readPngFile ( void )
{

unsigned char r, g, b;
int i, iw, extra, status, useBaseBG;
int screen_num, row, col, depth;
Visual *visual;
char name[127+1];
uch *imageData, *src, *dest, a;
unsigned long pixel;
int numTrans;
png_byte *trans = NULL;
png_color_16 *transValues;

double LUT_exponent = 1.0;
double CRT_exponent = 2.2;
double default_display_exponent, display_exponent;

int imageAllocated = 0;
int imageCreated = 0;
int fileOpened = 0;
struct stat statBuf;
expStringClass expStr;

  this->actWin->substituteSpecial( 127, pngFileName, name );

  expStr.setRaw( name );
  expStr.expand1st( actWin->numMacros, actWin->macros, actWin->expansions );

  default_display_exponent = LUT_exponent * CRT_exponent;
  display_exponent = default_display_exponent;

  i = 0;
  do {
    this->actWin->appCtx->expandFileName( i, name, expStr.getExpanded(),
     ".png", 127 );
    //fp = fopen( name, "rb" );
    fp = fileOpen( name, "rb" );
    i++;
  } while ( ( i < actWin->appCtx->numPaths ) && !fp );
  if ( !fp ) {
    return 0;
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

  discardColorList();

  discardPixels();

  if ( image ) {
    XDestroyImage( image );
    image = NULL;
  }

  status = readpng_init( fp, &image_width, &image_height );
  if ( !( status & 1 ) ) {
    fileClose(fp);
    goto error_return;
  }

  // need current background r,g,b
  bg_red = 0;
  bg_green = 0;
  bg_blue = 0;

  imageData = readpng_get_image( display_exponent, &image_channels,
   &image_rowbytes );

  fileClose(fp);

  w = image_width;
  h = image_height;

  if ( ( w == 0 ) || ( h == 0 ) ) {
    goto error_return;
  }

  status = allocColors();
  if ( !( status & 1 ) ) {
    discardColorList();
    actWin->appCtx->postMessage(
     "Not enough colors available to display image" );
    goto error_return;
  }

  if ( usePixelArray ) {
    fillPixelArray();
  }

  discardColorList();

  if ( usePixelArray ) {
    png_get_tRNS( png_ptr, info_ptr, &trans, &numTrans,
     &transValues );
  }

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

    xData = (unsigned char*) XtMalloc( iw*h );
    if ( !xData ) {
      goto error_return;
    }
    imageAllocated = 1;

    for ( row=0; row<h; row++ ) {

      src = imageData + row * image_rowbytes;
      dest = xData + row * iw;

      for ( col=w; col>0; col-- ) {

        useBaseBG = 0;

        if ( usePixelArray ) {

          if ( trans ) {
            if ( *src < numTrans ) {
              if ( trans[*src] < 128 ) useBaseBG = 1;
            }
	  }

          if ( useBaseBG )
            pixel = actWin->drawGc.getBaseBG();
          else
            pixel = pixels[*src];

          src++;

	}
	else {

          if ( image_channels == 3 ) {
            r = *src++;
            g = *src++;
            b = *src++;
          }
          else { // == 4
            r = *src++;
            g = *src++;
            b = *src++;
            a = *src++;
            if ( a == 0 ) {
              r = bg_red;
              g = bg_green;
              b = bg_blue;
              useBaseBG = 1;
            }
          }

          if ( useBaseBG )
            pixel = actWin->drawGc.getBaseBG();
          else
            pixel = getPixel( r, g, b );

	}

        *dest++ = pixel;

      }

    }

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

    xData = (unsigned char*) XtMalloc( iw*h*2 );
    if ( !xData ) {
      goto error_return;
    }
    imageAllocated = 1;

    for ( row=0; row<h; row++ ) {

      src = imageData + row * image_rowbytes;
      dest = xData + row * iw * 2;

      for ( col=w; col>0; col-- ) {

        useBaseBG = 0;

        if ( usePixelArray ) {

          if ( trans ) {
            if ( *src < numTrans ) {
              if ( trans[*src] < 128 ) useBaseBG = 1;
            }
	  }

          if ( useBaseBG )
            pixel = actWin->drawGc.getBaseBG();
          else
            pixel = pixels[*src];

          src++;

	}
	else {

          if ( image_channels == 3 ) {
            r = *src++;
            g = *src++;
            b = *src++;
          }
          else { // == 4
            r = *src++;
            g = *src++;
            b = *src++;
            a = *src++;
            if ( a == 0 ) {
              r = bg_red;
              g = bg_green;
              b = bg_blue;
              useBaseBG = 1;
            }
          }

          if ( useBaseBG )
            pixel = actWin->drawGc.getBaseBG();
          else
            pixel = getPixel( r, g, b );

	}

        if ( littleEndian() ) {
          *dest++  = pixel & 0xff;
          *dest++ = ( pixel & 0xff00 ) >> 8;
        }
        else {
          *dest++ = ( pixel & 0xff00 ) >> 8;
          *dest++  = pixel & 0xff;
        }

      }

    }

    break;

  case 24:
 
    iw = w;
    extra = 0;

    xData = (unsigned char*) XtMalloc( iw*h*4 );
    if ( !xData ) {
      goto error_return;
    }
    imageAllocated = 1;

    for ( row=0; row<h; row++ ) {

      src = imageData + row * image_rowbytes;
      dest = xData + row * iw * 4;

      for ( col=w; col>0; col-- ) {

        useBaseBG = 0;

        if ( usePixelArray ) {

          if ( trans ) {
            if ( *src < numTrans ) {
              if ( trans[*src] < 128 ) useBaseBG = 1;
            }
	  }

          if ( useBaseBG )
            pixel = actWin->drawGc.getBaseBG();
          else
            pixel = pixels[*src];

          src++;

	}
	else {

          if ( image_channels == 3 ) {
            r = *src++;
            g = *src++;
            b = *src++;
          }
          else { // == 4
            r = *src++;
            g = *src++;
            b = *src++;
            a = *src++;
            if ( a == 0 ) {
              r = bg_red;
              g = bg_green;
              b = bg_blue;
              useBaseBG = 1;
            }
          }

          if ( useBaseBG )
            pixel = actWin->drawGc.getBaseBG();
          else
            pixel = getPixel( r, g, b );

	}

        if ( littleEndian() ) {
          *dest++  = pixel & 0xff;
          *dest++ = ( pixel & 0xff00 ) >> 8;
          *dest++ = ( pixel & 0xff0000 ) >> 16;
          *dest++ = 0;
        }
        else {
          *dest++ = 0;
          *dest++ = ( pixel & 0xff0000 ) >> 16;
          *dest++ = ( pixel & 0xff00 ) >> 8;
          *dest++  = pixel & 0xff;
        }

      }

    }

    break;

  default:

    goto error_return;

  }

  image = XCreateImage( actWin->display(), visual,
   depth, ZPixmap, 0, (char *) xData, w, h, 32, 0 );
  if ( !image ) {
    goto error_return;
  }

  if ( littleEndian() )
    image->byte_order = LSBFirst;
  else
    image->byte_order = MSBFirst;

  imageCreated = 1;

  readpng_cleanup( TRUE );

  noFile = 0;

  return 1;

error_return:

  readpng_cleanup( TRUE );

  discardPixels();

  if ( imageCreated ) {
    imageCreated = 0;
    XDestroyImage( image );
    image = NULL;
  }
  else if ( imageAllocated ) {
    imageAllocated = 0;
  }

  return 0;

}

int activePngClass::save (
 FILE *f )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
static char *emptyStr = "";

  major = APNGC_MAJOR_VERSION;
  minor = APNGC_MINOR_VERSION;
  release = APNGC_RELEASE;

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
  tag.loadW( "file", pngFileName, emptyStr );
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

int activePngClass::old_save (
 FILE *f )
{

  fprintf( f, "%-d %-d %-d\n", APNGC_MAJOR_VERSION, APNGC_MINOR_VERSION,
   APNGC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  writeStringToFile( f, pngFileName );

  fprintf( f, "%-d\n", refreshRate );
  fprintf( f, "%-d\n", uniformSize );
  fprintf( f, "%-d\n", fastErase );

  return 1;

}

int activePngClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat, status;

tagClass tag;

int zero = 0;
static char *emptyStr = "";

  this->actWin = _actWin;

  strcpy( pngFileName, "" );
  refreshRate = 0;
  uniformSize = 0;
  fastErase = 0;
  noErase = 0;

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
  tag.loadR( "file", 127, pngFileName, emptyStr );
  tag.loadR( "refreshRate", &refreshRate, &zero );
  tag.loadR( "uniformSize", &uniformSize, &zero );
  tag.loadR( "fastErase", &fastErase, &zero );
  tag.loadR( "noErase", &noErase, &zero );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > APNGC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  status = readPngFile();
  if ( !( status & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, activePngClass_str1, actWin->fileName,
     pngFileName );
    actWin->appCtx->postMessage( msg );
  }

  return stat;

}

int activePngClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int status;
int major, minor, release;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > APNGC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  readStringFromFile( pngFileName, 127+1, f ); actWin->incLine();

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

  status = readPngFile();
  if ( !( status & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, activePngClass_str1, actWin->fileName,
     pngFileName );
    actWin->appCtx->postMessage( msg );
  }

  this->initSelectBox();

  return 1;

}

int activePngClass::erase ( void ) {

  if ( noFile || activeMode || deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activePngClass::eraseActive ( void ) {

  if ( noErase ) return 1;

  if ( !enabled || noFile || !activeMode ) return 1;

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  return 1;

}

int activePngClass::draw ( void ) {

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

int activePngClass::draw (
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

int activePngClass::drawActive ( void ) {

int screen_num, depth, stat;
Visual *visual;
Pixmap pixmap;

  stat = drawActive( x, y, x+w, y+h );
  return stat;

//  if ( noFile || !image || !activeMode ) return 1;
  if ( noFile || !activeMode ) return 1;

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

int activePngClass::drawActive (
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

int activePngClass::activate (
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
         actWin->appCtx->appContext(), timerValue, apngc_update, this );
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

int activePngClass::deactivate (
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

int activePngClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

  return 0; // no resize allowed

}

int activePngClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

  return 0; // no resize allowed

}


void activePngClass::executeDeferred ( void ) {

int status;

  actWin->appCtx->proc->lock();
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  checkPngFileTime();

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
    status = readPngFile();
    drawAllActive();
  }

}

void activePngClass::readpng_version_info ( void ) {

  fprintf(stderr, "   Compiled with libpng %s; using libpng %s.\n",
   PNG_LIBPNG_VER_STRING, png_libpng_ver);

  fprintf(stderr, "   Compiled with zlib %s\n",
   ZLIB_VERSION );

}

/* return value = 1 for success, 0 for bad sig, 2 for bad IHDR, 4 for no mem */

int activePngClass::readpng_init (
  FILE *infile,
  ulg *pWidth,
  ulg *pHeight
) {

uch sig[8];

  /* first do a quick check that the file really is a PNG image; could
   * have used slightly more general png_sig_cmp() function instead */

  fread(sig, 1, 8, infile);
  if (!png_check_sig(sig, 8))
    return 0;   /* bad signature */

  /* could pass pointers to user-defined error handlers instead of NULLs: */

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
    return 4;   /* out of memory */

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return 4;   /* out of memory */
  }


  /* we could create a second info struct here (end_info), but it's only
   * useful if we want to keep pre- and post-IDAT chunk info separated
   * (mainly for PNG-aware image editors and converters) */

  /* setjmp() must be called in every function that calls a PNG-reading
   * libpng function */

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 2;
  }

  png_init_io(png_ptr, infile);
  png_set_sig_bytes(png_ptr, 8);  /* we already read the 8 signature bytes */

  png_read_info(png_ptr, info_ptr);  /* read all PNG info up to image data */

  /* alternatively, could make separate calls to png_get_image_width(),
   * etc., but want bit_depth and color_type for later [don't care about
   * compression_type and filter_type => NULLs] */

  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
   NULL, NULL, NULL);
  *pWidth = width;
  *pHeight = height;

  /* OK, that's all we need for now; return happy */

  return 1;

}

/* returns 1 if succeeds, 0 if fails due to no bKGD chunk, 2 if libpng error;
 * scales values to 8-bit if necessary */

int activePngClass::readpng_get_bgcolor (
  uch *red,
  uch *green,
  uch *blue
) {

png_color_16p pBackground;

  /* setjmp() must be called in every function that calls a PNG-reading
   * libpng function */

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return 2;
  }

  if (!png_get_valid(png_ptr, info_ptr, PNG_INFO_bKGD))
    return 0;

  /* it is not obvious from the libpng documentation, but this function
   * takes a pointer to a pointer, and it always returns valid red, green
   * and blue values, regardless of color_type: */

  png_get_bKGD(png_ptr, info_ptr, &pBackground);

  /* however, it always returns the raw bKGD data, regardless of any
   * bit-depth transformations, so check depth and adjust if necessary */

  if (bit_depth == 16) {
    *red   = pBackground->red   >> 8;
    *green = pBackground->green >> 8;
    *blue  = pBackground->blue  >> 8;
  } else if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
    if (bit_depth == 1)
      *red = *green = *blue = pBackground->gray? 255 : 0;
    else if (bit_depth == 2)
      *red = *green = *blue = (255/3) * pBackground->gray;
    else /* bit_depth == 4 */
      *red = *green = *blue = (255/15) * pBackground->gray;
  } else {
    *red   = (uch)pBackground->red;
    *green = (uch)pBackground->green;
    *blue  = (uch)pBackground->blue;
  }

  return 1;

}

/* display_exponent == LUT_exponent * CRT_exponent */

uch *activePngClass::readpng_get_image (
  double display_exponent,
  int *pChannels,
  ulg *pRowbytes
) {

double  gamma;
png_uint_32  i, rowbytes;
png_bytepp  row_pointers = NULL;
int numColors, ii, red, green, blue, row, col, status, alpha;
png_color *palette;
uch *src;

  /* setjmp() must be called in every function that calls a PNG-reading
   * libpng function */

  if (setjmp(png_jmpbuf(png_ptr))) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  /* expand palette images to RGB, low-bit-depth grayscale images to 8 bits,
   * transparency chunks to full alpha channel; strip 16-bit-per-sample
   * images to 8 bits per sample; and convert grayscale to RGB[A] */

  if ( color_type == PNG_COLOR_TYPE_PALETTE ) {

    usePixelArray = 1;

    if ( bit_depth < 8 ) {
      png_set_packing( png_ptr );
    }
    else if ( bit_depth > 8 ) {
      png_set_strip_16( png_ptr );
    }


  }
  else {

    usePixelArray = 0;

    if ( color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8 )
      png_set_expand(png_ptr);
    if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) )
      png_set_expand(png_ptr);
    if ( bit_depth == 16 )
      png_set_strip_16(png_ptr);
    if ( color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
      png_set_gray_to_rgb(png_ptr);

  }

  /* unlike the example in the libpng documentation, we have *no* idea where
   * this file may have come from--so if it doesn't have a file gamma, don't
   * do any correction ("do no harm") */

  if (png_get_gAMA(png_ptr, info_ptr, &gamma))
    png_set_gamma(png_ptr, display_exponent, gamma);

  /* all transformations have been registered; now update info_ptr data,
   * get rowbytes and channels, and allocate image memory */

  png_read_update_info(png_ptr, info_ptr);

  *pRowbytes = rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  *pChannels = (int)png_get_channels(png_ptr, info_ptr);

  if ((image_data = (uch *)malloc(rowbytes*height)) == NULL) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return NULL;
  }

  if ((row_pointers = (png_bytepp)malloc(height*sizeof(png_bytep))) == NULL) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    free(image_data);
    image_data = NULL;
    return NULL;
  }

  /* set the individual row_pointers to point at the correct offsets */

  for (i = 0;  i < height;  ++i)
    row_pointers[i] = image_data + i*rowbytes;

  /* now we can go ahead and just read the whole image */

  png_read_image(png_ptr, row_pointers);

  /* and we're done!  (png_read_end() can be omitted if no processing of
   * post-IDAT text/time/etc. is desired) */

  free(row_pointers);
  row_pointers = NULL;

  png_read_end(png_ptr, NULL);

  // build a color list for rgb to pixel translation

  if ( usePixelArray ) {

    png_get_PLTE( png_ptr, info_ptr, &palette,
     &numColors );

    if ( pixels ) delete[] pixels;
    pixels = new unsigned int[numColors+1];

    for ( ii=0; ii<numColors; ii++ ) {

      status = addColorToList( palette[ii].red, palette[ii].green,
       palette[ii].blue );

    }

  }
  else {

    if ( color_type == PNG_COLOR_TYPE_PALETTE ) { //bld color list from palette

      png_get_PLTE( png_ptr, info_ptr, &palette,
       &numColors );

      for ( ii=0; ii<numColors; ii++ ) {

        status = addColorToList( palette[ii].red, palette[ii].green,
         palette[ii].blue );

      }

    }
    else { // build list manually, ugh!

      for ( row=0; row<h; row++ ) {

        src = image_data + row * rowbytes;

        for ( col=w; col>0; col-- ) {

          alpha = 255;
          if ( *pChannels == 3 ) {
            red = *src++;
            green = *src++;
            blue = *src++;
          }
          else { // == 4
            red = *src++;
            green = *src++;
            blue = *src++;
            alpha = *src++;
          }

          if ( alpha != 0 ) {
            status = addColorToList( red, green, blue );
	  }

        }

      }

    }

  }

  return image_data;

}

void activePngClass::readpng_cleanup (
  int free_image_data
) {

  if (free_image_data && image_data) {
    free(image_data);
    image_data = NULL;
  }

  if (png_ptr && info_ptr) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    png_ptr = NULL;
    info_ptr = NULL;
  }

}

char *activePngClass::getSearchString (
  int index
) {

  if ( index == 0 ) {
    return pngFileName;
  }
  else {
    return NULL;
  }

}

void activePngClass::replaceString (
  int i,
  int max,
  char *string
) {

int status;
int l = 127;

  if ( max < l ) l = max;

  if ( i == 0 ) {

    strncpy( pngFileName, string, l );
    pngFileName[l] = 0;

    status = readPngFile();

    initSelectBox();

    if ( !( status & 1 ) ) {
      char msg[255+1];
      snprintf( msg, 255, activePngClass_str1, actWin->fileName,
       pngFileName );
      actWin->appCtx->postMessage( msg );
    }

  }

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activePngClassPtr ( void ) {

activePngClass *ptr;

  ptr = new activePngClass;
  return (void *) ptr;

}

void *clone_activePngClassPtr (
  void *_srcPtr )
{

activePngClass *ptr, *srcPtr;

  srcPtr = (activePngClass *) _srcPtr;

  ptr = new activePngClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
