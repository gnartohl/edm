/***
/* includes
*/
#include $vaxelnc
#include $kernelmsg

static void kernel_mode_halt_processor( void ) {

/* this is wrong - need to fix it */
#define SCRATCH_RAM_PHYS_ADDR 0x0
#define ONE_PAGE 512

int stat, *addr;

  /* for now, lets just access violate */
  addr = 0;
  *addr = 1;	/* should die here */

  ker$allocate_system_region( &stat, &addr, ONE_PAGE, SCRATCH_RAM_PHYS_ADDR );

  *addr |= 3;	/* set halt code to "halt" (fisrt two bits in console mbox) */
  _HALT();

}

void halt_processor( void ) {

struct {
  int num_args;
} arg_block;

  arg_block.num_args = 0;
  ker$enter_kernel_context( NULL, kernel_mode_halt_processor, &arg_block );

}
