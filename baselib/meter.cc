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

static void meterc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMeterClass *metero = (activeMeterClass *) client;

  metero->actWin->setChanged();

  metero->eraseSelectBoxCorners();
  metero->erase();

  metero->fgColorMode = metero->bufFgColorMode;
  if ( metero->fgColorMode == METERC_K_COLORMODE_ALARM )
    metero->fgColor.setAlarmSensitive();
  else
    metero->fgColor.setAlarmInsensitive();
  metero->fgColor.setColor( metero->bufFgColor, metero->actWin->ci );

  metero->meterColorMode = metero->bufMeterColorMode;
  if ( metero->meterColorMode == METERC_K_COLORMODE_ALARM )
    metero->meterColor.setAlarmSensitive();
  else
    metero->meterColor.setAlarmInsensitive();
  metero->meterColor.setColor( metero->bufMeterColor, metero->actWin->ci );

  metero->scaleColorMode = metero->bufScaleColorMode;
  if ( metero->scaleColorMode == METERC_K_COLORMODE_ALARM )
    metero->scaleColor.setAlarmSensitive();
  else
    metero->scaleColor.setAlarmInsensitive();
  metero->scaleColor.setColor( metero->bufScaleColor, metero->actWin->ci );

  metero->shadowMode = metero->bufShadowMode;

  metero->meterAngle = metero->bufMeterAngle;

  metero->scaleLimitsFromDb = metero->bufScaleLimitsFromDb;
  metero->scaleMin   = metero->bufScaleMin;
  metero->scaleMax   = metero->bufScaleMax;
  strncpy( metero->scaleFormat, metero->bufScaleFormat, 2 );
  metero->scalePrecision = metero->bufScalePrecision;

  metero->needleType = metero->bufNeedleType;

  metero->majorIntervals = metero->bufMajorIntervals;
  metero->minorIntervals = metero->bufMinorIntervals;

  metero->bgColor.setColor( metero->bufBgColor, metero->actWin->ci );

  metero->tsColor.setColor( metero->bufTsColor, metero->actWin->ci );
  metero->bsColor.setColor( metero->bufBsColor, metero->actWin->ci );

  metero->scaleColor.setColor( metero->bufScaleColor, metero->actWin->ci );
  metero->labelColor.setColor( metero->bufLabelColor, metero->actWin->ci );

  metero->controlPvExpStr.setRaw( metero->bufControlPvName );
  metero->readPvExpStr.setRaw( metero->bufReadPvName );

  strncpy( metero->label, metero->bufLabel, 39 );
  strncpy( metero->literalLabel, metero->bufLiteralLabel, 39 );

  metero->labelType = metero->bufLabelType;

  strncpy( metero->scaleFontTag, metero->scaleFm.currentFontTag(), 63 );

  metero->actWin->fi->loadFontTag( metero->scaleFontTag );
  metero->scaleFs = metero->actWin->fi->getXFontStruct( metero->scaleFontTag );

  strncpy( metero->labelFontTag, metero->labelFm.currentFontTag(), 63 );

  metero->actWin->fi->loadFontTag( metero->labelFontTag );
  metero->labelFs = metero->actWin->fi->getXFontStruct( metero->labelFontTag );

  metero->showScale = metero->bufShowScale;

  metero->x = metero->bufX;
  metero->sboxX = metero->bufX;

  metero->y = metero->bufY;
  metero->sboxY = metero->bufY;

  metero->w = metero->bufW;
  metero->sboxW = metero->bufW;

  metero->h = metero->bufH;
  metero->sboxH = metero->bufH;

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

  metero->erase();
  metero->deleteRequest = 1;
  metero->ef.popdown();
  metero->operationCancel();
  metero->drawAll();

}

#ifdef __epics__

static void meter_monitor_read_connect_state (
  struct connection_handler_args arg )
{

activeMeterClass *metero = (activeMeterClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    metero->needConnectInit = 1;
    metero->actWin->appCtx->proc->lock();
    metero->actWin->addDefExeNode( metero->aglPtr );
    metero->actWin->appCtx->proc->unlock();

  }
  else {

    metero->readPvConnected = 0;
    metero->active = 0;
    metero->meterColor.setDisconnected();
    metero->fgColor.setDisconnected();
    metero->bufInvalidate();
    metero->needDraw = 1;
    metero->actWin->appCtx->proc->lock();
    metero->actWin->addDefExeNode( metero->aglPtr );
    metero->actWin->appCtx->proc->unlock();

  }

}

static void meter_infoUpdate (
  struct event_handler_args ast_args )
{

activeMeterClass *metero = (activeMeterClass *) ast_args.usr;
struct dbr_gr_double controlRec = *( (dbr_gr_double *) ast_args.dbr );

  metero->needInfoInit = 1;
  metero->actWin->appCtx->proc->lock();
  metero->actWin->addDefExeNode( metero->aglPtr );
  metero->actWin->appCtx->proc->unlock();

  metero->curReadV = controlRec.value;
  metero->readMin = controlRec.lower_disp_limit;
  metero->readMax = controlRec.upper_disp_limit;


  metero->needErase = 1;
  metero->needDraw = 1;
  metero->actWin->appCtx->proc->lock();
  metero->actWin->addDefExeNode( metero->aglPtr );
  metero->actWin->appCtx->proc->unlock();
}

static void meter_readUpdate (
  struct event_handler_args ast_args )
{

  activeMeterClass *metero = (activeMeterClass *) ast_args.usr;

  if ( metero->active ) {

    metero->curReadV = *( (double *) ast_args.dbr );
    metero->needErase = 1;
    metero->needDraw = 1;
    metero->actWin->appCtx->proc->lock();
    metero->actWin->addDefExeNode( metero->aglPtr );
    metero->actWin->appCtx->proc->unlock();
  }

}

