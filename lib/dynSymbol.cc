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

#define __dynSymbol_cc 1

class undoDynSymbolOpClass;

#include "dynSymbol.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

class undoDynSymbolOpClass : public undoOpClass {

public:

activeDynSymbolClass *dso;

undoDynSymbolOpClass::undoDynSymbolOpClass ()
{

  printf( "undoDynSymbolOpClass::undoDynSymbolOpClass\n" );
  dso = NULL;

}

undoDynSymbolOpClass::undoDynSymbolOpClass (
  activeDynSymbolClass *_dso
) {

int i;
activeGraphicListPtr head, cur, next, sourceHead, curSource;

  // printf( "undoDynSymbolOpClass::undoDynSymbolOpClass\n" );

  // copy display list and editable attributes from current symbol
  dso = new activeDynSymbolClass;

  // copy base class info
  dso->x = _dso->x;
  dso->y = _dso->y;
  dso->w = _dso->w;
  dso->h = _dso->h;
  dso->sboxX = _dso->sboxX;
  dso->sboxY = _dso->sboxY;
  dso->sboxW = _dso->sboxW;
  dso->sboxH = _dso->sboxH;
  dso->orientation = _dso->orientation;
  dso->nextToEdit = _dso->nextToEdit;
  dso->nextSelectedToEdit = NULL;
  dso->inGroup = _dso->inGroup;

  // steal current dyn symbol obj list - we don't want a copy,
  // we want the current information without changing the
  // value of pointers. These pointers are referenced in the
  // dyn symbol's undo object used to undo its children.
  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {

    head = new activeGraphicListType;
    head->flink = head;
    head->blink = head;

    sourceHead = (activeGraphicListPtr) _dso->voidHead[i];
    curSource = sourceHead->flink;
    while ( curSource != sourceHead ) {

      next = curSource->flink;

      cur = curSource;

      cur->blink = head->blink;
      head->blink->flink = cur;
      head->blink = cur;
      cur->flink = head;

      curSource = next;

    }

    dso->voidHead[i] = (void *) head;

  }

  // make current dyn symbol obj list empty (because we stold all
  // the information above); when the edit operation proceeds and
  // the display list is destroyed and recreated from the current
  // contents of the dyn symbol file, we don't want the current image
  // list destroyed.
  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {
    sourceHead = (activeGraphicListPtr) _dso->voidHead[i];
    sourceHead->flink = sourceHead;
    sourceHead->blink = sourceHead;
  }

  dso->index = 0;
  dso->initialIndex = _dso->initialIndex;
  dso->controlV = _dso->controlV;
  dso->controlVal = _dso->controlVal;
  dso->iValue = _dso->iValue;
  dso->gateUpPvExpStr.setRaw( _dso->gateUpPvExpStr.rawString );
  dso->gateDownPvExpStr.setRaw( _dso->gateDownPvExpStr.rawString );
  dso->useGate = _dso->useGate;
  dso->continuous = _dso->continuous;
  dso->rate = _dso->rate;
  dso->gateUpValue = _dso->gateUpValue;
  dso->gateDownValue = _dso->gateDownValue;

  strncpy( dso->dynSymbolFileName, _dso->dynSymbolFileName, 127 );

  dso->numStates = _dso->numStates;
  for ( i=0; i<_dso->numStates; i++ ) {
    dso->stateMinValue[i] = _dso->stateMinValue[i];
    dso->stateMaxValue[i] = _dso->stateMaxValue[i];
  }

  dso->showOOBState = _dso->showOOBState;
  dso->useOriginalSize = _dso->useOriginalSize;
  dso->useOriginalColors = _dso->useOriginalColors;
  dso->fgCb = _dso->fgCb;
  dso->bgCb = _dso->bgCb;
  dso->fgColor = _dso->fgColor;
  dso->bgColor = _dso->bgColor;
  dso->colorPvExpStr.setRaw( _dso->colorPvExpStr.rawString );

}

undoDynSymbolOpClass::~undoDynSymbolOpClass ()
{

  if ( dso ) {
    delete dso;
    dso = NULL;
  }

}

};

static void dsc_updateControl (
  XtPointer client,
  XtIntervalId *id )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) client;

  dso->timer = 0;

  if ( !dso->timerActive ) return;

  if ( dso->continuous ) {

    if ( dso->useGate ) {

      if ( dso->up ) { // use only the gate-up pv
        (dso->curCount)++;
        if ( dso->curCount > dso->numStates-1 ) {
          if ( dso->showOOBState ) {
            dso->curCount = 0;
	  }
	  else {
            dso->curCount = 1;
	  }
	}
      }
      else {
        dso->timerActive = 0;
      }

    }
    else {

      dso->timerActive = 1;
      (dso->curCount)++;
      if ( dso->curCount > dso->numStates-1 ) {
        if ( dso->showOOBState ) {
          dso->curCount = 0;
        }
        else {
          dso->curCount = 1;
        }
      }

    }

  }
  else {

    if ( dso->useGate ) {
      if ( dso->up )
        (dso->curCount)++;
      else if ( dso->down )
        (dso->curCount)--;
      else
        dso->timerActive = 0;
    }
    else {
      (dso->curCount)++;
    }

    if ( dso->up ) {
      if ( dso->curCount >= dso->numStates-1 ) {
        dso->curCount = dso->numStates-1;
        dso->timerActive = 0;
      }
    }
    else if ( dso->down ) {
      if ( dso->showOOBState ) {
        if ( dso->curCount <= 0 ) {
          dso->curCount = 0;
          dso->timerActive = 0;
        }
      }
      else {
        if ( dso->curCount <= 1 ) {
          dso->curCount = 1;
          dso->timerActive = 0;
        }
      }
    }

  }

  dso->curControlV = (double) (dso->curCount);
  dso->needRefresh = 1;
  dso->actWin->appCtx->proc->lock();
  dso->actWin->addDefExeNode( dso->aglPtr );
  dso->actWin->appCtx->proc->unlock();

  if ( dso->timerActive ) {
    dso->timer = XtAppAddTimeOut( dso->actWin->appCtx->appContext(),
     (unsigned long) (dso->rate*1000.0), dsc_updateControl, client );
  }
  else {
    dso->timer = 0;
  }

}

#ifdef __epics__

static void dynSymbol_monitor_color_connect_state (
  struct connection_handler_args arg )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    dso->needColorInit = 1;
    dso->colorPvConnected = 1;

  }
  else {

    dso->colorPvConnected = 0;
    dso->active = 0;
    dso->bufInvalidate();
    dso->needDraw = 1;

  }

  dso->actWin->appCtx->proc->lock();
  dso->actWin->addDefExeNode( dso->aglPtr );
  dso->actWin->appCtx->proc->unlock();

}

static void dynSymbol_colorUpdate (
  struct event_handler_args arg )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) ca_puser(arg.chid);

  if ( dso->active ) {

    dso->curColorV = *( (double *) arg.dbr );

    dso->needColorRefresh = 1;
    dso->actWin->appCtx->proc->lock();
    dso->actWin->addDefExeNode( dso->aglPtr );
    dso->actWin->appCtx->proc->unlock();

  }

}

static void dynSymbol_monitor_gateUp_connect_state (
  struct connection_handler_args arg )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    dso->needGateUpConnect = 1;
    dso->gateUpPvConnected = 1;

  }
  else {

    dso->gateUpPvConnected = 0;
    dso->active = 0;
    dso->bufInvalidate();
    dso->needDraw = 1;

  }

  dso->actWin->appCtx->proc->lock();
  dso->actWin->addDefExeNode( dso->aglPtr );
  dso->actWin->appCtx->proc->unlock();

}

