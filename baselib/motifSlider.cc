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

#define __motifSlider_cc 1

#include "motifSlider.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) client;

  if ( !mslo->init ) {
    mslo->needToDrawUnconnected = 1;
    mslo->needDraw = 1;
    mslo->actWin->addDefExeNode( mslo->aglPtr );
  }

  mslo->unconnectedTimer = 0;

}
static void msloValueChangeCB (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int v, stat;
activeMotifSliderClass *mslo;
double fvalue;

  // this only gets called when the left mouse button is clicked
  // on the scrollbar background or indicator

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  if ( mslo->increment == 0 ) return;

  if ( mslo->dragIndicator ) {
    mslo->dragIndicator = 0;
    return;
  }

  XmScaleGetValue( w, &v );

  if ( mslo->prevScaleV == -1 ) mslo->prevScaleV = v;

  if ( v > mslo->prevScaleV ) {

    if ( mslo->positive ) {
      fvalue = mslo->controlV + mslo->increment;
      if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
      if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
    }
    else {
      fvalue = mslo->controlV - mslo->increment;
      if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
      if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
    }

  }
  else {

    if ( mslo->positive ) {
      fvalue = mslo->controlV - mslo->increment;
      if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
      if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
    }
    else {
      fvalue = mslo->controlV + mslo->increment;
      if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
      if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
    }

  }

  mslo->prevScaleV = v;

  mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
   mslo->factor + 0.5 );

  XmScaleSetValue( w, mslo->controlX );

  mslo->oldControlV = mslo->oneControlV;

  mslo->eraseActiveControlText();

  mslo->actWin->appCtx->proc->lock();
  mslo->controlV = mslo->oneControlV = mslo->curControlV;
  mslo->actWin->appCtx->proc->unlock();

  mslo->controlV = fvalue;

  sprintf( mslo->controlValue, mslo->controlFormat, mslo->controlV );

  stat = mslo->drawActiveControlText();

  if ( mslo->controlExists ) {
    if ( mslo->controlPvId ) {
      stat = mslo->controlPvId->put( fvalue );
      //if ( stat != ECA_NORMAL ) printf( activeMotifSliderClass_str59 );
    }
  }

  mslo->controlAdjusted = 1;

}

static void msloIndicatorDragCB (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int v, stat;
activeMotifSliderClass *mslo;
double fvalue;

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  mslo->dragIndicator = 1;

  XmScaleGetValue( w, &v );

  fvalue = mslo->factor * (double) v + mslo->minFv;
  if ( mslo->positive ) {
    if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
    if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
  }
  else {
    if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
    if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
  }

  mslo->prevScaleV = v;

  mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
   mslo->factor + 0.5 );

  XmScaleSetValue( w, mslo->controlX );

  mslo->oldControlV = mslo->oneControlV;

  mslo->eraseActiveControlText();

  mslo->actWin->appCtx->proc->lock();
  mslo->controlV = mslo->oneControlV = mslo->curControlV;
  mslo->actWin->appCtx->proc->unlock();

  mslo->controlV = fvalue;

  sprintf( mslo->controlValue, mslo->controlFormat, mslo->controlV );

  stat = mslo->drawActiveControlText();

  if ( mslo->controlExists ) {
    if ( mslo->controlPvId ) {
      stat = mslo->controlPvId->put( fvalue );
      //if ( stat != ECA_NORMAL ) printf( activeMotifSliderClass_str59 );
    }
  }

  mslo->controlAdjusted = 1;

}

static void calcIncRange (
  double minV,
  double maxV,
  char *strVal,
  double *incArray
) {

double max, lmin, lmax;
int i, j, start, end;
char tmpStr[255+1];

    if ( minV == 0 ) {
      lmin = 0;
    }
    else {
      lmin = rint( log10( fabs( minV ) ) );
    }

    if ( maxV == 0 ) {
      lmax = 0;
    }
    else {
      lmax = rint( log10( fabs( maxV ) ) );
    }

    max = 0;
    if ( lmin != 0 ) max = lmin;
    if ( lmax != 0 ) max = lmax;

    if ( lmin != 0 ) {
      if ( lmin > max ) max = lmin;
    }

    if ( lmax != 0 ) {
      if ( lmax > max ) max = lmax;
    }

    start = (int) max - 1;
    end = start - 5;
    strcpy( strVal, "---|" );
    incArray[0] = 0;
    for ( i=1,j=start; i<6; i++,j-- ) {
      sprintf( tmpStr, "10^%-d|", j );
      strncat( strVal, tmpStr, 255 );
      incArray[i] = pow( 10, (double) j );
    }
    sprintf( tmpStr, "10^%-d", end );
    strncat( strVal, tmpStr, 255 );
    incArray[6] = pow( 10, (double) j );

}

static void changeParams (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeMotifSliderClass *mslo;
XButtonEvent *be;
char title[32], *ptr;
char strVal[255+1];

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  if ( !mslo->ef.formIsPoppedUp() ) {

    be = (XButtonEvent *) e;

    ptr = mslo->actWin->obj.getNameFromClass( "activeMotifSliderClass" );
    if ( ptr )
      strncpy( title, ptr, 31 );
    else
      strncpy( title, activeMotifSliderClass_str54, 31 );

    strncat( title, activeMotifSliderClass_str55, 31 );

    mslo->bufIncrement = mslo->increment;
    mslo->bufControlV = mslo->controlV;
    mslo->valueFormX = mslo->actWin->x + mslo->x  + be->x;
    mslo->valueFormY = mslo->actWin->y + mslo->y + be->y;
    mslo->valueFormW = 0;
    mslo->valueFormH = 0;
    mslo->valueFormMaxH = 600;

    mslo->ef.create( mslo->actWin->top,
     mslo->actWin->appCtx->ci.getColorMap(),
     &mslo->valueFormX, &mslo->valueFormY,
     &mslo->valueFormW, &mslo->valueFormH, &mslo->valueFormMaxH,
     title, NULL, NULL, NULL );

    mslo->ef.addTextField( activeMotifSliderClass_str57, 20,
     &mslo->bufControlV );

    mslo->ef.addTextField( activeMotifSliderClass_str58, 20,
     &mslo->bufIncrement );

    calcIncRange( mslo->minFv, mslo->maxFv, strVal, mslo->incArray );
    mslo->incIndex = 0;
    mslo->ef.addOption( activeMotifSliderClass_str58, strVal,
     &mslo->incIndex );

    mslo->ef.finished( mslc_value_ok, mslc_value_apply, mslc_value_cancel,
     mslo );

    mslo->ef.popup();

  }

}