static void meter_alarmUpdate (
  struct event_handler_args ast_args )
{

activeMeterClass *metero = (activeMeterClass *) ast_args.usr;
struct dbr_sts_double statusRec;

  statusRec = *( (struct dbr_sts_double *) ast_args.dbr );

  metero->fgColor.setStatus( statusRec.status, statusRec.severity );
  metero->scaleColor.setStatus( statusRec.status, statusRec.severity );
  metero->meterColor.setStatus( statusRec.status, statusRec.severity );

  metero->bufInvalidate();
  metero->needDraw = 1;
  metero->actWin->appCtx->proc->lock();
  metero->actWin->addDefExeNode( metero->aglPtr );
  metero->actWin->appCtx->proc->unlock();

}

#endif

activeMeterClass::activeMeterClass ( void ) {

  name = new char[strlen("activeMeterClass")+1];
  strcpy( name, "activeMeterClass" );
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
  majorIntervals = 10;
  minorIntervals = 5;
  strcpy( scaleFormat, "g" );
  scalePrecision = 3;
  needleType = 1;
  shadowMode = 1;

  meterColorMode = METERC_K_COLORMODE_STATIC;
  scaleColorMode = METERC_K_COLORMODE_STATIC;
  fgColorMode = METERC_K_COLORMODE_STATIC;
  labelType = METERC_K_PV_LABEL;
  showScale = 1;
  useDisplayBg = 1;

  scaleLimitsFromDb = 1;

  readMin = scaleMin;
  readMax = scaleMax;

}

