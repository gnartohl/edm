#include stdlib
#include stdio
#include math
#include ctype
#include descrip
#include ssdef

#include psgraph_priv

#define MAKE_FIXED_DESCRIP( list, num_bytes, d_list )\
  d_list.dsc$w_length = num_bytes;\
  d_list.dsc$b_dtype = 14;\
  d_list.dsc$b_class = 1;\
  d_list.dsc$a_pointer = list;

#define X_RES 10000.0
#define Y_RES 10000.0
#define DEF_PAPER_LEN 11.0
#define DEF_PAPER_HT 8.5
#define POINT_SIZE (1.0/72.0)

typedef struct win_list_tag {
  void *id;
  int id_type;
  struct win_list_tag *flink;
} WIN_LIST_TYPE, *WIN_LIST_PTR;

static WIN_LIST_PTR g_win_head=NULL, g_win_tail=NULL;

static g_init = 1;
static FILE *g_pf;
static int g_screen_num;
static Visual *g_visual;
static Display *g_display;
static Colormap g_colormap;
static int g_foreground, g_background;
static double g_display_height = Y_RES;
static double g_display_width = X_RES;
static double g_paper_len = DEF_PAPER_LEN;
static double g_paper_ht = DEF_PAPER_HT;
static double g_x, g_y, g_saved_x, g_saved_y;
static int g_font_size = 10;
static char g_font_name[128+1];
static double g_x_scale, g_y_scale, g_x_scale_text, g_y_scale_text;
static int g_saved_line_width = 1, g_cur_line_width = 1;

static int psgraph___log_adjust_low_limit (
  double *vlimit
) {

double limit;

  if ( *vlimit == 0.0 ) return PSGRAPH_BAD_LOG_VALUE;

  *vlimit = log10(*vlimit);
  limit = (double) ( (int) *vlimit );
  if ( limit != *vlimit ) *vlimit = limit - 1.0;

  return PSGRAPH_SUCCESS;

}

static int psgraph___log_adjust_high_limit (
  double *vlimit
) {

double limit;

  if ( *vlimit == 0.0 ) return PSGRAPH_BAD_LOG_VALUE;

  *vlimit = log10(*vlimit);
  limit = (double) ( (int) *vlimit );
  if ( limit != *vlimit ) *vlimit = limit + 1.0;

  return PSGRAPH_SUCCESS;

}

