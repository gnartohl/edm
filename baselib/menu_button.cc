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

#define __menu_button_cc 1

#include "menu_button.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) {

activeMenuButtonClass *ambo = (activeMenuButtonClass *) ptr;

  if ( !ambo->activeMode ) {
    if ( ambo->isSelected() ) ambo->drawSelectBoxCorners(); // erase via xor
    ambo->smartDrawAll();
    if ( ambo->isSelected() ) ambo->drawSelectBoxCorners();
  }
  else {
    ambo->bufInvalidate();
    ambo->needDraw = 1;
    ambo->actWin->addDefExeNode( ambo->aglPtr );
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeMenuButtonClass *ambo = (activeMenuButtonClass *) client;

  if ( !ambo->connection.pvsConnected() ) {
    ambo->needToDrawUnconnected = 1;
    ambo->needDraw = 1;
    ambo->actWin->addDefExeNode( ambo->aglPtr );
  }

  ambo->unconnectedTimer = 0;

}

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;
int i;
short value;

  if ( mbto->controlPvId ) {
    if ( !mbto->controlPvId->have_write_access() ) return;
  }

  if ( mbto->stateStringPvId ) {

    for ( i=0; i<(int)mbto->stateStringPvId->get_enum_count(); i++ ) {

      if ( w == mbto->pb[i] ) {
        value = (short) i;
        mbto->controlPvId->put(
         XDisplayName(mbto->actWin->appCtx->displayName),
         value );
        break;
      }

    }

  }

}

static void mbt_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) userarg;

  if ( pv->is_valid() ) {

    mbto->connection.setPvConnected( (void *) mbto->controlPvConnection );
    mbto->needConnectInit = 1;

    if ( mbto->connection.pvsConnected() ) {
      mbto->fgColor.setConnected();
    }

  }
  else {

    mbto->connection.setPvDisconnected( (void *) mbto->controlPvConnection );
    mbto->fgColor.setDisconnected();
    mbto->controlValid = 0;
    mbto->needDraw = 1;
    mbto->active = 0;

  }

  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) userarg;
int st, sev;

  mbto->curValue = (short) pv->get_int();

  st = pv->get_status();
  sev = pv->get_severity();
  if ( ( st != mbto->oldStat ) || ( sev != mbto->oldSev ) ) {
    mbto->oldStat = st;
    mbto->oldSev = sev;
    mbto->fgColor.setStatus( st, sev );
    mbto->bufInvalidate();
  }

  mbto->controlValid = 1;
  mbto->needRefresh = 1;
  mbto->needDraw = 1;
  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) userarg;

  if ( pv->is_valid() ) {

    mbto->connection.setPvConnected( (void *) mbto->readPvConnection );
    mbto->needReadConnectInit = 1;

    if ( mbto->connection.pvsConnected() ) {
      mbto->fgColor.setConnected();
    }

  }
  else {

    mbto->connection.setPvDisconnected( (void *) mbto->readPvConnection );
    mbto->fgColor.setDisconnected();
    mbto->readValid = 0;
    mbto->needDraw = 1;
    mbto->active = 0;

  }

  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) userarg;
int st, sev;

  mbto->curReadValue = (short) pv->get_int();

  if ( !mbto->controlExists ) {
    st = pv->get_status();
    sev = pv->get_severity();
    if ( ( st != mbto->oldStat ) || ( sev != mbto->oldSev ) ) {
      mbto->oldStat = st;
      mbto->oldSev = sev;
      mbto->fgColor.setStatus( st, sev );
      mbto->bufInvalidate();
    }
  }

  mbto->readValid = 1;
  mbto->needRefresh = 1;
  mbto->needDraw = 1;
  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) userarg;

  if ( pv->is_valid() ) {

    mbto->needVisConnectInit = 1;

  }
  else {

    mbto->connection.setPvDisconnected( (void *) mbto->visPvConnection );
    mbto->fgColor.setDisconnected();
    mbto->active = 0;
    mbto->needDraw = 1;

  }

  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_visUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) userarg;

  mbto->curVisValue = pv->get_double();

  mbto->actWin->appCtx->proc->lock();
  mbto->needVisUpdate = 1;
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) userarg;

  if ( pv->is_valid() ) {

    mbto->needColorConnectInit = 1;

  }
  else {

    mbto->connection.setPvDisconnected( (void *) mbto->colorPvConnection );
    mbto->fgColor.setDisconnected();
    mbto->active = 0;
    mbto->needDraw = 1;

  }

  mbto->actWin->appCtx->proc->lock();
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbt_colorUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) userarg;

  mbto->curColorValue = pv->get_double();

  mbto->actWin->appCtx->proc->lock();
  mbto->needColorUpdate = 1;
  mbto->actWin->addDefExeNode( mbto->aglPtr );
  mbto->actWin->appCtx->proc->unlock();

}