static void dummy (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

}

static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeMotifSliderClass *mslo;
int stat;

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  stat = mslo->startDrag( w, e );

}

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeMotifSliderClass *mslo;
int stat;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  stat = mslo->selectDragValue( mslo->x + be->x, mslo->y + be->y );

}

static void mslc_updateControl (
  XtPointer client,
  XtIntervalId *id )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) client;
int stat;
double fv;

  if ( mslo->updateControlTimerActive ) {
    mslo->updateControlTimer = appAddTimeOut(
     mslo->actWin->appCtx->appContext(),
     mslo->updateControlTimerValue, mslc_updateControl, client );
  }
  else {
    return;
  }

  if ( !mslo->active || !mslo->init ) return;

  if ( mslo->controlAdjusted ) {
    mslo->controlAdjusted = 0;
    return;
  }

  if ( mslo->prevScaleV != -1 ) {
    if ( mslo->oldControlV == mslo->oneControlV ) return;
  }

  mslo->oldControlV = mslo->oneControlV;

  mslo->eraseActiveControlText();

  mslo->actWin->appCtx->proc->lock();
  mslo->controlV = mslo->oneControlV = mslo->curControlV;
  mslo->actWin->appCtx->proc->unlock();

  if ( mslo->positive ) {

    if ( mslo->controlV < mslo->minFv )
      fv = mslo->minFv;
    else if ( mslo->controlV > mslo->maxFv )
      fv = mslo->maxFv;
    else
      fv = mslo->controlV;

  }
  else {

    if ( mslo->controlV > mslo->minFv )
      fv = mslo->minFv;
    else if ( mslo->controlV < mslo->maxFv )
      fv = mslo->maxFv;
    else
      fv = mslo->controlV;

  }

  mslo->controlX = (int) ( ( fv - mslo->minFv ) /
   mslo->factor + 0.5 );

  mslo->savedX = (int) ( ( mslo->savedV - mslo->minFv ) /
   mslo->factor + 0.5 );

  sprintf( mslo->controlValue, mslo->controlFormat, mslo->controlV );

  stat = mslo->drawActiveControlText();

  XmScaleSetValue( mslo->scaleWidget, mslo->controlX );
  mslo->prevScaleV = mslo->controlX;

}

static void mslc_value_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int stat;
double fvalue;
activeMotifSliderClass *mslo = (activeMotifSliderClass *) client;

  fvalue = (double) mslo->bufControlV;

  if ( mslo->positive ) {
    if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
    if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
  }
  else {
    if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
    if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
  }

  mslo->controlV = fvalue;

  mslo->increment = mslo->bufIncrement;

  if ( ( mslo->incIndex > 0 ) && ( mslo->incIndex < 7 ) ) {
    mslo->increment = mslo->incArray[mslo->incIndex];
  }

  sprintf( mslo->incString, mslo->controlFormat, mslo->increment );

  mslo->actWin->appCtx->proc->lock();
  mslo->curControlV = mslo->controlV;
  mslo->actWin->appCtx->proc->unlock();

// for EPICS support

  if ( mslo->controlExists ) {
    if ( mslo->controlPvId ) {
      stat = mslo->controlPvId->put( fvalue );
      //if ( stat != ECA_NORMAL ) printf( activeMotifSliderClass_str3 );
      mslo->actWin->appCtx->proc->lock();
      mslo->actWin->addDefExeNode( mslo->aglPtr );
      mslo->actWin->appCtx->proc->unlock();
    }
  }

  mslo->controlAdjusted = 1;

  mslo->needErase = 1;
  mslo->needDraw = 1;

}

static void mslc_value_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) client;

  mslc_value_apply ( w, client, call );
  mslo->ef.popdown();

}

static void mslc_value_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) client;

  mslo->ef.popdown();

}

static void mslc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) client;

  mslo->actWin->setChanged();

  mslo->eraseSelectBoxCorners();
  mslo->erase();

  mslo->fgColor.setColorIndex( mslo->bufFgColor, mslo->actWin->ci );
  mslo->bgColor.setColorIndex( mslo->bufBgColor, mslo->actWin->ci );

  mslo->bgColorMode = mslo->bufBgColorMode;
  if ( mslo->bgColorMode == MSLC_K_COLORMODE_ALARM ) {
    mslo->bgColor.setAlarmSensitive();
  }
  else {
    mslo->bgColor.setAlarmInsensitive();
  }

  mslo->shadeColor = mslo->bufShadeColor;
  mslo->topColor = mslo->bufTopColor;
  mslo->botColor = mslo->bufBotColor;

  mslo->increment = mslo->bufIncrement;
  sprintf( mslo->incString, mslo->controlFormat, mslo->increment );

  mslo->controlPvName.setRaw( mslo->controlBufPvName );

  mslo->controlLabelName.setRaw( mslo->controlBufLabelName );

  mslo->controlLabelType = mslo->bufControlLabelType;

  mslo->formatType = mslo->bufFormatType;

  mslo->limitsFromDb = mslo->bufLimitsFromDb;
  mslo->efPrecision = mslo->bufEfPrecision;
  mslo->efScaleMin = mslo->bufEfScaleMin;
  mslo->efScaleMax = mslo->bufEfScaleMax;

  mslo->minFv = mslo->scaleMin = mslo->efScaleMin.value();
  mslo->maxFv = mslo->scaleMax = mslo->efScaleMax.value();

  if ( mslo->efPrecision.isNull() )
    mslo->precision = 1;
  else
    mslo->precision = mslo->efPrecision.value();

  strncpy( mslo->fontTag, mslo->fm.currentFontTag(), 63 );
  mslo->actWin->fi->loadFontTag( mslo->fontTag );
  mslo->fs = mslo->actWin->fi->getXFontStruct( mslo->fontTag );

  mslo->showLimits = mslo->bufShowLimits;
  mslo->showLabel = mslo->bufShowLabel;
  mslo->showValue = mslo->bufShowValue;

  mslo->orientation = mslo->bufOrientation;

  mslo->x = mslo->bufX;
  mslo->sboxX = mslo->bufX;

  mslo->y = mslo->bufY;
  mslo->sboxY = mslo->bufY;

  if ( mslo->bufW < mslo->minW ) mslo->bufW = mslo->minW;

  mslo->w = mslo->bufW;
  mslo->sboxW = mslo->bufW;

  if ( mslo->bufH < mslo->minH ) mslo->bufH = mslo->minH;

  mslo->h = mslo->bufH;
  mslo->sboxH = mslo->bufH;

  mslo->updateDimensions();

  // one more iteration
  if ( mslo->h < mslo->minH ) mslo->h = mslo->minH;
  mslo->sboxH = mslo->h;

  mslo->erase();
  mslo->draw();

}

