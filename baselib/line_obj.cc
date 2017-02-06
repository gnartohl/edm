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

#define __line_obj_cc 1

#include "line_obj.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void doBlink (
  void *ptr
) {

activeLineClass *alo = (activeLineClass *) ptr;

  if ( !alo->activeMode ) {
    if ( alo->isSelected() ) alo->drawSelectBoxCorners(); // erase via xor
    alo->smartDrawAll();
    if ( alo->isSelected() ) alo->drawSelectBoxCorners();
  }
  else {
    alo->bufInvalidate();
    alo->smartDrawAllActive();
  }

}

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeLineClass *alo = (activeLineClass *) client;

  if ( !alo->init ) {
    alo->needToDrawUnconnected = 1;
    alo->needRefresh = 1;
    alo->actWin->addDefExeNode( alo->aglPtr );
  }

  alo->unconnectedTimer = 0;

}

class undoLineOpClass : public undoOpClass {

public:

int n;
int *x;
int *y;

undoLineOpClass ()
{

  fprintf( stderr, "undoLineOpClass::undoLineOpClass\n" );
  n = 0;

}

undoLineOpClass (
  int _n,
  XPoint *_xpoints
) {

int i;

  n = _n;
  x = new int[n];
  y = new int[n];

  for ( i=0; i<n; i++ ) {
    x[i] = _xpoints[i].x;
    y[i] = _xpoints[i].y;
  }

}

~undoLineOpClass ()
{

  delete[] x;
  x = NULL;
  delete[] y;
  y = NULL;
  n = 0;

}

};

static void alc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeLineClass *alo = (activeLineClass *) client;

  alo->actWin->setChanged();

  alo->eraseSelectBoxCorners();
  alo->erase();

  alo->lineColorMode = alo->eBuf->bufLineColorMode;
  if ( alo->lineColorMode == ALC_K_COLORMODE_ALARM )
    alo->lineColor.setAlarmSensitive();
  else
    alo->lineColor.setAlarmInsensitive();
  alo->lineColor.setColorIndex( alo->eBuf->bufLineColor, alo->actWin->ci );

  alo->fill = alo->eBuf->bufFill;

  alo->fillColorMode = alo->eBuf->bufFillColorMode;
  if ( alo->fillColorMode == ALC_K_COLORMODE_ALARM )
    alo->fillColor.setAlarmSensitive();
  else
    alo->fillColor.setAlarmInsensitive();
  alo->fillColor.setColorIndex( alo->eBuf->bufFillColor, alo->actWin->ci );

  alo->lineWidth = alo->eBuf->bufLineWidth;

  if ( alo->eBuf->bufLineStyle == 0 )
    alo->lineStyle = LineSolid;
  else if ( alo->eBuf->bufLineStyle == 1 )
    alo->lineStyle = LineOnOffDash;

  alo->alarmPvExpStr.setRaw( alo->eBuf->bufAlarmPvName );

  alo->visPvExpStr.setRaw( alo->eBuf->bufVisPvName );

  if ( alo->eBuf->bufVisInverted )
    alo->visInverted = 0;
  else
    alo->visInverted = 1;

  strncpy( alo->minVisString, alo->eBuf->bufMinVisString, 39 );
  strncpy( alo->maxVisString, alo->eBuf->bufMaxVisString, 39 );

  alo->closePolygon = alo->eBuf->bufClosePolygon;

  alo->arrows = alo->eBuf->bufArrows;

  alo->x = alo->eBuf->bufX;
  alo->sboxX = alo->eBuf->bufX;

  alo->y = alo->eBuf->bufY;
  alo->sboxY = alo->eBuf->bufY;

  alo->w = alo->eBuf->bufW;
  alo->sboxW = alo->eBuf->bufW;

  alo->h = alo->eBuf->bufH;
  alo->sboxH = alo->eBuf->bufH;

  alo->updateDimensions();

}

static void alc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeLineClass *alo = (activeLineClass *) client;

  alc_edit_update( w, client, call );
  alo->refresh( alo );

}

static void alc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int oneX, oneY, oneW, oneH;
activeLineClass *alo = (activeLineClass *) client;
pointPtr cur;

  alo->actWin->drawGc.saveFg();
  alo->actWin->drawGc.setFG( alo->lineColor.pixelColor() );

  oneW = oneH = alo->ctlBoxLen();

  cur = alo->head->flink;
  while ( cur != alo->head ) {

    oneX = cur->x;
    oneY = cur->y;

    alo->actWin->drawGc.setLineStyle( LineSolid );
    alo->actWin->drawGc.setLineWidth( 1 );

    XDrawRectangle( alo->actWin->display(),
     XtWindow(alo->actWin->drawWidgetId()),
     alo->actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

    alo->actWin->drawGc.setLineStyle( alo->lineStyle );
    alo->actWin->drawGc.setLineWidth( alo->lineWidth );

    if ( cur->blink != alo->head ) {
      XDrawLine( alo->actWin->display(),
       XtWindow(alo->actWin->drawWidgetId()),
       alo->actWin->drawGc.xorGC(), cur->blink->x, cur->blink->y,
       cur->x, cur->y );
    }

    cur = cur->flink;

  }

  alc_edit_update( w, client, call );
  alo->ef.popdown();

  alo->actWin->drawGc.setFG( alo->lineColor.pixelColor() );

  cur = alo->head->flink;
  while ( cur != alo->head ) {

    oneX = cur->x;
    oneY = cur->y;

    alo->actWin->drawGc.setLineStyle( LineSolid );
    alo->actWin->drawGc.setLineWidth( 1 );

    XDrawRectangle( alo->actWin->display(),
     XtWindow(alo->actWin->drawWidgetId()),
     alo->actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

    alo->actWin->drawGc.setLineStyle( alo->lineStyle );
    alo->actWin->drawGc.setLineWidth( alo->lineWidth );

    if ( cur->blink != alo->head ) {
      XDrawLine( alo->actWin->display(),
       XtWindow(alo->actWin->drawWidgetId()),
       alo->actWin->drawGc.xorGC(), cur->blink->x, cur->blink->y,
       cur->x, cur->y );
    }

    cur = cur->flink;

  }

  alo->actWin->drawGc.restoreFg();
  alo->actWin->drawGc.setLineStyle( LineSolid );
  alo->actWin->drawGc.setLineWidth( 1 );

  alo->actWin->setCurrentPointObject( alo );

  alo->lineCreateBegin();

}

static void alc_edit_prop_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeLineClass *alo = (activeLineClass *) client;

  alc_edit_update( w, client, call );
  alo->ef.popdown();
  alo->operationComplete();

}

static void alc_edit_prop_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeLineClass *alo = (activeLineClass *) client;

  alo->ef.popdown();
  alo->operationCancel();

}

void alc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeLineClass *alo = (activeLineClass *) client;

  alo->ef.popdown();
  alo->operationCancel();
  alo->erase();
  alo->deleteRequest = 1;
  alo->drawAll();

}

void activeLineClass::alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeLineClass *alo = (activeLineClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    alo->connection.setPvDisconnected( (void *) alo->alarmPvConnection );
    alo->lineColor.setDisconnected();
    alo->fillColor.setDisconnected();

    alo->actWin->appCtx->proc->lock();
    alo->needRefresh = 1;
    alo->actWin->addDefExeNode( alo->aglPtr );
    alo->actWin->appCtx->proc->unlock();

  }

}

void activeLineClass::alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeLineClass *alo = (activeLineClass *) userarg;

  if ( !alo->connection.pvsConnected() ) {

    if ( pv->is_valid() ) {

      alo->connection.setPvConnected( (void *) alarmPvConnection );

      if ( alo->connection.pvsConnected() ) {
        alo->actWin->appCtx->proc->lock();
        alo->needConnectInit = 1;
        alo->actWin->addDefExeNode( alo->aglPtr );
        alo->actWin->appCtx->proc->unlock();
      }

    }

  }
  else {

    alo->actWin->appCtx->proc->lock();
    alo->needAlarmUpdate = 1;
    alo->actWin->addDefExeNode( alo->aglPtr );
    alo->actWin->appCtx->proc->unlock();

  }

}

void activeLineClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeLineClass *alo = (activeLineClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    alo->connection.setPvDisconnected( (void *) alo->visPvConnection );
    alo->lineColor.setDisconnected();
    alo->fillColor.setDisconnected();

    alo->actWin->appCtx->proc->lock();
    alo->needRefresh = 1;
    alo->actWin->addDefExeNode( alo->aglPtr );
    alo->actWin->appCtx->proc->unlock();

  }

}

void activeLineClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

activeLineClass *alo = (activeLineClass *) userarg;

  if ( !alo->connection.pvsConnected() ) {

    if ( pv->is_valid() ) {

      alo->connection.setPvConnected( (void *) visPvConnection );

      if ( alo->connection.pvsConnected() ) {
        alo->actWin->appCtx->proc->lock();
        alo->needConnectInit = 1;
        alo->actWin->addDefExeNode( alo->aglPtr );
        alo->actWin->appCtx->proc->unlock();
      }

    }

  }
  else {

    alo->actWin->appCtx->proc->lock();
    alo->needVisUpdate = 1;
    alo->actWin->addDefExeNode( alo->aglPtr );
    alo->actWin->appCtx->proc->unlock();

    }

}

