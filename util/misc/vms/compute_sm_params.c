#include stdlib
#include stdio

main() {

int panel_switch_setting;
int num_pages_desired;
int size, vme300_total_pages, memory_limit, register_addr;
char input[20+1];
unsigned int vme300_base_addr[16] = {
  0x2000000,
  0x2800000,
  0x3000000,
  0x3800000,
  0x4000000,
  0x4800000,
  0x5000000,
  0x5800000,
  0x6000000,
  0x6800000,
  0x7000000,
  0x7800000,
  0x8000000,
  0x8800000,
  0x9000000,
  0xFFFFFFFF
};

  do {
    printf( "Enter VME300 memory size (4,8,16): " );
    gets( input );
    size = atoi( input );
  } while ( size != 4 && size != 8 && size != 16 );
  switch ( size ) {
    case 4:
      vme300_total_pages = 8192;
      break;
    case 8:
      vme300_total_pages = 16384;
      break;
    case 16:
      vme300_total_pages = 32768;
      break;
  }
    
  do {
    printf( "Enter number of bytes to reserve as shared memory:" );
    gets( input );
    size = atoi( input );
    num_pages_desired = size / 512;
    if ( (size%512) != 0 ) num_pages_desired++;
  } while ( num_pages_desired < 2 || num_pages_desired > 8192 );

  memory_limit = vme300_total_pages - num_pages_desired;
    
  do {
    printf( "Enter VME300 panel switch setting (0-E): " );
    gets( input );
    sscanf( input, "%x", &panel_switch_setting );
  } while ( panel_switch_setting < 0 || panel_switch_setting > 14 );

  register_addr = vme300_base_addr[panel_switch_setting] + memory_limit * 512;

  printf( " \n" );
  printf( "Ebuild parameters:\n\n" );

  printf( "Edit System Characteristics\n" );
  printf( "   Memory limit = %d\n\n", memory_limit );

  printf( "Edit Device Descriptions/SHARED_MEMORY\n" );
  printf( "   Register address = %%O%O\n\n", register_addr );

  printf( "Edit Device Descriptions/SHARED_MEMORY\n" );
  printf( "   Device-dependent parameter = %%X%X\n\n", num_pages_desired );

}
