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

#ifndef __pv_callback_h
#define __pv_callback_h 1

#include "pv_factory.h"
#include "pvConnection.h"
#include "expString.h"
#include "utility.h"

class PvCallbackClass {

public:

  PvCallbackClass ( void );

  PvCallbackClass (
    expStringClass &pvExpString,
    pvConnectionClass *_connection,
    const int _pvId,
    void *_userarg,
    PVCallback _valueCallback,
    PVCallback _connectCallback
  );

  PvCallbackClass (
    expStringClass &pvExpString,
    pvConnectionClass *_connection,
    const int _pvId,
    void *_userarg,
    PVCallback _valueCallback,
    PVCallback _connectCallback,
    int _deferValueCallback
  );

  ~PvCallbackClass ( void );

  static void genericConnCallback (
    ProcessVariable *_pv,
    void *_userarg
  );

  void doValueCallback ( void );

  ProcessVariable *getPv ( void ) const;

  pvConnectionClass *getConn ( void ) const;

  int getId ( void ) const;

  void *getUserArg( void ) const;

  int getPvExists ( void ) const;

  int getStatus( void ) const;

private:

  ProcessVariable *procVar;
  pvConnectionClass *connection;
  void *userarg;
  int pvId;
  PVCallback valueCallback;
  PVCallback connectCallback;
  int firstConnect;
  int pvExists;
  int status;
  int deferValueCallback;
  int valueCallbackPerformed;

};

#endif
