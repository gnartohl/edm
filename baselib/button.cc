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

#define __button_cc 1

#include "button.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) {

activeButtonClass *bto = (activeButtonClass *) ptr;

  if ( !bto->activeMode ) {
    if ( bto->isSelected() ) bto->drawSelectBoxCorners(); // erase via xor
    bto->smartDrawAll();
    if ( bto->isSelected() ) bto->drawSelectBoxCorners();
  }
  else {
    bto->bufInvalidate();
    bto->needDraw = 1;
    bto->actWin->addDefExeNode( bto->aglPtr );
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeButtonClass *bto = (activeButtonClass *) client;

  if ( !bto->init ) {
    bto->needToDrawUnconnected = 1;
    bto->needDraw = 1;
    bto->actWin->addDefExeNode( bto->aglPtr );
  }

  bto->unconnectedTimer = 0;

}

static void btc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeButtonClass *bto = (activeButtonClass *) client;

  bto->actWin->setChanged();

  bto->eraseSelectBoxCorners();
  bto->erase();

  bto->fgColorMode = bto->eBuf->bufFgColorMode;
  if ( bto->fgColorMode == BTC_K_COLORMODE_ALARM )
    bto->fgColor.setAlarmSensitive();
  else
    bto->fgColor.setAlarmInsensitive();
  bto->fgColor.setColorIndex( bto->eBuf->bufFgColor, bto->actWin->ci );

  bto->onColor.setColorIndex( bto->eBuf->bufOnColor, bto->actWin->ci );

  bto->offColor.setColorIndex( bto->eBuf->bufOffColor, bto->actWin->ci );

  bto->inconsistentColor.setColorIndex( bto->eBuf->bufInconsistentColor,
   bto->actWin->ci );

  bto->topShadowColor = bto->eBuf->bufTopShadowColor;
  bto->botShadowColor = bto->eBuf->bufBotShadowColor;

  
  bto->controlPvName.setRaw( bto->eBuf->controlBufPvName );
  bto->readPvName.setRaw( bto->eBuf->readBufPvName );

  strncpy( bto->onLabel, bto->eBuf->bufOnLabel, MAX_ENUM_STRING_SIZE );
  strncpy( bto->offLabel, bto->eBuf->bufOffLabel, MAX_ENUM_STRING_SIZE );

  if ( strcmp( bto->labelTypeString, activeButtonClass_str3 ) == 0 )
    bto->labelType = BTC_K_PV_STATE;
  else
    bto->labelType = BTC_K_LITERAL;

  strncpy( bto->fontTag, bto->fm.currentFontTag(), 63 );
  bto->actWin->fi->loadFontTag( bto->fontTag );
  bto->fs = bto->actWin->fi->getXFontStruct( bto->fontTag );

  if ( strcmp( bto->buttonTypeStr, activeButtonClass_str4 ) == 0 ) {
    bto->toggle = 0;
    bto->buttonType = BTC_K_PUSH;
  }
  else {
    bto->toggle = 1;
    bto->buttonType = BTC_K_TOGGLE;
  }

  if ( strcmp( bto->_3DString, activeButtonClass_str5 ) == 0 )
    bto->_3D = 1;
  else
    bto->_3D = 0;

  if ( strcmp( bto->invisibleString, activeButtonClass_str6 ) == 0 )
    bto->invisible = 1;
  else
    bto->invisible = 0;

  strncpy( bto->id, bto->bufId, 31 );
  bto->downCallbackFlag = bto->eBuf->bufDownCallbackFlag;
  bto->upCallbackFlag = bto->eBuf->bufUpCallbackFlag;
  bto->activateCallbackFlag = bto->eBuf->bufActivateCallbackFlag;
  bto->deactivateCallbackFlag = bto->eBuf->bufDeactivateCallbackFlag;
  bto->anyCallbackFlag = bto->downCallbackFlag || bto->upCallbackFlag ||
   bto->activateCallbackFlag || bto->deactivateCallbackFlag;

  bto->visPvExpString.setRaw( bto->eBuf->bufVisPvName );
  strncpy( bto->minVisString, bto->eBuf->bufMinVisString, 39 );
  strncpy( bto->maxVisString, bto->eBuf->bufMaxVisString, 39 );

  if ( bto->eBuf->bufVisInverted )
    bto->visInverted = 0;
  else
    bto->visInverted = 1;

  bto->colorPvExpString.setRaw( bto->eBuf->bufColorPvName );

  bto->efControlBitPos = bto->eBuf->bufEfControlBitPos;
  if ( bto->efControlBitPos.isNull() ) {
    bto->controlIsBit = 0;
    bto->controlBitPos = 0;
  }
  else {
    bto->controlIsBit = 1;
    bto->controlBitPos = bto->efControlBitPos.value();
    if ( bto->controlBitPos > 31 ) bto->controlBitPos = 31;
    if ( bto->controlBitPos < 0 ) bto->controlBitPos = 0;
  }

  bto->efReadBitPos = bto->eBuf->bufEfReadBitPos;
  if ( bto->efReadBitPos.isNull() ) {
    bto->readIsBit = 0;
    bto->readBitPos = 0;
  }
  else {
    bto->readIsBit = 1;
    bto->readBitPos = bto->efReadBitPos.value();
    if ( bto->readBitPos > 31 ) bto->readBitPos = 31;
    if ( bto->readBitPos < 0 ) bto->readBitPos = 0;
  }

  bto->x = bto->eBuf->bufX;
  bto->sboxX = bto->eBuf->bufX;

  bto->y = bto->eBuf->bufY;
  bto->sboxY = bto->eBuf->bufY;

  bto->w = bto->eBuf->bufW;
  bto->sboxW = bto->eBuf->bufW;

  bto->h = bto->eBuf->bufH;
  bto->sboxH = bto->eBuf->bufH;

  bto->updateDimensions();

}

static void btc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeButtonClass *bto = (activeButtonClass *) client;

  btc_edit_update ( w, client, call );
  bto->refresh( bto );

}

static void btc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeButtonClass *bto = (activeButtonClass *) client;

  btc_edit_update ( w, client, call );
  bto->ef.popdown();
  bto->operationComplete();

}

static void btc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeButtonClass *bto = (activeButtonClass *) client;

  bto->ef.popdown();
  bto->operationCancel();

}

static void btc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeButtonClass *bto = (activeButtonClass *) client;

  bto->ef.popdown();
  bto->operationCancel();
  bto->erase();
  bto->deleteRequest = 1;
  bto->drawAll();

}

