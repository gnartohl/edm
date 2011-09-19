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

#define __multSegRampButton_cc 1

#include "multSegRampButton.h"
#include "app_pkg.h"
#include "act_win.h"

static void doBlink (
  void *ptr
) {

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) ptr;

  if ( !msrbto->activeMode ) {
    if ( msrbto->isSelected() ) msrbto->drawSelectBoxCorners(); //erase via xor
    msrbto->smartDrawAll();
    if ( msrbto->isSelected() ) msrbto->drawSelectBoxCorners();
  }
  else {
    msrbto->bufInvalidate();
    msrbto->needDraw = 1;
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) client;

  if ( !msrbto->init ) {
    msrbto->needToDrawUnconnected = 1;
    msrbto->needDraw = 1;
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
  }

  msrbto->unconnectedTimer = 0;

}

static void msrbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) client;
int i;

  msrbto->actWin->setChanged();

  msrbto->eraseSelectBoxCorners();
  msrbto->erase();

  msrbto->fgColor.setColorIndex( msrbto->eBuf->bufFgColor, msrbto->actWin->ci );

  msrbto->bgColor.setColorIndex( msrbto->eBuf->bufBgColor, msrbto->actWin->ci );

  msrbto->topShadowColor = msrbto->eBuf->bufTopShadowColor;
  msrbto->botShadowColor = msrbto->eBuf->bufBotShadowColor;

  msrbto->destPvExpString.setRaw( msrbto->eBuf->bufDestPvName );

  for ( i=0; i<MAXSEGS; i++ ) {
    msrbto->finalPvExpString[i].setRaw( msrbto->eBuf->bufFinalPvName[i] );
  }

  for ( i=0; i<MAXSEGS; i++ ) {
    msrbto->rampRatePvExpString[i].setRaw( msrbto->eBuf->bufRampRatePvName[i] );
  }

  msrbto->rampStatePvExpString.setRaw( msrbto->eBuf->bufRampStatePvName );

  msrbto->label.setRaw( msrbto->eBuf->bufLabel );

  strncpy( msrbto->fontTag, msrbto->fm.currentFontTag(), 63 );
  msrbto->actWin->fi->loadFontTag( msrbto->fontTag );
  msrbto->fs = msrbto->actWin->fi->getXFontStruct( msrbto->fontTag );

  msrbto->_3D = msrbto->eBuf->buf3D;

  msrbto->invisible = msrbto->eBuf->bufInvisible;

  msrbto->updateRate = msrbto->eBuf->bufUpdateRate;
  if ( msrbto->updateRate < 0.1 ) msrbto->updateRate = 0.1;
  if ( msrbto->updateRate > 10.0 ) msrbto->updateRate = 10.0;

  msrbto->limitsFromDb = msrbto->eBuf->bufLimitsFromDb;

  msrbto->efScaleMin = msrbto->eBuf->bufEfScaleMin;
  msrbto->efScaleMax = msrbto->eBuf->bufEfScaleMax;

  msrbto->minDv = msrbto->scaleMin = msrbto->efScaleMin.value();
  msrbto->maxDv = msrbto->scaleMax = msrbto->efScaleMax.value();

  msrbto->efRateMax = msrbto->eBuf->bufEfRateMax;
  
  if ( msrbto->efRateMax.isNull() ) {
    msrbto->rateMax = 600.0;
  }
  else {
    msrbto->rateMax = msrbto->efRateMax.value();
  }

  msrbto->visPvExpString.setRaw( msrbto->eBuf->bufVisPvName );
  strncpy( msrbto->minVisString, msrbto->eBuf->bufMinVisString, 39 );
  strncpy( msrbto->maxVisString, msrbto->eBuf->bufMaxVisString, 39 );

  if ( msrbto->eBuf->bufVisInverted )
    msrbto->visInverted = 0;
  else
    msrbto->visInverted = 1;

  msrbto->colorPvExpString.setRaw( msrbto->eBuf->bufColorPvName );

  msrbto->x = msrbto->eBuf->bufX;
  msrbto->sboxX = msrbto->eBuf->bufX;

  msrbto->y = msrbto->eBuf->bufY;
  msrbto->sboxY = msrbto->eBuf->bufY;

  msrbto->w = msrbto->eBuf->bufW;
  msrbto->sboxW = msrbto->eBuf->bufW;

  msrbto->h = msrbto->eBuf->bufH;
  msrbto->sboxH = msrbto->eBuf->bufH;

  msrbto->updateDimensions();

}

static void msrbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) client;

  msrbtc_edit_update ( w, client, call );
  msrbto->refresh( msrbto );

}

static void msrbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) client;

  msrbtc_edit_update ( w, client, call );
  msrbto->ef.popdown();
  msrbto->operationComplete();

}

static void msrbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) client;

  msrbto->ef.popdown();
  msrbto->operationCancel();

}

static void msrbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) client;

  msrbto->ef.popdown();
  msrbto->operationCancel();
  msrbto->erase();
  msrbto->deleteRequest = 1;
  msrbto->drawAll();

}

static void msrbtc_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;

  if ( pv->is_valid() ) {

    msrbto->needConnectInit = 1;

  }
  else {

    msrbto->connection.setPvDisconnected( (void *) msrbto->destPvConnection );
    msrbto->active = 0;
    msrbto->bgColor.setDisconnected();
    msrbto->needDraw = 1;

  }

  msrbto->actWin->appCtx->proc->lock();
  msrbto->actWin->addDefExeNode( msrbto->aglPtr );
  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_monitor_final_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;
int i, index = -1;

  for ( i=0; i<MAXSEGS; i++ ) {
    if ( pv == msrbto->finalPvId[i] ) {
      index = i;
      break;
    }
  }

  if ( index == -1 ) return;

  if ( pv->is_valid() ) {

    msrbto->needFinalConnectInit[index] = 1;
    msrbto->actWin->appCtx->proc->lock();
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
    msrbto->actWin->appCtx->proc->unlock();

  }
  else {

    msrbto->connection.setPvDisconnected( (void *) msrbto->finalPvConnection[index] );
    msrbto->active = 0;
    msrbto->bgColor.setDisconnected();
    msrbto->needDraw = 1;

  }

  msrbto->actWin->appCtx->proc->lock();
  msrbto->actWin->addDefExeNode( msrbto->aglPtr );
  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_monitor_rampRate_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;
