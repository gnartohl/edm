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

#define __barGen_cc 1

#include "barGen.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void barc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeBarClass *baro = (activeBarClass *) client;
char fmt[31+1], str[31+1];
int l;

  baro->actWin->setChanged();

  baro->eraseSelectBoxCorners();
  baro->erase();

  baro->fgColorMode = baro->bufFgColorMode;
  if ( baro->fgColorMode == BARC_K_COLORMODE_ALARM )
    baro->fgColor.setAlarmSensitive();
  else
    baro->fgColor.setAlarmInsensitive();
  baro->fgColor.setColor( baro->bufFgColor, baro->actWin->ci );

  baro->barColorMode = baro->bufBarColorMode;
  if ( baro->barColorMode == BARC_K_COLORMODE_ALARM )
    baro->barColor.setAlarmSensitive();
  else
    baro->barColor.setAlarmInsensitive();
  baro->barColor.setColor( baro->bufBarColor, baro->actWin->ci );

  baro->bgColor.setColor( baro->bufBgColor, baro->actWin->ci );

  baro->controlPvExpStr.setRaw( baro->bufControlPvName );
  baro->readPvExpStr.setRaw( baro->bufReadPvName );

  baro->markerColorMode = baro->bufMarkerColorMode;
  if ( baro->markerColorMode == BARC_K_COLORMODE_ALARM )
    baro->markerColor.setAlarmSensitive();
  else
    baro->markerColor.setAlarmInsensitive();
  baro->markerColor.setColor( baro->bufMarkerColor, baro->actWin->ci );

  baro->markerWidth = baro->bufMarkerWidth;

  baro->markerPvExpStr.setRaw( baro->bufMarkerPvName );

  strncpy( baro->label, baro->bufLabel, 39 );

  baro->labelType = baro->bufLabelType;

  strncpy( baro->fontTag, baro->fm.currentFontTag(), 63 );
  baro->actWin->fi->loadFontTag( baro->fontTag );
  baro->fs = baro->actWin->fi->getXFontStruct( baro->fontTag );
  baro->actWin->drawGc.setFontTag( baro->fontTag, baro->actWin->fi );

  if ( baro->fs ) {
    baro->barStrLen = XTextWidth( baro->fs, "10", 2 );
  }

  baro->border = baro->bufBorder;

  strncpy( baro->scaleFormat, baro->bufScaleFormat, 2 );
  baro->showScale = baro->bufShowScale;
  baro->labelTicks = baro->bufLabelTicks;
  baro->majorTicks = baro->bufMajorTicks;
  baro->minorTicks = baro->bufMinorTicks;

  baro->x = baro->bufX;
  baro->sboxX = baro->bufX;

  baro->y = baro->bufY;
  baro->sboxY = baro->bufY;

  baro->w = baro->bufW;
  baro->sboxW = baro->bufW;

  baro->h = baro->bufH;
  baro->sboxH = baro->bufH;

  if ( baro->w >= baro->h )
    baro->horizontal = 1;
  else
    baro->horizontal = 0;

  baro->limitsFromDb = baro->bufLimitsFromDb;
  baro->efPrecision = baro->bufEfPrecision;
  baro->efReadMin = baro->bufEfReadMin;
  baro->efReadMax = baro->bufEfReadMax;
  baro->efBarOriginX = baro->bufEfBarOriginX;

  if ( baro->efPrecision.isNull() )
    baro->precision = 2;
  else
    baro->precision = baro->efPrecision.value();

  sprintf( fmt, "%%.%-d%s", baro->precision, baro->scaleFormat );

  strncpy( baro->pvUserClassName,
   baro->actWin->pvObj.getPvName(baro->pvNameIndex), 15 );

  strncpy( baro->pvClassName,
   baro->actWin->pvObj.getPvClassName(baro->pvNameIndex), 15 );

  if ( ( baro->efReadMin.isNull() ) && ( baro->efReadMax.isNull() ) ) {
    baro->readMin = 0;
    baro->readMax = 10;
  }
  else{
    baro->readMin = baro->efReadMin.value();
    baro->readMax = baro->efReadMax.value();
  }

  if ( baro->efBarOriginX.isNull() ) {
    baro->barOriginX = baro->readMin;
  }
  else {
    baro->barOriginX = baro->efBarOriginX.value();
  }

  sprintf( str, fmt, baro->readMin );
  if ( baro->fs ) {
    baro->barStrLen = XTextWidth( baro->fs, str, strlen(str) );
  }
  sprintf( str, fmt, baro->readMax );
  if ( baro->fs ) {
    l = XTextWidth( baro->fs, str, strlen(str) );
    if ( l > baro->barStrLen ) baro->barStrLen = l;
  }

  baro->updateDimensions();

  if ( baro->horizontal ) {

    if ( baro->h < baro->minH ) {
      baro->h = baro->minH;
      baro->sboxH = baro->minH;
    }

  }
  else {

    if ( baro->h < baro->minVertH ) {
      baro->h = baro->minVertH;
      baro->sboxH = baro->minVertH;
    }

  }

}

static void barc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeBarClass *baro = (activeBarClass *) client;

  barc_edit_update( w, client, call );
  baro->refresh( baro );

}

static void barc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeBarClass *baro = (activeBarClass *) client;

  barc_edit_apply ( w, client, call );
  baro->ef.popdown();
  baro->operationComplete();

}

static void barc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeBarClass *baro = (activeBarClass *) client;

  baro->ef.popdown();
  baro->operationCancel();

}

static void barc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeBarClass *baro = (activeBarClass *) client;

  baro->ef.popdown();
  baro->operationCancel();
  baro->erase();
  baro->deleteRequest = 1;
  baro->drawAll();

}

static void bar_monitor_read_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeBarClass *baro = (activeBarClass *) clientData;

  baro->actWin->appCtx->proc->lock();

  if ( !baro->activeMode ) {
    baro->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    baro->needConnectInit = 1;

  }
  else {

    baro->readPvConnected = 0;
    baro->active = 0;
    baro->barColor.setDisconnected();
    baro->bufInvalidate();
    baro->needDraw = 1;

  }

  baro->actWin->addDefExeNode( baro->aglPtr );

  baro->actWin->appCtx->proc->unlock();

}

static void bar_monitor_marker_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeBarClass *baro = (activeBarClass *) clientData;

  baro->actWin->appCtx->proc->lock();

  if ( !baro->activeMode ) {
    baro->actWin->appCtx->proc->unlock();
    return;
  }
  
  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    baro->needMarkerConnectInit = 1;
  }
  else {

    baro->markerPvConnected = 0;
    baro->active = 0;
    baro->markerColor.setDisconnected();
    baro->bufInvalidate();
    baro->needDraw = 1;

  }

  baro->actWin->addDefExeNode( baro->aglPtr );

  baro->actWin->appCtx->proc->unlock();

}

static void bar_infoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeBarClass *baro = (activeBarClass *) clientData;

  baro->actWin->appCtx->proc->lock();

  if ( !baro->activeMode ) {
    baro->actWin->appCtx->proc->unlock();
    return;
  }

  baro->curReadV = *( (double *) classPtr->getValue( args ) );

  if ( baro->limitsFromDb || baro->efReadMin.isNull() ) {
    baro->readMin = (double) classPtr->getLoOpr( args );
  }

  if ( baro->limitsFromDb || baro->efReadMax.isNull() ) {
    baro->readMax = (double) classPtr->getHiOpr( args );
  }

  if ( baro->limitsFromDb || baro->efPrecision.isNull() ) {
    baro->precision = (int) classPtr->getPrecision( args );
  }

  baro->needInfoInit = 1;

  baro->actWin->addDefExeNode( baro->aglPtr );

  baro->actWin->appCtx->proc->unlock();

}

static void bar_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeBarClass *baro = (activeBarClass *) clientData;

  baro->actWin->appCtx->proc->lock();

  if ( !baro->activeMode ) {
    baro->actWin->appCtx->proc->unlock();
    return;
  }

  baro->curReadV = *( (double *) classPtr->getValue( args ) );

  baro->needDrawCheck = 1;

  baro->actWin->addDefExeNode( baro->aglPtr );

  baro->actWin->appCtx->proc->unlock();

}

static void bar_alarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeBarClass *baro = (activeBarClass *) clientData;

  baro->actWin->appCtx->proc->lock();

  if ( !baro->activeMode ) {
    baro->actWin->appCtx->proc->unlock();
    return;
  }

  baro->fgColor.setStatus( classPtr->getStatus( args ),
   classPtr->getSeverity( args ) );
  baro->barColor.setStatus( classPtr->getStatus( args ),
   classPtr->getSeverity( args ) );

  baro->needFullDraw = 1;

  baro->actWin->addDefExeNode( baro->aglPtr );

  baro->actWin->appCtx->proc->unlock();

}

