#include stdlib
#include descrip
#include ssdef
#include lnmdef
#include syidef

#include ipnis_priv
#include iprpc


int ipnis_get_service (
  int num_connect_fails,
  char *generic_service_name,
  char *service_name,
  char *node,
  int *mode
) {

static int init = 1;
static char nodes[2][31+1];
static int modes[] = { IPRPC_K_MASTER, IPRPC_K_LOCAL };
int stat, ret_stat, i, item;
unsigned short len;
struct dsc$descriptor dsc;
char *ptr, buf[127+1];

  if ( init ) {
    strcpy( nodes[0], "134.167.20.34" );
    strcpy( nodes[1], "134.167.20.34" );
    init = 0;
  }

  if ( strstr( generic_service_name, "::" ) ) {

    strncpy( buf, generic_service_name, 127 );
    ptr = strtok( buf, "::" );
    strncpy( node, ptr, 16 );
    ptr = strtok( NULL, "::" );
    strcpy( service_name, ptr );

  }
  else if ( strncmp( generic_service_name,
    "IOSCNR", strlen("IOSCNR") ) == 0 ) {

    i = 0;

    strcpy( service_name, "5100" );
    strncpy( node, nodes[i], 16 );
    *mode = modes[i];

  }
  else if ( strncmp( generic_service_name,
    "REM_CONSOLE", strlen("REM_CONSOLE") ) == 0 ) {

    i = 0;

    strcpy( service_name, "5101" );
    strncpy( node, nodes[i], 16 );
    *mode = modes[i];

  }
  else if ( strncmp( generic_service_name,
    "DBSCANSRV", strlen("DBSCANSRV") ) == 0 ) {

    i = 0;

    strcpy( service_name, "5102" );
    strncpy( node, nodes[i], 16 );
    *mode = modes[i];

  }
  else if ( strncmp( generic_service_name,
    "TEST", strlen("TEST") ) == 0 ) {

    i = 0;

    strcpy( service_name, "5200" );
    strncpy( node, nodes[i], 16 );
    *mode = modes[i];

  }
  else {

    ret_stat = IPNIS_UNKSVC;
    goto err_return;

  }

norm_return:
  return IPNIS_SUCCESS;

err_return:

  strcpy( service_name, "" );
  strcpy( node, "" );
  *mode = 0;

  return ret_stat;

}
