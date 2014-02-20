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

#define __asignal_cc 1

#include "asignal.h"
#include "app_pkg.h"
#include "act_win.h"

static void sigc_doBlink (
  void *ptr
) {

activeSignalClass *sigo = (activeSignalClass *) ptr;

  if ( !sigo->activeMode ) {
    if ( sigo->isSelected() ) sigo->drawSelectBoxCorners(); //erase via xor
    sigo->smartDrawAll();
    if ( sigo->isSelected() ) sigo->drawSelectBoxCorners();
  }
  else {
    sigo->bufInvalidate();
    sigo->needDraw = 1;
    sigo->actWin->addDefExeNode( sigo->aglPtr );
  }

}

static void sigc_unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeSignalClass *sigo = (activeSignalClass *) client;

  if ( !sigo->init ) {
    sigo->needToDrawUnconnected = 1;
    sigo->needDraw = 1;
    sigo->actWin->addDefExeNode( sigo->aglPtr );
  }

  sigo->unconnectedTimer = 0;

}

static void sigc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSignalClass *sigo = (activeSignalClass *) client;

  sigo->actWin->setChanged();

  sigo->eraseSelectBoxCorners();
  sigo->erase();

  sigo->fgColor.setColorIndex( sigo->eBuf->bufFgColor, sigo->actWin->ci );

  sigo->bgColor.setColorIndex( sigo->eBuf->bufBgColor, sigo->actWin->ci );

  sigo->topShadowColor = sigo->eBuf->bufTopShadowColor;
  sigo->botShadowColor = sigo->eBuf->bufBotShadowColor;

  sigo->destPvExpString.setRaw( sigo->eBuf->bufDestPvName );

  sigo->signalStatePvExpString.setRaw( sigo->eBuf->bufSignalStatePvName );

  sigo->label.setRaw( sigo->eBuf->bufLabel );

  strncpy( sigo->fontTag, sigo->fm.currentFontTag(), 63 );
  sigo->actWin->fi->loadFontTag( sigo->fontTag );
  sigo->fs = sigo->actWin->fi->getXFontStruct( sigo->fontTag );

  sigo->_3D = sigo->eBuf->buf3D;

  sigo->invisible = sigo->eBuf->bufInvisible;

  sigo->updateRate = sigo->eBuf->bufUpdateRate;
  if ( sigo->updateRate < 0.1 ) sigo->updateRate = 0.1;
  if ( sigo->updateRate > 10.0 ) sigo->updateRate = 10.0;

  sigo->amplPvExpString.setRaw( sigo->eBuf->bufAmplPvName );

  sigo->signalAmplitude = sigo->eBuf->bufSignalAmplitude;

  sigo->freqPvExpString.setRaw( sigo->eBuf->bufFreqPvName );

  sigo->signalFrequency = sigo->eBuf->bufSignalFrequency;

  sigo->phasePvExpString.setRaw( sigo->eBuf->bufPhasePvName );

  sigo->signalPhase = sigo->eBuf->bufSignalPhase;

  sigo->offsetPvExpString.setRaw( sigo->eBuf->bufOffsetPvName );

  sigo->signalOffset = sigo->eBuf->bufSignalOffset;

  sigo->signalType = sigo->eBuf->bufSignalType;

  sigo->limitsFromDb = sigo->eBuf->bufLimitsFromDb;

  sigo->efScaleMin = sigo->eBuf->bufEfScaleMin;
  sigo->efScaleMax = sigo->eBuf->bufEfScaleMax;

  sigo->minDv = sigo->scaleMin = sigo->efScaleMin.value();
  sigo->maxDv = sigo->scaleMax = sigo->efScaleMax.value();

  sigo->visPvExpString.setRaw( sigo->eBuf->bufVisPvName );
  strncpy( sigo->minVisString, sigo->eBuf->bufMinVisString, 39 );
  strncpy( sigo->maxVisString, sigo->eBuf->bufMaxVisString, 39 );

  if ( sigo->eBuf->bufVisInverted )
    sigo->visInverted = 0;
  else
    sigo->visInverted = 1;

  sigo->colorPvExpString.setRaw( sigo->eBuf->bufColorPvName );

  sigo->x = sigo->eBuf->bufX;
  sigo->sboxX = sigo->eBuf->bufX;

  sigo->y = sigo->eBuf->bufY;
  sigo->sboxY = sigo->eBuf->bufY;

  sigo->w = sigo->eBuf->bufW;
  sigo->sboxW = sigo->eBuf->bufW;

  sigo->h = sigo->eBuf->bufH;
  sigo->sboxH = sigo->eBuf->bufH;

  sigo->updateDimensions();

}

static void sigc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSignalClass *sigo = (activeSignalClass *) client;

  sigc_edit_update ( w, client, call );
  sigo->refresh( sigo );

}