static void dynSymbol_gateUpUpdate (
  struct event_handler_args ast_args )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) ca_puser(ast_args.chid);

  if ( *( (int *) ast_args.dbr ) == dso->gateUpValue ) {

    if ( dso->active ) {
      dso->needGateUp = 1;
      dso->actWin->appCtx->proc->lock();
      dso->actWin->addDefExeNode( dso->aglPtr );
      dso->actWin->appCtx->proc->unlock();
    }

  }

}

static void dynSymbol_monitor_gateDown_connect_state (
  struct connection_handler_args arg )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    dso->needGateDownConnect = 1;
    dso->gateDownPvConnected = 1;

  }
  else {

    dso->gateDownPvConnected = 0;
    dso->active = 0;
    dso->bufInvalidate();
    dso->needDraw = 1;

  }

  dso->actWin->appCtx->proc->lock();
  dso->actWin->addDefExeNode( dso->aglPtr );
  dso->actWin->appCtx->proc->unlock();

}

static void dynSymbol_gateDownUpdate (
  struct event_handler_args ast_args )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) ca_puser(ast_args.chid);

  if ( *( (int *) ast_args.dbr ) == dso->gateDownValue  ) {

    if ( dso->active ) {
      dso->needGateDown = 1;
      dso->actWin->appCtx->proc->lock();
      dso->actWin->addDefExeNode( dso->aglPtr );
      dso->actWin->appCtx->proc->unlock();
    }

  }

}

#endif

static void dsc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) client;
int stat, resizeStat, saveW, saveH;

  dso->actWin->setChanged();

  dso->eraseSelectBoxCorners();
  dso->erase();

// =============================================

// New requirement for edit undo
  dso->confirmEdit();

// =============================================

  strncpy( dso->id, dso->bufId, 31 );

  dso->x = dso->bufX;
  dso->sboxX = dso->bufX;

  dso->y = dso->bufY;
  dso->sboxY = dso->bufY;

//    dso->controlPvExpStr.setRaw( dso->bufControlPvName );

  strncpy( dso->dynSymbolFileName, dso->bufDynSymbolFileName, 127 );

  dso->useOriginalSize = dso->bufUseOriginalSize;

  dso->useOriginalColors = dso->bufUseOriginalColors;

  dso->fgColor = dso->bufFgColor;
  dso->bgColor = dso->bufBgColor;

  dso->gateUpPvExpStr.setRaw( dso->bufGateUpPvName );
  dso->gateDownPvExpStr.setRaw( dso->bufGateDownPvName );
  dso->colorPvExpStr.setRaw( dso->bufColorPvName );
  dso->useGate = dso->bufUseGate;
  dso->gateUpValue = dso->bufGateUpValue;
  dso->gateDownValue = dso->bufGateDownValue;
  dso->continuous = dso->bufContinuous;
  dso->rate = dso->bufRate;
  dso->initialIndex = dso->bufInitialIndex;
  dso->showOOBState = dso->bufShowOOBState;

  if ( dso->rate < 0.05 ) dso->rate = 0.05;

  if ( dso->useOriginalSize ) {
    stat = dso->readDynSymbolFile();
  }
  else {
    saveW = dso->w;
    saveH = dso->h;
    stat = dso->readDynSymbolFile();
    if ( ( saveW != dso->w ) || ( saveH != dso->h ) ) {
      resizeStat = dso->checkResizeSelectBoxAbs( -1, -1, saveW, saveH );
      if ( resizeStat & 1 ) {
        dso->resizeSelectBoxAbs( -1, -1, saveW, saveH );
        dso->resizeAbs( -1, -1, saveW, saveH );
      }
      else {
        dso->actWin->appCtx->postMessage(
         activeDynSymbolClass_str3 );
      }
    }
  }

  if ( !( stat & 1 ) ) {
    dso->actWin->appCtx->postMessage( activeDynSymbolClass_str4 );
  }

}

static void dsc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) client;

  dsc_edit_update ( w, client, call );
  dso->refresh( dso );

}

static void dsc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) client;

  dsc_edit_update ( w, client, call );
  dso->ef.popdown();
  dso->operationComplete();

}

static void dsc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) client;

  dso->ef.popdown();
  dso->operationCancel();

}

static void dsc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeDynSymbolClass *dso = (activeDynSymbolClass *) client;

  dso->ef.popdown();
  dso->operationCancel();
  dso->erase();
  dso->deleteRequest = 1;
  dso->drawAll();

}

activeDynSymbolClass::activeDynSymbolClass ( void ) {

activeGraphicListPtr head;
int i;

  name = new char[strlen("activeDynSymbolClass")+1];
  strcpy( name, "activeDynSymbolClass" );

  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {

    head = new activeGraphicListType;
    head->flink = head;
    head->blink = head;

    voidHead[i] = (void *) head;

  }

  activeMode = 0;
  numStates = 2;
  index = 0;
  initialIndex = 1;
  controlV = 0.0;
  controlVal = 0.0;
  iValue = 0;
  strcpy( dynSymbolFileName, "" );
  useGate = 0;
  gateUpValue = 1;
  gateDownValue = 0;
  continuous = 0;
  rate = 1.0;
  useOriginalSize = 0;
  useOriginalColors = 1;
  showOOBState = 0;

  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {
    stateMinValue[i] = i;
    stateMaxValue[i] = i+1;
  }

  btnDownActionHead = new btnActionListType;
  btnDownActionHead->flink = btnDownActionHead;
  btnDownActionHead->blink = btnDownActionHead;

  btnUpActionHead = new btnActionListType;
  btnUpActionHead->flink = btnUpActionHead;
  btnUpActionHead->blink = btnUpActionHead;

  btnMotionActionHead = new btnActionListType;
  btnMotionActionHead->flink = btnMotionActionHead;
  btnMotionActionHead->blink = btnMotionActionHead;

}

activeDynSymbolClass::~activeDynSymbolClass ( void ) {

//   printf( "In activeDynSymbolClass::~activeDynSymbolClass\n" );

activeGraphicListPtr head;
activeGraphicListPtr cur, next;
btnActionListPtr curBtnAction, nextBtnAction;
int i;

  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {
      next = cur->flink;
      delete cur->node;
      delete cur;
      cur = next;
    }
    head->flink = NULL;
    head->blink = NULL;
    delete head;

  }

  // btn down action list

  curBtnAction = btnDownActionHead->flink;
  while ( curBtnAction != btnDownActionHead ) {
    nextBtnAction = curBtnAction->flink;
    delete curBtnAction->node;
    delete curBtnAction;
    curBtnAction = nextBtnAction;
  }
  btnDownActionHead->flink = NULL;
  btnDownActionHead->blink = NULL;
  delete btnDownActionHead;

  // btn up action list

  curBtnAction = btnUpActionHead->flink;
  while ( curBtnAction != btnUpActionHead ) {
    nextBtnAction = curBtnAction->flink;
    delete curBtnAction->node;
    delete curBtnAction;
    curBtnAction = nextBtnAction;
  }
  btnUpActionHead->flink = NULL;
  btnUpActionHead->blink = NULL;
  delete btnUpActionHead;

  // btn motion action list

  curBtnAction = btnMotionActionHead->flink;
  while ( curBtnAction != btnMotionActionHead ) {
    nextBtnAction = curBtnAction->flink;
    delete curBtnAction->node;
    delete curBtnAction;
    curBtnAction = nextBtnAction;
  }
  btnMotionActionHead->flink = NULL;
  btnMotionActionHead->blink = NULL;
  delete btnMotionActionHead;

  if ( name ) delete name;

}

