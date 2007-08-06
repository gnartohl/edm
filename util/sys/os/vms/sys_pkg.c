#include stdio
#include stdlib
#include descrip
#include ssdef
#include iodef
#include secdef
#include psldef
#include jpidef
#include libdef
#include libdtdef

#include sys_types


void sys_wait_seconds (
  float *seconds
) {

  lib$wait( seconds );

}

int sys_get_proc_id (
  SYS_PROC_ID_PTR proc_id
) {

int stat, item;
unsigned int pid;

  item = JPI$_PID;
  stat = lib$getjpi( &item, 0, 0, &pid, 0, 0 );

  if ( stat % 2 )
    proc_id->id = pid;
  else
    proc_id->id = 0;

  return stat;

}

int sys_get_user_name (
  int max_chars,
  char *name
) {

struct dsc$descriptor dsc;
int stat, item;
short len;

  MAKE_FIXED_DESCRIP( name, max_chars, dsc );
  item = JPI$_USERNAME;
  stat = lib$getjpi( &item, 0, 0, 0, &dsc, &len );

  name[len] = 0;

  return stat;

}

int sys_get_time (
  SYS_TIME_PTR time
) {

int stat;

  stat = sys$gettim( time );

  return stat;

}

int sys_get_datetime_string (
  int string_size,
  char *string
) {

SYS_TIME_TYPE time;
struct dsc$descriptor dsc;
int i, flag, stat;
short len;

  stat = sys$gettim( &time );

  for ( i=0; i<string_size; i++ ) string[i] = ' ';

  MAKE_FIXED_DESCRIP( string, string_size, dsc );

  flag = 0;
  stat = lib$sys_asctim ( &len, &dsc, &time, &flag );

  return stat;

}

int sys_cvt_string_to_time (
  char *string,
  int string_size,
  SYS_TIME_PTR time
) {

struct dsc$descriptor dsc;
int i, flag, stat, context, len;

  len = 0;
  for ( i=0; i<string_size; i++ ) {
    if ( string[i] == 0 ) break;
    len = i+1;
  }

  MAKE_FIXED_DESCRIP( string, len, dsc );

  flag = 0;
  context = 0;
  stat = lib$convert_date_string ( &dsc, time, &context, &flag, 0, 0 );

  return stat;

}

int sys_cvt_time_to_string (
  SYS_TIME_PTR time,
  int string_size,
  char *string
) {

struct dsc$descriptor dsc;
short len;
int flag, stat;

  MAKE_FIXED_DESCRIP( string, string_size, dsc );

  flag = 0;
  stat = lib$sys_asctim ( &len, &dsc, time, &flag );

  return stat;

}

int sys_cvt_time_to_julian_date (
  SYS_TIME_PTR time,
  int *julian_date
) {

int stat;
int op = LIB$K_JULIAN_DATE;

  stat = lib$cvt_from_internal_time( &op, julian_date, NULL );

  return stat;

}

int sys_get_time_diff_in_hours (
  SYS_TIME_PTR cur_time,
  SYS_TIME_PTR time,
  float *hours
) {

int stat;
unsigned int op;
SYS_TIME_TYPE diff;

  stat = lib$sub_times( cur_time, time, &diff );
  if ( !( stat % 2 ) ) return stat;

  op = LIB$K_DELTA_HOURS_F;
  stat = lib$cvtf_from_internal_time( &op, hours, &diff );

  return stat;

}

int sys_cvt_hours_to_time (
  float hours,
  SYS_TIME_PTR time
) {

int stat;
unsigned int op;
SYS_TIME_TYPE cur_time, delta_time;

/***
/* hours must be positive
*/
  if ( hours < 0.01 ) hours = 0.01;

/***
/* first, convert hours to delta time
*/
  op = LIB$K_DELTA_HOURS_F;
  stat = lib$cvtf_to_internal_time( &op, &hours, &delta_time );
  if ( !( stat % 2 ) ) goto err_return;

/***
/* get current time
*/
  stat = sys_get_time( &cur_time );
  if ( !( stat % 2 ) ) goto err_return;

/***
/* now compute an absolute time from the delta time
*/
  stat = lib$sub_times( &cur_time, &delta_time, time );
  if ( !( stat % 2 ) ) goto err_return;

err_return:
  return stat;

}

int sys_add_times (	/* time1 = time1 + time2 */
  SYS_TIME_PTR time1,
  SYS_TIME_PTR time2
) {

int stat;
SYS_TIME_TYPE result;

  stat = lib$add_times( time1, time2, &result );
  if ( !( stat % 2 ) ) goto err_return;

  *time1 = result;

err_return:
  return stat;

}

int sys_subtract_times (	/* time1 = time1 - time2 */
  SYS_TIME_PTR time1,
  SYS_TIME_PTR time2
) {

int stat;
SYS_TIME_TYPE result;

  stat = lib$sub_times( time1, time2, &result );
  if ( !( stat % 2 ) ) goto err_return;

  *time1 = result;

err_return:
  return stat;

}

int sys_cvt_seconds_to_timeout (
  float seconds,
  SYS_TIME_PTR time
) {

int stat;
unsigned int op;

/***
/* seconds must be positive
*/
  if ( seconds < 0.01 ) seconds = 0.01;

/***
/* convert seconds to delta time
*/
  op = LIB$K_DELTA_SECONDS_F;
  stat = lib$cvtf_to_internal_time( &op, &seconds, time );

  return stat;

}

