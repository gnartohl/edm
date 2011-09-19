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

#define __menu_mux_cc 1

#include "menu_mux.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) {

menuMuxClass *mmo = (menuMuxClass *) ptr;

  if ( !mmo->activeMode ) {
    if ( mmo->isSelected() ) mmo->drawSelectBoxCorners(); // erase via xor
    mmo->smartDrawAll();
    if ( mmo->isSelected() ) mmo->drawSelectBoxCorners();
  }
  else {
    mmo->bufInvalidate();
    mmo->needDraw = 1;
    mmo->actWin->addDefExeNode( mmo->aglPtr );
  }

}

static void retryTimeout (
  XtPointer client,
  XtIntervalId *id )
{

menuMuxClass *mmo = (menuMuxClass *) client;

  mmo->actWin->appCtx->proc->lock();
  mmo->needUpdate = 1;
  mmo->actWin->addDefExeNode( mmo->aglPtr );
  mmo->actWin->appCtx->proc->unlock();

  mmo->retryTimer = 0;

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

menuMuxClass *mmo = (menuMuxClass *) client;

  if ( !mmo->controlPvConnected ) {
    if ( mmo->controlExists ) {
      mmo->actWin->appCtx->proc->lock();
      mmo->needToDrawUnconnected = 1;
      mmo->needDraw = 1;
      mmo->actWin->addDefExeNode( mmo->aglPtr );
      mmo->actWin->appCtx->proc->unlock();
    }
  }

  mmo->unconnectedTimer = 0;

}

static void mmuxSetItem (
  Widget w,
  XtPointer client,
  XtPointer call )
{

efSetItemCallbackDscPtr dsc = (efSetItemCallbackDscPtr) client;
entryFormClass *ef = (entryFormClass *) dsc->ef;
menuMuxClass *mmo = (menuMuxClass *) dsc->obj;
int i;

  mmo->elbt->setValue( mmo->eBuf->bufTag[ef->index] );

  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    mmo->elbm[i]->setValue( mmo->eBuf->bufM[ef->index][i] );
    mmo->elbe[i]->setValue( mmo->eBuf->bufE[ef->index][i] );
  }

}

static void mmux_putValueNoPv (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;
int i;

  if ( !mmuxo->active ) return;

  for ( i=0; i<mmuxo->numStates; i++ ) {

    if ( w == mmuxo->pb[i] ) {

      mmuxo->actWin->appCtx->proc->lock();

      mmuxo->curControlV = i;

      if ( mmuxo->curControlV < 0 )
        mmuxo->curControlV = 0;
      else if ( mmuxo->curControlV >= mmuxo->numStates )
        mmuxo->curControlV = mmuxo->numStates - 1;

      mmuxo->needUpdate = 1;

      mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );

      mmuxo->actWin->appCtx->proc->unlock();

    }

  }

}

static void mmux_putValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;
int i, value;

  for ( i=0; i<mmuxo->numStates; i++ ) {

    if ( w == mmuxo->pb[i] ) {
      value = i;
      //mmuxo->controlPvId->put(
      // XDisplayName(mmuxo->actWin->appCtx->displayName),
      // value );
      mmuxo->controlPvId->put( value );
      return;
    }

  }

}

static void mmux_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

menuMuxClass *mmuxo = (menuMuxClass *) userarg;

  if ( pv->is_valid() ) {

    mmuxo->needConnectInit = 1;

  }
  else {

    mmuxo->needDisconnect = 1;

  }

  mmuxo->actWin->appCtx->proc->lock();
  mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );
  mmuxo->actWin->appCtx->proc->unlock();

}

static void mmux_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

menuMuxClass *mmuxo = (menuMuxClass *) userarg;
int st, sev;

  if ( !mmuxo->active ) return;

  mmuxo->actWin->appCtx->proc->lock();

  mmuxo->curControlV = pv->get_int();
  if ( mmuxo->curControlV < 0 )
    mmuxo->curControlV = 0;
  else if ( mmuxo->curControlV >= mmuxo->numStates )
    mmuxo->curControlV = mmuxo->numStates - 1;

  st = pv->get_status();
  sev = pv->get_severity();
  if ( ( st != mmuxo->oldStat ) || ( sev != mmuxo->oldSev ) ) {
    mmuxo->oldStat = st;
    mmuxo->oldSev = sev;
    mmuxo->fgColor.setStatus( st, sev );
    mmuxo->bgColor.setStatus( st, sev );
    mmuxo->bufInvalidate();
    mmuxo->needDraw = 1;
  }

  mmuxo->needUpdate = 1;
  mmuxo->actWin->addDefExeNode( mmuxo->aglPtr );

  mmuxo->actWin->appCtx->proc->unlock();

}