// copy constructor
activeMeterClass::activeMeterClass
 ( const activeMeterClass *source ) {

activeGraphicClass *metero = (activeGraphicClass *) this;

  metero->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMeterClass")+1];
  strcpy( name, "activeMeterClass" );

  meterCb = source->meterCb;
  fgCb = source->fgCb;
  bgCb = source->bgCb;
  tsCb = source->tsCb;
  bsCb = source->bsCb;
  scaleCb = source->scaleCb;
  labelCb = source->labelCb;

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

  controlPvExpStr.copy( source->controlPvExpStr );
  readPvExpStr.copy( source->readPvExpStr );

  strncpy( label, source->label, 39 );
  strncpy( literalLabel, source->literalLabel, 39);

  meterColorMode = source->meterColorMode;
  scaleColorMode = source->scaleColorMode;
  fgColorMode = source->fgColorMode;
  shadowMode = source->shadowMode;
  scaleLimitsFromDb = source->scaleLimitsFromDb;
  needleType = source->needleType;
  scalePrecision = source->scalePrecision;
  strncpy( scaleFormat, source->scaleFormat, 2 );
  meterAngle = source->meterAngle;
  scaleMin = source->scaleMin;
  scaleMax = source->scaleMax;
  labelType = source->labelType;
  showScale = source->showScale;
  useDisplayBg = source->useDisplayBg;
  drawStaticFlag = source->drawStaticFlag;

  majorIntervals = source->majorIntervals;
  minorIntervals = source->minorIntervals;

  readMin = scaleMin;
  readMax = scaleMax;

  minW = 100;
  minH = 50;
  activeMode = 0;

  updateDimensions();

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

  meterColor.setColor( actWin->defaultBgColor, actWin->ci );
  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColor( actWin->defaultOffsetColor, actWin->ci );
  scaleColor.setColor( actWin->defaultTextFgColor, actWin->ci);
  labelColor.setColor( actWin->defaultTextFgColor, actWin->ci );

  tsColor.setColor( actWin->defaultTopShadowColor, actWin->ci );
  bsColor.setColor( actWin->defaultBotShadowColor, actWin->ci );

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

int index;

  fprintf( f, "%-d %-d %-d\n", METERC_MAJOR_VERSION, METERC_MINOR_VERSION,
   METERC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getIndex( meterColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", meterColorMode );

  actWin->ci->getIndex( scaleColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", scaleColorMode );

  actWin->ci->getIndex( labelColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  actWin->ci->getIndex( fgColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fgColorMode );

  actWin->ci->getIndex( bgColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  actWin->ci->getIndex( tsColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  actWin->ci->getIndex( bsColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  if ( controlPvExpStr.getRaw() )
    writeStringToFile( f, controlPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( readPvExpStr.getRaw() )
    writeStringToFile( f, readPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, literalLabel );
  writeStringToFile( f, label );

  fprintf( f, "%-d\n", labelType );

  fprintf( f, "%-d\n", showScale );
  writeStringToFile( f, scaleFormat );
  fprintf( f, "%-d\n", scalePrecision);
  fprintf( f, "%-d\n", scaleLimitsFromDb );
  fprintf( f, "%-d\n", useDisplayBg );
  fprintf( f, "%-d\n", majorIntervals );
  fprintf( f, "%-d\n", minorIntervals );
  fprintf( f, "%-d\n", needleType );
  fprintf( f, "%-d\n", shadowMode );

  fprintf( f, "%-g\n",scaleMin  );
  fprintf( f, "%-g\n",scaleMax );


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

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[39+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    meterColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &meterColorMode ); actWin->incLine();
    if ( meterColorMode == METERC_K_COLORMODE_ALARM )
      meterColor.setAlarmSensitive();
    else
      meterColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    scaleColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &scaleColorMode ); actWin->incLine();
    if ( scaleColorMode == METERC_K_COLORMODE_ALARM )
      scaleColor.setAlarmSensitive();
    else
      scaleColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    labelColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    fgColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == METERC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    bgColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    tsColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    bsColor.setColor( pixel, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    meterColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &meterColorMode ); actWin->incLine();
    if ( meterColorMode == METERC_K_COLORMODE_ALARM )
      meterColor.setAlarmSensitive();
    else
      meterColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    scaleColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &scaleColorMode ); actWin->incLine();
    if ( scaleColorMode == METERC_K_COLORMODE_ALARM )
      scaleColor.setAlarmSensitive();
    else
      scaleColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    labelColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    fgColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &fgColorMode ); actWin->incLine();

    if ( fgColorMode == METERC_K_COLORMODE_ALARM )
      fgColor.setAlarmSensitive();
    else
      fgColor.setAlarmInsensitive();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    bgColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    tsColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    bsColor.setColor( pixel, actWin->ci );

  }

  bgColor.setAlarmInsensitive();

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  readPvExpStr.setRaw( oneName );

  readStringFromFile( literalLabel, 39, f ); actWin->incLine();

  readStringFromFile( label, 39, f ); actWin->incLine();

  fscanf( f, "%d\n", &labelType ); actWin->incLine();

  fscanf( f, "%d\n", &showScale ); actWin->incLine();

  if ( major > 1 || minor > 1 ) {
    readStringFromFile( oneName, 39, f ); actWin->incLine();
    strncpy( scaleFormat, oneName, 1 );
  }

  fscanf( f, "%d\n", &scalePrecision ); actWin->incLine();

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
  fscanf( f, "%d\n", &needleType ); actWin->incLine();
  fscanf( f, "%d\n", &shadowMode ); actWin->incLine();

  fscanf( f, "%lg\n", &scaleMin ); actWin->incLine();
  fscanf( f, "%lg\n", &scaleMax ); actWin->incLine();

  readStringFromFile( labelFontTag, 63, f ); actWin->incLine();
  actWin->fi->loadFontTag( labelFontTag );
  labelFs = actWin->fi->getXFontStruct( labelFontTag );
  updateFont( labelFontTag, &labelFs, &labelFontAscent, &labelFontDescent, &labelFontHeight );


  readStringFromFile( scaleFontTag, 63, f ); actWin->incLine();
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

  ptr = actWin->obj.getNameFromClass( "activeMeterClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeMeterClass_str2, 31 );

  strncat( title, activeMeterClass_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufMeterColor = meterColor.pixelColor();
  bufMeterColorMode = meterColorMode;
  bufScaleColorMode = scaleColorMode;

  bufMajorIntervals = majorIntervals;
  bufMinorIntervals = minorIntervals;

  bufFgColor = fgColor.pixelColor();
  bufFgColorMode = fgColorMode;
  
  bufShadowMode = shadowMode;
  bufTsColor = tsColor.pixelColor();
  bufBsColor = bsColor.pixelColor();

  bufScaleColor = scaleColor.pixelColor();
  bufLabelColor = labelColor.pixelColor();

  bufBgColor = bgColor.pixelColor();

  strncpy( bufScaleFontTag, scaleFontTag, 63 );
  strncpy( bufLabelFontTag, labelFontTag, 63 );

  if ( readPvExpStr.getRaw() )
    strncpy( bufReadPvName, readPvExpStr.getRaw(), 39 );
  else
    strncpy( bufReadPvName, "", 39 );

  if ( controlPvExpStr.getRaw() )
    strncpy( bufControlPvName, controlPvExpStr.getRaw(), 39 );
  else
    strncpy( bufControlPvName, "", 39 );

  strncpy( bufLabel, label, 39 );
  strncpy( bufLiteralLabel, literalLabel, 39);

  bufLabelType = labelType;
  bufScaleLimitsFromDb = scaleLimitsFromDb;
  bufMeterAngle = meterAngle;
  bufNeedleType = needleType;
  bufScalePrecision = scalePrecision;
  bufScaleMin = scaleMin;
  bufScaleMax = scaleMax;
  bufShowScale = showScale;
  bufUseDisplayBg = useDisplayBg;
  strncpy( bufScaleFormat, scaleFormat, 2 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeMeterClass_str4, 27, &bufX );
  ef.addTextField( activeMeterClass_str5, 27, &bufY );
  ef.addTextField( activeMeterClass_str6, 27, &bufW );
  ef.addTextField( activeMeterClass_str7, 27, &bufH );
  //   ef.addTextField( activeMeterClass_str8, 27, bufControlPvName, 39 );
  ef.addTextField( activeMeterClass_str9, 27, bufReadPvName, 39 );
  ef.addOption( activeMeterClass_str10, activeMeterClass_str11, &bufLabelType );
  ef.addTextField( activeMeterClass_str12, 27, bufLiteralLabel, 39 );
  ef.addFontMenu( activeMeterClass_str13, actWin->fi, &labelFm, labelFontTag );
  ef.addColorButton( activeMeterClass_str14, actWin->ci,&labelCb,&bufLabelColor);
  ef.addTextField(activeMeterClass_str15,27,&bufMeterAngle);
  ef.addToggle( activeMeterClass_str16, &bufShowScale );
  ef.addOption( activeMeterClass_str17, "g|f|e", bufScaleFormat, 2 );
  ef.addTextField( activeMeterClass_str19,27,&bufScalePrecision);
  ef.addToggle( activeMeterClass_str20, &bufScaleLimitsFromDb );
  ef.addTextField(activeMeterClass_str21,27,&bufScaleMin);
  ef.addTextField(activeMeterClass_str22,27,&bufScaleMax);
  ef.addFontMenu( activeMeterClass_str23, actWin->fi, &scaleFm, scaleFontTag );
  ef.addColorButton( activeMeterClass_str24, actWin->ci,&scaleCb,&bufScaleColor);
  ef.addToggle( activeMeterClass_str25, &bufScaleColorMode );
  ef.addTextField(activeMeterClass_str26 ,27 , &bufMajorIntervals);
  ef.addTextField(activeMeterClass_str27,27, &bufMinorIntervals);
  ef.addToggle(activeMeterClass_str28, &bufNeedleType);  
  ef.addColorButton( activeMeterClass_str29, actWin->ci, &fgCb, &bufFgColor );
  ef.addToggle( activeMeterClass_str30, &bufFgColorMode );
  ef.addColorButton( activeMeterClass_str31, actWin->ci, &meterCb, &bufMeterColor );
  ef.addToggle( activeMeterClass_str32, &bufMeterColorMode );
  ef.addColorButton( activeMeterClass_str33, actWin->ci, &bgCb, &bufBgColor );
  ef.addToggle(activeMeterClass_str34,&bufShadowMode);
  ef.addColorButton(activeMeterClass_str35,actWin->ci, &tsCb,&bufTsColor);
  ef.addColorButton(activeMeterClass_str36,actWin->ci,&bsCb,&bufBsColor);

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

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMeterClass::eraseActive ( void ) {

  if ( !activeMode) return 1;

  if ( bufInvalid) { 

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
		    actWin->executeGc.eraseGC(), oldMeterX, y, oldMeterW, h );
  }

  return 1;

}

int activeMeterClass::draw ( void ) {

  int tX, tY;
  char scaleMinString[39+1],scaleMaxString[39+1],scaleString[39+1];
  double majorAngle, majorAngleIncr, minorAngleIncr;
  double needleLength, insideArc;
  double majorTickSize = 10, minorTickSize = 5, midTickSize = 7;
  double needlePlusScale, scaleTextIncr, scaleValue;
  double meterTotalAngle, beginAngle, endAngle;
  double descentAngle;
  double verticalExtent, visibleFraction;
  double biggestHorizNeedlePlusScale,biggestVertNeedlePlusScale;
  int caseWidth = 5;
  int scaleMinWidth,scaleMaxWidth,scaleFontWidth;
  int faceX,faceY,faceW,faceH;
  double tickSize;
  int  farEndX, farEndY, nearEndX, nearEndY, i;
  char fmt[31+1];

  if (meterAngle < 10)  meterAngle = 10;
  if (meterAngle > 360) meterAngle = 360;

  meterTotalAngle = 3.1415926535898 * meterAngle / 180.0;
  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

 if (scaleLimitsFromDb){
   scaleMin = readMin;
   scaleMax = readMax;
 }

 faceX = x + caseWidth;
 faceY = y + caseWidth;
 faceW = w - 2 * caseWidth;
 faceH = h - 2 * caseWidth;

// if (labelType = 0 || !(strlen(literalLabel))) {
//    fetch pv name ...
//}
// else if (labelType = 1) {
//    fetch pv label ...
//}
// else if (labelType =2) {
 if (strlen(literalLabel)){
   strncpy (label, literalLabel, 39);
   faceH = faceH + caseWidth - 4 - labelFontHeight;
 }
 else strcpy (label,"");

//}

 if(scalePrecision > 10 || scalePrecision < 0) scalePrecision = 1;
 
 sprintf( fmt, "%%.%-d%s", scalePrecision, scaleFormat );

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

 if (majorIntervals < 1) majorIntervals = 1;
 scaleTextIncr = (scaleMax - scaleMin) / majorIntervals;

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
	
 // needleLength = 0.83 * needlePlusScale;
 // insideArc    = 0.85 * needlePlusScale;
 // // majorTickSize= 0.15 * needlePlusScale;

 majorTickSize = scaleFontHeight * 0.8;
 if ( majorTickSize > 15 ) majorTickSize = 15;
 insideArc     = needlePlusScale - majorTickSize;
 needleLength = 0.98 * insideArc;

 // printf ("major tick size is   %f\n",majorTickSize);
 // printf ("scale font height is %d\n\n",scaleFontHeight);

 minorTickSize= 0.5 * majorTickSize;
 midTickSize  = 0.7 * majorTickSize;

 meterNeedleXorigin = faceW / 2;
 meterNeedleYorigin = (int) ( needlePlusScale + 4 + scaleFontHeight);

 meterNeedleXorigin += faceX;
 meterNeedleYorigin += faceY;
 meterNeedleXend    += faceX;
 meterNeedleYend    += faceY;    

   actWin->executeGc.setFG( meterColor.getColor() );
   
   XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
		   actWin->executeGc.normGC(), x, y, w, h );

   
   actWin->executeGc.setFG( bgColor.getColor() );
   
   XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
		   actWin->executeGc.normGC(),
		   faceX, faceY, faceW, faceH );

   actWin->executeGc.setFG( labelColor.getColor() );

   if ( strcmp( labelFontTag, "" ) != 0 ) {
     actWin->executeGc.setFontTag( labelFontTag, actWin->fi );
   }

   if (strlen (label)){
     drawText (actWin->executeWidget, &actWin->executeGc,
	       labelFs, faceX + 2, faceY + faceH + 2,
	       XmALIGNMENT_BEGINNING, label);
   }

   if (shadowMode) {

     actWin->executeGc.setFG( tsColor.getColor() );

     XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
		   actWin->executeGc.normGC(), x, y, w, h );

     XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
		   actWin->executeGc.normGC(),
		     faceX, faceY, faceW, faceH );

    actWin->executeGc.setFG( bsColor.getColor() );

    XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       x,y+h,
	       x+w,y+h);

    XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       x+w, y,
	       x+w,y+h);

    XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       faceX, faceY,
	       faceX, faceY + faceH);

    XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       faceX, faceY,
	       faceX + faceW, faceY );

   }
 
 // if (drawStaticFlag && showScale){
 
 
 if ((majorIntervals != 0) && (minorIntervals !=0 && showScale)){

   //   majorAngleIncr = 3.14159265 / majorIntervals;
   majorAngleIncr = meterTotalAngle / majorIntervals;
   minorAngleIncr = majorAngleIncr / minorIntervals;
   beginAngle = descentAngle;
   endAngle   = meterTotalAngle + beginAngle;

   for (majorAngle = beginAngle, scaleValue = scaleMax;
	majorAngle <= (1.001 * endAngle);
	majorAngle += majorAngleIncr, scaleValue -= scaleTextIncr){
     
     farEndX = (int) (meterNeedleXorigin + (insideArc + majorTickSize) * cos(majorAngle));
     farEndY = (int) (meterNeedleYorigin - (insideArc + majorTickSize) * sin(majorAngle));
     nearEndX = (int) (meterNeedleXorigin + insideArc * cos(majorAngle));
     nearEndY = (int) (meterNeedleYorigin - insideArc * sin(majorAngle));
     
     actWin->executeGc.setFG( scaleColor.getColor() );
     XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
		actWin->executeGc.normGC(),
		farEndX,farEndY,
		nearEndX,nearEndY);
     
     if ( strcmp( scaleFontTag, "" ) != 0 ) {
       actWin->executeGc.setFontTag( scaleFontTag, actWin->fi );
     }
     
     updateDimensions();
     
     if (majorAngle < (beginAngle + 0.001)){
       tX = farEndX + 2;
       tY = farEndY - scaleFontAscent/2;
       drawText (actWin->executeWidget, &actWin->executeGc, scaleFs, tX, tY,
		 XmALIGNMENT_BEGINNING, scaleMaxString);
     }
     else if (majorAngle <= 1.50){
       sprintf (scaleString, fmt, scaleValue);
       tX = farEndX + 2;
       tY = farEndY - scaleFontAscent;
       drawText (actWin->executeWidget, &actWin->executeGc, scaleFs, tX, tY,
		 XmALIGNMENT_BEGINNING, scaleString);
     }
     else if (majorAngle <= 1.65){
       sprintf (scaleString, fmt, scaleValue);
       tX = farEndX;
       tY = farEndY - scaleFontAscent - 2;
       drawText (actWin->executeWidget, &actWin->executeGc, scaleFs, tX, tY,
		 XmALIGNMENT_CENTER, scaleString);
     }
     else if (majorAngle <= (endAngle - 0.001)){
       sprintf (scaleString, fmt, scaleValue);
       tX = farEndX - 2;
       tY = farEndY - scaleFontAscent;
       drawText (actWin->executeWidget, &actWin->executeGc, scaleFs, tX, tY,
		 XmALIGNMENT_END, scaleString);
     }
     else if (majorAngle > (endAngle - 0.001)){
       tX = farEndX -2;
       tY = farEndY - scaleFontAscent/2;
       drawText (actWin->executeWidget, &actWin->executeGc, scaleFs, tX, tY,
		 XmALIGNMENT_END, scaleMinString);
     }       
   }
   for (majorAngle = beginAngle;
	majorAngle <(0.99 *endAngle);
	majorAngle += majorAngleIncr){
     
     for (i=1; i < minorIntervals; i++){
       if ((i * minorAngleIncr) == (majorAngleIncr /2))
	 tickSize = midTickSize;
       else
	 tickSize = minorTickSize;
       
       farEndX = (int) (meterNeedleXorigin +
	 (insideArc + tickSize) * cos(i * minorAngleIncr + majorAngle));
       farEndY = (int) (meterNeedleYorigin -
	 (insideArc + tickSize) * sin(i * minorAngleIncr + majorAngle));
       nearEndX = (int) (meterNeedleXorigin + insideArc * cos(i * minorAngleIncr + majorAngle));
       nearEndY = (int) (meterNeedleYorigin - insideArc * sin(i * minorAngleIncr + majorAngle));
       XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
		  actWin->executeGc.normGC(),
		  farEndX,farEndY,
		  nearEndX,nearEndY);
     }
   }
 }
 // }
 actWin->drawGc.restoreFg();
 
 return 1;
 
}