int i, index = -1;

  for ( i=0; i<MAXSEGS; i++ ) {
    if ( pv == msrbto->rampRatePvId[i] ) {
      index = i;
      break;
    }
  }

  if ( index == -1 ) return;

  if ( pv->is_valid() ) {

    msrbto->needRampRateConnectInit[index] = 1;
    msrbto->actWin->appCtx->proc->lock();
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
    msrbto->actWin->appCtx->proc->unlock();

  }
  else {

    msrbto->connection.setPvDisconnected( (void *) msrbto->rampRatePvConnection[index] );
    msrbto->active = 0;
    msrbto->bgColor.setDisconnected();
    msrbto->needDraw = 1;

  }

  msrbto->actWin->appCtx->proc->lock();
  msrbto->actWin->addDefExeNode( msrbto->aglPtr );
  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_monitor_rampState_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;

  if ( pv->is_valid() ) {

    msrbto->needRampStateConnectInit = 1;
    msrbto->actWin->appCtx->proc->lock();
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
    msrbto->actWin->appCtx->proc->unlock();

  }
  else {

    msrbto->connection.setPvDisconnected( (void *) msrbto->rampStatePvConnection );
    msrbto->active = 0;
    msrbto->bgColor.setDisconnected();
    msrbto->needDraw = 1;

  }

  msrbto->actWin->appCtx->proc->lock();
  msrbto->actWin->addDefExeNode( msrbto->aglPtr );
  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;

  msrbto->actWin->appCtx->proc->lock();

  msrbto->curControlV = pv->get_double();

  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_finalUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;
int i, index = -1;

  for ( i=0; i<MAXSEGS; i++ ) {
    if ( pv == msrbto->finalPvId[i] ) {
      index = i;
      break;
    }
  }

  if ( index == -1 ) return;

  msrbto->actWin->appCtx->proc->lock();

  msrbto->curFinalV[index] = pv->get_double();

  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_rampRateUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;
int i, index = -1;

  for ( i=0; i<MAXSEGS; i++ ) {
    if ( pv == msrbto->rampRatePvId[i] ) {
      index = i;
      break;
    }
  }

  if ( index == -1 ) return;

  msrbto->actWin->appCtx->proc->lock();

  msrbto->curRampRateV[index] = pv->get_double();

  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;

  if ( pv->is_valid() ) {

    msrbto->needVisConnectInit = 1;

  }
  else {

    msrbto->connection.setPvDisconnected( (void *) msrbto->visPvConnection );
    msrbto->active = 0;
    msrbto->bgColor.setDisconnected();
    msrbto->needDraw = 1;

  }

  msrbto->actWin->appCtx->proc->lock();
  msrbto->actWin->addDefExeNode( msrbto->aglPtr );
  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_visUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;

  msrbto->curVisValue = pv->get_double();

  msrbto->actWin->appCtx->proc->lock();
  msrbto->needVisUpdate = 1;
  msrbto->actWin->addDefExeNode( msrbto->aglPtr );
  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;

  if ( pv->is_valid() ) {

    msrbto->needColorConnectInit = 1;

  }
  else {

    msrbto->connection.setPvDisconnected( (void *) msrbto->colorPvConnection );
    msrbto->active = 0;
    msrbto->bgColor.setDisconnected();
    msrbto->needDraw = 1;

  }

  msrbto->actWin->appCtx->proc->lock();
  msrbto->actWin->addDefExeNode( msrbto->aglPtr );
  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_colorUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) userarg;

  msrbto->curColorValue = pv->get_double();

  msrbto->actWin->appCtx->proc->lock();
  msrbto->needColorUpdate = 1;
  msrbto->actWin->addDefExeNode( msrbto->aglPtr );
  msrbto->actWin->appCtx->proc->unlock();

}

static void msrbtc_decrement (
  XtPointer client,
  XtIntervalId *id )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) client;
double dval, seconds, adjust;
struct timeval curTime;
int needToAddTimeout = 1;

  //fprintf( stderr, "msrbtc_decrement\n" );
  //fprintf( stderr, "msrbto->rampSegment = %-d\n", msrbto->rampSegment );
  //fprintf( stderr, "msrbto->increment[%-d] = %-f\n",
  // msrbto->rampSegment, msrbto->increment[msrbto->rampSegment] );
  //fprintf( stderr, "msrbto->rampFinalV[%-d] = %-f\n",
  // msrbto->rampSegment, msrbto->rampFinalV[msrbto->rampSegment] );

  gettimeofday( &curTime, NULL );
  seconds = curTime.tv_sec - msrbto->baseTime.tv_sec +
    ( curTime.tv_usec - msrbto->baseTime.tv_usec ) * 0.000001;
  msrbto->baseTime = curTime;

  adjust = seconds / msrbto->updateRate;
  if ( adjust > 1.1 ) adjust = 1.1;
  if ( adjust < 0.9 ) adjust = 0.9;

  if ( !msrbto->incrementTimerActive ) {
    msrbto->incrementTimer = 0;
    return;
  }

  msrbto->actWin->appCtx->proc->lock();
  dval = msrbto->curControlV;
  msrbto->actWin->appCtx->proc->unlock();

  dval -= msrbto->increment[msrbto->rampSegment]; // * adjust;

  if ( dval <= msrbto->rampFinalV[msrbto->rampSegment] ) {

    needToAddTimeout = 0;

    dval = msrbto->rampFinalV[msrbto->rampSegment];

    // increment rampSegment
    do {
      (msrbto->rampSegment)++;
    } while ( ( msrbto->rampSegment < MAXSEGS ) &&
              ( msrbto->increment[msrbto->rampSegment] == 0 ) );

    if ( msrbto->rampSegment >= MAXSEGS ) {
      if ( msrbto->rampStateExists ) {
        msrbto->buttonPressed = 0;
        msrbto->rampStatePvId->put(
         XDisplayName(msrbto->actWin->appCtx->displayName),
         msrbto->buttonPressed );
      }
      msrbto->incrementTimerActive = 0;
      msrbto->buttonPressed = 0;
    }
    else {

      //fprintf( stderr, "change msrbto->rampSegment = %-d\n", msrbto->rampSegment );
      //fprintf( stderr, "change msrbto->increment[msrbto->rampSegment] = %-f\n",
      // msrbto->increment[msrbto->rampSegment] );

      if ( msrbto->rampStateExists ) {
        msrbto->rampStatePvId->put(
         XDisplayName(msrbto->actWin->appCtx->displayName),
         msrbto->rampSegment+1 );
      }

      msrbto->incrementTimerActive = 1;
      if ( msrbto->rampFinalV[msrbto->rampSegment] < dval ) {
        msrbto->incrementTimer = appAddTimeOut( msrbto->actWin->appCtx->appContext(),
         msrbto->incrementTimerValue, msrbtc_decrement, client );
      }
      else {
        msrbto->incrementTimer = appAddTimeOut( msrbto->actWin->appCtx->appContext(),
         msrbto->incrementTimerValue, msrbtc_increment, client );
      }

    }

    msrbto->actWin->appCtx->proc->lock();
    msrbto->needRefresh = 1;
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
    msrbto->actWin->appCtx->proc->unlock();

  }

  if ( dval <= msrbto->minDv ) {
    needToAddTimeout = 0;
    dval = msrbto->minDv;
    msrbto->incrementTimerActive = 0;
    msrbto->buttonPressed = 0;
    if ( msrbto->rampStateExists ) {
      msrbto->rampStatePvId->put(
       XDisplayName(msrbto->actWin->appCtx->displayName),
       msrbto->buttonPressed );
    }
    msrbto->actWin->appCtx->proc->lock();
    msrbto->needRefresh = 1;
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
    msrbto->actWin->appCtx->proc->unlock();
  }
  else if ( dval >= msrbto->maxDv ) {
    needToAddTimeout = 0;
    dval = msrbto->maxDv;
    msrbto->incrementTimerActive = 0;
    msrbto->buttonPressed = 0;
    if ( msrbto->rampStateExists ) {
      msrbto->rampStatePvId->put(
       XDisplayName(msrbto->actWin->appCtx->displayName),
       msrbto->buttonPressed );
    }
    msrbto->actWin->appCtx->proc->lock();
    msrbto->needRefresh = 1;
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
    msrbto->actWin->appCtx->proc->unlock();
  }
  if ( needToAddTimeout ) {

    msrbto->incrementTimer = appAddTimeOut(
     msrbto->actWin->appCtx->appContext(),
     msrbto->incrementTimerValue, msrbtc_decrement, client );

  }

  if ( msrbto->destExists ) {
    msrbto->destPvId->put(
     XDisplayName(msrbto->actWin->appCtx->displayName),
     dval );
  }

}