static void mmuxc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;
int i, ii;

  mmuxo->actWin->setChanged();

  mmuxo->eraseSelectBoxCorners();
  mmuxo->erase();

  strncpy( mmuxo->fontTag, mmuxo->fm.currentFontTag(), 63 );
  mmuxo->fontTag[63] = 0;
  mmuxo->actWin->fi->loadFontTag( mmuxo->fontTag );
  mmuxo->actWin->drawGc.setFontTag( mmuxo->fontTag, mmuxo->actWin->fi );
  mmuxo->actWin->fi->getTextFontList( mmuxo->fontTag, &mmuxo->fontList );
  mmuxo->fs = mmuxo->actWin->fi->getXFontStruct( mmuxo->fontTag );

  mmuxo->topShadowColor = mmuxo->eBuf->bufTopShadowColor;
  mmuxo->botShadowColor = mmuxo->eBuf->bufBotShadowColor;

  mmuxo->fgColorMode = mmuxo->eBuf->bufFgColorMode;
  if ( mmuxo->fgColorMode == MMUXC_K_COLORMODE_ALARM )
    mmuxo->fgColor.setAlarmSensitive();
  else
    mmuxo->fgColor.setAlarmInsensitive();
  mmuxo->fgColor.setColorIndex( mmuxo->eBuf->bufFgColor, mmuxo->actWin->ci );

  mmuxo->bgColorMode = mmuxo->eBuf->bufBgColorMode;
  if ( mmuxo->bgColorMode == MMUXC_K_COLORMODE_ALARM )
    mmuxo->bgColor.setAlarmSensitive();
  else
    mmuxo->bgColor.setAlarmInsensitive();
  mmuxo->bgColor.setColorIndex( mmuxo->eBuf->bufBgColor, mmuxo->actWin->ci );

  mmuxo->x = mmuxo->eBuf->bufX;
  mmuxo->sboxX = mmuxo->eBuf->bufX;

  mmuxo->y = mmuxo->eBuf->bufY;
  mmuxo->sboxY = mmuxo->eBuf->bufY;

  mmuxo->w = mmuxo->eBuf->bufW;
  mmuxo->sboxW = mmuxo->eBuf->bufW;

  mmuxo->h = mmuxo->eBuf->bufH;
  mmuxo->sboxH = mmuxo->eBuf->bufH;

  mmuxo->controlPvExpStr.setRaw( mmuxo->eBuf->bufControlPvName );

  mmuxo->initialStateExpStr.setRaw( mmuxo->eBuf->bufInitialState );

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strncpy( mmuxo->tag[i], mmuxo->eBuf->bufTag[i], MMUX_MAX_STRING_SIZE );
    mmuxo->tag[i][MMUX_MAX_STRING_SIZE] = 0;
    if ( strlen(mmuxo->tag[i]) == 0 ) {
      strcpy( mmuxo->tag[i], "?" );
    }
  }

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strncpy( mmuxo->m[i][ii], mmuxo->eBuf->bufM[i][ii], MMUX_MAX_STRING_SIZE );
      mmuxo->m[i][ii][MMUX_MAX_STRING_SIZE] = 0;
      strncpy( mmuxo->e[i][ii], mmuxo->eBuf->bufE[i][ii], MMUX_MAX_STRING_SIZE );
      mmuxo->e[i][ii][MMUX_MAX_STRING_SIZE] = 0;
    }
  }

  mmuxo->numItems = mmuxo->ef.numItems;

  mmuxo->updateDimensions();

}

static void mmuxc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;

  mmuxc_edit_update( w, client, call );
  mmuxo->refresh( mmuxo );

}

static void mmuxc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;

  mmuxc_edit_update( w, client, call );
  mmuxo->ef.popdown();
  mmuxo->operationComplete();

}

static void mmuxc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;

  mmuxo->ef.popdown();
  mmuxo->operationCancel();

}

static void mmuxc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

menuMuxClass *mmuxo = (menuMuxClass *) client;

  mmuxo->ef.popdown();
  mmuxo->operationCancel();
  mmuxo->erase();
  mmuxo->deleteRequest = 1;
  mmuxo->drawAll();

}

menuMuxClass::menuMuxClass ( void ) {

int i, ii;

  name = new char[strlen("menuMuxClass")+1];
  strcpy( name, "menuMuxClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  numStates = 0;

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    stateString[i] = NULL;
    pb[i] = NULL;
  }

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strcpy( tag[i], "" );
  }
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strcpy( m[i][ii], "" );
      strcpy( e[i][ii], "" );
    }
  }

  numItems = 2;
  numMac = 0;
  mac = NULL;
  exp = NULL;

  fgColorMode = MMUXC_K_COLORMODE_STATIC;
  bgColorMode = MMUXC_K_COLORMODE_STATIC;

  active = 0;
  activeMode = 0;
  widgetsCreated = 0;
  fontList = NULL;
  unconnectedTimer = retryTimer = 0;

  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

}

