#ifndef __pvConnection_h
#define __pvConnection_h 1

#include "math.h"
#include "cadef.h"

class pvConnectionClass {

private:

int maxPvs;
int numPvs;
int numConnectionsExpected;
void **id; // dynamic array
chid *pvId; // dynamic array
unsigned int *mask; // dynamic array
unsigned int connectionMask;

int pvConnectionClass::findPv (
  chid Pv );

int pvConnectionClass::addPvToList (
  chid Pv );

int pvConnectionClass::findPv (
  void *Pv );

int pvConnectionClass::addPvToList (
  void *Pv );

public:

pvConnectionClass::pvConnectionClass ( void );

pvConnectionClass::~pvConnectionClass ( void );

int pvConnectionClass::setMaxPvs (
  int _maxPvs );

int pvConnectionClass::setPvConnected (
  chid Pv );

int pvConnectionClass::setPvDisconnected (
  chid Pv );

int pvConnectionClass::setPvConnected (
  void *Pv );

int pvConnectionClass::setPvDisconnected (
  void *Pv );

void pvConnectionClass::init ( void );

int pvConnectionClass::addPv ( void );

int pvConnectionClass::pvsConnected ( void );

};

#endif