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

#define __x_text_dsp_obj_cc 1

#include "x_text_dsp_obj.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void xtdoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->editDialogIsActive = 0;

}

static void xtdoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat;

  axtdo->editDialogIsActive = 0;
  stat = ca_put( DBR_DOUBLE, axtdo->pvId, &axtdo->kpDouble );

}

static void xtdoSetKpIntValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat;

  axtdo->editDialogIsActive = 0;
  stat = ca_put( DBR_LONG, axtdo->pvId, &axtdo->kpInt );

}

static void xtdoSetValueChanged (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->widget_value_changed = 1;

  if ( axtdo->changeCallback ) {
    (*axtdo->changeCallback)( axtdo );
  }

}

static void xtdoSetSelection (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int l;
char *buf;
Arg args[10];
int n;

  axtdo->widget_value_changed = 0;

  buf = XmTextGetString( axtdo->tf_widget );
  l = strlen(buf);
  XtFree( buf );

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) True ); n++;
  XtSetValues( axtdo->tf_widget, args, n );

  XmTextSetSelection( axtdo->tf_widget, 0, l,
   XtLastTimestampProcessed( axtdo->actWin->display() ) );

  XmTextSetInsertionPosition( axtdo->tf_widget, l );

}

static void xtdoTextFieldToStringA (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat;
char string[39+1];
char *buf;

  buf = XmTextGetString( axtdo->tf_widget );
  strncpy( axtdo->entryValue, buf, 39 );
  axtdo->entryValue[39] = 0;
  XtFree( buf );

  strncpy( axtdo->curValue, axtdo->entryValue, 39 );
  axtdo->curValue[39] = 0;
  strncpy( string, axtdo->entryValue, 39 );
  string[39] = 0;
  if ( axtdo->pvExists ) {
#ifdef __epics__
    stat = ca_put( DBR_STRING, axtdo->pvId, &string );
#endif
  }
  else {
    axtdo->needUpdate = 1;
    axtdo->actWin->appCtx->proc->lock();
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );
    axtdo->actWin->appCtx->proc->unlock();
  }

  XmTextSetInsertionPosition( axtdo->tf_widget, 0 );

}

static void xtdoTextFieldToStringLF (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat, l;
char string[39+1];
char *buf;
Arg args[10];
int n;

  if ( !axtdo->widget_value_changed ) return;
 
  buf = XmTextGetString( axtdo->tf_widget );
  l = strlen(buf);
  strncpy( axtdo->entryValue, buf, 39 );
  axtdo->entryValue[39] = 0;
  XtFree( buf );

  strncpy( axtdo->curValue, axtdo->entryValue, 39 );
  axtdo->curValue[39] = 0;
  strncpy( string, axtdo->entryValue, 39 );
  string[39] = 0;
  if ( axtdo->pvExists ) {
#ifdef __epics__
    stat = ca_put( DBR_STRING, axtdo->pvId, &string );
#endif
  }
  else {
    axtdo->needUpdate = 1;
    axtdo->actWin->appCtx->proc->lock();
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );
    axtdo->actWin->appCtx->proc->unlock();
  }

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) False ); n++;
  XtSetValues( axtdo->tf_widget, args, n );

  XmTextSetInsertionPosition( axtdo->tf_widget, l );

}

static void xtdoTextFieldToIntA (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int ivalue, stat;
char *buf;

  buf = XmTextGetString( axtdo->tf_widget );
  strncpy( axtdo->entryValue, buf, 39 );
  axtdo->entryValue[39] = 0;
  XtFree( buf );

  if ( isLegalInteger(axtdo->entryValue) ) {

    strncpy( axtdo->curValue, axtdo->entryValue, 39 );
    axtdo->curValue[39] = 0;

    ivalue = atol( axtdo->entryValue );
    if ( axtdo->pvExists ) {
#ifdef __epics__
      stat = ca_put( DBR_LONG, axtdo->pvId, &ivalue );
#endif
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

    XmTextSetInsertionPosition( axtdo->tf_widget, 0 );

  }

}

static void xtdoTextFieldToIntLF (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int ivalue, stat, l;
char *buf;
Arg args[10];
int n;

  if ( !axtdo->widget_value_changed ) return;

  buf = XmTextGetString( axtdo->tf_widget );
  l = strlen(buf);
  strncpy( axtdo->entryValue, buf, 39 );
  axtdo->entryValue[39] = 0;
  XtFree( buf );

  if ( isLegalInteger(axtdo->entryValue) ) {

    strncpy( axtdo->curValue, axtdo->entryValue, 39 );
    axtdo->curValue[39] = 0;

    ivalue = atol( axtdo->entryValue );
    if ( axtdo->pvExists ) {
#ifdef __epics__
      stat = ca_put( DBR_LONG, axtdo->pvId, &ivalue );
#endif
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

  }

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) False ); n++;
  XtSetValues( axtdo->tf_widget, args, n );

  XmTextSetInsertionPosition( axtdo->tf_widget, l );

}

static void xtdoTextFieldToDoubleA (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat;
double dvalue;
char *buf;

  buf = XmTextGetString( axtdo->tf_widget );
  strncpy( axtdo->entryValue, buf, 39 );
  axtdo->entryValue[39] = 0;
  XtFree( buf );

  if ( isLegalFloat(axtdo->entryValue) ) {

    strncpy( axtdo->curValue, axtdo->entryValue, 39 );
    axtdo->curValue[39] = 0;

    dvalue = atof( axtdo->entryValue );
    if ( axtdo->pvExists ) {
#ifdef __epics__
      stat = ca_put( DBR_DOUBLE, axtdo->pvId, &dvalue );
#endif
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

    XmTextSetInsertionPosition( axtdo->tf_widget, 0 );

  }

}

static void xtdoTextFieldToDoubleLF (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
int stat, l;
double dvalue;
char *buf;
Arg args[10];
int n;

  if ( !axtdo->widget_value_changed ) return;

  buf = XmTextGetString( axtdo->tf_widget );
  l = strlen(buf);
  strncpy( axtdo->entryValue, buf, 39 );
  axtdo->entryValue[39] = 0;
  XtFree( buf );

  if ( isLegalFloat(axtdo->entryValue) ) {

    strncpy( axtdo->curValue, axtdo->entryValue, 39 );
    axtdo->curValue[39] = 0;

    dvalue = atof( axtdo->entryValue );
    if ( axtdo->pvExists ) {
#ifdef __epics__
      stat = ca_put( DBR_DOUBLE, axtdo->pvId, &dvalue );
#endif
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

  }

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) False ); n++;
  XtSetValues( axtdo->tf_widget, args, n );

  XmTextSetInsertionPosition( axtdo->tf_widget, l );

}

#ifdef __epics__

