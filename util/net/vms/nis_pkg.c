#include stdlib
#include descrip
#include ssdef
#include lnmdef
#include syidef

#include nis_priv
#include rpc


int nis_get_service (
  int num_connect_fails,
  char *generic_service_name,
  char *service_name,
  char *node,
  int *mode
) {

static int init = 1;
static char nodes[2][31+1];
static int modes[] = { RPC_K_MASTER, RPC_K_LOCAL };
int stat, ret_stat, i, item;
unsigned short len;
struct dsc$descriptor dsc;
char *ptr, buf[127+1];

  if ( init ) {
    strcpy( nodes[0], "ORIB01" );
    strcpy( nodes[1], "ORIB01" );
    init = 0;
  }

  if ( strstr( generic_service_name, "::" ) ) {

    strncpy( buf, generic_service_name, 127 );
    ptr = strtok( buf, "::" );
    strncpy( node, ptr, 7 );
    ptr = strtok( NULL, "::" );
    strcpy( service_name, ptr );

  }
  else if ( strncmp( generic_service_name,
    "IOSCNR", strlen("IOSCNR") ) == 0 ) {

    i = 0;

    strcpy( service_name, "IOS" );
    strncpy( node, nodes[i], 7 );
    *mode = modes[i];

  }
  else if ( strncmp( generic_service_name,
    "REM_CONSOLE", strlen("REM_CONSOLE") ) == 0 ) {

    i = 0;

    strcpy( service_name, "CONSOLE" );
    strncpy( node, nodes[i], 7 );
    *mode = modes[i];

  }
  else if ( strncmp( generic_service_name,
    "DBSCANSRV", strlen("DBSCANSRV") ) == 0 ) {

    i = 0;

    strcpy( service_name, "DBSCANSRV" );
    strncpy( node, nodes[i], 7 );
    *mode = modes[i];

  }
  else if ( strncmp( generic_service_name,
    "TEST", strlen("TEST") ) == 0 ) {

    i = 0;

    strcpy( service_name, "TEST" );
    strncpy( node, nodes[i], 7 );
    *mode = modes[i];

  }
  else {

    ret_stat = NIS_UNKSVC;
    goto err_return;

  }

norm_return:
  return NIS_SUCCESS;

err_return:

  strcpy( service_name, "" );
  strcpy( node, "" );
  *mode = 0;

  return ret_stat;

}
