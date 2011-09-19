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

#define __radio_button_cc 1

#include "radio_button.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

#include "Xm/CascadeBG.h"

static int g_transInit = 1;
static XtTranslations g_parsedTrans;

static char g_dragTrans[] =
  "#override\n\
   ~Ctrl~Shift<Btn2Down>: startDrag()\n\
   Ctrl~Shift<Btn2Down>: dummy()\n\
   Ctrl~Shift<Btn2Up>: selectActions()\n\
   Shift Ctrl<Btn2Down>: pvInfo()\n\
   Shift~Ctrl<Btn2Down>: dummy()\n\
   Shift~Ctrl<Btn2Up>: selectDrag()";

static XtActionsRec g_dragActions[] = {
  { "startDrag", (XtActionProc) drag },
  { "pvInfo", (XtActionProc) pvInfo },
  { "dummy", (XtActionProc) dummy },
  { "selectActions", (XtActionProc) selectActions },
  { "selectDrag", (XtActionProc) selectDrag }
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  if ( !rbto->connection.pvsConnected() ) {
    rbto->needToDrawUnconnected = 1;
    rbto->needDraw = 1;
    rbto->actWin->addDefExeNode( rbto->aglPtr );
  }

  rbto->unconnectedTimer = 0;

}

static void radioBoxEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;
int b2Op;
XButtonEvent *be;

  *continueToDispatch = True;

  if ( !rbto->active ) return;

  if ( e->type == EnterNotify ) {
    if ( rbto->controlPvId ) {
      if ( !rbto->controlPvId->have_write_access() ) {
        rbto->actWin->cursor.set( XtWindow(rbto->actWin->executeWidget),
         CURSOR_K_NO );
      }
      else {
        rbto->actWin->cursor.set( XtWindow(rbto->actWin->executeWidget),
         CURSOR_K_DEFAULT );
      }
    }
  }

  if ( e->type == LeaveNotify ) {
    rbto->actWin->cursor.set( XtWindow(rbto->actWin->executeWidget),
     CURSOR_K_DEFAULT );
  }

  // allow Button2 operations when no write access
  b2Op = 0;
  if ( ( e->type == ButtonPress ) || ( e->type == ButtonRelease ) ) {
    be = (XButtonEvent *) e;
    if ( be->button == Button2 ) {
      b2Op = 1;
    }
  }

  if ( rbto->controlPvId ) {
    if ( !rbto->controlPvId->have_write_access() && !b2Op ) {
      *continueToDispatch = False;
    }
  }

}

static void putValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;
int i;

  if ( rbto->firstValueChange ) {
    rbto->firstValueChange = 0;
    return;
  }

  for ( i=0; i<(int)rbto->controlPvId->get_enum_count(); i++ ) {

    if ( w == rbto->pb[i] ) {
      if ( rbto->curValue != i ) {
        rbto->curValue = i;
        rbto->controlPvId->put(
         XDisplayName(rbto->actWin->appCtx->displayName), rbto->curValue );
      }
      break;
    }

  }

#if 0
  rbto->actWin->appCtx->proc->lock();
  rbto->needRefresh = 1;
  rbto->needDraw = 1;
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();
#endif

}

static void rbt_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) userarg;

  if ( pv->is_valid() ) {

    rbto->connection.setPvConnected( rbto->controlPvId );
    rbto->needConnectInit = 1;

    if ( rbto->connection.pvsConnected() ) {
      rbto->fgColor.setConnected();
    }

  }
  else {

    rbto->connection.setPvDisconnected( rbto->controlPvId );
    rbto->fgColor.setDisconnected();
    rbto->needRefresh = 1;
    rbto->needDraw = 1;
    rbto->active = 0;

  }

  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbt_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) userarg;
int st, sev;

  rbto->curValue = (short) pv->get_int();

  st = pv->get_status();
  sev = pv->get_severity();
  if ( ( st != rbto->oldStat ) || ( sev != rbto->oldSev ) ) {
    rbto->oldStat = st;
    rbto->oldSev = sev;
    rbto->fgColor.setStatus( st, sev );
    rbto->bufInvalidate();
  }

  rbto->needRefresh = 1;
  rbto->needDraw = 1;
  rbto->actWin->appCtx->proc->lock();
  rbto->actWin->addDefExeNode( rbto->aglPtr );
  rbto->actWin->appCtx->proc->unlock();

}