int activeMeterClass::drawActive ( void ) {

  int tX, tY;
  //XRectangle xR = { x, y, w, h };
 char scaleMinString[39+1],scaleMaxString[39+1],scaleString[39+1];
 double majorAngle, majorAngleIncr, minorAngleIncr;
 double meterTotalAngle, beginAngle, endAngle;
 double descentAngle;
 double verticalExtent, visibleFraction;
 double biggestVertNeedlePlusScale, biggestHorizNeedlePlusScale;
 double needleAngle;
 double needleLength, insideArc;
 double majorTickSize = 10, minorTickSize = 5, midTickSize = 7;
 double needlePlusScale, scaleTextIncr, scaleValue;
 int caseWidth = 5;
 int scaleMinWidth,scaleMaxWidth,scaleFontWidth;
 int faceX,faceY,faceW,faceH;
 XRectangle xR;
 double tickSize;
 int  farEndX, farEndY, nearEndX, nearEndY, i;
 char fmt[31+1];

XPoint xpoints[3];

 meterTotalAngle = 3.1415926535898 * meterAngle / 180.0;

 if ( !activeMode || !activeInitFlag) return 1;

 if (scaleLimitsFromDb){
   scaleMin = readMin;
   scaleMax = readMax;
 }

 faceX = x + caseWidth;
 faceY = y + caseWidth;
 faceW = w - 2 * caseWidth;
 faceH = h - 2 * caseWidth;

// if (labelType = 0) {
//    fetch pv name ...
//}
// else if (labelType = 1) {
//    fetch pv label ...
//}
// else if (labelType =2) {
 if (strlen(literalLabel)){
   strncpy (label, literalLabel, 39);
   faceH = faceH + caseWidth - 4 - labelFontHeight;
 }
 else strcpy (label,"");

//}

 actWin->executeGc.saveFg();

 if(scalePrecision > 10 || scalePrecision < 0) scalePrecision = 1;
 
 sprintf( fmt, "%%.%-d%s", scalePrecision, scaleFormat );

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

 if (majorIntervals < 1) majorIntervals = 1;
 scaleTextIncr = (scaleMax - scaleMin) / majorIntervals;

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
 // majorTickSize= 0.15 * needlePlusScale;

 majorTickSize = scaleFontHeight * 0.8;
 if ( majorTickSize > 15 ) majorTickSize = 15;
 insideArc     = needlePlusScale - majorTickSize;
 needleLength = 0.98 * insideArc;

 minorTickSize= 0.5 * majorTickSize;
 midTickSize  = 0.7 * majorTickSize;

 // crude attempt to prevent divide by zero in angle calc
 if ((scaleMax - scaleMin) < 0.000001 * (readV - scaleMin)){
   scaleMax = scaleMin +1.0;
   // printf ("check meter scale min and max\n");
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
   
   XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
		   actWin->executeGc.normGC(), x, y, w, h );

   
   actWin->executeGc.setFG( bgColor.getColor() );
   
   XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
		   actWin->executeGc.normGC(),
		   faceX, faceY, faceW, faceH );

   actWin->executeGc.setFG( labelColor.getColor() );

   if ( strcmp( labelFontTag, "" ) != 0 ) {
     actWin->executeGc.setFontTag( labelFontTag, actWin->fi );
   }

   if (strlen (label)){
     drawText (actWin->executeWidget, &actWin->executeGc,
	       labelFs, faceX + 2, faceY + faceH + 2,
	       XmALIGNMENT_BEGINNING, label);
   }

   if (shadowMode) {

     actWin->executeGc.setFG( tsColor.getColor() );

     XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
		   actWin->executeGc.normGC(), x, y, w, h );

     XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
		   actWin->executeGc.normGC(),
		     faceX, faceY, faceW, faceH );

    actWin->executeGc.setFG( bsColor.getColor() );

    XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       x,y+h,
	       x+w,y+h);

    XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       x+w, y,
	       x+w,y+h);

    XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       faceX, faceY,
	       faceX, faceY + faceH);

    XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(),
	       faceX, faceY,
	       faceX + faceW, faceY );

   }   
   
 }
 
 
 if (drawStaticFlag && showScale){

   drawStaticFlag = 0;

   
   if ((majorIntervals != 0) && (minorIntervals !=0 && showScale)){
     
     xR.x = faceX;
     xR.y = faceY;
     xR.width  = faceW;
     xR.height = faceH;

     actWin->executeGc.addNormXClipRectangle( xR );

     //     majorAngleIncr = 3.14159265 / majorIntervals;
     majorAngleIncr = meterTotalAngle / majorIntervals;
     minorAngleIncr = majorAngleIncr / minorIntervals;
     beginAngle = descentAngle;
     endAngle   = meterTotalAngle + beginAngle;

     //      printf ("needleLength %5.1f meterNeedleXOrigin %5d\n",needleLength,meterNeedleXorigin);
     
     //      printf ("majorAngleIncr %5.1f minorAngleIncr %5.1f\n",majorAngleIncr,minorAngleIncr);
     //      printf ("insideArc %5.1f majorTickSize %5.1f\n",insideArc, majorTickSize);
     
     for (majorAngle = beginAngle, scaleValue = scaleMax;
	  majorAngle <= (1.001 * endAngle);
	  majorAngle += majorAngleIncr, scaleValue -= scaleTextIncr){
       
       farEndX = (int) (meterNeedleXorigin + (insideArc + majorTickSize) * cos(majorAngle));
       farEndY = (int) (meterNeedleYorigin - (insideArc + majorTickSize) * sin(majorAngle));
       nearEndX = (int) (meterNeedleXorigin + insideArc * cos(majorAngle));
       nearEndY = (int) (meterNeedleYorigin - insideArc * sin(majorAngle));
       
       actWin->executeGc.setFG( scaleColor.getColor() );
       XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
		  actWin->executeGc.normGC(),
		  farEndX,farEndY,
		  nearEndX,nearEndY);

       if ( strcmp( scaleFontTag, "" ) != 0 ) {
         actWin->executeGc.setFontTag( scaleFontTag, actWin->fi );
       }

       updateDimensions();

       if (majorAngle < (beginAngle + 0.001)){
	 tX = farEndX + 2;
	 tY = farEndY - scaleFontAscent/2;
	 drawText (actWin->executeWidget, &actWin->executeGc, scaleFs, tX, tY,
		   XmALIGNMENT_BEGINNING, scaleMaxString);
       }
       else if (majorAngle <= 1.50){
	 sprintf (scaleString, fmt, scaleValue);
	 tX = farEndX + 2;
	 tY = farEndY - scaleFontAscent;
	 drawText (actWin->executeWidget, &actWin->executeGc, scaleFs, tX, tY,
		   XmALIGNMENT_BEGINNING, scaleString);
       }
       else if (majorAngle <= 1.65){
	 sprintf (scaleString, fmt, scaleValue);
	 tX = farEndX;
	 tY = farEndY - scaleFontAscent - 2;
	 drawText (actWin->executeWidget, &actWin->executeGc, scaleFs, tX, tY,
		   XmALIGNMENT_CENTER, scaleString);
       }
       else if (majorAngle <= (endAngle - 0.001)){
	 sprintf (scaleString, fmt, scaleValue);
	 tX = farEndX - 2;
	 tY = farEndY - scaleFontAscent;
	 drawText (actWin->executeWidget, &actWin->executeGc, scaleFs, tX, tY,
		   XmALIGNMENT_END, scaleString);
       }
       else if (majorAngle > (endAngle - 0.001)){
	 tX = farEndX -2;
	 tY = farEndY - scaleFontAscent/2;
	 drawText (actWin->executeWidget, &actWin->executeGc, scaleFs, tX, tY,
		   XmALIGNMENT_END, scaleMinString);
       }       
     }
     for (majorAngle = beginAngle;
	  majorAngle < (0.99 * endAngle);
	  majorAngle += majorAngleIncr){
       
       for (i=1; i < minorIntervals; i++){
	 if ((i * minorAngleIncr) == (majorAngleIncr /2))
	   tickSize = midTickSize;
	 else
	   tickSize = minorTickSize;
	 
	 farEndX = (int) (meterNeedleXorigin +
	   (insideArc + tickSize) * cos(i * minorAngleIncr + majorAngle));
	 farEndY = (int) (meterNeedleYorigin -
	   (insideArc + tickSize) * sin(i * minorAngleIncr + majorAngle));
	 nearEndX = (int) (meterNeedleXorigin +
			   insideArc * cos(i * minorAngleIncr + majorAngle));
	 nearEndY = (int) (meterNeedleYorigin -
			   insideArc * sin(i * minorAngleIncr + majorAngle));
	 XDrawLine (actWin->d, XtWindow(actWin->executeWidget),
		    actWin->executeGc.normGC(),
		    farEndX,farEndY,
		    nearEndX,nearEndY);
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

   XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
	      actWin->executeGc.normGC(),
	      oldMeterNeedleXEnd, oldMeterNeedleYEnd,
	      oldMeterNeedleXOrigin, oldMeterNeedleYOrigin);
   }
   else {

     //printf ("needleType is %5d\n",needleType);
#if 0
   XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
	      actWin->executeGc.normGC(),
	      oldMeterNeedleXEnd, oldMeterNeedleYEnd,
	      oldMeterNeedleXOrigin, oldMeterNeedleYOrigin);
   XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
	      actWin->executeGc.normGC(),
	      oldMeterNeedleXEnd, oldMeterNeedleYEnd,
	      oldMeterNeedleXOrigin+1, oldMeterNeedleYOrigin);
   XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
	      actWin->executeGc.normGC(),
	      oldMeterNeedleXEnd, oldMeterNeedleYEnd,
	      oldMeterNeedleXOrigin, oldMeterNeedleYOrigin-1);
   XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
	      actWin->executeGc.normGC(),
	      oldMeterNeedleXEnd, oldMeterNeedleYEnd,
	      oldMeterNeedleXOrigin-1, oldMeterNeedleYOrigin);
   XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
	      actWin->executeGc.normGC(),
	      oldMeterNeedleXEnd, oldMeterNeedleYEnd,
	      oldMeterNeedleXOrigin, oldMeterNeedleYOrigin+1);
