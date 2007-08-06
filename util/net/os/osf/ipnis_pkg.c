#include <stdlib.h>
#include <ctype.h>

#include "ipnis_priv.h"
#include "iprpc.h"


int ipnis_get_service (
  int num_connect_fails,
  char *generic_service_name,
  char *service_name,
  char *node,
  int *mode
) {

static char *g_nodes[2] = { "134.167.20.89", "134.167.21.95" };
static int g_modes[] = { RPC_K_MASTER, RPC_K_LOCAL };
int i;

  if ( strncmp( generic_service_name,
    "IOSCNR", strlen("IOSCNR") ) == 0 ) {

    i = 0;

    strcpy( service_name, "5200" );
    strncpy( node, "134.167.21.95", strlen("134.167.21.95") );
    *mode = g_modes[i];

  }
  else if ( strncmp( generic_service_name,
    "TEST", strlen("TEST") ) == 0 ) {

    i = 0;

    strcpy( service_name, "5200" );
    strncpy( node, "134.167.20.34", strlen("134.167.20.34") );
    *mode = g_modes[i];

  }
  else {

    for ( i=0; i<=strlen(generic_service_name); i++ ) {
      service_name[i] = toupper(generic_service_name[i]);
    }

    i = 0;
    strncpy( node, g_nodes[i], strlen(g_nodes[i]) );
    *mode = g_modes[i];

  }

  return IPNIS_SUCCESS;

}
