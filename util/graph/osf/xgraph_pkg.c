#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "xgraph_priv.h"

#define TRUE 1
#define FALSE 0

typedef struct win_list_tag {
  Window w;
  void *id;
  int id_type;
  struct win_list_tag *flink;
} WIN_LIST_TYPE, *WIN_LIST_PTR;

static WIN_LIST_PTR g_win_head=NULL, g_win_tail=NULL;

static int g_screen_num;
static Visual *g_visual;
static Colormap g_colormap;
static Display *g_display;
static int g_foreground, g_background;
static int g_display_height, g_display_width;
static g_init = 1;

int xgraph_sync (
  int flag
) {

  XSynchronize( g_display, flag );

}

static int xgraph___full_font_name (
  char *partial_font_name,
  char *full_font_name
) {

int stat;
char *ptr, buf[127+1];

  /* if there is a space in the font spec then assume it is the short
  ** form: "family weight size", e.g. "helvetica medium 10"
  */

  if ( strstr( partial_font_name, " " ) ) {

    strncpy( buf, partial_font_name, 127 );

    strcpy( full_font_name, "-*-" );

    ptr = strtok( buf, " " );
    if ( ptr )
      strcat( full_font_name, ptr );
    else
      return 0;

    strcat( full_font_name, "-" );

    ptr = strtok( NULL, " " );
    if ( ptr )
      strcat( full_font_name, ptr );
    else
      return 0;

    strcat( full_font_name, "-r-normal--*-" );

    ptr = strtok( NULL, " " );
    if ( ptr )
      strcat( full_font_name, ptr );
    else
      return 0;

    strcat( full_font_name, "0-*-*-*-*-iso8859-*" );

  }
  else {

    strcpy( full_font_name, partial_font_name );

  }

  return XGRAPH_SUCCESS;

}

static int xgraph___log_adjust_low_limit (
  double *vlimit
) {

double limit;

  if ( *vlimit == 0.0 ) return XGRAPH_BAD_LOG_VALUE;

  *vlimit = log10(*vlimit);
  limit = (double) ( (int) *vlimit );
  if ( limit != *vlimit ) *vlimit = limit - 1.0;

  return XGRAPH_SUCCESS;

}

static int xgraph___log_adjust_high_limit (
  double *vlimit
) {

double limit;

  if ( *vlimit == 0.0 ) return XGRAPH_BAD_LOG_VALUE;

  *vlimit = log10(*vlimit);
  limit = (double) ( (int) *vlimit );
  if ( limit != *vlimit ) *vlimit = limit + 1.0;

  return XGRAPH_SUCCESS;

}

static void xgraph___discard_trailing_blanks (
  char *text
) {

int i;

  for ( i=strlen(text)-1; i>0; i-- ) {

    if ( ( text[i] == ' ' ) || ( text[i] == '.' ) )
      text[i] = 0;
    else
      return;

  }

}

static int xgraph___alloc_color (
  char *color_name,
  Colormap cmap,
  XColor *best_color
) {

int stat;
XColor color;

  stat = XLookupColor( g_display, cmap, color_name, &color, best_color );
  if ( stat ) {
    stat = XAllocColor( g_display, cmap, best_color );
    if ( stat ) return XGRAPH_SUCCESS;
  }

  return XGRAPH_NO_COLOR;

}

int xgraph_init(
  char *display_name
) {

int stat;

  if ( g_init ) {

    /* open display */
    g_display = XOpenDisplay( display_name );
    if ( !g_display ) return XGRAPH_NO_DISPLAY;

    g_screen_num = DefaultScreen( g_display );
    g_visual = DefaultVisual( g_display, g_screen_num );
    g_colormap = DefaultColormap( g_display, g_screen_num );
    g_display_height = DisplayHeight( g_display, g_screen_num );
    g_display_width = DisplayWidth( g_display, g_screen_num );
    g_background = WhitePixel( g_display, g_screen_num );
    g_foreground = BlackPixel( g_display, g_screen_num );

    g_win_head = (WIN_LIST_PTR) calloc( 1, sizeof(WIN_LIST_TYPE) );
    if ( !g_win_head ) return XGRAPH_NO_MEM;
    g_win_tail = g_win_head;
    g_win_tail->flink = NULL;

    g_init = 0;

  }

  return XGRAPH_SUCCESS;

}

int xgraph_create_window (
  double origin_x,       /* percent */
  double origin_y,       /* percent */
  double width,          /* percent */
  double height,         /* percent */
  char *window_name,
  char *icon_name,
  char *bg_color_name,
  GWIN_ID *id
) {

GWIN_PRIV_ATTR_PTR attr_ptr;
GWIN_PRIV_PTR gwin_ptr;
XTextProperty wname, iname;
int stat, ret_stat;
unsigned long event_mask;
WIN_LIST_PTR cur_win;
XGCValues values;

  /* allocation graph window object */
  gwin_ptr = (GWIN_PRIV_PTR) calloc( 1, sizeof(GWIN_PRIV_TYPE) );
  if ( !gwin_ptr ) return XGRAPH_NO_MEM;

  /* allocate attributes ptr */
  attr_ptr = (GWIN_PRIV_ATTR_PTR) calloc( 1, sizeof(GWIN_PRIV_ATTR_TYPE) );
  if ( !attr_ptr ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }

  /* init attributes */
  attr_ptr->window_name = (char *) malloc( strlen(window_name)+1 );
  if ( !attr_ptr->window_name ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }
  strcpy( attr_ptr->window_name, window_name );

  attr_ptr->icon_name = (char *) malloc( strlen(icon_name)+1 );
  if ( !attr_ptr->icon_name ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }
  strcpy( attr_ptr->icon_name, icon_name );

  attr_ptr->dsp = g_display;

  attr_ptr->gc = DefaultGC( g_display, g_screen_num );

  attr_ptr->xorgc = XCreateGC( g_display, RootWindow(g_display,g_screen_num),
   0, &values );

  XCopyGC( g_display, attr_ptr->gc, 0xffffffff, attr_ptr->xorgc );

  values.function = GXxor;
  XChangeGC( g_display, attr_ptr->xorgc, GCFunction, &values );

  stat = xgraph___alloc_color( bg_color_name, g_colormap, &attr_ptr->bg_color );
  if ( stat == XGRAPH_SUCCESS )
    attr_ptr->attributes.background_pixel = attr_ptr->bg_color.pixel;
  else {
    attr_ptr->bg_color.pixel = g_background;
    attr_ptr->attributes.background_pixel = g_background;
  }

  attr_ptr->sizehints.height = height * 0.01 * g_display_height;
  if ( attr_ptr->sizehints.height < 0 ) attr_ptr->sizehints.height = 0;
  if ( attr_ptr->sizehints.height > g_display_height )
    attr_ptr->sizehints.height = g_display_height;

  attr_ptr->sizehints.width = width * 0.01 * g_display_width;
  if ( attr_ptr->sizehints.width < 0 ) attr_ptr->sizehints.width = 0;
  if ( attr_ptr->sizehints.width > g_display_width )
    attr_ptr->sizehints.width = g_display_width;

  attr_ptr->sizehints.x = origin_x * 0.01 * g_display_width-1;
  if ( attr_ptr->sizehints.x < 0 ) attr_ptr->sizehints.x = 0;
  if ( attr_ptr->sizehints.x >= g_display_width )
    attr_ptr->sizehints.x = g_display_width - 1;

  attr_ptr->sizehints.y = origin_y * 0.01 * g_display_height-1;
  if ( attr_ptr->sizehints.y < 0 ) attr_ptr->sizehints.y = 0;
  if ( attr_ptr->sizehints.y >= g_display_height )
    attr_ptr->sizehints.y = g_display_height - 1;

  attr_ptr->sizehints.y = g_display_height - 1 - attr_ptr->sizehints.y -
   attr_ptr->sizehints.height;

  attr_ptr->sizehints.flags = USPosition | USSize;

  attr_ptr->x = attr_ptr->sizehints.x;
  attr_ptr->y = attr_ptr->sizehints.y;
  attr_ptr->w = attr_ptr->sizehints.width;
  attr_ptr->h = attr_ptr->sizehints.height;

  gwin_ptr->id_type = GRAPH_K_GWIN_ID;
  gwin_ptr->obj_head = NULL;
  gwin_ptr->obj_tail = NULL;
  gwin_ptr->attr = attr_ptr;

  /* create the window */
  attr_ptr->win = XCreateWindow( g_display, DefaultRootWindow( g_display ),
   attr_ptr->sizehints.x, attr_ptr->sizehints.y, attr_ptr->sizehints.width,
   attr_ptr->sizehints.height, 1, DefaultDepth( g_display, g_screen_num ),
   InputOutput, CopyFromParent, CWBackPixel, &attr_ptr->attributes );

  XStringListToTextProperty( &attr_ptr->window_name, 1, &wname );
  XStringListToTextProperty( &attr_ptr->icon_name, 1, &iname );

  XSetWMName( g_display, attr_ptr->win, &wname );
  XSetWMIconName( g_display, attr_ptr->win, &iname );

  event_mask = ExposureMask;
  XSelectInput( g_display, attr_ptr->win, event_mask );
  XMapWindow( g_display, attr_ptr->win );

  /* create sentinel nodes for object list */
  gwin_ptr->obj_head = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
  if ( !gwin_ptr ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }

  gwin_ptr->obj_tail = gwin_ptr->obj_head;
  gwin_ptr->obj_tail->next = NULL;

  cur_win = (WIN_LIST_PTR) calloc( 1, sizeof(WIN_LIST_TYPE) );
  if ( !cur_win ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }
  cur_win->w = attr_ptr->win;
  cur_win->id = (void *) gwin_ptr;
  cur_win->id_type = GRAPH_K_GWIN_ID;
  g_win_tail->flink = cur_win;
  g_win_tail = cur_win;
  g_win_tail->flink = NULL;

  *id = (GWIN_ID *) gwin_ptr;

norm_return:
  return XGRAPH_SUCCESS;

err_return:
  gwin_ptr->id_type = 0;
  *id = (GWIN_ID *) gwin_ptr;
  return ret_stat;

}

