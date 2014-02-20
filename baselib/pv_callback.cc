#include "pv_callback.h"

void PvCallbackClass::genericConnCallback (
  ProcessVariable *_pv,
  void *_userarg
) {

  PvCallbackClass *pco = (PvCallbackClass *) _userarg;

  if ( pco->procVar->is_valid() ) {
    pco->connection->setPvConnected( (void *) pco );
  }
  else {
    pco->connection->setPvDisconnected( (void *) pco );
  }

  (pco->connectCallback)( pco->procVar, pco );

  if ( pco->procVar->is_valid() ) {

    if ( !pco->deferValueCallback ) {
      if ( pco->firstConnect ) {
        pco->firstConnect = 0;
        if ( !pco->valueCallbackPerformed ) {
          pco->procVar->add_value_callback( pco->valueCallback, pco );
          pco->valueCallbackPerformed = 1;
        }
      }
    }

  }

}

PvCallbackClass::PvCallbackClass ( void ) {

  procVar = NULL;
  firstConnect = 1;
  pvExists = 0;
  status = 2; // invalid

}

// prerequisites
//   _connection has been initialized and max pvs have been set

PvCallbackClass::PvCallbackClass (
  expStringClass &pvExpString,
  pvConnectionClass *_connection,
  const int _pvId,
  void *_userarg,
  PVCallback _connectCallback,
  PVCallback _valueCallback
) {

  connection = _connection;
  pvId = _pvId;
  userarg = _userarg;
  valueCallback = _valueCallback;
  connectCallback = _connectCallback;

  procVar = NULL;
  firstConnect = 1;
  pvExists = 1;
  status = 1; // valid

  deferValueCallback = 0;
  valueCallbackPerformed = 0;

  if ( !pvExpString.getExpanded() ||
     blankOrComment( pvExpString.getExpanded() ) ) {
    pvExists = 0;
    return;
  }

  procVar = the_PV_Factory->create( pvExpString.getExpanded() );
  if ( procVar ) {
    if ( connectCallback ) {
      connection->addPv();
      procVar->add_conn_state_callback( PvCallbackClass::genericConnCallback, this );
    }
  }
  else {
    pvExists = 0;
    status = 2; // invalid
  }

}

PvCallbackClass::PvCallbackClass (
  expStringClass &pvExpString,
  pvConnectionClass *_connection,
  const int _pvId,
  void *_userarg,
  PVCallback _connectCallback,
  PVCallback _valueCallback,
  int _deferValueCallback
) {

  connection = _connection;
  pvId = _pvId;
  userarg = _userarg;
  valueCallback = _valueCallback;
  connectCallback = _connectCallback;

  procVar = NULL;
  firstConnect = 1;
  pvExists = 1;
  status = 1; // valid

  deferValueCallback = _deferValueCallback;
  valueCallbackPerformed = 0;

  if ( !pvExpString.getExpanded() ||
     blankOrComment( pvExpString.getExpanded() ) ) {
    pvExists = 0;
    return;
  }

  procVar = the_PV_Factory->create( pvExpString.getExpanded() );
  if ( procVar ) {
    if ( connectCallback ) {
      connection->addPv();
      procVar->add_conn_state_callback( PvCallbackClass::genericConnCallback, this );
    }
  }
  else {
    pvExists = 0;
    status = 2; // invalid
  }

}

PvCallbackClass::~PvCallbackClass ( void ) {

  if ( pvExists ) {

    if ( procVar ) {

      procVar->remove_conn_state_callback( PvCallbackClass::genericConnCallback, this );
      procVar->remove_value_callback( valueCallback, this );
      procVar->release();
      procVar = NULL;
      pvExists = 0;

    }

  }

}

void PvCallbackClass::doValueCallback ( void ) {

  if ( !valueCallbackPerformed ) {
    procVar->add_value_callback( valueCallback, this );
    valueCallbackPerformed = 1;
  }

}

ProcessVariable *PvCallbackClass::getPv ( void ) const {

  return procVar;

}

pvConnectionClass *PvCallbackClass::getConn ( void ) const {

  return connection;

}

int PvCallbackClass::getId ( void ) const {

  return pvId;

}

void *PvCallbackClass::getUserArg( void ) const {

  return userarg;

}

int PvCallbackClass::getPvExists ( void ) const {

  return pvExists;

}

int PvCallbackClass::getStatus ( void ) const {

  return status;

}