static void bt_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeButtonClass *bto = (activeButtonClass *) userarg;

  if ( pv->is_valid() ) {

    bto->needCtlConnectInit = 1;

  }
  else {

    bto->connection.setPvDisconnected( (void *) bto->controlPvConnection );
    bto->controlValid = 0;
    bto->controlPvConnected = 0;
    bto->active = 0;
    bto->onColor.setDisconnected();
    bto->offColor.setDisconnected();
    bto->inconsistentColor.setDisconnected();
    bto->needDraw = 1;

  }

  bto->actWin->appCtx->proc->lock();
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeButtonClass *bto = (activeButtonClass *) userarg;
int st, sev;

  bto->controlValid = 1;
  //bto->curControlV = (short) pv->get_int();
  bto->curControlV = pv->get_int();

  if ( bto->controlIsBit ) {
    bto->controlBit = ( ( bto->curControlV & ( 1 << bto->controlBitPos ) ) > 0 );
  }

  if ( !bto->readExists ) {

    st = pv->get_status();
    sev = pv->get_severity();
    if ( ( st != bto->oldStat ) || ( sev != bto->oldSev ) ) {
      bto->oldStat = st;
      bto->oldSev = sev;
      bto->fgColor.setStatus( st, sev );
      bto->bufInvalidate();
    }

  }

  bto->needCtlRefresh = 1;
  bto->actWin->appCtx->proc->lock();
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeButtonClass *bto = (activeButtonClass *) userarg;

  if ( pv->is_valid() ) {

    bto->needReadConnectInit = 1;

  }
  else {

    bto->connection.setPvDisconnected( (void *) bto->readPvConnection );
    bto->readValid = 0;
    bto->readPvConnected = 0;
    bto->active = 0;
    bto->onColor.setDisconnected();
    bto->offColor.setDisconnected();
    bto->inconsistentColor.setDisconnected();
    bto->needDraw = 1;

  }

  bto->actWin->appCtx->proc->lock();
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeButtonClass *bto = (activeButtonClass *) userarg;
int st, sev;

  bto->readValid = 1;
  //bto->curReadV = (short) pv->get_int();
  bto->curReadV = pv->get_int();

  if ( bto->readIsBit ) {
    bto->readBit = ( ( bto->curReadV & ( 1 << bto->readBitPos ) ) > 0 );
  }

  st = pv->get_status();
  sev = pv->get_severity();
  if ( ( st != bto->oldStat ) || ( sev != bto->oldSev ) ) {
    bto->oldStat = st;
    bto->oldSev = sev;
    bto->fgColor.setStatus( st, sev );
    bto->bufInvalidate();
  }

  if ( bto->readIsBit ) {
    if ( bto->initReadBit || ( bto->readBit != bto->prevReadBit ) ) {
      bto->initReadBit = 0;
      bto->prevReadBit = bto->readBit;
      bto->needReadRefresh = 1;
      bto->actWin->appCtx->proc->lock();
      bto->actWin->addDefExeNode( bto->aglPtr );
      bto->actWin->appCtx->proc->unlock();
    }
  }
  else {
    bto->needReadRefresh = 1;
    bto->actWin->appCtx->proc->lock();
    bto->actWin->addDefExeNode( bto->aglPtr );
    bto->actWin->appCtx->proc->unlock();
  }

}

static void bt_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeButtonClass *bto = (activeButtonClass *) userarg;

  if ( pv->is_valid() ) {

    bto->needVisConnectInit = 1;

  }
  else {

    bto->connection.setPvDisconnected( (void *) bto->visPvConnection );
    bto->active = 0;
    bto->onColor.setDisconnected();
    bto->offColor.setDisconnected();
    bto->inconsistentColor.setDisconnected();
    bto->needDraw = 1;

  }

  bto->actWin->appCtx->proc->lock();
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_visUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeButtonClass *bto = (activeButtonClass *) userarg;

  bto->curVisValue = pv->get_double();

  bto->actWin->appCtx->proc->lock();
  bto->needVisUpdate = 1;
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeButtonClass *bto = (activeButtonClass *) userarg;

  if ( pv->is_valid() ) {

    bto->needColorConnectInit = 1;

  }
  else {

    bto->connection.setPvDisconnected( (void *) bto->colorPvConnection );
    bto->active = 0;
    bto->onColor.setDisconnected();
    bto->offColor.setDisconnected();
    bto->needDraw = 1;

  }

  bto->actWin->appCtx->proc->lock();
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

static void bt_colorUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeButtonClass *bto = (activeButtonClass *) userarg;

  bto->curColorValue = pv->get_double();

  bto->actWin->appCtx->proc->lock();
  bto->needColorUpdate = 1;
  bto->actWin->addDefExeNode( bto->aglPtr );
  bto->actWin->appCtx->proc->unlock();

}

activeButtonClass::activeButtonClass ( void ) {

  name = new char[strlen("activeButtonClass")+1];
  strcpy( name, "activeButtonClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  deleteRequest = 0;
  selected = 0;
  strcpy( id, "" );
  downCallbackFlag = 0;
  upCallbackFlag = 0;
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;
  anyCallbackFlag = 0;
  downCallback = NULL;
  upCallback = NULL;
  activateCallback = NULL;
  deactivateCallback = NULL;
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  connection.setMaxPvs( 4 );
  activeMode = 0;
  buttonIsDown = 0;

  controlIsBit = readIsBit = 0;
  prevControlBit = prevReadBit = 0;
  controlBitPos = readBitPos = 0;
  initControlBit = initReadBit = 0;
  efControlBitPos.setNull(1);
  efReadBitPos.setNull(1);

  fgColorMode = BTC_K_COLORMODE_STATIC;

  unconnectedTimer = 0;

  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

}

// copy constructor
activeButtonClass::activeButtonClass
 ( const activeButtonClass *source ) {

activeGraphicClass *bto = (activeGraphicClass *) this;

  bto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeButtonClass")+1];
  strcpy( name, "activeButtonClass" );

  deleteRequest = 0;

  strcpy( id, source->id );

  downCallbackFlag = source->downCallbackFlag;
  upCallbackFlag = source->upCallbackFlag;
  activateCallbackFlag = source->activateCallbackFlag;
  deactivateCallbackFlag = source->deactivateCallbackFlag;
  anyCallbackFlag = downCallbackFlag || upCallbackFlag ||
   activateCallbackFlag || deactivateCallbackFlag;
  downCallback = NULL;
  upCallback = NULL;
  activateCallback = NULL;
  deactivateCallback = NULL;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  fgColor.copy( source->fgColor );
  offColor.copy( source->offColor );
  onColor.copy( source->onColor );
  inconsistentColor.copy( source->inconsistentColor );

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  controlPvName.copy( source->controlPvName );
  readPvName.copy( source->readPvName );
  visPvExpString.copy( source->visPvExpString );
  colorPvExpString.copy( source->colorPvExpString );

  strncpy( onLabel, source->onLabel, MAX_ENUM_STRING_SIZE );
  strncpy( offLabel, source->offLabel, MAX_ENUM_STRING_SIZE );

  labelType = source->labelType;

  buttonType = source->buttonType;
  if ( buttonType == BTC_K_TOGGLE )
    toggle = 1;
  else
    toggle = 0;

  _3D = source->_3D;
  invisible = source->invisible;

  unconnectedTimer = 0;

  visibility = 0;
  prevVisibility = -1;
  visInverted = source->visInverted;
  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );
  activeMode = 0;
  buttonIsDown = 0;

  prevControlBit = prevReadBit = 0;
  initControlBit = initReadBit = 0;
  controlIsBit = source->controlIsBit;
  readIsBit = source->readIsBit;
  controlBitPos = source->controlBitPos;
  efControlBitPos = source->efControlBitPos;
  readBitPos = source->readBitPos;
  efReadBitPos = source->efReadBitPos;

  fgColorMode = source->fgColorMode;

  connection.setMaxPvs( 4 );

  doAccSubs( controlPvName );
  doAccSubs( readPvName );
  doAccSubs( colorPvExpString );
  doAccSubs( visPvExpString );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );
  doAccSubs( onLabel, MAX_ENUM_STRING_SIZE );
  doAccSubs( offLabel, MAX_ENUM_STRING_SIZE );

  updateDimensions();

  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

}