// copy constructor
activeDynSymbolClass::activeDynSymbolClass
 ( const activeDynSymbolClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
activeGraphicListPtr head, cur, curSource, sourceHead;
int i;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeDynSymbolClass")+1];
  strcpy( name, "activeDynSymbolClass" );

  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {

    head = new activeGraphicListType;
    head->flink = head;
    head->blink = head;

    sourceHead = (activeGraphicListPtr) source->voidHead[i];
    curSource = sourceHead->flink;
    while ( curSource != sourceHead ) {

      cur = new activeGraphicListType;
      cur->node = actWin->obj.clone( curSource->node->objName(),
       curSource->node );

      cur->blink = head->blink;
      head->blink->flink = cur;
      head->blink = cur;
      cur->flink = head;

      curSource = curSource->flink;

    }

    voidHead[i] = (void *) head;

  }

  btnDownActionHead = new btnActionListType;
  btnDownActionHead->flink = btnDownActionHead;
  btnDownActionHead->blink = btnDownActionHead;

  btnUpActionHead = new btnActionListType;
  btnUpActionHead->flink = btnUpActionHead;
  btnUpActionHead->blink = btnUpActionHead;

  btnMotionActionHead = new btnActionListType;
  btnMotionActionHead->flink = btnMotionActionHead;
  btnMotionActionHead->blink = btnMotionActionHead;

  activeMode = 0;
  index = 0;
  initialIndex = source->initialIndex;
  controlV = 0.0;
  controlVal = 0.0;
  iValue = 0;
  gateUpPvExpStr.setRaw( source->gateUpPvExpStr.rawString );
  gateDownPvExpStr.setRaw( source->gateDownPvExpStr.rawString );
  useGate = source->useGate;
  continuous = source->continuous;
  rate = source->rate;
  gateUpValue = source->gateUpValue;
  gateDownValue = source->gateDownValue;

  strncpy( dynSymbolFileName, source->dynSymbolFileName, 127 );

  numStates = source->numStates;

  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {
    stateMinValue[i] = i;
    stateMaxValue[i] = i+1;
  }

  showOOBState = source->showOOBState;
  useOriginalSize = source->useOriginalSize;
  useOriginalColors = source->useOriginalColors;
  fgCb = source->fgCb;
  bgCb = source->bgCb;
  fgColor = source->fgColor;
  bgColor = source->bgColor;
  colorPvExpStr.setRaw( source->colorPvExpStr.rawString );

}

int activeDynSymbolClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h )
{

  actWin = (activeWindowClass *) aw_obj;
  xOrigin = 0;
  yOrigin = 0;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  activeMode = 0;
  index = 0;
  fgColor = actWin->defaultTextFgColor;
  bgColor = actWin->defaultBgColor;

  this->editCreate();

  return 1;

}

int activeDynSymbolClass::genericEdit ( void )
{

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeDynSymbolClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeDynSymbolClass_str5, 31 );

  strncat( title, activeDynSymbolClass_str6, 31 );

  strncpy( bufId, id, 31 );

  bufX = x;
  bufY = y;

  strncpy( bufDynSymbolFileName, dynSymbolFileName, 127 );

//    if ( controlPvExpStr.getRaw() )
//      strncpy( bufControlPvName, controlPvExpStr.getRaw(),
//       activeGraphicClass::MAX_PV_NAME );
//    else
//      strcpy( bufControlPvName, "" );

  if ( gateUpPvExpStr.getRaw() )
    strncpy( bufGateUpPvName, gateUpPvExpStr.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufGateUpPvName, "" );

  if ( gateDownPvExpStr.getRaw() )
    strncpy( bufGateDownPvName, gateDownPvExpStr.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufGateDownPvName, "" );

  bufUseOriginalSize = useOriginalSize;

  bufUseOriginalColors = useOriginalColors;

  bufUseGate = useGate;

  bufGateUpValue = gateUpValue;

  bufGateDownValue = gateDownValue;

  bufContinuous = continuous;

  bufRate = rate;

  bufInitialIndex = initialIndex;

  if ( colorPvExpStr.getRaw() )
    strncpy( bufColorPvName, colorPvExpStr.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufColorPvName, "" );

  bufUseOriginalColors = useOriginalColors;

  bufFgColor = fgColor;
  bufBgColor = bgColor;

  bufShowOOBState = showOOBState;

