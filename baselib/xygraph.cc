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

#define __xygraph_cc 1

#include "xygraph.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

#ifdef __epics__

static void xygo_monitor_plot_connect_state (
  struct connection_handler_args arg );

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(ast_args.chid);
xyGraphClass *axygo = (xyGraphClass *) ptr->objPtr;

  if ( axygo->activeMode ) {

    if ( arg.op == CA_OP_CONN_UP ) {

      axygo->pvType = (int) ca_field_type( axygo->pvId );

      axygo->needPlotConnectInit = 1;
      axygo->needPlotConnect[ptr->index] = 1;
      axygo->notPlotPvConnected &= ptr->clrMask;

    }
    else {

      axygo->notPlotPvConnected |= ptr->setMask;
      axygo->fgColor.setDisconnected();
      axygo->needRefresh = 1;

    }

    axygo->actWin->appCtx->proc->lock();
    axygo->actWin->addDefExeNode( axygo->aglPtr );
    axygo->actWin->appCtx->proc->unlock();

  }

}

static void plotInfoUpdate (
  struct event_handler_args ast_args );

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(ast_args.chid);
xyGraphClass *axygo = (xyGraphClass *) ptr->objPtr;

int i, n;
struct dbr_gr_double doubleInfoRec;

  doubleInfoRec = *( (dbr_gr_double *) ast_args.dbr );

  if ( axygo->limitsFromDb || axygo->efPrecision.isNull() ) {
    axygo->precision = doubleInfoRec.precision;
  }

  axygo->needPlotInfoInit = 1;
  axygo->needPlotInfo[ptr->index] = 1;
  axygo->actWin->appCtx->proc->lock();
  axygo->actWin->addDefExeNode( axygo->aglPtr );
  axygo->actWin->appCtx->proc->unlock();

}

static void plotUpdate (
  struct event_handler_args ast_args );

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(ast_args.chid);
xyGraphClass *axygo = (xyGraphClass *) ptr->objPtr;

double dvalue;

  dvalue = *( (double *) ast_args.dbr );
  axygo->needUpdate = 1;
  axygo->actWin->appCtx->proc->lock();
  axygo->actWin->addDefExeNode( axygo->aglPtr );
  axygo->actWin->appCtx->proc->unlock();

}


#endif

static void axygc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;
int i;

  axygo->actWin->setChanged();

  axygo->eraseSelectBoxCorners();
  axygo->erase();

  axygo->fgColor.setColor( axygo->bufFgColor, axygo->actWin->ci );
  axygo->fgColor.setConnectSensitive();

  for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {
    strncpy( axygo->pvName[i], axygo->bufPvName[i], 39 );
    axygo->pvExpStr[i].setRaw( axygo->pvName[i] );
    axygo->plotColor[i].setColor( axygo->bufPlotColor[i], axygo->actWin->ci );
  }

  strncpy( axygo->ctlPvName, axygo->bufCtlPvName, 39 );
  axygo->ctlPvExpStr.setRaw( axygo->ctlPvName );

  strncpy( axygo->fontTag, axygo->fm.currentFontTag(), 63 );
  axygo->actWin->fi->loadFontTag( axygo->fontTag );
  axygo->actWin->drawGc.setFontTag( axygo->fontTag, axygo->actWin->fi );

  axygo->fs = axygo->actWin->fi->getXFontStruct( axygo->fontTag );

  axygo->autoScale = axygo->bufAutoScale;

  strncpy( axygo->formatType,  axygo->bufFormatType, 1 );

  strncpy( axygo->plotTitle, axygo->bufPlotTitle, 79 );

  labelType = bufLabelType;
  border = bufBorder;
  xLabelTicks = bufXLabelTicks;
  xMajorTicks = bufXMajorTicks;
  xMinorTicks = bufXMinorTicks;
  yLabelTicks = bufYLabelTicks;
  yMajorTicks = bufYMajorTicks;
  yMinorTicks = bufYMinorTicks;

  axygo->efPrecision = axygo->bufEfPrecision;

  if ( axygo->efPrecision.isNull() )
    axygo->precision = 3;
  else
    axygo->precision = axygo->efPrecision.value();

  axygo->bgColor = axygo->bufBgColor;

  strncpy( axygo->id, axygo->bufId, 31 );

  axygo->activateCallbackFlag = axygo->bufActivateCallbackFlag;
  axygo->deactivateCallbackFlag = axygo->bufDeactivateCallbackFlag;
  axygo->anyCallbackFlag = axygo->activateCallbackFlag ||
   axygo->deactivateCallbackFlag;

  axygo->x = axygo->bufX;
  axygo->sboxX = axygo->bufX;

  axygo->y = axygo->bufY;
  axygo->sboxY = axygo->bufY;

  axygo->w = axygo->bufW;
  axygo->sboxW = axygo->bufW;

  axygo->h = axygo->bufH;
  axygo->sboxH = axygo->bufH;

}

