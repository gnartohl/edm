#include $vaxelnc

void get_local_crate_id( int *local_crate_id ) {

globalref KER$GL_SYSTEM_PARAMETER4;	/* from Ebuild system characteristics */

  *local_crate_id = KER$GL_SYSTEM_PARAMETER4;

}