static void mslc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) client;

  mslc_edit_update ( w, client, call );
  mslo->refresh( mslo );

}

static void mslc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) client;

  mslc_edit_update ( w, client, call );
  mslo->ef.popdown();
  mslo->operationComplete();

}

static void mslc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) client;

  mslo->ef.popdown();
  mslo->operationCancel();

}

static void mslc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) client;

  mslo->ef.popdown();
  mslo->operationCancel();
  mslo->erase();
  mslo->deleteRequest = 1;
  mslo->drawAll();

}

// for EPICS support

void activeMotifSliderClass::monitorControlConnectState (
  ProcessVariable *pv,
  void *userarg )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) userarg;

  if ( pv->is_valid() ) {

    if ( mslo->limitsFromDb || mslo->efScaleMin.isNull() ) {
      mslo->scaleMin = pv->get_lower_disp_limit();
    }

    if ( mslo->limitsFromDb || mslo->efScaleMax.isNull() ) {
      mslo->scaleMax = pv->get_upper_disp_limit();
    }

    if ( mslo->limitsFromDb || mslo->efPrecision.isNull() ) {
      mslo->precision = pv->get_precision();
    }

    if ( mslo->formatType == MSLC_K_FORMAT_FLOAT ) {
      sprintf( mslo->controlFormat, "%%.%-df", mslo->precision );
    }
    else if ( mslo->formatType == MSLC_K_FORMAT_EXPONENTIAL ) {
      sprintf( mslo->controlFormat, "%%.%-de", mslo->precision );
    }
    else {
      sprintf( mslo->controlFormat, "%%.%-dg", mslo->precision );
    }

    mslo->minFv = mslo->scaleMin;

    mslo->maxFv = mslo->scaleMax;

    mslo->curControlV = pv->get_double();

    mslo->needCtlConnectInit = 1;
    mslo->needCtlInfoInit = 1;

  }
  else {

    mslo->controlPvConnected = 0;
    mslo->active = 0;
    mslo->fgColor.setDisconnected();
    mslo->bgColor.setDisconnected();
    mslo->bufInvalidate();
    mslo->needErase = 1;
    mslo->needDraw = 1;

  }

  mslo->actWin->appCtx->proc->lock();
  mslo->actWin->addDefExeNode( mslo->aglPtr );
  mslo->actWin->appCtx->proc->unlock();

}

void activeMotifSliderClass::monitorControlLabelConnectState (
  ProcessVariable *pv,
  void *userarg )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) userarg;

  if ( pv->is_valid() ) {

    mslo->needCtlLabelConnectInit = 1;
    mslo->actWin->appCtx->proc->lock();
    mslo->actWin->addDefExeNode( mslo->aglPtr );
    mslo->actWin->appCtx->proc->unlock();

  }

}

void activeMotifSliderClass::controlLabelUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) userarg;

  pv->get_string( mslo->controlLabel, PV_Factory::MAX_PV_NAME );

  mslo->needCtlLabelInfoInit = 1;
  mslo->actWin->appCtx->proc->lock();
  mslo->actWin->addDefExeNode( mslo->aglPtr );
  mslo->actWin->appCtx->proc->unlock();

}

void activeMotifSliderClass::controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMotifSliderClass *mslo = (activeMotifSliderClass *) userarg;
int st, sev;

  mslo->curControlV = mslo->oneControlV = pv->get_double();

  st = pv->get_status();
  sev = pv->get_severity();
  if ( ( st != mslo->oldStat ) || ( sev != mslo->oldSev ) ) {
    mslo->oldStat = st;
    mslo->oldSev = sev;
    mslo->bgColor.setStatus( st, sev );
    mslo->bufInvalidate();
    mslo->needErase = 1;
    mslo->needDraw = 1;
    mslo->actWin->appCtx->proc->lock();
    mslo->actWin->addDefExeNode( mslo->aglPtr );
    mslo->actWin->appCtx->proc->unlock();
  }

  // xtimer updates image

}

activeMotifSliderClass::activeMotifSliderClass ( void ) {

  name = new char[strlen("activeMotifSliderClass")+1];
  strcpy( name, "activeMotifSliderClass" );
  deleteRequest = 0;
  selected = 0;
  positive = 1;
  strcpy( id, "" );

  scaleMin = 0;
  scaleMax = 10;

  limitsFromDb = 1;
  efScaleMin.setNull(1);
  efScaleMax.setNull(1);
  efPrecision.setNull(1);
  formatType = MSLC_K_FORMAT_FLOAT;
  precision = 1;

  controlLabelType = MSLC_K_PV_NAME;

  showLimits = 0;
  showLabel = 0;
  showValue = 0;

  orientation = MSLC_K_HORIZONTAL;

  limitsH = 0;
  labelH = 0;
  midVertScaleY = 0;

  frameWidget = NULL;
  scaleWidget = NULL;

  unconnectedTimer = 0;

}

// copy constructor
activeMotifSliderClass::activeMotifSliderClass
 ( const activeMotifSliderClass *source ) {

activeGraphicClass *mslo = (activeGraphicClass *) this;

  mslo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMotifSliderClass")+1];
  strcpy( name, "activeMotifSliderClass" );

  deleteRequest = 0;

  bgColor.copy( source->bgColor );
  fgColor.copy( source->fgColor );

  bgColorMode = source->bgColorMode;

  shadeColor = source->shadeColor;
  topColor = source->topColor;
  botColor = source->botColor;

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  shadeCb = source->shadeCb;
  topCb = source->topCb;
  botCb = source->botCb;

  controlPvName.copy( source->controlPvName );
  controlLabelName.copy( source->controlLabelName );

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  strcpy( controlValue, "0.0" );
  strcpy( controlLabel, "" );

  controlLabelType = source->controlLabelType;

  increment = source->increment;

  positive = source->positive;

  limitsFromDb = source->limitsFromDb;
  scaleMin = source->scaleMin;
  scaleMax = source->scaleMax;
  precision = source->precision;

  efScaleMin = source->efScaleMin;
  efScaleMax = source->efScaleMax;
  efPrecision = source->efPrecision;

  formatType = source->formatType ;

  showLimits = source->showLimits;
  showLabel = source->showLabel;
  showValue = source->showValue;

  orientation = source->orientation;

  frameWidget = NULL;
  scaleWidget = NULL;

  unconnectedTimer = 0;

}

