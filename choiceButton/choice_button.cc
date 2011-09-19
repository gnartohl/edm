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

#define __choice_button_cc 1

#include "choice_button.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) {

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) ptr;

  if ( !acbo->activeMode ) {
    if ( acbo->isSelected() ) acbo->drawSelectBoxCorners(); // erase via xor
    acbo->smartDrawAll();
    if ( acbo->isSelected() ) acbo->drawSelectBoxCorners();
  }
  else {
    acbo->bufInvalidate();
    acbo->needDraw = 1;
    acbo->actWin->addDefExeNode( acbo->aglPtr );
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) client;

  if ( !acbo->connection.pvsConnected() ) {
    acbo->needToDrawUnconnected = 1;
    acbo->needDraw = 1;
    acbo->actWin->addDefExeNode( acbo->aglPtr );
  }

  acbo->unconnectedTimer = 0;

}

static void acb_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) userarg;

  if ( pv->is_valid() ) {

    acbo->connection.setPvConnected( (void *) acbo->controlPvConnection );
    acbo->needConnectInit = 1;

    if ( acbo->connection.pvsConnected() ) {
      acbo->fgColor.setConnected();
    }

  }
  else {

    acbo->connection.setPvDisconnected( (void *) acbo->controlPvConnection );
    acbo->fgColor.setDisconnected();
    acbo->controlValid = 0;
    acbo->needDraw = 1;
    acbo->active = 0;

  }

  acbo->actWin->appCtx->proc->lock();
  acbo->actWin->addDefExeNode( acbo->aglPtr );
  acbo->actWin->appCtx->proc->unlock();

}

static void acb_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) userarg;
int st, sev;

  acbo->curValue = (short) pv->get_int();

  st = pv->get_status();
  sev = pv->get_severity();
  if ( ( st != acbo->oldStat ) || ( sev != acbo->oldSev ) ) {
    acbo->oldStat = st;
    acbo->oldSev = sev;
    acbo->fgColor.setStatus( st, sev );
    acbo->bufInvalidate();
  }

  acbo->controlValid = 1;
  acbo->needRefresh = 1;
  acbo->needDraw = 1;
  acbo->actWin->appCtx->proc->lock();
  acbo->actWin->addDefExeNode( acbo->aglPtr );
  acbo->actWin->appCtx->proc->unlock();

}

static void acb_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) userarg;

  if ( pv->is_valid() ) {

    acbo->connection.setPvConnected( (void *) acbo->readPvConnection );
    acbo->needReadConnectInit = 1;

    if ( acbo->connection.pvsConnected() ) {
      acbo->fgColor.setConnected();
    }

  }
  else {

    acbo->connection.setPvDisconnected( (void *) acbo->readPvConnection );
    acbo->fgColor.setDisconnected();
    acbo->readValid = 0;
    acbo->needDraw = 1;
    acbo->active = 0;

  }

  acbo->actWin->appCtx->proc->lock();
  acbo->actWin->addDefExeNode( acbo->aglPtr );
  acbo->actWin->appCtx->proc->unlock();

}

static void acb_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) userarg;
int st, sev;

  acbo->curReadValue = (short) pv->get_int();

  if ( !acbo->controlExists ) {
    st = pv->get_status();
    sev = pv->get_severity();
    if ( ( st != acbo->oldStat ) || ( sev != acbo->oldSev ) ) {
      acbo->oldStat = st;
      acbo->oldSev = sev;
      acbo->fgColor.setStatus( st, sev );
      acbo->bufInvalidate();
    }
  }

  acbo->readValid = 1;
  acbo->needRefresh = 1;
  acbo->needDraw = 1;
  acbo->actWin->appCtx->proc->lock();
  acbo->actWin->addDefExeNode( acbo->aglPtr );
  acbo->actWin->appCtx->proc->unlock();

}

static void acb_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) userarg;

  if ( pv->is_valid() ) {

    acbo->needVisConnectInit = 1;

  }
  else {

    acbo->connection.setPvDisconnected( (void *) acbo->visPvConnection );
    acbo->fgColor.setDisconnected();
    acbo->active = 0;
    acbo->needDraw = 1;

  }

  acbo->actWin->appCtx->proc->lock();
  acbo->actWin->addDefExeNode( acbo->aglPtr );
  acbo->actWin->appCtx->proc->unlock();

}

static void acb_visUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) userarg;

  acbo->curVisValue = pv->get_double();

  acbo->actWin->appCtx->proc->lock();
  acbo->needVisUpdate = 1;
  acbo->actWin->addDefExeNode( acbo->aglPtr );
  acbo->actWin->appCtx->proc->unlock();

}

static void acb_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) userarg;

  if ( pv->is_valid() ) {

    acbo->needColorConnectInit = 1;

  }
  else {

    acbo->connection.setPvDisconnected( (void *) acbo->colorPvConnection );
    acbo->fgColor.setDisconnected();
    acbo->active = 0;
    acbo->needDraw = 1;

  }

  acbo->actWin->appCtx->proc->lock();
  acbo->actWin->addDefExeNode( acbo->aglPtr );
  acbo->actWin->appCtx->proc->unlock();

}

static void acb_colorUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) userarg;

  acbo->curColorValue = pv->get_double();

  acbo->actWin->appCtx->proc->lock();
  acbo->needColorUpdate = 1;
  acbo->actWin->addDefExeNode( acbo->aglPtr );
  acbo->actWin->appCtx->proc->unlock();

}