static void psgraph___discard_trailing_blanks (
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

static int psgraph___alloc_color (
  char *color_name,
  Colormap cmap,
  XColor *best_color
) {

int stat;
XColor color;

  stat = XLookupColor( g_display, cmap, color_name, &color, best_color );
  if ( stat ) {
    stat = XAllocColor( g_display, cmap, best_color );
    if ( stat ) return PSGRAPH_SUCCESS;
  }

  return PSGRAPH_NO_COLOR;

}

int psgraph_init( void ) {

int stat;
char display_name[63+1];

  if ( g_init ) {

    strcpy( display_name, "" );

    /* open display */
    g_display = XOpenDisplay( display_name );
    if ( !g_display ) return PSGRAPH_NO_DISPLAY;

    g_screen_num = DefaultScreen( g_display );
    g_visual = DefaultVisual( g_display, g_screen_num );
    g_colormap = DefaultColormap( g_display, g_screen_num );
    g_background = WhitePixel( g_display, g_screen_num );
    g_foreground = BlackPixel( g_display, g_screen_num );

    g_win_head = (WIN_LIST_PTR) calloc( 1, sizeof(WIN_LIST_TYPE) );
    if ( !g_win_head ) return PSGRAPH_NO_MEM;
    g_win_tail = g_win_head;
    g_win_tail->flink = NULL;

    g_x_scale = g_paper_len / POINT_SIZE / g_display_width;
    g_y_scale = g_paper_ht / POINT_SIZE / g_display_height;
    g_x_scale_text = 1.0 / g_x_scale;
    g_y_scale_text = 1.0 / g_y_scale;

    g_init = 0;

  }

  return PSGRAPH_SUCCESS;

}

int psgraph_set_paper_size_inches (
  float len,
  float ht
) {
  
  g_paper_len = len;
  g_paper_ht = ht;
  g_x_scale = g_paper_len / POINT_SIZE / g_display_width;
  g_y_scale = g_paper_ht / POINT_SIZE / g_display_height;
  g_x_scale_text = 1.0 / g_x_scale;
  g_y_scale_text = 1.0 / g_y_scale;

  return PSGRAPH_SUCCESS;

}

int psgraph_open_file (
  char *filename
) {

  g_pf = fopen( filename, "w" );

  if ( !g_pf ) return 0;

/* write header */

  fprintf( g_pf, "%%!PS-Adobe-2.0\n" );
  fprintf( g_pf, "%%%%Title:tmp.ps\n" );
  fprintf( g_pf, "%%%%Pages: 1\n" );
  fprintf( g_pf, "%%%%EndProlog\n" );
  fprintf( g_pf, "%%%%Page: 1 1\n" );

  fprintf( g_pf, "\n" );
  fprintf( g_pf, "/F {\n" );
  fprintf( g_pf, "  findfont exch scalefont setfont\n" );
  fprintf( g_pf, "} bind def\n" );
  fprintf( g_pf, "\n" );

  fprintf( g_pf, "/GSTART {\n" );
  fprintf( g_pf, "%-f %-f scale\n", g_x_scale, g_y_scale );
  fprintf( g_pf, "gsave\n" );
  fprintf( g_pf, "} bind def\n" );
  fprintf( g_pf, "\n" );

  fprintf( g_pf, "/GEND {\n" );
  fprintf( g_pf, "grestore\n" );
  fprintf( g_pf, "} bind def\n" );
  fprintf( g_pf, "\n" );

  fprintf( g_pf, "/CSHOW {\n" );
  fprintf( g_pf, "dup stringwidth exch neg 2 div exch\n" );
  fprintf( g_pf, "rmoveto show\n" );
  fprintf( g_pf, "} bind def\n" );
  fprintf( g_pf, "\n" );

  fprintf( g_pf, "/vcshowdict 4 dict def\n" );
  fprintf( g_pf, "/VCSHOW1 {\n" );
  fprintf( g_pf, "vcshowdict begin\n" );
  fprintf( g_pf, "/thestring exch def\n" );
  fprintf( g_pf, "/lineskip exch def\n" );
  fprintf( g_pf, "thestring {\n" );
  fprintf( g_pf, "/charcode exch def\n" );
  fprintf( g_pf, "/thechar () dup 0 charcode put def\n" );
  fprintf( g_pf, "0 lineskip neg rmoveto\n" );
  fprintf( g_pf, "gsave\n" );
  fprintf( g_pf, "thechar stringwidth pop 2 div neg 0 rmoveto\n" );
  fprintf( g_pf, "thechar show\n" );
  fprintf( g_pf, "grestore\n" );
  fprintf( g_pf, "}forall\n" );
  fprintf( g_pf, "end\n" );
  fprintf( g_pf, "} def\n" );

  fprintf( g_pf, "/VCSHOW {\n" );
  fprintf( g_pf, "dup stringwidth neg exch neg exch\n" );
  fprintf( g_pf, "rmoveto show pop\n" );
  fprintf( g_pf, "} bind def\n" );
  fprintf( g_pf, "\n" );

  fprintf( g_pf, "/RSHOW {\n" );
  fprintf( g_pf, "dup stringwidth neg exch neg exch\n" );
  fprintf( g_pf, "rmoveto show\n" );
  fprintf( g_pf, "} bind def\n" );
  fprintf( g_pf, "\n" );

  fprintf( g_pf, "%%%%EndPrologue\n" );
  fprintf( g_pf, "gsave\n" );
  fprintf(g_pf, "\n" );
  fprintf( g_pf, "1 setlinewidth\n" );
  fprintf( g_pf, "612 0 translate\n" );
  fprintf( g_pf, "90 rotate\n" );
  fprintf( g_pf, "%-f %-f scale\n", g_x_scale, g_y_scale );
  fprintf( g_pf, "gsave\n" );
  fprintf( g_pf, "\n" );

  return PSGRAPH_SUCCESS;

/* write text

x y moveto
12 /Helvetica F (test) show

   write line

GSTART
newpath
x0 y0 moveto
x1 y1 lineto
stroke
GEND

*/

}

int psgraph_close_file ( void ) {

  fprintf( g_pf, "showpage\n" );
  fprintf( g_pf, "\n" );
  fprintf( g_pf, "grestore\n" );
  fprintf( g_pf, "grestore\n" );
  fprintf( g_pf, "\%\%Trailor\n" );

  fclose( g_pf );

  return PSGRAPH_SUCCESS;

}

void PsGsave ( void ) {

  fprintf( g_pf, "gsave\n" );

}

void PsGrestore ( void ) {

  fprintf( g_pf, "grestore\n" );

}

void PsSetLineWidth (
  int width
) {

  g_saved_line_width = g_cur_line_width;
  g_cur_line_width = width;
  fprintf( g_pf, "%-d setlinewidth\n", width );

}

void PsRestoreLineWidth ( void ) {

  g_cur_line_width = g_saved_line_width;
  fprintf( g_pf, "%-d setlinewidth\n", g_cur_line_width );

}

void PsSetForeground (
  unsigned short r,
  unsigned short g,
  unsigned short b
) {

float fr, fg, fb;

  fr = (float) r / 65535.0;
  fg = (float) g / 65535.0;
  fb = (float) b / 65535.0;
  fprintf( g_pf, "%-f %-f %-f setrgbcolor\n", fr, fg, fb );

}

void PsSetFont (
  int font_size,
  char *font_name
) {

  g_font_size = font_size;
  strncpy( g_font_name, font_name, 128 );
  return;

  fprintf( g_pf, "gsave %-f %-f scale %-d (%s) F grestore\n",
   g_x_scale_text, g_y_scale_text, font_size, font_name );

}

void PsSetClipRectangle (
  float x,
  float y,
  float w,
  float h
) {

  fprintf( g_pf, "newpath %-f %-f moveto %-f 0.0 rlineto 0.0 %-f rlineto \
%-f neg 0.0 rlineto closepath clip\n", x, y, w, h, w );

}

void PsClipArea (
  AREA_ID area_id
) {

AREA_PRIV_PTR area_ptr;
float x, y, w, h;

  area_ptr = (AREA_PRIV_PTR) area_id;

  x = area_ptr->attr->gwin_ptr->attr->x + area_ptr->attr->x;
  y = area_ptr->attr->gwin_ptr->attr->y + area_ptr->attr->y;
  w = area_ptr->attr->w;
  h = area_ptr->attr->h;

  fprintf( g_pf, "%%! Set area clip rectangle\n" );
  PsSetClipRectangle( x, y, w, h );

}

void PsClipPlot (
  AREA_ID area_id
) {

AREA_PRIV_PTR area_ptr;
float x, y, w, h;

  area_ptr = (AREA_PRIV_PTR) area_id;

  x = area_ptr->attr->gwin_ptr->attr->x + area_ptr->attr->x +
   area_ptr->attr->plot_x;
  y = area_ptr->attr->gwin_ptr->attr->y + area_ptr->attr->y +
   area_ptr->attr->plot_y;
  w = area_ptr->attr->plot_w;
  h = area_ptr->attr->plot_h;

  fprintf( g_pf, "%%! Set plot clip rectangle\n" );
  PsSetClipRectangle( x, y, w, h );

}

void PsDrawLine (
  float x0,
  float y0,
  float x1,
  float y1
) {

  fprintf( g_pf, "newpath %-f %-f moveto %-f %-f lineto stroke\n",
   x0+g_x, y0+g_y, x1+g_x, y1+g_y );

}

void PsDrawLines (
  float *x,
  float *y,
  int n
) {

int i;

  fprintf( g_pf, "newpath " );
  fprintf( g_pf, "%-f %-f moveto\n", x[0]+g_x, y[0]+g_y );

  for ( i=1; i<n; i++ ) {
    fprintf( g_pf, "%-f %-f lineto\n", x[i]+g_x, y[i]+g_y );
  }

  fprintf( g_pf, "stroke\n" );

}

void PsFillPolygon (
  float *x,
  float *y,
  int n
) {

int i;

  fprintf( g_pf, "newpath " );
  fprintf( g_pf, "%-f %-f moveto\n", x[0]+g_x, y[0]+g_y );

  for ( i=1; i<n; i++ ) {
    fprintf( g_pf, "%-f %-f lineto\n", x[i]+g_x, y[i]+g_y );
  }

  fprintf( g_pf, "closepath fill\n" );

}

void PsFillArcFromCenter (
  float x,
  float y,
  float diameter,
  float start_angle,
  float end_angle
) {

  fprintf( g_pf, "newpath %-f %-f %-f %-f %-f arc closepath fill\n",
   x+g_x, y+g_y, diameter/2.0, start_angle, end_angle );

}

void PsDrawArcFromCenter (
  float x,
  float y,
  float diameter,
  float start_angle,
  float end_angle
) {

  fprintf( g_pf, "newpath %-f %-f %-f %-f %-f arc stroke\n",
   x+g_x, y+g_y, diameter/2.0, start_angle, end_angle );

}

void PsFillRectangleFromCenter (
  float x,
  float y,
  float width,
  float height
) {

  fprintf( g_pf, "newpath %-f %-f moveto %-f %-f rmoveto 0 %-f rlineto \
   %-f 0 rlineto 0 %-f neg rlineto %-f neg 0 rlineto closepath fill\n",
   x+g_x, y+g_y, width*-0.5, height*-0.5, height, width, height, width );

}

void PsDrawRectangleFromCenter (
  float x,
  float y,
  float width,
  float height
) {

  fprintf( g_pf, "newpath %-f %-f moveto %-f %-f rmoveto 0 %-f rlineto \
   %-f 0 rlineto 0 %-f neg rlineto %-f neg 0 rlineto stroke\n",
   x+g_x, y+g_y, width*-0.5, height*-0.5, height, width, height, width );

}

void PsFillRectangle (
  float x,
  float y,
  float width,
  float height
) {

  fprintf( g_pf, "newpath %-f %-f moveto 0 %-f rlineto \
   %-f 0 rlineto 0 %-f neg rlineto %-f neg 0 rlineto closepath fill\n",
   x+g_x, y+g_y, height, width, height, width );

}

void PsDrawRectangle (
  float x,
  float y,
  float width,
  float height
) {

  fprintf( g_pf, "newpath %-f %-f moveto 0 %-f rlineto \
   %-f 0 rlineto 0 %-f neg rlineto %-f neg 0 rlineto stroke\n",
   x+g_x, y+g_y, height, width, height, width );

}

void PsDrawString (
  float x,
  float y,
  char *string,
  int size
) {

  fprintf( g_pf, "newpath %-f %-f moveto gsave %-f %-f scale %-d (%s) F (%s) show grestore\n",
   x+g_x, y+g_y, g_x_scale_text, g_y_scale_text, g_font_size, g_font_name, string );

}

void PsDrawCenteredString (
  float x,
  float y,
  char *string,
  int size
) {

  fprintf( g_pf, "newpath %-f %-f moveto gsave %-f %-f scale %-d (%s) F (%s) CSHOW grestore\n",
   x+g_x, y+g_y, g_x_scale_text, g_y_scale_text, g_font_size, g_font_name, string );

}

void PsDrawCenteredStringUnder (
  float x,
  float y,
  char *string,
  int size
) {

  fprintf( g_pf, "newpath %-f %-f moveto gsave %-f %-f scale \
%-d (%s) F 0 %-d neg rmoveto (%s) CSHOW grestore\n",
   x+g_x, y+g_y, g_x_scale_text, g_y_scale_text, g_font_size, g_font_name,
   g_font_size, string );

}

void PsDrawCenteredStringOver (
  float x,
  float y,
  char *string,
  int size
) {

  fprintf( g_pf, "newpath %-f %-f moveto gsave %-f %-f scale \
%-d (%s) F 0 %-d rmoveto (%s) CSHOW grestore\n",
   x+g_x, y+g_y, g_x_scale_text, g_y_scale_text, g_font_size, g_font_name,
   g_font_size, string );

}

void PsDrawVertRotCenteredString (
  float x,
  float y,
  char *string,
  int size
) {

  fprintf( g_pf, "gsave %-f %-f translate 90 rotate 0 0 moveto \
%-f %-f scale %-d (%s) F 0 %-d neg rmoveto (%s) CSHOW grestore\n",
   x+g_x, y+g_y, g_x_scale_text, g_y_scale_text, g_font_size, g_font_name,
   g_font_size, string );

}

void PsDrawVertRotCenteredStringDown (
  float x,
  float y,
  char *string,
  int size
) {

  fprintf( g_pf, "gsave %-f %-f translate 90 rotate 0 0 moveto \
%-f %-f scale %-d (%s) F 0 %d neg rmoveto (%s) CSHOW grestore\n",
   x+g_x, y+g_y, g_x_scale_text, g_y_scale_text, g_font_size, g_font_name,
   g_font_size, string );

}

void PsDrawVertCenteredString (
  float x,
  float y,
  char *string,
  int size
) {

  fprintf( g_pf, "newpath %-f %-f moveto gsave %-f %-f scale %-d (%s) F %-d (%s) VCSHOW grestore\n",
   x+g_x, y+g_y, g_x_scale_text, g_y_scale_text, g_font_size, g_font_name, g_font_size, string );

}

void PsDrawRightJustString (
  float x,
  float y,
  char *string,
  int size
) {

  fprintf( g_pf, "newpath %-f %-f moveto gsave %-f %-f scale %-d (%s) F (%s) RSHOW grestore\n",
   x+g_x, y+g_y, g_x_scale_text, g_y_scale_text, g_font_size, g_font_name, string );

}

void PsDrawRightJustStringDown (
  float x,
  float y,
  char *string,
  int size
) {

  fprintf( g_pf, "newpath %-f %-f moveto gsave %-f %-f scale %-d (%s) F \
0 %-d 2 div neg rmoveto (%s) RSHOW grestore\n",
   x+g_x, y+g_y, g_x_scale_text, g_y_scale_text, g_font_size, g_font_name,
   g_font_size, string );

}

void PsDrawRightJustStringUp (
  float x,
  float y,
  char *string,
  int size
) {

  fprintf( g_pf, "newpath %-f %-f moveto gsave %-f %-f scale %-d (%s) F \
0 %-d rmoveto (%s) RSHOW grestore\n",
   x+g_x, y+g_y, g_x_scale_text, g_y_scale_text, g_font_size, g_font_name,
   g_font_size, string );

}

int psgraph_create_window (
  double origin_x,       /* percent */
  double origin_y,       /* percent */
  double width,          /* percent */
  double height,         /* percent */
  GWIN_ID *id
) {

GWIN_PRIV_ATTR_PTR attr_ptr;
GWIN_PRIV_PTR gwin_ptr;
int stat, ret_stat;
WIN_LIST_PTR cur_win;

  /* allocation graph window object */
  gwin_ptr = (GWIN_PRIV_PTR) calloc( 1, sizeof(GWIN_PRIV_TYPE) );
  if ( !gwin_ptr ) return PSGRAPH_NO_MEM;

  /* allocate attributes ptr */
  attr_ptr = (GWIN_PRIV_ATTR_PTR) calloc( 1, sizeof(GWIN_PRIV_ATTR_TYPE) );
  if ( !attr_ptr ) {
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }

  attr_ptr->x = origin_x * 0.01 * g_display_width-1;
  attr_ptr->y = origin_y * 0.01 * g_display_height-1;
  attr_ptr->w = width * 0.01 * g_display_width;
  attr_ptr->h = height * 0.01 * g_display_height;

  attr_ptr->origin_x = origin_x;
  attr_ptr->origin_y = origin_y;
  attr_ptr->height = height;
  attr_ptr->width = width;

  gwin_ptr->id_type = GRAPH_K_GWIN_ID;
  gwin_ptr->obj_head = NULL;
  gwin_ptr->obj_tail = NULL;
  gwin_ptr->attr = attr_ptr;

  /* create sentinel nodes for object list */
  gwin_ptr->obj_head = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
  if ( !gwin_ptr ) {
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }

  gwin_ptr->obj_tail = gwin_ptr->obj_head;
  gwin_ptr->obj_tail->next = NULL;

  cur_win = (WIN_LIST_PTR) calloc( 1, sizeof(WIN_LIST_TYPE) );
  if ( !cur_win ) {
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }
  cur_win->id = (void *) gwin_ptr;
  cur_win->id_type = GRAPH_K_GWIN_ID;
  g_win_tail->flink = cur_win;
  g_win_tail = cur_win;
  g_win_tail->flink = NULL;

  *id = (GWIN_ID *) gwin_ptr;

norm_return:
  return PSGRAPH_SUCCESS;

err_return:
  *id = (GWIN_ID *) NULL;
  return ret_stat;

}

int psgraph_create_area (
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
  int title_font_size,
  char *title_font_name,
  int label_font_size,
  char *label_font_name,
  int area_border_size,
  AREA_ID *area_id
) {

AREA_PRIV_ATTR_PTR attr_ptr;
AREA_PRIV_PTR area_ptr;
GWIN_PRIV_PTR gwin_ptr;
OBJ_LIST_PTR cur_obj;
int stat, ret_stat, top_margin, bottom_margin, left_margin, right_margin;
double min, max;
WIN_LIST_PTR cur_win;

  if ( x_scale_type == PSGRAPH_SCALE_LOG ) {
    stat = psgraph___log_adjust_low_limit( &xmin );
    stat = psgraph___log_adjust_high_limit( &xmax );
  }

  if ( y_scale_type == PSGRAPH_SCALE_LOG ) {
    stat = psgraph___log_adjust_low_limit( &ymin );
    stat = psgraph___log_adjust_high_limit( &ymax );
  }

  gwin_ptr = (GWIN_PRIV_PTR) win_id;

  /* allocation area window object */
  area_ptr = (AREA_PRIV_PTR) calloc( 1, sizeof(AREA_PRIV_TYPE) );
  if ( !area_ptr ) return PSGRAPH_NO_MEM;

  if ( !gwin_ptr ) {
    ret_stat = PSGRAPH_BAD_OBJ;
    goto err_return;
  }
  if ( !gwin_ptr->id_type ) {
    ret_stat = PSGRAPH_BAD_OBJ;
    goto err_return;
  }

  /* allocate attributes ptr */
  attr_ptr = (AREA_PRIV_ATTR_PTR) calloc( 1, sizeof(AREA_PRIV_ATTR_TYPE) );
  if ( !attr_ptr ) {
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }

  /* init attributes */

  attr_ptr->area_border_size = area_border_size;

  attr_ptr->xlabel_grid = attr_ptr->xmajor_grid = attr_ptr->xminor_grid = 0;
  attr_ptr->ylabel_grid = attr_ptr->ymajor_grid = attr_ptr->yminor_grid = 0;

  attr_ptr->gwin_ptr = gwin_ptr;

  stat = psgraph___alloc_color( bg_color_name, g_colormap, &attr_ptr->bg_color );
  if ( stat != PSGRAPH_SUCCESS ) attr_ptr->bg_color.pixel = g_background;

  stat = psgraph___alloc_color( fg_color_name, g_colormap, &attr_ptr->fg_color );
  if ( stat != PSGRAPH_SUCCESS ) attr_ptr->fg_color.pixel = g_foreground;

  stat = psgraph___alloc_color( fg_color_name, g_colormap, &attr_ptr->grid_color );
  if ( stat != PSGRAPH_SUCCESS ) attr_ptr->grid_color.pixel = g_foreground;

  attr_ptr->title_font_name = (char *) malloc( strlen(title_font_name)+1 );
  strcpy( attr_ptr->title_font_name, title_font_name );

  attr_ptr->title_font_size = title_font_size;

  attr_ptr->label_font_name = (char *) malloc( strlen(label_font_name)+1 );
  strcpy( attr_ptr->label_font_name, label_font_name );

  attr_ptr->label_font_size = label_font_size;

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

  attr_ptr->x_scale_type = x_scale_type;
  attr_ptr->y_scale_type = y_scale_type;

  area_ptr->id_type = GRAPH_K_AREA_ID;
  area_ptr->obj_head = NULL;
  area_ptr->obj_tail = NULL;
  area_ptr->attr = attr_ptr;

  /* do plot region init */
  top_margin = ( attr_ptr->title_font_size + attr_ptr->label_font_size ) * 1.5 *
   POINT_SIZE / g_paper_ht * Y_RES;
  bottom_margin = attr_ptr->label_font_size * 6 * POINT_SIZE / g_paper_ht *
   Y_RES;
  left_margin = attr_ptr->label_font_size * 10 * POINT_SIZE / g_paper_len *
   X_RES;
  right_margin = 0.05 * attr_ptr->w;

  attr_ptr->plot_x = left_margin;
  attr_ptr->plot_y = bottom_margin;
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

  /* create sentinel nodes for object list */
  area_ptr->obj_head = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
  if ( !area_ptr ) {
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }

  area_ptr->obj_tail = area_ptr->obj_head;
  area_ptr->obj_tail->next = NULL;

  /* link area ptr into gwin list */
  cur_obj = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
  if ( !cur_obj ) {
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }

  cur_obj->ptr = (void *) area_ptr;
  gwin_ptr->obj_tail->next = cur_obj;
  gwin_ptr->obj_tail = cur_obj;
  gwin_ptr->obj_tail->next = NULL;

  /* add info to window list */

  cur_win = (WIN_LIST_PTR) calloc( 1, sizeof(WIN_LIST_TYPE) );
  if ( !cur_win ) {
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }
  cur_win->id = (void *) area_ptr;
  cur_win->id_type = GRAPH_K_AREA_ID;
  g_win_tail->flink = cur_win;
  g_win_tail = cur_win;
  g_win_tail->flink = NULL;

  cur_win = (WIN_LIST_PTR) calloc( 1, sizeof(WIN_LIST_TYPE) );
  if ( !cur_win ) {
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }
  cur_win->id = (void *) area_ptr;
  cur_win->id_type = GRAPH_K_PLOT_AREA_ID;
  g_win_tail->flink = cur_win;
  g_win_tail = cur_win;
  g_win_tail->flink = NULL;

  *area_id = (AREA_ID *) area_ptr;

norm_return:
  return PSGRAPH_SUCCESS;

err_return:
  area_ptr->id_type = 0;
  *area_id = (AREA_ID *) NULL;
  return ret_stat;

}

int psgraph_set_grid_color (
  AREA_ID id,
  char *color
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  stat = psgraph___alloc_color( color, g_colormap, &ptr->attr->grid_color );
  if ( stat != PSGRAPH_SUCCESS ) ptr->attr->grid_color.pixel = g_foreground;

}

int psgraph_x_label_grid (
  AREA_ID id,
  int on_flag
) {

AREA_PRIV_PTR ptr;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  ptr->attr->xlabel_grid = on_flag;

}

int psgraph_x_major_grid (
  AREA_ID id,
  int on_flag
) {

AREA_PRIV_PTR ptr;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  ptr->attr->xmajor_grid = on_flag;

}

int psgraph_x_minor_grid (
  AREA_ID id,
  int on_flag
) {

AREA_PRIV_PTR ptr;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  ptr->attr->xminor_grid = on_flag;

}

int psgraph_y_label_grid (
  AREA_ID id,
  int on_flag
) {

AREA_PRIV_PTR ptr;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  ptr->attr->ylabel_grid = on_flag;

}

int psgraph_y_major_grid (
  AREA_ID id,
  int on_flag
) {

AREA_PRIV_PTR ptr;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  ptr->attr->ymajor_grid = on_flag;

}

int psgraph_y_minor_grid (
  AREA_ID id,
  int on_flag
) {

AREA_PRIV_PTR ptr;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  ptr->attr->yminor_grid = on_flag;

}

void set_win_origin (
  GWIN_PRIV_PTR ptr
) {

  g_saved_x = g_x;
  g_saved_y = g_y;
  g_x = ptr->attr->x;
  g_y = ptr->attr->y;

}

void set_area_origin (
  AREA_PRIV_PTR ptr
) {

  g_saved_x = g_x;
  g_saved_y = g_y;
  g_x = ptr->attr->gwin_ptr->attr->x + ptr->attr->x;
  g_y = ptr->attr->gwin_ptr->attr->y + ptr->attr->y;

}

void set_plot_origin (
  AREA_PRIV_PTR ptr
) {

  g_saved_x = g_x;
  g_saved_y = g_y;
  g_x = ptr->attr->gwin_ptr->attr->x + ptr->attr->x +
   ptr->attr->plot_x;

  g_y = ptr->attr->gwin_ptr->attr->y + ptr->attr->y +
   ptr->attr->plot_y;

}

void set_prev_origin ( void ) {

  g_x = g_saved_x;
  g_y = g_saved_y;

}

int psgraph_draw_area_border (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  if ( ptr->attr->area_border_size > 0 ) {

    fprintf( g_pf, "%%! Area Border\n" );

    set_area_origin( ptr );
    PsSetLineWidth( ptr->attr->area_border_size );
    PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
     ptr->attr->fg_color.blue );

    PsDrawRectangle( 0.0, 0.0, ptr->attr->w, ptr->attr->h );

    PsRestoreLineWidth();
    set_prev_origin();

  }

}

