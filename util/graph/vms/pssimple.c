#include stdlib
#include stdio
#include math

#include "psgraph.h"

main() {

GWIN_ID gwin_id;
AREA_ID area_id, area_id2;
PLOT_ID plot_id, plot_id2;
TEXT_ID win_text, area_text, t2;
int stat, key, i, n;
float delay;
static char *big_title_font = "-*-helvetica-medium-r-normal--*-120-*-*-*-*-iso8859-*";
static char *big_label_font = "-*-helvetica-medium-r-normal--*-100-*-*-*-*-iso8859-*";
static char *little_title_font = "-*-helvetica-medium-r-normal--*-120-*-*-*-*-iso8859-*";
static char *little_label_font = "-*-helvetica-medium-r-normal--*-100-*-*-*-*-iso8859-*";
static double x[200];
static double y[200];
int reading, button;
float xp, yp;
char old[64+1], new[64+1];

  stat = psgraph_init();

  stat = psgraph_open_file( "sys$login:tmp.ps" );

  stat = psgraph_create_window( 0.0, 0.0, 100.0, 100.0, &gwin_id );

  stat = psgraph_create_text( gwin_id, &t2 );
  stat = psgraph_set_text_attr( t2, "brown", 14.0, "helvetica", 'l' );
  stat = psgraph_put_text( t2, 5.0, 95.0, "This is window text" );

  stat = psgraph_create_area( gwin_id, 2.5, 5.0, 95.0, 40.0,
   PSGRAPH_SCALE_LINEAR, PSGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0,
   2.5, "black", "navajo white", 18, "helvetica", 12, "helvetica", 2,
   &area_id );

  psgraph_x_label_grid( area_id, 0 );
  psgraph_y_label_grid( area_id, 1 );

  stat = psgraph_x_scale( area_id, 5.0, 1.0, 10.0 );
  stat = psgraph_x_annotation( area_id, "%-5.1f" );
  stat = psgraph_x_label( area_id, "X Axis Label" );
  stat = psgraph_y_scale( area_id, 0.5, 0.1, 0.5 );
  stat = psgraph_y_annotation( area_id, "%-5.1f" );
  stat = psgraph_y_label( area_id, "Y Axis Label" );
  stat = psgraph_horz_y_label( area_id, "Y Axis Label" );
  stat = psgraph_title( area_id, "Graph Title" );

  stat = psgraph_create_plot( area_id, PSGRAPH_LINE, "RoyalBlue",
   PSGRAPH_SCALE_LINEAR, PSGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0, 2.5, &plot_id );

  stat = psgraph_set_symbol_attr( plot_id, PSGRAPH_TRIANGLE, "red", 1, 1.0,
   1.0 );

  stat = psgraph_create_text( area_id, &area_text );
  stat = psgraph_set_text_attr( area_text, "green", 8.0, "helvetica", 'l' );
  stat = psgraph_put_text( area_text, 5.0, 90.0, "This is plot text" );

  n = 0;
  for ( i=n; i<101; i++ ) {
    x[i] = (double) i;
    if ( x[i] == 0.0 )
      y[i] = 0.25;
    else
      y[i] = log10( x[i] ) + 0.5;
    stat = psgraph_add_points( plot_id, 1, &x[i], &y[i] );
  }

  stat = psgraph_create_area( gwin_id, 2.5, 50.0, 95.0, 40.0,
   PSGRAPH_SCALE_LINEAR, PSGRAPH_SCALE_LOG, 0.0, 100.0, 0.0,
   100.0, "black", "navajo white", 18, "helvetica", 6, "helvetica", 2,
   &area_id2 );

  psgraph_x_label_grid( area_id2, 1 );
  psgraph_y_major_grid( area_id2, 1 );
  psgraph_y_minor_grid( area_id2, 1 );

  stat = psgraph_x_scale( area_id2, 5.0, 1.0, 10.0 );
  stat = psgraph_x_annotation( area_id2, "%-5.1f" );
  stat = psgraph_x_label( area_id2, "X Axis Label" );
  stat = psgraph_y_scale( area_id2, 1.0, 1.0, 0.1 );
  stat = psgraph_y_annotation( area_id2, "%-5.1f" );
  stat = psgraph_y_label( area_id2, "Y Axis Label" );
  stat = psgraph_horz_y_label( area_id2, "Y Axis Label" );
  stat = psgraph_title( area_id2, "Graph Title" );

  stat = psgraph_create_plot( area_id2, PSGRAPH_LINE, "RoyalBlue",
   PSGRAPH_SCALE_LINEAR, PSGRAPH_SCALE_LOG, 0.0, 100.0, 0.0, 100.0, &plot_id2 );

  stat = psgraph_set_symbol_attr( plot_id2, PSGRAPH_TRIANGLE, "red", 1, 1.0,
   1.0 );

  stat = psgraph_create_text( area_id2, &area_text );
  stat = psgraph_set_text_attr( area_text, "orange", 8.0, "helvetica", 'l' );
  stat = psgraph_put_text( area_text, 5.0, 90.0, "This is plot text" );

  n = 0;
  for ( i=n; i<101; i++ ) {
    x[i] = (double) i;
    if ( x[i] == 0.0 )
      y[i] = 0.25;
    else
      y[i] = x[i];
    stat = psgraph_add_points( plot_id2, 1, &x[i], &y[i] );
  }

  stat = psgraph_plot_gwin( gwin_id );

  stat = psgraph_close_file();

}