activeLineClass::activeLineClass ( void ) {

  name = new char[strlen("activeLineClass")+1];
  strcpy( name, "activeLineClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  visPvExists = alarmPvExists = 0;
  activeMode = 0;
  fill = 0;
  lineColorMode = ALC_K_COLORMODE_STATIC;
  fillColorMode = ALC_K_COLORMODE_STATIC;
  lineWidth = 1;
  lineStyle = LineSolid;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  xpoints = NULL;
  closePolygon = 0;
  arrows = ARROW_NONE;

  wasSelected = 0;

  head = new pointType;
  head->flink = head;
  head->blink = head;

  connection.setMaxPvs( 2 );

  unconnectedTimer = 0;

  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

}

activeLineClass::~activeLineClass ( void ) {

  if ( name ) delete[] name;
  if ( eBuf ) delete eBuf;
  if ( head ) delete head;
  if ( xpoints ) delete[] xpoints;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  updateBlink( 0 );

}

// copy constructor
activeLineClass::activeLineClass
( const activeLineClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
int i;

//  fprintf( stderr, "In copy constructor\n" );

  ago->clone( (activeGraphicClass *) source );

  oldX = x;
  oldY = y;
  oldW = w;
  oldH = h;

  name = new char[strlen("activeLineClass")+1];
  strcpy( name, "activeLineClass" );

  lineColor.copy(source->lineColor);
  lineColorMode = source->lineColorMode;

  fill = source->fill;

  fillColor.copy(source->fillColor);
  fillColorMode = source->fillColorMode;

  visInverted = source->visInverted;

  alarmPvExpStr.setRaw( source->alarmPvExpStr.rawString );
  visPvExpStr.setRaw( source->visPvExpStr.rawString );

  visibility = 0;
  prevVisibility = -1;
  visPvExists = alarmPvExists = 0;
  activeMode = 0;

  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

  head = new pointType;
  head->flink = head;
  head->blink = head;

  numPoints = source->numPoints;
  xpoints = new XPoint[source->numPoints+1];

  for ( i=0; i<numPoints; i++ ) {
    xpoints[i].x = source->xpoints[i].x;
    xpoints[i].y = source->xpoints[i].y;
  }

  capStyle = source->capStyle;
  joinStyle = source->joinStyle;
  lineStyle = source->lineStyle;
  lineWidth = source->lineWidth;
  closePolygon = source->closePolygon;
  arrows = source->arrows;

  wasSelected = 0;

  connection.setMaxPvs( 2 );

  unconnectedTimer = 0;

  eBuf = NULL;

  setBlinkFunction( (void *) doBlink );

  doAccSubs( alarmPvExpStr );
  doAccSubs( visPvExpStr );
  doAccSubs( minVisString, 39 );
  doAccSubs( maxVisString, 39 );

}

int activeLineClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

//   fprintf( stderr, "In activeLineClass::createInteractive\n" );

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  oldX = x;
  oldY = y;
  oldW = w;
  oldH = h;

  numPoints = 0;
  xpoints = (XPoint *) NULL;

  lineColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  fillColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  this->editCreate();

  return 1;

}

int activeLineClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeLineClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeLineClass_str4, 31 );

  Strncat( title, activeLineClass_str5, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufLineColor = lineColor.pixelIndex();
  eBuf->bufLineColorMode = lineColorMode;

  eBuf->bufFill = fill;

  eBuf->bufFillColor = fillColor.pixelIndex();
  eBuf->bufFillColorMode = fillColorMode;

  eBuf->bufLineWidth = lineWidth;

  if ( lineStyle == LineSolid )
    eBuf->bufLineStyle = 0;
  else if ( lineStyle == LineOnOffDash )
    eBuf->bufLineStyle = 1;

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

  eBuf->bufArrows = arrows;
  eBuf->bufClosePolygon = closePolygon;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeLineClass_str6, 30, &eBuf->bufX );
  ef.addTextField( activeLineClass_str7, 30, &eBuf->bufY );
  ef.addTextField( activeLineClass_str8, 30, &eBuf->bufW );
  ef.addTextField( activeLineClass_str9, 30, &eBuf->bufH );
  ef.addOption( activeLineClass_str10, activeLineClass_str11, &eBuf->bufLineWidth );
  ef.addOption( activeLineClass_str12, activeLineClass_str13, &eBuf->bufLineStyle );
  ef.addOption( activeLineClass_str34, activeLineClass_str35, &eBuf->bufArrows );
  ef.addToggle( activeLineClass_str33, &eBuf->bufClosePolygon );
  ef.addColorButton( activeLineClass_str14, actWin->ci, &eBuf->lineCb,
   &eBuf->bufLineColor );
  ef.addToggle( activeLineClass_str15, &eBuf->bufLineColorMode );

  ef.addToggle( activeLineClass_str16, &eBuf->bufFill );
  fillEntry = ef.getCurItem();
  ef.addColorButton( activeLineClass_str17, actWin->ci, &eBuf->fillCb,
   &eBuf->bufFillColor );
  fillColorEntry = ef.getCurItem();
  fillEntry->addDependency( fillColorEntry );
  ef.addToggle( activeLineClass_str18, &eBuf->bufFillColorMode );
  fillAlarmSensEntry = ef.getCurItem();
  fillEntry->addDependency( fillAlarmSensEntry );
  fillEntry->addDependencyCallbacks();

  ef.addTextField( activeLineClass_str19, 30, eBuf->bufAlarmPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeLineClass_str20, 30, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  invisPvEntry = ef.getCurItem();
  ef.addOption( " ", activeLineClass_str22, &eBuf->bufVisInverted );
  visInvEntry = ef.getCurItem();
  invisPvEntry->addDependency( visInvEntry );
  ef.addTextField( activeLineClass_str23, 30, eBuf->bufMinVisString, 39 );
  minVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( minVisEntry );
  ef.addTextField( activeLineClass_str24, 30, eBuf->bufMaxVisString, 39 );
  maxVisEntry = ef.getCurItem();
  invisPvEntry->addDependency( maxVisEntry );
  invisPvEntry->addDependencyCallbacks();

  return 1;

}

int activeLineClass::editCreate ( void ) {

  this->wasSelected = 0;
  this->genericEdit();
  ef.finished( alc_edit_ok, alc_edit_apply, alc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeLineClass::edit ( void ) {

  this->genericEdit();
  ef.finished( alc_edit_prop_ok, alc_edit_apply, alc_edit_prop_cancel,
   this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeLineClass::editLineSegments ( void ) {

int i, oneX, oneY, oneW, oneH;
pointPtr cur;

// The intent of this next block of code is to get rid of the select
// box corners. The function eraseSelectBoxCorners() doesn't work if
// the object is on top of another object because the select box corners
// are merely drawn with the background color. We have to deselect the
// object, refresh the screen, and re-select the object to get rid of the
// select box corners.

  confirmEdit(); // this creates an "edit undo" object

  if ( this->isSelected() ) {
    this->wasSelected = 1;
    this->eraseSelectBoxCorners();
    this->deselect();
    this->refresh();
  }
  else {
    this->wasSelected = 0;
  }

  this->erase();
  this->actWin->refreshGrid();

  if ( this->numPoints > 0 ) {

    this->actWin->drawGc.saveFg();
    this->actWin->drawGc.setFG( this->lineColor.pixelColor() );
    oneW = oneH = ctlBoxLen();

    for ( i=0; i<this->numPoints; i++ ) {

      cur = new pointType;
      cur->x = this->xpoints[i].x;
      cur->y = this->xpoints[i].y;
      this->head->blink->flink = cur;
      cur->blink = this->head->blink;
      this->head->blink = cur;
      cur->flink = this->head;

      oneX = cur->x;
      oneY = cur->y;

      this->actWin->drawGc.setLineStyle( LineSolid );
      this->actWin->drawGc.setLineWidth( 1 );

      XDrawRectangle( this->actWin->display(),
       XtWindow(this->actWin->drawWidgetId()),
       this->actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

      this->actWin->drawGc.setLineStyle( this->lineStyle );
      this->actWin->drawGc.setLineWidth( this->lineWidth );

      if ( cur->blink != this->head ) {
        XDrawLine( this->actWin->display(),
         XtWindow(this->actWin->drawWidgetId()),
         this->actWin->drawGc.xorGC(), cur->blink->x, cur->blink->y, cur->x,
         cur->y );
      }

    }

    if ( this->numPoints > 0 ) {
      this->numPoints = 0;
      delete[] this->xpoints;
    }

  }

  this->actWin->drawGc.restoreFg();
  this->actWin->drawGc.setLineStyle( LineSolid );
  this->actWin->drawGc.setLineWidth( 1 );

  actWin->setCurrentPointObject( this );

  lineEditBegin();

  cur = head->blink;
  if ( cur != head ) {

    if ( cur->blink != head ) {
      strcpy( actWin->refPoint[0].label, "" );
      actWin->refPoint[0].x = cur->blink->x;
      actWin->refPoint[0].y = cur->blink->y;
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 2;
    }
    else {
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 1;
    }

  }
  else {

    actWin->numRefPoints = 0;

  }

  return 1;

}

int activeLineClass::insertPoint (
  int x,
  int y )
{

pointPtr cur, selectedOne;
int oneX, oneY, oneW, oneH;

  selectedOne = selectPoint( x, y );
  if ( !selectedOne ) {
    XBell( actWin->d, 50 );
    return 1;
  }

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( lineColor.pixelColor() );

  // erase old via xor gc

  actWin->drawGc.setLineStyle( this->lineStyle );
  actWin->drawGc.setLineWidth( this->lineWidth );

  if ( selectedOne->flink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), selectedOne->x, selectedOne->y,
     selectedOne->flink->x, selectedOne->flink->y );
  }

  cur = new pointType;

  if ( selectedOne->flink != head ) {
    cur->x = ( selectedOne->x + selectedOne->flink->x ) / 2;
    cur->y = ( selectedOne->y + selectedOne->flink->y ) / 2;
  }
  else {
    cur->x = selectedOne->x + 10;
    cur->y = selectedOne->y + 10;
  }

  cur->flink = selectedOne->flink;
  cur->blink = selectedOne;
  selectedOne->flink->blink = cur;
  selectedOne->flink = cur;

  // draw with new inserted

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), selectedOne->x, selectedOne->y,
   selectedOne->flink->x, selectedOne->flink->y );

  oneX = cur->x;
  oneY = cur->y;

  oneW = oneH = ctlBoxLen();

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

  actWin->drawGc.setLineStyle( this->lineStyle );
  actWin->drawGc.setLineWidth( this->lineWidth );

  if ( cur->flink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), cur->x, cur->y,
     cur->flink->x, cur->flink->y );
  }

  cur = head->blink;
  if ( cur != head ) {

    if ( cur->blink != head ) {
      strcpy( actWin->refPoint[0].label, "" );
      actWin->refPoint[0].x = cur->blink->x;
      actWin->refPoint[0].y = cur->blink->y;
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 2;
    }
    else {
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 1;
    }

  }
  else {

    actWin->numRefPoints = 0;

  }

  return 1;

}