int psgraph_draw_plot_border (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  fprintf( g_pf, "%%! Plot Border\n" );

  set_plot_origin( ptr );
  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  PsDrawRectangle( 0.0, 0.0, ptr->attr->plot_w, ptr->attr->plot_h );

  set_prev_origin();

}

int psgraph_draw_title (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, x0, y0, x1, y1, lastx;
int major_tick_height, minor_tick_height;
int scale_len;
double dx, inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  set_area_origin( ptr );

  scale_len = ptr->attr->plot_w;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  PsSetFont( ptr->attr->title_font_size, ptr->attr->title_font_name );

  x0 = ptr->attr->plot_x - 1;
  x1 = x0 + scale_len;
  x = ( x0 + x1 ) * 0.5;	/* mdipoint */

  /* draw title */
  y0 = ptr->attr->h;

  PsDrawCenteredStringUnder( x, y0, ptr->attr->title, strlen(ptr->attr->title) );

  return PSGRAPH_SUCCESS;

}

int psgraph_title (
  AREA_ID id,
  char *title
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  set_area_origin( ptr );

  strncpy( ptr->attr->title, title, TITLE_MAX_CHARS );
  ptr->attr->title[TITLE_MAX_CHARS] = 0;

  return stat;

}

int psgraph_draw_x_linear_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, x0, y0, x1, y1, lastx;
int label_tick_height, major_tick_height, minor_tick_height;
int count, at_end, scale_len;
double dx, inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  /* draw axis */
  x0 = ptr->attr->plot_x;
  y0 = ptr->attr->plot_y;
  x1 = x0 + scale_len;
  y1 = y0;
  PsDrawLine( x0, y0, x1, y1 );

  lastx = x1;

  label_tick_height = g_display_height / 100.0;
  major_tick_height = 0.8 * label_tick_height;
  minor_tick_height = major_tick_height * 0.5;

  /* draw label ticks */

  if ( ptr->attr->x_tick_label_inc > 0.0 ) {

    x0 = ptr->attr->plot_x;
    x1 = x0;

    dx = ptr->attr->xmin;
    inc = ptr->attr->x_tick_label_inc;

    count = 0;
    at_end = 0;
    while ( dx < ( ptr->attr->xmax + 0.5 * inc ) ) {

      y0 = ptr->attr->plot_y;
      y1 = y0 - label_tick_height;

      PsDrawLine( x0, y0, x1, y1 );

      if ( ( dx + inc ) > ( 1.001 * ptr->attr->xmax ) )
        at_end = 1;

      if ( ptr->attr->xlabel_grid ) {
        y0 = ptr->attr->plot_y + ptr->attr->plot_h;
        y1 = ptr->attr->plot_y;
        if ( count++ && !at_end )
          PsSetForeground( ptr->attr->grid_color.red,
           ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
        PsDrawLine( x0, y0, x1, y1 );
        PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
         ptr->attr->fg_color.blue );
      }

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
       ptr->attr->plot_x;

      if ( x0 > ( ptr->attr->plot_x + ptr->attr->plot_w ) )
        x0 = x1 = ptr->attr->plot_x + ptr->attr->plot_w;

    }

  }

  /* draw major ticks */

  if ( ptr->attr->x_major_tick_inc > 0.0 ) {

    PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
     ptr->attr->fg_color.blue );

    x0 = ptr->attr->plot_x;
    x1 = x0;

    dx = ptr->attr->xmin;
    inc = ptr->attr->x_major_tick_inc;

    count = 0;
    at_end = 0;
    while ( dx < ( ptr->attr->xmax + 0.5 * inc ) ) {

      y0 = ptr->attr->plot_y;
      y1 = y0 - major_tick_height;

      PsDrawLine( x0, y0, x1, y1 );

      if ( ( dx + inc ) > ( 1.001 * ptr->attr->xmax ) )
        at_end = 1;

      if ( ptr->attr->xmajor_grid ) {
        y0 = ptr->attr->plot_y + ptr->attr->plot_h;
        y1 = ptr->attr->plot_y;
        if ( count++ && !at_end )
          PsSetForeground( ptr->attr->grid_color.red,
           ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
        PsDrawLine( x0, y0, x1, y1 );
        PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
         ptr->attr->fg_color.blue );
      }

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
       ptr->attr->plot_x;

      if ( x0 > ( ptr->attr->plot_x + ptr->attr->plot_w ) )
        x0 = x1 = ptr->attr->plot_x + ptr->attr->plot_w;

    }

  }

  /* draw minor ticks */
  if ( ptr->attr->x_minor_tick_inc > 0.0 ) {

    PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
     ptr->attr->fg_color.blue );

    x0 = ptr->attr->plot_x;
    x1 = x0;

    dx = ptr->attr->xmin;
    inc = ptr->attr->x_minor_tick_inc;

    count = 0;
    at_end = 0;
    while ( dx < ( ptr->attr->xmax + 0.5 * inc ) ) {

      y0 = ptr->attr->plot_y;
      y1 = y0 - minor_tick_height;

      PsDrawLine( x0, y0, x1, y1 );

      if ( ( dx + inc ) > ( 1.001 * ptr->attr->xmax ) )
        at_end = 1;

      if ( ptr->attr->xminor_grid ) {
        y0 = ptr->attr->plot_y + ptr->attr->plot_h;
        y1 = ptr->attr->plot_y;
        if ( count++ && !at_end )
          PsSetForeground( ptr->attr->grid_color.red,
           ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
        PsDrawLine( x0, y0, x1, y1 );
        PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
         ptr->attr->fg_color.blue );
      }

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
       ptr->attr->plot_x;

      if ( x0 > ( ptr->attr->plot_x + ptr->attr->plot_w ) )
        x0 = x1 = ptr->attr->plot_x + ptr->attr->plot_w;

    }

  }

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  return PSGRAPH_SUCCESS;

}

