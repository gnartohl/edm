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

#ifndef __pv_h
#define __pv_h 1

#include <stdlib.h>
#include <string.h>

#include "expString.h"

#define PV_E_SUCCESS 1
#define PV_E_FAIL 1000

#define PV_K_VALUE 0x1000
#define PV_K_ALARM 0x1001

// these are from /epics/base/include/alarm.h (EPICS channel access)
// severity
#define PV_K_NO_ALARM 0
#define PV_K_MINOR_ALARM 1
#define PV_K_MAJOR_ALARM 2
#define PV_K_INVALID_ALARM 3
// status
#define PV_K_HIHI_ALARM 3
#define PV_K_HIGH_ALARM 4
#define PV_K_LOLO_ALARM 5
#define PV_K_LOW_ALARM 6
#define PV_K_STATE_ALARM 7
// end of status/severity defs

#define PV_OP_CONN_UP 0x1001
#define PV_OP_CONN_DOWN 0x1002

#define PV_TYPE_STS_LONG 0x2000
#define PV_TYPE_STS_REAL 0x2001
#define PV_TYPE_STS_DOUBLE 0x2002
#define PV_TYPE_STS_ENUM 0x2003
#define PV_TYPE_STS_STRING 0x2004

#define PV_TYPE_GR_LONG 0x2005
#define PV_TYPE_GR_REAL 0x2006
#define PV_TYPE_GR_DOUBLE 0x2007
#define PV_TYPE_GR_ENUM 0x2008
#define PV_TYPE_GR_STRING 0x2009

#define PV_TYPE_LONG 0x200A
#define PV_TYPE_REAL 0x200B
#define PV_TYPE_DOUBLE 0x200C
#define PV_TYPE_ENUM 0x200D
#define PV_TYPE_STRING 0x200E

class pvClass;

typedef void (*pvCbFunc)( pvClass *classPtr, void *clientData,
 void *eventArg );

class pvEventClass {

public:

char *type;

pvEventClass::pvEventClass ( void );

pvEventClass::pvEventClass ( pvEventClass &source );

virtual pvEventClass::~pvEventClass ( void );

virtual void pvEventClass::clone ( const pvEventClass *source );

virtual int pvEventClass::eventAdded ( void );

};

class pvClass {

protected:

char *type;
int initialConnection;
int connected;

public:

pvClass::pvClass ( void );

pvClass::pvClass ( pvClass &source );

virtual pvClass::~pvClass ( void );

void pvClass::clone ( const pvClass *source );

#ifndef __pv_c

virtual int pvClass::pvrShort ( void );
virtual int pvClass::pvrLong ( void );
virtual int pvClass::pvrFloat ( void );
virtual int pvClass::pvrDouble ( void );
virtual int pvClass::pvrEnum ( void );
virtual int pvClass::pvrString ( void );

virtual int pvClass::pvrStsShort ( void );
virtual int pvClass::pvrStsLong ( void );
virtual int pvClass::pvrStsFloat ( void );
virtual int pvClass::pvrStsDouble ( void );
virtual int pvClass::pvrStsEnum ( void );
virtual int pvClass::pvrStsString ( void );

virtual int pvClass::pvrGrShort ( void );
virtual int pvClass::pvrGrLong ( void );
virtual int pvClass::pvrGrFloat ( void );
virtual int pvClass::pvrGrDouble ( void );
virtual int pvClass::pvrGrEnum ( void );
virtual int pvClass::pvrGrString ( void );

virtual int pvClass::pveValue ( void );

virtual int pvClass::pveAlarm ( void );

virtual int pvClass::pvkOpConnUp ( void );

virtual int pvClass::pvkOpConnDown ( void );

#endif

#ifdef __pv_c

virtual int pvClass::pvrShort ( void ) {
  return 0;
}
virtual int pvClass::pvrLong ( void ) {
  return 0;
}
virtual int pvClass::pvrFloat ( void ) {
  return 0;
}
virtual int pvClass::pvrDouble ( void ) {
  return 0;
}
virtual int pvClass::pvrEnum ( void ) {
  return 0;
}
virtual int pvClass::pvrString ( void ) {
  return 0;
}

virtual int pvClass::pvrStsShort ( void ) {
  return 0;
}
virtual int pvClass::pvrStsLong ( void ) {
  return 0;
}
virtual int pvClass::pvrStsFloat ( void ) {
  return 0;
}
virtual int pvClass::pvrStsDouble ( void ) {
  return 0;
}
virtual int pvClass::pvrStsEnum ( void ) {
  return 0;
}
virtual int pvClass::pvrStsString ( void ) {
  return 0;
}

virtual int pvClass::pvrGrShort ( void ) {
  return 0;
 }
virtual int pvClass::pvrGrLong ( void ) {
  return 0;
}
virtual int pvClass::pvrGrFloat ( void ) {
  return 0;
}
virtual int pvClass::pvrGrDouble ( void ) {
  return 0;
}
virtual int pvClass::pvrGrEnum ( void ) {
  return 0;
}
virtual int pvClass::pvrGrString ( void ) {
  return 0;
}

virtual int pvClass::pveValue ( void ) {
  return 0;
}

virtual int pvClass::pveAlarm ( void ) {
  return 0;
}

virtual int pvClass::pvkOpConnUp ( void ) {
  return 0;
}

virtual int pvClass::pvkOpConnDown ( void ) {
  return 0;
}

#endif

virtual int pvClass::search (
  expStringClass *name );

virtual int pvClass::searchAndConnect (
  expStringClass *name,
  pvCbFunc callback,
  void *clientData );

virtual int pvClass::createEventId (
  pvEventClass **ptr );

virtual int pvClass::destroyEventId (
  pvEventClass **ptr );

virtual int pvClass::pendIo (
  float sec );

virtual int pvClass::pendEvent (
  float sec );

virtual int pvClass::put (
  int type,
  void *value );

virtual int pvClass::get (
  int type,
  void *value );

virtual int pvClass::getCallback (
  int type,
  pvCbFunc callback,
  void *clientData );

virtual int pvClass::addEvent (
  int type,
  int numElements,
  pvCbFunc callback,
  void *clientData,
  pvEventClass *eventId,
  int eventType );

virtual int pvClass::checkReconnect ( void );

virtual char *pvClass::getName ( void );

virtual int pvClass::getType ( void );

virtual int pvClass::getSize ( void );

virtual int pvClass::maxStringSize ( void );

virtual int pvClass::enumStringSize ( void );

virtual int pvClass::pvNameSize ( void );

virtual int pvClass::clearChannel ( void );

virtual int pvClass::clearEvent (
  pvEventClass *eventId );

// event argument manipulation

virtual void *pvClass::getValue (
  void *eventArg );

virtual int pvClass::getType (
  void *eventArg );

virtual int pvClass::getStatus (
  void *eventArg );

virtual int pvClass::getSeverity (
  void *eventArg );

virtual int pvClass::getPrecision(
  void *eventArg );

virtual char *pvClass::getStateString(
  void *eventArg, int index );
  
virtual int pvClass::getNumStates(
  void *eventArg );

virtual void *pvClass::getEventUserData (
  void *eventArg );

virtual void *pvClass::getConnectUserData (
  void *connectArg );

virtual int pvClass::getOp (
  void *connectArg );

virtual double pvClass::getLoOpr (
  void *connectArg );

virtual double pvClass::getHiOpr (
  void *connectArg );

virtual int pvClass::pvErrorCode (
  int code );

};

  //class factory functions
  //-----------------------
class pvClass *createPV (
  char *className );

class pvClass *clonePV (
  char *className,
  class pvClass *sourcePV );

#endif
