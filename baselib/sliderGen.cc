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

#define __sliderGen_cc 1

#include "sliderGen.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void slc_updateControl (
  XtPointer client,
  XtIntervalId *id )
{

activeSliderClass *slo = (activeSliderClass *) client;
int stat, xOfs;
float fv;

  if ( slo->updateControlTimerActive ) {
    slo->updateControlTimer = XtAppAddTimeOut(
     slo->actWin->appCtx->appContext(),
     slo->updateControlTimerValue, slc_updateControl, client );
  }
  else {
    return;
  }

  if ( slo->controlAdjusted ) {
    slo->controlAdjusted = 0;
    return;
  }

  if ( slo->oldControlV == slo->oneControlV ) return;

  slo->oldControlV = slo->oneControlV;

  slo->eraseActiveControlText();
  slo->eraseActivePointers();

  slo->actWin->appCtx->proc->lock();
  slo->controlV = slo->oneControlV = slo->curControlV;
  slo->actWin->appCtx->proc->unlock();

  if ( slo->positive ) {

    if ( slo->controlV < slo->minFv )
      fv = slo->minFv;
    else if ( slo->controlV > slo->maxFv )
      fv = slo->maxFv;
    else
      fv = slo->controlV;

  }
  else {

    if ( slo->controlV > slo->minFv )
      fv = slo->minFv;
    else if ( slo->controlV < slo->maxFv )
      fv = slo->maxFv;
    else
      fv = slo->controlV;

  }

  xOfs = ( slo->w - 4 - slo->controlW ) / 2;

  slo->controlX = (int) ( ( fv - slo->minFv ) /
   slo->factor + 0.5 ) + xOfs;

  slo->savedX = (int) ( ( slo->savedV - slo->minFv ) /
   slo->factor + 0.5 ) + xOfs;

  sprintf( slo->controlValue, slo->controlFormat, slo->controlV );

  stat = slo->drawActiveControlText();
  stat = slo->drawActivePointers();

  if ( slo->changeCallback ) {
    (*slo->changeCallback)( slo );
  }

}

static void slc_decrement (
  XtPointer client,
  XtIntervalId *id )
{

activeSliderClass *slo = (activeSliderClass *) client;
float fvalue;
int stat, xOfs;

  if ( slo->incrementTimerActive ) {
    if ( slo->incrementTimerValue > 50 ) slo->incrementTimerValue -= 10;
    slo->incrementTimer = XtAppAddTimeOut( slo->actWin->appCtx->appContext(),
     slo->incrementTimerValue, slc_decrement, client );
  }

  slo->eraseActiveControlText();
  slo->eraseActivePointers();

  fvalue = slo->controlV - slo->increment;

  if ( slo->positive ) {
    if ( fvalue < slo->minFv ) fvalue = slo->minFv;
    if ( fvalue > slo->maxFv ) fvalue = slo->maxFv;
  }
  else {
    if ( fvalue > slo->minFv ) fvalue = slo->minFv;
    if ( fvalue < slo->maxFv ) fvalue = slo->maxFv;
  }

  slo->controlV = fvalue;

  xOfs = ( slo->w - 4 - slo->controlW ) / 2;

  slo->controlX = (int) ( ( fvalue - slo->minFv ) /
   slo->factor + 0.5 ) + xOfs;

  slo->savedX = (int) ( ( slo->savedV - slo->minFv ) /
   slo->factor + 0.5 ) + xOfs;

  sprintf( slo->controlValue, slo->controlFormat, slo->controlV );
  stat = slo->drawActiveControlText();
  stat = slo->drawActivePointers();

  slo->actWin->appCtx->proc->lock();
  slo->curControlV = slo->controlV;
  slo->actWin->appCtx->proc->unlock();

// for PV support

  if ( slo->controlExists ) {
  stat = slo->controlPvId->put( slo->controlPvId->pvrFloat( ), &fvalue );

  // stat = ca_put( DBR_FLOAT, slo->controlPvId, &fvalue );
  // if ( stat != ECA_NORMAL ) printf( "ca_put failed\n" );
  }
  else if ( slo->anyCallbackFlag ) {
    slo->needCtlRefresh = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();
  }

  slo->controlAdjusted = 1;

  if ( slo->changeCallback ) {
    (*slo->changeCallback)( slo );
  }

}

static void slc_increment  (
  XtPointer client,
  XtIntervalId *id )
{

activeSliderClass *slo = (activeSliderClass *) client;
float fvalue;
int stat, xOfs;

  if ( slo->incrementTimerActive ) {
    if ( slo->incrementTimerValue > 50 ) slo->incrementTimerValue -= 10;
    slo->incrementTimer = XtAppAddTimeOut( slo->actWin->appCtx->appContext(),
     slo->incrementTimerValue, slc_increment, client );
  }

  slo->eraseActiveControlText();
  slo->eraseActivePointers();

  fvalue = slo->controlV + slo->increment;

  if ( slo->positive ) {
    if ( fvalue < slo->minFv ) fvalue = slo->minFv;
    if ( fvalue > slo->maxFv ) fvalue = slo->maxFv;
  }
  else {
    if ( fvalue > slo->minFv ) fvalue = slo->minFv;
    if ( fvalue < slo->maxFv ) fvalue = slo->maxFv;
  }

  slo->controlV = fvalue;

  xOfs = ( slo->w - 4 - slo->controlW ) / 2;

  slo->controlX = (int) ( ( fvalue - slo->minFv ) /
   slo->factor + 0.5 ) + xOfs;

  slo->savedX = (int) ( ( slo->savedV - slo->minFv ) /
   slo->factor + 0.5 ) + xOfs;

  sprintf( slo->controlValue, slo->controlFormat, slo->controlV );
  stat = slo->drawActiveControlText();
  stat = slo->drawActivePointers();

  slo->actWin->appCtx->proc->lock();
  slo->curControlV = slo->controlV;
  slo->actWin->appCtx->proc->unlock();

  if ( slo->controlExists ) {
  stat = slo->controlPvId->put( slo->controlPvId->pvrFloat( ), &fvalue );

  // stat = ca_put( DBR_FLOAT, slo->controlPvId, &fvalue );
  // if ( stat != ECA_NORMAL ) printf( "ca_put failed\n" );
  }
  else if ( slo->anyCallbackFlag ) {
    slo->needCtlRefresh = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();
  }

  slo->controlAdjusted = 1;

  if ( slo->changeCallback ) {
    (*slo->changeCallback)( slo );
  }

}

static void slc_value_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int stat;
float fvalue;
activeSliderClass *slo = (activeSliderClass *) client;

  fvalue = (float) slo->bufControlV;

  if ( slo->positive ) {
    if ( fvalue < slo->minFv ) fvalue = slo->minFv;
    if ( fvalue > slo->maxFv ) fvalue = slo->maxFv;
  }
  else {
    if ( fvalue > slo->minFv ) fvalue = slo->minFv;
    if ( fvalue < slo->maxFv ) fvalue = slo->maxFv;
  }

  slo->controlV = fvalue;

  slo->increment = slo->bufIncrement;
  sprintf( slo->incString, slo->controlFormat, slo->increment );

  slo->actWin->appCtx->proc->lock();
  slo->curControlV = slo->controlV;
  slo->actWin->appCtx->proc->unlock();


  if ( slo->controlExists ) {
    stat = slo->controlPvId->put( slo->controlPvId->pvrFloat( ), &fvalue );


    // stat = ca_put( DBR_FLOAT, slo->controlPvId, &fvalue );
    // if ( stat != ECA_NORMAL ) printf( "ca_put failed\n" );


    slo->needCtlRefresh = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();
  }
  else if ( slo->anyCallbackFlag ) {
    slo->needCtlRefresh = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();
  }

  slo->controlAdjusted = 1;

  if ( slo->changeCallback ) {
    (*slo->changeCallback)( slo );
  }

}

static void slc_value_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSliderClass *slo = (activeSliderClass *) client;

  slc_value_apply ( w, client, call );
  slo->ef.popdown();

}

static void slc_value_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSliderClass *slo = (activeSliderClass *) client;

  slo->ef.popdown();

}

