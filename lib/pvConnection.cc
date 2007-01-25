#define __pvConnection_cc 1

#include "pvConnection.h"

pvConnectionClass::pvConnectionClass ( void )
{

  maxPvs = 0;
  numPvs = 0;
  id = NULL;
  mask = NULL;
  connectionMask = 0;
  numConnectionsExpected = 0;

}

pvConnectionClass::~pvConnectionClass ( void )
{

  if ( id ) delete[] id;
  if ( mask ) delete[] mask;

}

int pvConnectionClass::findPv (
  void *Pv )
{

  int i;

  for ( i=0; i<numPvs; i++ ) {
    if ( id[i] == Pv ) return i;
  }

  return -1;

}

int pvConnectionClass::addPvToList (
  void *Pv )
{

int i;

  if ( numPvs == maxPvs ) return -1; // error

  i = findPv( Pv );
  if ( i != -1 ) return i; // already in list

  i = numPvs;
  id[i] = Pv;
  numPvs++;

  return i;

}

int pvConnectionClass::setMaxPvs (
  int _maxPvs )
{

int i;

  if ( maxPvs ) return 0; // error

  maxPvs = _maxPvs;
  id = new void *[maxPvs];
  mask = new unsigned int[maxPvs];

  for ( i=0; i<maxPvs; i++ ) {
    id[i] = NULL;
    //mask[i] = (unsigned int) pow(2,i); <-- solaris compile prob
    mask[i] = 1 << i;
  }

  return 1;

}

int pvConnectionClass::setPvConnected (
  void *Pv )
{

int i;

  i = findPv( Pv );
  if ( i == -1 ) {
    i = addPvToList( Pv );
    if ( i == -1 ) return 0; // error
  }

  connectionMask &= ~(mask[i]);

  return 1;

}

int pvConnectionClass::setPvDisconnected (
  void *Pv )
{

int i;

  i = findPv( Pv );
  if ( i == -1 ) {
    i = addPvToList( Pv );
    if ( i == -1 ) return 0; // error
  }

  connectionMask |= mask[i];

  return 1;

}

void pvConnectionClass::init ( void )
{

int i;

  numConnectionsExpected = 0;
  connectionMask = 0;
  numPvs = 0;

  for ( i=0; i<maxPvs; i++ ) {
    id[i] = NULL;
  }

}

int pvConnectionClass::addPv ( void )
{

  if ( numConnectionsExpected >= maxPvs ) return 0; //error

  connectionMask |= mask[numConnectionsExpected];
  numConnectionsExpected++;

  return 1;

}

int pvConnectionClass::pvsConnected ( void )
{

  return !connectionMask;

}
