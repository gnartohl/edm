#ifndef __pvConnection_h
#define __pvConnection_h 1

#include "math.h"
#include "stdio.h"

class pvConnectionClass {

private:

int maxPvs;
int numPvs;
int numConnectionsExpected;
void **id; // dynamic array
unsigned int *mask; // dynamic array
unsigned int connectionMask;

int findPv (
  void *Pv );

int addPvToList (
  void *Pv );

public:

pvConnectionClass ( void );

~pvConnectionClass ( void );

int setMaxPvs (
  int _maxPvs );

int setPvConnected (
  void *Pv );

int setPvDisconnected (
  void *Pv );

void init ( void );

int addPv ( void );

int pvsConnected ( void );

};

#endif