int psgraph_draw_x_log_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, x0, y0, x1, y1, lastx;
int label_tick_height, major_tick_height, minor_tick_height;
int count, at_end, scale_len;
double dx, inc, n, logval;
double decade_tick_inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  /* draw axis */

  if ( ptr->attr->x_tick_label_inc == 0.1 )
    decade_tick_inc = 1.0;
  else
    decade_tick_inc = ptr->attr->x_tick_label_inc;

  x0 = ptr->attr->plot_x;
  y0 = ptr->attr->plot_y;
  x1 = x0 + scale_len;
  y1 = y0;
  PsDrawLine( x0, y0, x1, y1 );

  lastx = x1;

  label_tick_height = g_display_height / 100.0;
  major_tick_height = 0.8 * label_tick_height;
  minor_tick_height = major_tick_height * 0.5;

  /* draw label ticks */

  if ( decade_tick_inc ) {

    x0 = ptr->attr->plot_x;
    x1 = x0;

    dx = ptr->attr->xmin;
    inc = decade_tick_inc;

    count = 0;
    at_end = 0;
    while ( dx < ( ptr->attr->xmax + 0.5 * inc ) ) {

      y0 = ptr->attr->plot_y;
      y1 = y0 - label_tick_height;

      PsDrawLine( x0, y0, x1, y1 );

      if ( ( dx + inc ) > ( 1.001 * ptr->attr->xmax ) )
        at_end = 1;

      if ( ptr->attr->xlabel_grid ) {
        y0 = ptr->attr->plot_y + ptr->attr->plot_h;
        y1 = ptr->attr->plot_y;
        if ( count++ && !at_end )
          PsSetForeground( ptr->attr->grid_color.red,
           ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
        PsDrawLine( x0, y0, x1, y1 );
        PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
         ptr->attr->fg_color.blue );
      }

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
       ptr->attr->plot_x;

      if ( x0 > ( ptr->attr->plot_x + ptr->attr->plot_w ) )
        x0 = x1 = ptr->attr->plot_x + ptr->attr->plot_w;

    }

  }

  /* draw major ticks */

  if ( ptr->attr->x_major_tick_inc > 0.0 ) {

    PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
     ptr->attr->fg_color.blue );

    x0 = ptr->attr->plot_x;
    x1 = x0;

    dx = ptr->attr->xmin;
    inc = ptr->attr->x_major_tick_inc;

    count = 0;
    at_end = 0;
    while ( dx < ( ptr->attr->xmax + 0.5 * inc ) ) {

      y0 = ptr->attr->plot_y;
      y1 = y0 - major_tick_height;

      PsDrawLine( x0, y0, x1, y1 );

      if ( ( dx + inc ) > ( 1.001 * ptr->attr->xmax ) )
        at_end = 1;

      if ( ptr->attr->xmajor_grid || ptr->attr->xminor_grid ) {
        y0 = ptr->attr->plot_y + ptr->attr->plot_h;
        y1 = ptr->attr->plot_y;
        if ( count++ && !at_end )
          PsSetForeground( ptr->attr->grid_color.red,
           ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
        PsDrawLine( x0, y0, x1, y1 );
        PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
         ptr->attr->fg_color.blue );
      }

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) + 
       ptr->attr->plot_x;

      if ( x0 > ( ptr->attr->plot_x + ptr->attr->plot_w ) )
        x0 = x1 = ptr->attr->plot_x + ptr->attr->plot_w;

    }

  }

  /* draw minor ticks (within each decade) */
  if ( ptr->attr->x_minor_tick_inc > 0.0 ) {

    PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
     ptr->attr->fg_color.blue );

    x0 = ptr->attr->plot_x;
    x1 = x0;

    count = 0;
    for ( n=ptr->attr->xmin; n<ptr->attr->xmax; n+=1.0 ) {

      dx = 1.0;
      inc = ptr->attr->x_minor_tick_inc;

      while ( dx < 9.5 ) {

        y0 = ptr->attr->plot_y;
        y1 = y0 - minor_tick_height;

        PsDrawLine( x0, y0, x1, y1 );

        if ( ptr->attr->xminor_grid ) {
          y0 = ptr->attr->plot_y + ptr->attr->plot_h;
          y1 = ptr->attr->plot_y;
          if ( count++ )
            PsSetForeground( ptr->attr->grid_color.red,
             ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
          PsDrawLine( x0, y0, x1, y1 );
          PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
           ptr->attr->fg_color.blue );
        }

        dx += inc;
        logval = log10(dx) + n;
        x0 = x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
         ptr->attr->plot_x;

        if ( x0 > ( ptr->attr->plot_x + ptr->attr->plot_w ) )
          x0 = x1 = ptr->attr->plot_x + ptr->attr->plot_w;

      }

    }

  }

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  return PSGRAPH_SUCCESS;

}

int psgraph_draw_x_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( ptr->attr->x_scale_type == PSGRAPH_SCALE_LINEAR )
    stat = psgraph_draw_x_linear_scale( id );
  else
    stat = psgraph_draw_x_log_scale( id );

  return stat;

}

int psgraph_x_scale (
  AREA_ID id,
  double major_tick_inc,
  double minor_tick_inc,
  double tick_label_inc	/* label every increment */
) {

AREA_PRIV_PTR ptr;
int stat;
double inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  set_area_origin( ptr );

  if ( ptr->attr->x_scale_type == PSGRAPH_SCALE_LOG ) {

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

  stat = psgraph_draw_x_scale( id );

  return stat;

}

int psgraph_draw_x_lin_annotation (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, x0, y0, x1, y1, lastx;
int label_tick_height;
int scale_len;
double dx, inc;
char value[127+1];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  PsSetFont( ptr->attr->label_font_size, ptr->attr->label_font_name );

  x0 = ptr->attr->plot_x;
  x1 = x0 + scale_len;
  lastx = x1;

  /* draw annotation */

  if ( ptr->attr->x_tick_label_inc > 0.0 ) {

    label_tick_height = g_display_height / 100.0;
    x0 = ptr->attr->plot_x;
    x1 = x0;
    y0 = ptr->attr->plot_y;
    y1 = y0 - 1.5 * label_tick_height;

    dx = ptr->attr->xmin;
    inc = ptr->attr->x_tick_label_inc;

    while ( dx < ( ptr->attr->xmax + 0.5 * inc ) ) {

      sprintf( value, ptr->attr->xformat, dx );
      psgraph___discard_trailing_blanks( value );

      PsDrawCenteredStringUnder( x0, y1, value, strlen(value) );

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
       ptr->attr->plot_x;

    }

  }

  return PSGRAPH_SUCCESS;

}

int psgraph_draw_x_log_annotation (
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

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  PsSetFont( ptr->attr->label_font_size, ptr->attr->label_font_name );

  x0 = ptr->attr->plot_x;
  x1 = x0 + scale_len;
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

    label_tick_height = g_display_height / 100.0;
    x0 = ptr->attr->plot_x;
    x1 = x0;
    y0 = ptr->attr->plot_y;
    y1 = y0 - 1.5 * label_tick_height;

    dx = ptr->attr->xmin;
    inc = decade_tick_inc;

    while ( dx < ( ptr->attr->xmax + 0.5 * inc ) ) {

      if ( dx < 0.0 )
        logval = pow( 10.0, dx );
      else
        logval = (double) ( (int) ( pow( 10.0, dx ) + 0.5 ) );
      sprintf( value, ptr->attr->xformat, logval );
      psgraph___discard_trailing_blanks( value );

      PsDrawCenteredStringUnder( x0, y1, value, strlen(value) );

      dx += inc;
      x0 = x1 = (int) ( dx * ptr->attr->xfact + ptr->attr->xofs + 0.5 ) +
       ptr->attr->plot_x;

    }

  }

  /* label minor ticks (within each decade) */
  if ( ( inner_tick_inc == 1.0 ) && ( ptr->attr->x_minor_tick_inc > 0.0 ) ) {

    label_tick_height = g_display_height / 100.0;

    for ( n=ptr->attr->xmin; n<ptr->attr->xmax; n+=1.0 ) {

      x0 = ptr->attr->plot_x;
      x1 = x0;
      y0 = ptr->attr->plot_y;
      y1 = y0 - 1.5 * label_tick_height;

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
        psgraph___discard_trailing_blanks( value );

        PsDrawCenteredStringUnder( x0, y1, value, strlen(value) );

      }

    }

  }

  return PSGRAPH_SUCCESS;

}

