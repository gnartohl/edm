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

#define __updownButton_cc 1

#include "updownButton.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) {

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) ptr;

  if ( !udbto->activeMode ) {
    if ( udbto->isSelected() ) udbto->drawSelectBoxCorners(); //erase via xor
    udbto->smartDrawAll();
    if ( udbto->isSelected() ) udbto->drawSelectBoxCorners();
  }
  else {
    udbto->bufInvalidate();
    udbto->needDraw = 1;
    udbto->actWin->addDefExeNode( udbto->aglPtr );
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;

  if ( !udbto->init ) {
    udbto->needToDrawUnconnected = 1;
    udbto->needDraw = 1;
    udbto->actWin->addDefExeNode( udbto->aglPtr );
  }

  udbto->unconnectedTimer = 0;

}

static void udbtoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;

  udbto->keyPadOpen = 0;

}

static void udbtoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

double v;
activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;

  udbto->keyPadOpen = 0;

  if ( udbto->kpDest == udbto->kpCoarseDest ) {
    udbto->coarse = udbto->kpDouble;
  }
  else if ( udbto->kpDest == udbto->kpFineDest ) {
    udbto->fine = udbto->kpDouble;
  }
  else if ( udbto->kpDest == udbto->kpRateDest ) {
    udbto->rate = udbto->kpDouble;
    udbto->incrementTimerValue = (int) ( 1000.0 * udbto->rate );
    if ( udbto->incrementTimerValue < 50 ) udbto->incrementTimerValue = 50;
  }
  else if ( udbto->kpDest == udbto->kpValueDest ) {
    if ( udbto->destExists ) {
      if ( udbto->kpDouble < udbto->minDv ) {
        v = udbto->minDv;
      }
      else if ( udbto->kpDouble > udbto->maxDv ) {
        v = udbto->maxDv;
      }
      else {
	v = udbto->kpDouble;
      }
      udbto->destPvId->put(
       XDisplayName(udbto->actWin->appCtx->displayName), v );
    }
  }

}

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

double v;
activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;
Widget parent;

  if ( useAppTopParent() ) {
    parent = udbto->actWin->appCtx->apptop();
  }
  else {
    parent = udbto->actWin->top;
  }

  if ( w == udbto->pbCoarse ) {

    udbto->kpDest = udbto->kpCoarseDest;
    udbto->kp.create( parent,
     udbto->rootX, udbto->rootY, "", &udbto->kpDouble,
     (void *) client,
     (XtCallbackProc) udbtoSetKpDoubleValue,
     (XtCallbackProc) udbtoCancelKp );
    udbto->keyPadOpen = 1;

  }
  else if ( w == udbto->pbFine ) {

    udbto->kpDest = udbto->kpFineDest;
    udbto->kp.create( parent,
     udbto->rootX, udbto->rootY, "", &udbto->kpDouble,
     (void *) client,
     (XtCallbackProc) udbtoSetKpDoubleValue,
     (XtCallbackProc) udbtoCancelKp );
    udbto->keyPadOpen = 1;

  }
  else if ( w == udbto->pbRate ) {

    udbto->kpDest = udbto->kpRateDest;
    udbto->kp.create( parent,
     udbto->rootX, udbto->rootY, "", &udbto->kpDouble,
     (void *) client,
     (XtCallbackProc) udbtoSetKpDoubleValue,
     (XtCallbackProc) udbtoCancelKp );
    udbto->keyPadOpen = 1;

  }
  else if ( w == udbto->pbValue ) {

    udbto->kpDest = udbto->kpValueDest;
    udbto->kp.create( parent,
     udbto->rootX, udbto->rootY, "", &udbto->kpDouble,
     (void *) client,
     (XtCallbackProc) udbtoSetKpDoubleValue,
     (XtCallbackProc) udbtoCancelKp );
    udbto->keyPadOpen = 1;

  }
  else if ( w == udbto->pbSave ) {

    if ( udbto->savePvConnected ) {
      udbto->savePvId->put(
       XDisplayName(udbto->actWin->appCtx->displayName),
       udbto->curControlV );
    }
    else {
      XBell( udbto->actWin->d, 50 );
    }

  }
  else if ( w == udbto->pbRestore ) {

    if ( udbto->savePvConnected ) {
      if ( udbto->curSaveV < udbto->minDv ) {
        v = udbto->minDv;
      }
      else if ( udbto->curSaveV > udbto->maxDv ) {
        v = udbto->maxDv;
      }
      else {
	v = udbto->curSaveV;
      }
      udbto->destPvId->put(
       XDisplayName(udbto->actWin->appCtx->displayName), v );
    }
    else {
      XBell( udbto->actWin->d, 50 );
    }

  }

}

