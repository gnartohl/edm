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

#define __epicsPv_c
#include "epicsPv.h"

static void __epicsPvConnectHandler (
  struct connection_handler_args arg )
{

__epicsPvEvArgPtr eventArg = (__epicsPvEvArgPtr) ca_puser(arg.chid);

//  fprintf( stderr, "__epicsPvConnectHandler\n" );

  (*eventArg->callback)( eventArg->classPtr,
   (void *) eventArg->clientData, (void *) &arg );

}

static void __epicsPvEventHandler (
  struct event_handler_args arg )
{

__epicsPvEvArgPtr eventArg = (__epicsPvEvArgPtr) arg.usr;

//  fprintf( stderr, "__epicsPvEventHandler\n" );

  (*eventArg->callback)( eventArg->classPtr,
   (void *) eventArg->clientData, (void *) &arg );

}

static void __epicsPvGetCbHandler (
  struct event_handler_args arg )
{

__epicsPvEvArgPtr eventArg = (__epicsPvEvArgPtr) arg.usr;

//  fprintf( stderr, "__epicsPvGetCbHandler\n" );

  (*eventArg->callback)( eventArg->classPtr,
   (void *) eventArg->clientData, (void *) &arg );

  delete eventArg;

}

epicsPvEventClass::epicsPvEventClass ( void )
{

  type = new char[strlen("epics")+1];
  strcpy( type, "epics" );

  evId = 0;

}

epicsPvEventClass::epicsPvEventClass ( epicsPvEventClass &source )
{

  type = new char[strlen(source.type)+1];
  strcpy( type, source.type );

  evId = source.evId;

}

epicsPvEventClass::~epicsPvEventClass ( void )
{

  delete type;
  type = NULL;

}

epicsPvClass::epicsPvClass ( void )
{

  type = new char[strlen("epics")+1];
  strcpy( type, "epics" );

  strcpy( dummy, " " );

  initialConnection = 0;
  connected = 0;
  id = NULL;

  eventHead = new eventListType;
  eventTail = eventHead;
  eventTail->flink = NULL;

}

epicsPvClass::epicsPvClass (
  epicsPvClass &source )
{

eventListPtr cur, newOne;

  type = new char[strlen(source.type)+1];
  strcpy( type, source.type );

  initialConnection = source.initialConnection;
  connected = source.connected;
  id = source.id;

  eventHead = new eventListType;
  eventTail = eventHead;
  eventTail->flink = NULL;

  cur = source.eventHead->flink;
  while ( cur ) {

    newOne = new eventListType;
    *newOne = *cur;
    eventTail->flink = newOne;
    eventTail = newOne;
    eventTail->flink = NULL;

    cur = cur->flink;

  }

}

epicsPvClass::~epicsPvClass ( void ) {

eventListPtr cur, next;

  delete type;
  type = NULL;

  cur = eventHead->flink;
  while ( cur ) {

    next = cur->flink;
    delete cur;
    cur = next;

  }

  delete eventHead;
  eventHead = NULL;
  eventTail = NULL;

}

void epicsPvClass::clone (
  const epicsPvClass *source )
{

  type = new char[strlen(source->type)+1];
  strcpy( type, source->type );

  initialConnection = source->initialConnection;
  connected = source->connected;
  id = source->id;

} 

int epicsPvEventClass::eventAdded ( void )
{

  if ( evId )
    return 1;
  else
    return 0;

}

int epicsPvClass::search (
expStringClass *name )
{

int stat;

  stat = ca_search( name->getExpanded(), &id );

  return pvErrorCode( stat );

}

int epicsPvClass::searchAndConnect (
  expStringClass *name,
  pvCbFunc callback,
  void *clientData )
{

int stat;

//  fprintf( stderr, "epicsPvClass::searchAndConnect\n" );

  this->eventArg.clientData = clientData;
  this->eventArg.classPtr = this;
  this->eventArg.callback = (pvCbFunc) callback;

//    fprintf( stderr, "name = [%s]\n", name->getExpanded() );

  stat = ca_search_and_connect( name->getExpanded(), &id,
   __epicsPvConnectHandler, &this->eventArg );

  return pvErrorCode( stat );

}

