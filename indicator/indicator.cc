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

#define __indicator_cc 1

#include "indicator.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeIndicatorClass *indicatoro = (activeIndicatorClass *) client;

  if ( !indicatoro->init ) {
    indicatoro->needToDrawUnconnected = 1;
    indicatoro->needRefresh = 1;
    indicatoro->actWin->addDefExeNode( indicatoro->aglPtr );
  }

  indicatoro->unconnectedTimer = 0;

}

static void indicatorc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeIndicatorClass *indicatoro = (activeIndicatorClass *) client;
char fmt[31+1], str[31+1];
int l;

  indicatoro->actWin->setChanged();

  indicatoro->eraseSelectBoxCorners();
  indicatoro->erase();

  indicatoro->fgColorMode = indicatoro->bufFgColorMode;
  if ( indicatoro->fgColorMode == INDICATORC_K_COLORMODE_ALARM )
    indicatoro->fgColor.setAlarmSensitive();
  else
    indicatoro->fgColor.setAlarmInsensitive();
  indicatoro->fgColor.setColorIndex( indicatoro->bufFgColor, indicatoro->actWin->ci );

  indicatoro->indicatorColorMode = indicatoro->bufIndicatorColorMode;
  if ( indicatoro->indicatorColorMode == INDICATORC_K_COLORMODE_ALARM )
    indicatoro->indicatorColor.setAlarmSensitive();
  else
    indicatoro->indicatorColor.setAlarmInsensitive();
  indicatoro->indicatorColor.setColorIndex( indicatoro->bufIndicatorColor, indicatoro->actWin->ci );

  indicatoro->bgColor.setColorIndex( indicatoro->bufBgColor, indicatoro->actWin->ci );

  indicatoro->controlPvExpStr.setRaw( indicatoro->eBuf->bufControlPvName );
  indicatoro->readPvExpStr.setRaw( indicatoro->eBuf->bufReadPvName );
  indicatoro->nullPvExpStr.setRaw( indicatoro->eBuf->bufNullPvName );

  indicatoro->label.setRaw( indicatoro->eBuf->bufLabel );

  indicatoro->labelType = indicatoro->bufLabelType;

  strncpy( indicatoro->fontTag, indicatoro->fm.currentFontTag(), 63 );
  indicatoro->actWin->fi->loadFontTag( indicatoro->fontTag );
  indicatoro->fs = indicatoro->actWin->fi->getXFontStruct( indicatoro->fontTag );
  indicatoro->actWin->drawGc.setFontTag( indicatoro->fontTag, indicatoro->actWin->fi );

  if ( indicatoro->fs ) {
    indicatoro->indicatorStrLen = XTextWidth( indicatoro->fs, "10", 2 );
  }

  indicatoro->border = indicatoro->bufBorder;

  strncpy( indicatoro->scaleFormat, indicatoro->bufScaleFormat, 15 );
  indicatoro->showScale = indicatoro->bufShowScale;

  indicatoro->labelTicksExpStr.setRaw( indicatoro->bufLabelTicks );
  indicatoro->majorTicksExpStr.setRaw( indicatoro->bufMajorTicks );
  indicatoro->minorTicksExpStr.setRaw( indicatoro->bufMinorTicks );

  indicatoro->x = indicatoro->bufX;
  indicatoro->sboxX = indicatoro->bufX;

  indicatoro->y = indicatoro->bufY;
  indicatoro->sboxY = indicatoro->bufY;

  indicatoro->w = indicatoro->bufW;
  indicatoro->sboxW = indicatoro->bufW;

  indicatoro->h = indicatoro->bufH;
  indicatoro->sboxH = indicatoro->bufH;

  indicatoro->horizontal = indicatoro->bufHorizontal;

  indicatoro->halfW = indicatoro->bufHalfW;
  if ( indicatoro->halfW < 0 ) indicatoro->halfW = 0;

  indicatoro->pointerOpposite = indicatoro->bufPointerOpposite;

  indicatoro->limitsFromDb = indicatoro->bufLimitsFromDb;

  indicatoro->precisionExpStr.setRaw( indicatoro->bufPrecision );

  indicatoro->readMinExpStr.setRaw( indicatoro->bufReadMin );
  indicatoro->readMaxExpStr.setRaw( indicatoro->bufReadMax );

  // set edit-mode display values
  indicatoro->precision = 0;
  indicatoro->readMin = 0;
  indicatoro->readMax = 10;
  indicatoro->labelTicks = 10;
  indicatoro->majorTicks = 2;
  indicatoro->minorTicks = 2;

  if ( strcmp( indicatoro->scaleFormat, "GFloat" ) == 0 ) {
    sprintf( fmt, "%%.%-dg", indicatoro->precision );
  }
  else if ( strcmp( indicatoro->scaleFormat, "Exponential" ) == 0 ) {
    sprintf( fmt, "%%.%-de", indicatoro->precision );
  }
  else {
    sprintf( fmt, "%%.%-df", indicatoro->precision );
  }

  formatString( indicatoro->readMin, str, 31, fmt );
  if ( indicatoro->fs ) {
    indicatoro->indicatorStrLen = XTextWidth( indicatoro->fs, str, strlen(str) );
  }
  formatString( indicatoro->readMax, str, 31, fmt );
  if ( indicatoro->fs ) {
    l = XTextWidth( indicatoro->fs, str, strlen(str) );
    if ( l > indicatoro->indicatorStrLen ) indicatoro->indicatorStrLen = l;
  }

  indicatoro->updateDimensions();

  if ( indicatoro->horizontal ) {

    if ( indicatoro->h < indicatoro->minH ) {
      indicatoro->h = indicatoro->minH;
      indicatoro->sboxH = indicatoro->minH;
    }

  }
  else {

    if ( indicatoro->h < indicatoro->minVertH ) {
      indicatoro->h = indicatoro->minVertH;
      indicatoro->sboxH = indicatoro->minVertH;
    }

  }

}

static void indicatorc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeIndicatorClass *indicatoro = (activeIndicatorClass *) client;

  indicatorc_edit_update ( w, client, call );
  indicatoro->refresh( indicatoro );

}

static void indicatorc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeIndicatorClass *indicatoro = (activeIndicatorClass *) client;

  indicatorc_edit_update ( w, client, call );
  indicatoro->ef.popdown();
  indicatoro->operationComplete();

}

static void indicatorc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeIndicatorClass *indicatoro = (activeIndicatorClass *) client;

  indicatoro->ef.popdown();
  indicatoro->operationCancel();

}

static void indicatorc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeIndicatorClass *indicatoro = (activeIndicatorClass *) client;

  indicatoro->ef.popdown();
  indicatoro->operationCancel();
  indicatoro->erase();
  indicatoro->deleteRequest = 1;
  indicatoro->drawAll();

}

static void indicator_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeIndicatorClass *indicatoro = (activeIndicatorClass *) userarg;

  indicatoro->actWin->appCtx->proc->lock();

  if ( indicatoro->activeMode ) {

    if ( pv->is_valid() ) {

      indicatoro->pvNotConnectedMask &= ~( (unsigned char) 1 );
      if ( !indicatoro->pvNotConnectedMask ) { // if all are connected
        indicatoro->needConnectInit = 1;
        indicatoro->actWin->addDefExeNode( indicatoro->aglPtr );
      }

    }
    else {

      indicatoro->pvNotConnectedMask |= 1; // read pv not connected
      indicatoro->active = 0;
      indicatoro->indicatorColor.setDisconnected();
      indicatoro->fgColor.setDisconnected();
      indicatoro->bufInvalidate();
      indicatoro->needFullDraw = 1;
      indicatoro->actWin->addDefExeNode( indicatoro->aglPtr );

    }

  }

  indicatoro->actWin->appCtx->proc->unlock();

}

static void indicator_monitor_null_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeIndicatorClass *indicatoro = (activeIndicatorClass *) userarg;

  indicatoro->actWin->appCtx->proc->lock();

  if ( indicatoro->activeMode ) {

    if ( pv->is_valid() ) {

      indicatoro->pvNotConnectedMask &= ~( (unsigned char) 2 );
      if ( !indicatoro->pvNotConnectedMask ) { // if all are connected
        indicatoro->needConnectInit = 1;
        indicatoro->actWin->addDefExeNode( indicatoro->aglPtr );
      }

    }
    else {

      indicatoro->pvNotConnectedMask |= 2; // null pv not connected
      indicatoro->active = 0;
      indicatoro->indicatorColor.setDisconnected();
      indicatoro->fgColor.setDisconnected();
      indicatoro->bufInvalidate();
      indicatoro->needDraw = 1;
      indicatoro->actWin->addDefExeNode( indicatoro->aglPtr );

    }

  }

  indicatoro->actWin->appCtx->proc->unlock();

}

