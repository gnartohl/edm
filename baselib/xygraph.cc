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

// This is the EPICS specific line right now:
static PV_Factory *pv_factory = new EPICS_PV_Factory();

static void xMonitorConnection (
  struct connection_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;

  if ( arg.op == CA_OP_CONN_UP ) {

    if ( !xyo->connection.pvsConnected() ) {

      xyo->connection.setPvConnected( (void *) ptr->index );
      if ( xyo->connection.pvsConnected() ) {

        printf( "all connected\n" );
        xyo->actWin->appCtx->proc->lock();
        xyo->needConnect = 1;
        xyo->actWin->addDefExeNode( xyo->aglPtr );
        xyo->actWin->appCtx->proc->unlock();

      }

    }

  }
  else {

    xyo->connection.setPvDisconnected( (void *) ptr->index );
    xyo->actWin->appCtx->proc->lock();
    xyo->active = 0;
    xyo->bufInvalidate();
    xyo->needErase = 1;
    xyo->needDraw = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();


  }

}

static void xInfoUpdate (
  struct event_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
struct dbr_gr_double grRec = *( (dbr_gr_double *) arg.dbr );
int i =  ptr->index;

  printf( "xInfoUpdate\n" );

  xyo->dbXMin[i] = grRec.lower_disp_limit;
  xyo->dbXMax[i] = grRec.upper_disp_limit;
  xyo->dbXPrec[i] = grRec.precision;

  xyo->actWin->appCtx->proc->lock();
  xyo->needInit = 1;
  xyo->xArrayNeedInit[i] = 1;
  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void xValueUpdate (
  struct event_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
int i =  ptr->index;

  printf( "xValueUpdate\n" );

  xyo->actWin->appCtx->proc->lock();
  memcpy( xyo->xPvData[i], arg.dbr, xyo->xPvSize[i] );
  xyo->needUpdate = 1;
  xyo->xArrayNeedUpdate[i] = 1;
  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void yMonitorConnection (
  struct connection_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;

  if ( arg.op == CA_OP_CONN_UP ) {

    if ( !xyo->connection.pvsConnected() ) {

      xyo->connection.setPvConnected( (void *) ptr->index );

      if ( xyo->connection.pvsConnected() ) {

        printf( "all connected\n" );
        xyo->actWin->appCtx->proc->lock();
        xyo->needConnect = 1;
        xyo->actWin->addDefExeNode( xyo->aglPtr );
        xyo->actWin->appCtx->proc->unlock();

      }

    }

  }
  else {

    xyo->connection.setPvDisconnected( (void *) ptr->index );
    xyo->actWin->appCtx->proc->lock();
    xyo->active = 0;
    xyo->bufInvalidate();
    xyo->needErase = 1;
    xyo->needDraw = 1;
    xyo->actWin->addDefExeNode( xyo->aglPtr );
    xyo->actWin->appCtx->proc->unlock();


  }

}

static void yInfoUpdate (
  struct event_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
struct dbr_gr_double grRec = *( (dbr_gr_double *) arg.dbr );
int i =  ptr->index;

  printf( "yInfoUpdate\n" );

  xyo->dbYMin[i] = grRec.lower_disp_limit;
  xyo->dbYMax[i] = grRec.upper_disp_limit;
  xyo->dbYPrec[i] = grRec.precision;

  xyo->actWin->appCtx->proc->lock();
  xyo->needInit = 1;
  xyo->yArrayNeedInit[i] = 1;
  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void yValueUpdate (
  struct event_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
xyGraphClass *xyo = (xyGraphClass *) ptr->objPtr;
int i =  ptr->index;

  printf( "yValueUpdate, i=%-d, size=%-d\n", i, xyo->yPvSize[i] );

  xyo->actWin->appCtx->proc->lock();
  memcpy( xyo->yPvData[i], arg.dbr, xyo->yPvSize[i] );
  xyo->needUpdate = 1;
  xyo->yArrayNeedUpdate[i] = 1;
  xyo->actWin->addDefExeNode( xyo->aglPtr );
  xyo->actWin->appCtx->proc->unlock();

}

static void axygc_edit_ok_trace (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygo->efTrace->popdownNoDestroy();

}

//-------------------------------------------------------------------------

static void axygc_edit_ok_axis (
  Widget w,
  XtPointer client,
  XtPointer call )
{

xyGraphClass *axygo = (xyGraphClass *) client;

  axygo->efAxis->popdownNoDestroy();

}

//-------------------------------------------------------------------------

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

  axygo->x = axygo->eBuf->bufX;
  axygo->sboxX = axygo->eBuf->bufX;

  axygo->y = axygo->eBuf->bufY;
  axygo->sboxY = axygo->eBuf->bufY;

  axygo->w = axygo->eBuf->bufW;
  axygo->sboxW = axygo->eBuf->bufW;

  axygo->h = axygo->eBuf->bufH;
  axygo->sboxH = axygo->eBuf->bufH;

  axygo->graphTitle.setRaw( axygo->eBuf->bufGraphTitle );

  axygo->xLabel.setRaw( axygo->eBuf->bufXLabel );

  axygo->yLabel.setRaw( axygo->eBuf->bufYLabel );

  axygo->fgColor = axygo->eBuf->bufFgColor;

  axygo->bgColor = axygo->eBuf->bufBgColor;

  axygo->plotStyle = axygo->eBuf->bufPlotStyle;

  axygo->plotMode = axygo->eBuf->bufPlotMode;

  axygo->count = axygo->eBuf->bufCount;

  axygo->numTraces = 0;
  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    axygo->plotColor[i] = axygo->eBuf->bufPlotColor[i];
    if ( ( !blank( axygo->eBuf->bufXPvName[i] ) ) ||
         ( !blank( axygo->eBuf->bufYPvName[i] ) ) ) {
      (axygo->numTraces)++;
      axygo->xPvExpStr[i].setRaw( axygo->eBuf->bufXPvName[i] );
      axygo->yPvExpStr[i].setRaw( axygo->eBuf->bufYPvName[i] );
    }
    else {
      axygo->xPvExpStr[i].setRaw( "" );
      axygo->yPvExpStr[i].setRaw( "" );
    }
  }

  axygo->xAxisStyle = axygo->eBuf->bufXAxisStyle;
  axygo->xAxisSource = axygo->eBuf->bufXAxisSource;
  axygo->xMin = axygo->eBuf->bufXMin;
  axygo->xMax = axygo->eBuf->bufXMax;
  axygo->xAxisTimeFormat = axygo->eBuf->bufXAxisTimeFormat;

  axygo->y1AxisStyle = axygo->eBuf->bufY1AxisStyle;
  axygo->y1AxisSource = axygo->eBuf->bufY1AxisSource;
  axygo->y1Min = axygo->eBuf->bufY1Min;
  axygo->y1Max = axygo->eBuf->bufY1Max;

  axygo->y2AxisStyle = axygo->eBuf->bufY2AxisStyle;
  axygo->y2AxisSource = axygo->eBuf->bufY2AxisSource;
  axygo->y2Min = axygo->eBuf->bufY2Min;
  axygo->y2Max = axygo->eBuf->bufY2Max;

  axygo->trigPvExpStr.setRaw( axygo->eBuf->bufTrigPvName );
  axygo->erasePvExpStr.setRaw( axygo->eBuf->bufErasePvName );
  axygo->eraseMode = axygo->eBuf->bufEraseMode;

  strncpy( axygo->fontTag, axygo->fm.currentFontTag(), 63 );
  axygo->actWin->fi->loadFontTag( axygo->fontTag );
  axygo->actWin->drawGc.setFontTag( axygo->fontTag, axygo->actWin->fi );

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

//-------------------------------------------------------------------------

xyGraphClass::xyGraphClass ( void ) {

int i;

  name = new char[strlen("xyGraphClass")+1];
  strcpy( name, "xyGraphClass" );

  plotStyle = XYGC_K_PLOT_STYLE_POINT;
  plotMode = XYGC_K_PLOT_MODE_PLOT_N_STOP;
  count = 1;

  numTraces = 0;

  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    xPv[i] = NULL;
    yPv[i] = NULL;
  }
  trigPv = NULL;
  erasePv = NULL;

  eraseMode = 0;
  xAxisStyle = 0;
  xAxisSource = 0;
  xAxisTimeFormat = 0;

  y1AxisStyle = 0;
  y1AxisSource = 0;
  y1AxisTimeFormat = 0;

  y2AxisStyle = 0;
  y2AxisSource = 0;
  y2AxisTimeFormat = 0;

  xFormatType = 0;
  strcpy( xFormat, "f" );

  y1FormatType = 0;
  strcpy( y1Format, "f" );

  y2FormatType = 0;
  strcpy( y2Format, "f" );

  border = 0;

  activeMode = 0;

  connection.setMaxPvs( XYGC_K_MAX_TRACES + 2 );

  eBuf = NULL;

}

// copy constructor
xyGraphClass::xyGraphClass
 ( const xyGraphClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
int i;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("xyGraphClass")+1];
  strcpy( name, "xyGraphClass" );

  graphTitle.copy( source->graphTitle );
  xLabel.copy( source->xLabel );
  yLabel.copy( source->yLabel );

  fgCb = source->fgCb;
  bgCb = source->bgCb;

  plotStyle = source->plotStyle;
  plotMode = source->plotMode;
  count = source->count;

  numTraces = source->numTraces;

  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    xPv[i] = NULL;
    yPv[i] = NULL;
    plotColor[i] = source->plotColor[i];
    xPvExpStr[i].copy( source->xPvExpStr[i] );
    yPvExpStr[i].copy( source->yPvExpStr[i] );
  }

  trigPv = NULL;
  trigPvExpStr.copy( source->trigPvExpStr );

  erasePv = NULL;
  erasePvExpStr.copy( source->erasePvExpStr );
  eraseMode = source->eraseMode;

  xAxisStyle = source->xAxisStyle;
  xAxisSource = source->xAxisSource;
  xAxisTimeFormat = source->xAxisTimeFormat;

  y1AxisStyle = source->y1AxisStyle;
  y1AxisSource = source->y1AxisSource;
  y1AxisTimeFormat = source->y1AxisTimeFormat;

  y2AxisStyle = source->y2AxisStyle;
  y2AxisSource = source->y2AxisSource;
  y2AxisTimeFormat = source->y2AxisTimeFormat;

  xFormatType = source->xFormatType;
  strncpy( xFormat, source->xFormat, 15 );

  y1FormatType = source->y1FormatType;
  strncpy( y1Format, source->y1Format, 15 );

  y2FormatType = source->y2FormatType;
  strncpy( y2Format, source->y2Format, 15 );

  border = source->border;

  activeMode = source->activeMode;

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );
  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;

  connection.setMaxPvs( XYGC_K_MAX_TRACES + 2 );

  eBuf = NULL;

}