int epicsPvClass::createEventId (
  pvEventClass **ptr )
{

epicsPvEventClass *event;
eventListPtr newOne;

  event = new epicsPvEventClass;

  newOne = new eventListType;
  newOne->evPtr = event;

  // link into list contained in epicsPvClass object
  eventTail->flink = newOne;
  eventTail = newOne;
  eventTail->flink = NULL;

  // allocate sentinel node
  event->head = new eventArgListType;
  event->tail = event->head;
  event->tail->flink = NULL;

  *ptr = event;

  return PV_E_SUCCESS;

}

int epicsPvClass::destroyEventId (
  pvEventClass **ptr )
{

eventListPtr cur, next, prev;

  prev = eventHead;
  cur = eventHead->flink;
  while ( cur ) {

    next = cur->flink;

    if ( cur->evPtr == *ptr ) {
      prev->flink = next;
      delete cur->evPtr;
      *ptr = NULL;
      delete cur;
    }
    else {
      prev = cur;
    }

    cur = next;

  }

  return PV_E_SUCCESS;

}

int epicsPvClass::pendIo (
  float sec )
{

int stat;

  stat = ca_pend_io( sec );

  return pvErrorCode( stat );

}

int epicsPvClass::pendEvent (
  float sec )
{

int stat;

  stat = ca_pend_event( sec );

  return pvErrorCode( stat );

}

int epicsPvClass::put (
  int type,
  void *value )
{

int stat;

  stat = ca_put( type, id, value );
  return pvErrorCode( stat );

}

int epicsPvClass::get (
  int type,
  void *value )
{

int stat;

  stat = ca_get( type, id, value );
  return pvErrorCode( stat );

}

int epicsPvClass::getCallback (
  int type,
  pvCbFunc callback,
  void *clientData )
{

int stat;
__epicsPvEvArgPtr eventArg;

  eventArg = new __epicsPvEvArgType;
  eventArg->clientData = clientData;
  eventArg->classPtr = this;
  eventArg->callback = (pvCbFunc) callback;
  eventArg->requestType = type;

  stat = ca_get_callback( type, id, __epicsPvGetCbHandler, eventArg );
  return pvErrorCode( stat );

}

int epicsPvClass::addEvent (
  int type,
  int numElements,
  pvCbFunc callback,
  void *clientData,
  pvEventClass *eventId,
  int eventType )
{

int stat;
eventArgListPtr cur;
epicsPvEventClass *epicsEventId = (epicsPvEventClass *) eventId;
__epicsPvEvArgPtr eventArg;

  eventArg = new __epicsPvEvArgType;
  eventArg->clientData = clientData;
  eventArg->classPtr = this;
  eventArg->callback = (pvCbFunc) callback;
  eventArg->requestType = type;

  cur = new eventArgListType;
  cur->ptr = eventArg;

  epicsEventId->tail->flink = cur;
  epicsEventId->tail = cur;
  epicsEventId->tail->flink = NULL;

//  fprintf( stderr, "ca_add_masked_array_event\n" );

  stat = ca_add_masked_array_event( type, numElements, id,
   __epicsPvEventHandler, eventArg, (float) 0.0, (float) 0.0,
   (float) 0.0, &epicsEventId->evId, eventType );

  return pvErrorCode( stat );

}

int epicsPvClass::checkReconnect ( void )
{

  // channel access takes care of this internally

  return PV_E_SUCCESS;

}

char *epicsPvClass::getName ( void )
{

  return (char *) ca_name(id);

}

int epicsPvClass::getType ( void )
{

  return (int) ca_field_type(id);

}

int epicsPvClass::getSize ( void )
{

  return ca_element_count(id);

}

int epicsPvClass::maxStringSize ( void )
{

  return 39;
  
}

int epicsPvClass::enumStringSize ( void )
{

  return 25;
  
}

int epicsPvClass::pvNameSize ( void )
{

  return 39;
  
}

