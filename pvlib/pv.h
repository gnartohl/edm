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

pvEventClass ( void );

pvEventClass ( pvEventClass &source );

virtual ~pvEventClass ( void );

virtual void clone ( const pvEventClass *source );

virtual int eventAdded ( void );

};

class pvClass {

protected:

char *type;
int initialConnection;
int connected;

public:

pvClass ( void );

pvClass ( pvClass &source );

virtual ~pvClass ( void );

void clone ( const pvClass *source );

#ifndef __pv_c

virtual int pvrShort ( void );
virtual int pvrLong ( void );
virtual int pvrFloat ( void );
virtual int pvrDouble ( void );
virtual int pvrEnum ( void );
virtual int pvrString ( void );

virtual int pvrStsShort ( void );
virtual int pvrStsLong ( void );
virtual int pvrStsFloat ( void );
virtual int pvrStsDouble ( void );
virtual int pvrStsEnum ( void );
virtual int pvrStsString ( void );

virtual int pvrGrShort ( void );
virtual int pvrGrLong ( void );
virtual int pvrGrFloat ( void );
virtual int pvrGrDouble ( void );
virtual int pvrGrEnum ( void );
virtual int pvrGrString ( void );

virtual int pveValue ( void );

virtual int pveAlarm ( void );

virtual int pvkOpConnUp ( void );

virtual int pvkOpConnDown ( void );

#endif

#ifdef __pv_c

virtual int pvrShort ( void ) {
  return 0;
}
virtual int pvrLong ( void ) {
  return 0;
}
virtual int pvrFloat ( void ) {
  return 0;
}
virtual int pvrDouble ( void ) {
  return 0;
}
virtual int pvrEnum ( void ) {
  return 0;
}
virtual int pvrString ( void ) {
  return 0;
}

virtual int pvrStsShort ( void ) {
  return 0;
}
virtual int pvrStsLong ( void ) {
  return 0;
}
virtual int pvrStsFloat ( void ) {
  return 0;
}
virtual int pvrStsDouble ( void ) {
  return 0;
}
virtual int pvrStsEnum ( void ) {
  return 0;
}
virtual int pvrStsString ( void ) {
  return 0;
}

virtual int pvrGrShort ( void ) {
  return 0;
 }
virtual int pvrGrLong ( void ) {
  return 0;
}
virtual int pvrGrFloat ( void ) {
  return 0;
}
virtual int pvrGrDouble ( void ) {
  return 0;
}
virtual int pvrGrEnum ( void ) {
  return 0;
}
virtual int pvrGrString ( void ) {
  return 0;
}

virtual int pveValue ( void ) {
  return 0;
}

virtual int pveAlarm ( void ) {
  return 0;
}

virtual int pvkOpConnUp ( void ) {
  return 0;
}

virtual int pvkOpConnDown ( void ) {
  return 0;
}

#endif

virtual int search (
  expStringClass *name );

virtual int searchAndConnect (
  expStringClass *name,
  pvCbFunc callback,
  void *clientData );

virtual int createEventId (
  pvEventClass **ptr );

virtual int destroyEventId (
  pvEventClass **ptr );

virtual int pendIo (
  float sec );

virtual int pendEvent (
  float sec );

virtual int put (
  int type,
  void *value );

virtual int get (
  int type,
  void *value );

virtual int getCallback (
  int type,
  pvCbFunc callback,
  void *clientData );

virtual int addEvent (
  int type,
  int numElements,
  pvCbFunc callback,
  void *clientData,
  pvEventClass *eventId,
  int eventType );

virtual int checkReconnect ( void );

virtual char *getName ( void );

virtual int getType ( void );

virtual int getSize ( void );

virtual int maxStringSize ( void );

virtual int enumStringSize ( void );

virtual int pvNameSize ( void );

virtual int clearChannel ( void );

virtual int clearEvent (
  pvEventClass *eventId );

// event argument manipulation

virtual void *getValue (
  void *eventArg );

virtual int getType (
  void *eventArg );

virtual int getStatus (
  void *eventArg );

virtual int getSeverity (
  void *eventArg );

virtual int getPrecision(
  void *eventArg );

virtual char *getStateString(
  void *eventArg, int index );
  
virtual int getNumStates(
  void *eventArg );

virtual void *getEventUserData (
  void *eventArg );

virtual void *getConnectUserData (
  void *connectArg );

virtual int getOp (
  void *connectArg );

virtual double getLoOpr (
  void *connectArg );

virtual double getHiOpr (
  void *connectArg );

virtual int pvErrorCode (
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