activeMotifSliderClass::~activeMotifSliderClass ( void ) {

  if ( name ) delete name;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

int activeMotifSliderClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = aw_obj;
  xOrigin = 0;
  yOrigin = 0;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  increment = 0.0;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  bgColorMode = 0;

  shadeColor =actWin->defaultOffsetColor;
  topColor = actWin->defaultTopShadowColor;
  botColor = actWin->defaultBotShadowColor;

  strcpy( controlValue, "0.0" );
  strcpy( controlLabel, "" );

  controlLabelType = MSLC_K_PV_NAME;

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

int activeMotifSliderClass::save (
  FILE *f )
{

int index, stat;

  fprintf( f, "%-d %-d %-d\n", MSLC_MAJOR_VERSION, MSLC_MINOR_VERSION,
   MSLC_RELEASE );

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

  actWin->ci->writeColorIndex( f, shadeColor );
  //fprintf( f, "%-d\n", shadeColor );

  actWin->ci->writeColorIndex( f, topColor );
  //fprintf( f, "%-d\n", topColor );

  actWin->ci->writeColorIndex( f, botColor );
  //fprintf( f, "%-d\n", botColor );

  fprintf( f, "%-g\n", increment );

  if ( controlPvName.getRaw() )
    writeStringToFile( f, controlPvName.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( controlLabelName.getRaw() )
    writeStringToFile( f, controlLabelName.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", controlLabelType );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", bgColorMode );

  fprintf( f, "%-d\n", limitsFromDb );
  stat = efPrecision.write( f );
  stat = efScaleMin.write( f );
  stat = efScaleMax.write( f );

  fprintf( f, "%-d\n", formatType );

  fprintf( f, "%-d\n", showLimits );
  fprintf( f, "%-d\n", showLabel );
  fprintf( f, "%-d\n", showValue );

  fprintf( f, "%-d\n", orientation );

  return 1;

}

int activeMotifSliderClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int stat, index;
int major, minor, release;
char oneName[PV_Factory::MAX_PV_NAME+1];
float val;

  actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > MSLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 0 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    shadeColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    topColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    botColor = index;

  }
  else {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &shadeColor ); actWin->incLine();

    fscanf( f, "%d\n", &topColor ); actWin->incLine();

    fscanf( f, "%d\n", &botColor ); actWin->incLine();

  }

  fscanf( f, "%g\n", &val ); actWin->incLine();
  increment = (double) val;

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvName.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlLabelName.setRaw( oneName );

  fscanf( f, "%d\n", &controlLabelType ); actWin->incLine();

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

  if ( bgColorMode == MSLC_K_COLORMODE_ALARM ) {
    bgColor.setAlarmSensitive();
  }
  else {
    bgColor.setAlarmInsensitive();
  }

  fscanf( f, "%d\n", &limitsFromDb ); actWin->incLine();

  stat = efPrecision.read( f ); actWin->incLine();

  efScaleMin.read( f ); actWin->incLine();

  efScaleMax.read( f ); actWin->incLine();

  fscanf( f, "%d\n", &formatType ); actWin->incLine();

  fscanf( f, "%d\n", &showLimits ); actWin->incLine();
  fscanf( f, "%d\n", &showLabel ); actWin->incLine();
  fscanf( f, "%d\n", &showValue ); actWin->incLine();

  fscanf( f, "%d\n", &orientation ); actWin->incLine();

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

  controlX = 0;

  strcpy( controlValue, "0.0" );
  strcpy( controlLabel, "" );

  curControlV = oneControlV = controlV = 0.0;

  return 1;

}

int activeMotifSliderClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeMotifSliderClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMotifSliderClass_str10, 31 );

  strncat( title, activeMotifSliderClass_str11, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;
  bufFgColor = fgColor.pixelIndex();
  bufBgColor = bgColor.pixelIndex();
  bufBgColorMode = bgColorMode;
  bufShadeColor = shadeColor;
  bufTopColor = topColor;
  bufBotColor = botColor;
  bufIncrement = increment;
  strncpy( bufFontTag, fontTag, 63 );

  bufShowLimits = showLimits;
  bufShowLabel = showLabel;
  bufShowValue = showValue;

  bufOrientation = orientation;

  if ( controlPvName.getRaw() )
    strncpy( controlBufPvName, controlPvName.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( controlBufPvName, "" );

  if ( controlLabelName.getRaw() )
    strncpy( controlBufLabelName, controlLabelName.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( controlBufLabelName, "" );

  bufControlLabelType = controlLabelType;

  bufLimitsFromDb = limitsFromDb;
  bufEfPrecision = efPrecision;
  bufEfScaleMin = efScaleMin;
  bufEfScaleMax = efScaleMax;

  bufFormatType = formatType;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeMotifSliderClass_str19, 35, &bufX );
  ef.addTextField( activeMotifSliderClass_str20, 35, &bufY );
  ef.addTextField( activeMotifSliderClass_str21, 35, &bufW );
  ef.addTextField( activeMotifSliderClass_str22, 35, &bufH );

  ef.addTextField( activeMotifSliderClass_str36, 35, controlBufPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeMotifSliderClass_str37, 35, controlBufLabelName,
   PV_Factory::MAX_PV_NAME );
  ef.addOption( activeMotifSliderClass_str38, activeMotifSliderClass_str39,
   &bufControlLabelType );

  ef.addToggle( activeMotifSliderClass_str86, &bufShowLimits );
  ef.addToggle( activeMotifSliderClass_str87, &bufShowLabel );
  ef.addToggle( activeMotifSliderClass_str88, &bufShowValue );

  ef.addOption( activeMotifSliderClass_str89,
   activeMotifSliderClass_str90, &bufOrientation );

  ef.addTextField( activeMotifSliderClass_str28, 35, &bufIncrement );

  ef.addToggle( activeMotifSliderClass_str29, &bufLimitsFromDb );
  ef.addOption( activeMotifSliderClass_str30, activeMotifSliderClass_str35,
   &bufFormatType );
  ef.addTextField( activeMotifSliderClass_str31, 35, &bufEfPrecision );
  ef.addTextField( activeMotifSliderClass_str32, 35, &bufEfScaleMin );
  ef.addTextField( activeMotifSliderClass_str33, 35, &bufEfScaleMax );

  ef.addColorButton( activeMotifSliderClass_str24, actWin->ci, &fgCb,
   &bufFgColor );
  ef.addColorButton( activeMotifSliderClass_str26, actWin->ci, &bgCb,
   &bufBgColor );
  ef.addToggle( activeMotifSliderClass_str25, &bufBgColorMode );

  ef.addColorButton( activeMotifSliderClass_str27, actWin->ci, &shadeCb,
   &bufShadeColor );
  ef.addColorButton( activeMotifSliderClass_str40, actWin->ci, &topCb,
   &bufTopColor );
  ef.addColorButton( activeMotifSliderClass_str46, actWin->ci, &botCb,
   &bufBotColor );

  ef.addFontMenu( activeMotifSliderClass_str23, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeMotifSliderClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( mslc_edit_ok, mslc_edit_apply, mslc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMotifSliderClass::edit ( void ) {

  this->genericEdit();
  ef.finished( mslc_edit_ok, mslc_edit_apply, mslc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMotifSliderClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMotifSliderClass::eraseActive ( void ) {

  if ( !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();
  actWin->executeGc.setFG( bgColor.getColor() );

  XDrawRectangle( actWin->d, XtWindow(frameWidget),
   actWin->executeGc.normGC(), 0, 0, w, h );

  XFillRectangle( actWin->d, XtWindow(frameWidget),
   actWin->executeGc.normGC(), 0, 0, w, h );

  actWin->executeGc.restoreFg();

  return 1;

}

int activeMotifSliderClass::draw ( void ) {

int tX, tY;

  if ( deleteRequest ) return 1;

  if ( orientation == MSLC_K_HORIZONTAL ) {
    scaleX = 1;
    scaleW = w - 2;
    scaleY = labelH + limitsH + 1;
    scaleH = h - scaleY - 2;
  }
  else {
    if ( showLimits || showValue ) {
      scaleX = (int) ( 0.6 * (double) w );
      scaleW = w - scaleX - 2;
      if ( scaleW < 14 ) {
        scaleW = 14;
        scaleX = w - scaleW - 2;
      }
    }
    else {
      scaleX = 1;
      scaleW = w - 2;
    }
    scaleY = labelH + 1;
    scaleH = h - scaleY - 2;
    midVertScaleY = scaleH/2 + scaleY -
     (int) ( (double) fontHeight * 0.5 );
  }

  actWin->drawGc.saveFg();
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  actWin->drawGc.setFG( bgColor.pixelColor() );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( actWin->ci->pix(shadeColor) );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+scaleX+2, y+scaleY+2, scaleW-4, scaleH-4 );

  actWin->drawGc.setFG( fgColor.pixelColor() );

  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  if ( showLimits ) {

    if ( orientation == MSLC_K_HORIZONTAL ) {

      tX = 2;
      tY = labelH;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, x+tX, y+tY,
       XmALIGNMENT_BEGINNING, "0.0" );

      tX = w - 2;
      tY = labelH;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_END, "10.0" );

    }
    else {

      tX = scaleX;
      tY = h - 2 - limitsH;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_END, "0.0" );

      tX = scaleX;
      tY = scaleY;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_END, "10.0" );

    }

  }

  if ( showValue ) {

    if ( orientation == MSLC_K_HORIZONTAL ) {
      tX = w/2;
      tY = labelH;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_CENTER, "0.0" );
    }
    else {
      tX = scaleX;
      tY = midVertScaleY;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_END, "0.0" );
    }

  }

  if ( showLabel ) {

    if ( orientation == MSLC_K_HORIZONTAL ) {
      tX = 2;
      tY = 0;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_BEGINNING, "Label" );
    }
    else {
      tX = w - 2;
      tY = 0;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_END, "Label" );
    }

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeMotifSliderClass::eraseActiveControlText ( void ) {

int tX, tY;

  if ( !activeMode || !init || !showValue ) return 1;

  if ( fs && controlExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( bgColor.getColor() );

    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    if ( orientation == MSLC_K_HORIZONTAL ) {
      tX = w/2;
      tY = labelH;
      drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_CENTER, controlValue );
    }
    else {
      tX = scaleX;
      tY = midVertScaleY;
      drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, controlValue );
    }

    actWin->executeGc.restoreFg();

  }

  return 1;

}

int activeMotifSliderClass::drawActiveControlText ( void ) {

int tX, tY;

  if ( !activeMode || !init || !showValue ) return 1;

  if ( fs && controlExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.setFG( fgColor.getColor() );

    if ( fs ) {

      actWin->executeGc.setFontTag( fontTag, actWin->fi );

      if ( orientation == MSLC_K_HORIZONTAL ) {
        tX = w/2;
        tY = labelH;
        drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_CENTER, controlValue );
      }
      else {
        tX = scaleX;
        tY = midVertScaleY;
        drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_END, controlValue );
      }

    }

    actWin->executeGc.restoreFg();

  }

  return 1;

}

int activeMotifSliderClass::drawActive ( void ) {

int tX, tY;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( bgColor.getDisconnected() );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
  }

  if ( !activeMode || !init ) return 1;

  XtVaSetValues( frameWidget,
   XmNbackground, bgColor.getColor(),
   NULL );
  XtVaSetValues( scaleWidget,
   XmNbackground, bgColor.getColor(),
   NULL );

  actWin->executeGc.saveFg();
  actWin->executeGc.setFG( fgColor.getColor() );

  if ( fs ) {

    if ( controlExists ) {

      actWin->executeGc.setFontTag( fontTag, actWin->fi );

      if ( showLimits ) {

        if ( orientation == MSLC_K_HORIZONTAL ) {

          tX = 2;
          tY = labelH;
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_BEGINNING, minValue );

          tX = w - 2;
          tY = labelH;
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_END, maxValue );

	}
	else {

          tX = scaleX;
          tY = h - 2 - limitsH;
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_END, minValue );

          tX = scaleX;
          tY = scaleY;
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_END, maxValue );

	}

      }

      if ( showValue ) {

        if ( orientation == MSLC_K_HORIZONTAL ) {
          tX = w/2;
          tY = labelH;
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_CENTER, controlValue );
        }
        else {
          tX = scaleX;
          tY = midVertScaleY;
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_END, controlValue );
        }

      }

    }

    if ( showLabel && controlLabelExists ) {

      if ( orientation == MSLC_K_HORIZONTAL ) {
        tX = 2;
        tY = 0;
        drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_BEGINNING, controlLabel );
      }
      else {
        tX = w - 2;
        tY = 0;
        drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_END, controlLabel );
      }

    }

  }

  actWin->executeGc.restoreFg();

  return 1;

}