int xgraph_clear_window (
  GWIN_ID id
) {

GWIN_PRIV_PTR ptr;
int stat;

  ptr = (GWIN_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  XClearWindow( ptr->attr->dsp, ptr->attr->win );

}

int xgraph_close_window (
  GWIN_ID id
) {

GWIN_PRIV_PTR ptr;
int stat;

  ptr = (GWIN_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  XUnmapWindow( ptr->attr->dsp, ptr->attr->win );

}

int xgraph_open_window (
  GWIN_ID id
) {

GWIN_PRIV_PTR ptr;
int stat;

  ptr = (GWIN_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  XMapWindow( ptr->attr->dsp, ptr->attr->win );

}

int xgraph_lower_window (
  GWIN_ID id
) {

GWIN_PRIV_PTR ptr;
int stat;

  ptr = (GWIN_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  XLowerWindow( ptr->attr->dsp, ptr->attr->win );

}

int xgraph_raise_window (
  GWIN_ID id
) {

GWIN_PRIV_PTR ptr;
int stat;

  ptr = (GWIN_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  XRaiseWindow( ptr->attr->dsp, ptr->attr->win );

}

int xgraph_create_area (
  GWIN_ID win_id,
  double origin_x,       /* percent */
  double origin_y,       /* percent */
  double width,          /* percent */
  double height,         /* percent */
  int x_scale_type,
  int y_scale_type,
  double xmin,
  double xmax,
  double ymin,
  double ymax,
  char *fg_color_name,
  char *bg_color_name,
  char *title_font,
  char *label_font,
  int area_border_size,
  AREA_ID *area_id
) {

AREA_PRIV_ATTR_PTR attr_ptr;
AREA_PRIV_PTR area_ptr;
GWIN_PRIV_PTR gwin_ptr;
OBJ_LIST_PTR cur_obj;
int stat, ret_stat, top_margin, bottom_margin, left_margin, right_margin;
unsigned long event_mask;
double min, max;
WIN_LIST_PTR cur_win;
XGCValues values;
char full_font[127+1];

  if ( x_scale_type == XGRAPH_SCALE_LOG ) {
    stat = xgraph___log_adjust_low_limit( &xmin );
    stat = xgraph___log_adjust_high_limit( &xmax );
  }

  if ( y_scale_type == XGRAPH_SCALE_LOG ) {
    stat = xgraph___log_adjust_low_limit( &ymin );
    stat = xgraph___log_adjust_high_limit( &ymax );
  }

  gwin_ptr = (GWIN_PRIV_PTR) win_id;

  /* allocation area window object */
  area_ptr = (AREA_PRIV_PTR) calloc( 1, sizeof(AREA_PRIV_TYPE) );
  if ( !area_ptr ) return XGRAPH_NO_MEM;

  if ( !gwin_ptr ) {
    ret_stat = XGRAPH_BAD_OBJ;
    goto err_return;
  }
  if ( !gwin_ptr->id_type ) {
    ret_stat = XGRAPH_BAD_OBJ;
    goto err_return;
  }

  /* allocate attributes ptr */
  attr_ptr = (AREA_PRIV_ATTR_PTR) calloc( 1, sizeof(AREA_PRIV_ATTR_TYPE) );
  if ( !attr_ptr ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }

  /* init attributes */
  attr_ptr->dsp = g_display;

  attr_ptr->gc = DefaultGC( g_display, g_screen_num );

  attr_ptr->xorgc = XCreateGC( g_display, RootWindow(g_display,g_screen_num),
   0, &values );

  XCopyGC( g_display, attr_ptr->gc, 0xffffffff, attr_ptr->xorgc );

  values.function = GXxor;
  XChangeGC( g_display, attr_ptr->xorgc, GCFunction, &values );

  stat = xgraph___alloc_color( bg_color_name, g_colormap, &attr_ptr->bg_color );
  if ( stat == XGRAPH_SUCCESS )
    attr_ptr->attributes.background_pixel = attr_ptr->bg_color.pixel;
  else {
    attr_ptr->bg_color.pixel = g_background;
    attr_ptr->attributes.background_pixel = g_background;
  }

  stat = xgraph___alloc_color( fg_color_name, g_colormap, &attr_ptr->fg_color );
  if ( stat != XGRAPH_SUCCESS ) attr_ptr->fg_color.pixel = g_foreground;

  stat = xgraph___full_font_name( title_font, full_font );
  if ( stat != XGRAPH_SUCCESS ) strncpy( full_font, title_font, 127 );

  attr_ptr->title_fontstruct = XLoadQueryFont( g_display, full_font );
  if ( attr_ptr->title_fontstruct )
    attr_ptr->title_font_height = attr_ptr->title_fontstruct->ascent +
     attr_ptr->title_fontstruct->descent;
  else {
    attr_ptr->title_font_height = 0;
    ret_stat = XGRAPH_NO_FONT;
    goto err_return;
  }

  stat = xgraph___full_font_name( label_font, full_font );
  if ( stat != XGRAPH_SUCCESS ) strncpy( full_font, label_font, 127 );

  attr_ptr->label_fontstruct = XLoadQueryFont( g_display, full_font );
  if ( attr_ptr->label_fontstruct )
    attr_ptr->label_font_height = attr_ptr->label_fontstruct->ascent +
     attr_ptr->label_fontstruct->descent;
  else {
    attr_ptr->label_font_height = 0;
    ret_stat = XGRAPH_NO_FONT;
    goto err_return;
  }

  attr_ptr->h = height * 0.01 * gwin_ptr->attr->h;
  if ( attr_ptr->h < 0 ) attr_ptr->h = 0;
  if ( attr_ptr->h > gwin_ptr->attr->h ) attr_ptr->h = gwin_ptr->attr->h;

  attr_ptr->w = width * 0.01 * gwin_ptr->attr->w;
  if ( attr_ptr->w < 0 ) attr_ptr->w = 0;
  if ( attr_ptr->w > gwin_ptr->attr->w ) attr_ptr->w = gwin_ptr->attr->w;

  attr_ptr->x = origin_x * 0.01 * gwin_ptr->attr->w-1;
  if ( attr_ptr->x < 0 ) attr_ptr->x = 0;
  if ( attr_ptr->x >= gwin_ptr->attr->w ) attr_ptr->x = gwin_ptr->attr->w-1;

  attr_ptr->y = origin_y * 0.01 * gwin_ptr->attr->h-1;
  if ( attr_ptr->y < 0 ) attr_ptr->y = 0;
  if ( attr_ptr->y >= gwin_ptr->attr->h ) attr_ptr->y = gwin_ptr->attr->h-1;

  attr_ptr->y = gwin_ptr->attr->h - 1 - attr_ptr->y - attr_ptr->h;

  attr_ptr->x_scale_type = x_scale_type;
  attr_ptr->y_scale_type = y_scale_type;

  area_ptr->id_type = GRAPH_K_AREA_ID;
  area_ptr->obj_head = NULL;
  area_ptr->obj_tail = NULL;
  area_ptr->attr = attr_ptr;

  /* create the area window */
  attr_ptr->win = XCreateSimpleWindow( g_display, gwin_ptr->attr->win,
   attr_ptr->x, attr_ptr->y, attr_ptr->w, attr_ptr->h, area_border_size,
   attr_ptr->fg_color.pixel, attr_ptr->bg_color.pixel );

  /* do plot region init */
  top_margin = attr_ptr->title_font_height + 3 +
   attr_ptr->label_font_height + 3;
  bottom_margin = attr_ptr->label_font_height * 4;
  left_margin = XTextWidth( attr_ptr->label_fontstruct, "888888888888", 12 ) +
   attr_ptr->label_font_height * 2;
  right_margin = 0.05 * attr_ptr->w;

  attr_ptr->plot_x = left_margin;
  attr_ptr->plot_y = top_margin;
  attr_ptr->plot_w = attr_ptr->w - left_margin - right_margin;
  attr_ptr->plot_h = attr_ptr->h - top_margin - bottom_margin;

  attr_ptr->xmin = xmin;
  attr_ptr->xmax = xmax;
  attr_ptr->ymin = ymin;
  attr_ptr->ymax = ymax;

  attr_ptr->xfact = (double) ( attr_ptr->plot_w - 1 ) / ( xmax - xmin );
  attr_ptr->xofs = (double) ( attr_ptr->xfact * xmin * -1.0 );
  attr_ptr->yfact = (double) ( attr_ptr->plot_h - 1 ) / ( ymax - ymin );
  attr_ptr->yofs = (double) ( attr_ptr->yfact * ymin * -1.0 );

  /* create the plot region window */
  attr_ptr->plotwin = XCreateSimpleWindow( g_display, attr_ptr->win,
   attr_ptr->plot_x, attr_ptr->plot_y, attr_ptr->plot_w, attr_ptr->plot_h,
   0, attr_ptr->fg_color.pixel, attr_ptr->bg_color.pixel );

  event_mask = ExposureMask;
  XSelectInput( g_display, attr_ptr->win, event_mask );
  XMapWindow( g_display, attr_ptr->win );

  event_mask = ExposureMask | ButtonPressMask;
  XSelectInput( g_display, attr_ptr->plotwin, event_mask );
  XMapWindow( g_display, attr_ptr->plotwin );

  /* create sentinel nodes for object list */
  area_ptr->obj_head = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
  if ( !area_ptr ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }

  area_ptr->obj_tail = area_ptr->obj_head;
  area_ptr->obj_tail->next = NULL;

  /* link area ptr into gwin list */
  cur_obj = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
  if ( !cur_obj ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }

  cur_obj->ptr = (void *) area_ptr;
  gwin_ptr->obj_tail->next = cur_obj;
  gwin_ptr->obj_tail = cur_obj;
  gwin_ptr->obj_tail->next = NULL;

  /* add info to window list */

  cur_win = (WIN_LIST_PTR) calloc( 1, sizeof(WIN_LIST_TYPE) );
  if ( !cur_win ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }
  cur_win->w = attr_ptr->win;
  cur_win->id = (void *) area_ptr;
  cur_win->id_type = GRAPH_K_AREA_ID;
  g_win_tail->flink = cur_win;
  g_win_tail = cur_win;
  g_win_tail->flink = NULL;

  cur_win = (WIN_LIST_PTR) calloc( 1, sizeof(WIN_LIST_TYPE) );
  if ( !cur_win ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }
  cur_win->w = attr_ptr->plotwin;
  cur_win->id = (void *) area_ptr;
  cur_win->id_type = GRAPH_K_PLOT_AREA_ID;
  g_win_tail->flink = cur_win;
  g_win_tail = cur_win;
  g_win_tail->flink = NULL;

  *area_id = (AREA_ID *) area_ptr;

norm_return:
  return XGRAPH_SUCCESS;

err_return:
  area_ptr->id_type = 0;
  *area_id = (AREA_ID *) area_ptr;
  return ret_stat;

}

int xgraph_crosshair_area_cursor (
  AREA_ID id,
  char *fg_name,
  char *bg_name
) {

AREA_PRIV_PTR ptr;
int stat;
static Cursor crosshair;
static init = 1;

#define source_x_hot 7
#define source_y_hot 7
Pixmap pm;
Pixmap spm;
static char source[] = {
   0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x40, 0x01, 0x0e, 0x38, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
   0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
Pixmap mpm;
static char mask[] = {
   0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x40, 0x01, 0x0e, 0x38, 0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
   0x80, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
XColor fg, bg;
XColor Color;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( init ) {
    init = 0;
    stat = xgraph___alloc_color( bg_name, g_colormap, &bg );
    stat = xgraph___alloc_color( fg_name, g_colormap, &fg );
    pm = XCreatePixmap( g_display, XDefaultRootWindow(g_display), 1, 1, 1 );
    spm = XCreatePixmapFromBitmapData( g_display, pm, source,
     16, 16, 1, 0, 1 );
    mpm = XCreatePixmapFromBitmapData( g_display, pm, mask,
     16, 16, 1, 0, 1 );
    crosshair = XCreatePixmapCursor( g_display, spm, mpm, &fg, &bg,
     source_x_hot, source_y_hot );
  }

  XDefineCursor( g_display, ptr->attr->plotwin, crosshair );

  return XGRAPH_SUCCESS;

}

int xgraph_downarrow_area_cursor (
  AREA_ID id,
  char *fg_name,
  char *bg_name
) {

AREA_PRIV_PTR ptr;
int stat;
static Cursor downarrow;
static init = 1;

#define down_x_hot 8
#define down_y_hot 15
Pixmap pm;
Pixmap spm;
static char source[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
   0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x20, 0x09,
   0x40, 0x05, 0x80, 0x02, 0x00, 0x01, 0x00, 0x00};
Pixmap mpm;
static char mask[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
   0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x20, 0x09,
   0x40, 0x05, 0x80, 0x02, 0x00, 0x01, 0x00, 0x00};
XColor fg, bg;
XColor Color;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( init ) {
    init = 0;
    stat = xgraph___alloc_color( bg_name, g_colormap, &bg );
    stat = xgraph___alloc_color( fg_name, g_colormap, &fg );
    pm = XCreatePixmap( g_display, XDefaultRootWindow(g_display), 1, 1, 1 );
    spm = XCreatePixmapFromBitmapData( g_display, pm, source,
     16, 16, 1, 0, 1 );
    mpm = XCreatePixmapFromBitmapData( g_display, pm, mask,
     16, 16, 1, 0, 1 );
    downarrow = XCreatePixmapCursor( g_display, spm, mpm, &fg, &bg,
     down_x_hot, down_y_hot );
  }

  XDefineCursor( g_display, ptr->attr->plotwin, downarrow );

  return XGRAPH_SUCCESS;

}

int xgraph_rightarrow_area_cursor (
  AREA_ID id,
  char *fg_name,
  char *bg_name
) {

AREA_PRIV_PTR ptr;
int stat;
static Cursor rightarrow;
static init = 1;

#define right_x_hot 15
#define right_y_hot 8
Pixmap pm;
Pixmap spm;
static char source[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x15, 0x10, 0x2a, 0x30, 0xff, 0x7f, 0x2a, 0x30, 0x15, 0x10, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
Pixmap mpm;
static char mask[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x15, 0x10, 0x2a, 0x30, 0xff, 0x7f, 0x2a, 0x30, 0x15, 0x10, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
XColor fg, bg;
XColor Color;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( init ) {
    init = 0;
    stat = xgraph___alloc_color( bg_name, g_colormap, &bg );
    stat = xgraph___alloc_color( fg_name, g_colormap, &fg );
    pm = XCreatePixmap( g_display, XDefaultRootWindow(g_display), 1, 1, 1 );
    spm = XCreatePixmapFromBitmapData( g_display, pm, source,
     16, 16, 1, 0, 1 );
    mpm = XCreatePixmapFromBitmapData( g_display, pm, mask,
     16, 16, 1, 0, 1 );
    rightarrow = XCreatePixmapCursor( g_display, spm, mpm, &fg, &bg,
     right_x_hot, right_y_hot );
  }

  XDefineCursor( g_display, ptr->attr->plotwin, rightarrow );

  return XGRAPH_SUCCESS;

}

int xgraph_pause_area_cursor (
  AREA_ID id,
  char *fg_name,
  char *bg_name
) {

AREA_PRIV_PTR ptr;
int stat;
static Cursor pause;
static init = 1;

#define pause_x_hot 8
#define pause_y_hot 8
Pixmap pm;
Pixmap spm;
static char source[] = {
   0xfc, 0x3f, 0x04, 0x20, 0x08, 0x10, 0xe8, 0x17, 0xd0, 0x0b, 0xa0, 0x05,
   0xa0, 0x04, 0x40, 0x03, 0xc0, 0x02, 0x20, 0x05, 0xa0, 0x04, 0x90, 0x09,
   0xc8, 0x13, 0xe8, 0x17, 0xf4, 0x2f, 0xfc, 0x3f};
Pixmap mpm;
static char mask[] = {
   0xfc, 0x3f, 0x04, 0x20, 0x08, 0x10, 0xe8, 0x17, 0xd0, 0x0b, 0xa0, 0x05,
   0xa0, 0x04, 0x40, 0x03, 0xc0, 0x02, 0x20, 0x05, 0xa0, 0x04, 0x90, 0x09,
   0xc8, 0x13, 0xe8, 0x17, 0xf4, 0x2f, 0xfc, 0x3f};
XColor fg, bg;
XColor Color;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( init ) {
    init = 0;
    stat = xgraph___alloc_color( bg_name, g_colormap, &bg );
    stat = xgraph___alloc_color( fg_name, g_colormap, &fg );
    pm = XCreatePixmap( g_display, XDefaultRootWindow(g_display), 1, 1, 1 );
    spm = XCreatePixmapFromBitmapData( g_display, pm, source,
     16, 16, 1, 0, 1 );
    mpm = XCreatePixmapFromBitmapData( g_display, pm, mask,
     16, 16, 1, 0, 1 );
    pause = XCreatePixmapCursor( g_display, spm, mpm, &fg, &bg,
     pause_x_hot, pause_y_hot );
  }

  XDefineCursor( g_display, ptr->attr->plotwin, pause );

  return XGRAPH_SUCCESS;

}

int xgraph_error_area_cursor (
  AREA_ID id,
  char *fg_name,
  char *bg_name
) {

AREA_PRIV_PTR ptr;
int stat;
static Cursor error;
static init = 1;

#define error_x_hot 8
#define error_y_hot 11
Pixmap pm;
Pixmap spm;
static char source[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x21, 0x8e, 0x23, 0x91, 0x25, 0x91, 0x25, 0x91, 0x29, 0x91,
   0x29, 0x91, 0x31, 0x11, 0x21, 0x8e, 0x00, 0x00};
Pixmap mpm;
static char mask[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x21, 0x8e, 0x23, 0x91, 0x25, 0x91, 0x25, 0x91, 0x29, 0x91,
   0x29, 0x91, 0x31, 0x11, 0x21, 0x8e, 0x00, 0x00};
XColor fg, bg;
XColor Color;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( init ) {
    init = 0;
    stat = xgraph___alloc_color( bg_name, g_colormap, &bg );
    stat = xgraph___alloc_color( fg_name, g_colormap, &fg );
    pm = XCreatePixmap( g_display, XDefaultRootWindow(g_display), 1, 1, 1 );
    spm = XCreatePixmapFromBitmapData( g_display, pm, source,
     16, 16, 1, 0, 1 );
    mpm = XCreatePixmapFromBitmapData( g_display, pm, mask,
     16, 16, 1, 0, 1 );
    error = XCreatePixmapCursor( g_display, spm, mpm, &fg, &bg,
     error_x_hot, error_y_hot );
  }

  XDefineCursor( g_display, ptr->attr->plotwin, error );

  return XGRAPH_SUCCESS;

}

int xgraph_clear_area (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  XClearWindow( ptr->attr->dsp, ptr->attr->win );

}

int xgraph_clear_plot_area (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  XClearWindow( ptr->attr->dsp, ptr->attr->plotwin );

}

int xgraph_draw_title (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, x0, y0, x1, y1, lastx;
int major_tick_height, minor_tick_height;
int scale_len;
double dx, inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );
  XSetFont( g_display, ptr->attr->gc, ptr->attr->title_fontstruct->fid );

  x0 = ptr->attr->plot_x - 1;
  x1 = x0 + scale_len;

  /* draw title */
  y0 = 0;
  y1 = y0 + ptr->attr->title_font_height + 3;

  x = ( x0 + x1 ) * 0.5;	/* mdipoint */
  delta = XTextWidth( ptr->attr->title_fontstruct, ptr->attr->title,
   strlen(ptr->attr->title) )
   / 2;
  x -= delta;

  XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y1,
  ptr->attr->title, strlen(ptr->attr->title) );

  return XGRAPH_SUCCESS;

}

int xgraph_title (
  AREA_ID id,
  char *title
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  strncpy( ptr->attr->title, title, TITLE_MAX_CHARS );
  ptr->attr->title[TITLE_MAX_CHARS] = 0;

  stat = xgraph_draw_title( id );

  return stat;

}

int xgraph_draw_x_linear_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, x0, y0, x1, y1, lastx;
int label_tick_height, major_tick_height, minor_tick_height;
int scale_len;
double dx, inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );

  /* draw axis */
  x0 = ptr->attr->plot_x;
/*  y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1; */
  y0 = ptr->attr->plot_y + ptr->attr->plot_h;
  x1 = x0 + scale_len - 1;
  y1 = y0;
  XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0, x1, y1 );

  lastx = x1;

  label_tick_height = 0.8 * abs( ptr->attr->label_font_height - 2 );
  major_tick_height = 0.8 * label_tick_height;
  minor_tick_height = major_tick_height * 0.5;

  /* draw label ticks */

  if ( ptr->attr->x_tick_label_inc > 0.0 ) {

    x0 = ptr->attr->plot_x;
    x1 = x0;
    /* y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1; */
    y0 = ptr->attr->plot_y + ptr->attr->plot_h;
    y1 = y0 + label_tick_height;

    dx = ptr->attr->xmin;
    inc = ptr->attr->x_tick_label_inc;

    while ( dx < ( ptr->attr->xmax + inc * 0.5 ) ) {

      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
       ptr->attr->plot_x;

    }

    dx = ptr->attr->xmax;
    x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
     ptr->attr->plot_x;
    XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
     x1, y1 );

  }

  /* draw major ticks */

  if ( ptr->attr->x_major_tick_inc > 0.0 ) {

    x0 = ptr->attr->plot_x;
    x1 = x0;
    /* y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1; */
    y0 = ptr->attr->plot_y + ptr->attr->plot_h;
    y1 = y0 + major_tick_height;

    dx = ptr->attr->xmin;
    inc = ptr->attr->x_major_tick_inc;

    while ( dx < ( ptr->attr->xmax + inc * 0.5 ) ) {

      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
       ptr->attr->plot_x;

    }

    dx = ptr->attr->xmax;
    x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
     ptr->attr->plot_x;
    XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
     x1, y1 );

  }

  /* draw minor ticks */
  if ( ptr->attr->x_minor_tick_inc > 0.0 ) {

    x0 = ptr->attr->plot_x;
    x1 = x0;
    /* y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1; */
    y0 = ptr->attr->plot_y + ptr->attr->plot_h;
    y1 = y0 + minor_tick_height;

    dx = ptr->attr->xmin;
    inc = ptr->attr->x_minor_tick_inc;

    while ( dx < ( ptr->attr->xmax + inc * 0.5 ) ) {

      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
       ptr->attr->plot_x;

    }

    dx = ptr->attr->xmax;
    x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
     ptr->attr->plot_x;
    XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
     x1, y1 );

  }

  return XGRAPH_SUCCESS;

}

int xgraph_draw_x_log_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, x0, y0, x1, y1, lastx;
int label_tick_height, major_tick_height, minor_tick_height;
int scale_len;
double dx, inc, n, logval;
double decade_tick_inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );

  /* draw axis */

  if ( ptr->attr->x_tick_label_inc == 0.1 )
    decade_tick_inc = 1.0;
  else
    decade_tick_inc = ptr->attr->x_tick_label_inc;

  x0 = ptr->attr->plot_x;
  /* y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1; */
  y0 = ptr->attr->plot_y + ptr->attr->plot_h;
  x1 = x0 + scale_len - 1;
  y1 = y0;
  XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0, x1, y1 );

  lastx = x1;

  label_tick_height = 0.8 * abs( ptr->attr->label_font_height - 2 );
  major_tick_height = 0.8 * label_tick_height;
  minor_tick_height = major_tick_height * 0.5;

  /* draw label ticks */

  if ( decade_tick_inc ) {

    x0 = ptr->attr->plot_x;
    x1 = x0;
    /* y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1; */
    y0 = ptr->attr->plot_y + ptr->attr->plot_h;
    y1 = y0 + label_tick_height;

    dx = ptr->attr->xmin;
    inc = decade_tick_inc;

    while ( dx < ( ptr->attr->xmax + inc * 0.5 ) ) {

      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
       ptr->attr->plot_x;

    }

    dx = ptr->attr->xmax;
    x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
     ptr->attr->plot_x;
    XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
     x1, y1 );

  }

  /* draw major ticks */

  if ( ptr->attr->x_major_tick_inc > 0.0 ) {

    x0 = ptr->attr->plot_x;
    x1 = x0;
    /* y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1; */
    y0 = ptr->attr->plot_y + ptr->attr->plot_h;
    y1 = y0 + major_tick_height;

    dx = ptr->attr->xmin;
    inc = ptr->attr->x_major_tick_inc;

    while ( dx < ( ptr->attr->xmax + inc * 0.5 ) ) {

      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
       ptr->attr->plot_x;

    }

    dx = ptr->attr->xmax;
    x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
     ptr->attr->plot_x;
    XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
     x1, y1 );

  }

  /* draw minor ticks (within each decade) */
  if ( ptr->attr->x_minor_tick_inc > 0.0 ) {

    for ( n=ptr->attr->xmin; n<ptr->attr->xmax; n+=1.0 ) {

      x0 = ptr->attr->plot_x;
      x1 = x0;
      /* y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1; */
      y0 = ptr->attr->plot_y + ptr->attr->plot_h;
      y1 = y0 + minor_tick_height;

      dx = 1.0;
      inc = ptr->attr->x_minor_tick_inc;

      while ( dx < 9.5 ) {

        XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
         x1, y1 );

        dx += inc;
        logval = log10(dx) + n;
        x0 = x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
         ptr->attr->plot_x;

      }

      dx = 10.0;
      logval = log10(dx) + n;
      x0 = x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
       ptr->attr->plot_x;
      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

    }

  }

  return XGRAPH_SUCCESS;

}

