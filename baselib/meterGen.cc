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

#define __meterGen_cc 1

#include "meterGen.h"
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

  strncpy( metero->pvUserClassName,
   metero->actWin->pvObj.getPvName(metero->pvNameIndex), 15 );

  strncpy( metero->pvClassName,
   metero->actWin->pvObj.getPvClassName(metero->pvNameIndex), 15 );

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

  meterc_edit_update( w, client, call );
  metero->refresh( metero );

}

static void meterc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMeterClass *metero = (activeMeterClass *) client;

  meterc_edit_apply ( w, client, call );
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
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeMeterClass *metero = (activeMeterClass *) clientData;

  metero->actWin->appCtx->proc->lock();

  if ( ! metero->activeMode ) {
    metero->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    metero->needConnectInit = 1;

    metero->actWin->addDefExeNode( metero->aglPtr );

  }
  else {

    metero->readPvConnected = 0;
    metero->active = 0;
    metero->meterColor.setDisconnected();
    metero->fgColor.setDisconnected();
    metero->bufInvalidate();
    metero->needDraw = 1;

    metero->actWin->addDefExeNode( metero->aglPtr );

  }

  metero->actWin->appCtx->proc->unlock();

}

static void meter_infoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeMeterClass *metero = (activeMeterClass *) clientData;

  metero->actWin->appCtx->proc->lock();

  if ( ! metero->activeMode ) {
    metero->actWin->appCtx->proc->unlock();
    return;
  }

  metero->needInfoInit = 1;
  metero->actWin->addDefExeNode( metero->aglPtr );

  metero->curReadV = *( (double *) classPtr->getValue( args ) );
  metero->readMin = (double) classPtr->getLoOpr( args );
  metero->readMax = (double) classPtr->getHiOpr( args );

  metero->actWin->appCtx->proc->unlock();

}

static void meter_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

  activeMeterClass *metero = (activeMeterClass *) clientData;

  metero->actWin->appCtx->proc->lock();

  if ( ! metero->activeMode ) {
    metero->actWin->appCtx->proc->unlock();
    return;
  }

  metero->curReadV = *( (double *) classPtr->getValue( args ) );
  metero->needErase = 1;
  metero->needDraw = 1;

  metero->actWin->addDefExeNode( metero->aglPtr );

  metero->actWin->appCtx->proc->unlock();

}

static void meter_alarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeMeterClass *metero = (activeMeterClass *) clientData;

  metero->actWin->appCtx->proc->lock();

  if ( ! metero->activeMode ) {
    metero->actWin->appCtx->proc->unlock();
    return;
  }

  metero->fgColor.setStatus( classPtr->getStatus( args ),
   classPtr->getSeverity( args ) );
  metero->scaleColor.setStatus( classPtr->getStatus( args ),
   classPtr->getSeverity( args ) );
  metero->meterColor.setStatus( classPtr->getStatus( args ),
   classPtr->getSeverity( args ) );

  metero->bufInvalidate();
  metero->needDraw = 1;

  metero->actWin->addDefExeNode( metero->aglPtr );

  metero->actWin->appCtx->proc->unlock();

}

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

  strcpy( pvClassName, "" );
  strcpy( pvUserClassName, "" );

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

  strncpy( pvClassName, source->pvClassName, 15 );
  strncpy( pvUserClassName, source->pvUserClassName, 15 );

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

  strncpy( pvUserClassName, actWin->defaultPvType, 15 );

  this->editCreate();

  return 1;

}