static void mbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;

  mbto->actWin->setChanged();

  mbto->eraseSelectBoxCorners();
  mbto->erase();

  strncpy( mbto->fontTag, mbto->fm.currentFontTag(), 63 );
  mbto->actWin->fi->loadFontTag( mbto->fontTag );
  mbto->actWin->drawGc.setFontTag( mbto->fontTag, mbto->actWin->fi );
  mbto->actWin->fi->getTextFontList( mbto->fontTag, &mbto->fontList );
  mbto->fs = mbto->actWin->fi->getXFontStruct( mbto->fontTag );

  mbto->topShadowColor = mbto->bufTopShadowColor;
  mbto->botShadowColor = mbto->bufBotShadowColor;

  mbto->fgColorMode = mbto->bufFgColorMode;
  if ( mbto->fgColorMode == MBTC_K_COLORMODE_ALARM )
    mbto->fgColor.setAlarmSensitive();
  else
    mbto->fgColor.setAlarmInsensitive();
  mbto->fgColor.setColorIndex( mbto->bufFgColor, mbto->actWin->ci );

  mbto->bgColorMode = mbto->bufBgColorMode;
  if ( mbto->bgColorMode == MBTC_K_COLORMODE_ALARM )
    mbto->bgColor.setAlarmSensitive();
  else
    mbto->bgColor.setAlarmInsensitive();
  mbto->bgColor.setColorIndex( mbto->bufBgColor, mbto->actWin->ci );

  mbto->inconsistentColor.setColorIndex( mbto->bufInconsistentColor,
   mbto->actWin->ci );

  mbto->visPvExpStr.setRaw( mbto->eBuf->bufVisPvName );
  strncpy( mbto->minVisString, mbto->bufMinVisString, 39 );
  strncpy( mbto->maxVisString, mbto->bufMaxVisString, 39 );

  if ( mbto->bufVisInverted )
    mbto->visInverted = 0;
  else
    mbto->visInverted = 1;

  mbto->colorPvExpStr.setRaw( mbto->eBuf->bufColorPvName );

  mbto->x = mbto->bufX;
  mbto->sboxX = mbto->bufX;

  mbto->y = mbto->bufY;
  mbto->sboxY = mbto->bufY;

  mbto->w = mbto->bufW;
  mbto->sboxW = mbto->bufW;

  mbto->h = mbto->bufH;
  mbto->sboxH = mbto->bufH;

  mbto->controlPvExpStr.setRaw( mbto->eBuf->bufControlPvName );

  mbto->readPvExpStr.setRaw( mbto->eBuf->bufReadPvName );

  mbto->updateDimensions();

}

static void mbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;

  mbtc_edit_update ( w, client, call );
  mbto->refresh( mbto );

}

static void mbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;

  mbtc_edit_update ( w, client, call );
  mbto->ef.popdown();
  mbto->operationComplete();

}

static void mbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;

  mbto->ef.popdown();
  mbto->operationCancel();

}

static void mbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMenuButtonClass *mbto = (activeMenuButtonClass *) client;

  mbto->ef.popdown();
  mbto->operationCancel();
  mbto->erase();
  mbto->deleteRequest = 1;
  mbto->drawAll();

}

