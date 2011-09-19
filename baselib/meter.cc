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

#define __meter_cc 1

#include "meter.h"
#include "app_pkg.h"
#include "act_win.h"
#include <math.h>

#include "thread.h"

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeMeterClass *metero = (activeMeterClass *) client;

  if ( !metero->activeInitFlag ) {
    metero->needToDrawUnconnected = 1;
    metero->needDraw = 1;
    metero->actWin->addDefExeNode( metero->aglPtr );
  }

  metero->unconnectedTimer = 0;

}

static void meterc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMeterClass *metero = (activeMeterClass *) client;

  metero->actWin->setChanged();

  metero->eraseSelectBoxCorners();
  metero->erase();

  metero->fgColorMode = metero->eBuf->bufFgColorMode;
  if ( metero->fgColorMode == METERC_K_COLORMODE_ALARM )
    metero->fgColor.setAlarmSensitive();
  else
    metero->fgColor.setAlarmInsensitive();
  metero->fgColor.setColorIndex( metero->eBuf->bufFgColor, metero->actWin->ci );

  metero->meterColorMode = metero->eBuf->bufMeterColorMode;
  if ( metero->meterColorMode == METERC_K_COLORMODE_ALARM )
    metero->meterColor.setAlarmSensitive();
  else
    metero->meterColor.setAlarmInsensitive();
  metero->meterColor.setColorIndex( metero->eBuf->bufMeterColor, metero->actWin->ci );

  metero->scaleColorMode = metero->eBuf->bufScaleColorMode;
  if ( metero->scaleColorMode == METERC_K_COLORMODE_ALARM )
    metero->scaleColor.setAlarmSensitive();
  else
    metero->scaleColor.setAlarmInsensitive();
  metero->scaleColor.setColorIndex( metero->eBuf->bufScaleColor, metero->actWin->ci );

  metero->shadowMode = metero->eBuf->bufShadowMode;

  metero->meterAngle = metero->eBuf->bufMeterAngle;

  metero->scaleLimitsFromDb = metero->eBuf->bufScaleLimitsFromDb;

  //metero->scaleMin   = metero->eBuf->bufScaleMin;
  metero->scaleMinExpStr.setRaw( metero->eBuf->bufScaleMin );

  //metero->scaleMax   = metero->eBuf->bufScaleMax;
  metero->scaleMaxExpStr.setRaw( metero->eBuf->bufScaleMax );

  strncpy( metero->scaleFormat, metero->eBuf->bufScaleFormat, 15 );

  //metero->scalePrecision = metero->eBuf->bufScalePrecision;
  metero->scalePrecExpStr.setRaw( metero->eBuf->bufScalePrecision );

  metero->needleType = metero->eBuf->bufNeedleType;

  //metero->labelIntervals = metero->eBuf->bufLabelIntervals;
  metero->labIntExpStr.setRaw( metero->eBuf->bufLabelIntervals );

  //metero->majorIntervals = metero->eBuf->bufMajorIntervals;
  metero->majorIntExpStr.setRaw( metero->eBuf->bufMajorIntervals );

  //metero->minorIntervals = metero->eBuf->bufMinorIntervals;
  metero->minorIntExpStr.setRaw( metero->eBuf->bufMinorIntervals );

  metero->bgColor.setColorIndex( metero->eBuf->bufBgColor, metero->actWin->ci );

  metero->tsColor.setColorIndex( metero->eBuf->bufTsColor, metero->actWin->ci );
  metero->bsColor.setColorIndex( metero->eBuf->bufBsColor, metero->actWin->ci );

  metero->scaleColor.setColorIndex( metero->eBuf->bufScaleColor, metero->actWin->ci );
  metero->labelColor.setColorIndex( metero->eBuf->bufLabelColor, metero->actWin->ci );

  metero->readPvExpStr.setRaw( metero->eBuf->bufReadPvName );

  strncpy( metero->literalLabel, metero->eBuf->bufLiteralLabel,
   PV_Factory::MAX_PV_NAME );

  metero->readPvLabelExpStr.setRaw( metero->literalLabel );

  metero->labelType = metero->eBuf->bufLabelType;

  strncpy( metero->scaleFontTag, metero->scaleFm.currentFontTag(), 63 );

  metero->actWin->fi->loadFontTag( metero->scaleFontTag );
  metero->scaleFs = metero->actWin->fi->getXFontStruct( metero->scaleFontTag );

  strncpy( metero->labelFontTag, metero->labelFm.currentFontTag(), 63 );

  metero->actWin->fi->loadFontTag( metero->labelFontTag );
  metero->labelFs = metero->actWin->fi->getXFontStruct( metero->labelFontTag );

  metero->trackDelta = metero->eBuf->bufTrackDelta;

  metero->showScale = metero->eBuf->bufShowScale;

  metero->x = metero->eBuf->bufX;
  metero->sboxX = metero->eBuf->bufX;

  metero->y = metero->eBuf->bufY;
  metero->sboxY = metero->eBuf->bufY;

  metero->w = metero->eBuf->bufW;
  metero->sboxW = metero->eBuf->bufW;

  metero->h = metero->eBuf->bufH;
  metero->sboxH = metero->eBuf->bufH;

  metero->updateDimensions();

}

static void meterc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMeterClass *metero = (activeMeterClass *) client;

  meterc_edit_update ( w, client, call );
  metero->refresh( metero );

}

static void meterc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMeterClass *metero = (activeMeterClass *) client;

  meterc_edit_update ( w, client, call );
  metero->ef.popdown();
  metero->operationComplete();

}

static void meterc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMeterClass *metero = (activeMeterClass *) client;

  metero->ef.popdown();
  metero->operationCancel();

}

static void meterc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMeterClass *metero = (activeMeterClass *) client;

  metero->ef.popdown();
  metero->operationCancel();
  metero->erase();
  metero->deleteRequest = 1;
  metero->drawAll();

}

static void meter_monitor_read_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMeterClass *metero = (activeMeterClass *) userarg;

  metero->actWin->appCtx->proc->lock();

  if ( pv->is_valid() ) {

    metero->connection.setPvConnected( (void *) metero->readPvConnection );

    if ( metero->connection.pvsConnected() ) {
      metero->needConnectInit = 1;
      metero->actWin->addDefExeNode( metero->aglPtr );
    }

  }
  else {

    metero->connection.setPvDisconnected( (void *) metero->readPvConnection );
    metero->active = 0;
    metero->meterColor.setDisconnected();
    metero->fgColor.setDisconnected();
    metero->scaleColor.setDisconnected();
    metero->bufInvalidate();
    metero->needDraw = 1;
    metero->actWin->addDefExeNode( metero->aglPtr );

  }

  metero->actWin->appCtx->proc->unlock();

}

static void meter_monitor_read_label_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

activeMeterClass *metero = (activeMeterClass *) userarg;

  metero->actWin->appCtx->proc->lock();

  if ( pv->is_valid() ) {

    metero->connection.setPvConnected( (void *) metero->readPvLabelConnection );

    if ( metero->connection.pvsConnected() ) {
      metero->needConnectInit = 1;
      metero->actWin->addDefExeNode( metero->aglPtr );
    }

  }
  else {

    metero->connection.setPvDisconnected( (void *) metero->readPvLabelConnection );
    metero->bufInvalidate();
    metero->needDraw = 1;
    metero->actWin->addDefExeNode( metero->aglPtr );

  }

  metero->actWin->appCtx->proc->unlock();

}

static void meter_readUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMeterClass *metero = (activeMeterClass *) userarg;
int st, sev;

  metero->curReadV = pv->get_double() - metero->baseV;

  if ( metero->active ) {

    st = pv->get_status();
    sev = pv->get_severity();
    if ( ( st != metero->oldStat ) || ( sev != metero->oldSev ) ) {
      metero->oldStat = st;
      metero->oldSev = sev;
      metero->fgColor.setStatus( st, sev );
      metero->scaleColor.setStatus( st, sev );
      metero->meterColor.setStatus( st, sev );
      metero->bufInvalidate();
    }

    metero->curReadV = pv->get_double() - metero->baseV;
    metero->needErase = 1;
    metero->needDraw = 1;
    metero->actWin->appCtx->proc->lock();
    metero->actWin->addDefExeNode( metero->aglPtr );
    metero->actWin->appCtx->proc->unlock();

  }

}

static void meter_readLabelUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeMeterClass *metero = (activeMeterClass *) userarg;

  pv->get_string( metero->readPvLabel, PV_Factory::MAX_PV_NAME );
  metero->readPvLabel[PV_Factory::MAX_PV_NAME] = 0;

  if ( metero->active ) {
    metero->bufInvalidate();
    metero->needErase = 1;
    metero->needDraw = 1;
    metero->actWin->appCtx->proc->lock();
    metero->actWin->addDefExeNode( metero->aglPtr );
    metero->actWin->appCtx->proc->unlock();
  }

}