// copy constructor
menuMuxClass::menuMuxClass
 ( const menuMuxClass *source ) {

int i, ii;
activeGraphicClass *mmuxo = (activeGraphicClass *) this;

  mmuxo->clone( (activeGraphicClass *) source );

  name = new char[strlen("menuMuxClass")+1];
  strcpy( name, "menuMuxClass" );

  numItems = source->numItems;

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    stateString[i] = NULL;
    pb[i] = NULL;
  }

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strncpy( tag[i], source->tag[i], MMUX_MAX_STRING_SIZE );
    tag[i][MMUX_MAX_STRING_SIZE] = 0;
  }
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strncpy( m[i][ii], source->m[i][ii], MMUX_MAX_STRING_SIZE );
      m[i][ii][MMUX_MAX_STRING_SIZE] = 0;
      strncpy( e[i][ii], source->e[i][ii], MMUX_MAX_STRING_SIZE );
      e[i][ii][MMUX_MAX_STRING_SIZE] = 0;
    }
  }

  numMac = 0;
  mac = NULL;
  exp = NULL;

  strncpy( fontTag, source->fontTag, 63 );
  fontTag[63] = 0;
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);

  fgColorMode = source->fgColorMode;
  bgColorMode = source->bgColorMode;

  controlPvExpStr.copy ( source->controlPvExpStr );

  initialStateExpStr.copy( source->initialStateExpStr );

  widgetsCreated = 0;
  active = 0;
  activeMode = 0;
  unconnectedTimer = retryTimer = 0;

  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

  doAccSubs( controlPvExpStr );
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    doAccSubs( tag[i], MMUX_MAX_STRING_SIZE );
  }
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      doAccSubs( m[i][ii], MMUX_MAX_STRING_SIZE );
      doAccSubs( e[i][ii], MMUX_MAX_STRING_SIZE );
    }
  }

}

menuMuxClass::~menuMuxClass ( void ) {

int i;

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  if ( retryTimer ) {
    XtRemoveTimeOut( retryTimer );
    retryTimer = 0;
  }

  if ( mac && exp ) {
    for ( i=0; i<numMac; i++ ) {
      if ( mac[i] ) {
	delete[] mac[i];
      }
      if ( exp[i] ) {
	delete[] exp[i];
      }
    }
  }
  if ( mac ) delete[] mac;
  if ( exp ) delete[] exp;

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    if ( stateString[i] ) delete[] stateString[i];
  }

  if ( fontList ) XmFontListFree( fontList );

  updateBlink( 0 );

}

int menuMuxClass::createInteractive (
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

  strncpy( fontTag, actWin->defaultBtnFontTag, 63 );
  fontTag[63] = 0;
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int menuMuxClass::save (
  FILE *f )
{

int i, ii, stat, major, minor, release;
char tmpS[MMUX_MAX_ENTRIES][15+1], tmpV[MMUX_MAX_ENTRIES][15+1];
char tmpBufS[MMUX_MAX_ENTRIES][MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE+1];
char tmpBufV[MMUX_MAX_ENTRIES][MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE+1];

tagClass itemTag;

int zero = 0;
char *emptyStr = "";

  major = MMUXC_MAJOR_VERSION;
  minor = MMUXC_MINOR_VERSION;
  release = MMUXC_RELEASE;

  itemTag.init();
  itemTag.loadW( "beginObjectProperties" );
  itemTag.loadW( "major", &major );
  itemTag.loadW( "minor", &minor );
  itemTag.loadW( "release", &release );
  itemTag.loadW( "x", &x );
  itemTag.loadW( "y", &y );
  itemTag.loadW( "w", &w );
  itemTag.loadW( "h", &h );
  itemTag.loadW( "fgColor", actWin->ci, &fgColor );
  itemTag.loadBoolW( "fgAlarm", &fgColorMode, &zero );
  itemTag.loadW( "bgColor", actWin->ci, &bgColor );
  itemTag.loadBoolW( "bgAlarm", &bgColorMode, &zero );
  itemTag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  itemTag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  itemTag.loadW( "controlPv", &controlPvExpStr, emptyStr );
  itemTag.loadW( "font", fontTag );
  itemTag.loadW( "initialState", &initialStateExpStr, emptyStr );
  itemTag.loadW( "numItems", &numItems );
  itemTag.loadW( "symbolTag", MMUX_MAX_STRING_SIZE+1, tag[0], numItems,
   emptyStr );
  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_STATES; ii++ ) {
      strcpy( tmpBufS[i][ii], m[ii][i] );
      strcpy( tmpBufV[i][ii], e[ii][i] );
    }
  }
  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    snprintf( tmpS[i], 15, "symbol%-d", i );
    itemTag.loadW( tmpS[i], MMUX_MAX_STRING_SIZE+1, tmpBufS[i][0], numItems,
     emptyStr );
    snprintf( tmpV[i], 15, "value%-d", i );
    itemTag.loadW( tmpV[i], MMUX_MAX_STRING_SIZE+1, tmpBufV[i][0], numItems,
     emptyStr );
  }
  itemTag.loadW( unknownTags );
  itemTag.loadW( "endObjectProperties" );
  itemTag.loadW( "" );

  stat = itemTag.writeTags( f );

  return stat;

}