static void indicator_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeIndicatorClass *indicatoro = (activeIndicatorClass *) userarg;
int st, sev;

  indicatoro->actWin->appCtx->proc->lock();

  if ( indicatoro->active ) {

    st = pv->get_status();
    sev = pv->get_severity();
    if ( ( st != indicatoro->oldStat ) || ( sev != indicatoro->oldSev ) ) {
      indicatoro->oldStat = st;
      indicatoro->oldSev = sev;
      indicatoro->fgColor.setStatus( st, sev );
      indicatoro->indicatorColor.setStatus( st, sev );
      indicatoro->needFullDraw = 1;
    }

    indicatoro->curReadV = pv->get_double();
    indicatoro->needDrawCheck = 1;
    indicatoro->actWin->addDefExeNode( indicatoro->aglPtr );

  }

  indicatoro->actWin->appCtx->proc->unlock();

}

static void indicator_nullUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeIndicatorClass *indicatoro = (activeIndicatorClass *) userarg;

  indicatoro->actWin->appCtx->proc->lock();

  if ( indicatoro->active ) {

    indicatoro->curNullV = pv->get_double();

    indicatoro->needDrawCheck = 1;
    indicatoro->actWin->addDefExeNode( indicatoro->aglPtr );

  }

  indicatoro->actWin->appCtx->proc->unlock();

}

activeIndicatorClass::activeIndicatorClass ( void ) {

  name = new char[strlen("activeIndicatorClass")+1];
  strcpy( name, "activeIndicatorClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  minW = 50;
  minH = 2;
  minVertW = 2;
  minVertH = 10;
  indicatorStrLen = 10;
  strcpy( fontTag, "" );
  fs = NULL;
  activeMode = 0;

  indicatorColorMode = INDICATORC_K_COLORMODE_STATIC;
  fgColorMode = INDICATORC_K_COLORMODE_STATIC;
  labelType = INDICATORC_K_LITERAL;
  border = 1;
  showScale = 1;
  labelTicksExpStr.setRaw( "" );
  majorTicksExpStr.setRaw( "" );
  minorTicksExpStr.setRaw( "" );
  readMinExpStr.setRaw( "" );
  readMaxExpStr.setRaw( "" );

  readMin = 0;
  readMax = 10;
  labelTicks = 10;
  majorTicks = 2;
  minorTicks = 2;

  halfW = 5;

  pointerOpposite = 0;

  limitsFromDb = 1;
  precisionExpStr.setRaw( "" );
  strcpy( scaleFormat, "FFloat" );
  precision = 0;
  unconnectedTimer = 0;
  eBuf = NULL;

}

// copy constructor
activeIndicatorClass::activeIndicatorClass
 ( const activeIndicatorClass *source ) {

activeGraphicClass *indicatoro = (activeGraphicClass *) this;

  indicatoro->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeIndicatorClass")+1];
  strcpy( name, "activeIndicatorClass" );

  indicatorCb = source->indicatorCb;
  fgCb = source->fgCb;
  bgCb = source->bgCb;

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );

  indicatorColor.copy( source->indicatorColor );
  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );

  controlPvExpStr.copy( source->controlPvExpStr );
  readPvExpStr.copy( source->readPvExpStr );
  nullPvExpStr.copy( source->nullPvExpStr );
  label.copy( source->label );

  indicatorColorMode = source->indicatorColorMode;
  fgColorMode = source->fgColorMode;
  labelType = source->labelType;
  border = source->border;
  showScale = source->showScale;
  labelTicksExpStr.copy( source->labelTicksExpStr );
  majorTicksExpStr.copy( source->majorTicksExpStr );
  minorTicksExpStr.copy( source->minorTicksExpStr );
  indicatorStrLen = source->indicatorStrLen;

  minW = 50;
  minH = 2;
  minVertW = 2;
  minVertH = 10;
  activeMode = 0;

  readMin = source->readMin;
  readMax = source->readMax;
  labelTicks = source->labelTicks;
  majorTicks = source->majorTicks;
  minorTicks = source->minorTicks;

  limitsFromDb = source->limitsFromDb;
  readMinExpStr.copy( source->readMinExpStr );
  readMaxExpStr.copy( source->readMaxExpStr );

  precisionExpStr.copy( source->precisionExpStr );
  precision = source->precision;

  strncpy( scaleFormat, source->scaleFormat, 15 );

  horizontal = source->horizontal;

  halfW = source->halfW;

  pointerOpposite = source->pointerOpposite;

  unconnectedTimer = 0;

  eBuf = NULL;

  doAccSubs( readPvExpStr );
  doAccSubs( nullPvExpStr );
  doAccSubs( label );

  updateDimensions();

}

activeIndicatorClass::~activeIndicatorClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

int activeIndicatorClass::createInteractive (
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

  if ( _w >= _h ) {
    horizontal = 1;
  }
  else {
    horizontal = 0;
  }

  if ( horizontal ) {
    if ( w < minW ) w = minW;
    if ( h < minH ) h = minH;
  }
  else {
    if ( w < minVertW ) w = minVertW;
    if ( h < minVertH ) h = minVertH;
  }

  indicatorColor.setColorIndex( actWin->defaultFg1Color, actWin->ci );
  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  strcpy( fontTag, actWin->defaultCtlFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  this->draw();

  this->editCreate();

  return 1;

}

int activeIndicatorClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

static int zero = 0;
static char *emptyStr = "";

int lit = 1;
static char *labelTypeEnumStr[2] = {
  "pvName",
  "literal"
};
static int labelTypeEnum[2] = {
  0,
  1
};

int horz = 1;
static char *orienTypeEnumStr[2] = {
  "vertical",
  "horizontal"
};
static int orienTypeEnum[2] = {
  0,
  1
};

  major = INDICATORC_MAJOR_VERSION;
  minor = INDICATORC_MINOR_VERSION;
  release = INDICATORC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "indicatorColor", actWin->ci, &indicatorColor );
  tag.loadBoolW( "indicatorAlarm", &indicatorColorMode, &zero );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadBoolW( "fgAlarm", &fgColorMode, &zero );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadW( "indicatorPv", &readPvExpStr, emptyStr );
  tag.loadW( "nullPv", &nullPvExpStr, emptyStr );
  tag.loadW( "label", &label, emptyStr );
  tag.loadW( "labelType", 2, labelTypeEnumStr, labelTypeEnum,
   &labelType, &lit );
  tag.loadBoolW( "showScale", &showScale, &zero );
  tag.loadW( "font", fontTag );
  tag.loadW( "labelTicks", &labelTicksExpStr, emptyStr );
  tag.loadW( "majorTicks", &majorTicksExpStr, emptyStr );
  tag.loadW( "minorTicks", &minorTicksExpStr, emptyStr );
  tag.loadBoolW( "border", &border, &zero );
  tag.loadBoolW( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadW( "precision", &precisionExpStr, emptyStr );
  tag.loadW( "min", &readMinExpStr, emptyStr );
  tag.loadW( "max", &readMaxExpStr, emptyStr );
  tag.loadW( "scaleFormat", scaleFormat );
  tag.loadW( "orientation", 2, orienTypeEnumStr, orienTypeEnum,
   &horizontal, &horz );
  tag.loadW( "halfWidth", &halfW, &zero );
  tag.loadW( "pointerOpposite", &pointerOpposite, &zero );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeIndicatorClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

static int zero = 0;
static char *emptyStr = "";

int lit = 1;
static char *labelTypeEnumStr[2] = {
  "pvName",
  "literal"
};
static int labelTypeEnum[2] = {
  0,
  1
};

int horz = 1;
static char *orienTypeEnumStr[2] = {
  "vertical",
  "horizontal"
};
static int orienTypeEnum[2] = {
  0,
  1
};

int l;
char fmt[31+1], str[31+1];

efInt efLabelTicks, efMajorTicks, efMinorTicks, efPrec;
efDouble efMin, efMax;

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
  tag.loadR( "indicatorColor", actWin->ci, &indicatorColor );
  tag.loadR( "indicatorAlarm", &indicatorColorMode, &zero );
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "fgAlarm", &fgColorMode, &zero );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "indicatorPv", &readPvExpStr, emptyStr );
  tag.loadR( "nullPv", &nullPvExpStr, emptyStr );
  tag.loadR( "label", &label, emptyStr );
  tag.loadR( "labelType", 2, labelTypeEnumStr, labelTypeEnum,
   &labelType, &lit );
  tag.loadR( "showScale", &showScale, &zero );
  tag.loadR( "font", 63, fontTag );

  tag.loadR( "labelTicks", &labelTicksExpStr, emptyStr );
  tag.loadR( "majorTicks", &majorTicksExpStr, emptyStr );
  tag.loadR( "minorTicks", &minorTicksExpStr, emptyStr );

  tag.loadR( "border", &border, &zero );
  tag.loadR( "limitsFromDb", &limitsFromDb, &zero );

  tag.loadR( "precision", &precisionExpStr, emptyStr );
  tag.loadR( "min", &readMinExpStr, emptyStr );
  tag.loadR( "max", &readMaxExpStr, emptyStr );

  tag.loadR( "scaleFormat", 15, scaleFormat );
  tag.loadR( "orientation", 2, orienTypeEnumStr, orienTypeEnum,
   &horizontal, &horz );

  tag.loadR( "halfWidth", &halfW, &zero );

  tag.loadR( "pointerOpposite", &pointerOpposite, &zero );

  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > INDICATORC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( indicatorColorMode == INDICATORC_K_COLORMODE_ALARM )
    indicatorColor.setAlarmSensitive();
  else
    indicatorColor.setAlarmInsensitive();

  if ( fgColorMode == INDICATORC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  bgColor.setAlarmInsensitive();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  if ( fs ) {
    indicatorStrLen = XTextWidth( fs, "10", 2 );
  }

  // set edit-mode display values
  readMin = 0;
  readMax = 10;
  labelTicks = 10;
  majorTicks = 2;
  minorTicks = 2;

  if ( strcmp( scaleFormat, "GFloat" ) == 0 ) {
    sprintf( fmt, "%%.%-dg", precision );
  }
  else if ( strcmp( scaleFormat, "Exponential" ) == 0 ) {
    sprintf( fmt, "%%.%-de", precision );
  }
  else {
    sprintf( fmt, "%%.%-df", precision );
  }

  formatString( readMin, str, 31, fmt );
  if ( fs ) {
    indicatorStrLen = XTextWidth( fs, str, strlen(str) );
  }

  formatString( readMax, str, 31, fmt );
  if ( fs ) {
    l = XTextWidth( fs, str, strlen(str) );
    if ( l > indicatorStrLen ) indicatorStrLen = l;
  }

  if ( halfW < 0 ) halfW = 0;

  readV = 0.0;
  curReadV = 0.0;
  curNullV = 0.0;
  updateDimensions();

  return stat;

}

int activeIndicatorClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  if ( horizontal )
    strcpy( title, activeIndicatorClass_str3 );
  else
    strcpy( title, activeIndicatorClass_str4 );

  ptr = actWin->obj.getNameFromClass( "activeIndicatorClass" );
  if ( ptr )
    Strncat( title, ptr, 31 );
  else
    Strncat( title, activeIndicatorClass_str5, 31 );

  Strncat( title, activeIndicatorClass_str6, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufIndicatorColor = indicatorColor.pixelIndex();
  bufIndicatorColorMode = indicatorColorMode;

  bufFgColor = fgColor.pixelIndex();
  bufFgColorMode = fgColorMode;

  bufBgColor = bgColor.pixelIndex();

  strncpy( bufFontTag, fontTag, 63 );

  if ( readPvExpStr.getRaw() )
    strncpy( eBuf->bufReadPvName, readPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufReadPvName, "" );

  if ( controlPvExpStr.getRaw() )
    strncpy( eBuf->bufControlPvName, controlPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufControlPvName, "" );

  if ( nullPvExpStr.getRaw() )
    strncpy( eBuf->bufNullPvName, nullPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufNullPvName, "" );

  if ( label.getRaw() )
    strncpy( eBuf->bufLabel, label.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufLabel, "" );

  bufLabelType = labelType;

  bufBorder = border;

  bufShowScale = showScale;

  if ( labelTicksExpStr.getRaw() ) {
    strncpy( bufLabelTicks, labelTicksExpStr.getRaw(), 15 );
    bufLabelTicks[15] = 0;
  }
  else {
    strcpy( bufLabelTicks, "" );
  }

  if ( majorTicksExpStr.getRaw() ) {
    strncpy( bufMajorTicks, majorTicksExpStr.getRaw(), 15 );
    bufMajorTicks[15] = 0;
  }
  else {
    strcpy( bufMajorTicks, "" );
  }

  if ( minorTicksExpStr.getRaw() ) {
    strncpy( bufMinorTicks, minorTicksExpStr.getRaw(), 15 );
    bufMinorTicks[15] = 0;
  }
  else {
    strcpy( bufMinorTicks, "" );
  }

  bufLimitsFromDb = limitsFromDb;

  if ( precisionExpStr.getRaw() ) {
    strncpy( bufPrecision, precisionExpStr.getRaw(), 15 );
    bufPrecision[15] = 0;
  }
  else {
    strcpy( bufPrecision, "" );
  }

  if ( readMinExpStr.getRaw() ) {
    strncpy( bufReadMin, readMinExpStr.getRaw(), 15 );
    bufReadMin[15] = 0;
  }
  else {
    strcpy( bufReadMin, "" );
  }

  if ( readMaxExpStr.getRaw() ) {
    strncpy( bufReadMax, readMaxExpStr.getRaw(), 15 );
    bufReadMax[15] = 0;
  }
  else {
    strcpy( bufReadMax, "" );
  }

  strncpy( bufScaleFormat, scaleFormat, 15 );
  bufHorizontal = horizontal;

  bufHalfW = halfW;

  bufPointerOpposite = pointerOpposite;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeIndicatorClass_str7, 35, &bufX );
  ef.addTextField( activeIndicatorClass_str8, 35, &bufY );
  ef.addTextField( activeIndicatorClass_str9, 35, &bufW );
  ef.addTextField( activeIndicatorClass_str10, 35, &bufH );
  ef.addTextField( activeIndicatorClass_str12, 35, eBuf->bufReadPvName, PV_Factory::MAX_PV_NAME );
  ef.addTextField( activeIndicatorClass_str13, 35, eBuf->bufNullPvName, PV_Factory::MAX_PV_NAME );

  ef.addOption( activeIndicatorClass_str14, activeIndicatorClass_str15, &bufLabelType );
  labelTypeEntry = ef.getCurItem();
  labelTypeEntry->setNumValues( 2 );
  ef.addTextField( activeIndicatorClass_str16, 35, eBuf->bufLabel, PV_Factory::MAX_PV_NAME );
  labelEntry = ef.getCurItem();
  labelTypeEntry->addInvDependency( 0, labelEntry );
  labelTypeEntry->addDependencyCallbacks();

  ef.addToggle( activeIndicatorClass_str18, &bufBorder );

  ef.addToggle( activeIndicatorClass_str19, &bufShowScale );
  showScaleEntry = ef.getCurItem();
  ef.addTextField( activeIndicatorClass_str20, 35, bufLabelTicks, 15 );
  labelTicksEntry = ef.getCurItem();
  showScaleEntry->addDependency( labelTicksEntry );
  ef.addTextField( activeIndicatorClass_str21, 35, bufMajorTicks, 15 );
  majorTicksEntry = ef.getCurItem();
  showScaleEntry->addDependency( majorTicksEntry );
  ef.addTextField( activeIndicatorClass_str22, 35, bufMinorTicks, 15 );
  minorTicksEntry = ef.getCurItem();
  showScaleEntry->addDependency( minorTicksEntry );

  ef.addToggle( activeIndicatorClass_str23, &bufLimitsFromDb );
  limitsFromDbEntry = ef.getCurItem();

  ef.addOption( activeIndicatorClass_str24, activeIndicatorClass_str25, bufScaleFormat, 15 );
  scaleFormatEntry = ef.getCurItem();
  showScaleEntry->addDependency( scaleFormatEntry );
  showScaleEntry->addDependencyCallbacks();

  ef.addTextField( activeIndicatorClass_str26, 35, bufPrecision, 15 );
  scalePrecEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scalePrecEntry );
  ef.addTextField( activeIndicatorClass_str27, 35, bufReadMin, 15 );
  scaleMinEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scaleMinEntry );
  ef.addTextField( activeIndicatorClass_str28, 35, bufReadMax, 15 );
  scaleMaxEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scaleMaxEntry );
  limitsFromDbEntry->addDependencyCallbacks();

  ef.addTextField( activeIndicatorClass_str46, 35, &bufHalfW );

  ef.addToggle( activeIndicatorClass_str47, &bufPointerOpposite );

  ef.addOption( activeIndicatorClass_str44, activeIndicatorClass_str45,
   &bufHorizontal );

  ef.addColorButton( activeIndicatorClass_str30, actWin->ci, &indicatorCb, &bufIndicatorColor );
  ef.addToggle( activeIndicatorClass_str31, &bufIndicatorColorMode );
  ef.addColorButton( activeIndicatorClass_str32, actWin->ci, &fgCb, &bufFgColor );
  ef.addToggle( activeIndicatorClass_str33, &bufFgColorMode );
  ef.addColorButton( activeIndicatorClass_str34, actWin->ci, &bgCb, &bufBgColor );

  ef.addFontMenu( activeIndicatorClass_str17, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeIndicatorClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( indicatorc_edit_ok, indicatorc_edit_apply, indicatorc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeIndicatorClass::edit ( void ) {

  this->genericEdit();
  ef.finished( indicatorc_edit_ok, indicatorc_edit_apply, indicatorc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeIndicatorClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeIndicatorClass::eraseActive ( void ) {

XRectangle xR = { x-1, y-1, w+2, h+2 };
int clipStat, effHalfW;

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.setFG( bgColor.getColor() );

  if ( bufInvalid ) {

    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );

    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

    XFillRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

  }
  else {

    XPoint points[3];

    clipStat = actWin->executeGc.addNormXClipRectangle( xR );

    effHalfW = halfW;

    if ( horizontal ) {

      switch ( oldShape ) {
      case INDICATORC_K_SHAPE_LT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorH/2 ) {
          effHalfW = indicatorH/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = oldIndX+effHalfW+effHalfW;
        points[0].y = oldIndY;
        points[1].x = oldIndX;
        points[1].y = oldIndY+indicatorH/2;
        points[2].x = oldIndX+effHalfW+effHalfW;
        points[2].y = oldIndY+indicatorH;
	break;
      case INDICATORC_K_SHAPE_GT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorH/2 ) {
          effHalfW = indicatorH/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = oldIndX-effHalfW-effHalfW;
        points[0].y = oldIndY;
        points[1].x = oldIndX;
        points[1].y = oldIndY+indicatorH/2;
        points[2].x = oldIndX-effHalfW-effHalfW;
        points[2].y = oldIndY+indicatorH;
	break;
      default:
        if ( pointerOpposite ) {
          points[0].x = oldIndX-effHalfW;
          points[0].y = oldIndY+indicatorH;
          points[1].x = oldIndX;
          points[1].y = oldIndY;
          points[2].x = oldIndX+effHalfW;
          points[2].y = oldIndY+indicatorH;
	}
	else {
          points[0].x = oldIndX-effHalfW;
          points[0].y = oldIndY;
          points[1].x = oldIndX;
          points[1].y = oldIndY+indicatorH;
          points[2].x = oldIndX+effHalfW;
          points[2].y = oldIndY;
	}
	break;
      }
      if ( effHalfW ) {
        XFillPolygon( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 3, Complex, CoordModeOrigin );
      }
      else {
        XDrawLines( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 2, CoordModeOrigin );
      }

    }
    else {

      switch ( oldShape ) {
      case INDICATORC_K_SHAPE_LT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorW/2 ) {
          effHalfW = indicatorW/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = oldIndX+indicatorW;
        points[0].y = oldIndY-effHalfW-effHalfW;
        points[1].x = oldIndX+indicatorW/2;
        points[1].y = oldIndY;
        points[2].x = oldIndX;
        points[2].y = oldIndY-effHalfW-effHalfW;
	break;
      case INDICATORC_K_SHAPE_GT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorW/2 ) {
          effHalfW = indicatorW/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = oldIndX+indicatorW;
        points[0].y = oldIndY+effHalfW+effHalfW;
        points[1].x = oldIndX+indicatorW/2;
        points[1].y = oldIndY;
        points[2].x = oldIndX;
        points[2].y = oldIndY+effHalfW+effHalfW;
	break;
      default:
        if ( pointerOpposite ) {
          points[0].x = oldIndX;
          points[0].y = oldIndY-effHalfW;
          points[1].x = oldIndX+indicatorW;
          points[1].y = oldIndY;
          points[2].x = oldIndX;
          points[2].y = oldIndY+effHalfW;
	}
	else {
          points[0].x = oldIndX+indicatorW;
          points[0].y = oldIndY-effHalfW;
          points[1].x = oldIndX;
          points[1].y = oldIndY;
          points[2].x = oldIndX+indicatorW;
          points[2].y = oldIndY+effHalfW;
	}
	break;
      }
      if ( effHalfW ) {
        XFillPolygon( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 3, Complex, CoordModeOrigin );
        }
        else {
          XDrawLines( actWin->d, drawable(actWin->executeWidget),
           actWin->executeGc.normGC(), points, 2, CoordModeOrigin );
        }

    }

    if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();

  }

  return 1;

}

void activeIndicatorClass::drawHorzScale (
  Widget widget,
  Drawable dr,
  gcClass *gc )
{

char fmt[31+1];

  if ( strcmp( scaleFormat, "GFloat" ) == 0 ) {
    sprintf( fmt, "%%.%-dg", precision );
  }
  else if ( strcmp( scaleFormat, "Exponential" ) == 0 ) {
    sprintf( fmt, "%%.%-de", precision );
  }
  else {
    sprintf( fmt, "%%.%-df", precision );
  }

  drawXLinearScale ( actWin->d, dr, gc, 1, indicatorAreaX,
   indicatorY + indicatorH + 3, indicatorAreaW, readMin, readMax, labelTicks,
   majorTicks, minorTicks, fgColor.pixelColor(),
   bgColor.pixelColor(), 0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 0,
   fmt );

}

void activeIndicatorClass::drawVertScale (
  Widget widget,
  Drawable dr,
  gcClass *gc )
{

char fmt[31+1];

  if ( strcmp( scaleFormat, "GFloat" ) == 0 ) {
    sprintf( fmt, "%%.%-dg", precision );
  }
  else if ( strcmp( scaleFormat, "Exponential" ) == 0 ) {
    sprintf( fmt, "%%.%-de", precision );
  }
  else {
    sprintf( fmt, "%%.%-df", precision );
  }

  drawYLinearScale ( actWin->d, dr, gc, 1, indicatorAreaX - 4,
   indicatorAreaY, indicatorAreaH, readMin, readMax, labelTicks,
   majorTicks, minorTicks, fgColor.pixelColor(),
   bgColor.pixelColor(), 0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 0,
   fmt );

}

void activeIndicatorClass::drawScale (
  Widget widget,
  Drawable dr,
  gcClass *gc )
{

  if ( horizontal )
    drawHorzScale( widget, dr, gc );
  else
    drawVertScale( widget, dr, gc );

}

int activeIndicatorClass::draw ( void ) {

XRectangle xR = { x-1, y-1, w+2, h+2 };
int clipStat;
int tX, tY;

  if ( deleteRequest ) return 1;

  clipStat = actWin->drawGc.addNormXClipRectangle( xR );

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  actWin->drawGc.saveFg();

  if ( horizontal ) {

    XPoint points[3];

    actWin->drawGc.setFG( bgColor.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( indicatorColor.pixelColor() );

    indX = indicatorX + indicatorAreaW / 2;
    indY = indicatorY;

    if ( pointerOpposite ) {
      points[0].x = indX-halfW;
      points[0].y = indY+indicatorH;
      points[1].x = indX;
      points[1].y = indY;
      points[2].x = indX+halfW;
      points[2].y = indY+indicatorH;
    }
    else {
      points[0].x = indX-halfW;
      points[0].y = indY;
      points[1].x = indX;
      points[1].y = indY+indicatorH;
      points[2].x = indX+halfW;
      points[2].y = indY;
    }
    if ( halfW ) {
      XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), points, 3, Complex, CoordModeOrigin );
    }
    else {
      XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), points, 2, CoordModeOrigin );
    }

    actWin->drawGc.setFG( fgColor.getColor() );

    if ( showScale ) drawScale( actWin->drawWidget,
     XtWindow(actWin->drawWidget), &actWin->drawGc );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    if ( strcmp( label.getRaw(), "" ) != 0 ) {
      if ( fs ) {
        actWin->drawGc.setFontTag( fontTag, actWin->fi );
        tX = indicatorAreaX;
        tY = y + 2;
        if ( border ) tY += 2;
        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_BEGINNING, label.getRaw() );
      }
    }

  }
  else { // vertical

    XPoint points[3];

    actWin->drawGc.setFG( bgColor.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( indicatorColor.pixelColor() );

    indY = indicatorY - indicatorAreaH / 2;
    indX = indicatorX;

    if ( pointerOpposite ) {
      points[0].x = indX;
      points[0].y = indY-halfW;
      points[1].x = indX+indicatorW;
      points[1].y = indY;
      points[2].x = indX;
      points[2].y = indY+halfW;
    }
    else {
      points[0].x = indX+indicatorW;
      points[0].y = indY-halfW;
      points[1].x = indX;
      points[1].y = indY;
      points[2].x = indX+indicatorW;
      points[2].y = indY+halfW;
    }

    if ( halfW ) {
      XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), points, 3, Complex, CoordModeOrigin );
    }
    else {
      XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), points, 2, CoordModeOrigin );
    }

    actWin->drawGc.setFG( fgColor.getColor() );

    if ( showScale ) drawScale( actWin->drawWidget,
     XtWindow(actWin->drawWidget), &actWin->drawGc );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    if ( strcmp( label.getRaw(), "" ) != 0 ) {
      if ( fs ) {
        actWin->drawGc.setFontTag( fontTag, actWin->fi );
        tX = indicatorAreaX + indicatorAreaW;
        tY = y + (int) ( .25 * (double) fontHeight );
        if ( border ) tY += 2;
        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_END, label.getRaw() );
      }
    }

  }

  actWin->drawGc.restoreFg();

  if ( clipStat & 1 ) actWin->drawGc.removeNormXClipRectangle();

  return 1;

}