#endif

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

    XFillPolygon( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), xpoints, 6, Nonconvex, CoordModeOrigin );

   }
   
   /* draw new needle */

   actWin->executeGc.setFG( fgColor.getColor() );


   if (needleType == 0){

   XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
	      actWin->executeGc.normGC(),
	      meterNeedleXend, meterNeedleYend,
	      meterNeedleXorigin, meterNeedleYorigin);
   }
   else {

#if 0
     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		actWin->executeGc.normGC(),
		meterNeedleXend, meterNeedleYend,
		meterNeedleXorigin+1, meterNeedleYorigin);
     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		actWin->executeGc.normGC(),
		meterNeedleXend, meterNeedleYend,
		meterNeedleXorigin-1, meterNeedleYorigin);
     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		actWin->executeGc.normGC(),
		meterNeedleXend, meterNeedleYend,
		meterNeedleXorigin, meterNeedleYorigin-1);
     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		actWin->executeGc.normGC(),
		meterNeedleXend, meterNeedleYend,
		meterNeedleXorigin, meterNeedleYorigin+1);
#endif

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

     XFillPolygon( actWin->d, XtWindow(actWin->executeWidget),
      actWin->executeGc.normGC(), xpoints, 6, Nonconvex, CoordModeOrigin );

     actWin->executeGc.setFG( meterColor.getColor() );