activeMeterClass::activeMeterClass ( void ) {

  name = new char[strlen("activeMeterClass")+1];
  strcpy( name, "activeMeterClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  minW = 100;
  minH = 50;
  strcpy( scaleFontTag, "" );
  strcpy( labelFontTag, "" );
  scaleFs = NULL;
  labelFs = NULL;
  strcpy( label, "" );
  strcpy( literalLabel, "" );
  activeMode = 0;
  meterAngle = 180.0;
  scaleMin = 0;
  scaleMax = 10;
  labelIntervals = 10;
  majorIntervals = 2;
  minorIntervals = 5;
  strcpy( scaleFormat, "FFloat" );
  scalePrecision = 0;
  needleType = 1;
  shadowMode = 1;

  meterColorMode = METERC_K_COLORMODE_STATIC;
  scaleColorMode = METERC_K_COLORMODE_STATIC;
  fgColorMode = METERC_K_COLORMODE_STATIC;
  labelType = METERC_K_PV_LABEL;
  showScale = 1;
  useDisplayBg = 1;
  trackDelta = 0;

  scaleLimitsFromDb = 1;

  readMin = scaleMin;
  readMax = scaleMax;

  eBuf = NULL;

  unconnectedTimer = 0;

  baseV = 0;

  strcpy( readPvLabel, "" );

  connection.setMaxPvs( 2 );

}

// copy constructor
activeMeterClass::activeMeterClass
 ( const activeMeterClass *source ) {

activeGraphicClass *metero = (activeGraphicClass *) this;

  metero->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMeterClass")+1];
  strcpy( name, "activeMeterClass" );

  strncpy( scaleFontTag, source->scaleFontTag, 63 );
  scaleFs = actWin->fi->getXFontStruct( scaleFontTag );

  strncpy( labelFontTag, source->labelFontTag, 63 );
  labelFs = actWin->fi->getXFontStruct( labelFontTag );

  meterColor.copy( source->meterColor );
  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );
  tsColor.copy(source->tsColor);
  bsColor.copy(source->bsColor);
  scaleColor.copy(source->scaleColor);
  labelColor.copy(source->labelColor);

  readPvExpStr.copy( source->readPvExpStr );

  strncpy( label, source->label, PV_Factory::MAX_PV_NAME );
  strncpy( literalLabel, source->literalLabel, PV_Factory::MAX_PV_NAME );

  meterColorMode = source->meterColorMode;
  scaleColorMode = source->scaleColorMode;
  fgColorMode = source->fgColorMode;
  shadowMode = source->shadowMode;
  scaleLimitsFromDb = source->scaleLimitsFromDb;
  needleType = source->needleType;
  scalePrecision = source->scalePrecision;
  scalePrecExpStr.copy( source->scalePrecExpStr );
  strncpy( scaleFormat, source->scaleFormat, 15 );
  meterAngle = source->meterAngle;
  scaleMin = source->scaleMin;
  scaleMinExpStr.copy( source->scaleMinExpStr );
  scaleMax = source->scaleMax;
  scaleMaxExpStr.copy( source->scaleMaxExpStr );
  labelType = source->labelType;
  trackDelta = source->trackDelta;
  showScale = source->showScale;
  useDisplayBg = source->useDisplayBg;
  drawStaticFlag = source->drawStaticFlag;

  labelIntervals = source->labelIntervals;
  labIntExpStr.copy( source->labIntExpStr );

  majorIntervals = source->majorIntervals;
  majorIntExpStr.copy( source->majorIntExpStr );

  minorIntervals = source->minorIntervals;
  minorIntExpStr.copy( source->minorIntExpStr );

  readMin = scaleMin;
  readMax = scaleMax;

  minW = 100;
  minH = 50;
  activeMode = 0;

  unconnectedTimer = 0;

  eBuf = NULL;

  baseV = 0;

  connection.setMaxPvs( 2 );

  strcpy( readPvLabel, "" );

  updateDimensions();

  doAccSubs( readPvExpStr );
  doAccSubs( literalLabel, PV_Factory::MAX_PV_NAME );
  readPvLabelExpStr.setRaw( literalLabel );

}

activeMeterClass::~activeMeterClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

int activeMeterClass::createInteractive (
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

  if ( w < minW ) w = minW;
  if ( h < minH ) h = minH;

  meterColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultOffsetColor, actWin->ci );
  scaleColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci);
  labelColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );

  tsColor.setColorIndex( actWin->defaultTopShadowColor, actWin->ci );
  bsColor.setColorIndex( actWin->defaultBotShadowColor, actWin->ci );

  strcpy( scaleFontTag, actWin->defaultCtlFontTag );
  actWin->fi->loadFontTag( scaleFontTag );
  scaleFs = actWin->fi->getXFontStruct( scaleFontTag );

  strcpy( labelFontTag, actWin->defaultFontTag );
  actWin->fi->loadFontTag( labelFontTag );
  labelFs = actWin->fi->getXFontStruct( labelFontTag );

  updateDimensions();

  this->draw();

  this->editCreate();

  return 1;

}

int activeMeterClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double a180 = 180;
char *emptyStr = "";

int lit = 2;
static char *labelTypeEnumStr[3] = {
  "pvName",
  "pvLabel",
  "literal"
};
static int labelTypeEnum[3] = {
  0,
  1,
  2
};

  major = METERC_MAJOR_VERSION;
  minor = METERC_MINOR_VERSION;
  release = METERC_RELEASE;

  strncpy( literalLabel, readPvLabelExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  literalLabel[PV_Factory::MAX_PV_NAME] = 0;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "caseColor", actWin->ci, &meterColor );
  tag.loadBoolW( "caseAlarm", &meterColorMode, &zero );
  tag.loadW( "scaleColor", actWin->ci, &scaleColor );
  tag.loadBoolW( "scaleAlarm", &scaleColorMode, &zero );
  tag.loadW( "labelColor", actWin->ci, &labelColor );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadBoolW( "fgAlarm", &fgColorMode, &zero );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadW( "topShadowColor", actWin->ci, &tsColor );
  tag.loadW( "botShadowColor", actWin->ci, &bsColor );
  tag.loadW( "readPv", &readPvExpStr, emptyStr );
  tag.loadW( "label", literalLabel, emptyStr );
  tag.loadW( "labelType", 3, labelTypeEnumStr, labelTypeEnum,
   &labelType, &lit );
  tag.loadBoolW( "trackDelta", &trackDelta, &zero );
  tag.loadBoolW( "showScale", &showScale, &zero );
  tag.loadW( "scaleFormat", scaleFormat );
  tag.loadW( "scalePrecision", &scalePrecExpStr );
  tag.loadBoolW( "scaleLimitsFromDb", &scaleLimitsFromDb, &zero );
  tag.loadBoolW( "useDisplayBg", &useDisplayBg, &zero );
  tag.loadW( "labelIntervals", &labIntExpStr, emptyStr );
  tag.loadW( "majorIntervals", &majorIntExpStr, emptyStr );
  tag.loadW( "minorIntervals", &minorIntExpStr, emptyStr );
  tag.loadBoolW( "complexNeedle", &needleType, &zero );
  tag.loadBoolW( "3d", &shadowMode, &zero );
  tag.loadW( "scaleMin", &scaleMinExpStr, emptyStr );
  tag.loadW( "scaleMax", &scaleMaxExpStr, emptyStr );
  tag.loadW( "labelFontTag", labelFontTag );
  tag.loadW( "scaleFontTag", scaleFontTag );
  tag.loadW( "meterAngle", &meterAngle, &a180 );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeMeterClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", METERC_MAJOR_VERSION, METERC_MINOR_VERSION,
   METERC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = meterColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", meterColorMode );

  index = scaleColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", scaleColorMode );

  index = labelColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fgColorMode );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = tsColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = bsColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  writeStringToFile( f, "" ); // use to be controlpv

  if ( readPvExpStr.getRaw() )
    writeStringToFile( f, readPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, literalLabel );
  writeStringToFile( f, label );

  fprintf( f, "%-d\n", labelType );

  fprintf( f, "%-d\n", showScale );
  writeStringToFile( f, scaleFormat );

  if ( scalePrecExpStr.getRaw() )
    writeStringToFile( f, scalePrecExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", scaleLimitsFromDb );
  fprintf( f, "%-d\n", useDisplayBg );
  fprintf( f, "%-d\n", majorIntervals );
  fprintf( f, "%-d\n", minorIntervals );
  fprintf( f, "%-d\n", needleType );
  fprintf( f, "%-d\n", shadowMode );

  if ( scaleMinExpStr.getRaw() )
    writeStringToFile( f, scaleMinExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( scaleMaxExpStr.getRaw() )
    writeStringToFile( f, scaleMaxExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, labelFontTag );

  writeStringToFile( f, scaleFontTag );

  fprintf( f, "%-g\n", meterAngle );  

  return 1;

}

int activeMeterClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
double a180 = 180;
char *emptyStr = "";

int lit = 2;
static char *labelTypeEnumStr[3] = {
  "pvName",
  "pvLabel",
  "literal"
};
static int labelTypeEnum[3] = {
  0,
  1,
  2
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
  tag.loadR( "caseColor", actWin->ci, &meterColor );
  tag.loadR( "caseAlarm", &meterColorMode, &zero );
  tag.loadR( "scaleColor", actWin->ci, &scaleColor );
  tag.loadR( "scaleAlarm", &scaleColorMode, &zero );
  tag.loadR( "labelColor", actWin->ci, &labelColor );
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "fgAlarm", &fgColorMode, &zero );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "topShadowColor", actWin->ci, &tsColor );
  tag.loadR( "botShadowColor", actWin->ci, &bsColor );
  tag.loadR( "readPv", &readPvExpStr, emptyStr );
  tag.loadR( "label", PV_Factory::MAX_PV_NAME, literalLabel, emptyStr );
  tag.loadR( "labelType", 3, labelTypeEnumStr, labelTypeEnum,
   &labelType, &lit );
  tag.loadR( "trackDelta", &trackDelta, &zero );
  tag.loadR( "showScale", &showScale, &zero );
  tag.loadR( "scaleFormat", 15, scaleFormat );
  tag.loadR( "scalePrecision", &scalePrecExpStr );
  tag.loadR( "scaleLimitsFromDb", &scaleLimitsFromDb, &zero );
  tag.loadR( "useDisplayBg", &useDisplayBg, &zero );
  tag.loadR( "labelIntervals", &labIntExpStr, emptyStr );
  tag.loadR( "majorIntervals", &majorIntExpStr, emptyStr );
  tag.loadR( "minorIntervals", &minorIntExpStr, emptyStr );
  tag.loadR( "complexNeedle", &needleType, &zero );
  tag.loadR( "3d", &shadowMode, &zero );
  tag.loadR( "scaleMin", &scaleMinExpStr, emptyStr );
  tag.loadR( "scaleMax", &scaleMaxExpStr, emptyStr );
  tag.loadR( "labelFontTag", 63, labelFontTag );
  tag.loadR( "scaleFontTag", 63, scaleFontTag );
  tag.loadR( "meterAngle", &meterAngle, &a180 );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > METERC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  if ( ( major == 4 ) && ( minor < 1 ) ) {

    if ( blank(scaleMinExpStr.getRaw()) ) {
      scaleMinExpStr.setRaw( "0" );
    }

    if ( blank(scaleMaxExpStr.getRaw()) ) {
      scaleMaxExpStr.setRaw( "0" );
    }

    if ( blank(scalePrecExpStr.getRaw()) ) {
      scalePrecExpStr.setRaw( "0" );
    }

    if ( blank(labIntExpStr.getRaw()) ) {
      labIntExpStr.setRaw( "0" );
    }

    if ( blank(majorIntExpStr.getRaw()) ) {
      majorIntExpStr.setRaw( "0" );
    }

    if ( blank(minorIntExpStr.getRaw()) ) {
      minorIntExpStr.setRaw( "0" );
    }

  }

  this->initSelectBox(); // call after getting x,y,w,h


  if ( meterColorMode == METERC_K_COLORMODE_ALARM )
    meterColor.setAlarmSensitive();
  else
    meterColor.setAlarmInsensitive();

  if ( scaleColorMode == METERC_K_COLORMODE_ALARM )
    scaleColor.setAlarmSensitive();
  else
    scaleColor.setAlarmInsensitive();

  if ( fgColorMode == METERC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  bgColor.setAlarmInsensitive();

  actWin->fi->loadFontTag( labelFontTag );
  labelFs = actWin->fi->getXFontStruct( labelFontTag );
  updateFont( labelFontTag, &labelFs, &labelFontAscent, &labelFontDescent,
   &labelFontHeight );

  actWin->fi->loadFontTag( scaleFontTag );
  scaleFs = actWin->fi->getXFontStruct( scaleFontTag );
  updateFont( scaleFontTag, &scaleFs, &scaleFontAscent, &scaleFontDescent,
   &scaleFontHeight );

  readPvLabelExpStr.setRaw( literalLabel );

  updateDimensions();

  return stat;

}

int activeMeterClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME+1], str[15+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > METERC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    meterColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &meterColorMode ); actWin->incLine();
    if ( meterColorMode == METERC_K_COLORMODE_ALARM )
      meterColor.setAlarmSensitive();
    else
      meterColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    scaleColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &scaleColorMode ); actWin->incLine();
    if ( scaleColorMode == METERC_K_COLORMODE_ALARM )
      scaleColor.setAlarmSensitive();
    else
      scaleColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    labelColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == METERC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    tsColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bsColor.setColorIndex( index, actWin->ci );

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    meterColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &meterColorMode ); actWin->incLine();
    if ( meterColorMode == METERC_K_COLORMODE_ALARM )
      meterColor.setAlarmSensitive();
    else
      meterColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    scaleColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &scaleColorMode ); actWin->incLine();
    if ( scaleColorMode == METERC_K_COLORMODE_ALARM )
      scaleColor.setAlarmSensitive();
    else
      scaleColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    labelColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == METERC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    tsColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bsColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    meterColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &meterColorMode ); actWin->incLine();
    if ( meterColorMode == METERC_K_COLORMODE_ALARM )
      meterColor.setAlarmSensitive();
    else
      meterColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    scaleColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &scaleColorMode ); actWin->incLine();
    if ( scaleColorMode == METERC_K_COLORMODE_ALARM )
      scaleColor.setAlarmSensitive();
    else
      scaleColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    labelColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == METERC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    tsColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    bsColor.setColorIndex( index, actWin->ci );

  }

  bgColor.setAlarmInsensitive();

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  //controlPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  readPvExpStr.setRaw( oneName );

  readStringFromFile( literalLabel, PV_Factory::MAX_PV_NAME+1,
   f ); actWin->incLine();

  readStringFromFile( label, PV_Factory::MAX_PV_NAME+1, f ); actWin->incLine();

  fscanf( f, "%d\n", &labelType ); actWin->incLine();

  fscanf( f, "%d\n", &showScale ); actWin->incLine();

  if ( major > 1 || minor > 1 ) {
    readStringFromFile( oneName, 39+1, f ); actWin->incLine();
    strncpy( scaleFormat, oneName, 15 );
  }

  if ( strcmp( scaleFormat, "g" ) == 0 ) {
    strcpy( scaleFormat, "GFloat" );
  }
  else if ( strcmp( scaleFormat, "f" ) == 0 ) {
    strcpy( scaleFormat, "FFloat" );
  }
  else if ( strcmp( scaleFormat, "e" ) == 0 ) {
    strcpy( scaleFormat, "Exponential" );
  }

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  scalePrecExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &scaleLimitsFromDb ); actWin->incLine();

  if ( major < 2 && minor < 2 ) { /* sense changed */
    if ( scaleLimitsFromDb )
      scaleLimitsFromDb = 0;
    else
      scaleLimitsFromDb = 1;
  }

  fscanf( f, "%d\n", &useDisplayBg ); actWin->incLine();
  fscanf( f, "%d\n", &majorIntervals ); actWin->incLine();
  fscanf( f, "%d\n", &minorIntervals ); actWin->incLine();

  labelIntervals = majorIntervals;
  majorIntervals = minorIntervals;
  minorIntervals = 1;

  snprintf( str, 15, "%-d", labelIntervals );
  labIntExpStr.setRaw( str );

  snprintf( str, 15, "%-d", majorIntervals );
  majorIntExpStr.setRaw( str );

  snprintf( str, 15, "%-d", minorIntervals );
  minorIntExpStr.setRaw( str );

  fscanf( f, "%d\n", &needleType ); actWin->incLine();
  fscanf( f, "%d\n", &shadowMode ); actWin->incLine();

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  scaleMinExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  scaleMaxExpStr.setRaw( oneName );

  readStringFromFile( labelFontTag, 63+1, f ); actWin->incLine();
  actWin->fi->loadFontTag( labelFontTag );
  labelFs = actWin->fi->getXFontStruct( labelFontTag );
  updateFont( labelFontTag, &labelFs, &labelFontAscent, &labelFontDescent, &labelFontHeight );


  readStringFromFile( scaleFontTag, 63+1, f ); actWin->incLine();
  actWin->fi->loadFontTag( scaleFontTag );
  scaleFs = actWin->fi->getXFontStruct( scaleFontTag );
  updateFont( scaleFontTag, &scaleFs, &scaleFontAscent, &scaleFontDescent, &scaleFontHeight );

  if ( major > 1 || minor > 0 ) {
    fscanf( f, "%lg\n", &meterAngle ); actWin->incLine();
  }

  updateDimensions();

  return 1;

}

int activeMeterClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeMeterClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMeterClass_str2, 31 );

  Strncat( title, activeMeterClass_str3, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufMeterColor = meterColor.pixelIndex();
  eBuf->bufMeterColorMode = meterColorMode;
  eBuf->bufScaleColorMode = scaleColorMode;

  //eBuf->bufLabelIntervals = labelIntervals;
  if ( labIntExpStr.getRaw() )
    strncpy( eBuf->bufLabelIntervals, labIntExpStr.getRaw(),
     15 );
  else
    strcpy( eBuf->bufLabelIntervals, "" );

  //eBuf->bufMajorIntervals = majorIntervals;
  if ( majorIntExpStr.getRaw() )
    strncpy( eBuf->bufMajorIntervals, majorIntExpStr.getRaw(),
     15 );
  else
    strcpy( eBuf->bufMajorIntervals, "" );

  //eBuf->bufMinorIntervals = minorIntervals;
  if ( minorIntExpStr.getRaw() )
    strncpy( eBuf->bufMinorIntervals, minorIntExpStr.getRaw(),
     15 );
  else
    strcpy( eBuf->bufMinorIntervals, "" );

  eBuf->bufFgColor = fgColor.pixelIndex();
  eBuf->bufFgColorMode = fgColorMode;
  
  eBuf->bufShadowMode = shadowMode;
  eBuf->bufTsColor = tsColor.pixelIndex();
  eBuf->bufBsColor = bsColor.pixelIndex();

  eBuf->bufScaleColor = scaleColor.pixelIndex();
  eBuf->bufLabelColor = labelColor.pixelIndex();

  eBuf->bufBgColor = bgColor.pixelIndex();

  if ( readPvExpStr.getRaw() )
    strncpy( eBuf->bufReadPvName, readPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufReadPvName, "" );

  strncpy( eBuf->bufLiteralLabel, literalLabel, PV_Factory::MAX_PV_NAME );

  eBuf->bufLabelType = labelType;
  eBuf->bufScaleLimitsFromDb = scaleLimitsFromDb;
  eBuf->bufMeterAngle = meterAngle;
  eBuf->bufNeedleType = needleType;

  //eBuf->bufScalePrecision = scalePrecision;
  if ( scalePrecExpStr.getRaw() )
    strncpy( eBuf->bufScalePrecision, scalePrecExpStr.getRaw(),
     15 );
  else
    strcpy( eBuf->bufScalePrecision, "" );

  //eBuf->bufScaleMin = scaleMin;
  if ( scaleMinExpStr.getRaw() )
    strncpy( eBuf->bufScaleMin, scaleMinExpStr.getRaw(),
     15 );
  else
    strcpy( eBuf->bufScaleMin, "" );

  //eBuf->bufScaleMax = scaleMax;
  if ( scaleMaxExpStr.getRaw() )
    strncpy( eBuf->bufScaleMax, scaleMaxExpStr.getRaw(),
     15 );
  else
    strcpy( eBuf->bufScaleMax, "" );

  eBuf->bufTrackDelta = trackDelta;
  eBuf->bufShowScale = showScale;
  eBuf->bufUseDisplayBg = useDisplayBg;
  strncpy( eBuf->bufScaleFormat, scaleFormat, 15 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeMeterClass_str4, 35, &eBuf->bufX );
  ef.addTextField( activeMeterClass_str5, 35, &eBuf->bufY );
  ef.addTextField( activeMeterClass_str6, 35, &eBuf->bufW );
  ef.addTextField( activeMeterClass_str7, 35, &eBuf->bufH );
  ef.addTextField( activeMeterClass_str9, 35, eBuf->bufReadPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addOption( activeMeterClass_str10, activeMeterClass_str11, &eBuf->bufLabelType );
  labelTypeEntry = ef.getCurItem();
  labelTypeEntry->setNumValues( 3 );
  ef.addTextField( activeMeterClass_str12, 35, eBuf->bufLiteralLabel,
   PV_Factory::MAX_PV_NAME );
  labelEntry = ef.getCurItem();
  labelTypeEntry->addInvDependency( 0, labelEntry );
  labelTypeEntry->addDependencyCallbacks();

  ef.addColorButton( activeMeterClass_str14, actWin->ci,&eBuf->labelCb,&eBuf->bufLabelColor);
  ef.addTextField(activeMeterClass_str15, 35, &eBuf->bufMeterAngle);
  ef.addToggle( activeMeterClass_str45, &eBuf->bufTrackDelta );

  ef.addToggle( activeMeterClass_str16, &eBuf->bufShowScale );
  showScaleEntry = ef.getCurItem();
  ef.addOption( activeMeterClass_str17, activeMeterClass_str43,
   eBuf->bufScaleFormat, 15 );
  scaleFormatEntry = ef.getCurItem();
  showScaleEntry->addDependency( scaleFormatEntry );
  ef.addTextField(activeMeterClass_str19, 35, eBuf->bufScalePrecision, 15 );
  scalePrecEntry = ef.getCurItem();
  showScaleEntry->addDependency( scalePrecEntry );

  ef.addToggle( activeMeterClass_str20, &eBuf->bufScaleLimitsFromDb );
  scaleLimFromDbEntry = ef.getCurItem();
  ef.addTextField(activeMeterClass_str21, 35, eBuf->bufScaleMin, 15 );
  scaleMinEntry = ef.getCurItem();
  scaleLimFromDbEntry->addInvDependency( scaleMinEntry );
  ef.addTextField(activeMeterClass_str22, 35, eBuf->bufScaleMax, 15 );
  scaleMaxEntry = ef.getCurItem();
  scaleLimFromDbEntry->addInvDependency( scaleMaxEntry );
  scaleLimFromDbEntry->addDependencyCallbacks();

  ef.addColorButton( activeMeterClass_str24, actWin->ci,&eBuf->scaleCb,&eBuf->bufScaleColor);
  scaleColorEntry = ef.getCurItem();
  showScaleEntry->addDependency( scaleColorEntry );
  ef.addToggle( activeMeterClass_str25, &eBuf->bufScaleColorMode );
  scaleColorModeEntry = ef.getCurItem();
  showScaleEntry->addDependency( scaleColorModeEntry );
  ef.addTextField(activeMeterClass_str44, 35, eBuf->bufLabelIntervals, 15 );
  labelIntEntry = ef.getCurItem();
  showScaleEntry->addDependency( labelIntEntry );
  ef.addTextField(activeMeterClass_str26, 35, eBuf->bufMajorIntervals, 15 );
  majorIntEntry = ef.getCurItem();
  showScaleEntry->addDependency( majorIntEntry );
  ef.addTextField(activeMeterClass_str27, 35, eBuf->bufMinorIntervals, 15 );
  minorIntEntry = ef.getCurItem();
  showScaleEntry->addDependency( minorIntEntry );
  showScaleEntry->addDependencyCallbacks();

  ef.addToggle(activeMeterClass_str28, &eBuf->bufNeedleType);  
  ef.addColorButton( activeMeterClass_str29, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addToggle( activeMeterClass_str30, &eBuf->bufFgColorMode );
  ef.addColorButton( activeMeterClass_str31, actWin->ci, &eBuf->meterCb, &eBuf->bufMeterColor );
  ef.addToggle( activeMeterClass_str32, &eBuf->bufMeterColorMode );
  ef.addColorButton( activeMeterClass_str33, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  ef.addToggle(activeMeterClass_str34,&eBuf->bufShadowMode);
  ef.addColorButton(activeMeterClass_str35,actWin->ci, &eBuf->tsCb,&eBuf->bufTsColor);
  ef.addColorButton(activeMeterClass_str36,actWin->ci,&eBuf->bsCb,&eBuf->bufBsColor);

  ef.addFontMenu( activeMeterClass_str13, actWin->fi, &labelFm, labelFontTag );
  ef.addFontMenu( activeMeterClass_str23, actWin->fi, &scaleFm, scaleFontTag );

  XtUnmanageChild( scaleFm.alignWidget() ); // no alignment info
  XtUnmanageChild( labelFm.alignWidget() ); // no alignment info

  return 1;

}

int activeMeterClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( meterc_edit_ok, meterc_edit_apply, meterc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMeterClass::edit ( void ) {

  this->genericEdit();
  ef.finished( meterc_edit_ok, meterc_edit_apply, meterc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMeterClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMeterClass::eraseActive ( void ) {

  if ( !enabled || !activeMode) return 1;

  if ( bufInvalid) { 

    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setLineWidth( 1 );

    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
		    actWin->executeGc.eraseGC(), x, y, w, h );

    XFillRectangle( actWin->d, drawable(actWin->executeWidget),
		    actWin->executeGc.eraseGC(), x, y, w, h );
  }

  return 1;

}

int activeMeterClass::draw ( void ) {

  int i, ii, tX, tY;
  char scaleMinString[39+1],scaleMaxString[39+1],scaleString[39+1];
  double labelAngle, labelAngleIncr, majorAngleIncr, minorAngleIncr;
  double needleLength, insideArc;
  double labelTickSize = 10, majorTickSize = 5, minorTickSize = 7;
  double needlePlusScale, scaleTextIncr, scaleValue;
  double meterTotalAngle, beginAngle, endAngle;
  double descentAngle;
  double verticalExtent, visibleFraction;
  double biggestHorizNeedlePlusScale,biggestVertNeedlePlusScale;
  int caseWidth = 5;
  int scaleMinWidth,scaleMaxWidth,scaleFontWidth;
  int faceX,faceY,faceW,faceH;
  double tickSize;
  int  farEndX, farEndY, nearEndX, nearEndY;
  char fmt[31+1];
  XRectangle xR;

  if (meterAngle < 10)  meterAngle = 10;
  if (meterAngle > 360) meterAngle = 360;

  meterTotalAngle = 3.1415926535898 * meterAngle / 180.0;
  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

 if (scaleLimitsFromDb){
   scaleMin = readMin;
   scaleMax = readMax;
 }

 faceX = x + caseWidth;
 faceY = y + caseWidth;
 faceW = w - 2 * caseWidth;
 faceH = h - 2 * caseWidth;

  if ( labelType == 0 ) {
    strncpy( label, readPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
    label[PV_Factory::MAX_PV_NAME] = 0;
  }
  else if ( labelType == 1 ) {
    if ( strlen(literalLabel) ) {
      strncpy ( label, literalLabel, PV_Factory::MAX_PV_NAME );
      label[PV_Factory::MAX_PV_NAME] = 0;
    }
    else {
      strcpy (label,"");
    }
  }
  else if ( labelType == 2 ) {
    if ( strlen(literalLabel) ) {
      strncpy ( label, literalLabel, PV_Factory::MAX_PV_NAME );
      label[PV_Factory::MAX_PV_NAME] = 0;
    }
    else {
      strcpy (label,"");
    }
  }
  else {
    strcpy (label,"");
  }
  faceH = faceH + caseWidth - 4 - labelFontHeight;

 if(scalePrecision > 10 || scalePrecision < 0) scalePrecision = 1;

 if ( strcmp( scaleFormat, "GFloat" ) == 0 ) {
   sprintf( fmt, "%%.%-dg", scalePrecision );
 }
 else if ( strcmp( scaleFormat, "Exponential" ) == 0 ) {
   sprintf( fmt, "%%.%-de", scalePrecision );
 }
 else {
   sprintf( fmt, "%%.%-df", scalePrecision );
 }

 sprintf (scaleMinString, fmt, scaleMin);
 sprintf (scaleMaxString, fmt, scaleMax);

 if ( scaleFs ) { 
   scaleMinWidth = XTextWidth(scaleFs,scaleMinString,strlen(scaleMinString));
   scaleMaxWidth = XTextWidth(scaleFs,scaleMaxString,strlen(scaleMaxString));
 }
 else {
   scaleMinWidth = 10;
   scaleMaxWidth = 10;
 }
 
 if (scaleMinWidth >= scaleMaxWidth)
   scaleFontWidth =  scaleMinWidth;
 else
   scaleFontWidth = scaleMaxWidth;

 if (labelIntervals < 1) labelIntervals = 1;
 scaleTextIncr = (scaleMax - scaleMin) / labelIntervals;

 scaleValue = scaleMax - scaleTextIncr;
 sprintf (scaleString, fmt, scaleValue);

 if ( scaleFs ) {
   if ( XTextWidth(scaleFs,scaleString,strlen(scaleString)) > scaleFontWidth)
     scaleFontWidth = XTextWidth(scaleFs,scaleString,strlen(scaleString));
 }

 scaleValue = scaleMin + scaleTextIncr;
 sprintf (scaleString, fmt, scaleValue);

 if ( scaleFs ) {
   if ( XTextWidth(scaleFs,scaleString,strlen(scaleString)) > scaleFontWidth)
     scaleFontWidth = XTextWidth(scaleFs,scaleString,strlen(scaleString));
 }

 descentAngle = 3.1415926535898/2 - meterTotalAngle/2;

 biggestHorizNeedlePlusScale = 0.5 * faceW -4 -scaleFontWidth;
 if (descentAngle > 0)
   biggestHorizNeedlePlusScale /= cos(descentAngle);
 
 if (descentAngle > 0){
   //   visibleFraction = 1.1 * (1 - .45 * descentAngle);
   visibleFraction = 1.1 * (1 - .60 * descentAngle);
   biggestVertNeedlePlusScale = (faceH - scaleFontHeight -4) /
     visibleFraction;
 }
 else{
   visibleFraction = 1-sin(descentAngle);
   biggestVertNeedlePlusScale = (faceH - scaleFontHeight -12) /
     visibleFraction;
 }
 
 if (biggestVertNeedlePlusScale <biggestHorizNeedlePlusScale)
   needlePlusScale = biggestVertNeedlePlusScale;
 else{
   needlePlusScale = biggestHorizNeedlePlusScale;
   verticalExtent = visibleFraction * needlePlusScale +
     scaleFontHeight + 12;
   if ((1.1 * verticalExtent) < faceH)
     faceH = (int) (verticalExtent);
 }
	
 // needleLength = 0.83 * needlePlusScale;
 // insideArc    = 0.85 * needlePlusScale;
 // // labelTickSize= 0.15 * needlePlusScale;

 labelTickSize = scaleFontHeight * 0.8;
 if ( labelTickSize > 15 ) labelTickSize = 15;
 insideArc     = needlePlusScale - labelTickSize;
 needleLength = 0.98 * insideArc;

 majorTickSize= 0.7 * labelTickSize;
 minorTickSize  = 0.4 * labelTickSize;

 meterNeedleXorigin = faceW / 2;
 meterNeedleYorigin = (int) ( needlePlusScale + 4 + scaleFontHeight);

 meterNeedleXorigin += faceX;
 meterNeedleYorigin += faceY;
 meterNeedleXend    += faceX;
 meterNeedleYend    += faceY;    

   actWin->drawGc.setFG( meterColor.pixelColor() );
   
   XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
		   actWin->drawGc.normGC(), x, y, w, h );

   
   actWin->drawGc.setFG( bgColor.pixelColor() );
   
   XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
		   actWin->drawGc.normGC(),
		   faceX, faceY, faceW, faceH );

   actWin->drawGc.setFG( labelColor.pixelColor() );

   if ( strcmp( labelFontTag, "" ) != 0 ) {
     actWin->drawGc.setFontTag( labelFontTag, actWin->fi );
   }

   if (strlen (label)){
     xR.x = faceX;
     xR.y = faceY + faceH + 2;
     xR.width  = faceW;
     xR.height = h - faceH;
     actWin->drawGc.addNormXClipRectangle( xR );
     drawText (actWin->drawWidget, &actWin->drawGc,
	       labelFs, x+w/2, faceY + faceH + 2,
	       XmALIGNMENT_CENTER, label);
     actWin->drawGc.removeNormXClipRectangle();
   }

   if (shadowMode) {

     actWin->drawGc.setFG( tsColor.pixelColor() );

     XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
		   actWin->drawGc.normGC(), x, y, w, h );

     XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
		   actWin->drawGc.normGC(),
		     faceX, faceY, faceW, faceH );

    actWin->drawGc.setFG( bsColor.pixelColor() );

    XDrawLine (actWin->d, XtWindow(actWin->drawWidget),
	       actWin->drawGc.normGC(),
	       x,y+h,
	       x+w,y+h);

    XDrawLine (actWin->d, XtWindow(actWin->drawWidget),
	       actWin->drawGc.normGC(),
	       x+w, y,
	       x+w,y+h);

    XDrawLine (actWin->d, XtWindow(actWin->drawWidget),
	       actWin->drawGc.normGC(),
	       faceX, faceY,
	       faceX, faceY + faceH);

    XDrawLine (actWin->d, XtWindow(actWin->drawWidget),
	       actWin->drawGc.normGC(),
	       faceX, faceY,
	       faceX + faceW, faceY );

   }
 
   if ((labelIntervals != 0) && (labelIntervals !=0 && showScale)){

   labelAngleIncr = meterTotalAngle / labelIntervals;
   majorAngleIncr = labelAngleIncr / majorIntervals;
   minorAngleIncr = majorAngleIncr / minorIntervals;
   beginAngle = descentAngle;
   endAngle   = meterTotalAngle + beginAngle;

   for (labelAngle = beginAngle, scaleValue = scaleMax;
	labelAngle <= (1.001 * endAngle);
	labelAngle += labelAngleIncr, scaleValue -= scaleTextIncr){
     
     farEndX = (int) (meterNeedleXorigin + (insideArc + labelTickSize) *
      cos(labelAngle));
     farEndY = (int) (meterNeedleYorigin - (insideArc + labelTickSize) *
      sin(labelAngle));
     nearEndX = (int) (meterNeedleXorigin + insideArc * cos(labelAngle));
     nearEndY = (int) (meterNeedleYorigin - insideArc * sin(labelAngle));
     
     actWin->drawGc.setFG( scaleColor.pixelColor() );
     XDrawLine (actWin->d, XtWindow(actWin->drawWidget),
		actWin->drawGc.normGC(),
		farEndX,farEndY,
		nearEndX,nearEndY);
     
     if ( strcmp( scaleFontTag, "" ) != 0 ) {
       actWin->drawGc.setFontTag( scaleFontTag, actWin->fi );
     }
     
     updateDimensions();
     
     if (labelAngle < (beginAngle + 0.001)){
       tX = farEndX + 2;
       tY = farEndY - scaleFontAscent/2;
       drawText (actWin->drawWidget, &actWin->drawGc, scaleFs, tX, tY,
		 XmALIGNMENT_BEGINNING, scaleMaxString);
     }
     else if (labelAngle <= 1.50){
       sprintf (scaleString, fmt, scaleValue);
       tX = farEndX + 2;
       tY = farEndY - scaleFontAscent;
       drawText (actWin->drawWidget, &actWin->drawGc, scaleFs, tX, tY,
		 XmALIGNMENT_BEGINNING, scaleString);
     }
     else if (labelAngle <= 1.65){
       sprintf (scaleString, fmt, scaleValue);
       tX = farEndX;
       tY = farEndY - scaleFontAscent - 2;
       drawText (actWin->drawWidget, &actWin->drawGc, scaleFs, tX, tY,
		 XmALIGNMENT_CENTER, scaleString);
     }
     else if (labelAngle <= (endAngle - 0.001)){
       sprintf (scaleString, fmt, scaleValue);
       tX = farEndX - 2;
       tY = farEndY - scaleFontAscent;
       drawText (actWin->drawWidget, &actWin->drawGc, scaleFs, tX, tY,
		 XmALIGNMENT_END, scaleString);
     }
     else if (labelAngle > (endAngle - 0.001)){
       tX = farEndX -2;
       tY = farEndY - scaleFontAscent/2;
       drawText (actWin->drawWidget, &actWin->drawGc, scaleFs, tX, tY,
		 XmALIGNMENT_END, scaleMinString);
     }       

   }

   for (labelAngle = beginAngle;
	labelAngle <(0.99 *endAngle);
	labelAngle += labelAngleIncr){
     
     for (i=0; i<majorIntervals; i++){

       for (ii=1; ii<minorIntervals; ii++){

         tickSize = minorTickSize;
       
         farEndX = (int) (meterNeedleXorigin +
           (insideArc + tickSize) * cos(ii * minorAngleIncr + labelAngle +
           i * majorAngleIncr));
         farEndY = (int) (meterNeedleYorigin -
           (insideArc + tickSize) * sin(ii * minorAngleIncr + labelAngle +
           i * majorAngleIncr));
         nearEndX = (int) (meterNeedleXorigin +
          insideArc * cos(ii * minorAngleIncr + labelAngle +
          i * majorAngleIncr));
         nearEndY = (int) (meterNeedleYorigin -
          insideArc * sin(ii * minorAngleIncr + labelAngle +
          i * majorAngleIncr));
         XDrawLine (actWin->d, XtWindow(actWin->drawWidget),
          actWin->drawGc.normGC(), farEndX, farEndY, nearEndX, nearEndY );

       }

       if ( i > 0 ) {

         tickSize = majorTickSize;
       
         farEndX = (int) (meterNeedleXorigin +
  	 (insideArc + tickSize) * cos(i * majorAngleIncr + labelAngle));
         farEndY = (int) (meterNeedleYorigin -
  	 (insideArc + tickSize) * sin(i * majorAngleIncr + labelAngle));
         nearEndX = (int) (meterNeedleXorigin +
          insideArc * cos(i * majorAngleIncr + labelAngle));
         nearEndY = (int) (meterNeedleYorigin -
          insideArc * sin(i * majorAngleIncr + labelAngle));
         XDrawLine (actWin->d, XtWindow(actWin->drawWidget),
          actWin->drawGc.normGC(), farEndX, farEndY, nearEndX, nearEndY );

       }

     }

   }

 }

 // }

 actWin->drawGc.restoreFg();
 
 return 1;
 
}

int activeMeterClass::drawActive ( void ) {

 int i, ii, tX, tY;
 char scaleMinString[39+1],scaleMaxString[39+1],scaleString[39+1];
 double labelAngle, labelAngleIncr, majorAngleIncr, minorAngleIncr;
 double meterTotalAngle, beginAngle, endAngle;
 double descentAngle;
 double verticalExtent, visibleFraction;
 double biggestVertNeedlePlusScale, biggestHorizNeedlePlusScale;
 double needleAngle;
 double needleLength, insideArc;
 double labelTickSize = 10, majorTickSize = 5, minorTickSize = 7;
 double needlePlusScale, scaleTextIncr, scaleValue;
 int caseWidth = 5;
 int scaleMinWidth,scaleMaxWidth,scaleFontWidth;
 int faceX,faceY,faceW,faceH;
 XRectangle xR;
 double tickSize;
 int  farEndX, farEndY, nearEndX, nearEndY;
 char fmt[31+1];

XPoint xpoints[6];

  if ( !activeInitFlag && !connection.pvsConnected() ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( meterColor.getDisconnected() );
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

 if ( !enabled || !activeMode || !activeInitFlag) return 1;

 meterTotalAngle = 3.1415926535898 * meterAngle / 180.0;

 if (scaleLimitsFromDb){
   scaleMin = readMin;
   scaleMax = readMax;
 }

 faceX = x + caseWidth;
 faceY = y + caseWidth;
 faceW = w - 2 * caseWidth;
 faceH = h - 2 * caseWidth;

  if ( labelType == 0 ) {
    strncpy( label, readPvExpStr.getExpanded(), PV_Factory::MAX_PV_NAME );
    label[PV_Factory::MAX_PV_NAME] = 0;
  }
  else if ( labelType == 1 ) {
    strncpy( label, readPvLabel, PV_Factory::MAX_PV_NAME );
    label[PV_Factory::MAX_PV_NAME] = 0;
  }
  else if ( labelType == 2 ) {
    if ( strlen(literalLabel) ) {
      strncpy( label, literalLabel, PV_Factory::MAX_PV_NAME );
      label[PV_Factory::MAX_PV_NAME] = 0;
    }
    else {
      strcpy (label,"");
    }
  }
  else {
    strcpy (label,"");
  }
  faceH = faceH + caseWidth - 4 - labelFontHeight;

 actWin->executeGc.saveFg();

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

 if(scalePrecision > 10 || scalePrecision < 0) scalePrecision = 1;

 if ( strcmp( scaleFormat, "GFloat" ) == 0 ) {
   sprintf( fmt, "%%.%-dg", scalePrecision );
 }
 else if ( strcmp( scaleFormat, "Exponential" ) == 0 ) {
   sprintf( fmt, "%%.%-de", scalePrecision );
 }
 else {
   sprintf( fmt, "%%.%-df", scalePrecision );
 }

 sprintf (scaleMinString,fmt,scaleMin);
 sprintf (scaleMaxString,fmt,scaleMax);

 if ( scaleFs ) {  
 scaleMinWidth = XTextWidth(scaleFs,scaleMinString,strlen(scaleMinString));
 scaleMaxWidth = XTextWidth(scaleFs,scaleMaxString,strlen(scaleMaxString));
 }
 else {
   scaleMinWidth = 10;
   scaleMaxWidth = 10;
 }

 if (scaleMinWidth >= scaleMaxWidth)
   scaleFontWidth =  scaleMinWidth;
 else
   scaleFontWidth = scaleMaxWidth;

 if (labelIntervals < 1) labelIntervals = 1;
 scaleTextIncr = (scaleMax - scaleMin) / labelIntervals;

 scaleValue = scaleMax - scaleTextIncr;
 sprintf (scaleString, fmt, scaleValue);

 if ( XTextWidth(scaleFs,scaleString,strlen(scaleString)) > scaleFontWidth)
   scaleFontWidth = XTextWidth(scaleFs,scaleString,strlen(scaleString));
						    
 scaleValue = scaleMin + scaleTextIncr;
 sprintf (scaleString, fmt, scaleValue);

 if ( XTextWidth(scaleFs,scaleString,strlen(scaleString)) > scaleFontWidth)
   scaleFontWidth = XTextWidth(scaleFs,scaleString,strlen(scaleString));

 descentAngle = 3.1415926535898/2 - meterTotalAngle/2;

 biggestHorizNeedlePlusScale = 0.5 * faceW -4 -scaleFontWidth;
 if (descentAngle > 0)
   biggestHorizNeedlePlusScale /= cos(descentAngle);
 
 if (descentAngle > 0){
   //   visibleFraction = 1.1 * (1 - .45 * descentAngle);
   visibleFraction = 1.1 * (1 - .60 * descentAngle);
   biggestVertNeedlePlusScale = (faceH - scaleFontHeight -4) /
     visibleFraction;
 }
 else{
   visibleFraction = 1-sin(descentAngle);
   biggestVertNeedlePlusScale = (faceH - scaleFontHeight -12) /
     visibleFraction;
 }
 
 if (biggestVertNeedlePlusScale <biggestHorizNeedlePlusScale)
   needlePlusScale = biggestVertNeedlePlusScale;
 else{
   needlePlusScale = biggestHorizNeedlePlusScale;
   verticalExtent = visibleFraction * needlePlusScale +
     scaleFontHeight + 12;
   if ((1.1 * verticalExtent) < faceH)
     faceH = (int) (verticalExtent);
 }

 meterNeedleXorigin = faceW / 2;
 meterNeedleYorigin = (int) ( needlePlusScale + 4 + scaleFontHeight);

 // needleLength = 0.83 * needlePlusScale;
 // insideArc    = 0.85 * needlePlusScale;
 // labelTickSize= 0.15 * needlePlusScale;

 labelTickSize = scaleFontHeight * 0.8;
 if ( labelTickSize > 15 ) labelTickSize = 15;
 insideArc     = needlePlusScale - labelTickSize;
 needleLength = 0.98 * insideArc;

 majorTickSize= 0.7 * labelTickSize;
 minorTickSize  = 0.4 * labelTickSize;

 // crude attempt to prevent divide by zero in angle calc
 if ((scaleMax - scaleMin) < 0.000001 * (readV - scaleMin)){
   scaleMax = scaleMin +1.0;
   // fprintf(stderr,"check meter scale min and max\n");
 }

 beginAngle = descentAngle;
 endAngle   = meterTotalAngle + beginAngle;

 needleAngle = beginAngle +
   meterTotalAngle * (1.0 -((readV - scaleMin)/(scaleMax - scaleMin)));

 if ( needleAngle < beginAngle)  needleAngle = beginAngle;

 if ( needleAngle > endAngle )  needleAngle = endAngle;

 meterNeedleXend = (int) (meterNeedleXorigin + needleLength * cos(needleAngle));
 meterNeedleYend = (int) (meterNeedleYorigin - needleLength * sin(needleAngle));
 
 meterNeedleXorigin += faceX;
 meterNeedleYorigin += faceY;
 meterNeedleXend    += faceX;
 meterNeedleYend    += faceY;    
 
if (drawStaticFlag){

   actWin->executeGc.setFG( meterColor.getColor() );
   
   XFillRectangle( actWin->d, drawable(actWin->executeWidget),
		   actWin->executeGc.normGC(), x, y, w, h );

   
   actWin->executeGc.setFG( bgColor.getColor() );
   
   XFillRectangle( actWin->d, drawable(actWin->executeWidget),
		   actWin->executeGc.normGC(),
		   faceX, faceY, faceW, faceH );

   actWin->executeGc.setFG( labelColor.getColor() );

   if ( strcmp( labelFontTag, "" ) != 0 ) {
     actWin->executeGc.setFontTag( labelFontTag, actWin->fi );
   }

   if (strlen (label)){
     xR.x = faceX;
     xR.y = faceY + faceH + 2;
     xR.width  = faceW;
     xR.height = h - faceH;
     actWin->executeGc.addNormXClipRectangle( xR );
     drawText (actWin->executeWidget, drawable(actWin->executeWidget),
      &actWin->executeGc, labelFs, x+w/2, faceY + faceH + 2,
      XmALIGNMENT_CENTER, label);
     actWin->executeGc.removeNormXClipRectangle();
   }

   if (shadowMode) {

     actWin->executeGc.setFG( tsColor.getColor() );

     XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
		   actWin->executeGc.normGC(), x, y, w, h );

     XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
		   actWin->executeGc.normGC(),
		     faceX, faceY, faceW, faceH );

    actWin->executeGc.setFG( bsColor.getColor() );

    XDrawLine (actWin->d, drawable(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       x,y+h,
	       x+w,y+h);

    XDrawLine (actWin->d, drawable(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       x+w, y,
	       x+w,y+h);

    XDrawLine (actWin->d, drawable(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       faceX, faceY,
	       faceX, faceY + faceH);

    XDrawLine (actWin->d, drawable(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       faceX, faceY,
	       faceX + faceW, faceY );

   }   
   
 }
 
 
 if (drawStaticFlag && showScale){

   drawStaticFlag = 0;

   
   if ((labelIntervals != 0) && (labelIntervals !=0 && showScale)){
     
     xR.x = faceX;
     xR.y = faceY;
     xR.width  = faceW;
     xR.height = faceH;

     actWin->executeGc.addNormXClipRectangle( xR );

     labelAngleIncr = meterTotalAngle / labelIntervals;
     majorAngleIncr = labelAngleIncr / majorIntervals;
     minorAngleIncr = majorAngleIncr / minorIntervals;
     beginAngle = descentAngle;
     endAngle   = meterTotalAngle + beginAngle;

     for (labelAngle = beginAngle, scaleValue = scaleMax;
	  labelAngle <= (1.001 * endAngle);
	  labelAngle += labelAngleIncr, scaleValue -= scaleTextIncr){
       
       farEndX = (int) (meterNeedleXorigin + (insideArc + labelTickSize) * cos(labelAngle));
       farEndY = (int) (meterNeedleYorigin - (insideArc + labelTickSize) * sin(labelAngle));
       nearEndX = (int) (meterNeedleXorigin + insideArc * cos(labelAngle));
       nearEndY = (int) (meterNeedleYorigin - insideArc * sin(labelAngle));
       
       actWin->executeGc.setFG( scaleColor.getColor() );
       XDrawLine (actWin->d, drawable(actWin->executeWidget),
		  actWin->executeGc.normGC(),
		  farEndX,farEndY,
		  nearEndX,nearEndY);

       if ( strcmp( scaleFontTag, "" ) != 0 ) {
         actWin->executeGc.setFontTag( scaleFontTag, actWin->fi );
       }

       updateDimensions();

       if (labelAngle < (beginAngle + 0.001)){
	 tX = farEndX + 2;
	 tY = farEndY - scaleFontAscent/2;
	 drawText (actWin->executeWidget, drawable(actWin->executeWidget),
          &actWin->executeGc, scaleFs, tX, tY,
          XmALIGNMENT_BEGINNING, scaleMaxString);
       }
       else if (labelAngle <= 1.50){
	 sprintf (scaleString, fmt, scaleValue);
	 tX = farEndX + 2;
	 tY = farEndY - scaleFontAscent;
	 drawText (actWin->executeWidget, drawable(actWin->executeWidget),
          &actWin->executeGc, scaleFs, tX, tY,
          XmALIGNMENT_BEGINNING, scaleString);
       }
       else if (labelAngle <= 1.65){
	 sprintf (scaleString, fmt, scaleValue);
	 tX = farEndX;
	 tY = farEndY - scaleFontAscent - 2;
	 drawText (actWin->executeWidget, drawable(actWin->executeWidget),
          &actWin->executeGc, scaleFs, tX, tY,
          XmALIGNMENT_CENTER, scaleString);
       }
       else if (labelAngle <= (endAngle - 0.001)){
	 sprintf (scaleString, fmt, scaleValue);
	 tX = farEndX - 2;
	 tY = farEndY - scaleFontAscent;
	 drawText (actWin->executeWidget, drawable(actWin->executeWidget),
          &actWin->executeGc, scaleFs, tX, tY,
          XmALIGNMENT_END, scaleString);
       }
       else if (labelAngle > (endAngle - 0.001)){
	 tX = farEndX -2;
	 tY = farEndY - scaleFontAscent/2;
	 drawText (actWin->executeWidget, drawable(actWin->executeWidget),
          &actWin->executeGc, scaleFs, tX, tY,
          XmALIGNMENT_END, scaleMinString);
       }       

     }

     for (labelAngle = beginAngle;
	  labelAngle <(0.99 *endAngle);
	  labelAngle += labelAngleIncr){
     
       for (i=0; i<majorIntervals; i++){

         for (ii=1; ii<minorIntervals; ii++){

           tickSize = minorTickSize;
       
           farEndX = (int) (meterNeedleXorigin +
             (insideArc + tickSize) * cos(ii * minorAngleIncr + labelAngle +
             i * majorAngleIncr));
           farEndY = (int) (meterNeedleYorigin -
             (insideArc + tickSize) * sin(ii * minorAngleIncr + labelAngle +
             i * majorAngleIncr));
           nearEndX = (int) (meterNeedleXorigin +
            insideArc * cos(ii * minorAngleIncr + labelAngle +
            i * majorAngleIncr));
           nearEndY = (int) (meterNeedleYorigin -
            insideArc * sin(ii * minorAngleIncr + labelAngle +
            i * majorAngleIncr));
           XDrawLine (actWin->d, drawable(actWin->executeWidget),
            actWin->executeGc.normGC(), farEndX, farEndY, nearEndX, nearEndY );

         }

         if ( i > 0 ) {

           tickSize = majorTickSize;
       
           farEndX = (int) (meterNeedleXorigin +
  	   (insideArc + tickSize) * cos(i * majorAngleIncr + labelAngle));
           farEndY = (int) (meterNeedleYorigin -
  	   (insideArc + tickSize) * sin(i * majorAngleIncr + labelAngle));
           nearEndX = (int) (meterNeedleXorigin +
            insideArc * cos(i * majorAngleIncr + labelAngle));
           nearEndY = (int) (meterNeedleYorigin -
            insideArc * sin(i * majorAngleIncr + labelAngle));
           XDrawLine (actWin->d, drawable(actWin->executeWidget),
            actWin->executeGc.normGC(), farEndX, farEndY, nearEndX, nearEndY );

         }

       }

     }

     actWin->executeGc.removeNormXClipRectangle();

   }
 }

 if (oldMeterNeedleXEnd != meterNeedleXend || 
     oldMeterNeedleYEnd != meterNeedleYend ){

   xR.x = faceX;
   xR.y = faceY;
   xR.width  = faceW;
   xR.height = faceH;

   actWin->executeGc.addNormXClipRectangle( xR );

   /* erase old needle */

   actWin->executeGc.setFG( bgColor.pixelColor() );

   if (needleType == 0){

   XDrawLine( actWin->d, drawable(actWin->executeWidget),
	      actWin->executeGc.normGC(),
	      oldMeterNeedleXEnd, oldMeterNeedleYEnd,
	      oldMeterNeedleXOrigin, oldMeterNeedleYOrigin);
   }
   else {

     xpoints[0].x = oldMeterNeedleXOrigin-1;
     xpoints[0].y = oldMeterNeedleYOrigin;

     xpoints[1].x = oldMeterNeedleXEnd;
     xpoints[1].y = oldMeterNeedleYEnd;

     xpoints[2].x = oldMeterNeedleXOrigin+1;
     xpoints[2].y = oldMeterNeedleYOrigin;

     xpoints[3].x = oldMeterNeedleXOrigin;
     xpoints[3].y = oldMeterNeedleYOrigin-1;

     xpoints[4].x = oldMeterNeedleXEnd;
     xpoints[4].y = oldMeterNeedleYEnd;

     xpoints[5].x = oldMeterNeedleXOrigin;
     xpoints[5].y = oldMeterNeedleYOrigin+1;

    XDrawLines( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), xpoints, 6, CoordModeOrigin );

   }
   
   /* draw new needle */

   actWin->executeGc.setFG( fgColor.getColor() );


   if (needleType == 0){

   XDrawLine( actWin->d, drawable(actWin->executeWidget),
	      actWin->executeGc.normGC(),
	      meterNeedleXend, meterNeedleYend,
	      meterNeedleXorigin, meterNeedleYorigin);
   }
   else {

     xpoints[0].x = meterNeedleXorigin-1;
     xpoints[0].y = meterNeedleYorigin;

     xpoints[1].x = meterNeedleXend;
     xpoints[1].y = meterNeedleYend;

     xpoints[2].x = meterNeedleXorigin+1;
     xpoints[2].y = meterNeedleYorigin;

     xpoints[3].x = meterNeedleXorigin;
     xpoints[3].y = meterNeedleYorigin-1;

     xpoints[4].x = meterNeedleXend;
     xpoints[4].y = meterNeedleYend;

     xpoints[5].x = meterNeedleXorigin;
     xpoints[5].y = meterNeedleYorigin+1;

     XDrawLines( actWin->d, drawable(actWin->executeWidget),
      actWin->executeGc.normGC(), xpoints, 6, CoordModeOrigin );

     actWin->executeGc.setFG( meterColor.getColor() );

   }

   actWin->executeGc.removeNormXClipRectangle();

   oldMeterNeedleXEnd = meterNeedleXend;
   oldMeterNeedleYEnd = meterNeedleYend;
   oldMeterNeedleXOrigin = meterNeedleXorigin;
   oldMeterNeedleYOrigin = meterNeedleYorigin;
 }
 if (bufInvalid){

   xR.x = faceX;
   xR.y = faceY;
   xR.width = faceW;
   xR.height = faceH;

   actWin->executeGc.addNormXClipRectangle( xR );

   actWin->executeGc.setFG( fgColor.getColor() );

   if (needleType == 0){

     XDrawLine( actWin->d, drawable(actWin->executeWidget),
		actWin->executeGc.normGC(),
		meterNeedleXend, meterNeedleYend,
		meterNeedleXorigin, meterNeedleYorigin);
   }    
   else{

     xpoints[0].x = meterNeedleXorigin-1;
     xpoints[0].y = meterNeedleYorigin;

     xpoints[1].x = meterNeedleXend;
     xpoints[1].y = meterNeedleYend;

     xpoints[2].x = meterNeedleXorigin+1;
     xpoints[2].y = meterNeedleYorigin;

     xpoints[3].x = meterNeedleXorigin;
     xpoints[3].y = meterNeedleYorigin-1;

     xpoints[4].x = meterNeedleXend;
     xpoints[4].y = meterNeedleYend;

     xpoints[5].x = meterNeedleXorigin;
     xpoints[5].y = meterNeedleYorigin+1;

     XDrawLines( actWin->d, drawable(actWin->executeWidget),
      actWin->executeGc.normGC(), xpoints, 6, CoordModeOrigin );

   }

   actWin->executeGc.removeNormXClipRectangle();

 }

  actWin->executeGc.restoreFg();

  bufInvalid = 0;

  return 1;

}

int activeMeterClass::activate (
  int pass,
  void *ptr )
{

int opStat;

  switch ( pass ) {

  case 1:

    needConnectInit = needInfoInit = needRefresh = needErase = needDraw = 0;
    needToEraseUnconnected = 0;
    needToDrawUnconnected = 0;
    unconnectedTimer = 0;
    aglPtr = ptr;
    meterW = 0;
    oldMeterW = 0;
    meterX = 0;
    oldMeterX = 0;
    opComplete = 0;
    oldMeterNeedleXEnd = 0;
    oldMeterNeedleYEnd = 0;
    meterNeedleXend = 1;
    meterNeedleYend = 1;
    oldMeterNeedleXOrigin = 0;
    oldMeterNeedleYOrigin = 0;
    readPvId = readPvLabelId = NULL;
    initialReadConnection = initialReadLabelConnection = 1;
    oldStat = oldSev = -1;
    strcpy( readPvLabel, "" );
    connection.init();

    drawStaticFlag = 1;

    active = activeInitFlag= 0;
    activeMode = 1;

    break;

  case 2:

    if ( !opComplete ) {

      if ( !readPvExpStr.getExpanded() ||
        blankOrComment( readPvExpStr.getExpanded() ) ) {
        readExists = 0;
      }
      else {
        readExists = 1;
        connection.addPv();
        meterColor.setConnectSensitive();
        fgColor.setConnectSensitive();
        scaleColor.setConnectSensitive();
      }

      if ( labelType == 1 ) {
        if ( !readPvLabelExpStr.getExpanded() ||
          blankOrComment( readPvLabelExpStr.getExpanded() ) ) {
          readLabelExists = 0;
        }
        else {
          readLabelExists = 1;
          connection.addPv();
        }
      }
      else {
        readLabelExists = 0;
      }

      if ( scaleMinExpStr.getExpanded() ) {
        scaleMin = atof( scaleMinExpStr.getExpanded() );
      }
      else {
        scaleMin = 0;
      }

      if ( scaleMaxExpStr.getExpanded() ) {
        scaleMax = atof( scaleMaxExpStr.getExpanded() );
      }
      else {
        scaleMax = 0;
      }

      if ( scalePrecExpStr.getExpanded() ) {
        scalePrecision = atol( scalePrecExpStr.getExpanded() );
      }
      else {
        scalePrecision = 0;
      }

      if ( labIntExpStr.getExpanded() ) {
        labelIntervals = atol( labIntExpStr.getExpanded() );
      }
      else {
        labelIntervals = 0;
      }

      if ( majorIntExpStr.getExpanded() ) {
        majorIntervals = atol( majorIntExpStr.getExpanded() );
      }
      else {
        majorIntervals = 0;
      }

      if ( minorIntExpStr.getExpanded() ) {
        minorIntervals = atol( minorIntExpStr.getExpanded() );
      }
      else {
        minorIntervals = 0;
      }

      initEnable();

      opStat = 1;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      if ( readExists ) {
	readPvId = the_PV_Factory->create( readPvExpStr.getExpanded() );
	if ( readPvId ) {
	  readPvId->add_conn_state_callback( meter_monitor_read_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeMeterClass_str38 );
          opStat = 0;
        }
      }

      if ( readLabelExists ) {
	readPvLabelId = the_PV_Factory->create(
         readPvLabelExpStr.getExpanded() );
	if ( readPvLabelId ) {
	  readPvLabelId->add_conn_state_callback(
           meter_monitor_read_label_connect_state,
           this );
	}
	else {
          fprintf( stderr, activeMeterClass_str46 );
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

int activeMeterClass::deactivate (
  int pass
) {

  if ( pass == 1 ) {

    active = 0;
    activeMode = 0;

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    if ( readExists ) {
      if ( readPvId ) {
        readPvId->remove_conn_state_callback( meter_monitor_read_connect_state,
         this );
        readPvId->remove_value_callback( meter_readUpdate, this );
        readPvId->release();
        readPvId = NULL;
      }
    }

    if ( readLabelExists ) {
      if ( readPvLabelId ) {
        readPvLabelId->remove_conn_state_callback(
         meter_monitor_read_label_connect_state,
         this );
        readPvLabelId->remove_value_callback( meter_readLabelUpdate, this );
        readPvLabelId->release();
        readPvLabelId = NULL;
      }
    }

  }

  return 1;

}

void activeMeterClass::updateDimensions ( void )
{

  if ( scaleFs ) {
    scaleFontAscent = scaleFs->ascent;
    scaleFontDescent = scaleFs->descent;
    scaleFontHeight = scaleFontAscent + scaleFontDescent;
  }
  else {
    scaleFontAscent = 10;
    scaleFontDescent = 5;
    scaleFontHeight = scaleFontAscent + scaleFontDescent;
  }

  if ( labelFs ) {
    labelFontAscent = labelFs->ascent;
    labelFontDescent = labelFs->descent;
    labelFontHeight = labelFontAscent + labelFontDescent;
  }
  else {
    labelFontAscent = 10;
    labelFontDescent = 5;
    labelFontHeight = labelFontAscent + labelFontDescent;
  }

}

void activeMeterClass::btnUp (
  int x,
  int y,
  int meterState,
  int meterNumber )
{

  if ( !enabled ) return;

}

void activeMeterClass::btnDown (
  int x,
  int y,
  int meterState,
  int meterNumber )
{

  if ( !enabled ) return;

}

void activeMeterClass::btnDrag (
  int x,
  int y,
  int meterState,
  int meterNumber )
{

  if ( !enabled ) return;

}

int activeMeterClass::getMeterActionRequest (
  int *up,
  int *down,
  int *drag )
{

  *up = 0;
  *down = 0;
  *drag = 0;
  return 1;

}

void activeMeterClass::bufInvalidate ( void )
{
  drawStaticFlag = 1;
  bufInvalid = 1;

}

int activeMeterClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( readPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( scaleMinExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  scaleMinExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( scaleMaxExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  scaleMaxExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( scalePrecExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  scalePrecExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( labIntExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  labIntExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( majorIntExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  majorIntExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( minorIntExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  minorIntExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( readPvLabelExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  readPvLabelExpStr.setRaw( tmpStr.getExpanded() );

  strncpy( literalLabel, readPvLabelExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  literalLabel[PV_Factory::MAX_PV_NAME] = 0;

  return 1;

}


int activeMeterClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = readPvLabelExpStr.expand1st( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;
  strncpy( literalLabel, readPvLabelExpStr.getExpanded(),
   PV_Factory::MAX_PV_NAME );
  literalLabel[PV_Factory::MAX_PV_NAME] = 0;

  stat = readPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = scaleMinExpStr.expand1st( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = scaleMaxExpStr.expand1st( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = scalePrecExpStr.expand1st( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = labIntExpStr.expand1st( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = majorIntExpStr.expand1st( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = minorIntExpStr.expand1st( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeMeterClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat = 1;

  stat = readPvLabelExpStr.expand2nd( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;
  strncpy( literalLabel, readPvLabelExpStr.getExpanded(),
   PV_Factory::MAX_PV_NAME );
  literalLabel[PV_Factory::MAX_PV_NAME] = 0;

  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = scaleMinExpStr.expand2nd( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = scaleMaxExpStr.expand2nd( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = scalePrecExpStr.expand2nd( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = labIntExpStr.expand2nd( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = majorIntExpStr.expand2nd( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  stat = minorIntExpStr.expand2nd( numMacros, macros, expansions );
  if ( !(stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeMeterClass::containsMacros ( void ) {

int result;

 return 1;

  result = readPvLabelExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = readPvExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = scaleMinExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = scaleMaxExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = scalePrecExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = labIntExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = majorIntExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  result = minorIntExpStr.containsPrimaryMacros();
  if ( result ) return 1;

  return 0;

}

int activeMeterClass::checkResizeSelectBox (
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

int activeMeterClass::checkResizeSelectBoxAbs (
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

void activeMeterClass::executeDeferred ( void ) {

double v;
int nc, ni, nr, ne, nd;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  v = curReadV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

  if ( nc ) {

    v = readV = curReadV = readPvId->get_double() - baseV;
    readMin = readPvId->get_lower_disp_limit();
    readMax = readPvId->get_upper_disp_limit();

    ni = ne = nd = 1;

  }

  if ( ni ) {

    if ( readExists ) {

      if ( initialReadConnection ) {

        initialReadConnection = 0;

        readPvId->add_value_callback( meter_readUpdate, this );

        if ( trackDelta ) {
          baseV = readPvId->get_double();
        }
        else {
	  baseV = 0;
        }

      }

    }

    if ( readLabelExists ) {

      if ( initialReadLabelConnection ) {

        initialReadLabelConnection = 0;

        readPvLabelId->add_value_callback( meter_readLabelUpdate, this );

      }

    }

    if ( readMax <= readMin ) readMax = readMin + 1.0;

    if ( readMax >= readMin ) {
      mode = METERC_K_MAX_GE_MIN;
    }
    else {
      mode = METERC_K_MAX_LT_MIN;
    }

    meterColor.setConnected();
    fgColor.setConnected();
    scaleColor.setConnected();

    active = 1;
    activeInitFlag = 1;

    v = readV = curReadV = readPvId->get_double() - baseV;
    bufInvalidate();
    eraseActive();
    drawActive();

  }

  if ( nr ) {
    bufInvalidate();
    eraseActive();
    if ( readPvId && readPvId->is_valid() ) {
      v = readV = curReadV = readPvId->get_double() - baseV;
    }
    drawActive();
  }

  if ( ne ) {
    eraseActive();
  }

  if ( nd ) {
    if ( readPvId && readPvId->is_valid() ) {
      v = readV = curReadV = readPvId->get_double() - baseV;
    }
    drawActive();
  }

}

char *activeMeterClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeMeterClass::nextDragName ( void ) {

  return NULL;

}

char *activeMeterClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    return readPvExpStr.getExpanded();

  }
  else {

    return readPvExpStr.getRaw();

  }

}

void activeMeterClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    meterColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColor.setColorIndex( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_OFFSETCOLOR_MASK )
    bgColor.setColorIndex( _offsetColor, actWin->ci );

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    scaleColor.setColorIndex( _textFgColor, actWin->ci);

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    labelColor.setColorIndex( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    tsColor.setColorIndex( _topShadowColor, actWin->ci );

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    bsColor.setColorIndex( _botShadowColor, actWin->ci );

  if ( _flag & ACTGRF_CTLFONTTAG_MASK ) {

    strcpy( scaleFontTag, _ctlFontTag );
    actWin->fi->loadFontTag( scaleFontTag );
    scaleFs = actWin->fi->getXFontStruct( scaleFontTag );

    strcpy( labelFontTag, _fontTag );
    actWin->fi->loadFontTag( labelFontTag );
    labelFs = actWin->fi->getXFontStruct( labelFontTag );

    updateDimensions();

  }

}

void activeMeterClass::changePvNames (
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

void activeMeterClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 1 ) {
    *n = 0;
    return;
  }

  *n = 1;
  pvs[0] = readPvId;

}

char *activeMeterClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return readPvExpStr.getRaw();
  }
  else if ( i == 1 ) {
    return literalLabel;
    // return readPvLabelExpStr.getRaw();
  }

  return NULL;

}

void activeMeterClass::replaceString (
  int i,
  int max,
  char *string
) {

int l;

  if ( i == 0 ) {
    readPvExpStr.setRaw( string );
  }
  else if ( i == 1 ) {
    l = max;
    if ( l > PV_Factory::MAX_PV_NAME ) l = PV_Factory::MAX_PV_NAME;
    strncpy( literalLabel, string, l );
    literalLabel[PV_Factory::MAX_PV_NAME] = 0;
    readPvLabelExpStr.setRaw( string );
  }

}

// crawler functions may return blank pv names
char *activeMeterClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return readPvExpStr.getExpanded();

}

char *activeMeterClass::crawlerGetNextPv ( void ) {

  return NULL;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMeterClassPtr ( void ) {

activeMeterClass *ptr;

  ptr = new activeMeterClass;
  return (void *) ptr;

}

void *clone_activeMeterClassPtr (
  void *_srcPtr )
{

activeMeterClass *ptr, *srcPtr;

  srcPtr = (activeMeterClass *) _srcPtr;

  ptr = new activeMeterClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