static void slc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSliderClass *slo = (activeSliderClass *) client;

  slo->actWin->setChanged();

  slo->eraseSelectBoxCorners();
  slo->erase();

  slo->fgColor.setColor( slo->bufFgColor, slo->actWin->ci );
  slo->bgColor.setColor( slo->bufBgColor, slo->actWin->ci );
  slo->shadeColor.setColor( slo->bufShadeColor, slo->actWin->ci );
  slo->controlColor.setColor( slo->bufControlColor, slo->actWin->ci );
  slo->readColor.setColor( slo->bufReadColor, slo->actWin->ci );

  slo->fgColorMode = slo->bufFgColorMode;
  if ( slo->fgColorMode == SLC_K_COLORMODE_ALARM )
    slo->fgColor.setAlarmSensitive();
  else
    slo->fgColor.setAlarmInsensitive();

  slo->controlColorMode = slo->bufControlColorMode;
  if ( slo->controlColorMode == SLC_K_COLORMODE_ALARM )
    slo->controlColor.setAlarmSensitive();
  else
    slo->controlColor.setAlarmInsensitive();

  slo->readColorMode = slo->bufReadColorMode;
  if ( slo->readColorMode == SLC_K_COLORMODE_ALARM )
    slo->readColor.setAlarmSensitive();
  else
    slo->readColor.setAlarmInsensitive();

  slo->increment = slo->bufIncrement;
  sprintf( slo->incString, slo->controlFormat, slo->increment );

  slo->controlPvName.setRaw( slo->controlBufPvName );
  slo->readPvName.setRaw( slo->readBufPvName );
  slo->savedValuePvName.setRaw( slo->savedValueBufPvName );

  slo->controlLabelName.setRaw( slo->controlBufLabelName );
  if ( strcmp( slo->controlLabelTypeStr, "PV Label" ) == 0 )
    slo->controlLabelType = SLC_K_LABEL;
  else if ( strcmp( slo->controlLabelTypeStr, "PV Name" ) == 0 )
    slo->controlLabelType = SLC_K_PV_NAME;
  else
    slo->controlLabelType = SLC_K_LITERAL;

  slo->readLabelName.setRaw( slo->readBufLabelName );
  if ( strcmp( slo->readLabelTypeStr, "PV Label" ) == 0 )
    slo->readLabelType = SLC_K_LABEL;
  else if ( strcmp( slo->readLabelTypeStr, "PV Name" ) == 0 )
    slo->readLabelType = SLC_K_PV_NAME;
  else
    slo->readLabelType = SLC_K_LITERAL;

  //  slo->formatType = slo->bufFormatType;

  strncpy( slo->displayFormat, slo->bufDisplayFormat, 2 );

  slo->limitsFromDb = slo->bufLimitsFromDb;
  slo->efPrecision = slo->bufEfPrecision;
  slo->efScaleMin = slo->bufEfScaleMin;
  slo->efScaleMax = slo->bufEfScaleMax;

  slo->minFv = slo->scaleMin = slo->efScaleMin.value();
  slo->maxFv = slo->scaleMax = slo->efScaleMax.value();

  if ( slo->efPrecision.isNull() )
    slo->precision = 2;
  else
    slo->precision = slo->efPrecision.value();

  strncpy( slo->fontTag, slo->fm.currentFontTag(), 63 );
  slo->actWin->fi->loadFontTag( slo->fontTag );
  slo->fs = slo->actWin->fi->getXFontStruct( slo->fontTag );

  strncpy( slo->id, slo->bufId, 31 );
  slo->changeCallbackFlag = slo->bufChangeCallbackFlag;
  slo->activateCallbackFlag = slo->bufActivateCallbackFlag;
  slo->deactivateCallbackFlag = slo->bufDeactivateCallbackFlag;
  slo->anyCallbackFlag = slo->changeCallbackFlag ||
   slo->activateCallbackFlag || slo->deactivateCallbackFlag;

  strncpy( slo->pvUserClassName, 
    slo->actWin->pvObj.getPvName(slo->pvNameIndex), 15);
  strncpy( slo->pvClassName, 
    slo->actWin->pvObj.getPvClassName(slo->pvNameIndex), 15);

  slo->x = slo->bufX;
  slo->sboxX = slo->bufX;

  slo->y = slo->bufY;
  slo->sboxY = slo->bufY;

  if ( slo->bufW < slo->minW ) slo->bufW = slo->minW;

  slo->w = slo->bufW;
  slo->sboxW = slo->bufW;

  if ( slo->bufH < slo->minH ) slo->bufH = slo->minH;

  slo->h = slo->bufH;
  slo->sboxH = slo->bufH;

  slo->updateDimensions();

  // one more iteration
  if ( slo->h < slo->minH ) slo->h = slo->minH;
  slo->sboxH = slo->h;

}

static void slc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSliderClass *slo = (activeSliderClass *) client;

  slc_edit_update( w, client, call );
  slo->refresh( slo );

}

static void slc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSliderClass *slo = (activeSliderClass *) client;

  slc_edit_apply ( w, client, call );
  slo->ef.popdown();
  slo->operationComplete();

}

static void slc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSliderClass *slo = (activeSliderClass *) client;

  slo->ef.popdown();
  slo->operationCancel();

}

static void slc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSliderClass *slo = (activeSliderClass *) client;

  slo->ef.popdown();
  slo->operationCancel();
  slo->erase();
  slo->deleteRequest = 1;
  slo->drawAll();

}

static void sl_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = 
  (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    slo->needCtlConnectInit = 1;

  }
  else {

    slo->controlPvConnected = 0;
    slo->active = 0;
    slo->fgColor.setDisconnected();
    slo->controlColor.setDisconnected();
    slo->readColor.setDisconnected();
    slo->bufInvalidate();
    slo->needErase = 1;
    slo->needDraw = 1;

  }

  slo->actWin->addDefExeNode( slo->aglPtr );

  slo->actWin->appCtx->proc->unlock();

}

static void sl_monitor_read_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = 
  (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    slo->needReadConnectInit = 1;

  }
  else {

    slo->readPvConnected = 0;
    slo->bufInvalidate();
    slo->needDraw = 1;

  }

  slo->actWin->addDefExeNode( slo->aglPtr );

  slo->actWin->appCtx->proc->unlock();

}

static void sl_monitor_saved_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = 
  (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    slo->needSavedConnectInit = 1;

  }
  else {

    slo->savedValuePvConnected = 0;

  }

  slo->actWin->addDefExeNode( slo->aglPtr );

  slo->actWin->appCtx->proc->unlock();

}

static void sl_monitor_control_label_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = 
  (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    slo->needCtlLabelConnectInit = 1;

    slo->actWin->addDefExeNode( slo->aglPtr );

  }

  slo->actWin->appCtx->proc->unlock();

}

static void sl_controlLabelUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = 
  (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  strncpy( slo->controlLabel, 
    (char *) slo->controlLabelPvId->getValue( args ), 39 );

  slo->needCtlLabelInfoInit = 1;

  slo->actWin->addDefExeNode( slo->aglPtr );

  slo->actWin->appCtx->proc->unlock();

}

static void sl_monitor_read_label_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = 
  (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    slo->needReadLabelConnectInit = 1;

    slo->actWin->addDefExeNode( slo->aglPtr );

  }

  slo->actWin->appCtx->proc->unlock();

}

static void sl_readLabelUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = 
  (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  strncpy( slo->readLabel, 
    (char *) slo->readLabelPvId->getValue( args ), 39 );

  slo->needReadLabelInfoInit = 1;

  slo->actWin->addDefExeNode( slo->aglPtr );

  slo->actWin->appCtx->proc->unlock();

}

static void sl_readInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = (activeSliderClass *) clientData;
int prec;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( slo->limitsFromDb || slo->efPrecision.isNull() )
    prec = slo->readPvId->getPrecision( args );
  else
    prec = slo->precision;

  sprintf( slo->readFormat, "%%.%-d%s", prec, slo->displayFormat );

  slo->curReadV = *( (float *) slo->readPvId->getValue( args ) );

  slo->needReadInfoInit = 1;

  slo->actWin->addDefExeNode( slo->aglPtr );

  slo->actWin->appCtx->proc->unlock();

}

static void sl_infoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( slo->limitsFromDb || slo->efScaleMin.isNull() ) {
    slo->scaleMin = slo->controlPvId->getLoOpr( args );
  }

  if ( slo->limitsFromDb || slo->efScaleMax.isNull() ) {
    slo->scaleMax = slo->controlPvId->getHiOpr( args );
  }

  if ( slo->limitsFromDb || slo->efPrecision.isNull() ) {
    slo->precision = slo->controlPvId->getPrecision( args );
  }

  sprintf( slo->controlFormat, "%%.%-d%s", slo->precision,
   slo->displayFormat );

  slo->minFv = slo->scaleMin;

  slo->maxFv = slo->scaleMax;

  slo->curControlV = *( (float *) slo->controlPvId->getValue( args ) );

  slo->needCtlInfoInit = 1;

  slo->actWin->addDefExeNode( slo->aglPtr );

  slo->actWin->appCtx->proc->unlock();

}

static void sl_controlUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = 
  (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  slo->oneControlV = *( (float *) slo->controlPvId->getValue( args ) );
  // an xtimer updates image
  slo->curControlV = slo->oneControlV;

  slo->actWin->appCtx->proc->unlock();

}

static void sl_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = 
  (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  slo->curReadV = *( (float *) slo->readPvId->getValue( args ) );

  slo->needReadRefresh = 1;

  slo->actWin->addDefExeNode( slo->aglPtr );

  slo->actWin->appCtx->proc->unlock();

}

static void sl_savedValueUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeSliderClass *slo = 
  (activeSliderClass *) clientData;

  slo->actWin->appCtx->proc->lock();

  if ( !slo->activeMode ) {
    slo->actWin->appCtx->proc->unlock();
    return;
  }

  slo->newSavedV = *( (float *) slo->savedValuePvId->getValue( args ) );
  
  slo->needSavedRefresh = 1;

  slo->actWin->addDefExeNode( slo->aglPtr );

  slo->actWin->appCtx->proc->unlock();

}

activeSliderClass::activeSliderClass ( void ) {

  name = new char[strlen("activeSliderClass")+1];
  strcpy( name, "activeSliderClass" );
  deleteRequest = 0;
  selected = 0;
  positive = 1;
  strcpy( id, "" );

  changeCallbackFlag = 0;
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;
  anyCallbackFlag = 0;
  changeCallback = NULL;
  activateCallback = NULL;
  deactivateCallback = NULL;

  scaleMin = 0;
  scaleMax = 10;

  limitsFromDb = 1;
  efScaleMin.setNull(1);
  efScaleMax.setNull(1);
  efPrecision.setNull(1);
  strcpy( displayFormat, "g" );
  precision = 3;

  strcpy( pvClassName, "" );
  strcpy( pvUserClassName, "" );
 
}

