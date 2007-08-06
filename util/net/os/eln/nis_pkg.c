#include stdlib
#include descrip
#include $vaxelnc
#include $kernelmsg

#include nis_priv
#include rpc

globalref VARYING_STRING(6) KER$GT_NODE_NAME;

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
char eln_node_name[10+1];
char *ptr, buf[127+1];

  if ( init ) {
    strcpy( nodes[0], "45.655" );
    strcpy( nodes[1], "45.655" );
    init = 0;
  }

  VARYING_TO_CSTRING( KER$GT_NODE_NAME, eln_node_name );

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

    strcpy( service_name, eln_node_name );
    strcat( service_name, "_IOS_MSG" );
    strncpy( node, nodes[i], 7 );
    *mode = modes[i];

  }
  else if ( strncmp( generic_service_name,
    "REM_CONSOLE", strlen("REM_CONSOLE") ) == 0 ) {

    i = 0;

    strcpy( service_name, eln_node_name );
    strcat( service_name, "_CONSOLE" );
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
