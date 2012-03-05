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

#define __x_regtext_obj_cc 1

#include "x_regtext_obj.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

void activeXRegTextClass::edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeXRegTextClass *me = (activeXRegTextClass *) client;

  me->actWin->setChanged();

  me->eraseSelectBoxCorners();
  me->erase();

  strncpy( me->id, me->bufId, 31 );

  me->fgColorMode = me->bufFgColorMode;
  if ( me->fgColorMode == AXTC_K_COLORMODE_ALARM )
    me->fgColor.setAlarmSensitive();
  else
    me->fgColor.setAlarmInsensitive();
  me->fgColor.setColorIndex( me->bufFgColor, me->actWin->ci );

  me->bgColorMode = me->bufBgColorMode;
  if ( me->bgColorMode == AXTC_K_COLORMODE_ALARM )
    me->bgColor.setAlarmSensitive();
  else
    me->bgColor.setAlarmInsensitive();
  me->bgColor.setColorIndex( me->bufBgColor, me->actWin->ci );

  me->alarmPvExpStr.setRaw( me->bufAlarmPvName );

  me->visPvExpStr.setRaw( me->bufVisPvName );

  if ( me->bufVisInverted )
    me->visInverted = 0;
  else
    me->visInverted = 1;

  strncpy( me->minVisString, me->bufMinVisString, 39 );
  strncpy( me->maxVisString, me->bufMaxVisString, 39 );

  me->value.setRaw( me->bufValue );

  strncpy( me->fontTag, me->fm.currentFontTag(), 63 );
  me->actWin->fi->loadFontTag( me->fontTag );
  me->actWin->drawGc.setFontTag( me->fontTag, me->actWin->fi );

  me->stringLength = strlen( me->value.getRaw() );

  me->fs = me->actWin->fi->getXFontStruct( me->fontTag );

  me->updateFont( me->value.getRaw(), me->fontTag, &me->fs,
   &me->fontAscent, &me->fontDescent, &me->fontHeight,
   &me->stringWidth );

  me->useDisplayBg = me->bufUseDisplayBg;

  me->autoSize = me->bufAutoSize;

  me->x = me->bufX;
  me->sboxX = me->bufX;

  me->y = me->bufY;
  me->sboxY = me->bufY;

  me->w = me->bufW;
  me->sboxW = me->bufW;

  me->h = me->bufH;
  me->sboxH = me->bufH;

  me->alignment = me->fm.currentFontAlignment();

  if ( me->alignment == XmALIGNMENT_BEGINNING )
    me->stringX = me->x;
  else if ( me->alignment == XmALIGNMENT_CENTER )
    me->stringX = me->x + me->w/2 - me->stringWidth/2;
  else if ( me->alignment == XmALIGNMENT_END )
    me->stringX = me->x + me->w - me->stringWidth;

  me->updateDimensions();

  if ( me->autoSize && me->fs ) {
    me->sboxW = me->w = me->stringBoxWidth;
    me->sboxH = me->h = me->stringBoxHeight;
  }

  me->stringY = me->y + me->fontAscent + me->h/2 -
   me->stringBoxHeight/2;

//-----------------------------------
  strncpy( me->regExpStr, me->bufRegExp, 39 );
//-----------------------------------

}

void activeXRegTextClass::edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeXRegTextClass *me = (activeXRegTextClass *) client;

  edit_update ( w, client, call );
  me->refresh( me );

}

void activeXRegTextClass::edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeXRegTextClass *me = (activeXRegTextClass *) client;

  edit_update ( w, client, call );
  me->ef.popdown();
  me->operationComplete();

}

  void activeXRegTextClass::edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeXRegTextClass *me = (activeXRegTextClass *) client;

  me->ef.popdown();
  me->operationCancel();

}

void activeXRegTextClass::edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeXRegTextClass *me = (activeXRegTextClass *) client;

  me->ef.popdown();
  me->operationCancel();
  me->erase();
  me->deleteRequest = 1;
  me->drawAll();

}