int epicsPvClass::clearChannel ( void )
{

int stat, oneStat;
eventListPtr cur, next;

  stat = ECA_NORMAL;

  cur = eventHead->flink;
  while ( cur ) {

    next = cur->flink;
    oneStat = this->clearEvent( cur->evPtr );
    if ( oneStat != ECA_NORMAL ) stat = oneStat;
    cur = next;

  }

  oneStat = ca_clear_channel( id );
  if ( oneStat != ECA_NORMAL ) stat = oneStat;

  return pvErrorCode( stat );

}

int epicsPvClass::clearEvent (
  pvEventClass *eventId )
{

int stat;
eventArgListPtr cur, next;
epicsPvEventClass *epicsEventId = (epicsPvEventClass *) eventId;

  if ( epicsEventId->evId )
    stat = ca_clear_event( epicsEventId->evId );
  else
    stat = ECA_NORMAL;

  cur = epicsEventId->head->flink;
  while ( cur ) {

    next = cur->flink;
    delete cur->ptr;
    delete cur;
    cur = next;

  }

  epicsEventId->tail = epicsEventId->head;
  epicsEventId->tail->flink = NULL;

  return pvErrorCode( stat );

}

// event argument manipulation

void *epicsPvClass::getValue (
  void *eventArg )
{

struct event_handler_args *args =
 (struct event_handler_args *) eventArg;

__epicsPvEvArgPtr epicsEventArg = (__epicsPvEvArgPtr) args->usr;

  switch ( epicsEventArg->requestType ) {

  case DBR_DOUBLE:
    return (void *) args->dbr;
    break;

  case DBR_STS_DOUBLE:
    return (void *) &( (dbr_sts_double *) args->dbr )->value;
    break;

  case DBR_GR_DOUBLE:
    return (void *) &( ( (dbr_gr_double *) args->dbr )->value );
    break;

  case DBR_FLOAT:
    return (void *) args->dbr;
    break;

  case DBR_STS_FLOAT:
    return (void *) &( (dbr_sts_float *) args->dbr )->value;
    break;

  case DBR_GR_FLOAT:
    return (void *) &( ( (dbr_gr_float *) args->dbr )->value );
    break;

  case DBR_LONG:
    return (void *) args->dbr;
    break;

  case DBR_STS_LONG:
    return (void *) &( (dbr_sts_long *) args->dbr )->value;
    break;

  case DBR_GR_LONG:
    return (void *) &( ( (dbr_gr_long *) args->dbr )->value );
    break;

  case DBR_ENUM:
    return (void *) args->dbr;
    break;

  case DBR_STS_ENUM:
    return (void *) &( (dbr_sts_enum *) args->dbr )->value;
    break;

  case DBR_GR_ENUM:
    return (void *) &( ( (dbr_gr_enum *) args->dbr )->value );
    break;

  case DBR_STRING:
    return (void *) args->dbr;
    break;

  }

  return NULL;

}

int epicsPvClass::getType (
  void *eventArg )
{

struct event_handler_args args =
 *( (struct event_handler_args *) eventArg );

  return (int) args.type;

}

int epicsPvClass::getStatus (
  void *eventArg )
{

struct event_handler_args args =
 *( (struct event_handler_args *) eventArg );

struct dbr_sts_string *dbs =
 (struct dbr_sts_string *) args.dbr;

  return dbs->status;

}

int epicsPvClass::getSeverity (
  void *eventArg )
{

struct event_handler_args args =
 *( (struct event_handler_args *) eventArg );

struct dbr_sts_string *dbs =
 (struct dbr_sts_string *) args.dbr;

  return dbs->severity;

}

int epicsPvClass::getPrecision (
  void *eventArg )
{

struct event_handler_args *args =
 (struct event_handler_args *) eventArg;

__epicsPvEvArgPtr epicsEventArg = (__epicsPvEvArgPtr) args->usr;

  switch ( epicsEventArg->requestType ) {

  case DBR_GR_DOUBLE:
    return (int) ( ( (dbr_gr_double *) args->dbr )->precision );

  case DBR_GR_FLOAT:
    return (int) ( ( (dbr_gr_float *) args->dbr )->precision );

  }

  return (int) 0;

}