static void acbc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) client;

  acbo->actWin->setChanged();

  acbo->eraseSelectBoxCorners();
  acbo->erase();

  strncpy( acbo->fontTag, acbo->fm.currentFontTag(), 63 );
  acbo->actWin->fi->loadFontTag( acbo->fontTag );
  acbo->actWin->drawGc.setFontTag( acbo->fontTag, acbo->actWin->fi );
  acbo->actWin->fi->getTextFontList( acbo->fontTag, &acbo->fontList );
  acbo->fs = acbo->actWin->fi->getXFontStruct( acbo->fontTag );

  acbo->topShadowColor = acbo->bufTopShadowColor;
  acbo->botShadowColor = acbo->bufBotShadowColor;

  acbo->fgColorMode = acbo->bufFgColorMode;
  if ( acbo->fgColorMode == ACBC_K_COLORMODE_ALARM )
    acbo->fgColor.setAlarmSensitive();
  else
    acbo->fgColor.setAlarmInsensitive();
  acbo->fgColor.setColorIndex( acbo->bufFgColor, acbo->actWin->ci );

  acbo->bgColorMode = acbo->bufBgColorMode;
  if ( acbo->bgColorMode == ACBC_K_COLORMODE_ALARM )
    acbo->bgColor.setAlarmSensitive();
  else
    acbo->bgColor.setAlarmInsensitive();
  acbo->bgColor.setColorIndex( acbo->bufBgColor, acbo->actWin->ci );

  acbo->selColor.setColorIndex( acbo->bufSelColor, acbo->actWin->ci );

  acbo->inconsistentColor.setColorIndex( acbo->bufInconsistentColor,
   acbo->actWin->ci );

  acbo->visPvExpStr.setRaw( acbo->eBuf->bufVisPvName );
  strncpy( acbo->minVisString, acbo->bufMinVisString, 39 );
  strncpy( acbo->maxVisString, acbo->bufMaxVisString, 39 );

  if ( acbo->bufVisInverted )
    acbo->visInverted = 0;
  else
    acbo->visInverted = 1;

  acbo->colorPvExpStr.setRaw( acbo->eBuf->bufColorPvName );

  acbo->x = acbo->bufX;
  acbo->sboxX = acbo->bufX;

  acbo->y = acbo->bufY;
  acbo->sboxY = acbo->bufY;

  acbo->w = acbo->bufW;
  acbo->sboxW = acbo->bufW;

  acbo->h = acbo->bufH;
  acbo->sboxH = acbo->bufH;

  acbo->controlPvExpStr.setRaw( acbo->eBuf->bufControlPvName );

  acbo->readPvExpStr.setRaw( acbo->eBuf->bufReadPvName );

  acbo->orientation = acbo->bufOrientation;


  acbo->updateDimensions();

}

static void acbc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) client;

  acbc_edit_update ( w, client, call );
  acbo->refresh( acbo );

}

static void acbc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) client;

  acbc_edit_update ( w, client, call );
  acbo->ef.popdown();
  acbo->operationComplete();

}

static void acbc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) client;

  acbo->ef.popdown();
  acbo->operationCancel();

}

static void acbc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeChoiceButtonClass *acbo = (activeChoiceButtonClass *) client;

  acbo->ef.popdown();
  acbo->operationCancel();
  acbo->erase();
  acbo->deleteRequest = 1;
  acbo->drawAll();

}

activeChoiceButtonClass::activeChoiceButtonClass ( void ) {

  name = new char[strlen("activeChoiceButtonClass")+1];
  strcpy( name, "activeChoiceButtonClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  fgColorMode = ACBC_K_COLORMODE_STATIC;
  bgColorMode = ACBC_K_COLORMODE_STATIC;

  active = 0;
  activeMode = 0;

  fontList = NULL;

  connection.setMaxPvs( 4 );

  unconnectedTimer = 0;

  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );

  orientation = ACBC_K_ORIENTATION_VERT;

  eBuf = NULL;

  crawlerPvIndex = 0;

  setBlinkFunction( (void *) doBlink );

}

activeChoiceButtonClass::~activeChoiceButtonClass ( void ) {

  if ( name ) delete[] name;
  if ( eBuf ) delete eBuf;
  if ( fontList ) XmFontListFree( fontList );

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

// copy constructor
activeChoiceButtonClass::activeChoiceButtonClass
 ( const activeChoiceButtonClass *source ) {

activeGraphicClass *acbo = (activeGraphicClass *) this;

  acbo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeChoiceButtonClass")+1];
  strcpy( name, "activeChoiceButtonClass" );

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);
  selColor = source->selColor;
  inconsistentColor = source->inconsistentColor;
  fgCb = source->fgCb;
  bgCb = source->bgCb;
  selCb = source->selCb;
  inconsistentCb = source->inconsistentCb;

  fgColorMode = source->fgColorMode;
  bgColorMode = source->bgColorMode;

  controlPvExpStr.copy( source->controlPvExpStr );
  readPvExpStr.copy( source->readPvExpStr );
  visPvExpStr.copy( source->visPvExpStr );
  colorPvExpStr.copy( source->colorPvExpStr );

  active = 0;
  activeMode = 0;

  connection.setMaxPvs( 4 );

  unconnectedTimer = 0;

  visibility = 0;
  prevVisibility = -1;
  visInverted = source->visInverted;
  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

  orientation = source->orientation;

  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

  crawlerPvIndex = 0;

  doAccSubs( controlPvExpStr );
  doAccSubs( readPvExpStr );
  doAccSubs( colorPvExpStr );
  doAccSubs( visPvExpStr );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );

}