static void msrbtc_increment (
  XtPointer client,
  XtIntervalId *id )
{

activeMultSegRampButtonClass *msrbto = (activeMultSegRampButtonClass *) client;
double dval, seconds, adjust;
struct timeval curTime;
int needToAddTimeout = 1;

  //fprintf( stderr, "msrbtc_increment\n" );
  //fprintf( stderr, "msrbto->rampSegment = %-d\n", msrbto->rampSegment );
  //fprintf( stderr, "msrbto->increment[%-d] = %-f\n",
  // msrbto->rampSegment, msrbto->increment[msrbto->rampSegment] );
  //fprintf( stderr, "msrbto->rampFinalV[%-d] = %-f\n",
  // msrbto->rampSegment, msrbto->rampFinalV[msrbto->rampSegment] );

  gettimeofday( &curTime, NULL );
  seconds = curTime.tv_sec - msrbto->baseTime.tv_sec +
    ( curTime.tv_usec - msrbto->baseTime.tv_usec ) * 0.000001;
  msrbto->baseTime = curTime;

  adjust = seconds / msrbto->updateRate;
  if ( adjust > 1.1 ) adjust = 1.1;
  if ( adjust < 0.9 ) adjust = 0.9;

  if ( !msrbto->incrementTimerActive ) {
    msrbto->incrementTimer = 0;
    return;
  }

  msrbto->actWin->appCtx->proc->lock();
  dval = msrbto->curControlV;
  msrbto->actWin->appCtx->proc->unlock();

  dval += msrbto->increment[msrbto->rampSegment]; // * adjust;

  if ( dval >= msrbto->rampFinalV[msrbto->rampSegment] ) {

    needToAddTimeout = 0;

    dval = msrbto->rampFinalV[msrbto->rampSegment];

    // increment rampSegment
    do {
      (msrbto->rampSegment)++;
    } while ( ( msrbto->rampSegment < MAXSEGS ) &&
              ( msrbto->increment[msrbto->rampSegment] == 0 ) );

    if ( msrbto->rampSegment >= MAXSEGS ) {
      msrbto->buttonPressed = 0;
      if ( msrbto->rampStateExists ) {
        msrbto->rampStatePvId->put(
         XDisplayName(msrbto->actWin->appCtx->displayName),
         msrbto->buttonPressed );
      }
      msrbto->incrementTimerActive = 0;
      msrbto->buttonPressed = 0;
    }
    else {

      //fprintf( stderr, "change msrbto->rampSegment = %-d\n", msrbto->rampSegment );
      //fprintf( stderr, "change msrbto->increment[msrbto->rampSegment] = %-f\n",
      // msrbto->increment[msrbto->rampSegment] );

      if ( msrbto->rampStateExists ) {
        msrbto->rampStatePvId->put(
         XDisplayName(msrbto->actWin->appCtx->displayName),
         msrbto->rampSegment+1 );
      }

      msrbto->incrementTimerActive = 1;
      if ( msrbto->rampFinalV[msrbto->rampSegment] < dval ) {
        msrbto->incrementTimer = appAddTimeOut( msrbto->actWin->appCtx->appContext(),
         msrbto->incrementTimerValue, msrbtc_decrement, client );
      }
      else {
        msrbto->incrementTimer = appAddTimeOut( msrbto->actWin->appCtx->appContext(),
         msrbto->incrementTimerValue, msrbtc_increment, client );
      }

    }

    msrbto->actWin->appCtx->proc->lock();
    msrbto->needRefresh = 1;
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
    msrbto->actWin->appCtx->proc->unlock();

  }

  if ( dval <= msrbto->minDv ) {
    needToAddTimeout = 0;
    dval = msrbto->minDv;
    msrbto->incrementTimerActive = 0;
    msrbto->buttonPressed = 0;
    if ( msrbto->rampStateExists ) {
      msrbto->rampStatePvId->put(
       XDisplayName(msrbto->actWin->appCtx->displayName),
       msrbto->buttonPressed );
    }
    msrbto->actWin->appCtx->proc->lock();
    msrbto->needRefresh = 1;
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
    msrbto->actWin->appCtx->proc->unlock();
  }
  else if ( dval >= msrbto->maxDv ) {
    needToAddTimeout = 0;
    dval = msrbto->maxDv;
    msrbto->incrementTimerActive = 0;
    msrbto->buttonPressed = 0;
    if ( msrbto->rampStateExists ) {
      msrbto->rampStatePvId->put(
       XDisplayName(msrbto->actWin->appCtx->displayName),
       msrbto->buttonPressed );
    }
    msrbto->actWin->appCtx->proc->lock();
    msrbto->needRefresh = 1;
    msrbto->actWin->addDefExeNode( msrbto->aglPtr );
    msrbto->actWin->appCtx->proc->unlock();
  }

  if ( needToAddTimeout ) {

    msrbto->incrementTimer = appAddTimeOut(
     msrbto->actWin->appCtx->appContext(),
     msrbto->incrementTimerValue, msrbtc_increment, client );

  }

  if ( msrbto->destExists ) {
    msrbto->destPvId->put(
     XDisplayName(msrbto->actWin->appCtx->displayName),
     dval );
  }

}