int psgraph_draw_x_annotation (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( ptr->attr->x_scale_type == PSGRAPH_SCALE_LINEAR )
    stat = psgraph_draw_x_lin_annotation( id );
  else
    stat = psgraph_draw_x_log_annotation( id );

  return stat;

}

int psgraph_x_annotation (
  AREA_ID id,
  char *format
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  set_area_origin( ptr );

  strncpy( ptr->attr->xformat, format, FORMAT_MAX_CHARS );
  ptr->attr->xformat[FORMAT_MAX_CHARS] = 0;

  stat = psgraph_draw_x_annotation( id );

  return stat;

}

int psgraph_draw_x_label (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, x0, y0, x1, y1, lastx;
int scale_len;
double dx, inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_w;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  PsSetFont( ptr->attr->label_font_size, ptr->attr->label_font_name );

  x0 = ptr->attr->plot_x;
  x1 = x0 + scale_len;

  /* draw label */
  y0 = 0;

  x = ( x0 + x1 ) * 0.5;	/* mdipoint */

  PsDrawCenteredStringOver( x, y0, ptr->attr->xlabel, strlen(ptr->attr->xlabel) );

  return PSGRAPH_SUCCESS;

}

int psgraph_x_label (
  AREA_ID id,
  char *label
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  set_area_origin( ptr );

  strncpy( ptr->attr->xlabel, label, LABEL_MAX_CHARS );
  ptr->attr->xlabel[LABEL_MAX_CHARS] = 0;

  stat = psgraph_draw_x_label( id );

  return stat;

}

int psgraph_draw_y_linear_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, x0, y0, x1, y1, lasty;
int label_tick_width, major_tick_width, minor_tick_width;
int count, at_end, scale_len;
double dy, inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  /* draw axis */
  x0 = ptr->attr->plot_x;
  y0 = ptr->attr->plot_y;
  x1 = x0;
  y1 = y0 + scale_len;
  PsDrawLine( x0, y0, x1, y1 );

  lasty = y1;

  label_tick_width = g_display_width / 100.0;
  major_tick_width = 0.8 * label_tick_width;
  minor_tick_width = major_tick_width * 0.5;

  /* draw label ticks */

  if ( ptr->attr->y_tick_label_inc > 0.0 ) {

    y0 = ptr->attr->plot_y;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = ptr->attr->y_tick_label_inc;

    count = 0;
    at_end = 0;
    while ( dy < ( ptr->attr->ymax + 0.5 * inc ) ) {

      x0 = ptr->attr->plot_x;
      x1 = x0 - label_tick_width;

      PsDrawLine( x0, y0, x1, y1 );

      if ( ( dy + inc ) > ( 1.001 * ptr->attr->ymax ) )
        at_end = 1;

      if ( ptr->attr->ylabel_grid ) {
        x0 = ptr->attr->plot_x + ptr->attr->plot_w;
        x1 = ptr->attr->plot_x;
        if ( count++ && !at_end )
          PsSetForeground( ptr->attr->grid_color.red,
           ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
        PsDrawLine( x0, y0, x1, y1 );
        PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
         ptr->attr->fg_color.blue );
      }

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 ) +
       ptr->attr->plot_y;

      if ( y0 > ( ptr->attr->plot_x + ptr->attr->plot_h ) )
        y0 = y1 = ptr->attr->plot_x + ptr->attr->plot_h;

    }

  }

  /* draw major ticks */

  if ( ptr->attr->y_major_tick_inc > 0.0 ) {

    PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
     ptr->attr->fg_color.blue );

    y0 = ptr->attr->plot_y;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = ptr->attr->y_major_tick_inc;

    count = 0;
    at_end = 0;
    while ( dy < ( ptr->attr->ymax + 0.5 * inc ) ) {

      x0 = ptr->attr->plot_x;
      x1 = x0 - major_tick_width;

      PsDrawLine( x0, y0, x1, y1 );

      if ( ( dy + inc ) > ( 1.001 * ptr->attr->ymax ) )
        at_end = 1;

      if ( ptr->attr->ymajor_grid ) {
        x0 = ptr->attr->plot_x + ptr->attr->plot_w;
        x1 = ptr->attr->plot_x;
        if ( count++ && !at_end )
          PsSetForeground( ptr->attr->grid_color.red,
           ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
        PsDrawLine( x0, y0, x1, y1 );
        PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
         ptr->attr->fg_color.blue );
      }

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 ) +
       ptr->attr->plot_y;

      if ( y0 > ( ptr->attr->plot_x + ptr->attr->plot_h ) )
        y0 = y1 = ptr->attr->plot_x + ptr->attr->plot_h;

    }

  }

  /* draw minor ticks */
  if ( ptr->attr->y_minor_tick_inc > 0.0 ) {

    PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
     ptr->attr->fg_color.blue );

    y0 = ptr->attr->plot_y;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = ptr->attr->y_minor_tick_inc;

    count = 0;
    at_end = 0;
    while ( dy < ( ptr->attr->ymax + 0.5 * inc ) ) {

      x0 = ptr->attr->plot_x;
      x1 = x0 - minor_tick_width;

      PsDrawLine( x0, y0, x1, y1 );

      if ( ( dy + inc ) > ( 1.001 * ptr->attr->ymax ) )
        at_end = 1;

      if ( ptr->attr->yminor_grid ) {
        x0 = ptr->attr->plot_x + ptr->attr->plot_w;
        x1 = ptr->attr->plot_x;
        if ( count++ && !at_end )
          PsSetForeground( ptr->attr->grid_color.red,
           ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
        PsDrawLine( x0, y0, x1, y1 );
        PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
         ptr->attr->fg_color.blue );
      }

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 ) +
       ptr->attr->plot_y;

      if ( y0 > ( ptr->attr->plot_x + ptr->attr->plot_h ) )
        y0 = y1 = ptr->attr->plot_x + ptr->attr->plot_h;

    }

  }

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  return PSGRAPH_SUCCESS;

}

int psgraph_draw_y_log_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, x0, y0, x1, y1, lasty;
int label_tick_width, major_tick_width, minor_tick_width;
int count, at_end, scale_len;
double n, dy, inc, logval;
double decade_tick_inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  fprintf( g_pf, "%%!psgraph_draw_y_log_scale - begin\n" );

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  /* draw axis */

  if ( ptr->attr->x_tick_label_inc == 0.1 )
    decade_tick_inc = 1.0;
  else
    decade_tick_inc = ptr->attr->x_tick_label_inc;

  x0 = ptr->attr->plot_x;
  y0 = ptr->attr->plot_y;
  x1 = x0;
  y1 = y0 + scale_len;
  PsDrawLine( x0, y0, x1, y1 );

  lasty = y1;

  label_tick_width = g_display_width / 100.0;
  major_tick_width = 0.8 * label_tick_width;
  minor_tick_width = major_tick_width * 0.5;

  /* draw label ticks */

  if ( decade_tick_inc > 0.0 ) {

    y0 = ptr->attr->plot_y;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = decade_tick_inc;

    count = 0;
    at_end = 0;
    while ( dy < ( ptr->attr->ymax + 0.5 * inc ) ) {

      x0 = ptr->attr->plot_x;
      x1 = x0 - label_tick_width;

      fprintf( g_pf, "%%!psgraph_draw_y_log_scale - label tick\n" );
      PsDrawLine( x0, y0, x1, y1 );

      if ( ( dy + inc ) > ( 1.001 * ptr->attr->ymax ) )
        at_end = 1;

      if ( ptr->attr->ylabel_grid ) {
        x0 = ptr->attr->plot_x + ptr->attr->plot_w;
        x1 = ptr->attr->plot_x;
        if ( count++ && !at_end )
          PsSetForeground( ptr->attr->grid_color.red,
           ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
        fprintf( g_pf, "%%!psgraph_draw_y_log_scale - label grid line\n" );
        PsDrawLine( x0, y0, x1, y1 );
        PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
         ptr->attr->fg_color.blue );
      }

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 ) +
       ptr->attr->plot_y;

      if ( y0 > ( ptr->attr->plot_x + ptr->attr->plot_h ) )
        y0 = y1 = ptr->attr->plot_x + ptr->attr->plot_h;

    }

  }

  /* draw major ticks */

  if ( ptr->attr->y_major_tick_inc > 0.0 ) {

    PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
     ptr->attr->fg_color.blue );

    y0 = ptr->attr->plot_y;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = ptr->attr->y_major_tick_inc;

    count = 0;
    at_end = 0;
    while ( dy < ( ptr->attr->ymax + 0.5 * inc ) ) {

      x0 = ptr->attr->plot_x;
      x1 = x0 - major_tick_width;

      fprintf( g_pf, "%%!psgraph_draw_y_log_scale - major tick\n" );
      PsDrawLine( x0, y0, x1, y1 );

      if ( ( dy + inc ) > ( 1.001 * ptr->attr->ymax ) )
        at_end = 1;

      if ( ptr->attr->ymajor_grid || ptr->attr->yminor_grid ) {
        x0 = ptr->attr->plot_x + ptr->attr->plot_w;
        x1 = ptr->attr->plot_x;
        if ( count++ && !at_end )
          PsSetForeground( ptr->attr->grid_color.red,
           ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
        fprintf( g_pf, "%%!psgraph_draw_y_log_scale - major grid line\n" );
        PsDrawLine( x0, y0, x1, y1 );
        PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
         ptr->attr->fg_color.blue );
      }

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 ) +
       ptr->attr->plot_y;

      if ( y0 > ( ptr->attr->plot_x + ptr->attr->plot_h ) )
        y0 = y1 = ptr->attr->plot_x + ptr->attr->plot_h;

    }

  }

  /* draw minor ticks (within each decade) */
  if ( ptr->attr->y_minor_tick_inc > 0.0 ) {

    PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
     ptr->attr->fg_color.blue );

    y0 = ptr->attr->plot_y;
    y1 = y0;

    count = 0;
    for ( n=ptr->attr->ymin; n<ptr->attr->ymax; n+=1.0 ) {

      dy = 1.0;
      inc = ptr->attr->y_minor_tick_inc;

      while ( dy < 9.5 ) {

        x0 = ptr->attr->plot_x;
        x1 = x0 - minor_tick_width;

        fprintf( g_pf, "%%!psgraph_draw_y_log_scale - minor tick\n" );
        PsDrawLine( x0, y0, x1, y1 );

        if ( ptr->attr->yminor_grid ) {
          x0 = ptr->attr->plot_x + ptr->attr->plot_w;
          x1 = ptr->attr->plot_x;
          if ( count++ )
            PsSetForeground( ptr->attr->grid_color.red,
             ptr->attr->grid_color.green, ptr->attr->grid_color.blue );
          fprintf( g_pf, "%%!psgraph_draw_y_log_scale - minor grid line\n" );
          PsDrawLine( x0, y0, x1, y1 );
          PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
           ptr->attr->fg_color.blue );
        }

        dy += inc;
        logval = log10(dy) + n;
        y0 = y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 ) +
         ptr->attr->plot_y;

        if ( y0 > ( ptr->attr->plot_x + ptr->attr->plot_h ) )
          y0 = y1 = ptr->attr->plot_x + ptr->attr->plot_h;

      }

    }

  }

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  fprintf( g_pf, "%%!psgraph_draw_y_log_scale - end\n" );


  return PSGRAPH_SUCCESS;

}