void activeMotifSliderClass::bufInvalidate ( void ) {

  bufInvalid = 1;

}

static void scrollBarEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

activeMotifSliderClass *mslo;

  *continueToDispatch = True;

  mslo = (activeMotifSliderClass *) client;

  if ( !mslo->active ) return;

  if ( e->type == EnterNotify ) {
    if ( mslo->controlPvId ) {
      if ( !mslo->controlPvId->have_write_access() ) {
        mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
         CURSOR_K_NO );
      }
      else {
        mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
         CURSOR_K_DEFAULT );
      }
    }
  }

  if ( e->type == LeaveNotify ) {
    mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
     CURSOR_K_DEFAULT );
  }

}

static void motifSliderEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

XButtonEvent *be;
activeMotifSliderClass *mslo;
 int stat;
char title[32], *ptr, strVal[255+1];

  *continueToDispatch = True;

  mslo = (activeMotifSliderClass *) client;

  if ( !mslo->active ) return;

  if ( e->type == EnterNotify ) {
    if ( mslo->controlPvId ) {
      if ( !mslo->controlPvId->have_write_access() ) {
        mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
         CURSOR_K_NO );
      }
      else {
        mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
         CURSOR_K_DEFAULT );
      }
    }
  }

  if ( e->type == LeaveNotify ) {
    mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
     CURSOR_K_DEFAULT );
  }

  ptr = mslo->actWin->obj.getNameFromClass( "activeMotifSliderClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMotifSliderClass_str54, 31 );

  strncat( title, activeMotifSliderClass_str55, 31 );

  if ( e->type == Expose ) {

    mslo->bufInvalidate();
    stat = mslo->drawActive();
    return;

  }

  if ( mslo->controlPvId ) {
    if ( !mslo->controlPvId->have_write_access() ) return;
  }

  if ( e->type == ButtonPress ) {

    be = (XButtonEvent *) e;

    switch ( be->button ) {

//========== B2 Press ========================================

    case Button2:

      if ( !( be->state & ShiftMask ) ) {
        stat = mslo->startDrag( w, e );
      }

      break;

//========== B2 Press ========================================

//========== B3 Press ========================================

    case Button3:

      mslo->bufIncrement = mslo->increment;
      mslo->bufControlV = mslo->controlV;
      mslo->valueFormX = mslo->actWin->x + mslo->x  + be->x;
      mslo->valueFormY = mslo->actWin->y + mslo->y + be->y;
      mslo->valueFormW = 0;
      mslo->valueFormH = 0;
      mslo->valueFormMaxH = 600;

      mslo->ef.create( mslo->actWin->top,
       mslo->actWin->appCtx->ci.getColorMap(),
       &mslo->valueFormX, &mslo->valueFormY,
       &mslo->valueFormW, &mslo->valueFormH, &mslo->valueFormMaxH,
       title, NULL, NULL, NULL );

      mslo->ef.addTextField( activeMotifSliderClass_str57, 20,
       &mslo->bufControlV );

      mslo->ef.addTextField( activeMotifSliderClass_str58, 20,
       &mslo->bufIncrement );

      calcIncRange( mslo->efScaleMin.value(), mslo->efScaleMax.value(),
       strVal, mslo->incArray );
      mslo->incIndex = 0;
      mslo->ef.addOption( activeMotifSliderClass_str58, strVal,
       &mslo->incIndex );

      mslo->ef.finished( mslc_value_ok, mslc_value_apply, mslc_value_cancel,
       mslo );
      mslo->ef.popup();

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
        stat = mslo->selectDragValue( mslo->x+be->x, mslo->y+be->y );
      }

      break;

    }

//========== Any B Release ========================================

  }

}

int activeMotifSliderClass::activate (
  int pass,
  void *ptr )
{

int opStat;

  switch ( pass ) {

  case 1:

    opComplete = 0;
    break;

  case 2:

    if ( !opComplete ) {

      opComplete = 1;

      oldStat = -1;
      oldSev = -1;
      prevScaleV = -1;
      dragIndicator = 0;
      controlPvId = controlLabelPvId = 0;

      strcpy( controlValue, "" );
      strcpy( incString, "" );
      activeMode = 1;
      init = 0;
      active = 0;
      aglPtr = ptr;
      curControlV = oneControlV = 0.0;
      controlV = 0.0;
      needCtlConnectInit = needCtlInfoInit = needCtlRefresh =
       needCtlLabelConnectInit = needCtlLabelInfoInit =
       needErase = needDraw = 0;

      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      oldControlV = 0;
      updateControlTimerActive = 0;
      updateControlTimer = 0;
      controlAdjusted = 0;
      incrementTimerActive = 0;

      controlPvConnected = 0;

      if ( !controlPvName.getExpanded() ||
         ( strcmp( controlPvName.getExpanded(), "" ) == 0 ) ) {
        controlExists = 0;
      }
      else {
        controlExists = 1;
        fgColor.setConnectSensitive();
        bgColor.setConnectSensitive();
      }

      if ( !controlLabelName.getExpanded() ||
         ( strcmp( controlLabelName.getExpanded(), "" ) == 0 ) ) {
        controlLabelExists = 0;
      }
      else {
        controlLabelExists = 1;
      }

      if ( controlLabelType == MSLC_K_PV_NAME ) {
        controlLabelExists = 1;
        strncpy( controlLabel, controlPvName.getExpanded(),
         PV_Factory::MAX_PV_NAME );
      }
      else {
        strncpy( controlLabel, controlLabelName.getExpanded(),
         PV_Factory::MAX_PV_NAME );
      }

    }

    break;

  case 3:

    opComplete = 0;
    break;

  case 4:

    if ( !opComplete ) {

      opStat = 1;

      if ( controlExists ) {
        controlPvId = the_PV_Factory->create( controlPvName.getExpanded() );
        if ( controlPvId ) {
          controlPvId->add_conn_state_callback(
           monitorControlConnectState, this );
          controlPvId->add_value_callback(
           controlUpdate, this );
	}
      }

      if ( controlLabelExists && ( controlLabelType == MSLC_K_LABEL )  ) {
        controlLabelPvId =
         the_PV_Factory->create( controlLabelName.getExpanded() );
        if ( controlLabelPvId ) {
          controlLabelPvId->add_conn_state_callback(
           monitorControlLabelConnectState, this );
          controlLabelPvId->add_value_callback(
           controlLabelUpdate, this );
	}

      }

      if ( opStat & 1 ) {

        updateControlTimerActive = 1;
        updateControlTimerValue = 100;
        updateControlTimer = appAddTimeOut( actWin->appCtx->appContext(),
         updateControlTimerValue, mslc_updateControl, (void *) this );

        opComplete = 1;

      }

      return opStat;

    }

    break;

  case 5:
  case 6:

    break;

  }

  return 1;

}

