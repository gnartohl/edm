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

#include "color_pkg.h"

#include "thread.h"

static int showRGB = 0;

void doColorBlink (
  XtPointer client,
  XtIntervalId *id )
{

colorInfoClass *cio = (colorInfoClass *) client;
int i;

  cio->incrementTimer = XtAppAddTimeOut( cio->appCtx,
   cio->incrementTimerValue, doColorBlink, client );

  if ( cio->blink ) {
    cio->blink = 0;
    for ( i=0; i<cio->num_blinking_colors; i++ ) {
      XStoreColor( cio->display, cio->cmap, &cio->offBlinkingXColor[i] );
    }
  }
  else {
    cio->blink = 1;
    for ( i=0; i<cio->num_blinking_colors; i++ ) {
      XStoreColor( cio->display, cio->cmap, &cio->blinkingXColor[i] );
    }
  }

  XFlush( cio->display );

}

void colorFormEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

XExposeEvent *expe;
XButtonEvent *be;
Widget curw;
colorInfoClass *cio;
int x, y, i, r, c, ncols, nrows, remainder, index;
Arg arg[10];
int n;
unsigned int fg, *dest;
int red, green, blue;

  cio = (colorInfoClass *) client;

  *continueToDispatch = False;

  if ( e->type == Expose ) {

    expe = (XExposeEvent *) e;
    if ( !expe->count ) {

      ncols = cio->num_color_cols;
      nrows = (cio->max_colors+cio->num_blinking_colors) / ncols;
      remainder = (cio->max_colors+cio->num_blinking_colors) % ncols;

      i = 0;
      for ( r=0; r<nrows; r++ ) {

        for ( c=0; c<ncols; c++ ) {

          x = c*5 + c*20 + 5;
          y = r*5 + r*20 + 5;

          cio->gc.setFG( cio->colors[i] );
          XFillRectangle( cio->display, XtWindow(cio->form), cio->gc.normGC(),
           x, y, 20, 20 );

          if ( i == cio->curIndex ) {
            cio->gc.setFG(
             BlackPixel( cio->display, DefaultScreen(cio->display) ) );
            XDrawRectangle( cio->display, XtWindow(cio->form),
             cio->gc.normGC(), x-2, y-2, 23, 23 );
          }

          i++;

        }

      }

      if ( remainder ) {

        r = nrows;

        for ( c=0; c<remainder; c++ ) {

          x = c*5 + c*20 + 5;
          y = r*5 + r*20 + 5;

          cio->gc.setFG( cio->colors[i] );
          XFillRectangle( cio->display, XtWindow(cio->form), cio->gc.normGC(),
           x, y, 20, 20 );

          if ( i == cio->curIndex ) {
            cio->gc.setFG(
             BlackPixel( cio->display, DefaultScreen(cio->display) ) );
            XDrawRectangle( cio->display, XtWindow(cio->form),
             cio->gc.normGC(), x-2, y-2, 23, 23 );
          }

          i++;

        }

      }

    }

  }
  else if ( e->type == ButtonPress ) {

    be = (XButtonEvent *) e;

    ncols = cio->num_color_cols;
    nrows = (cio->max_colors+cio->num_blinking_colors) / ncols;
    remainder = (cio->max_colors+cio->num_blinking_colors) % ncols;
    if ( remainder ) nrows++;

    r = be->y / 25;
    if ( r > nrows-1 ) r = nrows-1;
    c = be->x / 25;
    if ( c > ncols-1 ) c = ncols-1;

    i = r * ncols + c;
    if ( i > cio->numColors-1 ) i = cio->numColors-1;

    cio->setCurIndexByPixel( cio->colors[i] );

    fg = cio->colors[i];

    cio->change = 1;

    curw = cio->getActiveWidget();

    if ( curw ) {
      n = 0;
      XtSetArg( arg[n], XmNbackground, (XtArgVal) fg ); n++;
      XtSetValues( curw, arg, n );
    }

    dest = cio->getCurDestination();
    if ( dest ) {
      *dest = fg;
    }

    if ( showRGB ) {
      cio->getRGB( fg, &red, &green, &blue );
      cio->getIndex( fg, &index );
      printf( "index=%-d,  r=%-d, g=%-d, b=%-d\n", index, red, green, blue );
    }

  }

}

