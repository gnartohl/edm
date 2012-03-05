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

#define __x_text_obj_cc 1

#include "x_text_obj.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) {

activeXTextClass *axto = (activeXTextClass *) ptr;

  if ( !axto->activeMode ) {
    if ( axto->isSelected() ) axto->drawSelectBoxCorners(); // erase via xor
    axto->smartDrawAll();
    if ( axto->isSelected() ) axto->drawSelectBoxCorners();
  }
  else {
    axto->bufInvalidate();
    axto->smartDrawAllActive();
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeXTextClass *axto = (activeXTextClass *) client;

  if ( !axto->init ) {
    axto->needToDrawUnconnected = 1;
    axto->needRefresh = 1;
    axto->actWin->addDefExeNode( axto->aglPtr );
  }

  axto->unconnectedTimer = 0;

}

static void axtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextClass *axto = (activeXTextClass *) client;

  axto->actWin->setChanged();

  axto->eraseSelectBoxCorners();
  axto->erase();

  strncpy( axto->id, axto->bufId, 31 );

  axto->fgColorMode = axto->eBuf->bufFgColorMode;
  if ( axto->fgColorMode == AXTC_K_COLORMODE_ALARM )
    axto->fgColor.setAlarmSensitive();
  else
    axto->fgColor.setAlarmInsensitive();
  axto->fgColor.setColorIndex( axto->eBuf->bufFgColor, axto->actWin->ci );

  axto->bgColorMode = axto->eBuf->bufBgColorMode;
  if ( axto->bgColorMode == AXTC_K_COLORMODE_ALARM )
    axto->bgColor.setAlarmSensitive();
  else
    axto->bgColor.setAlarmInsensitive();
  axto->bgColor.setColorIndex( axto->eBuf->bufBgColor, axto->actWin->ci );

  axto->alarmPvExpStr.setRaw( axto->eBuf->bufAlarmPvName );

  axto->visPvExpStr.setRaw( axto->eBuf->bufVisPvName );

  if ( axto->eBuf->bufVisInverted )
    axto->visInverted = 0;
  else
    axto->visInverted = 1;

  strncpy( axto->minVisString, axto->eBuf->bufMinVisString, 39 );
  strncpy( axto->maxVisString, axto->eBuf->bufMaxVisString, 39 );

  if ( axto->bufValue ) {
    axto->value.setRaw( axto->bufValue );
  }

  strncpy( axto->fontTag, axto->fm.currentFontTag(), 63 );
  axto->actWin->fi->loadFontTag( axto->fontTag );
  axto->actWin->drawGc.setFontTag( axto->fontTag, axto->actWin->fi );

  axto->stringLength = strlen( axto->value.getRaw() );

  axto->fs = axto->actWin->fi->getXFontStruct( axto->fontTag );

  axto->updateFont( axto->value.getRaw(), axto->fontTag, &axto->fs,
   &axto->fontAscent, &axto->fontDescent, &axto->fontHeight,
   &axto->stringWidth );

  axto->useDisplayBg = axto->eBuf->bufUseDisplayBg;

  axto->autoSize = axto->eBuf->bufAutoSize;

  axto->border = axto->eBuf->bufBorder;
  axto->lineThk = axto->eBuf->bufLineThk;

  axto->x = axto->eBuf->bufX;
  axto->sboxX = axto->eBuf->bufX;

  axto->y = axto->eBuf->bufY;
  axto->sboxY = axto->eBuf->bufY;

  axto->w = axto->eBuf->bufW;
  axto->sboxW = axto->eBuf->bufW;

  axto->h = axto->eBuf->bufH;
  axto->sboxH = axto->eBuf->bufH;

  axto->alignment = axto->fm.currentFontAlignment();

  if ( axto->alignment == XmALIGNMENT_BEGINNING )
    axto->stringX = axto->x;
  else if ( axto->alignment == XmALIGNMENT_CENTER )
    axto->stringX = axto->x + axto->w/2 - axto->stringWidth/2;
  else if ( axto->alignment == XmALIGNMENT_END )
    axto->stringX = axto->x + axto->w - axto->stringWidth;

  axto->updateDimensions();

  if ( axto->autoSize && axto->fs ) {
    axto->sboxW = axto->w = axto->stringBoxWidth;
    axto->sboxH = axto->h = axto->stringBoxHeight;
  }

  axto->stringY = axto->y + axto->fontAscent + axto->h/2 -
   axto->stringBoxHeight/2;

}

static void axtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextClass *axto = (activeXTextClass *) client;

  axtc_edit_update ( w, client, call );
  axto->refresh( axto );

}

static void axtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextClass *axto = (activeXTextClass *) client;

  axtc_edit_update ( w, client, call );

  if ( axto->bufValue ) {
    delete[] axto->bufValue;
    axto->bufValue = NULL;
  }

  axto->ef.popdown();
  axto->operationComplete();

}

static void axtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextClass *axto = (activeXTextClass *) client;

  if ( axto->bufValue ) {
    delete[] axto->bufValue;
    axto->bufValue = NULL;
  }

  axto->ef.popdown();
  axto->operationCancel();

}

static void axtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeXTextClass *axto = (activeXTextClass *) client;

  if ( axto->bufValue ) {
    delete[] axto->bufValue;
    axto->bufValue = NULL;
  }

  axto->ef.popdown();
  axto->operationCancel();
  axto->erase();
  axto->deleteRequest = 1;
  axto->drawAll();

}

void activeXTextClass::alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeXTextClass *axto = (activeXTextClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    axto->connection.setPvDisconnected( (void *) axto->alarmPvConnection );
    axto->fgColor.setDisconnected();
    axto->bgColor.setDisconnected();

    axto->actWin->appCtx->proc->lock();
    axto->needRefresh = 1;
    axto->actWin->addDefExeNode( axto->aglPtr );
    axto->actWin->appCtx->proc->unlock();

  }

}