activeButtonClass::~activeButtonClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

int activeButtonClass::createInteractive (
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
  inconsistentColor.setColorIndex( actWin->defaultOffsetColor,
   actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  strcpy( fontTag, actWin->defaultBtnFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

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

  updateDimensions();

  strcpy( onLabel, "" );
  strcpy( offLabel, "" );

  labelType = BTC_K_PV_STATE;
  buttonType = BTC_K_TOGGLE;
  toggle = 1;
  _3D = 1;
  invisible = 0;

  this->draw();

  this->editCreate();

  return 1;

}

int activeButtonClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
char *emptyStr = "";

int labelTypePvState = BTC_K_PV_STATE;
static char *labelEnumStr[2] = {
  "pvState",
  "literal"
};
static int labelEnum[2] = {
  BTC_K_PV_STATE,
  BTC_K_LITERAL
};

int buttonTypeToggle = BTC_K_TOGGLE;
static char *buttonTypeEnumStr[2] = {
  "toggle",
  "push"
};
static int buttonTypeEnum[2] = {
  BTC_K_TOGGLE,
  BTC_K_PUSH
};

int objTypeUnknown = activeGraphicClass::UNKNOWN;
static char *objTypeEnumStr[4] = {
  "unknown",
  "graphics",
  "monitors",
  "controls",
};
static int objTypeEnum[4] = {
  activeGraphicClass::UNKNOWN,
  activeGraphicClass::GRAPHICS,
  activeGraphicClass::MONITORS,
  activeGraphicClass::CONTROLS
};

  major = BTC_MAJOR_VERSION;
  minor = BTC_MINOR_VERSION;
  release = BTC_RELEASE;

  if ( toggle )
    buttonType = BTC_K_TOGGLE;
  else
    buttonType = BTC_K_PUSH;

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
  tag.loadBoolW( "fgAlarm", &fgColorMode, &zero );
  tag.loadW( "onColor", actWin->ci, &onColor );
  tag.loadW( "offColor", actWin->ci, &offColor );
  tag.loadW( "inconsistentColor", actWin->ci, &inconsistentColor );
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "controlPv", &controlPvName, emptyStr );
  tag.loadW( "indicatorPv", &readPvName, emptyStr );
  tag.loadW( "onLabel", onLabel, emptyStr );
  tag.loadW( "offLabel", offLabel, emptyStr );
  tag.loadW( "labelType", 2, labelEnumStr, labelEnum, &labelType,
   &labelTypePvState );
  tag.loadW( "buttonType", 2, buttonTypeEnumStr, buttonTypeEnum, &buttonType,
   &buttonTypeToggle );
  tag.loadBoolW( "3d", &_3D, &zero );
  tag.loadBoolW( "invisible", &invisible, &zero );
  tag.loadW( "font", fontTag );
  tag.loadW( "objType", 4, objTypeEnumStr, objTypeEnum, &objType,
   &objTypeUnknown );
  tag.loadW( "visPv", &visPvExpString, emptyStr );
  tag.loadBoolW( "visInvert", &visInverted, &zero );
  tag.loadW( "visMin", minVisString, emptyStr );
  tag.loadW( "visMax", maxVisString, emptyStr );
  tag.loadW( "colorPv", &colorPvExpString, emptyStr  );
  tag.loadW( "controlBitPos", &efControlBitPos );
  tag.loadW( "readBitPos", &efReadBitPos );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeButtonClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", BTC_MAJOR_VERSION, BTC_MINOR_VERSION,
   BTC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fgColorMode );

  index = onColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = offColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = inconsistentColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = topShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = botShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  if ( controlPvName.getRaw() )
    writeStringToFile( f, controlPvName.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( readPvName.getRaw() )
    writeStringToFile( f, readPvName.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, onLabel );
  writeStringToFile( f, offLabel );

  fprintf( f, "%-d\n", labelType );

  if ( toggle )
    buttonType = BTC_K_TOGGLE;
  else
    buttonType = BTC_K_PUSH;

  fprintf( f, "%-d\n", buttonType );

  fprintf( f, "%-d\n", _3D );

  fprintf( f, "%-d\n", invisible );

  writeStringToFile( f, fontTag );

  // version 1.3.0
  writeStringToFile( f, id );
  fprintf( f, "%-d\n", downCallbackFlag );
  fprintf( f, "%-d\n", upCallbackFlag );
  fprintf( f, "%-d\n", activateCallbackFlag );
  fprintf( f, "%-d\n", deactivateCallbackFlag );

  // version 2.1
  fprintf( f, "%-d\n", objType );

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

  return 1;

}

int activeButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
char *emptyStr = "";

int labelTypePvState = BTC_K_PV_STATE;
static char *labelEnumStr[2] = {
  "pvState",
  "literal"
};
static int labelEnum[2] = {
  BTC_K_PV_STATE,
  BTC_K_LITERAL
};

int buttonTypeToggle = BTC_K_TOGGLE;
static char *buttonTypeEnumStr[2] = {
  "toggle",
  "push"
};
static int buttonTypeEnum[2] = {
  BTC_K_TOGGLE,
  BTC_K_PUSH
};