static int compare_nodes_by_index (
  void *node1,
  void *node2
) {

colorCachePtr p1, p2;

  p1 = (colorCachePtr) node1;
  p2 = (colorCachePtr) node2;

  if ( p1->index > p2->index )
      return 1;
  else if ( p1->index < p2->index )
    return -1;

  return 0;

}

static int compare_key_by_index (
  void *key,
  void *node
) {

colorCachePtr p;
int *oneIndex;

  p = (colorCachePtr) node;
  oneIndex = (int *) key;

  if ( *oneIndex > p->index )
      return 1;
  else if ( *oneIndex < p->index )
    return -1;

  return 0;

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
unsigned int *onePixel;

  p = (colorCachePtr) node;
  onePixel = (unsigned int *) key;

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
unsigned int *oneRgb;

  p = (colorCachePtr) node;
  oneRgb = (unsigned int *) key;

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

colorInfoClass::colorInfoClass ( void ) {

int stat;

  change = 1;
  max_colors = 0;
  num_blinking_colors = 0;
  num_color_cols = 0;
  usingPrivateColorMap = 0;

  fg = 0;
  activeWidget = NULL;
  curDestination = NULL;
  colorWindowIsOpen = 0;

  stat = avl_init_tree( compare_nodes_by_color,
   compare_key_by_color, copy_nodes, &(this->colorCacheByColorH) );
  if ( !( stat & 1 ) ) this->colorCacheByColorH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_pixel,
   compare_key_by_pixel, copy_nodes, &(this->colorCacheByPixelH) );
  if ( !( stat & 1 ) ) this->colorCacheByPixelH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_index,
   compare_key_by_index, copy_nodes, &(this->colorCacheByIndexH) );
  if ( !( stat & 1 ) ) this->colorCacheByIndexH = (AVL_HANDLE) NULL;

}

colorInfoClass::~colorInfoClass ( void ) {

//    printf( "colorInfoClass::~colorInfoClass\n" );

  XtDestroyWidget( shell );

}

static void file_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmPushButtonCallbackStruct *cb;
int num;
Widget p, curP;

  num = (int) client;
  cb = (XmPushButtonCallbackStruct *) call;

  if ( num == 0 ) {   // close window

    /* find topmost widget */
    curP = p = w;
    do {
      p = XtParent(p);
      if ( p ) curP = p;
    } while ( p );

    XtUnmapWidget( curP );

  }
  else if ( num == 1 ) {

    if ( showRGB )
     showRGB = 0;
    else
      showRGB = 1;

  }

}

int colorInfoClass::initFromFile (
  XtAppContext app,
  Display *d,
  Widget top,
  char *fileName )
{

char line[127+1], *ptr, *tk;
int i, index, iOn, iOff, n, stat, nrows, ncols, remainder, dup, nSpecial;
FILE *f;
XColor color;
Arg arg[20];
XmString str1, str2;
colorCachePtr cur, curSpecial;
int rgb[3], red, green, blue;
unsigned long plane_masks[1], bgColor;
int major, minor, release;

  if ( !this->colorCacheByColorH ) return 0;

  appCtx = app;
  display = d;
  screen = DefaultScreen( d );
  depth = DefaultDepth( d, screen );
  visual = DefaultVisual( d, screen );
  cmap = DefaultColormap( d, screen );

  goto firstTry;

restart:

  // at this point, a private color map is being used

  delete colors;
  delete blinkingColorCells;
  delete blinkingXColor;
  delete offBlinkingXColor;

  stat = avl_init_tree( compare_nodes_by_color,
   compare_key_by_color, copy_nodes, &(this->colorCacheByColorH) );
  if ( !( stat & 1 ) ) this->colorCacheByColorH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_pixel,
   compare_key_by_pixel, copy_nodes, &(this->colorCacheByPixelH) );
  if ( !( stat & 1 ) ) this->colorCacheByPixelH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_index,
   compare_key_by_index, copy_nodes, &(this->colorCacheByIndexH) );
  if ( !( stat & 1 ) ) this->colorCacheByIndexH = (AVL_HANDLE) NULL;

  fclose( f );