//    ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
//     &actWin->appCtx->entryFormX,
//     &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
//     &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
//     title, DYNSYMBOL_K_NUM_STATES, numStates,
//     dynSymbolSetItem, (void *) this, NULL, NULL, NULL );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  //ef.addTextField( activeDynSymbolClass_str7, 27, bufId, 31 );

  ef.addTextField( activeDynSymbolClass_str8, 27, &bufX );
  ef.addTextField( activeDynSymbolClass_str9, 27, &bufY );
  ef.addTextField( activeDynSymbolClass_str10, 27, bufDynSymbolFileName, 127 );
  ef.addTextField( activeDynSymbolClass_str34, 27, bufColorPvName,
   activeGraphicClass::MAX_PV_NAME );

  ef.addToggle( activeDynSymbolClass_str13, &bufUseGate );
  ef.addTextField( activeDynSymbolClass_str14, 27, bufGateUpPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addOption( activeDynSymbolClass_str15, activeDynSymbolClass_str16,
   &bufGateUpValue );
  ef.addTextField( activeDynSymbolClass_str17, 27, bufGateDownPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addOption( activeDynSymbolClass_str18, activeDynSymbolClass_str19,
   &bufGateDownValue );

  ef.addToggle( activeDynSymbolClass_str20, &bufContinuous );
  ef.addTextField( activeDynSymbolClass_str21, 27, &bufRate );

  ef.addTextField( activeDynSymbolClass_str22, 27, &bufInitialIndex );

  ef.addToggle( activeDynSymbolClass_str38, &bufShowOOBState );

  ef.addToggle( activeDynSymbolClass_str11, &bufUseOriginalSize );

  ef.addToggle( activeDynSymbolClass_str35, &bufUseOriginalColors );

  ef.addColorButton(activeDynSymbolClass_str36, actWin->ci, &fgCb,
   &bufFgColor );

  ef.addColorButton(activeDynSymbolClass_str37, actWin->ci, &bgCb,
   &bufBgColor );

  return 1;

}

int activeDynSymbolClass::edit ( void )
{

  this->genericEdit();
  ef.finished( dsc_edit_ok, dsc_edit_apply, dsc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeDynSymbolClass::editCreate ( void )
{

  this->genericEdit();
  ef.finished( dsc_edit_ok, dsc_edit_apply, dsc_edit_cancel_delete,
   this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeDynSymbolClass::readDynSymbolFile ( void )
{

int l, more, maxW, maxH, gX, gY, gW, gH, dX, dY, stat, saveLine;
int winMajor, winMinor, winRelease;
char itemName[127+1], *gotOne, *tk;
activeGraphicListPtr head, cur, next;
int i;
FILE *f;
char name[127+1];
expStringClass expStr;

  saveLine = actWin->line();

  // delete current info ( if any )

  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {
      next = cur->flink;
      delete cur->node;
      delete cur;
      cur = next;
    }
    head->flink = head;
    head->blink = head;

  }

  if ( strcmp( dynSymbolFileName, "" ) == 0 ) return 0;

  actWin->substituteSpecial( 127, dynSymbolFileName, name );

  expStr.setRaw( name );
  expStr.expand1st( actWin->numMacros, actWin->macros, actWin->expansions );

  f = actWin->openAnySymFile( expStr.getExpanded(), "r" );
  if ( !f ) {
    numStates = 0;
    return 0;
  }

  actWin->discardWinLoadData( f, &winMajor, &winMinor, &winRelease );

  // for forward compatibility
  stat = actWin->readUntilEndOfData( f, winMajor, winMinor, winRelease );
  if ( !( stat & 1 ) ) {
    fclose( f );
    actWin->setLine( saveLine );
    return stat;
  }

  index = 0;
  maxW = 0;
  maxH = 0;

  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    gotOne = fgets( itemName, 127, f ); // discard "activeGroupClass"
    if ( !gotOne ) {
      if ( i == 0 ) {
        numStates = 0;
        fclose( f );
        actWin->setLine( saveLine );
        return 0;
      }
      break;
    }

    numStates = i+1;

    tk = strtok( itemName, " \t\n" );
    if ( strcmp( tk, "activeGroupClass" ) != 0 ) {
      numStates = 0;
      fclose( f );
      actWin->setLine( saveLine );
      return 0;
    }

    fscanf( f, "%d\n", &gX );
    fscanf( f, "%d\n", &gY );
    fscanf( f, "%d\n", &gW );
    fscanf( f, "%d\n", &gH );

    if ( gW > maxW ) maxW = gW;
    if ( gH > maxH ) maxH = gH;

    dX = x - gX;
    dY = y - gY;

    fgets( itemName, 127, f ); // discard "{"

    do {

      // read and create sub-objects until a "}" is found

      gotOne = fgets( itemName, 127, f );
      if ( !gotOne ) {
        numStates = 0;
        fclose( f );
        actWin->setLine( saveLine );
        return 0;
      }

      l = strlen(itemName);
      if ( l > 127 ) l = 127;
      itemName[l-1] = 0;

      if ( strcmp( itemName, "}" ) == 0 )
        more = 0;
      else
        more = 1;

      if ( more ) {

        cur = new activeGraphicListType;
        if ( !cur ) {
          fclose( f );
          printf( "Insufficient virtual memory - abort\n" );
          numStates = 0;
          fclose( f );
          actWin->setLine( saveLine );
          return 0;
        }

        cur->node = actWin->obj.createNew( itemName );

        if ( cur->node ) {

          cur->node->createFromFile( f, itemName, actWin );

          // for forward compatibility
          stat = actWin->readUntilEndOfData( f, winMajor, winMinor,
           winRelease );
          if ( !( stat & 1 ) ) {
            fclose( f );
            actWin->setLine( saveLine );
            return stat;
          }

          // adjust origin
          cur->node->move( dX, dY );

          cur->blink = head->blink;
          head->blink->flink = cur;
          head->blink = cur;
          cur->flink = head;

        }
        else {
          fclose( f );
          printf( "Insufficient virtual memory - abort\n" );
          numStates = 0;
          actWin->setLine( saveLine );
          return 0;
        }

      }

    } while ( more );

    // for forward compatibility
    stat = actWin->readUntilEndOfData( f, winMajor, winMinor, winRelease );
    if ( !( stat & 1 ) ) {
      fclose( f );
      actWin->setLine( saveLine );
      return stat;
    }

  }

  fclose( f );

  w = maxW;
  sboxW = w;
  h = maxH;
  sboxH = h;

  actWin->setLine( saveLine );
  return 1;

}

int activeDynSymbolClass::save (
 FILE *f )
{

  fprintf( f, "%-d %-d %-d\n", DSC_MAJOR_VERSION, DSC_MINOR_VERSION,
   DSC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  writeStringToFile( f, dynSymbolFileName );

//    if ( controlPvExpStr.getRaw() )
//      writeStringToFile( f, controlPvExpStr.getRaw() );
//    else
//      writeStringToFile( f, "" );

  if ( gateUpPvExpStr.getRaw() )
    writeStringToFile( f, gateUpPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( gateDownPvExpStr.getRaw() )
    writeStringToFile( f, gateDownPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", useGate );

  fprintf( f, "%-d\n", gateUpValue );

  fprintf( f, "%-d\n", gateDownValue );

  fprintf( f, "%-d\n", continuous );

  fprintf( f, "%-g\n", rate );

  fprintf( f, "%-d\n", numStates );

  fprintf( f, "%-d\n", useOriginalSize );

  // version 1.1.0
  writeStringToFile( f, this->id );
  fprintf( f, "%-d\n", initialIndex );

  // version 1.2.0
  if ( colorPvExpStr.getRaw() )
    writeStringToFile( f, colorPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1.3.0
  fprintf( f, "%-d\n", useOriginalColors );
  fprintf( f, "%-d\n", fgColor );
  fprintf( f, "%-d\n", bgColor );

  // version 1.4.0
  fprintf( f, "%-d\n", showOOBState );

  return 1;

}

int activeDynSymbolClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int stat, resizeStat, saveW, saveH;
int major, minor, release;
char string[activeGraphicClass::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  readStringFromFile( dynSymbolFileName, 127+1, f ); actWin->incLine();

//    readStringFromFile( string, activeGraphicClass::MAX_PV_NAME+1, f );
//    actWin->incLine();
//    controlPvExpStr.setRaw( string );

  readStringFromFile( string, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  gateUpPvExpStr.setRaw( string );

  readStringFromFile( string, activeGraphicClass::MAX_PV_NAME+1, f );
   actWin->incLine();
  gateDownPvExpStr.setRaw( string );

  fscanf( f, "%d\n", &useGate ); actWin->incLine();

  fscanf( f, "%d\n", &gateUpValue ); actWin->incLine();

  fscanf( f, "%d\n", &gateDownValue ); actWin->incLine();

  fscanf( f, "%d\n", &continuous ); actWin->incLine();

  fscanf( f, "%lg\n", &rate ); actWin->incLine();

  fscanf( f, "%d\n", &numStates ); actWin->incLine();

  if ( numStates < 1 ) numStates = 1;
  if ( numStates > DYNSYMBOL_K_NUM_STATES ) numStates = DYNSYMBOL_K_NUM_STATES;

  fscanf( f, "%d\n", &useOriginalSize ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    readStringFromFile( this->id, 31+1, f ); actWin->incLine();
    fscanf( f, "%d\n", &initialIndex ); actWin->incLine();
  }
  else {
    strcpy( this->id, "" );
    initialIndex = 1;
  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( string, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    colorPvExpStr.setRaw( string );
  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    fscanf( f, "%d\n", &useOriginalColors ); actWin->incLine();
    fscanf( f, "%d\n", &fgColor ); actWin->incLine();
    fscanf( f, "%d\n", &bgColor ); actWin->incLine();
  }
  else {
    useOriginalColors = 1;
    fgColor = actWin->defaultTextFgColor;
    bgColor = actWin->defaultBgColor;
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    fscanf( f, "%d\n", &showOOBState ); actWin->incLine();
  }
  else {
    showOOBState = 0;
  }

  saveW = w;
  saveH = h;

  stat = readDynSymbolFile();
  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( activeDynSymbolClass_str23 );
  }
  else {
    if ( !useOriginalSize ) {
      if ( ( saveW != w ) || ( saveH != h ) ) {
        resizeStat = checkResizeSelectBoxAbs( -1, -1, saveW, saveH );
        if ( resizeStat & 1 ) {
          resizeSelectBoxAbs( -1, -1, saveW, saveH );
          resizeAbs( -1, -1, saveW, saveH );
        }
        else {
          actWin->appCtx->postMessage(
           activeDynSymbolClass_str24 );
        }
      }
    }
  }

  return 1;

}

int activeDynSymbolClass::erase ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;

  if ( activeMode || deleteRequest ) return 1;

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  if ( numStates > 1 )
    head = (activeGraphicListPtr) voidHead[1];
  else
    head = (activeGraphicListPtr) voidHead[0];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->removeBlink();
    cur->node->erase();

    cur = cur->flink;

  }

  return 1;

}

int activeDynSymbolClass::eraseActive ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;

  if ( !init || !activeMode || ( numStates < 1 ) ) return 1;

  if ( ( prevIndex < 0 ) || ( prevIndex >= numStates ) ) return 1;

  head = (activeGraphicListPtr) voidHead[prevIndex];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->removeBlink();
    cur->node->eraseActive();

    cur = cur->flink;

  }

  return 1;

}

int activeDynSymbolClass::draw ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( activeMode || deleteRequest ) return 1;

  for ( i=0; i<numStates; i++ ) {
    head = (activeGraphicListPtr) voidHead[i];
    cur = head->flink;
    while ( cur != head ) {
      if ( !useOriginalColors ) {
        cur->node->changeDisplayParams(
         ACTGRF_TEXTFGCOLOR_MASK | ACTGRF_FG1COLOR_MASK | ACTGRF_BGCOLOR_MASK,
	 "", 0, "", 0, "", 0, fgColor, fgColor, 0, 0, bgColor,
         0, 0 );
      }
      cur = cur->flink;
    }
  }

  actWin->drawGc.saveFg();
  actWin->drawGc.setFG( BlackPixel( actWin->d, DefaultScreen(actWin->d) ) );
  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.restoreFg();

  if ( numStates > 1 )
    head = (activeGraphicListPtr) voidHead[1];
  else
    head = (activeGraphicListPtr) voidHead[0];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->draw();

    cur = cur->flink;

  }

  return 1;

}

int activeDynSymbolClass::drawActive ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;

  if ( !init || !activeMode || ( numStates < 1 ) ) return 1;

  if ( ( index < 0 ) || ( index >= numStates ) ) return 1;

  head = (activeGraphicListPtr) voidHead[index];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->drawActive();

    cur = cur->flink;

  }

  prevIndex = index;

  return 1;

}

int activeDynSymbolClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag )
{

  *up = 1;
  *down = 1;
  *drag = 0;

  return 1;

}

void activeDynSymbolClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

  // inc index on button down, dec index on shift button down

}

void activeDynSymbolClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

}

int activeDynSymbolClass::activate (
  int pass,
  void *ptr ) {

int i, stat, opStat;
activeGraphicListPtr head;
activeGraphicListPtr cur;

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      if ( !useOriginalColors ) {
        cur->node->changeDisplayParams(
         ACTGRF_TEXTFGCOLOR_MASK | ACTGRF_FG1COLOR_MASK | ACTGRF_BGCOLOR_MASK,
	 "", 0, "", 0, "", 0, fgColor, fgColor, 0, 0, bgColor,
         0, 0 );
      }
      cur->node->activate( pass, (void *) cur );
      cur->node->removeBlink();
      cur = cur->flink;

    }

  }

  switch ( pass ) {

  case 1:

    needErase = needDraw = needRefresh = needGateUpConnect = needGateUp =
     needGateDownConnect = needGateDown = needColorInit = needColorRefresh = 0;
    aglPtr = ptr;
    iValue = 0; /* this get set via OR/AND operations */
    prevIndex = -1;
    init = 0;
//      controlExists = 0;
    gateUpExists = gateDownExists = 0;
    opComplete = 0;
    active = 0;
    activeMode = 1;
    curCount = 1;
    curControlV = controlV = 1;
    timerActive = 0;
    timer = 0;
    up = 0;
    down = 0;

#ifdef __epics__
    gateUpEventId = gateDownEventId = colorEventId = 0;
#endif

    gateUpPvConnected = gateDownPvConnected = 0;

    if ( useGate ) {

      if ( gateUpPvExpStr.getExpanded() &&
           !blank( gateUpPvExpStr.getExpanded() ) ) {

        gateUpExists = 1;

      }

      if ( gateDownPvExpStr.getExpanded() &&
           !blank( gateDownPvExpStr.getExpanded() ) ) {

        gateDownExists = 1;

      }

      if ( gateUpExists ) {
        if ( !gateDownExists ) {
          gateDownPvExpStr.setRaw( gateUpPvExpStr.getRaw() );
          gateDownExists = 1;
        }
      }
      else {
        if ( gateDownExists ) {
          gateUpPvExpStr.setRaw( gateDownPvExpStr.getRaw() );
          gateUpExists = 1;
        }
      }

    }

    if ( blank( colorPvExpStr.getExpanded() ) ) {
      colorExists = 0;
    }
    else {
      colorExists = 1;
    }

    break;

  case 2:

    if ( !opComplete ) {

      opStat = 1;

      argRec.objPtr = (void *) this;
      argRec.index = 0;
      argRec.setMask = (unsigned int) 1 << i;
      argRec.clrMask = ~(argRec.setMask);

#ifdef __epics__

      if ( gateUpExists ) {
        stat = ca_search_and_connect( gateUpPvExpStr.getExpanded(),
         &gateUpPvId, dynSymbol_monitor_gateUp_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeDynSymbolClass_str25 );
          opStat = 0;
        }
      }

      if ( gateDownExists ) {
        stat = ca_search_and_connect( gateDownPvExpStr.getExpanded(),
         &gateDownPvId, dynSymbol_monitor_gateDown_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeDynSymbolClass_str26 );
          opStat = 0;
        }
      }

      if ( colorExists ) {
        stat = ca_search_and_connect( colorPvExpStr.getExpanded(),
         &colorPvId, dynSymbol_monitor_color_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeDynSymbolClass_str26 );
          opStat = 0;
        }
      }

#endif

      // initialIndex is used only when no pv's are connected
      if ( !gateUpExists && !gateDownExists ) {
        init = 1;
        active = 1;
        if ( initialIndex > numStates-1 ) initialIndex = numStates-1;
        if ( showOOBState ) {
          if ( initialIndex < 0 ) initialIndex = 0;
	}
	else {
          if ( initialIndex < 1 ) initialIndex = 1;
	}
        controlV = initialIndex;
        index = initialIndex;
        curCount = initialIndex;
      }

      if ( opStat & 1 ) opComplete = 1;

      if ( opComplete ) {

        if ( continuous && !useGate ) {
          timer = XtAppAddTimeOut( actWin->appCtx->appContext(),
           (unsigned long) 1000, dsc_updateControl, this );
          timerActive = 1;
          up = 1;
          init = 1;
          active = 1;
          if ( showOOBState ) {
            curCount = 0;
            controlV = curCount;
            index = 0;
	  }
	  else {
            curCount = 1;
            controlV = curCount;
            index = 1;
	  }
        }

      }

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

int activeDynSymbolClass::deactivate (
  int pass
) {

int i, stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;

  timerActive = 0;

  if ( timer ) {
    XtRemoveTimeOut( timer );
    timer = 0;
  }

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->deactivate( pass );
      cur->node->removeBlink();

      cur = cur->flink;

    }

  }

  if ( pass == 1 ) {

  active = 0;
  activeMode = 0;

#ifdef __epics__

  if ( gateUpEventId ) {
    stat = ca_clear_event( gateUpEventId );
    if ( stat != ECA_NORMAL )
      printf( activeDynSymbolClass_str27 );
    gateUpEventId = 0;
  }

  if ( gateDownEventId ) {
    stat = ca_clear_event( gateDownEventId );
    if ( stat != ECA_NORMAL )
      printf( activeDynSymbolClass_str28 );
    gateDownEventId = 0;
  }

  if ( colorEventId ) {
    stat = ca_clear_event( colorEventId );
    if ( stat != ECA_NORMAL )
      printf( activeDynSymbolClass_str28 );
    colorEventId = 0;
  }

  if ( gateUpExists ) {
    stat = ca_clear_channel( gateUpPvId );
    if ( stat != ECA_NORMAL )
      printf( activeDynSymbolClass_str29 );
  }

  if ( gateDownExists ) {
    stat = ca_clear_channel( gateDownPvId );
    if ( stat != ECA_NORMAL )
      printf( activeDynSymbolClass_str30 );
  }

  if ( colorExists ) {
    stat = ca_clear_channel( colorPvId );
    if ( stat != ECA_NORMAL )
      printf( activeDynSymbolClass_str30 );
  }

#endif

  }

  return 1;

}

int activeDynSymbolClass::moveSelectBox (
  int _x,
  int _y )
{

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  sboxX += _x;
  sboxY += _y;

  for ( i=0; i<numStates; i++ ) {

  head = (activeGraphicListPtr) voidHead[i];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->moveSelectBox( _x, _y );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  }

  return 1;

}