static void axygc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygc_edit_update( w, client, call );
  axygo->refresh( axygo );

}

static void axygc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygc_edit_update( w, client, call );
  axygo->ef.popdown();
  axygo->operationComplete();

}

static void axygc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygo->ef.popdown();
  axygo->operationCancel();

}

static void axygc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygo->ef.popdown();
  axygo->operationCancel();
  axygo->erase();
  axygo->deleteRequest = 1;
  axygo->drawAll();

}

xyGraphClass::xyGraphClass ( void ) {

int i;

  name = new char[strlen("xyGraphClass")+1];
  strcpy( name, "xyGraphClass" );

  for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {
    strcpy( plotPvName[i], "" );
  }

  strcpy( ctlPvName, "" );

  strcpy( formatType, "g" );
  efPrecision.setNull(1);
  precision = 3;
  activeMode = 0;
  strcpy( id, "" );
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;
  anyCallbackFlag = 0;
  activateCallback = NULL;
  deactivateCallback = NULL;
  autoScale = 0;

  strcpy( plotTitle, "" );

  labelType = XYGC_K_PVNAME;
  border = 1;
  xLabelTicks = 10;
  xMajorTicks = 20;
  xMinorTicks = 2;
  yLabelTicks = 10;
  yMajorTicks = 20;
  yMinorTicks = 2;

}

// copy constructor
xyGraphClass::xyGraphClass
 ( const xyGraphClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
int i;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("xyGraphClass")+1];
  strcpy( name, "xyGraphClass" );

  for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {
    strncpy( pvName[i], source->pvName[i], 39 );
    pvExpStr[i].copy( source->pvExpStr[i] );
    plotColor[i].copy( source->plotColor[i] );
    plotCb[i] = source->plotCb[i];
  }

  strncpy( ctlPvName, source->ctlPvName, 39 ); );
  ctlPvExpStr.copy( source->ctlPvExpStr );
  strncpy( formatType, source->formatType, 1 );
  efPrecision = source->efPrecision;
  precision = source->precision;
  activeMode = source->activeMode;
  strncpy( id, source->id, 31 );
  activateCallbackFlag = source->activateCallbackFlag;
  deactivateCallbackFlag = source->deactivateCallbackFlag;
  anyCallbackFlag = source->anyCallbackFlag;
  activateCallback = NULL;
  deactivateCallback = NULL;
  autoScale = source->autoScale;
  bgColor = source->bgColor;
  fgColor.copy(source->fgColor);
  strncpy( fontTag, source->fontTag, 63 );
  strncpy( bufFontTag, source->bufFontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );
  fgCb = source->fgCb;
  bgCb = source->bgCb;
  activeMode = 0;
  strncpy( id, source->id, 31 );

  strncpy( plotTitle, source->plotTitle, 79 );

  labelType = source->labelType;
  border = source->border;
  xLabelTicks = source->xLabelTicks;
  xMajorTicks = source->xMajorTicks;
  xMinorTicks = source->xMinorTicks;
  yLabelTicks = source->yLabelTicks;
  yMajorTicks = source->yMajorTicks;
  yMinorTicks = source->yMinorTicks;

}

int xyGraphClass::createInteractive (
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

int i;

  plotColor[0].setColor( actWin->defaultFg1Color, actWin->ci );
  for ( i=1; i<XYGC_K_MAX_PLOTS; i++ ) {
    plotColor[i].setColor( actWin->defaultFg2Color, actWin->ci );
  }

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  bgColor = actWin->defaultBgColor;

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

  this->draw();

  this->editCreate();

  return 1;

}

