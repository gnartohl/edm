//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#define __pv_c
#include "pv.h"

pvEventClass::pvEventClass ( void )
{

  type = new char[strlen("base")+1];
  strcpy( type, "base" );

}

pvEventClass::pvEventClass ( pvEventClass &source )
{

  type = new char[strlen(source.type)+1];
  strcpy( type, source.type );

}

pvEventClass::~pvEventClass ( void )
{

  delete type;

}

void pvEventClass::clone ( const pvEventClass *source )
{

  type = new char[strlen(source->type)+1];
  strcpy( type, source->type );

}

int pvEventClass::eventAdded ( void )
{

  return 0;

}

pvClass::pvClass ( void )
{

  type = new char[strlen("base")+1];
  strcpy( type, "base" );

  initialConnection = 1;
  connected = 1;

}

pvClass::pvClass (
  pvClass &source )
{

  type = new char[strlen(source.type)+1];
  strcpy( type, source.type );

  initialConnection = source.initialConnection;
  connected = source.connected;

}

pvClass::~pvClass ( void ) {

  delete type;

}

void pvClass::clone (
  const pvClass *source )
{

  type = new char[strlen(source->type)+1];
  strcpy( type, source->type );

  initialConnection = source->initialConnection;
  connected = source->connected;

} 

int pvClass::search (
expStringClass *name )
{

  return PV_E_SUCCESS;

}

int pvClass::searchAndConnect (
  expStringClass *name,
  pvCbFunc callback,
  void *clientData )
{

  return PV_E_SUCCESS;

}

int pvClass::createEventId (
 pvEventClass **ptr )
{

  *ptr = NULL;

  return PV_E_SUCCESS;

}

int pvClass::destroyEventId (
 pvEventClass **ptr )
{

  *ptr = NULL;

  return PV_E_SUCCESS;

}

int pvClass::pendIo (
  float sec )
{

  return PV_E_SUCCESS;

}

int pvClass::pendEvent (
  float sec )
{

  return PV_E_SUCCESS;

}

int pvClass::put (
  int type,
  void *value )
{

  return PV_E_SUCCESS;

}

int pvClass::get (
  int type,
  void *value )
{

  return PV_E_SUCCESS;

}

int pvClass::getCallback (
  int type,
  pvCbFunc callback,
  void *clientData )
{

  return PV_E_SUCCESS;

}

int pvClass::addEvent (
  int type,
  int numElements,
  pvCbFunc callback,
  void *clientData,
  pvEventClass *eventId,
  int eventType )
{

  return PV_E_SUCCESS;

}

int pvClass::checkReconnect ( void )
{

  return PV_E_SUCCESS;

}

char *pvClass::getName ( void )
{

  return NULL;

}

int pvClass::getType ( void )
{

  return 0;

}

int pvClass::getSize ( void )
{

  return 0;

}

int pvClass::maxStringSize( void )
{

  return 0;
  
}

int pvClass::enumStringSize( void )
{

  return 0;
  
}

int pvClass::pvNameSize( void )
{

  return 0;
  
}

int pvClass::clearChannel ( void )
{

  return PV_E_SUCCESS;

}

int pvClass::clearEvent (
  pvEventClass *eventId )
{

  return PV_E_SUCCESS;

}

// event argument manipulation

void *pvClass::getValue (
  void *eventArg )
{

  return NULL;

}

int pvClass::getType (
  void *eventArg )
{

  return 0;

}

int pvClass::getStatus (
  void *eventArg )
{

  return 0;

}

int pvClass::getSeverity (
  void *eventArg )
{

  return 0;

}

void *pvClass::getEventUserData (
  void *eventArg )
{

  return NULL;

}

void *pvClass::getConnectUserData (
  void *connectArg )
{

  return NULL;

}

int pvClass::getPrecision(
  void *eventArg)
{

  return 0;
  
}

char * pvClass::getStateString(
  void *eventArg, int index )
{

  return (char *) NULL; 

}
  
int pvClass::getNumStates(
  void *eventArg )
{

  return 0;

}

int pvClass::getOp (
  void *connectArg )
{

  return 0;

}

double pvClass::getLoOpr (
  void *connectArg )
{

  return 0.0;

}

double pvClass::getHiOpr (
  void *connectArg )
{

  return 0.0;

}

int pvClass::pvErrorCode (
  int code )
{

  return PV_E_SUCCESS;

}