static void udbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;

  udbto->actWin->setChanged();

  udbto->eraseSelectBoxCorners();
  udbto->erase();

  udbto->fgColor.setColorIndex( udbto->eBuf->bufFgColor, udbto->actWin->ci );

  udbto->bgColor.setColorIndex( udbto->eBuf->bufBgColor, udbto->actWin->ci );

  udbto->topShadowColor = udbto->eBuf->bufTopShadowColor;
  udbto->botShadowColor = udbto->eBuf->bufBotShadowColor;

  udbto->destPvExpString.setRaw( udbto->eBuf->bufDestPvName );

  udbto->savePvExpString.setRaw( udbto->eBuf->bufSavePvName );

  udbto->fineExpString.setRaw( udbto->eBuf->bufFine );

  udbto->coarseExpString.setRaw( udbto->eBuf->bufCoarse );

  udbto->label.setRaw( udbto->eBuf->bufLabel );

  strncpy( udbto->fontTag, udbto->fm.currentFontTag(), 63 );
  udbto->actWin->fi->loadFontTag( udbto->fontTag );
  udbto->fs = udbto->actWin->fi->getXFontStruct( udbto->fontTag );

  udbto->_3D = udbto->eBuf->buf3D;

  udbto->invisible = udbto->eBuf->bufInvisible;

  udbto->rate = udbto->eBuf->bufRate;

  udbto->limitsFromDb = udbto->eBuf->bufLimitsFromDb;

  udbto->efScaleMin = udbto->eBuf->bufEfScaleMin;
  udbto->efScaleMax = udbto->eBuf->bufEfScaleMax;

  udbto->minDv = udbto->scaleMin = udbto->efScaleMin.value();
  udbto->maxDv = udbto->scaleMax = udbto->efScaleMax.value();

  udbto->visPvExpString.setRaw( udbto->eBuf->bufVisPvName );
  strncpy( udbto->minVisString, udbto->eBuf->bufMinVisString, 39 );
  strncpy( udbto->maxVisString, udbto->eBuf->bufMaxVisString, 39 );

  if ( udbto->eBuf->bufVisInverted )
    udbto->visInverted = 0;
  else
    udbto->visInverted = 1;

  udbto->colorPvExpString.setRaw( udbto->eBuf->bufColorPvName );

  udbto->x = udbto->eBuf->bufX;
  udbto->sboxX = udbto->eBuf->bufX;

  udbto->y = udbto->eBuf->bufY;
  udbto->sboxY = udbto->eBuf->bufY;

  udbto->w = udbto->eBuf->bufW;
  udbto->sboxW = udbto->eBuf->bufW;

  udbto->h = udbto->eBuf->bufH;
  udbto->sboxH = udbto->eBuf->bufH;

  udbto->updateDimensions();

}

static void udbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;

  udbtc_edit_update ( w, client, call );
  udbto->refresh( udbto );

}

static void udbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;

  udbtc_edit_update ( w, client, call );
  udbto->ef.popdown();
  udbto->operationComplete();

}

static void udbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;

  udbto->ef.popdown();
  udbto->operationCancel();

}

static void udbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;

  udbto->ef.popdown();
  udbto->operationCancel();
  udbto->erase();
  udbto->deleteRequest = 1;
  udbto->drawAll();

}

static void udbtc_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) userarg;

  if ( pv->is_valid() ) {

    udbto->needConnectInit = 1;

  }
  else {

    udbto->connection.setPvDisconnected( (void *) udbto->destPvConnection );
    udbto->active = 0;
    udbto->bgColor.setDisconnected();
    udbto->needDraw = 1;

  }

  udbto->actWin->appCtx->proc->lock();
  udbto->actWin->addDefExeNode( udbto->aglPtr );
  udbto->actWin->appCtx->proc->unlock();

}

static void udbtc_monitor_save_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) userarg;

  if ( pv->is_valid() ) {

    udbto->needSaveConnectInit = 1;
    udbto->actWin->appCtx->proc->lock();
    udbto->actWin->addDefExeNode( udbto->aglPtr );
    udbto->actWin->appCtx->proc->unlock();

  }
  else {

    udbto->savePvConnected = 0;

  }

}

static void udbtc_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) userarg;

  udbto->actWin->appCtx->proc->lock();

  udbto->curControlV = pv->get_double();

  if ( udbto->savePvConnected ) {
    if ( !udbto->isSaved && ( udbto->curControlV == udbto->curSaveV ) ) {
      udbto->isSaved = 1;
      udbto->needRefresh = 1;
      udbto->actWin->addDefExeNode( udbto->aglPtr );
    }
    else if ( udbto->isSaved && ( udbto->curControlV != udbto->curSaveV ) ) {
      udbto->isSaved = 0;
      udbto->needRefresh = 1;
      udbto->actWin->addDefExeNode( udbto->aglPtr );
    }
  }

  udbto->actWin->appCtx->proc->unlock();

}

static void udbtc_saveUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) userarg;

  udbto->actWin->appCtx->proc->lock();

  udbto->curSaveV = pv->get_double();

  if ( !udbto->isSaved && ( udbto->curControlV == udbto->curSaveV ) ) {
    udbto->isSaved = 1;
    udbto->needRefresh = 1;
    udbto->actWin->addDefExeNode( udbto->aglPtr );
  }
  else if ( udbto->isSaved && ( udbto->curControlV != udbto->curSaveV ) ) {
    udbto->isSaved = 0;
    udbto->needRefresh = 1;
    udbto->actWin->addDefExeNode( udbto->aglPtr );
  }

  udbto->actWin->appCtx->proc->unlock();

  udbto->savePvConnected = 1;

}

