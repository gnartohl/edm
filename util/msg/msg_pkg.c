#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "os.h"
#include "msg_priv.h"


int msg_severity(
  int error_code
) {

  return ( error_code & 0xF0000000 );

}

int msg_facility(
  int error_code
) {

  return ( error_code & 0xFFF000 );

}

int msg_code(
  int error_code
) {

  return ( error_code & 0xFFF );

}

void msg_get_text(
  int error_code,
  char *sym_code,
  char *error_text
) {

FILE *msg_file;
char string[127+1];
char *str;
int i, first, code, stat;

  stat = os_get_filespec( "util_err_msg_file", string );
  if ( stat != OS_SUCCESS ) {
    strcpy( sym_code, "???" );
    strcpy( error_text, "Message file does not exist!" );
    return;
  }

  msg_file = fopen( string, "r" );
  if ( !msg_file ) {
    strcpy( sym_code, "???" );
    strcpy( error_text, "Message file cannot be opened!" );
    return;
  }

  while ( !feof(msg_file) ) {

    fscanf( msg_file, "%x %s %[^\n]", &code, sym_code, string );
    if ( code == error_code ) {
      if ( string[0] == '~' )
        first = 1;
      else
        first = 0;
      strcpy( error_text, &string[first] );
      str = strstr( error_text, "\\n" );
      while ( str ) {
        for ( i=0; i<strlen(str); i++ ) str[i] = str[i+1];
        str[0] = '\n';
        str = strstr( error_text, "\\n" );
      }
      fclose( msg_file );
      return;
    }

  }

  fclose( msg_file );

  strcpy( sym_code, "???" );
  strcpy( error_text, "Unknown message" );

}

void msg_show_error_message(
  int error_code
) {

char string[127+1];
char symbollic_code[40+1];
char sub[60+1];
int i, msg_fac, fac, severity, need_sep;

  need_sep = 0;

  strcpy( string, "%" );
  strcat( string, MSG_MAIN_FACILITY );
  for ( i=0; i<strlen(string); i++ ) string[i] = toupper( string[i] );
  need_sep = 1;

  fac = msg_facility( error_code );

  strcpy( sub, "???" );
  msg_fac = 0;
  for ( i=0; i<MSG_MAX_FAC; i++ ) {
    MSG_GET_FAC(i,msg_fac);
    if ( fac == msg_fac ) {
      MSG_GET_STR(i,sub);
      break;
    }
  }

  if ( need_sep ) strcat( string, "-" );
  strcat( string, sub );
  need_sep = 1;

  severity = msg_severity( error_code );

  switch ( severity ) {
    case _INF_:
      strcpy( sub, "I" );
      break;
    case _WRN_ :
      strcpy( sub, "W" );
      break;
    case _ERR_:
      strcpy( sub, "E" );
      break;
    case _MSG_:
      strcpy( sub, "M" );
      break;
    default:
      strcpy( sub, "?" );
      break;
  }
  if ( need_sep ) strcat( string, "-" );
  strcat( string, sub );
  need_sep = 1;

  msg_get_text( error_code, symbollic_code, sub );

  if ( need_sep ) strcat( string, "-" );
  strcat( string, symbollic_code );
  need_sep = 1;

  if ( error_code == UNIX_ERROR ) {

    perror( string );

  }
  else {

    if ( need_sep ) strcat( string, ", " );
    strcat( string, sub );
    if ( strlen(string) > 0 ) {
      printf( "%s\n", string );
    }

  }

}
