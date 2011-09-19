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

#define __message_button_cc 1

#include "message_button.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) {

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) ptr;

  if ( !msgbto->activeMode ) {
    if ( msgbto->isSelected() ) msgbto->drawSelectBoxCorners(); //erase via xor
    msgbto->smartDrawAll();
    if ( msgbto->isSelected() ) msgbto->drawSelectBoxCorners();
  }
  else {
    msgbto->bufInvalidate();
    msgbto->needDraw = 1;
    msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  }

}

static void pw_ok (
  Widget w,
  XtPointer client,
  XtPointer call ) {

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->ef.popdown();

  if ( strcmp( msgbto->bufPw1, msgbto->pw ) == 0 ) {
    msgbto->needPerformDownAction = 1;
    msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  }
  else {
    msgbto->needWarning = 1;
    msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  }

}

static void pw_apply (
  Widget w,
  XtPointer client,
  XtPointer call ) {

}

static void pw_cancel (
  Widget w,
  XtPointer client,
  XtPointer call ) {

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->ef.popdown();

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  if ( !msgbto->init ) {
    msgbto->needToDrawUnconnected = 1;
    msgbto->needDraw = 1;
    msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  }

  msgbto->unconnectedTimer = 0;

}

static void msgbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->actWin->setChanged();

  msgbto->eraseSelectBoxCorners();
  msgbto->erase();

  msgbto->fgColor.setColorIndex( msgbto->eBuf->bufFgColor, msgbto->actWin->ci );

  msgbto->onColor.setColorIndex( msgbto->eBuf->bufOnColor, msgbto->actWin->ci );

  msgbto->offColor.setColorIndex( msgbto->eBuf->bufOffColor, msgbto->actWin->ci );

  msgbto->topShadowColor = msgbto->eBuf->bufTopShadowColor;
  msgbto->botShadowColor = msgbto->eBuf->bufBotShadowColor;

  msgbto->destPvExpString.setRaw( msgbto->eBuf->bufDestPvName );
  msgbto->sourcePressPvExpString.setRaw( msgbto->eBuf->bufSourcePressPvName );
  msgbto->sourceReleasePvExpString.setRaw( msgbto->eBuf->bufSourceReleasePvName );

  msgbto->onLabel.setRaw( msgbto->eBuf->bufOnLabel );
  msgbto->offLabel.setRaw( msgbto->eBuf->bufOffLabel );

  strncpy( msgbto->fontTag, msgbto->fm.currentFontTag(), 63 );
  msgbto->actWin->fi->loadFontTag( msgbto->fontTag );
  msgbto->fs = msgbto->actWin->fi->getXFontStruct( msgbto->fontTag );

  msgbto->toggle = msgbto->eBuf->bufToggle;

  msgbto->pressAction = msgbto->eBuf->bufPressAction;
  msgbto->releaseAction = msgbto->eBuf->bufReleaseAction;

  msgbto->_3D = msgbto->eBuf->buf3D;

  msgbto->invisible = msgbto->eBuf->bufInvisible;

  msgbto->x = msgbto->eBuf->bufX;
  msgbto->sboxX = msgbto->eBuf->bufX;

  msgbto->y = msgbto->eBuf->bufY;
  msgbto->sboxY = msgbto->eBuf->bufY;

  msgbto->w = msgbto->eBuf->bufW;
  msgbto->sboxW = msgbto->eBuf->bufW;

  msgbto->h = msgbto->eBuf->bufH;
  msgbto->sboxH = msgbto->eBuf->bufH;

  if ( blank(msgbto->bufPw1) || blank(msgbto->bufPw2) ) {
    if ( blank(msgbto->pw) ) {
      msgbto->usePassword = 0;
    }
    else {
      msgbto->usePassword = 1;
    }
  }
  else if ( strcmp( msgbto->bufPw1, msgbto->bufPw2 ) != 0 ) {
    msgbto->actWin->appCtx->postMessage( activeMessageButtonClass_str33 );
    if ( blank(msgbto->pw) ) {
      msgbto->usePassword = 0;
    }
    else if ( strcmp( msgbto->pw, "*" ) == 0 ) {
      strcpy( msgbto->pw, "" );
      msgbto->usePassword = 0;
    }
    else {
      msgbto->usePassword = 1;
    }
  }
  else {
    strcpy( msgbto->pw, msgbto->bufPw2 );
    if ( strcmp( msgbto->pw, "*" ) == 0 ) {
      strcpy( msgbto->pw, "" );
      msgbto->usePassword = 0;
    }
    else {
      msgbto->usePassword = 1;
    }
  }

  msgbto->lock = msgbto->eBuf->bufLock;

  msgbto->visPvExpString.setRaw( msgbto->eBuf->bufVisPvName );
  strncpy( msgbto->minVisString, msgbto->eBuf->bufMinVisString, 39 );
  strncpy( msgbto->maxVisString, msgbto->eBuf->bufMaxVisString, 39 );

  if ( msgbto->eBuf->bufVisInverted )
    msgbto->visInverted = 0;
  else
    msgbto->visInverted = 1;

  msgbto->useEnumNumeric = msgbto->eBuf->bufUseEnumNumeric;

  msgbto->colorPvExpString.setRaw( msgbto->eBuf->bufColorPvName );

  msgbto->updateDimensions();

}

static void msgbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbtc_edit_update ( w, client, call );
  msgbto->refresh( msgbto );

}

static void msgbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbtc_edit_update ( w, client, call );
  msgbto->ef.popdown();
  msgbto->operationComplete();

}

static void msgbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->ef.popdown();
  msgbto->operationCancel();

}