firstTry:

  change = 1;
  blink = 0;
  curIndex = 0;
  curX = 5;
  curY = 5;

  f = fopen( fileName, "r" );
  if ( !f ) {
    return COLORINFO_NO_FILE;
  }

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  if ( major < 2 ) {
    max_colors = 88;
    num_blinking_colors = 8;
    num_color_cols = 11;
  }
  else {
    fscanf( f, "%d %d %d\n", &max_colors, &num_blinking_colors,
     &num_color_cols );
  }

  colors = new unsigned int[max_colors+num_blinking_colors];
  blinkingColorCells = new unsigned long[num_blinking_colors];
  blinkingXColor = new XColor[num_blinking_colors];
  offBlinkingXColor = new XColor[num_blinking_colors];

  numColors = 0;

  index = 0;
  for ( i=0; i<(max_colors); i++ ) {

    ptr = fgets ( line, 127, f );
    if ( ptr ) {

      numColors++;

      tk = strtok( line, " \t\n" );
      if ( tk )
        red = atol( tk );
      else
        red = 0;

      if ( major < 2 ) red *= 256;
      color.red = red;

      tk = strtok( NULL, " \t\n" );
      if ( tk )
        green = atol( tk );
      else
        green = 0;

      if ( major < 2 ) green *= 256;
      color.green = green;

      tk = strtok( NULL, " \t\n" );
      if ( tk )
        blue = atol( tk );
      else
        blue = 0;

      if ( major < 2 ) blue *= 256;
      color.blue = blue;

      stat = XAllocColor( display, cmap, &color );

      if ( stat ) {
        colors[i] = color.pixel;
      }
      else {

        if ( !usingPrivateColorMap ) {
          usingPrivateColorMap = 1;
	  cmap = XCopyColormapAndFree( display, cmap );
	  XSetWindowColormap( display, XtWindow(top), cmap );
          goto restart;
	}

        colors[i] = BlackPixel( display, screen );

      }

    }
    else {
      if ( i ) {
        colors[i] = BlackPixel( display, screen );
      }
      else {
        colors[i] = WhitePixel( display, screen );
      }
    }

    cur = new colorCacheType;
    if ( !cur ) return 0;

    cur->rgb[0] = (unsigned int) red;
    cur->rgb[1] = (unsigned int) green;
    cur->rgb[2] = (unsigned int) blue;
    cur->pixel = colors[i];
    cur->index = index;

    stat = avl_insert_node( this->colorCacheByColorH, (void *) cur,
     &dup );
    if ( !( stat & 1 ) ) {
      delete cur;
      fclose( f );
      return stat;
    }

    if ( dup ) delete cur;

    cur = new colorCacheType;
    if ( !cur ) return 0;

    cur->rgb[0] = (unsigned int) red;
    cur->rgb[1] = (unsigned int) green;
    cur->rgb[2] = (unsigned int) blue;
    cur->pixel = colors[i];
    cur->index = index;

    stat = avl_insert_node( this->colorCacheByPixelH, (void *) cur,
     &dup );
    if ( !( stat & 1 ) ) {
      delete cur;
      fclose( f );
      return stat;
    }

    if ( dup ) delete cur;

    cur = new colorCacheType;
    if ( !cur ) return 0;

    cur->rgb[0] = (unsigned int) red;
    cur->rgb[1] = (unsigned int) green;
    cur->rgb[2] = (unsigned int) blue;
    cur->pixel = colors[i];
    cur->index = index;

    stat = avl_insert_node( this->colorCacheByIndexH, (void *) cur,
     &dup );
    if ( !( stat & 1 ) ) {
      delete cur;
      fclose( f );
      return stat;
    }

    if ( dup ) delete cur;

    index++;

  }

  stat = XAllocColorCells( display, cmap, False, plane_masks, 0,
   blinkingColorCells, num_blinking_colors );

  if ( stat ) { // success

    // blinking colors
    iOn = 0;
    iOff = 0;
    for ( i=0; i<num_blinking_colors*2; i++ ) {

      ptr = fgets ( line, 127, f );
      if ( ptr ) {

        tk = strtok( line, " \t\n" );
        if ( tk )
          red = atol( tk );
        else
          red = 0;

        if ( major < 2 ) red *= 256;
        color.red = red;

        tk = strtok( NULL, " \t\n" );
        if ( tk )
          green = atol( tk );
        else
          green = 0;

        if ( major < 2 ) green *= 256;
        color.green = green;

        tk = strtok( NULL, " \t\n" );
        if ( tk )
          blue = atol( tk );
        else
          blue = 0;

        if ( major < 2 ) blue *= 256;
        color.blue = blue;

        if ( !( i % 2 ) ) {
          color.pixel = blinkingColorCells[iOn];
          color.flags = DoRed | DoGreen | DoBlue;
          colors[numColors] = color.pixel;
          blinkingXColor[iOn] = color;
          iOn++;
          XStoreColor( display, cmap, &color );
        }
        else {
          offBlinkingXColor[iOff] = color;
          iOff++;
        }

      }
      else {
        if ( numColors ) {
          colors[numColors] = BlackPixel( display, screen );
        }
        else {
          colors[numColors] = WhitePixel( display, screen );
        }
      }

      if ( !( i % 2 ) ) {

        cur = new colorCacheType;
        if ( !cur ) return 0;

        cur->rgb[0] = (unsigned int) red;
        cur->rgb[1] = (unsigned int) green;
        cur->rgb[2] = (unsigned int) blue;
        cur->pixel = colors[numColors];
        cur->index = index;

        stat = avl_insert_node( this->colorCacheByColorH, (void *) cur,
         &dup );
        if ( !( stat & 1 ) ) {
          delete cur;
          fclose( f );
          return stat;
        }

        if ( dup ) delete cur;

        cur = new colorCacheType;
        if ( !cur ) return 0;

        cur->rgb[0] = (unsigned int) red;
        cur->rgb[1] = (unsigned int) green;
        cur->rgb[2] = (unsigned int) blue;
        cur->pixel = colors[numColors];
        cur->index = index;

        stat = avl_insert_node( this->colorCacheByPixelH, (void *) cur,
         &dup );
        if ( !( stat & 1 ) ) {
          delete cur;
          fclose( f );
          return stat;
        }

        if ( dup ) delete cur;

        cur = new colorCacheType;
        if ( !cur ) return 0;

        cur->rgb[0] = (unsigned int) red;
        cur->rgb[1] = (unsigned int) green;
        cur->rgb[2] = (unsigned int) blue;
        cur->pixel = colors[numColors];
        cur->index = index;

        stat = avl_insert_node( this->colorCacheByIndexH, (void *) cur,
         &dup );
        if ( !( stat & 1 ) ) {
          delete cur;
          fclose( f );
          return stat;
        }

        if ( dup ) delete cur;

        numColors++;
        index++;

      }

    }

  }
  else {

    if ( !usingPrivateColorMap ) {
      usingPrivateColorMap = 1;
      cmap = XCopyColormapAndFree( display, cmap );
      XSetWindowColormap( display, XtWindow(top), cmap );
      goto restart;
    }

    printf( colorInfoClass_str1 );
    // discard file contents
    for ( i=0; i<num_blinking_colors*2; i++ ) {
      ptr = fgets ( line, 127, f );
    }
    num_blinking_colors = 0;

  }

  // special colors are disconnected, severity=invalid,
  // severity=minor, severity=major, severity=noalarm
  if ( major > 2 ) {
    nSpecial = NUM_SPECIAL_COLORS;
  }
  else {
    nSpecial = NUM_SPECIAL_COLORS - 2; // don't include ack alarm colors
    special[NUM_SPECIAL_COLORS - 2] = (int) BlackPixel( display, screen );
    special[NUM_SPECIAL_COLORS - 1] = (int) BlackPixel( display, screen );
  }

  for ( i=0; i<nSpecial; i++ ) {

    ptr = fgets ( line, 127, f );
    if ( ptr ) {

      tk = strtok( line, " \t\n" );
      if ( tk )
        rgb[0] = atol( tk );
      else
        rgb[0] = 0;

      tk = strtok( NULL, " \t\n" );
      if ( tk )
        rgb[1] = atol( tk );
      else
        rgb[1] = 0;

      tk = strtok( NULL, " \t\n" );
      if ( tk )
        rgb[2] = atol( tk );
      else
        rgb[2] = 0;

      if ( rgb[0] != -1 ) {

        if ( major < 2 ) {
          rgb[0] *= 256;
          rgb[1] *= 256;
          rgb[2] *= 256;
        }

        stat = avl_get_match( this->colorCacheByColorH, (void *) rgb,
         (void **) &curSpecial );

        if ( ( stat & 1 ) && curSpecial ) {
          special[i] = (int) curSpecial->pixel;
        }
        else {
          special[i] = (int) BlackPixel( display, screen );
        }

      }
      else {

        special[i] = -1;

      }

    }
    else {
      special[i] = (int) BlackPixel( display, screen );
    }

  }

  fclose( f );

  // create window

  shell = XtVaAppCreateShell( colorInfoClass_str2, colorInfoClass_str2,
   topLevelShellWidgetClass,
   XtDisplay(top),
   XtNmappedWhenManaged, False,
   NULL );

  rc = XtVaCreateWidget( "", xmRowColumnWidgetClass, shell,
   XmNorientation, XmVERTICAL,
   XmNnumColumns, 1,
   NULL );

  str1 = XmStringCreateLocalized( colorInfoClass_str3 );
  mbar = XmVaCreateSimpleMenuBar( rc, "",
   XmVaCASCADEBUTTON, str1, 'f',
   NULL );
  XmStringFree( str1 );

  str1 = XmStringCreateLocalized( colorInfoClass_str4 );
  str2 = XmStringCreateLocalized( colorInfoClass_str5 );
  mb1 = XmVaCreateSimplePulldownMenu( mbar, "", 0, file_cb,
   XmVaPUSHBUTTON, str1, 'x', NULL, NULL,
   XmVaPUSHBUTTON, str2, 's', NULL, NULL,
   NULL );
  XmStringFree( str1 );
  XmStringFree( str2 );

