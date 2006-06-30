#ifndef __pvAction_h
#define __pvAction_h 1

// class that manages a list of pv-related actions

#include <stdio.h>
#include "expString.h"
#include "thread.h"
#include "utility.h"
#include "environment.str"

typedef struct threadParamBlockTag {
  int multipleInstancesAllowed;
  char *cmd;
  float secondsToDelay;
} threadParamBlockType, *threadParamBlockPtr;

class pvActionClass {

public:

pvActionClass ( void );

virtual ~pvActionClass ( void );

int numActions ( void );

void setPv ( char *pvName );

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

int n;
char **name;
char **action;
expStringClass *expAction;
THREAD_HANDLE thread;
threadParamBlockPtr threadParamBlock;

};

#endif
