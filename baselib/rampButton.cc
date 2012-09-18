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

#define __rampButton_cc 1

#include "rampButton.h"
#include "app_pkg.h"
#include "act_win.h"

static void rbtc_doBlink (
  void *ptr
) {

activeRampButtonClass *rbto = (activeRampButtonClass *) ptr;

  if ( !rbto->activeMode ) {
    if ( rbto->isSelected() ) rbto->drawSelectBoxCorners(); //erase via xor
    rbto->smartDrawAll();
    if ( rbto->isSelected() ) rbto->drawSelectBoxCorners();
  }
  else {
    rbto->bufInvalidate();
    rbto->needDraw = 1;
    rbto->actWin->addDefExeNode( rbto->aglPtr );
  }

}

static void rbtc_unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) client;

  if ( !rbto->init ) {
    rbto->needToDrawUnconnected = 1;
    rbto->needDraw = 1;
    rbto->actWin->addDefExeNode( rbto->aglPtr );
  }

  rbto->unconnectedTimer = 0;

}

static void rbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) client;

  rbto->actWin->setChanged();

  rbto->eraseSelectBoxCorners();
  rbto->erase();

  rbto->fgColor.setColorIndex( rbto->eBuf->bufFgColor, rbto->actWin->ci );

  rbto->bgColor.setColorIndex( rbto->eBuf->bufBgColor, rbto->actWin->ci );

  rbto->topShadowColor = rbto->eBuf->bufTopShadowColor;
  rbto->botShadowColor = rbto->eBuf->bufBotShadowColor;

  rbto->destPvExpString.setRaw( rbto->eBuf->bufDestPvName );

  rbto->finalPvExpString.setRaw( rbto->eBuf->bufFinalPvName );

  rbto->rampStatePvExpString.setRaw( rbto->eBuf->bufRampStatePvName );

  rbto->label.setRaw( rbto->eBuf->bufLabel );

  strncpy( rbto->fontTag, rbto->fm.currentFontTag(), 63 );
  rbto->actWin->fi->loadFontTag( rbto->fontTag );
  rbto->fs = rbto->actWin->fi->getXFontStruct( rbto->fontTag );

  rbto->_3D = rbto->eBuf->buf3D;

  rbto->invisible = rbto->eBuf->bufInvisible;

  rbto->updateRate = rbto->eBuf->bufUpdateRate;
  if ( rbto->updateRate < 0.1 ) rbto->updateRate = 0.1;
  if ( rbto->updateRate > 10.0 ) rbto->updateRate = 10.0;

  rbto->rampRate = rbto->eBuf->bufRampRate;

  rbto->limitsFromDb = rbto->eBuf->bufLimitsFromDb;

  rbto->efScaleMin = rbto->eBuf->bufEfScaleMin;
  rbto->efScaleMax = rbto->eBuf->bufEfScaleMax;

  rbto->minDv = rbto->scaleMin = rbto->efScaleMin.value();
  rbto->maxDv = rbto->scaleMax = rbto->efScaleMax.value();

  rbto->visPvExpString.setRaw( rbto->eBuf->bufVisPvName );
  strncpy( rbto->minVisString, rbto->eBuf->bufMinVisString, 39 );
  strncpy( rbto->maxVisString, rbto->eBuf->bufMaxVisString, 39 );

  if ( rbto->eBuf->bufVisInverted )
    rbto->visInverted = 0;
  else
    rbto->visInverted = 1;

  rbto->colorPvExpString.setRaw( rbto->eBuf->bufColorPvName );

  rbto->x = rbto->eBuf->bufX;
  rbto->sboxX = rbto->eBuf->bufX;

  rbto->y = rbto->eBuf->bufY;
  rbto->sboxY = rbto->eBuf->bufY;

  rbto->w = rbto->eBuf->bufW;
  rbto->sboxW = rbto->eBuf->bufW;

  rbto->h = rbto->eBuf->bufH;
  rbto->sboxH = rbto->eBuf->bufH;

  rbto->updateDimensions();

}

static void rbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) client;

  rbtc_edit_update ( w, client, call );
  rbto->refresh( rbto );

}

static void rbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) client;

  rbtc_edit_update ( w, client, call );
  rbto->ef.popdown();
  rbto->operationComplete();

}

static void rbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) client;

  rbto->ef.popdown();
  rbto->operationCancel();

}

static void rbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) client;

  rbto->ef.popdown();
  rbto->operationCancel();
  rbto->erase();
  rbto->deleteRequest = 1;
  rbto->drawAll();

}

static void rbtc_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) userarg;

  if ( pv->is_valid() ) {

    rbto->needConnectInit = 1;

  }
  else {

    rbto->connection.setPvDisconnected( (void *) rbto->destPvConnection );
    rbto->active = 0;
    rbto->bgColor.setDisconnected();
    rbto->needDraw = 1;

  }

  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbtc_monitor_final_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) userarg;

  if ( pv->is_valid() ) {

    rbto->needFinalConnectInit = 1;
    rbto->actWin->appCtx->proc->lock();
    rbto->actWin->addDefExeNode( rbto->aglPtr );
    rbto->actWin->appCtx->proc->unlock();

  }
  else {

    rbto->connection.setPvDisconnected( (void *) rbto->finalPvConnection );
    rbto->active = 0;
    rbto->bgColor.setDisconnected();
    rbto->needDraw = 1;

  }

  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbtc_monitor_rampState_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) userarg;

  if ( pv->is_valid() ) {

    rbto->needRampStateConnectInit = 1;
    rbto->actWin->appCtx->proc->lock();
    rbto->actWin->addDefExeNode( rbto->aglPtr );
    rbto->actWin->appCtx->proc->unlock();

  }
  else {

    rbto->connection.setPvDisconnected( (void *) rbto->rampStatePvConnection );
    rbto->active = 0;
    rbto->bgColor.setDisconnected();
    rbto->needDraw = 1;

  }

  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbtc_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) userarg;

  rbto->actWin->appCtx->proc->lock();

  rbto->curControlV = pv->get_double();

  rbto->actWin->appCtx->proc->unlock();

}