activeMenuButtonClass::activeMenuButtonClass ( void ) {
int i;

  name = new char[strlen("activeMenuButtonClass")+1];
  strcpy( name, "activeMenuButtonClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    pb[i] = NULL;
  }

  fgColorMode = MBTC_K_COLORMODE_STATIC;
  bgColorMode = MBTC_K_COLORMODE_STATIC;

  active = 0;
  activeMode = 0;
  widgetsCreated = 0;

  fontList = NULL;

  connection.setMaxPvs( 4 );

  unconnectedTimer = 0;

  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );

  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

}

activeMenuButtonClass::~activeMenuButtonClass ( void ) {

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
activeMenuButtonClass::activeMenuButtonClass
 ( const activeMenuButtonClass *source ) {

activeGraphicClass *mbto = (activeGraphicClass *) this;
int i;

  mbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMenuButtonClass")+1];
  strcpy( name, "activeMenuButtonClass" );

  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    pb[i] = NULL;
  }

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
  inconsistentColor = source->inconsistentColor;
  fgCb = source->fgCb;
  bgCb = source->bgCb;
  inconsistentCb = source->inconsistentCb;

  fgColorMode = source->fgColorMode;
  bgColorMode = source->bgColorMode;

  controlPvExpStr.copy( source->controlPvExpStr );
  readPvExpStr.copy( source->readPvExpStr );
  visPvExpStr.copy( source->visPvExpStr );
  colorPvExpStr.copy( source->colorPvExpStr );

  widgetsCreated = 0;
  active = 0;
  activeMode = 0;

  connection.setMaxPvs( 4 );

  unconnectedTimer = 0;

  visibility = 0;
  prevVisibility = -1;
  visInverted = source->visInverted;
  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

  doAccSubs( controlPvExpStr );
  doAccSubs( readPvExpStr );
  doAccSubs( colorPvExpStr );
  doAccSubs( visPvExpStr );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );

}

int activeMenuButtonClass::createInteractive (
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
  inconsistentColor.setColorIndex( actWin->defaultOffsetColor,
   actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int activeMenuButtonClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
char *emptyStr = "";

  major = MBTC_MAJOR_VERSION;
  minor = MBTC_MINOR_VERSION;
  release = MBTC_RELEASE;

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
  tag.loadW( "colorPv", &colorPvExpStr, emptyStr  );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeMenuButtonClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", MBTC_MAJOR_VERSION, MBTC_MINOR_VERSION,
   MBTC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fgColorMode );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", bgColorMode );

  index = topShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = botShadowColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  if ( controlPvExpStr.getRaw() )
    writeStringToFile( f, controlPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  if ( readPvExpStr.getRaw() )
    writeStringToFile( f, readPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 2.1.0
  index = inconsistentColor.pixelIndex();
  fprintf( f, "%-d\n", index );

  //ver 2.3.0
  if ( visPvExpStr.getRaw() )
    writeStringToFile( f, visPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );
  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  //ver 2.4.0
  if ( colorPvExpStr.getRaw() )
    writeStringToFile( f, colorPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  return 1;

}

int activeMenuButtonClass::createFromFile (
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
  tag.loadR( "fgAlarm", &fgColorMode, &zero );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "bgAlarm", &bgColorMode, &zero );
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
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > MBTC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( fgColorMode == MBTC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  if ( bgColorMode == MBTC_K_COLORMODE_ALARM )
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

int activeMenuButtonClass::old_createFromFile (
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

  if ( major > MBTC_MAJOR_VERSION ) {
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

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == MBTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == MBTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

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

    if ( fgColorMode == MBTC_K_COLORMODE_ALARM )
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
    bgColor.setColorIndex( index, actWin->ci );

  }

  fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

  if ( bgColorMode == MBTC_K_COLORMODE_ALARM )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  if ( ( major > 1 ) || ( minor > 0 ) ) {

    if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {

      actWin->ci->readColorIndex( f, &index );
      actWin->incLine(); actWin->incLine();
      topShadowColor = index;

      actWin->ci->readColorIndex( f, &index );
      actWin->incLine(); actWin->incLine();
      botShadowColor = index;

    }
    else if ( major > 1 ) {

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
      topShadowColor = actWin->ci->pixIndex( pixel );

      fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
      if ( ( major < 2 ) && ( minor < 2 ) ) {
        r *= 256;
        g *= 256;
        b *= 256;
      }
      actWin->ci->setRGB( r, g, b, &pixel );
      botShadowColor = actWin->ci->pixIndex( pixel );

    }

  }
  else {

    topShadowColor = actWin->ci->pixIndex( WhitePixel( actWin->display(),
     DefaultScreen(actWin->display()) ) );

    botShadowColor = actWin->ci->pixIndex( BlackPixel( actWin->display(),
     DefaultScreen(actWin->display()) ) );

  }

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    readPvExpStr.setRaw( oneName );
  }
  else {
    readPvExpStr.setRaw( "" );
  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {
    fscanf( f, "%d\n", &index ); actWin->incLine();
    inconsistentColor.setColorIndex( index, actWin->ci );
  }
  else {
    inconsistentColor.setColorIndex( bgColor.pixelIndex(), actWin->ci );
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 2 ) ) ) {

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    visPvExpStr.setRaw( oneName );

    fscanf( f, "%d\n", &visInverted ); actWin->incLine();

    readStringFromFile( minVisString, 39+1, f ); actWin->incLine();

    readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();

  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 3 ) ) ) {

    readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    colorPvExpStr.setRaw( oneName );

  }

  updateDimensions();

  return 1;

}

int activeMenuButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeMenuButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMenuButtonClass_str2, 31 );

  Strncat( title, activeMenuButtonClass_str3, 31 );

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

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeMenuButtonClass_str4, 35, &bufX );
  ef.addTextField( activeMenuButtonClass_str5, 35, &bufY );
  ef.addTextField( activeMenuButtonClass_str6, 35, &bufW );
  ef.addTextField( activeMenuButtonClass_str7, 35, &bufH );
  ef.addTextField( activeMenuButtonClass_str17, 35, eBuf->bufControlPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeMenuButtonClass_str18, 35, eBuf->bufReadPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addColorButton( activeMenuButtonClass_str8, actWin->ci, &fgCb,
   &bufFgColor );
  ef.addToggle( activeMenuButtonClass_str10, &bufFgColorMode );

  ef.addColorButton( activeMenuButtonClass_str11, actWin->ci, &bgCb,
   &bufBgColor );

  ef.addColorButton( activeMenuButtonClass_str29, actWin->ci,
   &inconsistentCb, &bufInconsistentColor );

  ef.addColorButton( activeMenuButtonClass_str14, actWin->ci, &topShadowCb,
   &bufTopShadowColor );

  ef.addColorButton( activeMenuButtonClass_str15, actWin->ci, &botShadowCb,
   &bufBotShadowColor );

  ef.addFontMenu( activeMenuButtonClass_str16, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  ef.addTextField( activeMenuButtonClass_str34, 35, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeMenuButtonClass_str30, 35, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeMenuButtonClass_str31, &bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeMenuButtonClass_str32, 35, bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeMenuButtonClass_str33, 35, bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  return 1;

}

int activeMenuButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( mbtc_edit_ok, mbtc_edit_apply, mbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMenuButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( mbtc_edit_ok, mbtc_edit_apply, mbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMenuButtonClass::erase ( void ) {

  if ( deleteRequest || activeMode ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMenuButtonClass::eraseActive ( void ) {

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

int activeMenuButtonClass::draw ( void ) {

int tX, tY, bumpX, bumpY;
XRectangle xR = { x+3, y, w-23, h };
int blink = 0;

  if ( deleteRequest || activeMode ) return 1;

  actWin->drawGc.saveFg();

  //actWin->drawGc.setFG( bgColor.pixelColor() );
  actWin->drawGc.setFG( bgColor.pixelIndex(), &blink );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

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

  // draw bump

  bumpX = x+w-10-10;
  bumpY = y+h/2-5;

  actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), bumpX, bumpY+10, bumpX, bumpY );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), bumpX, bumpY, bumpX+10, bumpY );

  actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), bumpX+10, bumpY, bumpX+10, bumpY+10 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), bumpX+10, bumpY+10, bumpX, bumpY+10 );

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    //actWin->drawGc.setFG( fgColor.pixelColor() );
    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2 - 10;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
     XmALIGNMENT_CENTER, "Menu" );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeMenuButtonClass::drawActive ( void ) {

int tX, tY, bumpX, bumpY;
short v;
XRectangle xR = { x+3, y, w-23, h };
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
    eraseActive();
    smartDrawAllActive();
  }

  if ( !enabled || !init || !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  actWin->executeGc.saveFg();
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  if ( controlExists && readExists ) {

    if ( ( value != readValue ) || !controlValid || !readValid ) {
      //actWin->executeGc.setFG( inconsistentColor.getColor() );
      actWin->executeGc.setFG( inconsistentColor.getIndex(), &blink );
    }
    else {
      //actWin->executeGc.setFG( bgColor.getColor() );
      actWin->executeGc.setFG( bgColor.getIndex(), &blink );
    }

    v = readValue;

  }
  else if ( readExists ) {

    //actWin->executeGc.setFG( bgColor.getColor() );
    actWin->executeGc.setFG( bgColor.getIndex(), &blink );

    v = readValue;

  }
  else if ( controlExists ) {

    //actWin->executeGc.setFG( bgColor.getColor() );
    actWin->executeGc.setFG( bgColor.getIndex(), &blink );

    v = value;

  }
  else {

    //actWin->executeGc.setFG( inconsistentColor.getColor() );
    actWin->executeGc.setFG( inconsistentColor.getIndex(), &blink );
    v = -1;
    init = 1;

  }

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

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

  // draw bump

  bumpX = x+w-10-10;
  bumpY = y+h/2-5;

  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), bumpX, bumpY+10, bumpX, bumpY );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), bumpX, bumpY, bumpX+10, bumpY );

  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), bumpX+10, bumpY, bumpX+10, bumpY+10 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), bumpX+10, bumpY+10, bumpX, bumpY+10 );

  if ( fs ) {

    actWin->executeGc.addNormXClipRectangle( xR );

    //actWin->executeGc.setFG( fgColor.getColor() );
    actWin->executeGc.setFG( fgColor.getIndex(), &blink );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2 - 10;
    tY = y + h/2 - fontAscent/2;

    if ( stateStringPvId ) {

      if ( ( v >= 0 ) && ( v < (short) stateStringPvId->get_enum_count() ) ) {
        drawText( actWin->executeWidget, drawable(actWin->executeWidget),
         &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_CENTER, (char *) stateStringPvId->get_enum( v ) );
      }
      else {
        drawText( actWin->executeWidget, drawable(actWin->executeWidget),
         &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_CENTER, "?" );
      }

    }
    else {

      drawText( actWin->executeWidget, drawable(actWin->executeWidget),
       &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_CENTER, "?" );

    }

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int activeMenuButtonClass::expandTemplate (
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

int activeMenuButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;;

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

int activeMenuButtonClass::expand2nd (
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

int activeMenuButtonClass::containsMacros ( void ) {

  if ( controlPvExpStr.containsPrimaryMacros() ) return 1;

  if ( readPvExpStr.containsPrimaryMacros() ) return 1;

  if ( visPvExpStr.containsPrimaryMacros() ) return 1;

  if ( colorPvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeMenuButtonClass::activate (
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
      controlPvId = readPvId = stateStringPvId = visPvId = colorPvId = NULL;
      usePvId = 0;
      controlExists = readExists = visExists = colorExists = 0;
      pvCheckExists = 0;
      connection.init();
      initialConnection = initialReadConnection = initialVisConnection =
       initialColorConnection = 1;
      value = readValue = 0;
      controlValid = readValid = 0;
      enumCount = 0;

      initEnable();

      oldStat = -1;
      oldSev = -1;

      init = 0;
      active = 0;
      activeMode = 1;

      buttonPressed = 0;

      popUpMenu = (Widget) NULL;

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
           mbt_monitor_control_connect_state, this );
          usePvId = activeMenuButtonClass::useControlPvId;
	}
	else {
          fprintf( stderr, activeMenuButtonClass_str20,
           controlPvExpStr.getExpanded() );
          opStat = 0;
        }

      }

      if ( readExists ) {

	readPvId = the_PV_Factory->create( readPvExpStr.getExpanded() );
        if ( readPvId ) {
	  readPvId->add_conn_state_callback(
           mbt_monitor_read_connect_state, this );
          if ( !controlExists ) usePvId = activeMenuButtonClass::useReadPvId;
	}
	else {
          fprintf( stderr, activeMenuButtonClass_str20,
           readPvExpStr.getExpanded() );
          opStat = 0;
        }

      }

      if ( visExists ) {

	visPvId = the_PV_Factory->create( visPvExpStr.getExpanded() );
        if ( visPvId ) {
	  visPvId->add_conn_state_callback(
           mbt_monitor_vis_connect_state, this );
	}
	else {
          fprintf( stderr, activeMenuButtonClass_str20,
           visPvExpStr.getExpanded() );
          opStat = 0;
        }

      }

      if ( colorExists ) {

	colorPvId = the_PV_Factory->create( colorPvExpStr.getExpanded() );
        if ( colorPvId ) {
	  colorPvId->add_conn_state_callback(
           mbt_monitor_color_connect_state, this );
	}
	else {
          fprintf( stderr, activeMenuButtonClass_str20,
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

int activeMenuButtonClass::deactivate (
  int pass
) {

int i;

  active = 0;
  activeMode = 0;

  switch ( pass ) {

  case 1:

    if ( stateStringPvId ) {
      enumCount = (int) stateStringPvId->get_enum_count();
    }
    else {
      enumCount = 0;
    }

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    //updateBlink( 0 );

    if ( controlExists ) {
      if ( controlPvId ) {
        controlPvId->remove_conn_state_callback(
         mbt_monitor_control_connect_state, this );
        controlPvId->remove_value_callback(
         mbt_controlUpdate, this );
	controlPvId->release();
        controlPvId = NULL;
      }
    }

    if ( readExists ) {
      if ( readPvId ) {
        readPvId->remove_conn_state_callback(
         mbt_monitor_read_connect_state, this );
        readPvId->remove_value_callback(
         mbt_readUpdate, this );
	readPvId->release();
        readPvId = NULL;
      }
    }

    if ( visExists ) {
      if ( visPvId ) {
        visPvId->remove_conn_state_callback(
         mbt_monitor_vis_connect_state, this );
        visPvId->remove_value_callback(
         mbt_visUpdate, this );
	visPvId->release();
        visPvId = NULL;
      }
    }

    if ( colorExists ) {
      if ( colorPvId ) {
        colorPvId->remove_conn_state_callback(
         mbt_monitor_color_connect_state, this );
        colorPvId->remove_value_callback(
         mbt_colorUpdate, this );
	colorPvId->release();
        colorPvId = NULL;
      }
    }

    stateStringPvId = NULL;

    break;

  case 2:

    if ( widgetsCreated ) {
      for ( i=0; i<enumCount; i++ ) {
        XtDestroyWidget( pb[i] );
      }
      XtDestroyWidget( pullDownMenu );
      XtDestroyWidget( popUpMenu );
      widgetsCreated = 0;
    }

    break;

  }

  return 1;

}

void activeMenuButtonClass::updateDimensions ( void )
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

void activeMenuButtonClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled || !init || !visibility ) return;

  if ( !buttonPressed ) return;

  buttonPressed = 0;

  if ( !controlExists ) return;

  if ( controlPvId ) {
    if ( !controlPvId->have_write_access() ) return;
  }

  if ( buttonNumber == 1 ) {

    XmMenuPosition( popUpMenu, be );
    XtManageChild( popUpMenu );

  }

}

void activeMenuButtonClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled || !init || !visibility ) return;

  if ( !controlExists ) return;

  if ( controlPvId ) {
    if ( !controlPvId->have_write_access() ) return;
  }

  if ( buttonNumber == 1 ) {
    buttonPressed = 1;
  }

}

void activeMenuButtonClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled || !init || !visibility ) return;

  if ( controlPvId ) {
    if ( !controlPvId->have_write_access() ) {
      actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );
    }
    else {
      actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_DEFAULT );
    }
  }

  activeGraphicClass::pointerIn( _x, _y, buttonState );

}

