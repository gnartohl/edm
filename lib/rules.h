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

#ifndef __rules_h
#define __rules_h 1

#include "ulBindings.h"
#include "pvConnection.h"

#include "cadef.h"

#ifdef __rules_cc

static void monitor_connect_state (
 struct connection_handler_args arg );

static void valueUpdate (
  struct event_handler_args ast_args );

#endif

typedef void (*RULECALLBACK)( void *classPtr, int id, int value );

class ruleClass;

typedef struct ruleElementTag {
  ruleClass *r;
  int i;
} ruleElementType, *ruleElementPtr;

class ruleClass {

private:

friend void monitor_connect_state (
 struct connection_handler_args arg );

friend void valueUpdate (
  struct event_handler_args ast_args );

int numIds, activated, eventsBooked, ruleId;
chid id[32];
evid eventId[32];
double val[32];
short pvType[32];
ruleElementType re[32];
pvConnectionClass connection;
void *userPtr;
RULECALLBACK userFunc;
RULEFUNC func;

public:

ruleClass::ruleClass ( void );

ruleClass::ruleClass (
  void *_userPtr,
  int _ruleId,
  RULECALLBACK _userFunc );

ruleClass::~ruleClass( void );

ruleClass::init (
  void *_userPtr,
  int _ruleId,
  RULECALLBACK _userFunc );

int ruleClass::activate (
  ulBindingClass *ul,
  char *string );

int ruleClass::bookEvents ( void );

int ruleClass::deactivate ( void );

};

#endif