int psgraph_draw_y_scale (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( ptr->attr->y_scale_type == PSGRAPH_SCALE_LINEAR )
    stat = psgraph_draw_y_linear_scale( id );
  else
    stat = psgraph_draw_y_log_scale( id );

  return stat;

}

int psgraph_y_scale (
  AREA_ID id,
  double major_tick_inc,
  double minor_tick_inc,
  double tick_label_inc /* label every increment */
) {

AREA_PRIV_PTR ptr;
int stat;
double inc;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  set_area_origin( ptr );

  if ( ptr->attr->y_scale_type == PSGRAPH_SCALE_LOG ) {

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

  stat = psgraph_draw_y_scale( id );

  return stat;

}

int psgraph_draw_y_lin_annotation (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, y, x0, y0, x1, y1, lasty;
int label_tick_width;
int scale_len;
double dy, inc;
char value[31+1];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  PsSetFont( ptr->attr->label_font_size, ptr->attr->label_font_name );

  y0 = ptr->attr->plot_y;
  y1 = y0 + scale_len;
  lasty = y1;

  /* draw annotation */

  if ( ptr->attr->y_tick_label_inc > 0.0 ) {

    label_tick_width = g_display_width / 100.0;
    x0 = ptr->attr->plot_x;
    y0 = ptr->attr->plot_y;
    x1 = x0 - 1.5 * label_tick_width;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = ptr->attr->y_tick_label_inc;

    while ( dy < ( ptr->attr->ymax + 0.5 * inc ) ) {

      sprintf( value, ptr->attr->yformat, dy );
      psgraph___discard_trailing_blanks( value );

      PsDrawRightJustStringDown( x1, y0, value, strlen(value) );

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 ) +
       ptr->attr->plot_y;

    }

  }

  return PSGRAPH_SUCCESS;

}

int psgraph_draw_y_log_annotation (
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

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  PsSetFont( ptr->attr->label_font_size, ptr->attr->label_font_name );

  y0 = ptr->attr->plot_y;
  y1 = y0 + scale_len;
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

    label_tick_width = g_display_width / 100.0;
    x0 = ptr->attr->plot_x;
    y0 = ptr->attr->plot_y;
    x1 = x0 - 1.5 * label_tick_width;
    y1 = y0;

    dy = ptr->attr->ymin;
    inc = decade_tick_inc;

    while ( dy < ( ptr->attr->ymax + 0.5 * inc ) ) {

      if ( dy < 0.0 )
        logval = pow( 10.0, dy );
      else
        logval = (double) ( (int) ( pow( 10.0, dy ) + 0.5 ) );
      sprintf( value, ptr->attr->yformat, logval );
      psgraph___discard_trailing_blanks( value );

      PsDrawRightJustStringDown( x1, y0, value, strlen(value) );

      dy += inc;
      y0 = y1 = (int) ( dy * ptr->attr->yfact + ptr->attr->yofs + 0.5 ) +
       ptr->attr->plot_y;

    }

  }

  /* label minor ticks (within each decade) */
  if ( ( inner_tick_inc == 1.0 ) && ( ptr->attr->y_minor_tick_inc > 0.0 ) ) {

    label_tick_width = g_display_width / 100.0;
    for ( n=ptr->attr->ymin; n<ptr->attr->ymax; n+=1.0 ) {

      x0 = ptr->attr->plot_x;
      y0 = ptr->attr->plot_y;
      x1 = x0 - 1.5 * label_tick_width;
      y1 = y0;

      if ( n < 0.0 )
        dy = pow( 10.0, n );
      else
        dy = (double) ( (int) ( pow( 10.0, n ) + 0.5 ) );

      inc = dy;

      for ( nn=0; nn<9; nn++ ) {

        dy += inc;
        logval = log10( dy );
        y0 = y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 ) +
         ptr->attr->plot_y;

        sprintf( value, ptr->attr->yformat, dy );
        psgraph___discard_trailing_blanks( value );

        PsDrawRightJustStringDown( x1, y0, value, strlen(value) );

      }

    }

  }

  return PSGRAPH_SUCCESS;

}

int psgraph_draw_y_annotation (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( ptr->attr->y_scale_type == PSGRAPH_SCALE_LINEAR )
    stat = psgraph_draw_y_lin_annotation( id );
  else
    stat = psgraph_draw_y_log_annotation( id );

  return stat;

}

int psgraph_y_annotation (
  AREA_ID id,
  char *format
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  set_area_origin( ptr );

  strncpy( ptr->attr->yformat, format, FORMAT_MAX_CHARS );
  ptr->attr->yformat[FORMAT_MAX_CHARS] = 0;

  stat = psgraph_draw_y_annotation( id );

  return stat;

}

int psgraph_draw_vert_y_label (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, char_width, max_char_width, delta, x, y, x0, y0, x1, y1, lastx;
int scale_len;
double dx, inc;
char chr[2];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  PsSetFont( ptr->attr->label_font_size, ptr->attr->label_font_name );

  y0 = ptr->attr->plot_y;
  y1 = y0 + scale_len;

  /* draw label */
  x0 = 0;

  y = ( y0 + y1 ) * 0.5;	/* mdipoint */

  PsDrawVertRotCenteredString( x0, y, ptr->attr->ylabel,
   strlen(ptr->attr->ylabel) );

  return PSGRAPH_SUCCESS;

}

int psgraph_draw_horz_y_label (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat, i, char_width, max_char_width, delta, x, y, x0, y0, x1, y1, lastx;
int scale_len;
double dx, inc;
char chr[2];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  scale_len = ptr->attr->plot_h;

  PsSetForeground( ptr->attr->fg_color.red, ptr->attr->fg_color.green,
   ptr->attr->fg_color.blue );

  PsSetFont( ptr->attr->label_font_size, ptr->attr->label_font_name );

  /* draw label */

  y0 = ptr->attr->plot_y + ptr->attr->plot_h + 1;
  delta = 1.5 * ptr->attr->label_font_size;
  y0 += delta;

  x0 = ptr->attr->plot_x;

  PsDrawRightJustStringUp( x0, y0, ptr->attr->ylabel, strlen(ptr->attr->ylabel) );

  return PSGRAPH_SUCCESS;

}

int psgraph_draw_y_label (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
int stat;

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  if ( ptr->attr->ylabel_orientation == GRAPH_K_VERTICAL )
    stat = psgraph_draw_vert_y_label( id );
  else
    stat = psgraph_draw_horz_y_label( id );

  return stat;

}

int psgraph_y_label (
  AREA_ID id,
  char *label
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, y, x0, y0, x1, y1, lastx;
int scale_len;
double dx, inc;
char chr[2];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  set_area_origin( ptr );

  ptr->attr->ylabel_orientation = GRAPH_K_VERTICAL;
  strncpy( ptr->attr->ylabel, label, LABEL_MAX_CHARS );
  ptr->attr->ylabel[LABEL_MAX_CHARS] = 0;

  stat = psgraph_draw_y_label( id );

  return stat;

}

int psgraph_horz_y_label (
  AREA_ID id,
  char *label
) {

AREA_PRIV_PTR ptr;
int stat, i, delta, x, y, x0, y0, x1, y1, lastx;
int scale_len;
double dx, inc;
char chr[2];

  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  set_area_origin( ptr );

  ptr->attr->ylabel_orientation = GRAPH_K_HORIZONTAL;
  strncpy( ptr->attr->ylabel, label, LABEL_MAX_CHARS );
  ptr->attr->ylabel[LABEL_MAX_CHARS] = 0;

  stat = psgraph_draw_y_label( id );

  return stat;

}

int psgraph_create_plot (
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

  if ( x_scale_type == PSGRAPH_SCALE_LOG ) {
    stat = psgraph___log_adjust_low_limit( &xmin );
    stat = psgraph___log_adjust_high_limit( &xmax );
  }

  if ( y_scale_type == PSGRAPH_SCALE_LOG ) {
    stat = psgraph___log_adjust_low_limit( &ymin );
    stat = psgraph___log_adjust_high_limit( &ymax );
  }

  ptr = (PLOT_PRIV_PTR) calloc( 1, sizeof(PLOT_PRIV_TYPE) );
  if ( !ptr ) return PSGRAPH_NO_MEM;

  area_ptr = (AREA_PRIV_PTR) area_id;

  if ( !area_ptr ) {
    ret_stat = PSGRAPH_BAD_OBJ;
    goto err_return;
  }
  if ( !area_ptr->id_type ) {
    ret_stat = PSGRAPH_BAD_OBJ;
    goto err_return;
  }

  attr_ptr = (PLOT_PRIV_ATTR_PTR) calloc( 1, sizeof(PLOT_PRIV_ATTR_TYPE) );
  if ( !attr_ptr ) {
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }

  attr_ptr->area_ptr = area_ptr;
  attr_ptr->type = type;
  attr_ptr->x_scale_type = x_scale_type;
  attr_ptr->y_scale_type = y_scale_type;

  stat = psgraph___alloc_color( color_name, g_colormap, &attr_ptr->color );
  if ( stat != PSGRAPH_SUCCESS ) attr_ptr->color.pixel = g_foreground;

  attr_ptr->line_type = 1;
  attr_ptr->plot_symbol = PSGRAPH_NOSYMBOL;
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
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }

  cur_obj->ptr = (void *) ptr;
  area_ptr->obj_tail->next = cur_obj;
  area_ptr->obj_tail = cur_obj;
  area_ptr->obj_tail->next = NULL;

  *id = (PLOT_ID) ptr;

norm_return:
  return PSGRAPH_SUCCESS;

err_return:
  ptr->id_type = 0;
  *id = (PLOT_ID) ptr;
  return ret_stat;

}