//   form = XtVaCreateManagedWidget( "", xmFormWidgetClass, rc,
//    NULL );

  ncols = num_color_cols;
  nrows = (max_colors+num_blinking_colors) / ncols;
  remainder = (max_colors+num_blinking_colors) % ncols;
  if ( remainder ) nrows++;

  form = XtVaCreateManagedWidget( "", xmDrawingAreaWidgetClass, rc,
   XmNwidth, ncols*20 + ncols*5 + 5,
   XmNheight, nrows*20 + nrows*5 + 5,
   NULL );

  XtAddEventHandler( form,
   ButtonPressMask|ExposureMask, False,
   colorFormEventHandler, (XtPointer) this );

  Atom wm_delete_window = XmInternAtom( XtDisplay(shell), "WM_DELETE_WINDOW",
   False );

  XmAddWMProtocolCallback( shell, wm_delete_window, file_cb,
    (int) 0 );

  XtVaSetValues( shell, XmNdeleteResponse, XmDO_NOTHING, NULL );

  XtManageChild( mbar );
  XtManageChild( rc );
  XtRealizeWidget( shell );
  XSetWindowColormap( display, XtWindow(shell), cmap );

  gc.create( shell );

   n = 0;
   XtSetArg( arg[n], XmNbackground, &bgColor ); n++;
   XtGetValues( form, arg, n );

  gc.setBG( bgColor );
  gc.setBaseBG( bgColor );

  if ( num_blinking_colors ) {
    incrementTimerValue = 500;
    incrementTimer = XtAppAddTimeOut( appCtx, incrementTimerValue,
     doColorBlink, this );
  }

  return 1;

}