int xgraph_draw_x_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LINEAR )
    stat = xgraph_draw_x_linear_scale( id );
  else
    stat = xgraph_draw_x_log_scale( id );

  return stat;

}

int xgraph_x_scale (
  AREA_ID id,
  double major_tick_inc,
  double minor_tick_inc,
  double tick_label_inc	/* label every increment */
) {

AREA_PRIV_PTR ptr;
int stat;
double inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LOG ) {

    if ( ( tick_label_inc < 1.0 ) && ( tick_label_inc != 0.0 ) ) {
      tick_label_inc = 0.1;
    }
    else {
      inc = (double) ( (int) tick_label_inc );
      if ( tick_label_inc > inc )
        tick_label_inc = inc + 1.0;
      else
        tick_label_inc = inc;
    }

    if ( ( major_tick_inc < 1.0 ) && ( major_tick_inc != 0.0 ) ) {
      major_tick_inc = 1.0;
    }
    else {
      inc = (double) ( (int) major_tick_inc );
      if ( major_tick_inc > inc )
        major_tick_inc = inc + 1.0;
      else
        major_tick_inc = inc;
    }

    if ( ( minor_tick_inc < 1.0 ) && ( minor_tick_inc != 0.0 ) ) {
      minor_tick_inc = 1.0;
    }
    else {
      inc = (double) ( (int) minor_tick_inc );
      if ( minor_tick_inc > inc )
        minor_tick_inc = inc + 1.0;
      else
        minor_tick_inc = inc;
    }

  }

  ptr->attr->x_major_tick_inc = major_tick_inc;
  ptr->attr->x_minor_tick_inc = minor_tick_inc;
  ptr->attr->x_tick_label_inc = tick_label_inc;

  stat = xgraph_draw_x_scale( id );

  return stat;

}

int xgraph_draw_x_linear_annotation (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, x0, y0, x1, y1, lastx;
int label_tick_height;
int scale_len;
double dx, inc;
char value[127+1];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );
  XSetFont( g_display, ptr->attr->gc, ptr->attr->label_fontstruct->fid );

  x0 = ptr->attr->plot_x;
  x1 = x0 + scale_len - 1;
  lastx = x1;

  /* draw annotation */

  if ( ptr->attr->x_tick_label_inc > 0.0 ) {

    label_tick_height = 0.8 * abs( ptr->attr->label_font_height - 2 );
    x0 = ptr->attr->plot_x;
    x1 = x0;
    y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1;
    y1 = y0 + 1.2 * label_tick_height + ptr->attr->label_font_height;

    dx = ptr->attr->xmin;
    inc = ptr->attr->x_tick_label_inc;

    while ( dx < ( ptr->attr->xmax + inc * 0.5 ) ) {

      sprintf( value, ptr->attr->xformat, dx );
      xgraph___discard_trailing_blanks( value );
      delta = XTextWidth( ptr->attr->label_fontstruct, value, strlen(value) ) / 2.0;
      x = x0 - delta;

      XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y1,
       value, strlen(value) );

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
       ptr->attr->plot_x;

    }

    dx = ptr->attr->xmax;
    x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
     ptr->attr->plot_x;
    sprintf( value, ptr->attr->xformat, dx );
    xgraph___discard_trailing_blanks( value );
    delta = XTextWidth( ptr->attr->label_fontstruct, value, strlen(value) ) / 2.0;
    x = x0 - delta;
    XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y1,
     value, strlen(value) );

  }

  return XGRAPH_SUCCESS;

}

int xgraph_draw_x_log_annotation (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, n, nn, delta, x, x0, y0, x1, y1, lastx;
int label_tick_height;
int scale_len;
double dx, inc, logval;
char value[127+1];
double decade_tick_inc, inner_tick_inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );
  XSetFont( g_display, ptr->attr->gc, ptr->attr->label_fontstruct->fid );

  x0 = ptr->attr->plot_x;
  x1 = x0 + scale_len - 1;
  lastx = x1;

  /* draw annotation */

  if ( ptr->attr->x_tick_label_inc == 0.1 ) {
    inner_tick_inc = 1.0;
    decade_tick_inc = 1.0;
  }
  else {
    inner_tick_inc = 0.0;
    decade_tick_inc = ptr->attr->x_tick_label_inc;
  }

  if ( decade_tick_inc > 0.0 ) {

    label_tick_height = 0.8 * abs( ptr->attr->label_font_height - 2 );
    x0 = ptr->attr->plot_x;
    x1 = x0;
    y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1;
    y1 = y0 + 1.2 * label_tick_height + ptr->attr->label_font_height;

    dx = ptr->attr->xmin;
    inc = decade_tick_inc;

    while ( dx < ( ptr->attr->xmax + inc * 0.5 ) ) {

      if ( dx < 0.0 )
        logval = pow( 10.0, dx );
      else
        logval = (double) ( (int) ( pow( 10.0, dx ) + 0.5 ) );
      sprintf( value, ptr->attr->xformat, logval );
      xgraph___discard_trailing_blanks( value );
      delta = XTextWidth( ptr->attr->label_fontstruct, value, strlen(value) ) / 2.0;
      x = x0 - delta;

      XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y1,
       value, strlen(value) );

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
       ptr->attr->plot_x;

    }

    dx = ptr->attr->xmax;
    x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
     ptr->attr->plot_x;
    if ( dx < 0.0 )
      logval = pow( 10.0, dx );
    else
      logval = (double) ( (int) ( pow( 10.0, dx ) + 0.5 ) );
    sprintf( value, ptr->attr->xformat, logval );
    xgraph___discard_trailing_blanks( value );
    delta = XTextWidth( ptr->attr->label_fontstruct, value, strlen(value) ) / 2.0;
    x = x0 - delta;
    XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y1,
     value, strlen(value) );

  }

  /* label minor ticks (within each decade) */
  if ( ( inner_tick_inc == 1.0 ) && ( ptr->attr->x_minor_tick_inc > 0.0 ) ) {

    label_tick_height = 0.8 * abs( ptr->attr->label_font_height - 2 );

    for ( n=ptr->attr->xmin; n<ptr->attr->xmax; n+=1.0 ) {

      x0 = ptr->attr->plot_x;
      x1 = x0;
      y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1;
      y1 = y0 + 1.2 * label_tick_height + ptr->attr->label_font_height;

      if ( n < 0.0 )
        dx = pow( 10.0, n );
      else
        dx = (double) ( (int) ( pow( 10.0, n ) + 0.5 ) );

      inc = dx;

      for ( nn=0; nn<9; nn++ ) {

        dx += inc;
        logval = log10( dx );
        x0 = x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
         ptr->attr->plot_x;

        sprintf( value, ptr->attr->xformat, dx );
        xgraph___discard_trailing_blanks( value );
        delta = XTextWidth( ptr->attr->label_fontstruct, value, strlen(value) ) / 2.0;
        x = x0 - delta;

        XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y1,
         value, strlen(value) );

      }

    }

  }

  return XGRAPH_SUCCESS;

}

