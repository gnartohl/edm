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
int stat;

  if ( slo->controlExists ) {
    stat = ca_put( DBR_DOUBLE, slo->controlPvId, &slo->kpCtlDouble );
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
double fvalue;
int stat, xOfs;

  if ( slo->incrementTimerActive ) {
    if ( slo->incrementTimerValue > 1 ) slo->incrementTimerValue -= 10;
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

// for EPICS support

  if ( slo->controlExists ) {
#ifdef __epics__
  stat = ca_put( DBR_DOUBLE, slo->controlPvId, &fvalue );
  if ( stat != ECA_NORMAL ) printf( activeSliderClass_str1 );
#endif
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

  if ( slo->incrementTimerActive ) {
    if ( slo->incrementTimerValue > 1 ) slo->incrementTimerValue -= 10;
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

// for EPICS support

  if ( slo->controlExists ) {
#ifdef __epics__
  stat = ca_put( DBR_DOUBLE, slo->controlPvId, &fvalue );
  if ( stat != ECA_NORMAL ) printf( activeSliderClass_str2 );
#endif
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

  fvalue = (double) slo->bufControlV;

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

// for EPICS support

  if ( slo->controlExists ) {
#ifdef __epics__
    stat = ca_put( DBR_DOUBLE, slo->controlPvId, &fvalue );
    if ( stat != ECA_NORMAL ) printf( activeSliderClass_str3 );
    //slo->needCtlRefresh = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();
#endif
  }
  else if ( slo->anyCallbackFlag ) {
    //slo->needCtlRefresh = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();
  }

  slo->controlAdjusted = 1;

  if ( slo->changeCallback ) {
    (*slo->changeCallback)( slo );
  }

  slo->needErase = 1;
  slo->needDraw = 1;

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

  slo->fgColor.setColorIndex( slo->bufFgColor, slo->actWin->ci );
  slo->bgColor.setColorIndex( slo->bufBgColor, slo->actWin->ci );
  slo->shadeColor.setColorIndex( slo->bufShadeColor, slo->actWin->ci );
  slo->controlColor.setColorIndex( slo->bufControlColor, slo->actWin->ci );
  slo->readColor.setColorIndex( slo->bufReadColor, slo->actWin->ci );

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
  if ( strcmp( slo->controlLabelTypeStr, activeSliderClass_str4 ) == 0 )
    slo->controlLabelType = SLC_K_LABEL;
  else if ( strcmp( slo->controlLabelTypeStr, activeSliderClass_str5 ) == 0 )
    slo->controlLabelType = SLC_K_PV_NAME;
  else
    slo->controlLabelType = SLC_K_LITERAL;

  slo->readLabelName.setRaw( slo->readBufLabelName );
  if ( strcmp( slo->readLabelTypeStr, activeSliderClass_str6 ) == 0 )
    slo->readLabelType = SLC_K_LABEL;
  else if ( strcmp( slo->readLabelTypeStr, activeSliderClass_str7 ) == 0 )
    slo->readLabelType = SLC_K_PV_NAME;
  else
    slo->readLabelType = SLC_K_LITERAL;

  //  slo->formatType = slo->bufFormatType;

  strncpy( slo->displayFormat, slo->bufDisplayFormat, 15 );

  slo->limitsFromDb = slo->bufLimitsFromDb;
  slo->efPrecision = slo->bufEfPrecision;
  slo->efScaleMin = slo->bufEfScaleMin;
  slo->efScaleMax = slo->bufEfScaleMax;

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
  slo->changeCallbackFlag = slo->bufChangeCallbackFlag;
  slo->activateCallbackFlag = slo->bufActivateCallbackFlag;
  slo->deactivateCallbackFlag = slo->bufDeactivateCallbackFlag;
  slo->anyCallbackFlag = slo->changeCallbackFlag ||
   slo->activateCallbackFlag || slo->deactivateCallbackFlag;

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

// for EPICS support
#ifdef __epics__

static void sl_monitor_control_connect_state (
  struct connection_handler_args arg )
{

activeSliderClass *slo = (activeSliderClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

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
  struct connection_handler_args arg )
{

activeSliderClass *slo = (activeSliderClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

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
  struct connection_handler_args arg )
{

activeSliderClass *slo = (activeSliderClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

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
  struct connection_handler_args arg )
{

activeSliderClass *slo = (activeSliderClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    slo->needCtlLabelConnectInit = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();

  }

}

static void sl_controlLabelUpdate (
  struct event_handler_args ast_args )
{

activeSliderClass *slo = (activeSliderClass *) ast_args.usr;

  strncpy( slo->controlLabel, (char *) ast_args.dbr, 39 );

  slo->needCtlLabelInfoInit = 1;
  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

static void sl_monitor_read_label_connect_state (
  struct connection_handler_args arg )
{

activeSliderClass *slo = (activeSliderClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    slo->needReadLabelConnectInit = 1;
    slo->actWin->appCtx->proc->lock();
    slo->actWin->addDefExeNode( slo->aglPtr );
    slo->actWin->appCtx->proc->unlock();

  }

}

static void sl_readLabelUpdate (
  struct event_handler_args ast_args )
{

activeSliderClass *slo = (activeSliderClass *) ast_args.usr;

  strncpy( slo->readLabel, (char *) ast_args.dbr, 39 );

  slo->needReadLabelInfoInit = 1;
  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

static void sl_readInfoUpdate (
  struct event_handler_args ast_args )
{

activeSliderClass *slo = (activeSliderClass *) ast_args.usr;
struct dbr_gr_double readRec = *( (dbr_gr_double *) ast_args.dbr );
int prec;

  if ( slo->limitsFromDb || slo->efPrecision.isNull() )
    prec = readRec.precision;
  else
    prec = slo->precision;

  if ( strcmp( slo->displayFormat, "GFloat" ) == 0 ) {
    sprintf( slo->readFormat, "%%.%-dg", prec );
  }
  else if ( strcmp( slo->displayFormat, "Exponential" ) == 0 ) {
    sprintf( slo->readFormat, "%%.%-de", prec );
  }
  else {
    sprintf( slo->readFormat, "%%.%-df", prec );
  }

  slo->curReadV = readRec.value;

  slo->needReadInfoInit = 1;
  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

static void sl_infoUpdate (
  struct event_handler_args ast_args )
{

activeSliderClass *slo = (activeSliderClass *) ast_args.usr;
struct dbr_gr_double controlRec = *( (dbr_gr_double *) ast_args.dbr );

  if ( slo->limitsFromDb || slo->efScaleMin.isNull() ) {
    slo->scaleMin = controlRec.lower_disp_limit;
  }

  if ( slo->limitsFromDb || slo->efScaleMax.isNull() ) {
    slo->scaleMax = controlRec.upper_disp_limit;
  }

  if ( slo->limitsFromDb || slo->efPrecision.isNull() ) {
    slo->precision = controlRec.precision;
  }

  if ( strcmp( slo->displayFormat, "GFloat" ) == 0 ) {
    sprintf( slo->controlFormat, "%%.%-dg", slo->precision );
  }
  else if ( strcmp( slo->displayFormat, "Exponential" ) == 0 ) {
    sprintf( slo->controlFormat, "%%.%-de", slo->precision );
  }
  else {
    sprintf( slo->controlFormat, "%%.%-df", slo->precision );
  }

  slo->minFv = slo->scaleMin;

  slo->maxFv = slo->scaleMax;

  slo->curControlV = controlRec.value;

  slo->needCtlInfoInit = 1;
  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

static void sl_controlUpdate (
  struct event_handler_args ast_args )
{

activeSliderClass *slo = (activeSliderClass *) ast_args.usr;

  slo->oneControlV = *( (double *) ast_args.dbr ); // an xtimer updates image
  slo->curControlV = slo->oneControlV;

}

static void sl_readUpdate (
  struct event_handler_args ast_args )
{

activeSliderClass *slo = (activeSliderClass *) ast_args.usr;

  slo->curReadV = *( (double *) ast_args.dbr );

  slo->needReadRefresh = 1;
  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

static void sl_savedValueUpdate (
  struct event_handler_args ast_args )
{

activeSliderClass *slo = (activeSliderClass *) ast_args.usr;

  slo->newSavedV = *( (double *) ast_args.dbr );

  slo->needSavedRefresh = 1;
  slo->actWin->appCtx->proc->lock();
  slo->actWin->addDefExeNode( slo->aglPtr );
  slo->actWin->appCtx->proc->unlock();

}

#endif

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
  strcpy( displayFormat, "FFloat" );
  precision = 1;

  frameWidget = NULL;

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

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  controlColor.setColorIndex( actWin->defaultFg1Color, actWin->ci );
  readColor.setColorIndex( actWin->defaultFg2Color, actWin->ci );
  shadeColor.setColorIndex( actWin->defaultOffsetColor, actWin->ci );

  fgColorMode = 0;
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

int index, stat;

  fprintf( f, "%-d %-d %-d\n", SLC_MAJOR_VERSION, SLC_MINOR_VERSION,
   SLC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  fprintf( f, "%-d\n", index );
  index = bgColor.pixelIndex();
  fprintf( f, "%-d\n", index );
  index = shadeColor.pixelIndex();
  fprintf( f, "%-d\n", index );
  index = controlColor.pixelIndex();
  fprintf( f, "%-d\n", index );
  index = readColor.pixelIndex();
  fprintf( f, "%-d\n", index );

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

  return 1;

}

int activeSliderClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, xOfs, stat, index;
int major, minor, release;
unsigned int pixel;
char oneName[activeGraphicClass::MAX_PV_NAME+1];
float val;

  actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( major > 1 ) {

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

  readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvName.setRaw( oneName );

  readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  readPvName.setRaw( oneName );

  readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  savedValuePvName.setRaw( oneName );

  readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlLabelName.setRaw( oneName );

  fscanf( f, "%d\n", &controlLabelType ); actWin->incLine();

  readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  readLabelName.setRaw( oneName );

  fscanf( f, "%d\n", &readLabelType ); actWin->incLine();

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  if ( ( major == 1 ) && ( minor < 4 ) ) {
    fscanf( f, "%d\n", &formatType ); // no longer in use actWin->incLine();
  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

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

  return 1;

}

int activeSliderClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeSliderClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeSliderClass_str10, 31 );

  strncat( title, activeSliderClass_str11, 31 );

  strncpy( bufId, id, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;
  bufFgColor = fgColor.pixelIndex();
  bufBgColor = bgColor.pixelIndex();
  bufShadeColor = shadeColor.pixelIndex();
  bufControlColor = controlColor.pixelIndex();
  bufReadColor = readColor.pixelIndex();
  bufFgColorMode = fgColorMode;
  bufControlColorMode = controlColorMode;
  bufReadColorMode = readColorMode;
  bufIncrement = increment;
  strncpy( bufFontTag, fontTag, 63 );

  bufChangeCallbackFlag = changeCallbackFlag;
  bufActivateCallbackFlag = activateCallbackFlag;
  bufDeactivateCallbackFlag = deactivateCallbackFlag;

  if ( controlPvName.getRaw() )
    strncpy( controlBufPvName, controlPvName.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strncpy( controlBufPvName, "", 39 );

  if ( readPvName.getRaw() )
    strncpy( readBufPvName, readPvName.getRaw(),
    activeGraphicClass::MAX_PV_NAME );
  else
    strncpy( readBufPvName, "", 39 );

  if ( savedValuePvName.getRaw() )
    strncpy( savedValueBufPvName, savedValuePvName.getRaw(),
    activeGraphicClass::MAX_PV_NAME );
  else
    strncpy( savedValueBufPvName, "", 39 );

  if ( controlLabelName.getRaw() )
    strncpy( controlBufLabelName, controlLabelName.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strncpy( controlBufLabelName, "", 39 );

  if ( readLabelName.getRaw() )
    strncpy( readBufLabelName, readLabelName.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strncpy( readBufLabelName, "", 39 );

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

  bufLimitsFromDb = limitsFromDb;
  bufEfPrecision = efPrecision;
  bufEfScaleMin = efScaleMin;
  bufEfScaleMax = efScaleMax;
  strncpy( bufDisplayFormat, displayFormat, 15 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  //ef.addTextField( activeSliderClass_str18, 30, bufId, 31 );
  ef.addTextField( activeSliderClass_str19, 30, &bufX );
  ef.addTextField( activeSliderClass_str20, 30, &bufY );
  ef.addTextField( activeSliderClass_str21, 30, &bufW );
  ef.addTextField( activeSliderClass_str22, 30, &bufH );

  ef.addTextField( activeSliderClass_str36, 30, controlBufPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addTextField( activeSliderClass_str42, 30, readBufPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addTextField( activeSliderClass_str48, 30, savedValueBufPvName,
   activeGraphicClass::MAX_PV_NAME );

  ef.addTextField( activeSliderClass_str37, 30, controlBufLabelName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addOption( activeSliderClass_str38, activeSliderClass_str39,
   controlLabelTypeStr, 15 );

  ef.addTextField( activeSliderClass_str43, 30, readBufLabelName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addOption( activeSliderClass_str44, activeSliderClass_str45,
   readLabelTypeStr, 15 );

  ef.addTextField( activeSliderClass_str28, 30, &bufIncrement );

  ef.addToggle( activeSliderClass_str29, &bufLimitsFromDb );
  ef.addOption( activeSliderClass_str30, activeSliderClass_str35,
   bufDisplayFormat, 15 );
  ef.addTextField( activeSliderClass_str31, 30, &bufEfPrecision );
  ef.addTextField( activeSliderClass_str32, 30, &bufEfScaleMin );
  ef.addTextField( activeSliderClass_str33, 30, &bufEfScaleMax );

  ef.addColorButton( activeSliderClass_str24, actWin->ci, &fgCb, &bufFgColor );
  ef.addToggle( activeSliderClass_str25, &bufFgColorMode );
  ef.addColorButton( activeSliderClass_str26, actWin->ci, &bgCb, &bufBgColor );
  ef.addColorButton( activeSliderClass_str27, actWin->ci, &shadeCb, &bufShadeColor );
  ef.addColorButton( activeSliderClass_str40, actWin->ci, &controlCb,
   &bufControlColor );
  ef.addToggle( activeSliderClass_str41, &bufControlColorMode );
  ef.addColorButton( activeSliderClass_str46, actWin->ci, &readCb,
   &bufReadColor );
  ef.addToggle( activeSliderClass_str47, &bufReadColorMode );

  ef.addFontMenu( activeSliderClass_str23, actWin->fi, &fm, fontTag );

  //ef.addToggle( activeSliderClass_str49, &bufActivateCallbackFlag );
  //ef.addToggle( activeSliderClass_str50, &bufDeactivateCallbackFlag );
  //ef.addToggle( activeSliderClass_str51, &bufChangeCallbackFlag );

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

  if ( !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( shadeColor.getColor() );
  actWin->executeGc.setArcModePieSlice();

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

  actWin->executeGc.setArcModePieSlice();

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

    actWin->executeGc.saveBg();
    actWin->executeGc.setBG( bgColor.getColor() );

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

  if ( !activeMode || !init ) return 1;

  if ( fs && controlExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( controlColor.getColor() );

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

  if ( !activeMode || !init ) return 1;

  if ( fs && readExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( bgColor.getColor() );

    actWin->executeGc.saveBg();
    actWin->executeGc.setBG( bgColor.getColor() );

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

  if ( !activeMode || !init ) return 1;

  if ( fs && readExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( readColor.getColor() );

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

  printf( "zzz\n" );

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

  actWin->executeGc.setArcModePieSlice();

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

      actWin->executeGc.setFG( readColor.getColor() );

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
int stat, deltaX, xOfs, newX;
double fvalue;
char title[32], *ptr;
int tX, tY, x0, y0, x1, y1, incX0, incY0, incX1, incY1;

  slo = (activeSliderClass *) client;

  if ( !slo->active ) return;

  if ( e->type == EnterNotify ) {
    if ( !ca_write_access( slo->controlPvId ) ) {
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

  strncat( title, activeSliderClass_str55, 31 );

  if ( e->type == Expose ) {

    slo->bufInvalidate();
    stat = slo->drawActive();
//      stat = slo->drawActiveControlText();
//      stat = slo->drawActivePointers();

  }

  if ( !ca_write_access( slo->controlPvId ) ) return;

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
          stat = ca_put( DBR_DOUBLE, slo->savedValuePvId, &slo->savedV );
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

// for EPICS support

        if ( slo->controlExists ) {
#ifdef __epics__
          stat = ca_put( DBR_DOUBLE, slo->controlPvId, &fvalue );
          if ( stat != ECA_NORMAL ) printf( activeSliderClass_str56 );
#endif
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

        slo->incrementTimerActive = 1;
        slo->incrementTimerValue = 101;

        slo->incrementTimer = XtAppAddTimeOut(
         slo->actWin->appCtx->appContext(), 300,
         slc_increment, (void *) slo );

      }
      else if ( ( be->x < slo->controlX + slo->controlAreaH/2 ) &&
              ( be->y > slo->valueAreaH ) ) {

        /* auto dec */

        slo->incrementTimerActive = 1;
        slo->incrementTimerValue = 101;

        slo->incrementTimer = XtAppAddTimeOut(
         slo->actWin->appCtx->appContext(), 300,
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

          slo->kp.create( slo->actWin->top, be->x+slo->actWin->x+slo->x,
           be->y+slo->actWin->y+slo->y+y1-y0, "", &slo->kpCtlDouble,
           (void *) slo,
           (XtCallbackProc) sloSetCtlKpDoubleValue,
           (XtCallbackProc) sloCancelKp );

        }
        else if ( ( be->x > incX0 ) &&
             ( be->x < incX1 ) &&
             ( be->y > incY0 ) &&
             ( be->y < incY1 ) ) {

          slo->kp.create( slo->actWin->top, be->x+slo->actWin->x+slo->x,
           be->y+slo->actWin->y+slo->y+y1-y0, "", &slo->kpIncDouble,
           (void *) slo,
           (XtCallbackProc) sloSetIncKpDoubleValue,
           (XtCallbackProc) sloCancelKp );

        }
	else {

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

          slo->ef.addTextField( activeSliderClass_str57, 14,
           &slo->bufControlV );
          slo->ef.addTextField( activeSliderClass_str58, 14,
           &slo->bufIncrement );
          slo->ef.finished( slc_value_ok, slc_value_apply, slc_value_cancel,
           slo );
          slo->ef.popup();

	}

      }

      break;

//========== B1 Press ========================================

//========== B2 Press ========================================

    case Button2:

      if ( !( be->state & ShiftMask ) ) {
        stat = slo->startDrag( w, e );
      }

      break;

//========== B2 Press ========================================

//========== B3 Press ========================================

    case Button3:

      if ( ( be->x > slo->controlX - slo->controlAreaH/2 ) &&
           ( be->x < slo->controlX + slo->controlAreaH/2 ) &&
           ( be->y > slo->valueAreaH ) &&
           ( be->y < slo->valueAreaH + slo->controlAreaH ) ) {

        slo->xRef = be->x;

        slo->controlState = SLC_STATE_MOVING;

      }

      break;

//========== B3 Press ========================================

    }

  }
  if ( e->type == ButtonRelease ) {

//========== Any B Release ========================================

    be = (XButtonEvent *) e;

    switch ( be->button ) {

    case Button2:

      if ( be->state & ShiftMask ) {
        stat = slo->selectDragValue( slo->x+be->x, slo->y+be->y );
      }

      break;

    default:

      slo->controlState = SLC_STATE_IDLE;
      slo->incrementTimerActive = 0;

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

// for EPICS support
        if ( slo->controlExists ) {
#ifdef __epics__
          stat = ca_put( DBR_DOUBLE, slo->controlPvId, &fvalue );
          if ( stat != ECA_NORMAL ) printf( activeSliderClass_str59 );
#endif
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

// for EPICS support
        if ( slo->controlExists ) {
#ifdef __epics__
        stat = ca_put( DBR_DOUBLE, slo->controlPvId, &fvalue );
        if ( stat != ECA_NORMAL ) printf( activeSliderClass_str60 );
#endif
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
      printf( activeSliderClass_str61 );
      return 0;
    }

    sliderWidget = XtVaCreateManagedWidget( "", xmDrawingAreaWidgetClass,
     frameWidget,
     XmNwidth, w-4,
     XmNheight, h-4,
     XmNbackground, bgColor.pixelColor(),
     NULL );

    if ( !sliderWidget ) {
      printf( activeSliderClass_str62 );
      return 0;
    }

    XtAddEventHandler( sliderWidget,
     ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|ExposureMask|
     EnterWindowMask|LeaveWindowMask, False,
     sliderEventHandler, (XtPointer) this );

    XtMapWidget( frameWidget );

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
    oldControlV = 0;
    updateControlTimerActive = 0;
    controlAdjusted = 0;
    incrementTimerActive = 0;
    opComplete = 0;

#ifdef __epics__
    controlEventId = readEventId = controlLabelEventId =
     readLabelEventId = savedEventId = 0;
#endif

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
      strncpy( controlLabel, controlPvName.getExpanded(),
       activeGraphicClass::MAX_PV_NAME );
    }
    else {
      strncpy( controlLabel, controlLabelName.getExpanded(),
       activeGraphicClass::MAX_PV_NAME );
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
       activeGraphicClass::MAX_PV_NAME );
    }
    else {
      strncpy( readLabel, readLabelName.getExpanded(),
       activeGraphicClass::MAX_PV_NAME );
    }

    break;

  case 2:

    if ( !opComplete ) {

      opStat = 1;

      if ( controlExists ) {
#ifdef __epics__
        stat = ca_search_and_connect( controlPvName.getExpanded(),
         &controlPvId, sl_monitor_control_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeSliderClass_str63 );
          opStat = 0;
        }
#endif
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
          strncat( callbackName, activeSliderClass_str64, 63 );
          changeCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          strncat( callbackName, activeSliderClass_str65, 63 );
          activateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( deactivateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          strncat( callbackName, activeSliderClass_str66, 63 );
          deactivateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallback ) {
          (*activateCallback)( this );
        }


      }

      if ( controlLabelExists && ( controlLabelType == SLC_K_LABEL )  ) {
#ifdef __epics__
        stat = ca_search_and_connect( controlLabelName.getExpanded(),
         &controlLabelPvId, sl_monitor_control_label_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeSliderClass_str67 );
          opStat = 0;
        }
#endif
      }

      if ( readExists ) {
#ifdef __epics__
        stat = ca_search_and_connect( readPvName.getExpanded(), &readPvId,
         sl_monitor_read_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeSliderClass_str68 );
          opStat = 0;
        }
#endif
      }

      if ( readLabelExists && ( readLabelType == SLC_K_LABEL )  ) {
#ifdef __epics__
        stat = ca_search_and_connect( readLabelName.getExpanded(),
         &readLabelPvId, sl_monitor_read_label_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeSliderClass_str69 );
          opStat = 0;
        }
#endif
      }

      if ( savedValueExists ) {
#ifdef __epics__
        stat = ca_search_and_connect( savedValuePvName.getExpanded(),
         &savedValuePvId, sl_monitor_saved_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeSliderClass_str70 );
          opStat = 0;
        }
#endif
      }

      if ( opStat & 1 ) {

        updateControlTimerActive = 1;
        updateControlTimerValue = 100;
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

    updateControlTimerActive = 0;
    XtRemoveTimeOut( updateControlTimer );

    XtRemoveEventHandler( sliderWidget,
     ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|ExposureMask|
     EnterWindowMask|LeaveWindowMask, False,
     sliderEventHandler, (XtPointer) this );

// for EPICS support
#ifdef __epics__

    if ( controlExists ) {
      stat = ca_clear_channel( controlPvId );
      if ( stat != ECA_NORMAL )
        printf( activeSliderClass_str71 );
    }
  
    if ( controlLabelExists && ( controlLabelType == SLC_K_LABEL )  ) {
      stat = ca_clear_channel( controlLabelPvId );
      if ( stat != ECA_NORMAL )
        printf( activeSliderClass_str72 );
    }

    if ( readExists ) {
      stat = ca_clear_channel( readPvId );
      if ( stat != ECA_NORMAL )
        printf( activeSliderClass_str73 );
    }

    if ( readLabelExists && ( readLabelType == SLC_K_LABEL )  ) {
      stat = ca_clear_channel( readLabelPvId );
      if ( stat != ECA_NORMAL )
        printf( activeSliderClass_str74 );
    }

    if ( savedValueExists ) {
      stat = ca_clear_channel( savedValuePvId );
      if ( stat != ECA_NORMAL )
        printf( activeSliderClass_str75 );
    }

#endif

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

int activeSliderClass::createWidgets ( void ) {

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
 nrli, ne, nd;
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

#ifdef __epics__

  if ( ncc ) {

    controlPvConnected = 1;

    stat = ca_get_callback( DBR_GR_DOUBLE, controlPvId,
     sl_infoUpdate, (void *) this );
    if ( stat != ECA_NORMAL )
      printf( activeSliderClass_str78 );

  }

#endif

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

    if ( controlExists && !controlEventId ) {
#ifdef __epics__
      stat = ca_add_masked_array_event( DBR_DOUBLE, 1, controlPvId,
       sl_controlUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
       &controlEventId, DBE_VALUE );
      if ( stat != ECA_NORMAL )
        printf( activeSliderClass_str79 );
#endif
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

#ifdef __epics__

  if ( nrc ) {

    readPvConnected = 1;

    stat = ca_get_callback( DBR_GR_DOUBLE, readPvId,
     sl_readInfoUpdate, (void *) this );
    if ( stat != ECA_NORMAL )
      printf( activeSliderClass_str80 );

  }

//----------------------------------------------------------------------------

  if ( nri ) {

    readV = rv;

    sprintf( readValue, readFormat, readV );

    if ( !readEventId ) {

      stat = ca_add_masked_array_event( DBR_DOUBLE, 1, readPvId,
       sl_readUpdate, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
       &readEventId, DBE_VALUE );
      if ( stat != ECA_NORMAL )
        printf( activeSliderClass_str81 );

    }

  }

//----------------------------------------------------------------------------

  if ( nsc ) {

    if ( !savedEventId ) {

      stat = ca_add_masked_array_event( DBR_DOUBLE, 1, savedValuePvId,
       sl_savedValueUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &savedEventId, DBE_VALUE );
      if ( stat != ECA_NORMAL )
        printf( activeSliderClass_str82 );

    }

    savedValuePvConnected = 1;

  }

#endif

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

#ifdef __epics__

  if ( nclc ) {

    stat = ca_get_callback( DBR_STRING, controlLabelPvId,
     sl_controlLabelUpdate, (void *) this );
    if ( stat != ECA_NORMAL )
      printf( activeSliderClass_str83 );

  }

#endif

//----------------------------------------------------------------------------

  if ( ncli ) {

    if ( active ) {
      stat = eraseActive();
      stat = drawActive();
    }

    bufInvalidate();

  }

//----------------------------------------------------------------------------

#ifdef __epics__

  if ( nrlc ) {

    stat = ca_get_callback( DBR_STRING, readLabelPvId,
     sl_readLabelUpdate, (void *) this );
    if ( stat != ECA_NORMAL )
      printf( activeSliderClass_str84 );

  }

#endif

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

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeSliderClass::nextDragName ( void ) {

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
