#include stdlib
#include stdio
#include math

#include "xgraph.h"

main() {

GWIN_ID gwin_id;
AREA_ID area_id;
PLOT_ID plot_id;
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

  stat = xgraph_init();

  stat = xgraph_create_window( 10.0, 10.0, 80.0, 25.0, "screen", "icon", 
   "navajo white", &gwin_id );

  stat = xgraph_create_transient_text( gwin_id, &t2 );
    stat = xgraph_set_text_attr( t2, "black", little_title_font, 'l' );

  stat = xgraph_create_area( gwin_id, 2.5, 5.0, 95.0, 85.0,
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0,
   2.5, "steel blue", "navajo white", little_title_font, little_label_font, 2,
   &area_id );

  stat = xgraph_x_scale( area_id, 10.0, 5.0, 10.0 );
  stat = xgraph_x_annotation( area_id, "%-5.1f" );
  stat = xgraph_x_label( area_id, "X Axis Label" );
  stat = xgraph_y_scale( area_id, 0.5, 0.1, 0.5 );
  stat = xgraph_y_annotation( area_id, "%-5.1f" );
  stat = xgraph_y_label( area_id, "Y Axis Label" );
  stat = xgraph_title( area_id, "Graph Title" );

  stat = xgraph_refresh_gwin( gwin_id );
  xgraph_flush();

  stat = xgraph_create_plot( area_id, XGRAPH_LINE, "RoyalBlue",
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 0.0, 100.0, 0.0, 2.5, &plot_id );

  stat = xgraph_set_symbol_attr( plot_id, XGRAPH_TRIANGLE, "white", 1, 1.0,
   1.0 );

  stat = xgraph_create_text( area_id, &area_text );
  stat = xgraph_set_text_attr( area_text, "purple4", little_label_font, 'l' );
  stat = xgraph_put_text( area_text, 5.0, 90.0, "This is plot text" );

  stat = xgraph_refresh_gwin( gwin_id );
  xgraph_flush();

  n = 0;
  for ( i=n; i<101; i++ ) {
    x[i] = (double) i;
    if ( x[i] == 0.0 )
      y[i] = 0.25;
    else
      y[i] = log10( x[i] ) + 0.5;
    stat = xgraph_add_points( plot_id, 1, &x[i], &y[i] );
  }

  stat = xgraph_refresh_gwin( gwin_id );
  xgraph_flush();

  do {

    reading = xgraph_read_pointer( &xp, &yp, &button );
    if ( reading == XGRAPH_SUCCESS ) {
      if ( !button ) {
        sprintf( new, "x=%-6.1f, y=%-6.1f ", xp, yp );
        if ( old[0] ) stat = xgraph_output_text( t2, 2.5, 95.0, old, 0 );
        stat = xgraph_output_text( t2, 2.5, 95.0, new, 1 );
        strcpy( old, new );
      }
    }
    else {
      lib$wait( &delay );
    }

  } while ( !button );

}
