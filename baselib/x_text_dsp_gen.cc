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

#define __x_text_dsp_gen_cc 1

#include "x_text_dsp_gen.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

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
char string[256+1];
char *buf;
int maxStringSize;

  buf = XmTextGetString( axtdo->tf_widget );

  axtdo->actWin->appCtx->proc->lock();
  maxStringSize = axtdo->pvId->maxStringSize();
  strncpy( axtdo->entryValue, buf, maxStringSize );
  axtdo->entryValue[maxStringSize] = '\0';
  XtFree( buf );

  strncpy( axtdo->curValue, axtdo->entryValue, maxStringSize );
  axtdo->curValue[maxStringSize] = '\0';
  axtdo->actWin->appCtx->proc->unlock();

  strncpy( string, axtdo->entryValue, maxStringSize );
  string[maxStringSize] = '\0';
  if ( axtdo->pvExists ) {
    stat = axtdo->pvId->put( axtdo->pvId->pvrString( ), &string );
  }
  else {
    axtdo->actWin->appCtx->proc->lock();
    axtdo->needUpdate = 1;
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
char string[256+1];
char *buf;
Arg args[10];
int n;
int maxStringSize;

  if ( !axtdo->widget_value_changed ) return;
 
  buf = XmTextGetString( axtdo->tf_widget );
  l = strlen(buf);

  axtdo->actWin->appCtx->proc->lock();
  maxStringSize = axtdo->pvId->maxStringSize();  
  strncpy( axtdo->entryValue, buf, maxStringSize );
  axtdo->entryValue[maxStringSize] = '\0';
  XtFree( buf );

  strncpy( axtdo->curValue, axtdo->entryValue, maxStringSize );
  axtdo->curValue[maxStringSize] = '\0';
  axtdo->actWin->appCtx->proc->unlock();

  strncpy( string, axtdo->entryValue, maxStringSize );
  string[maxStringSize] = '\0';
  if ( axtdo->pvExists ) {
    stat = axtdo->pvId->put( axtdo->pvId->pvrString(), &string );
  }
  else {
    axtdo->actWin->appCtx->proc->lock();
    axtdo->needUpdate = 1;
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
  strncpy( axtdo->entryValue, buf, 127 );
  axtdo->entryValue[127] = '\0';
  XtFree( buf );

  if ( isLegalInteger(axtdo->entryValue) ) {

    axtdo->actWin->appCtx->proc->lock();
    strncpy( axtdo->curValue, axtdo->entryValue, 127 );
    axtdo->curValue[127] = '\0';
    axtdo->actWin->appCtx->proc->unlock();

    ivalue = atol( axtdo->entryValue );
    if ( axtdo->pvExists ) {
      stat = axtdo->pvId->put( axtdo->pvId->pvrLong(), &ivalue );
    }
    else {
      axtdo->actWin->appCtx->proc->lock();
      axtdo->needUpdate = 1;
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
  strncpy( axtdo->entryValue, buf, 127 );
  axtdo->entryValue[127] = '\0';
  XtFree( buf );

  if ( isLegalInteger(axtdo->entryValue) ) {

    axtdo->actWin->appCtx->proc->lock();
    strncpy( axtdo->curValue, axtdo->entryValue, 127 );
    axtdo->curValue[127] = '\0';
    axtdo->actWin->appCtx->proc->unlock();

    ivalue = atol( axtdo->entryValue );
    if ( axtdo->pvExists ) {
    stat = axtdo->pvId->put( axtdo->pvId->pvrLong(), &ivalue );
    }
    else {
      axtdo->actWin->appCtx->proc->lock();
      axtdo->needUpdate = 1;
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
  strncpy( axtdo->entryValue, buf, 127 );
  axtdo->entryValue[127] = '\0';
  XtFree( buf );

  if ( isLegalFloat(axtdo->entryValue) ) {

    axtdo->actWin->appCtx->proc->lock();
    strncpy( axtdo->curValue, axtdo->entryValue, 127 );
    axtdo->curValue[127] = '\0';
    axtdo->actWin->appCtx->proc->unlock();

    dvalue = atof( axtdo->entryValue );
    if ( axtdo->pvExists ) {
      stat = axtdo->pvId->put( axtdo->pvId->pvrDouble(), &dvalue );
    }
    else {
      axtdo->actWin->appCtx->proc->lock();
      axtdo->needUpdate = 1;
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
  strncpy( axtdo->entryValue, buf, 127 );
  axtdo->entryValue[127] = '\0';
  XtFree( buf );

  if ( isLegalFloat(axtdo->entryValue) ) {

    axtdo->actWin->appCtx->proc->lock();
    strncpy( axtdo->curValue, axtdo->entryValue, 127 );
    axtdo->curValue[127] = '\0';
    axtdo->actWin->appCtx->proc->unlock();

    dvalue = atof( axtdo->entryValue );
    if ( axtdo->pvExists ) {
      stat = axtdo->pvId->put( axtdo->pvId->pvrDouble(), &dvalue );
    }
    else {
      axtdo->actWin->appCtx->proc->lock();
      axtdo->needUpdate = 1;
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      axtdo->actWin->appCtx->proc->unlock();
    }

  }

  n = 0;
  XtSetArg( args[n], XmNcursorPositionVisible, (XtArgVal) False ); n++;
  XtSetValues( axtdo->tf_widget, args, n );

  XmTextSetInsertionPosition( axtdo->tf_widget, l );

}

static void xtdo_monitor_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) clientData;

  axtdo->actWin->appCtx->proc->lock();

  if ( !axtdo->activeMode ) {
    axtdo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    // Gets pvType, so other functions can use it directly.
    axtdo->pvType = axtdo->pvId->getType();

    axtdo->pvNotConnectedMask &= ~( (unsigned char) 1 );
    if ( !axtdo->pvNotConnectedMask ) { // if all are connected
      axtdo->needConnectInit = 1;
    }

  }
  else {

    axtdo->pvNotConnectedMask |= 1; // pv not connected
    axtdo->fgColor.setDisconnected();
    axtdo->needRefresh = 1;

  }

  axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  axtdo->actWin->appCtx->proc->unlock();

}

static void xtdo_monitor_fg_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) clientData;

  axtdo->actWin->appCtx->proc->lock();

  if ( !axtdo->activeMode ) {
    axtdo->actWin->appCtx->proc->unlock();
    return;
  }
 
  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    axtdo->pvNotConnectedMask &= ~( (unsigned char) 4 );
    if ( !axtdo->pvNotConnectedMask ) { // if all are connected
      axtdo->needConnectInit = 1;
    }

  }
  else {

    axtdo->pvNotConnectedMask |= 4; // fg pv not connected
    axtdo->fgColor.setDisconnected();
    axtdo->needRefresh = 1;

  }

  axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextDspUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args ) {

double dvalue;
int ivalue;
short svalue;
int maxStringSize, maxEnumString;

class activeXTextDspClass *axtdo = 
  (activeXTextDspClass *) clientData;

  axtdo->actWin->appCtx->proc->lock();


  if ( !axtdo->activeMode ) {
    axtdo->actWin->appCtx->proc->unlock();
    return;
  }

  maxStringSize = axtdo->pvId->maxStringSize();
  maxEnumString = axtdo->pvId->enumStringSize();

  if ( axtdo->pvType == axtdo->pvId->pvrString() ) {
    strncpy( axtdo->curValue, (char *) axtdo->pvId->getValue( args ), maxStringSize );
    axtdo->curValue[ maxStringSize ] = '\0';
  }

  else if ( axtdo->pvType == axtdo->pvId->pvrFloat() ) {
    dvalue = (double) *( (float *) axtdo->pvId->getValue( args ) );
    sprintf( axtdo->curValue, axtdo->format, dvalue );
  }

  else if ( axtdo->pvType == axtdo->pvId->pvrDouble() ) {
    dvalue = *( (double *) axtdo->pvId->getValue( args ) );
    sprintf( axtdo->curValue, axtdo->format, dvalue );
  }

  // Long must be handled BEFORE short. Otherwise, values are 
  // truncated to short for PV Classes that use a
  // generic integer type.
  else if ( axtdo->pvType == axtdo->pvId->pvrLong() ) {
    ivalue = *( (int *) axtdo->pvId->getValue( args ) );
    sprintf( axtdo->curValue, axtdo->format, ivalue );
  }

  else if ( axtdo->pvType == axtdo->pvId->pvrShort() ) {
    ivalue = (int) *( (short *) axtdo->pvId->getValue( args ) );
    sprintf( axtdo->curValue, axtdo->format, ivalue );
  }

  else if ( axtdo->pvType == axtdo->pvId->pvrEnum() ) {
    svalue = *( (short *) axtdo->pvId->getValue( args ) );
    if ( ( svalue >= 0 ) && ( svalue < axtdo->numStates ) ) {
      strncpy( axtdo->curValue, axtdo->stateString[svalue], maxEnumString );
      axtdo->curValue[ maxEnumString ] = '\0';
      axtdo->entryState = (int) svalue;
    }
    else {
      strcpy( axtdo->curValue, "" );
    }
  }

  else {
    strcpy( axtdo->curValue, "" );
  }

  axtdo->needUpdate = 1;
  axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextDspFgUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args ) {

unsigned int color;
int index;
class activeXTextDspClass *axtdo = 
  (activeXTextDspClass *) clientData;

  axtdo->actWin->appCtx->proc->lock();

  if ( !axtdo->activeMode ) {
    axtdo->actWin->appCtx->proc->unlock();
    return;
  }

  index = *( (int *) axtdo->pvId->getValue( args ) );
  color = axtdo->actWin->ci->getPixelByIndex( index );
  axtdo->fgColor.changeColor( color, axtdo->actWin->ci );
  axtdo->bufInvalidate();
  axtdo->needRefresh = 1;
  axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

class activeXTextDspClass *axtdo = 
  (activeXTextDspClass *) clientData;

double dvalue;
int ivalue;
short svalue;

  axtdo->actWin->appCtx->proc->lock();

  if ( !axtdo->activeMode ) {
    axtdo->actWin->appCtx->proc->unlock();
    return;
  }
  
  // puts("alarm callback\n");

// Generic pvType definitions:
int maxStringSize = axtdo->pvId->maxStringSize();
int enumStringSize = axtdo->pvId->enumStringSize();
int genericStringType = (int) axtdo->pvId->pvrString();
int genericFloatType = (int) axtdo->pvId->pvrFloat();
int genericDoubleType = (int) axtdo->pvId->pvrDouble();
int genericShortType = (int) axtdo->pvId->pvrShort();
int genericLongType = (int) axtdo->pvId->pvrLong();
int genericEnumType = (int)  axtdo->pvId->pvrEnum();

  int status = axtdo->pvId->getStatus( args );
  int severity = axtdo->pvId->getSeverity( args );
  axtdo->fgColor.setStatus( status, severity );

  if ( axtdo->pvType == genericStringType ) {
    strncpy( axtdo->curValue, (char *) axtdo->pvId->getValue( args ), maxStringSize );
    axtdo->curValue[ maxStringSize ] = '\0';
  }
  else if ( axtdo->pvType == genericFloatType ) {
    dvalue = (double) * ( (float *) axtdo->pvId->getValue( args ) );
    sprintf( axtdo->curValue, axtdo->format, dvalue );
  }
  else if ( axtdo->pvType == genericDoubleType ) {
    dvalue = *( (double *) axtdo->pvId->getValue( args ) );
    sprintf( axtdo->curValue, axtdo->format, dvalue );
  }
  else if ( axtdo->pvType == genericLongType ) {
    ivalue = *( (int *) axtdo->pvId->getValue( args ) );
    sprintf( axtdo->curValue, axtdo->format, ivalue );
  }
  else if ( axtdo->pvType == genericShortType ) {
    svalue = *( (short *) axtdo->pvId->getValue( args ) );
    sprintf( axtdo->curValue, axtdo->format, svalue );
  }
  else if ( axtdo->pvType == genericEnumType ) {
    svalue = *( (short *) axtdo->pvId->getValue( args ) );
    if ( ( svalue >= 0 ) && ( svalue < axtdo->numStates ) ) {
      strncpy( axtdo->curValue, axtdo->stateString[svalue], enumStringSize );
      axtdo->curValue[ enumStringSize ] = '\0';
      axtdo->entryState = (int) svalue;
    }
    else {
      strcpy( axtdo->curValue, "" );
    }
  }
  else {
    strcpy( axtdo->curValue, "" );
  }

  axtdo->needRefresh = 1;
  axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  axtdo->actWin->appCtx->proc->unlock();

}

static void XtextDspInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

int i, n;
short svalue;
class activeXTextDspClass *axtdo = (activeXTextDspClass *) clientData;

  axtdo->actWin->appCtx->proc->lock();

  if ( !axtdo->activeMode ) {
    axtdo->actWin->appCtx->proc->unlock();
    return;
  }

// Generic pvType definitions:
// int maxStringSize = axtdo->pvId->maxStringSize();
int enumStringSize = axtdo->pvId->enumStringSize();
int genericStringType = (int) axtdo->pvId->pvrString();
int genericFloatType = (int) axtdo->pvId->pvrFloat();
int genericDoubleType = (int) axtdo->pvId->pvrDouble();
int genericShortType = (int) axtdo->pvId->pvrShort();
int genericLongType = (int) axtdo->pvId->pvrLong();
int genericEnumType = (int)  axtdo->pvId->pvrEnum();

  int status = axtdo->pvId->getStatus( args );
  int severity = axtdo->pvId->getSeverity( args );
  
  if ( axtdo->pvType == genericStringType ) {

    axtdo->fgColor.setStatus( status, severity );
  }
  else if ( axtdo->pvType == genericFloatType ) {

    if ( axtdo->limitsFromDb || axtdo->efPrecision.isNull() ) {
      axtdo->precision = axtdo->pvId->getPrecision( args );
    }

    axtdo->fgColor.setStatus( status, severity );
  }
  else if ( axtdo->pvType == genericDoubleType ) {

    if ( axtdo->limitsFromDb || axtdo->efPrecision.isNull() ) {
      axtdo->precision = axtdo->pvId->getPrecision( args );
    }

    axtdo->fgColor.setStatus( status, severity );
  }
  else if ( axtdo->pvType == genericLongType ) {

    axtdo->fgColor.setStatus( status, severity );
  }
  else if ( axtdo->pvType == genericShortType) {

    axtdo->fgColor.setStatus( status, severity );
  }

  else if ( axtdo->pvType == genericEnumType ) {

    n = axtdo->pvId->getNumStates( args );
    
    for ( i=0; i<n; i++ ) {

      if ( axtdo->stateString[i] == NULL ) {
        // axtdo->stateString[i] = new char[MAX_ENUM_STRING_SIZE+1];
        axtdo->stateString[i] = new char[enumStringSize + 1]; // can we do this?
      }

      strncpy( axtdo->stateString[i], 
        axtdo->pvId->getStateString( args, i ), enumStringSize );
      axtdo->stateString[i][enumStringSize] = '\0';
    }

    axtdo->numStates = n;

    svalue = *( (short *) axtdo->pvId->getValue( args ) );
    if ( ( svalue >= 0 ) && ( svalue < axtdo->numStates ) ) {
      strncpy( axtdo->value, axtdo->stateString[svalue], enumStringSize );
      axtdo->value[ enumStringSize ] = '\0';
      axtdo->entryState = (int) svalue;
    }
    else {
      strcpy( axtdo->value, "" );
    }

    strncpy( axtdo->curValue, axtdo->value, enumStringSize );
    axtdo->curValue[ enumStringSize ] = '\0';

    axtdo->isWidget = 0;

    axtdo->fgColor.setStatus( status, severity );
  }

  axtdo->needInfoInit = 1;
  axtdo->actWin->addDefExeNode( axtdo->aglPtr );

  axtdo->actWin->appCtx->proc->unlock();

}

static void axtdc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextDspClass *axtdo = (activeXTextDspClass *) client;
double dvalue;
int ivalue, stat;
short svalue;
char string[256+1];

  axtdo->actWin->appCtx->proc->lock();

  if ( !axtdo->activeMode ) {
    axtdo->actWin->appCtx->proc->unlock();
    return;
  }

// Generic pvType definitions:
int maxStringSize = axtdo->pvId->maxStringSize();
int genericStringType = (int) axtdo->pvId->pvrString();
int genericFloatType = (int) axtdo->pvId->pvrFloat();
int genericDoubleType = (int) axtdo->pvId->pvrDouble();
int genericShortType = (int) axtdo->pvId->pvrShort();
int genericLongType = (int) axtdo->pvId->pvrLong();
int genericEnumType = (int)  axtdo->pvId->pvrEnum();

  strncpy( axtdo->curValue, axtdo->entryValue, maxStringSize );
  axtdo->curValue[ maxStringSize ] = '\0';

  if ( ( axtdo->pvType == genericFloatType ) ||
  ( axtdo->pvType == genericDoubleType ) ) {
    if ( isLegalFloat(axtdo->entryValue) ) {
      dvalue = atof( axtdo->entryValue );
      if ( axtdo->pvExists ) {
        stat = axtdo->pvId->put( axtdo->pvId->pvrDouble(), &dvalue );
      }
      else {
        axtdo->needUpdate = 1;
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      }

    }
  }
  else if ( ( axtdo->pvType == genericShortType ) ||
    ( axtdo->pvType == genericLongType ) ) {
    if ( isLegalInteger(axtdo->entryValue) ) {
      ivalue = atol( axtdo->entryValue );
      if ( axtdo->pvExists ) {
        stat = axtdo->pvId->put( axtdo->pvId->pvrLong(), &ivalue );
      }
      else {
        axtdo->needUpdate = 1;
        axtdo->actWin->addDefExeNode( axtdo->aglPtr );
      }

    }
  }
  else if ( axtdo->pvType == genericStringType ) {
    strncpy( string, axtdo->entryValue, maxStringSize );
    string[ maxStringSize ] = '\0';
    if ( axtdo->pvExists ) {
      stat = axtdo->pvId->put( axtdo->pvId->pvrString(), &string );
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
    }
  }
  else if ( axtdo->pvType == genericEnumType ) {
    svalue = (short) axtdo->entryState;
    if ( axtdo->pvExists ) {
      stat = axtdo->pvId->put( axtdo->pvId->pvrEnum(), &svalue );
    }
    else {
      axtdo->needUpdate = 1;
      axtdo->actWin->addDefExeNode( axtdo->aglPtr );
    }
  } // End else if -- pvType

  axtdo->actWin->appCtx->proc->unlock();

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

  strncpy( axtdo->value, axtdo->bufPvName, 127 );
  axtdo->value[127] = '\0';
  strncpy( axtdo->curValue, axtdo->bufPvName, 127 );
  axtdo->curValue[127] = '\0';

  strncpy( axtdo->pvName, axtdo->bufPvName, 127 );
  axtdo->pvName[127] = '\0';
  axtdo->pvExpStr.setRaw( axtdo->pvName );

  axtdo->fgPvExpStr.setRaw( axtdo->fgCb.getPv() );

  strncpy( axtdo->fontTag, axtdo->fm.currentFontTag(), 63 );
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

  axtdo->efPrecision = axtdo->bufEfPrecision;

  if ( axtdo->efPrecision.isNull() )
    axtdo->precision = 2;
  else
    axtdo->precision = axtdo->efPrecision.value();

  axtdo->fgColor.setConnectSensitive();

  axtdo->colorMode = axtdo->bufColorMode;

  axtdo->editable = axtdo->bufEditable;

  axtdo->smartRefresh = axtdo->bufSmartRefresh;

  axtdo->isWidget = axtdo->bufIsWidget;

  if ( axtdo->colorMode == XTDC_K_COLORMODE_ALARM )
    axtdo->fgColor.setAlarmSensitive();
  else
    axtdo->fgColor.setAlarmInsensitive();

  axtdo->fgColor.setColor( axtdo->bufFgColor, axtdo->actWin->ci );
  axtdo->bgColor = axtdo->bufBgColor;

  strncpy( axtdo->id, axtdo->bufId, 31 );
  axtdo->changeCallbackFlag = axtdo->bufChangeCallbackFlag;
  axtdo->activateCallbackFlag = axtdo->bufActivateCallbackFlag;
  axtdo->deactivateCallbackFlag = axtdo->bufDeactivateCallbackFlag;
  axtdo->anyCallbackFlag = axtdo->changeCallbackFlag ||
  axtdo->activateCallbackFlag || axtdo->deactivateCallbackFlag;

  strncpy( axtdo->pvUserClassName, axtdo->actWin->pvObj.getPvName(axtdo->pvNameIndex), 15);
  strncpy( axtdo->pvClassName, axtdo->actWin->pvObj.getPvClassName(axtdo->pvNameIndex), 15);

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

  axtdc_edit_apply ( w, client, call );
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

  axtdo->ef.popdown();
  axtdo->operationCancel();
  axtdo->erase();
  axtdo->deleteRequest = 1;
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
  numStates = 0;
  entryState = 0;
  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    stateString[i] = NULL;
  }
  limitsFromDb = 1;
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
  
  strcpy( pvClassName, "" );
  strcpy( pvUserClassName, "" );
  

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

  bgColor = source->bgColor;

  fgColor.copy(source->fgColor);

  strncpy( fontTag, source->fontTag, 63 );
  strncpy( bufFontTag, source->bufFontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgCb = source->fgCb;
  bgCb = source->bgCb;

  strncpy( value, source->value, 127 );

  alignment = source->alignment;

  stringLength = source->stringLength;
  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;
  stringWidth = source->stringWidth;
  stringY = source->stringY;
  stringX = source->stringX;

  strncpy( pvName, source->pvName, 127 );

  pvExpStr.copy( source->pvExpStr );
  fgPvExpStr.copy( source->fgPvExpStr );

  limitsFromDb = source->limitsFromDb;
  precision = source->precision;
  efPrecision = source->efPrecision;

  activeMode = 0;

  strcpy( id, source->id );

  strncpy( pvClassName, source->pvClassName, 15 );
  strncpy( pvUserClassName, source->pvUserClassName, 15 );

  changeCallbackFlag = source->changeCallbackFlag;
  activateCallbackFlag = source->activateCallbackFlag;
  deactivateCallbackFlag = source->deactivateCallbackFlag;
  anyCallbackFlag = changeCallbackFlag ||
   activateCallbackFlag || deactivateCallbackFlag;
  changeCallback = NULL;
  activateCallback = NULL;
  deactivateCallback = NULL;

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
  bgColor = actWin->defaultBgColor;

  useDisplayBg = 1;

  autoHeight = 1;

  formatType = XTDC_K_FORMAT_NATURAL;

  colorMode = XTDC_K_COLORMODE_STATIC;

  editable = 0;
  isWidget = 0;
  
  smartRefresh = 0;

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

  strncpy( pvUserClassName, actWin->defaultPvType, 15 );

  this->editCreate();

  return 1;

}