int menuMuxClass::old_save (
  FILE *f )
{

int index, i, ii;

  fprintf( f, "%-d %-d %-d\n", MMUXC_MAJOR_VERSION, MMUXC_MINOR_VERSION,
   MMUXC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  fprintf( f, "%-d\n", fgColorMode );

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

  fprintf( f, "%-d\n", numItems );

  for ( i=0; i<numItems; i++ ) {
    writeStringToFile( f, tag[i] );
  }

  for ( i=0; i<numItems; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      writeStringToFile( f, m[i][ii] );
      writeStringToFile( f, e[i][ii] );
    }
  }

  // version 1.2.0
  if ( initialStateExpStr.getRaw() )
    writeStringToFile( f, initialStateExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  return 1;

}

int menuMuxClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, ii, n, stat, major, minor, release;
char tmpS[MMUX_MAX_ENTRIES][15+1], tmpV[MMUX_MAX_ENTRIES][15+1];
char tmpBufS[MMUX_MAX_ENTRIES][MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE+1];
char tmpBufV[MMUX_MAX_ENTRIES][MMUX_MAX_STATES][MMUX_MAX_STRING_SIZE+1];

tagClass itemTag;

int zero = 0;
char *emptyStr = "";

  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_STATES; ii++ ) {
      strcpy( tmpBufS[i][ii], "" );
      strcpy( tmpBufV[i][ii], "" );
    }
  }

  this->actWin = _actWin;

  itemTag.init();
  itemTag.loadR( "beginObjectProperties" );
  itemTag.loadR( unknownTags );
  itemTag.loadR( "major", &major );
  itemTag.loadR( "minor", &minor );
  itemTag.loadR( "release", &release );
  itemTag.loadR( "x", &x );
  itemTag.loadR( "y", &y );
  itemTag.loadR( "w", &w );
  itemTag.loadR( "h", &h );
  itemTag.loadR( "fgColor", actWin->ci, &fgColor );
  itemTag.loadR( "fgAlarm", &fgColorMode, &zero );
  itemTag.loadR( "bgColor", actWin->ci, &bgColor );
  itemTag.loadR( "bgAlarm", &bgColorMode, &zero );
  itemTag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  itemTag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  itemTag.loadR( "controlPv", &controlPvExpStr, emptyStr );
  itemTag.loadR( "font", 63, fontTag );
  itemTag.loadR( "initialState", &initialStateExpStr, emptyStr );
  itemTag.loadR( "numItems", &numItems );
  itemTag.loadR( "symbolTag", MMUX_MAX_STATES, MMUX_MAX_STRING_SIZE+1, tag[0],
   &numItems, emptyStr );
  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    snprintf( tmpS[i], 15, "symbol%-d", i );
    itemTag.loadR( tmpS[i], MMUX_MAX_STATES, MMUX_MAX_STRING_SIZE+1,
     tmpBufS[i][0], &n, emptyStr );
    snprintf( tmpV[i], 15, "value%-d", i );
    itemTag.loadR( tmpV[i], MMUX_MAX_STATES, MMUX_MAX_STRING_SIZE+1,
     tmpBufV[i][0], &n, emptyStr );
  }
  itemTag.loadR( "endObjectProperties" );

  stat = itemTag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( itemTag.errMsg() );
  }

  if ( major > MMUXC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_STATES; ii++ ) {
      strcpy( m[ii][i], tmpBufS[i][ii] );
      strcpy( e[ii][i], tmpBufV[i][ii] );
    }
  }

  if ( fgColorMode == MMUXC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  if ( bgColorMode == MMUXC_K_COLORMODE_ALARM )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  numMac = 0;
  mac = NULL;
  exp = NULL;

  return stat;

}

int menuMuxClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, i, ii, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > MMUXC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == MMUXC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

    if ( bgColorMode == MMUXC_K_COLORMODE_ALARM )
      bgColor.setAlarmSensitive();
    else
      bgColor.setAlarmInsensitive();

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

    if ( fgColorMode == MMUXC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

    if ( bgColorMode == MMUXC_K_COLORMODE_ALARM )
      bgColor.setAlarmSensitive();
    else
      bgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    topShadowColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    botShadowColor = index;

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == MMUXC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

    if ( bgColorMode == MMUXC_K_COLORMODE_ALARM )
      bgColor.setAlarmSensitive();
    else
      bgColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    topShadowColor = actWin->ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 1 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    botShadowColor = actWin->ci->pixIndex( pixel );

  }

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strcpy( tag[i], "" );
  }
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strcpy( m[i][ii], "" );
      strcpy( e[i][ii], "" );
    }
  }

  fscanf( f, "%d\n", &numItems ); actWin->incLine();

  for ( i=0; i<numItems; i++ ) {
    readStringFromFile( tag[i], MMUX_MAX_STRING_SIZE+1, f ); actWin->incLine();
  }

  for ( i=0; i<numItems; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      readStringFromFile( m[i][ii], MMUX_MAX_STRING_SIZE+1, f );
      actWin->incLine();
      readStringFromFile( e[i][ii], MMUX_MAX_STRING_SIZE+1, f );
      actWin->incLine();
    }
  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( oneName, 39+1, f ); actWin->incLine();
    initialStateExpStr.setRaw( oneName );
  }
  else {
    initialStateExpStr.setRaw( "0" );
  }

  numMac = 0;
  mac = NULL;
  exp = NULL;

  return 1;

}