int activeMotifSliderClass::deactivate (
  int pass
) {

  active = activeMode = 0;

  switch ( pass ) {

  case 1:

    if ( ef.formIsPoppedUp() ) {
      ef.popdown();
    }

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    updateControlTimerActive = 0;
    if ( updateControlTimer ) {
      XtRemoveTimeOut( updateControlTimer );
      updateControlTimer = 0;
    }

    if ( frameWidget ) {
      XtRemoveEventHandler( frameWidget,
       ButtonPressMask|ExposureMask|EnterWindowMask|LeaveWindowMask, False,
       motifSliderEventHandler, (XtPointer) this );
    }

// for EPICS support

    if ( controlPvId ) {
      controlPvId->remove_conn_state_callback(
       monitorControlConnectState, this );
      controlPvId->remove_value_callback(
       controlUpdate, this );
      controlPvId->release();
      controlPvId = 0;
    }

    if ( controlLabelPvId ) {
      controlLabelPvId->remove_conn_state_callback(
       monitorControlLabelConnectState, this );
      controlLabelPvId->remove_value_callback(
       controlLabelUpdate, this );
      controlLabelPvId->release();
      controlLabelPvId = 0;
    }

    break;

  case 2:

    if ( frameWidget ) {
      if ( scaleWidget ) {
        XtUnmapWidget( scaleWidget );
        XtDestroyWidget( scaleWidget );
        scaleWidget = NULL;
      }
      XtUnmapWidget( frameWidget );
      XtDestroyWidget( frameWidget );
      frameWidget = NULL;
    }

    break;

  }

  return 1;

}

