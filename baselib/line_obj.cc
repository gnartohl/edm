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

class undoLineOpClass : public undoOpClass {

public:

int n;
int *x;
int *y;

undoLineOpClass::undoLineOpClass ()
{

  printf( "undoLineOpClass::undoLineOpClass\n" );
  n = 0;

}

undoLineOpClass::undoLineOpClass (
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

undoLineOpClass::~undoLineOpClass ()
{

  delete x;
  x = NULL;
  delete y;
  y = NULL;
  n = 0;

}

};

#ifdef __epics__

static void aloMonitorAlarmPvConnectState (
  struct connection_handler_args arg )
{

activeLineClass *alo = (activeLineClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    alo->needAlarmConnectInit = 1;

  }
  else { // lost connection

    alo->alarmPvConnected = 0;
    alo->active = 0;
    alo->lineColor.setDisconnected();
    alo->fillColor.setDisconnected();
    alo->bufInvalidate();
    alo->needDraw = 1;

  }

  alo->actWin->appCtx->proc->lock();
  alo->actWin->addDefExeNode( alo->aglPtr );
  alo->actWin->appCtx->proc->unlock();

}

static void lineAlarmUpdate (
  struct event_handler_args ast_args )
{

class activeLineClass *alo;
struct dbr_sts_float statusRec;

  alo = (activeLineClass *) ast_args.usr;

  statusRec = *( (struct dbr_sts_float *) ast_args.dbr );
  alo->lineColor.setStatus( statusRec.status, statusRec.severity );
  alo->fillColor.setStatus( statusRec.status, statusRec.severity );

  if ( alo->active ) {
    alo->bufInvalidate();
    alo->needRefresh = 1;
    alo->actWin->appCtx->proc->lock();
    alo->actWin->addDefExeNode( alo->aglPtr );
    alo->actWin->appCtx->proc->unlock();
  }

}

static void aloMonitorVisPvConnectState (
  struct connection_handler_args arg )
{

activeLineClass *alo = (activeLineClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    alo->needVisConnectInit = 1;

  }
  else { // lost connection

    alo->visPvConnected = 0;
    alo->active = 0;
    alo->lineColor.setDisconnected();
    alo->fillColor.setDisconnected();
    alo->bufInvalidate();
    alo->needDraw = 1;

  }

  alo->actWin->appCtx->proc->lock();
  alo->actWin->addDefExeNode( alo->aglPtr );
  alo->actWin->appCtx->proc->unlock();

}

static void lineVisUpdate (
  struct event_handler_args ast_args )
{

pvValType pvV;
class activeLineClass *alo = (activeLineClass *) ast_args.usr;

  pvV.d = *( (double *) ast_args.dbr );
  if ( ( pvV.d >= alo->minVis.d ) && ( pvV.d < alo->maxVis.d ) )
    alo->visibility = 1 ^ alo->visInverted;
  else
    alo->visibility = 0 ^ alo->visInverted;

  if ( alo->active ) {

    if ( alo->visibility ) {

      alo->needRefresh = 1;

    }
    else {

      alo->needErase = 1;
      alo->needRefresh = 1;

    }

    alo->actWin->appCtx->proc->lock();
    alo->actWin->addDefExeNode( alo->aglPtr );
    alo->actWin->appCtx->proc->unlock();

  }

}

#endif

static void alc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeLineClass *alo = (activeLineClass *) client;

  alo->actWin->setChanged();

  alo->eraseSelectBoxCorners();
  alo->erase();

  alo->lineColorMode = alo->bufLineColorMode;
  if ( alo->lineColorMode == ALC_K_COLORMODE_ALARM )
    alo->lineColor.setAlarmSensitive();
  else
    alo->lineColor.setAlarmInsensitive();
  alo->lineColor.setColor( alo->bufLineColor, alo->actWin->ci );

  alo->fill = alo->bufFill;

  alo->fillColorMode = alo->bufFillColorMode;
  if ( alo->fillColorMode == ALC_K_COLORMODE_ALARM )
    alo->fillColor.setAlarmSensitive();
  else
    alo->fillColor.setAlarmInsensitive();
  alo->fillColor.setColor( alo->bufFillColor, alo->actWin->ci );

  alo->lineWidth = alo->bufLineWidth;