static void udbtc_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) userarg;

  if ( pv->is_valid() ) {

    udbto->needVisConnectInit = 1;

  }
  else {

    udbto->connection.setPvDisconnected( (void *) udbto->visPvConnection );
    udbto->active = 0;
    udbto->bgColor.setDisconnected();
    udbto->needDraw = 1;

  }

  udbto->actWin->appCtx->proc->lock();
  udbto->actWin->addDefExeNode( udbto->aglPtr );
  udbto->actWin->appCtx->proc->unlock();

}

static void udbtc_visUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) userarg;

  udbto->curVisValue = pv->get_double();

  udbto->actWin->appCtx->proc->lock();
  udbto->needVisUpdate = 1;
  udbto->actWin->addDefExeNode( udbto->aglPtr );
  udbto->actWin->appCtx->proc->unlock();

}

static void udbtc_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) userarg;

  if ( pv->is_valid() ) {

    udbto->needColorConnectInit = 1;

  }
  else {

    udbto->connection.setPvDisconnected( (void *) udbto->colorPvConnection );
    udbto->active = 0;
    udbto->bgColor.setDisconnected();
    udbto->needDraw = 1;

  }

  udbto->actWin->appCtx->proc->lock();
  udbto->actWin->addDefExeNode( udbto->aglPtr );
  udbto->actWin->appCtx->proc->unlock();

}

static void udbtc_colorUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) userarg;

  udbto->curColorValue = pv->get_double();

  udbto->actWin->appCtx->proc->lock();
  udbto->needColorUpdate = 1;
  udbto->actWin->addDefExeNode( udbto->aglPtr );
  udbto->actWin->appCtx->proc->unlock();

}

static void udbtc_decrement (
  XtPointer client,
  XtIntervalId *id )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;
double dval;
Window root, child;
int rootX, rootY, winX, winY;
unsigned int mask;

  XQueryPointer( udbto->actWin->d, XtWindow(udbto->actWin->top), &root, &child,
   &rootX, &rootY, &winX, &winY, &mask );

  if ( !( mask & Button1Mask ) ) {
    udbto->incrementTimerActive = 0;
  }

  if ( !udbto->incrementTimerActive ) {
    udbto->incrementTimer = 0;
    return;
  }

  udbto->incrementTimer = appAddTimeOut(
   udbto->actWin->appCtx->appContext(),
   udbto->incrementTimerValue, udbtc_decrement, client );

  udbto->actWin->appCtx->proc->lock();
  dval = udbto->curControlV;
  udbto->actWin->appCtx->proc->unlock();

  dval -= udbto->coarse;

  if ( dval < udbto->minDv ) {
    dval = udbto->minDv;
  }
  else if ( dval > udbto->maxDv ) {
    dval = udbto->maxDv;
  }

  if ( udbto->destExists ) {
    udbto->destPvId->put(
     XDisplayName(udbto->actWin->appCtx->displayName), dval );
  }

}

static void udbtc_increment (
  XtPointer client,
  XtIntervalId *id )
{

activeUpdownButtonClass *udbto = (activeUpdownButtonClass *) client;
double dval;
Window root, child;
int rootX, rootY, winX, winY;
unsigned int mask;

  XQueryPointer( udbto->actWin->d, XtWindow(udbto->actWin->top), &root, &child,
   &rootX, &rootY, &winX, &winY, &mask );

  if ( !( mask & Button3Mask ) ) {
    udbto->incrementTimerActive = 0;
  }

  if ( !udbto->incrementTimerActive ) {
    udbto->incrementTimer = 0;
    return;
  }

  udbto->incrementTimer = appAddTimeOut(
   udbto->actWin->appCtx->appContext(),
   udbto->incrementTimerValue, udbtc_increment, client );

  udbto->actWin->appCtx->proc->lock();
  dval = udbto->curControlV;
  udbto->actWin->appCtx->proc->unlock();

  dval += udbto->coarse;

  if ( dval < udbto->minDv ) {
    dval = udbto->minDv;
  }
  else if ( dval > udbto->maxDv ) {
    dval = udbto->maxDv;
  }

  if ( udbto->destExists ) {
    udbto->destPvId->put(
     XDisplayName(udbto->actWin->appCtx->displayName), dval );
  }

}

activeUpdownButtonClass::activeUpdownButtonClass ( void ) {

  name = new char[strlen("activeUpdownButtonClass")+1];
  strcpy( name, "activeUpdownButtonClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  buttonPressed = 0;
  _3D = 1;
  invisible = 0;
  rate = 0.1;
  curSaveV = 0.0;
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
  connection.setMaxPvs( 4 );
  activeMode = 0;
  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

}

// copy constructor
activeUpdownButtonClass::activeUpdownButtonClass
 ( const activeUpdownButtonClass *source ) {

activeGraphicClass *udbto = (activeGraphicClass *) this;

  udbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeUpdownButtonClass")+1];
  strcpy( name, "activeUpdownButtonClass" );

  buttonPressed = 0;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  destPvExpString.copy( source->destPvExpString );
  savePvExpString.copy( source->savePvExpString );
  visPvExpString.copy( source->visPvExpString );
  colorPvExpString.copy( source->colorPvExpString );

  fineExpString.copy( source->fineExpString );
  coarseExpString.copy( source->coarseExpString );

  label.copy( source->label );

  _3D = source->_3D;
  invisible = source->invisible;
  rate = source->rate;
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

  connection.setMaxPvs( 4 );

  setBlinkFunction( (void *) doBlink );

  doAccSubs( destPvExpString );
  doAccSubs( savePvExpString );
  doAccSubs( colorPvExpString );
  doAccSubs( visPvExpString );
  doAccSubs( label );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );

  updateDimensions();

}

