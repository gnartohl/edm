#ifndef __crc_h
#define __crc_h 1

#ifdef __cplusplus
extern "C" {
#endif

unsigned int updateCRC (
  unsigned int crc,
  char *data,
  int n
);

#ifdef __cplusplus
}
#endif

#endif
