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

#define __slider_cc 1

#include "slider.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void sloSetCtlKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSliderClass *slo = (activeSliderClass *) client;
double dvalue;

  dvalue = slo->kpCtlDouble;

  if ( slo->positive ) {
    if ( dvalue < slo->minFv ) dvalue = slo->minFv;
    if ( dvalue > slo->maxFv ) dvalue = slo->maxFv;
  }
  else {
    if ( dvalue > slo->minFv ) dvalue = slo->minFv;
    if ( dvalue < slo->maxFv ) dvalue = slo->maxFv;
  }

  if ( slo->controlExists ) {
    slo->controlPvId->put(
     XDisplayName(slo->actWin->appCtx->displayName), dvalue );
    slo->actWin->appCtx->proc->lock();
    slo->needCtlRefresh = 1;
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();
  }

}

static void sloSetIncKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSliderClass *slo = (activeSliderClass *) client;

  slo->increment = slo->kpIncDouble;
  sprintf( slo->incString, slo->controlFormat, slo->increment );
  slo->actWin->appCtx->proc->lock();
  slo->needErase = 1;
  slo->needDraw = 1;
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

static void sloCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call )
{

}

static void slc_updateControl (
  XtPointer client,
  XtIntervalId *id )
{

activeSliderClass *slo = (activeSliderClass *) client;
int stat, xOfs;
double fv;

  slo->updateControlTimerActive = 0;
  slo->updateControlTimer = 0;

  //if ( slo->updateControlTimerActive ) {
  //  if ( slo->updateControlTimerValue < 100 ) {
  //    slo->updateControlTimerValue = 100;
  //  }
  //  slo->updateControlTimer = appAddTimeOut(
  //   slo->actWin->appCtx->appContext(),
  //   slo->updateControlTimerValue, slc_updateControl, client );
  //}
  //else {
  //  return;
  //}

  if ( slo->controlAdjusted ) {
    slo->controlAdjusted = 0;
    //return;
  }

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
double fvalue;
int stat, xOfs;

  if ( !(slo->incrementTimerActive) ) return;

  if ( slo->incrementTimerValue > 50 ) slo->incrementTimerValue -= 5;
  if ( slo->incrementTimerValue < 45 ) slo->incrementTimerValue = 45;
  slo->incrementTimer = appAddTimeOut( slo->actWin->appCtx->appContext(),
   slo->incrementTimerValue, slc_decrement, client );

  slo->eraseActiveControlText();
  slo->eraseActivePointers();

  if ( slo->incrementTimerValue < 50 ) {
    fvalue = slo->controlV - slo->increment * slo->accelMultiplier;
  }
  else {
    fvalue = slo->controlV - slo->increment;
  }

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
    stat = slo->controlPvId->put(
     XDisplayName(slo->actWin->appCtx->displayName), fvalue );
    if ( !stat ) fprintf( stderr, activeSliderClass_str1 );
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
double fvalue;
int stat, xOfs;

  if ( !(slo->incrementTimerActive) ) return;

  if ( slo->incrementTimerValue > 50 ) slo->incrementTimerValue -= 5;
  if ( slo->incrementTimerValue < 45 ) slo->incrementTimerValue = 45;
  slo->incrementTimer = appAddTimeOut( slo->actWin->appCtx->appContext(),
   slo->incrementTimerValue, slc_increment, client );

  slo->eraseActiveControlText();
  slo->eraseActivePointers();

  if ( slo->incrementTimerValue < 50 ) {
    fvalue = slo->controlV + slo->increment * slo->accelMultiplier;
  }
  else {
    fvalue = slo->controlV + slo->increment;
  }

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
    stat = slo->controlPvId->put(
     XDisplayName(slo->actWin->appCtx->displayName), fvalue );
    if ( !stat ) fprintf( stderr, activeSliderClass_str2 );
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
double fvalue;
activeSliderClass *slo = (activeSliderClass *) client;

  fvalue = (double) slo->eBuf->bufControlV;

  if ( slo->positive ) {
    if ( fvalue < slo->minFv ) fvalue = slo->minFv;
    if ( fvalue > slo->maxFv ) fvalue = slo->maxFv;
  }
  else {
    if ( fvalue > slo->minFv ) fvalue = slo->minFv;
    if ( fvalue < slo->maxFv ) fvalue = slo->maxFv;
  }

  slo->controlV = fvalue;

  slo->increment = slo->eBuf->bufIncrement;
  sprintf( slo->incString, slo->controlFormat, slo->increment );

  slo->accelMultiplier = slo->eBuf->bufAccelMultiplier;

  slo->actWin->appCtx->proc->lock();
  slo->curControlV = slo->controlV;
  slo->actWin->appCtx->proc->unlock();

  if ( slo->controlExists ) {
    stat = slo->controlPvId->put(
     XDisplayName(slo->actWin->appCtx->displayName), fvalue );
    if ( !stat ) fprintf( stderr, activeSliderClass_str3 );
    slo->needErase = 1;
    slo->needDraw = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();
  }
  else if ( slo->anyCallbackFlag ) {
    slo->needErase = 1;
    slo->needDraw = 1;
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

  slo->fgColor.setColorIndex( slo->eBuf->bufFgColor, slo->actWin->ci );
  slo->bgColor.setColorIndex( slo->eBuf->bufBgColor, slo->actWin->ci );
  slo->shadeColor.setColorIndex( slo->eBuf->bufShadeColor, slo->actWin->ci );
  slo->controlColor.setColorIndex( slo->eBuf->bufControlColor, slo->actWin->ci );
  slo->readColor.setColorIndex( slo->eBuf->bufReadColor, slo->actWin->ci );

  slo->bgColorMode = slo->eBuf->bufBgColorMode;
  if ( slo->bgColorMode == SLC_K_COLORMODE_ALARM )
    slo->bgColor.setAlarmSensitive();
  else
    slo->bgColor.setAlarmInsensitive();

  slo->controlColorMode = slo->eBuf->bufControlColorMode;
  if ( slo->controlColorMode == SLC_K_COLORMODE_ALARM )
    slo->controlColor.setAlarmSensitive();
  else
    slo->controlColor.setAlarmInsensitive();

  slo->readColorMode = slo->eBuf->bufReadColorMode;
  if ( slo->readColorMode == SLC_K_COLORMODE_ALARM )
    slo->readColor.setAlarmSensitive();
  else
    slo->readColor.setAlarmInsensitive();

  slo->increment = slo->eBuf->bufIncrement;
  sprintf( slo->incString, slo->controlFormat, slo->increment );

  slo->accelMultiplier = slo->eBuf->bufAccelMultiplier;

  slo->controlPvName.setRaw( slo->eBuf->controlBufPvName );
  slo->readPvName.setRaw( slo->eBuf->readBufPvName );
  slo->savedValuePvName.setRaw( slo->eBuf->savedValueBufPvName );

  slo->controlLabelName.setRaw( slo->eBuf->controlBufLabelName );
  if ( strcmp( slo->controlLabelTypeStr, activeSliderClass_str4 ) == 0 )
    slo->controlLabelType = SLC_K_LABEL;
  else if ( strcmp( slo->controlLabelTypeStr, activeSliderClass_str5 ) == 0 )
    slo->controlLabelType = SLC_K_PV_NAME;
  else
    slo->controlLabelType = SLC_K_LITERAL;

  slo->readLabelName.setRaw( slo->eBuf->readBufLabelName );
  if ( strcmp( slo->readLabelTypeStr, activeSliderClass_str6 ) == 0 )
    slo->readLabelType = SLC_K_LABEL;
  else if ( strcmp( slo->readLabelTypeStr, activeSliderClass_str7 ) == 0 )
    slo->readLabelType = SLC_K_PV_NAME;
  else
    slo->readLabelType = SLC_K_LITERAL;

  //  slo->formatType = slo->eBuf->bufFormatType;

  strncpy( slo->displayFormat, slo->eBuf->bufDisplayFormat, 15 );

  slo->limitsFromDb = slo->eBuf->bufLimitsFromDb;
  slo->efPrecision = slo->eBuf->bufEfPrecision;
  slo->efScaleMin = slo->eBuf->bufEfScaleMin;
  slo->efScaleMax = slo->eBuf->bufEfScaleMax;

  slo->minFv = slo->scaleMin = slo->efScaleMin.value();
  slo->maxFv = slo->scaleMax = slo->efScaleMax.value();

  if ( slo->efPrecision.isNull() )
    slo->precision = 1;
  else
    slo->precision = slo->efPrecision.value();

  strncpy( slo->fontTag, slo->fm.currentFontTag(), 63 );
  slo->actWin->fi->loadFontTag( slo->fontTag );
  slo->fs = slo->actWin->fi->getXFontStruct( slo->fontTag );

  strncpy( slo->id, slo->bufId, 31 );
  slo->changeCallbackFlag = slo->eBuf->bufChangeCallbackFlag;
  slo->activateCallbackFlag = slo->eBuf->bufActivateCallbackFlag;
  slo->deactivateCallbackFlag = slo->eBuf->bufDeactivateCallbackFlag;
  slo->anyCallbackFlag = slo->changeCallbackFlag ||
   slo->activateCallbackFlag || slo->deactivateCallbackFlag;

  slo->x = slo->eBuf->bufX;
  slo->sboxX = slo->eBuf->bufX;

  slo->y = slo->eBuf->bufY;
  slo->sboxY = slo->eBuf->bufY;

  if ( slo->eBuf->bufW < slo->minW ) slo->eBuf->bufW = slo->minW;

  slo->w = slo->eBuf->bufW;
  slo->sboxW = slo->eBuf->bufW;

  if ( slo->eBuf->bufH < slo->minH ) slo->eBuf->bufH = slo->minH;

  slo->h = slo->eBuf->bufH;
  slo->sboxH = slo->eBuf->bufH;

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

  slc_edit_update ( w, client, call );

  slo->refresh( slo );

}

static void slc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSliderClass *slo = (activeSliderClass *) client;

  slc_edit_update ( w, client, call );

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
  ProcessVariable *pv,
  void *userarg )
{

activeSliderClass *slo = (activeSliderClass *) userarg;

  if ( pv->is_valid() ) {

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

  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

static void sl_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeSliderClass *slo = (activeSliderClass *) userarg;

  if ( pv->is_valid() ) {

    slo->needReadConnectInit = 1;

  }
  else {

    slo->readPvConnected = 0;
    slo->bufInvalidate();
    slo->needDraw = 1;

  }

  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

static void sl_monitor_saved_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeSliderClass *slo = (activeSliderClass *) userarg;

  if ( pv->is_valid() ) {

    slo->needSavedConnectInit = 1;

  }
  else {

    slo->savedValuePvConnected = 0;

  }

  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

static void sl_monitor_control_label_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeSliderClass *slo = (activeSliderClass *) userarg;

  if ( pv->is_valid() ) {

    slo->needCtlLabelConnectInit = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();

  }

}

static void sl_monitor_read_label_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeSliderClass *slo = (activeSliderClass *) userarg;

  if ( pv->is_valid() ) {

    slo->needReadLabelConnectInit = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();

  }

}