activeUpdownButtonClass::~activeUpdownButtonClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

int activeUpdownButtonClass::createInteractive (
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

int activeUpdownButtonClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

  major = UDBTC_MAJOR_VERSION;
  minor = UDBTC_MINOR_VERSION;
  release = UDBTC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadR( unknownTags );
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
  tag.loadW( "savedValuePv", &savePvExpString, emptyStr );
  tag.loadW( "coarseValue",  &coarseExpString, emptyStr );
  tag.loadW( "fineValue",  &fineExpString, emptyStr );
  tag.loadW( "label", &label, emptyStr );
  tag.loadBoolW( "3d", &_3D, &zero );
  tag.loadBoolW( "invisible", &invisible, &zero );
  tag.loadW( "rate", &rate, &dzero );
  tag.loadW( "font", fontTag );
  tag.loadBoolW( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadW( "scaleMin", &efScaleMin );
  tag.loadW( "scaleMax", &efScaleMax );
  tag.loadW( "visPv", &visPvExpString, emptyStr );
  tag.loadBoolW( "visInvert", &visInverted, &zero );
  tag.loadW( "visMin", minVisString, emptyStr );
  tag.loadW( "visMax", maxVisString, emptyStr );
  tag.loadW( "colorPv", &colorPvExpString, emptyStr  );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeUpdownButtonClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", UDBTC_MAJOR_VERSION, UDBTC_MINOR_VERSION,
   UDBTC_RELEASE );

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

  index = topShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = botShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  if ( destPvExpString.getRaw() )
    writeStringToFile( f, destPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( fineExpString.getRaw() )
    writeStringToFile( f, fineExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( coarseExpString.getRaw() )
    writeStringToFile( f, coarseExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( label.getRaw() )
    writeStringToFile( f, label.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", _3D );

  fprintf( f, "%-d\n", invisible );

  fprintf( f, "%-g\n", rate );

  writeStringToFile( f, fontTag );

  // ver 1.1.0
  if ( savePvExpString.getRaw() )
    writeStringToFile( f, savePvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  // ver 1.2.0
  fprintf( f, "%-d\n", limitsFromDb );
  efScaleMin.write( f );
  efScaleMax.write( f );

  // ver 1.4.0
  if ( visPvExpString.getRaw() )
    writeStringToFile( f, visPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );
  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  //ver 1.5.0
  if ( colorPvExpString.getRaw() )
    writeStringToFile( f, colorPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  return 1;

}

int activeUpdownButtonClass::createFromFile (
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
  tag.loadR( "savedValuePv", &savePvExpString, emptyStr );
  tag.loadR( "coarseValue",  &coarseExpString, emptyStr );
  tag.loadR( "fineValue",  &fineExpString, emptyStr );
  tag.loadR( "label", &label, emptyStr );
  tag.loadR( "3d", &_3D, &zero );
  tag.loadR( "invisible", &invisible, &zero );
  tag.loadR( "rate", &rate, &dzero );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadR( "scaleMin", &efScaleMin );
  tag.loadR( "scaleMax", &efScaleMax );
  tag.loadR( "visPv", &visPvExpString, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "colorPv", &colorPvExpString, emptyStr );
  tag.loadW( unknownTags );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > UDBTC_MAJOR_VERSION ) {
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

int activeUpdownButtonClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int index;
int major, minor, release;
char oneName[PV_Factory::MAX_PV_NAME+1];
float fval;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > UDBTC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 2 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    topShadowColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    botShadowColor = index;

  }
  else {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    topShadowColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    botShadowColor = index;

  }

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  destPvExpString.setRaw( oneName );

  readStringFromFile( oneName, 39+1, f ); actWin->incLine();
  fineExpString.setRaw( oneName );

  readStringFromFile( oneName, 39+1, f ); actWin->incLine();
  coarseExpString.setRaw( oneName );

  readStringFromFile( oneName, 39+1, f ); actWin->incLine();
  label.setRaw( oneName );

  fscanf( f, "%d\n", &_3D ); actWin->incLine();

  fscanf( f, "%d\n", &invisible ); actWin->incLine();

  fscanf( f, "%g\n", &fval ); actWin->incLine();
  rate = (double) fval;

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 0 ) ) ) {
    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    savePvExpString.setRaw( oneName );
  }

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 1 ) ) ) {
    fscanf( f, "%d\n", &limitsFromDb ); actWin->incLine();
    efScaleMin.read( f ); actWin->incLine();
    efScaleMax.read( f ); actWin->incLine();
    if ( ( limitsFromDb || efScaleMin.isNull() ) &&
         ( limitsFromDb || efScaleMax.isNull() ) ) {
      minDv = scaleMin = 0;
      maxDv = scaleMax = 10;
    }
    else{
      minDv = scaleMin = efScaleMin.value();
      maxDv = scaleMax = efScaleMax.value();
    }
  }
  else {
    scaleMin = 0;
    scaleMax = 10;
  }

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 3 ) ) ) {

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    visPvExpString.setRaw( oneName );

    fscanf( f, "%d\n", &visInverted ); actWin->incLine();

    readStringFromFile( minVisString, 39+1, f ); actWin->incLine();

    readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();

  }

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 4 ) ) ) {

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    colorPvExpString.setRaw( oneName );

  }

  updateDimensions();

  return 1;

}

int activeUpdownButtonClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin ){

int fgR, fgG, fgB, bgR, bgG, bgB, more, index;
unsigned int pixel;
char *tk, *gotData, *context, buf[255+1];
char tmpDestPvName[PV_Factory::MAX_PV_NAME+1];
char tmpFine[39+1];
char tmpCoarse[39+1];

  fgR = 0xffff;
  fgG = 0xffff;
  fgB = 0xffff;

  bgR = 0xffff;
  bgG = 0xffff;
  bgB = 0xffff;

  this->actWin = _actWin;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;
  strcpy( fontTag, actWin->defaultBtnFontTag );

  label.setRaw( "" );

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
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
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "fgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        fgR = atol( tk );

      }
            
      else if ( strcmp( tk, "fggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        fgG = atol( tk );

      }
            
      else if ( strcmp( tk, "fgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        fgB = atol( tk );

      }
            
      else if ( strcmp( tk, "bgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        bgR = atol( tk );

      }
            
      else if ( strcmp( tk, "bggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        bgG = atol( tk );

      }
            
      else if ( strcmp( tk, "bgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        bgB = atol( tk );

      }
            
      else if ( strcmp( tk, "invisible" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        invisible = atol( tk );

      }
            
      else if ( strcmp( tk, "rate" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        rate = atof( tk );

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeUpdownButtonClass_str1 );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }

      else if ( strcmp( tk, "controlpv" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( tmpDestPvName, tk, 28 );
          tmpDestPvName[28] = 0;
          destPvExpString.setRaw( tmpDestPvName );
	}

      }

      else if ( strcmp( tk, "fine" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( tmpFine, tk, 28 );
          tmpFine[28] = 0;
          fineExpString.setRaw( tmpFine );
	}

      }

      else if ( strcmp( tk, "coarse" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( tmpCoarse, tk, 28 );
          tmpCoarse[28] = 0;
          coarseExpString.setRaw( tmpCoarse );
	}

      }

      else if ( strcmp( tk, "label" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          label.setRaw( tk );
	}

      }

    }

  } while ( more );

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->ci->setRGB( fgR, fgG, fgB, &pixel );
  index = actWin->ci->pixIndex( pixel );
  fgColor.setColorIndex( index, actWin->ci );

  actWin->ci->setRGB( bgR, bgG, bgB, &pixel );
  index = actWin->ci->pixIndex( pixel );
  bgColor.setColorIndex( index, actWin->ci );

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeUpdownButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeUpdownButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeUpdownButtonClass_str2, 31 );

  Strncat( title, activeUpdownButtonClass_str3, 31 );

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

  if ( savePvExpString.getRaw() )
    strncpy( eBuf->bufSavePvName, savePvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufSavePvName, "" );

  if ( fineExpString.getRaw() )
    strncpy( eBuf->bufFine, fineExpString.getRaw(), 39 );
  else
    strncpy( eBuf->bufFine, "", 39 );

  if ( coarseExpString.getRaw() )
    strncpy( eBuf->bufCoarse, coarseExpString.getRaw(), 39 );
  else
    strncpy( eBuf->bufCoarse, "", 39 );

  if ( label.getRaw() )
    strncpy( eBuf->bufLabel, label.getRaw(), 39 );
  else
    strncpy( eBuf->bufLabel, "", 39 );

  eBuf->buf3D = _3D;
  eBuf->bufInvisible = invisible;
  eBuf->bufRate = rate;

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

  ef.addTextField( activeUpdownButtonClass_str4, 35, &eBuf->bufX );
  ef.addTextField( activeUpdownButtonClass_str5, 35, &eBuf->bufY );
  ef.addTextField( activeUpdownButtonClass_str6, 35, &eBuf->bufW );
  ef.addTextField( activeUpdownButtonClass_str7, 35, &eBuf->bufH );
  ef.addTextField( activeUpdownButtonClass_str8, 35, eBuf->bufDestPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeUpdownButtonClass_str25, 35, eBuf->bufSavePvName,
   PV_Factory::MAX_PV_NAME );

  ef.addToggle( activeUpdownButtonClass_str26, &eBuf->bufLimitsFromDb );
  limitsFromDbEntry = ef.getCurItem();
  ef.addTextField( activeUpdownButtonClass_str27, 35, &eBuf->bufEfScaleMin );
  minEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( minEntry );
  ef.addTextField( activeUpdownButtonClass_str28, 35, &eBuf->bufEfScaleMax );
  maxEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( maxEntry );
  limitsFromDbEntry->addDependencyCallbacks();

  ef.addTextField( activeUpdownButtonClass_str9, 35, eBuf->bufCoarse, 39 );
  ef.addTextField( activeUpdownButtonClass_str10, 35, eBuf->bufFine, 39 );
  ef.addTextField( activeUpdownButtonClass_str11, 35, &eBuf->bufRate );
  ef.addToggle( activeUpdownButtonClass_str12, &eBuf->buf3D );
  ef.addToggle( activeUpdownButtonClass_str13, &eBuf->bufInvisible );
  ef.addTextField( activeUpdownButtonClass_str14, 35, eBuf->bufLabel, 39 );
  ef.addColorButton( activeUpdownButtonClass_str16, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addColorButton( activeUpdownButtonClass_str17, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  ef.addColorButton( activeUpdownButtonClass_str18, actWin->ci, &eBuf->topShadowCb, &eBuf->bufTopShadowColor );
  ef.addColorButton( activeUpdownButtonClass_str19, actWin->ci, &eBuf->botShadowCb, &eBuf->bufBotShadowColor );

  ef.addFontMenu( activeUpdownButtonClass_str15, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeUpdownButtonClass_str33, 30, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeUpdownButtonClass_str29, 30, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeUpdownButtonClass_str30, &eBuf->bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeUpdownButtonClass_str31, 30, eBuf->bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeUpdownButtonClass_str32, 30, eBuf->bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  return 1;

}

int activeUpdownButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( udbtc_edit_ok, udbtc_edit_apply, udbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeUpdownButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( udbtc_edit_ok, udbtc_edit_apply, udbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeUpdownButtonClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeUpdownButtonClass::eraseActive ( void ) {

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

int activeUpdownButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  //actWin->drawGc.setFG( bgColor.pixelColor() );
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

  //actWin->drawGc.setFG( fgColor.pixelColor() );
  actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+5, y+9, x+w-5, y+9 );

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

int activeUpdownButtonClass::drawActive ( void ) {

int tX, tY;
char string[63+1];
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      //actWin->executeGc.setFG( bgColor.getDisconnected() );
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

  //actWin->executeGc.setFG( bgColor.getColor() );
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

  //actWin->executeGc.setFG( fgColor.getColor() );
  actWin->executeGc.setFG( fgColor.getIndex(), &blink );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+5, y+9, x+w-5, y+9 );

  if ( fs ) {

    if ( label.getExpanded() )
      strncpy( string, label.getExpanded(), 39 );
    else
      strncpy( string, "", 39 );

    if ( isSaved ) {
      Strncat( string, " *", 63 );
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

int activeUpdownButtonClass::activate (
  int pass,
  void *ptr )
{

int opStat, n;
Arg args[5];
XmString str;

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      connection.init();

      initEnable();

      needConnectInit = needSaveConnectInit = needCtlInfoInit = 
       needRefresh = needErase = needDraw = needVisConnectInit =
       needVisInit = needVisUpdate = needColorConnectInit =
       needColorInit = needColorUpdate = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      init = 0;
      aglPtr = ptr;
      widgetsCreated = 0;
      keyPadOpen = 0;
      isSaved = 0;
      incrementTimer = 0;
      incrementTimerActive = 0;
      destPvId = visPvId = colorPvId = savePvId = NULL;
      initialConnection = initialSavedValueConnection = initialVisConnection =
       initialColorConnection = -1;

      destPvConnected = savePvConnected = active = buttonPressed = 0;
      activeMode = 1;

      incrementTimerValue = (int) ( 1000.0 * rate );
      if ( incrementTimerValue < 50 ) incrementTimerValue = 50;

      if ( !fineExpString.getExpanded() ||
         ( strcmp( fineExpString.getExpanded(), "" ) == 0 ) ) {
        fine = 0;
      }
      else {
        fine = atof( fineExpString.getExpanded() );
      }

      if ( !coarseExpString.getExpanded() ||
         ( strcmp( coarseExpString.getExpanded(), "" ) == 0 ) ) {
        coarse = 0;
      }
      else {
        coarse = atof( coarseExpString.getExpanded() );
      }

      if ( !destPvExpString.getExpanded() ||
         // ( strcmp( destPvExpString.getExpanded(), "" ) == 0 ) ) {
         blankOrComment( destPvExpString.getExpanded() ) ) {
        destExists = 0;
      }
      else {
        destExists = 1;
        connection.addPv();
      }

      if ( !visPvExpString.getExpanded() ||
         // ( strcmp( visPvExpString.getExpanded(), "" ) == 0 ) ) {
         blankOrComment( visPvExpString.getExpanded() ) ) {
        visExists = 0;
        visibility = 1;
      }
      else {
        visExists = 1;
        connection.addPv();
      }

      if ( !colorPvExpString.getExpanded() ||
         // ( strcmp( colorPvExpString.getExpanded(), "" ) == 0 ) ) {
         blankOrComment( colorPvExpString.getExpanded() ) ) {
        colorExists = 0;
      }
      else {
        colorExists = 1;
        connection.addPv();
      }

      if ( !savePvExpString.getExpanded() ||
         // ( strcmp( savePvExpString.getExpanded(), "" ) == 0 ) ) {
         blankOrComment( savePvExpString.getExpanded() ) ) {
        saveExists = 0;
      }
      else {
        saveExists = 1;
      }

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      if ( !widgetsCreated ) {

        n = 0;
        XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
        popUpMenu = XmCreatePopupMenu( actWin->topWidgetId(), "", args, n );

        pullDownMenu = XmCreatePulldownMenu( popUpMenu, "", NULL, 0 );

        str = XmStringCreateLocalized( "Save" );
        pbSave = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbSave, XmNactivateCallback, menu_cb,
         (XtPointer) this );

        str = XmStringCreateLocalized( "Restore" );
        pbRestore = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbRestore, XmNactivateCallback, menu_cb,
         (XtPointer) this );

        str = XmStringCreateLocalized( "Set Coarse" );
        pbCoarse = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbCoarse, XmNactivateCallback, menu_cb,
         (XtPointer) this );

        str = XmStringCreateLocalized( "Set Fine" );
        pbFine = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbFine, XmNactivateCallback, menu_cb,
         (XtPointer) this );

        str = XmStringCreateLocalized( "Set Rate (sec)" );
        pbRate = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbRate, XmNactivateCallback, menu_cb,
         (XtPointer) this );

        str = XmStringCreateLocalized( "Set Value" );
        pbValue = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pbValue, XmNactivateCallback, menu_cb,
         (XtPointer) this );

	widgetsCreated = 1;

      }

      opStat = 1;

      if ( destExists ) {

	destPvId = the_PV_Factory->create( destPvExpString.getExpanded() );
	if ( destPvId ) {
	  destPvId->add_conn_state_callback( udbtc_monitor_dest_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeUpdownButtonClass_str20 );
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
	  visPvId->add_conn_state_callback( udbtc_monitor_vis_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeUpdownButtonClass_str20 );
          opStat = 0;
        }

      }

      if ( colorExists ) {

	colorPvId = the_PV_Factory->create( colorPvExpString.getExpanded() );
	if ( colorPvId ) {
	  colorPvId->add_conn_state_callback(
           udbtc_monitor_color_connect_state, this );
	}
	else {
          fprintf( stderr, activeUpdownButtonClass_str20 );
          opStat = 0;
        }

      }

      if ( saveExists ) {

	savePvId = the_PV_Factory->create( savePvExpString.getExpanded() );
	if ( savePvId ) {
	  savePvId->add_conn_state_callback( udbtc_monitor_save_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeUpdownButtonClass_str20 );
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

int activeUpdownButtonClass::deactivate (
  int pass
) {

  if ( pass == 1 ) {

  active = 0;
  activeMode = 0;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  //updateBlink( 0 );

  if ( incrementTimerActive ) {
    if ( incrementTimer ) {
      XtRemoveTimeOut( incrementTimer );
      incrementTimer = 0;
    }
    incrementTimerActive = 0;
  }

  if ( 	widgetsCreated ) {
    XtDestroyWidget( popUpMenu );
    widgetsCreated = 0;
  }

  if ( kp.isPoppedUp() ) {
    kp.popdown();
  }

  if ( destExists ) {
    if ( destPvId ) {
      destPvId->remove_conn_state_callback( udbtc_monitor_dest_connect_state,
       this );
      destPvId->remove_value_callback( udbtc_controlUpdate, this );
      destPvId->release();
      destPvId = NULL;
    }
  }

  if ( visExists ) {
    if ( visPvId ) {
      visPvId->remove_conn_state_callback( udbtc_monitor_vis_connect_state,
       this );
      visPvId->remove_value_callback( udbtc_visUpdate, this );
      visPvId->release();
      visPvId = NULL;
    }
  }

  if ( colorExists ) {
    if ( colorPvId ) {
      colorPvId->remove_conn_state_callback( udbtc_monitor_color_connect_state,
       this );
      colorPvId->remove_value_callback( udbtc_colorUpdate, this );
      colorPvId->release();
      colorPvId = NULL;
    }
  }

  if ( saveExists ) {
    if ( savePvId ) {
      savePvId->remove_conn_state_callback( udbtc_monitor_save_connect_state,
       this );
      savePvId->remove_value_callback( udbtc_saveUpdate, this );
      savePvId->release();
      savePvId = NULL;
    }
  }

  }

  return 1;

}

void activeUpdownButtonClass::updateDimensions ( void )
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

void activeUpdownButtonClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  rootX = be->x_root;
  rootY = be->y_root;

  if ( incrementTimerActive ) {
    if ( incrementTimer ) {
      XtRemoveTimeOut( incrementTimer );
      incrementTimer = 0;
    }
    incrementTimerActive = 0;
  }

  if ( !enabled || !init || !visibility ) return;

  if ( !destPvId->have_write_access() ) return;

  if ( ( be->y - y ) < 10 ) {
    XmMenuPosition( popUpMenu, be );
    XtManageChild( popUpMenu );
    return;
  }

  if ( !buttonPressed ) return;

  if ( keyPadOpen ) return;

  buttonPressed = 0;

//    fprintf( stderr, "btn up\n" );

  actWin->appCtx->proc->lock();
  needRefresh = 1;
  actWin->addDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

}

void activeUpdownButtonClass::btnDown (
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

  if ( !destPvId->have_write_access() ) return;

  if ( keyPadOpen ) return;

  if ( ( be->y - y ) < 10 ) return;

  buttonPressed = 1;

  //fprintf( stderr, "btn down, x=%-d, y=%-d, bn=%-d\n", _x-x, _y-y , buttonNumber );

  actWin->appCtx->proc->lock();
  dval = curControlV;
  needRefresh = 1;
  actWin->addDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( ( buttonNumber == 3 ) || ( buttonNumber == 4 ) ) {
    dval += fine;
  }
  else if ( ( buttonNumber ) == 1 || ( buttonNumber == 5 ) ) {
    dval -= fine;
  }

  if ( dval < minDv ) {
    dval = minDv;
  }
  else if ( dval > maxDv ) {
    dval = maxDv;
  }

  destPvId->put(
   XDisplayName(actWin->appCtx->displayName), dval );

  if ( buttonNumber == 3 ) {
    incrementTimer = appAddTimeOut( actWin->appCtx->appContext(),
     500, udbtc_increment, this );
    incrementTimerActive = 1;
  }
  else if ( buttonNumber == 1 ) {
    incrementTimer = appAddTimeOut( actWin->appCtx->appContext(),
     500, udbtc_decrement, this );
    incrementTimerActive = 1;
  }
  else {
    incrementTimerActive = 0;
  }

}

void activeUpdownButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled || !init || !visibility ) return;

  if ( !destPvId->have_write_access() ) {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );
  }
  else {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_UPDOWN );
  }

  activeGraphicClass::pointerIn( _x, _y, buttonState );

}

int activeUpdownButtonClass::getButtonActionRequest (
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

int activeUpdownButtonClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( destPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  destPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( savePvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  savePvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( fineExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  fineExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( coarseExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  coarseExpString.setRaw( tmpStr.getExpanded() );

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

int activeUpdownButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = destPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = savePvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = fineExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = coarseExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = label.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeUpdownButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = destPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = savePvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = fineExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = coarseExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = label.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return stat;

}

int activeUpdownButtonClass::containsMacros ( void ) {

  if ( destPvExpString.containsPrimaryMacros() ) return 1;

  if ( savePvExpString.containsPrimaryMacros() ) return 1;

  if ( fineExpString.containsPrimaryMacros() ) return 1;

  if ( coarseExpString.containsPrimaryMacros() ) return 1;

  if ( label.containsPrimaryMacros() ) return 1;

  if ( visPvExpString.containsPrimaryMacros() ) return 1;

  if ( colorPvExpString.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeUpdownButtonClass::executeDeferred ( void ) {

int nc, nsc, nci, nd, ne, nr, nvc, nvi, nvu, ncolc, ncoli, ncolu;
int stat, index, invisColor;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nsc = needSaveConnectInit; needSaveConnectInit = 0;
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

      destPvId->add_value_callback( udbtc_controlUpdate, this );

    }

    if ( connection.pvsConnected() ) {
      bgColor.setConnected();
      init = 1;
      smartDrawAllActive();
    }

  }

  if ( nsc ) {

    savePvConnected = 1;
    saveType = (int) savePvId->get_type().type;

    if ( initialSavedValueConnection ) {

      initialSavedValueConnection = 0;

      savePvId->add_value_callback( udbtc_saveUpdate, this );

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

      visPvId->add_value_callback( udbtc_visUpdate, this );

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

      colorPvId->add_value_callback( udbtc_colorUpdate, this );

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

char *activeUpdownButtonClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeUpdownButtonClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeUpdownButtonClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( i == 0 ) {
      return destPvExpString.getExpanded();
    }
    else if ( i == 1 ) {
      return savePvExpString.getExpanded();
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
      return destPvExpString.getRaw();
    }
    else if ( i == 1 ) {
      return savePvExpString.getRaw();
    }
    else if ( i == 2 ) {
      return colorPvExpString.getRaw();
    }
    else {
      return visPvExpString.getRaw();
    }

  }

}

void activeUpdownButtonClass::changeDisplayParams (
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

void activeUpdownButtonClass::changePvNames (
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

void activeUpdownButtonClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 2 ) {
    *n = 0;
    return;
  }

  *n = 2;
  pvs[0] = destPvId;
  pvs[1] = savePvId;

}

char *activeUpdownButtonClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return destPvExpString.getRaw();
  }
  else if ( i == 1 ) {
    return savePvExpString.getRaw();
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

  return NULL;

}

void activeUpdownButtonClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    destPvExpString.setRaw( string );
  }
  else if ( i == 1 ) {
    savePvExpString.setRaw( string );
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

}

// crawler functions may return blank pv names
char *activeUpdownButtonClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return destPvExpString.getExpanded();

}

char *activeUpdownButtonClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >=4 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return savePvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 2 ) {
    return visPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 3 ) {
    return colorPvExpString.getExpanded();
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeUpdownButtonClassPtr ( void ) {

activeUpdownButtonClass *ptr;

  ptr = new activeUpdownButtonClass;
  return (void *) ptr;

}

void *clone_activeUpdownButtonClassPtr (
  void *_srcPtr )
{

activeUpdownButtonClass *ptr, *srcPtr;

  srcPtr = (activeUpdownButtonClass *) _srcPtr;

  ptr = new activeUpdownButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