static void msgbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) client;

  msgbto->ef.popdown();
  msgbto->operationCancel();
  msgbto->erase();
  msgbto->deleteRequest = 1;
  msgbto->drawAll();

}

static void msgbt_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) userarg;

  if ( pv->is_valid() ) {

    msgbto->needConnectInit = 1;

  }
  else {

    msgbto->connection.setPvDisconnected( (void *) msgbto->destPvConnection );
    msgbto->active = 0;
    msgbto->onColor.setDisconnected();
    msgbto->offColor.setDisconnected();
    msgbto->needDraw = 1;

  }

  msgbto->actWin->appCtx->proc->lock();
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

static void msgbt_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) userarg;

  if ( pv->is_valid() ) {

    msgbto->needVisConnectInit = 1;

  }
  else {

    msgbto->connection.setPvDisconnected( (void *) msgbto->visPvConnection );
    msgbto->active = 0;
    msgbto->onColor.setDisconnected();
    msgbto->offColor.setDisconnected();
    msgbto->needDraw = 1;

  }

  msgbto->actWin->appCtx->proc->lock();
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

static void msgbt_visUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) userarg;

  msgbto->curVisValue = pv->get_double();

  msgbto->actWin->appCtx->proc->lock();
  msgbto->needVisUpdate = 1;
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

static void msgbt_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMessageButtonClass *msgbto =
  (activeMessageButtonClass *) userarg;

  if ( pv->is_valid() ) {

    msgbto->needColorConnectInit = 1;

  }
  else {

    msgbto->connection.setPvDisconnected( (void *) msgbto->colorPvConnection );
    msgbto->active = 0;
    msgbto->onColor.setDisconnected();
    msgbto->offColor.setDisconnected();
    msgbto->needDraw = 1;

  }

  msgbto->actWin->appCtx->proc->lock();
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

static void msgbt_colorUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMessageButtonClass *msgbto = (activeMessageButtonClass *) userarg;

  msgbto->curColorValue = pv->get_double();

  msgbto->actWin->appCtx->proc->lock();
  msgbto->needColorUpdate = 1;
  msgbto->actWin->addDefExeNode( msgbto->aglPtr );
  msgbto->actWin->appCtx->proc->unlock();

}

activeMessageButtonClass::activeMessageButtonClass ( void ) {

  name = new char[strlen("activeMessageButtonClass")+1];
  strcpy( name, "activeMessageButtonClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  buttonPressed = 0;

  toggle = 0;
  pressAction = 0;
  releaseAction = 0;
  _3D = 1;
  invisible = 0;
  unconnectedTimer = 0;
  strcpy( pw, "" );
  usePassword = 0;
  lock = 0;
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  connection.setMaxPvs( 3 );
  useEnumNumeric = 0;
  activeMode = 0;
  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

}

// copy constructor
activeMessageButtonClass::activeMessageButtonClass
 ( const activeMessageButtonClass *source ) {

activeGraphicClass *msgbto = (activeGraphicClass *) this;

  msgbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMessageButtonClass")+1];
  strcpy( name, "activeMessageButtonClass" );

  buttonPressed = 0;

  fgCb = source->fgCb;
  onCb = source->onCb;
  offCb = source->offCb;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  offColor.copy( source->offColor );
  onColor.copy( source->onColor );

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  destPvExpString.copy( source->destPvExpString );
  sourcePressPvExpString.copy( source->sourcePressPvExpString );
  sourceReleasePvExpString.copy( source->sourceReleasePvExpString );
  visPvExpString.copy( source->visPvExpString );
  colorPvExpString.copy( source->colorPvExpString );

  onLabel.copy( source->onLabel );
  offLabel.copy( source->offLabel );

  toggle = source->toggle;
  pressAction = source->pressAction;
  releaseAction = source->releaseAction;
  _3D = source->_3D;
  invisible = source->invisible;
  unconnectedTimer = 0;

  strcpy( pw, source->pw );
  usePassword = source->usePassword;
  lock = source->lock;

  visibility = 0;
  prevVisibility = -1;
  visInverted = source->visInverted;
  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );
  useEnumNumeric = source->useEnumNumeric;

  activeMode = 0;

  eBuf = NULL;

  connection.setMaxPvs( 3 );

  setBlinkFunction( (void *) doBlink );

  doAccSubs( destPvExpString );
  doAccSubs( visPvExpString );
  doAccSubs( colorPvExpString );
  doAccSubs( onLabel );
  doAccSubs( offLabel );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );

  updateDimensions();

}

activeMessageButtonClass::~activeMessageButtonClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

int activeMessageButtonClass::createInteractive (
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
  onColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  offColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  strcpy( fontTag, actWin->defaultBtnFontTag );

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  // strcpy( onLabel, "" );
  // strcpy( offLabel, "" );

  toggle = 0;
  pressAction = 0;
  releaseAction = 0;
  _3D = 1;
  invisible = 0;

  this->draw();

  this->editCreate();

  return 1;

}

int activeMessageButtonClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
char *emptyStr = "";

  major = MSGBTC_MAJOR_VERSION;
  minor = MSGBTC_MINOR_VERSION;
  release = MSGBTC_RELEASE;

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
  tag.loadW( "onColor", actWin->ci, &onColor );
  tag.loadW( "offColor", actWin->ci, &offColor );
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "controlPv", &destPvExpString, emptyStr );
  tag.loadW( "pressValue",  &sourcePressPvExpString, emptyStr );
  tag.loadW( "releaseValue",  &sourceReleasePvExpString, emptyStr );
  tag.loadW( "onLabel", &onLabel, emptyStr );
  tag.loadW( "offLabel", &offLabel, emptyStr );
  tag.loadBoolW( "toggle", &toggle, &zero );
  tag.loadBoolW( "closeOnPress", &pressAction, &zero );
  tag.loadBoolW( "closeOnRelease", &releaseAction, &zero );
  tag.loadBoolW( "3d", &_3D, &zero );
  tag.loadBoolW( "invisible", &invisible, &zero );
  tag.loadBoolW( "useEnumNumeric", &useEnumNumeric, &zero );
  tag.loadW( "password", pw, emptyStr );
  tag.loadBoolW( "lock", &lock, &zero );
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

int activeMessageButtonClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", MSGBTC_MAJOR_VERSION, MSGBTC_MINOR_VERSION,
   MSGBTC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = onColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = offColor.pixelIndex();
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

  if ( sourcePressPvExpString.getRaw() )
    writeStringToFile( f, sourcePressPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( sourceReleasePvExpString.getRaw() )
    writeStringToFile( f, sourceReleasePvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( onLabel.getRaw() )
    writeStringToFile( f, onLabel.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( offLabel.getRaw() )
    writeStringToFile( f, offLabel.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", toggle );

  fprintf( f, "%-d\n", pressAction );

  fprintf( f, "%-d\n", releaseAction );

  fprintf( f, "%-d\n", _3D );

  fprintf( f, "%-d\n", invisible );

  writeStringToFile( f, fontTag );

  // ver 2.1.0
  writeStringToFile( f, pw );
  fprintf( f, "%-d\n", lock );

  //ver 2.3.0
  if ( visPvExpString.getRaw() )
    writeStringToFile( f, visPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );
  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  //ver 2.4.0
  if ( colorPvExpString.getRaw() )
    writeStringToFile( f, colorPvExpString.getRaw() );
  else
    writeStringToFile( f, "" );

  //ver 2.5.0
  fprintf( f, "%-d\n", useEnumNumeric );

  return 1;

}

int activeMessageButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
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
  tag.loadR( "onColor", actWin->ci, &onColor );
  tag.loadR( "offColor", actWin->ci, &offColor );
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "controlPv", &destPvExpString, emptyStr );
  tag.loadR( "pressValue",  &sourcePressPvExpString, emptyStr );
  tag.loadR( "releaseValue",  &sourceReleasePvExpString, emptyStr );
  tag.loadR( "onLabel", &onLabel, emptyStr );
  tag.loadR( "offLabel", &offLabel, emptyStr );
  tag.loadR( "toggle", &toggle, &zero );
  tag.loadR( "closeOnPress", &pressAction, &zero );
  tag.loadR( "closeOnRelease", &releaseAction, &zero );
  tag.loadR( "3d", &_3D, &zero );
  tag.loadR( "invisible", &invisible, &zero );
  tag.loadR( "useEnumNumeric", &useEnumNumeric, &zero );
  tag.loadR( "password", 31, pw, emptyStr );
  tag.loadR( "lock", &lock, &zero );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "visPv", &visPvExpString, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "colorPv", &colorPvExpString, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > MSGBTC_MAJOR_VERSION ) {
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

  if ( blank(pw) ) {
    usePassword = 0;
  }
  else if ( strcmp( pw, "*" ) == 0 ) {
    usePassword = 0;
  }
  else {
    usePassword = 1;
  }

  updateDimensions();

  return 1;

}

int activeMessageButtonClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > MSGBTC_MAJOR_VERSION ) {
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
    onColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    offColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    topShadowColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    botShadowColor = index;

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    onColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    offColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    topShadowColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    botShadowColor = index;

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    onColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    offColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    topShadowColor = actWin->ci->pix(pixel);

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    botShadowColor = actWin->ci->pix(pixel);

  }

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  destPvExpString.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f ); actWin->incLine();
  sourcePressPvExpString.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f ); actWin->incLine();
  sourceReleasePvExpString.setRaw( oneName );

  readStringFromFile( oneName, MAX_ENUM_STRING_SIZE+1, f ); actWin->incLine();
  onLabel.setRaw( oneName );

  readStringFromFile( oneName, MAX_ENUM_STRING_SIZE+1, f ); actWin->incLine();
  offLabel.setRaw( oneName );

  fscanf( f, "%d\n", &toggle ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    fscanf( f, "%d\n", &pressAction ); actWin->incLine();
    fscanf( f, "%d\n", &releaseAction ); actWin->incLine();
  }
  else {
    pressAction = 0;
    releaseAction = 0;
  }

  fscanf( f, "%d\n", &_3D ); actWin->incLine();

  fscanf( f, "%d\n", &invisible ); actWin->incLine();

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {
    readStringFromFile( pw, 31+1, f ); actWin->incLine();
    if ( blank(pw) ) {
      usePassword = 0;
    }
    else if ( strcmp( pw, "*" ) == 0 ) {
      usePassword = 0;
    }
    else {
      usePassword = 1;
    }
    fscanf( f, "%d\n", &lock );
  }
  else {
    strcpy( pw, "" );
    usePassword = 0;
    lock = 0;
  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 2 ) ) ) {

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    visPvExpString.setRaw( oneName );

    fscanf( f, "%d\n", &visInverted ); actWin->incLine();

    readStringFromFile( minVisString, 39+1, f ); actWin->incLine();

    readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();

  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 3 ) ) ) {

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    colorPvExpString.setRaw( oneName );

  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 4 ) ) ) {

    fscanf( f, "%d\n", &useEnumNumeric ); actWin->incLine();

  }
  else {

    useEnumNumeric = 1;

  }

  updateDimensions();

  return 1;

}

int activeMessageButtonClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin ){

int fgR, fgG, fgB, bgR, bgG, bgB, more, index;
unsigned int pixel;
char *tk, *gotData, *context, buf[255+1];
char tmpDestPvName[PV_Factory::MAX_PV_NAME+1], tmpSourcePressPvName[PV_Factory::MAX_PV_NAME+1];

  fgR = 0xffff;
  fgG = 0xffff;
  fgB = 0xffff;

  bgR = 0xffff;
  bgG = 0xffff;
  bgB = 0xffff;

  this->actWin = _actWin;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  onColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  offColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;
  strcpy( fontTag, actWin->defaultBtnFontTag );

  onLabel.setRaw( "" );
  // strcpy( onLabel, "" );
  offLabel.setRaw( "" );
  // strcpy( offLabel, "" );

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
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
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "fgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        fgR = atol( tk );

      }
            
      else if ( strcmp( tk, "fggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        fgG = atol( tk );

      }
            
      else if ( strcmp( tk, "fgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        fgB = atol( tk );

      }
            
      else if ( strcmp( tk, "bgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        bgR = atol( tk );

      }
            
      else if ( strcmp( tk, "bggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        bgG = atol( tk );

      }
            
      else if ( strcmp( tk, "bgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        bgB = atol( tk );

      }
            
      else if ( strcmp( tk, "closecurrentonpress" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        pressAction = atol( tk );

      }
            
      else if ( strcmp( tk, "invisible" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        invisible = atol( tk );

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str1 );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }

      else if ( strcmp( tk, "pressvalue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( tmpSourcePressPvName, tk, PV_Factory::MAX_PV_NAME );
          tmpSourcePressPvName[PV_Factory::MAX_PV_NAME] = 0;
          sourcePressPvExpString.setRaw( tmpSourcePressPvName );
	}

      }

      else if ( strcmp( tk, "presspv" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          strncpy( tmpDestPvName, tk, PV_Factory::MAX_PV_NAME );
          tmpDestPvName[PV_Factory::MAX_PV_NAME] = 0;
          destPvExpString.setRaw( tmpDestPvName );
	}

      }

      else if ( strcmp( tk, "onlabel" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          onLabel.setRaw( tk );
          // strncpy( onLabel, tk, MAX_ENUM_STRING_SIZE );
          // onLabel[MAX_ENUM_STRING_SIZE] = 0;
	}

      }

      else if ( strcmp( tk, "offlabel" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n \t", &context );
        if ( tk ) {
          offLabel.setRaw( tk );
          // strncpy( offLabel, tk, MAX_ENUM_STRING_SIZE );
          // offLabel[MAX_ENUM_STRING_SIZE] = 0;
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
  onColor.setColorIndex( index, actWin->ci );
  offColor.setColorIndex( index, actWin->ci );

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeMessageButtonClass::genericEdit ( void ) {

char title[32], *ptr, *envPtr, saveLock = 0;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  envPtr = getenv( "EDMSUPERVISORMODE" );
  if ( envPtr ) {
    if ( strcmp( envPtr, "TRUE" ) == 0 ) {
      if ( lock ) {
        actWin->appCtx->postMessage( activeMessageButtonClass_str34 );
      }
      saveLock = lock;
      lock = 0;
    }
  }

  ptr = actWin->obj.getNameFromClass( "activeMessageButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMessageButtonClass_str2, 31 );

  Strncat( title, activeMessageButtonClass_str3, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufFgColor = fgColor.pixelIndex();

  eBuf->bufOnColor = onColor.pixelIndex();

  eBuf->bufOffColor = offColor.pixelIndex();

  eBuf->bufTopShadowColor = topShadowColor;
  eBuf->bufBotShadowColor = botShadowColor;

  if ( destPvExpString.getRaw() )
    strncpy( eBuf->bufDestPvName, destPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufDestPvName, "" );

  if ( sourcePressPvExpString.getRaw() )
    strncpy( eBuf->bufSourcePressPvName, sourcePressPvExpString.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strncpy( eBuf->bufSourcePressPvName, "", PV_Factory::MAX_PV_NAME );

  if ( sourceReleasePvExpString.getRaw() )
    strncpy( eBuf->bufSourceReleasePvName, sourceReleasePvExpString.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strncpy( eBuf->bufSourceReleasePvName, "", PV_Factory::MAX_PV_NAME );

  if ( onLabel.getRaw() )
    strncpy( eBuf->bufOnLabel, onLabel.getRaw(), MAX_ENUM_STRING_SIZE );
  else
    strncpy( eBuf->bufOnLabel, "", MAX_ENUM_STRING_SIZE );

  if ( offLabel.getRaw() )
    strncpy( eBuf->bufOffLabel, offLabel.getRaw(), MAX_ENUM_STRING_SIZE );
  else
    strncpy( eBuf->bufOffLabel, "", MAX_ENUM_STRING_SIZE );

  if ( visPvExpString.getRaw() )
    strncpy( eBuf->bufVisPvName, visPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufVisPvName, "" );

  if ( colorPvExpString.getRaw() )
    strncpy( eBuf->bufColorPvName, colorPvExpString.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufColorPvName, "" );

  eBuf->bufToggle = toggle;
  eBuf->bufPressAction = pressAction;
  eBuf->bufReleaseAction = releaseAction;
  eBuf->buf3D = _3D;
  eBuf->bufInvisible = invisible;

  strcpy( bufPw1, "" );
  strcpy( bufPw2, "" );

  if ( envPtr ) {
    if ( strcmp( envPtr, "TRUE" ) == 0 ) {
      eBuf->bufLock = saveLock;
    }
  }
  else {
    eBuf->bufLock = lock;
  }

  if ( visInverted )
    eBuf->bufVisInverted = 0;
  else
    eBuf->bufVisInverted = 1;

  strncpy( eBuf->bufMinVisString, minVisString, 39 );
  strncpy( eBuf->bufMaxVisString, maxVisString, 39 );

  eBuf->bufUseEnumNumeric = useEnumNumeric;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeMessageButtonClass_str4, 35, &eBuf->bufX );
  ef.addTextField( activeMessageButtonClass_str5, 35, &eBuf->bufY );
  ef.addTextField( activeMessageButtonClass_str6, 35, &eBuf->bufW );
  ef.addTextField( activeMessageButtonClass_str7, 35, &eBuf->bufH );

  if ( !lock ) {
    ef.addTextField( activeMessageButtonClass_str18, 35, eBuf->bufDestPvName,
     PV_Factory::MAX_PV_NAME );
  }
  else {
    ef.addLockedField( activeMessageButtonClass_str18, 35, eBuf->bufDestPvName,
     PV_Factory::MAX_PV_NAME );
  }

  ef.addOption( activeMessageButtonClass_str8, activeMessageButtonClass_str9,
   &eBuf->bufToggle );
  ef.addToggle( activeMessageButtonClass_str10, &eBuf->buf3D );
  ef.addToggle( activeMessageButtonClass_str11, &eBuf->bufInvisible );
  ef.addToggle( activeMessageButtonClass_str12, &eBuf->bufPressAction );
  ef.addToggle( activeMessageButtonClass_str13, &eBuf->bufReleaseAction );
  ef.addToggle( activeMessageButtonClass_str35, &eBuf->bufUseEnumNumeric );

  ef.addTextField( activeMessageButtonClass_str14, 35, eBuf->bufOnLabel,
   MAX_ENUM_STRING_SIZE );

  if ( !lock ) {
    ef.addTextField( activeMessageButtonClass_str16, 35, eBuf->bufSourcePressPvName,
     PV_Factory::MAX_PV_NAME );
  }
  else {
    ef.addLockedField( activeMessageButtonClass_str16, 35,
     eBuf->bufSourcePressPvName, PV_Factory::MAX_PV_NAME );
  }

  ef.addTextField( activeMessageButtonClass_str15, 35, eBuf->bufOffLabel,
   MAX_ENUM_STRING_SIZE );

  if ( !lock ) {

    ef.addTextField( activeMessageButtonClass_str17, 35,
     eBuf->bufSourceReleasePvName, PV_Factory::MAX_PV_NAME );

    ef.addPasswordField( activeMessageButtonClass_str36, 35, bufPw1, 31 );
    ef.addPasswordField( activeMessageButtonClass_str37, 35, bufPw2, 31 );
    ef.addToggle( activeMessageButtonClass_str38, &eBuf->bufLock );

  }
  else {

    ef.addLockedField( activeMessageButtonClass_str17, 35,
     eBuf->bufSourceReleasePvName, PV_Factory::MAX_PV_NAME );

    ef.addLockedField( activeMessageButtonClass_str36, 35, bufPw1, 31 );
    ef.addLockedField( activeMessageButtonClass_str37, 35, bufPw2, 31 );

  }

  ef.addColorButton( activeMessageButtonClass_str20, actWin->ci, &fgCb,
   &eBuf->bufFgColor );

  ef.addColorButton( activeMessageButtonClass_str21, actWin->ci, &onCb,
   &eBuf->bufOnColor );

  ef.addColorButton( activeMessageButtonClass_str22, actWin->ci, &offCb,
   &eBuf->bufOffColor );

  ef.addColorButton( activeMessageButtonClass_str23, actWin->ci, &topShadowCb,
   &eBuf->bufTopShadowColor );

  ef.addColorButton( activeMessageButtonClass_str24, actWin->ci, &botShadowCb,
    &eBuf->bufBotShadowColor );

  ef.addFontMenu( activeMessageButtonClass_str19, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeMessageButtonClass_str32, 30, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeMessageButtonClass_str28, 30, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeMessageButtonClass_str29, &eBuf->bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeMessageButtonClass_str30, 30, eBuf->bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeMessageButtonClass_str31, 30, eBuf->bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  if ( envPtr ) {
    if ( strcmp( envPtr, "TRUE" ) == 0 ) {
      lock = saveLock;
    }
  }

  return 1;

}

int activeMessageButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( msgbtc_edit_ok, msgbtc_edit_apply, msgbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMessageButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( msgbtc_edit_ok, msgbtc_edit_apply, msgbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMessageButtonClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMessageButtonClass::eraseActive ( void ) {

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

int activeMessageButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  //actWin->drawGc.setFG( onColor.pixelColor() );
  actWin->drawGc.setFG( onColor.pixelIndex(), &blink );

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

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    //actWin->drawGc.setFG( fgColor.pixelColor() );
    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    if ( onLabel.getRaw() )
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, onLabel.getRaw() );
    else
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, "" );

    // drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
    //  XmALIGNMENT_CENTER, onLabel );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeMessageButtonClass::drawActive ( void ) {

int tX, tY;
char string[39+1];
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      //actWin->executeGc.setFG( onColor.getDisconnected() );
      actWin->executeGc.setFG( onColor.getDisconnectedIndex(), &blink );
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
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  if ( !buttonPressed ) {
    //actWin->executeGc.setFG( offColor.getColor() );
    actWin->executeGc.setFG( offColor.getIndex(), &blink );
  }
  else {
    //actWin->executeGc.setFG( onColor.getColor() );
    actWin->executeGc.setFG( onColor.getIndex(), &blink );
  }

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( !_3D ) {

    actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  }

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( !buttonPressed ) {

    if ( offLabel.getExpanded() )
      strncpy( string, offLabel.getExpanded(), MAX_ENUM_STRING_SIZE );
    else
      strncpy( string, "", MAX_ENUM_STRING_SIZE );

    // strncpy( string, offLabel, MAX_ENUM_STRING_SIZE );

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

    if ( onLabel.getExpanded() )
      strncpy( string, onLabel.getExpanded(), MAX_ENUM_STRING_SIZE );
    else
      strncpy( string, "", MAX_ENUM_STRING_SIZE );

    // strncpy( string, onLabel, MAX_ENUM_STRING_SIZE );

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

  if ( fs ) {

    actWin->executeGc.addNormXClipRectangle( xR );

    //actWin->executeGc.setFG( fgColor.getColor() );
    actWin->executeGc.setFG( fgColor.getIndex(), &blink );
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

int activeMessageButtonClass::activate (
  int pass,
  void *ptr )
{

int opStat, i, l;
char tmpPvName[PV_Factory::MAX_PV_NAME+1];

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      connection.init();
      initEnable();

      numStates = 0;

      needConnectInit = needErase = needDraw = needPerformDownAction =
       needPerformUpAction = needWarning = needVisConnectInit =
       needVisInit = needVisUpdate = needColorConnectInit =
       needColorInit = needColorUpdate = 0;
       needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      initialVisConnection = 1;
      initialColorConnection = 1;
      init = 0;
      aglPtr = ptr;
      destPvId = visPvId = colorPvId = NULL;

      sourcePressExists = sourceReleaseExists = 0;

      destPvConnected = sourcePressPvConnected = sourceReleasePvConnected =
       active = buttonPressed = 0;
      activeMode = 1;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
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

      opStat = 1;

      destIsAckS = 0;

      if ( destExists ) {

        strncpy( tmpPvName, destPvExpString.getExpanded(),
         PV_Factory::MAX_PV_NAME );

        l = strlen(tmpPvName);
        if ( l > 5 ) {
          i = l - 5;
          if ( strcmp( &tmpPvName[i], ".ACKS" ) == 0 ) {
            destIsAckS = 1;
            tmpPvName[i] = 0;
	  }
	}

        destPvId = the_PV_Factory->create( tmpPvName );
	if ( destPvId ) {
	  destPvId->add_conn_state_callback( msgbt_monitor_dest_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeMessageButtonClass_str25 );
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
	  visPvId->add_conn_state_callback( msgbt_monitor_vis_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeMessageButtonClass_str25 );
          opStat = 0;
        }

      }

      if ( colorExists ) {

        colorPvId = the_PV_Factory->create( colorPvExpString.getExpanded() );
	if ( colorPvId ) {
	  colorPvId->add_conn_state_callback(
           msgbt_monitor_color_connect_state, this );
	}
	else {
          fprintf( stderr, activeMessageButtonClass_str25 );
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

int activeMessageButtonClass::deactivate (
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

  if ( destExists ) {
    if ( destPvId ) {
      destPvId->remove_conn_state_callback( msgbt_monitor_dest_connect_state,
       this );
      destPvId->release();
      destPvId = NULL;
    }
  }

  if ( visExists ) {
    if ( visPvId ) {
      visPvId->remove_conn_state_callback( msgbt_monitor_vis_connect_state,
       this );
      visPvId->remove_value_callback( msgbt_visUpdate, this );
      visPvId->release();
      visPvId = NULL;
    }
  }

  if ( colorExists ) {
    if ( colorPvId ) {
      colorPvId->remove_conn_state_callback( msgbt_monitor_color_connect_state,
       this );
      colorPvId->remove_value_callback( msgbt_colorUpdate, this );
      colorPvId->release();
      colorPvId = NULL;
    }
  }

  }

  return 1;

}

void activeMessageButtonClass::updateDimensions ( void )
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

void activeMessageButtonClass::performBtnUpAction ( void ) {

int stat;
char tmpBuf[PV_Factory::MAX_PV_NAME+1];

  if ( toggle ) return;

  buttonPressed = 0;
  smartDrawAllActive();

  if ( strcmp( sourceReleasePvExpString.getExpanded(), "" ) == 0 ) return;

  if ( destPvId ) {
    if ( !destPvId->have_write_access() ) {
      return;
    }
  }

  actWin->substituteSpecial( PV_Factory::MAX_PV_NAME,
   sourceReleasePvExpString.getExpanded(),
   tmpBuf );
  tmpBuf[PV_Factory::MAX_PV_NAME] = 0;

  if ( destIsAckS ) {

    destV.s = (short) atol( tmpBuf );
    destPvId->putAck( XDisplayName(actWin->appCtx->displayName), destV.s );

  }
  else {

    switch ( destType ) {

    case ProcessVariable::Type::real:
      destV.d = atof( tmpBuf );
      destPvId->put( XDisplayName(actWin->appCtx->displayName), destV.d );
      break;

    case ProcessVariable::Type::integer:
      destV.l = atol( tmpBuf );
      destPvId->put( XDisplayName(actWin->appCtx->displayName), destV.l );
      break;

    case ProcessVariable::Type::text:
      strncpy( destV.str, tmpBuf, 39 );
      destV.str[39] = 0;
      destPvId->put( XDisplayName(actWin->appCtx->displayName), destV.str );
      break;

    case ProcessVariable::Type::enumerated:
      if ( useEnumNumeric ) {
        destV.l = atol( tmpBuf );
        destPvId->put( XDisplayName(actWin->appCtx->displayName), destV.l );
      }
      else {
        stat = getEnumNumeric( tmpBuf, &destV.l );
        if ( !( stat & 1 ) ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str40 );
        }
        else {
          destPvId->put( XDisplayName(actWin->appCtx->displayName), destV.l );
        }
      }
      break;

    }

  }

}

void activeMessageButtonClass::btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  if ( !enabled || !visibility ) {
    *action = 0;
    return;
  }

  if ( destPvId ) {
    if ( !destPvId->have_write_access() ) {
      return;
    }
  }

  if ( usePassword ) {
    *action = 0;
    return;
  }

  performBtnUpAction();

  if ( actWin->isEmbedded ) {
    *action = 0;
  }
  else {
    *action = releaseAction;
  }

}

void activeMessageButtonClass::performBtnDownAction ( void ) {

int stat;
char labelValue[PV_Factory::MAX_PV_NAME+1];

  if ( toggle ) {
    if ( buttonPressed ) {
      buttonPressed = 0;
    }
    else {
      buttonPressed = 1;
    }
  }
  else {
    buttonPressed = 1;
  }

  if ( buttonPressed ) {
    actWin->substituteSpecial( PV_Factory::MAX_PV_NAME,
     sourcePressPvExpString.getExpanded(),
     labelValue );
    labelValue[PV_Factory::MAX_PV_NAME] = 0;
  }
  else {
    actWin->substituteSpecial( PV_Factory::MAX_PV_NAME,
     sourceReleasePvExpString.getExpanded(),
     labelValue );
    labelValue[PV_Factory::MAX_PV_NAME] = 0;
  }

  smartDrawAllActive();

  if ( strcmp( labelValue, "" ) == 0 ) return;

  if ( destPvId ) {
    if ( !destPvId->have_write_access() ) {
      return;
    }
  }

  if ( destIsAckS ) {

    destV.s = (short) atol( labelValue );
    destPvId->putAck( XDisplayName(actWin->appCtx->displayName), destV.s );

  }
  else {

    switch ( destType ) {

    case ProcessVariable::Type::real:
      destV.d = atof( labelValue );
      destPvId->put( XDisplayName(actWin->appCtx->displayName), destV.d );
      break;

    case ProcessVariable::Type::integer:
      destV.l = atol( labelValue );
      destPvId->put( XDisplayName(actWin->appCtx->displayName), destV.l );
      break;

    case ProcessVariable::Type::text:
      strncpy( destV.str, labelValue, 39 );
      destPvId->put( XDisplayName(actWin->appCtx->displayName), destV.str );
      break;

    case ProcessVariable::Type::enumerated:
      if ( useEnumNumeric ) {
        destV.l = atol( labelValue );
        destPvId->put( XDisplayName(actWin->appCtx->displayName), destV.l );
      }
      else {
        stat = getEnumNumeric( labelValue, &destV.l );
        if ( !( stat & 1 ) ) {
          actWin->appCtx->postMessage( activeMessageButtonClass_str39 );
        }
        else {
          destPvId->put( XDisplayName(actWin->appCtx->displayName), destV.l );
        }
      }
      break;

    }

  }

}

void activeMessageButtonClass::btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  if ( buttonNumber != 1 ) return;

  if ( !enabled || !visibility ) {
    *action = 0;
    return;
  }

  if ( destPvId ) {
    if ( !destPvId->have_write_access() ) {
      return;
    }
  }

  if ( usePassword ) {

    if ( !ef.formIsPoppedUp() ) {

      pwFormX = be->x_root;
      pwFormY = be->y_root;
      pwFormW = 0;
      pwFormH = 0;
      pwFormMaxH = 600;

      ef.create( actWin->top,
       actWin->appCtx->ci.getColorMap(),
       &pwFormX, &pwFormY,
       &pwFormW, &pwFormH, &pwFormMaxH,
       "", NULL, NULL, NULL );

      strcpy( bufPw1, "" );

      ef.addPasswordField( activeMessageButtonClass_str36, 35, bufPw1, 31 );

      ef.finished( pw_ok, pw_apply, pw_cancel, this );

      ef.popup();

      *action = 0;
      return;

    }
    else {

      *action = 0;
      return;

    }

  }

  performBtnDownAction();

  if ( actWin->isEmbedded ) {
    *action = 0;
  }
  else {
    *action = pressAction;
  }

}

void activeMessageButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled || !active || !visibility ) return;

  if ( destPvId ) {
    if ( !destPvId->have_write_access() ) {
      actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );
    }
    else {
      actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_DEFAULT );
    }
  }

  activeGraphicClass::pointerIn( _x, _y, buttonState );

}

int activeMessageButtonClass::getButtonActionRequest (
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

int activeMessageButtonClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( destPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  destPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( sourcePressPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  sourcePressPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( sourceReleasePvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  sourceReleasePvExpString.setRaw( tmpStr.getExpanded() );

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

int activeMessageButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = destPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = sourcePressPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = sourceReleasePvExpString.expand1st( numMacros, macros, expansions );
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

int activeMessageButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = destPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = sourcePressPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = sourceReleasePvExpString.expand2nd( numMacros, macros, expansions );
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

int activeMessageButtonClass::containsMacros ( void ) {

  if ( destPvExpString.containsPrimaryMacros() ) return 1;

  if ( sourcePressPvExpString.containsPrimaryMacros() ) return 1;

  if ( sourceReleasePvExpString.containsPrimaryMacros() ) return 1;

  if ( onLabel.containsPrimaryMacros() ) return 1;

  if ( offLabel.containsPrimaryMacros() ) return 1;

  if ( visPvExpString.containsPrimaryMacros() ) return 1;

  if ( colorPvExpString.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeMessageButtonClass::executeDeferred ( void ) {

int nc, nd, ne, npda, npua, nw, nvc, nvi, nvu, ncc, nci, ncu;
int stat, index, invisColor;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nd = needDraw; needDraw = 0;
  ne = needErase; needErase = 0;
  npda = needPerformDownAction; needPerformDownAction = 0;
  npua = needPerformUpAction; needPerformUpAction = 0;
  nw = needWarning; needWarning = 0;
  nvc = needVisConnectInit; needVisConnectInit = 0;
  nvi = needVisInit; needVisInit = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  ncc = needColorConnectInit; needColorConnectInit = 0;
  nci = needColorInit; needColorInit = 0;
  ncu = needColorUpdate; needColorUpdate = 0;
  visValue = curVisValue;
  colorValue = curColorValue;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    destType = (int) destPvId->get_type().type;

    if ( destType == ProcessVariable::Type::enumerated ) {

      numStates = (int) destPvId->get_enum_count();

    }

    connection.setPvConnected( (void *) destPvConnection );

    if ( connection.pvsConnected() ) {
      active = 1;
      init = 1;
      onColor.setConnected();
      offColor.setConnected();
      smartDrawAllActive();
    }

  }

  if ( nvc ) {

    curVisValue = visPvId->get_double();

    minVis = atof( minVisString );
    maxVis = atof( maxVisString );

    nvi = 1;

  }

  if ( nvi ) {

    if ( initialVisConnection ) {
      initialVisConnection = 0;
      visPvId->add_value_callback( msgbt_visUpdate, this );
    }

    if ( ( visValue >= minVis ) &&
         ( visValue < maxVis ) )
      visibility = 1 ^ visInverted;
    else
      visibility = 0 ^ visInverted;

    if ( prevVisibility != visibility ) {
      if ( !visibility ) eraseActive();
    }

    connection.setPvConnected( (void *) visPvConnection );

    if ( connection.pvsConnected() ) {
      active = 1;
      init = 1;
      onColor.setConnected();
      offColor.setConnected();
      smartDrawAllActive();
    }

  }

  if ( ncc ) {

    curColorValue = colorPvId->get_double();

    nci = 1;

  }

  if ( nci ) {

    if ( initialColorConnection ) {
      initialColorConnection = 0;
      colorPvId->add_value_callback( msgbt_colorUpdate, this );
    }

    invisColor = 0;

    index = actWin->ci->evalRule( onColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    onColor.changeIndex( index, actWin->ci );

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
      active = 1;
      init = 1;
      onColor.setConnected();
      offColor.setConnected();
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

  if ( npda ) {

    performBtnDownAction();

    if ( pressAction && !actWin->isEmbedded ) {

      actWin->closeDeferred( 2 );

    }
    else {

      if ( !toggle ) {
        actWin->appCtx->proc->lock();
        needPerformUpAction = 1;
        actWin->addDefExeNode( aglPtr );
        actWin->appCtx->proc->unlock();
      }

    }

  }

//----------------------------------------------------------------------------

  if ( npua ) {

    performBtnUpAction();

    if ( !actWin->isEmbedded ) {
      if ( releaseAction ) actWin->closeDeferred( 2 );
    }

  }

//----------------------------------------------------------------------------

  if ( nw ) {

    actWin->appCtx->postMessage( activeMessageButtonClass_str41 );

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

  if ( ncu ) {

    invisColor = 0;

    index = actWin->ci->evalRule( onColor.pixelIndex(), colorValue );
    invisColor |= actWin->ci->isInvisible( index );
    onColor.changeIndex( index, actWin->ci );

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

char *activeMessageButtonClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeMessageButtonClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeMessageButtonClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( i == 0 ) {
      return destPvExpString.getExpanded();
    }
    else if ( i == 1 ) {
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
      return colorPvExpString.getRaw();
    }
    else {
      return visPvExpString.getRaw();
    }

  }

}

void activeMessageButtonClass::changeDisplayParams (
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
    onColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    offColor.setColorIndex( _bgColor, actWin->ci );

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

void activeMessageButtonClass::changePvNames (
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

int activeMessageButtonClass::getEnumNumeric (
  char *string,
  int *value ) {

  int i;

  for ( i=0; i<numStates; i++ ) {
    if ( strcmp( string, destPvId->get_enum( i ) ) == 0 ) {
      *value = i;
      return 1;
    }
  }

  *value = 0;
  return 0;

}

void activeMessageButtonClass::getPvs (
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

char *activeMessageButtonClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return destPvExpString.getRaw();
  }
  else if ( i == 1 ) {
    return colorPvExpString.getRaw();
  }
  else if ( i == 2 ) {
    return visPvExpString.getRaw();
  }
  else if ( i == 3 ) {
    return onLabel.getRaw();
  }
  else if ( i == 4 ) {
    return offLabel.getRaw();
  }
  else if ( i == 5 ) {
    return minVisString;
  }
  else if ( i == 6 ) {
    return maxVisString;
  }

  return NULL;

}

void activeMessageButtonClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    destPvExpString.setRaw( string );
  }
  else if ( i == 1 ) {
    colorPvExpString.setRaw( string );
  }
  else if ( i == 2 ) {
    visPvExpString.setRaw( string );
  }
  else if ( i == 3 ) {
    onLabel.setRaw( string );
  }
  else if ( i == 4 ) {
    offLabel.setRaw( string );
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
char *activeMessageButtonClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return destPvExpString.getExpanded();

}

char *activeMessageButtonClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >=2 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return colorPvExpString.getExpanded();
  }
  else if ( crawlerPvIndex == 2 ) {
    return visPvExpString.getExpanded();
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMessageButtonClassPtr ( void ) {

activeMessageButtonClass *ptr;

  ptr = new activeMessageButtonClass;
  return (void *) ptr;

}

void *clone_activeMessageButtonClassPtr (
  void *_srcPtr )
{

activeMessageButtonClass *ptr, *srcPtr;

  srcPtr = (activeMessageButtonClass *) _srcPtr;

  ptr = new activeMessageButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
