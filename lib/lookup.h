#ifndef __lookup_h
#define __lookup_h 1

void getFirstFileNameExt (
  char *spec,
  int maxName,
  char *name,
  int maxExt,
  char *ext,
  int *found
);

void getNextFileNameExt (
  char *spec,
  int maxName,
  char *name,
  int maxExt,
  char *ext,
  int *found
);

void getFirstFile (
  char *spec,
  int maxName,
  char *name,
  int *found
);

void getNextFile (
  char *spec,
  int maxName,
  char *name,
  int *found
);

#endif