static void xtdo_monitor_connect_state (
  struct connection_handler_args arg )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) ca_puser(arg.chid);

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    if ( arg.op == CA_OP_CONN_UP ) {

      axtdo->pvType = (int) ca_field_type( axtdo->pvId );

      axtdo->connection.setPvConnected( axtdo->pvId );

      if ( axtdo->connection.pvsConnected() ) {
        axtdo->needConnectInit = 1;
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      }

    }
    else {

      axtdo->connection.setPvDisconnected( axtdo->pvId );
      axtdo->fgColor.setDisconnected();
      axtdo->needRefresh = 1;
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );

    }

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void xtdo_monitor_sval_connect_state (
  struct connection_handler_args arg )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) ca_puser(arg.chid);

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    if ( arg.op == CA_OP_CONN_UP ) {

      axtdo->svalPvType = (int) ca_field_type( axtdo->svalPvId );

      axtdo->connection.setPvConnected( axtdo->svalPvId );

      if ( axtdo->connection.pvsConnected() ) {
        axtdo->needConnectInit = 1;
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      }

    }
    else {

      axtdo->connection.setPvDisconnected( axtdo->svalPvId );
      axtdo->fgColor.setDisconnected();
      axtdo->needRefresh = 1;
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );

    }

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void xtdo_monitor_fg_connect_state (
  struct connection_handler_args arg )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) ca_puser(arg.chid);

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    if ( arg.op == CA_OP_CONN_UP ) {

      axtdo->connection.setPvConnected( axtdo->fgPvId );

      if ( axtdo->connection.pvsConnected() ) {
        axtdo->needConnectInit = 1;
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      }

    }
    else {

      axtdo->connection.setPvDisconnected( axtdo->fgPvId );
      axtdo->fgColor.setDisconnected();
      axtdo->needRefresh = 1;
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );

    }

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextDspUpdate (
  struct event_handler_args ast_args
) {

class activeXTextDspClass *axtdo;
int ivalue;
short svalue;

  axtdo = (activeXTextDspClass *) ast_args.usr;

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    switch ( axtdo->pvType ) {

    case DBR_STRING:
      strncpy( axtdo->curValue, (char *) ast_args.dbr, 127 );
      axtdo->curValue[127] = 0;
      break;

    case DBR_FLOAT:
      axtdo->curDoubleValue = (double) *( (float *) ast_args.dbr );
      sprintf( axtdo->curValue, axtdo->format, axtdo->curDoubleValue );
      if ( !axtdo->noSval ) {
        if ( axtdo->nullDetectMode == 0 ) {
          if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
	else if ( axtdo->nullDetectMode == 1 ) {
          if ( axtdo->curSvalValue == 0 ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      break;

    case DBR_DOUBLE:
      axtdo->curDoubleValue = *( (double *) ast_args.dbr );
      sprintf( axtdo->curValue, axtdo->format, axtdo->curDoubleValue );
      if ( !axtdo->noSval ) {
        if ( axtdo->nullDetectMode == 0 ) {
          if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
	else if ( axtdo->nullDetectMode == 1 ) {
          if ( axtdo->curSvalValue == 0 ) {
            axtdo->fgColor.setNull();
            axtdo->bufInvalidate();
	  }
	  else {
            axtdo->fgColor.setNotNull();
            axtdo->bufInvalidate();
	  }
	}
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      break;

    case DBR_SHORT:
      ivalue = (int) *( (short *) ast_args.dbr );
      sprintf( axtdo->curValue, axtdo->format, ivalue );
      break;

    case DBR_LONG:
      ivalue = *( (int *) ast_args.dbr );
      sprintf( axtdo->curValue, axtdo->format, ivalue );
      break;

    case DBR_ENUM:
      svalue = *( (short *) ast_args.dbr );
      if ( ( svalue >= 0 ) && ( svalue < axtdo->numStates ) ) {
        strncpy( axtdo->curValue, axtdo->stateString[svalue], 127 );
	axtdo->curValue[127] = 0;
        axtdo->entryState = (int) svalue;
      }
      else {
        strcpy( axtdo->curValue, "" );
      }

      break;

    default:
      strcpy( axtdo->curValue, "" );
      break;

    } // end switch

    axtdo->needUpdate = 1;
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextDspSvalUpdate (
  struct event_handler_args ast_args
) {

class activeXTextDspClass *axtdo;

  axtdo = (activeXTextDspClass *) ast_args.usr;

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    switch ( axtdo->svalPvType ) {

    case DBR_LONG:

      axtdo->curSvalValue = (double) *( (int *) ast_args.dbr );

      if ( axtdo->nullDetectMode == 0 ) {
        if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else if ( axtdo->nullDetectMode == 1 ) {
        if ( axtdo->curSvalValue == 0 ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else {
        axtdo->fgColor.setNotNull();
      }

      break;

    case DBR_FLOAT:

      axtdo->curSvalValue = (double) *( (float *) ast_args.dbr );

      if ( axtdo->nullDetectMode == 0 ) {
        if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else if ( axtdo->nullDetectMode == 1 ) {
        if ( axtdo->curSvalValue == 0 ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else {
        axtdo->fgColor.setNotNull();
      }

      break;

    case DBR_DOUBLE:

      axtdo->curSvalValue = *( (double *) ast_args.dbr );

      if ( axtdo->nullDetectMode == 0 ) {
        if ( axtdo->curDoubleValue == axtdo->curSvalValue ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else if ( axtdo->nullDetectMode == 1 ) {
        if ( axtdo->curSvalValue == 0 ) {
          axtdo->fgColor.setNull();
        }
        else {
          axtdo->fgColor.setNotNull();
        }
      }
      else {
        axtdo->fgColor.setNotNull();
      }

      break;

    default:
      break;

    } // end switch

    axtdo->noSval = 0;
    axtdo->bufInvalidate();
    axtdo->needRefresh = 1;
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextDspFgUpdate (
  struct event_handler_args ast_args
) {

class activeXTextDspClass *axtdo;
unsigned int color;
int index;

  axtdo = (activeXTextDspClass *) ast_args.usr;

  axtdo->actWin->appCtx->proc->lock();

  if ( axtdo->activeMode ) {

    index = *( (int *) ast_args.dbr );
    color = axtdo->actWin->ci->getPixelByIndex( index );
    axtdo->fgColor.changeColor( color, axtdo->actWin->ci );
    axtdo->bufInvalidate();
    axtdo->needRefresh = 1;
    axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  }

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextAlarmUpdate (
  struct event_handler_args ast_args )
{

class activeXTextDspClass *axtdo;
struct dbr_sts_float floatStatusRec;
struct dbr_sts_double doubleStatusRec;
struct dbr_sts_short shortStatusRec;
struct dbr_sts_long longStatusRec;
struct dbr_sts_string stringStatusRec;
struct dbr_sts_enum enumStatusRec;
short svalue;

  axtdo = (activeXTextDspClass *) ast_args.usr;

  switch ( axtdo->pvType ) {

  case DBR_STRING:
    stringStatusRec = *( (struct dbr_sts_string *) ast_args.dbr );
    axtdo->fgColor.setStatus( stringStatusRec.status,
     stringStatusRec.severity );
    strncpy( axtdo->curValue, stringStatusRec.value, 127 );
    axtdo->curValue[127] = 0;
    break;

  case DBR_FLOAT:
    floatStatusRec = *( (struct dbr_sts_float *) ast_args.dbr );
    axtdo->fgColor.setStatus( floatStatusRec.status,
     floatStatusRec.severity );
    sprintf( axtdo->curValue, axtdo->format, floatStatusRec.value );
    break;

  case DBR_DOUBLE:
    doubleStatusRec = *( (struct dbr_sts_double *) ast_args.dbr );
    axtdo->fgColor.setStatus( doubleStatusRec.status,
     doubleStatusRec.severity );
    sprintf( axtdo->curValue, axtdo->format, doubleStatusRec.value );
    break;

  case DBR_SHORT:
    shortStatusRec = *( (struct dbr_sts_short *) ast_args.dbr );
    axtdo->fgColor.setStatus( shortStatusRec.status, shortStatusRec.severity );
    sprintf( axtdo->curValue, axtdo->format, shortStatusRec.value );
    break;

  case DBR_LONG:
    longStatusRec = *( (struct dbr_sts_long *) ast_args.dbr );
    axtdo->fgColor.setStatus( longStatusRec.status, longStatusRec.severity );
    sprintf( axtdo->curValue, axtdo->format, longStatusRec.value );
    break;

  case DBR_ENUM:
    enumStatusRec = *( (struct dbr_sts_enum *) ast_args.dbr );
    axtdo->fgColor.setStatus( enumStatusRec.status, enumStatusRec.severity );
    svalue = enumStatusRec.value;
    if ( ( svalue >= 0 ) && ( svalue < axtdo->numStates ) ) {
      strncpy( axtdo->curValue, axtdo->stateString[svalue], 127 );
      axtdo->curValue[127] = 0;
      axtdo->entryState = (int) svalue;
    }
    else {
      strcpy( axtdo->curValue, "" );
    }

    break;

  default:
    strcpy( axtdo->curValue, "" );
    break;

  } // end switch

  axtdo->needRefresh = 1;
  axtdo->actWin->appCtx->proc->lock();
  axtdo->actWin->addDefExeNode( axtdo->aglPtr );
  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextDspInfoUpdate (
  struct event_handler_args ast_args )
{

int i, n;
short svalue;
class activeXTextDspClass *axtdo = (activeXTextDspClass *) ast_args.usr;
struct dbr_gr_float floatInfoRec;
struct dbr_gr_double doubleInfoRec;
struct dbr_sts_string stringInfoRec;
struct dbr_gr_short shortInfoRec;
struct dbr_gr_long longInfoRec;
struct dbr_gr_enum enumInfoRec;

  switch ( axtdo->pvType ) {

  case DBR_STRING:

    stringInfoRec = *( (dbr_sts_string *) ast_args.dbr );

    axtdo->fgColor.setStatus( stringInfoRec.status, stringInfoRec.severity );

    break;

  case DBR_FLOAT:

    floatInfoRec = *( (dbr_gr_float *) ast_args.dbr );

    if ( axtdo->limitsFromDb || axtdo->efPrecision.isNull() ) {
      axtdo->precision = floatInfoRec.precision;
    }

    axtdo->fgColor.setStatus( floatInfoRec.status, floatInfoRec.severity );

    break;

  case DBR_DOUBLE:

    doubleInfoRec = *( (dbr_gr_double *) ast_args.dbr );

    if ( axtdo->limitsFromDb || axtdo->efPrecision.isNull() ) {
      axtdo->precision = doubleInfoRec.precision;
    }

    axtdo->fgColor.setStatus( doubleInfoRec.status, doubleInfoRec.severity );

    break;

  case DBR_SHORT:

    shortInfoRec = *( (dbr_gr_short *) ast_args.dbr );

    axtdo->fgColor.setStatus( shortInfoRec.status, shortInfoRec.severity );

    break;

  case DBR_LONG:

    longInfoRec = *( (dbr_gr_long *) ast_args.dbr );

    axtdo->fgColor.setStatus( longInfoRec.status, longInfoRec.severity );

    break;

  case DBR_ENUM:

    enumInfoRec = *( (dbr_gr_enum *) ast_args.dbr );

    n = enumInfoRec.no_str;

    for ( i=0; i<n; i++ ) {

      if ( axtdo->stateString[i] == NULL ) {
        axtdo->stateString[i] = new char[MAX_ENUM_STRING_SIZE+1];
      }

      strncpy( axtdo->stateString[i], enumInfoRec.strs[i],
       MAX_ENUM_STRING_SIZE );
      axtdo->stateString[i][MAX_ENUM_STRING_SIZE] = 0;

    }

    axtdo->numStates = n;

    svalue = enumInfoRec.value;
    if ( ( svalue >= 0 ) && ( svalue < axtdo->numStates ) ) {
      strncpy( axtdo->value, axtdo->stateString[svalue], 127 );
      axtdo->value[127] = 0;
      axtdo->entryState = (int) svalue;
    }
    else {
      strcpy( axtdo->value, "" );
    }

    strncpy( axtdo->curValue, axtdo->value, 127 );
    axtdo->curValue[127] = 0;

    axtdo->isWidget = 0;

    axtdo->fgColor.setStatus( enumInfoRec.status, enumInfoRec.severity );

    break;

  } // end switch ( pvType )

  axtdo->needInfoInit = 1;
  axtdo->actWin->appCtx->proc->lock();
  axtdo->actWin->addDefExeNode( axtdo->aglPtr );
  axtdo->actWin->appCtx->proc->unlock();

}

#endif

static void axtdc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
double dvalue;
int ivalue, stat;
short svalue;
char string[39+1];

  strncpy( axtdo->curValue, axtdo->entryValue, 39 );
  axtdo->curValue[39] = 0;

  switch ( axtdo->pvType ) {

  case DBR_FLOAT:
  case DBR_DOUBLE:
    if ( isLegalFloat(axtdo->entryValue) ) {
      dvalue = atof( axtdo->entryValue );
      if ( axtdo->pvExists ) {
#ifdef __epics__
        stat = ca_put( DBR_DOUBLE, axtdo->pvId, &dvalue );
#endif
      }
      else {
        axtdo->needUpdate = 1;
        axtdo->actWin->appCtx->proc->lock();
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
        axtdo->actWin->appCtx->proc->unlock();
      }

    }
    break;

  case DBR_SHORT:
  case DBR_LONG:
    if ( isLegalInteger(axtdo->entryValue) ) {
      ivalue = atol( axtdo->entryValue );
      if ( axtdo->pvExists ) {
#ifdef __epics__
        stat = ca_put( DBR_LONG, axtdo->pvId, &ivalue );
#endif
      }
      else {
        axtdo->needUpdate = 1;
        axtdo->actWin->appCtx->proc->lock();
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
        axtdo->actWin->appCtx->proc->unlock();
      }

    }
    break;

  case DBR_STRING:
    strncpy( string, axtdo->entryValue, 39 );
    string[39] = 0;
    if ( axtdo->pvExists ) {
#ifdef __epics__
      stat = ca_put( DBR_STRING, axtdo->pvId, &string );
#endif
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

    break;

  case DBR_ENUM:
    svalue = (short) axtdo->entryState;
    if ( axtdo->pvExists ) {
#ifdef __epics__
      stat = ca_put( DBR_ENUM, axtdo->pvId, &svalue );
#endif
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->appCtx->proc->lock();
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

    break;

  }

}

static void axtdc_value_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdc_value_edit_apply ( w, client, call );
  axtdo->textEntry.popdown();
  axtdo->editDialogIsActive = 0;

}

static void axtdc_value_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->textEntry.popdown();
  axtdo->editDialogIsActive = 0;

}

static void axtdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->actWin->setChanged();

  axtdo->eraseSelectBoxCorners();
  axtdo->erase();

  strncpy( axtdo->value, axtdo->bufPvName, 39 );
  axtdo->value[39] = 0;
  strncpy( axtdo->curValue, axtdo->bufPvName, 39 );
  axtdo->curValue[39] = 0;

  strncpy( axtdo->pvName, axtdo->bufPvName, 39 );
  axtdo->pvName[39] = 0;
  axtdo->pvExpStr.setRaw( axtdo->pvName );

  axtdo->svalPvExpStr.setRaw( axtdo->bufSvalPvName );

  axtdo->fgPvExpStr.setRaw( axtdo->fgCb.getPv() );

  strncpy( axtdo->fontTag, axtdo->fm.currentFontTag(), 63 );
  axtdo->fontTag[63] = 0;
  axtdo->actWin->fi->loadFontTag( axtdo->fontTag );
  axtdo->actWin->drawGc.setFontTag( axtdo->fontTag, axtdo->actWin->fi );

  axtdo->stringLength = strlen( axtdo->curValue );

  axtdo->fs = axtdo->actWin->fi->getXFontStruct( axtdo->fontTag );

  axtdo->updateFont( axtdo->curValue, axtdo->fontTag, &axtdo->fs,
   &axtdo->fontAscent, &axtdo->fontDescent, &axtdo->fontHeight,
   &axtdo->stringWidth );

  axtdo->stringY = axtdo->y + axtdo->fontAscent;

  axtdo->alignment = axtdo->fm.currentFontAlignment();

  if ( axtdo->alignment == XmALIGNMENT_BEGINNING )
    axtdo->stringX = axtdo->x;
  else if ( axtdo->alignment == XmALIGNMENT_CENTER )
    axtdo->stringX = axtdo->x + axtdo->w/2 - axtdo->stringWidth/2;
  else if ( axtdo->alignment == XmALIGNMENT_END )
    axtdo->stringX = axtdo->x + axtdo->w - axtdo->stringWidth;

  axtdo->useDisplayBg = axtdo->bufUseDisplayBg;

  axtdo->autoHeight = axtdo->bufAutoHeight;

  axtdo->formatType = axtdo->bufFormatType;

  axtdo->limitsFromDb = axtdo->bufLimitsFromDb;

  axtdo->changeValOnLoseFocus = axtdo->bufChangeValOnLoseFocus;

  axtdo->efPrecision = axtdo->bufEfPrecision;

  if ( axtdo->efPrecision.isNull() )
    axtdo->precision = 2;
  else
    axtdo->precision = axtdo->efPrecision.value();

  axtdo->fgColor.setConnectSensitive();

  axtdo->colorMode = axtdo->bufColorMode;

  axtdo->editable = axtdo->bufEditable;

  axtdo->isWidget = axtdo->bufIsWidget;

  axtdo->useKp = axtdo->bufUseKp;

  if ( axtdo->colorMode == XTDC_K_COLORMODE_ALARM )
    axtdo->fgColor.setAlarmSensitive();
  else
    axtdo->fgColor.setAlarmInsensitive();

  axtdo->fgColor.setColor( axtdo->bufFgColor, axtdo->actWin->ci );
  axtdo->fgColor.setNullColor ( axtdo->bufSvalColor );
  axtdo->bgColor = axtdo->bufBgColor;

  axtdo->nullDetectMode = axtdo->bufNullDetectMode;

  axtdo->smartRefresh = axtdo->bufSmartRefresh;

  strncpy( axtdo->id, axtdo->bufId, 31 );
  axtdo->id[31] = 0;
  axtdo->changeCallbackFlag = axtdo->bufChangeCallbackFlag;
  axtdo->activateCallbackFlag = axtdo->bufActivateCallbackFlag;
  axtdo->deactivateCallbackFlag = axtdo->bufDeactivateCallbackFlag;
  axtdo->anyCallbackFlag = axtdo->changeCallbackFlag ||
   axtdo->activateCallbackFlag || axtdo->deactivateCallbackFlag;

  axtdo->x = axtdo->bufX;
  axtdo->sboxX = axtdo->bufX;

  axtdo->y = axtdo->bufY;
  axtdo->sboxY = axtdo->bufY;

  axtdo->w = axtdo->bufW;
  axtdo->sboxW = axtdo->bufW;

  axtdo->h = axtdo->bufH;
  axtdo->sboxH = axtdo->bufH;

  axtdo->updateDimensions();

  if ( axtdo->autoHeight && axtdo->fs ) {
    axtdo->h = axtdo->fontHeight;
    axtdo->sboxH = axtdo->h;
  }

}

static void axtdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdc_edit_update( w, client, call );
  axtdo->refresh( axtdo );

}

static void axtdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdc_edit_update( w, client, call );
  axtdo->ef.popdown();
  axtdo->operationComplete();

}

static void axtdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->ef.popdown();
  axtdo->operationCancel();

}

static void axtdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;

  axtdo->erase();
  axtdo->deleteRequest = 1;
  axtdo->ef.popdown();
  axtdo->operationCancel();
  axtdo->drawAll();

}

activeXTextDspClass::activeXTextDspClass ( void ) {

int i;

  name = new char[strlen("activeXTextDspClass")+1];
  strcpy( name, "activeXTextDspClass" );
  strcpy( value, "" );

  editable = 0;
  smartRefresh = 0;
  isWidget = 0;
  useKp = 0;
  numStates = 0;
  entryState = 0;
  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    stateString[i] = NULL;
  }
  limitsFromDb = 1;
  changeValOnLoseFocus = 0;

  efPrecision.setNull(1);
  precision = 3;

  activeMode = 0;

  strcpy( id, "" );

  changeCallbackFlag = 0;
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;
  anyCallbackFlag = 0;
  changeCallback = NULL;
  activateCallback = NULL;
  deactivateCallback = NULL;

  nullDetectMode = 0;

  connection.setMaxPvs( 3 );

}

// copy constructor
activeXTextDspClass::activeXTextDspClass
 ( const activeXTextDspClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
int i;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeXTextDspClass")+1];
  strcpy( name, "activeXTextDspClass" );

  numStates = 0;
  entryState = 0;
  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    stateString[i] = NULL;
  }

  useDisplayBg = source->useDisplayBg;

  autoHeight = source->autoHeight;

  formatType = source->formatType;

  colorMode = source->colorMode;

  editable = source->editable;

  smartRefresh = source->smartRefresh;

  isWidget = source->isWidget;

  useKp = source->useKp;

  bgColor = source->bgColor;

  fgColor.copy(source->fgColor);

  strncpy( fontTag, source->fontTag, 63 );
  fontTag[63] = 0;
  strncpy( bufFontTag, source->bufFontTag, 63 );
  bufFontTag[63] = 0;

  fs = actWin->fi->getXFontStruct( fontTag );

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  svalCb = source->svalCb;

  strncpy( value, source->value, 127 );
  value[127] = 0;

  alignment = source->alignment;

  stringLength = source->stringLength;
  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;
  stringWidth = source->stringWidth;
  stringY = source->stringY;
  stringX = source->stringX;

  strncpy( pvName, source->pvName, 39 );
  pvName[39] = 0;

  pvExpStr.copy( source->pvExpStr );
  svalPvExpStr.copy( source->svalPvExpStr );
  fgPvExpStr.copy( source->fgPvExpStr );

  limitsFromDb = source->limitsFromDb;
  changeValOnLoseFocus = source->changeValOnLoseFocus;
  precision = source->precision;
  efPrecision = source->efPrecision;

  activeMode = 0;

  strcpy( id, source->id );

  changeCallbackFlag = source->changeCallbackFlag;
  activateCallbackFlag = source->activateCallbackFlag;
  deactivateCallbackFlag = source->deactivateCallbackFlag;
  anyCallbackFlag = changeCallbackFlag ||
   activateCallbackFlag || deactivateCallbackFlag;
  changeCallback = NULL;
  activateCallback = NULL;
  deactivateCallback = NULL;

  nullDetectMode = source->nullDetectMode;

  connection.setMaxPvs( 3 );

}

activeXTextDspClass::~activeXTextDspClass ( void ) {

int i;

  if ( name ) delete name;

  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    if ( stateString[i] ) {
      stateString[i] = NULL;
      delete stateString[i];
    }
  }

}

int activeXTextDspClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  strcpy( value, "" );
  strcpy( pvName, "" );

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  fgColor.setNullColor( actWin->defaultFg2Color );
  bgColor = actWin->defaultBgColor;

  useDisplayBg = 1;

  autoHeight = 1;

  formatType = XTDC_K_FORMAT_NATURAL;

  colorMode = XTDC_K_COLORMODE_STATIC;

  editable = 0;
  smartRefresh = 0;
  isWidget = 0;
  useKp = 0;

  strcpy( fontTag, actWin->defaultFontTag );

  actWin->fi->loadFontTag( fontTag );

  fs = actWin->fi->getXFontStruct( fontTag );

  alignment = actWin->defaultAlignment;

  if ( fs ) {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
  }
  else {
    fontAscent = 0;
    fontDescent = 0;
    fontHeight = 0;
  }

  updateDimensions();

  this->draw();

  this->editCreate();

  return 1;

}

int activeXTextDspClass::save (
  FILE *f )
{

int index, stat;

  fprintf( f, "%-d %-d %-d\n", XTDC_MAJOR_VERSION, XTDC_MINOR_VERSION,
   XTDC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );
  writeStringToFile( f, pvName );
  writeStringToFile( f, fontTag );
  fprintf( f, "%-d\n", useDisplayBg );
  fprintf( f, "%-d\n", alignment );
  actWin->ci->getIndex( fgColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );
  actWin->ci->getIndex( bgColor, &index );
  fprintf( f, "%-d\n", index );
  fprintf( f, "%-d\n", formatType );
  fprintf( f, "%-d\n", colorMode );
  fprintf( f, "%-d\n", editable );
  fprintf( f, "%-d\n", autoHeight );

  fprintf( f, "%-d\n", isWidget );

  fprintf( f, "%-d\n", limitsFromDb );
  stat = efPrecision.write( f );

  // version 1.5.0
  writeStringToFile( f, id );
  fprintf( f, "%-d\n", changeCallbackFlag );
  fprintf( f, "%-d\n", activateCallbackFlag );
  fprintf( f, "%-d\n", deactivateCallbackFlag );

  // version 1.6.0
  if ( svalPvExpStr.getRaw() )
    writeStringToFile( f, svalPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1.7.0
  actWin->ci->getIndex( fgColor.nullColor(), &index );
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", nullDetectMode );

  // version 1.8.0
  if ( fgPvExpStr.getRaw() )
    writeStringToFile( f, fgPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1.9.0
  fprintf( f, "%-d\n", smartRefresh );

  // version 2.1.0
  fprintf( f, "%-d\n", useKp );

  // version 2.2.0
  fprintf( f, "%-d\n", changeValOnLoseFocus );

  return 1;

}

int activeXTextDspClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
int stat = 1;
char oneName[39+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  readStringFromFile( pvName, 39, f ); actWin->incLine();
  readStringFromFile( fontTag, 63, f ); actWin->incLine();
  fscanf( f, "%d\n", &useDisplayBg ); actWin->incLine();
  fscanf( f, "%d\n", &alignment ); actWin->incLine();

  if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &bufFgColor );
    fgColor.setColor( bufFgColor, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &bgColor );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &bufFgColor );
    fgColor.setColor( bufFgColor, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &bgColor );

  }

  fscanf( f, "%d\n", &formatType ); actWin->incLine();
  fscanf( f, "%d\n", &colorMode ); actWin->incLine();
  fscanf( f, "%d\n", &editable ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    fscanf( f, "%d\n", &autoHeight ); actWin->incLine();
  }
  else {
    autoHeight = 0;
  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    fscanf( f, "%d\n", &isWidget ); actWin->incLine();
  }
  else {
    isWidget = 0;
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    fscanf( f, "%d\n", &limitsFromDb ); actWin->incLine();
    stat = efPrecision.read( f ); actWin->incLine();
    if ( limitsFromDb || efPrecision.isNull() )
      precision = 3;
    else
      precision = efPrecision.value();
  }
  else {
    limitsFromDb = 1;
    precision = 3;
    efPrecision.setValue( 3 );
  }

  if ( ( major > 1 ) || ( minor > 4 ) ) {
    readStringFromFile( this->id, 31, f ); actWin->incLine();
    fscanf( f, "%d\n", &changeCallbackFlag ); actWin->incLine();
    fscanf( f, "%d\n", &activateCallbackFlag ); actWin->incLine();
    fscanf( f, "%d\n", &deactivateCallbackFlag ); actWin->incLine();
    anyCallbackFlag = changeCallbackFlag ||
     activateCallbackFlag || deactivateCallbackFlag;
  }
  else {
    strcpy( this->id, "" );
    changeCallbackFlag = 0;
    activateCallbackFlag = 0;
    deactivateCallbackFlag = 0;
    anyCallbackFlag = 0;
  }

  if ( colorMode == XTDC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  strncpy( value, pvName, 39 );
  value[39] = 0;

  pvExpStr.setRaw( pvName );

  if ( ( major > 1 ) || ( minor > 5 ) ) {

    readStringFromFile( oneName, 39, f ); actWin->incLine();
    svalPvExpStr.setRaw( oneName );

    if ( major > 1 ) {

      fscanf( f, "%d\n", &index ); actWin->incLine();
      actWin->ci->setIndex( index, &bufSvalColor );
      fgColor.setNullColor( bufSvalColor );

    }
    else {

      fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
      actWin->ci->setRGB( r, g, b, &bufSvalColor );
      fgColor.setNullColor( bufSvalColor );

    }

  }
  else {

    svalPvExpStr.setRaw( "" );
    bufSvalColor = bufFgColor;

  }

  if ( ( major > 1 ) || ( minor > 6 ) ) {

    fscanf( f, "%d\n", &nullDetectMode ); actWin->incLine();

  }
  else {

    nullDetectMode = 0;

  }

  if ( ( major > 1 ) || ( minor > 7 ) ) {

    readStringFromFile( oneName, 39, f ); actWin->incLine();
    fgPvExpStr.setRaw( oneName );

  }
  else {

    fgPvExpStr.setRaw( "" );

  }

  if ( ( major > 1 ) || ( minor > 8 ) ) {

    fscanf( f, "%d\n", &smartRefresh ); actWin->incLine();

  }
  else {

    smartRefresh = 0;

  }

  if ( ( ( major == 1 ) && ( minor > 0 ) ) || ( major > 1 ) ) {

    fscanf( f, "%d\n", &useKp ); actWin->incLine();

  }
  else {

    useKp = 0;

  }

  if ( ( ( major == 2 ) && ( minor > 1 ) ) || ( major > 2 ) ) {

    fscanf( f, "%d\n", &changeValOnLoseFocus ); actWin->incLine();

  }
  else {

    changeValOnLoseFocus = 1; // older version behavior

  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  stringLength = strlen( value );

  fs = actWin->fi->getXFontStruct( fontTag );

  updateFont( value, fontTag, &fs, &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  stringY = y + fontAscent;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  return stat;

}

int activeXTextDspClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, more;
int stat = 1;
char *tk, *gotData, *context, buf[255+1];

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;

  this->actWin = _actWin;

  strcpy( value, "" );
  strcpy( pvName, "" );

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  bgColor = actWin->defaultBgColor;

  useDisplayBg = 1;

  autoHeight = 1;

  formatType = XTDC_K_FORMAT_NATURAL;

  colorMode = XTDC_K_COLORMODE_STATIC;

  editable = 0;
  smartRefresh = 0;
  isWidget = 0;
  useKp = 0;

  strcpy( fontTag, actWin->defaultFontTag );

  alignment = actWin->defaultAlignment;

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( activeXTextDspClass_str3 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( activeXTextDspClass_str3 );
      return 0;
    }

    if ( strcmp( tk, "<eod>" ) == 0 ) {

      more = 0;

    }
    else {

      more = 1;

      if ( strcmp( tk, "x" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "ctlpv" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        strncpy( pvName, tk, 28 );
	pvName[28] = 0;

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        strncpy( fontTag, tk, 63 );
	fontTag[63] = 0;

      }
            
      else if ( strcmp( tk, "justify" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        alignment = atol( tk );

      }
            
      else if ( strcmp( tk, "red" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        r = atol( tk );

      }
            
      else if ( strcmp( tk, "green" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        g = atol( tk );

      }
            
      else if ( strcmp( tk, "blue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextDspClass_str3 );
          return 0;
        }

        b = atol( tk );

      }
            
    }

  } while ( more );

  actWin->ci->setRGB( r, g, b, &bufFgColor );
  fgColor.setColor( bufFgColor, actWin->ci );

  limitsFromDb = 1;
  changeValOnLoseFocus = 0;
  precision = 3;
  efPrecision.setValue( 3 );

  fgColor.setAlarmInsensitive();

  strncpy( value, pvName, 39 );
  value[39] = 0;

  pvExpStr.setRaw( pvName );

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  stringLength = strlen( value );

  fs = actWin->fi->getXFontStruct( fontTag );

  updateFont( value, fontTag, &fs, &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  y = y + fontDescent;

  this->initSelectBox(); // call after getting x,y,w,h

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  stringY = y + fontAscent;

  return stat;

}

int activeXTextDspClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeXTextDspClass" );
  if ( ptr ) {
    strncpy( title, ptr, 31 );
    title[31] = 0;
  }
  else {
    strncpy( title, activeXTextDspClass_str4, 31 );
    title[31] = 0;
  }

  strncat( title, activeXTextDspClass_str5, 31 );
  title[31] = 0;

  strncpy( bufId, id, 31 );
  bufId[31] = 0;

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;
  bufFgColor = fgColor.pixelColor();
  bufBgColor = bgColor;
  strncpy( bufFontTag, fontTag, 63 );
  bufFontTag[63] = 0;
  bufUseDisplayBg = useDisplayBg;
  bufAutoHeight = autoHeight;
  bufFormatType = formatType;
  bufColorMode = colorMode;
  strncpy( bufValue, value, 127 );
  bufValue[127] = 0;
  strncpy( bufPvName, pvName, 39 );
  bufPvName[39] = 0;
  if ( svalPvExpStr.getRaw() ) {
    strncpy( bufSvalPvName, svalPvExpStr.getRaw(), 39 );
    bufSvalPvName[39] = 0;
  }
  else {
    strncpy( bufSvalPvName, "", 39 );
    bufSvalPvName[39] = 0;
  }
  bufSvalColor = fgColor.nullColor();
  bufNullDetectMode = nullDetectMode;

  bufEditable = editable;
  bufSmartRefresh = smartRefresh;
  bufIsWidget = isWidget;
  bufUseKp = useKp;
  bufLimitsFromDb = limitsFromDb;
  bufChangeValOnLoseFocus = changeValOnLoseFocus;
  bufEfPrecision = efPrecision;
  bufChangeCallbackFlag = changeCallbackFlag;
  bufActivateCallbackFlag = activateCallbackFlag;
  bufDeactivateCallbackFlag = deactivateCallbackFlag;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeXTextDspClass_str6, 27, bufId, 31 );
  ef.addTextField( activeXTextDspClass_str7, 27, &bufX );
  ef.addTextField( activeXTextDspClass_str8, 27, &bufY );
  ef.addTextField( activeXTextDspClass_str9, 27, &bufW );
  ef.addTextField( activeXTextDspClass_str10, 27, &bufH );
  ef.addToggle( activeXTextDspClass_str11, &bufAutoHeight );
  ef.addFontMenu( activeXTextDspClass_str12, actWin->fi, &fm, fontTag );
  fm.setFontAlignment( alignment );
  ef.addOption( activeXTextDspClass_str13, activeXTextDspClass_str14, &bufColorMode );
  ef.addColorButtonWithText( activeXTextDspClass_str15, actWin->ci,
    &fgCb, &bufFgColor, 22, fgPvExpStr.getRaw() );
  ef.addColorButton( activeXTextDspClass_str16, actWin->ci, &bgCb, &bufBgColor );
  ef.addToggle( activeXTextDspClass_str17, &bufUseDisplayBg );
  ef.addOption( activeXTextDspClass_str18,
   activeXTextDspClass_str19, &bufFormatType );
  ef.addToggle( activeXTextDspClass_str20, &bufLimitsFromDb );
  ef.addTextField( activeXTextDspClass_str21, 27, &bufEfPrecision );
  ef.addTextField( activeXTextDspClass_str22, 27, bufPvName, 39 );
  ef.addOption( activeXTextDspClass_str23,
   activeXTextDspClass_str24,
   &bufNullDetectMode );
  ef.addTextField( activeXTextDspClass_str25, 27, bufSvalPvName, 39 );
  ef.addColorButton( activeXTextDspClass_str26, actWin->ci, &svalCb, &bufSvalColor );
  ef.addToggle( activeXTextDspClass_str27, &bufEditable );
  ef.addToggle( activeXTextDspClass_str67, &bufUseKp );
  ef.addToggle( activeXTextDspClass_str28, &bufSmartRefresh );
  ef.addToggle( activeXTextDspClass_str29, &bufIsWidget );
  ef.addToggle( activeXTextDspClass_str68, &bufChangeValOnLoseFocus );
  ef.addToggle( activeXTextDspClass_str30, &bufActivateCallbackFlag );
  ef.addToggle( activeXTextDspClass_str31, &bufDeactivateCallbackFlag );
  ef.addToggle( activeXTextDspClass_str32, &bufChangeCallbackFlag );

  return 1;

}

int activeXTextDspClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( axtdc_edit_ok, axtdc_edit_apply, axtdc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeXTextDspClass::edit ( void ) {

  this->genericEdit();
  ef.finished( axtdc_edit_ok, axtdc_edit_apply, axtdc_edit_cancel, this );
  fm.setFontAlignment( alignment );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeXTextDspClass::erase ( void ) {

XRectangle xR = { x, y, w, h };

  if ( activeMode || deleteRequest ) return 1;

  actWin->drawGc.addEraseXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    XDrawString( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), stringX, stringY,
     value, stringLength );

  }
  else {

    XDrawImageString( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), stringX, stringY,
     value, stringLength );

  }

  actWin->drawGc.removeEraseXClipRectangle();

  return 1;

}

int activeXTextDspClass::eraseActive ( void ) {

int len;

  if ( !init || !activeMode ) return 1;

  if ( isWidget ) return 1;

  if ( !bufInvalid && ( strlen(value) == strlen(bufValue) ) ) {
    if ( strcmp( value, bufValue ) == 0 ) return 1;
  }

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  len = strlen(bufValue);

  if ( useDisplayBg ) {

    XDrawString( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), stringX, stringY,
     bufValue, len );

  }
  else {

    actWin->executeGc.saveFg();
    actWin->executeGc.saveBg();

    actWin->executeGc.setFG( bgColor );
    actWin->executeGc.setBG( bgColor );

    XDrawImageString( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), stringX, stringY,
     bufValue, len );

    actWin->executeGc.restoreFg();
    actWin->executeGc.restoreBg();

  }

  return 1;

}

int activeXTextDspClass::draw ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;

  if ( activeMode || deleteRequest ) return 1;

  actWin->drawGc.saveFg();
  actWin->drawGc.saveBg();

  actWin->drawGc.setFG( fgColor.pixelColor() );
  actWin->drawGc.setBG( bgColor );

  clipStat = actWin->drawGc.addNormXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    XDrawString( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), stringX, stringY,
     value, stringLength );

  }
  else {

    XDrawImageString( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), stringX, stringY,
     value, stringLength );

  }

  if ( clipStat & 1 ) actWin->drawGc.removeNormXClipRectangle();

  actWin->drawGc.restoreFg();
  actWin->drawGc.restoreBg();

  return 1;

}

int activeXTextDspClass::drawActive ( void ) {

Arg args[10];
int n;

  if ( !activeMode || !init ) return 1;

  if ( isWidget ) {

     if ( tf_widget ) {

       strncpy( entryValue, value, 39 );
       entryValue[39] = 0;
       n = 0;
       XtSetArg( args[n], XmNvalue, (XtArgVal) entryValue ); n++;

       XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;

       XtSetValues( tf_widget, args, n );

     }

     return 1;

  }

  if ( !bufInvalid && ( strlen(value) == strlen(bufValue) ) ) {
    if ( strcmp( value, bufValue ) == 0 ) return 1;
  }

  actWin->executeGc.saveFg();
  actWin->executeGc.saveBg();

  actWin->executeGc.setFG( fgColor.getColor() );

  actWin->executeGc.setBG( bgColor );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  updateDimensions();

  if ( useDisplayBg ) {

    XDrawString( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), stringX, stringY,
     value, stringLength );

  }
  else {

    XDrawImageString( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), stringX, stringY, 
     value, stringLength );

  }

  actWin->executeGc.restoreFg();
  actWin->executeGc.restoreBg();

  strncpy( bufValue, value, 127 );
  bufValue[127] = 0;
  bufInvalid = 0;

  return 1;

}

void activeXTextDspClass::bufInvalidate ( void ) {

  bufInvalid = 1;

}

int activeXTextDspClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int stat;

  stat = pvExpStr.expand1st( numMacros, macros, expansions );
  stat = svalPvExpStr.expand1st( numMacros, macros, expansions );
  stat = fgPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeXTextDspClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int stat;

  stat = pvExpStr.expand2nd( numMacros, macros, expansions );
  stat = svalPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = fgPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeXTextDspClass::containsMacros ( void ) {

int result;

  result = pvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = svalPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = fgPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  return 0;

}

int activeXTextDspClass::activate (
  int pass,
  void *ptr )
{

int stat;
char callbackName[63+1];

  switch ( pass ) {

  case 1:

    deferredCount = 0;
    needConnectInit = needInfoInit = needErase = needDraw = needRefresh =
     needUpdate = 0;
    aglPtr = ptr;
    strcpy( curValue, "" );
    strcpy( value, "" );
    strcpy( bufValue, "" );
    updateDimensions();
    tf_widget = NULL;
    opComplete = 0;
    numStates = 0; // for enum type
    editDialogIsActive = 0;
    activeMode = 1;
    init = 0;
    curDoubleValue = 0.0;
    curSvalValue = 0.0;
    noSval = 1;

    pvExistCheck = 0;
    connection.init();

    break;

  case 2:

    if ( !opComplete ) {

      fgColor.setNotNull();

      if ( !pvExistCheck ) {

        pvExistCheck = 1;

        if ( pvExpStr.getExpanded() ) {
          if ( strcmp( pvExpStr.getExpanded(), "" ) != 0 ) {
            pvExists = 1;
            connection.addPv(); // must do this only once per pv
  	}
  	else {
            pvExists = 0;
  	}
        }
        else {
          pvExists = 0;
        }

        if ( svalPvExpStr.getExpanded() ) {
          if ( strcmp( svalPvExpStr.getExpanded(), "" ) != 0 ) {
            svalPvExists = 1;
            connection.addPv(); // must do this only once per pv
  	}
  	else {
            svalPvExists = 0;
  	}
        }
        else {
          svalPvExists = 0;
        }

        if ( fgPvExpStr.getExpanded() ) {
          if ( strcmp( fgPvExpStr.getExpanded(), "" ) != 0 ) {
            fgPvExists = 1;
            connection.addPv(); // must do this only once per pv
  	}
  	else {
            fgPvExists = 0;
  	}
        }
        else {
          fgPvExists = 0;
        }

      }

#ifdef __epics__
      eventId = 0;
      alarmEventId = 0;
      svalEventId = 0;
      fgEventId = 0;
#endif

      if ( pvExists ) {

#ifdef __epics__
        stat = ca_search_and_connect( pvExpStr.getExpanded(), &pvId,
         xtdo_monitor_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str33 );
          return 0;
        }

        if ( svalPvExists ) {

          stat = ca_search_and_connect( svalPvExpStr.getExpanded(), &svalPvId,
           xtdo_monitor_sval_connect_state, this );
          if ( stat != ECA_NORMAL ) {
            printf( activeXTextDspClass_str34 );
            return 0;
          }

        }

        if ( fgPvExists ) {

          stat = ca_search_and_connect( fgPvExpStr.getExpanded(), &fgPvId,
           xtdo_monitor_fg_connect_state, this );
          if ( stat != ECA_NORMAL ) {
            printf( activeXTextDspClass_str35 );
            return 0;
          }

        }

#endif

      }
      else if ( anyCallbackFlag ) {

        needInfoInit = 1;
        actWin->appCtx->proc->lock();
        actWin->addDefExeNode( aglPtr );
        actWin->appCtx->proc->unlock();

      }

      if ( anyCallbackFlag ) {

        if ( changeCallbackFlag ) {
          strncpy( callbackName, id, 63 );
	  callbackName[63] = 0;
          strncat( callbackName, activeXTextDspClass_str36, 63 );
          callbackName[63] = 0;
          changeCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
	  callbackName[63] = 0;
          strncat( callbackName, activeXTextDspClass_str37, 63 );
          callbackName[63] = 0;
          activateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( deactivateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
	  callbackName[63] = 0;
          strncat( callbackName, activeXTextDspClass_str38, 63 );
          callbackName[63] = 0;
          deactivateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallback ) {
          (*activateCallback)( this );
        }

      }

      opComplete = 1;

    }

    break;

  case 3:
  case 4:
  case 5:
  case 6:

    break;

  } // end switch

  return 1;

}

int activeXTextDspClass::deactivate (
  int pass
) {

int stat;

  if ( pass == 1 ) {

  activeMode = 0;

  if ( kp.isPoppedUp() ) {
    kp.popdown();
  }

  if ( deactivateCallback ) {
    (*deactivateCallback)( this );
  }

#ifdef __epics__

  if ( pvExists ) {

    if ( eventId ) {
      stat = ca_clear_event( eventId );
      eventId = 0;
      if ( stat != ECA_NORMAL )
        printf( activeXTextDspClass_str39 );
    }

    if ( alarmEventId ) {
      stat = ca_clear_event( alarmEventId );
      alarmEventId = 0;
      if ( stat != ECA_NORMAL )
        printf( activeXTextDspClass_str40 );
    }

    stat = ca_clear_channel( pvId );
    if ( stat != ECA_NORMAL )
      printf( activeXTextDspClass_str41 );

  }

  if ( svalPvExists ) {

    stat = ca_clear_channel( svalPvId );
    if ( stat != ECA_NORMAL )
      printf( activeXTextDspClass_str42 );

  }

  if ( fgPvExists ) {

    stat = ca_clear_channel( fgPvId );
    if ( stat != ECA_NORMAL )
      printf( activeXTextDspClass_str43 );

  }

#endif

  }
  else if ( pass == 2 ) {

  if ( tf_widget ) {
    XtDestroyWidget( tf_widget );
    tf_widget = NULL;
  }

  strcpy( value, pvName );
  strcpy( curValue, pvName );
  updateDimensions();

  }

  return 1;

}

void activeXTextDspClass::updateDimensions ( void )
{

  stringLength = strlen( value );

  if ( fs ) {
    stringWidth = XTextWidth( fs, value, stringLength );
  }
  else {
    stringWidth = 0;
  }

  stringY = y + fontAscent;

  stringX = x;

  if ( alignment == XmALIGNMENT_BEGINNING ) {
    // no change
  }
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

}

void activeXTextDspClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

}

void activeXTextDspClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

char selectString[127+1];
int i;

  if ( !editable || isWidget ) return;

  if ( buttonNumber != 1 ) return;

  if ( editDialogIsActive ) return;

  teX = x + actWin->x;
  teY = this->y + actWin->y + h;
  teW = w;
  teH = h;
  teLargestH = 600;

  if ( useKp ) {
    if ( ( pvType == DBR_FLOAT ) || ( pvType == DBR_DOUBLE ) ) {
      kp.create( actWin->top, teX, teY, "", &kpDouble,
       (void *) this,
       (XtCallbackProc) xtdoSetKpDoubleValue,
       (XtCallbackProc) xtdoCancelKp );
      editDialogIsActive = 1;
      return;
    }
    else if ( ( pvType == DBR_SHORT ) || ( pvType == DBR_LONG ) ) {
      kp.create( actWin->top, teX, teY, "", &kpInt,
       (void *) this,
       (XtCallbackProc) xtdoSetKpIntValue,
       (XtCallbackProc) xtdoCancelKp );
      editDialogIsActive = 1;
      return;
    }
  }

  strncpy( entryValue, value, 127 );
  entryValue[127] = 0;

  textEntry.create( actWin->top, &teX, &teY, &teW, &teH, &teLargestH, "",
  NULL, NULL, NULL );

#ifdef __epics__

  if ( pvType != DBR_ENUM ) {
    textEntry.addTextField( activeXTextDspClass_str44, 25, entryValue, 127 );
  }
  else {
    strcpy( selectString, "" );
    for ( i=0; i<numStates; i++ ) {
      strncat( selectString, stateString[i], 127 );
      selectString[127] = 0;
      if ( i != numStates-1 ) {
        strncat( selectString, "|", 127 );
        selectString[127] = 0;
      }
    }
    textEntry.addOption( activeXTextDspClass_str45, selectString, &entryState );
  }

#endif

  textEntry.finished( axtdc_value_edit_ok, axtdc_value_edit_apply,
   axtdc_value_edit_cancel, this );

  textEntry.popup();
  editDialogIsActive = 1;

}

int activeXTextDspClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag )
{

  *down = 1;
  *up = 0;
  *drag = 0;

  return 1;

}

static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

class activeXTextDspClass *atdo;
int stat;

  XtVaGetValues( w, XmNuserData, &atdo, NULL );

  stat = atdo->startDrag( w, e );

}

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

class activeXTextDspClass *atdo;
int stat;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &atdo, NULL );

  stat = atdo->selectDragValue( atdo->x + be->x, atdo->y + be->y );

}

void activeXTextDspClass::executeDeferred ( void ) {

int n, stat, numCols, width, csrPos;
int nc, ni, nu, nr, nd, ne;
Arg args[10];
unsigned int bg;
XmFontList textFontList = NULL;

#if 0
XtTranslations parsedTrans;

static char dragTrans[] =
  "#override\n\
   ~Shift<Btn2Down>: startDrag()\n\
   Shift<Btn2Up>: selectDrag()";

static XtActionsRec dragActions[] = {
  { "startDrag", (XtActionProc) drag },
  { "selectDrag", (XtActionProc) selectDrag }
};
#endif

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  if ( !needConnectInit && !needInfoInit && !needRefresh ) {
    deferredCount--;
    if ( deferredCount > 0 ) {
      actWin->appCtx->proc->unlock();
      return;
    }
    deferredCount = actWin->appCtx->proc->halfSecCount;
  }
  actWin->appCtx->proc->unlock();

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  ne = needErase; needErase = 0;
  strncpy( value, curValue, 127 );
  value[127] = 0;
  actWin->appCtx->proc->unlock();

#ifdef __epics__

  if ( nc ) {

    switch ( pvType ) {

    case DBR_STRING:

      stat = ca_get_callback( DBR_GR_STRING, pvId,
       XtextDspInfoUpdate, (void *) this );
      if ( stat != ECA_NORMAL ) {
        printf( activeXTextDspClass_str46 );
      }

      break;

    case DBR_FLOAT:
    case DBR_DOUBLE:

      stat = ca_get_callback( DBR_GR_DOUBLE, pvId,
       XtextDspInfoUpdate, (void *) this );
      if ( stat != ECA_NORMAL ) {
        printf( activeXTextDspClass_str47 );
      }

      break;

    case DBR_SHORT:
    case DBR_LONG:

      stat = ca_get_callback( DBR_GR_LONG, pvId,
       XtextDspInfoUpdate, (void *) this );
      if ( stat != ECA_NORMAL ) {
        printf( activeXTextDspClass_str48 );
      }

      break;

    case DBR_ENUM:

      stat = ca_get_callback( DBR_GR_ENUM, pvId,
       XtextDspInfoUpdate, (void *) this );
      if ( stat != ECA_NORMAL ) {
        printf( activeXTextDspClass_str49 );
      }
       break;

    } // end switch

    bufInvalidate();

  }
#endif

  if ( ni ) {

#ifdef __epics__
    if ( fgPvExists ) {
      if ( !fgEventId ) {
        stat = ca_add_masked_array_event( DBR_LONG, 1, fgPvId,
         XtextDspFgUpdate, (void *) this, (float) 0.0, (float) 0.0,
         (float) 0.0, &fgEventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str50 );
        }
      }
    }
#endif

    if ( pvExists ) {

    switch ( pvType ) {

    case DBR_STRING:

      sprintf( format, "%%s" );

#ifdef __epics__
      if ( !eventId ) {
        stat = ca_add_masked_array_event( DBR_STRING, 1, pvId,
         XtextDspUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
         &eventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str51 );
        }
      }

      if ( !alarmEventId ) {
        stat = ca_add_masked_array_event( DBR_STS_STRING, 1, pvId,
         XtextAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
         (float) 0.0, &alarmEventId, DBE_ALARM );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str52 );
        }
      }