int activeDynSymbolClass::moveSelectBoxAbs (
  int _x,
  int _y )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead[0];
activeGraphicListPtr cur;
int dx, dy;
int i;

  dx = _x - sboxX;
  dy = _y - sboxY;

  sboxX = _x;
  sboxY = _y;

  for ( i=0; i<numStates; i++ ) {

  head = (activeGraphicListPtr) voidHead[i];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->moveSelectBox( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;

  }
 
  }

  return 1;

}

int activeDynSymbolClass::moveSelectBoxMidpointAbs (
  int _x,
  int _y )
{

activeGraphicListPtr head = (activeGraphicListPtr) voidHead[0];
activeGraphicListPtr cur;
int dx, dy;
int i;

  dx = _x - sboxW/2 - sboxX;
  dy = _y - sboxH/2 - sboxY;

  sboxX = _x - sboxW/2;
  sboxY = _y - sboxH/2;

  for ( i=0; i<numStates; i++ ) {

  head = (activeGraphicListPtr) voidHead[i];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->moveSelectBox( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  }

  return 1;

}

int activeDynSymbolClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

int i, stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead[0];
activeGraphicListPtr cur;

  if ( useOriginalSize ) return 0;

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      stat = cur->node->checkResizeSelectBox( _x, _y, _w, _h );
      if ( !( stat & 1 ) ) {
        return stat;
      }

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h )
{

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i, savex, savey, savew, saveh, stat, ret_stat;

  if ( useOriginalSize ) return 1;

  savex = sboxX;
  savey = sboxY;
  savew = sboxW;
  saveh = sboxH;

  ret_stat = 1;

  sboxX += _x;
  sboxY += _y;

  sboxW += _w;

//   if ( sboxW < 5 ) {
//     sboxX = savex;
//     sboxW = savew;
//     ret_stat = 0;
//   }

  sboxH += _h;

//   if ( sboxH < 5 ) {
//     sboxY = savey;
//     sboxH = saveh;
//     ret_stat = 0;
//   }

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      stat = cur->node->resizeSelectBox( _x, _y, _w, _h );
      if ( stat & 1 ) {
        cur->node->updateDimensions();
      }
      else {
        ret_stat = stat;
      }

      cur = cur->flink;

    }

  }

  return ret_stat;

}

int activeDynSymbolClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

int i, stat;
activeGraphicListPtr head = (activeGraphicListPtr) voidHead[0];
activeGraphicListPtr cur;
int deltaX, deltaY;
double xScaleFactor, yScaleFactor, newX, newY, newW, newH;

  if ( useOriginalSize ) return 0;

  if ( _x == -1 )
    deltaX = 0;
  else
    deltaX = _x - sboxX;

  if ( _y == -1 )
    deltaY = 0;
  else
    deltaY = _y - sboxY;

  if ( _w == -1 )
    xScaleFactor = 1.0;
  else
    xScaleFactor = (double) _w / (double) sboxW;

  if ( _h == -1 )
    yScaleFactor = 1.0;
  else
    yScaleFactor = (double) _h / (double) sboxH;

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      newX = x + deltaX +
       (int) ( (double) ( cur->node->getX0() - x )
       * xScaleFactor + 0.5 );

      newW =
       (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

      newY = y + deltaY +
       (int) ( (double) ( cur->node->getY0() - y )
       * yScaleFactor + 0.5 );

      newH =
       (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

      stat = cur->node->checkResizeSelectBoxAbs( (int) newX, (int) newY,
       (int) newW, (int) newH );
      if ( !( stat & 1 ) ) {
        return stat;
      }

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h )
{

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i, stat, ret_stat;
int deltaX, deltaY;
double xScaleFactor, yScaleFactor, newX, newY, newW, newH;

  if ( useOriginalSize ) return 1;

  ret_stat = 1;

  if ( _w > 0 ) {
    if ( _w < 5 ) {
      return 0;
    }
  }

  if ( _h > 0 ) {
    if ( _h < 5 ) {
      return 0;
    }
  }

  if ( _x == -1 )
    deltaX = 0;
  else
    deltaX = _x - sboxX;

  if ( _y == -1 )
    deltaY = 0;
  else
    deltaY = _y - sboxY;

  if ( _w == -1 )
    xScaleFactor = 1.0;
  else
    xScaleFactor = (double) _w / (double) sboxW;

  if ( _h == -1 )
    yScaleFactor = 1.0;
  else
    yScaleFactor = (double) _h / (double) sboxH;


  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      newX = x + deltaX +
       (int) ( (double) ( cur->node->getX0() - x )
       * xScaleFactor + 0.5 );

      newW =
       (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

      newY = y + deltaY +
       (int) ( (double) ( cur->node->getY0() - y )
       * yScaleFactor + 0.5 );

      newH =
       (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

      stat = cur->node->resizeSelectBoxAbs( (int) newX, (int) newY,
       (int) newW, (int) newH );
      if ( stat & 1 ) {
        cur->node->updateDimensions();
      }
      else {
        ret_stat = stat;
      }

      cur = cur->flink;

    }

  }

  if ( _x > 0 ) sboxX = _x;
  if ( _y > 0 ) sboxY = _y;
  if ( _w > 0 ) sboxW = _w;
  if ( _h > 0 ) sboxH = _h;

  return ret_stat;

}

int activeDynSymbolClass::move (
  int _x,
  int _y ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead[0];
activeGraphicListPtr cur;
int i;

  x += _x;
  y += _y;

  for ( i=0; i<numStates; i++ ) {

  head = (activeGraphicListPtr) voidHead[i];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->move( _x, _y );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  }

  return 1;

}

int activeDynSymbolClass::moveAbs (
  int _x,
  int _y ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead[0];
activeGraphicListPtr cur;
int dx, dy;
int i;

  dx = _x - x;
  dy = _y - y;

  x = _x;
  y = _y;

  for ( i=0; i<numStates; i++ ) {

  head = (activeGraphicListPtr) voidHead[i];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->move( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  }

  return 1;

}

int activeDynSymbolClass::moveMidpointAbs (
  int _x,
  int _y ) {

activeGraphicListPtr head = (activeGraphicListPtr) voidHead[0];
activeGraphicListPtr cur;
int dx, dy;
int i;

  dx = _x - w/2 - x;
  dy = _y - h/2 - y;

  x = _x - w/2;
  y = _y - h/2;

  for ( i=0; i<numStates; i++ ) {

  head = (activeGraphicListPtr) voidHead[i];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->move( dx, dy );
    cur->node->updateDimensions();

    cur = cur->flink;

  }

  }

  return 1;

}

int activeDynSymbolClass::resize (
  int _x,
  int _y,
  int _w,
  int _h ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( useOriginalSize ) return 1;

  x += _x;
  y += _y;
  w += _w;
  h += _h;

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->resize( _x, _y, _w, _h );
      cur->node->updateDimensions();

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::resizeAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;
int deltaX, deltaY;
double xScaleFactor, yScaleFactor, newX, newY, newW, newH;
int i;

  if ( useOriginalSize ) return 1;

  if ( _x == -1 )
    deltaX = 0;
  else
    deltaX = _x - x;

  if ( _y == -1 )
    deltaY = 0;
  else
    deltaY = _y - y;

  if ( _w == -1 )
    xScaleFactor = 1.0;
  else
    xScaleFactor = (double) _w / (double) w;

  if ( _h == -1 )
    yScaleFactor = 1.0;
  else
    yScaleFactor = (double) _h / (double) h;

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      newX = x + deltaX +
       (int) ( (double) ( cur->node->getX0() - x )
       * xScaleFactor + 0.5 );

      newW =
       (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

      newY = y + deltaY +
       (int) ( (double) ( cur->node->getY0() - y )
       * yScaleFactor + 0.5 );

      newH =
       (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

      cur->node->resizeAbs( (int) newX, (int) newY, (int) newW, (int) newH );
      cur->node->resizeSelectBoxAbs( (int) newX, (int) newY, (int) newW,
       (int) newH );
      cur->node->updateDimensions();

      cur = cur->flink;

    }

  }

  if ( _x > 0 ) x = _x;
  if ( _y > 0 ) y = _y;
  if ( _w > 0 ) w = _w;
  if ( _h > 0 ) h = _h;

  return 1;

}

void activeDynSymbolClass::updateGroup ( void ) { // for paste operation

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( deleteRequest ) return;

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->actWin = actWin;

      cur = cur->flink;

    }

  }

}

int activeDynSymbolClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( deleteRequest ) return 1;

//    controlPvExpStr.expand1st( numMacros, macros, expansions );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->expand1st( numMacros, macros, expansions );

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( deleteRequest ) return 1;

//    controlPvExpStr.expand2nd( numMacros, macros, expansions );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->expand2nd( numMacros, macros, expansions );

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::containsMacros ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( deleteRequest ) return 1;

//    if ( controlPvExpStr.containsPrimaryMacros() ) return 1;

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      if ( cur->node->containsMacros() ) return 1;

      cur = cur->flink;

    }

  }

  return 0;

}

void activeDynSymbolClass::executeDeferred ( void ) {

double v;
int stat, i, nguc, ngdc, ngu, ngd, nr, ne, nd, ncolori, ncr;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  v = curControlV;
  nguc = needGateUpConnect; needGateUpConnect = 0;
  ngdc = needGateDownConnect; needGateDownConnect = 0;
  ngu = needGateUp; needGateUp = 0;
  ngd = needGateDown; needGateDown = 0;
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  ncolori = needColorInit; needColorInit = 0;
  ncr = needColorRefresh; needColorRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

#ifdef __epics__

  if ( nguc ) {

    if ( !gateUpEventId ) {

      stat = ca_add_masked_array_event( DBR_LONG, 1,
       gateUpPvId, dynSymbol_gateUpUpdate, (void *) &argRec,
       (float) 0.0, (float) 0.0, (float) 0.0, &gateUpEventId,
       DBE_VALUE );
      if ( stat != ECA_NORMAL )
        printf( activeDynSymbolClass_str31 );

    }

    if ( ( gateDownPvConnected || !gateDownExists ) &&
         ( colorPvConnected || !colorExists ) ) {
      active = 1;
    }

  }

  if ( ngdc ) {

    if ( !gateDownEventId ) {

      stat = ca_add_masked_array_event( DBR_LONG, 1,
       gateDownPvId, dynSymbol_gateDownUpdate, (void *) &argRec,
       (float) 0.0, (float) 0.0, (float) 0.0, &gateDownEventId,
       DBE_VALUE );
      if ( stat != ECA_NORMAL )
        printf( activeDynSymbolClass_str32 );

    }

    if ( ( gateUpPvConnected || !gateUpExists ) &&
         ( colorPvConnected || !colorExists ) ) {
      active = 1;
    }

  }

  if ( ncolori ) {

    if ( !colorEventId ) {

      stat = ca_add_masked_array_event( DBR_DOUBLE, 1,
       colorPvId, dynSymbol_colorUpdate, (void *) this,
       (float) 0.0, (float) 0.0, (float) 0.0, &colorEventId,
       DBE_VALUE );
      if ( stat != ECA_NORMAL )
        printf( activeDynSymbolClass_str32 );

    }

    if ( ( gateUpPvConnected || !gateUpExists ) &&
         ( gateDownPvConnected || !gateDownExists ) ) {

      active = 1;
    }

  }

#endif

//----------------------------------------------------------------------------

  if ( ngu ) {

    if ( !init ) {
      curCount = numStates-1;
      controlV = curCount;
      index = curCount;
      init = 1;
      stat = smartDrawAllActive();
    }

    up = 1;
    down = 0;
    timer = XtAppAddTimeOut( actWin->appCtx->appContext(),
     (unsigned long) (rate*1000.0), dsc_updateControl, this );
    timerActive = 1;

  }

//----------------------------------------------------------------------------

  if ( ngd ) {

    if ( !init ) {
      if ( showOOBState ) {
        curCount = 0;
      }
      else {
        curCount = 1;
      }
      controlV = curCount;
      index = curCount;
      init = 1;
      stat = smartDrawAllActive();
    }

    up = 0;
    down = 1;
    if ( !continuous ) {
      timer = XtAppAddTimeOut( actWin->appCtx->appContext(),
       (unsigned long) (rate*1000.0), dsc_updateControl, this );
      timerActive = 1;
    }

  }

//----------------------------------------------------------------------------

  if ( ncr ) {

    if ( !init ) {
      curCount = numStates-1;
      controlV = curCount;
      index = curCount;
      init = 1;
      stat = smartDrawAllActive();
    }

    updateColors( curColorV );

  }

//----------------------------------------------------------------------------


  if ( nr ) {

    controlV = v;

    index = 0;
    for ( i=0; i<numStates; i++ ) {

      if ( ( controlV >= stateMinValue[i] ) &&
           ( controlV < stateMaxValue[i] ) ) {

        index = i;
        break;

      }

    }

    if ( index != prevIndex ) {
      stat = eraseActive();
      stat = smartDrawAllActive();
      // stat = drawActive();
    }

  }

//----------------------------------------------------------------------------

  if ( ne ) {
    eraseActive();
  }

//----------------------------------------------------------------------------

  if ( nd ) {
    //drawActive();
    stat = smartDrawAllActive();
  }

//----------------------------------------------------------------------------

}

int activeDynSymbolClass::setProperty (
  char *prop,
  char *value )
{

  if ( strcmp( prop, "gate" ) == 0 ) {

    if ( strcmp( value, "up" ) == 0 ) {

      if ( up == 0 ) {
        up = 1;
        down = 0;
        if ( !continuous ) {
          timer = XtAppAddTimeOut( actWin->appCtx->appContext(),
           (unsigned long) (rate*1000.0), dsc_updateControl, this );
          timerActive = 1;
        }
      }

    }
    else if ( strcmp( value, "down" ) == 0 ) {

      if ( down == 0 ) {
        up = 0;
        down = 1;
        if ( !continuous ) {
          timer = XtAppAddTimeOut( actWin->appCtx->appContext(),
           (unsigned long) (rate*1000.0), dsc_updateControl, this );
          timerActive = 1;
        }
      }

    }

    return 1;

  }
  else if ( strcmp( prop, "continuous" ) == 0 ) {

    if ( strcmp( value, "yes" ) == 0 ) {

      if ( !continuous ) {
        continuous = 1;
        if ( !up && !down ) {
          if ( curCount == 1 )
            up = 1;
          else if ( curCount == numStates-1 )
            down = 1;
          else
            up = 1;
	}
        timer = XtAppAddTimeOut( actWin->appCtx->appContext(),
         (unsigned long) (rate*1000.0), dsc_updateControl, this );
        timerActive = 1;
      }

    }
    else if ( strcmp( value, "no" ) == 0 ) {

      if ( continuous ) {
        continuous = 0;
        timerActive = 0;
        if ( down )
          curCount = 1;
        else if ( up )
          curCount = numStates-1;
      }

    }

    return 1;

  }

  return 0;

}

char *activeDynSymbolClass::firstDragName ( void ) {

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeDynSymbolClass::nextDragName ( void ) {

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeDynSymbolClass::dragValue (
  int i ) {

  if ( i == 0 ) {
    return dynSymbolFileName;
  }
  else if ( i == 1 ) {
    return gateUpPvExpStr.getExpanded();
  }
  else {
    return gateDownPvExpStr.getExpanded();
  }

}

void activeDynSymbolClass::changePvNames (
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
      gateUpPvExpStr.setRaw( ctlPvs[0] );
      gateDownPvExpStr.setRaw( ctlPvs[0] );
    }
  }

}