  if ( alo->bufLineStyle == 0 )
    alo->lineStyle = LineSolid;
  else if ( alo->bufLineStyle == 1 )
    alo->lineStyle = LineOnOffDash;

  alo->alarmPvExpStr.setRaw( alo->bufAlarmPvName );

  alo->visPvExpStr.setRaw( alo->bufVisPvName );

  if ( alo->bufVisInverted )
    alo->visInverted = 0;
  else
    alo->visInverted = 1;

  strncpy( alo->minVisString, alo->bufMinVisString, 39 );
  strncpy( alo->maxVisString, alo->bufMaxVisString, 39 );

  alo->x = alo->bufX;
  alo->sboxX = alo->bufX;

  alo->y = alo->bufY;
  alo->sboxY = alo->bufY;

  alo->w = alo->bufW;
  alo->sboxW = alo->bufW;

  alo->h = alo->bufH;
  alo->sboxH = alo->bufH;

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

  oneW = 3;
  oneH = 3;

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

  alo->lineEditBegin();

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

  alo->erase();
  alo->deleteRequest = 1;
  alo->ef.popdown();
  alo->operationCancel();
  alo->drawAll();

}

activeLineClass::activeLineClass ( void ) {

  name = new char[strlen("activeLineClass")+1];
  strcpy( name, "activeLineClass" );
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  visPvConnected = alarmPvConnected = 0;
  visPvExists = alarmPvExists = 0;
  active = 0;
  activeMode = 0;
  fill = 0;
  lineColorMode = ALC_K_COLORMODE_STATIC;
  fillColorMode = ALC_K_COLORMODE_STATIC;
  lineWidth = 1;
  lineStyle = LineSolid;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  xpoints = NULL;

  wasSelected = 0;

  head = new pointType;
  head->flink = head;
  head->blink = head;

}

activeLineClass::~activeLineClass ( void ) {

  if ( name ) delete name;
  if ( head ) delete head;
  if ( xpoints ) delete xpoints;

}

// copy constructor
activeLineClass::activeLineClass
( const activeLineClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
int i;

//  printf( "In copy constructor\n" );

  ago->clone( (activeGraphicClass *) source );

  oldX = x;
  oldY = y;
  oldW = w;
  oldH = h;

  name = new char[strlen("activeLineClass")+1];
  strcpy( name, "activeLineClass" );

  lineColor.copy(source->lineColor);
  lineCb = source->lineCb;
  lineColorMode = source->lineColorMode;

  fill = source->fill;

  fillColor.copy(source->fillColor);
  fillCb = source->fillCb;
  fillColorMode = source->fillColorMode;

  visInverted = source->visInverted;

  alarmPvExpStr.setRaw( source->alarmPvExpStr.rawString );
  visPvExpStr.setRaw( source->visPvExpStr.rawString );

  visibility = 0;
  prevVisibility = -1;
  visPvConnected = alarmPvConnected = 0;
  visPvExists = alarmPvExists = 0;
  active = 0;
  activeMode = 0;

  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

  head = new pointType;
  head->flink = head;
  head->blink = head;

  numPoints = source->numPoints;
  xpoints = new XPoint[source->numPoints];

  for ( i=0; i<numPoints; i++ ) {
    xpoints[i].x = source->xpoints[i].x;
    xpoints[i].y = source->xpoints[i].y;
  }

  capStyle = source->capStyle;
  joinStyle = source->joinStyle;
  lineStyle = source->lineStyle;
  lineWidth = source->lineWidth;

  wasSelected = 0;

}

int activeLineClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

//   printf( "In activeLineClass::createInteractive\n" );

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

  lineColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  fillColor.setColor( actWin->defaultBgColor, actWin->ci );

  this->editCreate();

  return 1;

}

int activeLineClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeLineClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeLineClass_str4, 31 );

  strncat( title, activeLineClass_str5, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufLineColor = lineColor.pixelColor();
  bufLineColorMode = lineColorMode;

  bufFill = fill;

  bufFillColor = fillColor.pixelColor();
  bufFillColorMode = fillColorMode;

  bufLineWidth = lineWidth;

  if ( lineStyle == LineSolid )
    bufLineStyle = 0;
  else if ( lineStyle == LineOnOffDash )
    bufLineStyle = 1;

  if ( alarmPvExpStr.getRaw() )
    strncpy( bufAlarmPvName, alarmPvExpStr.getRaw(), 39 );
  else
    strncpy( bufAlarmPvName, "", 39 );

  if ( visPvExpStr.getRaw() )
    strncpy( bufVisPvName, visPvExpStr.getRaw(), 39 );
  else
    strncpy( bufVisPvName, "", 39 );

  if ( visInverted )
    bufVisInverted = 0;
  else
    bufVisInverted = 1;

  strncpy( bufMinVisString, minVisString, 39 );
  strncpy( bufMaxVisString, maxVisString, 39 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeLineClass_str6, 25, &bufX );
  ef.addTextField( activeLineClass_str7, 25, &bufY );
  ef.addTextField( activeLineClass_str8, 25, &bufW );
  ef.addTextField( activeLineClass_str9, 25, &bufH );
  ef.addOption( activeLineClass_str10, activeLineClass_str11, &bufLineWidth );
  ef.addOption( activeLineClass_str12, activeLineClass_str13, &bufLineStyle );
  ef.addColorButton( activeLineClass_str14, actWin->ci, &lineCb, &bufLineColor );
  ef.addToggle( activeLineClass_str15, &bufLineColorMode );
  ef.addToggle( activeLineClass_str16, &bufFill );
  ef.addColorButton( activeLineClass_str17, actWin->ci, &fillCb, &bufFillColor );
  ef.addToggle( activeLineClass_str18, &bufFillColorMode );
  ef.addTextField( activeLineClass_str19, 27, bufAlarmPvName, 39 );
  ef.addTextField( activeLineClass_str20, 27, bufVisPvName, 39 );
  ef.addOption( " ", activeLineClass_str22, &bufVisInverted );
  ef.addTextField( activeLineClass_str23, 27, bufMinVisString, 39 );
  ef.addTextField( activeLineClass_str24, 27, bufMaxVisString, 39 );

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
    oneW = 3;
    oneH = 3;

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
      delete this->xpoints;
    }

  }

  this->actWin->drawGc.restoreFg();
  this->actWin->drawGc.setLineStyle( LineSolid );
  this->actWin->drawGc.setLineWidth( 1 );

  actWin->setCurrentPointObject( this );

  lineEditBegin();

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

  oneW = 3;
  oneH = 3;

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

  return 1;

}

int activeLineClass::removeLastPoint ( void )
{

pointPtr cur;
int oneX, oneY, oneW, oneH;

//   printf( "In activeLineClass::removeLastPoint\n" );

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( lineColor.pixelColor() );

  cur = head->blink;
  if ( cur == head ) return 0;

  oneX = cur->x;
  oneY = cur->y;
  oneW = 3;
  oneH = 3;

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

pointPtr cur;
int d;

//   printf( "In activeLineClass::selectPoint\n" );
//   printf( "x = %-d, y = %-d\n", x, y );

  cur = head->flink;
  while ( cur != head ) {

    d = ( cur->x - x ) * ( cur->x - x ) + ( cur->y - y ) * ( cur->y - y );
//     printf( "d = %-d\n", d );
    if ( d <= 9 ) return cur;

    cur = cur->flink;

  }

  return (pointPtr) NULL;

}