int xgraph_draw_x_annotation (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LINEAR )
    stat = xgraph_draw_x_linear_annotation( id );
  else
    stat = xgraph_draw_x_log_annotation( id );

  return stat;

}

int xgraph_x_annotation (
  AREA_ID id,
  char *format
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  strncpy( ptr->attr->xformat, format, FORMAT_MAX_CHARS );
  ptr->attr->xformat[FORMAT_MAX_CHARS] = 0;

  stat = xgraph_draw_x_annotation( id );

  return stat;

}

int xgraph_draw_x_label (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, x0, y0, x1, y1, lastx;
int scale_len;
double dx, inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );
  XSetFont( g_display, ptr->attr->gc, ptr->attr->label_fontstruct->fid );

  x0 = ptr->attr->plot_x - 1;
  x1 = x0 + scale_len;

  /* draw label */
  y0 = ptr->attr->h;
  y1 = y0 - 3;

  x = ( x0 + x1 ) * 0.5;	/* mdipoint */
  delta = XTextWidth( ptr->attr->label_fontstruct, ptr->attr->xlabel,
   strlen(ptr->attr->xlabel) )
   / 2;
  x -= delta;

  XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y1,
  ptr->attr->xlabel, strlen(ptr->attr->xlabel) );

  return XGRAPH_SUCCESS;

}

int xgraph_x_label (
  AREA_ID id,
  char *label
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  strncpy( ptr->attr->xlabel, label, LABEL_MAX_CHARS );
  ptr->attr->xlabel[LABEL_MAX_CHARS] = 0;

  stat = xgraph_draw_x_label( id );

  return stat;

}

int xgraph_draw_y_linear_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, x0, y0, x1, y1, lasty;
int label_tick_width, major_tick_width, minor_tick_width;
int scale_len;
double dy, inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );

  /* draw axis */
  x0 = ptr->attr->plot_x - 1;
  /* y0 = ptr->attr->plot_y + ptr->attr->plot_h; */
  y0 = ptr->attr->plot_y + ptr->attr->plot_h - 1;
  x1 = x0;
  y1 = y0 - scale_len + 1;
  XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0, x1, y1 );

  lasty = y1;

  label_tick_width = 0.8 * abs( ptr->attr->label_font_height - 2 );
  major_tick_width = 0.8 * label_tick_width;
  minor_tick_width = major_tick_width * 0.5;

  /* draw label ticks */

  if ( ptr->attr->y_tick_label_inc > 0.0 ) {

    x0 = ptr->attr->plot_x - 1;
    /* y0 = ptr->attr->plot_y + ptr->attr->plot_h; */
    y0 = ptr->attr->plot_y + ptr->attr->plot_h - 1;
    x1 = x0 - label_tick_width;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = ptr->attr->y_tick_label_inc;

    while ( dy < ( ptr->attr->ymax + inc * 0.5 ) ) {

      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;

    }

    dy = ptr->attr->ymax;
    y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
    y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;
    XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
     x1, y1 );

  }

  /* draw major ticks */

  if ( ptr->attr->y_major_tick_inc > 0.0 ) {

    x0 = ptr->attr->plot_x - 1;
    /* y0 = ptr->attr->plot_y + ptr->attr->plot_h; */
    y0 = ptr->attr->plot_y + ptr->attr->plot_h - 1;
    x1 = x0 - major_tick_width;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = ptr->attr->y_major_tick_inc;

    while ( dy < ( ptr->attr->ymax + inc * 0.5 ) ) {

      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;

    }

    dy = ptr->attr->ymax;
    y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
    y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;
    XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
     x1, y1 );

  }

  /* draw minor ticks */
  if ( ptr->attr->y_minor_tick_inc > 0.0 ) {

    x0 = ptr->attr->plot_x - 1;
    /* y0 = ptr->attr->plot_y + ptr->attr->plot_h; */
    y0 = ptr->attr->plot_y + ptr->attr->plot_h - 1;
    x1 = x0 - minor_tick_width;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = ptr->attr->y_minor_tick_inc;

    while ( dy < ( ptr->attr->ymax + inc * 0.5 ) ) {

      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;

    }

    dy = ptr->attr->ymax;
    y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
    y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;
    XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
     x1, y1 );

  }

  return XGRAPH_SUCCESS;

}

int xgraph_draw_y_log_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, x0, y0, x1, y1, lasty;
int label_tick_width, major_tick_width, minor_tick_width;
int scale_len;
double n, dy, inc, logval;
double decade_tick_inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );

  /* draw axis */

  if ( ptr->attr->x_tick_label_inc == 0.1 )
    decade_tick_inc = 1.0;
  else
    decade_tick_inc = ptr->attr->x_tick_label_inc;

  x0 = ptr->attr->plot_x - 1;
  /* y0 = ptr->attr->plot_y + ptr->attr->plot_h; */
  y0 = ptr->attr->plot_y + ptr->attr->plot_h - 1;
  x1 = x0;
  y1 = y0 - scale_len + 1;
  XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0, x1, y1 );

  lasty = y1;

  label_tick_width = 0.8 * abs( ptr->attr->label_font_height - 2 );
  major_tick_width = 0.8 * label_tick_width;
  minor_tick_width = major_tick_width * 0.5;

  /* draw label ticks */

  if ( decade_tick_inc > 0.0 ) {

    x0 = ptr->attr->plot_x - 1;
    /* y0 = ptr->attr->plot_y + ptr->attr->plot_h; */
    y0 = ptr->attr->plot_y + ptr->attr->plot_h - 1;
    x1 = x0 - label_tick_width;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = decade_tick_inc;

    while ( dy < ( ptr->attr->ymax + inc * 0.5 ) ) {

      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;

    }

    dy = ptr->attr->ymax;
    y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
    y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;
    XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
     x1, y1 );

  }

  /* draw major ticks */

  if ( ptr->attr->y_major_tick_inc > 0.0 ) {

    x0 = ptr->attr->plot_x - 1;
    /* y0 = ptr->attr->plot_y + ptr->attr->plot_h; */
    y0 = ptr->attr->plot_y + ptr->attr->plot_h - 1;
    x1 = x0 - major_tick_width;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = ptr->attr->y_major_tick_inc;

    while ( dy < ( ptr->attr->ymax + inc * 0.5 ) ) {

      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;

    }

    dy = ptr->attr->ymax;
    y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
    y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;
    XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
     x1, y1 );

  }

  /* draw minor ticks (within each decade) */
  if ( ptr->attr->y_minor_tick_inc > 0.0 ) {

    for ( n=ptr->attr->ymin; n<ptr->attr->ymax; n+=1.0 ) {

      x0 = ptr->attr->plot_x - 1;
      /* y0 = ptr->attr->plot_y + ptr->attr->plot_h; */
      y0 = ptr->attr->plot_y + ptr->attr->plot_h - 1;
      x1 = x0 - minor_tick_width;
      y1 = y0;

      dy = 1.0;
      inc = ptr->attr->y_minor_tick_inc;

      while ( dy < 9.5 ) {

        XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
         x1, y1 );

        dy += inc;
        logval = log10(dy) + n;
        y0 = y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
        y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;

      }

      dy = 10.0;
      logval = log10(dy) + n;
      y0 = y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;
      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );

    }

  }

  return XGRAPH_SUCCESS;

}

int xgraph_draw_y_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LINEAR )
    stat = xgraph_draw_y_linear_scale( id );
  else
    stat = xgraph_draw_y_log_scale( id );

  return stat;

}

int xgraph_y_scale (
  AREA_ID id,
  double major_tick_inc,
  double minor_tick_inc,
  double tick_label_inc /* label every increment */
) {

AREA_PRIV_PTR ptr;
int stat;
double inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LOG ) {

    if ( ( tick_label_inc < 1.0 ) && ( tick_label_inc != 0.0 ) ) {
      tick_label_inc = 0.1;
    }
    else {
      inc = (double) ( (int) tick_label_inc );
      if ( tick_label_inc > inc )
        tick_label_inc = inc + 1.0;
      else
        tick_label_inc = inc;
    }

    if ( ( major_tick_inc < 1.0 ) && ( major_tick_inc != 0.0 ) ) {
      major_tick_inc = 1.0;
    }
    else {
      inc = (double) ( (int) major_tick_inc );
      if ( major_tick_inc > inc )
        major_tick_inc = inc + 1.0;
      else
        major_tick_inc = inc;
    }

    if ( ( minor_tick_inc < 1.0 ) && ( minor_tick_inc != 0.0 ) ) {
      minor_tick_inc = 1.0;
    }
    else {
      inc = (double) ( (int) minor_tick_inc );
      if ( minor_tick_inc > inc )
        minor_tick_inc = inc + 1.0;
      else
        minor_tick_inc = inc;
    }

  }

  ptr->attr->y_major_tick_inc = major_tick_inc;
  ptr->attr->y_minor_tick_inc = minor_tick_inc;
  ptr->attr->y_tick_label_inc = tick_label_inc;

  stat = xgraph_draw_y_scale( id );

  return stat;

}

int xgraph_draw_y_linear_annotation (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, y, x0, y0, x1, y1, lasty;
int label_tick_width;
int scale_len;
double dy, inc;
char value[31+1];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );
  XSetFont( g_display, ptr->attr->gc, ptr->attr->label_fontstruct->fid );

  y0 = ptr->attr->plot_y + ptr->attr->plot_h;
  y1 = y0 - scale_len + 1;
  lasty = y1;

  /* draw annotation */

  if ( ptr->attr->y_tick_label_inc > 0.0 ) {

    label_tick_width = 0.8 * abs( ptr->attr->label_font_height - 2 );
    x0 = ptr->attr->plot_x - 1;
    y0 = ptr->attr->plot_y + ptr->attr->plot_h;
    x1 = x0 - 1.2 * label_tick_width;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = ptr->attr->y_tick_label_inc;

    while ( dy < ( ptr->attr->ymax + inc * 0.5 ) ) {

      sprintf( value, ptr->attr->yformat, dy );
      xgraph___discard_trailing_blanks( value );
      delta = ptr->attr->label_font_height / 2;
      y = y0 + delta;
      delta = XTextWidth( ptr->attr->label_fontstruct, value, strlen(value) );
      x = x1 - delta;

      XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y,
       value, strlen(value) );

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;

    }

    dy = ptr->attr->ymax;
    y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
    y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;
    sprintf( value, ptr->attr->yformat, dy );
    xgraph___discard_trailing_blanks( value );
    delta = ptr->attr->label_font_height / 2;
    y = y0 + delta;
    delta = XTextWidth( ptr->attr->label_fontstruct, value, strlen(value) );
    x = x1 - delta;
    XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y,
     value, strlen(value) );

  }

  return XGRAPH_SUCCESS;

}

int xgraph_draw_y_log_annotation (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, n, nn, delta, x, y, x0, y0, x1, y1, lasty;
int label_tick_width;
int scale_len;
double dy, inc, logval;
char value[31+1];
double decade_tick_inc, inner_tick_inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );
  XSetFont( g_display, ptr->attr->gc, ptr->attr->label_fontstruct->fid );

  y0 = ptr->attr->plot_y + ptr->attr->plot_h;
  y1 = y0 - scale_len + 1;
  lasty = y1;

  /* draw annotation */

  if ( ptr->attr->y_tick_label_inc == 0.1 ) {
    inner_tick_inc = 1.0;
    decade_tick_inc = 1.0;
  }
  else {
    inner_tick_inc = 0.0;
    decade_tick_inc = ptr->attr->y_tick_label_inc;
  }

  if ( decade_tick_inc > 0.0 ) {

    label_tick_width = 0.8 * abs( ptr->attr->label_font_height - 2 );
    x0 = ptr->attr->plot_x - 1;
    y0 = ptr->attr->plot_y + ptr->attr->plot_h;
    x1 = x0 - 1.2 * label_tick_width;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = decade_tick_inc;

    while ( dy < ( ptr->attr->ymax + inc * 0.5 ) ) {

      if ( dy < 0.0 )
        logval = pow( 10.0, dy );
      else
        logval = (double) ( (int) ( pow( 10.0, dy ) + 0.5 ) );
      sprintf( value, ptr->attr->yformat, logval );
      xgraph___discard_trailing_blanks( value );
      delta = ptr->attr->label_font_height / 2;
      y = y0 + delta;
      delta = XTextWidth( ptr->attr->label_fontstruct, value, strlen(value) );
      x = x1 - delta;

      XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y,
       value, strlen(value) );

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;

    }

    dy = ptr->attr->ymax;
    y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
    y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;
    if ( dy < 0.0 )
      logval = pow( 10.0, dy );
    else
      logval = (double) ( (int) ( pow( 10.0, dy ) + 0.5 ) );
    sprintf( value, ptr->attr->yformat, logval );
    xgraph___discard_trailing_blanks( value );
    delta = ptr->attr->label_font_height / 2;
    y = y0 + delta;
    delta = XTextWidth( ptr->attr->label_fontstruct, value, strlen(value) );
    x = x1 - delta;
    XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y,
     value, strlen(value) );

  }

  /* label minor ticks (within each decade) */
  if ( ( inner_tick_inc == 1.0 ) && ( ptr->attr->y_minor_tick_inc > 0.0 ) ) {

    label_tick_width = 0.8 * abs( ptr->attr->label_font_height - 2 );
    for ( n=ptr->attr->ymin; n<ptr->attr->ymax; n+=1.0 ) {

      x0 = ptr->attr->plot_x - 1;
      y0 = ptr->attr->plot_y + ptr->attr->plot_h;
      x1 = x0 - 1.2 * label_tick_width;
      y1 = y0;

      if ( n < 0.0 )
        dy = pow( 10.0, n );
      else
        dy = (double) ( (int) ( pow( 10.0, n ) + 0.5 ) );

      inc = dy;

      for ( nn=0; nn<9; nn++ ) {

        dy += inc;
        logval = log10( dy );
        y0 = y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
        y0 = y1 = ptr->attr->plot_h - 1 - y0 + ptr->attr->plot_y;

        sprintf( value, ptr->attr->yformat, dy );
        xgraph___discard_trailing_blanks( value );
        delta = ptr->attr->label_font_height / 2;
        y = y0 + delta;
        delta = XTextWidth( ptr->attr->label_fontstruct, value, strlen(value) );
        x = x1 - delta;

        XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x, y,
         value, strlen(value) );

      }

    }

  }

  return XGRAPH_SUCCESS;

}