// copy constructor
activeSliderClass::activeSliderClass
 ( const activeSliderClass *source ) {

int xOfs;
activeGraphicClass *slo = (activeGraphicClass *) this;

  slo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeSliderClass")+1];
  strcpy( name, "activeSliderClass" );

  deleteRequest = 0;

  bgColor.copy( source->bgColor );
  fgColor.copy( source->fgColor );
  shadeColor.copy( source->shadeColor );
  controlColor.copy( source->controlColor );
  readColor.copy( source->readColor );

  fgColorMode = source->fgColorMode;
  controlColorMode = source->controlColorMode;
  readColorMode = source->readColorMode;

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  shadeCb = source->shadeCb;
  controlCb = source->controlCb;
  readCb = source->readCb;

  controlPvName.copy( source->controlPvName );
  readPvName.copy( source->readPvName );
  savedValuePvName.copy( source->savedValuePvName );
  controlLabelName.copy( source->controlLabelName );
  readLabelName.copy( source->readLabelName );

  // formatType = source->formatType;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  xOfs = ( w - controlW ) / 2;
  controlX = xOfs;
  readX = xOfs;
  arcStart = 90*64 - 30*64;
  arcStop = 60*64;

  strcpy( controlValue, "0.0" );
  strcpy( readValue, "0.0" );
  strcpy( controlLabel, "" );
  strcpy( readLabel, "" );

  controlLabelType = source->controlLabelType;
  strncpy( controlLabelTypeStr, source->controlLabelTypeStr, 15 );

  readLabelType = source->readLabelType;
  strncpy( readLabelTypeStr, source->readLabelTypeStr, 15 );

  increment = source->increment;
  savedV = source->savedV;

  positive = source->positive;

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

  limitsFromDb = source->limitsFromDb;
  scaleMin = source->scaleMin;
  scaleMax = source->scaleMax;
  precision = source->precision;

  efScaleMin = source->efScaleMin;
  efScaleMax = source->efScaleMax;
  efPrecision = source->efPrecision;
  strncpy( displayFormat, source->displayFormat, 2 );

}

int activeSliderClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

int xOfs;

  actWin = aw_obj;
  xOrigin = 0;
  yOrigin = 0;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  increment = 0.0;
  controlAreaW = w - 40;
  controlH = 16;
  controlAreaH = controlH + 1;
  controlAreaW = w - controlAreaH - controlAreaH;
  controlW = controlAreaW - controlAreaH - controlAreaH;
  xOfs = ( w - controlW ) / 2;
  controlX = xOfs;
  controlY = valueAreaH + controlAreaH - 1;
  readX = xOfs;
  readH = controlH / 2;
  readY = controlY; // bottom of triangle
  arcStart = 90*64 - 30*64;
  arcStop = 60*64;

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColor( actWin->defaultBgColor, actWin->ci );
  controlColor.setColor( actWin->defaultFg1Color, actWin->ci );
  readColor.setColor( actWin->defaultFg2Color, actWin->ci );
  shadeColor.setColor( actWin->defaultOffsetColor, actWin->ci );

  fgColorMode = 0;
  controlColorMode = 0;
  readColorMode = 0;

  strcpy( controlValue, "0.0" );
  strcpy( readValue, "0.0" );
  strcpy( controlLabel, "" );
  strcpy( readLabel, "" );

  controlLabelType = SLC_K_PV_NAME;
  strcpy( controlLabelTypeStr, "PV Name" );

  readLabelType = SLC_K_PV_NAME;
  strcpy( readLabelTypeStr, "PV Name" );

  // formatType = SLC_K_FORMAT_FLOAT;

  strcpy( fontTag, actWin->defaultCtlFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();
  if ( h < minH ) h = minH;
  if ( w < minW ) w = minW;

  this->draw();

  strncpy( pvUserClassName, actWin->defaultPvType, 15 );

  this->editCreate();

  return 1;

}

