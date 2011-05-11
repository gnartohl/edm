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

#define __mpStrobe_cc 1

#include "mpStrobe.h"
#include "app_pkg.h"
#include "act_win.h"
#include "math.h"
#include "time.h"

static void doBlink (
  void *ptr
) {

activeMpStrobeClass *mpso = (activeMpStrobeClass *) ptr;

  if ( !mpso->activeMode ) {
    if ( mpso->isSelected() ) mpso->drawSelectBoxCorners(); //erase via xor
    mpso->smartDrawAll();
    if ( mpso->isSelected() ) mpso->drawSelectBoxCorners();
  }
  else {
    mpso->bufInvalidate();
    mpso->needDraw = 1;
    mpso->actWin->addDefExeNode( mpso->aglPtr );
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) client;

  if ( !mpso->init ) {
    mpso->needToDrawUnconnected = 1;
    mpso->needDraw = 1;
    mpso->actWin->addDefExeNode( mpso->aglPtr );
  }

  mpso->unconnectedTimer = 0;

}

static void mpsc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) client;

  mpso->actWin->setChanged();

  mpso->eraseSelectBoxCorners();
  mpso->erase();

  mpso->fgColor.setColorIndex( mpso->eBuf->bufFgColor, mpso->actWin->ci );

  mpso->bgColor.setColorIndex( mpso->eBuf->bufBgColor, mpso->actWin->ci );

  mpso->offColor.setColorIndex( mpso->eBuf->bufOffColor, mpso->actWin->ci );

  mpso->topShadowColor = mpso->eBuf->bufTopShadowColor;
  mpso->botShadowColor = mpso->eBuf->bufBotShadowColor;

  mpso->controlPvExpString.setRaw( mpso->eBuf->bufControlPvName );

  mpso->destPvExpString.setRaw( mpso->eBuf->bufDestPvName );

  mpso->onLabel.setRaw( mpso->eBuf->bufOnLabel );

  mpso->offLabel.setRaw( mpso->eBuf->bufOffLabel );

  strncpy( mpso->fontTag, mpso->fm.currentFontTag(), 63 );
  mpso->actWin->fi->loadFontTag( mpso->fontTag );
  mpso->fs = mpso->actWin->fi->getXFontStruct( mpso->fontTag );

  mpso->autoPing = mpso->eBuf->bufAutoPing;

  mpso->_3D = mpso->eBuf->buf3D;

  mpso->invisible = mpso->eBuf->bufInvisible;

  mpso->disableBtn = mpso->eBuf->bufDisableBtn;

  if ( strcmp( mpso->eBuf->bufCycleTypeStr, activeMpStrobeClass_str25 ) == 0 ) {
    mpso->cycleType = MPSC_K_TOGGLE;
  }
  else if ( strcmp( mpso->eBuf->bufCycleTypeStr, activeMpStrobeClass_str26 ) == 0 ) {
    mpso->cycleType = MPSC_K_CYCLE;
  }
  else if ( strcmp( mpso->eBuf->bufCycleTypeStr, activeMpStrobeClass_str27 ) == 0 ) {
    mpso->cycleType = MPSC_K_TRIG;
  }
  else if ( strcmp( mpso->eBuf->bufCycleTypeStr, activeMpStrobeClass_str28 ) == 0 ) {
    mpso->cycleType = MPSC_K_RANDOM;
  }

  mpso->firstVal = mpso->eBuf->bufFirstVal;

  mpso->secondVal = mpso->eBuf->bufSecondVal;

  mpso->pingOnTime = mpso->eBuf->bufPingOnTime;
  if ( mpso->pingOnTime < 0.1 ) mpso->pingOnTime = 0.1;

  mpso->pingOffTime = mpso->eBuf->bufPingOffTime;
  if ( mpso->pingOffTime < 0.1 ) mpso->pingOffTime = 0.1;

  mpso->visPvExpString.setRaw( mpso->eBuf->bufVisPvName );
  strncpy( mpso->minVisString, mpso->eBuf->bufMinVisString, 39 );
  strncpy( mpso->maxVisString, mpso->eBuf->bufMaxVisString, 39 );

  if ( mpso->eBuf->bufVisInverted )
    mpso->visInverted = 0;
  else
    mpso->visInverted = 1;

  mpso->colorPvExpString.setRaw( mpso->eBuf->bufColorPvName );

  mpso->x = mpso->eBuf->bufX;
  mpso->sboxX = mpso->eBuf->bufX;

  mpso->y = mpso->eBuf->bufY;
  mpso->sboxY = mpso->eBuf->bufY;

  mpso->w = mpso->eBuf->bufW;
  mpso->sboxW = mpso->eBuf->bufW;

  mpso->h = mpso->eBuf->bufH;
  mpso->sboxH = mpso->eBuf->bufH;

  mpso->updateDimensions();

}