int activeLineClass::removePoint (
  int x,
  int y )
{

pointPtr cur, selectedOne;
int oneX, oneY, oneW, oneH;

  selectedOne = selectPoint( x, y );
  if ( !selectedOne ) {
    XBell( actWin->d, 50 );
    return 1;
  }

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( lineColor.pixelColor() );

  // erase old via xor gc

  actWin->drawGc.setLineStyle( this->lineStyle );
  actWin->drawGc.setLineWidth( this->lineWidth );

  if ( selectedOne->blink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), selectedOne->blink->x, selectedOne->blink->y,
     selectedOne->x, selectedOne->y );
  }

  oneX = selectedOne->x;
  oneY = selectedOne->y;

  oneW = oneH = ctlBoxLen();

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

  actWin->drawGc.setLineStyle( this->lineStyle );
  actWin->drawGc.setLineWidth( this->lineWidth );

  if ( selectedOne->flink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), selectedOne->x, selectedOne->y,
     selectedOne->flink->x, selectedOne->flink->y );
  }

  // unlink
  selectedOne->blink->flink = selectedOne->flink;
  selectedOne->flink->blink = selectedOne->blink;

  cur = selectedOne->blink;
  delete selectedOne;

  // draw with selected removed

  if ( ( cur != head ) && ( cur->flink != head ) ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), cur->x, cur->y,
     cur->flink->x, cur->flink->y );
  }

  actWin->drawGc.restoreFg();

  cur = head->blink;
  if ( cur != head ) {

    if ( cur->blink != head ) {
      strcpy( actWin->refPoint[0].label, "" );
      actWin->refPoint[0].x = cur->blink->x;
      actWin->refPoint[0].y = cur->blink->y;
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 2;
    }
    else {
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 1;
    }

  }
  else {

    actWin->numRefPoints = 0;

  }

  return 1;

}

int activeLineClass::addPoint (
  int oneX,
  int oneY )
{

pointPtr cur;
int oneW, oneH;

  cur = new pointType;

  head->blink->flink = cur;
  cur->blink = head->blink;
  head->blink = cur;
  cur->flink = head;

  if ( actWin->orthogonal ) {
    if ( cur->blink != head ) {
      if ( abs( oneX - cur->blink->x ) >= abs( oneY - cur->blink->y ) )
        oneY = cur->blink->y;
      else
        oneX = cur->blink->x;
    }
  }

  cur->x = oneX;
  cur->y = oneY;

  oneW = oneH = ctlBoxLen();

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( lineColor.pixelColor() );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

  actWin->drawGc.setLineStyle( this->lineStyle );
  actWin->drawGc.setLineWidth( this->lineWidth );

  if ( cur->blink != head ) {

    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), cur->blink->x, cur->blink->y, cur->x, cur->y );

  }

  actWin->drawGc.restoreFg();
  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  cur = head->blink;
  if ( cur != head ) {

    if ( cur->blink != head ) {
      strcpy( actWin->refPoint[0].label, "" );
      actWin->refPoint[0].x = cur->blink->x;
      actWin->refPoint[0].y = cur->blink->y;
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 2;
    }
    else {
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 1;
    }

  }
  else {

    actWin->numRefPoints = 0;

  }

  return 1;

}

int activeLineClass::removeLastPoint ( void )
{

pointPtr cur;
int oneX, oneY, oneW, oneH;

//   fprintf( stderr, "In activeLineClass::removeLastPoint\n" );

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( lineColor.pixelColor() );

  cur = head->blink;
  if ( cur == head ) return 0;

  oneX = cur->x;
  oneY = cur->y;
  oneW = oneH = ctlBoxLen();

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  // erase old via xor gc
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

  actWin->drawGc.setLineStyle( this->lineStyle );
  actWin->drawGc.setLineWidth( this->lineWidth );

  if ( cur->blink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), cur->blink->x, cur->blink->y, cur->x, cur->y );
  }

  // unlink
  cur->blink->flink = head;
  head->blink = cur->blink;

  delete cur;

  actWin->drawGc.restoreFg();
  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  this->actWin->refreshGrid();

  cur = head->blink;
  if ( cur != head ) {

    if ( cur->blink != head ) {
      strcpy( actWin->refPoint[0].label, "" );
      actWin->refPoint[0].x = cur->blink->x;
      actWin->refPoint[0].y = cur->blink->y;
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 2;
    }
    else {
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 1;
    }

  }
  else {

    actWin->numRefPoints = 0;

  }

  return 1;

}

// override base class select for this object

int activeLineClass::select (
  int _x,
  int _y )
{

int effectiveW, effectiveH, small;

  if ( ( w < 5 ) && ( h < 5 ) )
    small = 1;
  else
    small = 0;

  if ( w < 5 )
    effectiveW = 5;
  else
    effectiveW = w;

  if ( h < 5 )
    effectiveH = 5;
  else
    effectiveH = h;

  if ( deleteRequest ) return 0;

  if ( small ) {
    if ( ( _x >= x-effectiveW ) && ( _x <= x+effectiveW ) &&
         ( _y >= y-effectiveH ) && ( _y <= y+effectiveH ) ) {
      selected = 1;
      return 1;
    }
    else {
      return 0;
    }
  }
  else {
    if ( ( _x >= x ) && ( _x <= x+effectiveW ) &&
         ( _y >= y ) && ( _y <= y+effectiveH ) ) {
      selected = 1;
      return 1;
    }
    else {
      return 0;
    }
  }

}

pointPtr activeLineClass::selectPoint (
  int x,
  int y )
{

pointPtr cur, prev, next;
int i, d, lw, baseThreshold, threshold;

  if ( lineWidth > 0 ) {
    lw = lineWidth;
  }
  else {
    lw = 1;
  }
  baseThreshold = 3;

  // try with up to 4 threshold values
  for ( i=0; i<4; i++ ) {

    threshold = baseThreshold * baseThreshold;
    baseThreshold += 3;

    cur = head->flink;
    while ( cur != head ) {

      d = ( cur->x - x ) * ( cur->x - x ) + ( cur->y - y ) * ( cur->y - y );
      if ( 2*d <= threshold ) {

        prev = cur->blink;
        next = cur->flink;
        if ( prev != head ) {

          if ( prev->blink != head ) {
            strcpy( actWin->refPoint[0].label, "" );
            actWin->refPoint[0].x = prev->blink->x;
            actWin->refPoint[0].y = prev->blink->y;
            strcpy( actWin->refPoint[1].label, "Prev Vertex" );
            actWin->refPoint[1].x = prev->x;
            actWin->refPoint[1].y = prev->y;
            actWin->numRefPoints = 2;
          }
          else {
            strcpy( actWin->refPoint[1].label, "Prev Vertex" );
            actWin->refPoint[1].x = prev->x;
            actWin->refPoint[1].y = prev->y;
            actWin->numRefPoints = 1;
          }

        }
        else if ( next != head ) {

          if ( next->flink != head ) {
            strcpy( actWin->refPoint[0].label, "" );
            actWin->refPoint[0].x = next->flink->x;
            actWin->refPoint[0].y = next->flink->y;
            strcpy( actWin->refPoint[1].label, "Next Vertex" );
            actWin->refPoint[1].x = next->x;
            actWin->refPoint[1].y = next->y;
            actWin->numRefPoints = 2;
          }
          else {
            strcpy( actWin->refPoint[1].label, "Next Vertex" );
            actWin->refPoint[1].x = next->x;
            actWin->refPoint[1].y = next->y;
            actWin->numRefPoints = 1;
          }

        }
        else {

          actWin->numRefPoints = 0;

        }

        return cur;

      }

      cur = cur->flink;

    }

  }
  
  return (pointPtr) NULL;

}

void activeLineClass::deselectAllPoints ( void ) {

pointPtr cur;

  cur = head->blink;
  if ( cur != head ) {

    if ( cur->blink != head ) {
      strcpy( actWin->refPoint[0].label, "" );
      actWin->refPoint[0].x = cur->blink->x;
      actWin->refPoint[0].y = cur->blink->y;
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 2;
    }
    else {
      strcpy( actWin->refPoint[1].label, "Prev Vertex" );
      actWin->refPoint[1].x = cur->x;
      actWin->refPoint[1].y = cur->y;
      actWin->numRefPoints = 1;
    }

  }
  else {

    actWin->numRefPoints = 0;

  }

}

int activeLineClass::movePoint (
  pointPtr curPoint,
  int _x,
  int _y )
{

int oneX, oneY, oneW, oneH;

//   fprintf( stderr, "In activeLineClass::movePoint\n" );
//   fprintf( stderr, "x = %-d, y = %-d\n", x, y );

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( lineColor.pixelColor() );

  oneX = curPoint->x;
  oneY = curPoint->y;

  oneW = oneH = ctlBoxLen();

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  // erase old via xor gc
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

  actWin->drawGc.setLineStyle( this->lineStyle );
  actWin->drawGc.setLineWidth( this->lineWidth );

  if ( curPoint->blink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), curPoint->blink->x, curPoint->blink->y,
     curPoint->x, curPoint->y );
  }

  if ( curPoint->flink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), curPoint->x, curPoint->y,
     curPoint->flink->x, curPoint->flink->y );
  }

  if ( actWin->orthogonal ) {
    if ( curPoint->blink != head ) {
      if ( abs( oneX - curPoint->blink->x ) >=
           abs( oneY - curPoint->blink->y ) )
        _y = curPoint->blink->y;
      else
        _x = curPoint->blink->x;
    }
  }

  curPoint->x = _x;
  curPoint->y = _y;

  oneX = curPoint->x;
  oneY = curPoint->y;

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  // draw new
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

  actWin->drawGc.setLineStyle( this->lineStyle );
  actWin->drawGc.setLineWidth( this->lineWidth );

  if ( curPoint->blink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), curPoint->blink->x, curPoint->blink->y,
     curPoint->x, curPoint->y );
  }

  if ( curPoint->flink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), curPoint->x, curPoint->y,
     curPoint->flink->x, curPoint->flink->y );
  }

  actWin->drawGc.restoreFg();
  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  this->actWin->refreshGrid();

  return 1;

}