int menuMuxClass::genericEdit ( void ) {

int i, ii;
char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "menuMuxClass" );
  if ( ptr ) {
    strncpy( title, ptr, 31 );
  }
  else {
    strncpy( title, menuMuxClass_str2, 31 );
  }
  title[31] = 0;

  Strncat( title, menuMuxClass_str3, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufTopShadowColor = topShadowColor;
  eBuf->bufBotShadowColor = botShadowColor;

  eBuf->bufFgColor = fgColor.pixelIndex();
  eBuf->bufFgColorMode = fgColorMode;

  eBuf->bufBgColor = bgColor.pixelIndex();
  eBuf->bufBgColorMode = bgColorMode;

  if ( controlPvExpStr.getRaw() ) {
    strncpy( eBuf->bufControlPvName, controlPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  }
  else {
    strcpy( eBuf->bufControlPvName, "" );
  }
  eBuf->bufControlPvName[PV_Factory::MAX_PV_NAME] = 0;

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    strncpy( eBuf->bufTag[i], tag[i], MMUX_MAX_STRING_SIZE );
    eBuf->bufTag[i][MMUX_MAX_STRING_SIZE] = 0;
  }
  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      strncpy( eBuf->bufM[i][ii], m[i][ii], MMUX_MAX_STRING_SIZE );
      eBuf->bufM[i][ii][MMUX_MAX_STRING_SIZE] = 0;
      strncpy( eBuf->bufE[i][ii], e[i][ii], MMUX_MAX_STRING_SIZE );
      eBuf->bufE[i][ii][MMUX_MAX_STRING_SIZE] = 0;
    }
  }

  if ( initialStateExpStr.getRaw() ) {
    strncpy( eBuf->bufInitialState, initialStateExpStr.getRaw(), 15 );
  }
  else {
    strncpy( eBuf->bufInitialState, "0", 15 );
  }
  eBuf->bufInitialState[15] = 0;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, MMUX_MAX_STATES, numItems,
   mmuxSetItem, (void *) this, NULL, NULL, NULL );

  ef.addTextField( menuMuxClass_str4, 35, &eBuf->bufX );
  ef.addTextField( menuMuxClass_str5, 35, &eBuf->bufY );
  ef.addTextField( menuMuxClass_str6, 35, &eBuf->bufW );
  ef.addTextField( menuMuxClass_str7, 35, &eBuf->bufH );

  ef.addTextField( menuMuxClass_str17, 35, eBuf->bufControlPvName,
   PV_Factory::MAX_PV_NAME );
  pvNameEntry = ef.getCurItem();
  ef.addTextField( menuMuxClass_str18, 35, eBuf->bufInitialState, 30 );
  iniStateEntry = ef.getCurItem();
  pvNameEntry->addInvDependency( iniStateEntry );
  pvNameEntry->addDependencyCallbacks();

  ef.addColorButton( menuMuxClass_str8, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addToggle( menuMuxClass_str10, &eBuf->bufFgColorMode );
  ef.addColorButton( menuMuxClass_str11, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  ef.addColorButton( menuMuxClass_str14, actWin->ci, &eBuf->topShadowCb,
   &eBuf->bufTopShadowColor );
  ef.addColorButton( menuMuxClass_str15, actWin->ci, &eBuf->botShadowCb,
   &eBuf->bufBotShadowColor );

  ef.addFontMenu( menuMuxClass_str16, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    tagPtr[i] = (char *) &eBuf->bufTag[i];
  }

  for ( i=0; i<MMUX_MAX_STATES; i++ ) {
    for ( ii=0; ii<MMUX_MAX_ENTRIES; ii++ ) {
      mPtr[ii][i] = (char *) &eBuf->bufM[i][ii];
      ePtr[ii][i] = (char *) &eBuf->bufE[i][ii];
    }
  }

  ef.addTextFieldArray( menuMuxClass_str19, 35, tagPtr, MMUX_MAX_STRING_SIZE, &elbt );

  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    ef.addTextFieldArray( menuMuxClass_str20, 35, mPtr[i], MMUX_MAX_STRING_SIZE,
     &elbm[i] );
    ef.addTextFieldArray( menuMuxClass_str21, 35, ePtr[i], MMUX_MAX_STRING_SIZE,
     &elbe[i] );
  }

  return 1;

}

int menuMuxClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( mmuxc_edit_ok, mmuxc_edit_apply, mmuxc_edit_cancel_delete,
   this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int menuMuxClass::edit ( void ) {

  this->genericEdit();
  ef.finished( mmuxc_edit_ok, mmuxc_edit_apply, mmuxc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int menuMuxClass::erase ( void ) {

  if ( deleteRequest || activeMode ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int menuMuxClass::eraseActive ( void ) {

  if ( !activeMode ) return 1;

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int menuMuxClass::draw ( void ) {

int tX, tY, bumpX, bumpY;
XRectangle xR = { x+3, y, w-23, h };
int blink = 0;

  if ( deleteRequest || activeMode ) return 1;

  actWin->drawGc.saveFg();

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

    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2 - 10;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
     XmALIGNMENT_CENTER, "Mux" );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int menuMuxClass::drawActive ( void ) {

int tX, tY, bumpX, bumpY;
XRectangle xR = { x+3, y, w-23, h };
int blink = 0;
char string[MMUX_MAX_STRING_SIZE+1];

  if ( !controlPvConnected ) {
    if ( controlExists ) {
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
  }

  if ( !activeMode || !widgetsCreated ) return 1;

  actWin->executeGc.saveFg();
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setFG( bgColor.getIndex(), &blink );

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

    actWin->executeGc.setFG( fgColor.getIndex(), &blink );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2 - 10;
    tY = y + h/2 - fontAscent/2;

    if ( ( controlV >= 0 ) && ( controlV < numStates ) ) {
      strncpy( string, tag[controlV], MMUX_MAX_STRING_SIZE );
    }
    else {
      strcpy( string, "?" );
    }
    string[MMUX_MAX_STRING_SIZE] = 0;

    drawText( actWin->executeWidget, drawable(actWin->executeWidget),
     &actWin->executeGc, fs, tX, tY, XmALIGNMENT_CENTER, string );

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  updateBlink( blink );

  return 1;

}

int menuMuxClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( controlPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  controlPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( initialStateExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  initialStateExpStr.setRaw( tmpStr.getExpanded() );

  return 1;

}

int menuMuxClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand1st( numMacros, macros, expansions );
  stat = initialStateExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int menuMuxClass::getNumMacroSets ( void ) {

  return numItems;

}

int menuMuxClass::getMacrosSet (
  int *numMacros,
  char ***macro,
  char ***expansion,
  int n
) {

int i, ii, count;

  if ( ( n < 0 ) || ( n >= numItems ) ) {
    *numMacros = 0;
    *macro = NULL;
    *expansion = NULL;
  }

// count number of non-null entries
  count = 0;
  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    if ( ( strcmp( m[n][i], "" ) != 0 ) &&
         ( strcmp( e[n][i], "" ) != 0 ) ) {
      count++;
    }
  }

  if ( numMac < count ) {

    for ( i=0; i<numMac; i++ ) {
      if ( mac[i] ) {
        delete[] mac[i];
        mac[i] = NULL;
      }
      if ( exp[i] ) {
        delete[] exp[i];
        exp[i] = NULL;
      }
    }
    if ( mac ) {
      delete[] mac;
      mac = NULL;
    }
    if ( exp ) {
      delete[] exp;
      exp = NULL;
    }

    numMac = count;

    mac = new char*[numMac];
    exp = new char*[numMac];

    for ( i=0; i<numMac; i++ ) {
      mac[i] = new char[MMUX_MAX_STRING_SIZE+1];
      exp[i] = new char[MMUX_MAX_STRING_SIZE+1];
    }

  }

  // populate ptr arrays
  ii = 0;
  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    if ( ( strcmp( m[n][i], "" ) != 0 ) &&
         ( strcmp( e[n][i], "" ) != 0 ) ) {
      strncpy( mac[ii], m[n][i], MMUX_MAX_STRING_SIZE );
      mac[ii][MMUX_MAX_STRING_SIZE] = 0;
      strncpy( exp[ii], e[n][i], MMUX_MAX_STRING_SIZE );
      exp[ii][MMUX_MAX_STRING_SIZE] = 0;
      ii++;
    }
  }

  *numMacros = count;
  *macro = mac;
  *expansion = exp;

  return 1;

}

int menuMuxClass::getMacros (
  int *numMacros,
  char ***macro,
  char ***expansion ) {

int i, ii, n, count;

  if ( controlV < 0 )
    n = 0;
  else if ( controlV >= numItems )
    n = numItems - 1;
  else
    n = controlV;

// count number of non-null entries
  count = 0;
  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    if ( ( strcmp( m[n][i], "" ) != 0 ) &&
         ( strcmp( e[n][i], "" ) != 0 ) ) {
      count++;
    }
  }

  if ( numMac < count ) {

    for ( i=0; i<numMac; i++ ) {
      if ( mac[i] ) {
        delete[] mac[i];
        mac[i] = NULL;
      }
      if ( exp[i] ) {
        delete[] exp[i];
        exp[i] = NULL;
      }
    }
    if ( mac ) {
      delete[] mac;
      mac = NULL;
    }
    if ( exp ) {
      delete[] exp;
      exp = NULL;
    }

    numMac = count;

    mac = new char*[numMac];
    exp = new char*[numMac];

    for ( i=0; i<numMac; i++ ) {
      mac[i] = new char[MMUX_MAX_STRING_SIZE+1];
      exp[i] = new char[MMUX_MAX_STRING_SIZE+1];
    }

  }

  // populate ptr arrays
  ii = 0;
  for ( i=0; i<MMUX_MAX_ENTRIES; i++ ) {
    if ( ( strcmp( m[n][i], "" ) != 0 ) &&
         ( strcmp( e[n][i], "" ) != 0 ) ) {
      strncpy( mac[ii], m[n][i], MMUX_MAX_STRING_SIZE );
      mac[ii][MMUX_MAX_STRING_SIZE] = 0;
      strncpy( exp[ii], e[n][i], MMUX_MAX_STRING_SIZE );
      exp[ii][MMUX_MAX_STRING_SIZE] = 0;
      ii++;
    }
  }

  *numMacros = count;
  *macro = mac;
  *expansion = exp;

  return 1;

}

int menuMuxClass::activate (
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

      aglPtr = ptr;
      needConnectInit = needDisconnect = needInfoInit = needUpdate =
       needDraw = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = retryTimer = 0;
      widgetsCreated = 0;
      firstEvent = 1;
      controlV = 0;
      buttonPressed = 0;
      initialConnection = 1;
      oldStat = oldSev = -1;

      controlPvConnected = active = 0;
      activeMode = 1;
      controlPvId = NULL;

      popUpMenu = (Widget) NULL;

      // if ( strcmp( controlPvExpStr.getExpanded(), "" ) != 0 )
      if ( !blankOrComment( controlPvExpStr.getExpanded() ) )
        controlExists = 1;
      else
        controlExists = 0;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      opStat = 1;

      if ( controlExists ) {

	controlPvId = the_PV_Factory->create( controlPvExpStr.getExpanded() );
        if ( controlPvId ) {
	  controlPvId->add_conn_state_callback(
           mmux_monitor_control_connect_state, this );
	}
	else {
          fprintf( stderr, menuMuxClass_str23 );
          opStat = 0;
        }

      }
      else {

        actWin->appCtx->proc->lock();
        if ( initialStateExpStr.getExpanded() ) {
          curControlV = atol( initialStateExpStr.getExpanded() );
	}
	else {
          curControlV = 0;
	}
        needInfoInit = 1;
        actWin->addDefExeNode( aglPtr );
        actWin->appCtx->proc->unlock();

      }

      if ( !( opStat & 1 ) ) opComplete = 1;

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

int menuMuxClass::deactivate (
  int pass
) {

int i;

  active = 0;
  activeMode = 0;

  if ( pass == 1 ) {

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    //updateBlink( 0 );

    if ( controlExists ) {
      if ( controlPvId ) {
        controlPvId->remove_conn_state_callback(
         mmux_monitor_control_connect_state, this );
        controlPvId->remove_value_callback(
         mmux_controlUpdate, this );
	controlPvId->release();
        controlPvId = NULL;
      }
    }

  }
  else if ( pass == 2 ) {

    if ( widgetsCreated ) {
      for ( i=0; i<numStates; i++ ) {
        XtDestroyWidget( pb[i] );
      }
      XtDestroyWidget( pullDownMenu );
      XtDestroyWidget( popUpMenu );
      widgetsCreated = 0;
    }

  }

  return 1;

}

void menuMuxClass::updateDimensions ( void )
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

void menuMuxClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( !buttonPressed ) return;

  buttonPressed = 0;

  if ( buttonNumber == 1 ) {

    XmMenuPosition( popUpMenu, be );
    XtManageChild( popUpMenu );

  }

}

void menuMuxClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( controlExists ) {
    if ( !controlPvId->have_write_access() ) return;
  }

  if ( buttonNumber == 1 ) {
    buttonPressed = 1;
  }

}

void menuMuxClass::pointerIn (
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled ) return;

  if ( controlExists ) {
    if ( !controlPvId->have_write_access() ) {
      actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_NO );
    }
    else {
      actWin->cursor.set( XtWindow(actWin->executeWidget), CURSOR_K_DEFAULT );
    }
  }

  activeGraphicClass::pointerIn( _x, _y, buttonState );

}

int menuMuxClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;
  *focus = 1;
  *down = 1;
  *up = 1;

  return 1;

}

#if 0
static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

class menuMuxClass *mmo;
int stat;

  XtVaGetValues( w, XmNuserData, &mmo, NULL );

  stat = mmo->startDrag( w, e );

}

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

