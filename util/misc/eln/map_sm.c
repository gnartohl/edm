/* map_sm.c */

#module map_sm "$Revision$"

/****************************************************************************

 $Source$

 $Revision$           $Date$

 $Author$             $Locker$

 FACILITY:     map_sm

 ABSTRACT:     Routine to map VMEbus memory for a VME300. This code depends
	       on the ELN kernel and the VME300 library VME300.OLB from AEON.

	       If the memory to be mapped is local to this CPU, then it is
	       mapped non-cached.

	       There is a slight problem in determining whether the memory
	       is local. In order to use VME300 memory as shared memory, it
	       is necessary to make an EBUILD entry in the "Memory limit"
	       field of the "Edit System Characteristics" menu item to limit
	       the total memory that may be used by ELN. The problem arises
	       do to the fact that it is not possible to query the size of
	       CPU physical memory (which for a VME300 may be 4 or 8 Meg, and
	       for a VME300E may be 4, 8, or 16 Meg). As a result, the EBUILD
	       entry must be some value above a 4 Meg boundry. In addition
	       the total reserved memory must be less than 4 Meg. With these
	       constraints, the total system memory is deduced thus:

	       EBUILD entry			Total Memory
	       ------------			------------

		value <= 2000h pages		   2000h pages

	        2000h < value <= 4000h pages	   4000h pages

	        6000h < value <= 8000h pages	   8000h pages

 ENVIRONMENT:  VAXELN, kernel mode, P0=, P1=, Kernel Stack=

 BUILD
 REQUIREMENTS: compile with the text librarys eln$:vaxenc.tlb

               cc/noinclude/def=VAXELN

               link with librarys eln$vme300.olb

****************************************************************************/

#include $vaxelnc
#include $kernelmsg
#include $kerneldef
#include stdlib
#include stdio

globalvalue KER$GA_DEVICE_LIST;

static char *g_sm_base = 0;
static int *g_shared_mem_first_page = 0;

