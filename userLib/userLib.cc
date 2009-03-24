#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "act_grf.h"
#include "act_win.h"
#include "app_pkg.h"

#ifdef __cplusplus
extern "C" {
#endif

int checkElementChange (
  void *ptr
) {

char buf[131+1];

static char *ele_table[] = {
 "H", "HE", "LI", "BE", "B", "C", "N", "O", "F", "NE", "NA", "MG", "AL",
 "SI", "P", "S", "CL", "AR", "K", "CA", "SC", "TI", "V", "CR", "MN", "FE",
 "CO", "NI", "CU", "ZN", "GA", "GE", "AS", "SE", "BR", "KR", "RB", "SR",
 "Y", "ZR", "NB", "MO", "TC", "RU", "RH", "PD", "AG", "CD", "IN", "SN",
 "SB", "TE", "I", "XE", "CS", "BA", "LA", "CE", "PR", "ND", "PM", "SM", "EU",
 "GD", "TB", "DY", "HO", "ER", "TM", "YB", "LU", "HF", "TA", "W", "RE", "OS",
 "IR", "PT", "AU", "HG", "TL", "PB", "BI", "PO", "AT", "RN", "FR", "RA",
 "AC", "TH", "PA", "U", "NP", "PU", "AM", "CM", "BK", "CF", "ES", "FM",
 "MD", "NO", "LR", "RF", "HA", "NH", "NS", "HS", "MT" };

int i, l, size = sizeof(ele_table)/sizeof(char *);

  activeGraphicClass *ago = (activeGraphicClass *) ptr;

  ago->getProperty( "value", 131, buf );

  if ( strlen(buf) ) {
    for ( i=0; i<size; i++ ) {
      if ( strcasecmp( buf, ele_table[i] ) == 0 ) {
        return 0;
      }
    }
  }
  else {
    return 3;
  }

  return 2;

}

#ifdef __cplusplus
}
#endif