class menuMuxClass *mmo;
int stat;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &mmo, NULL );

  stat = mmo->selectDragValue( be );

}
#endif

void menuMuxClass::executeDeferred ( void ) {

int v;
int stat, i, nc, ndis, ni, nu, nd;
XmString str;
Arg args[15];
int n;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  ndis = needDisconnect; needDisconnect = 0;
  ni = needInfoInit; needInfoInit = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  v = curControlV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    v = curControlV = controlPvId->get_int();

    controlPvConnected = 1;
    fgColor.setConnected();

    ni = 1;

  }

//----------------------------------------------------------------------------

  if ( ndis ) {

    controlPvConnected = 0;
    fgColor.setDisconnected();
    active = 0;

    if ( widgetsCreated ) {

      for ( i=0; i<numStates; i++ ) {
        XtDestroyWidget( pb[i] );
      }
      XtDestroyWidget( pullDownMenu );
      XtDestroyWidget( popUpMenu );

      widgetsCreated = 0;

    }

  }

//----------------------------------------------------------------------------

  if ( ni ) {

    controlV = v;

    if ( widgetsCreated ) {

      for ( i=0; i<numStates; i++ ) {
        XtDestroyWidget( pb[i] );
      }
      XtDestroyWidget( pullDownMenu );
      XtDestroyWidget( popUpMenu );

      widgetsCreated = 0;

    }

    n = 0;
    XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
    popUpMenu = XmCreatePopupMenu( actWin->topWidgetId(), "", args, n );

    pullDownMenu = XmCreatePulldownMenu( popUpMenu, "", NULL, 0 );

    numStates = numItems;

    for ( i=0; i<numStates; i++ ) {

      stateString[i] = new char[strlen(tag[i])+1];
      strncpy( stateString[i], tag[i], strlen(tag[i]) );
      stateString[i][strlen(tag[i])] = 0;

      str = XmStringCreate( stateString[i], fontTag );

      pb[i] = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
       popUpMenu,
       XmNlabelString, str,
       NULL );

      XmStringFree( str );

      if ( controlExists ) {
        XtAddCallback( pb[i], XmNactivateCallback, mmux_putValue,
         (XtPointer) this );
      }
      else {
        XtAddCallback( pb[i], XmNactivateCallback, mmux_putValueNoPv,
         (XtPointer) this );
      }

    }

    widgetsCreated = 1;

    active = 1;

    if ( controlExists ) {

      if ( initialConnection ) {

        initialConnection = 0;

        controlPvId->add_value_callback( mmux_controlUpdate, this );

      }

    }
    else {

      firstEvent = 0;

    }

    nu = 1;

  }

