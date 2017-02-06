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

#define __pip_cc 1

#define SMALL_SYM_ARRAY_SIZE 10
#define SMALL_SYM_ARRAY_LEN 31

#define MAX_CONSECUTIVE_DEACTIVATE_ERRORS 100

#include "pip.h"
#include <sys/stat.h>
#include <unistd.h>
#include "utility.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activePipClass *pipo = (activePipClass *) client;

  if ( !pipo->init ) {
    pipo->actWin->appCtx->proc->lock();
    pipo->needToDrawUnconnected = 1;
    pipo->needConnectTimeout = 1;
    pipo->actWin->addDefExeNode( pipo->aglPtr );
    pipo->actWin->appCtx->proc->unlock();
  }

  pipo->unconnectedTimer = 0;

}

static void needUpdateTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activePipClass *pipo = (activePipClass *) client;

  pipo->actWin->appCtx->proc->lock();
  pipo->needUpdate = 1;
  pipo->actWin->addDefExeNode( pipo->aglPtr );
  pipo->actWin->appCtx->proc->unlock();

  pipo->retryTimerNU = 0;

}

static void needMenuUpdateTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activePipClass *pipo = (activePipClass *) client;

  pipo->actWin->appCtx->proc->lock();
  pipo->needMenuUpdate = 1;
  pipo->actWin->addDefExeNode( pipo->aglPtr );
  pipo->actWin->appCtx->proc->unlock();

  pipo->retryTimerNMU = 0;

}

static void needUnmapTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activePipClass *pipo = (activePipClass *) client;

  pipo->actWin->appCtx->proc->lock();
  pipo->needUnmap = 1;
  pipo->actWin->addDefExeNode( pipo->aglPtr );
  pipo->actWin->appCtx->proc->unlock();

  pipo->retryTimerNUM = 0;

}

static void needMapTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activePipClass *pipo = (activePipClass *) client;

  pipo->actWin->appCtx->proc->lock();
  pipo->needMap = 1;
  pipo->actWin->addDefExeNode( pipo->aglPtr );
  pipo->actWin->appCtx->proc->unlock();

  pipo->retryTimerNM = 0;

}

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int i;
activePipClass *pipo = (activePipClass *) client;

  for ( i=0; i<pipo->maxDsps; i++ ) {
    if ( w == pipo->pb[i] ) {
      pipo->readPvId->put( i );
      return;
    }
  }

}

static void pipc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;

  pipo->ef1->popdownNoDestroy();

}

static void pipc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;
int i, ii;

  pipo->actWin->setChanged();

  pipo->eraseSelectBoxCorners();
  pipo->erase();

  trimWhiteSpace( pipo->buf->bufDisplayFileName[0] );
  pipo->displayFileName[0].setRaw( pipo->buf->bufDisplayFileName[0] );
  if ( blank( pipo->displayFileName[0].getRaw() ) ) {
    pipo->propagateMacros[0] = 1;
    pipo->label[0].setRaw( "" );
    pipo->symbolsExpStr[0].setRaw( "" );
    pipo->replaceSymbols[0] = 0;
    pipo->numDsps = 0;
    ii = 0;
  }
  else {
    pipo->propagateMacros[0] = pipo->buf->bufPropagateMacros[0];
    pipo->label[0].setRaw( pipo->buf->bufLabel[0] );
    pipo->symbolsExpStr[0].setRaw( pipo->buf->bufSymbols[0] );
    pipo->replaceSymbols[0] = pipo->buf->bufReplaceSymbols[0];
    pipo->numDsps = 1;
    ii = 1;
  }

  for ( i=ii; i<pipo->maxDsps; i++ ) {
    if ( !blank( pipo->buf->bufDisplayFileName[i] ) ) {
      trimWhiteSpace( pipo->buf->bufDisplayFileName[i] );
      pipo->displayFileName[ii].setRaw( pipo->buf->bufDisplayFileName[i] );
      pipo->propagateMacros[ii] = pipo->buf->bufPropagateMacros[i];
      pipo->label[ii].setRaw( pipo->buf->bufLabel[i] );
      pipo->symbolsExpStr[ii].setRaw( pipo->buf->bufSymbols[i] );
      pipo->replaceSymbols[ii] = pipo->buf->bufReplaceSymbols[i];
      (pipo->numDsps)++;
      ii++;
    }
  }

  for ( i=pipo->numDsps; i<pipo->maxDsps; i++ ) {
    pipo->propagateMacros[i] = 1;
    pipo->label[i].setRaw( "" );
    pipo->symbolsExpStr[i].setRaw( "" );
    pipo->replaceSymbols[i] = 0;
  }

  pipo->fgColor.setColorIndex(
   pipo->buf->bufFgColor, pipo->actWin->ci );

  pipo->bgColor.setColorIndex(
   pipo->buf->bufBgColor, pipo->actWin->ci );

  pipo->topShadowColor.setColorIndex(
   pipo->buf->bufTopShadowColor, pipo->actWin->ci );

  pipo->botShadowColor.setColorIndex(
   pipo->buf->bufBotShadowColor, pipo->actWin->ci );

  pipo->readPvExpStr.setRaw( pipo->buf->bufReadPvName );

  pipo->labelPvExpStr.setRaw( pipo->buf->bufLabelPvName );

  trimWhiteSpace( pipo->buf->bufFileName );
  pipo->fileNameExpStr.setRaw( pipo->buf->bufFileName );

  pipo->displaySource = pipo->buf->bufDisplaySource;

  pipo->center = pipo->buf->bufCenter;
  pipo->setSize = pipo->buf->bufSetSize;
  pipo->sizeOfs = pipo->buf->bufSizeOfs;
  pipo->noScroll = pipo->buf->bufNoScroll;
  pipo->ignoreMultiplexors = pipo->buf->bufIgnoreMultiplexors;

  pipo->x = pipo->buf->bufX;
  pipo->sboxX = pipo->buf->bufX;

  pipo->y = pipo->buf->bufY;
  pipo->sboxY = pipo->buf->bufY;

  pipo->w = pipo->buf->bufW;
  pipo->sboxW = pipo->buf->bufW;

  pipo->h = pipo->buf->bufH;
  pipo->sboxH = pipo->buf->bufH;

  if ( pipo->h < pipo->minH ) {
    pipo->h = pipo->minH;
    pipo->sboxH = pipo->minH;
  }

}

static void pipc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;

  pipc_edit_update ( w, client, call );
  pipo->refresh( pipo );

}

static void pipc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;

  pipc_edit_update ( w, client, call );
  pipo->ef.popdown();
  pipo->operationComplete();

  delete pipo->buf;
  pipo->buf = NULL;

}

static void pipc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;

  pipo->ef.popdown();
  pipo->operationCancel();

  delete pipo->buf;
  pipo->buf = NULL;

}

static void pipc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;

  delete pipo->buf;
  pipo->buf = NULL;

  pipo->ef.popdown();
  pipo->operationCancel();
  pipo->erase();
  pipo->deleteRequest = 1;
  pipo->drawAll();

}

static void pip_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activePipClass *pipo = (activePipClass *) userarg;

  if ( pv->is_valid() ) {

    pipo->needConnectInit = 1;

  }
  else {

    pipo->readPvConnected = 0;
    pipo->active = 0;
    pipo->fgColor.setDisconnected();
    pipo->needDraw = 1;

  }

  pipo->actWin->appCtx->proc->lock();
  pipo->actWin->addDefExeNode( pipo->aglPtr );
  pipo->actWin->appCtx->proc->unlock();

}

static void pip_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activePipClass *pipo = (activePipClass *) userarg;

  if ( pipo->active ) {

    pv->get_string( pipo->curReadV, 39 );
    pipo->curReadV[39] = 0;

    pipo->actWin->appCtx->proc->lock();
    pipo->needUpdate = 1;
    pipo->actWin->addDefExeNode( pipo->aglPtr );
    pipo->actWin->appCtx->proc->unlock();

  }

}

static void pip_monitor_menu_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activePipClass *pipo = (activePipClass *) userarg;

  if ( pv->is_valid() ) {

    pipo->needMenuConnectInit = 1;

  }
  else {

    pipo->readPvConnected = 0;
    pipo->active = 0;
    pipo->fgColor.setDisconnected();
    pipo->needDraw = 1;

  }

  pipo->actWin->appCtx->proc->lock();
  pipo->actWin->addDefExeNode( pipo->aglPtr );
  pipo->actWin->appCtx->proc->unlock();

}