int xgraph_draw_y_annotation (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LINEAR )
    stat = xgraph_draw_y_linear_annotation( id );
  else
    stat = xgraph_draw_y_log_annotation( id );

  return stat;

}

int xgraph_y_annotation (
  AREA_ID id,
  char *format
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  strncpy( ptr->attr->yformat, format, FORMAT_MAX_CHARS );
  ptr->attr->yformat[FORMAT_MAX_CHARS] = 0;


  stat = xgraph_draw_y_annotation( id );

  return stat;

}

int xgraph_draw_vert_y_label (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, char_width, max_char_width, delta, x, y, x0, y0, x1, y1, lastx;
int scale_len;
double dx, inc;
char chr[2];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );
  XSetFont( g_display, ptr->attr->gc, ptr->attr->label_fontstruct->fid );

  y0 = ptr->attr->plot_y - 1;
  y1 = y0 + scale_len;

  /* draw label */
  x0 = 0;
  x1 = x0 + 3;

  y = ( y0 + y1 ) * 0.5;	/* mdipoint */
  delta = ( ptr->attr->label_font_height * strlen(ptr->attr->ylabel) ) * 0.5;
  y -= delta;
  if ( y < y0 ) y = y0;

  chr[0] = ptr->attr->ylabel[0];
  chr[1] = 0;
  max_char_width = XTextWidth( ptr->attr->label_fontstruct, chr, 1 );

  for ( i=1; i<strlen(ptr->attr->ylabel); i++ ) {

    chr[0] = ptr->attr->ylabel[i];
    chr[1] = 0;
    char_width = XTextWidth( ptr->attr->label_fontstruct, chr, 1 );
    if ( char_width > max_char_width ) max_char_width = char_width;

  }

  for ( i=0; i<strlen(ptr->attr->ylabel); i++ ) {

    chr[0] = ptr->attr->ylabel[i];
    chr[1] = 0;
    char_width = XTextWidth( ptr->attr->label_fontstruct, chr, 1 );
    delta = ( max_char_width - char_width ) / 2;

    XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x1+delta, y,
    chr, 1 );
    y += ptr->attr->label_font_height;

  }

  return XGRAPH_SUCCESS;

}

int xgraph_draw_horz_y_label (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, char_width, max_char_width, delta, x, y, x0, y0, x1, y1, lastx;
int scale_len;
double dx, inc;
char chr[2];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->fg_color.pixel );
  XSetFont( g_display, ptr->attr->gc, ptr->attr->label_fontstruct->fid );

  /* draw label */

  y0 = ptr->attr->plot_y - 1;
  delta = 0.5 * ptr->attr->label_font_height;
  y0 -= delta;

  x0 = ptr->attr->plot_x - 1;
  delta = XTextWidth( ptr->attr->label_fontstruct, ptr->attr->ylabel,
   strlen(ptr->attr->ylabel) );
  x1 = x0 - delta;
  if ( x1 < 5 ) x1 = 5;

  XDrawString( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x1, y0,
   ptr->attr->ylabel, strlen(ptr->attr->ylabel) );

  return XGRAPH_SUCCESS;

}

int xgraph_draw_y_label (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( ptr->attr->ylabel_orientation == GRAPH_K_VERTICAL )
    stat = xgraph_draw_vert_y_label( id );
  else
    stat = xgraph_draw_horz_y_label( id );

  return stat;

}

int xgraph_y_label (
  AREA_ID id,
  char *label
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, y, x0, y0, x1, y1, lastx;
int scale_len;
double dx, inc;
char chr[2];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  ptr->attr->ylabel_orientation = GRAPH_K_VERTICAL;
  strncpy( ptr->attr->ylabel, label, LABEL_MAX_CHARS );
  ptr->attr->ylabel[LABEL_MAX_CHARS] = 0;

  stat = xgraph_draw_y_label( id );

  return stat;

}

int xgraph_horz_y_label (
  AREA_ID id,
  char *label
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, y, x0, y0, x1, y1, lastx;
int scale_len;
double dx, inc;
char chr[2];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  ptr->attr->ylabel_orientation = GRAPH_K_HORIZONTAL;
  strncpy( ptr->attr->ylabel, label, LABEL_MAX_CHARS );
  ptr->attr->ylabel[LABEL_MAX_CHARS] = 0;

  stat = xgraph_draw_y_label( id );

  return stat;

}

int xgraph_create_plot (
  AREA_ID area_id,
  int type,
  char *color_name,
  int x_scale_type,
  int y_scale_type,
  double xmin,
  double xmax,
  double ymin,
  double ymax,
  PLOT_ID *id
) {

AREA_PRIV_PTR area_ptr;
PLOT_PRIV_ATTR_PTR attr_ptr;
PLOT_PRIV_PTR ptr;
OBJ_LIST_PTR cur_obj;
int stat, ret_stat;
WIN_LIST_PTR cur_win;
XGCValues values;
float ref;

  if ( x_scale_type == XGRAPH_SCALE_LOG ) {
    stat = xgraph___log_adjust_low_limit( &xmin );
    stat = xgraph___log_adjust_high_limit( &xmax );
  }

  if ( y_scale_type == XGRAPH_SCALE_LOG ) {
    stat = xgraph___log_adjust_low_limit( &ymin );
    stat = xgraph___log_adjust_high_limit( &ymax );
  }

  ptr = (PLOT_PRIV_PTR) calloc( 1, sizeof(PLOT_PRIV_TYPE) );
  if ( !ptr ) return XGRAPH_NO_MEM;

  area_ptr = (AREA_PRIV_PTR) area_id;

  if ( !area_ptr ) {
    ret_stat = XGRAPH_BAD_OBJ;
    goto err_return;
  }
  if ( !area_ptr->id_type ) {
    ret_stat = XGRAPH_BAD_OBJ;
    goto err_return;
  }

  attr_ptr = (PLOT_PRIV_ATTR_PTR) calloc( 1, sizeof(PLOT_PRIV_ATTR_TYPE) );
  if ( !attr_ptr ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }

  attr_ptr->area_ptr = area_ptr;
  attr_ptr->dsp = area_ptr->attr->dsp;
  attr_ptr->win = area_ptr->attr->plotwin;
  attr_ptr->type = type;
  attr_ptr->x_scale_type = x_scale_type;
  attr_ptr->y_scale_type = y_scale_type;

  attr_ptr->gc = DefaultGC( g_display, g_screen_num );

  attr_ptr->xorgc = XCreateGC( g_display, RootWindow(g_display,g_screen_num),
   0, &values );

  XCopyGC( g_display, attr_ptr->gc, 0xffffffff, attr_ptr->xorgc );

  values.function = GXxor;
  XChangeGC( g_display, attr_ptr->xorgc, GCFunction, &values );

  stat = xgraph___alloc_color( color_name, g_colormap, &attr_ptr->color );
  if ( stat != XGRAPH_SUCCESS ) attr_ptr->color.pixel = g_foreground;

  attr_ptr->line_type = 1;
  attr_ptr->plot_symbol = XGRAPH_NOSYMBOL;
  attr_ptr->symbol_fill = 0;
  attr_ptr->symbol_width = 1.0;
  attr_ptr->symbol_half_width = 0.5 * attr_ptr->symbol_width;
  attr_ptr->symbol_height = 1.0;
  attr_ptr->symbol_half_height = 0.5 * attr_ptr->symbol_height;
  ref = area_ptr->attr->plot_w;
  if ( ref > area_ptr->attr->plot_h ) ref = area_ptr->attr->plot_h;
  attr_ptr->i_symbol_width =
   (int) ( attr_ptr->symbol_width / 100.0 * ref );
  if ( attr_ptr->i_symbol_width % 2 ) attr_ptr->i_symbol_width++;
  attr_ptr->i_symbol_half_width =
   (int) ( attr_ptr->symbol_half_width / 100.0 * ref );
  attr_ptr->i_symbol_height =
   (int) ( attr_ptr->symbol_height / 100.0 * ref );
  if ( attr_ptr->i_symbol_height % 2 ) attr_ptr->i_symbol_height++;
  attr_ptr->i_symbol_half_height =
   (int) ( attr_ptr->symbol_half_height / 100.0 * ref );
  attr_ptr->xmin = xmin;
  attr_ptr->xmax = xmax;
  attr_ptr->ymin = ymin;
  attr_ptr->ymax = ymax;
  attr_ptr->xfact = (double) ( area_ptr->attr->plot_w - 1 ) / ( xmax - xmin );
  attr_ptr->xofs = (double) ( attr_ptr->xfact * xmin * -1.0 );
  attr_ptr->yfact = (double) ( area_ptr->attr->plot_h - 1 ) / ( ymax - ymin );
  attr_ptr->yofs = (double) ( attr_ptr->yfact * ymin * -1.0 );

  ptr->id_type = GRAPH_K_PLOT_ID;
  ptr->attr = attr_ptr;
  ptr->num_points = 0;

  /* create sentinel node */
  ptr->head = (PLOT_DATA_PTR) calloc( 1, sizeof(PLOT_DATA_TYPE) );
  ptr->tail = ptr->head;
  ptr->tail->blink = NULL;
  ptr->tail->flink = NULL;
  ptr->cur = NULL;

  ptr->lookaside_head = (PLOT_DATA_PTR) calloc( 1, sizeof(PLOT_DATA_TYPE) );
  ptr->lookaside_tail = ptr->lookaside_head;
  ptr->lookaside_tail->blink = NULL;
  ptr->lookaside_tail->flink = NULL;

  /* link plot ptr into area list */
  cur_obj = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
  if ( !cur_obj ) {
    ret_stat = XGRAPH_NO_MEM;
    goto err_return;
  }

  cur_obj->ptr = (void *) ptr;
  area_ptr->obj_tail->next = cur_obj;
  area_ptr->obj_tail = cur_obj;
  area_ptr->obj_tail->next = NULL;

  *id = (PLOT_ID) ptr;

norm_return:
  return XGRAPH_SUCCESS;

err_return:
  ptr->id_type = 0;
  *id = (PLOT_ID) ptr;
  return ret_stat;

}  

static int xgraph___draw_symbol (
  PLOT_PRIV_PTR ptr,
  int x,
  int y
) {

XPoint points[5];

  if ( ptr->attr->plot_symbol == XGRAPH_NOSYMBOL ) return XGRAPH_SUCCESS;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc,
   ptr->attr->symbol_color.pixel );

  switch ( ptr->attr->plot_symbol ) {

    case XGRAPH_CIRCLE:
      if ( ptr->attr->symbol_fill ) {
        XFillArc( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc,
         x-ptr->attr->i_symbol_half_width, y-ptr->attr->i_symbol_half_height,
         ptr->attr->i_symbol_width, ptr->attr->i_symbol_height, 0, 23040 );
      }
      else {
        XDrawArc( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc,
         x-ptr->attr->i_symbol_half_width, y-ptr->attr->i_symbol_half_height,
         ptr->attr->i_symbol_width, ptr->attr->i_symbol_height, 0, 23040 );
      }
      break;

    case XGRAPH_SQUARE:
      if ( ptr->attr->symbol_fill ) {
        XFillRectangle( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc,
         x-ptr->attr->i_symbol_half_width, y-ptr->attr->i_symbol_half_height,
         ptr->attr->i_symbol_width, ptr->attr->i_symbol_height );
      }
      else {
        XDrawRectangle( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc,
         x-ptr->attr->i_symbol_half_width, y-ptr->attr->i_symbol_half_height,
         ptr->attr->i_symbol_width, ptr->attr->i_symbol_height );
      }
      break;

    case XGRAPH_DIAMOND:
      points[0].x = x;
      points[0].y = y - ptr->attr->i_symbol_half_height;
      points[1].x = x + ptr->attr->i_symbol_half_width;
      points[1].y = y;
      points[2].x = points[0].x;
      points[2].y = y + ptr->attr->i_symbol_half_height;
      points[3].x = x - ptr->attr->i_symbol_half_width;
      points[3].y = y;
      if ( ptr->attr->symbol_fill ) {
        XFillPolygon( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc,
         points, 4, Convex, CoordModeOrigin );
      }
      else {
        points[4].x = points[0].x;
        points[4].y = points[0].y;
        XDrawLines( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc,
         points, 5, CoordModeOrigin );
      }
      break;

    case XGRAPH_TRIANGLE:
      points[0].x = x;
      points[0].y = y - ptr->attr->i_symbol_half_height;
      points[1].x = x + ptr->attr->i_symbol_half_width;
      points[1].y = y + ptr->attr->i_symbol_half_height;
      points[2].x = x - ptr->attr->i_symbol_half_width;
      points[2].y = points[1].y;
      if ( ptr->attr->symbol_fill ) {
        XFillPolygon( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc,
         points, 3, Convex, CoordModeOrigin );
      }
      else {
        points[3].x = points[0].x;
        points[3].y = points[0].y;
        XDrawLines( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc,
         points, 4, CoordModeOrigin );
      }
      break;

  }

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->color.pixel );

  return XGRAPH_SUCCESS;

}