int activeLineClass::movePointRel (
  pointPtr curPoint,
  int _xofs,
  int _yofs )
{

int oneX, oneY, oneW, oneH;

//   fprintf( stderr, "In activeLineClass::movePoint\n" );
//   fprintf( stderr, "x = %-d, y = %-d\n", x, y );

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( lineColor.pixelColor() );

  oneX = curPoint->x;
  oneY = curPoint->y;

  oneW = oneH = ctlBoxLen();

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  // erase old via xor gc
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

  actWin->drawGc.setLineStyle( this->lineStyle );
  actWin->drawGc.setLineWidth( this->lineWidth );

  if ( curPoint->blink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), curPoint->blink->x, curPoint->blink->y,
     curPoint->x, curPoint->y );
  }

  if ( curPoint->flink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), curPoint->x, curPoint->y,
     curPoint->flink->x, curPoint->flink->y );
  }

  if ( actWin->orthogonal ) {
    if ( curPoint->blink != head ) {
      if ( abs( oneX - curPoint->blink->x ) >=
           abs( oneY - curPoint->blink->y ) )
        _yofs = 0;
      else
        _xofs = 0;
    }
  }

  curPoint->x += _xofs;
  curPoint->y += _yofs;

  oneX = curPoint->x;
  oneY = curPoint->y;

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  // draw new
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

  actWin->drawGc.setLineStyle( this->lineStyle );
  actWin->drawGc.setLineWidth( this->lineWidth );

  if ( curPoint->blink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), curPoint->blink->x, curPoint->blink->y,
     curPoint->x, curPoint->y );
  }

  if ( curPoint->flink != head ) {
    XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), curPoint->x, curPoint->y,
     curPoint->flink->x, curPoint->flink->y );
  }

  actWin->drawGc.restoreFg();
  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  this->actWin->refreshGrid();

  return 1;

}

int activeLineClass::lineEditComplete ( void )
{

int stat;

  stat = lineEditDone();
  this->operationComplete();

  return stat;

}

int activeLineClass::lineEditCancel ( void )
{

int stat;

  stat = lineEditDone();
  this->operationCancel();

  return stat;

}

int activeLineClass::lineEditDone ( void )
{

pointPtr cur, next;
int n, oneX, oneY, oneW, oneH, minX, minY, maxX, maxY;

  oneW = oneH = ctlBoxLen();

//   fprintf( stderr, "In activeLineClass::lineEditComplete\n" );

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( lineColor.pixelColor() );

  // erase all points, count number of points, and find rectangular
  // extent
  minX = minY = 0x7fffffff;
  maxX = maxY = -1;
  n = 0;
  cur = head->flink;
  while ( cur != head ) {

    n++;
    numPoints++;
    oneX = cur->x;
    oneY = cur->y;

    if ( minX > oneX ) minX = oneX;
    if ( minY > oneY ) minY = oneY;
    if ( maxX < oneX ) maxX = oneX;
    if ( maxY < oneY ) maxY = oneY;

    actWin->drawGc.setLineStyle( LineSolid );
    actWin->drawGc.setLineWidth( 1 );

    // erase current rectangle via xor gc
    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.xorGC(), oneX-oneW/2, oneY-oneH/2, oneW, oneH );

    actWin->drawGc.setLineStyle( this->lineStyle );
    actWin->drawGc.setLineWidth( this->lineWidth );

    if ( cur->blink != head ) {
      XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.xorGC(), cur->blink->x, cur->blink->y, cur->x, cur->y );
    }

    cur = cur->flink;

  }

  if ( n ) {

    // set select box size
    oneW = maxX - minX;
    oneH = maxY - minY;

    x = minX;
    y = minY;
    w = oneW;
    h = oneH;

  }
  else {

    // set select box size
    oneW = 2;
    oneH = 2;
    w = oneW;
    h = oneH;

  }

  oldX = x;
  oldY = y;
  oldW = w;
  oldH = h;

//   fprintf( stderr, "minX=%-d, minY=%-d, w=%-d, h=%-d\n", x, y, w, h );

  initSelectBox();

  // build XPoint array
  xpoints = new XPoint[numPoints+1];
  n = 0;
  cur = head->flink;
  while ( cur != head ) {

    next = cur->flink;

    xpoints[n].x = cur->x;
    xpoints[n].y = cur->y;
    n++;

    delete cur;

    cur = next;

  }

  head->flink = head;
  head->blink = head;

  actWin->drawGc.setLineStyle( lineStyle );
  actWin->drawGc.setLineWidth( lineWidth );

  if ( fill ) {

    actWin->drawGc.setFG( fillColor.pixelColor() );

    XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), xpoints, numPoints, Complex, CoordModeOrigin );

  }

  actWin->drawGc.setFG( lineColor.pixelColor() );

  XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), xpoints, numPoints, CoordModeOrigin );

  actWin->drawGc.restoreFg();
  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  if ( this->wasSelected ) {
    this->setSelected();
  }

  this->refresh();

  actWin->numRefPoints = 0;

  return 1;

}

int activeLineClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

int i, *xArray, xSize, *yArray, ySize;

tagClass tag;

int zero = 0;
int one = 1;
static char *emptyStr = "";

int solid = LineSolid;
static char *styleEnumStr[2] = {
  "solid",
  "dash"
};
static int styleEnum[2] = {
  LineSolid,
  LineOnOffDash
};

int arrowsNone = 0;
static char *arrowsEnumStr[4] = {
  "none",
  "from",
  "to",
  "both"
};
static int arrowsEnum[4] = {
  0,
  1,
  2,
  3
};

  this->actWin = _actWin;

  // read file and process each "object" tag
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
  tag.loadR( "lineColor", actWin->ci, &lineColor );
  tag.loadR( "lineAlarm", &lineColorMode, &zero );
  tag.loadR( "fill", &fill, &zero );
  tag.loadR( "fillColor", actWin->ci, &fillColor );
  tag.loadR( "fillAlarm", &fillColorMode, &zero );
  tag.loadR( "lineWidth", &lineWidth, &one );
  tag.loadR( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &solid );
  tag.loadR( "alarmPv", &alarmPvExpStr, emptyStr );
  tag.loadR( "visPv", &visPvExpStr, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "closePolygon", &closePolygon, &zero );
  tag.loadR( "arrows", 4, arrowsEnumStr, arrowsEnum, &arrows, &arrowsNone );
  tag.loadR( "numPoints", &numPoints, &zero );
  tag.loadR( "xPoints", &xArray, &xSize );
  tag.loadR( "yPoints", &yArray, &ySize );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > ALC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  oldX = x;
  oldY = y;
  oldW = w;
  oldH = h;

  this->initSelectBox(); // call after getting x,y,w,h

  if ( numPoints == 0 ) {
    if ( xSize < ySize ) {
      numPoints = xSize;
    }
    else {
      numPoints = ySize;
    }
  }

  xpoints = new XPoint[numPoints+1];

  if ( xpoints ) {

    for ( i=0; i<numPoints; i++ ) {
      xpoints[i].x = (short) xArray[i];
      xpoints[i].y = (short) yArray[i];
    }

  }
  else {

    numPoints = 0;

  }

  delete[] xArray;
  delete[] yArray;

  if ( lineColorMode == ALC_K_COLORMODE_ALARM )
    lineColor.setAlarmSensitive();
  else
    lineColor.setAlarmInsensitive();

  if ( fillColorMode == ALC_K_COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  return stat;

  this->wasSelected = 0;

  return stat;

}

int activeLineClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, r, g, b, oneX, oneY, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > ALC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  oldX = x;
  oldY = y;
  oldW = w;
  oldH = h;

  this->initSelectBox(); // call after getting x,y,w,h

  fscanf( f, "%d\n", &numPoints ); actWin->incLine();

  xpoints = new XPoint[numPoints+1];

  for ( i=0; i<numPoints; i++ ) {
    fscanf( f, "%d %d\n", &oneX, &oneY ); actWin->incLine();
    xpoints[i].x = (short) oneX;
    xpoints[i].y = (short) oneY;
  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == ALC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fillColor.setColorIndex( index, actWin->ci );

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == ALC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fillColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == ALC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fillColor.setColorIndex( index, actWin->ci );

  }

  fscanf( f, "%d\n", &fillColorMode ); actWin->incLine();

  if ( fillColorMode == ALC_K_COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  fscanf( f, "%d\n", &lineWidth ); actWin->incLine();
  fscanf( f, "%d\n", &lineStyle ); actWin->incLine();

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  alarmPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  visPvExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    readStringFromFile( minVisString, 39+1, f ); actWin->incLine();
    readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();
  }
  else {
    strcpy( minVisString, "1" );
    strcpy( maxVisString, "1" );
  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {
    fscanf( f, "%d\n", &closePolygon ); actWin->incLine();
    fscanf( f, "%d\n", &arrows ); actWin->incLine();
  }
  else {
    closePolygon = 0;
    arrows = ARROW_NONE;
  }

  this->wasSelected = 0;

  return 1;

}