void activeXTextClass::alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeXTextClass *axto = (activeXTextClass *) userarg;

  if ( !axto->connection.pvsConnected() ) {

    if ( pv->is_valid() ) {

      axto->connection.setPvConnected( (void *) alarmPvConnection );

      if ( axto->connection.pvsConnected() ) {
        axto->actWin->appCtx->proc->lock();
        axto->needConnectInit = 1;
        axto->actWin->addDefExeNode( axto->aglPtr );
        axto->actWin->appCtx->proc->unlock();
      }

    }

  }
  else {

    axto->actWin->appCtx->proc->lock();
    axto->needAlarmUpdate = 1;
    axto->actWin->addDefExeNode( axto->aglPtr );
    axto->actWin->appCtx->proc->unlock();

  }

}

void activeXTextClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeXTextClass *axto = (activeXTextClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    axto->connection.setPvDisconnected( (void *) axto->visPvConnection );
    axto->fgColor.setDisconnected();
    axto->bgColor.setDisconnected();

    axto->actWin->appCtx->proc->lock();
    axto->needRefresh = 1;
    axto->actWin->addDefExeNode( axto->aglPtr );
    axto->actWin->appCtx->proc->unlock();

  }

}

void activeXTextClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeXTextClass *axto = (activeXTextClass *) userarg;

  if ( !axto->connection.pvsConnected() ) {

    if ( pv->is_valid() ) {

      axto->connection.setPvConnected( (void *) visPvConnection );

      if ( axto->connection.pvsConnected() ) {
        axto->actWin->appCtx->proc->lock();
        axto->needConnectInit = 1;
        axto->actWin->addDefExeNode( axto->aglPtr );
        axto->actWin->appCtx->proc->unlock();
      }

    }

  }
  else {

    axto->actWin->appCtx->proc->lock();
    axto->needVisUpdate = 1;
    axto->actWin->addDefExeNode( axto->aglPtr );
    axto->actWin->appCtx->proc->unlock();

    }

}

activeXTextClass::activeXTextClass ( void ) {

  name = new char[strlen("activeXTextClass")+1];
  strcpy( name, "activeXTextClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  visPvExists = alarmPvExists = 0;
  activeMode = 0;
  fgColorMode = AXTC_K_COLORMODE_STATIC;
  bgColorMode = AXTC_K_COLORMODE_STATIC;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  strcpy( id, "" );
  connection.setMaxPvs( 2 );
  unconnectedTimer = 0;
  setBlinkFunction( (void *) doBlink );
  border = 0;
  lineThk = 1;
  bufValue = NULL;
  eBuf = NULL;
  savedDims = 0;

}

// copy constructor
activeXTextClass::activeXTextClass
 ( const activeXTextClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeXTextClass")+1];
  strcpy( name, "activeXTextClass" );

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);
  fgColorMode = source->fgColorMode;
  bgColorMode = source->bgColorMode;
  visInverted = source->visInverted;

  alarmPvExpStr.setRaw( source->alarmPvExpStr.rawString );
  visPvExpStr.setRaw( source->visPvExpStr.rawString );

  visibility = 0;
  prevVisibility = -1;
  visPvExists = alarmPvExists = 0;
  activeMode = 0;

  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

  strncpy( id, source->id, 31 );

  useDisplayBg = source->useDisplayBg;

  autoSize = source->autoSize;

  border = source->border;
  lineThk = source->lineThk;

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  value.copy( source->value );

  alignment = source->alignment;

  stringLength = source->stringLength;
  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;
  stringWidth = source->stringWidth;
  stringY = source->stringY;
  stringX = source->stringX;
  stringBoxWidth = source->stringBoxWidth;
  stringBoxHeight = source->stringBoxHeight;

  connection.setMaxPvs( 2 );

  unconnectedTimer = 0;

  bufValue = NULL;
  eBuf = NULL;

  savedDims = 0;

  setBlinkFunction( (void *) doBlink );

  doAccSubs( alarmPvExpStr );
  doAccSubs( visPvExpStr );
  doAccSubs( value );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );

}

activeXTextClass::~activeXTextClass ( void ) {

  if ( name ) delete[] name;

  if ( bufValue ) delete[] bufValue;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

int activeXTextClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

int stat = 1;

  actWin = (activeWindowClass *) aw_obj;
  xOrigin = 0;
  yOrigin = 0;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  useDisplayBg = 1;
  autoSize = 1;
  border = 0;
  lineThk = 1;

  strcpy( fontTag, actWin->defaultFontTag );

  actWin->fi->loadFontTag( fontTag );

  fs = actWin->fi->getXFontStruct( fontTag );
  if ( fs ) {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
  }
  else {
    fontAscent = 0;
    fontDescent = 0;
    fontHeight = 0;
  }

  alignment = actWin->defaultAlignment;

  updateDimensions();

  this->draw();

  this->editCreate();

  return stat;

}

int activeXTextClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  if ( !bufValue ) {
    bufValue = new char[activeXTextClass::MAX_TEXT_LEN+1];
  }

  ptr = actWin->obj.getNameFromClass( "activeXTextClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeXTextClass_str4, 31 );

  Strncat( title, activeXTextClass_str5, 31 );

  strncpy( bufId, id, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufFgColor = fgColor.pixelIndex();
  eBuf->bufFgColorMode = fgColorMode;

  eBuf->bufBgColor = bgColor.pixelIndex();
  eBuf->bufBgColorMode = bgColorMode;

  if ( alarmPvExpStr.getRaw() )
    strncpy( eBuf->bufAlarmPvName, alarmPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufAlarmPvName, "" );

  if ( visPvExpStr.getRaw() )
    strncpy( eBuf->bufVisPvName, visPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufVisPvName, "" );

  if ( visInverted )
    eBuf->bufVisInverted = 0;
  else
    eBuf->bufVisInverted = 1;

  strncpy( eBuf->bufMinVisString, minVisString, 39 );
  strncpy( eBuf->bufMaxVisString, maxVisString, 39 );

  strncpy( eBuf->bufFontTag, fontTag, 63 );
  eBuf->bufUseDisplayBg = useDisplayBg;
  eBuf->bufAutoSize = autoSize;
  eBuf->bufBorder = border;
  eBuf->bufLineThk = lineThk;

  if ( value.getRaw() )
    strncpy( bufValue, value.getRaw(), activeXTextClass::MAX_TEXT_LEN );
  else
    strncpy( bufValue, "", activeXTextClass::MAX_TEXT_LEN );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  //ef.addTextField( activeXTextClass_str6, 35, bufId, 31 );

  ef.addTextField( activeXTextClass_str7, 35, &eBuf->bufX );
  ef.addTextField( activeXTextClass_str8, 35, &eBuf->bufY );
  ef.addTextField( activeXTextClass_str9, 35, &eBuf->bufW );
  ef.addTextField( activeXTextClass_str10, 35, &eBuf->bufH );

  ef.addTextBox( activeXTextClass_str23, 32, 10, bufValue,
   activeXTextClass::MAX_TEXT_LEN );

  //ef.addTextField( activeXTextClass_str23, 35, bufValue, 255 );

  ef.addToggle( activeXTextClass_str11, &eBuf->bufAutoSize );
  ef.addToggle( activeXTextClass_str26, &eBuf->bufBorder );
  ef.addOption( activeXTextClass_str27, activeXTextClass_str28,
   &eBuf->bufLineThk );
  ef.addColorButton( activeXTextClass_str13, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addToggle( activeXTextClass_str14, &eBuf->bufFgColorMode );

  ef.addToggle( activeXTextClass_str15, &eBuf->bufUseDisplayBg );
  fillEntry = ef.getCurItem();
  ef.addColorButton( activeXTextClass_str16, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  fillColorEntry = ef.getCurItem();
  fillEntry->addInvDependency( fillColorEntry );
  ef.addToggle( activeXTextClass_str17, &eBuf->bufBgColorMode );
  fillAlarmSensEntry = ef.getCurItem();
  fillEntry->addInvDependency( fillAlarmSensEntry );
  fillEntry->addDependencyCallbacks();

  ef.addFontMenu( activeXTextClass_str12, actWin->fi, &fm, fontTag );
  fm.setFontAlignment( alignment );
  ef.addTextField( activeXTextClass_str18, 35, eBuf->bufAlarmPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeXTextClass_str19, 35, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeXTextClass_str20, &eBuf->bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeXTextClass_str21, 35, eBuf->bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeXTextClass_str22, 35, eBuf->bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  return 1;

}

int activeXTextClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( axtc_edit_ok, axtc_edit_apply, axtc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeXTextClass::edit ( void ) {

  this->genericEdit();
  ef.finished( axtc_edit_ok, axtc_edit_apply, axtc_edit_cancel, this );
  fm.setFontAlignment( alignment );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeXTextClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
int one = 1;
char *emptyStr = "";

int left = XmALIGNMENT_BEGINNING;
static char *alignEnumStr[3] = {
  "left",
  "center",
  "right"
};
static int alignEnum[3] = {
  XmALIGNMENT_BEGINNING,
  XmALIGNMENT_CENTER,
  XmALIGNMENT_END
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
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "fgAlarm", &fgColorMode, &zero );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "bgAlarm", &bgColorMode, &zero );
  tag.loadR( "useDisplayBg", &useDisplayBg, &zero );
  tag.loadR( "alarmPv", &alarmPvExpStr, emptyStr );
  tag.loadR( "visPv", &visPvExpStr, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "value", &value, emptyStr );
  tag.loadR( "autoSize", &autoSize, &zero );
  tag.loadR( "border", &border, &zero );
  tag.loadR( "lineWidth", &lineThk, &one );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > AXTC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( fgColorMode )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  if ( bgColorMode )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  if ( value.getRaw() )
    stringLength = strlen( value.getRaw() );
  else
    stringLength = 0;

  fs = actWin->fi->getXFontStruct( fontTag );

  if ( value.getRaw() )
    updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );
  else
    updateFont( " ", fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

  updateDimensions();

  if ( autoSize && fs ) {
    sboxW = w = stringBoxWidth;
    sboxH = h = stringBoxHeight;
  }

  stringY = y + fontAscent + h/2 - stringBoxHeight/2;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  return stat;

}

int activeXTextClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneValue[activeXTextClass::MAX_TEXT_LEN+1],
 onePv[PV_Factory::MAX_PV_NAME+1];
int stat = 1;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > AXTC_MAJOR_VERSION ) {
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

    if ( fgColorMode == AXTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &useDisplayBg ); actWin->incLine();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == AXTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &useDisplayBg ); actWin->incLine();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 3 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == AXTC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &useDisplayBg ); actWin->incLine();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 3 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    bgColor.setColorIndex( index, actWin->ci );

  }

  fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

  if ( bgColorMode == AXTC_K_COLORMODE_ALARM )
    bgColor.setAlarmSensitive();
  else
    bgColor.setAlarmInsensitive();

  readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  alarmPvExpStr.setRaw( onePv );

  readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  visPvExpStr.setRaw( onePv );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    readStringFromFile( minVisString, 39+1, f ); actWin->incLine();
    readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();
  }
  else {
    strcpy( minVisString, "1" );
    strcpy( maxVisString, "1" );
  }

  readStringFromFile( oneValue, activeXTextClass::MAX_TEXT_LEN+1, f );
   actWin->incLine();
  value.setRaw( oneValue );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  fscanf( f, "%d\n", &alignment ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    fscanf( f, "%d\n", &autoSize ); actWin->incLine();
  }
  else {
    autoSize = 0;
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    readStringFromFile( this->id, 31+1, f ); actWin->incLine();
  }
  else {
    strcpy( this->id, "" );
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  if ( value.getRaw() )
    stringLength = strlen( value.getRaw() );
  else
    stringLength = 0;

  fs = actWin->fi->getXFontStruct( fontTag );

  if ( value.getRaw() )
    updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );
  else
    updateFont( " ", fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

  updateDimensions();

  stringY = y + fontAscent + h/2 - stringBoxHeight/2;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  return stat;

}

int activeXTextClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, more, index;
unsigned int pixel;
int stat = 1;
char *tk, *gotData, *context,
 oneValue[activeXTextClass::MAX_TEXT_LEN+1], buf[255+1];

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;

  this->actWin = _actWin;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  useDisplayBg = 1;
  autoSize = 1;

  strcpy( fontTag, actWin->defaultFontTag );

  alignment = actWin->defaultAlignment;

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    buf[255] = 0;
    if ( !gotData ) {
      actWin->appCtx->postMessage( activeXTextClass_str24 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( activeXTextClass_str24 );
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
          actWin->appCtx->postMessage( activeXTextClass_str24 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextClass_str24 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextClass_str24 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextClass_str24 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "value" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextClass_str24 );
          return 0;
        }

        strncpy( oneValue, tk, activeXTextClass::MAX_TEXT_LEN );
        oneValue[activeXTextClass::MAX_TEXT_LEN] = 0;

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextClass_str24 );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }
            
      else if ( strcmp( tk, "justify" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextClass_str24 );
          return 0;
        }

        alignment = atol( tk );

      }
            
      else if ( strcmp( tk, "red" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextClass_str24 );
          return 0;
        }

        r = atol( tk );

      }
            
      else if ( strcmp( tk, "green" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextClass_str24 );
          return 0;
        }

        g = atol( tk );

      }
            
      else if ( strcmp( tk, "blue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( activeXTextClass_str24 );
          return 0;
        }

        b = atol( tk );

      }
            
    }

  } while ( more );

  actWin->ci->setRGB( r, g, b, &pixel );
  index = actWin->ci->pixIndex( pixel );
  fgColor.setColorIndex( index, actWin->ci );

  fgColorMode = 0; // alarm insensitive
  fgColor.setAlarmInsensitive();

  bgColorMode = 0; // alarm insensitive
  bgColor.setAlarmInsensitive();

  alarmPvExpStr.setRaw( "" );

  visPvExpStr.setRaw( "" );

  visInverted = 0;

  strcpy( minVisString, "1" );
  strcpy( maxVisString, "1" );

  value.setRaw( oneValue );

  if ( value.getRaw() )
    stringLength = strlen( value.getRaw() );
  else
    stringLength = 0;

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  if ( fs ) {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
  }
  else {
    fontAscent = 0;
    fontDescent = 0;
    fontHeight = 0;
  }

  if ( value.getRaw() )
    updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );
  else
    updateFont( " ", fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

  updateDimensions();

  y = y + fontDescent;

  this->initSelectBox(); // call after getting x,y,w,h

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  stringY = y + fontAscent + h/2 - stringBoxHeight/2;

  return stat;

}

int activeXTextClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
int one = 1;
char *emptyStr = "";

int left = XmALIGNMENT_BEGINNING;
static char *alignEnumStr[3] = {
  "left",
  "center",
  "right"
};
static int alignEnum[3] = {
  XmALIGNMENT_BEGINNING,
  XmALIGNMENT_CENTER,
  XmALIGNMENT_END
};

  major = AXTC_MAJOR_VERSION;
  minor = AXTC_MINOR_VERSION;
  release = AXTC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "font", fontTag );
  tag.loadW( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadBoolW( "fgAlarm", &fgColorMode, &zero );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadBoolW( "bgAlarm", &bgColorMode, &zero );
  tag.loadBoolW( "useDisplayBg", &useDisplayBg, &zero );
  tag.loadW( "alarmPv", &alarmPvExpStr, emptyStr );
  tag.loadW( "visPv", &visPvExpStr, emptyStr );
  tag.loadBoolW( "visInvert", &visInverted, &zero );
  tag.loadW( "visMin", minVisString, emptyStr );
  tag.loadW( "visMax", maxVisString, emptyStr );
  tag.loadComplexW( "value", &value, emptyStr );
  tag.loadBoolW( "autoSize", &autoSize, &zero );
  tag.loadBoolW( "border", &border, &zero );
  tag.loadW( "lineWidth", &lineThk, &one );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeXTextClass::drawActive ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;
int blink = 0;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( fgColor.getDisconnectedIndex(), &blink );
      if ( strcmp( fontTag, "" ) != 0 ) {
        actWin->executeGc.setFontTag( fontTag, actWin->fi );
      }
      clipStat = actWin->executeGc.addNormXClipRectangle( xR );
      XDrawStringsAligned( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, stringY, w,
       value.getExpanded(), stringLength, &fs, alignment );
      if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
      updateBlink( blink );
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.saveFg();
    needToEraseUnconnected = 0;
    if ( strcmp( fontTag, "" ) != 0 ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
    }
    clipStat = actWin->executeGc.addEraseXClipRectangle( xR );
    XDrawStringsAligned( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, stringY, w,
     value.getExpanded(), stringLength, &fs, alignment );
    if ( clipStat & 1 ) actWin->executeGc.removeEraseXClipRectangle();
    actWin->executeGc.restoreFg();
  }

  if ( !enabled || !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  if ( fgVisibility ) {

    actWin->executeGc.saveFg();

    clipStat = actWin->executeGc.addNormXClipRectangle( xR );

    if ( strcmp( fontTag, "" ) != 0 ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
    }

    if ( useDisplayBg ) {

      actWin->executeGc.setFG( fgColor.getIndex(), &blink );

      XDrawStringsAligned( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, stringY, w,
       value.getExpanded(), stringLength, &fs, alignment );

    }
    else {

      actWin->executeGc.setFG( bgColor.getColor() );

      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      XFillRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      actWin->executeGc.setFG( fgColor.getIndex(), &blink );

      actWin->executeGc.saveBg();
      actWin->executeGc.setBG( bgColor.getColor() );

      XDrawImageStringsAligned( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, stringY, w,
       value.getExpanded(), stringLength, &fs, alignment );

      actWin->executeGc.restoreBg();

    }

    if ( border ) {
      actWin->executeGc.setFG( fgColor.getIndex(), &blink );
      actWin->executeGc.setLineWidth( lineThk );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x+lineThk/2, y+lineThk/2, w-lineThk, h-lineThk );
      actWin->executeGc.setLineWidth( 1 );
    }

    if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();

    actWin->executeGc.restoreFg();

  }

  updateBlink( blink );

  bufInvalid = 0;

  return 1;

}