int activeChoiceButtonClass::createInteractive (
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

  strcpy( fontTag, actWin->defaultBtnFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  selColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  inconsistentColor.setColorIndex( actWin->defaultOffsetColor,
   actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int activeChoiceButtonClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
char *emptyStr = "";

int vert = 0;
static char *orienTypeEnumStr[2] = {
  "vertical",
  "horizontal"
};
static int orienTypeEnum[2] = {
  0,
  1
};

  major = ACBC_MAJOR_VERSION;
  minor = ACBC_MINOR_VERSION;
  release = ACBC_RELEASE;

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
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadBoolW( "bgAlarm", &bgColorMode, &zero );
  tag.loadW( "selectColor", actWin->ci, &selColor );
  tag.loadW( "inconsistentColor", actWin->ci, &inconsistentColor );
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "controlPv", &controlPvExpStr, emptyStr );
  tag.loadW( "indicatorPv", &readPvExpStr, emptyStr );
  tag.loadW( "font", fontTag );
  tag.loadW( "visPv", &visPvExpStr, emptyStr );
  tag.loadBoolW( "visInvert", &visInverted, &zero );
  tag.loadW( "visMin", minVisString, emptyStr );
  tag.loadW( "visMax", maxVisString, emptyStr );
  tag.loadW( "colorPv", &colorPvExpStr, emptyStr );
  tag.loadW( "orientation", 2, orienTypeEnumStr, orienTypeEnum,
   &orientation, &vert );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeChoiceButtonClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", ACBC_MAJOR_VERSION, ACBC_MINOR_VERSION,
   ACBC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  fprintf( f, "%-d\n", fgColorMode );

  index = selColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  fprintf( f, "%-d\n", bgColorMode );

  index = topShadowColor;
  actWin->ci->writeColorIndex( f, index );

  index = botShadowColor;
  actWin->ci->writeColorIndex( f, index );

  if ( controlPvExpStr.getRaw() )
    writeStringToFile( f, controlPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  if ( readPvExpStr.getRaw() )
    writeStringToFile( f, readPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  index = inconsistentColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  if ( visPvExpStr.getRaw() )
    writeStringToFile( f, visPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );
  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  if ( colorPvExpStr.getRaw() )
    writeStringToFile( f, colorPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", orientation );

  return 1;

}

int activeChoiceButtonClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
char *emptyStr = "";

int vert = 0;
static char *orienTypeEnumStr[2] = {
  "vertical",
  "horizontal"
};
static int orienTypeEnum[2] = {
  0,
  1
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
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "bgAlarm", &bgColorMode, &zero );
  tag.loadR( "selectColor", actWin->ci, &selColor );
  tag.loadR( "inconsistentColor", actWin->ci, &inconsistentColor );
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "controlPv", &controlPvExpStr, emptyStr );
  tag.loadR( "indicatorPv", &readPvExpStr, emptyStr );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "visPv", &visPvExpStr, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "colorPv", &colorPvExpStr, emptyStr );
  tag.loadR( "orientation", 2, orienTypeEnumStr, orienTypeEnum,
   &orientation, &vert );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > ACBC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( fgColorMode == ACBC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  if ( bgColorMode == ACBC_K_COLORMODE_ALARM )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return 1;

}

int activeChoiceButtonClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int index;
int major, minor, release;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > ACBC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  fgColor.setColorIndex( index, actWin->ci );

  fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  selColor.setColorIndex( index, actWin->ci );

  if ( fgColorMode == ACBC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  bgColor.setColorIndex( index, actWin->ci );

  fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

  if ( bgColorMode == ACBC_K_COLORMODE_ALARM )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  topShadowColor = index;

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  botShadowColor = index;

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  readPvExpStr.setRaw( oneName );

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  inconsistentColor.setColorIndex( index, actWin->ci );

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  visPvExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  readStringFromFile( minVisString, 39+1, f ); actWin->incLine();

  readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  colorPvExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &orientation );

  updateDimensions();

  return 1;

}

int activeChoiceButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeChoiceButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeChoiceButtonClass_str2, 31 );

  Strncat( title, activeChoiceButtonClass_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  strncpy( bufFontTag, fontTag, 63 );

  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;

  bufFgColor = fgColor.pixelIndex();
  bufFgColorMode = fgColorMode;

  bufBgColor = bgColor.pixelIndex();
  bufBgColorMode = bgColorMode;

  bufSelColor = selColor.pixelIndex();

  bufInconsistentColor = inconsistentColor.pixelIndex();

  if ( controlPvExpStr.getRaw() )
    strncpy( eBuf->bufControlPvName, controlPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufControlPvName, "" );

  if ( readPvExpStr.getRaw() )
    strncpy( eBuf->bufReadPvName, readPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufReadPvName, "" );

  if ( visPvExpStr.getRaw() )
    strncpy( eBuf->bufVisPvName, visPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufVisPvName, "" );

  if ( visInverted )
    bufVisInverted = 0;
  else
    bufVisInverted = 1;

  if ( colorPvExpStr.getRaw() )
    strncpy( eBuf->bufColorPvName, colorPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufColorPvName, "" );

  strncpy( bufMinVisString, minVisString, 39 );
  strncpy( bufMaxVisString, maxVisString, 39 );

  bufOrientation = orientation;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeChoiceButtonClass_str4, 35, &bufX );
  ef.addTextField( activeChoiceButtonClass_str5, 35, &bufY );
  ef.addTextField( activeChoiceButtonClass_str6, 35, &bufW );
  ef.addTextField( activeChoiceButtonClass_str7, 35, &bufH );
  ef.addTextField( activeChoiceButtonClass_str17, 35, eBuf->bufControlPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeChoiceButtonClass_str18, 35, eBuf->bufReadPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addOption( activeChoiceButtonClass_str35, activeChoiceButtonClass_str36,
   &bufOrientation );

  ef.addColorButton( activeChoiceButtonClass_str8, actWin->ci, &fgCb,
   &bufFgColor );
  ef.addToggle( activeChoiceButtonClass_str10, &bufFgColorMode );

  ef.addColorButton( activeChoiceButtonClass_str37, actWin->ci,
   &selCb, &bufSelColor );

  ef.addColorButton( activeChoiceButtonClass_str11, actWin->ci, &bgCb,
   &bufBgColor );

  ef.addColorButton( activeChoiceButtonClass_str29, actWin->ci,
   &inconsistentCb, &bufInconsistentColor );

  ef.addColorButton( activeChoiceButtonClass_str14, actWin->ci, &topShadowCb,
   &bufTopShadowColor );

  ef.addColorButton( activeChoiceButtonClass_str15, actWin->ci, &botShadowCb,
   &bufBotShadowColor );

  ef.addFontMenu( activeChoiceButtonClass_str16, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeChoiceButtonClass_str34, 35, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeChoiceButtonClass_str30, 35, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeChoiceButtonClass_str31, &bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeChoiceButtonClass_str32, 35, bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeChoiceButtonClass_str33, 35, bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  return 1;

}

int activeChoiceButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( acbc_edit_ok, acbc_edit_apply, acbc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeChoiceButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( acbc_edit_ok, acbc_edit_apply, acbc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeChoiceButtonClass::erase ( void ) {

  if ( deleteRequest || activeMode ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeChoiceButtonClass::eraseActive ( void ) {

  if ( !enabled || !init || !activeMode ) return 1;

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

int activeChoiceButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };
int blink = 0;
int i, buttonX, buttonY, buttonH, buttonW, extra, lowExtra, highExtra,
 margin = 3;
int buttonNumStates = 3;
int buttonSelected = buttonNumStates-1;

char *buttonLabel[3] = { "0", "1", "2" };

  if ( deleteRequest || activeMode ) return 1;

  actWin->drawGc.saveFg();

  if ( orientation == ACBC_K_ORIENTATION_HORZ ) {

    buttonH = h;
    if ( buttonH < 3 ) buttonH = 3;
    if ( buttonNumStates > 0 ) {
      buttonW = ( w - (buttonNumStates-1) * margin ) / buttonNumStates;
      extra = w -
       ( buttonNumStates * buttonW ) -
       ( (buttonNumStates-1) * margin );
    }
    else {
      buttonW = 5;
      extra = 0;
    }

    if ( buttonW < 3 ) buttonW = 3;

    // background
    actWin->drawGc.setFG( bgColor.pixelIndex(), &blink );

    actWin->drawGc.setLineStyle( LineSolid );
    actWin->drawGc.setLineWidth( 1 );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );

    buttonX = x;
    buttonY = y;

    // buttons
    for ( i=0; i<buttonNumStates; i++ ) {

      if ( buttonSelected == i ) {

        actWin->drawGc.setFG( selColor.pixelIndex(), &blink );

        XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY,
         buttonW, buttonH );

        actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY, buttonX+buttonW, buttonY );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY, buttonX,
          buttonY+buttonH );

        actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX+buttonW, buttonY, buttonX+buttonW,
         buttonY+buttonH );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY+buttonH,
         buttonX+buttonW, buttonY+buttonH );

      }
      else {

        actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY, buttonX+buttonW, buttonY );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX+1, buttonY+1, buttonX+buttonW-1,
         buttonY+1 );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY, buttonX,
          buttonY+buttonH );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX+1, buttonY+1,
         buttonX+1, buttonY+buttonH-1 );

        actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX+buttonW, buttonY, buttonX+buttonW,
         buttonY+buttonH );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY+buttonH,
         buttonX+buttonW, buttonY+buttonH );

      }

      buttonX += buttonW + margin;

    }

    if ( fs ) {

      actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
      actWin->drawGc.setFontTag( fontTag, actWin->fi );

      buttonX = x;
      buttonY = y;

      // labels
      for ( i=0; i<buttonNumStates; i++ ) {

        xR.x = buttonX + 1;
        xR.y = buttonY + 1;
        xR.width = buttonW - 2;
        xR.height = buttonH - 2;

        actWin->drawGc.addNormXClipRectangle( xR );

        tX = buttonX + buttonW/2;
        tY = buttonY + buttonH/2 - fontAscent/2;

        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_CENTER, buttonLabel[i] );

        actWin->drawGc.removeNormXClipRectangle();

        buttonX += buttonW + margin;

      }

    }

  }
  else if ( orientation == ACBC_K_ORIENTATION_VERT ) {

    if ( buttonNumStates > 0 ) {
      buttonH = ( h - (buttonNumStates-1) * margin ) / buttonNumStates;
      extra = h -
       ( buttonNumStates * buttonH ) -
       ( (buttonNumStates-1) * margin );
    }
    else {
      buttonH = 5;
      extra = 0;
    }

    if ( buttonH < 3 ) buttonH = 3;
    buttonW = w;
    if ( buttonW < 3 ) buttonW = 3;

    lowExtra = extra / 2;
    highExtra = buttonNumStates - 1 - lowExtra - extra % 2;

    // background
    actWin->drawGc.setFG( bgColor.pixelIndex(), &blink );

    actWin->drawGc.setLineStyle( LineSolid );
    actWin->drawGc.setLineWidth( 1 );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );

    buttonX = x;
    buttonY = y;

    // buttons
    for ( i=0; i<buttonNumStates; i++ ) {

      if ( buttonSelected == i ) {

        actWin->drawGc.setFG( selColor.pixelIndex(), &blink );

        XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY,
         buttonW, buttonH );

        actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY, buttonX+buttonW, buttonY );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY, buttonX,
          buttonY+buttonH );

        actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX+buttonW, buttonY, buttonX+buttonW,
         buttonY+buttonH );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY+buttonH,
         buttonX+buttonW, buttonY+buttonH );

      }
      else {

        actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY, buttonX+buttonW, buttonY );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX+1, buttonY+1, buttonX+buttonW-1,
         buttonY+1 );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY, buttonX,
          buttonY+buttonH );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX+1, buttonY+1,
         buttonX+1, buttonY+buttonH-1 );

        actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX+buttonW, buttonY, buttonX+buttonW,
         buttonY+buttonH );

        XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
         actWin->drawGc.normGC(), buttonX, buttonY+buttonH,
         buttonX+buttonW, buttonY+buttonH );

      }

      buttonY += buttonH + margin;

      if ( i < lowExtra ) {
        buttonY++;
      }
      else if ( i >= highExtra ) {
        buttonY++;
      }

    }

    if ( fs ) {

      actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
      actWin->drawGc.setFontTag( fontTag, actWin->fi );

      buttonX = x;
      buttonY = y;

      // labels
      for ( i=0; i<buttonNumStates; i++ ) {

        xR.x = buttonX + 1;
        xR.y = buttonY + 1;
        xR.width = buttonW - 2;
        xR.height = buttonH - 2;

        actWin->drawGc.addNormXClipRectangle( xR );

        tX = buttonX + buttonW/2;
        tY = buttonY + buttonH/2 - fontAscent/2;

        if ( i < lowExtra ) {
          buttonY++;
        }
        else if ( i >= highExtra ) {
          buttonY++;
        }

        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_CENTER, buttonLabel[i] );

        actWin->drawGc.removeNormXClipRectangle();

        buttonY += buttonH + margin;

      }

    }

  }

  actWin->drawGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeChoiceButtonClass::drawActive ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };
int blink = 0;
int i, buttonX, buttonY, buttonH, buttonW, extra, lowExtra, highExtra,
 margin = 3;
int buttonNumStates;
int buttonSelected;
int inconsistent;

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
    eraseActive();
    smartDrawAllActive();
  }

  if ( !enabled || !init || !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  buttonNumStates = (int) stateStringPvId->get_enum_count();

  actWin->executeGc.saveFg();
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  inconsistent = 0;

  if ( controlExists && readExists ) {

    if ( ( value != readValue ) || !controlValid || !readValid ) {
      inconsistent = 1;
    }

    buttonSelected = readValue;

  }
  else if ( readExists ) {

    buttonSelected = readValue;

  }
  else if ( controlExists ) {

    buttonSelected = value;

  }
  else {

    inconsistent = 1;
    buttonSelected = -1;
    init = 1;

  }

  // background
  actWin->executeGc.setFG( bgColor.pixelIndex(), &blink );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( orientation == ACBC_K_ORIENTATION_HORZ ) {

    buttonH = h;
    if ( buttonH < 3 ) buttonH = 3;
    if ( buttonNumStates > 0 ) {
      buttonW = ( w - (buttonNumStates-1) * margin ) / buttonNumStates;
      extra = w -
       ( buttonNumStates * buttonW ) -
       ( (buttonNumStates-1) * margin );
    }
    else {
      buttonW = 5;
      extra = 0;
    }
    if ( buttonW < 3 ) buttonW = 3;

    lowExtra = extra / 2;
    highExtra = buttonNumStates - 1 - lowExtra - extra % 2;

    buttonX = x;
    buttonY = y;

    // buttons
    for ( i=0; i<buttonNumStates; i++ ) {

      if ( buttonSelected == i ) {

        if ( inconsistent ) {

          actWin->executeGc.setFG( inconsistentColor.getIndex(), &blink );

          XFillRectangle( actWin->d, drawable(actWin->executeWidget),
           actWin->executeGc.normGC(), buttonX, buttonY, buttonW, buttonH );

	}
	else {

          actWin->executeGc.setFG( selColor.getIndex(), &blink );

          XFillRectangle( actWin->d, drawable(actWin->executeWidget),
           actWin->executeGc.normGC(), buttonX, buttonY, buttonW, buttonH );

	}

        actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY, buttonX+buttonW, buttonY );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY, buttonX,
          buttonY+buttonH );

        actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX+buttonW, buttonY, buttonX+buttonW,
         buttonY+buttonH );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY+buttonH,
         buttonX+buttonW, buttonY+buttonH );

      }
      else {

        actWin->executeGc.setFG( bgColor.getIndex(), &blink );

        XFillRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY, buttonW, buttonH );

        actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY, buttonX+buttonW,
         buttonY );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX+1, buttonY+1, buttonX+buttonW-1,
         buttonY+1 );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY, buttonX,
          buttonY+buttonH );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX+1, buttonY+1,
         buttonX+1, buttonY+buttonH-1 );

        actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX+buttonW, buttonY, buttonX+buttonW,
         buttonY+buttonH );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY+buttonH,
         buttonX+buttonW, buttonY+buttonH );

      }

      buttonX += buttonW + margin;

      if ( i < lowExtra ) {
        buttonX++;
      }
      else if ( i >= highExtra ) {
        buttonX++;
      }

    }

    if ( fs ) {

      actWin->executeGc.setFontTag( fontTag, actWin->fi );

      buttonX = x;
      buttonY = y;

      // labels
      for ( i=0; i<buttonNumStates; i++ ) {

        if ( buttonSelected == i ) {
          actWin->executeGc.setFG( fgColor.getIndex(), &blink );
	}
	else {
          actWin->executeGc.setFG( fgColor.pixelIndex(), &blink );
	}

        xR.x = buttonX + 1;
        xR.y = buttonY + 1;
        xR.width = buttonW - 2;
        xR.height = buttonH - 2;

        actWin->executeGc.addNormXClipRectangle( xR );

        tX = buttonX + buttonW/2;
        tY = buttonY + buttonH/2 - fontAscent/2;

        if ( i < lowExtra ) {
          buttonX++;
        }
        else if ( i >= highExtra ) {
          buttonX++;
        }

        drawText( actWin->executeWidget, drawable(actWin->executeWidget),
         &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_CENTER, (char *) stateStringPvId->get_enum( i ) );

        actWin->executeGc.removeNormXClipRectangle();

        buttonX += buttonW + margin;

      }

    }

  }
  else if ( orientation == ACBC_K_ORIENTATION_VERT ) {

    if ( buttonNumStates > 0 ) {
      buttonH = ( h - (buttonNumStates-1) * margin ) / buttonNumStates;
      extra = h -
       ( buttonNumStates * buttonH ) -
       ( (buttonNumStates-1) * margin );
    }
    else {
      buttonH = 5;
      extra = 0;
    }

    if ( buttonH < 3 ) buttonH = 3;
    buttonW = w;
    if ( buttonW < 3 ) buttonW = 3;

    lowExtra = extra / 2;
    highExtra = buttonNumStates - 1 - lowExtra - extra % 2;

    buttonX = x;
    buttonY = y;

    // buttons
    for ( i=0; i<buttonNumStates; i++ ) {

      if ( buttonSelected == i ) {

        if ( inconsistent ) {

          actWin->executeGc.setFG( inconsistentColor.getIndex(), &blink );

          XFillRectangle( actWin->d, drawable(actWin->executeWidget),
           actWin->executeGc.normGC(), buttonX, buttonY, buttonW, buttonH );

	}
	else {

          actWin->executeGc.setFG( selColor.getIndex(), &blink );

          XFillRectangle( actWin->d, drawable(actWin->executeWidget),
           actWin->executeGc.normGC(), buttonX, buttonY, buttonW, buttonH );

	}

        actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY, buttonX+buttonW,
         buttonY );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY, buttonX,
          buttonY+buttonH );

        actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX+buttonW, buttonY, buttonX+buttonW,
         buttonY+buttonH );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY+buttonH,
         buttonX+buttonW, buttonY+buttonH );

      }
      else {

        actWin->executeGc.setFG( bgColor.getIndex(), &blink );

        XFillRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY, buttonW, buttonH );

        actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY, buttonX+buttonW,
         buttonY );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX+1, buttonY+1, buttonX+buttonW-1,
         buttonY+1 );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY, buttonX,
          buttonY+buttonH );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX+1, buttonY+1,
         buttonX+1, buttonY+buttonH-1 );

        actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX+buttonW, buttonY, buttonX+buttonW,
         buttonY+buttonH );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), buttonX, buttonY+buttonH,
         buttonX+buttonW, buttonY+buttonH );

      }

      buttonY += buttonH + margin;

      if ( i < lowExtra ) {
        buttonY++;
      }
      else if ( i >= highExtra ) {
        buttonY++;
      }

    }

    if ( fs ) {

      actWin->executeGc.setFontTag( fontTag, actWin->fi );

      buttonX = x;
      buttonY = y;

      // labels
      for ( i=0; i<buttonNumStates; i++ ) {

        if ( buttonSelected == i ) {
          actWin->executeGc.setFG( fgColor.getIndex(), &blink );
	}
	else {
          actWin->executeGc.setFG( fgColor.pixelIndex(), &blink );
	}

        xR.x = buttonX + 1;
        xR.y = buttonY + 1;
        xR.width = buttonW - 2;
        xR.height = buttonH - 2;

        actWin->executeGc.addNormXClipRectangle( xR );

        tX = buttonX + buttonW/2;
        tY = buttonY + buttonH/2 - fontAscent/2;

        if ( i < lowExtra ) {
          buttonY++;
        }
        else if ( i >= highExtra ) {
          buttonY++;
        }

        drawText( actWin->executeWidget, drawable(actWin->executeWidget),
         &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_CENTER, (char *) stateStringPvId->get_enum( i ) );

        actWin->executeGc.removeNormXClipRectangle();

        buttonY += buttonH + margin;

      }

    }

  }

  actWin->executeGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeChoiceButtonClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( controlPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  controlPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( readPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( visPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  visPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( colorPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  colorPvExpStr.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeChoiceButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = controlPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeChoiceButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = controlPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = colorPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeChoiceButtonClass::containsMacros ( void ) {

  if ( controlPvExpStr.containsPrimaryMacros() ) return 1;

  if ( readPvExpStr.containsPrimaryMacros() ) return 1;

  if ( visPvExpStr.containsPrimaryMacros() ) return 1;

  if ( colorPvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeChoiceButtonClass::activate (
  int pass,
  void *ptr
) {

int opStat;

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      aglPtr = ptr;
      needConnectInit = needInfoInit = needReadConnectInit = needReadInfoInit =
       needRefresh = needDraw = needVisConnectInit = needVisInit =
       needVisUpdate = needColorConnectInit =
       needColorInit = needColorUpdate = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      controlValid = readValid = 0;
      controlPvId = readPvId = visPvId = colorPvId = NULL;

      controlExists = readExists = visExists = colorExists = 0;

      pvCheckExists = 0;
      connection.init();
      initialConnection = initialReadConnection = initialVisConnection =
       initialColorConnection = 1;

      initEnable();

      oldStat = -1;
      oldSev = -1;

      init = 0;
      active = 0;
      activeMode = 1;

      buttonPressed = 0;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      if ( !pvCheckExists ) {

        pvCheckExists = 1;

        // if ( strcmp( controlPvExpStr.getExpanded(), "" ) != 0 ) {
	if ( !blankOrComment( controlPvExpStr.getExpanded() ) ) {
          controlExists = 1;
          connection.addPv(); // must do this only once per pv
	}
        else {
          controlExists = 0;
	}

        // if ( strcmp( readPvExpStr.getExpanded(), "" ) != 0 ) {
	if ( !blankOrComment( readPvExpStr.getExpanded() ) ) {
          readExists = 1;
          connection.addPv(); // must do this only once per pv
	}
        else {
          readExists = 0;
	}

        // if ( strcmp( visPvExpStr.getExpanded(), "" ) != 0 ) {
	if ( !blankOrComment( visPvExpStr.getExpanded() ) ) {
          visExists = 1;
          connection.addPv(); // must do this only once per pv
        }
        else {
          visExists = 0;
          visibility = 1;
        }

        // if ( strcmp( colorPvExpStr.getExpanded(), "" ) != 0 ) {
	if ( !blankOrComment( colorPvExpStr.getExpanded() ) ) {
          colorExists = 1;
          connection.addPv(); // must do this only once per pv
        }
        else {
          colorExists = 0;
        }
      }

      opStat = 1;

      if ( controlExists ) {

	controlPvId = the_PV_Factory->create( controlPvExpStr.getExpanded() );
        if ( controlPvId ) {
	  controlPvId->add_conn_state_callback(
           acb_monitor_control_connect_state, this );
          stateStringPvId = controlPvId;
	}
	else {
          fprintf( stderr, activeChoiceButtonClass_str20,
           controlPvExpStr.getExpanded() );
          opStat = 0;
        }

      }

      if ( readExists ) {

	readPvId = the_PV_Factory->create( readPvExpStr.getExpanded() );
        if ( readPvId ) {
	  readPvId->add_conn_state_callback(
           acb_monitor_read_connect_state, this );
          if ( !controlExists ) stateStringPvId = readPvId;
	}
	else {
          fprintf( stderr, activeChoiceButtonClass_str21,
           readPvExpStr.getExpanded() );
          opStat = 0;
        }

      }

      if ( visExists ) {

	visPvId = the_PV_Factory->create( visPvExpStr.getExpanded() );
        if ( visPvId ) {
	  visPvId->add_conn_state_callback(
           acb_monitor_vis_connect_state, this );
	}
	else {
          fprintf( stderr, activeChoiceButtonClass_str21,
           visPvExpStr.getExpanded() );
          opStat = 0;
        }

      }

      if ( colorExists ) {

	colorPvId = the_PV_Factory->create( colorPvExpStr.getExpanded() );
        if ( colorPvId ) {
	  colorPvId->add_conn_state_callback(
           acb_monitor_color_connect_state, this );
	}
	else {
          fprintf( stderr, activeChoiceButtonClass_str21,
           colorPvExpStr.getExpanded() );
          opStat = 0;
        }

      }

      opComplete = opStat;

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

int activeChoiceButtonClass::deactivate (
  int pass
) {

  active = 0;
  activeMode = 0;

  switch ( pass ) {

  case 1:

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    //updateBlink( 0 );

    if ( controlExists ) {
      if ( controlPvId ) {
        controlPvId->remove_conn_state_callback(
         acb_monitor_control_connect_state, this );
        controlPvId->remove_value_callback(
         acb_controlUpdate, this );
	controlPvId->release();
        controlPvId = NULL;
      }
    }

    if ( readExists ) {
      if ( readPvId ) {
        readPvId->remove_conn_state_callback(
         acb_monitor_read_connect_state, this );
        readPvId->remove_value_callback(
         acb_readUpdate, this );
	readPvId->release();
        readPvId = NULL;
      }
    }

    if ( visExists ) {
      if ( visPvId ) {
        visPvId->remove_conn_state_callback(
         acb_monitor_vis_connect_state, this );
        visPvId->remove_value_callback(
         acb_visUpdate, this );
	visPvId->release();
        visPvId = NULL;
      }
    }

    if ( colorExists ) {
      if ( colorPvId ) {
        colorPvId->remove_conn_state_callback(
         acb_monitor_color_connect_state, this );
        colorPvId->remove_value_callback(
         acb_colorUpdate, this );
	colorPvId->release();
        colorPvId = NULL;
      }
    }

    break;

  case 2:

    break;

  }

  return 1;

}

void activeChoiceButtonClass::updateDimensions ( void )
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

void activeChoiceButtonClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

}

void activeChoiceButtonClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

short value;
int stat, i, state, buttonX, buttonY, buttonH, buttonW,
 extra, lowExtra, highExtra,margin = 3, numStates;

  *action = 0;

  if ( !enabled || !init || !visibility ) return;

  if ( !controlExists ) return;

  if ( !controlPvId->have_write_access() ) return;

  numStates = (int) stateStringPvId->get_enum_count();

  if ( buttonNumber == 1 ) {

    state = -1;

    if ( orientation == ACBC_K_ORIENTATION_HORZ ) {

      buttonH = h;
      if ( buttonH < 3 ) buttonH = 3;
      if ( numStates > 0 ) {
        buttonW = ( w - (numStates-1) * margin ) / numStates;
        extra = w -
         ( numStates * buttonW ) -
         ( (numStates-1) * margin );
      }
      else {
        buttonW = 5;
        extra = 0;
      }
      if ( buttonW < 3 ) buttonW = 3;

      lowExtra = extra / 2;
      highExtra = numStates - 1 - lowExtra - extra % 2;

      buttonX = x;
      buttonY = y;

      // buttons
      for ( i=0; i<numStates; i++ ) {

        buttonX += buttonW + margin;

        if ( i < lowExtra ) {
          buttonX++;
        }
        else if ( i >= highExtra ) {
          buttonX++;
        }

        if ( ( buttonX - margin ) > be->x ) {
          state = i;
          break;
	}

      }

    }
    else if ( orientation == ACBC_K_ORIENTATION_VERT ) {

      if ( numStates > 0 ) {
        buttonH = ( h - (numStates-1) * margin ) / numStates;
        extra = h -
         ( numStates * buttonH ) -
         ( (numStates-1) * margin );
      }
      else {
        buttonH = 5;
        extra = 0;
      }
      if ( buttonH < 3 ) buttonH = 3;
      buttonW = w;
      if ( buttonW < 3 ) buttonW = 3;

      lowExtra = extra / 2;
      highExtra = numStates - 1 - lowExtra - extra % 2;

      buttonX = x;
      buttonY = y;

      // buttons
      for ( i=0; i<numStates; i++ ) {

        buttonY += buttonH + margin;

        if ( i < lowExtra ) {
          buttonY++;
        }
        else if ( i >= highExtra ) {
          buttonY++;
        }

        if ( ( buttonY - margin ) > be->y ) {
          state = i;
          break;
	}

      }

    }

    if ( ( state >= 0 ) && ( state < numStates ) ) {
      value = (short) state;
      stat = controlPvId->put(
       XDisplayName(actWin->appCtx->displayName),
       value );
    }

  }

}

void activeChoiceButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled || !init || !visibility ) return;

  if ( !controlPvId->have_write_access() ) {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );
  }
  else {
    actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_DEFAULT );
  }

  activeGraphicClass::pointerIn( _x, _y, buttonState );

}

int activeChoiceButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;

  if ( controlExists )
    *focus = 1;
  else
    *focus = 0;

  if ( !controlExists ) {
    *up = 0;
    *down = 0;
    return 1;
  }

  *down = 1;
  *up = 1;

  return 1;

}