static int psgraph___draw_symbol (
  PLOT_PRIV_PTR ptr,
  int x,
  int y
) {

float xpoints[5], ypoints[5];

  if ( ptr->attr->plot_symbol == PSGRAPH_NOSYMBOL ) return PSGRAPH_SUCCESS;

  PsSetForeground( ptr->attr->symbol_color.red, ptr->attr->symbol_color.green,
   ptr->attr->symbol_color.blue );

  switch ( ptr->attr->plot_symbol ) {

    case PSGRAPH_CIRCLE:
      if ( ptr->attr->symbol_fill ) {
        PsFillArcFromCenter( x, y, ptr->attr->i_symbol_width, 0.0, 360.0 );
      }
      else {
        PsDrawArcFromCenter( x, y, ptr->attr->i_symbol_width, 0.0, 360.0 );
      }
      break;

    case PSGRAPH_SQUARE:
      if ( ptr->attr->symbol_fill ) {
        PsFillRectangleFromCenter( x, y, ptr->attr->i_symbol_width,
         ptr->attr->i_symbol_height );
      }
      else {
        PsDrawRectangleFromCenter( x, y, ptr->attr->i_symbol_width,
         ptr->attr->i_symbol_height );
      }
      break;

    case PSGRAPH_DIAMOND:
      xpoints[0] = x;
      ypoints[0] = y - ptr->attr->i_symbol_half_height;
      xpoints[1] = x + ptr->attr->i_symbol_half_width;
      ypoints[1] = y;
      xpoints[2] = xpoints[0];
      ypoints[2] = y + ptr->attr->i_symbol_half_height;
      xpoints[3] = x - ptr->attr->i_symbol_half_width;
      ypoints[3] = y;
      xpoints[4] = xpoints[0];
      ypoints[4] = ypoints[0];
      if ( ptr->attr->symbol_fill ) {
        PsFillPolygon( xpoints, ypoints, 5 );
      }
      else {
        PsDrawLines( xpoints, ypoints, 5 );
      }
      break;

    case PSGRAPH_TRIANGLE:
      xpoints[0] = x;
      ypoints[0] = y - ptr->attr->i_symbol_half_height;
      xpoints[1] = x + ptr->attr->i_symbol_half_width;
      ypoints[1] = y + ptr->attr->i_symbol_half_height;
      xpoints[2] = x - ptr->attr->i_symbol_half_width;
      ypoints[2] = ypoints[1];
      xpoints[3] = xpoints[0];
      ypoints[3] = ypoints[0];
      if ( ptr->attr->symbol_fill ) {
        PsFillPolygon( xpoints, ypoints, 4 );
      }
      else {
        PsDrawLines( xpoints, ypoints, 4 );
      }
      break;

  }

  PsSetForeground( ptr->attr->color.red, ptr->attr->color.green,
   ptr->attr->color.blue );

  return PSGRAPH_SUCCESS;

}