activeMultSegRampButtonClass::activeMultSegRampButtonClass ( void ) {

int i;

  name = new char[strlen("activeMultSegRampButtonClass")+1];
  strcpy( name, "activeMultSegRampButtonClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  buttonPressed = 0;

  state = MSRBTC_IDLE;
  _3D = 1;
  invisible = 0;
  updateRate = 0.5;
  for ( i=0; i<MAXSEGS; i++ ) {
    curRampRateV[i] = 0.0;
    curFinalV[i] = 0.0;
    finalPvConnection[i] = i + 5;
    rampRatePvConnection[i] = i + 10;
    segExists[i] = 0;
  }
  scaleMin = 0;
  scaleMax = 10;
  rateMax = 600;
  limitsFromDb = 1;
  monotonic = 0;
  efScaleMin.setNull(1);
  efScaleMax.setNull(1);
  efRateMax.setNull(1);
  unconnectedTimer = 0;
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  connection.setMaxPvs( 9 );
  activeMode = 0;
  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

}

// copy constructor
activeMultSegRampButtonClass::activeMultSegRampButtonClass
 ( const activeMultSegRampButtonClass *source ) {

activeGraphicClass *msrbto = (activeGraphicClass *) this;
int i;

  msrbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMultSegRampButtonClass")+1];
  strcpy( name, "activeMultSegRampButtonClass" );

  buttonPressed = 0;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  destPvExpString.copy( source->destPvExpString );

  for ( i=0; i<MAXSEGS; i++ ) {
    finalPvExpString[i].copy( source->finalPvExpString[i] );
    finalPvConnection[i] = i + 5;
    rampRatePvExpString[i].copy( source->rampRatePvExpString[i] );
    rampRatePvConnection[i] = i + 10;
  }

  rampStatePvExpString.copy( source->rampStatePvExpString );
  visPvExpString.copy( source->visPvExpString );
  colorPvExpString.copy( source->colorPvExpString );

  label.copy( source->label );

  state = MSRBTC_IDLE;
  _3D = source->_3D;
  invisible = source->invisible;
  updateRate = source->updateRate;
  for ( i=0; i<MAXSEGS; i++ ) {
    curRampRateV[i] = 0.0;
    curFinalV[i] = 0.0;
    segExists[i] = 0;
  }
  limitsFromDb = source->limitsFromDb;
  monotonic = 0;
  scaleMin = source->scaleMin;
  scaleMax = source->scaleMax;
  efScaleMin = source->efScaleMin;
  efScaleMax = source->efScaleMax;
  rateMax = source->rateMax;
  efRateMax = source->efRateMax;
  unconnectedTimer = 0;

  visibility = 0;
  prevVisibility = -1;
  visInverted = source->visInverted;
  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );
  activeMode = 0;
  eBuf = NULL;

  connection.setMaxPvs( 9 );

  setBlinkFunction( (void *) doBlink );

  doAccSubs( destPvExpString );
  doAccSubs( rampStatePvExpString );
  doAccSubs( visPvExpString );
  doAccSubs( colorPvExpString );
  doAccSubs( label );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );
  for ( i=0; i<MAXSEGS; i++ ) {
    doAccSubs( finalPvExpString[i] );
    doAccSubs( rampRatePvExpString[i] );
  }

  updateDimensions();

}

activeMultSegRampButtonClass::~activeMultSegRampButtonClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

