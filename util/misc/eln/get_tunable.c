#include $vaxelnc
#include stdio
#include stdlib
#include descrip

/***
/* macros
*/
#define MAKE_DESCRIP( text, d_text )\
  d_text.dsc$w_length = strlen(text);\
  d_text.dsc$b_dtype = 14;\
  d_text.dsc$b_class = 1;\
  d_text.dsc$a_pointer = text;\

void get_int_tunable(
  char *chan_name,
  int *value,
  int *found
) {

int chix;
struct dsc$descriptor d_chan_name;

  MAKE_DESCRIP( chan_name, d_chan_name );

  chix = 0;
  if( !(*found = vdb_search( &d_chan_name, &chix ) ) ) return;

  vdb_iget( &chix, value );

}

void get_real_tunable(
  char *chan_name,
  float *value,
  int *found
) {

int chix;
struct dsc$descriptor d_chan_name;

  MAKE_DESCRIP( chan_name, d_chan_name );

  chix = 0;
  if( !(*found = vdb_search( &d_chan_name, &chix ) ) ) return;

  vdb_rget( &chix, value );

}
