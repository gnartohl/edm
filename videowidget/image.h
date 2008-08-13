#ifndef __image_h
#define __image_h 1

#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <math.h>
#include <stdlib.h>

class imageClass {

public:

imageClass ( void );

imageClass (
  Display *_display,
  Colormap _cmap,
  GC _gc,
  int _w,
  int _h,
  int _nbits
);

~imageClass ( void );

void preserveAspect (
  int flag
);

void update (
  int _srcW,
  int _srcH,
  double *data
);

XImage *ximage ( void );

void destroy ( void );

int validImage ( void );

private:

void transformImageData (
  int srcMaxIndex,
  int srcW,
  int srcH,
  double *src
);

// Convert value into index in the palette of XImage
unsigned char convert (
  double d
) {
	if(d<0) d = d + (1<<nbits);
	return ((unsigned int)d)>>(nbits-8);
};

Display *display;
XImage *image;
Colormap cmap;
GC gc;
int depth;
int numPixels;
unsigned long pixels[256];
unsigned char mask;
char *imageData;
int srcMaxIndex, w, h, size;
int oldDestW, oldDestH;
int valid;
int preserveAspectFlag;
int nbits;

};

#endif