static void sl_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeSliderClass *slo = (activeSliderClass *) userarg;
int st, sev;

  st = pv->get_status();
  sev = pv->get_severity();
  if ( ( st != slo->oldCtrlStat ) || ( sev != slo->oldCtrlSev ) ) {
    slo->oldCtrlStat = st;
    slo->oldCtrlSev = sev;
    slo->bgColor.setStatus( st, sev );
    slo->controlColor.setStatus( st, sev );
    slo->bufInvalidate();
    slo->needDraw = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();
  }

  slo->oneControlV = pv->get_double(); // xtimer updates widget indicator
  slo->curControlV = slo->oneControlV;

  if ( !slo->updateControlTimerActive ) {
    slo->updateControlTimerActive = 1;
    slo->updateControlTimerValue = 100;
    slo->updateControlTimer = appAddTimeOut(
     slo->actWin->appCtx->appContext(), slo->updateControlTimerValue,
     slc_updateControl, (void *) slo );
  }

}

static void sl_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeSliderClass *slo = (activeSliderClass *) userarg;
int st, sev;

  st = pv->get_status();
  sev = pv->get_severity();
  if ( ( st != slo->oldReadStat ) || ( sev != slo->oldReadSev ) ) {
    slo->oldReadStat = st;
    slo->oldReadSev = sev;
    slo->readColor.setStatus( st, sev );
    slo->bufInvalidate();
    slo->needDraw = 1;
  }

  slo->curReadV = pv->get_double();

  slo->needReadRefresh = 1;
  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

static void sl_savedValueUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeSliderClass *slo = (activeSliderClass *) userarg;

  slo->newSavedV = pv->get_double();

  slo->needSavedRefresh = 1;
  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