int activeXTextDspClass::save (
  FILE *f )
{

int r, g, b, stat;

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
  actWin->ci->getRGB( fgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  actWin->ci->getRGB( bgColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
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

  writeStringToFile( f, pvClassName );

  fprintf( f, "%-d\n", smartRefresh );

  // version 1.7.0
  if ( fgPvExpStr.getRaw() )
    writeStringToFile( f, fgPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  return 1;

}

int activeXTextDspClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b;
int major, minor, release;
int stat = 1;
char oneName[127+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  readStringFromFile( pvName, 127, f ); actWin->incLine();
  readStringFromFile( fontTag, 63, f ); actWin->incLine();
  fscanf( f, "%d\n", &useDisplayBg ); actWin->incLine();
  fscanf( f, "%d\n", &alignment ); actWin->incLine();

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

  strncpy( value, pvName, 127 );

  pvExpStr.setRaw( pvName );


  if ( ( major > 1 ) || ( minor > 5 ) ) {

    readStringFromFile( pvClassName, 15, f ); actWin->incLine();

     strncpy( pvUserClassName, actWin->pvObj.getNameFromClass( pvClassName ),
     15 );

  }

  if ( ( major > 1 ) || ( minor > 7 ) ) {
    fscanf( f, "%d\n", &smartRefresh ); actWin->incLine();
  }
  else {
    smartRefresh = 0;
  }

  if ( ( major > 1 ) || ( minor > 6 ) ) {

    readStringFromFile( oneName, 127, f ); actWin->incLine();
    fgPvExpStr.setRaw( oneName );

  }
  else {

    fgPvExpStr.setRaw( "" );

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

  strcpy( fontTag, actWin->defaultFontTag );

  alignment = actWin->defaultAlignment;

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( "import file syntax error" );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( "import file syntax error" );
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
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "ctlpv" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        strncpy( pvName, tk, 28 );

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }
            
      else if ( strcmp( tk, "justify" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        alignment = atol( tk );

      }
            
      else if ( strcmp( tk, "red" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        r = atol( tk );

      }
            
      else if ( strcmp( tk, "green" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        g = atol( tk );

      }
            
      else if ( strcmp( tk, "blue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        b = atol( tk );

      }
            
    }

  } while ( more );

  actWin->ci->setRGB( r, g, b, &bufFgColor );
  fgColor.setColor( bufFgColor, actWin->ci );

  limitsFromDb = 1;
  precision = 3;
  efPrecision.setValue( 3 );

  fgColor.setAlarmInsensitive();

  strncpy( value, pvName, 127 );

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
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  strncat( title, " Properties", 31 );

  strncpy( bufId, id, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;
  bufFgColor = fgColor.pixelColor();
  bufBgColor = bgColor;
  strncpy( bufFontTag, fontTag, 63 );
  bufUseDisplayBg = useDisplayBg;
  bufAutoHeight = autoHeight;
  bufFormatType = formatType;
  bufColorMode = colorMode;
  strncpy( bufValue, value, 127 );
  strncpy( bufPvName, pvName, 127 );
  bufEditable = editable;
  bufSmartRefresh = smartRefresh;
  bufIsWidget = isWidget;
  bufLimitsFromDb = limitsFromDb;
  bufEfPrecision = efPrecision;

  bufChangeCallbackFlag = changeCallbackFlag;
  bufActivateCallbackFlag = activateCallbackFlag;
  bufDeactivateCallbackFlag = deactivateCallbackFlag;

  ef.create( actWin->top, &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "ID", 27, bufId, 31 );
  ef.addTextField( "X", 27, &bufX );
  ef.addTextField( "Y", 27, &bufY );
  ef.addTextField( "Width", 27, &bufW );
  ef.addTextField( "Height", 27, &bufH );
  ef.addToggle( "Auto Height", &bufAutoHeight );
  ef.addFontMenu( "Font", actWin->fi, &fm, fontTag );
  fm.setFontAlignment( alignment );
  ef.addOption( "Color Mode", "Static|Alarm", &bufColorMode );
  ef.addColorButtonWithText( "Fg Color", actWin->ci, &fgCb, &bufFgColor,
   22, fgPvExpStr.getRaw() );
  ef.addColorButton( "Bg Color", actWin->ci, &bgCb, &bufBgColor );
  ef.addToggle( "Use Display Bg", &bufUseDisplayBg );
  ef.addOption( "Display Format",
   "Natural|Float|Exponential|Decimal|Hex|String", &bufFormatType );
  ef.addToggle( "Precision From DB", &bufLimitsFromDb );
  ef.addTextField( "Precision", 27, &bufEfPrecision );
  ef.addTextField( "PV Name", 27, bufPvName, 127 );
  ef.addToggle( "Editable", &bufEditable );
  ef.addToggle( "Smart Refresh", &bufSmartRefresh );
  ef.addToggle( "Motif Widget", &bufIsWidget );
  ef.addToggle( "Activate Callback", &bufActivateCallbackFlag );
  ef.addToggle( "Deactivate Callback", &bufDeactivateCallbackFlag );
  ef.addToggle( "Change Callback", &bufChangeCallbackFlag );

  actWin->pvObj.getOptionMenuList( pvOptionList, 255, &numPvTypes );
  if ( numPvTypes == 1 ) {
    pvNameIndex= 0;
  }
  else {
    // printf( "pvUserClassName = [%s]\n", pvUserClassName );
    pvNameIndex = actWin->pvObj.getNameNum( pvUserClassName );
    if ( pvNameIndex < 0 ) pvNameIndex = 0;
    // printf( "pvOptionList = [%s]\n", pvOptionList );
    // printf( "pvNameIndex = %-d\n", pvNameIndex );
    ef.addOption( "PV Type", pvOptionList, &pvNameIndex );
  }

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
  
  if ( !activeMode || !init ) goto endpoint;

  if ( isWidget ) {

     if ( tf_widget ) {

       strncpy( entryValue, value, 255 );
       n = 0;
       XtSetArg( args[n], XmNvalue, (XtArgVal) entryValue ); n++;

       XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;

       XtSetValues( tf_widget, args, n );

     }

     goto endpoint;

  }
  
  if ( !bufInvalid && ( strlen(value) == strlen(bufValue) ) ) {
    if ( strcmp( value, bufValue ) == 0 ) goto endpoint;
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

  strncpy( bufValue, value, 255 );
  bufInvalid = 0;

  endpoint:

  return 1;

}

void activeXTextDspClass::show ( void ) {

  printf( "base name addr = %-d\n", (int) baseName );
  printf( "name addr = %-d\n", (int) name );
  printf( "x = %-d\n", this->x );
  printf( "y = %-d\n", this->y );
  printf( "w = %-d\n", this->w );
  printf( "h = %-d\n", this->h );
  printf( "fgColor = %-d\n", this->fgColor.pixelColor() );
  printf( "bgColor = %-d\n", this->bgColor );
  printf( "font = %s\n", this->fontTag );
  printf( "PV Name = [%s]\n", this->pvName );

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
  stat = fgPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeXTextDspClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int stat;

  stat = pvExpStr.expand2nd( numMacros, macros, expansions );
  stat = fgPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeXTextDspClass::containsMacros ( void ) {

int result;

  result = pvExpStr.containsPrimaryMacros();
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
    pvConnected = 0;
    numStates = 0; // for enum type
    editDialogIsActive = 0;
    init = 0;
    pvNotConnectedMask = 0;

    actWin->appCtx->proc->lock();
    activeMode = 1;
    actWin->appCtx->proc->unlock();

    break;

  case 2:
  
    if ( !opComplete ) {

      if ( pvExpStr.getExpanded() ) {
        if ( strcmp( pvExpStr.getExpanded(), "" ) != 0 ) {
          pvExists = 1;
          pvNotConnectedMask |= 1;
	}
	else {
          pvExists = 0;
	}
      }
      else {
        pvExists = 0;
      }

      if ( fgPvExpStr.getExpanded() ) {
        if ( strcmp( fgPvExpStr.getExpanded(), "" ) != 0 ) {
          fgPvExists = 1;
          pvNotConnectedMask |= 4;
	}
	else {
          fgPvExists = 0;
	}
      }
      else {
        fgPvExists = 0;
      }
      
      eventId = NULL;
      alarmEventId = NULL;
      fgEventId = NULL;

      if ( pvExists ) {

          // printf( "pvNameIndex = %-d\n", pvNameIndex );
          // printf( "pv class name = [%s]\n", pvClassName );
          // printf( "pvOptionList = [%s]\n", pvOptionList );

          pvId = actWin->pvObj.createNew( pvClassName );
          if ( !pvId ) {
            printf( "Cannot create %s object", pvClassName );
            // actWin->appCtx->postMessage( msg );
            opComplete = 1;
            return 1;
	  }
          pvId->createEventId( &eventId );
	  pvId->createEventId( &alarmEventId );

        stat = pvId->searchAndConnect( &pvExpStr,
         xtdo_monitor_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect %s\n", pvExpStr.getExpanded() );
          return 0;
        }

      }
      
      if ( fgPvExists ) {


        fgPvId = actWin->pvObj.createNew( pvClassName );
        if ( !fgPvId ) {
          printf( "Cannot create %s object", pvClassName );
          // actWin->appCtx->postMessage( msg );
          opComplete = 1;
          return 1;
        }
        fgPvId->createEventId( &fgEventId );

        stat = fgPvId->searchAndConnect( &fgPvExpStr,
         xtdo_monitor_fg_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect %s\n", fgPvExpStr.getExpanded() );
          return 0;
        }

      }
      
      else if ( anyCallbackFlag ) {

        actWin->appCtx->proc->lock();
        needInfoInit = 1;
        actWin->addDefExeNode( aglPtr );
        actWin->appCtx->proc->unlock();

      }

      if ( anyCallbackFlag ) {

        if ( changeCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          strncat( callbackName, "Change", 63 );
          changeCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          strncat( callbackName, "Activate", 63 );
          activateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( deactivateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          strncat( callbackName, "Deactivate", 63 );
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

    if ( deactivateCallback ) {
      (*deactivateCallback)( this );
    }

    actWin->appCtx->proc->lock();

    activeMode = 0;

    if ( pvExists ) {

      stat = pvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = pvId->destroyEventId( &eventId );
      stat = pvId->destroyEventId( &alarmEventId );

      delete pvId;
      
      pvId = NULL;

    }

    if ( fgPvExists ) {

      stat = fgPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = fgPvId->destroyEventId( &fgEventId );

      delete fgPvId;

      pvId = NULL;

    }

    actWin->appCtx->proc->unlock();

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

void activeXTextDspClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

char selectString[256+1];
int i;
int genericEnumType = pvId->pvrEnum();
int enumStringSize = pvId->enumStringSize();
int maxStringSize = pvId->maxStringSize();

  if ( editDialogIsActive ) return;

  strncpy( entryValue, value, enumStringSize );
  teX = this->x + actWin->x;
  teY = this->y + actWin->y;
  teW = w;
  teH = h;
  teLargestH = 600;

  textEntry.create( actWin->top, &teX, &teY, &teW, &teH, &teLargestH, "",
  NULL, NULL, NULL );

  if ( pvType != genericEnumType ) {
    textEntry.addTextField( "New Value", 25, entryValue, maxStringSize );
  }
  else {
    strcpy( selectString, "" );
    for ( i=0; i<numStates; i++ ) {
      strncat( selectString, stateString[i], enumStringSize );
      selectString[enumStringSize] = '\0';
      if ( i != numStates-1 ) strncat( selectString, "|", enumStringSize );
    }
    actWin->appCtx->proc->lock();
    textEntry.addOption( "New Value", selectString, &entryState );
    actWin->appCtx->proc->unlock();
  }

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

  *up = 1;
  *drag = 0;

  if ( editable && !isWidget )
    *down = 1;
  else
    *down = 0;

  return 1;

}

void activeXTextDspClass::executeDeferred ( void ) {

int n, stat, numCols, width, csrPos;
int nc, ni, nu, nr, nd, ne;
Arg args[10];
unsigned int bg;
XmFontList textFontList = NULL;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  if ( !activeMode ) {
    actWin->remDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return;
  }

// Generic pvType definitions:
int maxStringSize = pvId->maxStringSize();
int genericStringType = pvId->pvrString();
int genericFloatType = pvId->pvrFloat();
int genericDoubleType = pvId->pvrDouble();
int genericShortType = pvId->pvrShort();
int genericLongType = pvId->pvrLong();
int genericEnumType = pvId->pvrEnum();
  // printf("PVType: %i  Enumerated: %i\n", pvType, genericEnumType);

  if ( !needConnectInit && !needInfoInit && !needRefresh ) {
    deferredCount--;
    if ( deferredCount > 0 ) {
      actWin->appCtx->proc->unlock();
      return;
    }
    deferredCount = actWin->appCtx->proc->halfSecCount;
  }

  nc = needConnectInit; needConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  ne = needErase; needErase = 0;
  strncpy( value, curValue, maxStringSize );
  value[maxStringSize] = '\0';
  // The following line was moved from the bottom of executeDeferred
  actWin->remDefExeNode( aglPtr );

  actWin->appCtx->proc->unlock();

  if ( nc ) {

    if ( pvType == genericStringType ) {
      stat = pvId->getCallback( pvId->pvrGrString(),
       XtextDspInfoUpdate, (void *) this );
      if ( stat != PV_E_SUCCESS ) {
        printf( "getCallback failed\n" );
      }
    }
    else if ( ( pvType == genericFloatType ) ||
      ( pvType == genericDoubleType ) ) {

      stat = pvId->getCallback( pvId->pvrGrDouble(),
       XtextDspInfoUpdate, (void *) this );
      if ( stat != PV_E_SUCCESS ) {
        printf( "getCallback failed\n" );
      }
    }
    else if ( ( pvType == genericShortType ) ||
      ( pvType == genericLongType ) ) {

      stat = pvId->getCallback( pvId->pvrGrLong(),
       XtextDspInfoUpdate, (void *) this );
      if ( stat != PV_E_SUCCESS ) {
        printf( "getCallback failed\n" );
      }
    }

    else if ( pvType == genericEnumType ) {
      stat = pvId->getCallback( pvId->pvrGrEnum(),
       XtextDspInfoUpdate, (void *) this );
      if ( stat != PV_E_SUCCESS ) {
        printf( "getCallback failed\n" );
      }
    }

    bufInvalidate();

  }

  if ( ni ) {

   if ( fgPvExists ) {

     if ( !fgEventId->eventAdded() ) {
       stat = fgPvId->addEvent( fgPvId->pvrLong(), 1,
        XtextDspFgUpdate, (void *) this, fgEventId, fgPvId->pveValue() );
       if ( stat != PV_E_SUCCESS ) {
         printf( "addEvent failed\n" );
       }
     }

   }

   if ( pvExists ) {
   
    // switch ( pvType ) {

    if (pvType == genericStringType) {

      sprintf( format, "%%s" );

      if ( !eventId->eventAdded() ) {
        stat = pvId->addEvent( pvId->pvrString(), 1,
         XtextDspUpdate, (void *) this, eventId, pvId->pveValue() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }
      
      if ( !alarmEventId->eventAdded() ) {
        // puts("Adding alarm event:");
        stat = pvId->addEvent( pvId->pvrStsString(), 1,
         XtextAlarmUpdate, (void *) this, alarmEventId, pvId->pveAlarm() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }
      
    }

    else if (pvType == genericFloatType) {

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
  
      if ( !eventId->eventAdded() ) {
        stat = pvId->addEvent( pvId->pvrFloat(), 1,
         XtextDspUpdate, (void *) this, eventId, pvId->pveValue() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }
      
      if ( !alarmEventId->eventAdded() ) {
        // puts("Adding alarm event:");
        stat = pvId->addEvent( pvId->pvrStsFloat(), 1,
         XtextAlarmUpdate, (void *) this, alarmEventId, pvId->pveAlarm() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }

    }

    else if (pvType == genericDoubleType) {

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

      if ( !eventId->eventAdded() ) {
        stat = pvId->addEvent( pvId->pvrDouble(), 1,
         XtextDspUpdate, (void *) this, eventId, pvId->pveValue() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }
      
      if ( !alarmEventId->eventAdded() ) {
        // puts("Adding alarm event:");
        stat = pvId->addEvent( pvId->pvrStsDouble(), 1,
         XtextAlarmUpdate, (void *) this, alarmEventId, pvId->pveAlarm() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }

    }
    // Long must be handled BEFORE short. Otherwise, values are 
    // truncated to short for PV Classes that use a
    // generic integer type.

    else if (pvType == genericLongType) {

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

      if ( !eventId->eventAdded() ) {
        stat = pvId->addEvent( pvId->pvrLong(), 1,
         XtextDspUpdate, (void *) this, eventId, pvId->pveValue() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }
      
      if ( !alarmEventId->eventAdded() ) {
        // puts("Adding alarm event:");
        stat = pvId->addEvent( pvId->pvrStsLong(), 1,
         XtextAlarmUpdate, (void *) this, alarmEventId, pvId->pveAlarm() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }

    }

    else if (pvType == genericShortType) {

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

      if ( !eventId->eventAdded() ) {
        stat = pvId->addEvent( pvId->pvrShort(), 1,
         XtextDspUpdate, (void *) this, eventId, pvId->pveValue() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }
      
      if ( !alarmEventId->eventAdded() ) {
        // puts("Adding alarm event:");
        stat = pvId->addEvent( pvId->pvrStsShort(), 1,
         XtextAlarmUpdate, (void *) this, alarmEventId, pvId->pveAlarm() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }

    }

    else if (pvType == genericEnumType) {

      sprintf( format, "%%s" );

      if ( !eventId->eventAdded() ) {
        stat = pvId->addEvent( pvId->pvrEnum(), 1,
         XtextDspUpdate, (void *) this, eventId, pvId->pveValue() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }
      
      if ( !alarmEventId->eventAdded() ) {
        // puts("Adding alarm event:");
        stat = pvId->addEvent( pvId->pvrStsEnum(), 1,
         XtextAlarmUpdate, (void *) this, alarmEventId, pvId->pveAlarm() );
        if ( stat != PV_E_SUCCESS ) {
          printf( "addEvent failed\n" );
        }
      }

    }

    else {
      sprintf( format, "%%s" );
    }

   }
   else {

      pvType = genericStringType;
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

      strncpy( entryValue, value, maxStringSize );
      entryValue[maxStringSize] = '\0';
      csrPos = strlen(entryValue);

      widget_value_changed = 0;

      if ( useDisplayBg )
        bg = actWin->executeGc.getBaseBG();
      else
        bg = bgColor;

      if ( !tf_widget ) {

      tf_widget = XtVaCreateManagedWidget( "", xmTextWidgetClass,
       actWin->executeWidget,
       XmNx, x,
       XmNy, y-3,
       XmNforeground, fgColor.getColor(),
       XmNbackground, bg,
       XmNhighlightThickness, 0,
       XmNcolumns, (short) numCols,
       XmNvalue, entryValue,
       XmNmaxLength, (short) 127,
       XmNpendingDelete, True,
       XmNmarginHeight, 0,
       XmNfontList, textFontList,
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

        // switch ( pvType ) {

        if ( pvType == genericStringType ) {

          XtAddCallback( tf_widget, XmNactivateCallback,
           xtdoTextFieldToStringA, this );

          XtAddCallback( tf_widget, XmNlosingFocusCallback,
           xtdoTextFieldToStringLF, this );

        }
        else if ( ( pvType == genericShortType ) ||
          (pvType == genericLongType) ) {

          XtAddCallback( tf_widget, XmNactivateCallback,
           xtdoTextFieldToIntA, this );

          XtAddCallback( tf_widget, XmNlosingFocusCallback,
           xtdoTextFieldToIntLF, this );

        }
        else if ( ( pvType == genericFloatType ) ||
          ( pvType == genericDoubleType ) ) {

          XtAddCallback( tf_widget, XmNactivateCallback,
           xtdoTextFieldToDoubleA, this );

          XtAddCallback( tf_widget, XmNlosingFocusCallback,
           xtdoTextFieldToDoubleLF, this );
	}

      }

     } // end if ( !tf_widget )

    } // end if ( isWidget )

    pvConnected = 1;
    fgColor.setConnected();
    bufInvalidate();
    init = 1;
    eraseActive();
    drawActive();

  }

  if ( nr ) {

    bufInvalidate();
    eraseActive();
    drawActive();

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
  
}

int activeXTextDspClass::getProperty (
  char *prop,
  int bufSize,
  char *_value )
{

int l;
char *buf;

  if ( strcmp( prop, "value" ) == 0 ) {

    if ( !tf_widget ) {

      l = strlen(curValue);
      if ( l > bufSize ) l = bufSize;

      actWin->appCtx->proc->lock();
      strncpy( _value, curValue, l );
      actWin->appCtx->proc->unlock();
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
  else if ( strcmp( prop, "widgetValue" ) == 0 ) {

    if ( !tf_widget ) {
      strncpy( _value, "", bufSize );
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
