#include stdlib
#include stdio
#include math

#include "xgraph.h"

main() {

GWIN_ID gwin_id, gwin_id1, gwin_id2, gwin_id3, gwin_id4;
AREA_ID area_id, area_id1, area_id2, area_id3, area_id4, area_id5, area_id6;
PLOT_ID plot_id1, plot_id2, plot_id3, plot_id4, plot_id5, plot_id6, plot_id7;
TEXT_ID win_text, area_text;
int stat, key, i, n;
float delay;

/*
static char *big_title_font = "-*-helvetica-medium-r-normal--*-120-*-*-*-*-iso8859-*";
static char *big_label_font = "-*-helvetica-medium-r-normal--*-100-*-*-*-*-iso8859-*";
static char *little_title_font = "-*-*-medium-r-normal--*-100-*-*-*-*-iso8859-*";
static char *little_label_font = "-*-*-medium-r-normal--*-60-*-*-*-*-iso8859-*";
*/

static char *big_title_font =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-140-*-*-P-*-ISO8859-1";
static char *big_label_font =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-140-*-*-P-*-ISO8859-1";
static char *little_title_font =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-140-100-100-P-*-ISO8859-1";
static char *little_label_font =
 "-Adobe-New Century Schoolbook-Medium-R-Normal--*-140-100-100-P-*-ISO8859-1";

static double x[200];
static double y[200];

  stat = xgraph_init();

  stat = xgraph_create_window( 10.0, 10.0, 80.0, 25.0, "screen", "icon", 
   "gray", &gwin_id1 );

  stat = xgraph_create_area( gwin_id1, 2.5, 5.0, 95.0, 90.0,
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0,
   100.0, "red", "light blue", little_title_font, little_label_font, 2, &area_id1 );

  stat = xgraph_x_scale( area_id1, 10.0, 1.0, 10.0 );
  stat = xgraph_x_annotation( area_id1, "%-5.0f" );
  stat = xgraph_x_label( area_id1, "X Axis Label" );
  stat = xgraph_y_scale( area_id1, 10.0, 1.0, 10.0 );
  stat = xgraph_y_annotation( area_id1, "%-5.0f" );
  stat = xgraph_y_label( area_id1, "Y Axis Label" );
  stat = xgraph_title( area_id1, "Graph Title (lf)" );

  stat = xgraph_create_plot( area_id1, XGRAPH_NEEDLE, "red",
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0, 2.5, &plot_id1 );

  delay = 0.25;
  n = 0;
  for ( i=n; i<101; i++ ) {
    x[i] = (double) i;
    if ( x[i] == 0.0 )
      y[i] = 0.25;
    else
      y[i] = log10( x[i] ) + 0.5;
    stat = xgraph_add_points( plot_id1, 1, &x[i], &y[i] );
  }

  xgraph_flush();

/* ========================================================================== */

  stat = xgraph_create_window( 10.0, 60.0, 40.0, 20.0, "screen", "icon",
   "light blue", &gwin_id2 );

  stat = xgraph_create_area( gwin_id2, 2.5, 5.0, 95.0, 90.0,
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0,
   100.0, "black", "light blue", little_title_font, little_label_font, 2, &area_id2 );

  stat = xgraph_x_scale( area_id2, 10.0, 5.0, 10.0 );
  stat = xgraph_x_annotation( area_id2, "%-3.1e" );
  stat = xgraph_x_label( area_id2, "X Axis Label" );
  stat = xgraph_y_scale( area_id2, 10.0, 5.0, 10.0 );
  stat = xgraph_y_annotation( area_id2, "%-3.1e" );
  stat = xgraph_y_label( area_id2, "Y Axis Label" );
  stat = xgraph_title( area_id2, "Graph Title" );

  stat = xgraph_create_plot( area_id2, XGRAPH_NEEDLE, "red",
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0, 2.5, &plot_id2 );

  delay = 0.25;
  n = 0;
  for ( i=n; i<101; i++ ) {
    x[i] = (double) i;
    if ( x[i] == 0.0 )
      y[i] = 0.25;
    else
      y[i] = log10( x[i] ) + 0.5;
    stat = xgraph_add_points( plot_id2, 1, &x[i], &y[i] );
  }

  xgraph_flush();

/* ========================================================================== */

  stat = xgraph_create_window( 10.0, 30.0, 80.0, 40.0, "screen", "icon",
   "gray", &gwin_id3 );

  stat = xgraph_create_area( gwin_id3, 1.0, 2.0, 48.0, 96.0,
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0,
   2.5, "black", "gray", big_label_font, big_label_font, 2, &area_id3 );

  stat = xgraph_x_scale( area_id3, 10.0, 5.0, 10.0 );
  stat = xgraph_x_annotation( area_id3, "%-5.0f" );
  stat = xgraph_x_label( area_id3, "X Axis Label" );
  stat = xgraph_y_scale( area_id3, 1.0, 0.5, 0.5 );
  stat = xgraph_y_annotation( area_id3, "%-5.3f" );
  stat = xgraph_y_label( area_id3, "Y Axis Label" );
  stat = xgraph_title( area_id3, "Graph Title" );

  stat = xgraph_create_plot( area_id3, XGRAPH_NEEDLE, "blue",
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0, 2.5, &plot_id3 );

  delay = 0.25;
  n = 0;
  for ( i=n; i<101; i++ ) {
    x[i] = (double) i;
    if ( x[i] == 0.0 )
      y[i] = 0.25;
    else
      y[i] = log10( x[i] ) + 0.5;
    stat = xgraph_add_points( plot_id3, 1, &x[i], &y[i] );
  }

  stat = xgraph_create_plot( area_id3, XGRAPH_LINE, "red",
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0, 2.5, &plot_id4 );

  delay = 0.25;
  n = 0;
  for ( i=n; i<101; i++ ) {
    x[i] = (double) i;
    if ( x[i] == 0.0 )
      y[i] = 0.25;
    else
      y[i] = log10( x[i] ) + 0.5;
    stat = xgraph_add_points( plot_id4, 1, &x[i], &y[i] );
  }

  stat = xgraph_create_area( gwin_id3, 51.0, 2.0, 48.0, 96.0,
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0,
   2.5, "black", "gray", big_label_font, big_label_font, 2, &area_id4 );

  stat = xgraph_x_scale( area_id4, 10.0, 5.0, 10.0 );
  stat = xgraph_x_annotation( area_id4, "%-5.0f" );
  stat = xgraph_x_label( area_id4, "X Axis Label" );
  stat = xgraph_y_scale( area_id4, 1.0, 0.5, 0.5 );
  stat = xgraph_y_annotation( area_id4, "%-5.3f" );
  stat = xgraph_y_label( area_id4, "Y Axis Label" );
  stat = xgraph_title( area_id4, "Graph Title" );

  stat = xgraph_create_plot( area_id4, XGRAPH_NEEDLE, "green",
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0, 2.5, &plot_id5 );

  delay = 0.25;
  n = 0;
  for ( i=n; i<101; i++ ) {
    x[i] = (double) i;
    if ( x[i] == 0.0 )
      y[i] = 0.25;
    else
      y[i] = log10( x[i] ) + 0.5;
    stat = xgraph_add_points( plot_id5, 1, &x[i], &y[i] );
  }

  xgraph_flush();

  key = getchar();

  stat = xgraph_area_rescale_x( area_id4, 0.0, 200.0, 10.0, 5.0, 50.0,
   "%-7.1e" );

  xgraph_flush();

  key = getchar();

  stat = xgraph_area_rescale_y( area_id4, 0.0, 3.0, 0.5, 0.25, 1.0,
   "%-7.1e" );

  xgraph_flush();

  key = getchar();

/* ========================================================================== */

  delay = 1.0;
  for ( i=0; i<5; i++ ) {
    xgraph_flush();
    lib$wait( &delay );
  }

  xgraph_delete_points( plot_id4 );
  xgraph_clear_plot_area( area_id3 );
  xgraph_flush();

  for ( i=0; i<1; i++ ) {
    xgraph_flush();
    lib$wait( &delay );
  }

  xgraph_close_window ( gwin_id2 );

  for ( i=0; i<1; i++ ) {
    xgraph_flush();
    lib$wait( &delay );
  }

  xgraph_open_window ( gwin_id2 );
  xgraph_flush();

  for ( i=0; i<5; i++ ) {
    xgraph_flush();
    lib$wait( &delay );
  }

  key = getchar();

/*
  stat = xgraph_create_text( area_id, &area_text );
  stat = xgraph_set_text_attr( area_text, "blue",
   "-adobe-itc avant garde gothic-book-o-normal--10-100-75-75-p-59-iso8859-1" );
  stat = xgraph_put_plot_text( area_text, 25.0, 50.0, "This is plot text" );
*/

}
