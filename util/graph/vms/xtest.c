#include stdlib
#include stdio
#include math
#include ctype
#include descrip
#include ssdef

#include "decw$include:xlib.h"
#include "decw$include:xutil.h"
#include "decw$include:cursorfont.h"
#include "decw$include:xresource.h"
#include "decw$include:keysym.h"

#define MAKE_FIXED_DESCRIP( list, num_bytes, d_list )\
  d_list.dsc$w_length = num_bytes;\
  d_list.dsc$b_dtype = 14;\
  d_list.dsc$b_class = 1;\
  d_list.dsc$a_pointer = list;

static int g_screen_num;
static Visual *g_visual;
static Colormap g_colormap;
static Display *g_display;
static int g_foreground, g_background;
static int g_display_height, g_display_width;
static g_init = 1;

main() {

Window w;
int stat, ret_stat;
XExposeEvent ev;
long mask;
char display_name[63+1];

  strcpy( display_name, "" );

  /* open display */
  g_display = XOpenDisplay( display_name );
  if ( !g_display ) {
    printf( "Can\'t open display\n" );
    exit(1);
  }

  g_screen_num = DefaultScreen( g_display );
  g_visual = DefaultVisual( g_display, g_screen_num );
  g_colormap = DefaultColormap( g_display, g_screen_num );
  g_display_height = DisplayHeight( g_display, g_screen_num );
  g_display_width = DisplayWidth( g_display, g_screen_num );
  g_background = WhitePixel( g_display, g_screen_num );
  g_foreground = BlackPixel( g_display, g_screen_num );

  ev.type = Expose;
  mask = ExposureMask;
  stat = XSendEvent( g_display, w, True, mask, &ev );

}