static void mpsc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) client;

  mpsc_edit_update ( w, client, call );
  mpso->refresh( mpso );

}

static void mpsc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) client;

  mpsc_edit_update ( w, client, call );
  mpso->ef.popdown();
  mpso->operationComplete();

}

static void mpsc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) client;

  mpso->ef.popdown();
  mpso->operationCancel();

}

static void mpsc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) client;

  mpso->ef.popdown();
  mpso->operationCancel();
  mpso->erase();
  mpso->deleteRequest = 1;
  mpso->drawAll();

}

static void mpsc_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) userarg;

  if ( pv->is_valid() ) {

    mpso->needConnectInit = 1;

  }
  else {

    mpso->connection.setPvDisconnected( (void *) mpso->controlPvConnection );
    mpso->active = 0;
    mpso->bgColor.setDisconnected();
    mpso->offColor.setDisconnected();
    mpso->needDraw = 1;

  }

  mpso->actWin->appCtx->proc->lock();
  mpso->actWin->addDefExeNode( mpso->aglPtr );
  mpso->actWin->appCtx->proc->unlock();

}

static void mpsc_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) userarg;

  if ( pv->is_valid() ) {

    mpso->needDestConnectInit = 1;
    mpso->actWin->appCtx->proc->lock();
    mpso->actWin->addDefExeNode( mpso->aglPtr );
    mpso->actWin->appCtx->proc->unlock();

  }
  else {

    mpso->connection.setPvDisconnected( (void *) mpso->destPvConnection );
    mpso->active = 0;
    mpso->bgColor.setDisconnected();
    mpso->offColor.setDisconnected();
    mpso->needDraw = 1;

  }

  mpso->actWin->appCtx->proc->lock();
  mpso->actWin->addDefExeNode( mpso->aglPtr );
  mpso->actWin->appCtx->proc->unlock();

}

static void mpsc_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) userarg;

  mpso->actWin->appCtx->proc->lock();

  mpso->curControlV = pv->get_double();
  mpso->needRefresh = 1;

  if ( ( mpso->curControlV != 0 ) &&
       !mpso->pingTimerActive &&
       ( mpso->cycleType == MPSC_K_TRIG ) ) {
    mpso->needDestUpdate = 1;
  }

  mpso->actWin->addDefExeNode( mpso->aglPtr );

  mpso->actWin->appCtx->proc->unlock();

}

static void mpsc_destUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) userarg;

  mpso->actWin->appCtx->proc->lock();

  mpso->curDestV = pv->get_double();
  mpso->destV = mpso->curDestV;
  mpso->needDestUpdate = 1;
  mpso->actWin->addDefExeNode( mpso->aglPtr );

  mpso->actWin->appCtx->proc->unlock();

}

static void mpsc_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) userarg;

  if ( pv->is_valid() ) {

    mpso->needVisConnectInit = 1;

  }
  else {

    mpso->connection.setPvDisconnected( (void *) mpso->visPvConnection );
    mpso->active = 0;
    mpso->bgColor.setDisconnected();
    mpso->offColor.setDisconnected();
    mpso->needDraw = 1;

  }

  mpso->actWin->appCtx->proc->lock();
  mpso->actWin->addDefExeNode( mpso->aglPtr );
  mpso->actWin->appCtx->proc->unlock();

}

static void mpsc_visUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) userarg;

  mpso->curVisValue = pv->get_double();

  mpso->actWin->appCtx->proc->lock();
  mpso->needVisUpdate = 1;
  mpso->actWin->addDefExeNode( mpso->aglPtr );
  mpso->actWin->appCtx->proc->unlock();

}

static void mpsc_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) userarg;

  if ( pv->is_valid() ) {

    mpso->needColorConnectInit = 1;

  }
  else {

    mpso->connection.setPvDisconnected( (void *) mpso->colorPvConnection );
    mpso->active = 0;
    mpso->bgColor.setDisconnected();
    mpso->offColor.setDisconnected();
    mpso->needDraw = 1;

  }

  mpso->actWin->appCtx->proc->lock();
  mpso->actWin->addDefExeNode( mpso->aglPtr );
  mpso->actWin->appCtx->proc->unlock();

}

static void mpsc_colorUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) userarg;

  mpso->curColorValue = pv->get_double();

  mpso->actWin->appCtx->proc->lock();
  mpso->needColorUpdate = 1;
  mpso->actWin->addDefExeNode( mpso->aglPtr );
  mpso->actWin->appCtx->proc->unlock();

}