int activeIndicatorClass::drawActive ( void ) {

XRectangle xR = { x-1, y-1, w+2, h+2 };
int clipStat, effHalfW;
int tX, tY;
char str[PV_Factory::MAX_PV_NAME+1];

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

  clipStat = actWin->executeGc.addNormXClipRectangle( xR );

  effHalfW = halfW;

  actWin->executeGc.saveFg();

  if ( horizontal ) {

    XPoint points[3];

    if ( bufInvalid ) {

      actWin->executeGc.setFG( bgColor.getColor() );

      XFillRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      actWin->executeGc.setFG( indicatorColor.getColor() );

      switch ( shape ) {
      case INDICATORC_K_SHAPE_LT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorH/2 ) {
          effHalfW = indicatorH/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = indX+effHalfW+effHalfW;
        points[0].y = indY;
        points[1].x = indX;
        points[1].y = indY+indicatorH/2;
        points[2].x = indX+effHalfW+effHalfW;
        points[2].y = indY+indicatorH;
	break;
      case INDICATORC_K_SHAPE_GT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorH/2 ) {
          effHalfW = indicatorH/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = indX-effHalfW-effHalfW;
        points[0].y = indY;
        points[1].x = indX;
        points[1].y = indY+indicatorH/2;
        points[2].x = indX-effHalfW-effHalfW;
        points[2].y = indY+indicatorH;
	break;
      default:
        if ( pointerOpposite ) {
          points[0].x = indX-effHalfW;
          points[0].y = indY+indicatorH;
          points[1].x = indX;
          points[1].y = indY;
          points[2].x = indX+effHalfW;
          points[2].y = indY+indicatorH;
	}
	else {
          points[0].x = indX-effHalfW;
          points[0].y = indY;
          points[1].x = indX;
          points[1].y = indY+indicatorH;
          points[2].x = indX+effHalfW;
          points[2].y = indY;
	}
	break;
      }
      if ( effHalfW ) {
        XFillPolygon( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 3, Complex, CoordModeOrigin );
      }
      else {
        XDrawLines( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 2, CoordModeOrigin );
      }

    }
    else {

      actWin->executeGc.setFG( indicatorColor.getColor() );

      switch ( shape ) {
      case INDICATORC_K_SHAPE_LT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorH/2 ) {
          effHalfW = indicatorH/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = indX+effHalfW+effHalfW;
        points[0].y = indY;
        points[1].x = indX;
        points[1].y = indY+indicatorH/2;
        points[2].x = indX+effHalfW+effHalfW;
        points[2].y = indY+indicatorH;
	break;
      case INDICATORC_K_SHAPE_GT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorH/2 ) {
          effHalfW = indicatorH/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = indX-effHalfW-effHalfW;
        points[0].y = indY;
        points[1].x = indX;
        points[1].y = indY+indicatorH/2;
        points[2].x = indX-effHalfW-effHalfW;
        points[2].y = indY+indicatorH;
	break;
      default:
        if ( pointerOpposite ) {
          points[0].x = indX-effHalfW;
          points[0].y = indY+indicatorH;
          points[1].x = indX;
          points[1].y = indY;
          points[2].x = indX+effHalfW;
          points[2].y = indY+indicatorH;
	}
	else {
          points[0].x = indX-effHalfW;
          points[0].y = indY;
          points[1].x = indX;
          points[1].y = indY+indicatorH;
          points[2].x = indX+effHalfW;
          points[2].y = indY;
	}
	break;
      }
      if ( effHalfW ) {
        XFillPolygon( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 3, Complex, CoordModeOrigin );
      }
      else {
        XDrawLines( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 2, CoordModeOrigin );
      }

    }

    oldIndX = indX;
    oldIndY = indY;
    oldShape = shape;

  }
  else { // vertical

    XPoint points[3];

    if ( bufInvalid ) {

      actWin->executeGc.setFG( bgColor.getColor() );

      XFillRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      actWin->executeGc.setFG( indicatorColor.getColor() );

      switch ( shape ) {
      case INDICATORC_K_SHAPE_LT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorW/2 ) {
          effHalfW = indicatorW/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = indX+indicatorW;
        points[0].y = indY-effHalfW-effHalfW;
        points[1].x = indX+indicatorW/2;
        points[1].y = indY;
        points[2].x = indX;
        points[2].y = indY-effHalfW-effHalfW;
	break;
      case INDICATORC_K_SHAPE_GT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorW/2 ) {
          effHalfW = indicatorW/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = indX+indicatorW;
        points[0].y = indY+effHalfW+effHalfW;
        points[1].x = indX+indicatorW/2;
        points[1].y = indY;
        points[2].x = indX;
        points[2].y = indY+effHalfW+effHalfW;
	break;
      default:
        if ( pointerOpposite ) {
          points[0].x = indX;
          points[0].y = indY-effHalfW;
          points[1].x = indX+indicatorW;
          points[1].y = indY;
          points[2].x = indX;
          points[2].y = indY+effHalfW;
	}
	else {
          points[0].x = indX+indicatorW;
          points[0].y = indY-effHalfW;
          points[1].x = indX;
          points[1].y = indY;
          points[2].x = indX+indicatorW;
          points[2].y = indY+effHalfW;
	}
	break;
      }
      if ( effHalfW ) {
        XFillPolygon( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 3, Complex, CoordModeOrigin );
      }
      else {
        XDrawLines( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 2, CoordModeOrigin );
      }

    }
    else {

      actWin->executeGc.setFG( indicatorColor.getColor() );

      switch ( shape ) {
      case INDICATORC_K_SHAPE_LT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorW/2 ) {
          effHalfW = indicatorW/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = indX+indicatorW;
        points[0].y = indY-effHalfW-effHalfW;
        points[1].x = indX+indicatorW/2;
        points[1].y = indY;
        points[2].x = indX;
        points[2].y = indY-effHalfW-effHalfW;
	break;
      case INDICATORC_K_SHAPE_GT:
        if ( effHalfW < 5 ) effHalfW = 5;
        if ( effHalfW > indicatorW/2 ) {
          effHalfW = indicatorW/2;
          if ( effHalfW < 2 ) effHalfW = 2;
	}
        points[0].x = indX+indicatorW;
        points[0].y = indY+effHalfW+effHalfW;
        points[1].x = indX+indicatorW/2;
        points[1].y = indY;
        points[2].x = indX;
        points[2].y = indY+effHalfW+effHalfW;
	break;
      default:
        if ( pointerOpposite ) {
          points[0].x = indX;
          points[0].y = indY-effHalfW;
          points[1].x = indX+indicatorW;
          points[1].y = indY;
          points[2].x = indX;
          points[2].y = indY+effHalfW;
	}
	else {
          points[0].x = indX+indicatorW;
          points[0].y = indY-effHalfW;
          points[1].x = indX;
          points[1].y = indY;
          points[2].x = indX+indicatorW;
          points[2].y = indY+effHalfW;
	}
	break;
      }
      if ( effHalfW ) {
        XFillPolygon( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 3, Complex, CoordModeOrigin );
      }
      else {
        XDrawLines( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), points, 2, CoordModeOrigin );
      }

    }

    oldIndX = indX;
    oldIndY = indY;
    oldShape = shape;

  }

  if ( bufInvalid ) { // draw scale, label, etc ...

    actWin->executeGc.setFG( fgColor.getColor() );

    if ( showScale ) {
      drawScale( actWin->executeWidget,
       drawable(actWin->executeWidget), &actWin->executeGc );
    }

    if ( labelType == INDICATORC_K_PV_NAME )
      strncpy( str, readPvId->get_name(), PV_Factory::MAX_PV_NAME );
    else
      strncpy( str, label.getExpanded(), PV_Factory::MAX_PV_NAME );

    if ( horizontal ) {

      if ( strcmp( str, "" ) != 0 ) {
        if ( fs ) {
          actWin->executeGc.setFontTag( fontTag, actWin->fi );
          tX = x + 2;
          tY = y + 2;
          if ( border ) tY += 2;
          drawText( actWin->executeWidget, drawable(actWin->executeWidget),
           &actWin->executeGc, fs, tX, tY, XmALIGNMENT_BEGINNING, str );
        }
      }

    }
    else {

      if ( strcmp( str, "" ) != 0 ) {
        if ( fs ) {
          actWin->executeGc.setFontTag( fontTag, actWin->fi );
          tX = indicatorAreaX + indicatorAreaW;
          tY = y + (int) ( .25 * (double) fontHeight );
          if ( border ) tY += 2;
          drawText( actWin->executeWidget, drawable(actWin->executeWidget),
           &actWin->executeGc, fs, tX, tY, XmALIGNMENT_END, str );
        }
      }

    }

    if ( border ) {
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
    }

    bufInvalid = 0;

  }

  actWin->executeGc.restoreFg();

  if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();

  return 1;

}

