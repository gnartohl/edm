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

#include "pv.h"
#include "cadef.h"

typedef struct __epicsPvEvArgTag {
  void *clientData;
  pvClass *classPtr;
  pvCbFunc callback;
  int requestType;
} __epicsPvEvArgType, *__epicsPvEvArgPtr;

typedef struct eventArgListTag {
  struct eventArgListTag *flink;
  __epicsPvEvArgPtr ptr;
} eventArgListType, *eventArgListPtr;

typedef void (*connectFunc)(struct connection_handler_args);

typedef void (*evHandlerFunc)(struct event_handler_args);

class epicsPvEventClass : public pvEventClass {

public:

evid evId;
eventArgListPtr head, tail;

epicsPvEventClass::epicsPvEventClass ( void );

epicsPvEventClass::epicsPvEventClass ( epicsPvEventClass &source );

epicsPvEventClass::~epicsPvEventClass ( void );

void epicsPvEventClass::clone ( const epicsPvEventClass *source );

int epicsPvEventClass::eventAdded ( void );

};

class epicsPvClass : public pvClass {

typedef struct eventListTag {
  struct eventListTag *flink;
  epicsPvEventClass *evPtr;
} eventListType, *eventListPtr;

public:

int initialConnection;
int connected;
chid id;
__epicsPvEvArgType eventArg;
eventListPtr eventHead, eventTail;
char dummy[4];

epicsPvClass::epicsPvClass ( void );

epicsPvClass::epicsPvClass ( epicsPvClass &source );

epicsPvClass::~epicsPvClass ( void );

void epicsPvClass::clone ( const epicsPvClass *source );

#ifndef __epicsPv_c

int epicsPvClass::pvrShort ( void );
int epicsPvClass::pvrLong ( void );
int epicsPvClass::pvrFloat ( void );
int epicsPvClass::pvrDouble ( void );
int epicsPvClass::pvrEnum ( void );
int epicsPvClass::pvrString ( void );

int epicsPvClass::pvrStsShort ( void );
int epicsPvClass::pvrStsLong ( void );
int epicsPvClass::pvrStsFloat ( void );
int epicsPvClass::pvrStsDouble ( void );
int epicsPvClass::pvrStsEnum ( void );
int epicsPvClass::pvrStsString ( void );

int epicsPvClass::pvrGrShort ( void );
int epicsPvClass::pvrGrLong ( void );
int epicsPvClass::pvrGrFloat ( void );
int epicsPvClass::pvrGrDouble ( void );
int epicsPvClass::pvrGrEnum ( void );
int epicsPvClass::pvrGrString ( void );

int epicsPvClass::pveValue ( void );

int epicsPvClass::pveAlarm ( void );

int epicsPvClass::pvkOpConnUp ( void );

int epicsPvClass::pvkOpConnDown ( void );

#endif

#ifdef __epicsPv_c

int epicsPvClass::pvrShort ( void ) {
  return DBR_SHORT;
}
int epicsPvClass::pvrLong ( void ) {
  return DBR_LONG;
}
int epicsPvClass::pvrFloat ( void ) {
  return DBR_FLOAT;
}
int epicsPvClass::pvrDouble ( void ) {
  return DBR_DOUBLE;
}
int epicsPvClass::pvrEnum ( void ) {
  return DBR_ENUM;
}
int epicsPvClass::pvrString ( void ) {
  return DBR_STRING;
}

int epicsPvClass::pvrStsShort ( void ) {
  return DBR_STS_SHORT;
}
int epicsPvClass::pvrStsLong ( void ) {
  return DBR_STS_LONG;
}
int epicsPvClass::pvrStsFloat ( void ) {
  return DBR_STS_FLOAT;
}
int epicsPvClass::pvrStsDouble ( void ) {
  return DBR_STS_DOUBLE;
}
int epicsPvClass::pvrStsEnum ( void ) {
  return DBR_STS_ENUM;
}
int epicsPvClass::pvrStsString ( void ) {
  return DBR_STS_STRING;
}

int epicsPvClass::pvrGrShort ( void ) {
  return DBR_GR_SHORT;
}
int epicsPvClass::pvrGrLong ( void ) {
  return DBR_GR_LONG;
}
int epicsPvClass::pvrGrFloat ( void ) {
  return DBR_GR_FLOAT;
}
int epicsPvClass::pvrGrDouble ( void ) {
  return DBR_GR_DOUBLE;
}
int epicsPvClass::pvrGrEnum ( void ) {
  return DBR_GR_ENUM;
}
int epicsPvClass::pvrGrString ( void ) {
  return DBR_GR_STRING;
}

int epicsPvClass::pveValue ( void ) {
  return DBE_VALUE;
}

int epicsPvClass::pveAlarm ( void ) {
  return DBE_ALARM;
}

int epicsPvClass::pvkOpConnUp ( void ) {
  return CA_OP_CONN_UP;
}

int epicsPvClass::pvkOpConnDown ( void ) {
  return CA_OP_CONN_DOWN;
}

#endif

int epicsPvClass::search (
  expStringClass *name );

int epicsPvClass::searchAndConnect (
  expStringClass *name,
  pvCbFunc callback,
  void *clientData );

int epicsPvClass::createEventId (
  pvEventClass **ptr );

int epicsPvClass::destroyEventId (
  pvEventClass **ptr );

int epicsPvClass::pendIo (
  float sec );

int epicsPvClass::pendEvent (
  float sec );

int epicsPvClass::put (
  int type,
  void *value );

int epicsPvClass::get (
  int type,
  void *value );

int epicsPvClass::getCallback (
  int type,
  pvCbFunc callback,
  void *clientData );

int epicsPvClass::addEvent (
  int type,
  int numElements,
  pvCbFunc callback,
  void *clientData,
  pvEventClass *eventId,
  int eventType );

int epicsPvClass::checkReconnect ( void );

char *epicsPvClass::getName ( void );

int epicsPvClass::getType ( void );

int epicsPvClass::getSize ( void );

int epicsPvClass::maxStringSize ( void );

int epicsPvClass::enumStringSize ( void );

int epicsPvClass::pvNameSize ( void );

int epicsPvClass::clearChannel ( void );

int epicsPvClass::clearEvent (
  pvEventClass *eventId );

// event argument manipulation

void *epicsPvClass::getValue (
  void *eventArg );

int epicsPvClass::getType (
  void *eventArg );

int epicsPvClass::getStatus (
  void *eventArg );

int epicsPvClass::getSeverity (
  void *eventArg );

int epicsPvClass::getPrecision(
  void *eventArg );

char *epicsPvClass::getStateString(
  void *eventArg, int index );

int epicsPvClass::getNumStates(
  void *eventArg );

void *epicsPvClass::getEventUserData (
  void *eventArg );

// connection argument manipulation

void *epicsPvClass::getConnectUserData (
  void *connectArg );

int epicsPvClass::getOp (
  void *connectArg );

double epicsPvClass::getLoOpr (
  void *eventArg );

double epicsPvClass::getHiOpr (
  void *eventArg );

int epicsPvClass::pvErrorCode (
  int code );

};