static void kernel_map_shared_memory( char *db_name, char **ret_addr ) {

int found_it, i, stat;
int dev_name_len, name_len;
char uc_name[60];
unsigned int vme_base_A24, vme_base_A32;
unsigned int mem_size, free_size, largest_size, total_mem_size;
int sh_mem_start, sh_mem_end;
int vme_space;
unsigned int loc_mem_start, loc_mem_end;
struct scr *head, *cur;
char *next_node;
char *addr;
int *panel_switch, value;
static unsigned int vme300_phys_addr[16] = {
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

/***
/* get physical mem and VMEbus space of this shared region from the system
/* configuration records
*/
  strcpy( uc_name, db_name );
  name_len = strlen(uc_name);
  for ( i=0; i<name_len; i++ ) uc_name[i] = toupper( uc_name[i] );
  found_it = 0;

  head = (struct scr *) KER$GA_DEVICE_LIST;
  cur = head;

  while ( cur->scr$l_next ) {

    next_node = (char *) cur + cur->scr$l_next;
    cur = (struct scr *) next_node;

    dev_name_len = (int) cur->scr$w_size;

    if ( dev_name_len == name_len ) {

      if ( strncmp( uc_name, cur->scr$t_name, dev_name_len ) == 0 ) {
        found_it = 1;
        break;
      }

    }

  }

  if ( !found_it ) goto err_ret;

  sh_mem_start = cur->scr$l_device / 512;			/* in pages */
  sh_mem_end = sh_mem_start + cur->scr$l_device_dependent - 1;	/* in pages */

  /* note that cur->scr$b_adapter_number is one greater than the value */
  /* of the ebuild field "Adapter # / VAXBI node #"                    */
  if ( cur->scr$b_adapter_number == 3 || sh_mem_end > 0x7FFF )
    vme_space = 32;
  else if ( cur->scr$b_adapter_number == 2 || sh_mem_end > 0x7F )
    vme_space = 24;
  else
    goto err_ret;			/* A16 not allowed */

/***
/* get physical mem size and deduce total mem size
*/
  ker$memory_size( &stat, &mem_size, &free_size, &largest_size );

  if ( mem_size <= 0x2000 )
    total_mem_size = 0x2000;
  else if ( mem_size <= 0x4000 )
    total_mem_size = 0x4000;
  else if ( mem_size <= 0x6000 )
    goto err_ret;
  else if ( mem_size <= 0x8000 )
    total_mem_size = 0x8000;
  else
    goto err_ret;

/***
/* get VMEbus base address for this CPU
*/
  vme_base_A24 = 0xFFFFFFFF;
  slave_decode_A24_base( &stat, &vme_base_A24 );
  if ( vme_base_A24 != 0xFFFFFFFF ) vme_base_A24 /= 0x200;    /* cvt to pages */
  if ( stat != 0x08648009 ) goto err_ret;      /* this is AEON's success code */

  vme_base_A32 = 0xFFFFFFFF;
  slave_decode_A32_base( &stat, &vme_base_A32 );
  if ( vme_base_A32 != 0xFFFFFFFF ) vme_base_A32 /= 0x200;    /* cvt to pages */
  if ( stat != 0x08648009 ) goto err_ret;      /* this is AEON's success code */

/***
/* slave_decode_A24_base & slave_decode_A32_base are not working in
/* the 3.2 version of the Aeon kernel - as a result, do the following:
*/

  stat = map_panel_switch( &panel_switch );
  if ( stat != 1 ) goto err_ret;      /* this is AEON's success code */

  value = *panel_switch & 0xF;
  if ( value < 0 || value > 15 ) goto err_ret;

  vme_base_A32 = vme300_phys_addr[value];
  if ( vme_base_A32 != 0xFFFFFFFF ) vme_base_A32 /= 0x200;    /* cvt to pages */

/***
/* consistency check
*/
/*  if ( vme_space == 24 && vme_base_A24 == 0xFFFFFFFF ) goto err_ret; */
/*  if ( vme_space == 32 && vme_base_A32 == 0xFFFFFFFF ) goto err_ret; */

/***
/* compute range of VMEbus memory which will be local
*/
  switch ( vme_space ) {

    case 24:

      if ( vme_base_A24 != 0xFFFFFFFF ) {
        loc_mem_start = vme_base_A24 + mem_size;		/* in pages */
        loc_mem_end = vme_base_A24 + total_mem_size - 1;
        if ( loc_mem_end > 0x7fff ) loc_mem_end = 0x7fff;
      }
      else {
        loc_mem_start = -1;
        loc_mem_end = -1;
      }

      if ( sh_mem_start < loc_mem_start && sh_mem_end >= loc_mem_start )
        goto err_ret;
      if ( sh_mem_start <= loc_mem_end && sh_mem_end > loc_mem_end )
        goto err_ret;

      if ( sh_mem_start >= loc_mem_start && sh_mem_end <= loc_mem_end ) {

        ker$allocate_memory( &stat, &addr,
         cur->scr$l_device_dependent * 0x200, NULL,
         ( (char *) cur->scr$l_device ) + 0x10000000 );
        if ( stat != KER$_SUCCESS ) goto err_ret;

      }
      else {

        stat = vmebus_map_a24_VME( cur->scr$l_device,
         cur->scr$l_device_dependent * 0x200, &addr );
        if ( stat != KER$_SUCCESS ) goto err_ret;

      }
      break;

    case 32:

      if ( vme_base_A32 != 0xFFFFFFFF ) {
        loc_mem_start = vme_base_A32 + mem_size;		/* in pages */
        loc_mem_end = vme_base_A32 + total_mem_size - 1;
        if ( loc_mem_end > 0x7fffff ) loc_mem_end = 0x7fffff;
      }
      else {
        loc_mem_start = -1;
        loc_mem_end = -1;
      }

      if ( sh_mem_start < loc_mem_start && sh_mem_end >= loc_mem_start )
        goto err_ret;
      if ( sh_mem_start <= loc_mem_end && sh_mem_end > loc_mem_end )
        goto err_ret;

      if ( sh_mem_start >= loc_mem_start && sh_mem_end <= loc_mem_end ) {

        ker$allocate_memory( &stat, &addr,
         cur->scr$l_device_dependent * 0x200, NULL,
         ( (char *) cur->scr$l_device ) + 0x10000000 );
        if ( stat != KER$_SUCCESS ) goto err_ret;

      }
      else {

        stat = vmebus_map_a32_VME( cur->scr$l_device,
         cur->scr$l_device_dependent * 0x200, &addr );
        if ( stat != KER$_SUCCESS ) goto err_ret;

      }
      break;

  }

norm_ret:	/* mem is mapped */
  *ret_addr = addr + 512;
  return;

err_ret:
  *ret_addr = 0;
  return;

}

char *map_shared_memory( char *db_name ) {

struct {
  int arg_count;
  char *name;
  char **ret_addr;
} arg_block;

int stat;
char *addr;

  arg_block.arg_count = 2;
  arg_block.name = db_name;
  arg_block.ret_addr = &addr;

  ker$enter_kernel_context( &stat, kernel_map_shared_memory, &arg_block );

  g_sm_base = addr;
  g_shared_mem_first_page = (int *) addr;
  g_shared_mem_first_page -= 128;

  return addr;

}

void get_shared_memory_params(
  char *db_gen_time,
  int *num_channels,
  char *db_name,
  int **chix_table,
  int **data_scratch
) {

int i;
int *iptr;
int temp[25];

  if ( g_sm_base == 0 ) {
    strcpy( db_gen_time, "" );
    *num_channels = 0;
    strcpy( db_name, "" );
    *chix_table = g_sm_base;
    *data_scratch = g_sm_base;
    return;
  }

  *chix_table = (int *) g_sm_base;
  *chix_table += 10021;

  *data_scratch = (int *) g_sm_base;
  *data_scratch += 5011;

  iptr = g_shared_mem_first_page;

  for ( i=0; i<8; i++ ) temp[i] = iptr[i];
  strcpy( db_gen_time, (char *) temp );

  *num_channels = iptr[8];

  iptr += 10;
  for ( i=0; i<25; i++ ) temp[i] = iptr[i];
  strcpy( db_name, (char *) temp );

}

void put_shared_memory_params(
  char *db_gen_time,
  int *num_channels,
  char *db_name
) {

int i;
int *iptr;
int temp[25];

  if ( g_sm_base == 0 ) return;

  iptr = g_shared_mem_first_page;

  strcpy( (char *) temp, db_gen_time );
  for ( i=0; i<8; i++ ) iptr[i] = temp[i];

  iptr[8] = *num_channels;

  iptr += 10;
  strcpy( (char *) temp, db_name );
  for ( i=0; i<25; i++ ) iptr[i] = temp[i];

}

int check_for_sm_db( char *name ) {

static int first = 1;
int i, l1, l2, temp[25], *iptr;
static char sm_db_name[25];

  if ( g_sm_base == 0 ) return 0;

  if ( first ) {
    first = 0;
    iptr = g_shared_mem_first_page;
    iptr += 10;
    for ( i=0; i<25; i++ ) temp[i] = iptr[i];
    strcpy( sm_db_name, (char *) temp );
    for ( i=0; i<strlen(sm_db_name); i++ )
      sm_db_name[i] = toupper( sm_db_name[i] );
  }

  l1 = strlen(name);
  l2 = strlen(sm_db_name);

  if ( l1 == l2 ) {
    for ( i=0; i<l1; i++ ) {
      if ( toupper( name[i] ) != sm_db_name[i] ) return 0;
    }
    return 1;
  }

  if ( l1 < (l2+2) ) return 0;

  for ( i=0; i<l2; i++ ) {
    if ( toupper( name[i] ) != sm_db_name[i] ) return 0;
  }
  if ( name[l2] != ':' && name[l2+1] != ':' ) return 0;

  return 1;

}