int colorInfoClass::openColorWindow( void ) {

  XtMapWidget( shell );
  XRaiseWindow( display, XtWindow(shell) );

  colorWindowIsOpen = 1;

  return 1;

}

int colorInfoClass::closeColorWindow( void ) {

  XtUnmapWidget( shell );

  colorWindowIsOpen = 0;

  return 1;

}

unsigned int colorInfoClass::getFg( void ) {

  return fg;

}

void colorInfoClass::setCurDestination( unsigned int *ptr ) {

  curDestination = ptr;

}

unsigned int *colorInfoClass::getCurDestination( void ) {

  return curDestination;

}

int colorInfoClass::setActiveWidget( Widget w ) {

  activeWidget = w;

  return 1;

}

Widget colorInfoClass::getActiveWidget( void ) {

  return activeWidget;

}

int colorInfoClass::getRGB(
  unsigned int pixel,
  int *r,
  int *g,
  int *b )
{

XColor color;
int stat, dup;
colorCachePtr cur;

  stat = avl_get_match( this->colorCacheByPixelH, (void *) &pixel,
   (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( cur ) {
    *r = cur->rgb[0];
    *g = cur->rgb[1];
    *b = cur->rgb[2];
    return COLORINFO_SUCCESS;
  }

  *r = 0;
  *g = 0;
  *b = 0;
  color.pixel = pixel;
  stat = XQueryColor( display, cmap, &color );

  if ( !stat ) return COLORINFO_FAIL;

  *r = (int) color.red;
  *g = (int) color.green;
  *b = (int) color.blue;

  cur = new colorCacheType;
  if ( !cur ) return 0;

  cur->rgb[0] = (unsigned int) *r;
  cur->rgb[1] = (unsigned int) *g;
  cur->rgb[2] = (unsigned int) *b;
  cur->pixel = (unsigned int) pixel;
  cur->index = -1;

  stat = avl_insert_node( this->colorCacheByPixelH, (void *) cur,
   &dup );
  if ( !( stat & 1 ) ) {
    delete cur;
    return stat;
  }

  if ( dup ) delete cur;

  cur = new colorCacheType;
  if ( !cur ) return 0;

  cur->rgb[0] = (unsigned int) *r;
  cur->rgb[1] = (unsigned int) *g;
  cur->rgb[2] = (unsigned int) *b;
  cur->pixel = (unsigned int) pixel;
  cur->index = -1;

  stat = avl_insert_node( this->colorCacheByColorH, (void *) cur,
   &dup );
  if ( !( stat & 1 ) ) {
    delete cur;
    return stat;
  }

  if ( dup ) delete cur;

  return COLORINFO_SUCCESS;

}

int colorInfoClass::setRGB (
  int r,
  int g,
  int b,
  unsigned int *pixel )
{

int stat;
unsigned int rgb[3];
colorCachePtr cur;
int diff, bestPixel, bestR, bestG, bestB, min;

  rgb[0] = (unsigned int) r;
  rgb[1] = (unsigned int) g;
  rgb[2] = (unsigned int) b;

  stat = avl_get_match( this->colorCacheByColorH, (void *) rgb,
   (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( cur ) {
    *pixel = cur->pixel;
    return COLORINFO_SUCCESS;
  }

  bestPixel = -1;
  bestR = 0;
  bestG = 0;
  bestB = 0;

  stat = avl_get_first( this->colorCacheByColorH, (void **) &cur );
  if ( !( stat & 1 ) ) return COLORINFO_FAIL;

  if ( cur ) {

    min = abs( r - cur->rgb[0] ) + abs( g - cur->rgb[1] ) +
          abs( b - cur->rgb[2] );
    bestR = cur->rgb[0];
    bestG = cur->rgb[1];
    bestB = cur->rgb[2];
  }

  while ( cur ) {

    diff = abs( r - cur->rgb[0] ) + abs( g - cur->rgb[1] ) +
          abs( b - cur->rgb[2] );
    if ( diff < min ) {
      min = diff;
      bestPixel = cur->pixel;
      bestR = cur->rgb[0];
      bestG = cur->rgb[1];
      bestB = cur->rgb[2];
    }

    stat = avl_get_next( this->colorCacheByColorH, (void **) &cur );
    if ( !( stat & 1 ) ) return COLORINFO_FAIL;

  }

  if ( bestPixel == -1 ) return COLORINFO_FAIL;

  *pixel = (unsigned int) bestPixel;

  return COLORINFO_SUCCESS;

}

int colorInfoClass::getIndex(
  unsigned int pixel,
  int *index )
{

int stat;
colorCachePtr cur;

  stat = avl_get_match( this->colorCacheByPixelH, (void *) &pixel,
   (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( cur ) {
    *index = cur->index;
    return COLORINFO_SUCCESS;
  }

  return COLORINFO_FAIL;

}

int colorInfoClass::setIndex (
  int index,
  unsigned int *pixel )
{

int stat;
colorCachePtr cur;;

  stat = avl_get_match( this->colorCacheByIndexH, (void *) &index,
   (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( cur ) {
    *pixel = cur->pixel;
    return COLORINFO_SUCCESS;
  }

  return COLORINFO_FAIL;

}

int colorInfoClass::getSpecialColor (
  int index ) {

  if ( index < 0 ) return -1;
  if ( index >= NUM_SPECIAL_COLORS ) return -1;

  return special[index];

}

Colormap colorInfoClass::getColorMap ( void ) {

  return cmap;

}

int colorInfoClass::setCurIndexByPixel (
  unsigned int pixel ) {

int x, y, i, r, c, ncols, nrows, remainder;

  for ( i=0; i<max_colors+num_blinking_colors; i++ ) {

    if ( colors[i] == pixel ) {

      curIndex = i;
      break;

    }

  }

  XDrawRectangle( display, XtWindow(form), gc.eraseGC(), curX-2, curY-2,
   23, 23 );

  ncols = num_color_cols;
  nrows = (max_colors+num_blinking_colors) / ncols;
  remainder = (max_colors+num_blinking_colors) % ncols;

  i = 0;
  for ( r=0; r<nrows; r++ ) {

    for ( c=0; c<ncols; c++ ) {

      if ( i == curIndex ) {
        x = c*5 + c*20 + 5;
        y = r*5 + r*20 + 5;
        gc.setFG( BlackPixel( display, DefaultScreen(display) ) );
        XDrawRectangle( display, XtWindow(form), gc.normGC(), x-2, y-2,
         23, 23 );
        curX = x;
        curY = y;
      }

      i++;

    }

  }

  if ( remainder ) {

    r = nrows;

    for ( c=0; c<remainder; c++ ) {

      if ( i == curIndex ) {
        x = c*5 + c*20 + 5;
        y = r*5 + r*20 + 5;
        gc.setFG( BlackPixel( display, DefaultScreen(display) ) );
        XDrawRectangle( display, XtWindow(form), gc.normGC(), x-2, y-2,
         23, 23 );
        curX = x;
        curY = y;
      }

      i++;

    }

  }

  return 1;

}

int colorInfoClass::canDiscardPixel (
  unsigned int pixel )
{

int stat;
colorCachePtr cur;

  stat = avl_get_match( this->colorCacheByPixelH, (void *) &pixel,
   (void **) &cur );
  if ( !(stat & 1) ) return 0;

  if ( cur )
    return 0;
  else
    return 1;

}

unsigned int colorInfoClass::getPixelByIndex (
  int index )
{

  if ( index >= max_colors+num_blinking_colors )
    return BlackPixel( display, screen );

  if ( index < 0 ) return WhitePixel( display, screen );

  return colors[index];

}