#endif

      break;

    case DBR_FLOAT:

      switch( formatType ) {
      case XTDC_K_FORMAT_FLOAT:
        sprintf( format, "%%.%-df", precision );
        break;
      case XTDC_K_FORMAT_EXPONENTIAL:
        sprintf( format, "%%.%-de", precision );
        break;
      default:
        sprintf( format, "%%.%-dg", precision );
        break;
      } // end switch( formatType )
  
#ifdef __epics__

      if ( !eventId ) {
        stat = ca_add_masked_array_event( DBR_FLOAT, 1, pvId,
         XtextDspUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
         &eventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str53 );
        }
      }

      if ( !alarmEventId ) {
        stat = ca_add_masked_array_event( DBR_STS_FLOAT, 1, pvId,
         XtextAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
         (float) 0.0, &alarmEventId, DBE_ALARM );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str54 );
        }
      }

      if ( svalPvExists ) {
        if ( !svalEventId ) {
          stat = ca_add_masked_array_event( svalPvType, 1, svalPvId,
           XtextDspSvalUpdate, (void *) this, (float) 0.0, (float) 0.0,
           (float) 0.0, &svalEventId, DBE_VALUE );
          if ( stat != ECA_NORMAL ) {
            printf( activeXTextDspClass_str55 );
          }
        }
      }