int activeLineClass::save (
  FILE *f )
{

int major, minor, release, stat;

int i, *xArray, *yArray;

tagClass tag;

int zero = 0;
int one = 1;
static char *emptyStr = "";

int solid = LineSolid;
static char *styleEnumStr[2] = {
  "solid",
  "dash"
};
static int styleEnum[2] = {
  LineSolid,
  LineOnOffDash
};

int arrowsNone = 0;
static char *arrowsEnumStr[4] = {
  "none",
  "from",
  "to",
  "both"
};
static int arrowsEnum[4] = {
  0,
  1,
  2,
  3
};

  major = ALC_MAJOR_VERSION;
  minor = ALC_MINOR_VERSION;
  release = ALC_RELEASE;

  // read file and process each "object" tag
  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "lineColor", actWin->ci, &lineColor );
  tag.loadBoolW( "lineAlarm", &lineColorMode, &zero );
  tag.loadBoolW( "fill", &fill, &zero );
  tag.loadW( "fillColor", actWin->ci, &fillColor );
  tag.loadBoolW( "fillAlarm", &fillColorMode, &zero );
  tag.loadW( "lineWidth", &lineWidth, &one );
  tag.loadW( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &solid );
  tag.loadW( "alarmPv", &alarmPvExpStr, emptyStr  );
  tag.loadW( "visPv", &visPvExpStr, emptyStr );
  tag.loadBoolW( "visInvert", &visInverted, &zero );
  tag.loadW( "visMin", minVisString, emptyStr );
  tag.loadW( "visMax", maxVisString, emptyStr );
  tag.loadBoolW( "closePolygon", &closePolygon, &zero );
  tag.loadW( "arrows", 4, arrowsEnumStr, arrowsEnum, &arrows, &arrowsNone );
  tag.loadW( "numPoints", &numPoints );

  xArray = new int[numPoints];
  yArray = new int[numPoints];
  for ( i=0; i<numPoints; i++ ) {
    xArray[i] = (int) xpoints[i].x;
    yArray[i] = (int) xpoints[i].y;
  }
  tag.loadW( "xPoints", xArray, numPoints );
  tag.loadW( "yPoints", yArray, numPoints );

  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  delete[] xArray;
  delete[] yArray;

  return stat;

}

int activeLineClass::old_save (
  FILE *f )
{

int i, index;

  fprintf( f, "%-d %-d %-d\n", ALC_MAJOR_VERSION, ALC_MINOR_VERSION,
   ALC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  fprintf( f, "%-d\n", numPoints );

  for ( i=0; i<numPoints; i++ ) {
    fprintf( f, "%-d %-d\n", xpoints[i].x, xpoints[i].y );
  }

  index = lineColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", lineColorMode );

  fprintf( f, "%-d\n", fill );

  index =  fillColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fillColorMode );

  fprintf( f, "%-d\n", lineWidth );
  fprintf( f, "%-d\n", lineStyle );

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

  // ver 2.2.0
  fprintf( f, "%-d\n", closePolygon );
  fprintf( f, "%-d\n", arrows );

  return 1;

}

int activeLineClass::drawActiveIfIntersects (
  int x0,
  int y0,
  int x1,
  int y1 ) {

int delta = lineWidth/2;

  if ( arrows != ARROW_NONE ) {
    delta += 6;
  }

  if ( intersects( x0-delta, y0-delta, x1+delta, y1+delta ) ) {
    bufInvalidate();
    drawActive();
  }

  return 1;

}

int activeLineClass::drawActive ( void )
{

int n, drawArrows = ARROW_NONE, blink = 0;
XPoint arrowXPoints[8];

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( lineColor.getDisconnectedIndex(), &blink );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
      updateBlink( blink );
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
  }

  if ( !enabled || !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  if ( ( numPoints > 1 ) && ( arrows != ARROW_NONE ) ) {
    drawArrows = arrows;
    getArrowCoords( drawArrows, arrowXPoints );
  }

  if ( ( numPoints > 2 ) && closePolygon ) {
    xpoints[numPoints].x = xpoints[0].x;
    xpoints[numPoints].y = xpoints[0].y;
    n = numPoints + 1;
  }
  else {
    n = numPoints;
  }

  if ( n > 0 ) {

    actWin->executeGc.setLineStyle( lineStyle );
    actWin->executeGc.setLineWidth( lineWidth );

    actWin->executeGc.saveFg();

    if ( fill && fillVisibility ) {

      //actWin->executeGc.setFG( fillColor.getColor() );
      actWin->executeGc.setFG( fillColor.getIndex(), &blink );

      XFillPolygon( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), xpoints, n, Complex,
       CoordModeOrigin );

    }

    if ( lineVisibility ) {

      //actWin->executeGc.setFG( lineColor.getColor() );
      actWin->executeGc.setFG( lineColor.getIndex(), &blink );

      XDrawLines( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), xpoints, n, CoordModeOrigin );

      if ( ( drawArrows == ARROW_FROM ) || ( drawArrows == ARROW_BOTH ) ) {
        actWin->executeGc.setLineStyle( LineSolid );
        XFillPolygon( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), arrowXPoints, 4, Complex,
         CoordModeOrigin );
        XDrawLines( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), arrowXPoints, 4, CoordModeOrigin );
        actWin->executeGc.setLineStyle( lineStyle );
      }
      if ( ( drawArrows == ARROW_TO ) || ( drawArrows == ARROW_BOTH ) ) {
        actWin->executeGc.setLineStyle( LineSolid );
        XFillPolygon( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), &arrowXPoints[4], 4, Complex,
         CoordModeOrigin );
        XDrawLines( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), &arrowXPoints[4], 4, CoordModeOrigin );
        actWin->executeGc.setLineStyle( lineStyle );
      }

    }

    actWin->executeGc.restoreFg();
    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setLineWidth( 1 );

  }

  updateBlink( blink );

  return 1;

}

int activeLineClass::eraseUnconditional ( void )
{
 
int n, drawArrows = ARROW_NONE;
XPoint arrowXPoints[8];

  if ( !enabled ) return 1;

  if ( ( numPoints > 1 ) && ( arrows != ARROW_NONE ) ) {
    drawArrows = arrows;
    getArrowCoords( drawArrows, arrowXPoints );
  }

  if ( ( numPoints > 2 ) && closePolygon ) {
    xpoints[numPoints].x = xpoints[0].x;
    xpoints[numPoints].y = xpoints[0].y;
    n = numPoints + 1;
  }
  else {
    n = numPoints;
  }

  if ( n > 0 ) {

    actWin->executeGc.setLineStyle( lineStyle );
    actWin->executeGc.setLineWidth( lineWidth );

    if ( fill ) {

      XFillPolygon( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), xpoints, n, Complex,
       CoordModeOrigin );

    }

    XDrawLines( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), xpoints, n, CoordModeOrigin );

    if ( ( drawArrows == ARROW_FROM ) || ( drawArrows == ARROW_BOTH ) ) {
      actWin->executeGc.setLineStyle( LineSolid );
      XFillPolygon( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), arrowXPoints, 4, Complex,
       CoordModeOrigin );
      XDrawLines( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), arrowXPoints, 4, CoordModeOrigin );
      actWin->executeGc.setLineStyle( lineStyle );
    }
    if ( ( drawArrows == ARROW_TO ) || ( drawArrows == ARROW_BOTH ) ) {
      actWin->executeGc.setLineStyle( LineSolid );
      XFillPolygon( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), &arrowXPoints[4], 4, Complex,
       CoordModeOrigin );
      XDrawLines( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), &arrowXPoints[4], 4, CoordModeOrigin );
      actWin->executeGc.setLineStyle( lineStyle );
    }

    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setLineWidth( 1 );

  }

  return 1;

}

int activeLineClass::eraseActive ( void )
{

int n, drawArrows = ARROW_NONE;
XPoint arrowXPoints[8];

  if ( !enabled || !activeMode ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  if ( ( numPoints > 1 ) && ( arrows != ARROW_NONE ) ) {
    drawArrows = arrows;
    getArrowCoords( drawArrows, arrowXPoints );
  }

  if ( ( numPoints > 2 ) && closePolygon ) {
    xpoints[numPoints].x = xpoints[0].x;
    xpoints[numPoints].y = xpoints[0].y;
    n = numPoints + 1;
  }
  else {
    n = numPoints;
  }

  if ( n > 0 ) {

    actWin->executeGc.setLineStyle( lineStyle );
    actWin->executeGc.setLineWidth( lineWidth );

    if ( fill ) {

      XFillPolygon( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), xpoints, n, Complex,
       CoordModeOrigin );

    }

    XDrawLines( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), xpoints, n, CoordModeOrigin );

    if ( ( drawArrows == ARROW_FROM ) || ( drawArrows == ARROW_BOTH ) ) {
      actWin->executeGc.setLineStyle( LineSolid );
      XFillPolygon( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), arrowXPoints, 4, Complex,
       CoordModeOrigin );
      XDrawLines( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), arrowXPoints, 4, CoordModeOrigin );
      actWin->executeGc.setLineStyle( lineStyle );
    }
    if ( ( drawArrows == ARROW_TO ) || ( drawArrows == ARROW_BOTH ) ) {
      actWin->executeGc.setLineStyle( LineSolid );
      XFillPolygon( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), &arrowXPoints[4], 4, Complex,
       CoordModeOrigin );
      XDrawLines( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.eraseGC(), &arrowXPoints[4], 4, CoordModeOrigin );
      actWin->executeGc.setLineStyle( lineStyle );
    }

    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setLineWidth( 1 );

  }

  return 1;

}

int activeLineClass::expandTemplate (
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

  return 1;

}

int activeLineClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand1st( numMacros, macros, expansions );
  stat = visPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeLineClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = visPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeLineClass::containsMacros ( void ) {

  if ( alarmPvExpStr.containsPrimaryMacros() ) return 1;
  if ( visPvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeLineClass::activate (
  int pass,
  void *ptr )
{

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

      connection.init();
      initEnable();

      curLineColorIndex = -1;
      curFillColorIndex = -1;
      curStatus = -1;
      curSeverity = -1;
      prevVisibility = -1;
      visibility = 0;
      prevLineVisibility = -1;
      lineVisibility = 0;
      prevFillVisibility = -1;
      fillVisibility = 0;

      needConnectInit = needAlarmUpdate = needVisUpdate = needRefresh = 0;

      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      aglPtr = ptr;

      alarmPvId = visPvId = 0;

      activeMode = 1;
      pvType = -1;

      init = 1; // this stays true if there are no pvs

      if ( !alarmPvExpStr.getExpanded() ||
           // ( strcmp( alarmPvExpStr.getExpanded(), "" ) == 0 ) ) {
           blankOrComment( alarmPvExpStr.getExpanded() ) ) {
        alarmPvExists = 0;
        lineVisibility = fillVisibility = 1;
      }
      else {
        connection.addPv();
        alarmPvExists = 1;
        lineColor.setConnectSensitive();
        fillColor.setConnectSensitive();
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
        lineVisibility = fillVisibility = 1;
        lineColor.setConnectSensitive();
        fillColor.setConnectSensitive();
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

int activeLineClass::deactivate (
  int pass )
{

  if ( pass == 1 ) {

    activeMode = 0;

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    //updateBlink( 0 );

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

int activeLineClass::draw ( void ) {

int n, drawArrows = ARROW_NONE, blink = 0;
XPoint arrowXPoints[8];

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  if ( ( numPoints > 1 ) && ( arrows != ARROW_NONE ) ) {
    drawArrows = arrows;
    getArrowCoords( drawArrows, arrowXPoints );
  }

  if ( ( numPoints > 2 ) && closePolygon ) {
    xpoints[numPoints].x = xpoints[0].x;
    xpoints[numPoints].y = xpoints[0].y;
    n = numPoints + 1;
  }
  else {
    n = numPoints;
  }

  actWin->drawGc.saveFg();

  if ( n > 0 ) {

    actWin->drawGc.setLineStyle( lineStyle );
    actWin->drawGc.setLineWidth( lineWidth );

    if ( fill ) {

      //actWin->drawGc.setFG( fillColor.pixelColor() );
      actWin->drawGc.setFG( fillColor.pixelIndex(), &blink );

      XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), xpoints, n, Complex,
       CoordModeOrigin );

    }

    //actWin->drawGc.setFG( lineColor.pixelColor() );
    actWin->drawGc.setFG( lineColor.pixelIndex(), &blink );

    XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), xpoints, n, CoordModeOrigin );

    if ( ( drawArrows == ARROW_FROM ) || ( drawArrows == ARROW_BOTH ) ) {
      actWin->drawGc.setLineStyle( LineSolid );
      XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), arrowXPoints, 4, Complex,
       CoordModeOrigin );
      XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), arrowXPoints, 4, CoordModeOrigin );
      actWin->drawGc.setLineStyle( lineStyle );
    }
    if ( ( drawArrows == ARROW_TO ) || ( drawArrows == ARROW_BOTH ) ) {
      actWin->drawGc.setLineStyle( LineSolid );
      XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), &arrowXPoints[4], 4, Complex,
       CoordModeOrigin );
      XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), &arrowXPoints[4], 4, CoordModeOrigin );
      actWin->drawGc.setLineStyle( lineStyle );
    }

    actWin->drawGc.restoreFg();
    actWin->drawGc.setLineStyle( LineSolid );
    actWin->drawGc.setLineWidth( 1 );

  }

  updateBlink( blink );

  return 1;

}

int activeLineClass::erase ( void )
{

int n, drawArrows = ARROW_NONE;
XPoint arrowXPoints[8];

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  if ( ( numPoints > 1 ) && ( arrows != ARROW_NONE ) ) {
    drawArrows = arrows;
    getArrowCoords( drawArrows, arrowXPoints );
  }

  if ( ( numPoints > 2 ) && closePolygon ) {
    xpoints[numPoints].x = xpoints[0].x;
    xpoints[numPoints].y = xpoints[0].y;
    n = numPoints + 1;
  }
  else {
    n = numPoints;
  }

  if ( n > 0 ) {

    actWin->drawGc.setLineStyle( lineStyle );
    actWin->drawGc.setLineWidth( lineWidth );

    if ( fill ) {

      XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), xpoints, n, Complex,
       CoordModeOrigin );

    }

    XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), xpoints, n, CoordModeOrigin );

    if ( ( drawArrows == ARROW_FROM ) || ( drawArrows == ARROW_BOTH ) ) {
      actWin->drawGc.setLineStyle( LineSolid );
      XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), arrowXPoints, 4, Complex,
       CoordModeOrigin );
      XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), arrowXPoints, 4, CoordModeOrigin );
      actWin->drawGc.setLineStyle( lineStyle );
    }
    if ( ( drawArrows == ARROW_TO ) || ( drawArrows == ARROW_BOTH ) ) {
      actWin->drawGc.setLineStyle( LineSolid );
      XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), &arrowXPoints[4], 4, Complex,
       CoordModeOrigin );
      XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), &arrowXPoints[4], 4, CoordModeOrigin );
      actWin->drawGc.setLineStyle( lineStyle );
    }

    actWin->drawGc.setLineStyle( LineSolid );
    actWin->drawGc.setLineWidth( 1 );

  }

  return 1;

}

void activeLineClass::updateDimensions ( void )
{

int i, xTranslate, yTranslate, newX, newY;
float xStretch, yStretch;

  xTranslate = x - oldX;
  yTranslate = y - oldY;
  if ( !oldW ) oldW = 1;
  xStretch = (float) w / (float) oldW;
  if ( !oldH ) oldH = 1;
  yStretch = (float) h / (float) oldH;

  for ( i=0; i<numPoints; i++ ) {

    newX = oldX + xTranslate +
     (int) ( ( (float) xpoints[i].x - (float) oldX ) * xStretch + 0.5 );
    if ( newX < x ) newX = x;
    if ( newX > ( x + w ) ) newX = x + w;

    xpoints[i].x = newX;

    newY = oldY + yTranslate +
     (int) ( ( (float) xpoints[i].y - (float) oldY ) * yStretch + 0.5 );
    if ( newY < y ) newY = y;
    if ( newY > ( y + h ) ) newY = y + h;

    xpoints[i].y = newY;

  }

  oldX = x;
  oldY = y;
  oldW = w;
  oldH = h;

}

int activeLineClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h )
{

int tmpx, tmpy, tmpw, tmph, ret_stat;

  tmpx = sboxX;
  tmpy = sboxY;
  tmpw = sboxW;
  tmph = sboxH;

  ret_stat = 1;

  tmpx += _x;
  tmpy += _y;

  tmpw += _w;
  if ( tmpw < 0 ) {
    ret_stat = 0;
  }

  tmph += _h;
  if ( tmph < 0 ) {
    ret_stat = 0;
  }

  return ret_stat;

}

int activeLineClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h )
{

int tmpx, tmpy, tmpw, tmph, ret_stat;

  tmpx = sboxX;
  tmpy = sboxY;
  tmpw = sboxW;
  tmph = sboxH;

  ret_stat = 1;

  if ( tmpw != -1 ) {
    if ( tmpw < 0 ) {
      ret_stat = 0;
    }
  }

  if ( tmph != -1 ) {
    if ( tmph < 0 ) {
      ret_stat = 0;
    }
  }

  return ret_stat;

}

int activeLineClass::resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h )
{

int savex, savey, savew, saveh, ret_stat;

  savex = sboxX;
  savey = sboxY;
  savew = sboxW;
  saveh = sboxH;

  ret_stat = 1;

  sboxX += _x;
  sboxY += _y;

  sboxW += _w;
  if ( sboxW < 0 ) {
    sboxX = savex;
    sboxW = savew;
    ret_stat = 0;
  }

  sboxH += _h;
  if ( sboxH < 0 ) {
    sboxY = savey;
    sboxH = saveh;
    ret_stat = 0;
  }

  return ret_stat;

}

int activeLineClass::resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h )
{

int savex, savey, savew, saveh, ret_stat;

  savex = sboxX;
  savey = sboxY;
  savew = sboxW;
  saveh = sboxH;

  ret_stat = 1;

  if ( _x >= 0 ) sboxX = _x;
  if ( _y >= 0 ) sboxY = _y;

  if ( _w >= 0 ) {
    sboxW = _w;
    if ( sboxW < 0 ) {
      sboxX = savex;
      sboxW = savew;
      ret_stat = 0;
    }
  }

  if ( _h >= 0 ) {
    sboxH = _h;
    if ( sboxH < 0 ) {
      sboxY = savey;
      sboxH = saveh;
      ret_stat = 0;
    }
  }

  return ret_stat;

}

int activeLineClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction ) // '+'=clockwise, '-'=counter clockwise
{

int i;
double dx0, dy0, dxOrig, dyOrig, dxPrime0, dyPrime0;

  // execute base class rotate
  ((activeGraphicClass *)this)->activeGraphicClass::rotate(
   xOrigin, yOrigin, direction );

  oldX = x;
  oldY = y;
  oldW = w;
  oldH = h;

  //fprintf( stderr, "activeLineClass::rotate %c, xO=%-d, yO=%-d\n",
  // direction, xOrigin, yOrigin );

  dxOrig = (double) xOrigin;
  dyOrig = (double) yOrigin;


  for ( i=0; i<numPoints; i++ ) {

    if ( direction == '+' ) { // clockwise

      // translate
      dx0 = (double) ( xpoints[i].x - dxOrig );
      dy0 = (double) ( dyOrig - xpoints[i].y );

      //fprintf( stderr, "1 dx0=%-g, dy0=%-g\n", dx0, dy0 );

      // rotate
      dxPrime0 = dy0;
      dyPrime0 = dx0 * -1.0;

      //fprintf( stderr, "2 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

      // translate
      dxPrime0 += dxOrig;
      dyPrime0 = dyOrig - dyPrime0;

      //fprintf( stderr, "3 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

      // set final x, y
      xpoints[i].x = (short) dxPrime0;
      xpoints[i].y = (short) dyPrime0;

    }
    else { // counterclockwise

      // translate
      dx0 = (double) ( xpoints[i].x - dxOrig );
      dy0 = (double) ( dyOrig - xpoints[i].y );

      //fprintf( stderr, "1 dx0=%-g, dy0=%-g\n", dx0, dy0 );

      // rotate
      dxPrime0 = dy0 * -1.0;
      dyPrime0 = dx0;

      //fprintf( stderr, "2 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

      // translate
      dxPrime0 += dxOrig;
      dyPrime0 = dyOrig - dyPrime0;

      //fprintf( stderr, "3 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

      // set final x, y
      xpoints[i].x = (short) dxPrime0;
      xpoints[i].y = (short) dyPrime0;

    }

  }

  return 1;

}