int activeSliderClass::save (
  FILE *f )
{

int r, g, b, stat;

  fprintf( f, "%-d %-d %-d\n", SLC_MAJOR_VERSION, SLC_MINOR_VERSION,
   SLC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getRGB( fgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  actWin->ci->getRGB( bgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  actWin->ci->getRGB( shadeColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  actWin->ci->getRGB( controlColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  actWin->ci->getRGB( readColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%g\n", increment );

  if ( controlPvName.getRaw() )
    writeStringToFile( f, controlPvName.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( readPvName.getRaw() )
    writeStringToFile( f, readPvName.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( savedValuePvName.getRaw() )
    writeStringToFile( f, savedValuePvName.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( controlLabelName.getRaw() )
    writeStringToFile( f, controlLabelName.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", controlLabelType );

  if ( readLabelName.getRaw() )
    writeStringToFile( f, readLabelName.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", readLabelType );

  writeStringToFile( f, fontTag );

  //  fprintf( f, "%-d\n", formatType );

  fprintf( f, "%-d\n", fgColorMode );

  fprintf( f, "%-d\n", controlColorMode );

  fprintf( f, "%-d\n", readColorMode );

  // version 1.3.0
  writeStringToFile( f, id );
  fprintf( f, "%-d\n", changeCallbackFlag );
  fprintf( f, "%-d\n", activateCallbackFlag );
  fprintf( f, "%-d\n", deactivateCallbackFlag );

  // version 1.4.0
  fprintf( f, "%-d\n", limitsFromDb );
  stat = efPrecision.write( f );
  stat = efScaleMin.write( f );
  stat = efScaleMax.write( f );
  writeStringToFile( f, displayFormat );

  // version 1.5.0
  writeStringToFile( f, pvClassName );

  return 1;

}

int activeSliderClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, xOfs, stat;
int major, minor, release;
unsigned int pixel;
char oneName[39+1];

  actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 1 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  fgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 1 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  bgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 1 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  shadeColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 1 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  controlColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 1 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  readColor.setColor( pixel, actWin->ci );

  fscanf( f, "%g\n", &increment ); actWin->incLine();

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  controlPvName.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  readPvName.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  savedValuePvName.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  controlLabelName.setRaw( oneName );

  fscanf( f, "%d\n", &controlLabelType ); actWin->incLine();

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  readLabelName.setRaw( oneName );

  fscanf( f, "%d\n", &readLabelType ); actWin->incLine();

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  if ( ( major == 1 ) && ( minor < 4 ) ) {
    fscanf( f, "%d\n", &formatType ); // no longer in use actWin->incLine();
  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    fscanf( f, "%d\n", &controlColorMode ); actWin->incLine();

    fscanf( f, "%d\n", &readColorMode ); actWin->incLine();

  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {
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

  if ( ( major > 1 ) || ( minor > 3 ) ) {

    fscanf( f, "%d\n", &limitsFromDb ); actWin->incLine();

    stat = efPrecision.read( f ); actWin->incLine();

    efScaleMin.read( f ); actWin->incLine();

    efScaleMax.read( f ); actWin->incLine();

    readStringFromFile( oneName, 39, f ); actWin->incLine();
    strncpy( displayFormat, oneName, 1 );

    if ( limitsFromDb || efPrecision.isNull() )
      precision = 2;
    else
      precision = efPrecision.value();

    if ( ( limitsFromDb || efScaleMin.isNull() ) &&
         ( limitsFromDb || efScaleMax.isNull() ) ) {
      minFv = scaleMin = 0;
      maxFv = scaleMax = 10;
    }
    else{
      minFv = scaleMin = efScaleMin.value();
      maxFv = scaleMax = efScaleMax.value();
    }

  }
  else {

    efPrecision.setValue( 2 );
    precision = 2;
    scaleMin = 0;
    scaleMax = 10;

  }

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  xOfs = ( w - controlW ) / 2;
  controlX = xOfs;
  readX = xOfs;
  arcStart = 90*64 - 30*64;
  arcStop = 60*64;

  strcpy( controlValue, "0.0" );
  strcpy( readValue, "0.0" );
  strcpy( controlLabel, "" );
  strcpy( readLabel, "" );

  if ( ( major > 1 ) || ( minor > 4 ) ) {

    readStringFromFile( pvClassName, 15, f ); actWin->incLine();

    strncpy( pvUserClassName, actWin->pvObj.getNameFromClass( pvClassName ),
     15 );

  }

  curControlV = oneControlV = curReadV = controlV = readV = 0.0;

  return 1;

}

int activeSliderClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeSliderClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  Strncat( title, " Properties", 31 );

  strncpy( bufId, id, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;
  bufFgColor = fgColor.pixelColor();
  bufBgColor = bgColor.pixelColor();
  bufShadeColor = shadeColor.pixelColor();
  bufControlColor = controlColor.pixelColor();
  bufReadColor = readColor.pixelColor();
  bufFgColorMode = fgColorMode;
  bufControlColorMode = controlColorMode;
  bufReadColorMode = readColorMode;
  bufIncrement = increment;
  strncpy( bufFontTag, fontTag, 63 );

  bufChangeCallbackFlag = changeCallbackFlag;
  bufActivateCallbackFlag = activateCallbackFlag;
  bufDeactivateCallbackFlag = deactivateCallbackFlag;

  if ( controlPvName.getRaw() )
    strncpy( controlBufPvName, controlPvName.getRaw(), 39 );
  else
    strncpy( controlBufPvName, "", 39 );

  if ( readPvName.getRaw() )
    strncpy( readBufPvName, readPvName.getRaw(), 39 );
  else
    strncpy( readBufPvName, "", 39 );

  if ( savedValuePvName.getRaw() )
    strncpy( savedValueBufPvName, savedValuePvName.getRaw(), 39 );
  else
    strncpy( savedValueBufPvName, "", 39 );

  if ( controlLabelName.getRaw() )
    strncpy( controlBufLabelName, controlLabelName.getRaw(), 39 );
  else
    strncpy( controlBufLabelName, "", 39 );

  if ( readLabelName.getRaw() )
    strncpy( readBufLabelName, readLabelName.getRaw(), 39 );
  else
    strncpy( readBufLabelName, "", 39 );

  if ( controlLabelType == SLC_K_LITERAL )
    strcpy( controlLabelTypeStr, "Literal" );
  else if ( controlLabelType == SLC_K_LABEL )
    strcpy( controlLabelTypeStr, "PV Label" );
  else if ( controlLabelType == SLC_K_PV_NAME )
    strcpy( controlLabelTypeStr, "PV Name" );

  if ( readLabelType == SLC_K_LITERAL )
    strcpy( readLabelTypeStr, "Literal" );
  else if ( readLabelType == SLC_K_LABEL )
    strcpy( readLabelTypeStr, "PV Label" );
  else if ( readLabelType == SLC_K_PV_NAME )
    strcpy( readLabelTypeStr, "PV Name" );

  bufLimitsFromDb = limitsFromDb;
  bufEfPrecision = efPrecision;
  bufEfScaleMin = efScaleMin;
  bufEfScaleMax = efScaleMax;
  strncpy( bufDisplayFormat, displayFormat, 2 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "ID", 27, bufId, 31 );
  ef.addTextField( "X", 27, &bufX );
  ef.addTextField( "Y", 27, &bufY );
  ef.addTextField( "Width", 27, &bufW );
  ef.addTextField( "Height", 27, &bufH );
  ef.addFontMenu( "Label Font", actWin->fi, &fm, fontTag );
  ef.addColorButton( "Fg Color", actWin->ci, &fgCb, &bufFgColor );
  ef.addToggle( "Alarm Sensitive", &bufFgColorMode );
  ef.addColorButton( "Bg Color", actWin->ci, &bgCb, &bufBgColor );
  ef.addColorButton( "Offset Color", actWin->ci, &shadeCb, &bufShadeColor );
  ef.addTextField( "Increment", 27, &bufIncrement );

  ef.addToggle( "Display Info From DB", &bufLimitsFromDb );
  ef.addOption( "Display Format", "g|f|e", bufDisplayFormat, 2 );
  ef.addTextField( "Display Precision", 27, &bufEfPrecision );
  ef.addTextField( "Min Scale Value", 27, &bufEfScaleMin );
  ef.addTextField( "Max Scale Value", 27, &bufEfScaleMax );

  //  ef.addOption( "Display Format", "Float|Exponential", &bufFormatType );

  ef.addTextField( "Control PV", 27, controlBufPvName, 39 );
  ef.addTextField( "Control Label PV", 27, controlBufLabelName, 39 );
  ef.addOption( "Label Type", "Literal|PV Label|PV Name",
   controlLabelTypeStr, 15 );
  ef.addColorButton( "Control Color", actWin->ci, &controlCb,
   &bufControlColor );
  ef.addToggle( "Alarm Sensitive", &bufControlColorMode );
  ef.addTextField( "Readback PV", 27, readBufPvName, 39 );
  ef.addTextField( "Readback Label PV", 27, readBufLabelName, 39 );
  ef.addOption( "Label Type", "Literal|PV Label|PV Name",
   readLabelTypeStr, 15 );
  ef.addColorButton( "Readback Color", actWin->ci, &readCb,
   &bufReadColor );
  ef.addToggle( "Alarm Sensitive", &bufReadColorMode );
  ef.addTextField( "Saved Value PV", 27, savedValueBufPvName, 39 );
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

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeSliderClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( slc_edit_ok, slc_edit_apply, slc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeSliderClass::edit ( void ) {

  this->genericEdit();
  ef.finished( slc_edit_ok, slc_edit_apply, slc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeSliderClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeSliderClass::eraseActive ( void ) {

  if ( !activeMode || !init ) return 1;

  XDrawRectangle( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.eraseGC(), 0, 0, w, h );

  XFillRectangle( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.eraseGC(), 0, 0, w, h );

  return 1;

}

int activeSliderClass::draw ( void ) {

int arcX, arcY, xOfs, lineX;

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( bgColor.pixelColor() );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( shadeColor.pixelColor() );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  xOfs = ( w - controlAreaW ) / 2;
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+xOfs, y+controlY, controlAreaW,
   controlAreaH );

  xOfs = ( w - controlW ) / 2;
  controlX = xOfs;
  readX = xOfs;

  actWin->drawGc.setFG( controlColor.pixelColor() );

  arcY = y + controlY;
  arcX = x + xOfs - controlH;
  XFillArc( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), arcX, arcY, controlH*2,
   controlH*2, arcStart, arcStop );
  XDrawArc( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), arcX, arcY, controlH*2,
   controlH*2, arcStart, arcStop );

  actWin->drawGc.setFG( readColor.pixelColor() );

  arcY = y + readY + controlH/2;
  arcX = x + readX - controlH/2;
  XFillArc( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), arcX, arcY, readH*2,
   readH*2, arcStart, arcStop );
  XDrawArc( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), arcX, arcY, readH*2,
   readH*2, arcStart, arcStop );

  actWin->drawGc.setFG( fgColor.pixelColor() );

  lineX = x + ( w - controlW ) / 2;

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), lineX, y+valueAreaH+controlAreaH+labelAreaH/2-4,
   lineX, y+valueAreaH+controlAreaH+4 );

  lineX = x + ( w - controlW ) / 2 + controlW / 2;

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), lineX, y+valueAreaH+controlAreaH+labelAreaH/2-4,
   lineX, y+valueAreaH+controlAreaH+4 );

  lineX = x + ( w - controlW ) / 2 + controlW;

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), lineX, y+valueAreaH+controlAreaH+labelAreaH/2-4,
   lineX, y+valueAreaH+controlAreaH+4 );

  if ( fs ) {

    actWin->drawGc.setFG( fgColor.pixelColor() );

    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    drawText( actWin->drawWidget, &actWin->drawGc, fs, x+xOfs,
     y+valueAreaH+controlAreaH+labelAreaH/2-2,
     XmALIGNMENT_BEGINNING, "0.0" );

    drawText( actWin->drawWidget, &actWin->drawGc, fs, x+w-xOfs,
     y+valueAreaH+controlAreaH+labelAreaH/2-2,
     XmALIGNMENT_END, "0.0" );

    drawText( actWin->drawWidget, &actWin->drawGc, fs, x+w-2, y+2,
     XmALIGNMENT_END, "0.0" );

    drawText( actWin->drawWidget, &actWin->drawGc, fs, x+w-2, y+4+fontHeight,
     XmALIGNMENT_END, "0.0" );

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeSliderClass::eraseActivePointers ( void ) {

int arcX, arcY, xOfs;
 
int adjW = w - 4;

  if ( !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( shadeColor.getColor() );

  xOfs = ( adjW - controlW ) / 2;

  arcY = controlY;
  arcX = controlX - controlH;
  XFillArc( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), arcX, arcY, controlH*2,
   controlH*2, arcStart, arcStop );
  XDrawArc( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), arcX, arcY, controlH*2,
   controlH*2, arcStart, arcStop );

  arcY = readY + controlH/2;
  arcX = readX - controlH/2;
  XFillArc( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), arcX, arcY, readH*2,
   readH*2, arcStart, arcStop );
  XDrawArc( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), arcX, arcY, readH*2,
   readH*2, arcStart, arcStop );

  XDrawLine( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), savedX, controlY, savedX,
   controlY + controlH );

  actWin->executeGc.restoreFg();

  return 1;

}

int activeSliderClass::drawActivePointers ( void ) {

int arcX, arcY, xOfs;

int adjW = w - 4;

  if ( !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( controlColor.getColor() );

  xOfs = ( adjW - controlW ) / 2;

  if ( controlExists || anyCallbackFlag ) {

    arcY = controlY;
    arcX = controlX - controlH;

    XFillArc( actWin->d, XtWindow(sliderWidget),
     actWin->executeGc.normGC(), arcX, arcY, controlH*2,
     controlH*2, arcStart, arcStop );

    XDrawArc( actWin->d, XtWindow(sliderWidget),
     actWin->executeGc.normGC(), arcX, arcY, controlH*2,
     controlH*2, arcStart, arcStop );

    XDrawLine( actWin->d, XtWindow(sliderWidget),
     actWin->executeGc.normGC(), savedX, controlY, savedX,
     controlY + controlH );

  }

  if ( readExists ) {

    actWin->executeGc.setFG( readColor.getColor() );

    arcY = readY + controlH/2;
    arcX = readX - controlH/2;

    XFillArc( actWin->d, XtWindow(sliderWidget),
     actWin->executeGc.normGC(), arcX, arcY, readH*2,
     readH*2, arcStart, arcStop );

    XDrawArc( actWin->d, XtWindow(sliderWidget),
     actWin->executeGc.normGC(), arcX, arcY, readH*2,
     readH*2, arcStart, arcStop );

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int activeSliderClass::eraseActiveControlText ( void ) {

int tX, tY, xOfs;

int adjW = w - 4;

  if ( !activeMode || !init ) return 1;

  if ( fs && controlExists ) {

    actWin->executeGc.saveFg();

    actWin->executeGc.setFG( bgColor.getColor() );

    xOfs = ( adjW - controlW ) / 2;

    if ( fs ) {

      actWin->executeGc.setFontTag( fontTag, actWin->fi );

      tX = adjW-2;
      tY = 2;
      drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, controlValue );

      tX = w / 2;
      tY = 2;
      drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_CENTER, incString );

    }

    actWin->executeGc.restoreFg();

  }

  return 1;

}

int activeSliderClass::drawActiveControlText ( void ) {

int tX, tY, xOfs;

int adjW = w - 4;

  if ( !activeMode || !init ) return 1;

  if ( fs && controlExists ) {

    actWin->executeGc.saveFg();

    actWin->executeGc.setFG( controlColor.getColor() );

    xOfs = ( adjW - controlW ) / 2;

    if ( fs ) {

      actWin->executeGc.setFontTag( fontTag, actWin->fi );

      tX = adjW-2;
      tY = 2;
      drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, controlValue );

      actWin->executeGc.setFG( controlColor.pixelColor() );

      tX = w / 2;
      tY = 2;
      drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_CENTER, incString );

    }

    actWin->executeGc.restoreFg();

  }

  return 1;

}

int activeSliderClass::eraseActiveReadText ( void ) {

int tX, tY, xOfs;

int adjW = w - 4;

  if ( !activeMode || !init ) return 1;

  if ( fs && readExists ) {

    actWin->executeGc.saveFg();

    actWin->executeGc.setFG( bgColor.getColor() );

    xOfs = ( adjW - controlW ) / 2;

    if ( fs ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
      tX = adjW-2;
      tY = 4+fontHeight;
      drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, readValue );
    }

    actWin->executeGc.restoreFg();

  }

  return 1;

}

int activeSliderClass::drawActiveReadText ( void ) {

int tX, tY, xOfs;

int adjW = w - 4;

  if ( !activeMode || !init ) return 1;

  if ( fs && readExists ) {

    actWin->executeGc.saveFg();

    actWin->executeGc.setFG( readColor.getColor() );

    xOfs = ( adjW - controlW ) / 2;

    if ( fs ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
      tX = adjW-2;
      tY = 4+fontHeight;
      drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, readValue );
    }

    actWin->executeGc.restoreFg();

  }

  return 1;

}