static void mpsc_ping (
  XtPointer client,
  XtIntervalId *id )
{

activeMpStrobeClass *mpso = (activeMpStrobeClass *) client;
double dval;

  if ( !mpso->pingTimerActive ) {
    mpso->pingTimer = 0;
    return;
  }

  if ( !( mpso->controlV ) ) {
    mpso->pingTimerActive = 0;
    return;
  }

  if ( mpso->destPvId ) {

    if ( mpso->destType == ProcessVariable::specificType::text ) {

      char time_string[31+1];
      sys_get_datetime_string( 31, time_string );

      if ( mpso->destSize > 1 ) {
        mpso->destPvId->putArrayText( time_string );
      }
      else {
        mpso->destPvId->putText(
         XDisplayName(mpso->actWin->appCtx->displayName),
         time_string );
      }

    }
    else {

      dval = mpso->destV;
      //printf( "dval = %-f, firstVal = %-f, secondVal = %-f\n",
      // dval, mpso->firstVal, mpso->secondVal );

      switch ( mpso->cycleType ) {

      case MPSC_K_CYCLE:

	dval += 1;
        if ( ( dval > mpso->secondVal ) || ( dval < mpso->firstVal ) ) {
          dval = mpso->firstVal;
	}

	break;

      case MPSC_K_TRIG:

	if ( round( dval ) == round( mpso->firstVal ) ) {
          dval = mpso->secondVal;
          mpso->actWin->appCtx->proc->lock();
          mpso->needRefresh = 1;
          mpso->actWin->addDefExeNode( mpso->aglPtr );
          mpso->actWin->appCtx->proc->unlock();
	}

	break;

      case MPSC_K_RANDOM:

        dval = (double) rand_r( &mpso->randSeed ) / (double) RAND_MAX;

	break;

      case MPSC_K_TOGGLE:
      default:

        if ( dval ) {
          dval = 0;
        }
        else {
          dval = 1;
        }

	break;

      }

      mpso->destPvId->put(
       XDisplayName(mpso->actWin->appCtx->displayName),
       dval );

      mpso->destV = dval;

    }

  }

  if ( mpso->cycleType != MPSC_K_TRIG ) {
    mpso->pingTimerValue = mpso->getPingTimerValue();
    mpso->pingTimer = appAddTimeOut(
     mpso->actWin->appCtx->appContext(),
     mpso->pingTimerValue, mpsc_ping, client );
  }
  else {
    mpso->pingTimerActive = 0;
  } 

}

activeMpStrobeClass::activeMpStrobeClass ( void ) {

  name = new char[strlen("activeMpStrobeClass")+1];
  strcpy( name, "activeMpStrobeClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  buttonPressed = 0;
  state = MPSC_IDLE;
  autoPing = 0;
  _3D = 1;
  invisible = 0;
  disableBtn = 0;
  cycleType = MPSC_K_TOGGLE;
  firstVal = 0;
  secondVal = 1;
  pingOnTime = 1.0;
  pingOffTime = 1.0;
  curDestV = destV = 0.0;
  curControlV = controlV = 0.0;
  unconnectedTimer = 0;
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  connection.setMaxPvs( 5 );
  activeMode = 0;
  eBuf = NULL;
  controlType = destType = destSize = 0;
  randSeed = (unsigned int) this + (unsigned int) clock();

  setBlinkFunction( (void *) doBlink );

}

// copy constructor
activeMpStrobeClass::activeMpStrobeClass
 ( const activeMpStrobeClass *source ) {

activeGraphicClass *mpso = (activeGraphicClass *) this;

  mpso->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMpStrobeClass")+1];
  strcpy( name, "activeMpStrobeClass" );

  buttonPressed = 0;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );
  offColor.copy( source->offColor );

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  controlPvExpString.copy( source->controlPvExpString );
  destPvExpString.copy( source->destPvExpString );
  visPvExpString.copy( source->visPvExpString );
  colorPvExpString.copy( source->colorPvExpString );

  onLabel.copy( source->onLabel );

  offLabel.copy( source->offLabel );

  state = MPSC_IDLE;
  autoPing = source->autoPing;
  _3D = source->_3D;
  invisible = source->invisible;
  disableBtn = source->disableBtn;
  cycleType = source->cycleType;
  firstVal = source->firstVal;
  secondVal = source->secondVal;
  pingOnTime = source->pingOnTime;
  pingOffTime = source->pingOffTime;
  curDestV = 0.0;
  unconnectedTimer = 0;

  visibility = 0;
  prevVisibility = -1;
  visInverted = source->visInverted;
  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );
  activeMode = 0;
  eBuf = NULL;
  controlType = destType = destSize = 0;
  randSeed = (unsigned int) this + (unsigned int) clock();

  connection.setMaxPvs( 5 );

  setBlinkFunction( (void *) doBlink );

  updateDimensions();

}

activeMpStrobeClass::~activeMpStrobeClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