void xyGraphClass::plotPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *axygo = (xyGraphClass *) ptr->objPtr;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    axygo->connection.setPvDisconnected( (void *) ptr->index );

    axygo->actWin->appCtx->proc->lock();
    axygo->needRefresh = 1;
    axygo->actWin->addDefExeNode( axygo->aglPtr );
    axygo->actWin->appCtx->proc->unlock();

  }

}

void xyGraphClass::plotUpdate (
  ProcessVariable *pv,
  void *userarg
) {

objPlusIndexPtr ptr = (objPlusIndexPtr) userarg;
xyGraphClass *axygo = (xyGraphClass *) ptr->objPtr;

  if ( !axygo->connection.pvsConnected() ) {

    axygo->connection.setPvConnected( (void *) ptr->index );

    if ( axygo->connection.pvsConnected() ) {
      axygo->actWin->appCtx->proc->lock();
      axygo->needConnect = 1;
      axygo->actWin->addDefExeNode( axygo->aglPtr );
      axygo->actWin->appCtx->proc->unlock();
    }

  }
  else {

    axygo->actWin->appCtx->proc->lock();
    axygo->needUpdate = 1;
    axygo->actWin->addDefExeNode( axygo->aglPtr );
    axygo->actWin->appCtx->proc->unlock();

  }

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
char traceColor[15+1];

  fgColor = actWin->defaultTextFgColor;
  bgColor = actWin->defaultBgColor;

  for ( i=0; i<XYGC_K_MAX_TRACES; i++ ) {
    sprintf( traceColor, "trace%-d", i );
    plotColor[i] = actWin->ci->colorIndexByAlias( traceColor );
  }

  strcpy( fontTag, actWin->defaultCtlFontTag );
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

int i, stat = 1;

  fprintf( f, "%-d %-d %-d\n", XYGC_MAJOR_VERSION, XYGC_MINOR_VERSION,
   XYGC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  if ( graphTitle.getRaw() )
    writeStringToFile( f, graphTitle.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( xLabel.getRaw() )
    writeStringToFile( f, xLabel.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( yLabel.getRaw() )
    writeStringToFile( f, yLabel.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", fgColor );

  fprintf( f, "%-d\n", bgColor );

  fprintf( f, "%-d\n", plotStyle );

  fprintf( f, "%-d\n", plotMode );

  fprintf( f, "%-d\n", count );

  fprintf( f, "%-d\n", numTraces );

  for ( i=0; i<numTraces; i++ ) {
    if ( xPvExpStr[i].getRaw() )
      writeStringToFile( f, xPvExpStr[i].getRaw() );
    else
      writeStringToFile( f, "" );
    if ( yPvExpStr[i].getRaw() )
      writeStringToFile( f, yPvExpStr[i].getRaw() );
    else
      writeStringToFile( f, "" );
    fprintf( f, "%-d\n", plotColor[i] ); actWin->incLine();
  }

  fprintf( f, "%-d\n", xAxisStyle );
  fprintf( f, "%-d\n", xAxisSource );
  stat = xMin.write( f );
  stat = xMax.write( f );
  fprintf( f, "%-d\n", xAxisTimeFormat );

  fprintf( f, "%-d\n", y1AxisStyle );
  fprintf( f, "%-d\n", y1AxisSource );
  stat = y1Min.write( f );
  stat = y1Max.write( f );

  fprintf( f, "%-d\n", y2AxisStyle );
  fprintf( f, "%-d\n", y2AxisSource );
  stat = y2Min.write( f );
  stat = y2Max.write( f );

  if ( trigPvExpStr.getRaw() )
    writeStringToFile( f, trigPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( erasePvExpStr.getRaw() )
    writeStringToFile( f, erasePvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", eraseMode );

  writeStringToFile( f, fontTag );

  return stat;

}

int xyGraphClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i;
int major, minor, release;
int stat = 1;
char str[127+1], traceColor[15+1], onePv[activeGraphicClass::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  readStringFromFile( str, 127+1, f ); actWin->incLine();
  graphTitle.setRaw( str );

  readStringFromFile( str, 127+1, f ); actWin->incLine();
  xLabel.setRaw( str );

  readStringFromFile( str, 127+1, f ); actWin->incLine();
  yLabel.setRaw( str );

  fscanf( f, "%d\n", &fgColor ); actWin->incLine();

  fscanf( f, "%d\n", &bgColor ); actWin->incLine();

  fscanf( f, "%d\n", &plotStyle ); actWin->incLine();

  fscanf( f, "%d\n", &plotMode ); actWin->incLine();

  fscanf( f, "%d\n", &count ); actWin->incLine();

  fscanf( f, "%d\n", &numTraces ); actWin->incLine();

  for ( i=0; i<numTraces; i++ ) {
    readStringFromFile( onePv, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    xPvExpStr[i].setRaw( onePv );
    readStringFromFile( onePv, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    yPvExpStr[i].setRaw( onePv );
    fscanf( f, "%d\n", &plotColor[i] ); actWin->incLine();
  }

  for ( i=numTraces; i<XYGC_K_MAX_TRACES; i++ ) {
    sprintf( traceColor, "trace%-d", i );
    plotColor[i] = actWin->ci->colorIndexByAlias( traceColor );
  }

  fscanf( f, "%d\n", &xAxisStyle ); actWin->incLine();
  fscanf( f, "%d\n", &xAxisSource ); actWin->incLine();
  stat = xMin.read( f ); actWin->incLine();
  stat = xMax.read( f ); actWin->incLine();
  fscanf( f, "%d\n", &xAxisTimeFormat ); actWin->incLine();

  fscanf( f, "%d\n", &y1AxisStyle ); actWin->incLine();
  fscanf( f, "%d\n", &y1AxisSource ); actWin->incLine();
  stat = y1Min.read( f ); actWin->incLine();
  stat = y1Max.read( f ); actWin->incLine();

  fscanf( f, "%d\n", &y2AxisStyle ); actWin->incLine();
  fscanf( f, "%d\n", &y2AxisSource ); actWin->incLine();
  stat = y2Min.read( f ); actWin->incLine();
  stat = y2Max.read( f ); actWin->incLine();

  readStringFromFile( onePv, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  trigPvExpStr.setRaw( onePv );

  readStringFromFile( onePv, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  erasePvExpStr.setRaw( onePv );

  fscanf( f, "%d\n", &eraseMode ); actWin->incLine();

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

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

  updateDimensions();

  return stat;

}

int xyGraphClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

#if 0
int r, g, b, more;
int stat = 1;
char *tk, *gotData, *context, buf[255+1];

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;
#endif

  this->actWin = _actWin;

  return 0; // not implemented

}

int xyGraphClass::genericEdit ( void ) {

char title[32], *ptr;
int i;

  ptr = actWin->obj.getNameFromClass( "xyGraphClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  strncat( title, " Properties", 31 );

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufPlotStyle = plotStyle;

  eBuf->bufPlotMode = plotMode;

  eBuf->bufCount = count;

  strncpy( eBuf->bufGraphTitle, graphTitle.getRaw(), 127 );
  strncpy( eBuf->bufXLabel, xLabel.getRaw(), 127 );
  strncpy( eBuf->bufYLabel, yLabel.getRaw(), 127 );
  eBuf->bufFgColor = fgColor;
  eBuf->bufBgColor = bgColor;
  strncpy( eBuf->bufTrigPvName, trigPvExpStr.getRaw(),
   activeGraphicClass::MAX_PV_NAME );
  strncpy( eBuf->bufErasePvName, erasePvExpStr.getRaw(),
   activeGraphicClass::MAX_PV_NAME );
  eBuf->bufEraseMode = eraseMode;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "X", 30, &eBuf->bufX );
  ef.addTextField( "Y", 30, &eBuf->bufY );
  ef.addTextField( "Width", 30, &eBuf->bufW );
  ef.addTextField( "Height", 30, &eBuf->bufH );
  ef.addTextField( "Title", 30, eBuf->bufGraphTitle, 127 );
  ef.addTextField( "X Label", 30, eBuf->bufXLabel, 127 );
  ef.addTextField( "Y Label", 30, eBuf->bufYLabel, 127 );
  ef.addColorButton( "Foreground", actWin->ci, &fgCb, &eBuf->bufFgColor );
  ef.addColorButton( "Background", actWin->ci, &bgCb, &eBuf->bufBgColor );
  ef.addOption( "Plot Style", "point|line", &eBuf->bufPlotStyle );
  ef.addOption( "Plot Mode", "plot n pts & stop|plot last n pts", &eBuf->bufPlotMode );
  ef.addTextField( "Count", 30, &eBuf->bufCount );

  ef.addEmbeddedEf( "X/Y/Trace Data", "... ", &efTrace );

    efTrace->create( actWin->top, actWin->appCtx->ci.getColorMap(),
     &actWin->appCtx->entryFormX,
     &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
     &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
     title, NULL, NULL, NULL );

    for ( i=0; i<numTraces; i++ ) {
      strncpy( eBuf->bufXPvName[i], xPvExpStr[i].getRaw(),
       activeGraphicClass::MAX_PV_NAME );
      strncpy( eBuf->bufYPvName[i], yPvExpStr[i].getRaw(),
       activeGraphicClass::MAX_PV_NAME );
      eBuf->bufPlotColor[i] = plotColor[i];
    }
    for ( i=numTraces; i<XYGC_K_MAX_TRACES; i++ ) {
      strcpy( eBuf->bufXPvName[i], "" );
      strcpy( eBuf->bufYPvName[i], "" );
      eBuf->bufPlotColor[i] = plotColor[i];
    }

    i = 0;
    efTrace->beginSubForm();
    efTrace->addTextField( "X ", 30, eBuf->bufXPvName[i],
     activeGraphicClass::MAX_PV_NAME );
    efTrace->addLabel( "Y " );
    efTrace->addTextField( "", 30, eBuf->bufYPvName[i],
     activeGraphicClass::MAX_PV_NAME );
    efTrace->addLabel( " " );
    efTrace->addColorButton( "", actWin->ci, &plotCb[i],
     &eBuf->bufPlotColor[i] );
    efTrace->endSubForm();

    for ( i=1; i<XYGC_K_MAX_TRACES; i++ ) {

      efTrace->beginLeftSubForm();
      efTrace->addTextField( "X ", 30, eBuf->bufXPvName[i],
       activeGraphicClass::MAX_PV_NAME );
      efTrace->addLabel( "Y " );
      efTrace->addTextField( "", 30, eBuf->bufYPvName[i],
       activeGraphicClass::MAX_PV_NAME );
      efTrace->addLabel( " " );
      efTrace->addColorButton( "", actWin->ci, &plotCb[i],
       &eBuf->bufPlotColor[i] );
      efTrace->endSubForm();

    }

    efTrace->finished( axygc_edit_ok_trace, this );

  eBuf->bufXAxisStyle = xAxisStyle;
  eBuf->bufXAxisSource = xAxisSource;
  eBuf->bufXMin = xMin;
  eBuf->bufXMax = xMax;
  eBuf->bufXAxisTimeFormat = xAxisTimeFormat;

  eBuf->bufY1AxisStyle = y1AxisStyle;
  eBuf->bufY1AxisSource = y1AxisSource;
  eBuf->bufY1Min = y1Min;
  eBuf->bufY1Max = y1Max;

  eBuf->bufY2AxisStyle = y2AxisStyle;
  eBuf->bufY2AxisSource = y2AxisSource;
  eBuf->bufY2Min = y2Min;
  eBuf->bufY2Max = y2Max;

  ef.addEmbeddedEf( "Axis Data", "... ", &efAxis );

    efAxis->create( actWin->top, actWin->appCtx->ci.getColorMap(),
     &actWin->appCtx->entryFormX,
     &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
     &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
     title, NULL, NULL, NULL );

    efAxis->beginSubForm();
    efAxis->addLabel( " X Axis " );
    efAxis->addLabel( " Style" );
    efAxis->addOption( "", "linear|log10|time", &eBuf->bufXAxisStyle );
    efAxis->addLabel( " Range" );
    efAxis->addOption( "", "from channel|user-specified|auto-scale",
     &eBuf->bufXAxisSource );
    efAxis->addLabel( " Minimum" );
    efAxis->addTextField( "", 10, &eBuf->bufXMin );
    efAxis->addLabel( " Maximum" );
    efAxis->addTextField( "", 10, &eBuf->bufXMax );
    efAxis->addLabel( " Time Format" );
    efAxis->addOption( "",
     "hh:mm:ss|hh:mm|hh:00|MMM DD YYYY|MMM DD|MMM DD hh:00|wd hh:00",
     &eBuf->bufXAxisTimeFormat );
    efAxis->endSubForm();

    efAxis->beginLeftSubForm();
    efAxis->addLabel( "Y1 Axis " );
    efAxis->addLabel( " Style" );
    efAxis->addOption( "", "linear|log10", &eBuf->bufY1AxisStyle );
    efAxis->addLabel( " Range" );
    efAxis->addOption( "", "from channel|user-specified|auto-scale",
     &eBuf->bufY1AxisSource );
    efAxis->addLabel( " Minimum" );
    efAxis->addTextField( "", 10, &eBuf->bufY1Min );
    efAxis->addLabel( " Maximum" );
    efAxis->addTextField( "", 10, &eBuf->bufY1Max );
    efAxis->endSubForm();

    efAxis->beginLeftSubForm();
    efAxis->addLabel( "Y2 Axis " );
    efAxis->addLabel( " Style" );
    efAxis->addOption( "", "linear|log10", &eBuf->bufY2AxisStyle );
    efAxis->addLabel( " Range" );
    efAxis->addOption( "", "from channel|user-specified|auto-scale",
     &eBuf->bufY2AxisSource );
    efAxis->addLabel( " Minimum" );
    efAxis->addTextField( "", 10, &eBuf->bufY2Min );
    efAxis->addLabel( " Maximum" );
    efAxis->addTextField( "", 10, &eBuf->bufY2Max );
    efAxis->endSubForm();

    efAxis->finished( axygc_edit_ok_axis, this );

  ef.addTextField( "Trigger Channel", 30, eBuf->bufTrigPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addTextField( "Erase Channel", 30, eBuf->bufErasePvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addOption( "Erase Mode", "if not zero|if zero", &eBuf->bufEraseMode );
  ef.addFontMenuNoAlignInfo( "Font", actWin->fi, &fm, fontTag );

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

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( actWin->ci->pix(fgColor) );
  //actWin->drawGc.setBG( actWin->ci->pix(bgColor) );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.restoreFg();

  return 1;

}

int xyGraphClass::drawActive ( void ) {

  if ( !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( actWin->ci->pix(fgColor) );
  //actWin->executeGc.setBG( actWin->ci->pix(bgColor) );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  actWin->executeGc.restoreFg();

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

  stat = graphTitle.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = xLabel.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = yLabel.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = trigPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = erasePvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  for ( i=0; i<numTraces; i++ ) {
    stat = xPvExpStr[i].expand1st( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
    stat = yPvExpStr[i].expand1st( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
  }

  return retStat;

}

int xyGraphClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] ) {

int i, stat, retStat = 1;

  stat = graphTitle.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = xLabel.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = yLabel.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = trigPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = erasePvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  for ( i=0; i<numTraces; i++ ) {
    stat = xPvExpStr[i].expand2nd( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
    stat = yPvExpStr[i].expand2nd( numMacros, macros, expansions );
    if ( !( stat & 1 ) ) retStat = stat;
  }

  return retStat;

}

int xyGraphClass::containsMacros ( void ) {

int i, result;

  result = graphTitle.containsPrimaryMacros();
  if ( result ) return result;

  result = xLabel.containsPrimaryMacros();
  if ( result ) return result;

  result = yLabel.containsPrimaryMacros();
  if ( result ) return result;

  result = trigPvExpStr.containsPrimaryMacros();
  if ( result ) return result;

  result = erasePvExpStr.containsPrimaryMacros();
  if ( result ) return result;

  for ( i=0; i<numTraces; i++ ) {
    result = xPvExpStr[i].containsPrimaryMacros();
    if ( result ) return result;
    result = yPvExpStr[i].containsPrimaryMacros();
    if ( result ) return result;
  }

  return 0;

}

int xyGraphClass::activate (
  int pass,
  void *ptr )
{

int i, stat;

  printf( "activate\n" );

  switch ( pass ) {

  case 1:

    opComplete = 0;
    break;

  case 2:

    if ( !opComplete ) {

      opComplete = 1;
      aglPtr = ptr;
      connection.init();
      init = 0;
      bufInvalid = 1;
      activeMode = 0;
      needConnect = needInit = needRefresh = needErase = needDraw = 
       needUpdate = 0;

      for ( i=0; i<numTraces; i++ ) {
        xArrayNeedInit[i] = 0;
        xArrayNeedUpdate[i] = 0;
        yArrayNeedInit[i] = 0;
        yArrayNeedUpdate[i] = 0;
        xPvData[i] = NULL;
        yPvData[i] = NULL;
        xPv[i] = NULL;
        yPv[i] = NULL;
        xEv[i] = NULL;
        yEv[i] = NULL;
      }

      printf( "numTraces = %-d\n", numTraces );

      for ( i=0; i<numTraces; i++ ) {

        argRec[i].objPtr = (void *) this;
        argRec[i].index = i;

        connection.addPv();

	stat = ca_search_and_connect( yPvExpStr[i].getExpanded(),
         &yPv[i], yMonitorConnection, &argRec[i] );
        if ( stat != ECA_NORMAL ) {
          printf( "ca_search_and_connect failed for [%s]\n",
           yPvExpStr[i].getExpanded() );
        }

      }

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

int xyGraphClass::deactivate (
  int pass
) {

int i, stat;

  switch ( pass ) {

  case 1:

    for ( i=0; i<numTraces; i++ ) {

      if ( xEv[i] ) {
        stat = ca_clear_channel( xPv[i] );
        xEv[i] = NULL;
      }

      if ( yEv[i] ) {
        stat = ca_clear_channel( yPv[i] );
        yEv[i] = NULL;
      }

      if ( xPvData[i] ) {
        delete (double *) xPvData[i];
        xPvData[i] = NULL;
      }

      if ( yPvData[i] ) {
        delete (double *) yPvData[i];
        yPvData[i] = NULL;
      }

    }

  }

  return 1;

}

void xyGraphClass::updateDimensions ( void )
{

}

void xyGraphClass::btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

}

void xyGraphClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
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

int i, ii, stat, nc, ni, nu, nr, ne, nd;

  if ( actWin->isIconified ) return;

  printf( "executeDeferred\n" );

  actWin->appCtx->proc->lock();
  nc = needConnect; needConnect = 0;
  ni = needInit; needInit = 0;
  nu = needUpdate; needUpdate = 0;
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( nc ) {

    printf( "need connect\n" );

    for ( i=0; i<numTraces; i++ ) {

      yPvType[i] = ca_field_type( yPv[i] );
      yPvCount[i] = ca_element_count( yPv[i] );
      yPvSize[i] = ca_element_count( yPv[i] ) * sizeof(double);

      argRec[i].objPtr = (void *) this;
      argRec[i].index = i;

      stat = ca_get_callback( DBR_GR_DOUBLE, yPv[i],
       yInfoUpdate, (void *) argRec );
      if ( stat != ECA_NORMAL ) {
        printf( "error from ca_get_callback\n" );
      }

    }

  }

  if ( ni ) {

    printf( "need init\n" );

    for ( i=0; i<numTraces; i++ ) {

      if ( yArrayNeedInit[i] ) {

        yArrayNeedInit[i] = 0;

        argRec[i].objPtr = (void *) this;
        argRec[i].index = i;

        if ( !yPvData[i] ) {
          printf( "count = %-d\n", yPvCount[i] );
          yPvData[i] = (void *) new double[yPvCount[i]];
          for ( ii=0; ii<yPvCount[i]; ii++ ) {
            ( (double *) yPvData[i])[ii] = 0.0;
	  }
        }

        stat = ca_add_array_event( DBR_DOUBLE, yPvCount[i], yPv[i],
         yValueUpdate, (void *) argRec, 0.0, 0.0, 0.0, &yEv[i] );
        if ( stat != ECA_NORMAL ) {
          printf( "error from ca_add_array_event\n" );
        }

      }

    }

  }

  if ( nu ) {

    printf( "need update\n" );

    for ( i=0; i<numTraces; i++ ) {

      if ( yArrayNeedUpdate[i] ) {

        yArrayNeedUpdate[i] = 0;

        printf( "\n\nupdate y %-d\n", i );

        for ( ii=0; ii<yPvCount[i]; ii++ ) {
          printf( "%d: %-g\n", ii, ( (double *) yPvData[i])[ii] );
	}

      }

    }

  }

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