int activeSliderClass::drawActive ( void ) {

int arcX, arcY, tX, tY, xOfs, lineX;

int adjW = w - 4;

  if ( !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( bgColor.getColor() );

  XFillRectangle( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), 0, 0, w, h );

  actWin->executeGc.setFG( shadeColor.getColor() );

  xOfs = ( adjW - controlAreaW ) / 2;
  XFillRectangle( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), xOfs, valueAreaH, controlAreaW,
   controlAreaH );

  actWin->executeGc.setFG( controlColor.getColor() );

  if ( controlExists || anyCallbackFlag ) {

    arcY = controlY;
    arcX = controlX - controlH;

    XFillArc( actWin->d, XtWindow(sliderWidget),
     actWin->executeGc.normGC(), arcX, arcY, controlH*2,
     controlH*2, arcStart, arcStop );

    XDrawArc( actWin->d, XtWindow(sliderWidget),
     actWin->executeGc.normGC(), arcX, arcY, controlH*2,
     controlH*2, arcStart, arcStop );

    XDrawLine( actWin->d, XtWindow(sliderWidget),
     actWin->executeGc.normGC(), savedX, controlY, savedX,
     controlY + controlH );

  }

  if ( readExists ) {

    actWin->executeGc.setFG( readColor.getColor() );

    arcY = readY + controlH/2;
    arcX = readX - controlH/2;

    XFillArc( actWin->d, XtWindow(sliderWidget),
     actWin->executeGc.normGC(), arcX, arcY, readH*2,
     readH*2, arcStart, arcStop );

    XDrawArc( actWin->d, XtWindow(sliderWidget),
     actWin->executeGc.normGC(), arcX, arcY, readH*2,
     readH*2, arcStart, arcStop );

  }

  actWin->executeGc.setFG( fgColor.getColor() );

  lineX = ( adjW - controlW ) / 2;

  XDrawLine( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), lineX, valueAreaH+controlAreaH+labelAreaH/2-4,
   lineX, valueAreaH+controlAreaH+4 );

  lineX = ( adjW - controlW ) / 2 + controlW / 2;

  XDrawLine( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), lineX, valueAreaH+controlAreaH+labelAreaH/2-4,
   lineX, valueAreaH+controlAreaH+4 );

  lineX = ( adjW - controlW ) / 2 + controlW;

  XDrawLine( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), lineX, valueAreaH+controlAreaH+labelAreaH/2-4,
   lineX, valueAreaH+controlAreaH+4 );

  if ( fs ) {

    xOfs = ( adjW - controlW ) / 2;

    if ( controlExists || anyCallbackFlag ) {

      actWin->executeGc.setFG( fgColor.getColor() );

      if ( fs ) {

        actWin->executeGc.setFontTag( fontTag, actWin->fi );

        tX = xOfs;
        tY = valueAreaH+controlAreaH+labelAreaH/2-2;
        drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_BEGINNING, minValue );

        tX = adjW-xOfs;
        tY = valueAreaH+controlAreaH+labelAreaH/2-2;
        drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_END, maxValue );

      }

      actWin->executeGc.setFG( controlColor.getColor() );

      if ( fs ) {

        tX = adjW-2;
        tY = 2;
        drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_END, controlValue );

        actWin->executeGc.setFG( controlColor.pixelColor() );

        tX = w / 2;
        tY = 2;
        drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_CENTER, incString );

      }

      actWin->executeGc.saveBg();
      actWin->executeGc.setFG( bgColor.getColor() );
      actWin->executeGc.setBG( fgColor.getColor() );
      tX = markX1;
      tY = markY0;
      drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, "save" );
      tX = restoreX0;
      tY = restoreY0;
      drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_BEGINNING, " rest" );
      actWin->executeGc.restoreBg();

    }

    if ( controlLabelExists ) {

      actWin->executeGc.setFG( controlColor.getColor() );

      if ( fs ) {
        tX = 2;
        tY = 2;
        drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_BEGINNING, controlLabel );
      }

    }

    if ( readExists ) {

      actWin->executeGc.setFG( readColor.getColor() );

      if ( fs ) {
        tX = adjW-2;
        tY = 4+fontHeight;
        drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_END, readValue );
      }

    }

    if ( readLabelExists ) {

      actWin->executeGc.setFG( readColor.getColor() );

      if ( fs ) {
        tX = 2;
        tY = 4+fontHeight;
        drawText( sliderWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_BEGINNING, readLabel );
      }

    }

  }

  actWin->executeGc.restoreFg();

  return 1;

}

void activeSliderClass::bufInvalidate ( void ) {

  bufInvalid = 1;

}