int activeLineClass::flip (
  int xOrigin,
  int yOrigin,
  char direction )
{

int i;
double dx0, dy0, dxOrig, dyOrig, dxPrime0, dyPrime0;

  //fprintf( stderr, "activeLineClass::flip %c, xO=%-d, yO=%-d\n",
  // direction, xOrigin, yOrigin );

  // execute base class flip
  ((activeGraphicClass *)this)->activeGraphicClass::flip(
   xOrigin, yOrigin, direction );

  oldX = x;
  oldY = y;
  oldW = w;
  oldH = h;

  dxOrig = (double) xOrigin;
  dyOrig = (double) yOrigin;


  for ( i=0; i<numPoints; i++ ) {

    //fprintf( stderr, "%-d: x=%-d, y=%-d\n", i, xpoints[i].x, xpoints[i].y );

    if ( direction == 'H' ) { // horizontal

      // translate
      dx0 = (double) ( xpoints[i].x - dxOrig );

      //fprintf( stderr, "1 dx0=%-g\n", dx0 );

      // move
      dxPrime0 = dx0 * -1.0;

      //fprintf( stderr, "2 dxPrime0=%-g\n", dxPrime0 );

      // translate
      dxPrime0 += dxOrig;

      //fprintf( stderr, "3 dxPrime0=%-g\n", dxPrime0 );

      // set final x
      xpoints[i].x = (short) dxPrime0;

    }
    else { // vertical

      // translate
      dy0 = (double) ( dyOrig - xpoints[i].y );

      //fprintf( stderr, "1 dx0=%-g, dy0=%-g\n", dx0, dy0 );

      // move
      dyPrime0 = dy0 * -1.0;

      //fprintf( stderr, "2 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

      // translate
      dyPrime0 = dyOrig - dyPrime0;

      //fprintf( stderr, "3 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

      // set final y
      xpoints[i].y = (short) dyPrime0;

    }

  }

  return 1;

}

int activeLineClass::isMultiPointObject ( void ) {

  return 1;

}