static void sigc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSignalClass *sigo = (activeSignalClass *) client;

  sigc_edit_update ( w, client, call );
  sigo->ef.popdown();
  sigo->operationComplete();

}

static void sigc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSignalClass *sigo = (activeSignalClass *) client;

  sigo->ef.popdown();
  sigo->operationCancel();

}

static void sigc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSignalClass *sigo = (activeSignalClass *) client;

  sigo->ef.popdown();
  sigo->operationCancel();
  sigo->erase();
  sigo->deleteRequest = 1;
  sigo->drawAll();

}

static void sigc_monitor_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

PvCallbackClass *pco = (PvCallbackClass *) userarg;
activeSignalClass *sigo = (activeSignalClass *) pco->getUserArg();

  sigo->actWin->appCtx->proc->lock();

  if ( !pv->is_valid() ) {
    sigo->active = 0;
    sigo->bgColor.setDisconnected();
    sigo->needDraw = 1;
  }

  if ( pco->getId() == activeSignalClass::destPvConnectionId ) {
    if ( pv->is_valid() ) {
      sigo->needConnectInit = 1;
    }
  }
  else if ( pco->getId() == activeSignalClass::signalStatePvConnectionId ) {
    if ( pv->is_valid() ) {
      sigo->needSignalStateConnectInit = 1;
    }
  }
  else if ( pco->getId() == activeSignalClass::visPvConnectionId ) {
    if ( pv->is_valid() ) {
      sigo->needVisConnectInit = 1;
    }
  }
  else if ( pco->getId() == activeSignalClass::colorPvConnectionId ) {
    if ( pv->is_valid() ) {
      sigo->needColorConnectInit = 1;
    }
  }
  else if ( pco->getId() == activeSignalClass::amplPvConnectionId ) {
    if ( pv->is_valid() ) {
      sigo->needAmplConnectInit = 1;
    }
  }
  else if ( pco->getId() == activeSignalClass::offsetPvConnectionId ) {
    if ( pv->is_valid() ) {
      sigo->needOffsetConnectInit = 1;
    }
  }
  else if ( pco->getId() == activeSignalClass::freqPvConnectionId ) {
    if ( pv->is_valid() ) {
      sigo->needFreqConnectInit = 1;
    }
  }
  else if ( pco->getId() == activeSignalClass::phasePvConnectionId ) {
    if ( pv->is_valid() ) {
      sigo->needPhaseConnectInit = 1;
    }
  }

  sigo->actWin->addDefExeNode( sigo->aglPtr );

  sigo->actWin->appCtx->proc->unlock();

}

static void sigc_update (
  ProcessVariable *pv,
  void *userarg ) {

PvCallbackClass *pco = (PvCallbackClass *) userarg;
activeSignalClass *sigo = (activeSignalClass *) pco->getUserArg();

  sigo->actWin->appCtx->proc->lock();

  if ( pco->getId() == activeSignalClass::destPvConnectionId ) {
    sigo->curControlV = pv->get_double();
  }
  else if ( pco->getId() == activeSignalClass::visPvConnectionId ) {
    sigo->curVisValue = pv->get_double();
    sigo->needVisUpdate = 1;
  }
  else if ( pco->getId() == activeSignalClass::colorPvConnectionId ) {
    sigo->curColorValue = pv->get_double();
    sigo->needColorUpdate = 1;
  }
  else if ( pco->getId() == activeSignalClass::amplPvConnectionId ) {
    sigo->signalAmplitude = pv->get_double();
  }
  else if ( pco->getId() == activeSignalClass::offsetPvConnectionId ) {
    sigo->signalOffset = pv->get_double();
  }
  else if ( pco->getId() == activeSignalClass::freqPvConnectionId ) {
    sigo->signalFrequency = pv->get_double();
  }
  else if ( pco->getId() == activeSignalClass::phasePvConnectionId ) {
    sigo->signalPhase = pv->get_double();
    sigo->signalPhaseRads = sigo->signalPhase * 0.017453;
  }

  sigo->actWin->addDefExeNode( sigo->aglPtr );

  sigo->actWin->appCtx->proc->unlock();

}