static void rbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  rbto->actWin->setChanged();

  rbto->eraseSelectBoxCorners();
  rbto->erase();

  strncpy( rbto->fontTag, rbto->fm.currentFontTag(), 63 );
  rbto->actWin->fi->loadFontTag( rbto->fontTag );
  rbto->actWin->drawGc.setFontTag( rbto->fontTag, rbto->actWin->fi );
  rbto->actWin->fi->getTextFontList( rbto->fontTag, &rbto->fontList );
  rbto->fs = rbto->actWin->fi->getXFontStruct( rbto->fontTag );

  rbto->buttonColor = rbto->bufButtonColor;
  rbto->topShadowColor = rbto->bufTopShadowColor;
  rbto->botShadowColor = rbto->bufBotShadowColor;
  rbto->selectColor = rbto->bufSelectColor;

  rbto->fgColorMode = rbto->bufFgColorMode;
  if ( rbto->fgColorMode == RBTC_K_COLORMODE_ALARM )
    rbto->fgColor.setAlarmSensitive();
  else
    rbto->fgColor.setAlarmInsensitive();
  rbto->fgColor.setColorIndex( rbto->bufFgColor, rbto->actWin->ci );

  rbto->bgColorMode = rbto->bufBgColorMode;
  if ( rbto->bgColorMode == RBTC_K_COLORMODE_ALARM )
    rbto->bgColor.setAlarmSensitive();
  else
    rbto->bgColor.setAlarmInsensitive();
  rbto->bgColor.setColorIndex( rbto->bufBgColor, rbto->actWin->ci );

  rbto->x = rbto->bufX;
  rbto->sboxX = rbto->bufX;

  rbto->y = rbto->bufY;
  rbto->sboxY = rbto->bufY;

  rbto->w = rbto->bufW;
  rbto->sboxW = rbto->bufW;

  rbto->h = rbto->bufH;
  rbto->sboxH = rbto->bufH;

  rbto->controlPvExpStr.setRaw( rbto->eBuf->bufControlPvName );

  rbto->updateDimensions();

}

static void rbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  rbtc_edit_update ( w, client, call );
  rbto->refresh( rbto );

}

static void rbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  rbtc_edit_update ( w, client, call );
  rbto->ef.popdown();
  rbto->operationComplete();

}

static void rbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  rbto->ef.popdown();
  rbto->operationCancel();

}

static void rbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeRadioButtonClass *rbto = (activeRadioButtonClass *) client;

  rbto->ef.popdown();
  rbto->operationCancel();
  rbto->erase();
  rbto->deleteRequest = 1;
  rbto->drawAll();

}

activeRadioButtonClass::activeRadioButtonClass ( void ) {

int i;

  name = new char[strlen("activeRadioButtonClass")+1];
  strcpy( name, "activeRadioButtonClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    pb[i] = NULL;
  }

  fgColorMode = RBTC_K_COLORMODE_STATIC;
  bgColorMode = RBTC_K_COLORMODE_STATIC;

  active = 0;
  activeMode = 0;
  widgetsCreated = 0;

  fontList = NULL;

  connection.setMaxPvs( 2 );

  unconnectedTimer = 0;

  eBuf = NULL;

}

activeRadioButtonClass::~activeRadioButtonClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( fontList ) XmFontListFree( fontList );

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