void activeChoiceButtonClass::executeDeferred ( void ) {

short v, rV;
int nc, nrc, ni, nri, nr, nd, nvc, nvi, nvu, ncolc, ncoli, ncolu;
int stat, index, invisColor;
char msg[79+1];

  if ( actWin->isIconified ) return;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nrc = needReadConnectInit; needReadConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nri = needReadInfoInit; needReadInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  nd = needDraw; needDraw = 0;
  nvc = needVisConnectInit; needVisConnectInit = 0;
  nvi = needVisInit; needVisInit = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  ncolc = needColorConnectInit; needColorConnectInit = 0;
  ncoli = needColorInit; needColorInit = 0;
  ncolu = needColorUpdate; needColorUpdate = 0;
  v = curValue;
  rV = curReadValue;
  visValue = curVisValue;
  colorValue = curColorValue;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    if ( controlPvId->get_type().type != ProcessVariable::Type::enumerated ) {
      strncpy( msg, actWin->obj.getNameFromClass( "activeChoiceButtonClass" ),
       79 );
      Strncat( msg, activeChoiceButtonClass_str38, 79 );
      actWin->appCtx->postMessage( msg );
      init = 0;
      needToDrawUnconnected = 1;
      drawActive();
      return;
    }

    v = curValue = controlPvId->get_int();

    ni = 1;

  }

  if ( nrc ) {

    rV = curReadValue = readPvId->get_int();

    nri = 1;

  }

  if ( ni ) {

    value = v;

    if ( initialConnection ) {

      initialConnection = 0;
      
      controlPvId->add_value_callback( acb_controlUpdate, this );

    }

    if ( connection.pvsConnected() ) {
      init = 1;
      active = 1;
      drawActive();
    }

  }

  if ( nri ) {

    curReadValue = rV;

    if ( initialReadConnection ) {

      initialReadConnection = 0;
      
      readPvId->add_value_callback( acb_readUpdate, this );

    }

    if ( connection.pvsConnected() ) {
      init = 1;
      active = 1;
      drawActive();
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
      
      visPvId->add_value_callback( acb_visUpdate, this );

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
      fgColor.setConnected();
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
      
      colorPvId->add_value_callback( acb_colorUpdate, this );

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
      active = 1;
      init = 1;
      fgColor.setConnected();
      smartDrawAllActive();
    }

  }