static void sigc_increment (
  XtPointer client,
  XtIntervalId *id )
{

activeSignalClass *sigo = (activeSignalClass *) client;
double dval, div1, div2, remainder1, remainder2, seconds;
struct timeval curTime;

  gettimeofday( &curTime, NULL );
  seconds = curTime.tv_sec - sigo->baseTime.tv_sec +
    ( curTime.tv_usec - sigo->baseTime.tv_usec ) * 0.000001;
  sigo->baseTime = curTime;

  if ( !sigo->incrementTimerActive ) {
    sigo->incrementTimer = 0;
    return;
  }

  sigo->incrementTimer = appAddTimeOut(
   sigo->actWin->appCtx->appContext(),
   sigo->incrementTimerValue, sigc_increment, client );

  sigo->actWin->appCtx->proc->lock();
  dval = sigo->curControlV;
  sigo->actWin->appCtx->proc->unlock();

  sigo->elapsedTime += seconds;


  if ( sigo->signalType == SIGC_K_SINE ) {
    dval = 0.5 * sigo->signalAmplitude *
     sin( 6.283185 * sigo->signalFrequency * sigo->elapsedTime - sigo->signalPhaseRads );
  }
  else if ( sigo->signalType == SIGC_K_SQUARE ) {
    if ( sigo->halfPeriod == 0.0 ) {
      dval = 0.0;
    }
    else {
      dval = sin( 6.283185 * sigo->signalFrequency * sigo->elapsedTime - sigo->signalPhaseRads );
      if ( dval >=0 ) {
        dval = sigo->signalAmplitude * 0.5;
      }
      else {
        dval = sigo->signalAmplitude * -0.5;
      }
    }
  }
  else if ( sigo->signalType == SIGC_K_TRIANGLE ) {
    if ( sigo->halfPeriod == 0.0 ) {
      dval = 0.0;
    }
    else {
      div1 = ( sigo->elapsedTime - sigo->signalPhaseRads / 6.28 * sigo->halfPeriod ) / sigo->halfPeriod;
      remainder1 = div1 - floor( div1 );
      if ( remainder1 <= 0.25 ) {
        dval = remainder1 * 2 * sigo->signalAmplitude;
      }
      else if ( remainder1 <= 0.75 ) {
        dval = sigo->signalAmplitude  * 0.5 - ( remainder1 - 0.25 ) * 2 * sigo->signalAmplitude;
      }
      else {
        dval = ( remainder1 - 0.75 ) * 2 * sigo->signalAmplitude - sigo->signalAmplitude * 0.5;
      }
    }
  }
  else if ( sigo->signalType == SIGC_K_SAWTOOTH ) {
    if ( sigo->halfPeriod == 0.0 ) {
      dval = 0.0;
    }
    else {
      div1 = ( sigo->elapsedTime - sigo->signalPhaseRads / 6.28 * sigo->halfPeriod ) / sigo->halfPeriod;
      remainder1 = div1 - floor( div1 );
      dval = remainder1 * sigo->signalAmplitude - sigo->signalAmplitude * 0.5;
    }
  }
  else { // SIGC_K_IMPULSE
    if ( sigo->halfPeriod == 0.0 ) {
      dval = 0.0;
    }
    else {
      dval = sin( 6.283185 * sigo->signalFrequency * sigo->elapsedTime - sigo->signalPhaseRads );
      if ( dval >=0 ) {
        if ( sigo->firstImpulse ) {
          sigo->firstImpulse = 0;
          dval = sigo->signalAmplitude * 0.5;
        }
        else {
          dval = sigo->signalAmplitude * -0.5;
        }
      }
      else {
        dval = sigo->signalAmplitude * -0.5;
        sigo->firstImpulse = 1;
      }
    }
  }

  dval += sigo->signalOffset;

  if ( dval <= sigo->minDv ) {
    dval = sigo->minDv;
  }
  else if ( dval >= sigo->maxDv ) {
    dval = sigo->maxDv;
  }

  if ( sigo->destExists ) {
    sigo->destPvId->put(
     XDisplayName(sigo->actWin->appCtx->displayName),
     dval );
  }

}