static void rbtc_finalUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) userarg;

  rbto->actWin->appCtx->proc->lock();

  rbto->curFinalV = pv->get_double();

  rbto->actWin->appCtx->proc->unlock();

}

static void rbtc_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) userarg;

  if ( pv->is_valid() ) {

    rbto->needVisConnectInit = 1;

  }
  else {

    rbto->connection.setPvDisconnected( (void *) rbto->visPvConnection );
    rbto->active = 0;
    rbto->bgColor.setDisconnected();
    rbto->needDraw = 1;

  }

  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbtc_visUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) userarg;

  rbto->curVisValue = pv->get_double();

  rbto->actWin->appCtx->proc->lock();
  rbto->needVisUpdate = 1;
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbtc_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) userarg;

  if ( pv->is_valid() ) {

    rbto->needColorConnectInit = 1;

  }
  else {

    rbto->connection.setPvDisconnected( (void *) rbto->colorPvConnection );
    rbto->active = 0;
    rbto->bgColor.setDisconnected();
    rbto->needDraw = 1;

  }

  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbtc_colorUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) userarg;

  rbto->curColorValue = pv->get_double();

  rbto->actWin->appCtx->proc->lock();
  rbto->needColorUpdate = 1;
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbtc_decrement (
  XtPointer client,
  XtIntervalId *id )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) client;
double dval, seconds, adjust;
struct timeval curTime;

  gettimeofday( &curTime, NULL );
  seconds = curTime.tv_sec - rbto->baseTime.tv_sec +
    ( curTime.tv_usec - rbto->baseTime.tv_usec ) * 0.000001;
  rbto->baseTime = curTime;

  adjust = seconds / rbto->updateRate;
  if ( adjust > 1.1 ) adjust = 1.1;
  if ( adjust < 0.9 ) adjust = 0.9;

  if ( !rbto->incrementTimerActive ) {
    rbto->incrementTimer = 0;
    return;
  }

  rbto->incrementTimer = appAddTimeOut(
   rbto->actWin->appCtx->appContext(),
   rbto->incrementTimerValue, rbtc_decrement, client );

  rbto->actWin->appCtx->proc->lock();
  dval = rbto->curControlV;
  rbto->actWin->appCtx->proc->unlock();

  dval -= rbto->increment * adjust;

  if ( dval <= rbto->rampFinalV ) {
    dval = rbto->rampFinalV;
    rbto->incrementTimerActive = 0;
    rbto->buttonPressed = 0;
    if ( rbto->rampStateExists ) {
      rbto->rampStatePvId->put(
       XDisplayName(rbto->actWin->appCtx->displayName),
       rbto->buttonPressed );
    }
    rbto->actWin->appCtx->proc->lock();
    rbto->needRefresh = 1;
    rbto->actWin->addDefExeNode( rbto->aglPtr );
    rbto->actWin->appCtx->proc->unlock();
  }
  if ( dval <= rbto->minDv ) {
    dval = rbto->minDv;
    rbto->incrementTimerActive = 0;
    rbto->buttonPressed = 0;
    if ( rbto->rampStateExists ) {
      rbto->rampStatePvId->put(
       XDisplayName(rbto->actWin->appCtx->displayName),
       rbto->buttonPressed );
    }
    rbto->actWin->appCtx->proc->lock();
    rbto->needRefresh = 1;
    rbto->actWin->addDefExeNode( rbto->aglPtr );
    rbto->actWin->appCtx->proc->unlock();
  }
  else if ( dval >= rbto->maxDv ) {
    dval = rbto->maxDv;
    rbto->incrementTimerActive = 0;
    rbto->buttonPressed = 0;
    if ( rbto->rampStateExists ) {
      rbto->rampStatePvId->put(
       XDisplayName(rbto->actWin->appCtx->displayName),
       rbto->buttonPressed );
    }
    rbto->actWin->appCtx->proc->lock();
    rbto->needRefresh = 1;
    rbto->actWin->addDefExeNode( rbto->aglPtr );
    rbto->actWin->appCtx->proc->unlock();
  }

  if ( rbto->destExists ) {
    rbto->destPvId->put(
     XDisplayName(rbto->actWin->appCtx->displayName),
     dval );
  }

}