int xgraph_draw_plot (
  PLOT_ID id
) {

PLOT_PRIV_PTR ptr;
AREA_PRIV_PTR area_ptr;
int i, x0, y0, x1, y1;
PLOT_DATA_PTR cur_point;
double logval;

  ptr = (PLOT_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  area_ptr = (AREA_PRIV_PTR) ptr->attr->area_ptr;

  cur_point = ptr->head->flink;
  if ( !cur_point ) return XGRAPH_SUCCESS;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->color.pixel );

  /* first point */
  if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LINEAR ) {
    x1 = (int) ( cur_point->x * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
  }
  else {
    if ( cur_point->x > 0.0 ) {
      logval = log10( cur_point->x );
      x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
    }
    else {
      return XGRAPH_BAD_LOG_VALUE;
    }
  }

  if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LINEAR ) {
    y1 = (int) ( cur_point->y * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
    y1 = area_ptr->attr->plot_h - 1 - y1;
  }
  else {
    if ( cur_point->y > 0.0 ) {
      logval = log10( cur_point->y );
      y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      y1 = area_ptr->attr->plot_h - 1 - y1;
    }
    else {
      return XGRAPH_BAD_LOG_VALUE;
    }
  }

  /* process plot type */
  switch ( ptr->attr->type ) {

    case XGRAPH_POINT:
      if ( ptr->attr->plot_symbol == XGRAPH_NOSYMBOL ) {
        XDrawPoint( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x1, y1 );
      }
      else {
        xgraph___draw_symbol( ptr, x1, y1 );
      }
      break;

    case XGRAPH_NEEDLE:
      x0 = x1;
      y0 = area_ptr->attr->plot_h - 1;
      XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
       x1, y1 );
      xgraph___draw_symbol( ptr, x1, y1 );
      break;

  }

  /* remaining points */

  cur_point = cur_point->flink;

  while( cur_point ) {

    x0 = x1;
    y0 = y1;

    if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LINEAR ) {
      x1 = (int) ( cur_point->x * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
    }
    else {
      if ( cur_point->x > 0.0 ) {
        logval = log10( cur_point->x );
        x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
      }
      else {
        return XGRAPH_BAD_LOG_VALUE;
      }
    }

    if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LINEAR ) {
      y1 = (int) ( cur_point->y * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      y1 = area_ptr->attr->plot_h - 1 - y1;
    }
    else {
      if ( cur_point->y > 0.0 ) {
        logval = log10( cur_point->y );
        y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
        y1 = area_ptr->attr->plot_h - 1 - y1;
      }
      else {
        return XGRAPH_BAD_LOG_VALUE;
      }
    }

    /* process plot type */
    switch ( ptr->attr->type ) {

      case XGRAPH_POINT:
        if ( ptr->attr->plot_symbol == XGRAPH_NOSYMBOL ) {
          XDrawPoint( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x1, y1 );
        }
        else {
          xgraph___draw_symbol( ptr, x1, y1 );
        }
        break;

      case XGRAPH_LINE:
        XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
         x1, y1 );
        xgraph___draw_symbol( ptr, x0, y0 );
        xgraph___draw_symbol( ptr, x1, y1 );
        break;

      case XGRAPH_NEEDLE:
        x0 = x1;
        y0 = area_ptr->attr->plot_h - 1;
        XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
         x1, y1 );
        xgraph___draw_symbol( ptr, x1, y1 );
        break;

    }

    cur_point = cur_point->flink;

  }

  return XGRAPH_SUCCESS;

}

int xgraph_set_symbol_attr (
  PLOT_ID id,
  int symbol_type,
  char *symbol_color_name,
  int symbol_fill,
  double width,
  double height
) {

PLOT_PRIV_PTR ptr;
AREA_PRIV_PTR area_ptr;
float ref;
int stat;

  ptr = (PLOT_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  area_ptr = (AREA_PRIV_PTR) ptr->attr->area_ptr;

  ptr->attr->plot_symbol = symbol_type;
  if ( symbol_type == XGRAPH_NOSYMBOL ) return XGRAPH_SUCCESS;

  ptr->attr->symbol_fill = symbol_fill;

  stat = xgraph___alloc_color( symbol_color_name, g_colormap,
   &ptr->attr->symbol_color );
  if ( stat != XGRAPH_SUCCESS ) ptr->attr->symbol_color.pixel = g_foreground;

  ptr->attr->symbol_width = width;
  if ( ptr->attr->i_symbol_width % 2 ) ptr->attr->i_symbol_width++;
  ptr->attr->symbol_half_width = 0.5 * ptr->attr->symbol_width;
  ptr->attr->symbol_height = height;
  if ( ptr->attr->i_symbol_height % 2 ) ptr->attr->i_symbol_height++;
  ptr->attr->symbol_half_height = 0.5 * ptr->attr->symbol_height;

  ref = area_ptr->attr->plot_w;
  if ( ref > area_ptr->attr->plot_h ) ref = area_ptr->attr->plot_h;

  ptr->attr->i_symbol_width =
   (int) ( ptr->attr->symbol_width / 100.0 * ref );
  ptr->attr->i_symbol_half_width =
   (int) ( ptr->attr->symbol_half_width / 100.0 * ref );
  ptr->attr->i_symbol_height =
   (int) ( ptr->attr->symbol_height / 100.0 * ref );
  ptr->attr->i_symbol_half_height =
   (int) ( ptr->attr->symbol_half_height / 100.0 * ref );

  return XGRAPH_SUCCESS;

}

int xgraph_add_points (
  PLOT_ID id,
  int num,
  double *x,
  double *y
) {

PLOT_PRIV_PTR ptr;
AREA_PRIV_PTR area_ptr;
int i, x0, y0, x1, y1;
PLOT_DATA_PTR cur, prev;
double logval;

  ptr = (PLOT_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  area_ptr = (AREA_PRIV_PTR) ptr->attr->area_ptr;

  XSetForeground( ptr->attr->dsp, ptr->attr->gc, ptr->attr->color.pixel );

  for ( i=0; i<num; i++ ) {

    if ( !ptr->num_points ) {

      if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LINEAR ) {
        x1 = (int) ( x[i] * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
      }
      else {
        if ( x[i] > 0.0 ) {
          logval = log10( x[i] );
          x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
        }
        else {
          return XGRAPH_BAD_LOG_VALUE;
        }
      }

      if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LINEAR ) {
        y1 = (int) ( y[i] * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
        y1 = area_ptr->attr->plot_h - 1 - y1;
      }
      else {
        if ( y[i] > 0.0 ) {
          logval = log10( y[i] );
          y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
          y1 = area_ptr->attr->plot_h - 1 - y1;
        }
        else {
          return XGRAPH_BAD_LOG_VALUE;
        }
      }

      /* process plot type */
      switch ( ptr->attr->type ) {

        case XGRAPH_POINT:
          if ( ptr->attr->plot_symbol == XGRAPH_NOSYMBOL ) {
            XDrawPoint( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x1, y1 );
          }
          else {
            xgraph___draw_symbol( ptr, x1, y1 );
          }
          break;

        case XGRAPH_NEEDLE:
          x0 = x1;
          y0 = area_ptr->attr->plot_h - 1;
          XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
           x1, y1 );
          xgraph___draw_symbol( ptr, x1, y1 );
          break;

      }

    }
    else {

      cur = ptr->tail;

      if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LINEAR ) {
        x0 = (int) ( cur->x * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
      }
      else {
        if ( cur->x > 0.0 ) {
          logval = log10( cur->x );
          x0 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
        }
        else {
          return XGRAPH_BAD_LOG_VALUE;
        }
      }

      if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LINEAR ) {
        y0 = (int) ( cur->y * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
        y0 = area_ptr->attr->plot_h - 1 - y0;
      }
      else {
        if ( cur->y > 0.0 ) {
          logval = log10( cur->y );
          y0 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
          y0 = area_ptr->attr->plot_h - 1 - y0;
        }
        else {
          return XGRAPH_BAD_LOG_VALUE;
        }
      }

      if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LINEAR ) {
        x1 = (int) ( x[i] * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
      }
      else {
        if ( x[i] > 0.0 ) {
          logval = log10( x[i] );
          x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
        }
        else {
          return XGRAPH_BAD_LOG_VALUE;
        }
      }

      if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LINEAR ) {
        y1 = (int) ( y[i] * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
        y1 = area_ptr->attr->plot_h - 1 - y1;
      }
      else {
        if ( y[i] > 0.0 ) {
          logval = log10( y[i] );
          y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
          y1 = area_ptr->attr->plot_h - 1 - y1;
        }
        else {
          return XGRAPH_BAD_LOG_VALUE;
        }
      }

      /* process plot type */
      switch ( ptr->attr->type ) {

        case XGRAPH_POINT:
          if ( ptr->attr->plot_symbol == XGRAPH_NOSYMBOL ) {
            XDrawPoint( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x1, y1 );
          }
          else {
            xgraph___draw_symbol( ptr, x1, y1 );
          }
          break;

        case XGRAPH_LINE:
          XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
           x1, y1 );
          xgraph___draw_symbol( ptr, x0, y0 );
          xgraph___draw_symbol( ptr, x1, y1 );
          break;

        case XGRAPH_NEEDLE:
          x0 = x1;
          y0 = area_ptr->attr->plot_h - 1;
          XDrawLine( ptr->attr->dsp, ptr->attr->win, ptr->attr->gc, x0, y0,
           x1, y1 );
          xgraph___draw_symbol( ptr, x1, y1 );
          break;

      }

    }

    if ( ptr->lookaside_head->flink ) {
      cur = ptr->lookaside_head->flink;
      ptr->lookaside_head->flink = cur->flink;
    }
    else {
      cur = (PLOT_DATA_PTR) calloc( 1, sizeof(PLOT_PRIV_ATTR_TYPE) );
    }
    if ( !cur ) return XGRAPH_NO_MEM;
    prev = ptr->tail;
    ptr->cur = cur;
    ptr->tail->flink = cur;
    ptr->tail = cur;
    ptr->tail->flink = NULL;
    ptr->tail->blink = prev;
    ptr->num_points++;
    cur->x = x[i];
    cur->y = y[i];

  }

  return XGRAPH_SUCCESS;

}