//----------------------------------------------------------------------------

  if ( nr ) {
    readValue = rV;
    value = v;
    eraseActive();
    smartDrawAllActive();
  }

//----------------------------------------------------------------------------

  if ( nd ) {
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

char *activeChoiceButtonClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeChoiceButtonClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeChoiceButtonClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( i == 0 ) {
    return controlPvExpStr.getExpanded();
  }
  else if ( i == 1 ) {
    return readPvExpStr.getExpanded();
  }
  else if ( i == 2 ) {
    return colorPvExpStr.getExpanded();
  }
  else {
    return visPvExpStr.getExpanded();
  }

}

void activeChoiceButtonClass::changeDisplayParams (
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

    strcpy( fontTag, _btnFontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    actWin->fi->getTextFontList( fontTag, &fontList );

    updateDimensions();

  }

}

void activeChoiceButtonClass::changePvNames (
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
      controlPvExpStr.setRaw( ctlPvs[0] );
    }
  }

  if ( flag & ACTGRF_VISPVS_MASK ) {
    if ( numVisPvs ) {
      visPvExpStr.setRaw( ctlPvs[0] );
    }
  }

}

void activeChoiceButtonClass::getPvs (
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
  pvs[2] = visPvId;
  pvs[3] = colorPvId;

}

