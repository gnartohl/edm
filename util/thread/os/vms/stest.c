#include stdlib
#include stdio
#include <cma.h>

static cma_t_attr g_test_attr;

void test ( void ) {

cma_t_natural size;
int array[30000], i;

  for ( i=0; i<30000; i++ ) {
    array[i] = i;
  }

  cma_attr_get_stacksize( &g_test_attr, &size );

}

main() {

cma_t_attr attr;
cma_t_natural size;
cma_t_thread tid;
int result, stat;

  attr = cma_c_null;

  cma_attr_create( &g_test_attr, &attr );
  cma_attr_get_stacksize( &g_test_attr, &size );
  cma_attr_get_guardsize( &g_test_attr, &size );

  size = 31000 * 4;
  cma_attr_set_stacksize( &g_test_attr, size );

  cma_thread_create( &tid, &g_test_attr, test, NULL );

  cma_thread_join( &tid, &stat, &result );

}
