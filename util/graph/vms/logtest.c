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
static char *big_title_font = "-*-helvetica-medium-r-normal--*-120-*-*-*-*-iso8859-*";
static char *big_label_font = "-*-helvetica-medium-r-normal--*-100-*-*-*-*-iso8859-*";
static char *little_title_font = "-*-*-medium-r-normal--*-100-*-*-*-*-iso8859-*";
static char *little_label_font = "-*-*-medium-r-normal--*-90-*-*-*-*-iso8859-*";
static double x[200];
static double y[200];

  stat = xgraph_init();

  stat = xgraph_create_window( 10.0, 10.0, 80.0, 50.0, "screen", "icon", 
   "gray", &gwin_id1 );

  stat = xgraph_create_area( gwin_id1, 2.5, 5.0, 95.0, 90.0,
   XGRAPH_SCALE_LOG, XGRAPH_SCALE_LOG, 10.0, 100.0, 100.0,
   10000.0, "red", "light blue", big_title_font, little_label_font,
   2, &area_id1 );

  stat = xgraph_x_scale( area_id1, 0.0, 1.0, 1.0 );
  xgraph_flush();
  stat = xgraph_x_annotation( area_id1, "%-5.0f" );
  xgraph_flush();
  stat = xgraph_x_label( area_id1, "X Axis Label" );
  xgraph_flush();
  stat = xgraph_y_scale( area_id1, 0.0, 1.0, 1.0 );
  xgraph_flush();
  stat = xgraph_y_annotation( area_id1, "%-11.5f" );
  xgraph_flush();
  stat = xgraph_y_label( area_id1, "Y Axis Label" );
  xgraph_flush();
  stat = xgraph_title( area_id1, "Graph Title" );
  xgraph_flush();

  stat = xgraph_create_plot( area_id1, XGRAPH_NEEDLE, "red",
   XGRAPH_SCALE_LOG, XGRAPH_SCALE_LOG, 10.0, 100.0, 100.0, 10000.0, &plot_id1 );

  stat = xgraph_create_plot( area_id1, XGRAPH_NEEDLE, "blue",
   XGRAPH_SCALE_LINEAR, XGRAPH_SCALE_LINEAR, 10.0, 100.0, 100.0, 10000.0,
   &plot_id2 );

  delay = 0.25;
  y[0] = 0.0;
  for ( i=0; i<100; i++ ) {
    x[i] = (double) i;
    y[i] += (double) ( i*100 );
    stat = xgraph_add_points( plot_id1, 1, &x[i], &y[i] );
    stat = xgraph_add_points( plot_id2, 1, &x[i], &y[i] );
    xgraph_flush();
    lib$wait( &delay );
  }

  while ( !sys_num_chars() ) {
    xgraph_flush();
    lib$wait( &delay );
  }

}