static void rbtc_increment (
  XtPointer client,
  XtIntervalId *id )
{

activeRampButtonClass *rbto = (activeRampButtonClass *) client;
double dval, seconds, adjust;
struct timeval curTime;

  gettimeofday( &curTime, NULL );
  seconds = curTime.tv_sec - rbto->baseTime.tv_sec +
    ( curTime.tv_usec - rbto->baseTime.tv_usec ) * 0.000001;
  rbto->baseTime = curTime;

  adjust = seconds / rbto->updateRate;
  if ( adjust > 1.1 ) adjust = 1.1;
  if ( adjust < 0.9 ) adjust = 0.9;

  if ( !rbto->incrementTimerActive ) {
    rbto->incrementTimer = 0;
    return;
  }

  rbto->incrementTimer = appAddTimeOut(
   rbto->actWin->appCtx->appContext(),
   rbto->incrementTimerValue, rbtc_increment, client );

  rbto->actWin->appCtx->proc->lock();
  dval = rbto->curControlV;
  rbto->actWin->appCtx->proc->unlock();

  dval += rbto->increment * adjust;

  if ( dval >= rbto->rampFinalV ) {
    dval = rbto->rampFinalV;
    rbto->incrementTimerActive = 0;
    rbto->buttonPressed = 0;
    if ( rbto->rampStateExists ) {
      rbto->rampStatePvId->put(
       XDisplayName(rbto->actWin->appCtx->displayName),
       rbto->buttonPressed );
    }
    rbto->actWin->appCtx->proc->lock();
    rbto->needRefresh = 1;
    rbto->actWin->addDefExeNode( rbto->aglPtr );
    rbto->actWin->appCtx->proc->unlock();
  }
  if ( dval <= rbto->minDv ) {
    dval = rbto->minDv;
    rbto->incrementTimerActive = 0;
    rbto->buttonPressed = 0;
    if ( rbto->rampStateExists ) {
      rbto->rampStatePvId->put(
       XDisplayName(rbto->actWin->appCtx->displayName),
       rbto->buttonPressed );
    }
    rbto->actWin->appCtx->proc->lock();
    rbto->needRefresh = 1;
    rbto->actWin->addDefExeNode( rbto->aglPtr );
    rbto->actWin->appCtx->proc->unlock();
  }
  else if ( dval >= rbto->maxDv ) {
    dval = rbto->maxDv;
    rbto->incrementTimerActive = 0;
    rbto->buttonPressed = 0;
    if ( rbto->rampStateExists ) {
      rbto->rampStatePvId->put(
       XDisplayName(rbto->actWin->appCtx->displayName),
       rbto->buttonPressed );
    }
    rbto->actWin->appCtx->proc->lock();
    rbto->needRefresh = 1;
    rbto->actWin->addDefExeNode( rbto->aglPtr );
    rbto->actWin->appCtx->proc->unlock();
  }

  if ( rbto->destExists ) {
    rbto->destPvId->put(
     XDisplayName(rbto->actWin->appCtx->displayName),
     dval );
  }

}

activeRampButtonClass::activeRampButtonClass ( void ) {

  name = new char[strlen("activeRampButtonClass")+1];
  strcpy( name, "activeRampButtonClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  buttonPressed = 0;
  state = RBTC_IDLE;
  _3D = 1;
  invisible = 0;
  updateRate = 0.5;
  rampRate = 0.0;
  curFinalV = 0.0;
  scaleMin = 0;
  scaleMax = 10;
  limitsFromDb = 1;
  efScaleMin.setNull(1);
  efScaleMax.setNull(1);
  unconnectedTimer = 0;
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  connection.setMaxPvs( 5 );
  activeMode = 0;
  eBuf = NULL;

  setBlinkFunction( (void *) rbtc_doBlink );

}

// copy constructor
activeRampButtonClass::activeRampButtonClass
 ( const activeRampButtonClass *source ) {

activeGraphicClass *rbto = (activeGraphicClass *) this;

  rbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeRampButtonClass")+1];
  strcpy( name, "activeRampButtonClass" );

  buttonPressed = 0;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  destPvExpString.copy( source->destPvExpString );
  finalPvExpString.copy( source->finalPvExpString );
  rampStatePvExpString.copy( source->rampStatePvExpString );
  visPvExpString.copy( source->visPvExpString );
  colorPvExpString.copy( source->colorPvExpString );

  label.copy( source->label );

  state = RBTC_IDLE;
  _3D = source->_3D;
  invisible = source->invisible;
  updateRate = source->updateRate;
  rampRate = source->rampRate;
  curFinalV = 0.0;
  limitsFromDb = source->limitsFromDb;
  scaleMin = source->scaleMin;
  scaleMax = source->scaleMax;
  efScaleMin = source->efScaleMin;
  efScaleMax = source->efScaleMax;
  unconnectedTimer = 0;

  visibility = 0;
  prevVisibility = -1;
  visInverted = source->visInverted;
  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );
  activeMode = 0;
  eBuf = NULL;

  connection.setMaxPvs( 5 );

  setBlinkFunction( (void *) rbtc_doBlink );

  doAccSubs( destPvExpString );
  doAccSubs( finalPvExpString );
  doAccSubs( rampStatePvExpString );
  doAccSubs( label );
  doAccSubs( colorPvExpString );
  doAccSubs( visPvExpString );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );

  updateDimensions();

}

activeRampButtonClass::~activeRampButtonClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

int activeRampButtonClass::createInteractive (
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

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  strcpy( fontTag, actWin->defaultBtnFontTag );

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  this->draw();

  this->editCreate();

  return 1;

}

int activeRampButtonClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

  major = RBTC_MAJOR_VERSION;
  minor = RBTC_MINOR_VERSION;
  release = RBTC_RELEASE;

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
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "controlPv", &destPvExpString, emptyStr );
  tag.loadW( "finalValuePv", &finalPvExpString, emptyStr );
  tag.loadW( "rampStateValuePv", &rampStatePvExpString, emptyStr );
  tag.loadW( "updateRate", &updateRate, &dzero );
  tag.loadW( "rampRate", &rampRate, &dzero );
  tag.loadW( "label", &label, emptyStr );
  tag.loadBoolW( "3d", &_3D, &zero );
  tag.loadBoolW( "invisible", &invisible, &zero );
  tag.loadW( "font", fontTag );
  tag.loadBoolW( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadW( "scaleMin", &efScaleMin );
  tag.loadW( "scaleMax", &efScaleMax );
  tag.loadW( "visPv", &visPvExpString, emptyStr );
  tag.loadBoolW( "visInvert", &visInverted, &zero );
  tag.loadW( "visMin", minVisString, emptyStr );
  tag.loadW( "visMax", maxVisString, emptyStr );
  tag.loadW( "colorPv", &colorPvExpString, emptyStr  );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeRampButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

  this->actWin = _actWin;

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
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "controlPv", &destPvExpString, emptyStr );
  tag.loadR( "finalValuePv", &finalPvExpString, emptyStr );
  tag.loadR( "rampStateValuePv", &rampStatePvExpString, emptyStr );
  tag.loadR( "updateRate", &updateRate, &dzero );
  tag.loadR( "rampRate", &rampRate, &dzero );
  tag.loadR( "label", &label, emptyStr );
  tag.loadR( "3d", &_3D, &zero );
  tag.loadR( "invisible", &invisible, &zero );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadR( "scaleMin", &efScaleMin );
  tag.loadR( "scaleMax", &efScaleMax );
  tag.loadR( "visPv", &visPvExpString, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "colorPv", &colorPvExpString, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( updateRate < 0.1 ) updateRate = 0.1;
  if ( updateRate > 10.0 ) updateRate = 10.0;

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > RBTC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( limitsFromDb || efScaleMin.isNull() ) &&
       ( limitsFromDb || efScaleMax.isNull() ) ) {
    minDv = scaleMin = 0;
    maxDv = scaleMax = 10;
  }
  else{
    minDv = scaleMin = efScaleMin.value();
    maxDv = scaleMax = efScaleMax.value();
  }

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeRampButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeRampButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeRampButtonClass_str2, 31 );

  Strncat( title, activeRampButtonClass_str3, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufFgColor = fgColor.pixelIndex();

  eBuf->bufBgColor = bgColor.pixelIndex();

  eBuf->bufTopShadowColor = topShadowColor;
  eBuf->bufBotShadowColor = botShadowColor;

  if ( destPvExpString.getRaw() )
    strncpy( eBuf->bufDestPvName, destPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufDestPvName, "" );

  if ( finalPvExpString.getRaw() )
    strncpy( eBuf->bufFinalPvName, finalPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufFinalPvName, "" );

  if ( rampStatePvExpString.getRaw() )
    strncpy( eBuf->bufRampStatePvName, rampStatePvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufRampStatePvName, "" );

  eBuf->bufUpdateRate = updateRate;

  eBuf->bufRampRate = rampRate;

  if ( label.getRaw() )
    strncpy( eBuf->bufLabel, label.getRaw(), 39 );
  else
    strncpy( eBuf->bufLabel, "", 39 );

  eBuf->buf3D = _3D;
  eBuf->bufInvisible = invisible;

  eBuf->bufLimitsFromDb = limitsFromDb;
  eBuf->bufEfScaleMin = efScaleMin;
  eBuf->bufEfScaleMax = efScaleMax;

  if ( visPvExpString.getRaw() )
    strncpy( eBuf->bufVisPvName, visPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufVisPvName, "" );

  if ( visInverted )
    eBuf->bufVisInverted = 0;
  else
    eBuf->bufVisInverted = 1;

  if ( colorPvExpString.getRaw() )
    strncpy( eBuf->bufColorPvName, colorPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufColorPvName, "" );

  strncpy( eBuf->bufMinVisString, minVisString, 39 );
  strncpy( eBuf->bufMaxVisString, maxVisString, 39 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeRampButtonClass_str4, 35, &eBuf->bufX );
  ef.addTextField( activeRampButtonClass_str5, 35, &eBuf->bufY );
  ef.addTextField( activeRampButtonClass_str6, 35, &eBuf->bufW );
  ef.addTextField( activeRampButtonClass_str7, 35, &eBuf->bufH );
  ef.addTextField( activeRampButtonClass_str8, 35, eBuf->bufDestPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeRampButtonClass_str9, 35, eBuf->bufFinalPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeRampButtonClass_str34, 35, eBuf->bufRampStatePvName,
   PV_Factory::MAX_PV_NAME );

  ef.addToggle( activeRampButtonClass_str26, &eBuf->bufLimitsFromDb );
  limitsFromDbEntry = ef.getCurItem();
  ef.addTextField( activeRampButtonClass_str27, 35, &eBuf->bufEfScaleMin );
  minEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( minEntry );
  ef.addTextField( activeRampButtonClass_str28, 35, &eBuf->bufEfScaleMax );
  maxEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( maxEntry );
  limitsFromDbEntry->addDependencyCallbacks();

  ef.addTextField( activeRampButtonClass_str10, 35, &eBuf->bufRampRate );
  ef.addTextField( activeRampButtonClass_str11, 35, &eBuf->bufUpdateRate );
  ef.addToggle( activeRampButtonClass_str12, &eBuf->buf3D );
  ef.addToggle( activeRampButtonClass_str13, &eBuf->bufInvisible );
  ef.addTextField( activeRampButtonClass_str14, 35, eBuf->bufLabel, 39 );
  ef.addColorButton( activeRampButtonClass_str16, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addColorButton( activeRampButtonClass_str17, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  ef.addColorButton( activeRampButtonClass_str18, actWin->ci, &eBuf->topShadowCb, &eBuf->bufTopShadowColor );
  ef.addColorButton( activeRampButtonClass_str19, actWin->ci, &eBuf->botShadowCb, &eBuf->bufBotShadowColor );

  ef.addFontMenu( activeRampButtonClass_str15, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeRampButtonClass_str33, 30, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeRampButtonClass_str29, 30, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeRampButtonClass_str30, &eBuf->bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeRampButtonClass_str31, 30, eBuf->bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeRampButtonClass_str32, 30, eBuf->bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  return 1;

}

int activeRampButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( rbtc_edit_ok, rbtc_edit_apply, rbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeRampButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( rbtc_edit_ok, rbtc_edit_apply, rbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeRampButtonClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeRampButtonClass::eraseActive ( void ) {

  if ( !enabled || !init || !activeMode || invisible ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeRampButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( bgColor.pixelIndex(), &blink );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( !_3D ) {

    actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  }

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( _3D ) {

  actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x, y+h );

   actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x, y+h, x+w, y+h );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x+w, y, x+w, y+h );

  actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+w-2, y+2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+2, y+h-2 );

  actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  }

  actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );

  //XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
  // actWin->drawGc.normGC(), x+5, y+9, x+w-5, y+9 );

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    if ( label.getRaw() )
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, label.getRaw() );
    else
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, "" );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeRampButtonClass::drawActive ( void ) {

int tX, tY;
char string[63+1];
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( bgColor.getDisconnectedIndex(), &blink );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
      updateBlink( blink );
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
    if ( invisible ) {
      eraseActive();
      smartDrawAllActive();
    }
  }

  if ( !enabled || !init || !activeMode || invisible || !visibility ) return 1;

  prevVisibility = visibility;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( bgColor.getIndex(), &blink );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( !_3D ) {

    actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  }

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( !buttonPressed ) {

    if ( _3D ) {

    actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x+w, y );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x, y+h );

    actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y+h, x+w, y+h );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w, y, x+w, y+h );

    // top
    actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x+1, y+1, x+w-1, y+1 );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x+2, y+2, x+w-2, y+2 );

    // left
    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x+1, y+1, x+1, y+h-1 );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x+2, y+2, x+2, y+h-2 );

    // bottom
    actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

    // right
    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

    }

  }
  else {

    if ( _3D ) {

    actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x+w, y );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, x, y+h );

    // top

    actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

    // bottom

    actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y+h, x+w, y+h );

    //right

    actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

    XDrawLine( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x+w, y, x+w, y+h );

    }

  }

  actWin->executeGc.setFG( fgColor.getIndex(), &blink );

  //XDrawLine( actWin->d, drawable(actWin->executeWidget),
  // actWin->executeGc.normGC(), x+5, y+9, x+w-5, y+9 );

  if ( fs ) {

    if ( label.getExpanded() )
      strncpy( string, label.getExpanded(), 39 );
    else
      strncpy( string, "", 39 );

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->executeWidget, drawable(actWin->executeWidget),
     &actWin->executeGc, fs, tX, tY, XmALIGNMENT_CENTER, string );

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeRampButtonClass::activate (
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

      connection.init();

      initEnable();

      needConnectInit = needFinalConnectInit = needRampStateConnectInit =
       needCtlInfoInit = 
       needRefresh = needErase = needDraw = needVisConnectInit =
       needVisInit = needVisUpdate = needColorConnectInit =
       needColorInit = needColorUpdate = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      init = 0;
      aglPtr = ptr;
      incrementTimer = 0;
      incrementTimerActive = 0;
      rampFinalV = 0;
      destPvId = visPvId = colorPvId = finalPvId = rampStatePvId = NULL;
      initialConnection = initialFinalValueConnection =
       initialRampStateValueConnection = initialVisConnection =
       initialColorConnection = -1;

      active = buttonPressed = 0;
      activeMode = 1;

      if ( updateRate < 0.1 ) updateRate = 0.1;
      if ( updateRate > 10.0 ) updateRate = 10.0;

      incrementTimerValue = (int) ( 1000.0 * updateRate );
      if ( incrementTimerValue < 100 ) incrementTimerValue = 100;

      if ( !destPvExpString.getExpanded() ||
         blankOrComment( destPvExpString.getExpanded() ) ) {
        destExists = 0;
      }
      else {
        destExists = 1;
        connection.addPv();
      }

      if ( !visPvExpString.getExpanded() ||
         blankOrComment( visPvExpString.getExpanded() ) ) {
        visExists = 0;
        visibility = 1;
      }
      else {
        visExists = 1;
        connection.addPv();
      }

      if ( !colorPvExpString.getExpanded() ||
         blankOrComment( colorPvExpString.getExpanded() ) ) {
        colorExists = 0;
      }
      else {
        colorExists = 1;
        connection.addPv();
      }

      if ( !finalPvExpString.getExpanded() ||
         blankOrComment( finalPvExpString.getExpanded() ) ) {
        finalExists = 0;
      }
      else {
        finalExists = 1;
        connection.addPv();
      }

      if ( !rampStatePvExpString.getExpanded() ||
         blankOrComment( rampStatePvExpString.getExpanded() ) ) {
        rampStateExists = 0;
      }
      else {
        rampStateExists = 1;
        connection.addPv();
      }

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, rbtc_unconnectedTimeout, this );
      }

      opStat = 1;

      if ( destExists ) {

	destPvId = the_PV_Factory->create( destPvExpString.getExpanded() );
	if ( destPvId ) {
	  destPvId->add_conn_state_callback( rbtc_monitor_dest_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeRampButtonClass_str20 );
          opStat = 0;
        }

      }
      else {

        init = 1;
        smartDrawAllActive();

      }

      if ( visExists ) {

	visPvId = the_PV_Factory->create( visPvExpString.getExpanded() );
	if ( visPvId ) {
	  visPvId->add_conn_state_callback( rbtc_monitor_vis_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeRampButtonClass_str20 );
          opStat = 0;
        }

      }

      if ( colorExists ) {

	colorPvId = the_PV_Factory->create( colorPvExpString.getExpanded() );
	if ( colorPvId ) {
	  colorPvId->add_conn_state_callback(
           rbtc_monitor_color_connect_state, this );
	}
	else {
          fprintf( stderr, activeRampButtonClass_str20 );
          opStat = 0;
        }

      }

      if ( finalExists ) {

	finalPvId = the_PV_Factory->create( finalPvExpString.getExpanded() );
	if ( finalPvId ) {
	  finalPvId->add_conn_state_callback( rbtc_monitor_final_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeRampButtonClass_str20 );
          opStat = 0;
        }

      }

      if ( rampStateExists ) {

	rampStatePvId = the_PV_Factory->create( rampStatePvExpString.getExpanded() );
	if ( rampStatePvId ) {
	  rampStatePvId->add_conn_state_callback( rbtc_monitor_rampState_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeRampButtonClass_str20 );
          opStat = 0;
        }

      }

      if ( opStat & 1 ) opComplete = 1;

      return opStat;

    }

    break;

  case 3:
  case 4:
  case 5:
  case 6:

    break;

  }

  return 1;

}