void activeDynSymbolClass::updateColors (
  double colorValue )
{

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->updateColors( colorValue );

      cur = cur->flink;

    }

  }

  if ( ( index < 0 ) || ( index >= numStates ) ) return;

  head = (activeGraphicListPtr) voidHead[index];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->eraseUnconditional();

    cur = cur->flink;

  }

  smartDrawAllActive();

}

int activeDynSymbolClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction ) // '+'=clockwise, '-'=counter clockwise
{

  actWin->appCtx->postMessage( activeDynSymbolClass_str39 );

  return 1;

}

int activeDynSymbolClass::flip (
  int xOrigin,
  int yOrigin,
  char direction )
{

  actWin->appCtx->postMessage( activeDynSymbolClass_str40 );

  return 1;

}

void activeDynSymbolClass::flushUndo ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  undoObj.flush();

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      cur->node->flushUndo();

      cur = cur->flink;

    }

  }

}

int activeDynSymbolClass::addUndoCreateNode (
  undoClass *_undoObj )
{

int stat;

 return 1;

  stat = _undoObj->addCreateNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  return 1;

}

int activeDynSymbolClass::addUndoMoveNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  stat = _undoObj->addMoveNode( this, NULL, x, y );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoMoveNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::addUndoResizeNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  stat = _undoObj->addResizeNode( this, NULL, x, y, w, h );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoResizeNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::addUndoCopyNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

 return 1;

  stat = _undoObj->addCopyNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoCopyNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::addUndoCutNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

 return 1;

  stat = _undoObj->addCutNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoCutNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::addUndoPasteNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

 return 1;

  stat = _undoObj->addPasteNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoPasteNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::addUndoReorderNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

 return 1;

  stat = _undoObj->addReorderNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoReorderNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::addUndoEditNode (
  undoClass *_undoObj )
{

int stat;
undoDynSymbolOpClass *undoDynSymbolOpPtr;

  undoDynSymbolOpPtr = new undoDynSymbolOpClass( this );

  stat = _undoObj->addEditNode( this, undoDynSymbolOpPtr );
  if ( !( stat & 1 ) ) return stat;

  return 1;

}

int activeDynSymbolClass::addUndoGroupNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

 return 1;

  stat = _undoObj->addGroupNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoGroupNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::addUndoRotateNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  stat = _undoObj->addRotateNode( this, NULL, x, y, w, h );
  if ( !( stat & 1 ) ) return stat;
 
  return 1;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoRotateNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::addUndoFlipNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  stat = _undoObj->addFlipNode( this, NULL, x, y, w, h );
  if ( !( stat & 1 ) ) return stat;

  return 1;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoFlipNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int activeDynSymbolClass::undoCreate (
  undoOpClass *opPtr
) {

  return 1;

}

int activeDynSymbolClass::undoMove (
  undoOpClass *opPtr,
  int x,
  int y )
{

int stat;

  moveAbs( x, y );
  moveSelectBoxAbs( x, y );

  stat = undoObj.performSubUndo();
  if ( !( stat & 1 ) ) XBell( actWin->d, 50 );

  return 1;

}

int activeDynSymbolClass::undoResize (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h )
{

int stat;

  resizeAbsFromUndo( x, y, w, h );
  resizeSelectBoxAbsFromUndo( x, y, w, h );

  stat = undoObj.performSubUndo();
  if ( !( stat & 1 ) ) XBell( actWin->d, 50 );

  return 1;

}

int activeDynSymbolClass::undoCopy (
  undoOpClass *opPtr
) {

  return 1;

}

int activeDynSymbolClass::undoCut (
  undoOpClass *opPtr
) {

  return 1;

}

int activeDynSymbolClass::undoPaste (
  undoOpClass *opPtr
) {

  return 1;

}

int activeDynSymbolClass::undoReorder (
  undoOpClass *opPtr
) {

  return 1;

}

int activeDynSymbolClass::undoEdit (
  undoOpClass *opPtr
) {

undoDynSymbolOpClass *ptr = (undoDynSymbolOpClass *) opPtr;
activeGraphicListPtr head, cur, next, curSource, sourceHead;
int i;

// ============================================================

  // delete current image list, saved image list from opPtr (from undo object)
  // will be restored to symbol object

  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {
      next = cur->flink;
      delete cur->node;
      delete cur;
      cur = next;
    }
    head->flink = NULL;
    head->blink = NULL;
    delete head;

  }

// ============================================================

  // copy base class info
  x = ptr->dso->x;
  y = ptr->dso->y;
  w = ptr->dso->w;
  h = ptr->dso->h;
  sboxX = ptr->dso->sboxX;
  sboxY = ptr->dso->sboxY;
  sboxW = ptr->dso->sboxW;
  sboxH = ptr->dso->sboxH;
  orientation = ptr->dso->orientation;
  nextToEdit = ptr->dso->nextToEdit;
  nextSelectedToEdit = NULL;
  inGroup = ptr->dso->inGroup;

// ============================================================

  // restore saved symbol image list
  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {

    head = new activeGraphicListType;
    head->flink = head;
    head->blink = head;

    sourceHead = (activeGraphicListPtr) ptr->dso->voidHead[i];
    curSource = sourceHead->flink;
    while ( curSource != sourceHead ) {

      next = curSource->flink;

      cur = curSource;

      cur->blink = head->blink;
      head->blink->flink = cur;
      head->blink = cur;
      cur->flink = head;

      curSource = next;

    }

    voidHead[i] = (void *) head;

  }

  // make saved symbol image list empty (when saved
  // object is deleted, we don't want image list to
  // be deleted (which has been restored to current symbol object)
  for ( i=0; i<DYNSYMBOL_K_NUM_STATES; i++ ) {
    sourceHead = (activeGraphicListPtr) ptr->dso->voidHead[i];
    sourceHead->flink = sourceHead;
    sourceHead->blink = sourceHead;
  }

// ============================================================

  // restore remaining attributes

  index = 0;
  initialIndex = ptr->dso->initialIndex;
  controlV = ptr->dso->controlV;
  controlVal = ptr->dso->controlVal;
  iValue = ptr->dso->iValue;
  gateUpPvExpStr.setRaw( ptr->dso->gateUpPvExpStr.rawString );
  gateDownPvExpStr.setRaw( ptr->dso->gateDownPvExpStr.rawString );
  useGate = ptr->dso->useGate;
  continuous = ptr->dso->continuous;
  rate = ptr->dso->rate;
  gateUpValue = ptr->dso->gateUpValue;
  gateDownValue = ptr->dso->gateDownValue;

  strncpy( dynSymbolFileName, ptr->dso->dynSymbolFileName, 127 );

  numStates = ptr->dso->numStates;
  for ( i=0; i<ptr->dso->numStates; i++ ) {
    stateMinValue[i] = ptr->dso->stateMinValue[i];
    stateMaxValue[i] = ptr->dso->stateMaxValue[i];
  }

  showOOBState = ptr->dso->showOOBState;
  useOriginalSize = ptr->dso->useOriginalSize;
  useOriginalColors = ptr->dso->useOriginalColors;
  fgCb = ptr->dso->fgCb;
  bgCb = ptr->dso->bgCb;
  fgColor = ptr->dso->fgColor;
  bgColor = ptr->dso->bgColor;
  colorPvExpStr.setRaw( ptr->dso->colorPvExpStr.rawString );

  return 1;

}

int activeDynSymbolClass::undoGroup (
  undoOpClass *opPtr
) {

  return 1;

}

int activeDynSymbolClass::undoRotate (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h )
{

  //printf( "activeDynSymbolClass::undoRotate - not implemented\n" );

  return 1;

}

int activeDynSymbolClass::undoFlip (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h )
{

  //printf( "activeDynSymbolClass::undoFlip - not implemented\n" );

  return 1;

}