#if 0     
     XFillArc( actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(), meterNeedleXorigin-5,
	       meterNeedleYorigin-5, 10, 10, 0, 23040 );
     XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(), meterNeedleXorigin-5,
	       meterNeedleYorigin-5, 10, 10, 0, 23040 );
#endif
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

     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		actWin->executeGc.normGC(),
		meterNeedleXend, meterNeedleYend,
		meterNeedleXorigin, meterNeedleYorigin);
   }    
   else{

     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		actWin->executeGc.normGC(),
		meterNeedleXend, meterNeedleYend,
		meterNeedleXorigin+1, meterNeedleYorigin);
     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		actWin->executeGc.normGC(),
		meterNeedleXend, meterNeedleYend,
		meterNeedleXorigin-1, meterNeedleYorigin);
     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		actWin->executeGc.normGC(),
		meterNeedleXend, meterNeedleYend,
		meterNeedleXorigin, meterNeedleYorigin-1);
     XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		actWin->executeGc.normGC(),
		meterNeedleXend, meterNeedleYend,
		meterNeedleXorigin, meterNeedleYorigin+1);
     
     actWin->executeGc.setFG( meterColor.getColor() );

#if 0       
     XFillArc( actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(), meterNeedleXorigin-5,
	       meterNeedleYorigin-5, 10, 10, 0, 23040 );
     XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(), meterNeedleXorigin-5,
	       meterNeedleYorigin-5, 10, 10, 0, 23040 );