activeSignalClass::activeSignalClass ( void ) {

  name = new char[strlen("activeSignalClass")+1];
  strcpy( name, "activeSignalClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  buttonPressed = 0;
  state = SIGC_IDLE;
  _3D = 1;
  invisible = 0;
  updateRate = 0.5;
  signalAmplitude = 1.0;
  signalFrequency = 1.0;
  signalPhase = 0.0;
  signalOffset = 0.0;
  signalType = SIGC_K_SINE;
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
  connection.setMaxPvs( 8 );
  activeMode = 0;
  eBuf = NULL;
  destPvCb = signalStatePvCb = visPvCb = colorPvCb = amplPvCb =
   offsetPvCb = freqPvCb = phasePvCb = NULL;

  setBlinkFunction( (void *) sigc_doBlink );

}

// copy constructor
activeSignalClass::activeSignalClass
 ( const activeSignalClass *source ) {

activeGraphicClass *sigo = (activeGraphicClass *) this;

  sigo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeSignalClass")+1];
  strcpy( name, "activeSignalClass" );

  buttonPressed = 0;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  destPvExpString.copy( source->destPvExpString );
  signalStatePvExpString.copy( source->signalStatePvExpString );
  visPvExpString.copy( source->visPvExpString );
  colorPvExpString.copy( source->colorPvExpString );
  amplPvExpString.copy( source->amplPvExpString );
  offsetPvExpString.copy( source->offsetPvExpString );
  freqPvExpString.copy( source->freqPvExpString );
  phasePvExpString.copy( source->phasePvExpString );

  label.copy( source->label );

  state = SIGC_IDLE;
  _3D = source->_3D;
  invisible = source->invisible;
  updateRate = source->updateRate;
  signalAmplitude = source->signalAmplitude;
  signalFrequency = source->signalFrequency;
  signalPhase = source->signalPhase;
  signalOffset = source->signalOffset;
  signalType = source->signalType;
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
  destPvCb = signalStatePvCb = visPvCb = colorPvCb = amplPvCb =
   offsetPvCb = freqPvCb = phasePvCb = NULL;

  connection.setMaxPvs( 8 );

  setBlinkFunction( (void *) sigc_doBlink );

  doAccSubs( destPvExpString );
  doAccSubs( signalStatePvExpString );
  doAccSubs( label );
  doAccSubs( colorPvExpString );
  doAccSubs( visPvExpString );
  doAccSubs( amplPvExpString );
  doAccSubs( offsetPvExpString );
  doAccSubs( freqPvExpString );
  doAccSubs( phasePvExpString );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );

  updateDimensions();

}

activeSignalClass::~activeSignalClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

