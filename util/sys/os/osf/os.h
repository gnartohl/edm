#ifndef __os_h
#define __os_h 1

#include "msg.h"
#include "os.msh"
#include "unix.msh"

#ifdef __cplusplus
#extern "C" {
#endif

int os_get_filespec(
  char *logical_file_name,
  char *filespec
);

#ifdef __cplusplus
}
#endif

#endif