void sliderEventHandler( 
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

// XExposeEvent *expe;
XMotionEvent *me;
XButtonEvent *be;
activeSliderClass *slo;
int stat, deltaX, xOfs, newX;
float fvalue;
char title[32], *ptr;

  slo = (activeSliderClass *) client;

  if ( !slo->active ) return;

  ptr = slo->actWin->obj.getNameFromClass( "activeSliderClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  Strncat( title, " Parameters", 31 );

  if ( e->type == Expose ) {

    slo->bufInvalidate();
    stat = slo->drawActive();
//      stat = slo->drawActiveControlText();
//      stat = slo->drawActivePointers();

  }
  else if ( e->type == ButtonPress ) {

    be = (XButtonEvent *) e;

    switch ( be->button ) {

//========== B1 Press ========================================

    case Button1:

      if ( ( be->x > slo->markX0 ) &&
         ( be->x < slo->markX1 ) &&
         ( be->y > slo->markY0 ) &&
         ( be->y < slo->markY1 ) ) {

        xOfs = ( slo->w - 4 - slo->controlW ) / 2;
        slo->savedX = (int) ( ( slo->savedV - slo->minFv ) /
         slo->factor + 0.5 ) + xOfs;
        slo->eraseActivePointers();

        slo->savedV = slo->controlV;

        if ( slo->savedValuePvConnected ) {
          stat = slo->savedValuePvId->put( slo->savedValuePvId->pvrFloat( ), 
	    &slo->savedV );
	}
	else {
          xOfs = ( slo->w - 4 - slo->controlW ) / 2;
          slo->savedX = (int) ( ( slo->savedV - slo->minFv ) /
           slo->factor + 0.5 ) + xOfs;
          slo->drawActivePointers();
	}

      }
      else if ( ( be->x > slo->restoreX0 ) &&
         ( be->x < slo->restoreX1 ) &&
         ( be->y > slo->restoreY0 ) &&
         ( be->y < slo->restoreY1 ) ) {

        slo->eraseActiveControlText();
        slo->eraseActivePointers();

        fvalue = slo->savedV;

        slo->controlV = fvalue;

        xOfs = ( slo->w - 4 - slo->controlW ) / 2;

        slo->controlX = (int) ( ( fvalue - slo->minFv ) /
         slo->factor + 0.5 ) + xOfs;

        slo->savedX = (int) ( ( slo->savedV - slo->minFv ) /
         slo->factor + 0.5 ) + xOfs;

        sprintf( slo->controlValue, slo->controlFormat, slo->controlV );
        stat = slo->drawActiveControlText();
        stat = slo->drawActivePointers();

        slo->actWin->appCtx->proc->lock();
        slo->curControlV = slo->controlV;
        slo->actWin->appCtx->proc->unlock();

        if ( slo->controlExists ) {
          stat = slo->controlPvId->put( slo->controlPvId->pvrFloat( ), 
	    &fvalue );
        }
        else if ( slo->anyCallbackFlag ) {
          slo->needCtlRefresh = 1;
          slo->actWin->appCtx->proc->lock();
          slo->actWin->addDefExeNode( slo->aglPtr );
          slo->actWin->appCtx->proc->unlock();
        }

        slo->controlAdjusted = 1;

        if ( slo->changeCallback ) {
          (*slo->changeCallback)( slo );
        }

      }
      else if ( ( be->x > slo->controlX - slo->controlAreaH/2 ) &&
           ( be->x < slo->controlX + slo->controlAreaH/2 ) &&
           ( be->y > slo->valueAreaH ) &&
           ( be->y < slo->valueAreaH + slo->controlAreaH ) ) {

        slo->xRef = be->x;

        slo->controlState = SLC_STATE_MOVING;

      }
      else if ( ( be->x > slo->controlX + slo->controlAreaH/2 ) &&
              ( be->y > slo->valueAreaH ) ) {

        slo->incrementTimerActive = 1;
        slo->incrementTimerValue = 150;

        slo->incrementTimer = XtAppAddTimeOut(
         slo->actWin->appCtx->appContext(), slo->incrementTimerValue,
         slc_increment, (void *) slo );

      }
      else if ( ( be->x < slo->controlX + slo->controlAreaH/2 ) &&
              ( be->y > slo->valueAreaH ) ) {

        slo->incrementTimerActive = 1;
        slo->incrementTimerValue = 150;

        slo->incrementTimer = XtAppAddTimeOut(
         slo->actWin->appCtx->appContext(), slo->incrementTimerValue,
         slc_decrement, (void *) slo );

      }
      else if ( ( be->y < slo->valueAreaH ) && !slo->ef.formIsPoppedUp() ) {

        slo->bufIncrement = slo->increment;
        slo->bufControlV = slo->controlV;
        slo->valueFormX = slo->actWin->x + slo->x  + be->x;
        slo->valueFormY = slo->actWin->y + slo->y + be->y;
        slo->valueFormW = 0;
        slo->valueFormH = 0;
        slo->valueFormMaxH = 600;

        slo->ef.create( slo->actWin->top,
         slo->actWin->appCtx->ci.getColorMap(),
         &slo->valueFormX, &slo->valueFormY,
         &slo->valueFormW, &slo->valueFormH, &slo->valueFormMaxH,
         title, NULL, NULL, NULL );

        slo->ef.addTextField( "Value", 14, &slo->bufControlV );
        slo->ef.addTextField( "Increment", 14, &slo->bufIncrement );
        slo->ef.finished( slc_value_ok, slc_value_apply, slc_value_cancel,
         slo );
        slo->ef.popup();

      }

      break;

//========== B1 Press ========================================

//========== B2 Press ========================================

    case Button2:

      if ( ( be->x > slo->controlX - slo->controlAreaH/2 ) &&
           ( be->x < slo->controlX + slo->controlAreaH/2 ) &&
           ( be->y > slo->valueAreaH ) &&
           ( be->y < slo->valueAreaH + slo->controlAreaH ) ) {

        slo->xRef = be->x;

        slo->controlState = SLC_STATE_MOVING;

      }

      break;

//========== B2 Press ========================================

    }

  }
  if ( e->type == ButtonRelease ) {

//========== Any B Release ========================================

    be = (XButtonEvent *) e;

      slo->controlState = SLC_STATE_IDLE;
      slo->incrementTimerActive = 0;

//========== Any B Release ========================================

  }
  else if ( e->type == MotionNotify ) {

    me = (XMotionEvent *) e;

//========== B1 Motion ========================================

    if ( me->state & Button1Mask ) {

      if ( slo->controlState == SLC_STATE_MOVING ) {

        slo->eraseActiveControlText();
        slo->eraseActivePointers();

        xOfs = ( slo->w - 4 - slo->controlW ) / 2;
//          deltaX = me->x - slo->xRef;
//          slo->xRef = me->x;
//          newX = slo->controlX + deltaX;
        newX = me->x;
        fvalue = slo->factor * (float) ( newX - xOfs ) + slo->minFv;

        if ( slo->positive ) {
          if ( fvalue < slo->minFv ) fvalue = slo->minFv;
          if ( fvalue > slo->maxFv ) fvalue = slo->maxFv;
        }
        else {
          if ( fvalue > slo->minFv ) fvalue = slo->minFv;
          if ( fvalue < slo->maxFv ) fvalue = slo->maxFv;
        }

        slo->controlV = fvalue;

        slo->controlX = (int) ( ( fvalue - slo->minFv ) /
         slo->factor + 0.5 ) + xOfs;

        slo->savedX = (int) ( ( slo->savedV - slo->minFv ) /
         slo->factor + 0.5 ) + xOfs;

        sprintf( slo->controlValue, slo->controlFormat, slo->controlV );
        stat = slo->drawActiveControlText();
        stat = slo->drawActivePointers();

        slo->actWin->appCtx->proc->lock();
        slo->curControlV = slo->controlV;
        slo->actWin->appCtx->proc->unlock();

        if ( slo->controlExists ) {
          stat = slo->controlPvId->put( slo->controlPvId->pvrFloat( ), 
	    &fvalue );
        }
        else if ( slo->anyCallbackFlag ) {
          slo->needCtlRefresh = 1;
          slo->actWin->appCtx->proc->lock();
          slo->actWin->addDefExeNode( slo->aglPtr );
          slo->actWin->appCtx->proc->unlock();
        }

        slo->controlAdjusted = 1;

        if ( slo->changeCallback ) {
          (*slo->changeCallback)( slo );
        }

      }

    }

//========== B1 Motion ========================================

//========== B2 Motion ========================================

    if ( me->state & Button2Mask ) {

      if ( slo->controlState == SLC_STATE_MOVING ) {

        deltaX = abs ( me->x - slo->xRef );
        if ( me->state & ShiftMask ) deltaX *= -1;

        slo->eraseActiveControlText();
        slo->eraseActivePointers();

        slo->xRef = me->x;
        fvalue = slo->controlV + (float) deltaX * slo->increment;

        if ( slo->positive ) {
          if ( fvalue < slo->minFv ) fvalue = slo->minFv;
          if ( fvalue > slo->maxFv ) fvalue = slo->maxFv;
        }
        else {
          if ( fvalue > slo->minFv ) fvalue = slo->minFv;
          if ( fvalue < slo->maxFv ) fvalue = slo->maxFv;
        }

        slo->controlV = fvalue;

        xOfs = ( slo->w - 4 - slo->controlW ) / 2;

        slo->controlX = (int) ( ( fvalue - slo->minFv ) /
         slo->factor + 0.5 ) + xOfs;

        slo->savedX = (int) ( ( slo->savedV - slo->minFv ) /
         slo->factor + 0.5 ) + xOfs;

        sprintf( slo->controlValue, slo->controlFormat, slo->controlV );
        stat = slo->drawActiveControlText();
        stat = slo->drawActivePointers();

        slo->actWin->appCtx->proc->lock();
        slo->curControlV = slo->controlV;
        slo->actWin->appCtx->proc->unlock();

        if ( slo->controlExists ) {
        stat = slo->controlPvId->put( slo->controlPvId->pvrFloat( ), 
	  &fvalue );
        }
        else if ( slo->anyCallbackFlag ) {
          slo->needCtlRefresh = 1;
          slo->actWin->appCtx->proc->lock();
          slo->actWin->addDefExeNode( slo->aglPtr );
          slo->actWin->appCtx->proc->unlock();
        }

        slo->controlAdjusted = 1;

        if ( slo->changeCallback ) {
          (*slo->changeCallback)( slo );
        }

      }

    }

//========== B2 Motion ========================================

  }

}

int activeSliderClass::activate (
  int pass,
  void *ptr )
{

int stat, opStat;
char callbackName[63+1];

  switch ( pass ) {

  case 1:

    frameWidget = XtVaCreateManagedWidget( "", xmFrameWidgetClass,
     actWin->executeWidgetId(),
     XmNx, x,
     XmNy, y,
     XmNmarginWidth, 0,
     XmNmarginHeight, 0,
     XmNtopShadowColor, shadeColor.pixelColor(),
     XmNbottomShadowColor, BlackPixel( actWin->display(),
      DefaultScreen(actWin->display()) ),
     XmNshadowType, XmSHADOW_ETCHED_OUT,
     XmNmappedWhenManaged, False,
     NULL );

    if ( !frameWidget ) {
      printf( "frameWidget create failed\n" );
      return 0;
    }

    sliderWidget = XtVaCreateManagedWidget( "", xmDrawingAreaWidgetClass,
     frameWidget,
     XmNwidth, w-4,
     XmNheight, h-4,
     XmNbackground, bgColor.pixelColor(),
     NULL );

    if ( !sliderWidget ) {
      printf( "activeSliderClass create failed\n" );
      return 0;
    }

    XtAddEventHandler( sliderWidget,
     ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|ExposureMask, False,
     sliderEventHandler, (XtPointer) this );

    XtMapWidget( frameWidget );

    strcpy( readValue, "" );
    strcpy( controlValue, "" );
    strcpy( incString, "" );

    actWin->appCtx->proc->lock();
    activeMode = 1;
    actWin->appCtx->proc->unlock();

    init = 0;
    active = 0;
    aglPtr = ptr;
    curControlV = oneControlV = curReadV = 0.0;
    controlV = readV = 0.0;
    needCtlConnectInit = needCtlInfoInit = needCtlRefresh =
     needReadConnectInit = needReadInfoInit = needReadRefresh =
     needCtlLabelConnectInit = needCtlLabelInfoInit =
     needReadLabelConnectInit = needReadLabelInfoInit =
     needSavedConnectInit = needSavedRefresh = needErase = needDraw = 0;
    oldControlV = 0;
    updateControlTimerActive = 0;
    controlAdjusted = 0;
    incrementTimerActive = 0;
    opComplete = 0;

    controlEventId = NULL;
    readEventId = NULL;
    controlLabelEventId = NULL;
    readLabelEventId = NULL;
    savedEventId = NULL;

    controlState = SLC_STATE_IDLE;

    controlPvConnected = readPvConnected = savedValuePvConnected = 0;

    fgColor.setConnectSensitive();

    if ( !controlPvName.getExpanded() ||
       ( strcmp( controlPvName.getExpanded(), "" ) == 0 ) ) {
      controlExists = 0;
    }
    else {
      controlExists = 1;
      controlColor.setConnectSensitive();
    }


    if ( !readPvName.getExpanded() ||
       ( strcmp( readPvName.getExpanded(), "" ) == 0 ) ) {
      readExists = 0;
    }
    else {
      readExists = 1;
      readColor.setConnectSensitive();
    }


    if ( !savedValuePvName.getExpanded() ||
       ( strcmp( savedValuePvName.getExpanded(), "" ) == 0 ) ) {
      savedValueExists = 0;
    }
    else {
      savedValueExists = 1;
    }


    if ( !controlLabelName.getExpanded() ||
       ( strcmp( controlLabelName.getExpanded(), "" ) == 0 ) ) {
      controlLabelExists = 0;
    }
    else {
      controlLabelExists = 1;
    }

    if ( controlLabelType == SLC_K_PV_NAME ) {
      controlLabelExists = 1;
      strncpy( controlLabel, controlPvName.getExpanded(), 39 );
    }
    else {
      strncpy( controlLabel, controlLabelName.getExpanded(), 39 );
    }


    if ( !readLabelName.getExpanded() ||
       ( strcmp( readLabelName.getExpanded(), "" ) == 0 ) ) {
      readLabelExists = 0;
    }
    else {
      readLabelExists = 1;
    }

    if ( readLabelType == SLC_K_PV_NAME ) {
      readLabelExists = 1;
      strncpy( readLabel, readPvName.getExpanded(), 39 );
    }
    else {
      strncpy( readLabel, readLabelName.getExpanded(), 39 );
    }

    break;

  case 2:

    if ( !opComplete ) {

      opStat = 1;

      if ( controlExists ) {

        controlPvId = actWin->pvObj.createNew( pvClassName );
        if ( !controlPvId ) {
          printf( "Cannot create %s object", pvClassName );
          // actWin->appCtx->postMessage( msg );
          opComplete = 1;
          return 1;
 	}
        // controlPvId->createEventId( &controlEventId );

        stat = controlPvId->searchAndConnect( &controlPvName,
         sl_monitor_control_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          return 0;
        }
      }
      else if ( anyCallbackFlag ) {

        init = 1;
        needCtlInfoInit = 1;
        actWin->appCtx->proc->lock();
        actWin->addDefExeNode( aglPtr );
        actWin->appCtx->proc->unlock();

      }

      if ( anyCallbackFlag ) {

        if ( changeCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, "Change", 63 );
          changeCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, "Activate", 63 );
          activateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( deactivateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, "Deactivate", 63 );
          deactivateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallback ) {
          (*activateCallback)( this );
        }


      }

      if ( controlLabelExists && ( controlLabelType == SLC_K_LABEL )  ) {

        // printf( "pvOptionList = [%s]\n", pvOptionList );

        controlLabelPvId = actWin->pvObj.createNew( pvClassName );
        if ( !controlLabelPvId ) {
          printf( "Cannot create %s object", pvClassName );
          // actWin->appCtx->postMessage( msg );
          opComplete = 1;
          opStat = 0;
 	}
        controlLabelPvId->createEventId( &controlLabelEventId );

        stat = controlLabelPvId->searchAndConnect( &controlLabelName,
         sl_monitor_control_label_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          opStat = 0;
        }
      }

      if ( readExists ) {

        // printf( "pvOptionList = [%s]\n", pvOptionList );

        readPvId = actWin->pvObj.createNew( pvClassName );
        if ( !readPvId ) {
          printf( "Cannot create %s object", pvClassName );
          // actWin->appCtx->postMessage( msg );
          opComplete = 1;
          opStat = 0;
 	}
        // readPvId->createEventId( &readEventId );

        stat = readPvId->searchAndConnect( &readPvName,
         sl_monitor_read_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          opStat = 0;
        }
      }

      if ( readLabelExists && ( readLabelType == SLC_K_LABEL )  ) {

        // printf( "pvOptionList = [%s]\n", pvOptionList );

        readLabelPvId = actWin->pvObj.createNew( pvClassName );
        if ( !readLabelPvId ) {
          printf( "Cannot create %s object", pvClassName );
          // actWin->appCtx->postMessage( msg );
          opComplete = 1;
          opStat = 0;
 	}
        readLabelPvId->createEventId( &readLabelEventId );

        stat = readLabelPvId->searchAndConnect( &readLabelName,
         sl_monitor_read_label_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          opStat = 0;
        }
      }

      if ( savedValueExists ) {

        // printf( "pvOptionList = [%s]\n", pvOptionList );

        savedValuePvId = actWin->pvObj.createNew( pvClassName );
        if ( !savedValuePvId ) {
          printf( "Cannot create %s object", pvClassName );
          // actWin->appCtx->postMessage( msg );
          opComplete = 1;
          opStat = 0;
 	}
        // savedValuePvId->createEventId( &savedEventId );

        stat = savedValuePvId->searchAndConnect( &savedValuePvName,
         sl_monitor_saved_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          opStat = 0;
        }
      }

      if ( opStat & 1 ) {

        updateControlTimerActive = 1;
        updateControlTimerValue = 500;
        updateControlTimer = XtAppAddTimeOut( actWin->appCtx->appContext(),
         updateControlTimerValue, slc_updateControl, (void *) this );

        opComplete = 1;

      }

      return opStat;

    }

    break;

  case 3:
  case 4:

    break;

  case 5:

    opComplete = 0;
    break;

  case 6:

    if ( !opComplete ) {
      opComplete = 1;
    }

    break;

  }

  return 1;

}

