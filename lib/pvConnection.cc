#define __pvConnection_cc 1

#include "pvConnection.h"

pvConnectionClass::pvConnectionClass ( void )
{

  maxPvs = 0;
  numPvs = 0;
  pvId = NULL;
  mask = NULL;
  connectionMask = 0;
  numConnectionsExpected = 0;

}

pvConnectionClass::~pvConnectionClass ( void )
{

  if ( pvId ) delete pvId;
  if ( mask ) delete mask;

}

int pvConnectionClass::findPv (
  chid Pv )
{

  int i;

  for ( i=0; i<numPvs; i++ ) {
    if ( pvId[i] == Pv ) return i;
  }

  return -1;

}

int pvConnectionClass::addPvToList (
  chid Pv )
{

int i;

  if ( numPvs == maxPvs ) return -1; // error

  i = findPv( Pv );
  if ( i != -1 ) return i; // already in list

  i = numPvs;
  pvId[i] = Pv;
  numPvs++;

  return i;

}

int pvConnectionClass::setMaxPvs (
  int _maxPvs )
{

int i;

  if ( maxPvs ) return 0; // error

  maxPvs = _maxPvs;
  pvId = new chid[maxPvs];
  mask = new (unsigned int)[maxPvs];

  for ( i=0; i<maxPvs; i++ ) {
    pvId[i] = NULL;
    mask[i] = (unsigned int) pow(2,i);
  }

  return 1;

}

int pvConnectionClass::setPvConnected (
  chid Pv )
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
  chid Pv )
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
    pvId[i] = NULL;
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