int activeXTextClass::eraseUnconditional ( void ) {

XRectangle xR = { x, y, w, h };

  if ( !enabled ) return 1;

  actWin->executeGc.addEraseXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    XDrawStringsAligned( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, stringY, w,
     value.getExpanded(), stringLength, &fs, alignment );

  }
  else {

    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

    XFillRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

    XDrawImageStringsAligned( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, stringY, w,
     value.getExpanded(), stringLength, &fs, alignment );

  }

  if ( border ) {
    actWin->executeGc.setLineWidth( lineThk );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x+lineThk/2, y+lineThk/2, w-lineThk, h-lineThk );
    actWin->executeGc.setLineWidth( 1 );
  }

  actWin->executeGc.removeEraseXClipRectangle();

  return 1;

}

int activeXTextClass::eraseActive ( void ) {

XRectangle xR = { x, y, w, h };

  if ( !enabled || !activeMode ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    actWin->executeGc.addEraseXClipRectangle( xR );

    XDrawStringsAligned( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, stringY, w,
     value.getExpanded(), stringLength, &fs, alignment );

    actWin->executeGc.removeEraseXClipRectangle();

  }
  else {

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.saveFg();
    actWin->executeGc.saveBg();

    if ( visibility && bgVisibility ) {

      actWin->executeGc.setBG( bgColor.getColor() );
      actWin->executeGc.setFG( bgColor.getColor() );

      if ( bufInvalid ) {

        XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.eraseGC(), x, y, w, h );

        XFillRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.eraseGC(), x, y, w, h );

      }
      else {

        XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), x, y, w, h );

        XFillRectangle( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), x, y, w, h );

        XDrawImageStringsAligned( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), x, stringY, w,
         value.getExpanded(), stringLength, &fs, alignment );

      }

    }

    if ( border ) {
      actWin->executeGc.setLineWidth( lineThk );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), x+lineThk/2, y+lineThk/2, w-lineThk, h-lineThk );
      actWin->executeGc.setLineWidth( 1 );
    }

    actWin->executeGc.restoreFg();
    actWin->executeGc.restoreBg();

    actWin->executeGc.removeNormXClipRectangle();

  }

  return 1;

}

int activeXTextClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( alarmPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  alarmPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( visPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  visPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( value.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  value.setRaw( tmpStr.getExpanded() );

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  if ( value.getRaw() )
    stringLength = strlen( value.getRaw() );
  else
    stringLength = 0;

  fs = actWin->fi->getXFontStruct( fontTag );

  if ( value.getRaw() )
    updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );
  else
    updateFont( " ", fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

  updateDimensions();

  if ( autoSize && fs ) {
    sboxW = w = stringBoxWidth;
    sboxH = h = stringBoxHeight;
  }

  stringY = y + fontAscent + h/2 - stringBoxHeight/2;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  return 1;

}

int activeXTextClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand1st( numMacros, macros, expansions );
  stat = visPvExpStr.expand1st( numMacros, macros, expansions );
  stat = value.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeXTextClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = visPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = value.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeXTextClass::containsMacros ( void ) {

  if ( alarmPvExpStr.containsPrimaryMacros() ) return 1;
  if ( visPvExpStr.containsPrimaryMacros() ) return 1;
  if ( value.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeXTextClass::activate (
  int pass,
  void *ptr )
{

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

      savedX = x;
      savedW = w;
      savedH = h;
      savedDims = 1;

      connection.init();
      initEnable();

      curFgColorIndex = -1;
      curBgColorIndex = -1;
      curStatus = -1;
      curSeverity = -1;
      prevVisibility = -1;
      visibility = 0;
      prevFgVisibility = -1;
      fgVisibility = 0;
      prevBgVisibility = -1;
      bgVisibility = 0;
      bufInvalid = 1;

      needConnectInit = needAlarmUpdate = needVisUpdate = needRefresh =
        needPropertyUpdate = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      if ( value.getExpanded() ) {
        stringLength = strlen( value.getExpanded() );
      } 
      else {
        stringLength = strlen( value.getRaw() );
      }

      updateFont( value.getExpanded(), fontTag, &fs, &fontAscent, &fontDescent,
       &fontHeight, &stringWidth );

      activeMode = 1; // this must appear before the call to updateDimensions

      updateDimensions();

      if ( autoSize && fs ) {
        if ( alignment == XmALIGNMENT_CENTER ) {
          x = x - stringBoxWidth/2 + w/2;
          sboxX = x;
	}
        else if ( alignment == XmALIGNMENT_END ) {
          x = x - stringBoxWidth + w;
          sboxX = x;
	}
        sboxW = w = stringBoxWidth;
        sboxH = h = stringBoxHeight;
      }

      stringY = y + fontAscent + h/2 - stringBoxHeight/2;

      if ( alignment == XmALIGNMENT_BEGINNING )
        stringX = x;
      else if ( alignment == XmALIGNMENT_CENTER )
        stringX = x + w/2 - stringWidth/2;
      else if ( alignment == XmALIGNMENT_END )
        stringX = x + w - stringWidth;

      aglPtr = ptr;

      alarmPvId = visPvId = 0;

      pvType = -1;

      init = 1; // this stays true if there are no pvs

      if ( !alarmPvExpStr.getExpanded() ||
           // ( strcmp( alarmPvExpStr.getExpanded(), "" ) == 0 ) ) {
           blankOrComment( alarmPvExpStr.getExpanded() ) ) {
        alarmPvExists = 0;
        fgVisibility = bgVisibility = 1;
      }
      else {
        connection.addPv();
        alarmPvExists = 1;
        fgColor.setConnectSensitive();
        bgColor.setConnectSensitive();
        init = 0;
      }

      if ( !visPvExpStr.getExpanded() ||
           //( strcmp( visPvExpStr.getExpanded(), "" ) == 0 ) ) {
           blankOrComment( visPvExpStr.getExpanded() ) ) {
        visPvExists = 0;
        visibility = 1;
      }
      else {
        connection.addPv();
        visPvExists = 1;
        visibility = 0;
        fgVisibility = bgVisibility = 1;
        fgColor.setConnectSensitive();
        bgColor.setConnectSensitive();
        init = 0;
      }

      if ( alarmPvExists ) {
        alarmPvId = the_PV_Factory->create( alarmPvExpStr.getExpanded() );
        if ( alarmPvId ) {
          alarmPvId->add_conn_state_callback( alarmPvConnectStateCallback,
           this );
          alarmPvId->add_value_callback( alarmPvValueCallback, this );
	}
      }

      if ( visPvExists ) {
        visPvId = the_PV_Factory->create( visPvExpStr.getExpanded() );
        if ( visPvId ) {
          visPvId->add_conn_state_callback( visPvConnectStateCallback, this );
          visPvId->add_value_callback( visPvValueCallback, this );
	}
      }

      opComplete = 1;

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

int activeXTextClass::deactivate (
  int pass )
{

  if ( pass == 1 ) {

  if ( savedDims ) {
    savedDims = 0;
    x = sboxX = savedX;
    w = sboxW = savedW;
    h = sboxH = savedH;
  }

  activeMode = 0;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  //updateBlink( 0 );

  if ( value.getRaw() )
    stringLength = strlen( value.getRaw() );
  else
    stringLength = 0;

  if ( value.getRaw() )
    updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );
  else
    updateFont( " ", fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

  updateDimensions();

  stringY = y + fontAscent + h/2 - stringBoxHeight/2;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

    if ( alarmPvId ) {
      alarmPvId->remove_conn_state_callback( alarmPvConnectStateCallback,
       this );
      alarmPvId->remove_value_callback( alarmPvValueCallback, this );
      alarmPvId->release();
      alarmPvId = 0;
    }

    if ( visPvId ) {
      visPvId->remove_conn_state_callback( visPvConnectStateCallback, this );
      visPvId->remove_value_callback( visPvValueCallback, this );
      visPvId->release();
      visPvId = 0;
    }

  }

  return 1;

}

int activeXTextClass::draw ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;
int blink = 0;

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();
  actWin->drawGc.saveBg();

  clipStat = actWin->drawGc.addNormXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    if ( value.getRaw() ) {

      actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
      actWin->drawGc.setBG( bgColor.pixelColor() );

      XDrawStringsAligned( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), x, stringY, w,
       value.getRaw(), stringLength, &fs, alignment );

    }

  }
  else {

    actWin->drawGc.setFG( bgColor.pixelColor() );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
    actWin->drawGc.setBG( bgColor.pixelColor() );

    if ( value.getRaw() ) {
      XDrawImageStringsAligned( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), x, stringY, w,
       value.getRaw(), stringLength, &fs, alignment );
    }

  }

  if ( border ) {
    actWin->drawGc.setFG( fgColor.pixelIndex(), &blink );
    actWin->drawGc.setLineWidth( lineThk );
    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x+lineThk/2, y+lineThk/2, w-lineThk, h-lineThk );
    actWin->drawGc.setLineWidth( 1 );
  }

  if ( clipStat & 1 )
    actWin->drawGc.removeNormXClipRectangle();
  //else
  //  fprintf( stderr, "clipStat = %-d\n", clipStat );

  actWin->drawGc.restoreFg();
  actWin->drawGc.restoreBg();

  updateBlink( blink );

  return 1;

}

int activeXTextClass::erase ( void ) {

XRectangle xR = { x, y, w, h };

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.addEraseXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    if ( value.getRaw() ) {
      XDrawStringsAligned( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), x, stringY, w,
       value.getRaw(), stringLength, &fs, alignment );
    }

  }
  else {

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x, y, w, h );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x, y, w, h );

    if ( value.getRaw() ) {
      XDrawImageStringsAligned( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), x, stringY, w,
       value.getRaw(), stringLength, &fs, alignment );
    }

  }

  if ( border ) {
    actWin->drawGc.setLineWidth( lineThk );
    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x+lineThk/2, y+lineThk/2, w-lineThk, h-lineThk );
    actWin->drawGc.setLineWidth( 1 );
  }

  actWin->drawGc.removeEraseXClipRectangle();

  return 1;

}

