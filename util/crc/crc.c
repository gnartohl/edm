#define WIDTH		32
#define POLYNOMIAL	0xd8
#define TOPBIT		( 1 << ( WIDTH - 1 ) )

static unsigned int computeByteCRC (
  unsigned int crc,
  unsigned char c
) {

static unsigned short crcTable[256];
static int init = 1;
unsigned int remainder;
int dividend;
unsigned char bit, data;

  if ( init ) {

    init = 0;

    for ( dividend=0; dividend<256; dividend++ ) {

      remainder = dividend << ( WIDTH - 8 );

      for ( bit=8; bit>0; bit-- ) {

        if ( remainder & TOPBIT ) {
          remainder = ( remainder << 1 ) ^ POLYNOMIAL;
	}
	else {
          remainder = remainder << 1;
	}

      }

      crcTable[dividend] = remainder;

    }

  }

  data = c ^ ( crc >> ( WIDTH - 8 ) );
  return crcTable[data] ^ ( crc << 8 );

}

/*
 * compute CRC
 *
 */

unsigned int updateCRC (
  unsigned int crc,
  char *data,
  int n
) {

unsigned char *p;

  for ( p = (unsigned char *) data; p < (unsigned char *) &data[n]; p++ ) {
    crc = computeByteCRC( crc, *p );
  }

  return crc;

}
