#ifdef __misc_included
#define __misc_included

/***
/* NAME_MAX_CHARS is the maximum character length of a Vsystem database name
*/
#define NAME_MAX_CHARS 60

/***
/* public prototypes
*/
void get_local_crate_id(
  int *local_crate_id );

int access_protected(
  char data_type,	/* 'B','W','L' */
  char access_mode,	/* 'R','W' */
  int num_items,
  void *address,	/* address to access */
  void *value );	/* type of value should correspond to data_type */

void get_int_tunable(
  char *chan_name,
  int *value,
  int *found );

void get_real_tunable(
  char *chan_name,
  float *value,
  int *found );

#endif