void activeXTextClass::updateDimensions ( void )
{

  if ( activeMode ) {
    if ( value.getExpanded() ) {
      getStringBoxSize( value.getExpanded(), stringLength, &fs, alignment,
       &stringBoxWidth, &stringBoxHeight );
    }
    else {
      getStringBoxSize( value.getRaw(), stringLength, &fs, alignment,
       &stringBoxWidth, &stringBoxHeight );
    }
  }
  else {
    getStringBoxSize( value.getRaw(), stringLength, &fs, alignment,
     &stringBoxWidth, &stringBoxHeight );
  }

  stringY = y + fontAscent + h/2 - stringBoxHeight/2;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  if ( activeMode ) {
    if ( value.getExpanded() )
      stringLength = strlen( value.getExpanded() );
    else
      stringLength = 0;
  }
  else {
    if ( value.getRaw() )
      stringLength = strlen( value.getRaw() );
    else
      stringLength = 0;
  }

}

void activeXTextClass::executeDeferred ( void ) {

int stat, nc, nau, nvu, nr, npu, index, change;
pvValType pvV;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nau = needAlarmUpdate; needAlarmUpdate = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  nr = needRefresh; needRefresh = 0;
  npu = needPropertyUpdate; needPropertyUpdate = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

  if ( nc ) {

    minVis.d = (double) atof( minVisString );
    maxVis.d = (double) atof( maxVisString );

    fgColor.setConnected();
    bgColor.setConnected();

    if ( alarmPvExists ) {

      curStatus = alarmPvId->get_status();
      curSeverity = alarmPvId->get_severity();

      fgColor.setStatus( curStatus, curSeverity );
      bgColor.setStatus( curStatus, curSeverity );

      curFgColorIndex = actWin->ci->evalRule( fgColor.pixelIndex(),
       alarmPvId->get_double() );
      fgColor.changeIndex( curFgColorIndex, actWin->ci );

      curBgColorIndex = actWin->ci->evalRule( bgColor.pixelIndex(),
       alarmPvId->get_double() );
      bgColor.changeIndex( curBgColorIndex, actWin->ci );

      if ( !visPvExists ) {

        if ( actWin->ci->isInvisible( curFgColorIndex ) ) {
          prevFgVisibility = fgVisibility = 0;
        }
        else {
          prevFgVisibility = fgVisibility = 1;
        }

        if ( actWin->ci->isInvisible( curBgColorIndex ) ) {
          prevBgVisibility = bgVisibility = 0;
        }
        else {
          prevBgVisibility = bgVisibility = 1;
        }

      }

    }

    if ( visPvExists ) {

      pvV.d = visPvId->get_double();
      if ( ( pvV.d >= minVis.d ) && ( pvV.d < maxVis.d ) )
        visibility = 1 ^ visInverted;
      else
        visibility = 0 ^ visInverted;

      prevVisibility = visibility;

    }

    init = 1;

    eraseUnconditional();
    stat = smartDrawAllActive();

  }

  if ( nau ) {

    change = 0;

    if ( alarmPvId ) {

      if ( curStatus != alarmPvId->get_status() ) {
        curStatus = alarmPvId->get_status();
        change = 1;
      }

      if ( curSeverity != alarmPvId->get_severity() ) {
        curSeverity = alarmPvId->get_severity();
        change = 1;
      }

      if ( change ) {
        fgColor.setStatus( curStatus, curSeverity );
        bgColor.setStatus( curStatus, curSeverity );
      }

      index = actWin->ci->evalRule( fgColor.pixelIndex(),
       alarmPvId->get_double() );

      if ( curFgColorIndex != index ) {
        curFgColorIndex = index;
        change = 1;
      }

      index = actWin->ci->evalRule( bgColor.pixelIndex(),
       alarmPvId->get_double() );

      if ( curBgColorIndex != index ) {
        curBgColorIndex = index;
        change = 1;
      }

    }

    if ( change ) {

      if ( !visPvExists ) {

        if ( actWin->ci->isInvisible( curFgColorIndex ) ) {
          fgVisibility = 0;
        }
        else {
          fgVisibility = 1;
        }

        if ( actWin->ci->isInvisible( curBgColorIndex ) ) {
          bgVisibility = 0;
        }
        else {
          bgVisibility = 1;
        }

      }

      fgColor.changeIndex( curFgColorIndex, actWin->ci );
      bgColor.changeIndex( curBgColorIndex, actWin->ci );
      if ( ( prevFgVisibility != fgVisibility ) ||
	   ( prevBgVisibility != bgVisibility ) ) {
        prevFgVisibility = fgVisibility;
        prevBgVisibility = bgVisibility;
        eraseUnconditional();
      }
      smartDrawAllActive();

    }

  }

  if ( nvu ) {

    pvV.d = visPvId->get_double();
    if ( ( pvV.d >= minVis.d ) && ( pvV.d < maxVis.d ) )
      visibility = 1 ^ visInverted;
    else
      visibility = 0 ^ visInverted;

    if ( prevVisibility != visibility ) {
      if ( !visibility ) eraseUnconditional();
      stat = smartDrawAllActive();
    }

  }

  if ( nr ) {
    stat = smartDrawAllActive();
  }

  if ( npu ) {

    eraseActive();

    if ( bufValue ) {
      value.setRaw( bufValue );
      stringLength = strlen( bufValue );
      delete[] bufValue;
      bufValue = NULL;
    }

    updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

    updateDimensions();

    if ( autoSize && fs ) {
      sboxW = w = stringBoxWidth;
      sboxH = h = stringBoxHeight;
    }

    stat = smartDrawAllActive();

  }

}

int activeXTextClass::setProperty (
  char *prop,
  char *_value )
{

  if ( !bufValue ) {
    bufValue = new char[activeXTextClass::MAX_TEXT_LEN+1];
  }

  if ( strcmp( prop, activeXTextClass_str33 ) == 0 ) {

    strncpy( bufValue, _value, activeXTextClass::MAX_TEXT_LEN );

    actWin->appCtx->proc->lock();
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();

    needPropertyUpdate = 1;

  }

  return 1;

}