void activeXRegTextClass::alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeXRegTextClass *axrto = (activeXRegTextClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    axrto->connection.setPvDisconnected( (void *) axrto->alarmPvConnection );
    axrto->fgColor.setDisconnected();
    axrto->bgColor.setDisconnected();

    axrto->actWin->appCtx->proc->lock();
    axrto->needRefresh = 1;
    axrto->actWin->addDefExeNode( axrto->aglPtr );
    axrto->actWin->appCtx->proc->unlock();

  }

}

void activeXRegTextClass::alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeXRegTextClass *axrto = (activeXRegTextClass *) userarg;

  if ( !axrto->connection.pvsConnected() ) {

    if ( pv->is_valid() ) {

      axrto->connection.setPvConnected( (void *) alarmPvConnection );

      if ( axrto->connection.pvsConnected() ) {
        axrto->actWin->appCtx->proc->lock();
        axrto->needConnectInit = 1;
        axrto->actWin->addDefExeNode( axrto->aglPtr );
        axrto->actWin->appCtx->proc->unlock();
      }

    }

  }
  else {

    axrto->actWin->appCtx->proc->lock();
    axrto->needAlarmUpdate = 1;
    axrto->actWin->addDefExeNode( axrto->aglPtr );
    axrto->actWin->appCtx->proc->unlock();

  }

}

void activeXRegTextClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeXRegTextClass *axrto = (activeXRegTextClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    axrto->connection.setPvDisconnected( (void *) axrto->visPvConnection );
    axrto->fgColor.setDisconnected();
    axrto->bgColor.setDisconnected();

    axrto->actWin->appCtx->proc->lock();
    axrto->needRefresh = 1;
    axrto->actWin->addDefExeNode( axrto->aglPtr );
    axrto->actWin->appCtx->proc->unlock();

  }

}

void activeXRegTextClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeXRegTextClass *axrto = (activeXRegTextClass *) userarg;

  if ( !axrto->connection.pvsConnected() ) {

    if ( pv->is_valid() ) {

      axrto->connection.setPvConnected( (void *) visPvConnection );

      if ( axrto->connection.pvsConnected() ) {
        axrto->actWin->appCtx->proc->lock();
        axrto->needConnectInit = 1;
        axrto->actWin->addDefExeNode( axrto->aglPtr );
        axrto->actWin->appCtx->proc->unlock();
      }

    }

  }
  else {

    axrto->actWin->appCtx->proc->lock();
    axrto->needVisUpdate = 1;
    axrto->actWin->addDefExeNode( axrto->aglPtr );
    axrto->actWin->appCtx->proc->unlock();

    }

}

activeXRegTextClass::activeXRegTextClass ( void ) {

//  fprintf( stderr,"RegText constructor\n");
  name = new char[strlen("activeXRegTextClass")+1];
  strcpy( name, "activeXRegTextClass" );
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
//-------------------------------------
  strcpy( regExpStr, "" );
//-------------------------------------

}

// copy constructor
activeXRegTextClass::activeXRegTextClass
 ( const activeXRegTextClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeXRegTextClass")+1];
  strcpy( name, "activeXRegTextClass" );

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);
  fgCb = source->fgCb;
  bgCb = source->bgCb;
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
//-------------------------------------
  strncpy( regExpStr, source->regExpStr, 39 );
  strncpy( bufRegExp, source->bufRegExp, 39 );
//-------------------------------------

  doAccSubs( alarmPvExpStr );
  doAccSubs( visPvExpStr );
  doAccSubs( value );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );
  doAccSubs( regExpStr, 39 );

}

int activeXRegTextClass::createInteractive (
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

  updateDimensions();

  alignment = actWin->defaultAlignment;

  this->draw();

  this->editCreate();

  return stat;

}

int activeXRegTextClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeXRegTextClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeXTextClass_str4, 31 );

  Strncat( title, activeXTextClass_str5, 31 );

  strncpy( bufId, id, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufFgColor = fgColor.pixelIndex();
  bufFgColorMode = fgColorMode;

  bufBgColor = bgColor.pixelIndex();
  bufBgColorMode = bgColorMode;

  if ( alarmPvExpStr.getRaw() )
    strncpy( bufAlarmPvName, alarmPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( bufAlarmPvName, "" );

  if ( visPvExpStr.getRaw() )
    strncpy( bufVisPvName, visPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( bufVisPvName, "" );

  if ( visInverted )
    bufVisInverted = 0;
  else
    bufVisInverted = 1;

  strncpy( bufMinVisString, minVisString, 39 );
  strncpy( bufMaxVisString, maxVisString, 39 );

  bufUseDisplayBg = useDisplayBg;
  bufAutoSize = autoSize;

  if ( value.getRaw() )
    strncpy( bufValue, value.getRaw(), 255 );
  else
    strncpy( bufValue, "", 255 );

//----------------------------------------
  // add dialog box entry field for regular expression
  if (regExpStr)
    	strncpy(bufRegExp, regExpStr, 39);
  else
    	strncpy(bufRegExp, "", 39);
 strncpy( bufRegExp, regExpStr, 39);
//----------------------------------------

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  //ef.addTextField( activeXTextClass_str6, 35, bufId, 31 );

  ef.addTextField( activeXTextClass_str7, 35, &bufX );
  ef.addTextField( activeXTextClass_str8, 35, &bufY );
  ef.addTextField( activeXTextClass_str9, 35, &bufW );
  ef.addTextField( activeXTextClass_str10, 35, &bufH );
  ef.addTextField( activeXTextClass_str23, 35, bufValue, 255 );
  ef.addToggle( activeXTextClass_str11, &bufAutoSize );
  ef.addColorButton( activeXTextClass_str13, actWin->ci, &fgCb, &bufFgColor );
  ef.addToggle( activeXTextClass_str14, &bufFgColorMode );

  ef.addToggle( activeXTextClass_str15, &bufUseDisplayBg );
  fillEntry = ef.getCurItem();
  ef.addColorButton( activeXTextClass_str16, actWin->ci, &bgCb, &bufBgColor );
  fillColorEntry = ef.getCurItem();
  fillEntry->addInvDependency( fillColorEntry );
  ef.addToggle( activeXTextClass_str17, &bufBgColorMode );
  fillAlarmSensEntry = ef.getCurItem();
  fillEntry->addInvDependency( fillAlarmSensEntry );
  fillEntry->addDependencyCallbacks();

  ef.addFontMenu( activeXTextClass_str12, actWin->fi, &fm, fontTag );
  fm.setFontAlignment( alignment );
  ef.addTextField( activeXTextClass_str18, 35, bufAlarmPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeXTextClass_str19, 35, bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeXTextClass_str20, &bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeXTextClass_str21, 35, bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeXTextClass_str22, 35, bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

//----------------------------------------
  ef.addTextField( "Reg. Exp.", 35, bufRegExp, 39 );
//----------------------------------------

  return 1;

}

int activeXRegTextClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( edit_ok, edit_apply, edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeXRegTextClass::edit ( void ) {

  this->genericEdit();
  ef.finished( edit_ok, edit_apply, edit_cancel, this );
  fm.setFontAlignment( alignment );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeXRegTextClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
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
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
  tag.loadR( "autoSize", &autoSize, &zero );
  tag.loadR( "regExpr", 39, regExpStr, emptyStr );
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

  stringY = y + fontAscent + h/2 - stringBoxHeight/2;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  return stat;

}

int activeXRegTextClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneValue[PV_Factory::MAX_PV_NAME+1];
int stat = 1;

//  fprintf( stderr,"RegText old_createFromFile\n");

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

  readStringFromFile( oneValue, PV_Factory::MAX_PV_NAME, f ); actWin->incLine();
  alarmPvExpStr.setRaw( oneValue );

  readStringFromFile( oneValue, PV_Factory::MAX_PV_NAME, f ); actWin->incLine();
  visPvExpStr.setRaw( oneValue );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    readStringFromFile( minVisString, 39, f ); actWin->incLine();
    readStringFromFile( maxVisString, 39, f ); actWin->incLine();
  }
  else {
    strcpy( minVisString, "1" );
    strcpy( maxVisString, "1" );
  }

  readStringFromFile( oneValue, 255, f ); actWin->incLine();
  value.setRaw( oneValue );

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  fscanf( f, "%d\n", &alignment ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    fscanf( f, "%d\n", &autoSize ); actWin->incLine();
  }
  else {
    autoSize = 0;
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    readStringFromFile( this->id, 31, f ); actWin->incLine();
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

  stringY = y + fontAscent;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

//---------------------------------------------
  // read regular expression from file
  readStringFromFile(regExpStr, 39, f);  
  actWin->incLine();
//---------------------------------------------

  return stat;

}

int activeXRegTextClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, more, index;
unsigned int pixel;
int stat = 1;
char *tk, *gotData, *context, oneValue[255+1], buf[255+1];

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

        strncpy( oneValue, tk, 255 );
        oneValue[255] = 0;

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

  y = y + fontDescent;

  this->initSelectBox(); // call after getting x,y,w,h

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  stringY = y + fontAscent;

  return stat;

}

int activeXRegTextClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
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
  tag.loadW( "regExpr", regExpStr, emptyStr );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeXRegTextClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", AXTC_MAJOR_VERSION, AXTC_MINOR_VERSION,
   AXTC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fgColorMode );

  fprintf( f, "%-d\n", useDisplayBg );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", bgColorMode );

  if ( alarmPvExpStr.getRaw() )
    writeStringToFile( f, alarmPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( visPvExpStr.getRaw() )
    writeStringToFile( f, visPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  if ( value.getRaw() )
    writeStringToFile( f, value.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", alignment );

  fprintf( f, "%-d\n", autoSize );

  // version 1.4.0
  writeStringToFile( f, this->id );

// --------------------------------------------------------
  writeStringToFile(f, regExpStr);
    				// plus what RegText adds
// --------------------------------------------------------

  return 1;

}

int activeXRegTextClass::drawActive ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;
//    fprintf( stderr,"in drawActive\n");

  if ( !enabled || !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  if ( fgVisibility ) {

    actWin->executeGc.saveFg();

    actWin->executeGc.setFG( fgColor.getColor() );

    clipStat = actWin->executeGc.addNormXClipRectangle( xR );

    if ( strcmp( fontTag, "" ) != 0 ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
    }

    char text[80];
    getProcessedText(text);

    if ( useDisplayBg ) {

      XDrawStrings( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), stringX, stringY, fontHeight,
       text, stringLength );

    }
    else {

      actWin->executeGc.saveBg();
      actWin->executeGc.setBG( bgColor.getColor() );

      XDrawImageStrings( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), stringX, stringY, fontHeight,
       text, stringLength );

      actWin->executeGc.restoreBg();

    }

    if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();

    actWin->executeGc.restoreFg();

  }

  bufInvalid = 0;

  return 1;

}

char * activeXRegTextClass::getProcessedText(char *text) {
    size_t len = 80;
//    fprintf( stderr,"in getProcessedText\n");
    strncpy( text, value.getExpanded(), 79 );
    if (re_valid)
    {
        regmatch_t pmatch[2];
        if (regexec(&compiled_re, text, 2, pmatch, 0) == 0)
        {
            // copy matched substring into display string
            // match 0 is always the full match,
            // match 1 is the first selected substring
            int start = pmatch[1].rm_so;
            int size = pmatch[1].rm_eo - pmatch[1].rm_so;
            
            if (start >= 0)
            {
                memmove(text, text+start, size);
                text[size] = '\0';
                len = size;
            }
            else
            {
                text[0] = '\0';
                len = 0;
            }
        }
    }
    return text;
}

int activeXRegTextClass::eraseUnconditional ( void ) {

XRectangle xR = { x, y, w, h };

  if ( !enabled ) return 1;

  actWin->executeGc.addEraseXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  char text[80];
  getProcessedText(text);
//    fprintf(stderr,"eraseUnconditional: text=%s\n", text);


  if ( useDisplayBg ) {

    XDrawStrings( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), stringX, stringY, fontHeight,
     text, stringLength );
//    fprintf(stderr,"eraseUnconditional: useDisplayBg; text=%s\n", text);

  }
  else {

    XDrawImageStrings( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), stringX, stringY, fontHeight,
     text, stringLength );
//    fprintf(stderr,"eraseUnconditional: !useDisplayBg; text=%s\n", text);

  }

  actWin->executeGc.removeEraseXClipRectangle();

  return 1;

}