int psgraph_draw_plot (
  PLOT_ID id
) {

PLOT_PRIV_PTR ptr;
AREA_PRIV_PTR area_ptr;
int ret_stat, i, x0, y0, x1, y1;
PLOT_DATA_PTR cur_point;
double logval;

  ptr = (PLOT_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  area_ptr = (AREA_PRIV_PTR) ptr->attr->area_ptr;

  set_plot_origin( area_ptr );

  cur_point = ptr->head->flink;
  if ( !cur_point ) {
    ret_stat = PSGRAPH_SUCCESS;
    goto err_return;
  }

  PsSetForeground( ptr->attr->color.red, ptr->attr->color.green,
   ptr->attr->color.blue );

  /* first point */
  if ( ptr->attr->x_scale_type == PSGRAPH_SCALE_LINEAR ) {
    x1 = (int) ( cur_point->x * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
  }
  else {
    if ( cur_point->x > 0.0 ) {
      logval = log10( cur_point->x );
      x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
    }
    else {
      ret_stat = PSGRAPH_BAD_LOG_VALUE;
      goto err_return;
    }
  }

  if ( ptr->attr->y_scale_type == PSGRAPH_SCALE_LINEAR ) {
    y1 = (int) ( cur_point->y * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
  }
  else {
    if ( cur_point->y > 0.0 ) {
      logval = log10( cur_point->y );
      y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
    }
    else {
      ret_stat = PSGRAPH_BAD_LOG_VALUE;
      goto err_return;
    }
  }

  /* process plot type */
  switch ( ptr->attr->type ) {

    case PSGRAPH_POINT:
      if ( ptr->attr->plot_symbol == PSGRAPH_NOSYMBOL ) {
        psgraph___draw_symbol( ptr, x1, y1 ); /* ??? */
      }
      else {
        psgraph___draw_symbol( ptr, x1, y1 );
      }
      break;

    case PSGRAPH_LINE:
      psgraph___draw_symbol( ptr, x1, y1 );
      break;

    case PSGRAPH_NEEDLE:
      x0 = x1;
      y0 = 0.0;
      PsDrawLine( x0, y0, x1, y1 );
      psgraph___draw_symbol( ptr, x1, y1 );
      break;

  }

  /* remaining points */

  cur_point = cur_point->flink;

  while( cur_point ) {

    x0 = x1;
    y0 = y1;

    if ( ptr->attr->x_scale_type == PSGRAPH_SCALE_LINEAR ) {
      x1 = (int) ( cur_point->x * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
    }
    else {
      if ( cur_point->x > 0.0 ) {
        logval = log10( cur_point->x );
        x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
      }
      else {
        ret_stat = PSGRAPH_BAD_LOG_VALUE;
        goto err_return;
      }
    }

    if ( ptr->attr->y_scale_type == PSGRAPH_SCALE_LINEAR ) {
      y1 = (int) ( cur_point->y * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
    }
    else {
      if ( cur_point->y > 0.0 ) {
        logval = log10( cur_point->y );
        y1 = (int) ( logval * ptr->attr->yfact + ptr->attr->yofs + 0.5 );
      }
      else {
        ret_stat = PSGRAPH_BAD_LOG_VALUE;
        goto err_return;
      }
    }

    /* process plot type */
    switch ( ptr->attr->type ) {

      case PSGRAPH_POINT:
        if ( ptr->attr->plot_symbol == PSGRAPH_NOSYMBOL ) {
          psgraph___draw_symbol( ptr, x1, y1 ); /* ??? */
        }
        else {
          psgraph___draw_symbol( ptr, x1, y1 );
        }
        break;

      case PSGRAPH_LINE:
        PsDrawLine( x0, y0, x1, y1 );
        psgraph___draw_symbol( ptr, x1, y1 );
        break;

      case PSGRAPH_NEEDLE:
        x0 = x1;
        y0 = 0.0;
        PsDrawLine( x0, y0, x1, y1 );
        psgraph___draw_symbol( ptr, x1, y1 );
        break;

    }

    cur_point = cur_point->flink;

  }

  set_area_origin( area_ptr );
  return PSGRAPH_SUCCESS;

err_return:
  set_area_origin( area_ptr );
  return ret_stat;

}

int psgraph_set_symbol_attr (
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

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  area_ptr = (AREA_PRIV_PTR) ptr->attr->area_ptr;

  ptr->attr->plot_symbol = symbol_type;
  if ( symbol_type == PSGRAPH_NOSYMBOL ) return PSGRAPH_SUCCESS;

  ptr->attr->symbol_fill = symbol_fill;

  stat = psgraph___alloc_color( symbol_color_name, g_colormap,
   &ptr->attr->symbol_color );
  if ( stat != PSGRAPH_SUCCESS ) ptr->attr->symbol_color.pixel = g_foreground;

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

  return PSGRAPH_SUCCESS;

}

int psgraph_add_points (
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

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  area_ptr = (AREA_PRIV_PTR) ptr->attr->area_ptr;

  for ( i=0; i<num; i++ ) {

    if ( !ptr->num_points ) {

      if ( ptr->attr->x_scale_type == PSGRAPH_SCALE_LINEAR ) {
        x1 = (int) ( x[i] * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
      }
      else {
        if ( x[i] > 0.0 ) {
          logval = log10( x[i] );
          x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
        }
        else {
          return PSGRAPH_BAD_LOG_VALUE;
        }
      }

      if ( ptr->attr->y_scale_type == PSGRAPH_SCALE_LINEAR ) {
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
          return PSGRAPH_BAD_LOG_VALUE;
        }
      }

    }
    else {

      cur = ptr->tail;

      if ( ptr->attr->x_scale_type == PSGRAPH_SCALE_LINEAR ) {
        x0 = (int) ( cur->x * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
      }
      else {
        if ( cur->x > 0.0 ) {
          logval = log10( cur->x );
          x0 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
        }
        else {
          return PSGRAPH_BAD_LOG_VALUE;
        }
      }

      if ( ptr->attr->y_scale_type == PSGRAPH_SCALE_LINEAR ) {
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
          return PSGRAPH_BAD_LOG_VALUE;
        }
      }

      if ( ptr->attr->x_scale_type == PSGRAPH_SCALE_LINEAR ) {
        x1 = (int) ( x[i] * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
      }
      else {
        if ( x[i] > 0.0 ) {
          logval = log10( x[i] );
          x1 = (int) ( logval * ptr->attr->xfact + ptr->attr->xofs + 0.5 );
        }
        else {
          return PSGRAPH_BAD_LOG_VALUE;
        }
      }

      if ( ptr->attr->y_scale_type == PSGRAPH_SCALE_LINEAR ) {
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
          return PSGRAPH_BAD_LOG_VALUE;
        }
      }

    }

    if ( ptr->lookaside_head->flink ) {
      cur = ptr->lookaside_head->flink;
      ptr->lookaside_head->flink = cur->flink;
    }
    else {
      cur = (PLOT_DATA_PTR) calloc( 1, sizeof(PLOT_PRIV_ATTR_TYPE) );
    }
    if ( !cur ) return PSGRAPH_NO_MEM;
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

  return PSGRAPH_SUCCESS;

}

int psgraph_get_plot_data (
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

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

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

  return PSGRAPH_SUCCESS;

}

int psgraph_delete_points (
  PLOT_ID id
) {

PLOT_PRIV_PTR ptr;
PLOT_DATA_PTR cur;

  ptr = (PLOT_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  cur = ptr->head->flink;

  if ( cur ) {

    ptr->lookaside_tail->flink = cur;
    ptr->lookaside_tail = cur;
    ptr->lookaside_tail->flink = NULL;

    ptr->head->flink = NULL;
    ptr->tail = ptr->head;

  }

  ptr->num_points = 0;

  return PSGRAPH_SUCCESS;

}

int psgraph_create_text (
  GENERIC_PTR parent_id,
  TEXT_ID *id
) {

GENERIC_PRIV_PTR ptr;
AREA_PRIV_PTR area_ptr;
GWIN_PRIV_PTR gwin_ptr;
TEXT_PRIV_PTR text_ptr;

OBJ_LIST_PTR cur_obj;

int stat, ret_stat;

  text_ptr = (TEXT_PRIV_PTR) calloc( 1, sizeof(TEXT_PRIV_TYPE) );
  if ( !text_ptr ) return PSGRAPH_NO_MEM;

  ptr = (GENERIC_PRIV_PTR) parent_id;

  if ( !ptr ) {
    ret_stat = PSGRAPH_BAD_OBJ;
    goto err_return;
  }
  if ( !ptr->id_type ) {
    ret_stat = PSGRAPH_BAD_OBJ;
    goto err_return;
  }

  text_ptr->color.red = 0.0;
  text_ptr->color.green = 0.0;
  text_ptr->color.blue = 0.0;

  text_ptr->font_size = 10.0;
  text_ptr->font_name = (char *) malloc( strlen("helvetica")+1 );
  if ( !text_ptr ) {
    ret_stat = PSGRAPH_NO_MEM;
    goto err_return;
  }
  strcpy( text_ptr->font_name, "helvetica" );

  switch ( ptr->id_type ) {

    text_ptr->text = NULL;

    case GRAPH_K_GWIN_ID:
      gwin_ptr = (GWIN_PRIV_PTR) parent_id;
      text_ptr->win_width = gwin_ptr->attr->w;
      text_ptr->win_height = gwin_ptr->attr->h;
      cur_obj = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
      if ( !cur_obj ) {
        ret_stat = PSGRAPH_NO_MEM;
        goto err_return;
      }
      cur_obj->ptr = (void *) text_ptr;
      gwin_ptr->obj_tail->next = cur_obj;
      gwin_ptr->obj_tail = cur_obj;
      gwin_ptr->obj_tail->next = NULL;
      break;

    case GRAPH_K_AREA_ID:
      area_ptr = (AREA_PRIV_PTR) parent_id;
      text_ptr->win_width = area_ptr->attr->w;
      text_ptr->win_height = area_ptr->attr->h;
      text_ptr->plotwin_width = area_ptr->attr->plot_w;
      text_ptr->plotwin_height = area_ptr->attr->plot_h;
      cur_obj = (OBJ_LIST_PTR) calloc( 1, sizeof(OBJ_LIST_TYPE) );
      if ( !cur_obj ) {
        ret_stat = PSGRAPH_NO_MEM;
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
  return PSGRAPH_SUCCESS;

err_return:
  text_ptr->id_type = 0;
  *id = (TEXT_ID) text_ptr;
  return ret_stat;

}

int psgraph_set_text_attr (
  TEXT_ID id,
  char *color,
  double font_size,
  char *font,
  char justify
) {

TEXT_PRIV_PTR text_ptr;
int l_old, l_new, stat;
AREA_PRIV_PTR area_ptr;
GWIN_PRIV_PTR gwin_ptr;
char full_font[127+1];

  text_ptr = (TEXT_PRIV_PTR) id;

  if ( !text_ptr ) return PSGRAPH_BAD_OBJ;
  if ( !text_ptr->id_type ) return PSGRAPH_BAD_OBJ;

  stat = psgraph___alloc_color( color, g_colormap, &text_ptr->color );
  if ( stat != PSGRAPH_SUCCESS ) text_ptr->color.pixel = g_foreground;

  text_ptr->font_size = font_size;

  if ( text_ptr->font_name ) {
    l_old = strlen(text_ptr->font_name);
    l_new = strlen(font);
    if ( l_new > l_old )
      text_ptr->font_name = (char *) realloc( text_ptr->font_name,
       strlen(font)+1 );
  }
  else {
    text_ptr->font_name = (char *) malloc( strlen(font)+1 );
  }

  if ( !text_ptr ) return PSGRAPH_NO_MEM;
  strcpy( text_ptr->font_name, font );

  justify = toupper( justify );
  if ( justify == 'C' )
    text_ptr->justify = GRAPH_K_CENTER;
  else if ( justify == 'R' )
    text_ptr->justify = GRAPH_K_RIGHT;
  else
    text_ptr->justify = GRAPH_K_LEFT;

  return PSGRAPH_SUCCESS;

}

static int psgraph___put_text (
  TEXT_ID id
) {

TEXT_PRIV_PTR text_ptr;
int stat;
int posx, posy;

  text_ptr = (TEXT_PRIV_PTR) id;

  if ( !text_ptr ) return PSGRAPH_BAD_OBJ;
  if ( !text_ptr->id_type ) return PSGRAPH_BAD_OBJ;

  if ( !text_ptr->text ) return PSGRAPH_SUCCESS;

  PsSetForeground( text_ptr->color.red, text_ptr->color.green,
   text_ptr->color.blue );

  PsSetFont( text_ptr->font_size, text_ptr->font_name );

  if ( text_ptr->parent_id->id_type == GRAPH_K_AREA_ID ) {

    set_plot_origin( text_ptr->parent_id );

    posx = text_ptr->x * 0.01 * text_ptr->plotwin_width;
    posy = text_ptr->y * 0.01 * text_ptr->plotwin_height;

    if ( text_ptr->justify == GRAPH_K_CENTER ) {
      PsDrawCenteredString( posx, posy, text_ptr->text,
       strlen(text_ptr->text) );
    }
    else if ( text_ptr->justify == GRAPH_K_RIGHT ) {
      PsDrawRightJustString( posx, posy, text_ptr->text,
       strlen(text_ptr->text) );
    }
    else {
      PsDrawString( posx, posy, text_ptr->text,
       strlen(text_ptr->text) );
    }

  }
  else if ( text_ptr->parent_id->id_type == GRAPH_K_GWIN_ID ) {

    set_win_origin( text_ptr->parent_id );

    posx = text_ptr->x * 0.01 * text_ptr->win_width;
    posy = text_ptr->y * 0.01 * text_ptr->win_height-1;

    if ( text_ptr->justify == GRAPH_K_CENTER ) {
      PsDrawCenteredString( posx, posy, text_ptr->text,
       strlen(text_ptr->text) );
    }
    else if ( text_ptr->justify == GRAPH_K_RIGHT ) {
      PsDrawRightJustString( posx, posy, text_ptr->text,
       strlen(text_ptr->text) );
    }
    else {
      PsDrawString( posx, posy, text_ptr->text,
       strlen(text_ptr->text) );
    }

  }

  set_prev_origin();

  return PSGRAPH_SUCCESS;

}

int psgraph_put_text (
  TEXT_ID id,
  double x,		/* percent */
  double y,		/* percent */
  char *text
) {

TEXT_PRIV_PTR text_ptr;
int l_old, l_new, stat;

  text_ptr = (TEXT_PRIV_PTR) id;

  if ( !text_ptr ) return PSGRAPH_BAD_OBJ;
  if ( !text_ptr->id_type ) return PSGRAPH_BAD_OBJ;

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

  return stat;

}

int psgraph_plot_area (
  AREA_ID id
) {

AREA_PRIV_PTR ptr;
OBJ_LIST_PTR cur_obj;
GENERIC_PRIV_PTR generic_id;
PLOT_ID plot_id;
TEXT_ID text_id;
int stat, ret_stat;

  ret_stat = PSGRAPH_SUCCESS;
  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  PsGsave();

  psgraph_draw_area_border( id );

  PsClipArea( id );

  if ( ptr->attr->xlabel_grid || ptr->attr->xmajor_grid ||
       ptr->attr->xminor_grid || ptr->attr->ylabel_grid ||
       ptr->attr->ymajor_grid || ptr->attr->yminor_grid ) {
    stat = psgraph_draw_plot_border ( id );
  }

  stat = psgraph_draw_title( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = psgraph_draw_x_scale( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = psgraph_draw_x_annotation( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = psgraph_draw_x_label( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = psgraph_draw_y_scale( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = psgraph_draw_y_annotation( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  stat = psgraph_draw_y_label( id );
  if ( !( stat & 1 ) ) ret_stat = stat;

  /* refresh all plots */
  cur_obj = ptr->obj_head->next;
  while ( cur_obj ) {
    generic_id = (GENERIC_PRIV_PTR) cur_obj->ptr;
    if ( generic_id->id_type == GRAPH_K_PLOT_ID ) {
      PsGrestore();
      PsGsave();
      PsClipPlot( id );
      plot_id = (PLOT_ID) cur_obj->ptr;
      stat = psgraph_draw_plot( plot_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
      PsGrestore();
      PsGsave();
      PsClipArea( id );
    }
    else if ( generic_id->id_type == GRAPH_K_TEXT_ID ) {
      text_id = (TEXT_ID) cur_obj->ptr;
      stat = psgraph___put_text( text_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    cur_obj = cur_obj->next;
  }

  PsGrestore();

  return ret_stat;

}

int psgraph_area_rescale_x (
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

  ret_stat = PSGRAPH_SUCCESS;
  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  if ( ptr->attr->x_scale_type == PSGRAPH_SCALE_LOG ) {
    stat = psgraph___log_adjust_low_limit( &xmin );
    stat = psgraph___log_adjust_high_limit( &xmax );
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

  return ret_stat;

}

int psgraph_area_rescale_y (
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

  ret_stat = PSGRAPH_SUCCESS;
  ptr = (AREA_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  if ( ptr->attr->y_scale_type == PSGRAPH_SCALE_LOG ) {
    stat = psgraph___log_adjust_low_limit( &ymin );
    stat = psgraph___log_adjust_high_limit( &ymax );
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

}

int psgraph_plot_gwin (
  GWIN_ID id
) {

GWIN_PRIV_PTR ptr;
AREA_ID area_id;
TEXT_ID text_id;
OBJ_LIST_PTR cur_obj;
GENERIC_PRIV_PTR generic_id;
int stat, ret_stat;

  ret_stat = PSGRAPH_SUCCESS;
  ptr = (GWIN_PRIV_PTR) id;

  if ( !ptr ) return PSGRAPH_BAD_OBJ;
  if ( !ptr->id_type ) return PSGRAPH_BAD_OBJ;

  /* refresh all areas */
  cur_obj = ptr->obj_head->next;
  while ( cur_obj ) {
    generic_id = (GENERIC_PRIV_PTR) cur_obj->ptr;
    if ( generic_id->id_type == GRAPH_K_AREA_ID ) {
      area_id = (AREA_ID) cur_obj->ptr;
      stat = psgraph_plot_area( area_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    else if ( generic_id->id_type == GRAPH_K_TEXT_ID ) {
      text_id = (TEXT_ID) cur_obj->ptr;
      stat = psgraph___put_text( text_id );
      if ( !( stat & 1 ) ) ret_stat = stat;
    }
    cur_obj = cur_obj->next;
  }

  return ret_stat;

}