activeSliderClass::activeSliderClass ( void ) {

  name = new char[strlen("activeSliderClass")+1];
  strcpy( name, "activeSliderClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
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
  strcpy( displayFormat, "FFloat" );
  precision = 1;

  frameWidget = NULL;

  eBuf = NULL;

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

  bgColorMode = source->bgColorMode;
  controlColorMode = source->controlColorMode;
  readColorMode = source->readColorMode;

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
  accelMultiplier = source->accelMultiplier;
  savedV = source->savedV;

  positive = source->positive;

  strcpy( id, source->id );

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
  strncpy( displayFormat, source->displayFormat, 15 );

  frameWidget = NULL;

  eBuf = NULL;

  doAccSubs( controlPvName );
  doAccSubs( readPvName );
  doAccSubs( savedValuePvName );
  doAccSubs( controlLabelName );
  doAccSubs( readLabelName );

}

void activeSliderClass::doIncrement ( void ) {

double fvalue;
int stat, xOfs;

  eraseActiveControlText();
  eraseActivePointers();

  fvalue = controlV + increment;

  if ( positive ) {
    if ( fvalue < minFv ) fvalue = minFv;
    if ( fvalue > maxFv ) fvalue = maxFv;
  }
  else {
    if ( fvalue > minFv ) fvalue = minFv;
    if ( fvalue < maxFv ) fvalue = maxFv;
  }

  controlV = fvalue;

  xOfs = ( w - 4 - controlW ) / 2;

  controlX = (int) ( ( fvalue - minFv ) / factor + 0.5 ) + xOfs;

  savedX = (int) ( ( savedV - minFv ) / factor + 0.5 ) + xOfs;

  sprintf( controlValue, controlFormat, controlV );
  stat = drawActiveControlText();
  stat = drawActivePointers();

  actWin->appCtx->proc->lock();
  curControlV = controlV;
  actWin->appCtx->proc->unlock();

  if ( controlExists ) {
    stat = controlPvId->put(
     XDisplayName(actWin->appCtx->displayName), fvalue );
    if ( !stat ) fprintf( stderr, activeSliderClass_str2 );
  }
  else if ( anyCallbackFlag ) {
    needCtlRefresh = 1;
    actWin->appCtx->proc->lock();
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
  }

  controlAdjusted = 1;

  if ( changeCallback ) {
    (*changeCallback)( this );
  }

}

void activeSliderClass::doDecrement ( void ) {

double fvalue;
int stat, xOfs;

  eraseActiveControlText();
  eraseActivePointers();

  fvalue = controlV - increment;

  if ( positive ) {
    if ( fvalue < minFv ) fvalue = minFv;
    if ( fvalue > maxFv ) fvalue = maxFv;
  }
  else {
    if ( fvalue > minFv ) fvalue = minFv;
    if ( fvalue < maxFv ) fvalue = maxFv;
  }

  controlV = fvalue;

  xOfs = ( w - 4 - controlW ) / 2;

  controlX = (int) ( ( fvalue - minFv ) /
   factor + 0.5 ) + xOfs;

  savedX = (int) ( ( savedV - minFv ) /
   factor + 0.5 ) + xOfs;

  sprintf( controlValue, controlFormat, controlV );
  stat = drawActiveControlText();
  stat = drawActivePointers();

  actWin->appCtx->proc->lock();
  curControlV = controlV;
  actWin->appCtx->proc->unlock();

  if ( controlExists ) {
    stat = controlPvId->put(
     XDisplayName(actWin->appCtx->displayName), fvalue );
    if ( !stat ) fprintf( stderr, activeSliderClass_str1 );
  }
  else if ( anyCallbackFlag ) {
    needCtlRefresh = 1;
    actWin->appCtx->proc->lock();
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
  }

  controlAdjusted = 1;

  if ( changeCallback ) {
    (*changeCallback)( this );
  }

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
  accelMultiplier = 1.0;
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

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  controlColor.setColorIndex( actWin->defaultFg1Color, actWin->ci );
  readColor.setColorIndex( actWin->defaultFg2Color, actWin->ci );
  shadeColor.setColorIndex( actWin->defaultOffsetColor, actWin->ci );

  bgColorMode = 0;
  controlColorMode = 0;
  readColorMode = 0;

  strcpy( controlValue, "0.0" );
  strcpy( readValue, "0.0" );
  strcpy( controlLabel, "" );
  strcpy( readLabel, "" );

  controlLabelType = SLC_K_PV_NAME;
  strcpy( controlLabelTypeStr, activeSliderClass_str8 );

  readLabelType = SLC_K_PV_NAME;
  strcpy( readLabelTypeStr, activeSliderClass_str9 );

  // formatType = SLC_K_FORMAT_FLOAT;

  strcpy( fontTag, actWin->defaultCtlFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();
  if ( h < minH ) h = minH;
  if ( w < minW ) w = minW;

  this->draw();

  this->editCreate();

  return 1;

}

int activeSliderClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

int labelTypeLiteral = 0;
static char *labelEnumStr[3] = {
  "literal",
  "pvLabel",
  "pvName"
};
static int labelEnum[3] = {
  0,
  1,
  2
};

  major = SLC_MAJOR_VERSION;
  minor = SLC_MINOR_VERSION;
  release = SLC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadW( "bgAlarm", &bgColorMode, &zero );
  tag.loadW( "2ndBgColor", actWin->ci, &shadeColor );
  tag.loadW( "controlColor", actWin->ci, &controlColor );
  tag.loadW( "controlAlarm", &controlColorMode, &zero );
  tag.loadW( "indicatorColor", actWin->ci, &readColor );
  tag.loadW( "indicatorAlarm", &readColorMode, &zero );
  tag.loadW( "font", fontTag );
  tag.loadW( "controlPv", &controlPvName, emptyStr );
  tag.loadW( "indicatorPv", &readPvName, emptyStr );
  tag.loadW( "savedValuePv", &savedValuePvName, emptyStr );
  tag.loadW( "controlLabel", &controlLabelName, emptyStr );
  tag.loadW( "controlLabelType", 3, labelEnumStr, labelEnum, &controlLabelType,
   &labelTypeLiteral );
  tag.loadW( "readLabel", &readLabelName, emptyStr );
  tag.loadW( "readLabelType", 3, labelEnumStr, labelEnum, &readLabelType,
   &labelTypeLiteral );
  tag.loadW( "increment", &increment, &dzero );
  tag.loadW( "incMultiplier", &accelMultiplier, &dzero );
  tag.loadBoolW( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadW( "precision", &efPrecision );
  tag.loadW( "scaleMin", &efScaleMin );
  tag.loadW( "scaleMax", &efScaleMax );
  tag.loadW( "displayFormat", displayFormat );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeSliderClass::old_save (
  FILE *f )
{

int index, stat;

  fprintf( f, "%-d %-d %-d\n", SLC_MAJOR_VERSION, SLC_MINOR_VERSION,
   SLC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = shadeColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = controlColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = readColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-g\n", increment );

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

  fprintf( f, "%-d\n", bgColorMode );

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

  // version 2.1.0
  fprintf( f, "%-g\n", accelMultiplier );

  return 1;

}

int activeSliderClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, xOfs, stat;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

int labelTypeLiteral = 0;
static char *labelEnumStr[3] = {
  "literal",
  "pvLabel",
  "pvName"
};
static int labelEnum[3] = {
  0,
  1,
  2
};

  actWin = _actWin;

  tag.init();
  tag.loadR( "beginObjectProperties" );
  tag.loadR( unknownTags );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );
  tag.loadR( "x", &x );
  tag.loadR( "y", &y );
  tag.loadR( "w", &w );
  tag.loadR( "h", &h );
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "bgAlarm", &bgColorMode, &zero );
  tag.loadR( "2ndBgColor", actWin->ci, &shadeColor );
  tag.loadR( "controlColor", actWin->ci, &controlColor );
  tag.loadR( "controlAlarm", &controlColorMode, &zero );
  tag.loadR( "indicatorColor", actWin->ci, &readColor );
  tag.loadR( "indicatorAlarm", &readColorMode, &zero );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "controlPv", &controlPvName, emptyStr );
  tag.loadR( "indicatorPv", &readPvName, emptyStr );
  tag.loadR( "savedValuePv", &savedValuePvName, emptyStr );
  tag.loadR( "controlLabel", &controlLabelName, emptyStr );
  tag.loadR( "controlLabelType", 3, labelEnumStr, labelEnum,
   &controlLabelType, &labelTypeLiteral );
  tag.loadR( "readLabel", &readLabelName, emptyStr );
  tag.loadR( "readLabelType", 3, labelEnumStr, labelEnum, &readLabelType,
   &labelTypeLiteral );
  tag.loadR( "increment", &increment, &dzero );
  tag.loadR( "incMultiplier", &accelMultiplier, &dzero );
  tag.loadR( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadR( "precision", &efPrecision );
  tag.loadR( "scaleMin", &efScaleMin );
  tag.loadR( "scaleMax", &efScaleMax );
  tag.loadR( "displayFormat", 15, displayFormat );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > SLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  strcpy( this->id, "" );
  changeCallbackFlag = 0;
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;
  anyCallbackFlag = 0;

  if ( limitsFromDb || efPrecision.isNull() )
    precision = 1;
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

  curControlV = oneControlV = curReadV = controlV = readV = 0.0;

  if ( bgColorMode == SLC_K_COLORMODE_ALARM )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  if ( controlColorMode == SLC_K_COLORMODE_ALARM )
    controlColor.setAlarmSensitive();
  else
    controlColor.setAlarmInsensitive();

  if ( readColorMode == SLC_K_COLORMODE_ALARM )
    readColor.setAlarmSensitive();
  else
    readColor.setAlarmInsensitive();

  return stat;

}

int activeSliderClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, xOfs, stat, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME+1];
float val;

  actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > SLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    shadeColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    controlColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    readColor.setColorIndex( index, actWin->ci );

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    shadeColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    controlColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    readColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    shadeColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    controlColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    readColor.setColorIndex( index, actWin->ci );

  }

  fscanf( f, "%g\n", &val ); actWin->incLine();
  increment = (double) val;

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvName.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  readPvName.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  savedValuePvName.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlLabelName.setRaw( oneName );

  fscanf( f, "%d\n", &controlLabelType ); actWin->incLine();

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  readLabelName.setRaw( oneName );

  fscanf( f, "%d\n", &readLabelType ); actWin->incLine();

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  if ( ( major == 1 ) && ( minor < 4 ) ) {
    fscanf( f, "%d\n", &formatType ); // no longer in use actWin->incLine();
  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {

    fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

    fscanf( f, "%d\n", &controlColorMode ); actWin->incLine();

    fscanf( f, "%d\n", &readColorMode ); actWin->incLine();

  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    readStringFromFile( this->id, 31+1, f ); actWin->incLine();
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

    readStringFromFile( oneName, 39+1, f ); actWin->incLine();
    strncpy( displayFormat, oneName, 15 );

    if ( limitsFromDb || efPrecision.isNull() )
      precision = 1;
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

    efPrecision.setValue( 1 );
    precision = 1;
    scaleMin = 0;
    scaleMax = 10;

  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {
    fscanf( f, "%g\n", &val );
    accelMultiplier = (double) val;
  }
  else {
    accelMultiplier = 1.0;
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

  curControlV = oneControlV = curReadV = controlV = readV = 0.0;

  if ( bgColorMode == SLC_K_COLORMODE_ALARM )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  if ( controlColorMode == SLC_K_COLORMODE_ALARM )
    controlColor.setAlarmSensitive();
  else
    controlColor.setAlarmInsensitive();

  if ( readColorMode == SLC_K_COLORMODE_ALARM )
    readColor.setAlarmSensitive();
  else
    readColor.setAlarmInsensitive();

  return 1;

}

int activeSliderClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeSliderClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeSliderClass_str10, 31 );

  Strncat( title, activeSliderClass_str11, 31 );

  strncpy( bufId, id, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;
  eBuf->bufFgColor = fgColor.pixelIndex();
  eBuf->bufBgColor = bgColor.pixelIndex();
  eBuf->bufShadeColor = shadeColor.pixelIndex();
  eBuf->bufControlColor = controlColor.pixelIndex();
  eBuf->bufReadColor = readColor.pixelIndex();
  eBuf->bufBgColorMode = bgColorMode;
  eBuf->bufControlColorMode = controlColorMode;
  eBuf->bufReadColorMode = readColorMode;
  eBuf->bufIncrement = increment;
  eBuf->bufAccelMultiplier = accelMultiplier;
  strncpy( eBuf->bufFontTag, fontTag, 63 );

  eBuf->bufChangeCallbackFlag = changeCallbackFlag;
  eBuf->bufActivateCallbackFlag = activateCallbackFlag;
  eBuf->bufDeactivateCallbackFlag = deactivateCallbackFlag;

  if ( controlPvName.getRaw() )
    strncpy( eBuf->controlBufPvName, controlPvName.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strncpy( eBuf->controlBufPvName, "", 39 );

  if ( readPvName.getRaw() )
    strncpy( eBuf->readBufPvName, readPvName.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strncpy( eBuf->readBufPvName, "", 39 );

  if ( savedValuePvName.getRaw() )
    strncpy( eBuf->savedValueBufPvName, savedValuePvName.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strncpy( eBuf->savedValueBufPvName, "", 39 );

  if ( controlLabelName.getRaw() )
    strncpy( eBuf->controlBufLabelName, controlLabelName.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strncpy( eBuf->controlBufLabelName, "", 39 );

  if ( readLabelName.getRaw() )
    strncpy( eBuf->readBufLabelName, readLabelName.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strncpy( eBuf->readBufLabelName, "", 39 );

  if ( controlLabelType == SLC_K_LITERAL )
    strcpy( controlLabelTypeStr, activeSliderClass_str12 );
  else if ( controlLabelType == SLC_K_LABEL )
    strcpy( controlLabelTypeStr, activeSliderClass_str13 );
  else if ( controlLabelType == SLC_K_PV_NAME )
    strcpy( controlLabelTypeStr, activeSliderClass_str14 );

  if ( readLabelType == SLC_K_LITERAL )
    strcpy( readLabelTypeStr, activeSliderClass_str15 );
  else if ( readLabelType == SLC_K_LABEL )
    strcpy( readLabelTypeStr, activeSliderClass_str16 );
  else if ( readLabelType == SLC_K_PV_NAME )
    strcpy( readLabelTypeStr, activeSliderClass_str17 );

  eBuf->bufLimitsFromDb = limitsFromDb;
  eBuf->bufEfPrecision = efPrecision;
  eBuf->bufEfScaleMin = efScaleMin;
  eBuf->bufEfScaleMax = efScaleMax;
  strncpy( eBuf->bufDisplayFormat, displayFormat, 15 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  //ef.addTextField( activeSliderClass_str18, 35, bufId, 31 );
  ef.addTextField( activeSliderClass_str19, 35, &eBuf->bufX );
  ef.addTextField( activeSliderClass_str20, 35, &eBuf->bufY );
  ef.addTextField( activeSliderClass_str21, 35, &eBuf->bufW );
  ef.addTextField( activeSliderClass_str22, 35, &eBuf->bufH );

  ef.addTextField( activeSliderClass_str36, 35, eBuf->controlBufPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeSliderClass_str42, 35, eBuf->readBufPvName,
   PV_Factory::MAX_PV_NAME );
  rdPvEntry = ef.getCurItem();

  ef.addTextField( activeSliderClass_str48, 35, eBuf->savedValueBufPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeSliderClass_str37, 35, eBuf->controlBufLabelName,
   PV_Factory::MAX_PV_NAME );
  ctlLabelEntry = ef.getCurItem();
  ef.addOption( activeSliderClass_str38, activeSliderClass_str39,
   controlLabelTypeStr, 15 );
  ctlLabelTypeEntry = ef.getCurItem();
  ctlLabelTypeEntry->setNumValues( 3 );
  ctlLabelTypeEntry->addInvDependency( 2, ctlLabelEntry );
  ctlLabelTypeEntry->addDependencyCallbacks();

  ef.addTextField( activeSliderClass_str43, 35, eBuf->readBufLabelName,
   PV_Factory::MAX_PV_NAME );
  rdLabelEntry = ef.getCurItem();
  ef.addOption( activeSliderClass_str44, activeSliderClass_str45,
   readLabelTypeStr, 15 );
  rdLabelTypeEntry = ef.getCurItem();
  rdLabelTypeEntry->setNumValues( 3 );
  rdLabelTypeEntry->addInvDependency( 2, rdLabelEntry );
  rdLabelTypeEntry->addDependencyCallbacks();

  ef.addTextField( activeSliderClass_str28, 35, &eBuf->bufIncrement );

  ef.addTextField( activeSliderClass_str86, 35, &eBuf->bufAccelMultiplier );

  ef.addToggle( activeSliderClass_str29, &eBuf->bufLimitsFromDb );
  limitsFromDbEntry = ef.getCurItem();
  ef.addOption( activeSliderClass_str30, activeSliderClass_str35,
   eBuf->bufDisplayFormat, 15 );
  ef.addTextField( activeSliderClass_str31, 35, &eBuf->bufEfPrecision );
  precEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( precEntry );
  ef.addTextField( activeSliderClass_str32, 35, &eBuf->bufEfScaleMin );
  scaleMinEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scaleMinEntry );
  ef.addTextField( activeSliderClass_str33, 35, &eBuf->bufEfScaleMax );
  scaleMaxEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scaleMaxEntry );
  limitsFromDbEntry->addDependencyCallbacks();

  ef.addColorButton( activeSliderClass_str24, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addColorButton( activeSliderClass_str26, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  ef.addToggle( activeSliderClass_str25, &eBuf->bufBgColorMode );
  ef.addColorButton( activeSliderClass_str27, actWin->ci, &eBuf->shadeCb, &eBuf->bufShadeColor );
  ef.addColorButton( activeSliderClass_str40, actWin->ci, &eBuf->controlCb,
   &eBuf->bufControlColor );
  ef.addToggle( activeSliderClass_str41, &eBuf->bufControlColorMode );

  ef.addColorButton( activeSliderClass_str46, actWin->ci, &eBuf->readCb,
   &eBuf->bufReadColor );
  rdPvColorEntry = ef.getCurItem();
  rdPvEntry->addDependency( rdPvColorEntry );
  ef.addToggle( activeSliderClass_str47, &eBuf->bufReadColorMode );
  rdPvAlarmSensEntry = ef.getCurItem();
  rdPvEntry->addDependency( rdPvAlarmSensEntry );
  rdPvEntry->addDependencyCallbacks();

  ef.addFontMenu( activeSliderClass_str23, actWin->fi, &fm, fontTag );

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

  if ( !enabled || !activeMode || !init ) return 1;

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

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

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
  actWin->drawGc.setArcModePieSlice();

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

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( shadeColor.getColor() );
  actWin->executeGc.setArcModePieSlice();

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

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

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( controlColor.getColor() );

  actWin->executeGc.setArcModePieSlice();

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

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

  if ( !enabled || !activeMode || !init ) return 1;

  if ( fs && controlExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( bgColor.pixelColor() );

    actWin->executeGc.saveBg();
    actWin->executeGc.setBG( bgColor.pixelColor() );

    xOfs = ( adjW - controlW ) / 2;

    if ( fs ) {

      actWin->executeGc.setFontTag( fontTag, actWin->fi );

      tX = adjW-2;
      tY = 2;
      drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, controlValue );

      tX = w / 2;
      tY = 2;
      drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_CENTER, incString );

    }

    actWin->executeGc.restoreFg();
    actWin->executeGc.restoreBg();

  }

  return 1;

}

int activeSliderClass::drawActiveControlText ( void ) {

int tX, tY, xOfs;

int adjW = w - 4;

  if ( !enabled || !activeMode || !init ) return 1;

  if ( fs && controlExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( controlColor.pixelColor() );

    actWin->executeGc.saveBg();
    actWin->executeGc.setBG( shadeColor.getColor() );

    xOfs = ( adjW - controlW ) / 2;

    if ( fs ) {

      actWin->executeGc.setFontTag( fontTag, actWin->fi );

      tX = adjW-2;
      tY = 2;
      drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, controlValue );

      actWin->executeGc.setFG( controlColor.pixelColor() );

      tX = w / 2;
      tY = 2;
      drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_CENTER, incString );

    }

    actWin->executeGc.restoreFg();
    actWin->executeGc.restoreBg();

  }

  return 1;

}

int activeSliderClass::eraseActiveReadText ( void ) {

int tX, tY, xOfs;

int adjW = w - 4;

  if ( !enabled || !activeMode || !init ) return 1;

  if ( fs && readExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( bgColor.pixelColor() );

    actWin->executeGc.saveBg();
    actWin->executeGc.setBG( bgColor.pixelColor() );

    xOfs = ( adjW - controlW ) / 2;

    if ( fs ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
      tX = adjW-2;
      tY = 4+fontHeight;
      drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, readValue );
    }

    actWin->executeGc.restoreFg();
    actWin->executeGc.restoreBg();

  }

  return 1;

}

int activeSliderClass::drawActiveReadText ( void ) {

int tX, tY, xOfs;

int adjW = w - 4;

  if ( !enabled || !activeMode || !init ) return 1;

  if ( fs && readExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( readColor.pixelColor() );

    actWin->executeGc.saveBg();
    actWin->executeGc.setBG( shadeColor.getColor() );

    xOfs = ( adjW - controlW ) / 2;

    if ( fs ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
      tX = adjW-2;
      tY = 4+fontHeight;
      drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, readValue );
    }

    actWin->executeGc.restoreFg();
    actWin->executeGc.restoreBg();

  }

  return 1;

}

void zzz( void ) {

  fprintf( stderr, "zzz\n" );

}

int activeSliderClass::drawActive ( void ) {

int arcX, arcY, tX, tY, xOfs, lineX;

int adjW = w - 4;

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();
  actWin->executeGc.setFG( bgColor.pixelColor() );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  XFillRectangle( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), 0, 0, w, h );

  actWin->executeGc.setFG( shadeColor.getColor() );

  actWin->executeGc.setArcModePieSlice();

  xOfs = ( adjW - controlAreaW ) / 2;
  XFillRectangle( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), xOfs, valueAreaH, controlAreaW,
   controlAreaH );

  actWin->executeGc.setFG( bgColor.getColor() );
  XFillRectangle( actWin->d, XtWindow(sliderWidget),
   actWin->executeGc.normGC(), 0, valueAreaH+controlAreaH, w,
   h-valueAreaH-controlAreaH );

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

      actWin->executeGc.setFG( controlColor.pixelColor() );

      actWin->executeGc.saveBg();
      actWin->executeGc.setBG( shadeColor.getColor() );

      if ( fs ) {

        tX = adjW-2;
        tY = 2;
        drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_END, controlValue );

        actWin->executeGc.setFG( controlColor.pixelColor() );

        tX = w / 2;
        tY = 2;
        drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_CENTER, incString );

      }

      actWin->executeGc.restoreBg();

      actWin->executeGc.saveBg();
      actWin->executeGc.setFG( fgColor.getColor() );
      actWin->executeGc.setBG( shadeColor.getColor() );
      tX = markX1;
      tY = markY0;
      drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, activeSliderClass_str52 );
      tX = restoreX0;
      tY = restoreY0;
      drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_BEGINNING, activeSliderClass_str53 );
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

      actWin->executeGc.setFG( readColor.pixelColor() );

      actWin->executeGc.saveBg();
      actWin->executeGc.setBG( shadeColor.getColor() );

      if ( fs ) {
        tX = adjW-2;
        tY = 4+fontHeight;
        drawImageText( sliderWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_END, readValue );
      }

      actWin->executeGc.restoreBg();

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
int stat, b2Op, deltaX, xOfs, newX, popupDialog = 0;
double fvalue;
char title[32], *ptr;
int tX, tY, x0, y0, x1, y1, incX0, incY0, incX1, incY1;

  *continueToDispatch = True;

  slo = (activeSliderClass *) client;

  if ( !slo->active ) return;

  if ( e->type == EnterNotify ) {
    if ( !slo->controlPvId->have_write_access() ) {
      slo->actWin->cursor.set( XtWindow(slo->actWin->executeWidget),
       CURSOR_K_NO );
    }
    else {
      slo->actWin->cursor.set( XtWindow(slo->actWin->executeWidget),
       CURSOR_K_DEFAULT );
    }
  }

  if ( e->type == LeaveNotify ) {
    slo->actWin->cursor.set( XtWindow(slo->actWin->executeWidget),
     CURSOR_K_DEFAULT );
  }

  ptr = slo->actWin->obj.getNameFromClass( "activeSliderClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeSliderClass_str54, 31 );

  Strncat( title, activeSliderClass_str55, 31 );

  if ( e->type == Expose ) {

    slo->bufInvalidate();
    stat = slo->drawActive();
//      stat = slo->drawActiveControlText();
//      stat = slo->drawActivePointers();

  }

  // allow Button2 operations when no write access
  b2Op = 0;
  if ( ( e->type == ButtonPress ) || ( e->type == ButtonRelease ) ) {
    be = (XButtonEvent *) e;
    if ( be->button == Button2 ) {
      b2Op = 1;
    }
  }

  if ( slo->controlPvId ) {
    if ( !slo->controlPvId->have_write_access() && !b2Op ) return;
  }

  if ( e->type == ButtonPress ) {

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
          slo->savedValuePvId->put(
           XDisplayName(slo->actWin->appCtx->displayName), slo->savedV );
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
          if ( slo->controlPvId ) {
            stat = slo->controlPvId->put(
             XDisplayName(slo->actWin->appCtx->displayName), fvalue );
            if ( !stat ) fprintf( stderr, activeSliderClass_str56 );
	  }
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

        /* dragging pointer */

        slo->xRef = be->x;

        slo->controlState = SLC_STATE_MOVING;

      }
      else if ( ( be->x > slo->controlX + slo->controlAreaH/2 ) &&
              ( be->y > slo->valueAreaH ) ) {

        /* auto inc */

        slo->doIncrement();

        slo->incrementTimerActive = 1;
        slo->incrementTimerValue = 101;

        slo->incrementTimer = appAddTimeOut(
         slo->actWin->appCtx->appContext(), 500,
         slc_increment, (void *) slo );

      }
      else if ( ( be->x < slo->controlX + slo->controlAreaH/2 ) &&
              ( be->y > slo->valueAreaH ) ) {

        /* auto dec */

        slo->doDecrement();

        slo->incrementTimerActive = 1;
        slo->incrementTimerValue = 101;

        slo->incrementTimer = appAddTimeOut(
         slo->actWin->appCtx->appContext(), 500,
         slc_decrement, (void *) slo );

      }
      else if ( ( be->y < slo->valueAreaH ) &&
                !slo->ef.formIsPoppedUp() &&
                !slo->kp.isPoppedUp() ) {

        tX = slo->w - 6;
        tY = 2;
        stat = textBoundaries( slo->fs, tX, tY, XmALIGNMENT_END,
         slo->controlValue, &x0, &y0, &x1, &y1 );

        tX = slo->w / 2;
        tY = 2;
        stat = textBoundaries( slo->fs, tX, tY, XmALIGNMENT_CENTER,
         slo->incString, &incX0, &incY0, &incX1, &incY1 );

        if ( ( be->x > x0 ) &&
             ( be->x < x1 ) &&
             ( be->y > y0 ) &&
             ( be->y < y1 ) ) {

	  Widget parent;
	  if ( useAppTopParent() ) {
            parent = slo->actWin->appCtx->apptop();
	  }
	  else {
	    parent = slo->actWin->top;
	  }
          slo->kp.create( parent,
           be->x_root, be->y_root,
           "", &slo->kpCtlDouble,
           (void *) slo,
           (XtCallbackProc) sloSetCtlKpDoubleValue,
           (XtCallbackProc) sloCancelKp );

        }
        else if ( ( be->x > incX0 ) &&
             ( be->x < incX1 ) &&
             ( be->y > incY0 ) &&
             ( be->y < incY1 ) ) {

	  Widget parent;
	  if ( useAppTopParent() ) {
            parent = slo->actWin->appCtx->apptop();
	  }
	  else {
	    parent = slo->actWin->top;
	  }
          slo->kp.create( parent,
           be->x_root, be->y_root,
           "", &slo->kpIncDouble,
           (void *) slo,
           (XtCallbackProc) sloSetIncKpDoubleValue,
           (XtCallbackProc) sloCancelKp );

        }
	else {

	  popupDialog = 1;

	}

      }

      break;

//========== B1 Press ========================================

//========== B2 Press ========================================

    case Button2:

      slo->controlState = SLC_STATE_IDLE;
      slo->incrementTimerActive = 0;
      slo->incrementTimerValue = 101;

      if ( !( be->state & ( ControlMask | ShiftMask ) ) ) {
        stat = slo->startDrag( w, e );
      }
      else if ( ( be->state & ShiftMask ) &&
                ( be->state & ControlMask ) ) {
        stat = slo->showPvInfo( be, be->x, be->y );
      }

      break;

//========== B2 Press ========================================

//========== B3 Press ========================================

    case Button3:

      slo->controlState = SLC_STATE_IDLE;
      slo->incrementTimerActive = 0;
      slo->incrementTimerValue = 101;

      if ( ( be->x > slo->controlX - slo->controlAreaH/2 ) &&
           ( be->x < slo->controlX + slo->controlAreaH/2 ) &&
           ( be->y > slo->valueAreaH ) &&
           ( be->y < slo->valueAreaH + slo->controlAreaH ) ) {

        slo->xRef = be->x;

        slo->controlState = SLC_STATE_MOVING;

      }
      else {

	popupDialog = 1;

      }

      break;

//========== B3 Press ========================================

//========== B4 Press ========================================

#if 0
    case Button4:

      slo->doIncrement();

      break;
#endif

//========== B4 Press ========================================

//========== B5 Press ========================================

#if 0
    case Button5:

      slo->doDecrement();

      break;
#endif

//========== B5 Press ========================================

    }

  }
  if ( e->type == ButtonRelease ) {

//========== Any B Release ========================================

    if ( slo->incrementTimerActive ) {
      XtRemoveTimeOut( slo->incrementTimer );
    }

    slo->controlState = SLC_STATE_IDLE;
    slo->incrementTimerActive = 0;
    slo->incrementTimerValue = 101;

    be = (XButtonEvent *) e;

    switch ( be->button ) {

    case Button2:

      if ( ( be->state & ShiftMask ) &&
           !( be->state & ControlMask ) ) {
        stat = slo->selectDragValue( be );
      }
      else if ( !( be->state & ShiftMask ) &&
                ( be->state & ControlMask ) ) {
        slo->doActions( be, be->x, be->y );
      }

      break;

    }

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
        fvalue = slo->factor * (double) ( newX - xOfs ) + slo->minFv;

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
          if ( slo->controlPvId ) {
            stat = slo->controlPvId->put(
             XDisplayName(slo->actWin->appCtx->displayName), fvalue );
            if ( !stat ) fprintf( stderr, activeSliderClass_str59 );
	  }
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

//========== B3 Motion ========================================

    if ( me->state & Button3Mask ) {

      if ( slo->controlState == SLC_STATE_MOVING ) {

        deltaX = abs ( me->x - slo->xRef );
        if ( me->state & ShiftMask ) deltaX *= -1;

        slo->eraseActiveControlText();
        slo->eraseActivePointers();

        slo->xRef = me->x;
        fvalue = slo->controlV + (double) deltaX * slo->increment;

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
          if ( slo->controlPvId ) {
            stat = slo->controlPvId->put(
             XDisplayName(slo->actWin->appCtx->displayName), fvalue );
            if ( !stat ) fprintf( stderr, activeSliderClass_str60 );
	  }
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

//========== B3 Motion ========================================

//========== Motion with no button press ======================

    if ( me->state == 0 ) {

      tX = slo->w - 6;
      tY = 2;
      stat = textBoundaries( slo->fs, tX, tY, XmALIGNMENT_END,
       slo->controlValue, &x0, &y0, &x1, &y1 );

      tX = slo->w / 2;
      tY = 2;
      stat = textBoundaries( slo->fs, tX, tY, XmALIGNMENT_CENTER,
       slo->incString, &incX0, &incY0, &incX1, &incY1 );

      if ( ( me->x > slo->markX0 ) &&
        ( me->x < slo->markX1 ) &&
        ( me->y > slo->markY0 ) &&
        ( me->y < slo->markY1 ) ) {

        if ( !slo->overSave ) {

          slo->overSave = 1;
          slo->actWin->executeGc.setFontTag( slo->fontTag, slo->actWin->fi );
          slo->actWin->executeGc.saveFg();
          slo->actWin->executeGc.saveBg();
          slo->actWin->executeGc.setBG( slo->fgColor.getColor() );
          slo->actWin->executeGc.setFG( slo->shadeColor.getColor() );
          drawImageText( slo->sliderWidget, &slo->actWin->executeGc, slo->fs,
           slo->markX1, slo->markY0, XmALIGNMENT_END,
           activeSliderClass_str52 );
          slo->actWin->executeGc.restoreFg();
          slo->actWin->executeGc.restoreBg();

	}

      }
      else {

	if ( slo->overSave ) {
	  slo->overSave = 0;
          slo->actWin->executeGc.setFontTag( slo->fontTag, slo->actWin->fi );
          slo->actWin->executeGc.saveFg();
          slo->actWin->executeGc.saveBg();
          slo->actWin->executeGc.setFG( slo->fgColor.getColor() );
          slo->actWin->executeGc.setBG( slo->shadeColor.getColor() );
          drawImageText( slo->sliderWidget, &slo->actWin->executeGc, slo->fs,
           slo->markX1, slo->markY0, XmALIGNMENT_END,
           activeSliderClass_str52 );
          slo->actWin->executeGc.restoreFg();
          slo->actWin->executeGc.restoreBg();
	}

      }

      if ( ( me->x > slo->restoreX0 ) &&

        ( me->x < slo->restoreX1 ) &&
        ( me->y > slo->restoreY0 ) &&
        ( me->y < slo->restoreY1 ) ) {

        if ( !slo->overRestore ) {
          slo->overRestore = 1;
          slo->actWin->executeGc.setFontTag( slo->fontTag, slo->actWin->fi );
          slo->actWin->executeGc.saveFg();
          slo->actWin->executeGc.saveBg();
          slo->actWin->executeGc.setBG( slo->fgColor.getColor() );
          slo->actWin->executeGc.setFG( slo->shadeColor.getColor() );
          drawImageText( slo->sliderWidget, &slo->actWin->executeGc, slo->fs,
           slo->restoreX0, slo->restoreY0, XmALIGNMENT_BEGINNING,
           activeSliderClass_str53 );
          slo->actWin->executeGc.restoreFg();
          slo->actWin->executeGc.restoreBg();
        }

      }
      else {

	if ( slo->overRestore ) {
	  slo->overRestore = 0;
          slo->actWin->executeGc.setFontTag( slo->fontTag, slo->actWin->fi );
          slo->actWin->executeGc.saveFg();
          slo->actWin->executeGc.saveBg();
          slo->actWin->executeGc.setFG( slo->fgColor.getColor() );
          slo->actWin->executeGc.setBG( slo->shadeColor.getColor() );
          drawImageText( slo->sliderWidget, &slo->actWin->executeGc, slo->fs,
           slo->restoreX0, slo->restoreY0, XmALIGNMENT_BEGINNING,
           activeSliderClass_str53 );
          slo->actWin->executeGc.restoreFg();
          slo->actWin->executeGc.restoreBg();
	}

      }

      if ( ( me->x > incX0 ) &&
        ( me->x < incX1 ) &&
        ( me->y > incY0 ) &&
        ( me->y < incY1 ) ) {

        if ( !slo->overInc ) {
          slo->overInc = 1;
          slo->actWin->executeGc.setFontTag( slo->fontTag, slo->actWin->fi );
          slo->actWin->executeGc.saveFg();
          slo->actWin->executeGc.saveBg();
          slo->actWin->executeGc.setBG( slo->fgColor.getColor() );
          slo->actWin->executeGc.setFG( slo->shadeColor.getColor() );
          drawImageText( slo->sliderWidget, &slo->actWin->executeGc, slo->fs,
           (int) (slo->w/2), 2, XmALIGNMENT_CENTER, slo->incString );
          slo->actWin->executeGc.restoreFg();
          slo->actWin->executeGc.restoreBg();
        }

      }
      else {

        if ( slo->overInc ) {
          slo->overInc = 0;
          slo->actWin->executeGc.setFontTag( slo->fontTag, slo->actWin->fi );
          slo->actWin->executeGc.saveFg();
          slo->actWin->executeGc.saveBg();
          slo->actWin->executeGc.setFG( slo->fgColor.getColor() );
          slo->actWin->executeGc.setBG( slo->shadeColor.getColor() );
          drawImageText( slo->sliderWidget, &slo->actWin->executeGc, slo->fs,
           (int) (slo->w/2), 2, XmALIGNMENT_CENTER, slo->incString );
          slo->actWin->executeGc.restoreFg();
          slo->actWin->executeGc.restoreBg();
	}

      }

      if ( ( me->x > x0 ) &&
        ( me->x < x1 ) &&
        ( me->y > y0 ) &&
        ( me->y < y1 ) ) {

        if ( !slo->overControl ) {
          slo->overControl = 1;
          slo->actWin->executeGc.setFontTag( slo->fontTag, slo->actWin->fi );
          slo->actWin->executeGc.saveFg();
          slo->actWin->executeGc.saveBg();
          slo->actWin->executeGc.setBG( slo->fgColor.getColor() );
          slo->actWin->executeGc.setFG( slo->shadeColor.getColor() );
          drawImageText( slo->sliderWidget, &slo->actWin->executeGc, slo->fs,
           x0, y0, XmALIGNMENT_BEGINNING, slo->controlValue );
          slo->actWin->executeGc.restoreFg();
          slo->actWin->executeGc.restoreBg();
        }

      }
      else {

        if ( slo->overControl ) {
          slo->overControl = 0;
          slo->actWin->executeGc.setFontTag( slo->fontTag, slo->actWin->fi );
          slo->actWin->executeGc.saveFg();
          slo->actWin->executeGc.saveBg();
          slo->actWin->executeGc.setFG( slo->fgColor.getColor() );
          slo->actWin->executeGc.setBG( slo->shadeColor.getColor() );
          drawImageText( slo->sliderWidget, &slo->actWin->executeGc, slo->fs,
           x0, y0, XmALIGNMENT_BEGINNING, slo->controlValue );
          slo->actWin->executeGc.restoreFg();
          slo->actWin->executeGc.restoreBg();
	}

      }


    }

//========== Motion with no button press ======================

  }

  if ( popupDialog ) {

    be = (XButtonEvent *) e;

    if ( !slo->eBuf ) {
      slo->eBuf = new activeSliderClass::editBufType;
    }

    slo->eBuf->bufIncrement = slo->increment;
    slo->eBuf->bufAccelMultiplier = slo->accelMultiplier;
    slo->eBuf->bufControlV = slo->controlV;
    slo->valueFormX = be->x_root;
    slo->valueFormY = be->y_root;
    slo->valueFormW = 0;
    slo->valueFormH = 0;
    slo->valueFormMaxH = 600;

    slo->ef.create( slo->actWin->top,
     slo->actWin->appCtx->ci.getColorMap(),
     &slo->valueFormX, &slo->valueFormY,
     &slo->valueFormW, &slo->valueFormH, &slo->valueFormMaxH,
     title, NULL, NULL, NULL );

    slo->ef.addTextField( activeSliderClass_str57, 14,
     &slo->eBuf->bufControlV );
    slo->ef.addTextField( activeSliderClass_str58, 14,
     &slo->eBuf->bufIncrement );
    slo->ef.addTextField( activeSliderClass_str86, 14,
     &slo->eBuf->bufAccelMultiplier );
    slo->ef.finished( slc_value_ok, slc_value_apply, slc_value_cancel,
     slo );
    slo->ef.popup();

  }

}

int activeSliderClass::activate (
  int pass,
  void *ptr )
{

int opStat;
char callbackName[63+1];

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      opStat = 1;

      initEnable();

      frameWidget = XtVaCreateManagedWidget( "", xmBulletinBoardWidgetClass,
       actWin->executeWidgetId(),
       XmNx, x,
       XmNy, y,
       XmNwidth, w,
       XmNheight, h,
       XmNmarginHeight, 0,
       XmNmarginWidth, 0,
       XmNshadowThickness, 2,
       XmNtopShadowColor, shadeColor.pixelColor(),
       XmNbottomShadowColor, BlackPixel( actWin->display(),
       DefaultScreen(actWin->display()) ),
       XmNshadowType, XmSHADOW_ETCHED_OUT,
       XmNmappedWhenManaged, False,
       NULL );

      if ( !frameWidget ) {
        fprintf( stderr, activeSliderClass_str61 );
        return 0;
      }

      sliderWidget = XtVaCreateManagedWidget( "", xmDrawingAreaWidgetClass,
       frameWidget,
       XmNx, 2,
       XmNy, 2,
       XmNwidth, w-4,
       XmNheight, h-4,
       XmNbackground, bgColor.pixelColor(),
       NULL );

      if ( !sliderWidget ) {
        fprintf( stderr, activeSliderClass_str62 );
        return 0;
      }

      overSave = overRestore = overInc = overControl = overRead = 0;

      XtAddEventHandler( sliderWidget,
       ButtonPressMask|ButtonReleaseMask|PointerMotionMask|ExposureMask|
       EnterWindowMask|LeaveWindowMask, False,
       sliderEventHandler, (XtPointer) this );

      if ( enabled ) {
        if ( frameWidget ) XtMapWidget( frameWidget );
      }

      strcpy( readValue, "" );
      strcpy( controlValue, "" );
      strcpy( incString, "" );
      activeMode = 1;
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
      initialConnection = initialReadConnection =
       initialSavedValueConnection = 1;
      oldCtrlStat = oldCtrlSev = oldReadStat = oldReadSev = -1;
      oldControlV = 0;
      updateControlTimerActive = 0;
      updateControlTimer = 0;
      controlAdjusted = 0;
      incrementTimerActive = 0;
      controlPvId = controlLabelPvId = readPvId = readLabelPvId =
       savedValuePvId = NULL;
      savedV = 0.0;
      minFv = maxFv = 0.0;
      factor = 1.0;

      controlState = SLC_STATE_IDLE;

      controlPvConnected = readPvConnected = savedValuePvConnected = 0;

      fgColor.setConnectSensitive();

      if ( !controlPvName.getExpanded() ||
	 // ( strcmp( controlPvName.getExpanded(), "" ) == 0 ) ) {
         blankOrComment( controlPvName.getExpanded() ) ) {
        controlExists = 0;
      }
      else {
        controlExists = 1;
        controlColor.setConnectSensitive();
      }

      if ( !readPvName.getExpanded() ||
	 // ( strcmp( readPvName.getExpanded(), "" ) == 0 ) ) {
         blankOrComment( readPvName.getExpanded() ) ) {
        readExists = 0;
      }
      else {
        readExists = 1;
        readColor.setConnectSensitive();
      }

      if ( !savedValuePvName.getExpanded() ||
	 // ( strcmp( savedValuePvName.getExpanded(), "" ) == 0 ) ) {
         blankOrComment( savedValuePvName.getExpanded() ) ) {
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
        strncpy( controlLabel, controlPvName.getExpanded(),
         PV_Factory::MAX_PV_NAME );
      }
      else {
        strncpy( controlLabel, controlLabelName.getExpanded(),
         PV_Factory::MAX_PV_NAME );
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
        strncpy( readLabel, readPvName.getExpanded(),
         PV_Factory::MAX_PV_NAME );
      }
      else {
        strncpy( readLabel, readLabelName.getExpanded(),
         PV_Factory::MAX_PV_NAME );
      }

      if ( controlExists ) {
	controlPvId = the_PV_Factory->create( controlPvName.getExpanded() );
	if ( controlPvId ) {
	  controlPvId->add_conn_state_callback(
           sl_monitor_control_connect_state, this );
	}
	else {
          fprintf( stderr, activeSliderClass_str63 );
          opStat = 0;
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
          Strncat( callbackName, activeSliderClass_str64, 63 );
          changeCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, activeSliderClass_str65, 63 );
          activateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( deactivateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, activeSliderClass_str66, 63 );
          deactivateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallback ) {
          (*activateCallback)( this );
        }


      }

      if ( controlLabelExists && ( controlLabelType == SLC_K_LABEL )  ) {
        controlLabelPvId = the_PV_Factory->create(
         controlLabelName.getExpanded() );
	if ( controlLabelPvId ) {
          controlLabelPvId->add_conn_state_callback(
           sl_monitor_control_label_connect_state, this );
	}
	else {
          fprintf( stderr, activeSliderClass_str63 );
          opStat = 0;
        }
      }

      if ( readExists ) {
        readPvId = the_PV_Factory->create( readPvName.getExpanded() );
	if ( readPvId ) {
	  readPvId->add_conn_state_callback( sl_monitor_read_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeSliderClass_str63 );
          opStat = 0;
        }
      }

      if ( readLabelExists && ( readLabelType == SLC_K_LABEL )  ) {
        readLabelPvId = the_PV_Factory->create(
         readLabelName.getExpanded() );
	if ( readLabelPvId ) {
          readLabelPvId->add_conn_state_callback(
           sl_monitor_read_label_connect_state, this );
	}
	else {
          fprintf( stderr, activeSliderClass_str63 );
          opStat = 0;
        }
      }

      if ( savedValueExists ) {
        savedValuePvId = the_PV_Factory->create(
         savedValuePvName.getExpanded() );
	if ( savedValuePvId ) {
          savedValuePvId->add_conn_state_callback(
           sl_monitor_saved_connect_state, this );
	}
	else {
          fprintf( stderr, activeSliderClass_str63 );
          opStat = 0;
        }
      }

      if ( opStat & 1 ) {

        //updateControlTimerActive = 1;
        //updateControlTimerValue = 100;
        //updateControlTimer = appAddTimeOut( actWin->appCtx->appContext(),
        // updateControlTimerValue, slc_updateControl, (void *) this );

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

  activeMode = 0;

  if ( ef.formIsPoppedUp() ) {
    ef.popdown();
  }

  if ( kp.isPoppedUp() ) {
    kp.popdown();
  }

  if ( deactivateCallback ) {
    (*deactivateCallback)( this );
  }

  switch ( pass ) {

  case 1:

    if ( updateControlTimerActive ) {
      updateControlTimerActive = 0;
      if ( updateControlTimer ) {
        XtRemoveTimeOut( updateControlTimer );
        updateControlTimer = 0;
      }
    }

    XtRemoveEventHandler( sliderWidget,
     ButtonPressMask|ButtonReleaseMask|PointerMotionMask|ExposureMask|
     EnterWindowMask|LeaveWindowMask, False,
     sliderEventHandler, (XtPointer) this );

    if ( controlExists ) {
      if ( controlPvId ) {
        controlPvId->remove_conn_state_callback(
         sl_monitor_control_connect_state, this );
	controlPvId->remove_value_callback( sl_controlUpdate, this );
        controlPvId->release();
	controlPvId = NULL;
      }
    }
  
    if ( controlLabelExists && ( controlLabelType == SLC_K_LABEL )  ) {
      if ( controlLabelPvId ) {
	controlLabelPvId->remove_conn_state_callback(
         sl_monitor_control_label_connect_state, this );
	controlLabelPvId->release();
	controlLabelPvId = NULL;
      }
    }

    if ( readExists ) {
      if ( readPvId ) {
        readPvId->remove_conn_state_callback(
         sl_monitor_read_connect_state, this );
	readPvId->remove_value_callback( sl_readUpdate, this );
        readPvId->release();
	readPvId = NULL;
      }
    }

    if ( readLabelExists && ( readLabelType == SLC_K_LABEL )  ) {
      if ( readLabelPvId ) {
	readLabelPvId->remove_conn_state_callback(
         sl_monitor_read_label_connect_state, this );
	readLabelPvId->release();
	readLabelPvId = NULL;
      }
    }

    if ( savedValueExists ) {
      if ( savedValuePvId ) {
        savedValuePvId->remove_conn_state_callback(
         sl_monitor_saved_connect_state, this );
	savedValuePvId->remove_value_callback( sl_savedValueUpdate, this );
        savedValuePvId->release();
	savedValuePvId = NULL;
      }
    }

    break;

  case 2:

    if ( frameWidget ) {
      XtUnmapWidget( frameWidget );
      XtDestroyWidget( frameWidget );
    }

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
    markX0 = markX1 - XTextWidth( fs, activeSliderClass_str76, 4 );
    markY1 = valueAreaH+controlAreaH+labelAreaH-2;
    markY0 = markY1 - fontHeight;
    restoreX0 = adjW/2+4;
    restoreX1 = restoreX0 + XTextWidth( fs, activeSliderClass_str77, 4 );
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

int activeSliderClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( controlPvName.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  controlPvName.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( readPvName.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readPvName.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( savedValuePvName.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  savedValuePvName.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( controlLabelName.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  controlLabelName.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( readLabelName.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readLabelName.setRaw( tmpStr.getExpanded() );

  return 1;

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
 nrli, ne, nd, prec;
double rv, cv, fv;

  if ( actWin->isIconified ) return;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
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

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( ncc ) {

    controlPvConnected = 1;

    if ( limitsFromDb || efScaleMin.isNull() ) {
      scaleMin = controlPvId->get_lower_disp_limit();
    }

    if ( limitsFromDb || efScaleMax.isNull() ) {
      scaleMax = controlPvId->get_upper_disp_limit();
    }

    if ( limitsFromDb || efPrecision.isNull() ) {
      precision = controlPvId->get_precision();
    }

    if ( strcmp( displayFormat, "GFloat" ) == 0 ) {
      sprintf( controlFormat, "%%.%-dg", precision );
    }
    else if ( strcmp( displayFormat, "Exponential" ) == 0 ) {
      sprintf( controlFormat, "%%.%-de", precision );
    }
    else {
      sprintf( controlFormat, "%%.%-df", precision );
    }

    minFv = scaleMin;

    maxFv = scaleMax;

    cv = curControlV = controlPvId->get_double();

    nci = 1;

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

    if ( controlExists && initialConnection ) {

      initialConnection = 0;

      controlPvId->add_value_callback( sl_controlUpdate, this );

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

    if ( limitsFromDb || efPrecision.isNull() )
      prec = readPvId->get_precision();
    else
      prec = precision;

    if ( strcmp( displayFormat, "GFloat" ) == 0 ) {
      sprintf( readFormat, "%%.%-dg", prec );
    }
    else if ( strcmp( displayFormat, "Exponential" ) == 0 ) {
      sprintf( readFormat, "%%.%-de", prec );
    }
    else {
      sprintf( readFormat, "%%.%-df", prec );
    }

    rv = curReadV = readPvId->get_double();

    nri = 1;

  }

//----------------------------------------------------------------------------

  if ( nri ) {

    readV = rv;

    sprintf( readValue, readFormat, readV );

    if ( initialReadConnection ) {

      initialReadConnection = 0;

      readPvId->add_value_callback( sl_readUpdate, this );

    }

  }

//----------------------------------------------------------------------------

  if ( nsc ) {

    if ( initialSavedValueConnection ) {

      initialSavedValueConnection = 0;

      savedValuePvId->add_value_callback( sl_savedValueUpdate, this );

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

    controlLabelPvId->get_string( controlLabel, 39 );
    controlLabel[39] = 0;

    ncli = 1;

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

    readLabelPvId->get_string( readLabel, 39 );
    readLabel[39] = 0;

    nrli = 1;

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

  if ( strcmp( prop, activeSliderClass_str85 ) == 0 ) {

    *_value = controlV;
    return 1;

  }

  return 0;

}

char *activeSliderClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeSliderClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeSliderClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    switch ( i ) {
    case 0:
      return controlPvName.getExpanded();
      break;
    case 1:
      return readPvName.getExpanded();
      break;
    case 2:
      return savedValuePvName.getExpanded();
      break;
    }

  }
  else {

    switch ( i ) {
    case 0:
      return controlPvName.getRaw();
      break;
    case 1:
      return readPvName.getRaw();
      break;
    case 2:
      return savedValuePvName.getRaw();
      break;
    }

  }

  return NULL;

}

void activeSliderClass::changeDisplayParams (
  unsigned int _flag,
  char *_fontTag,
  int _alignment,
  char *_ctlFontTag,
  int _ctlAlignment,
  char *_btnFontTag,
  int _btnAlignment,
  int _textFgColor,
  int _fg1Color,
  int _fg2Color,
  int _offsetColor,
  int _bgColor,
  int _topShadowColor,
  int _botShadowColor )
{

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColor.setColorIndex( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_FG1COLOR_MASK )
    controlColor.setColorIndex( _fg1Color, actWin->ci );

  if ( _flag & ACTGRF_FG2COLOR_MASK )
    readColor.setColorIndex( _fg2Color, actWin->ci );

  if ( _flag & ACTGRF_OFFSETCOLOR_MASK )
    shadeColor.setColorIndex( _offsetColor, actWin->ci );

  if ( _flag & ACTGRF_CTLFONTTAG_MASK ) {

    strcpy( fontTag, _ctlFontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );

    updateDimensions();
    if ( h < minH ) h = minH;
    if ( w < minW ) w = minW;

  }

}

void activeSliderClass::changePvNames (
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

  if ( flag & ACTGRF_CTLPVS_MASK ) {
    if ( numCtlPvs ) {
      controlPvName.setRaw( ctlPvs[0] );
    }
  }

  if ( flag & ACTGRF_READBACKPVS_MASK ) {
    if ( numReadbackPvs ) {
      readPvName.setRaw( readbackPvs[0] );
    }
  }

}

void activeSliderClass::map ( void ) {

  if ( frameWidget ) XtMapWidget( frameWidget );

}

void activeSliderClass::unmap ( void ) {

  if ( frameWidget ) XtUnmapWidget( frameWidget );
  controlState = SLC_STATE_IDLE;
  incrementTimerActive = 0;
  incrementTimerValue = 101;

}

void activeSliderClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 3 ) {
    *n = 0;
    return;
  }

  *n = 3;
  pvs[0] = controlPvId;
  pvs[1] = readPvId;
  pvs[2] = savedValuePvId;

}

char *activeSliderClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return controlPvName.getRaw();
  }
  else if ( i == 1 ) {
    return readPvName.getRaw();
  }
  else if ( i == 2 ) {
    return savedValuePvName.getRaw();
  }
  else if ( i == 3 ) {
    return controlLabelName.getRaw();
  }
  else if ( i == 4 ) {
    return readLabelName.getRaw();
  }

  return NULL;

}

void activeSliderClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    controlPvName.setRaw( string );
  }
  else if ( i == 1 ) {
    readPvName.setRaw( string );
  }
  else if ( i == 2 ) {
    savedValuePvName.setRaw( string );
  }
  else if ( i == 3 ) {
    controlLabelName.setRaw( string );
  }
  else if ( i == 4 ) {
    readLabelName.setRaw( string );
  }

}

// crawler functions may return blank pv names
char *activeSliderClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return controlPvName.getExpanded();

}

char *activeSliderClass::crawlerGetNextPv ( void ) {

int max;

  max = 2;

  if ( controlLabelType != SLC_K_LITERAL ) {
    max++;
  }

  if ( readLabelType != SLC_K_LITERAL ) {
    max++;
  }

  if ( crawlerPvIndex >=max ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return readPvName.getExpanded();
  }
  else if ( crawlerPvIndex == 2 ) {
    return savedValuePvName.getExpanded();
  }
  else if ( crawlerPvIndex == 3 ) {

    if ( controlLabelType != SLC_K_LITERAL ) {
      return controlLabelName.getExpanded();
    }
    else if ( readLabelType != SLC_K_LITERAL ) {
      return readLabelName.getExpanded();
    }

  }
  else if ( crawlerPvIndex == 4 ) {
    return readLabelName.getExpanded();
  }

  return NULL;

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