int activeMotifSliderClass::checkResizeSelectBox (
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

int activeMotifSliderClass::checkResizeSelectBoxAbs (
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

void activeMotifSliderClass::updateDimensions ( void )
{

  if ( fs ) {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
  }
  else {
    fontAscent = 10;
    fontDescent = 5;
    fontHeight = fontAscent + fontDescent;
  }

  if ( showLimits || showValue ) {
    limitsH = fontHeight;
  }
  else {
    limitsH = 0;
  }

  if ( showLabel ) {
    labelH = fontHeight;
  }
  else {
    labelH = 0;
  }

  if ( orientation == MSLC_K_HORIZONTAL ) {
    minW = 50;
    minH = 14 + limitsH + labelH;
  }
  else {
    minW = 14;
    minH = 50;
  }

  // dummy values at this point
  minFv = 0.0;
  maxFv = 10.0;
  positive = 1;

  factor = ( maxFv - minFv ) / 100000;
  if ( factor == 0.0 ) factor = 1.0;

  midVertScaleY = scaleH/2 + scaleY - (int) ( (double) fontHeight * 0.5 );

}

int activeMotifSliderClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

  int retStat, stat;

  retStat = 1;

  stat = controlPvName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = controlLabelName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeMotifSliderClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

  int retStat, stat;

  retStat = 1;

  stat = controlPvName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = controlLabelName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeMotifSliderClass::containsMacros ( void ) {

  if ( controlPvName.containsPrimaryMacros() ) return 1;

  if ( controlLabelName.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeMotifSliderClass::executeDeferred ( void ) {

int stat, ncc, nci, ncr, nclc, ncli, ne, nd, i;
unsigned char orien, pd;
double cv, fv;
XtTranslations parsedTrans;
WidgetList children;
Cardinal numChildren;

static char dragTrans[] =
  "#override None<Btn2Down>: startDrag()\n\
   Shift<Btn2Down>: dummy()\n\
   Shift<Btn2Up>: selectDrag()\n\
   Ctrl<Btn1Down>: dummy()\n\
   Ctrl<Btn1Up>: dummy()\n\
   <Btn3Up>: changeParams()\n\
   <Key>: dummy()";

static XtActionsRec dragActions[] = {
  { "startDrag", (XtActionProc) drag },
  { "dummy", (XtActionProc) dummy },
  { "changeParams", (XtActionProc) changeParams },
  { "selectDrag", (XtActionProc) selectDrag }
};

  if ( actWin->isIconified ) return;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  ncc = needCtlConnectInit; needCtlConnectInit = 0;
  nci = needCtlInfoInit; needCtlInfoInit = 0;
  ncr = needCtlRefresh; needCtlRefresh = 0;
  nclc = needCtlLabelConnectInit; needCtlLabelConnectInit = 0;
  ncli = needCtlLabelInfoInit; needCtlLabelInfoInit = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  cv = curControlV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( ncc ) {

    controlPvConnected = 1;

    if ( !frameWidget ) {

      frameWidget = XtVaCreateManagedWidget( "",
       xmDrawingAreaWidgetClass,
       actWin->executeWidgetId(),
       XmNx, x,
       XmNy, y,
       XmNwidth, w,
       XmNheight, h,
       XmNmarginHeight, 0,
       XmNmarginWidth, 0,
       XmNresizePolicy, XmRESIZE_NONE,
       XmNbackground, bgColor.pixelColor(),
       NULL );

      if ( frameWidget ) {

        XtAddEventHandler( frameWidget,
         ButtonPressMask|ButtonReleaseMask|ExposureMask|
         EnterWindowMask|LeaveWindowMask, False,
         motifSliderEventHandler, (XtPointer) this );

        if ( orientation == MSLC_K_HORIZONTAL ) {
          scaleX = 1;
          scaleW = w - 2;
          scaleY = labelH + limitsH + 1;
          scaleH = h - scaleY - 2;
        }
        else {
          if ( showLimits || showValue ) {
            scaleX = (int) ( 0.6 * (double) w );
            scaleW = w - scaleX - 2;
            if ( scaleW < 14 ) {
              scaleW = 14;
              scaleX = w - scaleW - 2;
            }
          }
          else {
            scaleX = 1;
            scaleW = w - 2;
          }
          scaleY = labelH + 1;
          scaleH = h - scaleY - 2;
          midVertScaleY = scaleH/2 + scaleY -
           (int) ( (double) fontHeight * 0.5 );
        }

        parsedTrans = XtParseTranslationTable( dragTrans );
        XtAppAddActions( actWin->appCtx->appContext(), dragActions,
         XtNumber(dragActions) );

        if ( orientation == MSLC_K_HORIZONTAL ) {
          orien = XmHORIZONTAL;
          pd = XmMAX_ON_RIGHT;
        }
        else {
          orien = XmVERTICAL;
          pd = XmMAX_ON_TOP;
        }

        scaleWidget = XtVaCreateManagedWidget(
         "", xmScaleWidgetClass,
         frameWidget,
         XmNx, scaleX,
         XmNy, scaleY,
         XmNwidth, scaleW,
         XmNheight, scaleH,
         XmNscaleWidth, scaleW,
         XmNscaleHeight, scaleH,
         XmNorientation, orien,
         XmNprocessingDirection, pd,
         XmNscaleMultiple, 1,
         XmNminimum, 0,
         XmNmaximum, 100000,
         XmNnavigationType, XmNONE,
         XmNtraversalOn, False,
         XmNhighlightOnEnter, True,
         XmNuserData, this,
         XmNforeground, fgColor.getColor(),
         XmNbackground, bgColor.pixelColor(),
         XmNtopShadowColor, actWin->ci->pix(topColor),
         XmNbottomShadowColor, actWin->ci->pix(botColor),
         NULL );

        XtVaGetValues( scaleWidget,
         XmNnumChildren, &numChildren,
         XmNchildren, &children,
         NULL );

        scrollBarWidget = NULL;
        for ( i=0; i<(int)numChildren; i++ ) {
          if ( XtClass( children[i] ) == xmScrollBarWidgetClass) {
            scrollBarWidget = children[i];
            XtVaSetValues( children[i],
             //XmNtranslations, parsedTrans,
             XmNuserData, this,
             NULL );
            XtOverrideTranslations( children[i], parsedTrans );
          }
        }

        if ( scrollBarWidget ) {

          XtVaSetValues( scrollBarWidget,
           XmNforeground, fgColor.getColor(),
           XmNbackground, bgColor.pixelColor(),
           XmNtroughColor, actWin->ci->pix(shadeColor),
           XmNtopShadowColor, actWin->ci->pix(topColor),
           XmNbottomShadowColor, actWin->ci->pix(botColor),
           XmNinitialDelay, 500,
           XmNrepeatDelay, 1,
           NULL );

          XtAddEventHandler( scrollBarWidget,
           EnterWindowMask|LeaveWindowMask, False,
           scrollBarEventHandler, (XtPointer) this );

        }

        XtAddCallback( scaleWidget, XmNvalueChangedCallback,
         msloValueChangeCB, (XtPointer) this );

        XtAddCallback( scaleWidget, XmNdragCallback,
         msloIndicatorDragCB, (XtPointer) this );

        XtManageChild( frameWidget );

      }

    }

  }

//----------------------------------------------------------------------------

  if ( nci ) {

    controlV = cv;

    sprintf( minValue, "%-g", minFv );

    sprintf( maxValue, "%-g", maxFv );

    if ( maxFv > minFv )
      positive = 1;
    else
      positive = 0;

    sprintf( controlValue, controlFormat, controlV );

    factor = ( maxFv - minFv ) / 100000;
    if ( factor == 0.0 ) factor = 1.0;

    controlX = (int) ( ( controlV - minFv ) /
     factor + 0.5 );

    sprintf( incString, controlFormat, increment );

    active = 1;
    init = 1;

    savedV = controlV;

    savedX = (int) ( ( savedV - minFv ) /
     factor + 0.5 );

    fgColor.setConnected();
    bgColor.setConnected();

    bufInvalidate();

    stat = eraseActive();
    stat = drawActive();

    bufInvalidate();

  }

//----------------------------------------------------------------------------

  if ( ncr ) {

    eraseActiveControlText();

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

    controlX = (int) ( ( fv - minFv ) /
     factor + 0.5 );

    savedX = (int) ( ( savedV - minFv ) /
     factor + 0.5 );

    sprintf( controlValue, controlFormat, controlV );

    stat = drawActiveControlText();

  }

//----------------------------------------------------------------------------

  if ( nclc ) {

    ; // do nothing

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

  if ( ne ) {

    eraseActive();

  }

//----------------------------------------------------------------------------

  if ( nd ) {

    drawActive();

  }

//----------------------------------------------------------------------------

}

int activeMotifSliderClass::getProperty (
  char *prop,
  double *_value )
{

  if ( strcmp( prop, activeMotifSliderClass_str85 ) == 0 ) {

    *_value = controlV;
    return 1;

  }

  return 0;

}

char *activeMotifSliderClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeMotifSliderClass::nextDragName ( void ) {

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeMotifSliderClass::dragValue (
  int i ) {

  switch ( i ) {
  case 0:
    return controlPvName.getExpanded();
    break;
  }

  return NULL;

}

void activeMotifSliderClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_OFFSETCOLOR_MASK )
    shadeColor = _offsetColor;

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topColor = _topShadowColor;

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botColor = _botShadowColor;

  if ( _flag & ACTGRF_CTLFONTTAG_MASK ) {

    strcpy( fontTag, _ctlFontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );

    updateDimensions();
    if ( h < minH ) h = minH;
    if ( w < minW ) w = minW;

  }

}

void activeMotifSliderClass::changePvNames (
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

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMotifSliderClassPtr ( void ) {

activeMotifSliderClass *ptr;

  ptr = new activeMotifSliderClass;
  return (void *) ptr;

}

void *clone_activeMotifSliderClassPtr (
  void *_srcPtr )
{

activeMotifSliderClass *ptr, *srcPtr;

  srcPtr = (activeMotifSliderClass *) _srcPtr;

  ptr = new activeMotifSliderClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