char *activeChoiceButtonClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return controlPvExpStr.getRaw();
  }
  else if ( i == 1 ) {
    return readPvExpStr.getRaw();
  }
  else if ( i == 2 ) {
    return colorPvExpStr.getRaw();
  }
  else if ( i == 3 ) {
    return visPvExpStr.getRaw();
  }
  else if ( i == 4 ) {
    return minVisString;
  }
  else if ( i == 5 ) {
    return maxVisString;
  }

  return NULL;

}

void activeChoiceButtonClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    controlPvExpStr.setRaw( string );
  }
  else if ( i == 1 ) {
    readPvExpStr.setRaw( string );
  }
  else if ( i == 2 ) {
    colorPvExpStr.setRaw( string );
  }
  else if ( i == 3 ) {
    visPvExpStr.setRaw( string );
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

}

// crawler functions may return blank pv names
char *activeChoiceButtonClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return controlPvExpStr.getExpanded();

}

char *activeChoiceButtonClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >= 3 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {

    return readPvExpStr.getExpanded();

  }
  else if ( crawlerPvIndex == 2 ) {

    return visPvExpStr.getExpanded();

  }

  return colorPvExpStr.getExpanded();

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeChoiceButtonClassPtr ( void ) {

activeChoiceButtonClass *ptr;

  ptr = new activeChoiceButtonClass;
  return (void *) ptr;

}

void *clone_activeChoiceButtonClassPtr (
  void *_srcPtr )
{

activeChoiceButtonClass *ptr, *srcPtr;

  srcPtr = (activeChoiceButtonClass *) _srcPtr;

  ptr = new activeChoiceButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