#endif
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

int stat, opStat;

  switch ( pass ) {

  case 1:

    needConnectInit = needInfoInit = needRefresh = needErase = needDraw = 0;
    aglPtr = ptr;
    meterW = 0;
    oldMeterW = 0;
    meterX = 0;
    oldMeterX = 0;
    opComplete = 0;
    oldMeterNeedleXEnd = 0;
    oldMeterNeedleYEnd = 0;
    oldMeterNeedleXOrigin = 0;
    oldMeterNeedleYOrigin = 0;

#ifdef __epics__
    readEventId = alarmEventId = 0;
#endif

    drawStaticFlag = 1;

    controlPvConnected = readPvConnected = active = activeInitFlag= 0;
    activeMode = 1;

    if ( !controlPvExpStr.getExpanded() ||
       ( strcmp( controlPvExpStr.getExpanded(), "" ) == 0 ) ) {
      controlExists = 0;
    }
    else {
      controlExists = 1;
    }

    if ( !readPvExpStr.getExpanded() ||
       ( strcmp( readPvExpStr.getExpanded(), "" ) == 0 ) ) {
      readExists = 0;
    }
    else {
      readExists = 1;
      meterColor.setConnectSensitive();
      fgColor.setConnectSensitive();
    }

    break;

  case 2:

    if ( !opComplete ) {

      opStat = 1;

#ifdef __epics__

//       if ( controlExists ) {
//         stat = ca_search_and_connect( controlPvExpStr.getExpanded(),
//          &controlPvId, meter_monitor_control_connect_state, this );
//         if ( stat != ECA_NORMAL ) {
//           printf( activeMeterClass_str37 );
//           opStat = 0;
//         }
//       }

      if ( readExists ) {
        stat = ca_search_and_connect( readPvExpStr.getExpanded(), &readPvId,
         meter_monitor_read_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeMeterClass_str38 );
          opStat = 0;
        }
      }

      if ( opStat & 1 ) opComplete = 1;

#endif

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

int stat;

  if ( pass == 1 ) {

  active = 0;
  activeMode = 0;

#ifdef __epics__

  if ( readExists ) {
    stat = ca_clear_channel( readPvId );
    if ( stat != ECA_NORMAL )
      printf( activeMeterClass_str39 );
  }

#endif

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

}

void activeMeterClass::btnDown (
  int x,
  int y,
  int meterState,
  int meterNumber )
{

}

void activeMeterClass::btnDrag (
  int x,
  int y,
  int meterState,
  int meterNumber )
{

}

int activeMeterClass::getMeterActionRequest (
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

void activeMeterClass::bufInvalidate ( void )
{
  drawStaticFlag = 1;
  bufInvalid = 1;

}

int activeMeterClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = readPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeMeterClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeMeterClass::containsMacros ( void ) {

int stat;

  stat = readPvExpStr.containsPrimaryMacros();

  return stat;

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

  if ( -h != -1 ) {
    tmph = _h;
    if ( tmph < minH ) {
      ret_stat = 0;
    }
  }

  return ret_stat;

}

void activeMeterClass::executeDeferred ( void ) {

double v;
int stat, nc, ni, nr, ne, nd;

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

#ifdef __epics__

  if ( nc ) {

    readPvConnected = 1;

    stat = ca_get_callback( DBR_GR_DOUBLE, readPvId, meter_infoUpdate,
     (void *) this );
    if ( stat != ECA_NORMAL )
      printf( activeMeterClass_str40 );

    return;

  }

  if ( ni ) {

    if ( !readEventId ) {

      stat = ca_add_masked_array_event( DBR_DOUBLE, 1, readPvId,
       meter_readUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &readEventId, DBE_VALUE );
      if ( stat != ECA_NORMAL )
        printf( activeMeterClass_str41 );

    }

    if ( !alarmEventId ) {

      stat = ca_add_masked_array_event( DBR_STS_DOUBLE, 1, readPvId,
       meter_alarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &alarmEventId, DBE_ALARM );
      if ( stat != ECA_NORMAL )
        printf( activeMeterClass_str42 );

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

    active = 1;
    activeInitFlag = 1;

    readV = v;
    bufInvalidate();
    eraseActive();
    drawActive();

    return;

  }

#endif

  if ( nr ) {
    bufInvalidate();
    eraseActive();
    drawActive();
  }

  if ( ne ) {
    eraseActive();
  }

  if ( nd ) {
    readV = v;
    drawActive();
  }

}

char *activeMeterClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeMeterClass::nextDragName ( void ) {

  return NULL;

}

char *activeMeterClass::dragValue (
  int i ) {

  return readPvExpStr.getExpanded();

}

void activeMeterClass::changeDisplayParams (
  unsigned int _flag,
  char *_fontTag,
  int _alignment,
  char *_ctlFontTag,
  int _ctlAlignment,
  char *_btnFontTag,
  int _btnAlignment,
  unsigned int _textFgColor,
  unsigned int _fg1Color,
  unsigned int _fg2Color,
  unsigned int _offsetColor,
  unsigned int _bgColor,
  unsigned int _topShadowColor,
  unsigned int _botShadowColor )
{

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    meterColor.setColor( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColor.setColor( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_OFFSETCOLOR_MASK )
    bgColor.setColor( _offsetColor, actWin->ci );

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    scaleColor.setColor( _textFgColor, actWin->ci);

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    labelColor.setColor( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    tsColor.setColor( _topShadowColor, actWin->ci );

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    bsColor.setColor( _botShadowColor, actWin->ci );

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