int activeRampButtonClass::deactivate (
  int pass
) {

  if ( pass == 1 ) {

  active = 0;
  activeMode = 0;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  if ( incrementTimerActive ) {
    if ( incrementTimer ) {
      actWin->appCtx->postMessage( activeRampButtonClass_str35 );
      XtRemoveTimeOut( incrementTimer );
      incrementTimer = 0;
    }
    incrementTimerActive = 0;
  }

  if ( destExists ) {
    if ( destPvId ) {
      destPvId->remove_conn_state_callback( rbtc_monitor_dest_connect_state,
       this );
      destPvId->remove_value_callback( rbtc_controlUpdate, this );
      destPvId->release();
      destPvId = NULL;
    }
  }

  if ( visExists ) {
    if ( visPvId ) {
      visPvId->remove_conn_state_callback( rbtc_monitor_vis_connect_state,
       this );
      visPvId->remove_value_callback( rbtc_visUpdate, this );
      visPvId->release();
      visPvId = NULL;
    }
  }

  if ( colorExists ) {
    if ( colorPvId ) {
      colorPvId->remove_conn_state_callback( rbtc_monitor_color_connect_state,
       this );
      colorPvId->remove_value_callback( rbtc_colorUpdate, this );
      colorPvId->release();
      colorPvId = NULL;
    }
  }

  if ( finalExists ) {
    if ( finalPvId ) {
      finalPvId->remove_conn_state_callback( rbtc_monitor_final_connect_state,
       this );
      finalPvId->remove_value_callback( rbtc_finalUpdate, this );
      finalPvId->release();
      finalPvId = NULL;
    }
  }

  if ( rampStateExists ) {
    if ( rampStatePvId ) {
      if ( rampStateExists ) {
        rampStatePvId->put(
         XDisplayName(actWin->appCtx->displayName),
         0 );
      }
      rampStatePvId->remove_conn_state_callback( rbtc_monitor_rampState_connect_state,
       this );
      rampStatePvId->release();
      rampStatePvId = NULL;
    }
  }

  }

  return 1;

}