int activeMeterClass::save (
  FILE *f )
{

int r, g, b;

  fprintf( f, "%-d %-d %-d\n", METERC_MAJOR_VERSION, METERC_MINOR_VERSION,
   METERC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getRGB( meterColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", meterColorMode );

  actWin->ci->getRGB( scaleColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", scaleColorMode );

  actWin->ci->getRGB( labelColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( fgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", fgColorMode );

  actWin->ci->getRGB( bgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( tsColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( bsColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

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

  writeStringToFile( f, pvClassName );

  return 1;

}

int activeMeterClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b;
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

  if ( ( major > 1 ) || ( minor > 2 ) ) {

    readStringFromFile( pvClassName, 15, f ); actWin->incLine();

    strncpy( pvUserClassName, actWin->pvObj.getNameFromClass( pvClassName ),
     15 );

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
    strncpy( title, "Unknown object", 31 );

  strncat( title, " Properties", 31 );

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

  ef.addTextField( "X", 27, &bufX );
  ef.addTextField( "Y", 27, &bufY );
  ef.addTextField( "Width", 27, &bufW );
  ef.addTextField( "Height", 27, &bufH );
  //   ef.addTextField( "Control PV", 27, bufControlPvName, 39 );
  ef.addTextField( "Readback PV", 27, bufReadPvName, 39 );
  ef.addOption( "Label Type", "PV Name|PV Label|Literal", &bufLabelType );
  ef.addTextField( "Label", 27, bufLiteralLabel, 39 );
  ef.addFontMenu( "Label Font", actWin->fi, &labelFm, labelFontTag );
  ef.addColorButton( "Label Color", actWin->ci,&labelCb,&bufLabelColor);
  ef.addTextField("Total Display Angle",27,&bufMeterAngle);
  ef.addToggle( "Show Scale", &bufShowScale );
  ef.addOption( "Scale Format", "g|f|e", bufScaleFormat, 2 );
  ef.addTextField( "Scale Precision",27,&bufScalePrecision);
  ef.addToggle( "Min & Max From DB", &bufScaleLimitsFromDb );
  ef.addTextField("Scale Min",27,&bufScaleMin);
  ef.addTextField("Scale Max",27,&bufScaleMax);
  ef.addFontMenu( "Scale Font", actWin->fi, &scaleFm, scaleFontTag );
  ef.addColorButton( "Scale Color", actWin->ci,&scaleCb,&bufScaleColor);
  ef.addToggle( "Alarm Sensitive", &bufScaleColorMode );
  ef.addTextField("Major Intervals" ,27 , &bufMajorIntervals);
  ef.addTextField("Minor Intervals per Major Interval",27, &bufMinorIntervals);
  ef.addToggle("Embellished Needle", &bufNeedleType);  
  ef.addColorButton( "Needle Color", actWin->ci, &fgCb, &bufFgColor );
  ef.addToggle( "Alarm Sensitive", &bufFgColorMode );
  ef.addColorButton( "Case Color", actWin->ci, &meterCb, &bufMeterColor );
  ef.addToggle( "Alarm Sensitive", &bufMeterColorMode );
  ef.addColorButton( "Bg Color", actWin->ci, &bgCb, &bufBgColor );
  ef.addToggle("3D",&bufShadowMode);
  ef.addColorButton("Top Shadow",actWin->ci, &tsCb,&bufTsColor);
  ef.addColorButton("Bottom Shadow",actWin->ci,&bsCb,&bufBsColor);

  actWin->pvObj.getOptionMenuList( pvOptionList, 255, &numPvTypes );
  if ( numPvTypes == 1 ) {
    pvNameIndex= 0;
  }
  else {
    // printf( "pvUserClassName = [%s]\n", pvUserClassName );
    pvNameIndex = actWin->pvObj.getNameNum( pvUserClassName );
    if ( pvNameIndex < 0 ) pvNameIndex = 0;
    // printf( "pvOptionList = [%s]\n", pvOptionList );
    // printf( "pvNameIndex = %-d\n", pvNameIndex );
    ef.addOption( "PV Type", pvOptionList, &pvNameIndex );
  }

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
 XRectangle xR = { faceX + 1, faceY + 1, faceW -2, faceH -2 };
 double tickSize;
 int  farEndX, farEndY, nearEndX, nearEndY, i;
 char fmt[31+1];

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

     if ( meterNeedleXend != meterNeedleXorigin ) {
       XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		  actWin->executeGc.normGC(),
		  meterNeedleXend, meterNeedleYend,
		  meterNeedleXorigin+1, meterNeedleYorigin);
       XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		  actWin->executeGc.normGC(),
	      meterNeedleXend, meterNeedleYend,
		  meterNeedleXorigin-1, meterNeedleYorigin);
     }
     if ( meterNeedleYend != meterNeedleYorigin ) {
       XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		  actWin->executeGc.normGC(),
		  meterNeedleXend, meterNeedleYend,
		  meterNeedleXorigin, meterNeedleYorigin-1);
       XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		  actWin->executeGc.normGC(),
		  meterNeedleXend, meterNeedleYend,
		  meterNeedleXorigin, meterNeedleYorigin+1);
     }

     actWin->executeGc.setFG( meterColor.getColor() );
     
     XFillArc( actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(), meterNeedleXorigin-5,
	       meterNeedleYorigin-5, 10, 10, 0, 23040 );
     XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
	       actWin->executeGc.normGC(), meterNeedleXorigin-5,
	       meterNeedleYorigin-5, 10, 10, 0, 23040 );
     
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

     if ( meterNeedleXend != meterNeedleXorigin ) {
       XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		  actWin->executeGc.normGC(),
		  meterNeedleXend, meterNeedleYend,
		  meterNeedleXorigin+1, meterNeedleYorigin);
       XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		  actWin->executeGc.normGC(),
		  meterNeedleXend, meterNeedleYend,
		  meterNeedleXorigin-1, meterNeedleYorigin);
     }
     
     if ( meterNeedleYend != meterNeedleYorigin ) {
       XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		  actWin->executeGc.normGC(),
		  meterNeedleXend, meterNeedleYend,
		  meterNeedleXorigin, meterNeedleYorigin-1);
       XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
		  actWin->executeGc.normGC(),
		  meterNeedleXend, meterNeedleYend,
		  meterNeedleXorigin, meterNeedleYorigin+1);
       
       actWin->executeGc.setFG( meterColor.getColor() );
       
       XFillArc( actWin->d, XtWindow(actWin->executeWidget),
		 actWin->executeGc.normGC(), meterNeedleXorigin-5,
		 meterNeedleYorigin-5, 10, 10, 0, 23040 );
       XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
		 actWin->executeGc.normGC(), meterNeedleXorigin-5,
		 meterNeedleYorigin-5, 10, 10, 0, 23040 );
       
     }
     
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

    readEventId = NULL;
    alarmEventId = NULL;

    drawStaticFlag = 1;

    controlPvConnected = readPvConnected = active = activeInitFlag= 0;

    actWin->appCtx->proc->lock();
    activeMode = 1;
    actWin->appCtx->proc->unlock();

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