#endif

      break;

    case DBR_DOUBLE:

      switch( formatType ) {
      case XTDC_K_FORMAT_FLOAT:
        sprintf( format, "%%.%-df", precision );
        break;
      case XTDC_K_FORMAT_EXPONENTIAL:
        sprintf( format, "%%.%-de", precision );
        break;
      default:
        sprintf( format, "%%.%-dg", precision );
        break;
      } // end switch( formatType )

#ifdef __epics__

      if ( !eventId ) {
        stat = ca_add_masked_array_event( DBR_DOUBLE, 1, pvId,
         XtextDspUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
         &eventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str56 );
        }
      }

      if ( !alarmEventId ) {
        stat = ca_add_masked_array_event( DBR_STS_DOUBLE, 1, pvId,
         XtextAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
         (float) 0.0, &alarmEventId, DBE_ALARM );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str57 );
        }
      }

      if ( svalPvExists ) {
        if ( !svalEventId ) {
          stat = ca_add_masked_array_event( svalPvType, 1, svalPvId,
           XtextDspSvalUpdate, (void *) this, (float) 0.0, (float) 0.0,
           (float) 0.0, &svalEventId, DBE_VALUE );
          if ( stat != ECA_NORMAL ) {
            printf( activeXTextDspClass_str58 );
          }
        }
      }

