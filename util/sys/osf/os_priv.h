#ifndef __os_priv_h
#define __os_priv_h 1

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "os.h"
#include "unix.msh"

#define OS_K_SUCCESS	0
#define OS_K_WARNING	1
#define OS_K_FAIL	2
#define OS_K_ABORT	3

typedef struct {
  ino_t unique_id[1];
} PRIV_UNIQUE_FILE_ID_T;
typedef PRIV_UNIQUE_FILE_ID_T *PRIV_UNIQUE_FILE_ID_P;

#endif