int activeLineClass::movePoint (
  pointPtr curPoint,
  int _x,
  int _y )
{

int oneX, oneY, oneW, oneH;

//   printf( "In activeLineClass::movePoint\n" );
//   printf( "x = %-d, y = %-d\n", x, y );

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( lineColor.pixelColor() );

  oneX = curPoint->x;
  oneY = curPoint->y;

  oneW = 3;
  oneH = 3;

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

  oneW = 3;
  oneH = 3;

//   printf( "In activeLineClass::lineEditComplete\n" );

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( lineColor.pixelColor() );

  // erase all points, count number of points, and find rectangular
  // extent
  minX = minY = 0x7fffffff;
  maxX = maxY = -1;
  cur = head->flink;
  while ( cur != head ) {

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

  // set select box size
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

//   printf( "minX=%-d, minY=%-d, w=%-d, h=%-d\n", x, y, w, h );

  initSelectBox();

  // build XPoint array
  xpoints = new XPoint[numPoints];
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

  return 1;

}

int activeLineClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, r, g, b, oneX, oneY, index;
int major, minor, release;
unsigned int pixel;
char oneName[39+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

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

  xpoints = new XPoint[numPoints];

  for ( i=0; i<numPoints; i++ ) {
    fscanf( f, "%d %d\n", &oneX, &oneY ); actWin->incLine();
    xpoints[i].x = (short) oneX;
    xpoints[i].y = (short) oneY;
  }

  if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    lineColor.setColor( pixel, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == ALC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    actWin->ci->setIndex( index, &pixel );
    fillColor.setColor( pixel, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    if ( ( major < 2 ) && ( minor < 2 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    actWin->ci->setRGB( r, g, b, &pixel );
    lineColor.setColor( pixel, actWin->ci );

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
    fillColor.setColor( pixel, actWin->ci );

  }

  fscanf( f, "%d\n", &fillColorMode ); actWin->incLine();

  if ( fillColorMode == ALC_K_COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  fscanf( f, "%d\n", &lineWidth ); actWin->incLine();
  fscanf( f, "%d\n", &lineStyle ); actWin->incLine();

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  alarmPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  visPvExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    readStringFromFile( minVisString, 39, f ); actWin->incLine();
    readStringFromFile( maxVisString, 39, f ); actWin->incLine();
  }
  else {
    strcpy( minVisString, "1" );
    strcpy( maxVisString, "1" );
  }

  this->wasSelected = 0;

  return 1;

}

int activeLineClass::save (
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

  actWin->ci->getIndex( lineColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", lineColorMode );

  fprintf( f, "%-d\n", fill );

  actWin->ci->getIndex( fillColor.pixelColor(), &index );
  fprintf( f, "%-d\n", index );

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

  return 1;

}

int activeLineClass::drawActive ( void )
{

  if ( !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  if ( numPoints > 0 ) {

    actWin->executeGc.setLineStyle( lineStyle );
    actWin->executeGc.setLineWidth( lineWidth );

    actWin->executeGc.saveFg();

    if ( fill ) {

      actWin->executeGc.setFG( fillColor.getColor() );

      XFillPolygon( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), xpoints, numPoints, Complex,
       CoordModeOrigin );

    }

    actWin->executeGc.setFG( lineColor.getColor() );

    XDrawLines( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), xpoints, numPoints, CoordModeOrigin );

    actWin->executeGc.restoreFg();
    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setLineWidth( 1 );

  }

  return 1;

}

int activeLineClass::eraseUnconditional ( void )
{

  if ( numPoints > 0 ) {

    actWin->executeGc.setLineStyle( lineStyle );
    actWin->executeGc.setLineWidth( lineWidth );

    if ( fill ) {

      XFillPolygon( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.eraseGC(), xpoints, numPoints, Complex,
       CoordModeOrigin );

    }

    XDrawLines( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), xpoints, numPoints, CoordModeOrigin );

    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setLineWidth( 1 );

  }

  return 1;

}

int activeLineClass::eraseActive ( void )
{

  if ( !activeMode ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  if ( numPoints > 0 ) {

    actWin->executeGc.setLineStyle( lineStyle );
    actWin->executeGc.setLineWidth( lineWidth );

    if ( fill ) {

      XFillPolygon( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.eraseGC(), xpoints, numPoints, Complex,
       CoordModeOrigin );

    }

    XDrawLines( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), xpoints, numPoints, CoordModeOrigin );

    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.setLineWidth( 1 );

  }

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

int stat;

  switch ( pass ) {

  case 1: // initialize

    needVisConnectInit = 0;
    needAlarmConnectInit = 0;
    needErase = needDraw = needRefresh = 0;
    aglPtr = ptr;
    opComplete = 0;

#ifdef __epics__
    alarmEventId = visEventId = 0;
#endif

    alarmPvConnected = visPvConnected = 0;
    activeMode = 1;
    prevVisibility = -1;

    init = 1;
    active = 1;

    if ( !alarmPvExpStr.getExpanded() ||
         ( strcmp( alarmPvExpStr.getExpanded(), "" ) == 0 ) ) {
      alarmPvExists = 0;
    }
    else {
      alarmPvExists = 1;
      lineColor.setConnectSensitive();
      fillColor.setConnectSensitive();
      init = 0;
      active = 0;
    }

    if ( !visPvExpStr.getExpanded() ||
         ( strcmp( visPvExpStr.getExpanded(), "" ) == 0 ) ) {
      visPvExists = 0;
      visibility = 1;
    }
    else {
      visPvExists = 1;
      visibility = 0;
      lineColor.setConnectSensitive();
      fillColor.setConnectSensitive();
      init = 0;
      active = 0;
    }

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

#ifdef __epics__

      if ( alarmPvExists ) {
        stat = ca_search_and_connect( alarmPvExpStr.getExpanded(), &alarmPvId,
         aloMonitorAlarmPvConnectState, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeLineClass_str25 );
          return 0;
        }
      }

      if ( visPvExists ) {
        stat = ca_search_and_connect( visPvExpStr.getExpanded(), &visPvId,
         aloMonitorVisPvConnectState, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeLineClass_str26 );
          return 0;
        }
      }

      opComplete = 1;
      this->bufInvalidate();

#endif

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

int stat;

  activeMode = 0;

  if ( pass == 1 ) {

#ifdef __epics__

  if ( alarmPvExists ) {
    stat = ca_clear_channel( alarmPvId );
    if ( stat != ECA_NORMAL )
      printf( activeLineClass_str29 );
  }

  if ( visPvExists ) {
    stat = ca_clear_channel( visPvId );
    if ( stat != ECA_NORMAL )
      printf( activeLineClass_str30 );
  }

#endif

  }

  return 1;

}

int activeLineClass::draw ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  if ( numPoints > 0 ) {

    actWin->drawGc.setLineStyle( lineStyle );
    actWin->drawGc.setLineWidth( lineWidth );

    if ( fill ) {

      actWin->drawGc.setFG( fillColor.pixelColor() );

      XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), xpoints, numPoints, Complex,
       CoordModeOrigin );

    }

    actWin->drawGc.setFG( lineColor.pixelColor() );

    XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), xpoints, numPoints, CoordModeOrigin );

    actWin->drawGc.restoreFg();
    actWin->drawGc.setLineStyle( LineSolid );
    actWin->drawGc.setLineWidth( 1 );

  }

  return 1;

}

