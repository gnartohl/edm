/* fixvic.c */

#module fixvic "$Revision$"

/*******************************************************************************

 $Source$

 $Revision$				$Date$

 $Author$				$Locker$

 ABSTRACT:              Program that touches the VME300 VIC chip to
			deassert sysfail.

 ENVIRONMENT:           VAXELN, kernel mode, kernel stack=4, priority=5

 BUILD
 REQUIREMENTS:          $_Build: /ELN/DEPENDS_ON=([---]util.tlb)$

 TEST DETAILS:          

*******************************************************************************/

#include $vaxelnc
#include vic

main() {

int stat;
unsigned char vic_reg;
struct vic *vicP;

/***
/* deassert sysfail
*/
  vic_address( &vicP );
  vic_reg = read_register( &vicP->vic_ipc6 );
  vic_reg &= ~( 0x40 ); /* clear bit 6 */
  write_register( vic_reg, &vicP->vic_ipc6 );

/***
/* reenable scheduling
*/
  ker$initialization_done( &stat );

}
