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

#define __rules_cc

#include "rules.h"

void _edmDebug ( void );

static void monitor_connect_state (
  struct connection_handler_args arg )
{

ruleElementPtr rep = (ruleElementPtr) ca_puser(arg.chid);
ruleClass *rc = (ruleClass *) rep->r;
int i = rep->i;
int n, stat;

  if ( arg.op == CA_OP_CONN_UP ) {

    //printf( "monitor_connect_state, connect %-d\n", i );

    rc->pvType[i] = (short) ca_field_type( arg.chid );
    rc->id[i] = arg.chid;
    rc->connection.setPvConnected( arg.chid );

    //printf( "set %-d connected\n", (int) arg.chid );
    //printf( "connection mask = %-x\n", rc->connection.pvsConnected() );

    stat = ca_add_masked_array_event( DBR_DOUBLE, 1, rc->id[i],
     valueUpdate, (void *) rep, (float) 0.0, (float) 0.0,
     (float) 0.0, &rc->eventId[i], DBE_VALUE );

    if ( rc->connection.pvsConnected() ) {
      (*rc->connectFunc)( rc->userPtr, rc->ruleId, arg.op );
    }

  }
  else {

    //printf( "monitor_connect_state - disconnect %-d\n", i );

    rc->connection.setPvDisconnected( arg.chid );
    (*rc->connectFunc)( rc->userPtr, rc->ruleId, arg.op );

    rc->valueConnection.setPvDisconnected( arg.chid );

  }

}

static void valueUpdate (
  struct event_handler_args arg
) {

ruleElementPtr rep = (ruleElementPtr) ca_puser(arg.chid);
ruleClass *rc = (ruleClass *) rep->r;
int result, n, i = rep->i;

// don't consider pv connected until we get a value change callback

  rc->val[i] = *( (double *) arg.dbr );

  if ( !rc->valueConnection.pvsConnected() ) {
    rc->valueConnection.setPvConnected( arg.chid );
    if ( !rc->valueConnection.pvsConnected() ) return;
  }

  //for ( n=0; n<rc->numIds; n++ ) {
    //printf( "val[%-d] = %-g\n", n, rc->val[n] );
  //}
  if ( rc->func ) {
    result = (*rc->func)( (void *) rc, rc->numIds, (void *) rc->val );
    //printf( "result = %-d\n", result );
    //printf( "ruleId = %-d\n", rc->ruleId );
    if ( rc->first || ( result != rc->prevValue ) ) {
      rc->first = 0;
      rc->prevValue = result;
      if ( rc->userFunc ) {
        (*rc->userFunc)( rc->userPtr, rc->ruleId, result );
      }
    }
  }

}

ruleClass::ruleClass ( void ) {

int i;

//printf( "ruleClass::ruleClass\n" );
  activated = 0;
  eventsBooked = 0;

  for ( i=0; i<32; i++ ) {
    id[i] = 0;
    eventId[i] = 0;
  }
  ruleId = -1;
  userPtr = NULL;
  userFunc = NULL;
  connectFunc = NULL;

}

ruleClass::ruleClass (
  void *_userPtr,
  int _ruleId,
  RULECALLBACK _connectFunc,
  RULECALLBACK _userFunc
) {

int i;

  activated = 0;
  eventsBooked = 0;

  for ( i=0; i<32; i++ ) {
    id[i] = 0;
    eventId[i] = 0;
  }
  ruleId = _ruleId;
  userPtr = _userPtr;
  connectFunc = _connectFunc;
  userFunc = _userFunc;

}

ruleClass::~ruleClass( void ) {

  //printf( "ruleClass::~ruleClass\n" );

}

void ruleClass::init (
  void *_userPtr,
  int _ruleId,
  RULECALLBACK _connectFunc,
  RULECALLBACK _userFunc
) {

int i;

  activated = 0;
  eventsBooked = 0;

  for ( i=0; i<32; i++ ) {
    id[i] = 0;
    eventId[i] = 0;
  }
  ruleId = _ruleId;
  userPtr = _userPtr;
  connectFunc = _connectFunc;
  userFunc = _userFunc;

}

int ruleClass::activate (
  ulBindingClass *ul,
  char *string
) {

int stat;

  char buf[255+1], *context, *tok;

  if ( activated ) return 1;

  activated = 1;
  first = 1;

  //printf( "ruleClass::activate, string = [%s]\n", string );

  strncpy( buf, string, 255 );
  buf[255] = 0;

  numIds = -1;

  context = NULL;
  tok = strtok_r( buf, "(),", &context );
  if ( !tok ) return 0;

  //_edmDebug();
  func = ul->getRuleFunc( tok );
  //printf( "func = %s, addr=%-x\n", tok, (int) func );

  connection.init();
  connection.setMaxPvs( 32 );

  valueConnection.init();
  valueConnection.setMaxPvs( 32 );

  numIds = 0;
  do {

    tok = strtok_r( NULL, "(),", &context );
    if ( tok ) {
      if ( numIds < 31 ) {
        //printf( "param = %s\n", tok );
        re[numIds].r = this;
        re[numIds].i = numIds;
        connection.addPv();
        valueConnection.addPv();
        stat = ca_search_and_connect( tok, &id[numIds], monitor_connect_state,
         &re[numIds] );
        numIds++;
      }
    }

  } while ( tok );

  //printf( "numIds = %-d\n", numIds );

  return 1;

}

int ruleClass::bookEvents ( void ) {

int stat, i;

  return 1;

  if ( eventsBooked ) return 1;

  eventsBooked = 1;

  if ( !connection.pvsConnected() ) return 0;

  for ( i=0; i<numIds; i++ ) {

    re[i].i = i;
    re[i].r = this;
    stat = ca_add_masked_array_event( DBR_DOUBLE, 1, id[i],
     valueUpdate, (void *) &re[i], (float) 0.0, (float) 0.0,
     (float) 0.0, &eventId[i], DBE_VALUE );

  }

  return 1;

}

int ruleClass::deactivate ( void ) {

int i, stat;

//printf( "ruleClass::deactivate\n" );

  for ( i=0; i<32; i++ ) {
    if ( id[i] ) {
      stat = ca_clear_channel( id[i] );
      id[i] = 0;
      eventId[i] = 0;
    }
  }

  activated = 0;
  eventsBooked = 0;

  return 1;

}