static void bar_markerInfoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeBarClass *baro = (activeBarClass *) clientData;

  baro->actWin->appCtx->proc->lock();

  if ( !baro->activeMode ) {
    baro->actWin->appCtx->proc->unlock();
    return;
  }

  // in the case of no read pv, do we need to take care of limits?

  baro->curMarkerV = *( (double *) classPtr->getValue( args ) );

  baro->needMarkerInfoInit = 1;

  baro->actWin->addDefExeNode( baro->aglPtr );

  baro->actWin->appCtx->proc->unlock();

}

static void bar_markerUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeBarClass *baro = (activeBarClass *) clientData;

  baro->actWin->appCtx->proc->lock();

  if ( !baro->activeMode ) {
    baro->actWin->appCtx->proc->unlock();
    return;
  }

  baro->curMarkerV = *( (double *) classPtr->getValue( args ) );
  
  baro->needDrawCheck = 1;

  baro->actWin->addDefExeNode( baro->aglPtr );

  baro->actWin->appCtx->proc->unlock();

}

static void bar_markerAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

activeBarClass *baro = (activeBarClass *) clientData;

  baro->actWin->appCtx->proc->lock();

  if ( !baro->activeMode ) {
    baro->actWin->appCtx->proc->unlock();
    return;
  }

  baro->markerColor.setStatus( classPtr->getStatus( args ),
   classPtr->getSeverity( args ) );

  baro->needFullDraw = 1;

  baro->actWin->addDefExeNode( baro->aglPtr );

  baro->actWin->appCtx->proc->unlock();

}

activeBarClass::activeBarClass ( void ) {

  name = new char[strlen("activeBarClass")+1];
  strcpy( name, "activeBarClass" );
  minW = 50;
  minH = 5;
  minVertW = 5;
  minVertH = 10;
  barStrLen = 10;
  strcpy( fontTag, "" );
  fs = NULL;
  strcpy( label, "" );
  activeMode = 0;

  barColorMode = BARC_K_COLORMODE_STATIC;
  fgColorMode = BARC_K_COLORMODE_STATIC;
  markerColorMode = BARC_K_COLORMODE_STATIC;
  markerWidth = 1;
  labelType = BARC_K_LITERAL;
  border = 1;
  showScale = 1;
  labelTicks = 10;
  majorTicks = 20;
  minorTicks = 2;
  barOriginX = 0.0;
  readMin = 0;
  readMax = 10;

  limitsFromDb = 1;
  efReadMin.setNull(1);
  efReadMax.setNull(1);
  efPrecision.setNull(1);
  efBarOriginX.setNull(1);
  strcpy( scaleFormat, "g" );
  precision = 3;
  strcpy( pvClassName, "" );
  strcpy( pvUserClassName, "" );

}

// copy constructor
activeBarClass::activeBarClass
 ( const activeBarClass *source ) {

activeGraphicClass *baro = (activeGraphicClass *) this;

  baro->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeBarClass")+1];
  strcpy( name, "activeBarClass" );

  barCb = source->barCb;
  fgCb = source->fgCb;
  bgCb = source->bgCb;

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );

  barColor.copy( source->barColor );
  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );
  markerColor.copy( source->markerColor );

  controlPvExpStr.copy( source->controlPvExpStr );
  readPvExpStr.copy( source->readPvExpStr );
  markerPvExpStr.copy( source->markerPvExpStr );

  strncpy( label, source->label, 39 );

  barColorMode = source->barColorMode;
  fgColorMode = source->fgColorMode;
  markerColorMode = source->markerColorMode;
  markerWidth = source->markerWidth;
  labelType = source->labelType;
  border = source->border;
  showScale = source->showScale;
  labelTicks = source->labelTicks;
  majorTicks = source->majorTicks;
  minorTicks = source->minorTicks;
  barOriginX = source->barOriginX;
  barStrLen = source->barStrLen;

  minW = 50;
  minH = 5;
  minVertW = 5;
  minVertH = 10;
  activeMode = 0;

  limitsFromDb = source->limitsFromDb;
  readMin = source->readMin;
  readMax = source->readMax;
  precision = source->precision;

  efReadMin = source->efReadMin;
  efReadMax = source->efReadMax;
  efPrecision = source->efPrecision;
  efBarOriginX = source->efBarOriginX;
  strncpy( scaleFormat, source->scaleFormat, 2 );

  strcpy( label, source->label );

  strncpy( pvClassName, source->pvClassName, 15 );
  strncpy( pvUserClassName, source->pvUserClassName, 15 );

  horizontal = source->horizontal;

  updateDimensions();

}

activeBarClass::~activeBarClass ( void ) {

/*   printf( "In activeBarClass::~activeBarClass\n" ); */

  if ( name ) delete name;

}