#endif

      break;

    case DBR_SHORT:

      switch( formatType ) {
      case XTDC_K_FORMAT_DECIMAL:
        sprintf( format, "%%-d" );
        break;
      case XTDC_K_FORMAT_HEX:
        sprintf( format, "%%-X" );
        break;
      default:
        sprintf( format, "%%-d" );
        break;
      } // end switch( formatType )

#ifdef __epics__

      if ( !eventId ) {
        stat = ca_add_masked_array_event( DBR_SHORT, 1, pvId,
         XtextDspUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
         &eventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str59 );
        }
      }

      if ( !alarmEventId ) {
        stat = ca_add_masked_array_event( DBR_STS_SHORT, 1, pvId,
         XtextAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
         (float) 0.0, &alarmEventId, DBE_ALARM );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str60 );
        }
      }

      break;

#endif

    case DBR_LONG:

      switch( formatType ) {
      case XTDC_K_FORMAT_DECIMAL:
        sprintf( format, "%%-d" );
        break;
      case XTDC_K_FORMAT_HEX:
        sprintf( format, "%%-X" );
        break;
      default:
        sprintf( format, "%%-d" );
        break;
      } // end switch( formatType )

#ifdef __epics__

      if ( !eventId ) {
        stat = ca_add_masked_array_event( DBR_LONG, 1, pvId,
         XtextDspUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
         &eventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str61 );
        }
      }

      if ( !alarmEventId ) {
        stat = ca_add_masked_array_event( DBR_STS_LONG, 1, pvId,
         XtextAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
         (float) 0.0, &alarmEventId, DBE_ALARM );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str62 );
        }
      }