int activeLineClass::erase ( void )
{

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  if ( numPoints > 0 ) {

    actWin->drawGc.setLineStyle( lineStyle );
    actWin->drawGc.setLineWidth( lineWidth );

    if ( fill ) {

      XFillPolygon( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), xpoints, numPoints, Complex,
       CoordModeOrigin );

    }

    XDrawLines( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), xpoints, numPoints, CoordModeOrigin );

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

  //printf( "activeLineClass::rotate %c, xO=%-d, yO=%-d\n",
  // direction, xOrigin, yOrigin );

  dxOrig = (double) xOrigin;
  dyOrig = (double) yOrigin;


  for ( i=0; i<numPoints; i++ ) {

    if ( direction == '+' ) { // clockwise

      // translate
      dx0 = (double) ( xpoints[i].x - dxOrig );
      dy0 = (double) ( dyOrig - xpoints[i].y );

      //printf( "1 dx0=%-g, dy0=%-g\n", dx0, dy0 );

      // rotate
      dxPrime0 = dy0;
      dyPrime0 = dx0 * -1.0;

      //printf( "2 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

      // translate
      dxPrime0 += dxOrig;
      dyPrime0 = dyOrig - dyPrime0;

      //printf( "3 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

      // set final x, y
      xpoints[i].x = (short) dxPrime0;
      xpoints[i].y = (short) dyPrime0;

    }
    else { // counterclockwise

      // translate
      dx0 = (double) ( xpoints[i].x - dxOrig );
      dy0 = (double) ( dyOrig - xpoints[i].y );

      //printf( "1 dx0=%-g, dy0=%-g\n", dx0, dy0 );

      // rotate
      dxPrime0 = dy0 * -1.0;
      dyPrime0 = dx0;

      //printf( "2 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

      // translate
      dxPrime0 += dxOrig;
      dyPrime0 = dyOrig - dyPrime0;

      //printf( "3 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

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

  //printf( "activeLineClass::flip %c, xO=%-d, yO=%-d\n",
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

    //printf( "%-d: x=%-d, y=%-d\n", i, xpoints[i].x, xpoints[i].y );

    if ( direction == 'H' ) { // horizontal

      // translate
      dx0 = (double) ( xpoints[i].x - dxOrig );

      //printf( "1 dx0=%-g\n", dx0 );

      // move
      dxPrime0 = dx0 * -1.0;

      //printf( "2 dxPrime0=%-g\n", dxPrime0 );

      // translate
      dxPrime0 += dxOrig;

      //printf( "3 dxPrime0=%-g\n", dxPrime0 );

      // set final x
      xpoints[i].x = (short) dxPrime0;

    }
    else { // vertical

      // translate
      dy0 = (double) ( dyOrig - xpoints[i].y );

      //printf( "1 dx0=%-g, dy0=%-g\n", dx0, dy0 );

      // move
      dyPrime0 = dy0 * -1.0;

      //printf( "2 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

      // translate
      dyPrime0 = dyOrig - dyPrime0;

      //printf( "3 dxPrime0=%-g, dyPrime0=%-g\n", dxPrime0, dyPrime0 );

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

int stat, nvc, nac, ne, nd, nr;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nvc = needVisConnectInit; needVisConnectInit = 0;
  nac = needAlarmConnectInit; needAlarmConnectInit = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nr = needRefresh; needRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

#ifdef __epics__

  if ( nvc ) {

    if ( ( ca_field_type(visPvId) == DBR_ENUM ) ||
         ( ca_field_type(visPvId) == DBR_INT ) ||
         ( ca_field_type(visPvId) == DBR_LONG ) ||
         ( ca_field_type(visPvId) == DBR_FLOAT ) ||
         ( ca_field_type(visPvId) == DBR_DOUBLE ) ) {

      visPvConnected = 1;

      pvType = ca_field_type( visPvId );

      minVis.d = (double) atof( minVisString );
      maxVis.d = (double) atof( maxVisString );

      if ( ( visPvConnected || !visPvExists ) &&
           ( alarmPvConnected || !alarmPvExists ) ) {

        active = 1;
        lineColor.setConnected();
        fillColor.setConnected();
        bufInvalidate();

        if ( init ) {
          eraseUnconditional();
	}

        init = 1;

        actWin->requestActiveRefresh();

      }

      if ( !visEventId ) {
        stat = ca_add_masked_array_event( DBR_DOUBLE, 1, visPvId,
         lineVisUpdate, (void *) this, (float) 0.0, (float) 0.0,
         (float) 0.0, &visEventId, DBE_VALUE );
        if ( stat != ECA_NORMAL ) {
          printf( activeLineClass_str31 );
        }
      }

    }
    else { // force a draw in the non-active state

      active = 0;
      lineColor.setDisconnected();
      fillColor.setDisconnected();
      bufInvalidate();
      drawActive();

    }

  }

  if ( nac ) {

    alarmPvConnected = 1;

    if ( ( visPvConnected || !visPvExists ) &&
         ( alarmPvConnected || !alarmPvExists ) ) {

      active = 1;
      lineColor.setConnected();
      fillColor.setConnected();
      bufInvalidate();

      if ( init ) {
        eraseUnconditional();
      }

      init = 1;

      actWin->requestActiveRefresh();

    }

    if ( !alarmEventId ) {
      stat = ca_add_masked_array_event( DBR_STS_FLOAT, 1, alarmPvId,
       lineAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &alarmEventId, DBE_ALARM );
      if ( stat != ECA_NORMAL ) {
        printf( activeLineClass_str32 );
      }
    }

  }

#endif

  if ( ne ) {
    eraseActive();
  }

  if ( nd ) {
//      drawActive();
    stat = smartDrawAllActive();
  }

  if ( nr ) {
//      actWin->requestActiveRefresh();
    stat = smartDrawAllActive();
  }

}

char *activeLineClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeLineClass::nextDragName ( void ) {

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

  switch ( i ) {

  case 1:
    return alarmPvExpStr.getExpanded();
    break;

  case 2:
    return visPvExpStr.getExpanded();
    break;

  }

  // else, disabled

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
  unsigned int _textFgColor,
  unsigned int _fg1Color,
  unsigned int _fg2Color,
  unsigned int _offsetColor,
  unsigned int _bgColor,
  unsigned int _topShadowColor,
  unsigned int _botShadowColor )
{

  if ( _flag & ACTGRF_FG1COLOR_MASK )
    lineColor.setColor( _fg1Color, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    fillColor.setColor( _bgColor, actWin->ci );

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