int sys_cvt_timeout_to_seconds (
  SYS_TIME_PTR time,
  float *seconds
) {

int stat;
unsigned int op;

/***
/* convert delta time to seconds
*/
  op = LIB$K_DELTA_SECONDS_F;
  stat = lib$cvtf_from_internal_time( &op, seconds, time );

  return stat;

}

int sys_time_is_later (
  int *status,
  char *string1,
  char *string2,
  int string_size
) {

/*
* return 1 if string1 is later than string2 else return 0
*/

struct dsc$descriptor dsc;
int i, flag, stat, context, len, result, blank;
SYS_TIME_TYPE time1, time2;

  /* if string1 or string2 is blank then return 0 */

  blank = TRUE;
  len = 0;
  for ( i=0; i<string_size; i++ ) {
    if ( string1[i] == 0 ) break;
    if ( string1[i] != ' ' ) blank = FALSE;
    len = i+1;
  }

  if ( blank ) {
    result = 0;
    goto norm_ret;
  }

  MAKE_FIXED_DESCRIP( string1, len, dsc );

  flag = 0;
  context = 0;
  stat = lib$convert_date_string ( &dsc, &time1, &context, &flag, 0, 0 );
  if ( !( stat % 2 ) ) goto err_ret;

  blank = TRUE;
  len = 0;
  for ( i=0; i<string_size; i++ ) {
    if ( string2[i] == 0 ) break;
    if ( string1[i] != ' ' ) blank = FALSE;
    len = i+1;
  }

  if ( blank ) {
    result = 0;
    goto norm_ret;
  }

  MAKE_FIXED_DESCRIP( string2, len, dsc );

  flag = 0;
  context = 0;
  stat = lib$convert_date_string ( &dsc, &time2, &context, &flag, 0, 0 );
  if ( !( stat % 2 ) ) goto err_ret;

  stat = sys_subtract_times( &time1, &time2 );
  if ( stat % 2 )
    result = 1;	/* time1 is later than time2 */
  else
    result = 0;	/* time2 is later than time1 */

norm_ret:
  *status = 1;
  return result;

err_ret:
  *status = stat;
  return 0;

}

static int tt_init = 1;
static $DESCRIPTOR( tt_dev, "tt:" );
static int tt_chan, tt_ef;

int sys_get_char ( void ) {

/* gets one character with noecho */

int stat, buf_size;
unsigned int func = IO$M_NOECHO | IO$_READVBLK;
IOSB_TYPE iosb;
char buf[4];

  if ( tt_init ) {
    tt_chan = 0;
    stat = sys$assign( &tt_dev, &tt_chan, 0, 0, 0 );
    if ( stat != SS$_NORMAL ) return 0;
    tt_init = 0;
    stat = lib$get_ef( &tt_ef );
    if ( stat != SS$_NORMAL ) tt_ef = 0;
  }

  buf_size = 1;
  stat = sys$qiow( tt_ef, tt_chan, func, &iosb, 0, 0, &buf,
   buf_size, 0, 0, 0, 0 );

  if ( stat != SS$_NORMAL )
    return 0;
  else
    return (int) buf[0];

}

int sys_next_char ( void ) {

/* gets next char in typeahead buffer (nondestructive) */

typedef struct {
  short num;
  char first;
  char res_c;
  long res_l;
} TT_TABUF_TYPE;

int stat, buf_size;
unsigned int func = IO$_SENSEMODE | IO$M_TYPEAHDCNT;
IOSB_TYPE iosb;
TT_TABUF_TYPE buf;

  if ( tt_init ) {
    tt_chan = 0;
    stat = sys$assign( &tt_dev, &tt_chan, 0, 0, 0 );
    if ( stat != SS$_NORMAL ) return 0;
    tt_init = 0;
    stat = lib$get_ef( &tt_ef );
    if ( stat != SS$_NORMAL ) tt_ef = 0;
  }

  buf_size = 1;
  stat = sys$qiow( tt_ef, tt_chan, func, &iosb, 0, 0, &buf,
   sizeof(buf), 0, 0, 0, 0 );

  if ( stat != SS$_NORMAL )
    return 0;
  else
    return (int) buf.first;

}

int sys_num_chars ( void ) {

/* gets number of chars in typeahead buffer */

typedef struct {
  short num;
  char first;
  char res_c;
  long res_l;
} TT_TABUF_TYPE;

int stat, buf_size;
unsigned int func = IO$_SENSEMODE | IO$M_TYPEAHDCNT;
IOSB_TYPE iosb;
TT_TABUF_TYPE buf;

  if ( tt_init ) {
    tt_chan = 0;
    stat = sys$assign( &tt_dev, &tt_chan, 0, 0, 0 );
    if ( stat != SS$_NORMAL ) return 0;
    tt_init = 0;
    stat = lib$get_ef( &tt_ef );
    if ( stat != SS$_NORMAL ) tt_ef = 0;
  }

  buf_size = 1;
  stat = sys$qiow( tt_ef, tt_chan, func, &iosb, 0, 0, &buf,
   sizeof(buf), 0, 0, 0, 0 );

  if ( stat != SS$_NORMAL )
    return 0;
  else
    return (int) buf.num;

}