#endif

      break;

    case DBR_ENUM:

      sprintf( format, "%%s" );

#ifdef __epics__

      if ( !eventId ) {
        stat = ca_add_masked_array_event( DBR_ENUM, 1, pvId,
         XtextDspUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
         &eventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str63 );
        }
      }

      if ( !alarmEventId ) {
        stat = ca_add_masked_array_event( DBR_STS_ENUM, 1, pvId,
         XtextAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
         (float) 0.0, &alarmEventId, DBE_ALARM );
        if ( stat != ECA_NORMAL ) {
          printf( activeXTextDspClass_str64 );
        }
      }

#endif

      break;

    default:
      sprintf( format, "%%s" );
      break;

    } // end switch ( pvType )

    }
    else {

      pvType = DBR_STRING;
      sprintf( format, "%%s" );

    }

    if ( isWidget ) {

      if ( fontTag ) {
        actWin->fi->getTextFontList( fontTag, &textFontList );
      }
      else {
        textFontList = NULL;
      }

      if ( fs ) {
        width = XTextWidth( fs, "1", 1 );
        numCols = (int) ( (float) ( w / width ) + 0.5 );
        if ( numCols < 1 ) numCols = 1;
      }
      else {
        numCols = 6;
      }

      strncpy( entryValue, value, 39 );
      entryValue[39] = 0;
      csrPos = strlen(entryValue);

      widget_value_changed = 0;

      if ( useDisplayBg )
        bg = actWin->executeGc.getBaseBG();
      else
        bg = bgColor;

      if ( !tf_widget ) {

#if 0
      parsedTrans = XtParseTranslationTable( dragTrans );
      XtAppAddActions( actWin->appCtx->appContext(), dragActions,
       XtNumber(dragActions) );
#endif

      tf_widget = XtVaCreateManagedWidget( "", xmTextWidgetClass,
       actWin->executeWidget,
       XmNx, x,
       XmNy, y-3,
       XmNforeground, fgColor.getColor(),
       XmNbackground, bg,
       XmNhighlightThickness, 0,
       XmNcolumns, (short) numCols,
       XmNvalue, entryValue,
       XmNmaxLength, (short) 39,
       XmNpendingDelete, True,
       XmNmarginHeight, 0,
       XmNfontList, textFontList,
       // XmNtranslations, parsedTrans,
       XmNuserData, this,
       NULL );

      if ( textFontList ) XmFontListFree( textFontList );

      if ( !editable ) {

        n = 0;
        XtSetArg( args[n], XmNeditable, (XtArgVal) False ); n++;
        XtSetArg( args[n], XmNnavigationType, (XtArgVal) XmNONE ); n++;
        XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) False ); n++;
        XtSetValues( tf_widget, args, n );

      }
      else {

        XmTextSetInsertionPosition( tf_widget, csrPos );

        XtAddCallback( tf_widget, XmNfocusCallback,
         xtdoSetSelection, this );

        XtAddCallback( tf_widget, XmNvalueChangedCallback,
         xtdoSetValueChanged, this );

        switch ( pvType ) {

        case DBR_STRING:

          XtAddCallback( tf_widget, XmNactivateCallback,
           xtdoTextFieldToStringA, this );

          if ( changeValOnLoseFocus ) {
            XtAddCallback( tf_widget, XmNlosingFocusCallback,
             xtdoTextFieldToStringLF, this );
	  }
          break;

        case DBR_SHORT:
        case DBR_LONG:

          XtAddCallback( tf_widget, XmNactivateCallback,
           xtdoTextFieldToIntA, this );

          if ( changeValOnLoseFocus ) {
            XtAddCallback( tf_widget, XmNlosingFocusCallback,
             xtdoTextFieldToIntLF, this );
	  }

          break;

        case DBR_FLOAT:
        case DBR_DOUBLE:

          XtAddCallback( tf_widget, XmNactivateCallback,
           xtdoTextFieldToDoubleA, this );

          if ( changeValOnLoseFocus ) {
            XtAddCallback( tf_widget, XmNlosingFocusCallback,
             xtdoTextFieldToDoubleLF, this );
	  }

          break;

        } // end switch

      }

      } // end if ( !tf_widget )

    } // end if ( isWidget )

    fgColor.setConnected();
    bufInvalidate();
    init = 1;
    eraseActive();
    drawActive();

  }

  if ( nr ) {

    bufInvalidate();
    eraseActive();
    if (smartRefresh) {
      smartDrawAllActive();
    }
    else {
      drawActive();
    }

  }

  if ( nu ) {

    eraseActive();
    if (smartRefresh) {
      smartDrawAllActive();
    }
    else {
      drawActive();
    }

    if ( changeCallback ) {
      (*changeCallback)( this );
    }

  }

  if ( ne ) {

    eraseActive();

  }

  if ( nd ) {

    drawActive();

  }

  actWin->appCtx->proc->lock();
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

}