char *activeXTextClass::firstDragName ( void ) {

#define MAXDRAGNAMES 2

int i;
int present[MAXDRAGNAMES];

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( !blank( alarmPvExpStr.getExpanded() ) ) {
      present[0] = 1;
    }
    else {
      present[0] = 0;
    }

    if ( !blank( visPvExpStr.getExpanded() ) ) {
      present[1] = 1;
    }
    else {
      present[1] = 0;
    }

  }
  else {

    if ( !blank( alarmPvExpStr.getRaw() ) ) {
      present[0] = 1;
    }
    else {
      present[0] = 0;
    }

    if ( !blank( visPvExpStr.getRaw() ) ) {
      present[1] = 1;
    }
    else {
      present[1] = 0;
    }

  }

  for ( i=0; i<MAXDRAGNAMES; i++ ) {
    if ( present[i] ) {
      dragIndex = i;
      return dragName[dragIndex];
    }
  }

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeXTextClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeXTextClass::dragValue (
  int i ) {

int offset = 0;

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( blank( alarmPvExpStr.getExpanded() ) ) {
      offset++;
      if ( blank( visPvExpStr.getExpanded() ) ) {
        offset++;
      }
    }

    switch ( i+offset ) {

    case 0:
      return alarmPvExpStr.getExpanded();

    case 1:
      return visPvExpStr.getExpanded();

    }

  }
  else {

    if ( blank( alarmPvExpStr.getRaw() ) ) {
      offset++;
      if ( blank( visPvExpStr.getRaw() ) ) {
        offset++;
      }
    }

    switch ( i+offset ) {

    case 0:
      return alarmPvExpStr.getRaw();

    case 1:
      return visPvExpStr.getRaw();

    }

  }

  return (char *) NULL;

}

void activeXTextClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_ALIGNMENT_MASK )
    alignment = _alignment;

  if ( _flag & ACTGRF_FONTTAG_MASK ) {

    strcpy( fontTag, _fontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );

    if ( fs ) {
      fontAscent = fs->ascent;
      fontDescent = fs->descent;
      fontHeight = fontAscent + fontDescent;
    }
    else {
      fontAscent = 0;
      fontDescent = 0;
      fontHeight = 0;
    }

    updateDimensions();

  }

}

void activeXTextClass::changePvNames (
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

  if ( flag & ACTGRF_VISPVS_MASK ) {
    if ( numVisPvs ) {
      visPvExpStr.setRaw( visPvs[0] );
    }
  }

  if ( flag & ACTGRF_ALARMPVS_MASK ) {
    if ( numAlarmPvs ) {
      alarmPvExpStr.setRaw( alarmPvs[0] );
    }
  }

}

void activeXTextClass::updateColors (
  double colorValue )
{

int index, change;

  change = 0;

  if ( alarmPvId ) {

    if ( curStatus != alarmPvId->get_status() ) {
      curStatus = alarmPvId->get_status();
      change = 1;
    }

    if ( curSeverity != alarmPvId->get_severity() ) {
      curSeverity = alarmPvId->get_severity();
      change = 1;
    }

  }

  index = actWin->ci->evalRule( fgColor.pixelIndex(),
   colorValue );

  if ( curFgColorIndex != index ) {
    curFgColorIndex = index;
    change = 1;
  }

  index = actWin->ci->evalRule( bgColor.pixelIndex(),
   colorValue );

  if ( curBgColorIndex != index ) {
    curBgColorIndex = index;
    change = 1;
  }

  if ( change ) {

    if ( actWin->ci->isInvisible( curFgColorIndex ) ) {
      fgVisibility = 0;
    }
    else {
      fgVisibility = 1;
    }

    if ( actWin->ci->isInvisible( curBgColorIndex ) ) {
      bgVisibility = 0;
    }
    else {
      bgVisibility = 1;
    }

    fgColor.changeIndex( curFgColorIndex, actWin->ci );
    bgColor.changeIndex( curBgColorIndex, actWin->ci );
    if ( ( prevFgVisibility != fgVisibility ) ||
       ( prevBgVisibility != bgVisibility ) ) {
      prevFgVisibility = fgVisibility;
      prevBgVisibility = bgVisibility;
    }

  }

}

void activeXTextClass::bufInvalidate ( void ) {

  bufInvalid = 1;

}

void activeXTextClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 2 ) {
    *n = 0;
    return;
  }

  *n = 2;
  pvs[0] = alarmPvId;
  pvs[1] = visPvId;

}

char *activeXTextClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return value.getRaw();
  }
  else if ( i == 1 ) {
    return alarmPvExpStr.getRaw();
  }
  else if ( i == 2 ) {
    return visPvExpStr.getRaw();
  }
  else if ( i == 3 ) {
    return minVisString;
  }
  else if ( i == 4 ) {
    return maxVisString;
  }

  return NULL;

}

void activeXTextClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    value.setRaw( string );
  }
  else if ( i == 1 ) {
    alarmPvExpStr.setRaw( string );
  }
  else if ( i == 2 ) {
    visPvExpStr.setRaw( string );
  }
  else if ( i == 3 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( minVisString, string, l );
    minVisString[l] = 0;
  }
  else if ( i == 4 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( maxVisString, string, l );
    maxVisString[l] = 0;
  }

  updateDimensions();

  if ( autoSize && fs ) {
    sboxW = w = stringBoxWidth;
    sboxH = h = stringBoxHeight;
  }

}

// crawler functions may return blank pv names
char *activeXTextClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return alarmPvExpStr.getExpanded();

}

char *activeXTextClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >= 1 ) return NULL;
  crawlerPvIndex++;
  return visPvExpStr.getExpanded();

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeXTextClassPtr ( void ) {

activeXTextClass *ptr;

  ptr = new activeXTextClass;
  return (void *) ptr;

}

void *clone_activeXTextClassPtr (
  void *_srcPtr )
{

activeXTextClass *ptr, *srcPtr;

  srcPtr = (activeXTextClass *) _srcPtr;

  ptr = new activeXTextClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
