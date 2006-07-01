#ifndef __pvAction_h
#define __pvAction_h 1

// class that manages a list of pv-related actions

#include <stdio.h>
#include "expString.h"
#include "thread.h"
#include "utility.h"
#include "environment.str"

class pvActionClass {

public:

pvActionClass ( void );

virtual ~pvActionClass ( void );

int numActions ( void );

void setInfo (
  char *pvName,
  char *displayName
);

char *getActionName (
  int index
);

char *getAction (
  int index
);

void executeAction (
  int index
);

private:

typedef struct threadParamBlockTag {
  int multipleInstancesAllowed;
  char *cmd;
  float secondsToDelay;
} threadParamBlockType, *threadParamBlockPtr;

int n;
char **name;
char **action;
expStringClass *expAction;
THREAD_HANDLE thread;
threadParamBlockPtr threadParamBlock;

#ifdef __linux__
static void *executeThread (
  THREAD_HANDLE h
);
#endif

#ifdef darwin
static void *executeThread (
  THREAD_HANDLE h
);
#endif

#ifdef __solaris__
static void *executeThread (
  THREAD_HANDLE h
);
#endif

#ifdef __osf__
static void executeThread (
  THREAD_HANDLE h
);
#endif

#ifdef HP_UX
static void *executeThread (
  THREAD_HANDLE h
);
#endif

};

#endif