int activeXTextDspClass::getProperty (
  char *prop,
  int bufSize,
  char *_value )
{

int l;
char *buf;

  if ( strcmp( prop, activeXTextDspClass_str65 ) == 0 ) {

    if ( !tf_widget ) {

      l = strlen(curValue);
      if ( l > bufSize ) l = bufSize;

      strncpy( _value, curValue, l );
      _value[l] = 0;

    }
    else {

      buf = XmTextGetString( tf_widget );

      l = strlen(buf);
      if ( l > bufSize ) l = bufSize;

      strncpy( _value, buf, l );
      _value[l] = 0;

      XtFree( buf );

    }

    return 1;

  }
  else if ( strcmp( prop, activeXTextDspClass_str66 ) == 0 ) {

    if ( !tf_widget ) {
      strncpy( _value, "", bufSize );
      _value[bufSize] = 0;
      return 0;
    }

    buf = XmTextGetString( tf_widget );

    l = strlen(buf);
    if ( l > bufSize ) l = bufSize;

    strncpy( _value, buf, l );
    _value[l] = 0;

    XtFree( buf );

    return 1;

  }

  return 0;

}

char *activeXTextDspClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeXTextDspClass::nextDragName ( void ) {

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeXTextDspClass::dragValue (
  int i ) {

  if ( !i ) {
    return pvExpStr.getExpanded();
  }
  else {
    return svalPvExpStr.getExpanded();
  }

}

void activeXTextDspClass::changeDisplayParams (
  unsigned int _flag,
  char *_fontTag,
  int _alignment,
  char *_ctlFontTag,
  int _ctlAlignment,
  char *_btnFontTag,
  int _btnAlignment,
  unsigned int _textFgColor,
  unsigned int _fg1Color,
  unsigned int _fg2Color,
  unsigned int _offsetColor,
  unsigned int _bgColor,
  unsigned int _topShadowColor,
  unsigned int _botShadowColor )
{

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColor.setColor( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_FG2COLOR_MASK )
    fgColor.setNullColor( _fg2Color );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor = _bgColor;

  if ( _flag & ACTGRF_ALIGNMENT_MASK )
    alignment = _alignment;

  if ( _flag & ACTGRF_FONTTAG_MASK ) {

    strcpy( fontTag, _fontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );

    if ( fs ) {
      fontAscent = fs->ascent;
      fontDescent = fs->descent;
      fontHeight = fontAscent + fontDescent;
    }
    else {
      fontAscent = 0;
      fontDescent = 0;
      fontHeight = 0;
    }

    updateDimensions();

  }

}

void activeXTextDspClass::changePvNames (
  int flag,
  int numCtlPvs,
  char *ctlPvs[],
  int numReadbackPvs,
  char *readbackPvs[],
  int numNullPvs,
  char *nullPvs[],
  int numVisPvs,
  char *visPvs[],
  int numAlarmPvs,
  char *alarmPvs[] )
{

int changed = 0;

  if ( editable ) {

    if ( flag & ACTGRF_CTLPVS_MASK ) {

      if ( numCtlPvs ) {

        changed = 1;

        strncpy( value, ctlPvs[0], 39 );
        value[39] = 0;
        strncpy( curValue, ctlPvs[0], 39 );
        curValue[39] = 0;

        strncpy( pvName, ctlPvs[0], 39 );
        pvName[39] = 0;
        pvExpStr.setRaw( pvName );

      }

    }

  }
  else {

    if ( flag & ACTGRF_READBACKPVS_MASK ) {

      if ( numReadbackPvs ) {

        changed = 1;

        strncpy( value, readbackPvs[0], 39 );
        value[39] = 0;
        strncpy( curValue, readbackPvs[0], 39 );
        curValue[39] = 0;

        strncpy( pvName, readbackPvs[0], 39 );
        pvName[39] = 0;
        pvExpStr.setRaw( pvName );

      }

    }

  }

  if ( changed ) {

    stringLength = strlen( curValue );

    updateFont( curValue, fontTag, &fs,
     &fontAscent, &fontDescent, &fontHeight,
     &stringWidth );

    stringY = y + fontAscent;

    if ( alignment == XmALIGNMENT_BEGINNING )
      stringX = x;
    else if ( alignment == XmALIGNMENT_CENTER )
      stringX = x + w/2 - stringWidth/2;
    else if ( alignment == XmALIGNMENT_END )
      stringX = x + w - stringWidth;

    updateDimensions();

  }

  if ( flag & ACTGRF_NULLPVS_MASK ) {
    if ( numNullPvs ) {
      svalPvExpStr.setRaw( nullPvs[0] );
    }
  }

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeXTextDspClassPtr ( void ) {

activeXTextDspClass *ptr;

  ptr = new activeXTextDspClass;
  return (void *) ptr;

}

void *clone_activeXTextDspClassPtr (
  void *_srcPtr )
{

activeXTextDspClass *ptr, *srcPtr;

  srcPtr = (activeXTextDspClass *) _srcPtr;

  ptr = new activeXTextDspClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
