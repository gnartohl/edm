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

epicsPvEventClass ( void );

epicsPvEventClass ( epicsPvEventClass &source );

~epicsPvEventClass ( void );

void clone ( const epicsPvEventClass *source );

int eventAdded ( void );

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

epicsPvClass ( void );

epicsPvClass ( epicsPvClass &source );

~epicsPvClass ( void );

void clone ( const epicsPvClass *source );

#ifndef __epicsPv_c

int pvrShort ( void );
int pvrLong ( void );
int pvrFloat ( void );
int pvrDouble ( void );
int pvrEnum ( void );
int pvrString ( void );

int pvrStsShort ( void );
int pvrStsLong ( void );
int pvrStsFloat ( void );
int pvrStsDouble ( void );
int pvrStsEnum ( void );
int pvrStsString ( void );

int pvrGrShort ( void );
int pvrGrLong ( void );
int pvrGrFloat ( void );
int pvrGrDouble ( void );
int pvrGrEnum ( void );
int pvrGrString ( void );

int pveValue ( void );

int pveAlarm ( void );

int pvkOpConnUp ( void );

int pvkOpConnDown ( void );

#endif

#ifdef __epicsPv_c

int pvrShort ( void ) {
  return DBR_SHORT;
}
int pvrLong ( void ) {
  return DBR_LONG;
}
int pvrFloat ( void ) {
  return DBR_FLOAT;
}
int pvrDouble ( void ) {
  return DBR_DOUBLE;
}
int pvrEnum ( void ) {
  return DBR_ENUM;
}
int pvrString ( void ) {
  return DBR_STRING;
}

int pvrStsShort ( void ) {
  return DBR_STS_SHORT;
}
int pvrStsLong ( void ) {
  return DBR_STS_LONG;
}
int pvrStsFloat ( void ) {
  return DBR_STS_FLOAT;
}
int pvrStsDouble ( void ) {
  return DBR_STS_DOUBLE;
}
int pvrStsEnum ( void ) {
  return DBR_STS_ENUM;
}
int pvrStsString ( void ) {
  return DBR_STS_STRING;
}

int pvrGrShort ( void ) {
  return DBR_GR_SHORT;
}
int pvrGrLong ( void ) {
  return DBR_GR_LONG;
}
int pvrGrFloat ( void ) {
  return DBR_GR_FLOAT;
}
int pvrGrDouble ( void ) {
  return DBR_GR_DOUBLE;
}
int pvrGrEnum ( void ) {
  return DBR_GR_ENUM;
}
int pvrGrString ( void ) {
  return DBR_GR_STRING;
}

int pveValue ( void ) {
  return DBE_VALUE;
}

int pveAlarm ( void ) {
  return DBE_ALARM;
}

int pvkOpConnUp ( void ) {
  return CA_OP_CONN_UP;
}

int pvkOpConnDown ( void ) {
  return CA_OP_CONN_DOWN;
}

#endif

int search (
  expStringClass *name );

int searchAndConnect (
  expStringClass *name,
  pvCbFunc callback,
  void *clientData );

int createEventId (
  pvEventClass **ptr );

int destroyEventId (
  pvEventClass **ptr );

int pendIo (
  float sec );

int pendEvent (
  float sec );

int put (
  int type,
  void *value );

int get (
  int type,
  void *value );

int getCallback (
  int type,
  pvCbFunc callback,
  void *clientData );

int addEvent (
  int type,
  int numElements,
  pvCbFunc callback,
  void *clientData,
  pvEventClass *eventId,
  int eventType );

int checkReconnect ( void );

char *getName ( void );

int getType ( void );

int getSize ( void );

int maxStringSize ( void );

int enumStringSize ( void );

int pvNameSize ( void );

int clearChannel ( void );

int clearEvent (
  pvEventClass *eventId );

// event argument manipulation

void *getValue (
  void *eventArg );

int getType (
  void *eventArg );

int getStatus (
  void *eventArg );

int getSeverity (
  void *eventArg );

int getPrecision(
  void *eventArg );

char *getStateString(
  void *eventArg, int index );

int getNumStates(
  void *eventArg );

void *getEventUserData (
  void *eventArg );

// connection argument manipulation

void *getConnectUserData (
  void *connectArg );

int getOp (
  void *connectArg );

double getLoOpr (
  void *eventArg );

double getHiOpr (
  void *eventArg );

int pvErrorCode (
  int code );

};