int objTypeUnknown = activeGraphicClass::UNKNOWN;
static char *objTypeEnumStr[4] = {
  "graphics",
  "monitors",
  "controls",
  "unknown"
};
static int objTypeEnum[4] = {
  activeGraphicClass::GRAPHICS,
  activeGraphicClass::MONITORS,
  activeGraphicClass::CONTROLS,
  activeGraphicClass::UNKNOWN
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
  tag.loadR( "fgAlarm", &fgColorMode, &zero );
  tag.loadR( "onColor", actWin->ci, &onColor );
  tag.loadR( "offColor", actWin->ci, &offColor );
  tag.loadR( "inconsistentColor", actWin->ci, &inconsistentColor );
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "controlPv", &controlPvName, emptyStr );
  tag.loadR( "indicatorPv", &readPvName, emptyStr );
  tag.loadR( "onLabel", MAX_ENUM_STRING_SIZE, onLabel, emptyStr );
  tag.loadR( "offLabel", MAX_ENUM_STRING_SIZE, offLabel, emptyStr );
  tag.loadR( "labelType", 2, labelEnumStr, labelEnum, &labelType,
   &labelTypePvState );
  tag.loadR( "buttonType", 2, buttonTypeEnumStr, buttonTypeEnum, &buttonType,
   &buttonTypeToggle );
  tag.loadR( "3d", &_3D, &zero );
  tag.loadR( "invisible", &invisible, &zero );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "objType", 4, objTypeEnumStr, objTypeEnum, &objType,
   &objTypeUnknown );
  tag.loadR( "visPv", &visPvExpString, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "colorPv", &colorPvExpString, emptyStr );
  tag.loadR( "controlBitPos", &efControlBitPos );
  tag.loadR( "readBitPos", &efReadBitPos );
  tag.loadR( "endObjectProperties" );
  tag.loadR( "" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > BTC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( efControlBitPos.isNull() ) {
    controlIsBit = 0;
    controlBitPos = 0;
  }
  else {
    controlIsBit = 1;
    controlBitPos = efControlBitPos.value();
    if ( controlBitPos > 31 ) controlBitPos = 31;
    if ( controlBitPos < 0 ) controlBitPos = 0;
  }

  if ( efReadBitPos.isNull() ) {
    readIsBit = 0;
    readBitPos = 0;
  }
  else {
    readIsBit = 1;
    readBitPos = efReadBitPos.value();
    if ( readBitPos > 31 ) readBitPos = 31;
    if ( readBitPos < 0 ) readBitPos = 0;
  }

  if ( fgColorMode == BTC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  if ( buttonType == BTC_K_TOGGLE )
    toggle = 1;
  else
    toggle = 0;

  strcpy( this->id, "" );
  downCallbackFlag = 0;
  upCallbackFlag = 0;
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;
  anyCallbackFlag = 0;

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return stat;

}

int activeButtonClass::old_createFromFile (
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

  if ( major > BTC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == BTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    onColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    offColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    inconsistentColor.setColorIndex( index, actWin->ci );

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

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == BTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    onColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    offColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    inconsistentColor.setColorIndex( index, actWin->ci );

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

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == BTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

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
    index = actWin->ci->pixIndex( pixel );
    inconsistentColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    topShadowColor = index;

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    botShadowColor = index;

  }

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();

  controlPvName.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  readPvName.setRaw( oneName );

  readStringFromFile( onLabel, MAX_ENUM_STRING_SIZE+1, f ); actWin->incLine();

  readStringFromFile( offLabel, MAX_ENUM_STRING_SIZE+1, f ); actWin->incLine();

  fscanf( f, "%d\n", &labelType ); actWin->incLine();

  fscanf( f, "%d\n", &buttonType ); actWin->incLine();

  if ( buttonType == BTC_K_TOGGLE )
    toggle = 1;
  else
    toggle = 0;

  fscanf( f, "%d\n", &_3D ); actWin->incLine();

  fscanf( f, "%d\n", &invisible ); actWin->incLine();

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    readStringFromFile( this->id, 31+1, f ); actWin->incLine();
    fscanf( f, "%d\n", &downCallbackFlag ); actWin->incLine();
    fscanf( f, "%d\n", &upCallbackFlag ); actWin->incLine();
    fscanf( f, "%d\n", &activateCallbackFlag ); actWin->incLine();
    fscanf( f, "%d\n", &deactivateCallbackFlag ); actWin->incLine();
    anyCallbackFlag = downCallbackFlag || upCallbackFlag ||
     activateCallbackFlag || deactivateCallbackFlag;
  }
  else {
    strcpy( this->id, "" );
    downCallbackFlag = 0;
    upCallbackFlag = 0;
    activateCallbackFlag = 0;
    deactivateCallbackFlag = 0;
    anyCallbackFlag = 0;
  }

  if ( ( ( major == 2 ) && ( minor > 0 ) ) || ( major > 2 ) ) {
    fscanf( f, "%d\n", &objType );
  }
  else {
    objType = -1;
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

  this->initSelectBox();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeButtonClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, more, param;
char *tk, *gotData, *context, buf[255+1];

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;

  this->actWin = _actWin;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  onColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  offColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  inconsistentColor.setColorIndex( actWin->defaultOffsetColor,
   actWin->ci );
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  strcpy( fontTag, actWin->defaultBtnFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

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

  updateDimensions();

  strcpy( onLabel, "" );
  strcpy( offLabel, "" );

  labelType = BTC_K_PV_STATE;
  buttonType = BTC_K_TOGGLE;
  toggle = 1;
  _3D = 1;
  invisible = 0;

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( activeButtonClass_str57 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( activeButtonClass_str57 );
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
          actWin->appCtx->postMessage( activeButtonClass_str57 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeButtonClass_str57 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeButtonClass_str57 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeButtonClass_str57 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "ctlpv" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );

        if ( tk )
          controlPvName.setRaw( tk );
        else
          controlPvName.setRaw( "" );

      }
            
      else if ( strcmp( tk, "readpv" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );

        if ( tk )
          readPvName.setRaw( tk );
        else
          readPvName.setRaw( "" );

      }
            
      else if ( strcmp( tk, "truelabel" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );

        if ( tk ) {
          strncpy( onLabel, tk, MAX_ENUM_STRING_SIZE );
          onLabel[MAX_ENUM_STRING_SIZE] = 0;
	}
	else {
          strcpy( onLabel, "" );
	}

      }
            
      else if ( strcmp( tk, "falselabel" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );

        if ( tk ) {
          strncpy( offLabel, tk, MAX_ENUM_STRING_SIZE );
          offLabel[MAX_ENUM_STRING_SIZE] = 0;
	}
	else {
          strcpy( offLabel, "" );
	}

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeButtonClass_str57 );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }
            
      else if ( strcmp( tk, "push" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeButtonClass_str57 );
          return 0;
        }

        param = atol( tk );
        if ( param == 1 ) {
          buttonType = BTC_K_PUSH;
          toggle = 0;
	}
	else {
          buttonType = BTC_K_TOGGLE;
          toggle = 1;
	}

      }
            
      else if ( strcmp( tk, "3d" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeButtonClass_str57 );
          return 0;
        }

        _3D = atol( tk );

      }
            
      else if ( strcmp( tk, "labelfrompv" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeButtonClass_str57 );
          return 0;
        }

        param = atol( tk );
        if ( param == 1 ) {
          labelType = BTC_K_PV_STATE;
	}
	else {
          labelType = BTC_K_LITERAL;
	}

      }
            
      else if ( strcmp( tk, "invisible" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeButtonClass_str57 );
          return 0;
        }

        invisible = atol( tk );

      }
            
    }

  } while ( more );

  this->initSelectBox();

  fgColor.setAlarmInsensitive();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int activeButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeButtonClass_str7, 31 );

  Strncat( title, activeButtonClass_str8, 31 );

  strncpy( bufId, id, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufFgColor = fgColor.pixelIndex();
  eBuf->bufFgColorMode = fgColorMode;

  eBuf->bufOnColor = onColor.pixelIndex();

  eBuf->bufOffColor = offColor.pixelIndex();

  eBuf->bufInconsistentColor = inconsistentColor.pixelIndex();

  eBuf->bufTopShadowColor = topShadowColor;
  eBuf->bufBotShadowColor = botShadowColor;
  strncpy( eBuf->bufFontTag, fontTag, 63 );

  if ( controlPvName.getRaw() )
    strncpy( eBuf->controlBufPvName, controlPvName.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->controlBufPvName, "" );

  if ( readPvName.getRaw() )
    strncpy( eBuf->readBufPvName, readPvName.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->readBufPvName, "" );

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

  strncpy( eBuf->bufOnLabel, onLabel, MAX_ENUM_STRING_SIZE );
  strncpy( eBuf->bufOffLabel, offLabel, MAX_ENUM_STRING_SIZE );

  eBuf->bufDownCallbackFlag = downCallbackFlag;
  eBuf->bufUpCallbackFlag = upCallbackFlag;
  eBuf->bufActivateCallbackFlag = activateCallbackFlag;
  eBuf->bufDeactivateCallbackFlag = deactivateCallbackFlag;

  if ( labelType == BTC_K_PV_STATE )
    strcpy( labelTypeString, activeButtonClass_str9 );
  else
    strcpy( labelTypeString, activeButtonClass_str10 );

  if ( toggle )
    strcpy( buttonTypeStr, activeButtonClass_str11 );
  else
    strcpy( buttonTypeStr, activeButtonClass_str12 );

  if ( _3D )
    strcpy( _3DString, activeButtonClass_str13 );
  else
    strcpy( _3DString, activeButtonClass_str14 );

  if ( invisible )
    strcpy( invisibleString, activeButtonClass_str15 );
  else
    strcpy( invisibleString, activeButtonClass_str16 );

  if ( visInverted )
    eBuf->bufVisInverted = 0;
  else
    eBuf->bufVisInverted = 1;

  strncpy( eBuf->bufMinVisString, minVisString, 39 );
  strncpy( eBuf->bufMaxVisString, maxVisString, 39 );

  eBuf->bufEfControlBitPos = efControlBitPos;
  eBuf->bufEfReadBitPos = efReadBitPos;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeButtonClass_str18, 35, &eBuf->bufX );
  ef.addTextField( activeButtonClass_str19, 35, &eBuf->bufY );
  ef.addTextField( activeButtonClass_str20, 35, &eBuf->bufW );
  ef.addTextField( activeButtonClass_str21, 35, &eBuf->bufH );

  ef.addTextField( activeButtonClass_str22, 35, eBuf->controlBufPvName,
   PV_Factory::MAX_PV_NAME );
  ctlPvEntry = ef.getCurItem();
  ef.addTextField( "Bit", 35, &eBuf->bufEfControlBitPos );
  ctlPvBitEntry = ef.getCurItem();
  ctlPvEntry->addDependency( ctlPvBitEntry );
  ctlPvEntry->addDependencyCallbacks();

  ef.addTextField( activeButtonClass_str23, 35, eBuf->readBufPvName,
   PV_Factory::MAX_PV_NAME );
  rdPvEntry = ef.getCurItem();
  ef.addTextField( "Bit", 35, &eBuf->bufEfReadBitPos );
  rdPvBitEntry = ef.getCurItem();
  rdPvEntry->addDependency( rdPvBitEntry );
  rdPvEntry->addDependencyCallbacks();

  ef.addOption( activeButtonClass_str24, activeButtonClass_str25,
   buttonTypeStr, 7 );
  ef.addOption( activeButtonClass_str26, activeButtonClass_str27, _3DString,
   7 );
  ef.addOption( activeButtonClass_str28, activeButtonClass_str29,
   invisibleString, 7 );
  ef.addOption( activeButtonClass_str30, activeButtonClass_str31,
   labelTypeString, 15 );
  ef.addTextField( activeButtonClass_str32, 35, eBuf->bufOnLabel,
   MAX_ENUM_STRING_SIZE );
  ef.addTextField( activeButtonClass_str33, 35, eBuf->bufOffLabel,
   MAX_ENUM_STRING_SIZE );

  ef.addColorButton( activeButtonClass_str39, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addToggle( activeButtonClass_str41, &eBuf->bufFgColorMode );
  ef.addColorButton( activeButtonClass_str42, actWin->ci, &eBuf->onCb, &eBuf->bufOnColor );
  ef.addColorButton( activeButtonClass_str43, actWin->ci, &eBuf->offCb,
   &eBuf->bufOffColor );
  ef.addColorButton( activeButtonClass_str44, actWin->ci,
   &eBuf->inconsistentCb, &eBuf->bufInconsistentColor );
  ef.addColorButton( activeButtonClass_str45, actWin->ci, &eBuf->topShadowCb,
   &eBuf->bufTopShadowColor );
  ef.addColorButton( activeButtonClass_str46, actWin->ci, &eBuf->botShadowCb,
   &eBuf->bufBotShadowColor );

  ef.addFontMenu( activeButtonClass_str38, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeButtonClass_str62, 30, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeButtonClass_str58, 30, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeButtonClass_str59, &eBuf->bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeButtonClass_str60, 30, eBuf->bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeButtonClass_str61, 30, eBuf->bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  return 1;

}

int activeButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( btc_edit_ok, btc_edit_apply, btc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( btc_edit_ok, btc_edit_apply, btc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeButtonClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeButtonClass::eraseActive ( void ) {

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

int activeButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };
int blink = 0;

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( onColor.pixelIndex(), &blink );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

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

    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
     XmALIGNMENT_CENTER, onLabel );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeButtonClass::drawActive ( void ) {

int cV, rV, tX, tY;
XRectangle xR = { x, y, w, h };
char string[MAX_ENUM_STRING_SIZE+1];
int blink = 0;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
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

  cV = controlV;
  rV = readV;

  if ( controlIsBit ) {
    cV = controlBit;
  }

  if ( readIsBit ) {
    rV = readBit;
  }

  actWin->executeGc.saveFg();
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  if ( controlExists && readExists ) {

    if ( ( cV != rV ) || !controlValid || !readValid ) {
      actWin->executeGc.setFG( inconsistentColor.getIndex(), &blink );
    }
    else if ( cV == 0 ) {
      actWin->executeGc.setFG( offColor.getIndex(), &blink );
    }
    else {
      actWin->executeGc.setFG( onColor.getIndex(), &blink );
    }

  }
  else if ( readExists ) {

    if ( rV == 0 ) {
      actWin->executeGc.setFG( offColor.getIndex(), &blink );
    }
    else {
      actWin->executeGc.setFG( onColor.getIndex(), &blink );
    }

    cV = rV;

  }
  else if ( controlExists ) {

    if ( cV == 0 ) {
      actWin->executeGc.setFG( offColor.getIndex(), &blink );
    }
    else {
      actWin->executeGc.setFG( onColor.getIndex(), &blink );
    }

  }
  else if ( anyCallbackFlag ) {

    if ( cV != rV ) {
      actWin->executeGc.setFG( inconsistentColor.getIndex(), &blink );
    }
    else if ( cV == 0 ) {
      actWin->executeGc.setFG( offColor.getIndex(), &blink );
    }
    else {
      actWin->executeGc.setFG( onColor.getIndex(), &blink );
    }

  }
  else {

    actWin->executeGc.setFG( inconsistentColor.getIndex(), &blink );

  }

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( cV == 0 ) {

    if ( labelType == BTC_K_LITERAL ) {
      strncpy( string, offLabel, MAX_ENUM_STRING_SIZE );
    }
    else {
      if ( stateStringPvId && stateStringPvId->get_enum_count() > 0 ) {
        strncpy( string, (char *) stateStringPvId->get_enum( 0 ),
         MAX_ENUM_STRING_SIZE );
      }
      else {
        strncpy( string, "0", MAX_ENUM_STRING_SIZE );
      }

    }

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

    if ( labelType == BTC_K_LITERAL ) {
      strncpy( string, onLabel, MAX_ENUM_STRING_SIZE );
    }
    else {
      if ( stateStringPvId && stateStringPvId->get_enum_count() > 1 ) {
        strncpy( string, (char *) stateStringPvId->get_enum( 1 ),
         MAX_ENUM_STRING_SIZE );
      }
      else {
        strncpy( string, "1", MAX_ENUM_STRING_SIZE );
      }
    }

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

    actWin->executeGc.setFG( fgColor.getIndex(), &blink );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    if ( labelType == BTC_K_LITERAL ) {

      drawText( actWin->executeWidget, drawable(actWin->executeWidget),
       &actWin->executeGc, fs, tX, tY, XmALIGNMENT_CENTER, string );

    }
    else {

      drawText( actWin->executeWidget, drawable(actWin->executeWidget),
       &actWin->executeGc, fs, tX, tY, XmALIGNMENT_CENTER, string );

    }

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeButtonClass::activate (
  int pass,
  void *ptr )
{

int opStat;
char callbackName[63+1];

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      connection.init();
      initEnable();

      aglPtr = ptr;
      needCtlConnectInit = needCtlInfoInit = needCtlRefresh =
       needReadConnectInit = needReadInfoInit = needReadRefresh =
       needErase = needDraw = needVisConnectInit = needVisInit =
       needVisUpdate = needColorConnectInit =
       needColorInit = needColorUpdate = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      initialConnection = initialReadConnection = initialVisConnection =
       initialColorConnection = 1;
      oldStat = -1;
      oldSev = -1;
      init = 0;
      controlValid = 0;
      readValid = 0;
      controlV = 0;
      controlPvId = readPvId = visPvId = colorPvId = stateStringPvId = NULL;

      controlPvConnected = readPvConnected = active = 0;
      activeMode = 1;

      if ( !controlPvName.getExpanded() ||
        blankOrComment( controlPvName.getExpanded() ) ) {
        controlExists = 0;
      }
      else {
        controlExists = 1;
        connection.addPv();
      }

      if ( !readPvName.getExpanded() ||
        blankOrComment( readPvName.getExpanded() ) ) {
        readExists = 0;
      }
      else {
        readExists = 1;
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

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      if ( anyCallbackFlag ) {

        if ( downCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, "Down", 63 );
          downCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( upCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          Strncat( callbackName, "Up", 63 );
          upCallback =
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

      opStat = 1;

      if ( controlExists ) {

	controlPvId = the_PV_Factory->create( controlPvName.getExpanded() );
        if ( controlPvId ) {
	  controlPvId->add_conn_state_callback(
           bt_monitor_control_connect_state, this );
          if ( !readExists ) stateStringPvId = controlPvId;
	}
	else {
          fprintf( stderr, activeButtonClass_str47 );
          opStat = 0;
        }

      }

      if ( readExists ) {

	readPvId = the_PV_Factory->create( readPvName.getExpanded() );
        if ( readPvId ) {
	  readPvId->add_conn_state_callback(
           bt_monitor_read_connect_state, this );
          stateStringPvId = readPvId;
	}
	else {
          fprintf( stderr, activeButtonClass_str47 );
          opStat = 0;
        }

      }

      if ( visExists ) {

	visPvId = the_PV_Factory->create( visPvExpString.getExpanded() );
        if ( visPvId ) {
	  visPvId->add_conn_state_callback(
           bt_monitor_vis_connect_state, this );
	}
	else {
          fprintf( stderr, activeButtonClass_str47 );
          opStat = 0;
        }

      }

      if ( colorExists ) {

	colorPvId = the_PV_Factory->create( colorPvExpString.getExpanded() );
        if ( colorPvId ) {
	  colorPvId->add_conn_state_callback(
           bt_monitor_color_connect_state, this );
	}
	else {
          fprintf( stderr, activeButtonClass_str47 );
          opStat = 0;
        }

      }

      if ( !( opStat & 1 ) ) opComplete = 1;

      if ( !controlExists && !readExists ) {
        init = 1;
        active = 1;
        onColor.setConnected();
        offColor.setConnected();
        inconsistentColor.setConnected();
        controlV = readV = 0;
      }

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

int activeButtonClass::deactivate (
  int pass
) {

  active = 0;
  activeMode = 0;

  if ( pass == 1 ) {

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  if ( deactivateCallback ) {
    (*deactivateCallback)( this );
  }

  if ( controlExists ) {
    if ( controlPvId ) {
      controlPvId->remove_conn_state_callback(
       bt_monitor_control_connect_state, this );
      controlPvId->remove_value_callback(
       bt_controlUpdate, this );
      controlPvId->release();
      controlPvId = NULL;
    }
  }

  if ( readExists ) {
    if ( readPvId ) {
      readPvId->remove_conn_state_callback(
       bt_monitor_read_connect_state, this );
      readPvId->remove_value_callback(
       bt_readUpdate, this );
      readPvId->release();
      readPvId = NULL;
    }
  }

  if ( visExists ) {
    if ( visPvId ) {
      visPvId->remove_conn_state_callback(
       bt_monitor_vis_connect_state, this );
      visPvId->remove_value_callback(
       bt_visUpdate, this );
      visPvId->release();
      visPvId = NULL;
    }
  }

  if ( colorExists ) {
    if ( colorPvId ) {
      colorPvId->remove_conn_state_callback(
       bt_monitor_color_connect_state, this );
      colorPvId->remove_value_callback(
       bt_colorUpdate, this );
      colorPvId->release();
      colorPvId = NULL;
    }
  }

  }

  return 1;

}

void activeButtonClass::updateDimensions ( void )
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

void activeButtonClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

//short value;
int value;
int stat;
unsigned int uival;

  if ( !enabled || !active || !visibility ) return;

  if ( !controlPvId->have_write_access() ) return;

  buttonIsDown = 0;

  if ( toggle ) return;

  value = 0;

  if ( !controlExists ) controlV = 0;

  if ( controlExists ) {
    if ( controlIsBit ) {
      controlBit = 0;
    }
  }

  if ( upCallback ) {
    (*upCallback)( this );
  }

  if ( !controlExists ) return;

  if ( controlIsBit ) {
    uival = controlV & ( ~( (unsigned int) 1 << controlBitPos ) );
    stat = controlPvId->put( XDisplayName(actWin->appCtx->displayName), (int) uival );
  }
  else {
    stat = controlPvId->put( XDisplayName(actWin->appCtx->displayName), value );
  }

}

void activeButtonClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

//short value;
int value;
int stat, curControlBit;
unsigned int uival;

  if ( !enabled || !active || !visibility ) return;

  if ( controlExists ) {
    if ( !controlPvId->have_write_access() ) return;
  }

  if ( buttonNumber != 1 ) return;

  buttonIsDown = 1;

  if ( controlExists && controlIsBit ) {

    curControlBit = ( ( controlV & ( 1 << controlBitPos ) ) > 0 );

    if ( toggle ) {
      if ( curControlBit == 0 ) {
        controlBit = 1;
      }
      else {
        controlBit = 0;
      }
    }
    else {
      controlBit = 1;
    }

    if ( controlBit ) {
      uival = controlV | ( (unsigned int) 1 << controlBitPos );
    }
    else {
      uival = controlV & ( ~( (unsigned int) 1 << controlBitPos ) );
    }

    stat = controlPvId->put( XDisplayName(actWin->appCtx->displayName), (int) uival );

  }
  else {

    if ( toggle ) {
      if ( controlV == 0 ) {
        value = 1;
        if ( !controlExists ) controlV = 1;
        if ( downCallback ) {
          (*downCallback)( this );
        }
      }
      else {
        value = 0;
        if ( !controlExists ) controlV = 0;
        if ( upCallback ) {
          (*upCallback)( this );
        }
      }
    }
    else {
      value = 1;
      if ( !controlExists ) controlV = 1;
      if ( downCallback ) {
        (*downCallback)( this );
      }
    }

    if ( !controlExists ) return;
    stat = controlPvId->put( XDisplayName(actWin->appCtx->displayName), value );

  }

}

void activeButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled || !active || !visibility ) return;

  if ( !controlPvId->have_write_access() ) {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );
  }
  else {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_DEFAULT );
  }

  activeGraphicClass::pointerIn( _x, _y, buttonState );

}

int activeButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;

  if ( controlExists ) {
    *focus = 1;
    *up = 1;
    *down = 1;
  }
  else {
    *focus = 0;
    *up = 0;
    *down = 0;
  }

  return 1;

}

int activeButtonClass::expandTemplate (
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

  tmpStr.setRaw( visPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  visPvExpString.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( colorPvExpString.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  colorPvExpString.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = controlPvName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readPvName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = controlPvName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readPvName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpString.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeButtonClass::containsMacros ( void ) {

  if ( controlPvName.containsPrimaryMacros() ) return 1;

  if ( readPvName.containsPrimaryMacros() ) return 1;

  if ( visPvExpString.containsPrimaryMacros() ) return 1;

  if ( colorPvExpString.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeButtonClass::executeDeferred ( void ) {

int ncc, nci, ncr, nrc, nri, nrr, ne, nd, nvc, nvi, nvu, ncolc, ncoli, ncolu;
int stat, index, invisColor;

//short rv, cv;
int rv, cv;
char msg[79+1];

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  ncc = needCtlConnectInit; needCtlConnectInit = 0;
  nci = needCtlInfoInit; needCtlInfoInit = 0;
  ncr = needCtlRefresh; needCtlRefresh = 0;
  nrc = needReadConnectInit; needReadConnectInit = 0;
  nri = needReadInfoInit; needReadInfoInit = 0;
  nrr = needReadRefresh; needReadRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nvc = needVisConnectInit; needVisConnectInit = 0;
  nvi = needVisInit; needVisInit = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  ncolc = needColorConnectInit; needColorConnectInit = 0;
  ncoli = needColorInit; needColorInit = 0;
  ncolu = needColorUpdate; needColorUpdate = 0;
  rv = curReadV;
  cv = curControlV;
  visValue = curVisValue;
  colorValue = curColorValue;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

  if ( ncc ) {

    if ( controlIsBit &&
         ( ( controlPvId->get_type().type == ProcessVariable::Type::integer ) ||
           ( controlPvId->get_type().type == ProcessVariable::Type::real ) ) ) {
      initControlBit = 1;
    }
    else if ( controlPvId->get_type().type !=
     ProcessVariable::Type::enumerated ) {
      strncpy( msg, actWin->obj.getNameFromClass( "activeButtonClass" ),
       79 );
      Strncat( msg, activeButtonClass_str51, 79 );
      actWin->appCtx->postMessage( msg );
      controlPvConnected = 0;
      active = 0;
      return;
    }

    //cv = curControlV = (short) controlPvId->get_int();
    cv = curControlV = controlPvId->get_int();

    if ( controlIsBit ) {
      prevControlBit = controlBit = ( ( cv & ( 1 << controlBitPos ) ) > 0 );
    }

    nci = 1;

  }

  if ( nci ) {

    connection.setPvConnected( (void *) controlPvConnection );

    if ( initialConnection ) {

      initialConnection = 0;
      
      controlPvId->add_value_callback( bt_controlUpdate, this );

    }

    controlPvConnected = 1;

    if ( connection.pvsConnected() ) {
      onColor.setConnected();
      offColor.setConnected();
      inconsistentColor.setConnected();
      init = 1;
      active = 1;
      eraseActive();
      readV = rv;
      controlV = cv;
      smartDrawAllActive();
    }

  }

  if ( ncr ) {

    eraseActive();
    readV = rv; 
    controlV = cv;
    smartDrawAllActive();

  }

  if ( nrc ) {

    if ( readIsBit &&
         ( ( readPvId->get_type().type == ProcessVariable::Type::integer ) ||
           ( readPvId->get_type().type == ProcessVariable::Type::real ) ) ) {
      initReadBit = 1;
    }
    else if ( readPvId->get_type().type !=
     ProcessVariable::Type::enumerated ) {
      strncpy( msg, actWin->obj.getNameFromClass( "activeButtonClass" ),
       79 );
      Strncat( msg, activeButtonClass_str54, 79 );
      actWin->appCtx->postMessage( msg );
      readPvConnected = 0;
      active = 0;
      return;
    }

    //rv = curReadV = (short) readPvId->get_int();
    rv = curReadV = readPvId->get_int();

    if ( readIsBit ) {
      prevReadBit = readBit = ( ( rv & ( 1 << readBitPos ) ) > 0 );
    }

    nri = 1;

  }

  if ( nri ) {

    connection.setPvConnected( (void *) readPvConnection );

    if ( initialReadConnection ) {

      initialReadConnection = 0;
      
      readPvId->add_value_callback( bt_readUpdate, this );

    }

    readPvConnected = 1;

    if ( connection.pvsConnected() ) {
      onColor.setConnected();
      offColor.setConnected();
      inconsistentColor.setConnected();
      init = 1;
      active = 1;
      eraseActive();
      controlV = cv;
      readV = rv;
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
      
      visPvId->add_value_callback( bt_visUpdate, this );

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
      active = 1;
      init = 1;
      onColor.setConnected();
      offColor.setConnected();
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
      
      colorPvId->add_value_callback( bt_colorUpdate, this );

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

  if ( nrr ) {

    eraseActive();
    controlV = cv;
    readV = rv;
    smartDrawAllActive();

  }

  if ( ne ) {

    eraseActive();

  }

  if ( nd ) {

    smartDrawAllActive();

  }

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

  if ( ncolu ) {

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

int activeButtonClass::setProperty (
  char *prop,
  int *value )
{

  if ( strcmp( prop, "controlValue" ) == 0 ) {

    //curControlV = (short) *value;
    curControlV = *value;
    needCtlRefresh = 1;
    actWin->appCtx->proc->lock();
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return 1;

  }
  else if ( strcmp( prop, "readValue" ) == 0 ) {

    //curReadV = (short) *value;
    curReadV = *value;
    needReadRefresh = 1;
    actWin->appCtx->proc->lock();
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return 1;

  }

  return 0;

}

char *activeButtonClass::firstDragName ( void ) {

  // If the button is down and we use the middle-click to drag the PV name, the toggle does not seem to reset.
  // Ignore the drag event in case the button is down
  if(buttonIsDown) return NULL;

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeButtonClass::nextDragName ( void ) {

  // If the button is down and we use the middle-click to drag the PV name, the toggle does not seem to reset.
  // Ignore the drag event in case the button is down
  if(buttonIsDown) return NULL;

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeButtonClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( i == 0 ) {
      return controlPvName.getExpanded();
    }
    else if ( i == 1 ) {
      return readPvName.getExpanded();
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
      return controlPvName.getRaw();
    }
    else if ( i == 1 ) {
      return readPvName.getRaw();
    }
    else if ( i == 2 ) {
      return colorPvExpString.getRaw();
    }
    else {
      return visPvExpString.getRaw();
    }

  }

}

void activeButtonClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_OFFSETCOLOR_MASK )
    inconsistentColor.setColorIndex( _offsetColor, actWin->ci );

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

void activeButtonClass::changePvNames (
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

  if ( flag & ACTGRF_VISPVS_MASK ) {
    if ( numVisPvs ) {
      visPvExpString.setRaw( ctlPvs[0] );
    }
  }

}

void activeButtonClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 4 ) {
    *n = 0;
    return;
  }

  *n = 4;
  pvs[0] = controlPvId;
  pvs[1] = readPvId;
  pvs[2] = colorPvId;
  pvs[3] = visPvId;

}

char *activeButtonClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return controlPvName.getRaw();
  }
  else if ( i == 1 ) {
    return readPvName.getRaw();
  }
  else if ( i == 2 ) {
    return colorPvExpString.getRaw();
  }
  else if ( i == 3 ) {
    return visPvExpString.getRaw();
  }
  else if ( i == 4 ) {
    return minVisString;
  }
  else if ( i == 5 ) {
    return maxVisString;
  }
  else if ( i == 6 ) {
    return onLabel;
  }
  else if ( i == 7 ) {
    return offLabel;
  }

  return NULL;

}

void activeButtonClass::replaceString (
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
    colorPvExpString.setRaw( string );
  }
  else if ( i == 3 ) {
    visPvExpString.setRaw( string );
  }
  else if ( i == 4 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( minVisString, string, l );
    minVisString[l] = 0;
  }
  else if ( i == 5 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( maxVisString, string, l );
    maxVisString[l] = 0;
  }
  else if ( i == 6 ) {
    int l = max;
    if ( MAX_ENUM_STRING_SIZE < max ) l = MAX_ENUM_STRING_SIZE;
    strncpy( onLabel, string, l );
    onLabel[l] = 0;
  }
  else if ( i == 7 ) {
    int l = max;
    if ( MAX_ENUM_STRING_SIZE < max ) l = MAX_ENUM_STRING_SIZE;
    strncpy( offLabel, string, l );
    offLabel[l] = 0;
  }

  updateDimensions();

}

// crawler functions may return blank pv names
char *activeButtonClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return controlPvName.getExpanded();

}

char *activeButtonClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >=3 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return readPvName.getExpanded();
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

void *create_activeButtonClassPtr ( void ) {

activeButtonClass *ptr;

  ptr = new activeButtonClass;
  return (void *) ptr;

}

void *clone_activeButtonClassPtr (
  void *_srcPtr )
{

activeButtonClass *ptr, *srcPtr;

  srcPtr = (activeButtonClass *) _srcPtr;

  ptr = new activeButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
