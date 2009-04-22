#include "image.h"

static unsigned char g_red[256], g_green[256], g_blue[256];

imageClass::imageClass ( void ) {

  valid = 0;

}

imageClass::imageClass (
  Display *_display,
  Colormap _cmap,
  GC _gc,
  int _w,
  int _h,
  int _nbits
) {

int i, x, y, screen_num, depth, result;
Visual *visual;
XColor color;

  display = _display;
  cmap = _cmap;
  gc = _gc;
  w = _w;
  h = _h;
  nbits = _nbits;

  screen_num = DefaultScreen( display );
  visual = DefaultVisual( display, screen_num );
  depth = DefaultDepth( display, screen_num );

  // depth must be 16 or 24

  valid = 1;

  if ( depth == 24 ) {
    size = 4;
  }
  else if ( depth == 16 ) {
    size = 2;
  }
  else {
    valid = 0;
    return;
  }

  // Allocate colors

  for ( i=0; i<256; i++ ) {
    g_red[i] = i;
    g_green[i] = i;
    g_blue[i] = i;
    pixels[i] = 0;
  }

  numPixels = 0;
  for ( i=0; i<256; i++ ) {

    mask = 0xff;

    do {

      color.flags = DoRed | DoGreen | DoBlue;
      color.red = ( g_red[i] & mask ) * 256;
      color.green = ( g_green[i] & mask ) * 256;
      color.blue = ( g_blue[i] & mask ) * 256;

      result = XAllocColor( display, cmap, &color );
      if ( result ) {
        pixels[i] = color.pixel;
        numPixels++;
      }
      else {
        mask <<= 1;
      }

    } while ( mask && !result );

  }

  // create image and image data

  imageData = (char *) malloc( w * h * size );

  image = XCreateImage( display, visual, depth, ZPixmap, 0,
   (char *) imageData, w, h, 8*size, w*size );

  for ( x=0; x<w; x++ ) {
    for ( y=0; y<h; y++ ) {
      XPutPixel( image, x, y, pixels[0] );
    }
  }

  oldDestW = oldDestH = -1;
  preserveAspectFlag = 1;

}

imageClass::~imageClass ( void ) {

}

void imageClass::preserveAspect (
  int flag
) {

  preserveAspectFlag = flag;

}

void imageClass::transformImageData (
  int srcMaxIndex,
  int srcW,
  int srcH,
  double *src
) {

int i, srcX, srcY, destX, destY, destW=1, destH=1,
 destOfsX, destOfsY, effW, effH;
double dSrcX, dSrcY, dSrcIncX, dSrcIncY, aspRatio, newW, newH;
unsigned char c;

  if ( preserveAspectFlag ) {

    aspRatio = (double) srcW / (double) srcH;

    newW = h * aspRatio;

    if ( (int) newW <= w ) {

      destH = h;
      destW = (int) newW;

    }
    else {

      newH = w / aspRatio;
      destH = (int) newH;
      destW = w;

    }

    if ( ( destW != oldDestW ) || ( destH != oldDestH ) ) {
      oldDestW = destW;
      oldDestH = destH;
      for ( destX=0; destX<w; destX++ ) {
	for ( destY=0; destY<h; destY++ ) {
	  XPutPixel( image, destX, destY, pixels[0] );
	}
      }
    }

    destOfsX = (int) ( ( w - destW ) * 0.5 );
    if ( destOfsX < 0 ) destOfsX = 0;
    if ( destOfsX > w ) destOfsX = w;
    effW = destW+destOfsX;
    if ( effW > w ) effW = w;

    destOfsY = (int) ( ( h - destH ) * 0.5 );
    if ( destOfsY < 0 ) destOfsY = 0;
    if ( destOfsY > h ) destOfsY = h;
    effH = destH+destOfsY;
    if ( effH > h ) effH = h;

  }
  else {

    destOfsX = 0;
    effW = w;
    destOfsY = 0;
    effH = h;

  }

  dSrcIncX = (double) srcW / (double) destW;
  dSrcIncY = (double) srcH / (double) destH;

  dSrcY = 0;
  for ( destY=destOfsY; destY<effH; destY++ ) {

    srcY = (int) floor( dSrcY );

    dSrcX = 0;
    for ( destX=destOfsX; destX<effW; destX++ ) {

      srcX = (int) floor( dSrcX );

      i = ( srcX + srcY*srcW );
      if ( i > srcMaxIndex ) i = srcMaxIndex;
      c = convert(src[i]);

      XPutPixel( image, destX, destY, pixels[c] );

      dSrcX += dSrcIncX;

    }

    dSrcY += dSrcIncY;

  }

}

void imageClass::update (
  int _srcW,
  int _srcH,
  double *data
) {

  if ( !valid ) return;

  srcMaxIndex = _srcW * _srcH - 1;
  transformImageData( srcMaxIndex, _srcW, _srcH, data );

}

XImage *imageClass::ximage ( void ) {

  if ( !valid ) return (XImage *) NULL;

  return image;

}

void imageClass::destroy ( void ) {

int i;

  if ( !valid ) return;

  for ( i=0; i<numPixels; i++ ) {
    XFreeColors( display, cmap, &pixels[i], 1, 0L );
  }

  // this also frees the image data
  XDestroyImage( image );

}

int imageClass::validImage ( void ) {

  return valid;

}