int activeXRegTextClass::eraseActive ( void ) {

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

  char text[80];
  getProcessedText(text);

  if ( useDisplayBg ) {

    actWin->executeGc.addEraseXClipRectangle( xR );

    XDrawStrings( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), stringX, stringY, fontHeight,
     text, stringLength );
//    fprintf(stderr,"eraseActive: useDisplayBg; text=%s\n", text);

    actWin->executeGc.removeEraseXClipRectangle();

  }
  else {

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.saveFg();
    actWin->executeGc.saveBg();

    if ( visibility && bgVisibility ) {

      if ( bufInvalid ) {

        XDrawImageStrings( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.eraseGC(), stringX, stringY, fontHeight,
         text, stringLength );

      }
      else {

        actWin->executeGc.setFG( bgColor.getColor() );
        actWin->executeGc.setBG( bgColor.getColor() );

        XDrawImageStrings( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), stringX, stringY, fontHeight,
         text, stringLength );

      }

    }

    actWin->executeGc.restoreFg();
    actWin->executeGc.restoreBg();

    actWin->executeGc.removeNormXClipRectangle();

  }

  return 1;

}

int activeXRegTextClass::expandTemplate (
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

  return 1;

}

int activeXRegTextClass::expand1st (
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

int activeXRegTextClass::expand2nd (
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

int activeXRegTextClass::containsMacros ( void ) {

  if ( alarmPvExpStr.containsPrimaryMacros() ) return 1;
  if ( visPvExpStr.containsPrimaryMacros() ) return 1;
  if ( value.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeXRegTextClass::activate (
  int pass,
  void *ptr )
{

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;
    re_valid = false;

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

      if ( !re_valid ) {
        if (strlen(regExpStr) > 0)
        {
            int res = regcomp(&compiled_re, regExpStr, REG_EXTENDED);
            if (res)
            {
	        // The compile failed, but memory was allocated (leak)
                char buf[100];
                regerror(res, &compiled_re, buf, sizeof buf);
                // fprintf( stderr,"Error in regular expression: %s\n", buf);
            }
            else {
                re_valid = true;
	    }
        }
      }

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

      char text[80];
      getProcessedText(text);

      stringLength = strlen( text );

      updateFont( text, fontTag, &fs, &fontAscent, &fontDescent,
       &fontHeight, &stringWidth );

      updateDimensions();

      stringY = y + fontAscent + h/2 - stringBoxHeight/2;

      if ( alignment == XmALIGNMENT_BEGINNING )
        stringX = x;
      else if ( alignment == XmALIGNMENT_CENTER )
        stringX = x + w/2 - stringWidth/2;
      else if ( alignment == XmALIGNMENT_END )
        stringX = x + w - stringWidth;

      aglPtr = ptr;

      alarmPvId = visPvId = 0;

      activeMode = 1;
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
         // ( strcmp( visPvExpStr.getExpanded(), "" ) == 0 ) ) {
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
          //if ( alarmPvId->is_valid() ) {
          //  alarmPvConnectStateCallback( alarmPvId, this );
          //  alarmPvValueCallback( alarmPvId, this );
	  //}
          alarmPvId->add_conn_state_callback( alarmPvConnectStateCallback,
           this );
          alarmPvId->add_value_callback( alarmPvValueCallback, this );
	}
      }

      if ( visPvExists ) {
        visPvId = the_PV_Factory->create( visPvExpStr.getExpanded() );
        if ( visPvId ) {
          //if ( visPvId->is_valid() ) {
          //  visPvConnectStateCallback( visPvId, this );
          //  visPvValueCallback( visPvId, this );
          //}
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

int activeXRegTextClass::deactivate (
  int pass )
{

  if ( pass == 1 ) {

// --------------------------------------------------------
    if ( re_valid ) {
      //fprintf( stderr, "regfree\n" );
      regfree(&compiled_re);
    }
// --------------------------------------------------------

  activeMode = 0;

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

int activeXRegTextClass::draw ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();
  actWin->drawGc.saveBg();

  actWin->drawGc.setFG( fgColor.pixelColor() );
  actWin->drawGc.setBG( bgColor.pixelColor() );

  clipStat = actWin->drawGc.addNormXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    if ( value.getRaw() ) {
      XDrawStrings( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), stringX, stringY, fontHeight,
       value.getRaw(), stringLength );
    }

  }
  else {

    if ( value.getRaw() ) {
      XDrawImageStrings( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), stringX, stringY, fontHeight,
       value.getRaw(), stringLength );
    }

  }

  if ( clipStat & 1 )
    actWin->drawGc.removeNormXClipRectangle();
  //else
  //  fprintf( stderr, "clipStat = %-d\n", clipStat );

  actWin->drawGc.restoreFg();
  actWin->drawGc.restoreBg();

  return 1;

}

int activeXRegTextClass::erase ( void ) {

XRectangle xR = { x, y, w, h };

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.addEraseXClipRectangle( xR );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( useDisplayBg ) {

    if ( value.getRaw() ) {
      XDrawStrings( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), stringX, stringY, fontHeight,
       value.getRaw(), stringLength );
    }

  }
  else {

    if ( value.getRaw() ) {
      XDrawImageStrings( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), stringX, stringY, fontHeight,
       value.getRaw(), stringLength );
    }

  }

  actWin->drawGc.removeEraseXClipRectangle();

  return 1;

}

void activeXRegTextClass::updateDimensions ( void )
{

  getStringBoxSize( value.getRaw(), stringLength, &fs, alignment,
   &stringBoxWidth, &stringBoxHeight );

  stringY = y + fontAscent + h/2 - stringBoxHeight/2;

  if ( alignment == XmALIGNMENT_BEGINNING )
    stringX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    stringX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    stringX = x + w - stringWidth;

  if ( activeMode ) {
    char text[80];
    getProcessedText(text);
    stringLength = strlen( text );
  }
  else {
    if ( value.getRaw() )
      stringLength = strlen( value.getRaw() );
    else
      stringLength = 0;
  }

}

void activeXRegTextClass::executeDeferred ( void ) {

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

    value.setRaw( bufValue );

    stringLength = strlen( bufValue );

    updateFont( value.getRaw(), fontTag, &fs, &fontAscent, &fontDescent,
     &fontHeight, &stringWidth );

    updateDimensions();

    if ( autoSize && fs ) {
      sboxW = w = stringBoxWidth;
      sboxH = h = stringBoxHeight;
    }

    updateDimensions();

    stat = smartDrawAllActive();

  }

}