int activeSliderClass::deactivate (
  int pass
) {

int stat;

  if ( ef.formIsPoppedUp() ) {
    ef.popdown();
  }

  if ( deactivateCallback ) {
    (*deactivateCallback)( this );
  }

  switch ( pass ) {

  case 1:

    actWin->appCtx->proc->lock();

    activeMode = 0;

    updateControlTimerActive = 0;
    XtRemoveTimeOut( updateControlTimer );

    XtRemoveEventHandler( sliderWidget,
     ButtonPressMask|ButtonReleaseMask|ButtonMotionMask, False,
     sliderEventHandler, (XtPointer) this );

    if ( controlExists ) {

      stat = controlPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = controlPvId->destroyEventId( &controlEventId );

      delete controlPvId;

      controlPvId = NULL;

    }
  
    if ( controlLabelExists && ( controlLabelType == SLC_K_LABEL )  ) {

      stat = controlLabelPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = controlLabelPvId->destroyEventId( &controlLabelEventId );

      delete controlLabelPvId;

      controlLabelPvId = NULL;

    }

    if ( readExists ) {

      stat = readPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = readPvId->destroyEventId( &readEventId );

      delete readPvId;

      readPvId = NULL;

    }

    if ( readLabelExists && ( readLabelType == SLC_K_LABEL )  ) {

      stat = readLabelPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = readLabelPvId->destroyEventId( &readLabelEventId );

      delete readLabelPvId;

      readLabelPvId = NULL;

    }

    if ( savedValueExists ) {

      stat = savedValuePvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = savedValuePvId->destroyEventId( &savedEventId );

      delete savedValuePvId;

      savedValuePvId = NULL;

    }

    actWin->appCtx->proc->unlock();

    break;

  case 2:

    XtUnmapWidget( frameWidget );
    XtDestroyWidget( frameWidget );

    break;

  }

  return 1;

}

int activeSliderClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  tmpw = sboxW;
  tmph = sboxH;

  ret_stat = 1;

  tmpw += _w;
  if ( tmpw < minW ) {
    ret_stat = 0;
  }

  tmph += _h;
  if ( tmph < minH ) {
    ret_stat = 0;
  }

  return ret_stat;

}

int activeSliderClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  ret_stat = 1;

  if ( _w != -1 ) {
    tmpw = _w;
    if ( tmpw < minW ) {
      ret_stat = 0;
    }
  }

  if ( _h != -1 ) {
    tmph = _h;
    if ( tmph < minH ) {
      ret_stat = 0;
    }
  }

  return ret_stat;

}

void activeSliderClass::updateDimensions ( void )
{

// adjust w & h to accomodate frame widget
int adjW = w - 4;
int adjH = h - 4;

  if ( fs ) {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
    markX1 = adjW/2-4;
    markX0 = markX1 - XTextWidth( fs, "M", 1 );
    markY1 = valueAreaH+controlAreaH+labelAreaH-2;
    markY0 = markY1 - fontHeight;
    restoreX0 = adjW/2+4;
    restoreX1 = restoreX0 + XTextWidth( fs, "R", 1 );
    restoreY1 = markY1;
    restoreY0 = markY0;
  }
  else {
    fontAscent = 10;
    fontDescent = 5;
    fontHeight = fontAscent + fontDescent;
    markX1 = -2;
    markX0 = -1;
    markY1 = -2;
    markY0 = -1;
    restoreX1 = -2;
    restoreX0 = -1;
    restoreY1 = -2;
    restoreY0 = -1;
  }

  valueAreaH = (int) ( 2.5 * fontHeight );
  labelAreaH = 2 * fontHeight;

  controlAreaH = adjH - valueAreaH - labelAreaH;
  if ( controlAreaH < 10 ) controlAreaH = 10;
  
  controlAreaW = adjW - 40;
  controlW = controlAreaW - controlAreaH;
  controlY = valueAreaH + 1;
  controlH = controlAreaH - 2;
  readH = controlH / 2;
  readY = controlY; // bottom of triangle

  minW = 200;
  minH = valueAreaH + 14 + labelAreaH;

  // dummy values at this point
  minFv = 0.0;
  maxFv = 10.0;
  positive = 1;

  factor = ( maxFv - minFv ) / controlW;
  if ( factor == 0.0 ) factor = 1.0;

  if ( fs ) {
    markX1 = adjW/2-4;
    markX0 = markX1 - XTextWidth( fs, "save", 4 );
    markY1 = valueAreaH+controlAreaH+labelAreaH-2;
    markY0 = markY1 - fontHeight;
    restoreX0 = adjW/2+4;
    restoreX1 = restoreX0 + XTextWidth( fs, " rest", 4 );
    restoreY1 = markY1;
    restoreY0 = markY0;
  }
  else {
    markX1 = -2;
    markX0 = -1;
    markY1 = -2;
    markY0 = -1;
    restoreX1 = -2;
    restoreX0 = -1;
    restoreY1 = -2;
    restoreY0 = -1;
  }

}

int activeSliderClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

  int retStat, stat;

  retStat = 1;

  stat = controlPvName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = readPvName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = savedValuePvName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = controlLabelName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = readLabelName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeSliderClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

  int retStat, stat;

  retStat = 1;

  stat = controlPvName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = readPvName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = savedValuePvName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = controlLabelName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = readLabelName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeSliderClass::containsMacros ( void ) {

  if ( controlPvName.containsPrimaryMacros() ) return 1;

  if ( readPvName.containsPrimaryMacros() ) return 1;

  if ( savedValuePvName.containsPrimaryMacros() ) return 1;

  if ( controlLabelName.containsPrimaryMacros() ) return 1;

  if ( readLabelName.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeSliderClass::executeDeferred ( void ) {

int stat, xOfs, ncc, nci, ncr, nrc, nri, nrr, nsc, nsr, nclc, ncli, nrlc,
 nrli, ne, nd;
float rv, cv, fv;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  if ( !activeMode ) {
    actWin->remDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return;
  }

//----------------------------------------------------------------------------

  ncc = needCtlConnectInit; needCtlConnectInit = 0;
  nci = needCtlInfoInit; needCtlInfoInit = 0;
  ncr = needCtlRefresh; needCtlRefresh = 0;
  nrc = needReadConnectInit; needReadConnectInit = 0;
  nri = needReadInfoInit; needReadInfoInit = 0;
  nrr = needReadRefresh; needReadRefresh = 0;
  nsc = needSavedConnectInit; needSavedConnectInit = 0;
  nsr = needSavedRefresh; needSavedRefresh = 0;
  nclc = needCtlLabelConnectInit; needCtlLabelConnectInit = 0;
  ncli = needCtlLabelInfoInit; needCtlLabelInfoInit = 0;
  nrlc = needReadLabelConnectInit; needReadLabelConnectInit = 0;
  nrli = needReadLabelInfoInit; needReadLabelInfoInit = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  rv = curReadV;
  cv = curControlV;
  actWin->remDefExeNode( aglPtr );

  actWin->appCtx->proc->unlock();

//----------------------------------------------------------------------------

  if ( ncc ) {

    controlPvConnected = 1;

    stat = controlPvId->getCallback( controlPvId->pvrGrFloat(),
     sl_infoUpdate, (void *) this );
    if ( stat != PV_E_SUCCESS ) {
      printf( "getCallback failed\n" );
    }

  }

//----------------------------------------------------------------------------

  if ( nci ) {

    controlV = cv;
    readV = rv;

    sprintf( minValue, "%-g", minFv );

    sprintf( maxValue, "%-g", maxFv );

    if ( maxFv > minFv )
      positive = 1;
    else
      positive = 0;

    sprintf( controlValue, controlFormat, controlV );
    xOfs = ( w - 4 - controlW ) / 2;

    factor = ( maxFv - minFv ) / controlW;
    if ( factor == 0.0 ) factor = 1.0;

    controlX = (int) ( ( controlV - minFv ) /
     factor + 0.5 ) + xOfs;

    sprintf( incString, controlFormat, increment );

    active = 1;
    init = 1;

    savedV = controlV;

    savedX = (int) ( ( savedV - minFv ) /
     factor + 0.5 ) + xOfs;

    readX = (int) ( ( readV - minFv ) /
     factor + 0.5 ) + xOfs;

    if ( !controlEventId ) {

      controlPvId->createEventId( &controlEventId );

      stat = controlPvId->addEvent( controlPvId->pvrFloat(), 1,
       sl_controlUpdate, (void *) this, controlEventId, 
       controlPvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }

    fgColor.setConnected();
    controlColor.setConnected();
    readColor.setConnected();

    bufInvalidate();

    stat = eraseActive();
    stat = drawActive();

    bufInvalidate();

  }

//----------------------------------------------------------------------------

  if ( ncr ) {

    eraseActiveControlText();
    eraseActivePointers();

    if ( positive ) {

      if ( controlV < minFv )
        fv = minFv;
      else if ( controlV > maxFv )
        fv = maxFv;
      else
        fv = controlV;

    }
    else {

      if ( controlV > minFv )
        fv = minFv;
      else if ( controlV < maxFv )
        fv = maxFv;
      else
        fv = controlV;

    }

    xOfs = ( w - 4 - controlW ) / 2;

    controlX = (int) ( ( fv - minFv ) /
     factor + 0.5 ) + xOfs;

    savedX = (int) ( ( savedV - minFv ) /
     factor + 0.5 ) + xOfs;

    sprintf( controlValue, controlFormat, controlV );

    stat = drawActiveControlText();
    stat = drawActivePointers();

    if ( changeCallback ) {
      (*changeCallback)( this );
    }

  }

//----------------------------------------------------------------------------

  if ( nrc ) {

    readPvConnected = 1;

    stat = readPvId->getCallback( readPvId->pvrGrFloat(),
     sl_readInfoUpdate, (void *) this );
    if ( stat != PV_E_SUCCESS ) {
      printf( "getCallback failed\n" );
    }
  }

//----------------------------------------------------------------------------

  if ( nri ) {

    readV = rv;

    sprintf( readValue, readFormat, readV );

    if ( !readEventId ) {

      readPvId->createEventId( &readEventId );

      stat = readPvId->addEvent( readPvId->pvrFloat(), 1,
       sl_readUpdate, (void *) this, readEventId, readPvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }

  }

//----------------------------------------------------------------------------

  if ( nsc ) {

    if ( !savedEventId ) {

      savedValuePvId->createEventId( &savedEventId );

      stat = savedValuePvId->addEvent( savedValuePvId->pvrFloat(), 1,
       sl_savedValueUpdate, (void *) this, savedEventId, 
       savedValuePvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }

    savedValuePvConnected = 1;

  }

//----------------------------------------------------------------------------

  if ( nsr ) {

    xOfs = ( w - 4 - controlW ) / 2;
    savedX = (int) ( ( savedV - minFv ) /
     factor + 0.5 ) + xOfs;
    eraseActivePointers();

    savedV = newSavedV;

    xOfs = ( w - 4 - controlW ) / 2;
    savedX = (int) ( ( savedV - minFv ) /
     factor + 0.5 ) + xOfs;
    drawActivePointers();

  }

//----------------------------------------------------------------------------

  if ( nrr ) {

    eraseActiveReadText();
    eraseActivePointers();

    readV = rv;
    controlV = cv;

    if ( positive ) {

      if ( readV < minFv )
        fv = minFv;
      else if ( readV > maxFv )
        fv = maxFv;
      else
        fv = readV;

    }
    else {

      if ( readV > minFv )
        fv = minFv;
      else if ( readV < maxFv )
        fv = maxFv;
      else
        fv = readV;

    }

    xOfs = ( w - 4 - controlW ) / 2;

    readX = (int) ( ( fv - minFv ) /
     factor + 0.5 ) + xOfs;

    sprintf( readValue, readFormat, readV );

    stat = drawActiveReadText();
    stat = drawActivePointers();

  }

//----------------------------------------------------------------------------

  if ( nclc ) {


    stat = controlLabelPvId->getCallback( controlLabelPvId->pvrGrString(),
     sl_controlLabelUpdate, (void *) this );
    if ( stat != PV_E_SUCCESS ) {
      printf( "getCallback failed\n" );
    }
  }

//----------------------------------------------------------------------------

  if ( ncli ) {

    if ( active ) {
      stat = eraseActive();
      stat = drawActive();
    }

    bufInvalidate();

  }

//----------------------------------------------------------------------------

  if ( nrlc ) {

    stat = readLabelPvId->getCallback( readLabelPvId->pvrGrString(),
     sl_readLabelUpdate, (void *) this );
    if ( stat != PV_E_SUCCESS ) {
      printf( "getCallback failed\n" );
    }
  }

//----------------------------------------------------------------------------

  if ( nrli ) {

    if ( active ) {
      stat = eraseActive();
      stat = drawActive();
    }

    bufInvalidate();

  }

//----------------------------------------------------------------------------

  if ( ne ) {

    eraseActive();

  }

//----------------------------------------------------------------------------

  if ( nd ) {

    drawActive();

  }

//----------------------------------------------------------------------------

}

int activeSliderClass::getProperty (
  char *prop,
  double *_value )
{

  if ( strcmp( prop, "controlValue" ) == 0 ) {

    *_value = controlV;
    return 1;

  }

  return 0;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeSliderClassPtr ( void ) {

activeSliderClass *ptr;

  ptr = new activeSliderClass;
  return (void *) ptr;

}

void *clone_activeSliderClassPtr (
  void *_srcPtr )
{

activeSliderClass *ptr, *srcPtr;

  srcPtr = (activeSliderClass *) _srcPtr;

  ptr = new activeSliderClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
