#ifndef __xgraph_priv_h
#define __xgraph_priv_h 1

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "xgraph.h"

/*
* constants
*/
#define GRAPH_K_GWIN_ID		0X1000
#define GRAPH_K_AREA_ID		0X1001
#define GRAPH_K_PLOT_AREA_ID	0X1002
#define GRAPH_K_PLOT_ID		0X1003
#define GRAPH_K_TEXT_ID		0X1004

#define GRAPH_K_VERTICAL	0X2000
#define GRAPH_K_HORIZONTAL	0X2001

#define GRAPH_K_LEFT		0x3000
#define GRAPH_K_CENTER		0x3001
#define GRAPH_K_RIGHT		0x3002

/*
* types
*/
typedef struct obj_list_tag {
  void *ptr;
  struct obj_list_tag *next;
} OBJ_LIST_TYPE, *OBJ_LIST_PTR;

typedef struct generic_tag {
  int id_type;
  OBJ_LIST_PTR obj_head;
  OBJ_LIST_PTR obj_tail;
} GENERIC_PRIV_TYPE, *GENERIC_PRIV_PTR;

typedef struct gwin_priv_attr_tag {
  Display *dsp;
  Window win;
  char *window_name;
  char *icon_name;
  XSizeHints sizehints;
  XWMHints wmhints;
  XClassHint classhint;
  XSetWindowAttributes attributes;
  GC gc;
  GC xorgc;
  XColor bg_color;
  double origin_x;
  double origin_y;
  double height;
  double width;
  int x;
  int y;
  int w;
  int h;
} GWIN_PRIV_ATTR_TYPE, *GWIN_PRIV_ATTR_PTR;

typedef struct gwin_priv_tag {
  int id_type;
  OBJ_LIST_PTR obj_head;
  OBJ_LIST_PTR obj_tail;
  GWIN_PRIV_ATTR_PTR attr;
} GWIN_PRIV_TYPE, *GWIN_PRIV_PTR;

typedef struct area_priv_attr_tag {
  Display *dsp;
  Window win;
  XSetWindowAttributes attributes;
  GC gc;
  GC xorgc;
  XColor bg_color;
  XColor fg_color;
  XFontStruct *title_fontstruct;
  int title_font_height;
  XFontStruct *label_fontstruct;
  int label_font_height;
  double origin_x;
  double origin_y;
  double height;
  double width;
  int x_scale_type;	/* linear or log */
  int y_scale_type;	/* linear or log */
  int x;
  int y;
  int w;
  int h;
  Window plotwin;
  int plot_x;
  int plot_y;
  int plot_w;
  int plot_h;
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double xfact;
  double xofs;
  double yfact;
  double yofs;
  double x_major_tick_inc;
  double x_minor_tick_inc;
  double x_tick_label_inc;
  double y_major_tick_inc;
  double y_minor_tick_inc;
  double y_tick_label_inc;
  int ylabel_orientation;
  char title[TITLE_MAX_CHARS+1];
  char xformat[FORMAT_MAX_CHARS+1];
  char xlabel[LABEL_MAX_CHARS+1];
  char yformat[FORMAT_MAX_CHARS+1];
  char ylabel[LABEL_MAX_CHARS+1];
} AREA_PRIV_ATTR_TYPE, *AREA_PRIV_ATTR_PTR;

typedef struct area_priv_tag {
  int id_type;
  OBJ_LIST_PTR obj_head;
  OBJ_LIST_PTR obj_tail;
  AREA_PRIV_ATTR_PTR attr;
} AREA_PRIV_TYPE, *AREA_PRIV_PTR;

typedef struct plot_priv_attr_tag {
  AREA_PRIV_PTR area_ptr;
  Display *dsp;
  Window win;
  GC gc;
  GC xorgc;
  int type;		/* XGRAPH_POINT=point, XGRAPH_LINE=line,
			   XGRAPH_NEEDLE=needle, XGRAPH_BAR=bar (not impl),
			   XGRAPH_FILL_BAR=filled_bar (not impl), */
  XColor color;
  int line_type;	/* (not impl) XGRAPH_SOLID=solid, XGRAPH_DASH=dash,
			   XGRAPH_DOT=dot, XGRAPH_DASH_DOT_DOT=dash-dot-dot */

  int plot_symbol;	/* XGRAPH_NOSYMBOL=none, XGRAPH_CIRCLE=circle,
			   XGRAPH_SQUARE=square, XGRAPH_DIAMOND=diamond,
			   XGRAPH_TRIANGLE=triangle */
  XColor symbol_color;
  int symbol_fill;
  float symbol_width;	/* in % of smaller axis */
  float symbol_height;	/* in % of smaller axis */
  float symbol_half_width;	/* in % of smaller axis */
  float symbol_half_height;	/* in % of smaller axis */
  int i_symbol_width;
  int i_symbol_height;
  int i_symbol_half_width;
  int i_symbol_half_height;
  double bar_width;
  int x_scale_type;	/* linear or log */
  int y_scale_type;	/* linear or log */
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double xfact;
  double xofs;
  double yfact;
  double yofs;
} PLOT_PRIV_ATTR_TYPE, *PLOT_PRIV_ATTR_PTR;

typedef struct plot_data_tag {
  double x;
  double y;
  struct plot_data_tag *flink;
  struct plot_data_tag *blink;
} PLOT_DATA_TYPE, *PLOT_DATA_PTR;

typedef struct plot_priv_tag {
  int id_type;
  int num_points;
  PLOT_DATA_PTR head;
  PLOT_DATA_PTR tail;
  PLOT_DATA_PTR cur;
  PLOT_DATA_PTR lookaside_head;
  PLOT_DATA_PTR lookaside_tail;
  PLOT_PRIV_ATTR_PTR attr;
} PLOT_PRIV_TYPE, *PLOT_PRIV_PTR;

typedef struct text_priv_tag {
  int id_type;
  GENERIC_PRIV_PTR parent_id;
  Display *dsp;
  Window win;
  Window plotwin;
  GC gc;
  GC xorgc;
  XColor color;
  int xor;
  XFontStruct *fontstruct;
  int win_width;
  int win_height;
  int plotwin_width;
  int plotwin_height;
  int justify;
  char *text;
  int x;
  int y;
} TEXT_PRIV_TYPE, *TEXT_PRIV_PTR;

#endif