int activeIndicatorClass::activate (
  int pass,
  void *ptr )
{

int opStat;
char fmt[31+1];

  switch ( pass ) {

  case 1:

    needConnectInit = needInfoInit = needRefresh = needErase = needDrawCheck =
     needDraw = needFullDraw = 0;
    needToEraseUnconnected = 0;
    needToDrawUnconnected = 0;
    unconnectedTimer = 0;

    readPvId = nullPvId = NULL;
    initialReadConnection = initialNullConnection = 1;
    oldStat = oldSev = -1;

    aglPtr = ptr;
    opComplete = 0;
    curNullV = 0.0;

    if ( horizontal ) {
      indicatorW = 0;
      oldIndicatorW = 0;
      indicatorX = 0;
      oldIndicatorX = 0;
    }
    else {
      indicatorH = 0;
      oldIndicatorH = 0;
      indicatorY = 0;
      oldIndicatorY = 0;
    }

    oldShape = INDICATORC_K_SHAPE_UNKNOWN;
    shape = INDICATORC_K_SHAPE_PTR;

    pvNotConnectedMask = active = init = 0;
    activeMode = 1;

    if ( !readPvExpStr.getExpanded() ||
       // ( strcmp( readPvExpStr.getExpanded(), "" ) == 0 ) ) {
       blankOrComment( readPvExpStr.getExpanded() ) ) {
      readExists = 0;
    }
    else {
      readExists = 1;
      pvNotConnectedMask |= 1;
      indicatorColor.setConnectSensitive();
      fgColor.setConnectSensitive();
    }

    if ( !nullPvExpStr.getExpanded() ||
       // ( strcmp( nullPvExpStr.getExpanded(), "" ) == 0 ) ) {
       blankOrComment( nullPvExpStr.getExpanded() ) ) {
      nullExists = 0;
    }
    else {
      nullExists = 1;
      pvNotConnectedMask |= 2;
    }

    break;

  case 2:

    if ( !opComplete ) {

      if ( blank( labelTicksExpStr.getExpanded() ) ) {
        labelTicks = 0;
      }
      else {
        labelTicks = atol( labelTicksExpStr.getExpanded() );
      }

      if ( blank( majorTicksExpStr.getExpanded() ) ) {
        majorTicks = 0;
      }
      else {
        majorTicks = atol( majorTicksExpStr.getExpanded() );
      }

      if ( blank( minorTicksExpStr.getExpanded() ) ) {
        minorTicks = 0;
      }
      else {
        minorTicks = atol( minorTicksExpStr.getExpanded() );
      }

      if ( blank( precisionExpStr.getExpanded() ) ) {
        precision = 0;
      }
      else {
        precision = atol( precisionExpStr.getExpanded() );
      }

      if ( blank( readMinExpStr.getExpanded() ) ) {
        readMin = 0;
      }
      else {
        readMin = atof( readMinExpStr.getExpanded() );
      }

      if ( blank( readMaxExpStr.getExpanded() ) ) {
        readMax = 0;
      }
      else {
        readMax = atof( readMaxExpStr.getExpanded() );
      }

      if ( readMax == readMin ) {
        readMax = readMin + 1;
      }

      if ( strcmp( scaleFormat, "GFloat" ) == 0 ) {
        sprintf( fmt, "%%.%-dg", precision );
      }
      else if ( strcmp( scaleFormat, "Exponential" ) == 0 ) {
        sprintf( fmt, "%%.%-de", precision );
      }
      else {
        sprintf( fmt, "%%.%-df", precision );
      }

      initEnable();

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      opStat = 1;

      if ( readExists ) {
	readPvId = the_PV_Factory->create( readPvExpStr.getExpanded() );
	if ( readPvId ) {
	  readPvId->add_conn_state_callback( indicator_monitor_read_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeIndicatorClass_str36 );
          opStat = 0;
        }
      }

      if ( nullExists ) {
	nullPvId = the_PV_Factory->create( nullPvExpStr.getExpanded() );
	if ( nullPvId ) {
	  nullPvId->add_conn_state_callback( indicator_monitor_null_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeIndicatorClass_str36 );
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

int activeIndicatorClass::deactivate (
  int pass
) {

char fmt[31+1], str[31+1];
int l;

  active = 0;
  activeMode = 0;

  if ( pass == 1 ) {

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    if ( readExists ) {
      if ( readPvId ) {
        readPvId->remove_conn_state_callback( indicator_monitor_read_connect_state,
         this );
        readPvId->remove_value_callback( indicator_readUpdate, this );
        readPvId->release();
        readPvId = NULL;
      }
    }

    if ( nullExists ) {
      if ( nullPvId ) {
        nullPvId->remove_conn_state_callback( indicator_monitor_null_connect_state,
         this );
        nullPvId->remove_value_callback( indicator_nullUpdate, this );
        nullPvId->release();
        nullPvId = NULL;
      }
    }

  }

  // set edit-mode display values
  readMin = 0;
  readMax = 10;
  labelTicks = 10;
  majorTicks = 2;
  minorTicks = 2;

  if ( strcmp( scaleFormat, "GFloat" ) == 0 ) {
    sprintf( fmt, "%%.%-dg", precision );
  }
  else if ( strcmp( scaleFormat, "Exponential" ) == 0 ) {
    sprintf( fmt, "%%.%-de", precision );
  }
  else {
    sprintf( fmt, "%%.%-df", precision );
  }

  formatString( readMin, str, 31, fmt );
  if ( fs ) {
    indicatorStrLen = XTextWidth( fs, str, strlen(str) );
  }
  formatString( readMax, str, 31, fmt );
  if ( fs ) {
    l = XTextWidth( fs, str, strlen(str) );
    if ( l > indicatorStrLen ) indicatorStrLen = l;
  }

  updateDimensions();

  return 1;

}

void activeIndicatorClass::updateDimensions ( void )
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

  indicatorAreaX = x;
  indicatorAreaY = y;
  indicatorAreaW = w;
  indicatorAreaH = h;

  if ( horizontal ) {

    minH = 2;
    indicatorY = y;

    indicatorAreaX = x;
    indicatorAreaW = w;

    if ( ( strcmp( label.getRaw(), "" ) != 0 ) ||
         ( labelType == INDICATORC_K_PV_NAME ) ) {
      minH += fontHeight + 5;
      indicatorY += fontHeight + 5;
      if ( border ) {
        minH += 9;
        indicatorY += 5;
        indicatorAreaX = x + 5;
        indicatorAreaW = w - 9;
      }
    }
    else {
      if ( border && showScale ) {
        minH += 9;
        indicatorY += 5;
      }
    }

    if ( showScale ) {
      minH += fontHeight + fontHeight + 5;
      indicatorAreaX = x + indicatorStrLen/2 + 3;
      indicatorAreaW = w - indicatorStrLen - 6;
    }

    if ( border && !showScale && ( ( strcmp( label.getRaw(), "" ) == 0 ) ||
     ( labelType == INDICATORC_K_PV_NAME ) ) ) {
      minH += 9;
      indicatorY += 5;
      indicatorAreaX = x + 5;
      indicatorAreaW = w - 9;
    }

    if ( h < minH ) {

      h = minH;
      sboxH = minH;

    }

    indicatorH = h;

    if ( ( strcmp( label.getRaw(), "" ) != 0 ) ||
         ( labelType == INDICATORC_K_PV_NAME ) ) {
      indicatorH -= ( fontHeight + 5 );
      if ( border ) indicatorH -= 9;
    }

    if ( showScale ) {
      indicatorH -= ( fontHeight + fontHeight + 5 );
    }

    if ( border && !showScale && ( ( strcmp( label.getRaw(), "" ) == 0 ) ||
     ( labelType == INDICATORC_K_PV_NAME ) ) ) {
      indicatorH -= 9;
    }

    indicatorAreaX += halfW;
    indicatorAreaW -= 2*halfW;

    indicatorX = indicatorAreaX;
    range = readMax - readMin;
    factor = indicatorAreaW / range;
    offset = indicatorX;

  }
  else {  // vertical

    minVertW = 2;
    minVertH = 10;

    if ( ( strcmp( label.getRaw(), "" ) != 0 ) ||
         ( labelType == INDICATORC_K_PV_NAME ) ) {
      minVertH += fontHeight + 5;
    }

    if ( showScale ) {
      minVertH += fontHeight;
      minVertW += 4 + indicatorStrLen + 10 + (int) rint( 0.5 * fontHeight );
    }
    else if ( border ) {
      minVertH += 8;
      minVertW += 4;
    }

    if ( w < minVertW ) {

      w = minVertW;
      sboxW = minVertW;

    }

    if ( h < minVertH ) {

      h = minVertH;
      sboxH = minVertH;

    }

    indicatorH = indicatorAreaH = h;
    indicatorY = indicatorAreaY = y + indicatorAreaH;
    indicatorX = indicatorAreaX = x;
    indicatorW = indicatorAreaW = w;

    if ( ( strcmp( label.getRaw(), "" ) != 0 ) ||
         ( labelType == INDICATORC_K_PV_NAME ) ) {
      indicatorAreaH -= (int) ( 1.5 * (double) fontHeight ) - 5;
      indicatorH = indicatorAreaH;
    }

    if ( showScale ) {
      indicatorH -= ( fontHeight );
      indicatorAreaH -= ( fontHeight );
    }
    else if ( border ) {
      indicatorH -= 8;
      indicatorAreaH -= 8;
    }

    if ( showScale ) {
      indicatorY -= (int) rint( 0.5 * fontHeight );
      indicatorAreaY -= (int) rint( 0.5 * fontHeight );

      indicatorAreaW -= ( 4 + indicatorStrLen + 8 + (int) rint( 0.5 * fontHeight ) );
      indicatorW -= ( 4 + indicatorStrLen + 8 + (int) rint( 0.5 * fontHeight ) );
      indicatorAreaX += 2 + indicatorStrLen + 8 + (int) rint( 0.5 * fontHeight );
      indicatorX += 2 + indicatorStrLen + 8 + (int) rint( 0.5 * fontHeight );

    }
    else if ( border ) {
      indicatorY -= 4;
      indicatorAreaY -= 4;
      indicatorAreaW -= 9;
      indicatorW -= 9;
      indicatorAreaX += 5;
      indicatorX += 5;
    }

    indicatorAreaY -= halfW;
    indicatorAreaH -= 2*halfW;

    indicatorY = indicatorAreaY;
    range = readMax - readMin;
    factor = indicatorAreaH / range;
    offset = indicatorY;

  }

  if ( readMax >= readMin ) {
    mode = INDICATORC_K_MAX_GE_MIN;
  }
  else {
    mode = INDICATORC_K_MAX_LT_MIN;
  }

  bufInvalidate();

}

void activeIndicatorClass::btnUp (
  int x,
  int y,
  int indicatorState,
  int indicatorNumber )
{

  if ( !enabled ) return;

}

void activeIndicatorClass::btnDown (
  int x,
  int y,
  int indicatorState,
  int indicatorNumber )
{

  if ( !enabled ) return;

}

void activeIndicatorClass::btnDrag (
  int x,
  int y,
  int indicatorState,
  int indicatorNumber )
{

  if ( !enabled ) return;

}

int activeIndicatorClass::getIndicatorActionRequest (
  int *up,
  int *down,
  int *drag )
{

  if ( !controlExists ) {
    *up = 0;
    *down = 0;
    *drag = 0;
    return 1;
  }

  *up = 1;
  *down = 1;
  *drag = 1;

  return 1;

}

void activeIndicatorClass::bufInvalidate ( void )
{

  bufInvalid = 1;

}

int activeIndicatorClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( label.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  label.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( readPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( nullPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  nullPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( labelTicksExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  labelTicksExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( majorTicksExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  majorTicksExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( minorTicksExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  minorTicksExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( readMinExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readMinExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( readMaxExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readMaxExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( precisionExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  precisionExpStr.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeIndicatorClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int retStat, stat;

  retStat = label.expand1st( numMacros, macros, expansions );
  stat = readPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = nullPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = labelTicksExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = majorTicksExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = minorTicksExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readMinExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readMaxExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = precisionExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeIndicatorClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int retStat, stat;

  retStat = label.expand2nd( numMacros, macros, expansions );
  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = nullPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = labelTicksExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = majorTicksExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = minorTicksExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readMinExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = readMaxExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = precisionExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeIndicatorClass::containsMacros ( void ) {

int result;

  result = label.containsPrimaryMacros();
  if ( result ) return 1;

  result = readPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = nullPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = labelTicksExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = majorTicksExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = minorTicksExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = readMinExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = readMaxExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = precisionExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  return 0;

}

int activeIndicatorClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  ret_stat = 1;

  tmpw = sboxW;
  tmph = sboxH;

  tmpw += _w;
  tmph += _h;

  if ( horizontal ) {

    if ( tmpw < minW ) {
      ret_stat = 0;
    }

    if ( tmph < minH ) {
      ret_stat = 0;
    }

  }
  else {

    if ( tmpw < minVertW ) {
      ret_stat = 0;
    }

    if ( tmph < minVertH ) {
      ret_stat = 0;
    }

  }

  return ret_stat;

}

int activeIndicatorClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  ret_stat = 1;

  tmpw = _w;
  tmph = _h;

  if ( horizontal ) {

    if ( tmpw != -1 ) {
      if ( tmpw < minW ) {
        ret_stat = 0;
      }
    }

    if ( tmph != -1 ) {
      if ( tmph < minH ) {
        ret_stat = 0;
      }
    }

  }
  else {

    if ( tmpw != -1 ) {
      if ( tmpw < minVertW ) {
        ret_stat = 0;
      }
    }

    if ( tmph != -1 ) {
      if ( tmph < minVertH ) {
        ret_stat = 0;
      }
    }

  }

  return ret_stat;

}

void activeIndicatorClass::updateIndicator ( void ) {

int checkX, checkY;

  checkX = indX;
  checkY = indY;

  if ( horizontal ) {

    switch ( mode ) {

    case INDICATORC_K_MAX_GE_MIN:

      if ( readV < readMin ) {
        indX = indicatorX;
	shape = INDICATORC_K_SHAPE_LT;
      }
      else if ( readV > readMax ) {
        indX = indicatorX + indicatorAreaW;
	shape = INDICATORC_K_SHAPE_GT;
      }
      else {
        indX = indicatorX + (int) ( factor * ( readV - readMin ) + 0.5 );
	shape = INDICATORC_K_SHAPE_PTR;
      }
      indY = indicatorY;

      break;

    case INDICATORC_K_MAX_LT_MIN:

      if ( readV < readMax ) {
        indX = indicatorX + indicatorAreaW;
	shape = INDICATORC_K_SHAPE_GT;
      }
      else if ( readV > readMin ) {
        indX = indicatorX;
	shape = INDICATORC_K_SHAPE_LT;
      }
      else {
        indX = indicatorX + indicatorAreaW +
         (int) ( factor * ( readV - readMax ) + 0.5 );
	shape = INDICATORC_K_SHAPE_PTR;
      }
      indY = indicatorY;

      break;

    }

  }
  else { // vertical

    switch ( mode ) {

    case INDICATORC_K_MAX_GE_MIN:

      indX = indicatorX;
      if ( readV < readMin ) {
        indY = indicatorY;
	shape = INDICATORC_K_SHAPE_LT;
      }
      else if ( readV > readMax ) {
        indY = indicatorY - indicatorAreaH;
	shape = INDICATORC_K_SHAPE_GT;
      }
      else {
        indY = indicatorY - (int) ( factor * ( readV - readMin ) + 0.5 );
	shape = INDICATORC_K_SHAPE_PTR;
      }

      break;

    case INDICATORC_K_MAX_LT_MIN:

      indX = indicatorX;
      if ( readV < readMax ) {
        indY = indicatorY - indicatorAreaH;
	shape = INDICATORC_K_SHAPE_GT;
      }
      else if ( readV > readMin ) {
        indY = indicatorY;
	shape = INDICATORC_K_SHAPE_LT;
      }
      else {
        indY = indicatorY - indicatorAreaH -
         (int) ( factor * ( readV - readMax ) + 0.5 );
	shape = INDICATORC_K_SHAPE_PTR;
      }

      break;

    }

  }

  if ( horizontal ) {
    if ( checkY != indY ) bufInvalidate();
  }
  else { // vertical
    if ( checkX != indX ) bufInvalidate();
  }

}

void activeIndicatorClass::executeDeferred ( void ) {

int l, nc, ni, nr, ne, nd, nfd, ndc;
char fmt[31+1], str[31+1];
double v;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nfd = needFullDraw; needFullDraw = 0;
  ndc = needDrawCheck; needDrawCheck = 0;
  v = curReadV - curNullV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nc ) {

    v = curReadV = readPvId->get_double();

    if ( limitsFromDb ) {
      readMin = readPvId->get_lower_disp_limit();
    }

    if ( limitsFromDb ) {
      readMax = readPvId->get_upper_disp_limit();
    }

    if ( limitsFromDb ) {
      precision = readPvId->get_precision();
    }

    if ( readMin == readMax ) readMax = readMin + 1;

    ni = 1;

  }

//----------------------------------------------------------------------------

  if ( ni ) {

    if ( strcmp( scaleFormat, "GFloat" ) == 0 ) {
      sprintf( fmt, "%%.%-dg", precision );
    }
    else if ( strcmp( scaleFormat, "Exponential" ) == 0 ) {
      sprintf( fmt, "%%.%-de", precision );
    }
    else {
      sprintf( fmt, "%%.%-df", precision );
    }

    formatString( readMin, str, 31, fmt );
    sprintf( str, fmt, readMin );
    if ( fs ) {
      indicatorStrLen = XTextWidth( fs, str, strlen(str) );
    }

    formatString( readMax, str, 31, fmt );
    if ( fs ) {
      l = XTextWidth( fs, str, strlen(str) );
      if ( l > indicatorStrLen ) indicatorStrLen = l;
    }

    updateDimensions();

    active = 1;
    init = 1;
    indicatorColor.setConnected();
    fgColor.setConnected();
    bufInvalidate();
    eraseActive();
    readV = v;
    updateDimensions();
    smartDrawAllActive();

    if ( initialReadConnection ) {

      initialReadConnection = 0;

      readPvId->add_value_callback( indicator_readUpdate, this );

    }

    if ( nullExists ) {

      if ( initialNullConnection ) {

        initialNullConnection = 0;

        nullPvId->add_value_callback( indicator_nullUpdate, this );

      }

    }

  }

//----------------------------------------------------------------------------

  if ( nr ) {

    bufInvalidate();

    eraseActive();

    readV = v;

    updateDimensions();

    smartDrawAllActive();

  }

//----------------------------------------------------------------------------

  if ( ne ) {
    eraseActive();
  }

//----------------------------------------------------------------------------

  if ( nd ) {
    readV = v;
    eraseActive();
    smartDrawAllActive();
  }

//----------------------------------------------------------------------------

  if ( nfd ) {
    readV = v;
    bufInvalidate();
    smartDrawAllActive();
  }

//----------------------------------------------------------------------------

  if ( ndc ) {

      readV = v;
      updateIndicator();
      eraseActive();
      smartDrawAllActive();

  }

//----------------------------------------------------------------------------

}

char *activeIndicatorClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeIndicatorClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeIndicatorClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    if ( !i ) {
      return readPvExpStr.getExpanded();
    }
    else {
      return nullPvExpStr.getExpanded();
    }

  }
  else {

    if ( !i ) {
      return readPvExpStr.getRaw();
    }
    else {
      return nullPvExpStr.getRaw();
    }

  }

}

void activeIndicatorClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_FG1COLOR_MASK )
    indicatorColor.setColorIndex( _fg1Color, actWin->ci );

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColor.setColorIndex( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_CTLFONTTAG_MASK ) {
    strcpy( fontTag, _ctlFontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    updateDimensions();
  }

}

void activeIndicatorClass::changePvNames (
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

  if ( flag & ACTGRF_READBACKPVS_MASK ) {
    if ( numReadbackPvs ) {
      readPvExpStr.setRaw( readbackPvs[0] );
    }
  }

  if ( flag & ACTGRF_NULLPVS_MASK ) {
    if ( numNullPvs ) {
      nullPvExpStr.setRaw( nullPvs[0] );
    }
  }

}

void activeIndicatorClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 2 ) {
    *n = 0;
    return;
  }

  *n = 2;
  pvs[0] = readPvId;
  pvs[1] = nullPvId;

}

char *activeIndicatorClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return readPvExpStr.getRaw();
  }
  else if ( i == 1 ) {
    return nullPvExpStr.getRaw();
  }
  else if ( i == 2 ) {
    return label.getRaw();
  }

  return NULL;

}

void activeIndicatorClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    readPvExpStr.setRaw( string );
  }
  else if ( i == 1 ) {
    nullPvExpStr.setRaw( string );
  }
  else if ( i == 2 ) {
    label.setRaw( string );
  }

}

// crawler functions may return blank pv names
char *activeIndicatorClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return readPvExpStr.getExpanded();

}

char *activeIndicatorClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >= 1 ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return nullPvExpStr.getExpanded();
  }

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeIndicatorClassPtr ( void ) {

activeIndicatorClass *ptr;

  ptr = new activeIndicatorClass;
  return (void *) ptr;

}

void *clone_activeIndicatorClassPtr (
  void *_srcPtr )
{

activeIndicatorClass *ptr, *srcPtr;

  srcPtr = (activeIndicatorClass *) _srcPtr;

  ptr = new activeIndicatorClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