int activeMultSegRampButtonClass::createInteractive (
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

int activeMultSegRampButtonClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

  major = MSRBTC_MAJOR_VERSION;
  minor = MSRBTC_MINOR_VERSION;
  release = MSRBTC_RELEASE;

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
  tag.loadW( "finalValuePv", finalPvExpString, 5, emptyStr );
  tag.loadW( "rampRatePv", rampRatePvExpString, 5, emptyStr );
  tag.loadW( "rampStateValuePv", &rampStatePvExpString, emptyStr );
  tag.loadW( "updateRate", &updateRate, &dzero );
  tag.loadW( "label", &label, emptyStr );
  tag.loadBoolW( "3d", &_3D, &zero );
  tag.loadBoolW( "invisible", &invisible, &zero );
  tag.loadW( "font", fontTag );
  tag.loadBoolW( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadW( "scaleMin", &efScaleMin );
  tag.loadW( "scaleMax", &efScaleMax );
  tag.loadW( "rateMax", &efRateMax );
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

int activeMultSegRampButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int numFinalPvs, numRampRatePvs, stat, major, minor, release;

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
  tag.loadR( "finalValuePv", MAXSEGS, finalPvExpString, &numFinalPvs,
   emptyStr );
  tag.loadR( "rampRatePv", MAXSEGS, rampRatePvExpString, &numRampRatePvs,
   emptyStr );
  tag.loadR( "rampStateValuePv", &rampStatePvExpString, emptyStr );
  tag.loadR( "updateRate", &updateRate, &dzero );
  tag.loadR( "label", &label, emptyStr );
  tag.loadR( "3d", &_3D, &zero );
  tag.loadR( "invisible", &invisible, &zero );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadR( "scaleMin", &efScaleMin );
  tag.loadR( "scaleMax", &efScaleMax );
  tag.loadR( "rateMax", &efRateMax );
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

  if ( major > MSRBTC_MAJOR_VERSION ) {
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

  if ( efRateMax.isNull() ) {
    rateMax = 600.0;
  }
  else {
    rateMax = efRateMax.value();
  }

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeMultSegRampButtonClass::genericEdit ( void ) {

char title[32], *ptr;
int i;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeMultSegRampButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMultSegRampButtonClass_str2, 31 );

  Strncat( title, activeMultSegRampButtonClass_str3, 31 );

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

  for ( i=0; i<MAXSEGS; i++ ) {

    if ( finalPvExpString[i].getRaw() )
      strncpy( eBuf->bufFinalPvName[i], finalPvExpString[i].getRaw(),
       PV_Factory::MAX_PV_NAME );
    else
      strcpy( eBuf->bufFinalPvName[i], "" );

    if ( rampRatePvExpString[i].getRaw() )
      strncpy( eBuf->bufRampRatePvName[i], rampRatePvExpString[i].getRaw(),
       PV_Factory::MAX_PV_NAME );
    else
      strcpy( eBuf->bufRampRatePvName[i], "" );

  }

  if ( rampStatePvExpString.getRaw() )
    strncpy( eBuf->bufRampStatePvName, rampStatePvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufRampStatePvName, "" );

  eBuf->bufUpdateRate = updateRate;

  if ( label.getRaw() )
    strncpy( eBuf->bufLabel, label.getRaw(), 39 );
  else
    strncpy( eBuf->bufLabel, "", 39 );

  eBuf->buf3D = _3D;
  eBuf->bufInvisible = invisible;

  eBuf->bufLimitsFromDb = limitsFromDb;
  eBuf->bufEfScaleMin = efScaleMin;
  eBuf->bufEfScaleMax = efScaleMax;

  eBuf->bufEfRateMax = efRateMax;

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

  ef.addTextField( activeMultSegRampButtonClass_str4, 35, &eBuf->bufX );
  ef.addTextField( activeMultSegRampButtonClass_str5, 35, &eBuf->bufY );
  ef.addTextField( activeMultSegRampButtonClass_str6, 35, &eBuf->bufW );
  ef.addTextField( activeMultSegRampButtonClass_str7, 35, &eBuf->bufH );
  ef.addTextField( activeMultSegRampButtonClass_str8, 35, eBuf->bufDestPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeMultSegRampButtonClass_str34, 35, eBuf->bufRampStatePvName,
   PV_Factory::MAX_PV_NAME );

  i = 0;
  ef.addTextField( activeMultSegRampButtonClass_str9, 35, eBuf->bufFinalPvName[i],
   PV_Factory::MAX_PV_NAME );
  finalEntry[i] = ef.getCurItem();
  ef.addTextField( activeMultSegRampButtonClass_str10, 35, eBuf->bufRampRatePvName[i],
   PV_Factory::MAX_PV_NAME );
  rateEntry[i] = ef.getCurItem();
  finalEntry[i]->addDependency( rateEntry[i] );
  finalEntry[i]->addDependencyCallbacks();
  i++;
  ef.addTextField( activeMultSegRampButtonClass_str35, 35, eBuf->bufFinalPvName[i],
   PV_Factory::MAX_PV_NAME );
  finalEntry[i] = ef.getCurItem();
  ef.addTextField( activeMultSegRampButtonClass_str39, 35, eBuf->bufRampRatePvName[i],
   PV_Factory::MAX_PV_NAME );
  rateEntry[i] = ef.getCurItem();
  finalEntry[i]->addDependency( rateEntry[i] );
  finalEntry[i]->addDependencyCallbacks();
  i++;
  ef.addTextField( activeMultSegRampButtonClass_str36, 35, eBuf->bufFinalPvName[i],
   PV_Factory::MAX_PV_NAME );
  finalEntry[i] = ef.getCurItem();
  ef.addTextField( activeMultSegRampButtonClass_str40, 35, eBuf->bufRampRatePvName[i],
   PV_Factory::MAX_PV_NAME );
  rateEntry[i] = ef.getCurItem();
  finalEntry[i]->addDependency( rateEntry[i] );
  finalEntry[i]->addDependencyCallbacks();
  i++;
  ef.addTextField( activeMultSegRampButtonClass_str37, 35, eBuf->bufFinalPvName[i],
   PV_Factory::MAX_PV_NAME );
  finalEntry[i] = ef.getCurItem();
  ef.addTextField( activeMultSegRampButtonClass_str41, 35, eBuf->bufRampRatePvName[i],
   PV_Factory::MAX_PV_NAME );
  rateEntry[i] = ef.getCurItem();
  finalEntry[i]->addDependency( rateEntry[i] );
  finalEntry[i]->addDependencyCallbacks();
  i++;
  ef.addTextField( activeMultSegRampButtonClass_str38, 35, eBuf->bufFinalPvName[i],
   PV_Factory::MAX_PV_NAME );
  finalEntry[i] = ef.getCurItem();
  ef.addTextField( activeMultSegRampButtonClass_str42, 35, eBuf->bufRampRatePvName[i],
   PV_Factory::MAX_PV_NAME );
  rateEntry[i] = ef.getCurItem();
  finalEntry[i]->addDependency( rateEntry[i] );
  finalEntry[i]->addDependencyCallbacks();
  i++;

  ef.addToggle( activeMultSegRampButtonClass_str26, &eBuf->bufLimitsFromDb );
  limitsFromDbEntry = ef.getCurItem();
  ef.addTextField( activeMultSegRampButtonClass_str27, 35, &eBuf->bufEfScaleMin );
  minEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( minEntry );
  ef.addTextField( activeMultSegRampButtonClass_str28, 35, &eBuf->bufEfScaleMax );
  maxEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( maxEntry );
  limitsFromDbEntry->addDependencyCallbacks();

  ef.addTextField( activeMultSegRampButtonClass_str43, 35, &eBuf->bufEfRateMax );
  ef.addTextField( activeMultSegRampButtonClass_str11, 35, &eBuf->bufUpdateRate );
  ef.addToggle( activeMultSegRampButtonClass_str12, &eBuf->buf3D );
  ef.addToggle( activeMultSegRampButtonClass_str13, &eBuf->bufInvisible );
  ef.addTextField( activeMultSegRampButtonClass_str14, 35, eBuf->bufLabel, 39 );
  ef.addColorButton( activeMultSegRampButtonClass_str16, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addColorButton( activeMultSegRampButtonClass_str17, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  ef.addColorButton( activeMultSegRampButtonClass_str18, actWin->ci, &eBuf->topShadowCb, &eBuf->bufTopShadowColor );
  ef.addColorButton( activeMultSegRampButtonClass_str19, actWin->ci, &eBuf->botShadowCb, &eBuf->bufBotShadowColor );

  ef.addFontMenu( activeMultSegRampButtonClass_str15, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeMultSegRampButtonClass_str33, 30, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeMultSegRampButtonClass_str29, 30, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeMultSegRampButtonClass_str30, &eBuf->bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeMultSegRampButtonClass_str31, 30, eBuf->bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeMultSegRampButtonClass_str32, 30, eBuf->bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  return 1;

}

int activeMultSegRampButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( msrbtc_edit_ok, msrbtc_edit_apply, msrbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMultSegRampButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( msrbtc_edit_ok, msrbtc_edit_apply, msrbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMultSegRampButtonClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMultSegRampButtonClass::eraseActive ( void ) {

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

int activeMultSegRampButtonClass::draw ( void ) {

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

int activeMultSegRampButtonClass::drawActive ( void ) {

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

int activeMultSegRampButtonClass::activate (
  int pass,
  void *ptr )
{

int opStat, i;

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      rampSegment = 0;

      connection.init();

      initEnable();

      needConnectInit = needRampStateConnectInit = needCtlInfoInit = 
       needRefresh = needErase = needDraw = needVisConnectInit =
       needVisInit = needVisUpdate = needColorConnectInit =
       needColorInit = needColorUpdate = 0;

      for ( i=0; i<MAXSEGS; i++ ) {
        needFinalConnectInit[i] = 0;
        rampFinalV[i] = 0;
        needRampRateConnectInit[i] = 0;
      }

      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      init = 0;
      aglPtr = ptr;
      incrementTimer = 0;
      incrementTimerActive = 0;
      destPvId = visPvId = colorPvId = rampStatePvId = NULL;
      for ( i=0; i<MAXSEGS; i++ ) {
	finalPvId[i] = NULL;
	rampRatePvId[i] = NULL;
      }
      initialConnection = initialRampStateValueConnection =
       initialVisConnection = initialColorConnection = -1;

      for ( i=0; i<MAXSEGS; i++ ) {
	initialFinalValueConnection[i] = -1;
	initialRampRateConnection[i] = -1;
      }

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

      for ( i=0; i<MAXSEGS; i++ ) {
        if ( !finalPvExpString[i].getExpanded() ||
           blankOrComment( finalPvExpString[i].getExpanded() ) ) {
          finalExists[i] = 0;
        }
        else {
          finalExists[i] = 1;
          connection.addPv();
        }
      }

      for ( i=0; i<MAXSEGS; i++ ) {
        if ( !rampRatePvExpString[i].getExpanded() ||
           blankOrComment( rampRatePvExpString[i].getExpanded() ) ) {
          rampRateExists[i] = 0;
        }
        else {
          rampRateExists[i] = 1;
          connection.addPv();
        }
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
         2000, unconnectedTimeout, this );
      }

      opStat = 1;

      if ( destExists ) {

	destPvId = the_PV_Factory->create( destPvExpString.getExpanded() );
	if ( destPvId ) {
	  destPvId->add_conn_state_callback( msrbtc_monitor_dest_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeMultSegRampButtonClass_str20 );
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
	  visPvId->add_conn_state_callback( msrbtc_monitor_vis_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeMultSegRampButtonClass_str20 );
          opStat = 0;
        }

      }

      if ( colorExists ) {

	colorPvId = the_PV_Factory->create( colorPvExpString.getExpanded() );
	if ( colorPvId ) {
	  colorPvId->add_conn_state_callback(
           msrbtc_monitor_color_connect_state, this );
	}
	else {
          fprintf( stderr, activeMultSegRampButtonClass_str20 );
          opStat = 0;
        }

      }

      for ( i=0; i<MAXSEGS; i++ ) {

        if ( finalExists[i] ) {

          finalPvId[i] = the_PV_Factory->create( finalPvExpString[i].getExpanded() );
          if ( finalPvId[i] ) {
            finalPvId[i]->add_conn_state_callback( msrbtc_monitor_final_connect_state,
             this );
          }
          else {
            fprintf( stderr, activeMultSegRampButtonClass_str20 );
            opStat = 0;
          }

        }

        if ( rampRateExists[i] ) {

          rampRatePvId[i] = the_PV_Factory->create( rampRatePvExpString[i].getExpanded() );
          if ( rampRatePvId[i] ) {
            rampRatePvId[i]->add_conn_state_callback( msrbtc_monitor_rampRate_connect_state,
             this );
          }
          else {
            fprintf( stderr, activeMultSegRampButtonClass_str20 );
            opStat = 0;
          }

        }

      }

      if ( rampStateExists ) {

	rampStatePvId = the_PV_Factory->create( rampStatePvExpString.getExpanded() );
	if ( rampStatePvId ) {
	  rampStatePvId->add_conn_state_callback( msrbtc_monitor_rampState_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeMultSegRampButtonClass_str20 );
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

int activeMultSegRampButtonClass::deactivate (
  int pass
) {

int i;

  if ( pass == 1 ) {

  active = 0;
  activeMode = 0;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  if ( incrementTimerActive ) {
    if ( incrementTimer ) {
      actWin->appCtx->postMessage( activeMultSegRampButtonClass_str46 );
      XtRemoveTimeOut( incrementTimer );
      incrementTimer = 0;
    }
    incrementTimerActive = 0;
  }

  if ( destExists ) {
    if ( destPvId ) {
      destPvId->remove_conn_state_callback( msrbtc_monitor_dest_connect_state,
       this );
      destPvId->remove_value_callback( msrbtc_controlUpdate, this );
      destPvId->release();
      destPvId = NULL;
    }
  }

  if ( visExists ) {
    if ( visPvId ) {
      visPvId->remove_conn_state_callback( msrbtc_monitor_vis_connect_state,
       this );
      visPvId->remove_value_callback( msrbtc_visUpdate, this );
      visPvId->release();
      visPvId = NULL;
    }
  }

  if ( colorExists ) {
    if ( colorPvId ) {
      colorPvId->remove_conn_state_callback( msrbtc_monitor_color_connect_state,
       this );
      colorPvId->remove_value_callback( msrbtc_colorUpdate, this );
      colorPvId->release();
      colorPvId = NULL;
    }
  }

  for ( i=0; i<MAXSEGS; i++ ) {

    if ( finalExists[i] ) {
      if ( finalPvId[i] ) {
        finalPvId[i]->remove_conn_state_callback( msrbtc_monitor_final_connect_state,
         this );
        finalPvId[i]->remove_value_callback( msrbtc_finalUpdate, this );
        finalPvId[i]->release();
        finalPvId[i] = NULL;
      }
    }

    if ( rampRateExists[i] ) {
      if ( rampRatePvId[i] ) {
        rampRatePvId[i]->remove_conn_state_callback( msrbtc_monitor_rampRate_connect_state,
         this );
        rampRatePvId[i]->remove_value_callback( msrbtc_rampRateUpdate, this );
        rampRatePvId[i]->release();
        rampRatePvId[i] = NULL;
      }
    }

  }

  if ( rampStateExists ) {
    if ( rampStatePvId ) {
      if ( rampStateExists ) {
        rampStatePvId->put(
         XDisplayName(actWin->appCtx->displayName),
         0 );
      }
      rampStatePvId->remove_conn_state_callback( msrbtc_monitor_rampState_connect_state,
       this );
      rampStatePvId->release();
      rampStatePvId = NULL;
    }
  }

  }

  return 1;

}

void activeMultSegRampButtonClass::updateDimensions ( void )
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

void activeMultSegRampButtonClass::btnUp (
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

void activeMultSegRampButtonClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

double dval, minVal, maxVal, checkVal;
int i, atLeastOne, minSeg, maxSeg, firstSeg, lastSeg, curSeg,
 increasing, decreasing;

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

  actWin->appCtx->proc->lock();
  dval = curControlV;
  for ( i=0; i<MAXSEGS; i++ ) {
    rampFinalV[i] = curFinalV[i];
    rampRate[i] = fabs(curRampRateV[i]);
    if ( rampRate[i] > rateMax ) rampRate[i] = rateMax;
  }
  needRefresh = 1;
  actWin->addDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  //fprintf( stderr, "btn down, x=%-d, y=%-d, bn=%-d\n", _x-x, _y-y , buttonNumber );
  //fprintf( stderr, "cv=%g, fv=%g\n", dval, rampFinalV[rampSegment] );

  if ( dval < minDv ) {
    dval = minDv;
  }
  else if ( dval > maxDv ) {
    dval = maxDv;
  }

  if ( updateRate < 0.1 ) updateRate = 0.1;
  if ( updateRate > 10.0 ) updateRate = 10.0;
  atLeastOne = 0;
  firstSeg = lastSeg = -1;
  for ( i=0; i<MAXSEGS; i++ ) {
    segExists[i] = 0;
    if ( finalExists[i] && rampRateExists[i] ) {
      if ( rampRate[i] > 0 ) segExists[i] = 1;
      if ( segExists[i] ) {
        lastSeg = i;
        increment[i] = fabs( rampRate[i] / 60 * updateRate );
        if ( !atLeastOne ) {
          firstSeg = i;
          atLeastOne = 1;
        }
      }
      else {
	increment[i] = 0;
      }
    }
    else {
      increment[i] = 0;
    }
  }

  if ( !atLeastOne ) {
    actWin->appCtx->postMessage( activeMultSegRampButtonClass_str45 );
    return;
  }

  buttonPressed = 1;

  //fprintf( stderr, "rampRate=%g\n", rampRate[rampSegment] );
  //fprintf( stderr, "updateRate=%g\n", updateRate );
  //fprintf( stderr, "increment=%g\n", increment[rampSegment] );

  minVal = rampFinalV[firstSeg];
  minSeg = firstSeg;
  maxVal = rampFinalV[firstSeg];
  maxSeg = firstSeg;
  for ( i=firstSeg+1; i<MAXSEGS; i++ ) {
    if ( segExists[i] ) {
      if ( rampFinalV[i] < minVal ) {
        minVal = rampFinalV[i];
        minSeg = i;
      }
      if ( rampFinalV[i] > maxVal ) {
        maxVal = rampFinalV[i];
        maxSeg = i;
      }
    }
  }

  if ( firstSeg < 0 ) firstSeg = 0; // sanity checks
  if ( lastSeg < 0 ) lastSeg = 0;

  curSeg = firstSeg;
  increasing = decreasing = 0;
  if ( rampFinalV[firstSeg] < rampFinalV[lastSeg] ) {
    increasing = 1;
  }
  else if ( rampFinalV[firstSeg] > rampFinalV[lastSeg] ) {
    decreasing = 1;
  }

  monotonic = 1;
  checkVal = rampFinalV[firstSeg];
  if ( increasing ) {
    for ( i=firstSeg+1; i<MAXSEGS; i++ ) {
      if ( segExists[i] ) {
        if ( rampFinalV[i] < checkVal ) {
          monotonic = 0;
	  break;
	}
	else {
	  checkVal = rampFinalV[i];
	}
      }
    }
  }
  else if ( decreasing ) {
    for ( i=firstSeg+1; i<MAXSEGS; i++ ) {
      if ( segExists[i] ) {
        if ( rampFinalV[i] > checkVal ) {
          monotonic = 0;
	  break;
	}
	else {
	  checkVal = rampFinalV[i];
	}
      }
    }
  }
  else {
    for ( i=firstSeg+1; i<MAXSEGS; i++ ) {
      if ( segExists[i] ) {
        if ( rampFinalV[i] != checkVal ) {
          monotonic = 0;
	  break;
	}
      }
    }
  }

  //fprintf( stderr, "increasing = %-d\n", increasing );
  //fprintf( stderr, "decreasing = %-d\n", decreasing );
  //fprintf( stderr, "monotonic = %-d\n\n", monotonic );

  if ( monotonic ) {

    if ( dval <= minVal ) {

      curSeg = minSeg;

    }
    else if ( dval >= maxVal ) {

      curSeg = maxSeg;

    }
    else {

      for ( i=firstSeg; i<MAXSEGS; i++ ) {

        if ( segExists[i] ) {
          if ( increasing ) {
            if ( dval <= rampFinalV[i] ) {
              curSeg = i;
	      break;
	    }
	  }
	  else if ( decreasing ) {
            if ( dval >= rampFinalV[i] ) {
              curSeg = i;
	      break;
	    }
	  }
	  else {
	    curSeg = firstSeg;
	  }
	}

      }

    }

  }
  else {

    curSeg = firstSeg;

  }

  if ( rampStateExists ) {
    rampStatePvId->put(
     XDisplayName(actWin->appCtx->displayName),
     curSeg+1 );
  }

  rampSegment = curSeg;
  if ( rampFinalV[rampSegment] >= dval ) {
    incrementTimer = appAddTimeOut( actWin->appCtx->appContext(),
     incrementTimerValue, msrbtc_increment, this );
    incrementTimerActive = 1;
  }
  else if ( rampFinalV[rampSegment] < dval ) {
    incrementTimer = appAddTimeOut( actWin->appCtx->appContext(),
     incrementTimerValue, msrbtc_decrement, this );
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

void activeMultSegRampButtonClass::pointerIn (
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

int activeMultSegRampButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

int i, atLeastOne;

  atLeastOne = 0;
  for ( i=0; i<MAXSEGS; i++ ) {
    if ( finalExists[i] && rampRateExists[i] ) {
      atLeastOne = 1;
      break;
    }
  }

  if ( !atLeastOne ) {
    actWin->appCtx->postMessage( activeMultSegRampButtonClass_str45 );
  }

  *drag = 0;

  if ( destExists && atLeastOne )
    *focus = 1;
  else
    *focus = 0;

  if ( !destExists || !atLeastOne ) {
    *up = 0;
    *down = 0;
    return 1;
  }

  *down = 1;
  *up = 1;

  return 1;

}

int activeMultSegRampButtonClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;
expStringClass tmpStr;

  tmpStr.setRaw( destPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  destPvExpString.setRaw( tmpStr.getExpanded() );

  for ( i=0; i<MAXSEGS; i++ ) {

    tmpStr.setRaw( finalPvExpString[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    finalPvExpString[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( rampRatePvExpString[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    rampRatePvExpString[i].setRaw( tmpStr.getExpanded() );

  }

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

int activeMultSegRampButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i, stat, retStat = 1;

  stat = destPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  for ( i=0; i<MAXSEGS; i++ ) {
    stat = finalPvExpString[i].expand1st( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
    stat = rampRatePvExpString[i].expand1st( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
  }

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

int activeMultSegRampButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i, stat, retStat = 1;

  stat = destPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  for ( i=0; i<MAXSEGS; i++ ) {
    stat = finalPvExpString[i].expand2nd( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
    stat = rampRatePvExpString[i].expand2nd( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
  }

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

int activeMultSegRampButtonClass::containsMacros ( void ) {

int i;

  if ( destPvExpString.containsPrimaryMacros() ) return 1;

  for ( i=0; i<MAXSEGS; i++ ) {
    if ( finalPvExpString[i].containsPrimaryMacros() ) return 1;
    if ( rampRatePvExpString[i].containsPrimaryMacros() ) return 1;
  }

  if ( rampStatePvExpString.containsPrimaryMacros() ) return 1;

  if ( label.containsPrimaryMacros() ) return 1;

  if ( visPvExpString.containsPrimaryMacros() ) return 1;

  if ( colorPvExpString.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeMultSegRampButtonClass::executeDeferred ( void ) {

int i, nc, nsc[MAXSEGS], nrrc[MAXSEGS], nci, nd, ne, nr, nvc, nvi, nvu, ncolc,
 ncoli, ncolu, nrsc;
int stat, index, invisColor;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;

  for ( i=0; i<MAXSEGS; i++ ) {
    nsc[i] = needFinalConnectInit[i]; needFinalConnectInit[i] = 0;
    nrrc[i] = needRampRateConnectInit[i]; needRampRateConnectInit[i] = 0;
  }

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

      destPvId->add_value_callback( msrbtc_controlUpdate, this );

    }

    if ( connection.pvsConnected() ) {
      bgColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

  for ( i=0; i<MAXSEGS; i++ ) {

    if ( nsc[i] ) {

      connection.setPvConnected( (void *) finalPvConnection[i] );
      finalType[i] = (int) finalPvId[i]->get_type().type;

      if ( initialFinalValueConnection[i] ) {

        initialFinalValueConnection[i] = 0;

        finalPvId[i]->add_value_callback( msrbtc_finalUpdate, this );

      }

      if ( connection.pvsConnected() ) {
        bgColor.setConnected();
        init = 1;
        smartDrawAllActive();
      }

    }

    if ( nrrc[i] ) {

      connection.setPvConnected( (void *) rampRatePvConnection[i] );
      rampRateType[i] = (int) rampRatePvId[i]->get_type().type;

      if ( initialRampRateConnection[i] ) {

        initialRampRateConnection[i] = 0;

        rampRatePvId[i]->add_value_callback( msrbtc_rampRateUpdate, this );

      }

      if ( connection.pvsConnected() ) {
        bgColor.setConnected();
        init = 1;
        smartDrawAllActive();
      }

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

      visPvId->add_value_callback( msrbtc_visUpdate, this );

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

      colorPvId->add_value_callback( msrbtc_colorUpdate, this );

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

char *activeMultSegRampButtonClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeMultSegRampButtonClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeMultSegRampButtonClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( i == 0 ) {
      return destPvExpString.getExpanded();
    }
    else if ( i == 1 ) {
      return rampStatePvExpString.getExpanded();
    }
    else if ( i == 2 ) {
      return colorPvExpString.getExpanded();
    }
    else if ( i == 3 ) {
      return visPvExpString.getExpanded();
    }
    else if ( i < 9 ) {
      return finalPvExpString[i-4].getExpanded();
    }
    else if ( i < 14 ) {
      return rampRatePvExpString[i-9].getExpanded();
    }

  }
  else {

    if ( i == 0 ) {
      return destPvExpString.getRaw();
    }
    else if ( i == 1 ) {
      return rampStatePvExpString.getRaw();
    }
    else if ( i == 2 ) {
      return colorPvExpString.getRaw();
    }
    else if ( i == 3 ) {
      return visPvExpString.getRaw();
    }
    else if ( i < 9 ) {
      return finalPvExpString[i-4].getRaw();
    }
    else if ( i < 14 ) {
      return rampRatePvExpString[i-9].getRaw();
    }

  }

  return NULL;

}

void activeMultSegRampButtonClass::changeDisplayParams (
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

void activeMultSegRampButtonClass::changePvNames (
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

void activeMultSegRampButtonClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n
) {

int i;

  if ( max < 11 ) {
    *n = 0;
    return;
  }

  *n = 11;
  pvs[0] = destPvId;
  for ( i=0; i<MAXSEGS; i++ ) {
    pvs[i+1] = finalPvId[i];
  }
  for ( i=0; i<MAXSEGS; i++ ) {
    pvs[i+MAXSEGS+1] = rampRatePvId[i];
  }

}

char *activeMultSegRampButtonClass::getSearchString (
  int i
) {

int num = MAXSEGS + MAXSEGS + 7;
int ii, selector, index;

  if ( i == 0 ) {
    return destPvExpString.getRaw();
  }
  else if ( i == 1 ) {
    return rampStatePvExpString.getRaw();
  }
  else if ( i == 2 ) {
    return colorPvExpString.getRaw();
  }
  else if ( i == 3 ) {
    return visPvExpString.getRaw();
  }
  else if ( i == 4 ) {
    return label.getRaw();
  }
  else if ( i == 5 ) {
    return minVisString;
  }
  else if ( i == 6 ) {
    return maxVisString;
  }
  else if ( ( i > 6 ) && ( i < num ) ) {
    ii = i - 7;
    selector = ii % 2;
    index = ii / 2;
    if ( selector == 0 ) {
      return finalPvExpString[index].getRaw();
    }
    else if ( selector == 1 ) {
      return rampRatePvExpString[index].getRaw();
    }

  }

  return NULL;

}

void activeMultSegRampButtonClass::replaceString (
  int i,
  int max,
  char *string
) {

int num = MAXSEGS + MAXSEGS + 7;
int ii, selector, index;

  if ( i == 0 ) {
    destPvExpString.setRaw( string );
  }
  else if ( i == 1 ) {
    rampStatePvExpString.setRaw( string );
  }
  else if ( i == 2 ) {
    colorPvExpString.setRaw( string );
  }
  else if ( i == 3 ) {
    visPvExpString.setRaw( string );
  }
  else if ( i == 4 ) {
    label.setRaw( string );
  }
  else if ( i == 5 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( minVisString, string, l );
    minVisString[l] = 0;
  }
  else if ( i == 6 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( maxVisString, string, l );
    maxVisString[l] = 0;
  }
  else if ( ( i > 6 ) && ( i < num ) ) {
    ii = i - 7;
    selector = ii % 2;
    index = ii / 2;
    if ( selector == 0 ) {
      finalPvExpString[index].setRaw( string );
    }
    else if ( selector == 1 ) {
      rampRatePvExpString[index].setRaw( string );
    }

  }

}

// crawler functions may return blank pv names
char *activeMultSegRampButtonClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return destPvExpString.getExpanded();

}

char *activeMultSegRampButtonClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >=14 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return rampStatePvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 2 ) {
    return visPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 3 ) {
    return colorPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex < 9 ) {
    return finalPvExpString[crawlerPvIndex-3].getExpanded();
  }
  else if ( crawlerPvIndex < 14 ) {
    return rampRatePvExpString[crawlerPvIndex-9].getExpanded();
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMultSegRampButtonClassPtr ( void ) {

activeMultSegRampButtonClass *ptr;

  ptr = new activeMultSegRampButtonClass;
  return (void *) ptr;

}

void *clone_activeMultSegRampButtonClassPtr (
  void *_srcPtr )
{

activeMultSegRampButtonClass *ptr, *srcPtr;

  srcPtr = (activeMultSegRampButtonClass *) _srcPtr;

  ptr = new activeMultSegRampButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