void activeRampButtonClass::updateDimensions ( void )
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

}

void activeRampButtonClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  return;

}

void activeRampButtonClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

double dval;

  *action = 0;

  gettimeofday( &baseTime, NULL );

  if ( !enabled || !init || !visibility ) return;

  if ( !destPvId->have_write_access() ) return;

  if ( buttonPressed ) {

    if ( incrementTimerActive ) {
      if ( incrementTimer ) {
        XtRemoveTimeOut( incrementTimer );
        incrementTimer = 0;
      }
      incrementTimerActive = 0;
    }

    buttonPressed = 0;

    if ( rampStateExists ) {
      rampStatePvId->put(
       XDisplayName(actWin->appCtx->displayName),
       buttonPressed );
    }

    actWin->appCtx->proc->lock();
    needRefresh = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();

    return;

  }

  buttonPressed = 1;

  if ( rampStateExists ) {
    rampStatePvId->put(
     XDisplayName(actWin->appCtx->displayName),
     buttonPressed );
  }

  actWin->appCtx->proc->lock();
  dval = curControlV;
  rampFinalV = curFinalV;
  needRefresh = 1;
  actWin->addDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  //fprintf( stderr, "btn down, x=%-d, y=%-d, bn=%-d\n", _x-x, _y-y , buttonNumber );
  //fprintf( stderr, "cv=%g, fv=%g\n", dval, rampFinalV );

  if ( dval < minDv ) {
    dval = minDv;
  }
  else if ( dval > maxDv ) {
    dval = maxDv;
  }

  if ( updateRate < 0.1 ) updateRate = 0.1;
  if ( updateRate > 10.0 ) updateRate = 10.0;
  increment = fabs( rampRate / 60 * updateRate );

  //fprintf( stderr, "rampRate=%g\n", rampRate );
  //fprintf( stderr, "updateRate=%g\n", updateRate );
  //fprintf( stderr, "increment=%g\n", increment );

  if ( rampFinalV > dval ) {
    incrementTimer = appAddTimeOut( actWin->appCtx->appContext(),
     incrementTimerValue, rbtc_increment, this );
    incrementTimerActive = 1;
  }
  else if ( rampFinalV < dval ) {
    incrementTimer = appAddTimeOut( actWin->appCtx->appContext(),
     incrementTimerValue, rbtc_decrement, this );
    incrementTimerActive = 1;
  }
  else {
    incrementTimerActive = 0;
    buttonPressed = 0;
    if ( rampStateExists ) {
      rampStatePvId->put(
       XDisplayName(actWin->appCtx->displayName),
       buttonPressed );
    }
    actWin->appCtx->proc->lock();
    needRefresh = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
  }

}

void activeRampButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled || !init || !visibility ) return;

  if ( !destPvId->have_write_access() ) {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );
  }
  else {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_DEFAULT );
  }

  activeGraphicClass::pointerIn( _x, _y, buttonState );

}

int activeRampButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;

  if ( destExists && finalExists )
    *focus = 1;
  else
    *focus = 0;

  if ( !destExists || !finalExists ) {
    *up = 0;
    *down = 0;
    return 1;
  }

  *down = 1;
  *up = 1;

  return 1;

}

int activeRampButtonClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( destPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  destPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( finalPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  finalPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( rampStatePvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  rampStatePvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( label.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  label.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( visPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  visPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( colorPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  colorPvExpString.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeRampButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = destPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = finalPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = rampStatePvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = label.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeRampButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = destPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = finalPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = rampStatePvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = label.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return stat;

}

int activeRampButtonClass::containsMacros ( void ) {

  if ( destPvExpString.containsPrimaryMacros() ) return 1;

  if ( finalPvExpString.containsPrimaryMacros() ) return 1;

  if ( rampStatePvExpString.containsPrimaryMacros() ) return 1;

  if ( label.containsPrimaryMacros() ) return 1;

  if ( visPvExpString.containsPrimaryMacros() ) return 1;

  if ( colorPvExpString.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeRampButtonClass::executeDeferred ( void ) {

int nc, nsc, nci, nd, ne, nr, nvc, nvi, nvu, ncolc, ncoli, ncolu, nrsc;
int stat, index, invisColor;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nsc = needFinalConnectInit; needFinalConnectInit = 0;
  nrsc = needRampStateConnectInit; needRampStateConnectInit = 0;
  nci = needCtlInfoInit; needCtlInfoInit = 0;
  nd = needDraw; needDraw = 0;
  ne = needErase; needErase = 0;
  nr = needRefresh; needRefresh = 0;
  nvc = needVisConnectInit; needVisConnectInit = 0;
  nvi = needVisInit; needVisInit = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  ncolc = needColorConnectInit; needColorConnectInit = 0;
  ncoli = needColorInit; needColorInit = 0;
  ncolu = needColorUpdate; needColorUpdate = 0;
  visValue = curVisValue;
  colorValue = curColorValue;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    connection.setPvConnected( (void *) destPvConnection );
    destType = (int) destPvId->get_type().type;

    if ( limitsFromDb || efScaleMin.isNull() ) {
      scaleMin = destPvId->get_lower_disp_limit();
    }

    if ( limitsFromDb || efScaleMax.isNull() ) {
      scaleMax = destPvId->get_upper_disp_limit();
    }

    minDv = scaleMin;

    maxDv = scaleMax;

    curControlV = destPvId->get_double();

    nci = 1;

  }

  if ( nci ) {

    if ( initialConnection ) {

      initialConnection = 0;

      destPvId->add_value_callback( rbtc_controlUpdate, this );

    }

    if ( connection.pvsConnected() ) {
      bgColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

  if ( nsc ) {

    connection.setPvConnected( (void *) finalPvConnection );
    finalType = (int) finalPvId->get_type().type;

    if ( initialFinalValueConnection ) {

      initialFinalValueConnection = 0;

      finalPvId->add_value_callback( rbtc_finalUpdate, this );

    }

    if ( connection.pvsConnected() ) {
      bgColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

  if ( nrsc ) {

    connection.setPvConnected( (void *) rampStatePvConnection );
    rampStateType = (int) rampStatePvId->get_type().type;

    if ( initialRampStateValueConnection ) {

      initialRampStateValueConnection = 0;

      if ( rampStateExists ) {
        rampStatePvId->put(
         XDisplayName(actWin->appCtx->displayName),
         0 );
      }

    }

    if ( connection.pvsConnected() ) {
      bgColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

  if ( nvc ) {

    minVis = atof( minVisString );
    maxVis = atof( maxVisString );

    connection.setPvConnected( (void *) visPvConnection );

    visValue = curVisValue = visPvId->get_double();

    nvi = 1;

  }

  if ( nvi ) {

    if ( initialVisConnection ) {

      initialVisConnection = 0;

      visPvId->add_value_callback( rbtc_visUpdate, this );

    }

    if ( ( visValue >= minVis ) &&
         ( visValue < maxVis ) )
      visibility = 1 ^ visInverted;
    else
      visibility = 0 ^ visInverted;

    if ( prevVisibility != visibility ) {
      if ( !visibility ) eraseActive();
    }

    if ( connection.pvsConnected() ) {
      bgColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

  if ( ncolc ) {

    colorValue = curColorValue = colorPvId->get_double();

    ncoli = 1;

  }

  if ( ncoli ) {

    if ( initialColorConnection ) {

      initialColorConnection = 0;

      colorPvId->add_value_callback( rbtc_colorUpdate, this );

    }

    invisColor = 0;

    index = actWin->ci->evalRule( bgColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    bgColor.changeIndex( index, actWin->ci );

    index = actWin->ci->evalRule( fgColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    fgColor.changeIndex( index, actWin->ci );

    if ( !visExists ) {

      if ( invisColor ) {
        visibility = 0;
      }
      else {
        visibility = 1;
      }

      if ( prevVisibility != visibility ) {
        if ( !visibility ) eraseActive();
      }

    }

    connection.setPvConnected( (void *) colorPvConnection );

    if ( connection.pvsConnected() ) {
      bgColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

//----------------------------------------------------------------------------

  if ( nd ) {

    smartDrawAllActive();

  }

//----------------------------------------------------------------------------

  if ( ne ) {

    eraseActive();

  }

//----------------------------------------------------------------------------

  if ( nr ) {

    eraseActive();
    smartDrawAllActive();

  }

//----------------------------------------------------------------------------

  if ( nvu ) {

    if ( ( visValue >= minVis ) &&
         ( visValue < maxVis ) )
      visibility = 1 ^ visInverted;
    else
      visibility = 0 ^ visInverted;

    if ( prevVisibility != visibility ) {
      if ( !visibility ) eraseActive();
      stat = smartDrawAllActive();
    }

  }

//----------------------------------------------------------------------------

  if ( ncolu ) {

    invisColor = 0;

    index = actWin->ci->evalRule( bgColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    bgColor.changeIndex( index, actWin->ci );

    index = actWin->ci->evalRule( fgColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    fgColor.changeIndex( index, actWin->ci );

    if ( !visExists ) {

      if ( invisColor ) {
        visibility = 0;
      }
      else {
        visibility = 1;
      }

      if ( prevVisibility != visibility ) {
        if ( !visibility ) eraseActive();
      }

    }

    stat = smartDrawAllActive();

  }

}

char *activeRampButtonClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeRampButtonClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeRampButtonClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( i == 0 ) {
      return destPvExpString.getExpanded();
    }
    else if ( i == 1 ) {
      return finalPvExpString.getExpanded();
    }
    else if ( i == 2 ) {
      return rampStatePvExpString.getExpanded();
    }
    else if ( i == 3 ) {
      return colorPvExpString.getExpanded();
    }
    else {
      return visPvExpString.getExpanded();
    }

  }
  else {

    if ( i == 0 ) {
      return destPvExpString.getRaw();
    }
    else if ( i == 1 ) {
      return finalPvExpString.getRaw();
    }
    else if ( i == 2 ) {
      return rampStatePvExpString.getRaw();

    }
    else if ( i == 3 ) {
      return colorPvExpString.getRaw();
    }
    else {
      return visPvExpString.getRaw();
    }

  }

}

void activeRampButtonClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topShadowColor = _topShadowColor;

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botShadowColor = _botShadowColor;

  if ( _flag & ACTGRF_BTNFONTTAG_MASK ) {
    strncpy( fontTag, _btnFontTag, 63 );
    fontTag[63] = 0;
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    updateDimensions();
  }

}

void activeRampButtonClass::changePvNames (
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
      destPvExpString.setRaw( ctlPvs[0] );
    }
  }

  if ( flag & ACTGRF_VISPVS_MASK ) {
    if ( numVisPvs ) {
      visPvExpString.setRaw( ctlPvs[0] );
    }
  }

}

void activeRampButtonClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 2 ) {
    *n = 0;
    return;
  }

  *n = 2;
  pvs[0] = destPvId;
  pvs[1] = finalPvId;

}

char *activeRampButtonClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return destPvExpString.getRaw();
  }
  else if ( i == 1 ) {
    return finalPvExpString.getRaw();
  }
  else if ( i == 2 ) {
    return rampStatePvExpString.getRaw();
  }
  else if ( i == 3 ) {
    return colorPvExpString.getRaw();
  }
  else if ( i == 4 ) {
    return visPvExpString.getRaw();
  }
  else if ( i == 5 ) {
    return label.getRaw();
  }
  else if ( i == 6 ) {
    return minVisString;
  }
  else if ( i == 7 ) {
    return maxVisString;
  }

  return NULL;

}

void activeRampButtonClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    destPvExpString.setRaw( string );
  }
  else if ( i == 1 ) {
    finalPvExpString.setRaw( string );
  }
  else if ( i == 2 ) {
    rampStatePvExpString.setRaw( string );
  }
  else if ( i == 3 ) {
    colorPvExpString.setRaw( string );
  }
  else if ( i == 4 ) {
    visPvExpString.setRaw( string );
  }
  else if ( i == 5 ) {
    label.setRaw( string );
  }
  else if ( i == 6 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( minVisString, string, l );
    minVisString[l] = 0;
  }
  else if ( i == 7 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( maxVisString, string, l );
    maxVisString[l] = 0;
  }

}

// crawler functions may return blank pv names
char *activeRampButtonClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return destPvExpString.getExpanded();

}

char *activeRampButtonClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >=5 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return finalPvExpString.getExpanded();
  }
  if ( crawlerPvIndex == 2 ) {
    return rampStatePvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 3 ) {
    return visPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 4 ) {
    return colorPvExpString.getExpanded();
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeRampButtonClassPtr ( void ) {

activeRampButtonClass *ptr;

  ptr = new activeRampButtonClass;
  return (void *) ptr;

}

void *clone_activeRampButtonClassPtr (
  void *_srcPtr )
{

activeRampButtonClass *ptr, *srcPtr;

  srcPtr = (activeRampButtonClass *) _srcPtr;

  ptr = new activeRampButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