// copy constructor
activeRadioButtonClass::activeRadioButtonClass
 ( const activeRadioButtonClass *source ) {

int i;
activeGraphicClass *rbto = (activeGraphicClass *) this;

  rbto->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeRadioButtonClass")+1];
  strcpy( name, "activeRadioButtonClass" );

  for ( i=0; i<MAX_ENUM_STATES; i++ ) {
    pb[i] = NULL;
  }

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;

  buttonColor = source->buttonColor;
  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;
  selectColor = source->selectColor;

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);

  fgColorMode = source->fgColorMode;
  bgColorMode = source->bgColorMode;

  controlPvExpStr.setRaw( source->controlPvExpStr.rawString );

  widgetsCreated = 0;
  active = 0;
  activeMode = 0;

  connection.setMaxPvs( 1 );

  unconnectedTimer = 0;

  eBuf = NULL;

  doAccSubs( controlPvExpStr );

}

int activeRadioButtonClass::createInteractive (
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

  buttonColor = actWin->defaultOffsetColor;
  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;
  selectColor = actWin->defaultFg1Color;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int activeRadioButtonClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
char *emptyStr = "";

  major = RBTC_MAJOR_VERSION;
  minor = RBTC_MINOR_VERSION;
  release = RBTC_RELEASE;

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
  tag.loadW( "buttonColor", actWin->ci, &buttonColor );
  tag.loadW( "selectColor", actWin->ci, &selectColor );
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "controlPv", &controlPvExpStr, emptyStr );
  tag.loadW( "font", fontTag );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeRadioButtonClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", RBTC_MAJOR_VERSION, RBTC_MINOR_VERSION,
   RBTC_RELEASE );

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

  index = buttonColor;
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

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

  // version 1.2.0
  index = selectColor;
  actWin->ci->writeColorIndex( f, index );

  return 1;

}

int activeRadioButtonClass::createFromFile (
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
  tag.loadR( "buttonColor", actWin->ci, &buttonColor );
  tag.loadR( "selectColor", actWin->ci, &selectColor );
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "controlPv", &controlPvExpStr, emptyStr );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > RBTC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( fgColorMode == RBTC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  if ( bgColorMode == RBTC_K_COLORMODE_ALARM )
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

int activeRadioButtonClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int index;
int major, minor, release;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > RBTC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 0 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == RBTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

    if ( bgColorMode == RBTC_K_COLORMODE_ALARM )
      bgColor.setAlarmSensitive();
    else
      bgColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    buttonColor = index;

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

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == RBTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

    if ( bgColorMode == RBTC_K_COLORMODE_ALARM )
      bgColor.setAlarmSensitive();
    else
      bgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    buttonColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    topShadowColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    botShadowColor = index;

  }

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 1 ) ) ) {
    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    selectColor = index;
  }
  else {
    selectColor = buttonColor;
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return 1;

}

int activeRadioButtonClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeRadioButtonClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeRadioButtonClass_str2, 31 );

  Strncat( title, activeRadioButtonClass_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufButtonColor = buttonColor;
  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;
  bufSelectColor = selectColor;

  bufFgColor = fgColor.pixelIndex();
  bufFgColorMode = fgColorMode;

  bufBgColor = bgColor.pixelIndex();
  bufBgColorMode = bgColorMode;

  if ( controlPvExpStr.getRaw() )
    strncpy( eBuf->bufControlPvName, controlPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufControlPvName, "" );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeRadioButtonClass_str4, 35, &bufX );
  ef.addTextField( activeRadioButtonClass_str5, 35, &bufY );
  ef.addTextField( activeRadioButtonClass_str6, 35, &bufW );
  ef.addTextField( activeRadioButtonClass_str7, 35, &bufH );
  ef.addTextField( activeRadioButtonClass_str17, 35, eBuf->bufControlPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addColorButton( activeRadioButtonClass_str8, actWin->ci,
   &eBuf->fgCb, &bufFgColor );
  ef.addToggle( activeRadioButtonClass_str10, &bufFgColorMode );

  ef.addColorButton( activeRadioButtonClass_str11, actWin->ci,
   &eBuf->bgCb, &bufBgColor );

  ef.addColorButton( activeRadioButtonClass_str28, actWin->ci,
   &eBuf->buttonCb, &bufButtonColor );

  ef.addColorButton( activeRadioButtonClass_str29, actWin->ci,
   &eBuf->selectCb, &bufSelectColor );

  ef.addColorButton( activeRadioButtonClass_str14, actWin->ci,
   &eBuf->topShadowCb, &bufTopShadowColor );

  ef.addColorButton( activeRadioButtonClass_str15, actWin->ci,
   &eBuf->botShadowCb, &bufBotShadowColor );

  ef.addFontMenu( activeRadioButtonClass_str16, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeRadioButtonClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( rbtc_edit_ok, rbtc_edit_apply, rbtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeRadioButtonClass::edit ( void ) {

  this->genericEdit();
  ef.finished( rbtc_edit_ok, rbtc_edit_apply, rbtc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeRadioButtonClass::erase ( void ) {

  if ( deleteRequest || activeMode ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeRadioButtonClass::eraseActive ( void ) {

  return 1;

}

int activeRadioButtonClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };

  actWin->drawGc.saveFg();

  if ( deleteRequest || activeMode ) return 1;

  actWin->drawGc.setFG( bgColor.pixelColor() );
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
	      XmALIGNMENT_CENTER, activeRadioButtonClass_str19 );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeRadioButtonClass::drawActive ( void ) {

  if ( !connection.pvsConnected() ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( fgColor.getDisconnected() );
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

  return 1;

}

int activeRadioButtonClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( controlPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  controlPvExpStr.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeRadioButtonClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeRadioButtonClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeRadioButtonClass::containsMacros ( void ) {

  if ( controlPvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeRadioButtonClass::activate (
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
      needConnectInit = needInfoInit = needRefresh = needDraw = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;
      controlPvId = NULL;
      firstValueChange = 1;

      active = 0;
      activeMode = 1;
      curValue = 0;

      bulBrd = (Widget) NULL;
      radioBox = (Widget) NULL;

      controlExists = 0;
      pvCheckExists = 0;
      connection.init();
      initialConnection = 1;

      initEnable();

      oldStat = -1;
      oldSev = -1;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      if ( !pvCheckExists ) {

        pvCheckExists = 1;

        //if ( strcmp( controlPvExpStr.getExpanded(), "" ) != 0 ) {
	if ( !blankOrComment( controlPvExpStr.getExpanded() ) ) {
          controlExists = 1;
          connection.addPv(); // must do this only once per pv
	}
        else {
          controlExists = 0;
	}

      }

      opStat = 1;

      if ( controlExists ) {

	controlPvId = the_PV_Factory->create( controlPvExpStr.getExpanded() );
        if ( controlPvId ) {
	  controlPvId->add_conn_state_callback(
           rbt_monitor_control_connect_state, this );
	}
	else {
          fprintf( stderr, activeRadioButtonClass_str20,
           controlPvExpStr.getExpanded() );
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

int activeRadioButtonClass::deactivate (
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

    if ( controlExists ) {
      if ( controlPvId ) {
        controlPvId->remove_conn_state_callback(
         rbt_monitor_control_connect_state, this );
        controlPvId->remove_value_callback(
         rbt_controlUpdate, this );
	controlPvId->release();
        controlPvId = NULL;
      }
    }

    break;

  case 2:

    if ( widgetsCreated ) {
      if ( bulBrd ) {
        XtUnmapWidget( bulBrd );
        XtDestroyWidget( radioBox );
        radioBox = NULL;
        XtDestroyWidget( bulBrd );
        bulBrd = NULL;
      }
      widgetsCreated = 0;
    }

    break;

  }

  return 1;

}

void activeRadioButtonClass::updateDimensions ( void )
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

static void dummy (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

}

static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeRadioButtonClass *rbto;
int stat;

  XtVaGetValues( w, XmNuserData, &rbto, NULL );

  stat = rbto->startDrag( w, e );

}

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeRadioButtonClass *rbto;
int stat;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &rbto, NULL );

  stat = rbto->selectDragValue( be );

}

static void selectActions (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeRadioButtonClass *rbto;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &rbto, NULL );

  rbto->doActions( be, be->x, be->y );

}

static void pvInfo (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeRadioButtonClass *rbto;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &rbto, NULL );

  rbto->showPvInfo( be, be->x, be->y );

}

void activeRadioButtonClass::executeDeferred ( void ) {

short value;
int i, nc, ni, nr, nd, notify;
XmString str;
Arg args[20];
int n;
char msg[79+1];

  if ( actWin->isIconified ) return;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  nd = needDraw; needDraw = 0;
  value = curValue;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    if ( controlPvId->get_type().type != ProcessVariable::Type::enumerated ) {
      strncpy( msg, actWin->obj.getNameFromClass( "activeRadioButtonClass" ),
       79 );
      Strncat( msg, activeRadioButtonClass_str30, 79 );
      actWin->appCtx->postMessage( msg );
      connection.setPvDisconnected( controlPvId );
      needToDrawUnconnected = 1;
      drawActive();
      return;
    }

    value = curValue = (short) controlPvId->get_int();

    ni = 1;

  }

  if ( ni ) {

    if ( widgetsCreated ) {
      if ( bulBrd ) {
        XtUnmapWidget( bulBrd );
        XtDestroyWidget( radioBox );
        radioBox = NULL;
        XtDestroyWidget( bulBrd );
        bulBrd = NULL;
      }
      widgetsCreated = 0;
    }

    bulBrd = XtVaCreateWidget( "", xmBulletinBoardWidgetClass,
    actWin->executeWidgetId(),
     XmNx, x,
     XmNy, y,
     XmNwidth, w,
     XmNheight, h,
     XmNbackground, (XtArgVal) bgColor.getColor(),
     XmNforeground, (XtArgVal) fgColor.getColor(),
     XmNmarginHeight, 0,
     XmNmarginWidth, 0,
     NULL );

    if ( g_transInit ) {
      g_transInit = 0;
      g_parsedTrans = XtParseTranslationTable( g_dragTrans );
    }
    actWin->appCtx->addActions( g_dragActions, XtNumber(g_dragActions) );

    n = 0;
    XtSetArg( args[n], XmNx, (XtArgVal) 0 ); n++;
    XtSetArg( args[n], XmNy, (XtArgVal) 0 ); n++;
    XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
    XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
    XtSetArg( args[n], XmNbackground, (XtArgVal) bgColor.getColor() ); n++;
    XtSetArg( args[n], XmNforeground, (XtArgVal) fgColor.getColor() ); n++;
    XtSetArg( args[n], XmNtranslations, g_parsedTrans ); n++;
    XtSetArg( args[n], XmNnavigationType, XmNONE ); n++;
    XtSetArg( args[n], XmNtraversalOn, False ); n++;
    XtSetArg( args[n], XmNuserData, this ); n++;
    XtSetArg( args[n], XmNmarginHeight, 0 ); n++;
    XtSetArg( args[n], XmNmarginWidth, 0 ); n++;
    XtSetArg( args[n], XmNspacing, 0 ); n++;
    XtSetArg( args[n], XmNentryBorder, 0 ); n++;

    radioBox = XmCreateRadioBox( bulBrd,
     "", args, n );
    
    for ( i=0; i<(int) controlPvId->get_enum_count(); i++ ) {

      str = XmStringCreate( (char *) controlPvId->get_enum( i ), fontTag );

      pb[i] = XtVaCreateManagedWidget( "", xmToggleButtonWidgetClass,
       radioBox,
       XmNlabelString, str,
       XmNfontList, fontList,
       XmNforeground, (XtArgVal) fgColor.getColor(),
       XmNbackground, (XtArgVal) bgColor.getColor(),
       XmNselectColor, (XtArgVal) actWin->ci->pix(buttonColor),
       XmNunselectColor, (XtArgVal) actWin->ci->pix(buttonColor),
       XmNtopShadowColor, (XtArgVal) actWin->ci->pix(topShadowColor),
       XmNbottomShadowColor, (XtArgVal) actWin->ci->pix(botShadowColor),
       XmNhighlightOnEnter, False,
       XmNindicatorType, XmN_OF_MANY,
       XmNindicatorOn, XmINDICATOR_CHECK_BOX,
       XmNtranslations, g_parsedTrans,
       XmNuserData, this,
       XmNmarginHeight, 0,
       XmNmarginWidth, 0,
       XmNselectColor, actWin->ci->pix(selectColor),
       NULL );

      XtAddCallback( pb[i], XmNvalueChangedCallback, putValue,
       (XtPointer) this );

      XmStringFree( str );

    }

{

WidgetList children;
Cardinal numChildren;
int ii;

    XtAddEventHandler( radioBox,
     ButtonPressMask|ButtonReleaseMask|EnterWindowMask|LeaveWindowMask,
     False, radioBoxEventHandler, (XtPointer) this );

    XtVaGetValues( radioBox,
     XmNnumChildren, &numChildren,
     XmNchildren, &children,
     NULL );

    for ( ii=0; ii<(int)numChildren; ii++ ) {

      XtAddEventHandler( children[ii],
       ButtonPressMask|ButtonReleaseMask|EnterWindowMask,
       False, radioBoxEventHandler, (XtPointer) this );

    }

}

    XtManageChild( radioBox );
    XtManageChild( bulBrd );

    widgetsCreated = 1;

    if ( bulBrd ) {
      if ( !enabled ) XtUnmapWidget( bulBrd );
    }

    if ( initialConnection ) {

      initialConnection = 0;
      
      controlPvId->add_value_callback( rbt_controlUpdate, this );

    }

    active = 1;

    for ( i=0; i<(int) controlPvId->get_enum_count(); i++ ) {
      if ( i == value )
        XmToggleButtonSetState( pb[i], (XtArgVal) True, (XtArgVal) True );
      else
        XmToggleButtonSetState( pb[i], (XtArgVal) False, (XtArgVal) True );
    }

    drawActive();

    nr = 1;

  }

//----------------------------------------------------------------------------

  if ( nr ) {

    notify = False;

    for ( i=0; i<(int) controlPvId->get_enum_count(); i++ ) {
      if ( i == value ) {
        XmToggleButtonSetState( pb[i], (XtArgVal) True, (XtArgVal) False );
        n = 0;
        XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;
        XtSetValues( pb[i], args, n );
      }
      else {
        XmToggleButtonSetState( pb[i], (XtArgVal) False, (XtArgVal) False );
        n = 0;
        XtSetArg( args[n], XmNforeground, fgColor.pixelColor() ); n++;
        XtSetValues( pb[i], args, n );
      }
    }

  }

//----------------------------------------------------------------------------

  if ( nd ) {
    drawActive();
  }

//----------------------------------------------------------------------------

}

char *activeRadioButtonClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeRadioButtonClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  return NULL;

}

char *activeRadioButtonClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    return controlPvExpStr.getExpanded();

  }
  else {

    return controlPvExpStr.getRaw();

  }

}

void activeRadioButtonClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_FG1COLOR_MASK )
    selectColor = _fg1Color;

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    buttonColor = _fg1Color;

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

void activeRadioButtonClass::changePvNames (
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

void activeRadioButtonClass::map ( void ) {

  if ( bulBrd ) {
    XtMapWidget( bulBrd );
  }

}

void activeRadioButtonClass::unmap ( void ) {

  if ( bulBrd ) {
    XtUnmapWidget( bulBrd );
  }

}

void activeRadioButtonClass::getPvs (
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

char *activeRadioButtonClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return controlPvExpStr.getRaw();
  }

  return NULL;

}

void activeRadioButtonClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    controlPvExpStr.setRaw( string );
  }

}

// crawler functions may return blank pv names
char *activeRadioButtonClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return controlPvExpStr.getExpanded();

}

char *activeRadioButtonClass::crawlerGetNextPv ( void ) {

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeRadioButtonClassPtr ( void ) {

activeRadioButtonClass *ptr;

  ptr = new activeRadioButtonClass;
  return (void *) ptr;

}

void *clone_activeRadioButtonClassPtr (
  void *_srcPtr )
{

activeRadioButtonClass *ptr, *srcPtr;

  srcPtr = (activeRadioButtonClass *) _srcPtr;

  ptr = new activeRadioButtonClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
