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
  int _h
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

};

#endif