//----------------------------------------------------------------------------

  if ( nu ) {

    controlV = v;
    stat = drawActive();

    if ( !firstEvent ) {
      if ( actWin->okToPreReexecute() ) {
        if ( retryTimer ) {
          XtRemoveTimeOut( retryTimer );
          retryTimer = 0;
        }
        stat = actWin->preReexecute();
        if ( stat & 1 ) {
          actWin->setNoRefresh();
          actWin->appCtx->reactivateActiveWindow( actWin );
        }
      }
      else {
        if ( !retryTimer ) {
          retryTimer = appAddTimeOut( actWin->appCtx->appContext(),
           50, retryTimeout, this );
        }
      }
    }
    firstEvent = 0;

  }

//----------------------------------------------------------------------------

  if ( nd ) {
    controlV = v;
    drawActive();
  }

//----------------------------------------------------------------------------

}

char *menuMuxClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *menuMuxClass::nextDragName ( void ) {

  return NULL;

}

char *menuMuxClass::dragValue (
  int i ) {

  if ( actWin->mode == AWC_EXECUTE ) {

    return controlPvExpStr.getExpanded();

  }
  else {

    return controlPvExpStr.getRaw();

  }

}

void menuMuxClass::changeDisplayParams (
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

void menuMuxClass::changePvNames (
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

}

void menuMuxClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 1 ) {
    *n = 0;
    return;
  }

  *n = 1;
  pvs[0] = controlPvId;

}

char *menuMuxClass::getSearchString (
  int i
) {

int n1 = 1 + MMUX_MAX_STATES;
int n2 = 1 + MMUX_MAX_STATES + MMUX_MAX_STATES * MMUX_MAX_ENTRIES;
int ii, selector, index, index1, index2;

  if ( i == 0 ) {
    return controlPvExpStr.getRaw();
  }
  else if ( ( i > 0 ) && ( i < n1 ) ) {
    index = i - 1;
    return tag[index];
  }
  else if ( ( i >= n1 ) && ( i < n2 ) ) {
    ii = i - n1 - 1;
    index2 = ( ii / 2 ) % MMUX_MAX_ENTRIES;
    selector = ii % 2;
    index1 = ii / MMUX_MAX_ENTRIES / 2;
    if ( selector == 0 ) {
      return m[index1][index2];
    }
    else {
      return e[index1][index2];
    }
  }

  return NULL;

}

void menuMuxClass::replaceString (
  int i,
  int max,
  char *string
) {

int n1 = 1 + MMUX_MAX_STATES;
int n2 = 1 + MMUX_MAX_STATES + MMUX_MAX_STATES * MMUX_MAX_ENTRIES;
int ii, selector, index, index1, index2;

  if ( i == 0 ) {
    controlPvExpStr.setRaw( string );
  }
  else if ( ( i > 0 ) && ( i < n1 ) ) {
    index = i - 1;
    int l = max;
    if ( MMUX_MAX_STRING_SIZE < max ) l = MMUX_MAX_STRING_SIZE;
    strncpy( tag[index], string, l );
    tag[index][l] = 0;
  }
  else if ( ( i >= n1 ) && ( i < n2 ) ) {
    ii = i - n1 - 1;
    index2 = ( ii / 2 ) % MMUX_MAX_ENTRIES;
    selector = ii % 2;
    index1 = ii / MMUX_MAX_ENTRIES / 2;
    if ( selector == 0 ) {
      int l = max;
      if ( MMUX_MAX_STRING_SIZE < max ) l = MMUX_MAX_STRING_SIZE;
      strncpy( m[index1][index2], string, l );
      m[index1][index2][l] = 0;
    }
    else {
      int l = max;
      if ( MMUX_MAX_STRING_SIZE < max ) l = MMUX_MAX_STRING_SIZE;
      strncpy( e[index1][index2], string, l );
      e[index1][index2][l] = 0;
    }
  }

}

// crawler functions may return blank pv names
char *menuMuxClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return controlPvExpStr.getExpanded();

}

char *menuMuxClass::crawlerGetNextPv ( void ) {

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_menuMuxClassPtr ( void ) {

menuMuxClass *ptr;

  ptr = new menuMuxClass;

  return (void *) ptr;

}

void *clone_menuMuxClassPtr (
  void *_srcPtr )
{

menuMuxClass *ptr, *srcPtr;

  srcPtr = (menuMuxClass *) _srcPtr;

  ptr = new menuMuxClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
