#ifndef __psgraph_priv_h
#define __psgraph_priv_h 1

#include "decw$include:xlib.h"
#include "decw$include:xutil.h"
#include "decw$include:cursorfont.h"
#include "decw$include:xresource.h"
#include "decw$include:keysym.h"

#include psgraph

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
  GWIN_PRIV_PTR gwin_ptr;
  int area_border_size;
  XColor grid_color;
  int xlabel_grid;
  int xmajor_grid;
  int xminor_grid;
  int ylabel_grid;
  int ymajor_grid;
  int yminor_grid;
  XColor bg_color;	/* let X determine the rgb value for the named color */
  XColor fg_color;	/* let X determine the rgb value for the named color */
  char *title_font_name;
  float title_font_size;
  char *label_font_name;
  float label_font_size;
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
  int type;		/* PSGRAPH_POINT=point, PSGRAPH_LINE=line,
			   PSGRAPH_NEEDLE=needle, PSGRAPH_BAR=bar (not impl),
			   PSGRAPH_FILL_BAR=filled_bar (not impl), */
  XColor color;
  int line_type;	/* (not impl) PSGRAPH_SOLID=solid, PSGRAPH_DASH=dash,
			   PSGRAPH_DOT=dot, PSGRAPH_DASH_DOT_DOT=dash-dot-dot */

  int plot_symbol;	/* PSGRAPH_NOSYMBOL=none, PSGRAPH_CIRCLE=circle,
			   PSGRAPH_SQUARE=square, PSGRAPH_DIAMOND=diamond,
			   PSGRAPH_TRIANGLE=triangle */
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
  XColor color;
  char *font_name;
  float font_size;
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