//       if ( controlExists ) {
//         stat = ca_search_and_connect( controlPvExpStr.getExpanded(),
//          &controlPvId, meter_monitor_control_connect_state, this );
//         if ( stat != ECA_NORMAL ) {
//           printf( "error from ca_search\n" );
//           opStat = 0;
//         }
//       }

      if ( readExists ) {


          // printf( "pvNameIndex = %-d\n", pvNameIndex );
          // printf( "pv class name = [%s]\n", pvClassName );
          // printf( "pvOptionList = [%s]\n", pvOptionList );

          readPvId = actWin->pvObj.createNew( pvClassName );
          if ( !readPvId ) {
            printf( "Cannot create %s object", pvClassName );
            // actWin->appCtx->postMessage( msg );
            opComplete = 1;
            return 1;
	  }
          readPvId->createEventId( &readEventId );
          readPvId->createEventId( &alarmEventId );

        stat = readPvId->searchAndConnect( &readPvExpStr,
         meter_monitor_read_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          return 0;
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

int stat;

  if ( pass == 1 ) {

    actWin->appCtx->proc->lock();

    active = 0;
    activeMode = 0;

    if ( readExists ) {

      stat = readPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = readPvId->destroyEventId ( &readEventId );
      stat = readPvId->destroyEventId ( &alarmEventId );

      delete readPvId;

      readPvId = NULL;

    }

    actWin->appCtx->proc->unlock();

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

  if ( !activeMode ) {
    actWin->remDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return;
  }

  nc = needConnectInit; needConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  v = curReadV;
  actWin->remDefExeNode( aglPtr );

  actWin->appCtx->proc->unlock();

  if ( nc ) {

    readPvConnected = 1;

    // Do we need to check for a numeric channel type here?

    // pvrGrDouble tells channel access to send a structure which
    // includes the value of the pv, the min, and the max as doubles
    // to the event callback
    stat = readPvId->getCallback( readPvId->pvrGrDouble(),
     meter_infoUpdate, (void *) this );
    if ( stat != PV_E_SUCCESS ) {
      printf( "getCallback failed\n" );
    }

    return;

  }

  if ( ni ) {


    // pvrDouble tells channel access to send the value of the pv
    // as a double to the event callback
    if ( !readEventId->eventAdded() ) {
      stat = readPvId->addEvent( readPvId->pvrDouble(), 1,
       meter_readUpdate, (void *) this, readEventId, readPvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }


    // pvrStsDouble tells channel access to send a structure which
    // includes the value of the pv as a double and all alarm information
    // to the event callback
    if ( !alarmEventId->eventAdded() ) {
      stat = readPvId->addEvent( readPvId->pvrStsDouble(), 1,
       meter_alarmUpdate, (void *) this, alarmEventId, readPvId->pveAlarm() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
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

    active = 1;
    activeInitFlag = 1;

    readV = v;
    bufInvalidate();
    eraseActive();
    drawActive();

    return;

  }

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