int activeSignalClass::createInteractive (
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

int activeSignalClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

int sigTypeSine = SIGC_K_SINE;
static char *sigTypeEnumStr[5] = {
  "sine",
  "square",
  "triangle",
  "sawtooth",
  "impulse"
};
static int sigTypeEnum[5] = {
  SIGC_K_SINE,
  SIGC_K_SQUARE,
  SIGC_K_TRIANGLE,
  SIGC_K_SAWTOOTH,
  SIGC_K_IMPULSE
};

  major = SIGC_MAJOR_VERSION;
  minor = SIGC_MINOR_VERSION;
  release = SIGC_RELEASE;

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
  tag.loadW( "signalStateValuePv", &signalStatePvExpString, emptyStr );
  tag.loadW( "updateRate", &updateRate, &dzero );
  tag.loadW( "amplitudePv", &amplPvExpString, emptyStr );
  //tag.loadW( "signalAmplitude", &signalAmplitude, &dzero );
  tag.loadW( "frequencyPv", &freqPvExpString, emptyStr );
  //tag.loadW( "signalFrequency", &signalFrequency, &dzero );
  tag.loadW( "phasePv", &phasePvExpString, emptyStr );
  //tag.loadW( "signalPhase", &signalPhase, &dzero );
  tag.loadW( "offsetPv", &offsetPvExpString, emptyStr );
  // tag.loadW( "signalOffset", &signalOffset, &dzero );
  tag.loadW( "signalType", 5, sigTypeEnumStr, sigTypeEnum, &signalType,
   &sigTypeSine );
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

int activeSignalClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

int sigTypeSine = SIGC_K_SINE;
static char *sigTypeEnumStr[5] = {
  "sine",
  "square",
  "triangle",
  "sawtooth",
  "impulse"
};
static int sigTypeEnum[5] = {
  SIGC_K_SINE,
  SIGC_K_SQUARE,
  SIGC_K_TRIANGLE,
  SIGC_K_SAWTOOTH,
  SIGC_K_IMPULSE
};

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
  tag.loadR( "signalStateValuePv", &signalStatePvExpString, emptyStr );
  tag.loadR( "updateRate", &updateRate, &dzero );
  tag.loadR( "amplitudePv", &amplPvExpString, emptyStr );
  //tag.loadR( "signalAmplitude", &signalAmplitude, &dzero );
  tag.loadR( "frequencyPv", &freqPvExpString, emptyStr );
  //tag.loadR( "signalFrequency", &signalFrequency, &dzero );
  tag.loadR( "phasePv", &phasePvExpString, emptyStr );
  //tag.loadR( "signalPhase", &signalPhase, &dzero );
  tag.loadR( "offsetPv", &offsetPvExpString, emptyStr );
  //tag.loadR( "signalOffset", &signalOffset, &dzero );
  tag.loadR( "signalType", 4, sigTypeEnumStr, sigTypeEnum, &signalType,
   &sigTypeSine );
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

  if ( major > SIGC_MAJOR_VERSION ) {
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

int activeSignalClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeSignalClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeSignalClass_str2, 31 );

  Strncat( title, activeSignalClass_str3, 31 );

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

  if ( signalStatePvExpString.getRaw() )
    strncpy( eBuf->bufSignalStatePvName, signalStatePvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufSignalStatePvName, "" );

  if ( amplPvExpString.getRaw() )
    strncpy( eBuf->bufAmplPvName, amplPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufAmplPvName, "" );

  if ( offsetPvExpString.getRaw() )
    strncpy( eBuf->bufOffsetPvName, offsetPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufOffsetPvName, "" );

  if ( freqPvExpString.getRaw() )
    strncpy( eBuf->bufFreqPvName, freqPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufFreqPvName, "" );

  if ( phasePvExpString.getRaw() )
    strncpy( eBuf->bufPhasePvName, phasePvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufPhasePvName, "" );

  eBuf->bufUpdateRate = updateRate;

  eBuf->bufSignalAmplitude = signalAmplitude;

  eBuf->bufSignalFrequency = signalFrequency;

  eBuf->bufSignalPhase = signalPhase;

  eBuf->bufSignalOffset = signalOffset;

  eBuf->bufSignalType = signalType;

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

  ef.addTextField( activeSignalClass_str4, 35, &eBuf->bufX );
  ef.addTextField( activeSignalClass_str5, 35, &eBuf->bufY );
  ef.addTextField( activeSignalClass_str6, 35, &eBuf->bufW );
  ef.addTextField( activeSignalClass_str7, 35, &eBuf->bufH );
  ef.addTextField( activeSignalClass_str8, 35, eBuf->bufDestPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeSignalClass_str34, 35, eBuf->bufSignalStatePvName,
   PV_Factory::MAX_PV_NAME );

  ef.addToggle( activeSignalClass_str26, &eBuf->bufLimitsFromDb );
  limitsFromDbEntry = ef.getCurItem();
  ef.addTextField( activeSignalClass_str27, 35, &eBuf->bufEfScaleMin );
  minEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( minEntry );
  ef.addTextField( activeSignalClass_str28, 35, &eBuf->bufEfScaleMax );
  maxEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( maxEntry );
  limitsFromDbEntry->addDependencyCallbacks();
  ef.addOption( activeSignalClass_str36, activeSignalClass_str41, &eBuf->bufSignalType );
  ef.addTextField( activeSignalClass_str37, 35, eBuf->bufAmplPvName,
   PV_Factory::MAX_PV_NAME );
  //ef.addTextField( activeSignalClass_str37, 35, &eBuf->bufSignalAmplitude );
  ef.addTextField( activeSignalClass_str38, 35, eBuf->bufFreqPvName,
   PV_Factory::MAX_PV_NAME );
  //ef.addTextField( activeSignalClass_str38, 35, &eBuf->bufSignalFrequency );
  ef.addTextField( activeSignalClass_str39, 35, eBuf->bufPhasePvName,
   PV_Factory::MAX_PV_NAME );
  //ef.addTextField( activeSignalClass_str39, 35, &eBuf->bufSignalPhase );
  ef.addTextField( activeSignalClass_str40, 35, eBuf->bufOffsetPvName,
   PV_Factory::MAX_PV_NAME );
  //ef.addTextField( activeSignalClass_str40, 35, &eBuf->bufSignalOffset );
  ef.addTextField( activeSignalClass_str11, 35, &eBuf->bufUpdateRate );
  ef.addToggle( activeSignalClass_str12, &eBuf->buf3D );
  ef.addToggle( activeSignalClass_str13, &eBuf->bufInvisible );
  ef.addTextField( activeSignalClass_str14, 35, eBuf->bufLabel, 39 );
  ef.addColorButton( activeSignalClass_str16, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addColorButton( activeSignalClass_str17, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  ef.addColorButton( activeSignalClass_str18, actWin->ci, &eBuf->topShadowCb, &eBuf->bufTopShadowColor );
  ef.addColorButton( activeSignalClass_str19, actWin->ci, &eBuf->botShadowCb, &eBuf->bufBotShadowColor );

  ef.addFontMenu( activeSignalClass_str15, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeSignalClass_str33, 30, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeSignalClass_str29, 30, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeSignalClass_str30, &eBuf->bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeSignalClass_str31, 30, eBuf->bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeSignalClass_str32, 30, eBuf->bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  return 1;

}

int activeSignalClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( sigc_edit_ok, sigc_edit_apply, sigc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeSignalClass::edit ( void ) {

  this->genericEdit();
  ef.finished( sigc_edit_ok, sigc_edit_apply, sigc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeSignalClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeSignalClass::eraseActive ( void ) {

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

int activeSignalClass::draw ( void ) {

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

int activeSignalClass::drawActive ( void ) {

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

int activeSignalClass::activate (
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

      needConnectInit = needSignalStateConnectInit = needAmplConnectInit =
       needOffsetConnectInit = needFreqConnectInit = needPhaseConnectInit =
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
      destPvId = visPvId = colorPvId = signalStatePvId = amplPvId =
       offsetPvId = freqPvId = phasePvId = NULL;
      initialSignalStateValueConnection = -1;

      active = buttonPressed = 0;
      activeMode = 1;
      elapsedTime = 0.0;
      wfVal = 0.0;
      if ( signalFrequency != 0.0 ) {
        halfPeriod = 1.0 / signalFrequency;
        wfInc = 0.25 * signalAmplitude * signalFrequency;
      }
      else {
        halfPeriod = 0.0;
      }
      signalPhaseRads = signalPhase * 0.017453;
      firstImpulse = 1;

      if ( updateRate < 0.1 ) updateRate = 0.1;
      if ( updateRate > 10.0 ) updateRate = 10.0;

      incrementTimerValue = (int) ( 1000.0 * updateRate );
      if ( incrementTimerValue < 100 ) incrementTimerValue = 100;

      // dest pv
      if ( !destPvCb ) {
        destPvCb = new PvCallbackClass(
         destPvExpString, &connection, destPvConnectionId, this,
         sigc_monitor_connect_state, sigc_update );
      }
      destExists = destPvCb->getPvExists();
      destPvId = destPvCb->getPv();
      if ( destExists && !destPvId ) {
        fprintf( stderr, activeSignalClass_str20 );
      }

      // signal state pv
      if ( !signalStatePvCb ) {
        signalStatePvCb = new PvCallbackClass(
         signalStatePvExpString, &connection, signalStatePvConnectionId, this,
         sigc_monitor_connect_state, sigc_update );
      }
      signalStateExists = signalStatePvCb->getPvExists();
      signalStatePvId = signalStatePvCb->getPv();
      if ( signalStateExists && !signalStatePvId ) {
        fprintf( stderr, activeSignalClass_str20 );
      }

      // amplitude pv
      if ( !amplPvCb ) {
        amplPvCb = new PvCallbackClass(
         amplPvExpString, &connection, amplPvConnectionId, this,
         sigc_monitor_connect_state, sigc_update );
      }
      amplExists = amplPvCb->getPvExists();
      amplPvId = amplPvCb->getPv();
      if ( amplExists && !amplPvId ) {
        fprintf( stderr, activeSignalClass_str20 );
      }

      // offset pv
      if ( !offsetPvCb ) {
        offsetPvCb = new PvCallbackClass(
         offsetPvExpString, &connection, offsetPvConnectionId, this,
         sigc_monitor_connect_state, sigc_update );
      }
      offsetExists = offsetPvCb->getPvExists();
      offsetPvId = offsetPvCb->getPv();
      if ( offsetExists && !offsetPvId ) {
        fprintf( stderr, activeSignalClass_str20 );
      }

      // frequency pv
      if ( !freqPvCb ) {
        freqPvCb = new PvCallbackClass(
         freqPvExpString, &connection, freqPvConnectionId, this,
         sigc_monitor_connect_state, sigc_update );
      }
      freqExists = freqPvCb->getPvExists();
      freqPvId = freqPvCb->getPv();
      if ( freqExists && !freqPvId ) {
        fprintf( stderr, activeSignalClass_str20 );
      }

      // phase pv
      if ( !phasePvCb ) {
        phasePvCb = new PvCallbackClass(
         phasePvExpString, &connection, phasePvConnectionId, this,
         sigc_monitor_connect_state, sigc_update );
      }
      phaseExists = phasePvCb->getPvExists();
      phasePvId = phasePvCb->getPv();
      if ( phaseExists && !phasePvId ) {
        fprintf( stderr, activeSignalClass_str20 );
      }

      // vis pv
      if ( !visPvCb ) {
        visPvCb = new PvCallbackClass(
         visPvExpString, &connection, visPvConnectionId, this,
         sigc_monitor_connect_state, sigc_update );
      }
      visExists = visPvCb->getPvExists();
      if ( !visExists ) visibility = 1;
      visPvId = visPvCb->getPv();
      if ( visExists && !visPvId ) {
        fprintf( stderr, activeSignalClass_str20 );
      }

      // color pv
      if ( !colorPvCb ) {
        colorPvCb = new PvCallbackClass(
         colorPvExpString, &connection, colorPvConnectionId, this,
         sigc_monitor_connect_state, sigc_update );
      }
      colorExists = colorPvCb->getPvExists();
      colorPvId = colorPvCb->getPv();
      if ( colorExists && !colorPvId ) {
        fprintf( stderr, activeSignalClass_str20 );
      }

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, sigc_unconnectedTimeout, this );
      }

      if ( !destExists ) {
        init = 1;
        smartDrawAllActive();
      }

      opStat = 1;
      opComplete = 1;

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

int activeSignalClass::deactivate (
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
        actWin->appCtx->postMessage( activeSignalClass_str35 );
        XtRemoveTimeOut( incrementTimer );
        incrementTimer = 0;
      }
      incrementTimerActive = 0;
    }

    if ( destPvCb ) {
      delete destPvCb;
      destPvCb = NULL;
      destPvId = NULL;
    }

    if ( signalStatePvCb ) {
      if ( signalStatePvCb->getPvExists() ) {
        if ( signalStatePvId ) {
          signalStatePvId->put(
           XDisplayName(actWin->appCtx->displayName),
           0 );
        }
      }
      delete signalStatePvCb;
      signalStatePvCb = NULL;
      signalStatePvId = NULL;
    }

    if ( amplPvCb ) {
      delete amplPvCb;
      amplPvCb = NULL;
      amplPvId = NULL;
    }

    if ( offsetPvCb ) {
      delete offsetPvCb;
      offsetPvCb = NULL;
      offsetPvId = NULL;
    }

    if ( freqPvCb ) {
      delete freqPvCb;
      freqPvCb = NULL;
      freqPvId = NULL;
    }

    if ( phasePvCb ) {
      delete phasePvCb;
      phasePvCb = NULL;
      phasePvId = NULL;
    }

    if ( visPvCb ) {
      delete visPvCb;
      visPvCb = NULL;
      visPvId = NULL;
    }

    if ( colorPvCb ) {
      delete colorPvCb;
      colorPvCb = NULL;
      colorPvId = NULL;
    }

  }

  return 1;

}

void activeSignalClass::updateDimensions ( void )
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

void activeSignalClass::btnUp (
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

void activeSignalClass::btnDown (
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

    if ( signalStateExists ) {
      signalStatePvId->put(
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

  if ( signalStateExists ) {
    signalStatePvId->put(
     XDisplayName(actWin->appCtx->displayName),
     buttonPressed );
  }

  actWin->appCtx->proc->lock();
  dval = curControlV;
  needRefresh = 1;
  actWin->addDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  //fprintf( stderr, "btn down, x=%-d, y=%-d, bn=%-d\n", _x-x, _y-y , buttonNumber );
  //fprintf( stderr, "cv=%g\n", dval );

  if ( dval < minDv ) {
    dval = minDv;
  }
  else if ( dval > maxDv ) {
    dval = maxDv;
  }

  if ( updateRate < 0.1 ) updateRate = 0.1;
  if ( updateRate > 10.0 ) updateRate = 10.0;

  //fprintf( stderr, "updateRate=%g\n", updateRate );
  //fprintf( stderr, "increment=%g\n", increment );

  incrementTimer = appAddTimeOut( actWin->appCtx->appContext(),
   incrementTimerValue, sigc_increment, this );
  incrementTimerActive = 1;

}

void activeSignalClass::pointerIn (
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

int activeSignalClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;

  if ( destExists )
    *focus = 1;
  else
    *focus = 0;

  if ( !destExists ) {
    *up = 0;
    *down = 0;
    return 1;
  }

  *down = 1;
  *up = 1;

  return 1;

}

int activeSignalClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( destPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  destPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( signalStatePvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  signalStatePvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( amplPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  amplPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( offsetPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  offsetPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( freqPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  freqPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( phasePvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  phasePvExpString.setRaw( tmpStr.getExpanded() );

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

int activeSignalClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = destPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = signalStatePvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = amplPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = offsetPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = freqPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = phasePvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = label.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeSignalClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = destPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = signalStatePvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = amplPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = offsetPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = freqPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = phasePvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = label.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return stat;

}

int activeSignalClass::containsMacros ( void ) {

  if ( destPvExpString.containsPrimaryMacros() ) return 1;

  if ( signalStatePvExpString.containsPrimaryMacros() ) return 1;

  if ( amplPvExpString.containsPrimaryMacros() ) return 1;

  if ( offsetPvExpString.containsPrimaryMacros() ) return 1;

  if ( freqPvExpString.containsPrimaryMacros() ) return 1;

  if ( phasePvExpString.containsPrimaryMacros() ) return 1;

  if ( label.containsPrimaryMacros() ) return 1;

  if ( visPvExpString.containsPrimaryMacros() ) return 1;

  if ( colorPvExpString.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeSignalClass::executeDeferred ( void ) {

  int nc, nsc, nci, naci, noci, nfci, npci, nd, ne, nr, nvc, nvi, nvu, ncolc, ncoli, ncolu, nrsc;
int stat, index, invisColor;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nrsc = needSignalStateConnectInit; needSignalStateConnectInit = 0;
  nci = needCtlInfoInit; needCtlInfoInit = 0;
  naci = needAmplConnectInit; needAmplConnectInit = 0;
  noci = needOffsetConnectInit; needOffsetConnectInit = 0;
  nfci = needFreqConnectInit; needFreqConnectInit = 0;
  npci = needPhaseConnectInit; needPhaseConnectInit = 0;
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

    if ( connection.pvsConnected() ) {
      bgColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

  if ( naci ) {

    if ( connection.pvsConnected() ) {
      bgColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

  if ( noci ) {

    if ( connection.pvsConnected() ) {
      bgColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

  if ( nrsc ) {

    signalStateType = (int) signalStatePvId->get_type().type;

    if ( initialSignalStateValueConnection ) {

      initialSignalStateValueConnection = 0;

      if ( signalStateExists ) {
        signalStatePvId->put(
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

    visValue = curVisValue = visPvId->get_double();

    nvi = 1;

  }

  if ( nvi ) {

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

char *activeSignalClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeSignalClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeSignalClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( i == 0 ) {
      return destPvExpString.getExpanded();
    }
    else if ( i == 1 ) {
      return signalStatePvExpString.getExpanded();
    }
    else if ( i == 2 ) {
      return amplPvExpString.getExpanded();
    }
    else if ( i == 3 ) {
      return offsetPvExpString.getExpanded();
    }
    else if ( i == 4 ) {
      return freqPvExpString.getExpanded();
    }
    else if ( i == 5 ) {
      return phasePvExpString.getExpanded();
    }
    else if ( i == 6 ) {
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
      return signalStatePvExpString.getRaw();

    }
    else if ( i == 2 ) {
      return amplPvExpString.getRaw();
    }
    else if ( i == 3 ) {
      return offsetPvExpString.getRaw();
    }
    else if ( i == 4 ) {
      return freqPvExpString.getRaw();
    }
    else if ( i == 5 ) {
      return phasePvExpString.getRaw();
    }
    else if ( i == 6 ) {
      return colorPvExpString.getRaw();
    }
    else {
      return visPvExpString.getRaw();
    }

  }

}

void activeSignalClass::changeDisplayParams (
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

void activeSignalClass::changePvNames (
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

  if ( flag & ACTGRF_ALARMPVS_MASK ) {
    if ( numAlarmPvs ) {
      colorPvExpString.setRaw( ctlPvs[0] );
    }
  }

}

void activeSignalClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 1 ) {
    *n = 0;
    return;
  }

  *n = 1;
  pvs[0] = destPvId;

}

char *activeSignalClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return destPvExpString.getRaw();
  }
  else if ( i == 1 ) {
    return signalStatePvExpString.getRaw();
  }
  else if ( i == 2 ) {
    return amplPvExpString.getRaw();
  }
  else if ( i == 3 ) {
    return offsetPvExpString.getRaw();
  }
  else if ( i == 4 ) {
    return freqPvExpString.getRaw();
  }
  else if ( i == 5 ) {
    return phasePvExpString.getRaw();
  }
  else if ( i == 6 ) {
    return colorPvExpString.getRaw();
  }
  else if ( i == 7 ) {
    return visPvExpString.getRaw();
  }
  else if ( i == 8 ) {
    return label.getRaw();
  }
  else if ( i == 9 ) {
    return minVisString;
  }
  else if ( i == 10 ) {
    return maxVisString;
  }

  return NULL;

}

void activeSignalClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    destPvExpString.setRaw( string );
  }
  else if ( i == 1 ) {
    signalStatePvExpString.setRaw( string );
  }
  else if ( i == 2 ) {
    amplPvExpString.setRaw( string );
  }
  else if ( i == 3 ) {
    offsetPvExpString.setRaw( string );
  }
  else if ( i == 4 ) {
    freqPvExpString.setRaw( string );
  }
  else if ( i == 5 ) {
    phasePvExpString.setRaw( string );
  }
  else if ( i == 6 ) {
    colorPvExpString.setRaw( string );
  }
  else if ( i == 7 ) {
    visPvExpString.setRaw( string );
  }
  else if ( i == 8 ) {
    label.setRaw( string );
  }
  else if ( i == 9 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( minVisString, string, l );
    minVisString[l] = 0;
  }
  else if ( i == 10 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( maxVisString, string, l );
    maxVisString[l] = 0;
  }

}

// crawler functions may return blank pv names
char *activeSignalClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return destPvExpString.getExpanded();

}

char *activeSignalClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >=5 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return signalStatePvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 2 ) {
    return amplPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 3 ) {
    return offsetPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 4 ) {
    return freqPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 5 ) {
    return phasePvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 6 ) {
    return visPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 7 ) {
    return colorPvExpString.getExpanded();
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeSignalClassPtr ( void ) {

activeSignalClass *ptr;

  ptr = new activeSignalClass;
  return (void *) ptr;

}

void *clone_activeSignalClassPtr (
  void *_srcPtr )
{

activeSignalClass *ptr, *srcPtr;

  srcPtr = (activeSignalClass *) _srcPtr;

  ptr = new activeSignalClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