int xyGraphClass::save (
  FILE *f )
{

int index, stat, i;

  fprintf( f, "%-d %-d %-d\n", XYGC_MAJOR_VERSION, XYGC_MINOR_VERSION,
   XYGC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  for ( i=1; i<XYGC_K_MAX_PLOTS; i++ ) {
    writeStringToFile( f, plotPvName[i] );
    actWin->ci->getIndex( plotColor[i].pixelColor(), &index );
    fprintf( f, "%-d\n", index );
  }

  writeStringToFile( f, ctlPvName );

  writeStringToFile( f, fontTag );

  actWin->ci->getIndex( fgColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  actWin->ci->getIndex( bgColor, &index );
  fprintf( f, "%-d\n", index );

  writeStringToFile( f, formatType );

  writeStringToFile( f, plotTitle );

  fprintf( f, "%-d\n", labelType );
  fprintf( f, "%-d\n", border );
  fprintf( f, "%-d\n", xLabelTicks );
  fprintf( f, "%-d\n", xMajorTicks );
  fprintf( f, "%-d\n", xMinorTicks );
  fprintf( f, "%-d\n", yLabelTicks );
  fprintf( f, "%-d\n", yMajorTicks );
  fprintf( f, "%-d\n", yMinorTicks );

  stat = efPrecision.write( f );

  fprintf( f, "%-d\n", autoScale );

  writeStringToFile( f, id );

  fprintf( f, "%-d\n", activateCallbackFlag );

  fprintf( f, "%-d\n", deactivateCallbackFlag );

  return 1;

}

int xyGraphClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, r, g, b, index;
int major, minor, release;
int stat = 1;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  for ( i=1; i<XYGC_K_MAX_PLOTS; i++ ) {

    readStringFromFile( plotPvName[i], 39, f ); actWin->incLine();
    plotPvExpStr[i].setRaw( plotPvName[i] );

    if ( major > 1 ) {
      fscanf( f, "%d\n", &index ); actWin->incLine();
      actWin->ci->setIndex( index, &bufPlotColor[i] );
    }
    else {
      fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
      actWin->ci->setRGB( r, g, b, &bufPlotColor[i] );
    }
    plotColor[i].setColor( bufPlotColor[i], actWin->ci );

  }

  readStringFromFile( ctlPvName, 39, f ); actWin->incLine();
  ctlPvExpStr.setRaw( ctlPvName );

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &bufFgColor );
    fgColor.setColor( bufFgColor, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &bgColor );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &bufFgColor );
    fgColor.setColor( bufFgColor, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &bgColor );

  }

  readStringFromFile( this->formatType, 2, f ); actWin->incLine();

  readStringFromFile( plotTitle, 79, f ); actWin->incLine();

  fscanf( f, "%d\n", &labelType ); actWin->incLine();
  fscanf( f, "%d\n", &border ); actWin->incLine();
  fscanf( f, "%d\n", &xLabelTicks ); actWin->incLine();
  fscanf( f, "%d\n", &xMajorTicks ); actWin->incLine();
  fscanf( f, "%d\n", &xMinorTicks ); actWin->incLine();
  fscanf( f, "%d\n", &yLabelTicks ); actWin->incLine();
  fscanf( f, "%d\n", &yMajorTicks ); actWin->incLine();
  fscanf( f, "%d\n", &yMinorTicks ); actWin->incLine();

  stat = efPrecision.read( f ); actWin->incLine();
  if ( efPrecision.isNull() )
    precision = 3;
  else
    precision = efPrecision.value();

  fscanf( f, "%d\n", &autoScale ); actWin->incLine();

  readStringFromFile( this->id, 31, f ); actWin->incLine();
  fscanf( f, "%d\n", &activateCallbackFlag ); actWin->incLine();
  fscanf( f, "%d\n", &deactivateCallbackFlag ); actWin->incLine();
  anyCallbackFlag = activateCallbackFlag || deactivateCallbackFlag;

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );

  return stat;

}

int xyGraphClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, more;
int stat = 1;
char *tk, *gotData, *context, buf[255+1];

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;

  this->actWin = _actWin;

  return 0; // not implemented

}

int xyGraphClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "xyGraphClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  strncat( title, " Properties", 31 );

  strncpy( bufId, id, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufFgColor = fgColor.pixelColor();
  bufBgColor = bgColor;
  strncpy( bufFontTag, fontTag, 63 );
  bufUseDisplayBg = useDisplayBg;
  bufAutoHeight = autoHeight;
  strncpy( bufFormatType, formatType, 1 );
  bufColorMode = colorMode;
  strncpy( bufValue, value, 127 );
  strncpy( bufPvName, pvName, 39 );
  bufEditable = editable;
  bufIsWidget = isWidget;
  bufLimitsFromDb = limitsFromDb;
  bufEfPrecision = efPrecision;

  strncpy( bufPlotTitle, plotTitle, 79 );

  bufLabelType = labelType;
  bufBorder = border;
  bufXLabelTicks = xLabelTicks;
  bufXMajorTicks = xMajorTicks;
  bufXMinorTicks = xMinorTicks;
  bufYLabelTicks = yLabelTicks;
  bufYMajorTicks = yMajorTicks;
  bufYMinorTicks = yMinorTicks;

  bufChangeCallbackFlag = changeCallbackFlag;
  bufActivateCallbackFlag = activateCallbackFlag;
  bufDeactivateCallbackFlag = deactivateCallbackFlag;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "ID", 30, bufId, 31 );
  ef.addTextField( "X", 30, &bufX );
  ef.addTextField( "Y", 30, &bufY );
  ef.addTextField( "Width", 30, &bufW );
  ef.addTextField( "Height", 30, &bufH );
  ef.addFontMenu( "Font", actWin->fi, &fm, fontTag );
  ef.addColorButton( "Fg Color", actWin->ci, &fgCb, &bufFgColor );
  ef.addColorButton( "Bg Color", actWin->ci, &bgCb, &bufBgColor );
  ef.addOption( "Scale Format", "g|f|e", bufFormatType, 1 );
  ef.addTextField( "Scale Precision", 30, &bufEfPrecision );
  ef.addTextField( "Ctl PV Name", 30, bufCtlPvName, 39 );
  ef.addToggle( "Activate Callback", &bufActivateCallbackFlag );
  ef.addToggle( "Deactivate Callback", &bufDeactivateCallbackFlag );

  return 1;

}

int xyGraphClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( axygc_edit_ok, axygc_edit_apply, axygc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int xyGraphClass::edit ( void ) {

  this->genericEdit();
  ef.finished( axygc_edit_ok, axygc_edit_apply, axygc_edit_cancel, this );
  fm.setFontAlignment( alignment );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int xyGraphClass::erase ( void ) {

  if ( activeMode || deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int xyGraphClass::eraseActive ( void ) {

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int xyGraphClass::draw ( void ) {

  if ( activeMode || deleteRequest ) return 1;

  actWin->drawGc.setFG( fgColor.pixelColor() );
  actWin->drawGc.setBG( bgColor );

  return 1;

}

int xyGraphClass::drawActive ( void ) {

  if ( !activeMode || !init ) return 1;

  actWin->drawGc.setFG( fgColor.pixelColor() );
  actWin->drawGc.setBG( bgColor );

  return 1;

}

void xyGraphClass::bufInvalidate ( void ) {

  bufInvalid = 1;

}

int xyGraphClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int i, stat, retStat = 1;

  stat = ctlPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {
    stat = plotPvExpStr.expand1st( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
  }

  return retStat;

}

int xyGraphClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int stat;

int i, stat, retStat = 1;

  stat = ctlPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {
    stat = plotPvExpStr.expand2nd( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
  }

  return retStat;

}

int xyGraphClass::containsMacros ( void ) {

int i, result;

  result = ctlPvExpStr.containsPrimaryMacros();
  if ( result ) return result;

  for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {
    result = plotPvExpStr.containsPrimaryMacros();
    if ( result ) return result;
  }

  return 0;

}

int xyGraphClass::activate (
  int pass,
  void *ptr )
{

int i, stat;
char callbackName[63+1];

  switch ( pass ) {

  case 1:

    aglPtr = ptr;
    plot_widget = NULL;
    opComplete = 0;
    activeMode = 1;
    init = 0;

    needPlotConnectInit = needPlotInfoInit = needErase = needDraw =
     needRefresh = needUpdate = 0;

    for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {

#ifdef __epics__
      plotEventId[i] = 0;
#endif

      needPlotConnect[i] = 0;
      needPlotInfo[i] = 0;

      plotPvExists[i] = 0;
      if ( plotPvExpStr[i].getExpanded() ) {
        if ( !blank( plotPvExpStr[i].getExpanded() ) ) {
          plotPvExists[i] = 1;
	}
      }

    }

#ifdef __epics__
      ctlEventId[i] = 0;
#endif

    ctlPvExists = 0;
    if ( ctlPvExpStr.getExpanded() ) {
      if ( !blank( ctlPvExpStr.getExpanded() ) ) {
        ctlPvExists = 1;
      }
    }

    break;

  case 2:

    if ( !opComplete ) {

      atLeastOnePlotPvExists = 0;
      for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {

        if ( plotPvExists[i] ) {

          atLeastOnePlotPvExists = 1;

          argRec[i].objPtr = (void *) this;
          argRec[i].index = i;
          argRec[i].setMask = (unsigned int) 1 << i;
          argRec[i].clrMask = ~(argRec[i].setMask);


#ifdef __epics__
          stat = ca_search_and_connect( plotExpStr[i].getExpanded(),
           &plotPvId[i], xygo_monitor_plot_connect_state, &argRec[i] );
          if ( stat != ECA_NORMAL ) {
            printf( "error from ca_search\n" );
            return 0;
          }
#endif

        }

      }

      if ( !atLeastOnePlotPvExists && anyCallbackFlag ) {

        needInfoInit = 1;
        actWin->appCtx->proc->lock();
        actWin->addDefExeNode( aglPtr );
        actWin->appCtx->proc->unlock();

      }

      if ( anyCallbackFlag ) {

        if ( activateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          strncat( callbackName, "Activate", 63 );
          activateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( deactivateCallbackFlag ) {
          strncpy( callbackName, id, 63 );
          strncat( callbackName, "Deactivate", 63 );
          deactivateCallback =
           actWin->appCtx->userLibObject.getFunc( callbackName );
	}

        if ( activateCallback ) {
          (*activateCallback)( this );
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

  } // end switch

  return 1;

}

int xyGraphClass::deactivate (
  int pass
) {

int i, stat;

  if ( pass == 1 ) {

  activeMode = 0;

  if ( deactivateCallback ) {
    (*deactivateCallback)( this );
  }

#ifdef __epics__

  for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {

    if ( plotPvExists[i] ) {

      stat = ca_clear_channel( plotPvId[i] );
      if ( stat != ECA_NORMAL )
        printf( "ca_clear_channel failure\n" );

    }

  }

#endif

  }
  else if ( pass == 2 ) {

    if ( plot_widget ) XtDestroyWidget( plot_widget );

  }

  return 1;

}

void xyGraphClass::updateDimensions ( void )
{

}

void xyGraphClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

}

int xyGraphClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag )
{

  *up = 0;
  *down = 0;
  *drag = 0;

  return 1;

}

void xyGraphClass::executeDeferred ( void ) {

int i, stat;
int npc, npi, ncc, nu, nr;
Arg args[10];
int pc[XYGC_K_MAX_PLOTS], pi[XYGC_K_MAX_PLOTS];

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  npc = needPlotConnectInit; needPlotConnectInit = 0;
  npi = needPlotInfoInit; needPlotInfoInit = 0;
  for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {
    pc[i] = needPlotConnect[i];
    pi[i] = needPlotInfo[i];

  ncc = needCtlConnectInit; needCtlConnectInit = 0;
  nr = needRefresh; needRefresh = 0;
  nu = needUpdate; needUpdate = 0;
  actWin->appCtx->proc->unlock();

  if ( npc ) {

    for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {

#ifdef __epics__
      stat = ca_get_callback( DBR_GR_DOUBLE, ?Id,
       ?InfoUpdate, (void *) this );
      if ( stat != ECA_NORMAL ) {
        printf( "ca_get_callback failed\n" );
      }
#endif

      bufInvalidate();

    }

  }

  if ( npi ) {

    for ( i=0; i<XYGC_K_MAX_PLOTS; i++ ) {

      if ( plotPvExists[i] ) {

        sprintf( format, "%%.%-df%%", precision, formatType );

#ifdef __epics__
        if ( !eventId ) {
          stat = ca_add_masked_array_event( DBR_DOUBLE, 1, ?Id,
           ?Update, (void *) this, (float) 0.0, (float) 0.0, (float) 0.0,
           &eventId, DBE_VALUE );
          if ( stat != ECA_NORMAL ) {
            printf( "ca_add_masked_array_event failed\n" );
          }
        }
#endif

      }

    }

    if ( !plot_widget ) {

      plot_widget = XtVaCreateManagedWidget( "", xmdrawingWidgetClass,
       actWin->executeWidget,
       XmNx, x,
       XmNy, y,
       XmNforeground, fgColor.getColor(),
       XmNbackground, bg,
       XmNhighlightThickness, 0,
       XmNmarginHeight, 0,
       NULL );

        XtAddCallback( plot_widget, XmN?Callback,
         xygo?, this );

    } // end if ( !plot_widget )

  init = 1;

  }

  if ( nr ) {

    bufInvalidate();
    eraseActive();
    drawActive();

  }

  if ( nu ) {

    eraseActive();
    drawActive();

  }

  actWin->appCtx->proc->lock();
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

}

int xyGraphClass::getProperty (
  char *prop,
  int bufSize,
  char *_value )
{

  return 0;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_xyGraphClassPtr ( void ) {

xyGraphClass *ptr;

  ptr = new xyGraphClass;
  return (void *) ptr;

}

void *clone_xyGraphClassPtr (
  void *_srcPtr )
{

xyGraphClass *ptr, *srcPtr;

  srcPtr = (xyGraphClass *) _srcPtr;

  ptr = new xyGraphClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