void activeLineClass::executeDeferred ( void ) {


int stat, nc, nau, nvu, nr, index, change;
pvValType pvV;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nau = needAlarmUpdate; needAlarmUpdate = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  nr = needRefresh; needRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

  if ( nc ) {

    minVis.d = (double) atof( minVisString );
    maxVis.d = (double) atof( maxVisString );

    lineColor.setConnected();
    fillColor.setConnected();

    if ( alarmPvExists ) {

      curStatus = alarmPvId->get_status();
      curSeverity = alarmPvId->get_severity();

      lineColor.setStatus( curStatus, curSeverity );
      fillColor.setStatus( curStatus, curSeverity );

      curLineColorIndex = actWin->ci->evalRule( lineColor.pixelIndex(),
       alarmPvId->get_double() );
      lineColor.changeIndex( curLineColorIndex, actWin->ci );

      curFillColorIndex = actWin->ci->evalRule( fillColor.pixelIndex(),
       alarmPvId->get_double() );
      fillColor.changeIndex( curFillColorIndex, actWin->ci );

      if ( !visPvExists ) {

        if ( actWin->ci->isInvisible( curLineColorIndex ) ) {
          prevLineVisibility = lineVisibility = 0;
        }
        else {
          prevLineVisibility = lineVisibility = 1;
        }

        if ( actWin->ci->isInvisible( curFillColorIndex ) ) {
          prevFillVisibility = fillVisibility = 0;
        }
        else {
          prevFillVisibility = fillVisibility = 1;
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
      lineColor.setStatus( curStatus, curSeverity );
      fillColor.setStatus( curStatus, curSeverity );
    }

    index = actWin->ci->evalRule( lineColor.pixelIndex(),
    alarmPvId->get_double() );

    if ( curLineColorIndex != index ) {
      curLineColorIndex = index;
      change = 1;
    }

    index = actWin->ci->evalRule( fillColor.pixelIndex(),
    alarmPvId->get_double() );

    if ( curFillColorIndex != index ) {
      curFillColorIndex = index;
      change = 1;
    }

    if ( change ) {

      if ( !visPvExists ) {

        if ( actWin->ci->isInvisible( curLineColorIndex ) ) {
          lineVisibility = 0;
        }
        else {
          lineVisibility = 1;
        }

        if ( actWin->ci->isInvisible( curFillColorIndex ) ) {
          fillVisibility = 0;
        }
        else {
          fillVisibility = 1;
        }

      }

      lineColor.changeIndex( curLineColorIndex, actWin->ci );
      fillColor.changeIndex( curFillColorIndex, actWin->ci );
      if ( ( prevLineVisibility != lineVisibility ) ||
	   ( prevFillVisibility != fillVisibility ) ) {
	prevLineVisibility = lineVisibility;
	prevFillVisibility = fillVisibility;
        eraseActive();
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
      if ( !visibility ) eraseActive();
      stat = smartDrawAllActive();
    }

  }

  if ( nr ) {
    stat = smartDrawAllActive();
  }

}

char *activeLineClass::firstDragName ( void ) {

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

char *activeLineClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeLineClass::dragValue (
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

void activeLineClass::changeDisplayParams (
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
    lineColor.setColorIndex( _fg1Color, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    fillColor.setColorIndex( _bgColor, actWin->ci );

}

void activeLineClass::changePvNames (
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

void activeLineClass::updateColors (
  double colorValue )
{

int index, change=0;

  index = actWin->ci->evalRule( lineColor.pixelIndex(), colorValue );

  if ( curLineColorIndex != index ) {
    curLineColorIndex = index;
    change = 1;
  }

  index = actWin->ci->evalRule( fillColor.pixelIndex(), colorValue );

  if ( curFillColorIndex != index ) {
    curFillColorIndex = index;
    change = 1;
  }

  if ( change ) {

    if ( actWin->ci->isInvisible( curLineColorIndex ) ) {
      lineVisibility = 0;
    }
    else {
      lineVisibility = 1;
    }

    if ( actWin->ci->isInvisible( curFillColorIndex ) ) {
      fillVisibility = 0;
    }
    else {
      fillVisibility = 1;
    }

    lineColor.changeIndex( curLineColorIndex, actWin->ci );
    fillColor.changeIndex( curFillColorIndex, actWin->ci );
    if ( ( prevLineVisibility != lineVisibility ) ||
         ( prevFillVisibility != fillVisibility ) ) {
      prevLineVisibility = lineVisibility;
      prevFillVisibility = fillVisibility;
    }

  }

}

int activeLineClass::addUndoEditNode ( 
  undoClass *undoObj
) {

int stat;
undoLineOpClass *ptr;

  ptr = new undoLineOpClass( numPoints, xpoints );

  stat = undoObj->addEditNode( this, ptr );

  return stat;

}

int activeLineClass::addUndoRotateNode ( 
  undoClass *undoObj
) {

int stat;
undoLineOpClass *ptr;

  ptr = new undoLineOpClass( numPoints, xpoints );

  stat = undoObj->addRotateNode( this, ptr, x, y, w, h );
  return stat;

}

int activeLineClass::addUndoFlipNode (
  undoClass *undoObj
) {

int stat;

  stat = addUndoRotateNode( undoObj );
  return stat;

}

int activeLineClass::undoEdit (
  undoOpClass *_opPtr )
{

undoLineOpClass *opPtr = (undoLineOpClass *) _opPtr;
int i;
int oneW, oneH, minX=0, minY=0, maxX=0, maxY=0;

  if ( xpoints ) delete[] xpoints;
  numPoints = opPtr->n;
  xpoints = new XPoint[numPoints+1];

  if ( numPoints > 0 ) {
    minX = maxX = opPtr->x[0];
    minY = maxY = opPtr->y[0];
  }
  else {
    minX = x;
    maxX = x + 1;
    minY = y;
    minY = y + 1;
  }

  for ( i=0; i<numPoints; i++ ) {
    if ( minX > opPtr->x[i] ) minX = opPtr->x[i];
    if ( minY > opPtr->y[i] ) minY = opPtr->y[i];
    if ( maxX < opPtr->x[i] ) maxX = opPtr->x[i];
    if ( maxY < opPtr->y[i] ) maxY = opPtr->y[i];
    xpoints[i].x = opPtr->x[i];
    xpoints[i].y = opPtr->y[i];
  }

  oneW = maxX - minX;
  oneH = maxY - minY;

  x = minX;
  y = minY;
  w = oneW;
  h = oneH;

  oldX = x;
  oldY = y;
  oldW = w;
  oldH = h;

  initSelectBox();

  return 1;

}

int activeLineClass::undoRotate (
  undoOpClass *_opPtr,
  int _x,
  int _y,
  int _w,
  int _h )
{

undoLineOpClass *opPtr = (undoLineOpClass *) _opPtr;
int i;

  for ( i=0; i<opPtr->n; i++ ) {
    xpoints[i].x = opPtr->x[i];
    xpoints[i].y = opPtr->y[i];
  }

  oldX = _x;
  oldY = _y;
  oldW = _w;
  oldH = _h;
  resizeAbs( _x, _y, _w, _h );
  resizeSelectBoxAbs( _x, _y, _w, _h );

  return 1;

}

int activeLineClass::undoFlip (
  undoOpClass *_opPtr,
  int x,
  int y,
  int w,
  int h )
{

int stat;

  stat = undoRotate( _opPtr, x, y, w, h );

  return stat;

}

void activeLineClass::getArrowCoords (
  int arrows,
  XPoint *points
) {

int n0, n1, i, slopeUndef, slopeUndefP;
double x0, x1, y0, y1, slope=1;
double x0P=0, x1P=0, y0P=0, y1P=0, slopeP=1;
double ax0, ay0, ax1, ay1, ax2, ay2, theta;

double len = 14.0;
double halfLen = 5.0;

  slopeUndef = slopeUndefP = 1;
  x0 = x1 = y0 = y1 = 0;

  if ( numPoints < 2 ) {
    for ( i=0; i<8; i++ ) {
      points[i].x = 0;
      points[i].y = 0;
    }
    return;
  }

  // slope for points 0 & 1
  if ( ( arrows == ARROW_FROM ) || ( arrows == ARROW_BOTH ) ) {

    x0 = (double) xpoints[0].x;
    y0 = (double) xpoints[0].y;
    x1 = (double) xpoints[1].x;
    y1 = (double) xpoints[1].y;

    if ( xpoints[1].x != xpoints[0].x ) {

      slope = ( y1 - y0 ) / ( x1 - x0 );
      slopeUndef = 0;

    }
    else {

      slope = 1e30;
      slopeUndef = 1;
      //fprintf( stderr, "1 slope undefined\n" );

    }

    //fprintf( stderr, "x0 = %-g\n", x0 );
    //fprintf( stderr, "y0 = %-g\n", y0 );
    //fprintf( stderr, "x1 = %-g\n", x1 );
    //fprintf( stderr, "y1 = %-g\n", y1 );
    //fprintf( stderr, "slope = %-g\n", slope );


    x0P = (double) xpoints[0].y;
    y0P = (double) xpoints[0].x;
    x1P = (double) xpoints[1].y;
    y1P = (double) xpoints[1].x;

    if ( xpoints[1].y != xpoints[0].y ) {

      slopeP = ( y0P - y1P ) / ( x1P - x0P );
      slopeUndefP = 0;

    }
    else {

      slopeP = 1e30;
      slopeUndefP = 1;
      //fprintf( stderr, "2 slopeP undefined\n" );

    }

  }

  //fprintf( stderr, "x0P = %-g\n", x0P );
  //fprintf( stderr, "y0P = %-g\n", y0P );
  //fprintf( stderr, "x1P = %-g\n", x1P );
  //fprintf( stderr, "y1P = %-g\n", y1P );
  //fprintf( stderr, "slopeP = %-g\n", slopeP );

  // point interscting first line
  if ( slopeUndef ) {
    ax0 = x0;
    if ( y0 >= y1 ) {
      ay0 = y0 - len;
    }
    else {
      ay0 = y0 + len;
    }
  }
  else {
    theta = atan( slope );
    //fprintf( stderr, "theta = %-g\n", theta );
    if ( x0 >= x1 ) {
      ax0 = x0 - len * fabs( cos( theta ) );
    }
    else {
      ax0 = x0 + len * fabs( cos( theta ) );
    }
    if ( y0 >= y1 ) {
      ay0 = y0 - len * fabs( sin( theta ) );
    }
    else {
      ay0 = y0 + len * fabs( sin( theta ) );
    }
  }

  //fprintf( stderr, "ax0 = %-g\n", ax0 );
  //fprintf( stderr, "ay0 = %-g\n", ay0 );


  // fisrt corner
  if ( slopeUndefP ) {
    ax1 = ax2 = ax0;
    if ( x0 >= x1 ) {
      ay1 = ay0 - halfLen;
      ay2 = ay0 + halfLen;
    }
    else {
      ay1 = ay0 + halfLen;
      ay2 = ay0 - halfLen;
    }
  }
  else {
    theta = atan( slopeP );
    if ( x0P >= x1P ) {
      ax1 = ax0 + halfLen * fabs( cos( theta ) );
      ax2 = ax0 - halfLen * fabs( cos( theta ) );
    }
    else {
      ax1 = ax0 - halfLen * fabs( cos( theta ) );
      ax2 = ax0 + halfLen * fabs( cos( theta ) );
    }
    if ( y0P >= y1P ) {
      ay1 = ay0 - halfLen * fabs( sin( theta ) );
      ay2 = ay0 + halfLen * fabs( sin( theta ) );
    }
    else {
      ay1 = ay0 + halfLen * fabs( sin( theta ) );
      ay2 = ay0 - halfLen * fabs( sin( theta ) );
    }

  }

  //fprintf( stderr, "ax1 = %-g\n", ax1 );
  //fprintf( stderr, "ay1 = %-g\n", ay1 );
  //fprintf( stderr, "ax2 = %-g\n", ax2 );
  //fprintf( stderr, "ay2 = %-g\n", ay2 );

  points[0].x = (short) rint(x0);
  points[0].y = (short) rint(y0);

  points[1].x = (short) rint(ax1);
  points[1].y = (short) rint(ay1);

  points[2].x = (short) rint(ax2);
  points[2].y = (short) rint(ay2);

  points[3].x = (short) rint(x0);
  points[3].y = (short) rint(y0);

  if ( ( arrows == ARROW_TO ) || ( arrows == ARROW_BOTH ) ) {

    n0 = numPoints - 1;
    n1 = numPoints - 2;

  // slope for points n-1 & n

    x0 = (double) xpoints[n0].x;
    y0 = (double) xpoints[n0].y;
    x1 = (double) xpoints[n1].x;
    y1 = (double) xpoints[n1].y;

    if ( xpoints[n1].x != xpoints[n0].x ) {

      slope = ( y1 - y0 ) / ( x1 - x0 );
      slopeUndef = 0;

    }
    else {

      slope = 1e30;
      slopeUndef = 1;
      //fprintf( stderr, "3 slope undefined\n" );

    }

    //fprintf( stderr, "x0 = %-g\n", x0 );
    //fprintf( stderr, "y0 = %-g\n", y0 );
    //fprintf( stderr, "x1 = %-g\n", x1 );
    //fprintf( stderr, "y1 = %-g\n", y1 );
    //fprintf( stderr, "slope = %-g\n", slope );


    x0P = (double) xpoints[n0].y;
    y0P = (double) xpoints[n0].x;
    x1P = (double) xpoints[n1].y;
    y1P = (double) xpoints[n1].x;

    if ( xpoints[n1].y != xpoints[n0].y ) {

      slopeP = ( y0P - y1P ) / ( x1P - x0P );
      slopeUndefP = 0;

    }
    else {

      slopeP = 1e30;
      slopeUndefP = 1;
      //fprintf( stderr, "4 slopeP undefined\n" );

    }

  }

  //fprintf( stderr, "x0P = %-g\n", x0P );
  //fprintf( stderr, "y0P = %-g\n", y0P );
  //fprintf( stderr, "x1P = %-g\n", x1P );
  //fprintf( stderr, "y1P = %-g\n", y1P );
  //fprintf( stderr, "slopeP = %-g\n", slopeP );

  // point interscting first line
  if ( slopeUndef ) {
    ax0 = x0;
    if ( y0 >= y1 ) {
      ay0 = y0 - len;
    }
    else {
      ay0 = y0 + len;
    }
  }
  else {
    theta = atan( slope );
    //fprintf( stderr, "theta = %-g\n", theta );
    if ( x0 >= x1 ) {
      ax0 = x0 - len * fabs( cos( theta ) );
    }
    else {
      ax0 = x0 + len * fabs( cos( theta ) );
    }
    if ( y0 >= y1 ) {
      ay0 = y0 - len * fabs( sin( theta ) );
    }
    else {
      ay0 = y0 + len * fabs( sin( theta ) );
    }
  }

  //fprintf( stderr, "ax0 = %-g\n", ax0 );
  //fprintf( stderr, "ay0 = %-g\n", ay0 );


  // fisrt corner
  if ( slopeUndefP ) {
    ax1 = ax2 = ax0;
    if ( x0 >= x1 ) {
      ay1 = ay0 - halfLen;
      ay2 = ay0 + halfLen;
    }
    else {
      ay1 = ay0 + halfLen;
      ay2 = ay0 - halfLen;
    }
  }
  else {
    theta = atan( slopeP );
    if ( x0P >= x1P ) {
      ax1 = ax0 + halfLen * fabs( cos( theta ) );
      ax2 = ax0 - halfLen * fabs( cos( theta ) );
    }
    else {
      ax1 = ax0 - halfLen * fabs( cos( theta ) );
      ax2 = ax0 + halfLen * fabs( cos( theta ) );
    }
    if ( y0P >= y1P ) {
      ay1 = ay0 - halfLen * fabs( sin( theta ) );
      ay2 = ay0 + halfLen * fabs( sin( theta ) );
    }
    else {
      ay1 = ay0 + halfLen * fabs( sin( theta ) );
      ay2 = ay0 - halfLen * fabs( sin( theta ) );
    }

  }

  //fprintf( stderr, "ax1 = %-g\n", ax1 );
  //fprintf( stderr, "ay1 = %-g\n", ay1 );
  //fprintf( stderr, "ax2 = %-g\n", ax2 );
  //fprintf( stderr, "ay2 = %-g\n", ay2 );

  points[4].x = (short) rint(x0);
  points[4].y = (short) rint(y0);

  points[5].x = (short) rint(ax1);
  points[5].y = (short) rint(ay1);

  points[6].x = (short) rint(ax2);
  points[6].y = (short) rint(ay2);

  points[7].x = (short) rint(x0);
  points[7].y = (short) rint(y0);

}

int activeLineClass::ctlBoxLen ( void ) {

  if ( lineWidth > 0 ) {
    return lineWidth + 2;
  }
  else {
    return 3;
  }

}

void activeLineClass::getPvs (
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

char *activeLineClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return alarmPvExpStr.getRaw();
  }
  else if ( i == 1 ) {
    return visPvExpStr.getRaw();
  }
  else if ( i == 2 ) {
    return minVisString;
  }
  else if ( i == 3 ) {
    return maxVisString;
  }
  else {
    return NULL;
  }

}

void activeLineClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    alarmPvExpStr.setRaw( string );
  }
  else if ( i == 1 ) {
    visPvExpStr.setRaw( string );
  }
  else if ( i == 2 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( minVisString, string, l );
    minVisString[l] = 0;
  }
  else if ( i == 3 ) {
    int l = max;
    if ( 39 < max ) l = 39;
    strncpy( maxVisString, string, l );
    maxVisString[l] = 0;
  }

}

// crawler functions may return blank pv names
char *activeLineClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return alarmPvExpStr.getExpanded();

}

char *activeLineClass::crawlerGetNextPv ( void ) {

  if ( crawlerPvIndex >= 1 ) return NULL;
  crawlerPvIndex++;
  return visPvExpStr.getExpanded();

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeLineClassPtr ( void ) {

activeLineClass *ptr;

  ptr = new activeLineClass;
  return (void *) ptr;

}

void *clone_activeLineClassPtr (
  void *_srcPtr )
{

activeLineClass *ptr, *srcPtr;

  srcPtr = (activeLineClass *) _srcPtr;

  ptr = new activeLineClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