char *epicsPvClass::getStateString (
  void *eventArg,
  int index )
{

struct event_handler_args *args =
 (struct event_handler_args *) eventArg;

__epicsPvEvArgPtr epicsEventArg = (__epicsPvEvArgPtr) args->usr;

  switch ( epicsEventArg->requestType ) {

  case DBR_GR_ENUM:
    if ( ( index > -1 ) && ( index < MAX_ENUM_STATES ) ) {
      return (char *) ( ( (dbr_gr_enum *) args->dbr )->strs[index] );
    }
    else {
      return (char *) dummy;
    }

  }

  return (char *) dummy;

}

int epicsPvClass::getNumStates (
  void *eventArg )
{

struct event_handler_args *args =
 (struct event_handler_args *) eventArg;

__epicsPvEvArgPtr epicsEventArg = (__epicsPvEvArgPtr) args->usr;

  switch ( epicsEventArg->requestType ) {

  case DBR_GR_ENUM:
    return (int) ( ( (dbr_gr_enum *) args->dbr )->no_str );

  }

  return (int) 0;

}

void *epicsPvClass::getEventUserData (
  void *eventArg )
{

struct event_handler_args *args =
 (struct event_handler_args *) eventArg;

__epicsPvEvArgPtr epicsEventArg = (__epicsPvEvArgPtr) args->usr;

  return (void *) epicsEventArg->clientData;

}

void *epicsPvClass::getConnectUserData (
  void *connectArg )
{

struct connection_handler_args *args =
  (struct connection_handler_args *) connectArg;

__epicsPvEvArgPtr eventArg = (__epicsPvEvArgPtr) ca_puser(args->chid);

  return (void *) eventArg->clientData;

}

int epicsPvClass::getOp (
  void *connectArg )
{

struct connection_handler_args *arg =
  (struct connection_handler_args *) connectArg;

  return (int) arg->op;

}

double epicsPvClass::getLoOpr (
  void *eventArg )
{

struct event_handler_args *args =
 (struct event_handler_args *) eventArg;

__epicsPvEvArgPtr epicsEventArg = (__epicsPvEvArgPtr) args->usr;

  switch ( epicsEventArg->requestType ) {

  case DBR_GR_DOUBLE:
    return (double) ( ( (dbr_gr_double *) args->dbr )->lower_disp_limit );
    break;

  case DBR_GR_FLOAT:
    return (double) ( ( (dbr_gr_float *) args->dbr )->lower_disp_limit );

  case DBR_GR_LONG:
    return (double) ( ( (dbr_gr_long *) args->dbr )->lower_disp_limit );

  }

  return (double) 0.0;

}

double epicsPvClass::getHiOpr (
  void *eventArg )
{

struct event_handler_args *args =
 (struct event_handler_args *) eventArg;

__epicsPvEvArgPtr epicsEventArg = (__epicsPvEvArgPtr) args->usr;

  switch ( epicsEventArg->requestType ) {

  case DBR_GR_DOUBLE:
    return (double) ( ( (dbr_gr_double *) args->dbr )->upper_disp_limit );

  case DBR_GR_FLOAT:
    return (double) ( ( (dbr_gr_float *) args->dbr )->upper_disp_limit );

  case DBR_GR_LONG:
    return (double) ( ( (dbr_gr_long *) args->dbr )->upper_disp_limit );

  }

  return (double) 0.0;

}

int epicsPvClass::pvErrorCode (
  int code )
{

  switch ( code ) {

  case ECA_NORMAL:
    return PV_E_SUCCESS;

  }

  return PV_E_FAIL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_epicsPvPtr ( void ) {

epicsPvClass *ptr;

  ptr = new epicsPvClass;
  return (void *) ptr;

}

void *clone_epicsPvClassPtr (
  void *_srcPtr )
{

epicsPvClass *ptr, *srcPtr;

  srcPtr = (epicsPvClass *) _srcPtr;

  ptr = new epicsPvClass( *srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