int xgraph_get_plot_data (
  PLOT_ID id,
  int *num,
  double **x,
  double **y
) {

PLOT_PRIV_PTR ptr;
AREA_PRIV_PTR area_ptr;
int i;
PLOT_DATA_PTR cur;
double *xarray, *yarray;

  ptr = (PLOT_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  area_ptr = (AREA_PRIV_PTR) ptr->attr->area_ptr;

  *num = ptr->num_points;

  xarray = (double *) malloc( sizeof(double) * ptr->num_points );
  yarray = (double *) malloc( sizeof(double) * ptr->num_points );

  i = 0;
  cur = ptr->head->flink;
  while ( cur ) {
    xarray[i] = cur->x;
    yarray[i] = cur->y;
    if ( i < (ptr->num_points-1) ) i++;
    cur = cur->flink;
  }

  *x = xarray;
  *y = yarray;

  return XGRAPH_SUCCESS;

}

int xgraph_delete_points (
  PLOT_ID id
) {

PLOT_PRIV_PTR ptr;
PLOT_DATA_PTR cur;

  ptr = (PLOT_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  cur = ptr->head->flink;

  if ( cur ) {

    ptr->lookaside_tail->flink = cur;
    ptr->lookaside_tail = cur;
    ptr->lookaside_tail->flink = NULL;

    ptr->head->flink = NULL;
    ptr->tail = ptr->head;

  }

  ptr->num_points = 0;

  return XGRAPH_SUCCESS;

}

int xgraph_create_text (
  GENERIC_PTR parent_id,
  TEXT_ID *id
) {

GENERIC_PRIV_PTR ptr;
AREA_PRIV_PTR area_ptr;
GWIN_PRIV_PTR gwin_ptr;
TEXT_PRIV_PTR text_ptr;
XGCValues values;

OBJ_LIST_PTR cur_obj;

int stat, ret_stat;

  text_ptr = (TEXT_PRIV_PTR) calloc( 1, sizeof(TEXT_PRIV_TYPE) );
  if ( !text_ptr ) return XGRAPH_NO_MEM;

  ptr = (GENERIC_PRIV_PTR) parent_id;

  if ( !ptr ) {
    ret_stat = XGRAPH_BAD_OBJ;
    goto err_return;
  }
  if ( !ptr->id_type ) {
    ret_stat = XGRAPH_BAD_OBJ;
    goto err_return;
  }

  values.foreground = BlackPixel(g_display,g_screen_num);
  values.background = WhitePixel(g_display,g_screen_num);

  text_ptr->gc = XCreateGC( g_display, RootWindow(g_display,g_screen_num),
   GCForeground | GCBackground, &values );

  XCopyGC( g_display, DefaultGC(g_display,g_screen_num), 0xffffffff,
   text_ptr->gc );

  text_ptr->xorgc = XCreateGC( g_display, RootWindow(g_display,g_screen_num),
   GCForeground | GCBackground, &values );

  XCopyGC( g_display, text_ptr->gc, 0xffffffff, text_ptr->xorgc );

  values.function = GXxor;
  XChangeGC( g_display, text_ptr->xorgc, GCFunction, &values );

  switch ( ptr->id_type ) {

    case GRAPH_K_GWIN_ID:
      text_ptr->text = NULL;
      gwin_ptr = (GWIN_PRIV_PTR) parent_id;
      text_ptr->dsp = gwin_ptr->attr->dsp;
      text_ptr->win = gwin_ptr->attr->win;
      text_ptr->win_width = gwin_ptr->attr->w;
      text_ptr->win_height = gwin_ptr->attr->h;
      cur_obj = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
      if ( !cur_obj ) {
        ret_stat = XGRAPH_NO_MEM;
        goto err_return;
      }
      cur_obj->ptr = (void *) text_ptr;
      gwin_ptr->obj_tail->next = cur_obj;
      gwin_ptr->obj_tail = cur_obj;
      gwin_ptr->obj_tail->next = NULL;
      break;

    case GRAPH_K_AREA_ID:
      text_ptr->text = NULL;
      area_ptr = (AREA_PRIV_PTR) parent_id;
      text_ptr->dsp = area_ptr->attr->dsp;
      text_ptr->win = area_ptr->attr->win;
      text_ptr->plotwin = area_ptr->attr->plotwin;
      text_ptr->win_width = area_ptr->attr->w;
      text_ptr->win_height = area_ptr->attr->h;
      text_ptr->plotwin_width = area_ptr->attr->plot_w;
      text_ptr->plotwin_height = area_ptr->attr->plot_h;
      cur_obj = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
      if ( !cur_obj ) {
        ret_stat = XGRAPH_NO_MEM;
        goto err_return;
      }
      cur_obj->ptr = (void *) text_ptr;
      area_ptr->obj_tail->next = cur_obj;
      area_ptr->obj_tail = cur_obj;
      area_ptr->obj_tail->next = NULL;
      break;

  }

  text_ptr->parent_id = ptr;
  text_ptr->id_type = GRAPH_K_TEXT_ID;

  *id = (TEXT_ID) text_ptr;

norm_return:
  return XGRAPH_SUCCESS;

err_return:
  text_ptr->id_type = 0;
  *id = (TEXT_ID) text_ptr;
  return ret_stat;

}

int xgraph_create_transient_text (
  GENERIC_PTR parent_id,
  TEXT_ID *id
) {

GENERIC_PRIV_PTR ptr;
AREA_PRIV_PTR area_ptr;
GWIN_PRIV_PTR gwin_ptr;
TEXT_PRIV_PTR text_ptr;
XGCValues values;

OBJ_LIST_PTR cur_obj;

int stat, ret_stat;

  text_ptr = (TEXT_PRIV_PTR) calloc( 1, sizeof(TEXT_PRIV_TYPE) );
  if ( !text_ptr ) return XGRAPH_NO_MEM;

  ptr = (GENERIC_PRIV_PTR) parent_id;

  if ( !ptr ) {
    ret_stat = XGRAPH_BAD_OBJ;
    goto err_return;
  }
  if ( !ptr->id_type ) {
    ret_stat = XGRAPH_BAD_OBJ;
    goto err_return;
  }

  values.foreground = BlackPixel(g_display,g_screen_num);
  values.background = WhitePixel(g_display,g_screen_num);

  text_ptr->gc = XCreateGC( g_display, RootWindow(g_display,g_screen_num),
   GCForeground | GCBackground, &values );

  XCopyGC( g_display, DefaultGC(g_display,g_screen_num), 0xffffffff,
   text_ptr->gc );

  text_ptr->xorgc = XCreateGC( g_display, RootWindow(g_display,g_screen_num),
   GCForeground | GCBackground, &values );

  XCopyGC( g_display, text_ptr->gc, 0xffffffff, text_ptr->xorgc );

  values.function = GXxor;
  XChangeGC( g_display, text_ptr->xorgc, GCFunction, &values );

  switch ( ptr->id_type ) {

    case GRAPH_K_GWIN_ID:
      text_ptr->text = NULL;
      gwin_ptr = (GWIN_PRIV_PTR) parent_id;
      text_ptr->dsp = gwin_ptr->attr->dsp;
      text_ptr->win = gwin_ptr->attr->win;
      text_ptr->win_width = gwin_ptr->attr->w;
      text_ptr->win_height = gwin_ptr->attr->h;
      break;

    case GRAPH_K_AREA_ID:
      text_ptr->text = NULL;
      area_ptr = (AREA_PRIV_PTR) parent_id;
      text_ptr->dsp = area_ptr->attr->dsp;
      text_ptr->win = area_ptr->attr->win;
      text_ptr->plotwin = area_ptr->attr->plotwin;
      text_ptr->win_width = area_ptr->attr->w;
      text_ptr->win_height = area_ptr->attr->h;
      text_ptr->plotwin_width = area_ptr->attr->plot_w;
      text_ptr->plotwin_height = area_ptr->attr->plot_h;
      break;

  }

  text_ptr->parent_id = ptr;
  text_ptr->id_type = GRAPH_K_TEXT_ID;

  *id = (TEXT_ID) text_ptr;

norm_return:
  return XGRAPH_SUCCESS;

err_return:
  text_ptr->id_type = 0;
  *id = (TEXT_ID) text_ptr;
  return ret_stat;

}

int xgraph_set_text_attr (
  TEXT_ID id,
  char *color,
  char *font,
  char justify
) {

TEXT_PRIV_PTR text_ptr;
int stat;
AREA_PRIV_PTR area_ptr;
GWIN_PRIV_PTR gwin_ptr;
char full_font[127+1];

  text_ptr = (TEXT_PRIV_PTR) id;

  if ( !text_ptr ) return XGRAPH_BAD_OBJ;
  if ( !text_ptr->id_type ) return XGRAPH_BAD_OBJ;

  stat = xgraph___alloc_color( color, g_colormap, &text_ptr->color );
  if ( stat != XGRAPH_SUCCESS ) text_ptr->color.pixel = g_foreground;

  stat = xgraph___full_font_name( font, full_font );
  if ( stat != XGRAPH_SUCCESS ) strncpy( full_font, font, 127 );

  text_ptr->fontstruct = XLoadQueryFont( g_display, full_font );
  if ( text_ptr->fontstruct ) {
    XSetFont( g_display, text_ptr->gc, text_ptr->fontstruct->fid );
    XSetFont( g_display, text_ptr->xorgc, text_ptr->fontstruct->fid );
  }
  else
    return XGRAPH_NO_FONT;

  justify = toupper( justify );
  if ( justify == 'C' )
    text_ptr->justify = GRAPH_K_CENTER;
  else if ( justify == 'R' )
    text_ptr->justify = GRAPH_K_RIGHT;
  else
    text_ptr->justify = GRAPH_K_LEFT;

  if ( text_ptr->parent_id->id_type == GRAPH_K_AREA_ID ) {

    area_ptr = (AREA_PRIV_PTR) text_ptr->parent_id;

    XSetBackground( g_display, text_ptr->xorgc,
     area_ptr->attr->bg_color.pixel );
    XSetForeground( g_display, text_ptr->xorgc,
     (text_ptr->color.pixel)^(area_ptr->attr->bg_color.pixel) );

    XSetBackground( g_display, text_ptr->gc,
     area_ptr->attr->bg_color.pixel );
    XSetForeground( g_display, text_ptr->gc, text_ptr->color.pixel );

  }
  else if ( text_ptr->parent_id->id_type == GRAPH_K_GWIN_ID ) {

    gwin_ptr = (GWIN_PRIV_PTR) text_ptr->parent_id;

    XSetBackground( g_display, text_ptr->xorgc,
     gwin_ptr->attr->bg_color.pixel );
    XSetForeground( g_display, text_ptr->xorgc,
     (text_ptr->color.pixel)^(gwin_ptr->attr->bg_color.pixel) );

    XSetBackground( g_display, text_ptr->gc,
     gwin_ptr->attr->bg_color.pixel );
    XSetForeground( g_display, text_ptr->gc, text_ptr->color.pixel );

  }

  return XGRAPH_SUCCESS;

}

static int put_text (
  TEXT_ID id
) {

TEXT_PRIV_PTR text_ptr;
int stat;
int len_1st, len, posx, posy;
char first[2];

  text_ptr = (TEXT_PRIV_PTR) id;

  if ( !text_ptr ) return XGRAPH_BAD_OBJ;
  if ( !text_ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( !text_ptr->text ) return XGRAPH_SUCCESS;

  if ( text_ptr->parent_id->id_type == GRAPH_K_AREA_ID ) {

    strncpy( first, text_ptr->text, 1 );
    len_1st = XTextWidth( text_ptr->fontstruct, first, 1 );

    posx = text_ptr->x * 0.01 * text_ptr->plotwin_width-1 + len_1st;

    len = XTextWidth( text_ptr->fontstruct, text_ptr->text,
     strlen(text_ptr->text) );
    if ( text_ptr->justify == GRAPH_K_CENTER ) {
      posx -= len * 0.5;
    }
    else if ( text_ptr->justify == GRAPH_K_RIGHT ) {
      posx -= len;
    }

    posy = text_ptr->y * 0.01 * text_ptr->plotwin_height-1;
    posy = text_ptr->plotwin_height - 1 - posy;

    XDrawString( text_ptr->dsp, text_ptr->plotwin, text_ptr->gc, posx,
     posy, text_ptr->text, strlen(text_ptr->text) );

  }
  else if ( text_ptr->parent_id->id_type == GRAPH_K_GWIN_ID ) {

    strncpy( first, text_ptr->text, 1 );
    len_1st = XTextWidth( text_ptr->fontstruct, first, 1 );

    posx = text_ptr->x * 0.01 * text_ptr->win_width-1 + len_1st;

    len = XTextWidth( text_ptr->fontstruct, text_ptr->text,
     strlen(text_ptr->text) );
    if ( text_ptr->justify == GRAPH_K_CENTER ) {
      posx -= len * 0.5;
    }
    else if ( text_ptr->justify == GRAPH_K_RIGHT ) {
      posx -= len;
    }

    posy = text_ptr->y * 0.01 * text_ptr->win_height-1;
    posy = text_ptr->win_height - 1 - posy;

    XDrawString( text_ptr->dsp, text_ptr->win, text_ptr->gc, posx,
     posy, text_ptr->text, strlen(text_ptr->text) );

  }

  return XGRAPH_SUCCESS;

}

int xgraph_put_text (
  TEXT_ID id,
  double x,		/* percent */
  double y,		/* percent */
  char *text
) {

TEXT_PRIV_PTR text_ptr;
int l_old, l_new, stat;

  text_ptr = (TEXT_PRIV_PTR) id;

  if ( !text_ptr ) return XGRAPH_BAD_OBJ;
  if ( !text_ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( text_ptr->text ) {
    l_old = strlen(text_ptr->text);
    l_new = strlen(text);
    if ( l_new > l_old )
      text_ptr->text = (char *) realloc( text_ptr->text, strlen(text)+1 );
  }
  else
    text_ptr->text = calloc( 1, strlen(text)+1 );

  strcpy( text_ptr->text, text );

  text_ptr->x = x;
  text_ptr->y = y;

  stat = put_text( id );

  return stat;

}

int xgraph_output_text (
  TEXT_ID id,
  double x,		/* percent */
  double y,		/* percent */
  char *text,
  int code		/* 1=norm, 2=xor, 0=erase */
) {

TEXT_PRIV_PTR text_ptr;
int l_old, l_new, stat;
int len_1st, len, posx, posy;
char first[2];
AREA_PRIV_PTR area_ptr;
GWIN_PRIV_PTR gwin_ptr;

  text_ptr = (TEXT_PRIV_PTR) id;

  if ( !text_ptr ) return XGRAPH_BAD_OBJ;
  if ( !text_ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( text_ptr->text ) {
    l_old = strlen(text_ptr->text);
    l_new = strlen(text);
    if ( l_new > l_old )
      text_ptr->text = (char *) realloc( text_ptr->text, strlen(text)+1 );
  }
  else
    text_ptr->text = calloc( 1, strlen(text)+1 );

  strcpy( text_ptr->text, text );

  text_ptr->x = x;
  text_ptr->y = y;

  if ( !text_ptr->text ) return XGRAPH_SUCCESS;

  if ( text_ptr->parent_id->id_type == GRAPH_K_AREA_ID ) {

    area_ptr = (AREA_PRIV_PTR) text_ptr->parent_id;

    strncpy( first, text_ptr->text, 1 );
    len_1st = XTextWidth( text_ptr->fontstruct, first, 1 );

    posx = text_ptr->x * 0.01 * text_ptr->plotwin_width-1 + len_1st;

    len = XTextWidth( text_ptr->fontstruct, text_ptr->text,
     strlen(text_ptr->text) );
    if ( text_ptr->justify == GRAPH_K_CENTER ) {
      posx -= len * 0.5;
    }
    else if ( text_ptr->justify == GRAPH_K_RIGHT ) {
      posx -= len;
    }

    posy = text_ptr->y * 0.01 * text_ptr->plotwin_height-1;
    posy = text_ptr->plotwin_height - 1 - posy;

    if ( code == 1 ) {
      XDrawString( text_ptr->dsp, text_ptr->plotwin, text_ptr->gc, posx,
       posy, text_ptr->text, strlen(text_ptr->text) );
    }
    else if ( code == 2 ) {
      XDrawString( text_ptr->dsp, text_ptr->plotwin, text_ptr->xorgc, posx,
       posy, text_ptr->text, strlen(text_ptr->text) );
    }
    else if ( code == 0 ) {
      XSetBackground( g_display, text_ptr->gc,
       text_ptr->color.pixel );
      XSetForeground( g_display, text_ptr->gc,
       area_ptr->attr->bg_color.pixel );
        XDrawString( text_ptr->dsp, text_ptr->plotwin, text_ptr->gc, posx,
       posy, text_ptr->text, strlen(text_ptr->text) );
      XSetBackground( g_display, text_ptr->gc,
       area_ptr->attr->bg_color.pixel );
      XSetForeground( g_display, text_ptr->gc, text_ptr->color.pixel );
    }

  }
  else if ( text_ptr->parent_id->id_type == GRAPH_K_GWIN_ID ) {

    gwin_ptr = (GWIN_PRIV_PTR) text_ptr->parent_id;

    strncpy( first, text_ptr->text, 1 );
    len_1st = XTextWidth( text_ptr->fontstruct, first, 1 );

    posx = text_ptr->x * 0.01 * text_ptr->win_width-1 + len_1st;

    len = XTextWidth( text_ptr->fontstruct, text_ptr->text,
     strlen(text_ptr->text) );
    if ( text_ptr->justify == GRAPH_K_CENTER ) {
      posx -= len * 0.5;
    }
    else if ( text_ptr->justify == GRAPH_K_RIGHT ) {
      posx -= len;
    }

    posy = text_ptr->y * 0.01 * text_ptr->win_height-1;
    posy = text_ptr->win_height - 1 - posy;

    if ( code == 1 ) {
      XDrawString( text_ptr->dsp, text_ptr->win, text_ptr->gc, posx,
       posy, text_ptr->text, strlen(text_ptr->text) );
    }
    else if ( code == 2 ) {
      XDrawString( text_ptr->dsp, text_ptr->win, text_ptr->xorgc, posx,
       posy, text_ptr->text, strlen(text_ptr->text) );
    }
    else if ( code == 0 ) {
      XSetBackground( g_display, text_ptr->gc,
       text_ptr->color.pixel );
      XSetForeground( g_display, text_ptr->gc,
       gwin_ptr->attr->bg_color.pixel );
        XDrawString( text_ptr->dsp, text_ptr->win, text_ptr->gc, posx,
       posy, text_ptr->text, strlen(text_ptr->text) );
      XSetBackground( g_display, text_ptr->gc,
       gwin_ptr->attr->bg_color.pixel );
      XSetForeground( g_display, text_ptr->gc, text_ptr->color.pixel );
    }

  }

  return XGRAPH_SUCCESS;

}

int xgraph_refresh_area (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
OBJ_LIST_PTR cur_obj;
GENERIC_PRIV_PTR generic_id;
PLOT_ID plot_id;
TEXT_ID text_id;
int stat, ret_stat;

  ret_stat = XGRAPH_SUCCESS;
  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  stat = xgraph_draw_title( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_x_scale( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_x_annotation( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_x_label( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_y_scale( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_y_annotation( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_y_label( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  /* refresh all plots */
  cur_obj = ptr->obj_head->next;
  while ( cur_obj ) {
    generic_id = (GENERIC_PRIV_PTR) cur_obj->ptr;
    if ( generic_id->id_type == GRAPH_K_PLOT_ID ) {
      plot_id = (PLOT_ID) cur_obj->ptr;
      stat = xgraph_draw_plot( plot_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    else if ( generic_id->id_type == GRAPH_K_TEXT_ID ) {
      text_id = (TEXT_ID) cur_obj->ptr;
      stat = put_text( text_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    cur_obj = cur_obj->next;
  }

  return ret_stat;

}

int xgraph_refresh_area_not_plot (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
OBJ_LIST_PTR cur_obj;
GENERIC_PRIV_PTR generic_id;
PLOT_ID plot_id;
TEXT_ID text_id;
int stat, ret_stat;

  ret_stat = XGRAPH_SUCCESS;
  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  stat = xgraph_draw_title( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_x_scale( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_x_annotation( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_x_label( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_y_scale( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_y_annotation( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_y_label( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  /* refresh all plots */

/*
  cur_obj = ptr->obj_head->next;
  while ( cur_obj ) {
    generic_id = (GENERIC_PRIV_PTR) cur_obj->ptr;
    if ( generic_id->id_type == GRAPH_K_PLOT_ID ) {
      plot_id = (PLOT_ID) cur_obj->ptr;
      stat = xgraph_draw_plot( plot_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    else if ( generic_id->id_type == GRAPH_K_TEXT_ID ) {
      text_id = (TEXT_ID) cur_obj->ptr;
      stat = put_text( text_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    cur_obj = cur_obj->next;
  }
*/

  return ret_stat;

}

int xgraph_refresh_plot (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
OBJ_LIST_PTR cur_obj;
GENERIC_PRIV_PTR generic_id;
PLOT_ID plot_id;
TEXT_ID text_id;
int stat, ret_stat;

  ret_stat = XGRAPH_SUCCESS;
  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

/*
  stat = xgraph_draw_title( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_x_scale( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_x_annotation( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_x_label( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_y_scale( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_y_annotation( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_draw_y_label( id );
  if ( !( stat & 1 ) ) ret_stat = stat;
*/

  /* refresh all plots */
  cur_obj = ptr->obj_head->next;
  while ( cur_obj ) {
    generic_id = (GENERIC_PRIV_PTR) cur_obj->ptr;
    if ( generic_id->id_type == GRAPH_K_PLOT_ID ) {
      plot_id = (PLOT_ID) cur_obj->ptr;
      stat = xgraph_draw_plot( plot_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    else if ( generic_id->id_type == GRAPH_K_TEXT_ID ) {
      text_id = (TEXT_ID) cur_obj->ptr;
      stat = put_text( text_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    cur_obj = cur_obj->next;
  }

  return ret_stat;

}

int xgraph_area_rescale_x (
  AREA_ID id,
  double xmin,
  double xmax,
  double x_major_tick_inc,
  double x_minor_tick_inc,
  double x_tick_label_inc,
  char *format
) {

AREA_PRIV_PTR ptr;
OBJ_LIST_PTR cur_obj;
GENERIC_PRIV_PTR generic_id;
PLOT_PRIV_PTR plot_ptr;
int stat, ret_stat;

  ret_stat = XGRAPH_SUCCESS;
  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LOG ) {
    stat = xgraph___log_adjust_low_limit( &xmin );
    stat = xgraph___log_adjust_high_limit( &xmax );
  }

  ptr->attr->xmin = xmin;
  ptr->attr->xmax = xmax;

  ptr->attr->xfact = (double) ( ptr->attr->plot_w - 1 ) / ( xmax - xmin );
  ptr->attr->xofs = (double) ( ptr->attr->xfact * xmin * -1.0 );

  ptr->attr->x_major_tick_inc = x_major_tick_inc;
  ptr->attr->x_minor_tick_inc = x_minor_tick_inc;
  ptr->attr->x_tick_label_inc = x_tick_label_inc;

  if ( format ) {
    if ( format[0] ) {
      strncpy( ptr->attr->xformat, format, FORMAT_MAX_CHARS );
      ptr->attr->xformat[FORMAT_MAX_CHARS] = 0;
    }
  }

  /* rescale all plots */
  cur_obj = ptr->obj_head->next;
  while ( cur_obj ) {
    generic_id = (GENERIC_PRIV_PTR) cur_obj->ptr;
    if ( generic_id->id_type == GRAPH_K_PLOT_ID ) {
      plot_ptr = (PLOT_PRIV_PTR) cur_obj->ptr;
      plot_ptr->attr->xmin = xmin;
      plot_ptr->attr->xmax = xmax;
      plot_ptr->attr->xfact = ptr->attr->xfact;
      plot_ptr->attr->xofs = ptr->attr->xofs;
    }
    cur_obj = cur_obj->next;
  }

  stat = xgraph_clear_area( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_clear_plot_area( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_refresh_area( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  return ret_stat;

}

int xgraph_area_rescale_y (
  AREA_ID id,
  double ymin,
  double ymax,
  double y_major_tick_inc,
  double y_minor_tick_inc,
  double y_tick_label_inc,
  char *format
) {

AREA_PRIV_PTR ptr;
OBJ_LIST_PTR cur_obj;
GENERIC_PRIV_PTR generic_id;
PLOT_PRIV_PTR plot_ptr;
int stat, ret_stat;

  ret_stat = XGRAPH_SUCCESS;
  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LOG ) {
    stat = xgraph___log_adjust_low_limit( &ymin );
    stat = xgraph___log_adjust_high_limit( &ymax );
  }

  ptr->attr->ymin = ymin;
  ptr->attr->ymax = ymax;

  ptr->attr->yfact = (double) ( ptr->attr->plot_h - 1 ) / ( ymax - ymin );
  ptr->attr->yofs = (double) ( ptr->attr->yfact * ymin * -1.0 );

  ptr->attr->y_major_tick_inc = y_major_tick_inc;
  ptr->attr->y_minor_tick_inc = y_minor_tick_inc;
  ptr->attr->y_tick_label_inc = y_tick_label_inc;

  if ( format ) {
    if ( format[0] ) {
      strncpy( ptr->attr->yformat, format, FORMAT_MAX_CHARS );
      ptr->attr->yformat[FORMAT_MAX_CHARS] = 0;
    }
  }

  /* rescale all plots */
  cur_obj = ptr->obj_head->next;
  while ( cur_obj ) {
    generic_id = (GENERIC_PRIV_PTR) cur_obj->ptr;
    if ( generic_id->id_type == GRAPH_K_PLOT_ID ) {
      plot_ptr = (PLOT_PRIV_PTR) cur_obj->ptr;
      plot_ptr->attr->ymin = ymin;
      plot_ptr->attr->ymax = ymax;
      plot_ptr->attr->yfact = ptr->attr->yfact;
      plot_ptr->attr->yofs = ptr->attr->yofs;
    }
    cur_obj = cur_obj->next;
  }

  stat = xgraph_clear_area( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_clear_plot_area( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = xgraph_refresh_area( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  return ret_stat;

}

int xgraph_refresh_gwin (
  GWIN_ID id
) {

GWIN_PRIV_PTR ptr;
AREA_ID area_id;
TEXT_ID text_id;
OBJ_LIST_PTR cur_obj;
GENERIC_PRIV_PTR generic_id;
int stat, ret_stat;

  ret_stat = XGRAPH_SUCCESS;
  ptr = (GWIN_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  /* refresh all areas */
  cur_obj = ptr->obj_head->next;
  while ( cur_obj ) {
    generic_id = (GENERIC_PRIV_PTR) cur_obj->ptr;
    if ( generic_id->id_type == GRAPH_K_AREA_ID ) {
      area_id = (AREA_ID) cur_obj->ptr;
      stat = xgraph_refresh_area( area_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    else if ( generic_id->id_type == GRAPH_K_TEXT_ID ) {
      text_id = (TEXT_ID) cur_obj->ptr;
      stat = put_text( text_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    cur_obj = cur_obj->next;
  }

  return ret_stat;

}

int xgraph_refresh_gwin_not_area (
  GWIN_ID id
) {

GWIN_PRIV_PTR ptr;
AREA_ID area_id;
TEXT_ID text_id;
OBJ_LIST_PTR cur_obj;
GENERIC_PRIV_PTR generic_id;
int stat, ret_stat;

  ret_stat = XGRAPH_SUCCESS;
  ptr = (GWIN_PRIV_PTR) id;

  if ( !ptr ) return XGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return XGRAPH_BAD_OBJ;

  /* refresh all areas */
  cur_obj = ptr->obj_head->next;
  while ( cur_obj ) {
    generic_id = (GENERIC_PRIV_PTR) cur_obj->ptr;
    if ( generic_id->id_type == GRAPH_K_AREA_ID ) {
      area_id = (AREA_ID) cur_obj->ptr;
/*
      stat = xgraph_refresh_area( area_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
*/
    }
    else if ( generic_id->id_type == GRAPH_K_TEXT_ID ) {
      text_id = (TEXT_ID) cur_obj->ptr;
      stat = put_text( text_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    cur_obj = cur_obj->next;
  }

  return ret_stat;

}

int xgraph_flush( void ) {

WIN_LIST_PTR cur_win, rw, cw;
XEvent event;
XAnyEvent *any;
int stat, rootx, rooty, win_x, win_y, mask;
float fx, fy;
AREA_PRIV_PTR ptr;

  while ( XPending( g_display ) ) {

    XNextEvent( g_display, &event );

    /* ????????????????? */
    any = (XAnyEvent *) &event;
    if ( any->send_event ) printf( "Got client event\n" );

    if ( event.type == Expose ) {

      if ( !event.xexpose.count ) {

        cur_win = g_win_head->flink;
        while ( cur_win ) {
          if ( cur_win->w == event.xany.window ) {
            if ( cur_win->id_type == GRAPH_K_GWIN_ID )
              stat = xgraph_refresh_gwin_not_area( (GWIN_ID) cur_win->id );
            else if ( cur_win->id_type == GRAPH_K_AREA_ID )
              stat = xgraph_refresh_area_not_plot( (AREA_ID) cur_win->id );
            else if ( cur_win->id_type == GRAPH_K_PLOT_AREA_ID )
              stat = xgraph_refresh_plot( (AREA_ID) cur_win->id );
            break;
          }
          cur_win = cur_win->flink;
        }

      }

    }

  }

  XFlush( g_display );

  return XGRAPH_SUCCESS;

}

int xgraph_read_pointer (
  float *x,
  float *y,
  int *button_pressed
) {

WIN_LIST_PTR cur_win;
Window rw, cw;
XEvent event;
int stat, rootx, rooty, win_x, win_y, mask;
AREA_PRIV_PTR ptr;

int no_reading = TRUE;
int no_expose = TRUE;

  *button_pressed = 0;

  while ( XPending( g_display ) ) {

    XNextEvent( g_display, &event );
    if ( event.type == Expose ) {

      if ( !event.xexpose.count ) {

        no_expose = FALSE;

        cur_win = g_win_head->flink;
        while ( cur_win ) {
          if ( cur_win->w == event.xany.window ) {
            if ( cur_win->id_type == GRAPH_K_GWIN_ID )
              stat = xgraph_refresh_gwin_not_area( (GWIN_ID) cur_win->id );
            else if ( cur_win->id_type == GRAPH_K_AREA_ID )
              stat = xgraph_refresh_area_not_plot( (AREA_ID) cur_win->id );
            else if ( cur_win->id_type == GRAPH_K_PLOT_AREA_ID )
              stat = xgraph_refresh_plot( (AREA_ID) cur_win->id );
            break;
          }
          cur_win = cur_win->flink;
        }

      }

    }
    else if ( event.type == MotionNotify ) {

      cur_win = g_win_head->flink;
      while ( cur_win ) {
        if ( cur_win->w == event.xany.window ) {
          ptr = (AREA_PRIV_PTR) cur_win->id;
          XQueryPointer( g_display, cur_win->w, &rw, &cw, &rootx, &rooty,
           &win_x, &win_y, &mask );
          if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LINEAR ) {
            *x = ( (float) win_x - ptr->attr->xofs ) / ptr->attr->xfact;
          }
          else {
            *x = ( (float) win_x - ptr->attr->xofs ) / ptr->attr->xfact;
            *x = pow( 10.0, *x );
          }
          if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LINEAR ) {
            *y = ( ptr->attr->plot_h - (float) win_y + ptr->attr->yofs ) /
             ptr->attr->yfact;
          }
          else {
            win_y = ptr->attr->plot_h - win_y;
            *y = ( (float) win_y - ptr->attr->yofs ) / ptr->attr->yfact;
            *y = pow( 10.0, *y );
          }
          no_reading = FALSE;
          break;
        }
        cur_win = cur_win->flink;
      }

    }

    else if ( event.type == ButtonPress ) {

      cur_win = g_win_head->flink;
      while ( cur_win ) {
        if ( cur_win->w == event.xany.window ) {
          ptr = (AREA_PRIV_PTR) cur_win->id;
          XQueryPointer( g_display, cur_win->w, &rw, &cw, &rootx, &rooty,
           &win_x, &win_y, &mask );
          if ( mask & Button3Mask ) *button_pressed = 1;
          if ( ptr->attr->x_scale_type == XGRAPH_SCALE_LINEAR ) {
            *x = ( (float) win_x - ptr->attr->xofs ) / ptr->attr->xfact;
          }
          else {
            *x = ( (float) win_x - ptr->attr->xofs ) / ptr->attr->xfact;
            *x = pow( 10.0, *x );
          }
          if ( ptr->attr->y_scale_type == XGRAPH_SCALE_LINEAR ) {
            *y = ( ptr->attr->plot_h - (float) win_y + ptr->attr->yofs ) /
             ptr->attr->yfact;
          }
          else {
            win_y = ptr->attr->plot_h - win_y;
            *y = ( (float) win_y - ptr->attr->yofs ) / ptr->attr->yfact;
            *y = pow( 10.0, *y );
          }
          no_reading = FALSE;
          break;
        }
        cur_win = cur_win->flink;
      }

    }

  }

  if ( !no_expose ) {
    XFlush( g_display );
  }

  if ( no_reading ) {
    *x = 0.0;
    *y = 0.0;
    return 0;
  }

  return XGRAPH_SUCCESS;

}

void xgraph_bell ( void ) {

  XBell( g_display, 100 );
  XFlush( g_display );

}
