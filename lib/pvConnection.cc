#define __pvConnection_cc 1

#include "pvConnection.h"

pvConnectionClass::pvConnectionClass ( void )
{

  maxPvs = 0;
  numPvs = 0;
  id = NULL;
  bit = NULL;
  connectionMask.reset();
  numConnectionsExpected = 0;

}

pvConnectionClass::~pvConnectionClass ( void )
{

  if ( id ) delete[] id;
  if ( bit ) delete[] bit;

}

int pvConnectionClass::findPv (
  void *Pv )
{

  int i;

  for ( i=0; i<numPvs; i++ ) {
    if ( id[i] == Pv ) {
      return i;
    }
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

  if ( _maxPvs > 1000 ) return 0; // error

  if ( maxPvs ) return 0; // error

  maxPvs = _maxPvs;
  id = new void *[maxPvs];
  bit = new short[maxPvs];

  for ( i=0; i<maxPvs; i++ ) {
    id[i] = NULL;
    bit[i] = i;
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
    if ( i == -1 ) {
      return 0; // error
    }
  }

  connectionMask.reset( bit[i] );

  return 1;

}

int pvConnectionClass::setPvDisconnected (
  void *Pv )
{

int i;

  // if we are disconnecting a pv not yet in the list, simply return
  // this may happen for calc pvs

  i = findPv( Pv );
  if ( i == -1 ) {
    return 1;
  }

  connectionMask.set( bit[i] );

  return 1;

}

void pvConnectionClass::init ( void )
{

int i;

  numConnectionsExpected = 0;
  connectionMask.reset();
  numPvs = 0;

  for ( i=0; i<maxPvs; i++ ) {
    id[i] = NULL;
  }

}

int pvConnectionClass::addPv ( void )
{

  if ( numConnectionsExpected >= maxPvs ) return 0; //error

  connectionMask.set( bit[numConnectionsExpected] );
  numConnectionsExpected++;

  return 1;

}

int pvConnectionClass::pvsConnected ( void )
{

  if ( connectionMask.none() ) {
    return 1;
  }

  return 0;

}