int activeMpStrobeClass::getPingTimerValue ( void ) {

int ptv;

  if ( destExists && destPvId->is_valid() ) {

    if ( cycleType == MPSC_K_TOGGLE ) {

      if ( destV == 0 ) {
        ptv = (int) ( 1000.0 * pingOffTime );
      }
      else {
        ptv = (int) ( 1000.0 * pingOnTime );
      }

    }
    else {

      ptv = (int) ( 1000.0 * pingOnTime );

    }

  }
  else {

    ptv = (int) ( 1000.0 * pingOnTime );

  }

  return ptv;

}

int activeMpStrobeClass::createInteractive (
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
  offColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
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

int activeMpStrobeClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

  major = MPSC_MAJOR_VERSION;
  minor = MPSC_MINOR_VERSION;
  release = MPSC_RELEASE;

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
  tag.loadW( "offColor", actWin->ci, &offColor );
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "controlPv", &controlPvExpString, emptyStr );
  tag.loadW( "destValuePv", &destPvExpString, emptyStr );
  tag.loadW( "pingOnTime", &pingOnTime, &dzero );
  tag.loadW( "pingOffTime", &pingOffTime, &dzero );
  tag.loadW( "onLabel", &onLabel, emptyStr );
  tag.loadW( "offLabel", &offLabel, emptyStr );
  tag.loadBoolW( "autoPing", &autoPing, &zero );
  tag.loadBoolW( "3d", &_3D, &zero );
  tag.loadBoolW( "invisible", &invisible, &zero );
  tag.loadBoolW( "disableBtn", &disableBtn, &zero );
  tag.loadW( "cycleType", &cycleType, &zero );
  tag.loadW( "firstVal", &firstVal, &dzero );
  tag.loadW( "secondVal", &secondVal, &dzero );
  tag.loadW( "font", fontTag );
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