int activeXRegTextClass::setProperty (
  char *prop,
  char *_value )
{

  if ( strcmp( prop, activeXTextClass_str33 ) == 0 ) {

    strncpy( bufValue, _value, 255 );

    actWin->appCtx->proc->lock();
    actWin->addDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();

    needPropertyUpdate = 1;

  }

  return 1;

}

char *activeXRegTextClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeXRegTextClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeXRegTextClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    switch ( i ) {

    case 1:
      return alarmPvExpStr.getExpanded();
      break;

    case 2:
      return visPvExpStr.getExpanded();
      break;

    }

  }
  else {

    switch ( i ) {

    case 1:
      return alarmPvExpStr.getRaw();
      break;

    case 2:
      return visPvExpStr.getRaw();
      break;

    }

  }

  // else, disabled

  return (char *) NULL;

}

void activeXRegTextClass::changeDisplayParams (
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

void activeXRegTextClass::changePvNames (
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

void activeXRegTextClass::updateColors (
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

void activeXRegTextClass::bufInvalidate ( void ) {

  bufInvalid = 1;

}

void activeXRegTextClass::getPvs (
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

char *activeXRegTextClass::getSearchString (
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
  else if ( i == 5 ) {
    return regExpStr;
  }

  return NULL;

}

void activeXRegTextClass::replaceString (
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
  else if ( i == 5 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( regExpStr, string, l );
    regExpStr[l] = 0;
  }

  updateDimensions();

  if ( autoSize && fs ) {
    sboxW = w = stringBoxWidth;
    sboxH = h = stringBoxHeight;
  }

}

// crawler functions may return blank pv names
char *activeXRegTextClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return alarmPvExpStr.getExpanded();

}

char *activeXRegTextClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >=1 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return visPvExpStr.getExpanded();
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeXRegTextClassPtr ( void ) {

activeXRegTextClass *ptr;

  ptr = new activeXRegTextClass;
  return (void *) ptr;

}

void *clone_activeXRegTextClassPtr (
  void *_srcPtr )
{

activeXRegTextClass *ptr, *srcPtr;

  srcPtr = (activeXRegTextClass *) _srcPtr;

  ptr = new activeXRegTextClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