static void pip_menuUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activePipClass *pipo = (activePipClass *) userarg;

  if ( pipo->active ) {

    pipo->curReadIV = pv->get_int();
    if ( pipo->curReadIV < -1 ) pipo->curReadIV = 0;
    if ( pipo->curReadIV >= pipo->numDsps ) pipo->curReadIV = pipo->numDsps;

    if ( pipo->firstEvent ) { // don't let menu pop up if pv is -1 initially
      pipo->firstEvent = 0;
      if ( pipo->curReadIV == -1 ) {
        pipo->curReadIV = 0;
        pv->put( pipo->curReadIV );
        return;
      }
    }

    pipo->actWin->appCtx->proc->lock();
    pipo->needMenuUpdate = 1;
    pipo->actWin->addDefExeNode( pipo->aglPtr );
    pipo->actWin->appCtx->proc->unlock();

  }

}

static void pip_monitor_label_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

}

activePipClass::activePipClass ( void ) {

int i;

  name = new char[strlen("activePipClass")+1];
  strcpy( name, "activePipClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  minW = 50;
  minH = 50;
  center = 0;
  setSize = 0;
  sizeOfs = 5;
  noScroll = 0;
  ignoreMultiplexors = 0;
  activeMode = 0;
  frameWidget = NULL;
  clipWidget = NULL;
  aw = NULL;
  strcpy( curFileName, "" );
  displaySource = 0;
  readPvId = NULL;
  labelPvId = NULL;
  activateIsComplete = 0;

  for ( i=0; i<maxDsps; i++ ) {
    propagateMacros[i] = 1;
    replaceSymbols[i] = 0;
  }

  numDsps = 0;
  popUpMenu = NULL;
  unconnectedTimer = retryTimerNU = retryTimerNMU = retryTimerNUM =
   retryTimerNM = 0;
  buf = NULL;

  consecutiveDeactivateErrors = 0;

}

// copy constructor
activePipClass::activePipClass
 ( const activePipClass *source ) {

activeGraphicClass *pipo = (activeGraphicClass *) this;
int i;

  pipo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activePipClass")+1];
  strcpy( name, "activePipClass" );

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  topCb = source->topCb;
  botCb = source->botCb;

  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );
  topShadowColor.copy( source->topShadowColor );
  botShadowColor.copy( source->botShadowColor );

  readPvExpStr.copy( source->readPvExpStr );
  labelPvExpStr.copy( source->labelPvExpStr );
  fileNameExpStr.copy( source->fileNameExpStr );

  minW = 50;
  minH = 50;
  center = source->center;
  setSize = source->setSize;
  sizeOfs = source->sizeOfs;
  noScroll = source->noScroll;
  ignoreMultiplexors = source->ignoreMultiplexors;
  frameWidget = NULL;
  clipWidget = NULL;
  aw = NULL;
  strcpy( curFileName, "" );
  displaySource = source->displaySource;
  readPvId = NULL;
  labelPvId = NULL;
  activateIsComplete = 0;

  for ( i=0; i<maxDsps; i++ ) {
    propagateMacros[i] = source->propagateMacros[i];
    replaceSymbols[i] = source->replaceSymbols[i];
    displayFileName[i].copy( source->displayFileName[i] );
    label[i].copy( source->label[i] );
    symbolsExpStr[i].copy( source->symbolsExpStr[i] );
  }

  numDsps = source->numDsps;
  popUpMenu = NULL;
  unconnectedTimer = retryTimerNU = retryTimerNMU = retryTimerNUM =
   retryTimerNM = 0;
  buf = NULL;

  consecutiveDeactivateErrors = 0;

  doAccSubs( readPvExpStr );
  doAccSubs( labelPvExpStr );
  doAccSubs( fileNameExpStr );
  for ( i=0; i<numDsps; i++ ) {
    doAccSubs( symbolsExpStr[i] );
    doAccSubs( label[i] );
    doAccSubs( displayFileName[i] );
  }

}

activePipClass::~activePipClass ( void ) {

  if ( name ) delete[] name;

  if ( buf ) {
    delete buf;
    buf = NULL;
  }

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  if ( retryTimerNU ) {
    XtRemoveTimeOut( retryTimerNU );
    retryTimerNU = 0;
  }

  if ( retryTimerNMU ) {
    XtRemoveTimeOut( retryTimerNMU );
    retryTimerNMU = 0;
  }

  if ( retryTimerNUM ) {
    XtRemoveTimeOut( retryTimerNUM );
    retryTimerNUM = 0;
  }

  if ( retryTimerNM ) {
    XtRemoveTimeOut( retryTimerNM );
    retryTimerNM = 0;
  }

}

int activePipClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = (activeWindowClass *) aw_obj;

  if ( _w < minW )
    w = minW;
  else
    w = _w;

  if ( _h < minH )
    h = minH;
  else
    h = _h;

  x = _x;
  y = _y;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  topShadowColor.setColorIndex( actWin->defaultTopShadowColor, actWin->ci );
  botShadowColor.setColorIndex( actWin->defaultBotShadowColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int activePipClass::save (
  FILE *f )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
int one = 1;
static char *emptyStr = "";

int displaySourceFromPV = 0;
static char *displaySourceEnumStr[3] = {
  "stringPV",
  "file",
  "menu"
};
static int displaySourceEnum[3] = {
  0,
  1,
  2
};

  major = PIPC_MAJOR_VERSION;
  minor = PIPC_MINOR_VERSION;
  release = PIPC_RELEASE;

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
  tag.loadW( "displaySource", 3, displaySourceEnumStr, displaySourceEnum,
   &displaySource, &displaySourceFromPV );
  tag.loadW( "filePv", &readPvExpStr, emptyStr );
  tag.loadW( "labelPv", &labelPvExpStr, emptyStr );
  tag.loadW( "file", &fileNameExpStr, emptyStr );
  tag.loadBoolW( "center", &center, &zero );
  tag.loadBoolW( "setSize", &setSize, &zero );
  tag.loadW( "sizeOfs", &sizeOfs, &zero );
  tag.loadW( "numDsps", &numDsps );
  tag.loadW( "displayFileName", displayFileName, numDsps, emptyStr );
  tag.loadW( "menuLabel", label, numDsps, emptyStr );
  tag.loadW( "symbols", symbolsExpStr, numDsps, emptyStr );
  tag.loadW( "replaceSymbols", replaceSymbols, numDsps, &zero );
  tag.loadW( "propagateMacros", propagateMacros, numDsps, &one );
  tag.loadBoolW( "noScroll", &noScroll, &zero );
  tag.loadBoolW( "ignoreMultiplexors", &ignoreMultiplexors, &zero );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activePipClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", PIPC_MAJOR_VERSION,
   PIPC_MINOR_VERSION, PIPC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = topShadowColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = botShadowColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  if ( readPvExpStr.getRaw() )
    writeStringToFile( f, readPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( fileNameExpStr.getRaw() )
    writeStringToFile( f, fileNameExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  return 1;

}

int activePipClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat, n;

tagClass tag;

int zero = 0;
int one = 1;
static char *emptyStr = "";

int displaySourceFromPV = 0;
static char *displaySourceEnumStr[3] = {
  "stringPV",
  "file",
  "menu"
};
static int displaySourceEnum[3] = {
  0,
  1,
  2
};

  this->actWin = _actWin;

  // read file and process each "object" tag
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
  tag.loadR( "displaySource", 3, displaySourceEnumStr, displaySourceEnum,
   &displaySource, &displaySourceFromPV );
  tag.loadR( "filePv", &readPvExpStr, emptyStr );
  tag.loadR( "labelPv", &labelPvExpStr, emptyStr );
  tag.loadR( "file", &fileNameExpStr, emptyStr );
  tag.loadR( "center", &center, &zero );
  tag.loadR( "setSize", &setSize, &zero );
  tag.loadR( "sizeOfs", &sizeOfs, &zero );
  tag.loadR( "numDsps", &numDsps, &zero );
  tag.loadR( "displayFileName", maxDsps, displayFileName, &n, emptyStr );
  tag.loadR( "menuLabel", maxDsps, label, &n, emptyStr );
  tag.loadR( "symbols", maxDsps, symbolsExpStr, &n, emptyStr );
  tag.loadR( "replaceSymbols", maxDsps, replaceSymbols, &n, &zero );
  tag.loadR( "propagateMacros", maxDsps, propagateMacros, &n, &one );
  tag.loadR( "noScroll", &noScroll, &zero );
  tag.loadR( "ignoreMultiplexors", &ignoreMultiplexors, &zero );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > PIPC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox();

  return stat;

}

int activePipClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int index;
int major, minor, release;
char oneName[PV_Factory::MAX_PV_NAME+1];
char oneFileName[127+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  if ( major > PIPC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }
  fscanf( f, "%d\n", &x );
  fscanf( f, "%d\n", &y );
  fscanf( f, "%d\n", &w );
  fscanf( f, "%d\n", &h );

  this->initSelectBox();

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  fgColor.setColorIndex( index, actWin->ci );

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  bgColor.setColorIndex( index, actWin->ci );

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  topShadowColor.setColorIndex( index, actWin->ci );

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  botShadowColor.setColorIndex( index, actWin->ci );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
  readPvExpStr.setRaw( oneName );

  readStringFromFile( oneFileName, 127, f );
  fileNameExpStr.setRaw( oneFileName );

  return 1;

}

int activePipClass::genericEdit ( void ) {

char title[32], *ptr;
int i;

  buf = new bufType;

  ptr = actWin->obj.getNameFromClass( "activePipClass" );
  if ( ptr ) {
    strncpy( title, ptr, 31 );
    title[31] = 0;
  }
  else {
    strncpy( title, activePipClass_str4, 31 );
    title[31] = 0;
  }

  Strncat( title, activePipClass_str5, 31 );

  buf->bufX = x;
  buf->bufY = y;
  buf->bufW = w;
  buf->bufH = h;

  buf->bufFgColor = fgColor.pixelIndex();
  buf->bufBgColor = bgColor.pixelIndex();
  buf->bufTopShadowColor = topShadowColor.pixelIndex();
  buf->bufBotShadowColor = botShadowColor.pixelIndex();

  if ( readPvExpStr.getRaw() ) {
    strncpy( buf->bufReadPvName, readPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
    buf->bufReadPvName[PV_Factory::MAX_PV_NAME] = 0;
  }
  else {
    strcpy( buf->bufReadPvName, "" );
    buf->bufReadPvName[PV_Factory::MAX_PV_NAME] = 0;
  }

  if ( labelPvExpStr.getRaw() ) {
    strncpy( buf->bufLabelPvName, labelPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
    buf->bufLabelPvName[PV_Factory::MAX_PV_NAME] = 0;
  }
  else {
    strcpy( buf->bufLabelPvName, "" );
  }

  if ( fileNameExpStr.getRaw() ) {
    strncpy( buf->bufFileName, fileNameExpStr.getRaw(), 127 );
    buf->bufFileName[127] = 0;
  }
  else {
    strcpy( buf->bufFileName, "" );
  }

  buf->bufDisplaySource = displaySource;

  buf->bufCenter = center;
  buf->bufSetSize = setSize;
  buf->bufSizeOfs = sizeOfs;
  buf->bufNoScroll = noScroll;
  buf->bufIgnoreMultiplexors = ignoreMultiplexors;

  for ( i=0; i<maxDsps; i++ ) {

    if ( displayFileName[i].getRaw() ) {
      strncpy( buf->bufDisplayFileName[i], displayFileName[i].getRaw(), 127 );
      buf->bufDisplayFileName[i][127] = 0;
    }
    else {
      strncpy( buf->bufDisplayFileName[i], "", 127 );
    }

    if ( label[i].getRaw() ) {
      strncpy( buf->bufLabel[i], label[i].getRaw(), 127 );
      buf->bufLabel[i][127] = 0;
    }
    else {
      strncpy( buf->bufLabel[i], "", 127 );
    }

    if ( symbolsExpStr[i].getRaw() ) {
      strncpy( buf->bufSymbols[i], symbolsExpStr[i].getRaw(), maxSymbolLen );
      buf->bufSymbols[i][maxSymbolLen] = 0;
    }
    else {
      strncpy( buf->bufSymbols[i], "", maxSymbolLen );
    }

    buf->bufPropagateMacros[i] = propagateMacros[i];

    buf->bufReplaceSymbols[i] = replaceSymbols[i];

  }

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activePipClass_str6, 35, &buf->bufX );
  ef.addTextField( activePipClass_str7, 35, &buf->bufY );
  ef.addTextField( activePipClass_str8, 35, &buf->bufW );
  ef.addTextField( activePipClass_str9, 35, &buf->bufH );

  ef.addOption( "Display Source", "String PV|Form|Menu",
   &buf->bufDisplaySource );
  disSrcEntry = ef.getCurItem();
  disSrcEntry->setNumValues( 3 );

  ef.addTextField( activePipClass_str11, 35, buf->bufReadPvName,
   PV_Factory::MAX_PV_NAME );
  pvNameEntry = ef.getCurItem();

  ef.addTextField( "Label PV", 35, buf->bufLabelPvName,
   PV_Factory::MAX_PV_NAME );
  labelPvNameEntry = ef.getCurItem();

  ef.addTextField( activePipClass_str12, 35, buf->bufFileName, 127 );
  fileNameEntry = ef.getCurItem();
  disSrcEntry->addDependency( 1, fileNameEntry );
  disSrcEntry->addInvDependency( 1, pvNameEntry );

  ef.addToggle( "Center", &buf->bufCenter );

  ef.addToggle( "Set Size", &buf->bufSetSize );
  setSizeEntry = ef.getCurItem();
  ef.addTextField( "Size Ofs", 35, &buf->bufSizeOfs );
  sizeOfsEntry = ef.getCurItem();
  setSizeEntry->addDependency( sizeOfsEntry );
  setSizeEntry->addDependencyCallbacks();

  ef.addToggle( "Disable Scroll Bars", &buf->bufNoScroll );
  ef.addToggle( "Ignore Multiplexors", &buf->bufIgnoreMultiplexors );

  ef.addEmbeddedEf( "Menu Info", "...", &ef1 );
  menuBtnEntry = ef.getCurItem();
  disSrcEntry->addDependency( 2, menuBtnEntry );
  disSrcEntry->addDependency( 2, labelPvNameEntry );
  disSrcEntry->addDependencyCallbacks();

  ef1->create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  for ( i=0; i<maxDsps; i++ ) {

    ef1->beginSubForm();
    ef1->addTextField( "Label", 35, buf->bufLabel[i], 127 );
    ef1->addLabel( "  File" );
    ef1->addTextField( "", 35, buf->bufDisplayFileName[i], 127 );
    ef1->addLabel( "  Macros" );
    ef1->addTextField( "", 35, buf->bufSymbols[i], maxSymbolLen );
    ef1->endSubForm();

    ef1->beginLeftSubForm();
    ef1->addLabel( "  Mode" );
    ef1->addOption( "", "Append|Replace",
     &buf->bufReplaceSymbols[i] );
    ef1->addLabel( " " );
    ef1->addToggle( " ", &buf->bufPropagateMacros[i] );
    ef1->addLabel( "Propagate  " );
    ef1->endSubForm();

  }

  ef1->finished( pipc_edit_ok1, this );

  ef.addColorButton( activePipClass_str16, actWin->ci, &fgCb,
   &buf->bufFgColor );
  ef.addColorButton( activePipClass_str18, actWin->ci, &bgCb,
   &buf->bufBgColor );
  ef.addColorButton( activePipClass_str19, actWin->ci, &topCb,
   &buf->bufTopShadowColor );
  ef.addColorButton( activePipClass_str20, actWin->ci, &botCb,
   &buf->bufBotShadowColor );

  return 1;

}

int activePipClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( pipc_edit_ok, pipc_edit_apply, pipc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activePipClass::edit ( void ) {

  this->genericEdit();
  ef.finished( pipc_edit_ok, pipc_edit_apply, pipc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activePipClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activePipClass::eraseActive ( void ) {

  return 1;

}

int activePipClass::draw ( void ) {

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( bgColor.pixelColor() );
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.restoreFg();

  return 1;

}

int activePipClass::drawActive ( void ) {

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( bgColor.getDisconnected() );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
  }

  if ( !enabled || !activeMode || !init ) return 1;

  if ( aw ) {
    if ( aw->loadFailure ) {
      aw = NULL;
      frameWidget = NULL;
    }
    else {
      if ( frameWidget ) {
        if ( *frameWidget ) XtMapWidget( *frameWidget );
      }
    }
  }

  return 1;

}

int activePipClass::reactivate (
  int pass,
  void *ptr ) {

  if ( ignoreMultiplexors ) {
    return 1;
  }

  return activate( pass, ptr );

}

int activePipClass::reactivate (
  int pass,
  void *ptr,
  int *numSubObjects ) {

  if ( ignoreMultiplexors ) {
    return 1;
  }

  return activate( pass, ptr );

}

int activePipClass::activate (
  int pass,
  void *ptr )
{

int i, n;
Arg args[5];
XmString str;

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      opComplete = 1;

      initEnable();

      aglPtr = ptr;
      needConnectInit = needUpdate = needMenuConnectInit = needMenuUpdate =
       needDraw = needFileOpen = needInitMenuFileOpen = needUnmap =
       needMap = needToEraseUnconnected = needToDrawUnconnected =
       needConnectTimeout = 0;
      unconnectedTimer = retryTimerNU = retryTimerNMU = retryTimerNUM =
       retryTimerNM = 0;
      activateIsComplete = 0;
      curReadIV = 0;
      strcpy( curReadV, "" );
      firstEvent = 1;
      initialReadConnection = initialMenuConnection =
       initialLabelConnection = 1;

      readPvId = labelPvId = NULL;

      readPvConnected = active = init = 0;
      activeMode = 1;

      if ( !readPvExpStr.getExpanded() ||
            blankOrComment( readPvExpStr.getExpanded() ) ) {
        readExists = 0;
      }
      else {
        readExists = 1;
        fgColor.setConnectSensitive();
      }

      if ( !labelPvExpStr.getExpanded() ||
            blankOrComment( labelPvExpStr.getExpanded() ) ) {
        labelExists = 0;
      }
      else {
        labelExists = 1;
      }

      if ( !fileNameExpStr.getExpanded() ||
            blank( fileNameExpStr.getExpanded() ) ) {
        fileExists = 0;
      }
      else {
        fileExists = 1;
      }

      switch ( displaySource ) {

      case displayFromPV:

        if ( readExists ) {

          if ( !unconnectedTimer ) {
            unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
             5000, unconnectedTimeout, this );
          }

	  readPvId = the_PV_Factory->create( readPvExpStr.getExpanded() );
	  if ( readPvId ) {
	    readPvId->add_conn_state_callback( pip_monitor_read_connect_state,
             this );
	  }
	  else {
            fprintf( stderr, activePipClass_str22 );
          }

        }

        activateIsComplete = 1;

      break;

      case displayFromForm:

        if ( fileExists ) {
          needFileOpen = 1;
          actWin->addDefExeNode( aglPtr );
        }
        else {
          activateIsComplete = 1;
        }

        break;

      case displayFromMenu:

        if ( readExists && ( numDsps > 0 ) ) {

          if ( !unconnectedTimer ) {
            unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
             5000, unconnectedTimeout, this );
          }

	  readPvId = the_PV_Factory->create( readPvExpStr.getExpanded() );
	  if ( readPvId ) {
	    readPvId->add_conn_state_callback( pip_monitor_menu_connect_state,
             this );
	  }
	  else {
            fprintf( stderr, activePipClass_str22 );
          }

          if ( labelExists ) {
	    labelPvId = the_PV_Factory->create( labelPvExpStr.getExpanded() );
	    if ( labelPvId ) {
	      labelPvId->add_conn_state_callback(
               pip_monitor_label_connect_state, this );
	    }
	    else {
              fprintf( stderr, activePipClass_str22 );
            }
	  }

          if ( !popUpMenu ) {

            n = 0;
            XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
            popUpMenu = XmCreatePopupMenu( actWin->topWidgetId(), "", args,
             n );

            pullDownMenu = XmCreatePulldownMenu( popUpMenu, "", NULL, 0 );

            for ( i=0; i<numDsps; i++ ) {

              if ( label[i].getExpanded() ) {
                str = XmStringCreateLocalized( label[i].getExpanded() );
              }
              else {
                str = XmStringCreateLocalized( " " );
              }
              pb[i] = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
               popUpMenu,
               XmNlabelString, str,
               NULL );
              XmStringFree( str );

              XtAddCallback( pb[i], XmNactivateCallback, menu_cb,
               (XtPointer) this );

            }

          }

#if 0
          if ( !blank( displayFileName[0].getExpanded() ) ) {
            needInitMenuFileOpen = 1;
            actWin->addDefExeNode( aglPtr );
          }
          else {
            activateIsComplete = 1;
          }
#endif

	}
	else {

          activateIsComplete = 1;

        }

        break;

      default:

        activateIsComplete = 1;
        break;

      }

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

int activePipClass::preReactivate ( int pass ) {

  if ( ignoreMultiplexors ) {
    return 1;
  }

  return deactivate( pass );

}

int activePipClass::preReactivate (
  int pass,
  int *numSubObjects ) {

  if ( ignoreMultiplexors ) {
    return 1;
  }

  return deactivate( pass );

}

int activePipClass::deactivate (
  int pass
) {

int okToClose, stat;
activeWindowListPtr cur;

  if ( pass == 1 ) {

    active = 0;
    activeMode = 0;

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    if ( retryTimerNU ) {
      XtRemoveTimeOut( retryTimerNU );
      retryTimerNU = 0;
    }

    if ( retryTimerNMU ) {
      XtRemoveTimeOut( retryTimerNMU );
      retryTimerNMU = 0;
    }

    if ( retryTimerNUM ) {
      XtRemoveTimeOut( retryTimerNUM );
      retryTimerNUM = 0;
    }

    if ( retryTimerNM ) {
      XtRemoveTimeOut( retryTimerNM );
      retryTimerNM = 0;
    }

    if ( aw ) {
      if ( aw->loadFailure ) {
        aw = NULL;
        frameWidget = NULL;
      }
    }

    if ( frameWidget ) {
      if ( *frameWidget ) XtUnmapWidget( *frameWidget );
    }

    if ( aw ) {

      okToClose = 0;
      // make sure the window was successfully opened
      cur = actWin->appCtx->head->flink;
      while ( cur != actWin->appCtx->head ) {
        if ( &cur->node == aw ) {
          okToClose = 1;
          break;
        }
        cur = cur->flink;
      }

      if ( okToClose ) {
        stat = aw->returnToEdit( 1 );
      }

      aw = NULL;

    }

    if ( frameWidget ) {
      frameWidget = NULL;
    }

    if ( readPvId ) {
      readPvId->remove_conn_state_callback( pip_monitor_read_connect_state,
       this );
      if ( !initialReadConnection ) {
        readPvId->remove_value_callback( pip_readUpdate, this );
      }
      if ( !initialMenuConnection ) {
        readPvId->remove_value_callback( pip_menuUpdate, this );
      }
      readPvId->release();
      readPvId = NULL;
    }

    if ( labelPvId ) {
      labelPvId->remove_conn_state_callback( pip_monitor_label_connect_state,
       this );
      labelPvId->release();
      labelPvId = NULL;
    }

    if ( popUpMenu ) {
      XtDestroyWidget( popUpMenu );
      popUpMenu = NULL;
    }

  }

  return 1;

}

// This widget is like a related display, but it can operate in 3 modes.
//
// Mode 1 gets the display name from a string pv and thus it is impossible
// to know what the value might be so num displays will be reported as 0.
//
// Mode 2 gets the display name from the form - one name with no macros
//
// Mode 3 gets the display name from a menu of entries with macros and with
// the propagate attribute

int activePipClass::isRelatedDisplay ( void ) {

  return 1;

}

void activePipClass::augmentRelatedDisplayMacros (
  char *buf
) {

  int i, l;
  char *newm;

  for ( i=0; i<numDsps; i++ ) {
    l = strlen( buf ) + strlen( symbolsExpStr[i].getRaw() );
    if ( l ) {
      newm = new char[l+1];
      strcpy( newm, "" );
      Strncat( newm, symbolsExpStr[i].getRaw(), l );
      trimWhiteSpace( newm );
      if ( strlen(newm) ) Strncat( newm, ",", l );
      Strncat( newm, buf, l );
      symbolsExpStr[i].setRaw( newm );
      delete[] newm;
    }
  }

}

int activePipClass::getNumRelatedDisplays ( void ) {

  if ( displaySource == displayFromPV ) {
    return 0;
  }
  else if ( displaySource == displayFromForm ) {
    return 1;
  }
  else if ( displaySource == displayFromMenu ) {
    return numDsps;
  }

  return 0;

}

int activePipClass::getRelatedDisplayProperty (
  int index,
  char *key
) {

  if ( displaySource == displayFromMenu ) {

    if ( strcmp( key, "propagate" ) == 0 ) {
      return propagateMacros[index];
    }
    else if ( strcmp( key, "replace" ) == 0 ) {
      return replaceSymbols[index];
    }

  }
  else {

    if ( strcmp( key, "propagate" ) == 0 ) {
      return 1;
    }
    else if ( strcmp( key, "replace" ) == 0 ) {
      return 0;
    }

  }

  return 0;

}

char *activePipClass::getRelatedDisplayName (
  int index
) {

  if ( displaySource == displayFromPV ) {

    return NULL;

  }
  else if ( displaySource == displayFromForm ) {

    if ( index != 0 ) return NULL;

    return fileNameExpStr.getExpanded();

  }
  else if ( displaySource == displayFromMenu ) {

    if ( ( index < 0 ) || ( index >= numDsps ) ) {
      return NULL;
    }

    return displayFileName[index].getExpanded();

  }

  return NULL;

}

char *activePipClass::getRelatedDisplayMacros (
  int index
) {

  if ( ( index < 0 ) || ( index >= numDsps ) ) {
    return NULL;
  }

  return symbolsExpStr[index].getExpanded();

}

int activePipClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;
expStringClass tmpStr;

  tmpStr.setRaw( readPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( labelPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  labelPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( fileNameExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  fileNameExpStr.setRaw( tmpStr.getExpanded() );


  for ( i=0; i<numDsps; i++ ) {

    tmpStr.setRaw( symbolsExpStr[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    symbolsExpStr[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( label[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    label[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( displayFileName[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    displayFileName[i].setRaw( tmpStr.getExpanded() );

  }

  return 1;

}

int activePipClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat, i;

  retStat = readPvExpStr.expand1st( numMacros, macros, expansions );

  stat = labelPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = fileNameExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  for ( i=0; i<numDsps; i++ ) {
    stat = symbolsExpStr[i].expand1st( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
    stat = label[i].expand1st( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
    stat = displayFileName[i].expand1st( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
  }

  return retStat;

}

int activePipClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat, i;

  retStat = readPvExpStr.expand2nd( numMacros, macros, expansions );

  stat = labelPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = fileNameExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  for ( i=0; i<numDsps; i++ ) {
    stat = symbolsExpStr[i].expand2nd( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
    stat = label[i].expand2nd( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
    stat = displayFileName[i].expand2nd( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
  }

  return retStat;

}

int activePipClass::containsMacros ( void ) {

int i;

  if ( readPvExpStr.containsPrimaryMacros() ) return 1;

  if ( labelPvExpStr.containsPrimaryMacros() ) return 1;

  if ( fileExists && fileNameExpStr.containsPrimaryMacros() ) return 1;

  for ( i=0; i<numDsps; i++ ) {
    if ( symbolsExpStr[i].containsPrimaryMacros() ) return 1;
    if ( label[i].containsPrimaryMacros() ) return 1;
    if ( displayFileName[i].containsPrimaryMacros() ) return 1;
  }

  return 0;

}

int activePipClass::createPipWidgets ( void ) {

  frameWidget = new Widget;
  *frameWidget = NULL;

  if ( noScroll ) {

    *frameWidget = XtVaCreateWidget( "", xmBulletinBoardWidgetClass,
     actWin->executeWidgetId(),
     XmNx, x,
     XmNy, y,
     XmNwidth, w,
     XmNheight, h,
     XmNnoResize, True,
     XmNresizePolicy, XmRESIZE_NONE,
     XmNmarginWidth, 0,
     XmNmarginHeight, 0,
     XmNtopShadowColor, topShadowColor.pixelColor(),
     XmNbottomShadowColor, botShadowColor.pixelColor(),
     XmNborderColor, bgColor.pixelColor(),
     XmNhighlightColor, bgColor.pixelColor(),
     XmNforeground, bgColor.pixelColor(),
     XmNbackground, bgColor.pixelColor(),
     NULL );

    if ( !(*frameWidget) ) {
      fprintf( stderr, activePipClass_str24 );
      frameWidget = NULL;
      return 0;
    }

  }
  else {

    *frameWidget = XtVaCreateWidget( "", xmScrolledWindowWidgetClass,
     actWin->executeWidgetId(),
     XmNx, x,
     XmNy, y,
     XmNwidth, w,
     XmNheight, h,
     XmNscrollBarDisplayPolicy, XmAS_NEEDED,
     XmNscrollingPolicy, XmAUTOMATIC,
     XmNvisualPolicy, XmCONSTANT,
     XmNmarginWidth, 0,
     XmNmarginHeight, 0,
     XmNtopShadowColor, topShadowColor.pixelColor(),
     XmNbottomShadowColor, botShadowColor.pixelColor(),
     XmNborderColor, bgColor.pixelColor(),
     XmNhighlightColor, bgColor.pixelColor(),
     XmNforeground, bgColor.pixelColor(),
     XmNbackground, bgColor.pixelColor(),
     NULL );

    if ( !(*frameWidget) ) {
      fprintf( stderr, activePipClass_str24 );
      frameWidget = NULL;
      return 0;
    }

    XtVaGetValues( *frameWidget,
     XmNclipWindow, &clipWidget,
     XmNhorizontalScrollBar, &hsbWidget,
     XmNverticalScrollBar, &vsbWidget,
     NULL );

    if ( clipWidget ) {
      XtVaSetValues( clipWidget,
        XmNtopShadowColor, topShadowColor.pixelColor(),
        XmNbottomShadowColor, botShadowColor.pixelColor(),
        XmNborderColor, bgColor.pixelColor(),
        XmNhighlightColor, bgColor.pixelColor(),
        XmNforeground, bgColor.pixelColor(),
        XmNbackground, bgColor.pixelColor(),
       NULL );
    }

    if ( hsbWidget ) {
      XtVaSetValues( hsbWidget,
        XmNtopShadowColor, topShadowColor.pixelColor(),
        XmNbottomShadowColor, botShadowColor.pixelColor(),
        XmNborderColor, bgColor.pixelColor(),
        XmNhighlightColor, bgColor.pixelColor(),
        XmNforeground, bgColor.pixelColor(),
        XmNbackground, bgColor.pixelColor(),
        XmNtroughColor, bgColor.pixelColor(),
        NULL );
    }

    if ( vsbWidget ) {
      XtVaSetValues( vsbWidget,
        XmNtopShadowColor, topShadowColor.pixelColor(),
        XmNbottomShadowColor, botShadowColor.pixelColor(),
        XmNborderColor, bgColor.pixelColor(),
        XmNhighlightColor, bgColor.pixelColor(),
        XmNforeground, bgColor.pixelColor(),
        XmNbackground, bgColor.pixelColor(),
        XmNtroughColor, bgColor.pixelColor(),
        NULL );
    }

  }

  return 1;

}

int activePipClass::checkResizeSelectBox (
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

int activePipClass::checkResizeSelectBoxAbs (
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

void activePipClass::openEmbeddedByIndex (
  int index )
{

activeWindowListPtr cur;
int i, l, stat;
char symbolsWithSubs[maxSymbolLen+1], nameWithSubs[maxSymbolLen+1];
int useSmallArrays, symbolCount, maxSymbolLength;
char smallNewMacros[SMALL_SYM_ARRAY_SIZE+1][SMALL_SYM_ARRAY_LEN+1+1];
char smallNewValues[SMALL_SYM_ARRAY_SIZE+1][SMALL_SYM_ARRAY_LEN+1+1];
char *newMacros[100];
char *newValues[100];
int numNewMacros, max, numFound;

char *formTk, *formContext, formBuf[maxSymbolLen+1], *fileTk, *fileContext,
 fileBuf[maxSymbolLen+1], *result, msg[79+1], macDefFileName[127+1];
FILE *f;
expStringClass symbolsFromFile;
int gotSymbolsFromFile;

  // allow the syntax: @filename s1=v1,s2=v2,...
  // which means read symbols from file and append list
  gotSymbolsFromFile = 0;
  strncpy( formBuf, symbolsExpStr[index].getExpanded(), maxSymbolLen );
  formBuf[maxSymbolLen] = 0;
  formContext = NULL;
  formTk = strtok_r( formBuf, " \t\n", &formContext );
  if ( formTk ) {
    if ( formTk[0] == '@' ) {
      if ( formTk[1] ) {
        f = actWin->openAnyGenericFile( &formTk[1], "r", macDefFileName, 127 );
	if ( !f ) {
          snprintf( msg, 79, activePipClass_str27, &formTk[1] );
	  msg[79] = 0;
          actWin->appCtx->postMessage( msg );
          symbolsFromFile.setRaw( "" );
	}
	else {
	  result = fgets( fileBuf, maxSymbolLen, f );
	  if ( result ) {
            fileContext = NULL;
            fileTk = strtok_r( fileBuf, "\n", &fileContext );
            if ( fileTk ) {
              symbolsFromFile.setRaw( fileTk );
	    }
	    else {
              snprintf( msg, 79, activePipClass_str28, macDefFileName );
              msg[79] = 0;
              actWin->appCtx->postMessage( msg );
              symbolsFromFile.setRaw( "" );
	    }
	  }
	  else {
            if ( errno ) {
              snprintf( msg, 79, activePipClass_str29, macDefFileName );
	    }
	    else {
              snprintf( msg, 79, activePipClass_str28, macDefFileName );
	    }
            msg[79] = 0;
            actWin->appCtx->postMessage( msg );
            symbolsFromFile.setRaw( "" );
	  }
	  fclose( f );
	}
      }
      // append inline list to file contents
      formTk = strtok_r( NULL, "\n", &formContext );
      if ( formTk ) {
        strncpy( fileBuf, symbolsFromFile.getRaw(), maxSymbolLen );
        fileBuf[maxSymbolLen] = 0;
        if ( blank(fileBuf) ) {
          strcpy( fileBuf, "" );
	}
        else {
          Strncat( fileBuf, ",", maxSymbolLen );
	}
	Strncat( fileBuf, formTk, maxSymbolLen );
        symbolsFromFile.setRaw( fileBuf );
      }
      // do special substitutions
      actWin->substituteSpecial( maxSymbolLen, symbolsFromFile.getExpanded(),
       symbolsWithSubs );
      gotSymbolsFromFile = 1;
    }
  }

  if ( !gotSymbolsFromFile ) {
    // do special substitutions
    actWin->substituteSpecial( maxSymbolLen, symbolsExpStr[index].getExpanded(),
     symbolsWithSubs );
  }

  numNewMacros = 0;

  // get info on whether to use the small local array for symbols
  stat = countSymbolsAndValues( symbolsWithSubs, &symbolCount,
   &maxSymbolLength );

  if ( !replaceSymbols[index] ) {

    if ( propagateMacros[index] ) {

      for ( i=0; i<actWin->numMacros; i++ ) {

        l = strlen(actWin->macros[i]);
        if ( l > maxSymbolLength ) maxSymbolLength = l;

        l = strlen(actWin->expansions[i]);
        if ( l > maxSymbolLength ) maxSymbolLength = l;

      }

      symbolCount += actWin->numMacros;

    }
    else {

      for ( i=0; i<actWin->appCtx->numMacros; i++ ) {

        l = strlen(actWin->appCtx->macros[i]);
        if ( l > maxSymbolLength ) maxSymbolLength = l;

        l = strlen(actWin->appCtx->expansions[i]);
        if ( l > maxSymbolLength ) maxSymbolLength = l;

      }

      symbolCount += actWin->appCtx->numMacros;

    }

  }

  useSmallArrays = 1;
  if ( symbolCount > SMALL_SYM_ARRAY_SIZE ) useSmallArrays = 0;
  if ( maxSymbolLength > SMALL_SYM_ARRAY_LEN ) useSmallArrays = 0;

  if ( useSmallArrays ) {

    for ( i=0; i<SMALL_SYM_ARRAY_SIZE; i++ ) {
      newMacros[i] = &smallNewMacros[i][0];
      newValues[i] = &smallNewValues[i][0];
    }

    if ( !replaceSymbols[index] ) {

      if ( propagateMacros[index] ) {

        for ( i=0; i<actWin->numMacros; i++ ) {

          strcpy( newMacros[i], actWin->macros[i] );

          strcpy( newValues[i], actWin->expansions[i] );

          numNewMacros++;

        }

      }
      else {

        for ( i=0; i<actWin->appCtx->numMacros; i++ ) {

          strcpy( newMacros[i], actWin->appCtx->macros[i] );

          strcpy( newValues[i], actWin->appCtx->expansions[i] );

          numNewMacros++;

        }

      }

    }

    max = SMALL_SYM_ARRAY_SIZE - numNewMacros;
    stat = parseLocalSymbolsAndValues( symbolsWithSubs, max,
     SMALL_SYM_ARRAY_LEN, &newMacros[numNewMacros], &newValues[numNewMacros],
     &numFound );
    numNewMacros += numFound;

  }
  else {

    if ( !replaceSymbols[index] ) {

      if ( propagateMacros[index] ) {

        for ( i=0; i<actWin->numMacros; i++ ) {

          l = strlen(actWin->macros[i]) + 1;
          newMacros[i] = (char *) new char[l];
          strcpy( newMacros[i], actWin->macros[i] );

          l = strlen(actWin->expansions[i]) + 1;
          newValues[i] = (char *) new char[l];
          strcpy( newValues[i], actWin->expansions[i] );

          numNewMacros++;

        }

      }
      else {

        for ( i=0; i<actWin->appCtx->numMacros; i++ ) {

          l = strlen(actWin->appCtx->macros[i]) + 1;
          newMacros[i] = (char *) new char[l];
          strcpy( newMacros[i], actWin->appCtx->macros[i] );

          l = strlen(actWin->appCtx->expansions[i]) + 1;
          newValues[i] = (char *) new char[l];
          strcpy( newValues[i], actWin->appCtx->expansions[i] );

          numNewMacros++;

        }

      }

    }

    max = 100 - numNewMacros;
    stat = parseSymbolsAndValues( symbolsWithSubs, max,
     &newMacros[numNewMacros], &newValues[numNewMacros], &numFound );
    numNewMacros += numFound;

  }

  cur = new activeWindowListType;
  actWin->appCtx->addActiveWindow( cur );

  cur->node.createEmbedded( actWin->appCtx, frameWidget, 0, 0, w, h,
   x, y, center, setSize, sizeOfs, noScroll, numNewMacros, newMacros,
   newValues );

  cur->node.realize();

  cur->node.setGraphicEnvironment( &cur->node.appCtx->ci,
   &cur->node.appCtx->fi );

  if ( index < 0 ) index = 0;
  if ( index >= numDsps ) index = numDsps;
  cur->node.storeFileName( displayFileName[index].getExpanded() );

  actWin->appCtx->openActivateActiveWindow( &cur->node, 0, 0 );

  aw = &cur->node;

  aw->parent = actWin;
  (actWin->numChildren)++;

  activateIsComplete = 1;

  if ( !useSmallArrays ) {

    for ( i=0; i<numNewMacros; i++ ) {
      delete[] newMacros[i];
      delete[] newValues[i];
    }

  }

}

void activePipClass::executeDeferred ( void ) {

int iv;
char v[39+1];
char nameWithSubs[maxSymbolLen+1];
int i, nc, nu, nmc, nmu, nd, nfo, nimfo, ncto, nmap, nunmap, okToClose, stat;
activeWindowListPtr cur;
Window root, child;
int rootX, rootY, winX, winY;
unsigned int mask;
XButtonEvent be;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nu = needUpdate; needUpdate = 0;
  nmc = needMenuConnectInit; needMenuConnectInit = 0;
  nmu = needMenuUpdate; needMenuUpdate = 0;
  nd = needDraw; needDraw = 0;
  nfo = needFileOpen; needFileOpen = 0;
  nimfo = needInitMenuFileOpen; needInitMenuFileOpen = 0;
  ncto = needConnectTimeout; needConnectTimeout = 0;
  nmap = needMap; needMap = 0;
  nunmap = needUnmap; needUnmap = 0;
  strncpy( v, curReadV, 39 );
  v[39] = 0;
  iv = curReadIV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    consecutiveDeactivateErrors = 0;

    readPvConnected = 1;
    active = 1;
    init = 1;

    if ( initialReadConnection ) {

      initialReadConnection = 0;

      readPvId->add_value_callback( pip_readUpdate, this );

    }

    fgColor.setConnected();
    drawActive();

  }

  if ( nmc ) {

    consecutiveDeactivateErrors = 0;

    readPvConnected = 1;
    active = 1;
    init = 1;

    if ( initialMenuConnection ) {

      initialMenuConnection = 0;

      readPvId->add_value_callback( pip_menuUpdate, this );

    }
    else {

      activateIsComplete = 1;

    }

    fgColor.setConnected();
    drawActive();

  }

  if ( nu ) {

    strncpy( readV, v, 39 );
    readV[39] = 0;
    //fprintf( stderr, "readV = [%s]\n", readV );

    if ( enabled && !blank( readV ) ) {

      // close old

      if ( aw ) {
        if ( aw->loadFailure ) {
	  aw = NULL;
	  frameWidget = NULL;
	}
      }

      if ( aw ) {

        okToClose = 0;
        cur = actWin->appCtx->head->flink;
        while ( cur != actWin->appCtx->head ) {
          if ( &cur->node == aw ) {
            okToClose = 1;
            break;
          }
          cur = cur->flink;
        }

        if ( okToClose ) {
          if ( !aw->okToDeactivate() ) {
            consecutiveDeactivateErrors++;
            if ( consecutiveDeactivateErrors < MAX_CONSECUTIVE_DEACTIVATE_ERRORS ) {
              if ( !retryTimerNU ) {
                retryTimerNU = appAddTimeOut( actWin->appCtx->appContext(),
                 50, needUpdateTimeout, this );
              }
              return;
            }
            else {
	      //printf( "1\n" );
              actWin->appCtx->postMessage( activePipClass_str30 );
              consecutiveDeactivateErrors = 0;
              return;
            }
          }
          else {
            if ( retryTimerNU ) {
              XtRemoveTimeOut( retryTimerNU );
              retryTimerNU = 0;
            }
          }
	}

      }

      consecutiveDeactivateErrors = 0;

      if ( frameWidget ) {
        if ( *frameWidget ) XtUnmapWidget( *frameWidget );
      }

      if ( aw ) {

        okToClose = 0;
        // make sure the window was successfully opened
        cur = actWin->appCtx->head->flink;
        while ( cur != actWin->appCtx->head ) {
          if ( &cur->node == aw ) {
            okToClose = 1;
            break;
          }
          cur = cur->flink;
        }

        if ( okToClose ) {
          stat = aw->returnToEdit( 1 ); // this frees frameWidget
        }

        aw = NULL;

      }

      if ( frameWidget ) {
        frameWidget = NULL;
      }

      // prevent possible mutual recursion
      if ( actWin->sameAncestorName( readV ) ) {

        actWin->appCtx->postMessage( activePipClass_str26 );
        activateIsComplete = 1;

      }
      else {

        // open new

        if ( !frameWidget ) {
          createPipWidgets();
        }

        if ( !aw ) {

          //fprintf( stderr, "Open file %s\n", readV );

          strncpy( curFileName, readV, 39 );
          curFileName[39] = 0;

          cur = new activeWindowListType;
          actWin->appCtx->addActiveWindow( cur );

          cur->node.createEmbedded( actWin->appCtx, frameWidget, 0, 0, w, h,
           x, y, center, setSize, sizeOfs, noScroll, actWin->numMacros,
           actWin->macros, actWin->expansions );

          cur->node.realize();

          cur->node.setGraphicEnvironment( &cur->node.appCtx->ci,
           &cur->node.appCtx->fi );

          cur->node.storeFileName( readV );

          actWin->appCtx->openActivateActiveWindow( &cur->node, 0, 0 );

          aw = &cur->node;

          aw->parent = actWin;
          (actWin->numChildren)++;

          activateIsComplete = 1;

          drawActive();

        }

      }

    }
    else {

      activateIsComplete = 1;

    }

    if ( !enabled ) { // copy filename to be used when enabled becomes true
      strncpy( curFileName, readV, 39 );
      curFileName[39] = 0;
    }

    activateIsComplete = 1;

  }

//----------------------------------------------------------------------------

  if ( nmu ) {

    i = iv;

    if ( enabled ) {

      if ( i == -1 ) {

        XQueryPointer( actWin->d, XtWindow(actWin->top), &root, &child,
         &rootX, &rootY, &winX, &winY, &mask );
        be.x_root = rootX;
        be.y_root = rootY;
        be.x = 0;
        be.y = 0;
        XmMenuPosition( popUpMenu, &be );
        XtManageChild( popUpMenu );

      }
      else {

        if ( i < numDsps ) {

          if ( !blank( displayFileName[i].getExpanded() ) ) {

            // close old

            if ( aw ) {
	      if ( aw->loadFailure ) {
                aw = NULL;
                frameWidget = NULL;
	      }
	    }

            if ( aw ) {

              okToClose = 0;
              cur = actWin->appCtx->head->flink;
              while ( cur != actWin->appCtx->head ) {
                if ( &cur->node == aw ) {
                  okToClose = 1;
                  break;
                }
                cur = cur->flink;
              }

              if ( okToClose ) {
                if ( !aw->okToDeactivate() ) {
                  consecutiveDeactivateErrors++;
                  if ( consecutiveDeactivateErrors < MAX_CONSECUTIVE_DEACTIVATE_ERRORS ) {
                    if ( !retryTimerNMU ) {
                      retryTimerNMU = appAddTimeOut( actWin->appCtx->appContext(),
                       50, needMenuUpdateTimeout, this );
                    }
                    return;
                  }
                  else {
	            // printf( "2\n" );
                    actWin->appCtx->postMessage( activePipClass_str30 );
                    consecutiveDeactivateErrors = 0;
                    return;
                  }
                }
                else {
                  if ( retryTimerNMU ) {
                    XtRemoveTimeOut( retryTimerNMU );
                    retryTimerNMU = 0;
                  }
                }
	      }

	    }

            consecutiveDeactivateErrors = 0;

            if ( frameWidget ) {
              if ( *frameWidget ) XtUnmapWidget( *frameWidget );
            }

            if ( aw ) {

              okToClose = 0;
              // make sure the window was successfully opened
              cur = actWin->appCtx->head->flink;
              while ( cur != actWin->appCtx->head ) {
                if ( &cur->node == aw ) {
                  okToClose = 1;
                  break;
                }
                cur = cur->flink;
              }

              if ( okToClose ) {
                stat = aw->returnToEdit( 1 ); // this frees frameWidget
              }

              aw = NULL;

            }

            if ( frameWidget ) {
              frameWidget = NULL;
            }

            // prevent possible mutual recursion
            if (actWin->sameAncestorName( displayFileName[i].getExpanded() )) {

              actWin->appCtx->postMessage( activePipClass_str26 );
              activateIsComplete = 1;

            }
            else {

              // open new

              if ( !frameWidget ) {
                createPipWidgets();
              }

              if ( !aw ) {

                //fprintf( stderr, "Open file %s\n", readV );

                strncpy( curFileName, displayFileName[i].getExpanded(), 127 );
                curFileName[127] = 0;

                openEmbeddedByIndex( i );

                if ( labelPvId ) {
		  labelPvId->putText( label[i].getExpanded() );
		}

                drawActive();

              }

            }

          }
	  else {

	    activateIsComplete = 1;

	  }

        }
	else {

	  activateIsComplete = 1;

	}

      }

    }
    else {

      activateIsComplete = 1;

    }

    if ( !enabled ) { // copy filename to be used when enabled becomes true
      strncpy( curFileName, displayFileName[i].getExpanded(), 127 );
      curFileName[127] = 0;
    }

    activateIsComplete = 1;

  }

//----------------------------------------------------------------------------

  if ( nd ) {

    consecutiveDeactivateErrors = 0;

    drawActive();

  }

//----------------------------------------------------------------------------

  if ( nfo ) {

    consecutiveDeactivateErrors = 0;

    if ( enabled && fileExists ) {

      strncpy( curFileName, fileNameExpStr.getExpanded(), 127 );
      curFileName[127] = 0;

      // prevent possible mutual recursion
      if ( actWin->sameAncestorName( curFileName ) ) {

        actWin->appCtx->postMessage( activePipClass_str26 );
        activateIsComplete = 1;

      }
      else {

        if ( !frameWidget ) {
          createPipWidgets();
        }

        if ( !aw ) {

          cur = new activeWindowListType;
          actWin->appCtx->addActiveWindow( cur );

          cur->node.createEmbedded( actWin->appCtx, frameWidget, 0, 0, w, h,
           x, y, center, setSize, sizeOfs, noScroll, actWin->numMacros,
           actWin->macros, actWin->expansions );

          cur->node.realize();

          cur->node.setGraphicEnvironment( &cur->node.appCtx->ci,
           &cur->node.appCtx->fi );

          cur->node.storeFileName( fileNameExpStr.getExpanded() );

          actWin->appCtx->openActivateActiveWindow( &cur->node, 0, 0 );

          aw = &cur->node;

          aw->parent = actWin;
          (actWin->numChildren)++;

          activateIsComplete = 1;

        }

      }

    }

    if ( !enabled ) { // copy filename to be used when enabled becomes true
      strncpy( curFileName, fileNameExpStr.getExpanded(), 127 );
      curFileName[127] = 0;
    }

    activateIsComplete = 1;

  }

//----------------------------------------------------------------------------

  if ( nimfo ) {

    consecutiveDeactivateErrors = 0;

    if ( enabled ) {

      strncpy( curFileName, displayFileName[0].getExpanded(), 127 );
      curFileName[127] = 0;

      // prevent possible mutual recursion
      if ( actWin->sameAncestorName( curFileName ) ) {

        actWin->appCtx->postMessage( activePipClass_str26 );
        activateIsComplete = 1;

      }
      else {

        if ( !frameWidget ) {
          createPipWidgets();
        }

        if ( !aw ) {

	  openEmbeddedByIndex( 0 );

          if ( labelPvId ) {
            labelPvId->putText( label[0].getExpanded() );
	  }
        }

      }

    }

    if ( !enabled ) { // copy filename to be used when enabled becomes true
      strncpy( curFileName, displayFileName[0].getExpanded(), 127 );
      curFileName[127] = 0;
    }

    activateIsComplete = 1;

  }

//----------------------------------------------------------------------------

  if ( ncto ) {

    consecutiveDeactivateErrors = 0;

    activateIsComplete = 1;

    drawActive();

  }

//----------------------------------------------------------------------------

  if ( nunmap ) {

    if ( frameWidget ) {
      if ( *frameWidget ) XtUnmapWidget( *frameWidget );
    }

#if 0
    if ( aw ) {

      okToClose = 0;
      cur = actWin->appCtx->head->flink;
      while ( cur != actWin->appCtx->head ) {
        if ( &cur->node == aw ) {
          okToClose = 1;
          break;
        }
        cur = cur->flink;
      }

      if ( okToClose ) {
        if ( !aw->okToDeactivate() ) {
          consecutiveDeactivateErrors++;
          if ( consecutiveDeactivateErrors < MAX_CONSECUTIVE_DEACTIVATE_ERRORS ) {
            if ( !retryTimerNUM ) {
              retryTimerNUM = appAddTimeOut( actWin->appCtx->appContext(),
               50, needUnmapTimeout, this );
            }
            return;
          }
          else {
	    // printf( "3\n" );
            actWin->appCtx->postMessage( activePipClass_str30 );
            consecutiveDeactivateErrors = 0;
            return;
          }
        }
        else {
          if ( retryTimerNUM ) {
            XtRemoveTimeOut( retryTimerNUM );
            retryTimerNUM = 0;
          }
        }
      }

    }

    consecutiveDeactivateErrors = 0;

    if ( frameWidget ) {
      if ( *frameWidget ) XtUnmapWidget( *frameWidget );
    }

    if ( aw ) {

      okToClose = 0;
      // make sure the window was successfully opened
      cur = actWin->appCtx->head->flink;
      while ( cur != actWin->appCtx->head ) {
        if ( &cur->node == aw ) {
          okToClose = 1;
          break;
        }
        cur = cur->flink;
      }

      if ( okToClose ) {
        stat = aw->returnToEdit( 1 ); // this frees frameWidget
      }

      aw = NULL;

    }

    if ( frameWidget ) {
      frameWidget = NULL;
    }
#endif

  }

//----------------------------------------------------------------------------

  if ( nmap ) {

    if ( frameWidget ) {
      if ( *frameWidget ) XtMapWidget( *frameWidget );
    }
    else {

      // curFileName should contain the file to open

      if ( !blank( curFileName ) ) {

        // close old ( however, one should not be open )

        if ( aw ) {
          if ( aw->loadFailure ) {
            aw = NULL;
            frameWidget = NULL;
          }
        }

        if ( aw ) {

          okToClose = 0;
          cur = actWin->appCtx->head->flink;
          while ( cur != actWin->appCtx->head ) {
            if ( &cur->node == aw ) {
              okToClose = 1;
              break;
            }
            cur = cur->flink;
          }

          if ( okToClose ) {
            if ( !aw->okToDeactivate() ) {
              consecutiveDeactivateErrors++;
              if ( consecutiveDeactivateErrors < MAX_CONSECUTIVE_DEACTIVATE_ERRORS ) {
                if ( !retryTimerNM ) {
                  retryTimerNM = appAddTimeOut( actWin->appCtx->appContext(),
                   50, needMapTimeout, this );
                }
                return;
              }
              else {
                //printf( "4\n" );
                actWin->appCtx->postMessage( activePipClass_str30 );
                consecutiveDeactivateErrors = 0;
                return;
              }
            }
	    else {
	      if ( retryTimerNM ) {
                XtRemoveTimeOut( retryTimerNM );
                retryTimerNM = 0;
              }
            }
          }

        }

        consecutiveDeactivateErrors = 0;

        if ( frameWidget ) {
          if ( *frameWidget ) XtUnmapWidget( *frameWidget );
        }

        if ( aw ) {

          okToClose = 0;
          // make sure the window was successfully opened
          cur = actWin->appCtx->head->flink;
          while ( cur != actWin->appCtx->head ) {
            if ( &cur->node == aw ) {
              okToClose = 1;
              break;
            }
            cur = cur->flink;
          }

          if ( okToClose ) {
            stat = aw->returnToEdit( 1 ); // this frees frameWidget
          }

          aw = NULL;

        }

        if ( frameWidget ) {
          frameWidget = NULL;
        }

        // prevent possible mutual recursion
        if ( actWin->sameAncestorName( curFileName ) ) {

          actWin->appCtx->postMessage( activePipClass_str26 );
          activateIsComplete = 1;

        }
        else {

          // open new

          if ( !frameWidget ) {
            createPipWidgets();
          }

          if ( !aw ) {

            if ( displaySource == 2 ) { // menu

              if ( iv < 0 ) iv = 0;
              if ( iv > numDsps ) iv = 0;
              openEmbeddedByIndex( iv );

            }
            else {

              cur = new activeWindowListType;
              actWin->appCtx->addActiveWindow( cur );

              cur->node.createEmbedded( actWin->appCtx, frameWidget, 0, 0, w, h,
               x, y, center, setSize, sizeOfs, noScroll, actWin->numMacros,
               actWin->macros, actWin->expansions );

              cur->node.realize();

              cur->node.setGraphicEnvironment( &cur->node.appCtx->ci,
               &cur->node.appCtx->fi );

              cur->node.storeFileName( curFileName );

              actWin->appCtx->openActivateActiveWindow( &cur->node, 0, 0 );

              aw = &cur->node;

              aw->parent = actWin;
              (actWin->numChildren)++;

            }

            drawActive();

          }

        }

      }

    }

    activateIsComplete = 1;

  }

//----------------------------------------------------------------------------

}

void activePipClass::changeDisplayParams (
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
    fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topShadowColor.setColorIndex( actWin->defaultTopShadowColor, actWin->ci );

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botShadowColor.setColorIndex( actWin->defaultBotShadowColor, actWin->ci );

}

void activePipClass::changePvNames (
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

  if ( flag & ACTGRF_READBACKPVS_MASK ) {
    if ( numReadbackPvs ) {
      readPvExpStr.setRaw( readbackPvs[0] );
    }
  }

}

int activePipClass::isWindowContainer ( void ) {

  return 1;

}

int activePipClass::activateBeforePreReexecuteComplete ( void ) {

  if ( ignoreMultiplexors ) {
    return 1;
  }

  return activateComplete();

}

int activePipClass::activateComplete ( void ) {

int flag;

  if ( aw ) {

    if ( aw->loadFailure ) {
      activateIsComplete = 1;
    }

  }

  if ( !activateIsComplete ) return 0;

  // this fails when I return to edit while things are updating
  if ( aw ) {
    flag = aw->okToDeactivate();
  }
  else {
    flag = 1;
  }

  return flag;

}

void activePipClass::map ( void ) {

  needMap = 1;
  actWin->addDefExeNode( aglPtr );

}

void activePipClass::unmap ( void ) {

  needUnmap = 1;
  actWin->addDefExeNode( aglPtr );

}

char *activePipClass::getSearchString (
  int i
) {

int index, ii;

  if ( i == 0 ) {
    return readPvExpStr.getRaw();
  }
  else if ( i == 1 ) {
    return labelPvExpStr.getRaw();
  }
  else if ( i == 2 ) {
    return fileNameExpStr.getRaw();
  }
  else if ( ( i > 2 ) && ( i < numDsps * 3 + 3 ) ) {
    ii = i % 3;
    index = ( i - 3 ) / 3;
    if ( ii == 0 ) {
      return symbolsExpStr[index].getRaw();
    }
    else if ( ii == 1 ) {
      return label[index].getRaw();
    }
    else if ( ii == 2 ) {
      return displayFileName[index].getRaw();
    }
  }

  return NULL;

}

void activePipClass::replaceString (
  int i,
  int max,
  char *string
) {

int index, ii;

  if ( i == 0 ) {
    readPvExpStr.setRaw( string );
  }
  else if ( i == 1 ) {
    labelPvExpStr.setRaw( string );
  }
  else if ( i == 2 ) {
    fileNameExpStr.setRaw( string );
  }
  else if ( ( i > 2 ) && ( i < numDsps * 3 + 3 ) ) {
    ii = i % 3;
    index = ( i - 3 ) / 3;
    if ( ii == 0 ) {
      symbolsExpStr[index].setRaw( string );
    }
    else if ( ii == 1 ) {
      label[index].setRaw( string );
    }
    else if ( ii == 2 ) {
      displayFileName[index].setRaw( string );
    }
  }

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activePipClassPtr ( void ) {

activePipClass *ptr;

  ptr = new activePipClass;
  return (void *) ptr;

}

void *clone_activePipClassPtr (
  void *_srcPtr )
{

activePipClass *ptr, *srcPtr;

  srcPtr = (activePipClass *) _srcPtr;

  ptr = new activePipClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