int activeMenuButtonClass::getButtonActionRequest (
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

void activeMenuButtonClass::executeDeferred ( void ) {

short v, rV;
int i, nc, nrc, ni, nri, nr, nd, nvc, nvi, nvu, ncolc, ncoli, ncolu;
int stat, index, invisColor;
XmString str;
Arg args[15];
int n;
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
      strncpy( msg, actWin->obj.getNameFromClass( "activeMenuButtonClass" ),
       79 );
      Strncat( msg, activeMenuButtonClass_str35, 79 );
      actWin->appCtx->postMessage( msg );
      init = 0;
      needToDrawUnconnected = 1;
      drawActive();
      return;
    }

    v = curValue = (short) controlPvId->get_int();

    if ( usePvId == activeMenuButtonClass::useControlPvId ) {
      stateStringPvId = controlPvId;
    }

    ni = 1;

  }

  if ( nrc ) {

    rV = curReadValue = (short) readPvId->get_int();

    if ( usePvId == activeMenuButtonClass::useReadPvId ) {
      stateStringPvId = readPvId;
    }

    nri = 1;

  }

  if ( ni ) {

    value = v;

    if ( widgetsCreated ) {

      if ( stateStringPvId ) {
        for ( i=0; i<(int)stateStringPvId->get_enum_count(); i++ ) {
          XtDestroyWidget( pb[i] );
        }
      }
      XtDestroyWidget( pullDownMenu );
      XtDestroyWidget( popUpMenu );

      widgetsCreated = 0;

    }

    n = 0;
    XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
    popUpMenu = XmCreatePopupMenu( actWin->topWidgetId(), "", args, n );

    pullDownMenu = XmCreatePulldownMenu( popUpMenu, "", NULL, 0 );

    if ( stateStringPvId ) {

      for ( i=0; i<(int)stateStringPvId->get_enum_count(); i++ ) {

        str = XmStringCreateLocalized(
         (char *) stateStringPvId->get_enum( i ) );

        pb[i] = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         //XmNfontList, fontList,
         NULL );

        XmStringFree( str );

        XtAddCallback( pb[i], XmNactivateCallback, menu_cb,
         (XtPointer) this );

      }

    }

    widgetsCreated = 1;

    if ( initialConnection ) {

      initialConnection = 0;
      
      controlPvId->add_value_callback( mbt_controlUpdate, this );

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
      
      readPvId->add_value_callback( mbt_readUpdate, this );

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
      
      visPvId->add_value_callback( mbt_visUpdate, this );

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
      
      colorPvId->add_value_callback( mbt_colorUpdate, this );

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

char *activeMenuButtonClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeMenuButtonClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeMenuButtonClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

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
  else {

    if ( i == 0 ) {
      return controlPvExpStr.getRaw();
    }
    else if ( i == 1 ) {
      return readPvExpStr.getRaw();
    }
    else if ( i == 2 ) {
      return colorPvExpStr.getRaw();
    }
    else {
      return visPvExpStr.getRaw();
    }

  }

}

void activeMenuButtonClass::changeDisplayParams (
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

void activeMenuButtonClass::changePvNames (
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

void activeMenuButtonClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 4) {
    *n = 0;
    return;
  }

  *n = 4;
  pvs[0] = controlPvId;
  pvs[1] = readPvId;
  pvs[2] = colorPvId;
  pvs[3] = visPvId;

}

char *activeMenuButtonClass::getSearchString (
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

void activeMenuButtonClass::replaceString (
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
char *activeMenuButtonClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return controlPvExpStr.getExpanded();

}

char *activeMenuButtonClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >=3 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return readPvExpStr.getExpanded();
  }
  else if ( crawlerPvIndex == 2 ) {
    return colorPvExpStr.getExpanded();
  }
  else if ( crawlerPvIndex == 3 ) {
    return visPvExpStr.getExpanded();
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMenuButtonClassPtr ( void ) {

activeMenuButtonClass *ptr;

  ptr = new activeMenuButtonClass;
  return (void *) ptr;

}

void *clone_activeMenuButtonClassPtr (
  void *_srcPtr )
{

activeMenuButtonClass *ptr, *srcPtr;

  srcPtr = (activeMenuButtonClass *) _srcPtr;

  ptr = new activeMenuButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