int activeMpStrobeClass::createFromFile (
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
  tag.loadR( "offColor", actWin->ci, &offColor );
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "controlPv", &controlPvExpString, emptyStr );
  tag.loadR( "destValuePv", &destPvExpString, emptyStr );
  tag.loadR( "pingOnTime", &pingOnTime, &dzero );
  tag.loadR( "pingOffTime", &pingOffTime, &dzero );
  tag.loadR( "onLabel", &onLabel, emptyStr );
  tag.loadR( "offLabel", &offLabel, emptyStr );
  tag.loadR( "autoPing", &autoPing, &zero );
  tag.loadR( "3d", &_3D, &zero );
  tag.loadR( "invisible", &invisible, &zero );
  tag.loadR( "disableBtn", &disableBtn, &zero );
  tag.loadR( "cycleType", &cycleType, &zero );
  tag.loadR( "firstVal", &firstVal, &dzero );
  tag.loadR( "secondVal", &secondVal, &dzero );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "visPv", &visPvExpString, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "colorPv", &colorPvExpString, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( pingOnTime < 0.1 ) pingOnTime = 0.1;
  if ( pingOffTime < 0.1 ) pingOffTime = 0.1;

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > MPSC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeMpStrobeClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeMpStrobeClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMpStrobeClass_str2, 31 );

  Strncat( title, activeMpStrobeClass_str3, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufFgColor = fgColor.pixelIndex();

  eBuf->bufBgColor = bgColor.pixelIndex();

  eBuf->bufOffColor = offColor.pixelIndex();

  eBuf->bufTopShadowColor = topShadowColor;
  eBuf->bufBotShadowColor = botShadowColor;

  if ( controlPvExpString.getRaw() )
    strncpy( eBuf->bufControlPvName, controlPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufControlPvName, "" );

  if ( destPvExpString.getRaw() )
    strncpy( eBuf->bufDestPvName, destPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufDestPvName, "" );

  eBuf->bufPingOnTime = pingOnTime;
  eBuf->bufPingOffTime = pingOffTime;

  if ( onLabel.getRaw() )
    strncpy( eBuf->bufOnLabel, onLabel.getRaw(), 39 );
  else
    strncpy( eBuf->bufOnLabel, "", 39 );

  if ( offLabel.getRaw() )
    strncpy( eBuf->bufOffLabel, offLabel.getRaw(), 39 );
  else
    strncpy( eBuf->bufOffLabel, "", 39 );

  eBuf->bufAutoPing = autoPing;
  eBuf->buf3D = _3D;
  eBuf->bufInvisible = invisible;
  eBuf->bufDisableBtn = disableBtn;

  if ( cycleType == MPSC_K_TOGGLE ) {
    strncpy( eBuf->bufCycleTypeStr, activeMpStrobeClass_str25, 31 );
    eBuf->bufCycleTypeStr[31] = 0;
  }
  else if ( cycleType == MPSC_K_CYCLE ) {
    strncpy( eBuf->bufCycleTypeStr, activeMpStrobeClass_str26, 31 );
    eBuf->bufCycleTypeStr[31] = 0;
  }
  else if ( cycleType == MPSC_K_TRIG ) {
    strncpy( eBuf->bufCycleTypeStr, activeMpStrobeClass_str27, 31 );
    eBuf->bufCycleTypeStr[31] = 0;
  }
  else if ( cycleType == MPSC_K_RANDOM ) {
    strncpy( eBuf->bufCycleTypeStr, activeMpStrobeClass_str28, 31 );
    eBuf->bufCycleTypeStr[31] = 0;
  }

  eBuf->bufFirstVal = firstVal;

  eBuf->bufSecondVal = secondVal;

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

  ef.addTextField( activeMpStrobeClass_str4, 35, &eBuf->bufX );
  ef.addTextField( activeMpStrobeClass_str5, 35, &eBuf->bufY );
  ef.addTextField( activeMpStrobeClass_str6, 35, &eBuf->bufW );
  ef.addTextField( activeMpStrobeClass_str7, 35, &eBuf->bufH );
  ef.addTextField( activeMpStrobeClass_str8, 35, eBuf->bufControlPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeMpStrobeClass_str9, 35, eBuf->bufDestPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addToggle( activeMpStrobeClass_str11, &eBuf->bufAutoPing );
  ef.addToggle( activeMpStrobeClass_str12, &eBuf->buf3D );
  ef.addToggle( activeMpStrobeClass_str13, &eBuf->bufInvisible );
  ef.addToggle( activeMpStrobeClass_str34, &eBuf->bufDisableBtn );

  ef.addOption( activeMpStrobeClass_str23, activeMpStrobeClass_str24,
   eBuf->bufCycleTypeStr, 31 );
  optEntry = (optionEntry *) ef.getCurItem();

  ef.addTextField( activeMpStrobeClass_str10, 35, &eBuf->bufPingOnTime );

  ef.addTextField( activeMpStrobeClass_str38, 35, &eBuf->bufPingOffTime );
  offTimeEntry = ef.getCurItem();
  optEntry->addDependency( 0, offTimeEntry );

  ef.addTextField( activeMpStrobeClass_str36, 35, &eBuf->bufFirstVal );
  firstValEntry = ef.getCurItem();
  optEntry->addDependency( 1, firstValEntry );
  optEntry->addDependency( 3, firstValEntry );

  ef.addTextField( activeMpStrobeClass_str37, 35, &eBuf->bufSecondVal );
  secondValEntry = ef.getCurItem();
  optEntry->addDependency( 1, secondValEntry );
  optEntry->addDependency( 3, secondValEntry );

  optEntry->addDependencyCallbacks();

  ef.addTextField( activeMpStrobeClass_str14, 35, eBuf->bufOnLabel, 39 );
  ef.addTextField( activeMpStrobeClass_str21, 35, eBuf->bufOffLabel, 39 );
  ef.addColorButton( activeMpStrobeClass_str16, actWin->ci, &eBuf->fgCb,
   &eBuf->bufFgColor );
  ef.addColorButton( activeMpStrobeClass_str17, actWin->ci, &eBuf->bgCb,
   &eBuf->bufBgColor );
  ef.addColorButton( activeMpStrobeClass_str22, actWin->ci, &eBuf->offCb,
   &eBuf->bufOffColor );
  ef.addColorButton( activeMpStrobeClass_str18, actWin->ci, &eBuf->topShadowCb,
   &eBuf->bufTopShadowColor );
  ef.addColorButton( activeMpStrobeClass_str19, actWin->ci, &eBuf->botShadowCb,
   &eBuf->bufBotShadowColor );

  ef.addFontMenu( activeMpStrobeClass_str15, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeMpStrobeClass_str33, 30, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeMpStrobeClass_str29, 30, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeMpStrobeClass_str30, &eBuf->bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeMpStrobeClass_str31, 30, eBuf->bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeMpStrobeClass_str32, 30, eBuf->bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  return 1;

}

int activeMpStrobeClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( mpsc_edit_ok, mpsc_edit_apply, mpsc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMpStrobeClass::edit ( void ) {

  this->genericEdit();
  ef.finished( mpsc_edit_ok, mpsc_edit_apply, mpsc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMpStrobeClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMpStrobeClass::eraseActive ( void ) {

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

int activeMpStrobeClass::draw ( void ) {

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

    actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

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

    if ( onLabel.getRaw() )
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, onLabel.getRaw() );
    else
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, "" );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeMpStrobeClass::drawActive ( void ) {

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

  // set color based on even/odd state of destination
  if ( (int) destV & 1 ) {
    actWin->executeGc.setFG( bgColor.getIndex(), &blink );
  }
  else {
    actWin->executeGc.setFG( offColor.getIndex(), &blink );
  }

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( !_3D ) {

    if ( (int) destV & 1 ) {
      actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );
    }
    else {
      actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );
    }

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

  // set label based on even/odd state of destination
    if ( (int) destV & 1 ) {

      if ( onLabel.getExpanded() )
        strncpy( string, onLabel.getExpanded(), 39 );
      else
        strncpy( string, "", 39 );

    }
    else {

      if ( offLabel.getExpanded() )
        strncpy( string, offLabel.getExpanded(), 39 );
      else
        strncpy( string, "", 39 );

    }

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

int activeMpStrobeClass::activate (
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

      needConnectInit = needDestConnectInit = needCtlInfoInit = 
       needRefresh = needErase = needDraw = needVisConnectInit =
       needVisInit = needVisUpdate = needColorConnectInit =
       needColorInit = needColorUpdate = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      init = 0;
      aglPtr = ptr;
      pingTimer = 0;
      pingTimerActive = 0;
      destV = 0;
      controlPvId = visPvId = colorPvId = destPvId = NULL;
      initialConnection = initialDestValueConnection =
       initialVisConnection = initialColorConnection = -1;

      active = buttonPressed = 0;
      activeMode = 1;

      if ( pingOnTime < 0.1 ) pingOnTime = 0.1;
      if ( pingOffTime < 0.1 ) pingOffTime = 0.1;

      if ( !controlPvExpString.getExpanded() ||
         blankOrComment( controlPvExpString.getExpanded() ) ) {
        controlExists = 0;
        curControlV = controlV = 1.0;
      }
      else {
        controlExists = 1;
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

      if ( !destPvExpString.getExpanded() ||
         blankOrComment( destPvExpString.getExpanded() ) ) {
        destExists = 0;
      }
      else {
        destExists = 1;
        connection.addPv();
      }

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      opStat = 1;

      if ( controlExists ) {

	controlPvId = the_PV_Factory->create( controlPvExpString.getExpanded() );
	if ( controlPvId ) {
	  controlPvId->add_conn_state_callback( mpsc_monitor_control_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeMpStrobeClass_str20 );
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
	  visPvId->add_conn_state_callback( mpsc_monitor_vis_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeMpStrobeClass_str20 );
          opStat = 0;
        }

      }

      if ( colorExists ) {

	colorPvId = the_PV_Factory->create( colorPvExpString.getExpanded() );
	if ( colorPvId ) {
	  colorPvId->add_conn_state_callback(
           mpsc_monitor_color_connect_state, this );
	}
	else {
          fprintf( stderr, activeMpStrobeClass_str20 );
          opStat = 0;
        }

      }

      if ( destExists ) {

	destPvId = the_PV_Factory->create( destPvExpString.getExpanded() );
	if ( destPvId ) {
	  destPvId->add_conn_state_callback( mpsc_monitor_dest_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeMpStrobeClass_str20 );
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

int activeMpStrobeClass::deactivate (
  int pass
) {

double dval;

  if ( pass == 1 ) {

  if ( controlExists ) {
    if ( controlPvId ) {
      dval = 0;
      controlPvId->put(
       XDisplayName(actWin->appCtx->displayName),
       dval );
    }
  }

  active = 0;
  activeMode = 0;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  if ( pingTimerActive ) {
    if ( pingTimer ) {
      //actWin->appCtx->postMessage( activeMpStrobeClass_str35 );
      XtRemoveTimeOut( pingTimer );
      pingTimer = 0;
    }
    pingTimerActive = 0;
  }

  if ( controlExists ) {
    if ( controlPvId ) {
      controlPvId->remove_conn_state_callback( mpsc_monitor_control_connect_state,
       this );
      controlPvId->remove_value_callback( mpsc_controlUpdate, this );
      controlPvId->release();
      controlPvId = NULL;
    }
  }

  if ( visExists ) {
    if ( visPvId ) {
      visPvId->remove_conn_state_callback( mpsc_monitor_vis_connect_state,
       this );
      visPvId->remove_value_callback( mpsc_visUpdate, this );
      visPvId->release();
      visPvId = NULL;
    }
  }

  if ( colorExists ) {
    if ( colorPvId ) {
      colorPvId->remove_conn_state_callback( mpsc_monitor_color_connect_state,
       this );
      colorPvId->remove_value_callback( mpsc_colorUpdate, this );
      colorPvId->release();
      colorPvId = NULL;
    }
  }

  if ( destExists ) {
    if ( destPvId ) {
      destPvId->remove_conn_state_callback( mpsc_monitor_dest_connect_state,
       this );
      destPvId->remove_value_callback( mpsc_destUpdate, this );
      destPvId->release();
      destPvId = NULL;
    }
  }

  }

  return 1;

}

void activeMpStrobeClass::updateDimensions ( void )
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

void activeMpStrobeClass::btnUp (
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

void activeMpStrobeClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

double dval;

  *action = 0;

  if ( !enabled || !init || !visibility ) return;

  if ( controlExists ) {
    if ( controlPvId ) {
      if ( !controlPvId->have_write_access() ) return;
    }
  }

  if ( buttonPressed ) {

    if ( pingTimerActive ) {
      if ( pingTimer ) {
        XtRemoveTimeOut( pingTimer );
        pingTimer = 0;
      }
      pingTimerActive = 0;
    }

    if ( controlExists ) {
      if ( controlPvId ) {
        dval = 0;
        controlPvId->put(
         XDisplayName(actWin->appCtx->displayName),
         dval );
      }
    }

    return;

  }

  if ( controlExists ) {
    if ( controlPvId ) {
    dval = 1;
    controlPvId->put(
     XDisplayName(actWin->appCtx->displayName),
     dval );
    }
  }

  if ( !pingTimerActive && ( cycleType != MPSC_K_TRIG ) ) {
    pingTimerValue = getPingTimerValue();
    pingTimer = appAddTimeOut( actWin->appCtx->appContext(),
     pingTimerValue, mpsc_ping, this );
    pingTimerActive = 1;
  }

  if ( !pingTimerActive && ( cycleType == MPSC_K_TRIG ) ) {
    actWin->appCtx->proc->lock();
    needDestUpdate = 1;
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
  }

}

void activeMpStrobeClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled || !init || !visibility ) return;

  if ( controlExists ) {
    if ( controlPvId ) {
      if ( !controlPvId->have_write_access() ) {
        actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );
      }
      else {
        actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_DEFAULT );
      }
    }
  }

  activeGraphicClass::pointerIn( _x, _y, buttonState );

}

int activeMpStrobeClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;

  if ( controlExists && !disableBtn )
    *focus = 1;
  else
    *focus = 0;

  if ( !controlExists || disableBtn ) {
    *up = 0;
    *down = 0;
    return 1;
  }

  *down = 1;
  *up = 1;

  return 1;

}

int activeMpStrobeClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( controlPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  controlPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( destPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  destPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( onLabel.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  onLabel.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( offLabel.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  offLabel.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( visPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  visPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( colorPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  colorPvExpString.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeMpStrobeClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = controlPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = destPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = onLabel.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = offLabel.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeMpStrobeClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = controlPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = destPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = onLabel.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = offLabel.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return stat;

}

int activeMpStrobeClass::containsMacros ( void ) {

  if ( controlPvExpString.containsPrimaryMacros() ) return 1;

  if ( destPvExpString.containsPrimaryMacros() ) return 1;

  if ( onLabel.containsPrimaryMacros() ) return 1;

  if ( offLabel.containsPrimaryMacros() ) return 1;

  if ( visPvExpString.containsPrimaryMacros() ) return 1;

  if ( colorPvExpString.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeMpStrobeClass::executeDeferred ( void ) {

int nc, ndc, nci, nd, ne, nr, nvc, nvi, nvu, ncolc, ncoli, ncolu, ndu;
int stat, index, invisColor;
double dval;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  ndc = needDestConnectInit; needDestConnectInit = 0;
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
  ndu = needDestUpdate; needDestUpdate = 0;
  visValue = curVisValue;
  colorValue = curColorValue;
  controlV = curControlV;
  destV = curDestV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    connection.setPvConnected( (void *) controlPvConnection );
    controlType = (int) controlPvId->get_type().type;

    controlV = curControlV = controlPvId->get_double();

    nci = 1;

  }

  if ( nci ) {

    if ( initialConnection ) {

      initialConnection = 0;

      controlPvId->add_value_callback( mpsc_controlUpdate, this );

    }

    if ( connection.pvsConnected() ) {
      if ( autoPing ) {
        if ( !pingTimerActive && ( cycleType != MPSC_K_TRIG ) ) {
          pingTimerValue = getPingTimerValue();
          pingTimer = appAddTimeOut(
           actWin->appCtx->appContext(),
           pingTimerValue, mpsc_ping, this );
	  pingTimerActive = 1;
	}
        dval = 1;
        controlPvId->put(
         XDisplayName(actWin->appCtx->displayName),
         dval );
      }
      bgColor.setConnected();
      offColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

  if ( ndc ) {

    connection.setPvConnected( (void *) destPvConnection );
    destType = (int) destPvId->get_type().type;
    destSize = destPvId->get_dimension();

    if ( destType == ProcessVariable::specificType::chr ) {
      if ( destPvId->get_dimension() > 1 ) {
        destType = ProcessVariable::specificType::text;
      }
    }

    if ( destType == ProcessVariable::specificType::text ) {
      cycleType = MPSC_K_TOGGLE;
    }

    if ( initialDestValueConnection ) {

      initialDestValueConnection = 0;

      destPvId->add_value_callback( mpsc_destUpdate, this );

    }

    if ( connection.pvsConnected() ) {
      if ( autoPing ) {
        if ( !pingTimerActive && ( cycleType != MPSC_K_TRIG ) ) {
          pingTimerValue = getPingTimerValue();
          pingTimer = appAddTimeOut(
           actWin->appCtx->appContext(),
           pingTimerValue, mpsc_ping, this );
	  pingTimerActive = 1;
	}
        if ( controlExists ) {
          if ( controlPvId ) {
          dval = 1;
          controlPvId->put(
           XDisplayName(actWin->appCtx->displayName),
           dval );
	  }
	}
      }
      bgColor.setConnected();
      offColor.setConnected();
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

      visPvId->add_value_callback( mpsc_visUpdate, this );

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
      if ( autoPing ) {
        if ( !pingTimerActive && ( cycleType != MPSC_K_TRIG ) ) {
          pingTimerValue = getPingTimerValue();
          pingTimer = appAddTimeOut(
           actWin->appCtx->appContext(),
           pingTimerValue, mpsc_ping, this );
	  pingTimerActive = 1;
	}
        if ( controlExists ) {
          if ( controlPvId ) {
            dval = 1;
            controlPvId->put(
             XDisplayName(actWin->appCtx->displayName),
             dval );
	  }
	}
      }
      bgColor.setConnected();
      offColor.setConnected();
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

      colorPvId->add_value_callback( mpsc_colorUpdate, this );

    }

    invisColor = 0;

    index = actWin->ci->evalRule( bgColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    bgColor.changeIndex( index, actWin->ci );

    index = actWin->ci->evalRule( offColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    offColor.changeIndex( index, actWin->ci );

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
      if ( autoPing ) {
        if ( !pingTimerActive && ( cycleType != MPSC_K_TRIG ) ) {
          pingTimerValue = getPingTimerValue();
          pingTimer = appAddTimeOut(
           actWin->appCtx->appContext(),
           pingTimerValue, mpsc_ping, this );
	  pingTimerActive = 1;
	}
        if ( controlExists ) {
          if ( controlPvId ) {
            dval = 1;
            controlPvId->put(
             XDisplayName(actWin->appCtx->displayName),
             dval );
	  }
	}
      }
      bgColor.setConnected();
      offColor.setConnected();
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

    if ( controlV ) {
      buttonPressed = 1;
    }
    else {
      buttonPressed = 0;
    }

    smartDrawAllActive();

  }

//----------------------------------------------------------------------------

  if ( ndu ) {

    if ( cycleType == MPSC_K_TRIG ) {

      //printf( "trig - round(destV) = %-f, round(firstVal) = %-f, round(secondVal) = %-f\n",
      // round(destV), round(firstVal), round(secondVal) );

      if ( round( destV ) == round( firstVal ) ) {
        if ( !pingTimerActive ) {
          pingTimerValue = getPingTimerValue();
          pingTimer = appAddTimeOut(
           actWin->appCtx->appContext(),
           pingTimerValue, mpsc_ping, this );
	  pingTimerActive = 1;
	}
      }

    }

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

    index = actWin->ci->evalRule( offColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    offColor.changeIndex( index, actWin->ci );

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

char *activeMpStrobeClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeMpStrobeClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeMpStrobeClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( i == 0 ) {
      return controlPvExpString.getExpanded();
    }
    else if ( i == 1 ) {
      return destPvExpString.getExpanded();
    }
    else if ( i == 2 ) {
      return colorPvExpString.getExpanded();
    }
    else {
      return visPvExpString.getExpanded();
    }

  }
  else {

    if ( i == 0 ) {
      return controlPvExpString.getRaw();
    }
    else if ( i == 1 ) {
      return destPvExpString.getRaw();
    }
    else if ( i == 2 ) {
      return colorPvExpString.getRaw();
    }
    else {
      return visPvExpString.getRaw();
    }

  }

}

void activeMpStrobeClass::changeDisplayParams (
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

void activeMpStrobeClass::changePvNames (
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
      controlPvExpString.setRaw( ctlPvs[0] );
    }
  }

  if ( flag & ACTGRF_VISPVS_MASK ) {
    if ( numVisPvs ) {
      visPvExpString.setRaw( ctlPvs[0] );
    }
  }

}

void activeMpStrobeClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 2 ) {
    *n = 0;
    return;
  }

  *n = 2;
  pvs[0] = controlPvId;
  pvs[1] = destPvId;

}

// crawler functions may return blank pv names
char *activeMpStrobeClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return controlPvExpString.getExpanded();

}

char *activeMpStrobeClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >=4 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return destPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 2 ) {
    return colorPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 3 ) {
    return visPvExpString.getExpanded();
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMpStrobeClassPtr ( void ) {

activeMpStrobeClass *ptr;

  ptr = new activeMpStrobeClass;
  return (void *) ptr;

}

void *clone_activeMpStrobeClassPtr (
  void *_srcPtr )
{

activeMpStrobeClass *ptr, *srcPtr;

  srcPtr = (activeMpStrobeClass *) _srcPtr;

  ptr = new activeMpStrobeClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