int activeBarClass::createInteractive (
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

  barColor.setColor( actWin->defaultFg1Color, actWin->ci );
  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColor( actWin->defaultBgColor, actWin->ci );
  markerColor.setColor( actWin->defaultFg2Color, actWin->ci );

  strcpy( fontTag, actWin->defaultCtlFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  this->draw();

  strncpy( pvUserClassName, actWin->defaultPvType, 15 );

  this->editCreate();

  return 1;

}

int activeBarClass::save (
  FILE *f )
{

int stat, r, g, b;

  fprintf( f, "%-d %-d %-d\n", BARC_MAJOR_VERSION, BARC_MINOR_VERSION,
   BARC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getRGB( barColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", barColorMode );

  actWin->ci->getRGB( fgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", fgColorMode );

  actWin->ci->getRGB( bgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( markerColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", markerColorMode );

  fprintf( f, "%-d\n", markerWidth );

  if ( controlPvExpStr.getRaw() )
    writeStringToFile( f, controlPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( readPvExpStr.getRaw() )
    writeStringToFile( f, readPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( markerPvExpStr.getRaw() )
    writeStringToFile( f, markerPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, label );

  fprintf( f, "%-d\n", labelType );

  fprintf( f, "%-d\n", showScale );

  stat = efBarOriginX.write( f );
//    fprintf( f, "%-g\n", barOriginX );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", labelTicks );
  fprintf( f, "%-d\n", majorTicks );
  fprintf( f, "%-d\n", minorTicks );

  fprintf( f, "%-d\n", border );

  fprintf( f, "%-d\n", limitsFromDb );

  stat = efPrecision.write( f );
  stat = efReadMin.write( f );
  stat = efReadMax.write( f );

  writeStringToFile( f, scaleFormat );

  writeStringToFile( f, pvClassName );

  return 1;

}

int activeBarClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, discard, l, stat;
int major, minor, release;
unsigned int pixel;
char oneName[39+1], fmt[31+1], str[31+1];
float fBarOriginX;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  fscanf( f, "%d\n", &x );
  fscanf( f, "%d\n", &y );
  fscanf( f, "%d\n", &w );
  fscanf( f, "%d\n", &h );

  this->initSelectBox();

  if ( w >= h ) {
    horizontal = 1;
  }
  else {
    horizontal = 0;
  }

  fscanf( f, "%d %d %d\n", &r, &g, &b );
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  barColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d\n", &barColorMode );

  if ( barColorMode == BARC_K_COLORMODE_ALARM )
    barColor.setAlarmSensitive();
  else
    barColor.setAlarmInsensitive();

  fscanf( f, "%d %d %d\n", &r, &g, &b );
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  fgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d\n", &fgColorMode );

  if ( fgColorMode == BARC_K_COLORMODE_ALARM )
    fgColor.setAlarmSensitive();
  else
    fgColor.setAlarmInsensitive();

  fscanf( f, "%d %d %d\n", &r, &g, &b );
  if ( ( major < 2 ) && ( minor < 2 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  bgColor.setColor( pixel, actWin->ci );

  bgColor.setAlarmInsensitive();

  if ( ( major > 1 ) || ( minor > 6 ) ) {
    fscanf( f, "%d %d %d\n", &r, &g, &b );
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    markerColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &markerColorMode );

    fscanf( f, "%d\n", &markerWidth );
  }

  if ( markerColorMode == BARC_K_COLORMODE_ALARM )
    markerColor.setAlarmSensitive();
  else
    markerColor.setAlarmInsensitive();

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  readPvExpStr.setRaw( oneName );

  if ( ( major > 1 ) || ( minor > 6 ) ) {
    readStringFromFile( oneName, 39, f ); actWin->incLine();
    markerPvExpStr.setRaw( oneName );
  }

  readStringFromFile( label, 39, f ); actWin->incLine();

  fscanf( f, "%d\n", &labelType ); actWin->incLine();

  fscanf( f, "%d\n", &showScale ); actWin->incLine();

  if ( ( major == 1 ) && ( minor == 0 ) && ( release == 0 ) ) {
    fscanf( f, "%d\n", &discard ); actWin->incLine(); // use to be useDisplayBg
  }

  if ( ( major > 1 ) || ( minor > 4 ) ) {

    stat = efBarOriginX.read( f ); actWin->incLine();
    barOriginX = efBarOriginX.value();

  }
  else {

    fscanf( f, "%g\n", &fBarOriginX ); actWin->incLine();
    barOriginX = (double) fBarOriginX;

  }

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  if ( fs ) {
    barStrLen = XTextWidth( fs, "10", 2 );
  }

  if ( ( major > 1 ) || ( minor > 0 ) ) {

    fscanf( f, "%d\n", &labelTicks ); actWin->incLine();
    fscanf( f, "%d\n", &majorTicks ); actWin->incLine();
    fscanf( f, "%d\n", &minorTicks ); actWin->incLine();

    fscanf( f, "%d\n", &border ); actWin->incLine();

  }

 if ( ( major > 1 ) || ( minor > 3 ) ) {
   fscanf( f, "%d\n", &limitsFromDb ); actWin->incLine();
  }
  else {
    limitsFromDb = 1;
  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {

    stat = efPrecision.read( f ); actWin->incLine();

    efReadMin.read( f ); actWin->incLine();

    efReadMax.read( f ); actWin->incLine();

    readStringFromFile( oneName, 39, f ); actWin->incLine();
    strncpy( scaleFormat, oneName, 1 );

    if ( limitsFromDb || efPrecision.isNull() )
      precision = 2;
    else
      precision = efPrecision.value();

    if ( ( limitsFromDb || efReadMin.isNull() ) &&
         ( limitsFromDb || efReadMax.isNull() ) ) {
      readMin = 0;
      readMax = 10;
    }
    else{
      readMin = efReadMin.value();
      readMax = efReadMax.value();
    }

  }
  else {

    efPrecision.setValue( 2 );
    precision = 2;
    readMin = 0;
    readMax = 10;

  }

  sprintf( fmt, "%%.%-d%s", precision, scaleFormat );
  sprintf( str, fmt, readMin );
  if ( fs ) {
    barStrLen = XTextWidth( fs, str, strlen(str) );
  }
  sprintf( str, fmt, readMax );
  if ( fs ) {
    l = XTextWidth( fs, str, strlen(str) );
    if ( l > barStrLen ) barStrLen = l;
  }

  readV = barOriginX;
  curReadV = barOriginX;
  updateDimensions();

  if ( ( major > 1 ) || ( minor > 3 ) ) {

    readStringFromFile( pvClassName, 15, f ); actWin->incLine();

    strncpy( pvUserClassName, actWin->pvObj.getNameFromClass( pvClassName ),
     15 );

  }

  return 1;

}

int activeBarClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( horizontal )
    strcpy( title, "Horizontal " );
  else
    strcpy( title, "Vertical " );

  ptr = actWin->obj.getNameFromClass( "activeBarClass" );
  if ( ptr )
    Strncat( title, ptr, 31 );
  else
    Strncat( title, "Unknown object", 31 );

  Strncat( title, " Properties", 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufBarColor = barColor.pixelColor();
  bufBarColorMode = barColorMode;

  bufFgColor = fgColor.pixelColor();
  bufFgColorMode = fgColorMode;

  bufBgColor = bgColor.pixelColor();
  
  bufMarkerColor = markerColor.pixelColor();
  bufMarkerColorMode = markerColorMode;
  bufMarkerWidth = markerWidth;

  strncpy( bufFontTag, fontTag, 63 );

  if ( readPvExpStr.getRaw() )
    strncpy( bufReadPvName, readPvExpStr.getRaw(), 39 );
  else
    strncpy( bufReadPvName, "", 39 );

  if ( controlPvExpStr.getRaw() )
    strncpy( bufControlPvName, controlPvExpStr.getRaw(), 39 );
  else
    strncpy( bufControlPvName, "", 39 );

  if ( markerPvExpStr.getRaw() )
    strncpy( bufMarkerPvName, markerPvExpStr.getRaw(), 39 );
  else
    strncpy( bufMarkerPvName, "", 39 );

  strncpy( bufLabel, label, 39 );

  bufLabelType = labelType;

  bufBorder = border;

  bufShowScale = showScale;
  bufLabelTicks = labelTicks;
  bufMajorTicks = majorTicks;
  bufMinorTicks = minorTicks;

  bufEfBarOriginX = efBarOriginX;

  bufLimitsFromDb = limitsFromDb;
  bufEfPrecision = efPrecision;
  bufEfReadMin = efReadMin;
  bufEfReadMax = efReadMax;
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
  ef.addTextField( "Marker PV", 27, bufMarkerPvName, 39 );
  ef.addOption( "Marker Thk", "1|2|3|4|5|6|7|8|9|10", &bufMarkerWidth );
  ef.addOption( "Label Type", "PV Name|Literal", &bufLabelType );
  ef.addTextField( "Label", 27, bufLabel, 39 );
  ef.addFontMenu( "Label Font", actWin->fi, &fm, fontTag );
  ef.addToggle( "Border", &bufBorder );
  ef.addToggle( "Show Scale", &bufShowScale );

  ef.addTextField( "Label Tick Intervals", 27, &bufLabelTicks );
  ef.addTextField( "Major Tick Intervals", 27, &bufMajorTicks );
  ef.addTextField( "Minor Tick Intervals", 27, &bufMinorTicks );

  ef.addToggle( "Scale Info From DB", &bufLimitsFromDb );
  ef.addOption( "Scale Format", "g|f|e", bufScaleFormat, 2 );
  ef.addTextField( "Scale Precision", 27, &bufEfPrecision );
  ef.addTextField( "Min Scale Value", 27, &bufEfReadMin );
  ef.addTextField( "Max Scale Value", 27, &bufEfReadMax );

  ef.addTextField( "Origin", 27, &bufEfBarOriginX );
  ef.addColorButton( "Bar Color", actWin->ci, &barCb, &bufBarColor );
  ef.addToggle( "Alarm Sensitive", &bufBarColorMode );
  ef.addColorButton( "Fg Color", actWin->ci, &fgCb, &bufFgColor );
  ef.addToggle( "Alarm Sensitive", &bufFgColorMode );
  ef.addColorButton( "Bg Color", actWin->ci, &bgCb, &bufBgColor );
  ef.addColorButton( "Marker Color", actWin->ci, &fgCb, &bufMarkerColor );
  ef.addToggle( "Alarm Sensitive", &bufMarkerColorMode );

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

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeBarClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( barc_edit_ok, barc_edit_apply, barc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeBarClass::edit ( void ) {

  this->genericEdit();
  ef.finished( barc_edit_ok, barc_edit_apply, barc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeBarClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeBarClass::eraseActive ( void ) {

  if ( !activeMode || !init ) return 1;

  actWin->executeGc.setFG( bgColor.getColor() );

  if ( bufInvalid ) {

    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );

    XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h );

    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h );

  }
  else {

//      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
//       actWin->executeGc.normGC(), oldBarX, barY, oldBarW, barH );

    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), oldBarX, barY, oldBarW, barH );

  }

  return 1;

}

void activeBarClass::drawHorzScale (
  Widget widget,
  gcClass *gc )
{

int stat, x0, y0, x1, y1;
int labelTickHeight, majorTickHeight, minorTickHeight;
int scale_len;
double dx, inc, minorDx, minorInc;
char fmt[31+1], str[31+1];

  if ( fs ) {
    gc->setFontTag( fontTag, actWin->fi );
  }

  sprintf( fmt, "%%.%-d%s", precision, scaleFormat );

  scale_len = w;

  // draw scale and annotation

  x0 = barAreaX;
  x1 = x0 + barAreaW;
  y0 = barY + barH + 3;
  y1 = y0;

  // draw axis
  XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

  labelTickHeight = (int) ( 0.6 * fontHeight );
  if ( labelTickHeight < 5 ) labelTickHeight = 5;
  majorTickHeight = (int) ( 0.8 * (float) labelTickHeight );
  minorTickHeight = (int) ( 0.5 * (float) labelTickHeight );

  // draw label ticks
  if ( labelTicks > 0 ) {

    x0 = barAreaX;
    x1 = x0;
    y0 = barY + barH + 3;
    y1 = y0 + labelTickHeight;

    dx = readMin;
    inc = ( readMax - readMin ) / labelTicks;

    if ( mode == BARC_K_MAX_GE_MIN ) {

      while ( dx < readMax - inc * 0.5 ) {

        XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

        sprintf( str, fmt, dx );
        stat = drawText( widget, gc, fs, x1, y1+2, XmALIGNMENT_CENTER, str );

        dx += inc;
        x1 = x0 = (int) rint( barAreaX + ( dx - readMin ) * barAreaW /
         ( readMax - readMin ) );

      }

    }
    else {

      while ( dx > readMax - inc * 0.5 ) {

        XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

        sprintf( str, fmt, dx );
        stat = drawText( widget, gc, fs, x1, y1+2, XmALIGNMENT_CENTER, str );

        dx += inc;
        x1 = x0 = (int) rint( barAreaX + ( dx - readMin ) * barAreaW /
         ( readMax - readMin ) );

      }

    }

    // draw last one

    x0 = barAreaX + barAreaW;
    x1 = x0;
    y0 = barY + barH + 3;
    y1 = y0 + labelTickHeight;

    XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

    dx = readMax;
    sprintf( str, fmt, dx );
    stat = drawText( widget, gc, fs, x1, y1+2, XmALIGNMENT_CENTER, str );

  }

  // draw major ticks
  if ( majorTicks > 0 ) {

    x0 = barAreaX;
    x1 = x0;
    y0 = barY + barH + 3;
    y1 = y0 + majorTickHeight;

    dx = readMin;
    inc = ( readMax - readMin ) / majorTicks;

    if ( mode == BARC_K_MAX_GE_MIN ) {

      while ( dx < readMax - inc * 0.5 ) {

        XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

        dx += inc;
        x1 = x0 = (int) rint( barAreaX + ( dx - readMin ) *
         barAreaW / ( readMax - readMin ) );

      }

    }
    else {

      while ( dx > readMax - inc * 0.5 ) {

        XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

        dx += inc;
        x1 = x0 = (int) rint( barAreaX + ( dx - readMin ) *
         barAreaW / ( readMax - readMin ) );

      }

    }

    // draw last one

    x0 = barAreaX + barAreaW;
    x1 = x0;
    y0 = barY + barH + 3;
    y1 = y0 + majorTickHeight;

    XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

  }

  // draw minor ticks
  if ( ( majorTicks > 0 ) && ( minorTicks > 0 ) ) {

    x0 = barAreaX;
    x1 = x0;
    y0 = barY + barH + 3;
    y1 = y0 + minorTickHeight;

    dx = readMin;
    inc = ( readMax - readMin ) / majorTicks;

    if ( mode == BARC_K_MAX_GE_MIN ) {

      while ( dx < readMax - inc * 0.5 ) {

        minorDx = dx;
        minorInc = inc / minorTicks;

        while ( minorDx < ( dx + inc - 1.5 * minorInc ) ) {

          minorDx += minorInc;
          x1 = x0 = (int) rint( barAreaX + ( minorDx - readMin ) *
           barAreaW / ( readMax - readMin ) );

          XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0,
           x1, y1 );

        }

        dx += inc;

      }

    }
    else {

      while ( dx > readMax - inc * 0.5 ) {

        minorDx = dx;
        minorInc = inc / minorTicks;

        while ( minorDx > ( dx + inc - 1.5 * minorInc ) ) {

          minorDx += minorInc;
          x1 = x0 = (int) rint( barAreaX + ( minorDx - readMin ) *
           barAreaW / ( readMax - readMin ) );

          XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0,
           x1, y1 );

        }

        dx += inc;

      }

    }

  }

}

void activeBarClass::drawVertScale (
  Widget widget,
  gcClass *gc )
{

int stat, x0, y0, x1, y1, ty;
int labelTickWidth, majorTickWidth, minorTickWidth;
double dy, inc, minorDy, minorInc;
char fmt[31+1], str[31+1];

  if ( fs ) {
    gc->setFontTag( fontTag, actWin->fi );
  }

  sprintf( fmt, "%%.%-d%s", precision, scaleFormat );

  // draw scale and annotation

  x0 = x1 = barAreaX - 4;
  y0 = barAreaY;
  y1 = barAreaY - barAreaH;

  // draw axis
  XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

  labelTickWidth = (int) ( 0.5 * fontHeight );
  if ( labelTickWidth < 5 ) labelTickWidth = 5;
  majorTickWidth = (int) ( 0.8 * (float) labelTickWidth );
  minorTickWidth = (int) ( 0.5 * (float) labelTickWidth );

  // draw label ticks
  if ( labelTicks > 0 ) {

    y0 = barAreaY;
    y1 = y0;
    x0 = barAreaX - 4;
    x1 = x0 - labelTickWidth;

    dy = readMin;
    inc = ( readMax - readMin ) / labelTicks;

    if ( mode == BARC_K_MAX_GE_MIN ) {

      while ( dy < readMax - inc * 0.5 ) {

        XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

        sprintf( str, fmt, dy );
        ty = y1 - (int) rint( 0.5 * fontHeight );
        stat = drawText( widget, gc, fs, x1-2, ty, XmALIGNMENT_END, str );

        dy += inc;
        y1 = y0 = (int) rint( barAreaY -
         ( dy - readMin ) * barAreaH / ( readMax - readMin ) );

      }

    }
    else {

      while ( dy > readMax - inc * 0.5 ) {

        XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

        sprintf( str, fmt, dy );
        ty = y1 - (int) rint( 0.5 * fontHeight );
        stat = drawText( widget, gc, fs, x1-2, ty, XmALIGNMENT_END, str );

        dy += inc;
        y1 = y0 = (int) rint( barAreaY -
         ( dy - readMin ) * barAreaH / ( readMax - readMin ) );

      }

    }

    // draw last one

    y0 = y1 = barAreaY - barAreaH;
    x0 = barAreaX - 4;
    x1 = x0 - labelTickWidth;

    XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

    dy = readMax;
    ty = y1 - (int) rint( 0.5 * fontHeight );
    sprintf( str, fmt, dy );
    stat = drawText( widget, gc, fs, x1-2, ty, XmALIGNMENT_END, str );

  }

  // draw major ticks
  if ( majorTicks > 0 ) {

    y0 = y1 = barAreaY;
    x0 = barAreaX - 4;
    x1 = x0 - majorTickWidth;

    dy = readMin;
    inc = ( readMax - readMin ) / majorTicks;

    if ( mode == BARC_K_MAX_GE_MIN ) {

      while ( dy < readMax - inc * 0.5 ) {

        XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

        dy += inc;
        y1 = y0 = (int) rint( barAreaY - ( dy - readMin ) *
         barAreaH / ( readMax - readMin ) );

      }

    }
    else {

      while ( dy > readMax - inc * 0.5 ) {

        XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

        dy += inc;
        y1 = y0 = (int) rint( barAreaY - ( dy - readMin ) *
         barAreaH / ( readMax - readMin ) );

      }

    }

    // draw last one

    y0 = y1 = barAreaY - barAreaH;
    x0 = barAreaX - 4;
    x1 = x0 - majorTickWidth;

    XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

  }

  // draw minor ticks
  if ( ( majorTicks > 0 ) && ( minorTicks > 0 ) ) {

    y0 = y1 = barAreaY;
    x0 = barAreaX - 4;
    x1 = x0 - minorTickWidth;

    dy = readMin;
    inc = ( readMax - readMin ) / majorTicks;

    if ( mode == BARC_K_MAX_GE_MIN ) {

      while ( dy < readMax - inc * 0.5 ) {

        minorDy = dy;
        minorInc = inc / minorTicks;

        while ( minorDy < ( dy + inc - 1.5 * minorInc ) ) {

          minorDy += minorInc;
          y1 = y0 = (int) rint( barAreaY - ( minorDy - readMin ) *
           barAreaH / ( readMax - readMin ) );

          XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0,
           x1, y1 );

        }

        dy += inc;

      }

    }
    else {

      while ( dy > readMax - inc * 0.5 ) {

        minorDy = dy;
        minorInc = inc / minorTicks;

        while ( minorDy > ( dy + inc - 1.5 * minorInc ) ) {

          minorDy += minorInc;
          y1 = y0 = (int) rint( barAreaY - ( minorDy - readMin ) *
           barAreaH / ( readMax - readMin ) );

          XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0,
           x1, y1 );

        }

        dy += inc;

      }

    }

  }

  // draw line along origin
  if ( border || showScale )
    x0 = barAreaX - 4;
  else
    x0 = x;
  x1 = x + w;
  dy = barOriginX;
  y1 = y0 = (int) rint( barAreaY -
   ( dy - readMin ) * barAreaH / ( readMax - readMin ) );
  XDrawLine( actWin->d, XtWindow(widget), gc->normGC(), x0, y0, x1, y1 );

}

void activeBarClass::drawScale (
  Widget widget,
  gcClass *gc )
{

  if ( horizontal )
    drawHorzScale( widget, gc );
  else
    drawVertScale( widget, gc );

}

int activeBarClass::draw ( void ) {

int tX, tY;

  if ( deleteRequest ) return 1;

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  actWin->drawGc.saveFg();

  if ( horizontal ) {

    actWin->drawGc.setFG( bgColor.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

//      XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
//       actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( barColor.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), barAreaX, barY, barAreaW, barH );

//      XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
//       actWin->drawGc.normGC(), barAreaX, barY, barAreaW, barH );

    actWin->drawGc.setFG( fgColor.pixelColor() );

    if ( showScale ) drawScale( actWin->drawWidget, &actWin->drawGc );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    if ( strcmp( label, "" ) != 0 ) {
      if ( fs ) {
        tX = barAreaX;
        tY = y + 2;
        if ( border ) tY += 2;
        drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
         XmALIGNMENT_BEGINNING, label );
      }
    }

  }
  else { // vertical

    actWin->drawGc.setFG( bgColor.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( barColor.pixelColor() );

    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), barAreaX, barAreaY-barAreaH,
     barAreaW, barAreaH );

    actWin->drawGc.setFG( fgColor.pixelColor() );

    if ( showScale ) drawScale( actWin->drawWidget, &actWin->drawGc );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, h );

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeBarClass::drawActive ( void ) {

int tX, tY, x0, y0, x1, y1;
char str[39+1];

  if ( !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  if ( horizontal ) {

    if ( bufInvalid ) {

      actWin->executeGc.setFG( bgColor.getColor() );

      XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      actWin->executeGc.setFG( barColor.getColor() );

      XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), barX, barY, barW, barH );

//        XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
//         actWin->executeGc.normGC(), barX, barY, barW, barH );

    }
    else {

      if ( zeroCrossover ) {

//    printf( "zc\n" );

        actWin->executeGc.setFG( bgColor.getColor() );

        XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
         actWin->executeGc.normGC(), oldBarX, barY, oldBarW, barH );

//          XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
//           actWin->executeGc.normGC(), oldBarX, barY, oldBarW, barH );

        actWin->executeGc.setFG( barColor.getColor() );

        XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
         actWin->executeGc.normGC(), barX, barY, barW, barH );

//          XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
//           actWin->executeGc.normGC(), barX, barY, barW, barH );

      }
      else {

        if ( aboveBarOrigin ) {

//    printf( "above\n" );

          if ( barW > oldBarW ) {

//    printf( "barW > oldBarW\n" );

            actWin->executeGc.setFG( barColor.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), oldBarX+oldBarW, barY,
             barW-oldBarW, barH );

//              XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
//               actWin->executeGc.normGC(), oldBarX+oldBarW, barY,
//               barW-oldBarW, barH );

          }
          else {

//    printf( "barW <= oldBarW\n" );

            actWin->executeGc.setFG( bgColor.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX+barW, barY,
             oldBarW-barW, barH );

//              XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
//               actWin->executeGc.normGC(), barX+barW, barY,
//               oldBarW-barW, barH );

          }

        }
        else {

//    printf( "below\n" );

          if ( barX < oldBarX ) {

            actWin->executeGc.setFG( barColor.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY,
             oldBarX-barX, barH );

//              XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
//               actWin->executeGc.normGC(), barX, barY,
//               oldBarX-barX, barH );

          }
          else {

            actWin->executeGc.setFG( bgColor.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), oldBarX, barY,
             barX-oldBarX, barH );

//              XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
//               actWin->executeGc.normGC(), oldBarX, barY,
//               barX-oldBarX, barH );

          }

        }

      }

    }

    oldBarX = barX;
    oldBarW = barW;

  }
  else { // vertical

    if ( bufInvalid ) {

      actWin->executeGc.setFG( bgColor.getColor() );

      XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );

      actWin->executeGc.setFG( barColor.getColor() );

      XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), barX, barY-barH, barW, barH );

      // draw line along origin
      if ( border || showScale )
        x0 = barAreaX - 4;
      else
        x0 = x;
      x1 = x + w;
      y1 = y0 = (int) rint( barAreaY -
       ( barOriginX - readMin ) * barAreaH / ( readMax - readMin ) );
      XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x0, y0, x1, y1 );

    }
    else {

      if ( zeroCrossover ) {

//    printf( "zc\n" );

        actWin->executeGc.setFG( bgColor.getColor() );

        XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
         actWin->executeGc.normGC(), barX, oldBarY-oldBarH, barW, oldBarH );

        actWin->executeGc.setFG( barColor.getColor() );

        XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
         actWin->executeGc.normGC(), barX, barY-barH, barW, barH );

        // draw line along origin
        if ( border || showScale )
          x0 = barAreaX - 4;
        else
          x0 = x;
        x1 = x + w;
        y1 = y0 = (int) rint( barAreaY -
         ( barOriginX - readMin ) * barAreaH / ( readMax - readMin ) );
        XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
         actWin->executeGc.normGC(), x0, y0, x1, y1 );

      }
      else {

        if ( aboveBarOrigin ) {

//    printf( "above\n" );

          if ( barH > oldBarH ) {

//    printf( "barH > oldBarH\n" );

            actWin->executeGc.setFG( barColor.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY-barH,
             barW, barH-oldBarH );

          }
          else {

//    printf( "barW <= oldBarW\n" );

            actWin->executeGc.setFG( bgColor.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY-oldBarH,
             barW, oldBarH-barH );

          }

        }
        else {

//    printf( "below\n" );

          if ( barY > oldBarY ) {

            actWin->executeGc.setFG( barColor.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, oldBarY,
             barW, barY-oldBarY );

          }
          else {

            actWin->executeGc.setFG( bgColor.getColor() );

            XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
             actWin->executeGc.normGC(), barX, barY,
             barW, oldBarY-barY );

          }

        }

      }

    }

    oldBarY = barY;
    oldBarH = barH;

  }

  // draw the marker line
  
  if (markerExists) {
    drawMarker();
  }
  
  if ( bufInvalid ) { // draw scale, label, etc ...

    actWin->executeGc.setFG( fgColor.pixelColor() );

    if ( showScale ) {
      drawScale( actWin->executeWidget, &actWin->executeGc );
    }

    if ( labelType == BARC_K_PV_NAME )
      strncpy( str, readPvExpStr.getRaw(), 39 );
      // strncpy( str, ca_name(readPvId), 39 );
    else
      strncpy( str, label, 39 );

    if ( horizontal ) {

      if ( strcmp( str, "" ) != 0 ) {
        actWin->executeGc.setFontTag( fontTag, actWin->fi );
        if ( fs ) {
          tX = barAreaX;
          tY = y + 2;
          if ( border ) tY += 2;
          drawText( actWin->executeWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_BEGINNING, str );
        }
      }

    }
    else {

      // no label for a vertical bar

    }

    if ( border ) {
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
    }

    bufInvalid = 0;

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int activeBarClass::drawMarker ( void ) {

  actWin->executeGc.setLineWidth( markerWidth );

  if ( horizontal ) {

    if ( bufInvalid ) {

      // don't need to erase, since bar was just redrawn

      actWin->executeGc.setFG( markerColor.getColor() );

      XDrawLine( actWin->d, XtWindow(actWin->executeWidget), 
       actWin->executeGc.normGC(), markerX1, markerY1, markerX2, markerY2 );

    }
    else  {

      // erase the old marker
      if ( (oldMarkerX1 < barX) || (oldMarkerX2 > (barX + barW)) ) {
        actWin->executeGc.setFG( bgColor.getColor() );
      }
      else {
        actWin->executeGc.setFG( barColor.getColor() );
      }
      XDrawLine( actWin->d, XtWindow(actWin->executeWidget), 
       actWin->executeGc.normGC(), 
       oldMarkerX1, oldMarkerY1, oldMarkerX2, oldMarkerY2 );

      // draw the current marker

      actWin->executeGc.setFG( markerColor.getColor() );
      XDrawLine( actWin->d, XtWindow(actWin->executeWidget), 
       actWin->executeGc.normGC(), 
       markerX1, markerY1, markerX2, markerY2 );
      
    }
  } 
  else { // vertical

    if ( bufInvalid ) {

      // don't need to erase, since bar was just redrawn

      actWin->executeGc.setFG( markerColor.getColor() );

      XDrawLine( actWin->d, XtWindow(actWin->executeWidget), 
       actWin->executeGc.normGC(), markerX1, markerY1, markerX2, markerY2 );

    }
    else  {

      // erase the old marker
      if ( (oldMarkerY1 < (barY - barH)) || (oldMarkerY2 > barY) ) {
        actWin->executeGc.setFG( bgColor.getColor() );
      }
      else {
        actWin->executeGc.setFG( barColor.getColor() );
      }
      XDrawLine( actWin->d, XtWindow(actWin->executeWidget), 
       actWin->executeGc.normGC(), 
       oldMarkerX1, oldMarkerY1, oldMarkerX2, oldMarkerY2 );

      // draw the current marker

      actWin->executeGc.setFG( markerColor.getColor() );
      XDrawLine( actWin->d, XtWindow(actWin->executeWidget), 
       actWin->executeGc.normGC(), 
       markerX1, markerY1, markerX2, markerY2 );
      
    }
  }

  actWin->executeGc.setLineWidth( 1 ); // so we don't have side effects

  oldMarkerX1 = markerX1;
  oldMarkerY1 = markerY1;
  oldMarkerX2 = markerX2;
  oldMarkerY2 = markerY2;
  
  return 1;
}

int activeBarClass::activate (
  int pass,
  void *ptr )
{

int stat, opStat;

  switch ( pass ) {

  case 1:

    zeroCrossover = 0;
    oldAboveBarOrigin = 0;
    needConnectInit = needInfoInit = needMarkerConnectInit = needRefresh = needErase =
     needDrawCheck = needDraw = 0;
    aglPtr = ptr;
    opComplete = 0;

    if ( horizontal ) {
      barW = 0;
      oldBarW = 0;
      barX = 0;
      oldBarX = 0;
    }
    else {
      barH = 0;
      oldBarH = 0;
      barY = 0;
      oldBarY = 0;
    }

    controlEventId = NULL;
    readEventId = NULL;
    alarmEventId = NULL;
    markerEventId = NULL;
    markerAlarmEventId = NULL;

    controlPvConnected = readPvConnected = markerPvConnected = active = init = 0;

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
      barColor.setConnectSensitive();
    }

    if ( !markerPvExpStr.getExpanded() ||
       ( strcmp( markerPvExpStr.getExpanded(), "" ) == 0 ) ) {
      markerExists = 0;
    }
    else {
      markerExists = 1;
      markerColor.setConnectSensitive();
    }


    break;

  case 2:

    if ( !opComplete ) {

      opStat = 1;

//       if ( controlExists ) {
//         stat = ca_search_and_connect( controlPvExpStr.getExpanded(),
//          &controlPvId, bar_monitor_control_connect_state, this );
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
         bar_monitor_read_connect_state, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect\n" );
          return 0;
        }

      }

      if ( markerExists ) {

          // printf( "pvNameIndex = %-d\n", pvNameIndex );
          // printf( "pv class name = [%s]\n", pvClassName );
          // printf( "pvOptionList = [%s]\n", pvOptionList );

          markerPvId = actWin->pvObj.createNew( pvClassName );
          if ( !markerPvId ) {
            printf( "Cannot create %s object", pvClassName );
            // actWin->appCtx->postMessage( msg );
            opComplete = 1;
            return 1;
	  }
          markerPvId->createEventId( &markerEventId );
          markerPvId->createEventId( &markerAlarmEventId );

        stat = markerPvId->searchAndConnect( &markerPvExpStr,
         bar_monitor_marker_connect_state, this );
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

int activeBarClass::deactivate (
  int pass
) {

int stat;

  active = 0;

  if ( pass == 1 ) {

    actWin->appCtx->proc->lock();

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

    if ( markerExists ) {

      stat = markerPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = markerPvId->destroyEventId ( &markerEventId );
      stat = markerPvId->destroyEventId ( &markerAlarmEventId );

      delete markerPvId;

      markerPvId = NULL;

    }

    actWin->appCtx->proc->unlock();

  }

  return 1;

}

void activeBarClass::updateDimensions ( void )
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

  barAreaX = x;
  barAreaY = y;
  barAreaW = w;
  barAreaH = h;

  if ( horizontal ) {

    minH = 5;
    barY = y;

    barAreaX = x;
    barAreaW = w;

    if ( ( strcmp( label, "" ) != 0 ) ||
         ( labelType == BARC_K_PV_NAME ) ) {
      minH += fontHeight + 5;
      barY += fontHeight + 5;
      if ( border ) {
        minH += 9;
        barY += 5;
        barAreaX = x + 5;
        barAreaW = w - 9;
      }
    }
    else {
      if ( border && showScale ) {
        minH += 9;
        barY += 5;
      }
    }

    if ( showScale ) {
      minH += fontHeight + fontHeight + 5;
      barAreaX = x + barStrLen/2 + 3;
      barAreaW = w - barStrLen - 6;
    }

    if ( border && !showScale && ( ( strcmp( label, "" ) == 0 ) ||
     ( labelType == BARC_K_PV_NAME ) ) ) {
      minH += 9;
      barY += 5;
      barAreaX = x + 5;
      barAreaW = w - 9;
    }

    if ( h < minH ) {

      h = minH;
      sboxH = minH;

    }

    barH = h;

    if ( ( strcmp( label, "" ) != 0 ) ||
         ( labelType == BARC_K_PV_NAME ) ) {
      barH -= ( fontHeight + 5 );
      if ( border ) barH -= 9;
    }

    if ( showScale ) {
      barH -= ( fontHeight + fontHeight + 5 );
    }

    if ( border && !showScale && ( ( strcmp( label, "" ) == 0 ) ||
     ( labelType == BARC_K_PV_NAME ) ) ) {
      barH -= 9;
    }

  }
  else {  // vertical

    minVertW = 5;
    minVertH = 10;

    if ( showScale ) {
      minVertH += fontHeight;
      minVertW += 4 + barStrLen + 10 + (int) rint( 0.5 * fontHeight );
    }
    else if ( border ) {
      minVertH += 8;
      minVertW += 4;
    }

//      if ( showScale ) {
//        minVertW += barStrLen + 10 + (int) rint( 0.5 * fontHeight );
//      }

    if ( w < minVertW ) {

      w = minVertW;
      sboxW = minVertW;

    }

    if ( h < minVertH ) {

      h = minVertH;
      sboxH = minVertH;

    }

    barH = barAreaH = h;
    barY = barAreaY = y + barAreaH;
    barX = barAreaX = x;
    barW = barAreaW = w;

    if ( showScale ) {
      barH -= ( fontHeight );
      barAreaH -= ( fontHeight );
    }
    else if ( border ) {
      barH -= 8;
      barAreaH -= 8;
    }

    if ( showScale ) {
      barY -= (int) rint( 0.5 * fontHeight );
      barAreaY -= (int) rint( 0.5 * fontHeight );
      barAreaW -= ( 4 +  barStrLen + 8 + (int) rint( 0.5 * fontHeight ) );
      barW -= ( 4 + barStrLen + 8 + (int) rint( 0.5 * fontHeight ) );
      barAreaX += 2 + barStrLen + 8 + (int) rint( 0.5 * fontHeight );
      barX += 2 + barStrLen + 8 + (int) rint( 0.5 * fontHeight );
    }
    else if ( border ) {
      barY -= 4;
      barAreaY -= 4;
      barAreaW -= 9;
      barW -= 9;
      barAreaX += 5;
      barX += 5;
    }

  }

  updateScaleInfo();

}

void activeBarClass::btnUp (
  int x,
  int y,
  int barState,
  int barNumber )
{

}

void activeBarClass::btnDown (
  int x,
  int y,
  int barState,
  int barNumber )
{

}

void activeBarClass::btnDrag (
  int x,
  int y,
  int barState,
  int barNumber )
{

}

int activeBarClass::getBarActionRequest (
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

void activeBarClass::bufInvalidate ( void )
{

  bufInvalid = 1;

}

int activeBarClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = readPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeBarClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeBarClass::containsMacros ( void ) {

int stat;

  stat = readPvExpStr.containsPrimaryMacros();

  return stat;

}

int activeBarClass::checkResizeSelectBox (
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

int activeBarClass::checkResizeSelectBoxAbs (
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

void activeBarClass::updateScaleInfo ( void ) {

  if ( horizontal )
    updateHorzScaleInfo();
  else
    updateVertScaleInfo();

}

void activeBarClass::updateHorzScaleInfo ( void ) {

int locW;

  if ( readMax == readMin ) readMax = readMin + 1.0;

  if ( readMax >= readMin ) {

    mode = BARC_K_MAX_GE_MIN;

    if ( barOriginX < readMin )
      barOriginX = readMin;
    else if ( barOriginX > readMax )
     barOriginX = readMax;

    originW = (int) rint( ( barOriginX - readMin ) *
     barAreaW / ( readMax - readMin ) );

    if ( readV > readMax ) readV = readMax;
    if ( readV < readMin ) readV = readMin;

  }
  else {

    mode = BARC_K_MAX_LT_MIN;

    if ( barOriginX > readMin )
      barOriginX = readMin;
    else if ( barOriginX < readMax )
     barOriginX = readMax;

    originW = (int) rint( ( barOriginX - readMin ) *
     barAreaW / ( readMax - readMin ) );

    if ( readV < readMax ) readV = readMax;
    if ( readV > readMin ) readV = readMin;

  }

  switch ( mode ) {

  case BARC_K_MAX_GE_MIN:

    if ( readV >= barOriginX ) {

      barX = originW;

      if ( barOriginX == readMax ) {

        barW = 0;
        factorGe = 0;

      }
      else {

        barW = (int) ( ( barAreaW - originW ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barW > ( barAreaW - originW ) )
          barW = barAreaW - originW;

        factorGe = ( barAreaW - originW ) / ( readMax - barOriginX );
        barMaxW = barAreaW - originW;

      }

    }
    else {

      if ( barOriginX == readMin ) {

        locW = 0;
        factorLt = 0;

      }
      else {

        locW = (int) ( ( originW ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorLt = originW / ( readMin - barOriginX );

      }

      barX = originW - locW;

      barW = abs( locW );

      if ( barX < 0 ) {
        barX = 0;
        barW = originW;
      }

    }

    break;

  case BARC_K_MAX_LT_MIN:

    if ( readV < barOriginX ) {

      barX = originW;

      if ( barOriginX == readMax ) {

        barW = 0;
        factorLt = 0;

      }
      else {

        barW = (int) ( ( barAreaW - originW ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barW > ( barAreaW - originW ) )
          barW = barAreaW - originW;

        factorLt = ( barAreaW - originW ) / ( readMax - barOriginX );

      }

      barMaxW = barAreaW - originW;

    }
    else {

      if ( barOriginX == readMin ) {

        locW = 0;
        factorGe = 0;

      }
      else {

        locW = (int) ( ( originW ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorGe = originW / ( readMin - barOriginX );

      }

      barX = originW - locW;

      barW = abs( locW );

      if ( barX < 0 ) {
        barX = 0;
        barW = originW;
      }

    }

    break;

  }

  barX += barAreaX;

}

void activeBarClass::updateVertScaleInfo ( void ) {

int locH;

  if ( readMax == readMin ) readMax = readMin + 1.0;

  if ( readMax >= readMin ) {

    mode = BARC_K_MAX_GE_MIN;

    if ( barOriginX < readMin )
      barOriginX = readMin;
    else if ( barOriginX > readMax )
     barOriginX = readMax;

    originH = (int) rint( ( barOriginX - readMin ) *
     barAreaH / ( readMax - readMin ) );

    if ( readV > readMax ) readV = readMax;
    if ( readV < readMin ) readV = readMin;

  }
  else {

    mode = BARC_K_MAX_LT_MIN;

    if ( barOriginX > readMin )
      barOriginX = readMin;
    else if ( barOriginX < readMax )
     barOriginX = readMax;

    originH = (int) rint( ( barOriginX - readMin ) *
     barAreaH / ( readMax - readMin ) );

    if ( readV < readMax ) readV = readMax;
    if ( readV > readMin ) readV = readMin;

  }

  switch ( mode ) {

  case BARC_K_MAX_GE_MIN:

    if ( readV >= barOriginX ) {

      barY = barAreaY - originH;

      if ( barOriginX == readMax ) {

        barH = 0;
        factorGe = 0;

      }
      else {

        barH = (int) ( ( barAreaH - originH ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barH > ( barAreaH - originH ) )
          barH = barAreaH - originH;

        factorGe = ( barAreaH - originH ) / ( readMax - barOriginX );
        barMaxH = barAreaH - originH;

      }

    }
    else {

      if ( barOriginX == readMin ) {

        locH = 0;
        factorLt = 0;

      }
      else {

        locH = (int) ( ( originH ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorLt = originH / ( readMin - barOriginX );

      }

      barY = barAreaY - ( originH - locH );

      barH = abs( locH );

      if ( barY < 0 ) {
        barY = 0;
        barH = originH;
      }

    }

    break;

  case BARC_K_MAX_LT_MIN:

    if ( readV < barOriginX ) {

      barY = barAreaY - originH;

      if ( barOriginX == readMax ) {

        barH = 0;
        factorLt = 0;

      }
      else {

        barH = (int) ( ( barAreaH - originH ) *
         ( readV - barOriginX ) /
         ( readMax - barOriginX ) + 0.5 );

        if ( barH > ( barAreaH - originH ) )
          barH = barAreaH - originH;

        factorLt = ( barAreaH - originH ) / ( readMax - barOriginX );

      }

      barMaxH = barAreaH - originH;

    }
    else {

      if ( barOriginX == readMin ) {

        locH = 0;
        factorGe = 0;

      }
      else {

        locH = (int) ( ( originH ) *
         ( readV - barOriginX ) /
         ( readMin - barOriginX ) + 0.5 );

        factorGe = originH / ( readMin - barOriginX );

      }

      barY = barAreaY - ( originH - locH );

      barH = abs( locH );

      if ( barY < 0 ) {
        barY = 0;
        barH = originH;
      }

    }

    break;

  }

  //  barY += barAreaY;

}

void activeBarClass::updateBar ( void ) {

int locW, locH;

  if ( horizontal ) {

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX ) {

        aboveBarOrigin = 1;

      }
      else {

        aboveBarOrigin = 0;

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX ) {

        aboveBarOrigin = 1;

      }
      else {

        aboveBarOrigin = 0;

      }

      break;

    }

    if ( aboveBarOrigin != oldAboveBarOrigin ) {
      oldAboveBarOrigin = aboveBarOrigin;
      zeroCrossover = 1;
      updateScaleInfo();
    }
    else {
      zeroCrossover = 0;
    }

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX ) {

        barX = originW;

        barW = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );

        if ( barW > barMaxW ) barW = barMaxW;

      }
      else {

        locW = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );

        barX = originW - locW;

        barW = abs( locW );

        if ( barX < 0 ) {
          barX = 0;
          barW = originW;
        }

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX ) {

        barX = originW;

        barW = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );

        if ( barW > barMaxW ) barW = barMaxW;

      }
      else {

        locW = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );

        barX = originW - locW;

        barW = abs( locW );

        if ( barX < 0 ) {
          barX = 0;
          barW = originW;
        }

      }

      break;

    }

    barX += barAreaX;

  }
  else { // vertical

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX ) {

        aboveBarOrigin = 1;

      }
      else {

        aboveBarOrigin = 0;

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX ) {

        aboveBarOrigin = 1;

      }
      else {

        aboveBarOrigin = 0;

      }

      break;

    }

    if ( aboveBarOrigin != oldAboveBarOrigin ) {
      oldAboveBarOrigin = aboveBarOrigin;
      zeroCrossover = 1;
      updateScaleInfo();
    }
    else {
      zeroCrossover = 0;
    }

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( readV >= barOriginX ) {

        barY = barAreaY - originH;

        barH = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );

        if ( barH > barMaxH ) barH = barMaxH;

      }
      else {

        locH = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );

        barY = barAreaY - ( originH - locH );

        barH = abs( locH );

        if ( barY > barAreaY ) {
          barY = barAreaY;
          barH = originH;
        }

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( readV < barOriginX ) {

        barY = barAreaY - originH;

        barH = (int) ( ( readV - barOriginX ) * factorLt + 0.5 );

        if ( barH > barMaxH ) barH = barMaxH;

      }
      else {

        locH = (int) ( factorGe * ( readV - barOriginX ) + 0.5 );

        barY = barAreaY - ( originH - locH );

        barH = abs( locH );

        if ( barY > barAreaY ) {
          barY = barAreaY;
          barH = originH;
        }

      }

      break;

    }

//    barY += barAreaY;

  }

}

void activeBarClass::updateMarker ( void ) {

int markerW, markerH, markerAboveOrigin;
double locFactorLt, locFactorGe;
int locBarMax;

  if ( horizontal ) {

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( markerV >= barOriginX ) {

        markerAboveOrigin = 1;

      }
      else {

        markerAboveOrigin = 0;

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( markerV < barOriginX ) {

        markerAboveOrigin = 1;

      }
      else {

        markerAboveOrigin = 0;

      }

      break;

    }

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( markerV >= barOriginX ) {
 
        locFactorGe = ( barAreaW - originW ) / ( readMax - barOriginX );
        locBarMax = barAreaW - originW;

        markerW = (int) ( locFactorGe * ( markerV - barOriginX ) + 0.5 );
	
        if ( markerW > locBarMax ) {
          markerW = locBarMax;
        }

        markerX2 = markerX1 = originW + markerW;
      }
      else {

        locFactorLt = originW / ( readMin - barOriginX );

        markerW = (int) ( ( markerV - barOriginX ) * locFactorLt + 0.5 );	

        markerX2 = markerX1 = originW - markerW;

        if ( markerX1 < 0 ) {
          markerX2 = markerX1 = 0;
         }

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( markerV < barOriginX ) {

        locFactorLt = ( barAreaW - originW ) / ( readMax - barOriginX );
        locBarMax = barAreaW - originW;

        markerW = (int) ( ( markerV - barOriginX ) * locFactorLt + 0.5 );

        if ( markerW > locBarMax ) {
	  markerW = locBarMax;
	}

        markerX2 = markerX1 = originW + markerW;
        
      }
      else {

        locFactorGe = originW / ( readMin - barOriginX );

        markerW = (int) ( locFactorGe * ( markerV - barOriginX ) + 0.5 );

        markerX2 = markerX1 = originW - markerW;

        if ( markerX1 < 0 ) {
          markerX2 = markerX1 = 0;
        }

      }

      break;

    }

    markerY1 = barY;
    markerY2 = barY + barH;
    if ( border || showScale ) {
      markerY2--;
    }
    markerX1 += barAreaX;
    markerX2 += barAreaX;

  }
  else { // vertical

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if (markerV >= barOriginX ) {

        markerAboveOrigin = 1;

      }
      else {

        markerAboveOrigin = 0;

      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( markerV < barOriginX ) {

        markerAboveOrigin = 1;

      }
      else {

        markerAboveOrigin = 0;

      }

      break;

    }

    switch ( mode ) {

    case BARC_K_MAX_GE_MIN:

      if ( markerV >= barOriginX ) {
      
        locFactorGe = ( barAreaH - originH ) / ( readMax - barOriginX );
        locBarMax = barAreaH - originH;

        markerH = (int) ( locFactorGe * ( markerV - barOriginX ) + 0.5 );

        if ( markerH > locBarMax ) markerH = locBarMax;

        markerY2 = markerY1 = barAreaY - (originH + markerH);

      }
      else {

        locFactorLt = originH / ( readMin - barOriginX );

        markerH = (int) ( ( markerV - barOriginX ) * locFactorLt + 0.5 );

        markerY1 = barAreaY - ( originH - markerH );

        if ( markerY1 > barAreaY ) {
          markerY1 = barAreaY;
        }
        markerY2 = markerY1;
      }

      break;

    case BARC_K_MAX_LT_MIN:

      if ( markerV < barOriginX ) {

        locFactorLt = ( barAreaH - originH ) / ( readMax - barOriginX );
        locBarMax = barAreaH - originH;

        markerH = (int) ( ( markerV - barOriginX ) * locFactorLt + 0.5 );

        if ( markerH > locBarMax ) {
	  markerH = locBarMax;
	}
 
        markerY2 = markerY1 = barAreaY - (originH + markerH);

      }
      else {

        locFactorGe = originH / ( readMin - barOriginX );

        markerH = (int) ( locFactorGe * ( markerV - barOriginX ) + 0.5 );

        markerY1 = barAreaY - ( originH - markerH );

        if ( markerY1 > barAreaY ) {
          markerY1 = barAreaY;
        }
        markerY2 = markerY1;
      }

      break;

    }

 
//  barY += barAreaY;
    markerX1 = barX;
    markerX2 = barX + barW;

  }

}


void activeBarClass::executeDeferred ( void ) {

int stat, l, nc, nmc, ni, nmi, nr, ne, nd, nfd, ndc;
char fmt[31+1], str[31+1];
double v, mv;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  if ( !activeMode ) {
    actWin->remDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return;
  }

  nc = needConnectInit; needConnectInit = 0;
  nmc = needMarkerConnectInit; needMarkerConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nmi = needMarkerInfoInit; needMarkerInfoInit = 0;
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nfd = needFullDraw; needFullDraw = 0;
  ndc = needDrawCheck; needDrawCheck = 0;
  v = curReadV;
  mv = curMarkerV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

//----------------------------------------------------------------------------

  if ( nc ) {

    // Do we need to check for a numeric channel type here?

    // pvrGrDouble tells channel access to send a structure which
    // includes the value of the pv, the min, and the max as doubles
    // to the event callback
    stat = readPvId->getCallback( readPvId->pvrGrDouble(),
     bar_infoUpdate, (void *) this );
    if ( stat != PV_E_SUCCESS ) {
      printf( "getCallback failed\n" );
    }
    
      // return;
  }

//----------------------------------------------------------------------------

  if ( nmc ) {

    // pvrGrDouble tells channel access to send a structure which
    // includes the value of the pv, the min, and the max as doubles
    // to the event callback
    stat = markerPvId->getCallback( markerPvId->pvrGrDouble(),
     bar_markerInfoUpdate, (void *) this );
    if ( stat != PV_E_SUCCESS ) {
      printf( "getCallback failed\n" );
    }

    // return;

  }

//----------------------------------------------------------------------------

  if ( ni ) {

    readPvConnected = 1;

    if ( efBarOriginX.isNull() ) {
      barOriginX = readMin;
    }

    sprintf( fmt, "%%.%-d%s", precision, scaleFormat );

    sprintf( str, fmt, readMin );
    if ( fs ) {
      barStrLen = XTextWidth( fs, str, strlen(str) );
    }

    sprintf( str, fmt, readMax );
    if ( fs ) {
      l = XTextWidth( fs, str, strlen(str) );
      if ( l > barStrLen ) barStrLen = l;
    }

    updateDimensions();

    active = 1;
    init = 1;
    barColor.setConnected();
    bufInvalidate();
    eraseActive();
    readV = v;
    updateDimensions();
    drawActive();


    // pvrDouble tells channel access to send the value of the pv
    // as a double to the event callback
    if ( !readEventId->eventAdded() ) {
      stat = readPvId->addEvent( readPvId->pvrDouble(), 1,
       bar_readUpdate, (void *) this, readEventId, readPvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }


    // pvrStsDouble tells channel access to send a structure which
    // includes the value of the pv as a double and all alarm information
    // to the event callback
    if ( !alarmEventId->eventAdded() ) {
      stat = readPvId->addEvent( readPvId->pvrStsDouble(), 1,
       bar_alarmUpdate, (void *) this, alarmEventId, readPvId->pveAlarm() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }

    // return;

  }

//----------------------------------------------------------------------------

  if ( nmi ) {

    markerPvConnected = 1;

    markerColor.setConnected();
    markerV = mv;

    // pvrDouble tells channel access to send the value of the pv
    // as a double to the event callback
    if ( !markerEventId->eventAdded() ) {
      stat = markerPvId->addEvent( markerPvId->pvrDouble(), 1,
       bar_markerUpdate, (void *) this, markerEventId, markerPvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }

    // pvrStsDouble tells channel access to send a structure which
    // includes the value of the pv as a double and all alarm information
    // to the event callback
    if ( !markerAlarmEventId->eventAdded() ) {
      stat = markerPvId->addEvent( markerPvId->pvrStsDouble(), 1,
       bar_markerAlarmUpdate, (void *) this, markerAlarmEventId, 
       markerPvId->pveAlarm() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }

    // return;

  }

//----------------------------------------------------------------------------

  if ( nr ) {

    bufInvalidate();

    eraseActive();

    readV = v;

    updateDimensions();

    drawActive();

  }

//----------------------------------------------------------------------------

  if ( ne ) {
    eraseActive();
  }

//----------------------------------------------------------------------------

  if ( nd ) {
    readV = v;
    drawActive();
  }

//----------------------------------------------------------------------------

  if ( nfd ) {
    readV = v;
    bufInvalidate();
    drawActive();
  }

//----------------------------------------------------------------------------

  if ( ndc ) {

      readV = v;
      updateBar();
      if (markerExists) {
        markerV = mv;
        updateMarker();
      }
      
      drawActive(); // also executes drawMarker();

  }

//----------------------------------------------------------------------------

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeBarClassPtr ( void ) {

activeBarClass *ptr;

  ptr = new activeBarClass;
  return (void *) ptr;

}

void *clone_activeBarClassPtr (
  void *_srcPtr )
{

activeBarClass *ptr, *srcPtr;

  srcPtr = (activeBarClass *) _srcPtr;

  ptr = new activeBarClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
