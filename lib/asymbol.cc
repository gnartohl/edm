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

#define __asymbol_cc 1

class undoAniSymbolOpClass;

#include "asymbol.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

class undoAniSymbolOpClass : public undoOpClass {

public:

aniSymbolClass *anso;

undoAniSymbolOpClass ()
{

  fprintf( stderr, "undoAniSymbolOpClass::undoAniSymbolOpClass\n" );
  anso = NULL;

}

undoAniSymbolOpClass (
  aniSymbolClass *_anso
) {

int i;
activeGraphicListPtr head, cur, next, sourceHead, curSource;

  // fprintf( stderr, "undoAniSymbolOpClass::undoAniSymbolOpClass\n" );

  // copy display list and editable attributes from current symbol
  anso = new aniSymbolClass;

  // copy base class info
  anso->x = _anso->x;
  anso->y = _anso->y;
  anso->w = _anso->w;
  anso->h = _anso->h;
  anso->sboxX = _anso->sboxX;
  anso->sboxY = _anso->sboxY;
  anso->sboxW = _anso->sboxW;
  anso->sboxH = _anso->sboxH;
  anso->orientation = _anso->orientation;
  anso->nextToEdit = _anso->nextToEdit;
  anso->nextSelectedToEdit = NULL;
  anso->inGroup = _anso->inGroup;

  // steal current symbol obj list - we don't want a copy,
  // we want the current information without changing the
  // value of pointers. These pointers are referenced in the
  // symbol's undo object used to undo its children.
  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {

    head = new activeGraphicListType;
    head->flink = head;
    head->blink = head;

    sourceHead = (activeGraphicListPtr) _anso->voidHead[i];
    curSource = sourceHead->flink;
    while ( curSource != sourceHead ) {

      next = curSource->flink;

      cur = curSource;
      cur->node->updateBlink(0);

      cur->blink = head->blink;
      head->blink->flink = cur;
      head->blink = cur;
      cur->flink = head;

      curSource = next;

    }

    anso->voidHead[i] = (void *) head;

  }

  // make current symbol obj list empty (because we stole all
  // the information above); when the edit operation proceeds and
  // the display list is destroyed and recreated from the current
  // contents of the symbol file, we don't want the current image
  // list destroyed.
  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {
    sourceHead = (activeGraphicListPtr) _anso->voidHead[i];
    sourceHead->flink = sourceHead;
    sourceHead->blink = sourceHead;
  }

  anso->index = 0;
  for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
    anso->controlVals[i] = 0.0;
    anso->controlPvExpStr[i].setRaw( _anso->controlPvExpStr[i].rawString );
    anso->xorMask[i] = 0;
    anso->andMask[i] = 0;
    anso->shiftCount[i] = 0;
    strcpy( anso->cXorMask[i], "0" );
    strcpy( anso->cAndMask[i], "0" );
  }

  strncpy( anso->symbolFileName, _anso->symbolFileName, 127 );

  anso->numStates = _anso->numStates;
  for ( i=0; i<_anso->numStates; i++ ) {
    anso->stateMinValue[i] = _anso->stateMinValue[i];
    anso->stateMaxValue[i] = _anso->stateMaxValue[i];
  }

  anso->useOriginalSize = _anso->useOriginalSize;

  anso->binaryTruthTable = _anso->binaryTruthTable;

  anso->orientation = _anso->orientation;

  anso->numPvs = _anso->numPvs;

  anso->useOriginalColors = _anso->useOriginalColors;
  anso->fgCb = _anso->fgCb;
  anso->bgCb = _anso->bgCb;
  anso->fgColor = _anso->fgColor;
  anso->bgColor = _anso->bgColor;
  anso->colorPvExpStr.setRaw( _anso->colorPvExpStr.rawString );
  anso->xPvExpStr.setRaw( _anso->xPvExpStr.rawString );
  anso->yPvExpStr.setRaw( _anso->yPvExpStr.rawString );
  anso->anglePvExpStr.setRaw( _anso->anglePvExpStr.rawString );

}

~undoAniSymbolOpClass ()
{

  if ( anso ) {
    delete anso;
    anso = NULL;
  }

}

};

static void asymUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

aniSymbolClass *anso = (aniSymbolClass *) client;

  if ( !anso->init ) {
    anso->needToDrawUnconnected = 1;
    anso->needDraw = 1;
    anso->actWin->addDefExeNode( anso->aglPtr );
  }

  anso->unconnectedTimer = 0;

}

static void asymbol_monitor_control_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

objPlusIndexPtr1 ptr = (objPlusIndexPtr1) userarg;
aniSymbolClass *anso = (aniSymbolClass *) ptr->objPtr;

  if ( pv->is_valid() ) {

    anso->needConnectInit = 1;
    anso->needConnect[ptr->index] = 1;

    anso->notControlPvConnected &= ptr->clrMask;

  }
  else {

    anso->notControlPvConnected |= ptr->setMask;
    anso->active = 0;
    anso->bufInvalidate();
    anso->needRefresh = 1;

  }

  anso->actWin->appCtx->proc->lock();
  anso->actWin->addDefExeNode( anso->aglPtr );
  anso->actWin->appCtx->proc->unlock();

}

static void asymbol_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

aniSymbolClass *anso = (aniSymbolClass *) userarg;

  if ( pv->is_valid() ) {

    anso->needColorInit = 1;
    anso->colorPvConnected = 1;

  }
  else {

    anso->colorPvConnected = 0;
    anso->active = 0;
    anso->bufInvalidate();
    anso->needDraw = 1;

  }

  anso->actWin->appCtx->proc->lock();
  anso->actWin->addDefExeNode( anso->aglPtr );
  anso->actWin->appCtx->proc->unlock();

}

static void asymbol_monitor_x_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

aniSymbolClass *anso = (aniSymbolClass *) userarg;

  if ( pv->is_valid() ) {

    anso->needXInit = 1;
    anso->xPvConnected = 1;

  }
  else {

    anso->xPvConnected = 0;
    anso->active = 0;
    anso->bufInvalidate();
    anso->needDraw = 1;

  }

  anso->actWin->appCtx->proc->lock();
  anso->actWin->addDefExeNode( anso->aglPtr );
  anso->actWin->appCtx->proc->unlock();

}

static void asymbol_monitor_y_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

aniSymbolClass *anso = (aniSymbolClass *) userarg;

  if ( pv->is_valid() ) {

    anso->needYInit = 1;
    anso->yPvConnected = 1;

  }
  else {

    anso->yPvConnected = 0;
    anso->active = 0;
    anso->bufInvalidate();
    anso->needDraw = 1;

  }

  anso->actWin->appCtx->proc->lock();
  anso->actWin->addDefExeNode( anso->aglPtr );
  anso->actWin->appCtx->proc->unlock();

}

static void asymbol_monitor_angle_connect_state (
  ProcessVariable *pv,
  void *userarg )
{

aniSymbolClass *anso = (aniSymbolClass *) userarg;

  if ( pv->is_valid() ) {

    anso->needAngleInit = 1;
    anso->anglePvConnected = 1;

  }
  else {

    anso->anglePvConnected = 0;
    anso->active = 0;
    anso->bufInvalidate();
    anso->needDraw = 1;

  }

  anso->actWin->appCtx->proc->lock();
  anso->actWin->addDefExeNode( anso->aglPtr );
  anso->actWin->appCtx->proc->unlock();

}

static void asymbol_controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

objPlusIndexPtr1 ptr = (objPlusIndexPtr1) userarg;
aniSymbolClass *anso = (aniSymbolClass *) ptr->objPtr;
unsigned int uiVal;
int i;

  if ( anso->activeMode ) {

    if ( anso->binaryTruthTable ) {

      anso->controlVals[ptr->index] = pv->get_double();
      if ( anso->controlVals[ptr->index] != 0 )
        anso->iValue |= ptr->setMask;
      else
        anso->iValue &= ptr->clrMask;
      anso->curControlV = (double) anso->iValue;

    }
    else {

      if ( anso->numPvs == 1 ) {

        if ( ( anso->andMask[ptr->index] == 0 ) &&
             ( anso->xorMask[ptr->index] == 0 ) &&
             ( anso->shiftCount[ptr->index] == 0 ) ) {

          anso->curControlV = pv->get_double();

	}
	else {

          anso->curUiVal[ptr->index] = (unsigned int) pv->get_int();

          if ( anso->andMask[ptr->index] ) {
            anso->curUiVal[ptr->index] &= anso->andMask[ptr->index];
	  }

          anso->curUiVal[ptr->index] ^= anso->xorMask[ptr->index];

          if ( anso->shiftCount[ptr->index] < 0 ) {
            anso->curUiVal[ptr->index] =
	      anso->curUiVal[ptr->index] >> abs(anso->shiftCount[ptr->index]);
          }
          else {
            anso->curUiVal[ptr->index] =
             anso->curUiVal[ptr->index] << anso->shiftCount[ptr->index];
	  }

          anso->curControlV = (double) anso->curUiVal[ptr->index];

	}

      }
      else {

        anso->curUiVal[ptr->index] = (unsigned int) pv->get_int();

        if ( anso->andMask[ptr->index] ) {
          anso->curUiVal[ptr->index] &= anso->andMask[ptr->index];
        }

        anso->curUiVal[ptr->index] ^= anso->xorMask[ptr->index];

        if ( anso->shiftCount[ptr->index] < 0 ) {
          anso->curUiVal[ptr->index] =
           anso->curUiVal[ptr->index] >> abs(anso->shiftCount[ptr->index]);
        }
        else {
          anso->curUiVal[ptr->index] =
           anso->curUiVal[ptr->index] << anso->shiftCount[ptr->index];
	}

        uiVal = 0;
        for ( i=0; i<anso->numPvs; i++ ) {
          uiVal |= anso->curUiVal[i];
	}

        anso->curControlV = (double) uiVal;

      }        

    }

    anso->needRefresh = 1;
    anso->actWin->appCtx->proc->lock();
    anso->actWin->addDefExeNode( anso->aglPtr );
    anso->actWin->appCtx->proc->unlock();

  }

}

static void asymbol_colorUpdate (
  ProcessVariable *pv,
  void *userarg )
{

aniSymbolClass *anso = (aniSymbolClass *) userarg;

  if ( anso->activeMode ) {

    anso->curColorV = pv->get_double();

    anso->needColorRefresh = 1;
    anso->actWin->appCtx->proc->lock();
    anso->actWin->addDefExeNode( anso->aglPtr );
    anso->actWin->appCtx->proc->unlock();

  }

}

static void asymbol_xUpdate (
  ProcessVariable *pv,
  void *userarg )
{

aniSymbolClass *anso = (aniSymbolClass *) userarg;

  if ( anso->activeMode ) {

    anso->curXV = pv->get_double();

    anso->needPosRefresh = 1;
    anso->actWin->appCtx->proc->lock();
    anso->actWin->addDefExeNode( anso->aglPtr );
    anso->actWin->appCtx->proc->unlock();

  }

}

static void asymbol_yUpdate (
  ProcessVariable *pv,
  void *userarg )
{

aniSymbolClass *anso = (aniSymbolClass *) userarg;

  if ( anso->activeMode ) {

    anso->curYV = pv->get_double();

    anso->needPosRefresh = 1;
    anso->actWin->appCtx->proc->lock();
    anso->actWin->addDefExeNode( anso->aglPtr );
    anso->actWin->appCtx->proc->unlock();

  }

}

static void asymbol_angleUpdate (
  ProcessVariable *pv,
  void *userarg )
{

aniSymbolClass *anso = (aniSymbolClass *) userarg;

  if ( anso->activeMode ) {

    anso->curAngleV = pv->get_double();

    anso->needPosRefresh = 1;
    anso->actWin->appCtx->proc->lock();
    anso->actWin->addDefExeNode( anso->aglPtr );
    anso->actWin->appCtx->proc->unlock();

  }

}

static void asymbolSetItem (
  Widget w,
  XtPointer client,
  XtPointer call )
{

efSetItemCallbackDscPtr dsc = (efSetItemCallbackDscPtr) client;
entryFormClass *ef = (entryFormClass *) dsc->ef;
aniSymbolClass *anso = (aniSymbolClass *) dsc->obj;
int i;

  for ( i=0; i<anso->numStates; i++ ) {
    anso->elsvMin->setValue( anso->eBuf->bufStateMinValue[ef->index] );
    anso->elsvMax->setValue( anso->eBuf->bufStateMaxValue[ef->index] );
  }

}

static void ansc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

aniSymbolClass *anso = (aniSymbolClass *) client;
int stat, resizeStat, i, saveW, saveH, saveX, saveY;

  anso->actWin->setChanged();

  anso->eraseSelectBoxCorners();
  anso->erase();

// =============================================

// New requirement for edit undo
  anso->confirmEdit();

// =============================================

  strncpy( anso->id, anso->bufId, 31 );

  anso->x = anso->eBuf->bufX;
  anso->sboxX = anso->eBuf->bufX;

  anso->y = anso->eBuf->bufY;
  anso->sboxY = anso->eBuf->bufY;

  anso->numPvs = 0;
  for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
    anso->shiftCount[i] = anso->eBuf->bufShiftCount[i];
    strncpy( anso->cXorMask[i], anso->eBuf->bufXorMask[i], 9 );
    strncpy( anso->cAndMask[i], anso->eBuf->bufAndMask[i], 9 );
    anso->controlPvExpStr[i].setRaw( anso->eBuf->bufControlPvName[i] );
    if ( !blank( anso->eBuf->bufControlPvName[i] ) )
      (anso->numPvs)++;
    else
      break; /* pv entries on form must be contiguous */
  }

  anso->colorPvExpStr.setRaw( anso->eBuf->bufColorPvName );

  anso->xPvExpStr.setRaw( anso->eBuf->bufXPvName );

  anso->yPvExpStr.setRaw( anso->eBuf->bufYPvName );

  anso->anglePvExpStr.setRaw( anso->eBuf->bufAnglePvName );

  strncpy( anso->symbolFileName, anso->eBuf->bufSymbolFileName, 127 );

  anso->numStates = anso->ef.numItems;

  anso->useOriginalSize = anso->eBuf->bufUseOriginalSize;

  anso->useOriginalColors = anso->eBuf->bufUseOriginalColors;

  anso->fgColor = anso->eBuf->bufFgColor;
  anso->bgColor = anso->eBuf->bufBgColor;

  anso->binaryTruthTable = anso->eBuf->bufBinaryTruthTable;

  anso->orientation = anso->eBuf->bufOrientation;

  for ( i=0; i<anso->numStates; i++ ) {
    anso->stateMinValue[i] = anso->eBuf->bufStateMinValue[i];
    anso->stateMaxValue[i] = anso->eBuf->bufStateMaxValue[i];
  }

  if ( anso->useOriginalSize ) {
    stat = anso->readSymbolFile();
  }
  else {
    if ( ( anso->prevOr == OR_CW ) || ( anso->prevOr == OR_CCW ) ) {
      saveW = anso->h;
      saveH = anso->w;
    }
    else {
      saveW = anso->w;
      saveH = anso->h;
    }
    stat = anso->readSymbolFile();
    if ( ( saveW != anso->w ) || ( saveH != anso->h ) ) {
      resizeStat = anso->checkResizeSelectBoxAbs( -1, -1, saveW, saveH );
      if ( resizeStat & 1 ) {
        anso->resizeSelectBoxAbs( -1, -1, saveW, saveH );
        anso->resizeAbs( -1, -1, saveW, saveH );
      }
      else {
        anso->actWin->appCtx->postMessage(
         aniSymbolClass_str7 );
      }
    }
  }

  anso->prevOr = anso->orientation;

  if ( !( stat & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, aniSymbolClass_str8, anso->actWin->fileName,
     anso->symbolFileName );
    anso->actWin->appCtx->postMessage( msg );
  }
  else {

    saveX = anso->x;
    saveY = anso->y;

    switch ( anso->orientation ) {

    case OR_CW:
      anso->rotateInternal( anso->getXMid(), anso->getYMid(), '+' );
      anso->moveAbs( saveX, saveY );
      anso->resizeSelectBoxAbsFromUndo( anso->getX0(), anso->getY0(),
       anso->getW(), anso->getH() );
      break;

    case OR_CCW:
      anso->rotateInternal( anso->getXMid(), anso->getYMid(), '-' );
      anso->moveAbs( saveX, saveY );
      anso->resizeSelectBoxAbsFromUndo( anso->getX0(), anso->getY0(),
       anso->getW(), anso->getH() );
      break;

    case OR_V:
      anso->flipInternal( anso->getXMid(), anso->getYMid(), 'V' );
      anso->moveAbs( saveX, saveY );
      anso->resizeSelectBoxAbsFromUndo( anso->getX0(), anso->getY0(),
       anso->getW(), anso->getH() );
      break;

    case OR_H:
      anso->flipInternal( anso->getXMid(), anso->getYMid(), 'H' );
      anso->moveAbs( saveX, saveY );
      anso->resizeSelectBoxAbsFromUndo( anso->getX0(), anso->getY0(),
       anso->getW(), anso->getH() );
      break;

    }

  }

}

static void ansc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

aniSymbolClass *anso = (aniSymbolClass *) client;

  ansc_edit_update ( w, client, call );
  anso->refresh( anso );

}

static void ansc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

aniSymbolClass *anso = (aniSymbolClass *) client;

  ansc_edit_update ( w, client, call );
  anso->ef.popdown();
  anso->operationComplete();

}

static void ansc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

aniSymbolClass *anso = (aniSymbolClass *) client;

  anso->ef.popdown();
  anso->operationCancel();

}

static void ansc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

aniSymbolClass *anso = (aniSymbolClass *) client;

  anso->ef.popdown();
  anso->operationCancel();
  anso->erase();
  anso->deleteRequest = 1;
  anso->drawAll();

}

aniSymbolClass::aniSymbolClass ( void ) {

activeGraphicListPtr head;
int i;

  name = new char[strlen("aniSymbolClass")+1];
  strcpy( name, "aniSymbolClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {

    head = new activeGraphicListType;
    head->flink = head;
    head->blink = head;

    voidHead[i] = (void *) head;

  }

  activeMode = 0;
  numStates = 2;
  index = 0;
  controlV = 0.0;
  for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
    controlVals[i] = 0.0;
    xorMask[i] = 0;
    andMask[i] = 0;
    shiftCount[i] = 0;
    strcpy( cXorMask[i], "0" );
    strcpy( cAndMask[i], "0" );
  }
  iValue = 0;
  strcpy( symbolFileName, "" );

  useOriginalSize = 0;
  binaryTruthTable = 0;
  orientation = 0;
  rAxisAngle = 0;
  numPvs = 1;

  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {
    stateMinValue[i] = 0;
    stateMaxValue[i] = 0;
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

  useOriginalColors = 1;
  unconnectedTimer = 0;

  eBuf = NULL;

}

aniSymbolClass::~aniSymbolClass ( void ) {

//   fprintf( stderr, "In aniSymbolClass::~aniSymbolClass\n" );

activeGraphicListPtr head;
activeGraphicListPtr cur, next;
btnActionListPtr curBtnAction, nextBtnAction;
int i;

  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {

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

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

// copy constructor
aniSymbolClass::aniSymbolClass
 ( const aniSymbolClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
activeGraphicListPtr head, cur, curSource, sourceHead;
int i;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("aniSymbolClass")+1];
  strcpy( name, "aniSymbolClass" );

  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {

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
  controlV = 0.0;
  for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
    controlVals[i] = 0.0;
    controlPvExpStr[i].setRaw( source->controlPvExpStr[i].rawString );
    strncpy( cXorMask[i], source->cXorMask[i], 9 );
    strncpy( cAndMask[i], source->cAndMask[i], 9 );
    shiftCount[i] = source->shiftCount[i];
  }
  iValue = 0;

  strncpy( symbolFileName, source->symbolFileName, 127 );

  numStates = source->numStates;
  for ( i=0; i<numStates; i++ ) {
    stateMinValue[i] = source->stateMinValue[i];
    stateMaxValue[i] = source->stateMaxValue[i];
  }

  useOriginalSize = source->useOriginalSize;

  binaryTruthTable = source->binaryTruthTable;

  orientation = source->orientation;

  numPvs = source->numPvs;

  useOriginalColors = source->useOriginalColors;
  fgCb = source->fgCb;
  bgCb = source->bgCb;
  fgColor = source->fgColor;
  bgColor = source->bgColor;
  colorPvExpStr.setRaw( source->colorPvExpStr.rawString );
  xPvExpStr.setRaw( source->xPvExpStr.rawString );
  yPvExpStr.setRaw( source->yPvExpStr.rawString );
  anglePvExpStr.setRaw( source->anglePvExpStr.rawString );

  unconnectedTimer = 0;

  eBuf = NULL;

  doAccSubs( symbolFileName, 127 );
  doAccSubs( colorPvExpStr );
  doAccSubs( xPvExpStr );
  doAccSubs( yPvExpStr );
  doAccSubs( anglePvExpStr );
  for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) {
    doAccSubs( controlPvExpStr[i] );
  }

}

int aniSymbolClass::createInteractive (
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

int aniSymbolClass::genericEdit ( void )
{

int i;
char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "aniSymbolClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, aniSymbolClass_str9, 31 );

  Strncat( title, aniSymbolClass_str10, 31 );

  strncpy( bufId, id, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;

  strncpy( eBuf->bufSymbolFileName, symbolFileName, 127 );

  if ( colorPvExpStr.getRaw() )
    strncpy( eBuf->bufColorPvName, colorPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufColorPvName, "" );

  if ( xPvExpStr.getRaw() )
    strncpy( eBuf->bufXPvName, xPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufXPvName, "" );

  if ( yPvExpStr.getRaw() )
    strncpy( eBuf->bufYPvName, yPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufYPvName, "" );

  if ( anglePvExpStr.getRaw() )
    strncpy( eBuf->bufAnglePvName, anglePvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufAnglePvName, "" );

  for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
    if ( controlPvExpStr[i].getRaw() )
      strncpy( eBuf->bufControlPvName[i], controlPvExpStr[i].getRaw(),
       PV_Factory::MAX_PV_NAME );
    else
      strcpy( eBuf->bufControlPvName[i], "" );
    strncpy( eBuf->bufXorMask[i], cXorMask[i], 9 );
    strncpy( eBuf->bufAndMask[i], cAndMask[i], 9 );
    eBuf->bufShiftCount[i] = shiftCount[i];
  }

  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {
    eBuf->bufStateMinValue[i] = stateMinValue[i];
    eBuf->bufStateMaxValue[i] = stateMaxValue[i];
  }

  eBuf->bufUseOriginalSize = useOriginalSize;

  eBuf->bufBinaryTruthTable = binaryTruthTable;

  prevOr = eBuf->bufOrientation = orientation;

  eBuf->bufUseOriginalColors = useOriginalColors;

  eBuf->bufFgColor = fgColor;
  eBuf->bufBgColor = bgColor;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, ASYMBOL_K_NUM_STATES, numStates,
   asymbolSetItem, (void *) this, NULL, NULL, NULL );

  ef.addTextField( aniSymbolClass_str12, 32, &eBuf->bufX );
  ef.addTextField( aniSymbolClass_str13, 32, &eBuf->bufY );
  ef.addTextField( aniSymbolClass_str42, 32, eBuf->bufXPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( aniSymbolClass_str43, 32, eBuf->bufYPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( aniSymbolClass_str41, 32, eBuf->bufAnglePvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( aniSymbolClass_str14, 32, eBuf->bufSymbolFileName, 127 );
  ef.addTextField( aniSymbolClass_str29, 32, eBuf->bufColorPvName,
   PV_Factory::MAX_PV_NAME );

  for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
    if ( i == 0 ) {
      ef.addTextField( aniSymbolClass_str17, 32, eBuf->bufControlPvName[i],
       PV_Factory::MAX_PV_NAME );
      cntlPvEntry[i] = ef.getCurItem();
    }
    else {
      ef.addTextField( " ", 32, eBuf->bufControlPvName[i],
       PV_Factory::MAX_PV_NAME );
      cntlPvEntry[i] = ef.getCurItem();
    }
    ef.beginSubForm();
    ef.addTextField( aniSymbolClass_str38, 4, eBuf->bufAndMask[i], 4 );
    andMaskEntry[i] = ef.getCurItem();
    cntlPvEntry[i]->addDependency( andMaskEntry[i] );
    ef.addLabel( aniSymbolClass_str39 );
    ef.addTextField( "", 4, eBuf->bufXorMask[i], 4 );
    xorMaskEntry[i] = ef.getCurItem();
    cntlPvEntry[i]->addDependency( xorMaskEntry[i] );
    ef.addLabel( aniSymbolClass_str40 );
    ef.addTextField( "", 3, &eBuf->bufShiftCount[i] );
    shiftCountEntry[i] = ef.getCurItem();
    cntlPvEntry[i]->addDependency( shiftCountEntry[i] );
    cntlPvEntry[i]->addDependencyCallbacks();
    ef.endSubForm();
  }

  ef.addToggle( aniSymbolClass_str16, &eBuf->bufBinaryTruthTable );

  ef.addOption( aniSymbolClass_str33, aniSymbolClass_str34,
   &eBuf->bufOrientation );

  ef.addToggle( aniSymbolClass_str15, &eBuf->bufUseOriginalSize );

  ef.addToggle( aniSymbolClass_str30, &eBuf->bufUseOriginalColors );
  presColorEntry = ef.getCurItem();
  ef.addColorButton(aniSymbolClass_str31, actWin->ci, &fgCb, &eBuf->bufFgColor );
  fgColorEntry = ef.getCurItem();
  presColorEntry->addInvDependency( fgColorEntry );
  ef.addColorButton(aniSymbolClass_str32, actWin->ci, &bgCb, &eBuf->bufBgColor );
  bgColorEntry = ef.getCurItem();
  presColorEntry->addInvDependency( bgColorEntry );
  presColorEntry->addDependencyCallbacks();

  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {
    minPtr[i] = &eBuf->bufStateMinValue[i];
    maxPtr[i] = &eBuf->bufStateMaxValue[i];
  }

  ef.addTextFieldArray( aniSymbolClass_str18, 32, eBuf->bufStateMinValue, &elsvMin );
  ef.addTextFieldArray( aniSymbolClass_str19, 32, eBuf->bufStateMaxValue, &elsvMax );

  return 1;

}

int aniSymbolClass::edit ( void )
{

  this->genericEdit();
  ef.finished( ansc_edit_ok, ansc_edit_apply, ansc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int aniSymbolClass::editCreate ( void )
{

  this->genericEdit();
  ef.finished( ansc_edit_ok, ansc_edit_apply, ansc_edit_cancel_delete,
   this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int aniSymbolClass::readSymbolFile ( void )
{

int l, more, maxW, maxH, gX, gY, gW, gH, dX, dY, saveLine;
int winMajor, winMinor, winRelease;
char itemName[127+1], *tk;
activeGraphicListPtr head, cur, next;
int i;
FILE *f;
char name[127+1];
expStringClass expStr;

int major, minor, release, stat, retStat = 1;
char *gotOne, tagName[255+1], val[4095+1];
int isCompound;
tagClass tag;

int zero = 0;
char *emptyStr = "";

// these are discarded if read
expStringClass visPvExpStr;
char minVisString[39+1];
char maxVisString[39+1];
int visInverted;

  tagClass::pushLevel();
  tagClass::setFileName( symbolFileName );

  saveLine = tag.line();

  // delete current info ( if any )

  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {

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

  if ( strcmp( symbolFileName, "" ) == 0 ) return 0;

  actWin->substituteSpecial( 127, symbolFileName, name );

  expStr.setRaw( name );
  expStr.expand1st( actWin->numMacros, actWin->macros, actWin->expansions );

  f = actWin->openAnySymFile( expStr.getExpanded(), "r" );
  if ( !f ) {
    // numStates = 0;
    return 0;
  }

  actWin->discardWinLoadData( f, &winMajor, &winMinor, &winRelease );

  if ( winMajor < 4 ) {

    // for forward compatibility
    stat = actWin->readUntilEndOfData( f, winMajor, winMinor, winRelease );
    if ( !( stat & 1 ) ) {
      fileClose( f );
      actWin->setLine( saveLine );
      tagClass::popLevel();
      return stat;
    }

    index = 0;
    maxW = 0;
    maxH = 0;

    for ( i=0; i<numStates; i++ ) {

      head = (activeGraphicListPtr) voidHead[i];

      gotOne = fgets( itemName, 127, f ); // discard "activeGroupClass"
      if ( !gotOne ) {
        if ( i == 0 ) {
          // numStates = 0;
          fileClose( f );
          actWin->setLine( saveLine );
          tagClass::popLevel();
          return 0;
        }
        numStates = i+1;
        fileClose( f );
        actWin->setLine( saveLine );
        tagClass::popLevel();
        return 1;
      }

      tk = strtok( itemName, " \t\n" );
      if ( strcmp( tk, "activeGroupClass" ) != 0 ) {
        // numStates = 0;
        fileClose( f );
        actWin->setLine( saveLine );
        tagClass::popLevel();
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
          // numStates = 0;
          fileClose( f );
          actWin->setLine( saveLine );
          tagClass::popLevel();
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
            fileClose( f );
            fprintf( stderr, aniSymbolClass_str20 );
            // numStates = 0;
            actWin->setLine( saveLine );
            tagClass::popLevel();
            return 0;
          }

          cur->node = actWin->obj.createNew( itemName );

          if ( cur->node ) {

            cur->node->old_createFromFile( f, itemName, actWin );

            // for forward compatibility
            stat = actWin->readUntilEndOfData( f, winMajor, winMinor,
             winRelease );
            if ( !( stat & 1 ) ) {
              fileClose( f );
              actWin->setLine( saveLine );
              tagClass::popLevel();
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
            fileClose( f );
            fprintf( stderr, aniSymbolClass_str21 );
            // numStates = 0;
            actWin->setLine( saveLine );
            tagClass::popLevel();
            return 0;
          }

        }

      } while ( more );

      // for forward compatibility
      stat = actWin->readUntilEndOfData( f, winMajor, winMinor, winRelease );
      if ( !( stat & 1 ) ) {
        fileClose( f );
        actWin->setLine( saveLine );
        tagClass::popLevel();
        return stat;
      }

    }

  }
  else {

    index = 0;
    maxW = 0;
    maxH = 0;

    for ( i=0; i<numStates; i++ ) {

      head = (activeGraphicListPtr) voidHead[i];

      tag.init();
      tag.loadR( "object", 127, itemName );

      gotOne = tag.getName( tagName, 255, f );

      if ( gotOne ) {

        //fprintf( stderr, "name = [%s]\n", tagName );

        if ( strcmp( tagName, "object" ) == 0 ) {

          tag.getValue( val, 4095, f, &isCompound );
          tag.decode( tagName, val, isCompound );

          if ( strcmp( itemName, "activeGroupClass" ) != 0 ) {
            //numStates = 0;
            fileClose( f );
            tag.setLine( saveLine );
            tagClass::popLevel();
            return 0;
            break;
          }

        }
        else {
          //numStates = 0;
          fileClose( f );
          tag.setLine( saveLine );
          tagClass::popLevel();
          return 0;
          break;
        }

      }
      else {

        if ( i == 0 ) {
          //numStates = 0;
          fileClose( f );
          tag.setLine( saveLine );
          tagClass::popLevel();
          return 0;
        }

        numStates = i+1;

        break;

      }

      // read in group properties
      tag.init();
      tag.loadR( "beginObjectProperties" );
      tag.loadR( unknownTags );
      tag.loadR( "major", &major );
      tag.loadR( "minor", &minor );
      tag.loadR( "release", &release );
      tag.loadR( "x", &gX );
      tag.loadR( "y", &gY );
      tag.loadR( "w", &gW );
      tag.loadR( "h", &gH );
      tag.loadR( "beginGroup" );

      stat = tag.readTags( f, "beginGroup" );

      if ( !( stat & 1 ) ) {
        retStat = stat;
        actWin->appCtx->postMessage( tag.errMsg() );
      }

      if ( major > AGC_MAJOR_VERSION ) {
        postIncompatable();
        return 0;
      }

      if ( major < 4 ) {
        postIncompatable();
        return 0;
      }

      if ( gW > maxW ) maxW = gW;
      if ( gH > maxH ) maxH = gH;

      dX = x - gX;
      dY = y - gY;

      tag.init();
      tag.loadR( "object", 63, itemName );
      tag.loadR( "endGroup" );
      do {

        // read and create sub-objects until "endGroup" is found

        gotOne = tag.getName( tagName, 255, f );
        if ( !gotOne ) {
          //numStates = 0;
          fileClose( f );
          tag.setLine( saveLine );
          tagClass::popLevel();
          return 0;
        }

        if ( gotOne ) {

          //fprintf( stderr, "name = [%s]\n", tagName );

          if ( strcmp( tagName, "object" ) == 0 ) {

            tag.getValue( val, 4095, f, &isCompound );
            tag.decode( tagName, val, isCompound );

            // =======================================================
            // Create object

            //fprintf( stderr, "objName = [%s]\n", itemName );

            more = 1;

            cur = new activeGraphicListType;
            if ( !cur ) {
              fileClose( f );
              fprintf( stderr, "Insufficient virtual memory - abort\n" );
              //numStates = 0;
              fileClose( f );
              tag.setLine( saveLine );
	      tagClass::popLevel();
              return 0;
            }

            cur->node = actWin->obj.createNew( itemName );

            if ( cur->node ) {

              cur->node->createFromFile( f, itemName, actWin );

              // adjust origin
              cur->node->move( dX, dY );

              cur->blink = head->blink;
              head->blink->flink = cur;
              head->blink = cur;
              cur->flink = head;

            }
            else {

              fileClose( f );
              fprintf( stderr, "Insufficient virtual memory - abort\n" );
              //numStates = 0;
              tag.setLine( saveLine );
	      tagClass::popLevel();
              return 0;

            }

          }
          else if ( strcmp( tagName, "endGroup" ) == 0 ) {

            more = 0;

          }
          else {
            //numStates = 0;
            fileClose( f );
            tag.setLine( saveLine );
	    tagClass::popLevel();
            return 0;
          }

        }

      } while ( more );

      // read group "endObjectProperties"
      tag.init();
      tag.loadR( "visPv", &visPvExpStr, emptyStr );
      tag.loadR( "visInvert", &visInverted, &zero );
      tag.loadR( "visMin", 39, minVisString, emptyStr );
      tag.loadR( "visMax", 39, maxVisString, emptyStr );
      tag.loadR( "endObjectProperties" );

      stat = tag.readTags( f, "endObjectProperties" );

      if ( !( stat & 1 ) ) {
        retStat = stat;
        actWin->appCtx->postMessage( tag.errMsg() );
      }

    }

  }

  fileClose( f );

  w = maxW;
  sboxW = w;
  h = maxH;
  sboxH = h;

  tag.setLine( saveLine );
  tagClass::popLevel();

  return retStat;

}

#if 0
int aniSymbolClass::old_readSymbolFile ( void )
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

  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {

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

  if ( strcmp( symbolFileName, "" ) == 0 ) return 0;

  actWin->substituteSpecial( 127, symbolFileName, name );

  expStr.setRaw( name );
  expStr.expand1st( actWin->numMacros, actWin->macros, actWin->expansions );

  f = actWin->openAnySymFile( expStr.getExpanded(), "r" );
  if ( !f ) {
    // numStates = 0;
    return 0;
  }

  actWin->discardWinLoadData( f, &winMajor, &winMinor, &winRelease );

  // for forward compatibility
  stat = actWin->readUntilEndOfData( f, winMajor, winMinor, winRelease );
  if ( !( stat & 1 ) ) {
    fileClose( f );
    actWin->setLine( saveLine );
    return stat;
  }

  index = 0;
  maxW = 0;
  maxH = 0;

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    gotOne = fgets( itemName, 127, f ); // discard "activeGroupClass"
    if ( !gotOne ) {
      if ( i == 0 ) {
        // numStates = 0;
        fileClose( f );
        actWin->setLine( saveLine );
        return 0;
      }
      numStates = i+1;
      fileClose( f );
      actWin->setLine( saveLine );
      return 1;
    }

    tk = strtok( itemName, " \t\n" );
    if ( strcmp( tk, "activeGroupClass" ) != 0 ) {
      // numStates = 0;
      fileClose( f );
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
        // numStates = 0;
        fileClose( f );
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
          fileClose( f );
          fprintf( stderr, aniSymbolClass_str20 );
          // numStates = 0;
          actWin->setLine( saveLine );
          return 0;
        }

        cur->node = actWin->obj.createNew( itemName );

        if ( cur->node ) {

          cur->node->old_createFromFile( f, itemName, actWin );

          // for forward compatibility
          stat = actWin->readUntilEndOfData( f, winMajor, winMinor,
           winRelease );
          if ( !( stat & 1 ) ) {
            fileClose( f );
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
          fileClose( f );
          fprintf( stderr, aniSymbolClass_str21 );
          // numStates = 0;
          actWin->setLine( saveLine );
          return 0;
        }

      }

    } while ( more );

    // for forward compatibility
    stat = actWin->readUntilEndOfData( f, winMajor, winMinor, winRelease );
    if ( !( stat & 1 ) ) {
      fileClose( f );
      actWin->setLine( saveLine );
      return stat;
    }

  }

  fileClose( f );

  w = maxW;
  sboxW = w;
  h = maxH;
  sboxH = h;

  actWin->setLine( saveLine );

  return 1;

}
#endif

int aniSymbolClass::save (
 FILE *f )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
unsigned int uzero = 0;
double dzero = 0;
char *emptyStr = "";

int orienOriginal = OR_ORIG;
static char *orienEnumStr[5] = {
  "original",
  "rotateCW",
  "rotateCCW",
  "FlipV",
  "FlipH"
};

static int orienEnum[5] = {
  OR_ORIG,
  OR_CW,
  OR_CCW,
  OR_V,
  OR_H
};

int i, saveX, saveY, origX, origY, origW, origH;

  major = ANSC_MAJOR_VERSION;
  minor = ANSC_MINOR_VERSION;
  release = ANSC_RELEASE;

  saveX = x;
  saveY = y;

  switch ( orientation ) {

  case OR_CW:
    rotateInternal( getXMid(), getYMid(), '-' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  case OR_CCW:
    rotateInternal( getXMid(), getYMid(), '+' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  }

  origX = x;
  origY = y;
  origW = w;
  origH = h;

  switch ( orientation ) {

  case OR_CW:
    rotateInternal( getXMid(), getYMid(), '+' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  case OR_CCW:
    rotateInternal( getXMid(), getYMid(), '-' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  }

  origX += ( saveX - x );
  origY += ( saveY - y );

  moveAbs( saveX, saveY );

  for ( i=0; i<numPvs; i++ ) {
    andMask[i] = strtol( cAndMask[i], NULL, 16 );
    xorMask[i] = strtol( cXorMask[i], NULL, 16 );
  }

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &origX );
  tag.loadW( "y", &origY );
  tag.loadW( "w", &origW );
  tag.loadW( "h", &origH );
  tag.loadW( "file", symbolFileName, emptyStr );
  tag.loadBoolW( "truthTable", &binaryTruthTable, &zero );
  tag.loadW( "numStates", &numStates );
  tag.loadW( "minValues", stateMinValue, numStates, &dzero );
  tag.loadW( "maxValues", stateMaxValue, numStates, &dzero );
  tag.loadW( "controlPvs", controlPvExpStr, numPvs, emptyStr );
  tag.loadW( "numPvs", &numPvs );
  tag.loadHexW( "andMask", andMask, numPvs, &uzero );
  tag.loadHexW( "xorMask", xorMask, numPvs, &uzero );
  tag.loadW( "shiftCount", shiftCount, numPvs, &zero );
  tag.loadBoolW( "useOriginalSize", &useOriginalSize, &zero );
  tag.loadW( "orientation", 5, orienEnumStr, orienEnum,
   &orientation, &orienOriginal );
  tag.loadW( "colorPv", &colorPvExpStr, emptyStr );
  tag.loadBoolW( "useOriginalColors", &useOriginalColors, &zero );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadW( "xPv", &xPvExpStr, emptyStr );
  tag.loadW( "yPv", &yPvExpStr, emptyStr );
  tag.loadW( "axisAnglePv", &anglePvExpStr, emptyStr );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int aniSymbolClass::old_save (
 FILE *f )
{

int i, saveX, saveY, origX, origY, origW, origH;

  saveX = x;
  saveY = y;

  switch ( orientation ) {

  case OR_CW:
    rotateInternal( getXMid(), getYMid(), '-' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  case OR_CCW:
    rotateInternal( getXMid(), getYMid(), '+' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  }

  origX = x;
  origY = y;
  origW = w;
  origH = h;

  switch ( orientation ) {

  case OR_CW:
    rotateInternal( getXMid(), getYMid(), '+' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  case OR_CCW:
    rotateInternal( getXMid(), getYMid(), '-' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  }

  origX += ( saveX - x );
  origY += ( saveY - y );

  moveAbs( saveX, saveY );

  fprintf( f, "%-d %-d %-d\n", ANSC_MAJOR_VERSION, ANSC_MINOR_VERSION,
   ANSC_RELEASE );

  fprintf( f, "%-d\n", origX );
  fprintf( f, "%-d\n", origY );
  fprintf( f, "%-d\n", origW );
  fprintf( f, "%-d\n", origH );

  writeStringToFile( f, symbolFileName );

  fprintf( f, "%-d\n", binaryTruthTable );
  fprintf( f, "%-d\n", numPvs );

  for ( i=0; i<numPvs; i++ ) {

    if ( controlPvExpStr[i].getRaw() )
      writeStringToFile( f, controlPvExpStr[i].getRaw() );
    else
      writeStringToFile( f, "" );

  }

  fprintf( f, "%d\n", numStates );

  for ( i=0; i<numStates; i++ ) {
    fprintf( f, "%-g\n", stateMinValue[i] );
    fprintf( f, "%-g\n", stateMaxValue[i] );
  }

  fprintf( f, "%-d\n", useOriginalSize );

  // version 1.3.0
  writeStringToFile( f, this->id );

  // version 1.4.0
  fprintf( f, "%-d\n", orientation );

  // version 1.5.0
  if ( colorPvExpStr.getRaw() )
    writeStringToFile( f, colorPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1.6.0
  fprintf( f, "%-d\n", useOriginalColors );
  fprintf( f, "%-d\n", fgColor );
  fprintf( f, "%-d\n", bgColor );

  // version 1.7.0
  for ( i=0; i<numPvs; i++ ) {
    writeStringToFile( f, cAndMask[i] );
    writeStringToFile( f, cXorMask[i] );
    fprintf( f, "%-d\n", shiftCount[i] );
  }

  return 1;

}

int aniSymbolClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
unsigned int uzero = 0;
double dzero = 0;
char *emptyStr = "";

int orienOriginal = OR_ORIG;
static char *orienEnumStr[5] = {
  "original",
  "rotateCW",
  "rotateCCW",
  "FlipV",
  "FlipH"
};

static int orienEnum[5] = {
  OR_ORIG,
  OR_CW,
  OR_CCW,
  OR_V,
  OR_H
};

int resizeStat, readSymfileStat, i, n1, n2, saveW, saveH;

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
  tag.loadR( "file", 127, symbolFileName, emptyStr );
  tag.loadR( "truthTable", &binaryTruthTable, &zero );
  tag.loadR( "numStates", &numStates, &zero );
  tag.loadR( "minValues", ASYMBOL_K_NUM_STATES, stateMinValue, &n1,
   &dzero );
  tag.loadR( "maxValues", ASYMBOL_K_NUM_STATES, stateMaxValue, &n1,
   &dzero );
  tag.loadR( "numPvs", &numPvs, &zero );
  tag.loadR( "controlPvs", ASYMBOL_K_MAX_PVS, controlPvExpStr, &n2,
   emptyStr );
  tag.loadR( "andMask", ASYMBOL_K_MAX_PVS, andMask, &n2, &uzero );
  tag.loadR( "xorMask", ASYMBOL_K_MAX_PVS, xorMask, &n2, &uzero );
  tag.loadR( "shiftCount", ASYMBOL_K_MAX_PVS, shiftCount, &n2, &zero );
  tag.loadR( "useOriginalSize", &useOriginalSize, &zero );
  tag.loadR( "orientation", 5, orienEnumStr, orienEnum,
   &orientation, &orienOriginal );
  tag.loadR( "colorPv", &colorPvExpStr, emptyStr );
  tag.loadR( "useOriginalColors", &useOriginalColors, &zero );
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "xPv", &xPvExpStr, emptyStr );
  tag.loadR( "yPv", &yPvExpStr, emptyStr );
  tag.loadR( "axisAnglePv", &anglePvExpStr, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > ANSC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox();

  if ( numStates < 1 ) numStates = 1;
  if ( numStates > ASYMBOL_K_NUM_STATES ) numStates = ASYMBOL_K_NUM_STATES;

  for ( i=0; i<numPvs; i++ ) {
    snprintf( cAndMask[i], 9, "%-x", andMask[i] );
    snprintf( cXorMask[i], 9, "%-x", xorMask[i] );
  }

  saveW = w;
  saveH = h;

  readSymfileStat = readSymbolFile();
  if ( !( readSymfileStat & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, aniSymbolClass_str22, actWin->fileName,
     symbolFileName );
    actWin->appCtx->postMessage( msg );
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
           aniSymbolClass_str23 );
        }
      }
    }

    switch ( orientation ) {

    case OR_CW:
      rotateInternal( getXMid(), getYMid(), '+' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    case OR_CCW:
      rotateInternal( getXMid(), getYMid(), '-' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    case OR_V:
      flipInternal( getXMid(), getYMid(), 'V' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    case OR_H:
      flipInternal( getXMid(), getYMid(), 'H' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    }

  }

  return stat;

}

int aniSymbolClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int stat, resizeStat, i, saveW, saveH;
int major, minor, release;
char string[PV_Factory::MAX_PV_NAME+1];
float val;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > ANSC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  readStringFromFile( symbolFileName, 127+1, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    fscanf( f, "%d\n", &binaryTruthTable ); actWin->incLine();
    fscanf( f, "%d\n", &numPvs ); actWin->incLine();
  }
  else {
    binaryTruthTable = 0;
    numPvs = 1;
  }

  for ( i=0; i<numPvs; i++ ) {
    readStringFromFile( string, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    controlPvExpStr[i].setRaw( string );
  }

  fscanf( f, "%d\n", &numStates ); actWin->incLine();

  if ( numStates < 1 ) numStates = 1;
  if ( numStates > ASYMBOL_K_NUM_STATES ) numStates = ASYMBOL_K_NUM_STATES;

  for ( i=0; i<numStates; i++ ) {
    fscanf( f, "%g\n", &val ); actWin->incLine();
    stateMinValue[i] = val;
    fscanf( f, "%g\n", &val ); actWin->incLine();
    stateMaxValue[i] = val;
  }

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    fscanf( f, "%d\n", &useOriginalSize ); actWin->incLine();
  }
  else {
    useOriginalSize = 0;
  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {
    readStringFromFile( this->id, 31+1, f ); actWin->incLine();
  }
  else {
    strcpy( this->id, "" );
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    fscanf( f, "%d\n", &orientation );
  }
  else {
    orientation = OR_ORIG;
  }

  if ( ( major > 1 ) || ( minor > 4 ) ) {
    readStringFromFile( string, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    colorPvExpStr.setRaw( string );
  }

  if ( ( major > 1 ) || ( minor > 5 ) ) {
    fscanf( f, "%d\n", &useOriginalColors ); actWin->incLine();
    fscanf( f, "%d\n", &fgColor ); actWin->incLine();
    fscanf( f, "%d\n", &bgColor ); actWin->incLine();
  }
  else {
    useOriginalColors = 1;
    fgColor = actWin->defaultTextFgColor;
    bgColor = actWin->defaultBgColor;
  }

  if ( ( major > 1 ) || ( minor > 6 ) ) {

    for ( i=0; i<numPvs; i++ ) {
      andMask[i] = 0;
      xorMask[i] = 0;
      readStringFromFile( cAndMask[i], 9, f );
      readStringFromFile( cXorMask[i], 9, f );
      fscanf( f, "%d\n", &shiftCount[i] );
    }

  }
  else {

    for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
      andMask[i] = 0;
      xorMask[i] = 0;
      shiftCount[i] = 0;
      strcpy( cXorMask[i], "0" );
      strcpy( cAndMask[i], "0" );
    }

    if ( !binaryTruthTable ) {
      numPvs = 1;
    }

  }

  saveW = w;
  saveH = h;

  stat = readSymbolFile();
  if ( !( stat & 1 ) ) {
    char msg[255+1];
    snprintf( msg, 255, aniSymbolClass_str22, actWin->fileName,
     symbolFileName );
    actWin->appCtx->postMessage( msg );
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
           aniSymbolClass_str23 );
        }
      }
    }

    switch ( orientation ) {

    case OR_CW:
      rotateInternal( getXMid(), getYMid(), '+' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    case OR_CCW:
      rotateInternal( getXMid(), getYMid(), '-' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    case OR_V:
      flipInternal( getXMid(), getYMid(), 'V' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    case OR_H:
      flipInternal( getXMid(), getYMid(), 'H' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    }

  }

  return 1;

}

int aniSymbolClass::erase ( void ) {

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

    cur->node->erase();

    cur = cur->flink;

  }

  return 1;

}

int aniSymbolClass::eraseActive ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;

  if ( !enabled || !init || !activeMode || ( numStates < 1 ) ) return 1;

  if ( ( prevIndex >= 0 ) && ( prevIndex < numStates ) ) {

    head = (activeGraphicListPtr) voidHead[prevIndex];

    cur = head->flink;
    while ( cur != head ) {

      // cur->node->removeBlink();
      cur->node->eraseActive();

      cur = cur->flink;

    }

  }

  prevIndex = index;

  return 1;

}

int aniSymbolClass::draw ( void ) {

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

int aniSymbolClass::drawActive ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;
pvColorClass tmpColor;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      tmpColor.setColorIndex( 0, actWin->ci );
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( tmpColor.getDisconnected() );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
    }
  }
  else if ( needToEraseUnconnected ) {
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
  }

  if ( !enabled || !init || !activeMode || ( numStates < 1 ) ) return 1;

  if ( ( index < 0 ) || ( index >= numStates ) ) return 1;

  head = (activeGraphicListPtr) voidHead[index];

  cur = head->flink;
  while ( cur != head ) {

    cur->node->drawActive();

    cur = cur->flink;

  }

  return 1;

}

void aniSymbolClass::removePrevBlink ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;

  if ( !enabled || !init || !activeMode || ( numStates < 1 ) ) return;

  if ( ( prevIndex >= 0 ) && ( prevIndex < numStates ) ) {

    head = (activeGraphicListPtr) voidHead[prevIndex];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->removeBlink();

      cur = cur->flink;

    }

  }

  return;

}

int aniSymbolClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag )
{

  *up = 1;
  *down = 1;
  *drag = 0;

  return 1;

}

void aniSymbolClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

  // inc index on button down, dec index on shift button down

  if ( !enabled ) return;

}

void aniSymbolClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

  if ( !enabled ) return;

}

int aniSymbolClass::activate (
  int pass,
  void *ptr,
  int *numSubObjects ) {

int i, opStat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int num;

  *numSubObjects = 0;
  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      if ( pass == 1 ) {

        if ( !useOriginalColors ) {
          cur->node->changeDisplayParams(
           ACTGRF_TEXTFGCOLOR_MASK | ACTGRF_FG1COLOR_MASK |
           ACTGRF_BGCOLOR_MASK, "", 0, "", 0, "", 0, fgColor,
           fgColor, 0, 0, bgColor, 0, 0 );
        }

        cur->node->initEnable();

      }

      cur->node->activate( pass, (void *) cur, &num );

      (*numSubObjects) += num;
      if ( *numSubObjects >= activeWindowClass::NUM_PER_PENDIO ) {
        pend_io( 5.0 );
        pend_event( 0.01 );
        //processAllEvents( actWin->appCtx->appContext(), actWin->d );
        *numSubObjects = 0;
      }

      cur->node->removeBlink();

      cur = cur->flink;

    }

  }

  switch ( pass ) {

  case 1:

    savedX = x;
    savedY = y;

    needErase = needDraw = needRefresh = needConnectInit = needPosRefresh =
     needColorInit = needXInit = needYInit = needAngleInit =
     needColorRefresh = 0;
    needToEraseUnconnected = 0;
    needToDrawUnconnected = 0;
    unconnectedTimer = 0;
    for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) needConnect[i] = 0;
    aglPtr = ptr;
    iValue = 0; /* this gets set via OR/AND operations */
    prevIndex = -1;
    init = 0;
    controlExists = 0;
    opComplete = 0;
    active = 0;
    activeMode = 1;
    controlV = 1;
    curXV = 0;
    curYV = 0;
    curAngleV = 0;
    initialColorConnection = 1;
    initialXPvConnection = 1;
    initialYPvConnection = 1;
    initialAnglePvConnection = 1;

    for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
      curUiVal[i] = 0; /* this gets set via XOR/AND/SHIFT operations */
      andMask[i] = strtol( cAndMask[i], NULL, 16 );
      xorMask[i] = strtol( cXorMask[i], NULL, 16 );
      controlPvId[i] = NULL;
      initialCtrlConnection[i] = 1;
    }
    colorPvId = NULL;
    xPvId = NULL;
    yPvId = NULL;
    anglePvId = NULL;

    //notControlPvConnected = (int) pow(2,numPvs) - 1; <-- solaris compile prob
    notControlPvConnected = (1 << numPvs) - 1;

    if ( numPvs ) {

      controlExists = 1;

      for ( i=0; i<numPvs; i++ ) {

        if ( !controlPvExpStr[i].getExpanded() ||
             blankOrComment( controlPvExpStr[i].getExpanded() ) ) {
               controlExists = 0;
	}

      }

    }
    else {

      controlExists = 0;

    }

    if ( blankOrComment( colorPvExpStr.getExpanded() ) ) {
      colorExists = 0;
    }
    else {
      colorExists = 1;
    }

    if ( blankOrComment( xPvExpStr.getExpanded() ) ) {
      xPvExists = 0;
    }
    else {
      xPvExists = 1;
    }

    if ( blankOrComment( yPvExpStr.getExpanded() ) ) {
      yPvExists = 0;
    }
    else {
      yPvExists = 1;
    }

    if ( blankOrComment( anglePvExpStr.getExpanded() ) ) {
      anglePvExists = 0;
    }
    else {
      anglePvExists = 1;
    }

    break;

  case 2:

    if ( !opComplete ) {

      opStat = 1;

      initEnable();

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, asymUnconnectedTimeout, this );
      }

      if ( controlExists ) {

        for ( i=0; i<numPvs; i++ ) {

          argRec[i].objPtr = (void *) this;
          argRec[i].index = i;
          argRec[i].setMask = (unsigned int) 1 << i;
          argRec[i].clrMask = ~(argRec[i].setMask);

	  controlPvId[i] = the_PV_Factory->create(
           controlPvExpStr[i].getExpanded() );
	  if ( controlPvId[i] ) {
	    controlPvId[i]->add_conn_state_callback(
             asymbol_monitor_control_connect_state, &argRec[i] );
	  }
	  else {
            fprintf( stderr, aniSymbolClass_str24 );
            opStat = 0;
          }

        }

      }
      else {

        init = 1;
        active = 1;
        index = 1;

      }

      if ( colorExists ) {

	colorPvId = the_PV_Factory->create( colorPvExpStr.getExpanded() );
	if ( colorPvId ) {
          colorPvId->add_conn_state_callback(
            asymbol_monitor_color_connect_state, this );
	}
	else {
          fprintf( stderr, aniSymbolClass_str24 );
          opStat = 0;
        }

      }

      if ( xPvExists ) {

	xPvId = the_PV_Factory->create( xPvExpStr.getExpanded() );
	if ( xPvId ) {
          xPvId->add_conn_state_callback(
            asymbol_monitor_x_connect_state, this );
	}
	else {
          fprintf( stderr, aniSymbolClass_str24 );
          opStat = 0;
        }

      }

      if ( yPvExists ) {

	yPvId = the_PV_Factory->create( yPvExpStr.getExpanded() );
	if ( yPvId ) {
          yPvId->add_conn_state_callback(
            asymbol_monitor_y_connect_state, this );
	}
	else {
          fprintf( stderr, aniSymbolClass_str24 );
          opStat = 0;
        }

      }

      if ( anglePvExists ) {

	anglePvId = the_PV_Factory->create( anglePvExpStr.getExpanded() );
	if ( anglePvId ) {
          anglePvId->add_conn_state_callback(
            asymbol_monitor_angle_connect_state, this );
	}
	else {
          fprintf( stderr, aniSymbolClass_str24 );
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

int aniSymbolClass::deactivate (
  int pass,
  int *numSubObjects ) {

int i;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int num;
int once = 1;

  if ( once ) {
    once = 0;
    moveAbs( savedX, savedY );
  }

  *numSubObjects = 0;
  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->deactivate( pass, &num );

      (*numSubObjects) += num;
      if ( *numSubObjects >= activeWindowClass::NUM_PER_PENDIO ) {
        pend_io( 5.0 );
        pend_event( 0.01 );
        //processAllEvents( actWin->appCtx->appContext(), actWin->d );
        *numSubObjects = 0;
      }

      cur->node->removeBlink();

      cur = cur->flink;

    }

  }

  if ( pass == 1 ) {

  active = 0;
  activeMode = 0;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  for ( i=0; i<numPvs; i++ ) {

    if ( controlExists ) {
      if ( controlPvId[i] ) {
        controlPvId[i]->remove_conn_state_callback(
         asymbol_monitor_control_connect_state, &argRec[i] );
        controlPvId[i]->remove_value_callback(
         asymbol_controlUpdate, &argRec[i] );
        controlPvId[i]->release();
        controlPvId[i] = NULL;
      }
    }

  }

  if ( colorExists ) {
    if ( colorPvId ) {
      colorPvId->remove_conn_state_callback(
       asymbol_monitor_color_connect_state, this );
      colorPvId->remove_value_callback(
       asymbol_colorUpdate, this );
      colorPvId->release();
      colorPvId = NULL;
    }
  }

  if ( xPvExists ) {
    if ( xPvId ) {
      xPvId->remove_conn_state_callback(
       asymbol_monitor_x_connect_state, this );
      xPvId->remove_value_callback(
       asymbol_xUpdate, this );
      xPvId->release();
      xPvId = NULL;
    }
  }

  if ( yPvExists ) {
    if ( yPvId ) {
      yPvId->remove_conn_state_callback(
       asymbol_monitor_y_connect_state, this );
      yPvId->remove_value_callback(
       asymbol_yUpdate, this );
      yPvId->release();
      yPvId = NULL;
    }
  }

  if ( anglePvExists ) {
    if ( anglePvId ) {
      anglePvId->remove_conn_state_callback(
       asymbol_monitor_angle_connect_state, this );
      anglePvId->remove_value_callback(
       asymbol_angleUpdate, this );
      anglePvId->release();
      anglePvId = NULL;
    }
  }

  }

  return 1;

}

int aniSymbolClass::moveSelectBox (
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

int aniSymbolClass::moveSelectBoxAbs (
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

int aniSymbolClass::moveSelectBoxMidpointAbs (
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

int aniSymbolClass::checkResizeSelectBox (
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

int aniSymbolClass::resizeSelectBox (
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

int aniSymbolClass::checkResizeSelectBoxAbs (
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

int aniSymbolClass::resizeSelectBoxAbs (
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

int aniSymbolClass::resizeSelectBoxAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h )
{

  if ( _x > 0 ) sboxX = _x;
  if ( _y > 0 ) sboxY = _y;
  if ( _w > 0 ) sboxW = _w;
  if ( _h > 0 ) sboxH = _h;

  return 1;

}

int aniSymbolClass::move (
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

int aniSymbolClass::moveAbs (
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

int aniSymbolClass::moveMidpointAbs (
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

int aniSymbolClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction )
{

  actWin->appCtx->postMessage( aniSymbolClass_str35 );
  actWin->appCtx->postMessage( aniSymbolClass_str37 );
  return 1;

}

int aniSymbolClass::rotateInternal (
  int xOrigin,
  int yOrigin,
  char direction )
{

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  // execute base class rotate
  ((activeGraphicClass *)this)->activeGraphicClass::rotate(
   xOrigin, yOrigin, direction );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->rotate( xOrigin, yOrigin, direction );
      cur->node->updateDimensions();

      cur->node->resizeSelectBoxAbsFromUndo( cur->node->getX0(),
       cur->node->getY0(), cur->node->getW(),
       cur->node->getH() );

      cur = cur->flink;

    }

  }

  return 1;

}

int aniSymbolClass::flip (
  int xOrigin,
  int yOrigin,
  char direction )
{

  actWin->appCtx->postMessage( aniSymbolClass_str36 );
  actWin->appCtx->postMessage( aniSymbolClass_str37 );
  return 1;

}

int aniSymbolClass::flipInternal (
  int xOrigin,
  int yOrigin,
  char direction )
{

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  // execute base class flip
  ((activeGraphicClass *)this)->activeGraphicClass::flip(
   xOrigin, yOrigin, direction );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->flip( xOrigin, yOrigin, direction );
      cur->node->updateDimensions();

      cur->node->resizeSelectBoxAbsFromUndo( cur->node->getX0(),
       cur->node->getY0(), cur->node->getW(),
       cur->node->getH() );

      cur = cur->flink;

    }

  }

  return 1;

}

int aniSymbolClass::resize (
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

int aniSymbolClass::resizeAbs (
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

int aniSymbolClass::resizeAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h )
{

  if ( _x != -1 ) x = _x;
  if ( _y != -1 ) y = _y;
  if ( _w != -1 ) w = _w;
  if ( _h != -1 ) h = _h;

  return 1;

}

void aniSymbolClass::updateGroup ( void ) { // for paste operation

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

int aniSymbolClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( deleteRequest ) return 1;

  for ( i=0; i<numPvs; i++ ) {

    tmpStr.setRaw( controlPvExpStr[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    controlPvExpStr[i].setRaw( tmpStr.getExpanded() );

  }

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->expandTemplate( numMacros, macros, expansions );

      cur = cur->flink;

    }

  }

  return 1;

}

int aniSymbolClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( deleteRequest ) return 1;

  for ( i=0; i<numPvs; i++ ) {

    controlPvExpStr[i].expand1st( numMacros, macros, expansions );

  }

  colorPvExpStr.expand1st( numMacros, macros, expansions );
  xPvExpStr.expand1st( numMacros, macros, expansions );
  yPvExpStr.expand1st( numMacros, macros, expansions );
  anglePvExpStr.expand1st( numMacros, macros, expansions );

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

int aniSymbolClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( deleteRequest ) return 1;

  for ( i=0; i<numPvs; i++ ) {

    controlPvExpStr[i].expand2nd( numMacros, macros, expansions );

  }

  colorPvExpStr.expand2nd( numMacros, macros, expansions );
  xPvExpStr.expand2nd( numMacros, macros, expansions );
  yPvExpStr.expand2nd( numMacros, macros, expansions );
  anglePvExpStr.expand2nd( numMacros, macros, expansions );

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

int aniSymbolClass::containsMacros ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( deleteRequest ) return 1;

  for ( i=0; i<numPvs; i++ ) {

    if ( controlPvExpStr[i].containsPrimaryMacros() ) return 1;

  }

  if ( colorPvExpStr.containsPrimaryMacros() ) return 1;
  if ( xPvExpStr.containsPrimaryMacros() ) return 1;
  if ( yPvExpStr.containsPrimaryMacros() ) return 1;
  if ( anglePvExpStr.containsPrimaryMacros() ) return 1;

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

void aniSymbolClass::executeDeferred ( void ) {

double v, av;
int stat, i, nci, nc[ASYMBOL_K_MAX_PVS], nr, ne, nd, ncolori, ncr, nxi, nyi,
 nai, npr, xv, yv, xv1, yv1;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  v = curControlV;
  xv = (int) curXV;
  yv = (int) curYV;
  av = curAngleV;
  nci = needConnectInit; needConnectInit = 0;
  for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
    nc[i] = needConnect[i];
    needConnect[i] = 0;
  }
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  ncolori = needColorInit; needColorInit = 0;
  ncr = needColorRefresh; needColorRefresh = 0;
  nxi = needXInit; needXInit = 0;
  nyi = needYInit; needYInit = 0;
  nai = needAngleInit; needAngleInit = 0;
  npr = needPosRefresh; needPosRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

  if ( nci ) {

    if ( !notControlPvConnected ) {
      if ( colorExists ) {
        if ( colorPvConnected ) {
          active = 1;
	}
      }
      else {
        active = 1;
      }
    }

    for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {

      if ( nc[i] ) {

        if ( initialCtrlConnection[i] ) {

	  initialCtrlConnection[i] = 0;

          controlPvId[i]->add_value_callback( asymbol_controlUpdate,
           (void *) &argRec[i] );

        }

      }

    }

  }

//----------------------------------------------------------------------------

  if ( ncolori ) {

    if ( !notControlPvConnected ) {
      if ( colorExists && colorPvConnected ) {
        active = 1;
      }
    }

    if ( initialColorConnection ) {

      initialColorConnection = 0;

      colorPvId->add_value_callback( asymbol_colorUpdate, this );

    }

  }

//----------------------------------------------------------------------------

  if ( nxi ) {

    if ( initialXPvConnection ) {

      initialXPvConnection = 0;

      xPvId->add_value_callback( asymbol_xUpdate, this );

    }

  }

//----------------------------------------------------------------------------

  if ( nyi ) {

    if ( initialYPvConnection ) {

      initialYPvConnection = 0;

      yPvId->add_value_callback( asymbol_yUpdate, this );

    }

  }

//----------------------------------------------------------------------------

  if ( nai ) {

    if ( initialAnglePvConnection ) {

      initialAnglePvConnection = 0;

      anglePvId->add_value_callback( asymbol_angleUpdate, this );

    }

  }

//----------------------------------------------------------------------------

  if ( npr ) {

    init = 1; /* active at least once */

    if ( !enabled || !activeMode || ( numStates < 1 ) ) return;

    rAxisAngle = av * M_PI / 180;

    xv1 = savedX +
     (int) ( xv * cos( rAxisAngle ) + yv * sin( rAxisAngle ) );
    yv1 = savedY +
     (int) ( yv * cos( rAxisAngle ) - xv * sin( rAxisAngle ) );

    if ( prevIndex == -1 ) prevIndex = index;

    // The needFullCopy reference is unfortunately necessary in
    // case double-buffered mode is being used because of implicit
    // assumptions in the drawing engine.  It is the result of the
    // moveAbs operation being performed in execute mode.
    eraseActive();
    stat = doSmartDrawAllButMeActive();
    moveAbs( xv1, yv1 );
    actWin->needFullCopy = 1;
    smartDrawAllActive();

  }

//----------------------------------------------------------------------------

  if ( nr ) {

    init = 1; /* active at least once */

    controlV = v;

    index = 0;
    for ( i=0; i<numStates; i++ ) {

      if ( ( controlV >= stateMinValue[i] ) &&
           ( controlV < stateMaxValue[i] ) ) {

        index = i;
        break;

      }

    }

    if ( !active ) {
      index = 0;
    }

    if ( index != prevIndex ) {
      removePrevBlink();
      eraseActive();
      stat = smartDrawAllActive();
    }

  }

//----------------------------------------------------------------------------

  if ( ne ) {
    eraseActive();
  }

//----------------------------------------------------------------------------

  if ( nd ) {
//      drawActive();
    stat = smartDrawAllActive();
//      actWin->requestActiveRefresh();
  }

//----------------------------------------------------------------------------

  if ( ncr ) {

    updateColors( curColorV );

  }

//----------------------------------------------------------------------------

}

int aniSymbolClass::setProperty (
  char *prop,
  int *value )
{

int i, stat;

  if ( strcmp( prop, aniSymbolClass_str28 ) == 0 ) {

    controlV = *value;

    index = 0;
    for ( i=0; i<numStates; i++ ) {

      if ( ( controlV >= stateMinValue[i] ) &&
           ( controlV < stateMaxValue[i] ) ) {

        index = i;
        break;

      }

    }

    if ( index != prevIndex ) {
      eraseActive();
      stat = smartDrawAllActive();
    }

  }

  return 0;

}

char *aniSymbolClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;

  return dragName[dragIndex];

}

char *aniSymbolClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) (sizeof(dragName)/sizeof(char *)) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *aniSymbolClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( i < 0 ) i = 0;
  if ( i > 4 ) i = 0;

  if ( actWin->mode == AWC_EXECUTE ) {

    return controlPvExpStr[i].getExpanded();

  }
  else {

    return controlPvExpStr[i].getRaw();

  }

}

void aniSymbolClass::changePvNames (
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
      controlPvExpStr[0].setRaw( readbackPvs[0] );
    }
  }

}

void aniSymbolClass::flushUndo ( void ) {

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

int aniSymbolClass::addUndoCreateNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

 return 1;

  stat = _undoObj->addCreateNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoCreateNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int aniSymbolClass::addUndoMoveNode (
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

int aniSymbolClass::addUndoResizeNode (
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

int aniSymbolClass::addUndoCopyNode (
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

int aniSymbolClass::addUndoCutNode (
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

int aniSymbolClass::addUndoPasteNode (
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

int aniSymbolClass::addUndoReorderNode (
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

int aniSymbolClass::addUndoEditNode (
  undoClass *_undoObj )
{

int stat;
undoAniSymbolOpClass *undoAniSymbolOpPtr;

  undoAniSymbolOpPtr = new undoAniSymbolOpClass( this );

  stat = _undoObj->addEditNode( this, undoAniSymbolOpPtr );
  if ( !( stat & 1 ) ) return stat;

  return 1;

}

int aniSymbolClass::addUndoGroupNode (
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

int aniSymbolClass::addUndoRotateNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  stat = _undoObj->addRotateNode( this, NULL, x, y, w, h );
  if ( !( stat & 1 ) ) return stat;

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

int aniSymbolClass::addUndoFlipNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  stat = _undoObj->addFlipNode( this, NULL, x, y, w, h );
  if ( !( stat & 1 ) ) return stat;

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

int aniSymbolClass::undoCreate (
  undoOpClass *opPtr
) {

  return 1;

}

int aniSymbolClass::undoMove (
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

int aniSymbolClass::undoResize (
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

int aniSymbolClass::undoCopy (
  undoOpClass *opPtr
) {

  return 1;

}

int aniSymbolClass::undoCut (
  undoOpClass *opPtr
) {

  return 1;

}

int aniSymbolClass::undoPaste (
  undoOpClass *opPtr
) {

  return 1;

}

int aniSymbolClass::undoReorder (
  undoOpClass *opPtr
) {

  return 1;

}

int aniSymbolClass::undoEdit (
  undoOpClass *opPtr
) {

undoAniSymbolOpClass *ptr = (undoAniSymbolOpClass *) opPtr;
activeGraphicListPtr head, cur, next, curSource, sourceHead;
int i;

// ============================================================

  // delete current image list, saved image list from opPtr (from undo object)
  // will be restored to symbol object

  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {

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
  x = ptr->anso->x;
  y = ptr->anso->y;
  w = ptr->anso->w;
  h = ptr->anso->h;
  sboxX = ptr->anso->sboxX;
  sboxY = ptr->anso->sboxY;
  sboxW = ptr->anso->sboxW;
  sboxH = ptr->anso->sboxH;
  orientation = ptr->anso->orientation;
  nextToEdit = ptr->anso->nextToEdit;
  nextSelectedToEdit = NULL;
  inGroup = ptr->anso->inGroup;

// ============================================================

  // restore saved symbol image list
  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {

    head = new activeGraphicListType;
    head->flink = head;
    head->blink = head;

    sourceHead = (activeGraphicListPtr) ptr->anso->voidHead[i];
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
  for ( i=0; i<ASYMBOL_K_NUM_STATES; i++ ) {
    sourceHead = (activeGraphicListPtr) ptr->anso->voidHead[i];
    sourceHead->flink = sourceHead;
    sourceHead->blink = sourceHead;
  }

// ============================================================

  index = 0;
  for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
    controlVals[i] = 0.0;
    controlPvExpStr[i].setRaw( ptr->anso->controlPvExpStr[i].rawString );
    strncpy( cAndMask[i], ptr->anso->cAndMask[i], 9 );
    strncpy( cXorMask[i], ptr->anso->cXorMask[i], 9 );
    shiftCount[i] = ptr->anso->shiftCount[i];
  }

  // restore remaining attributes

  strncpy( symbolFileName, ptr->anso->symbolFileName, 127 );

  numStates = ptr->anso->numStates;
  for ( i=0; i<numStates; i++ ) {
    stateMinValue[i] = ptr->anso->stateMinValue[i];
    stateMaxValue[i] = ptr->anso->stateMaxValue[i];
  }

  useOriginalSize = ptr->anso->useOriginalSize;

  binaryTruthTable = ptr->anso->binaryTruthTable;

  orientation = ptr->anso->orientation;

  numPvs = ptr->anso->numPvs;

  useOriginalColors = ptr->anso->useOriginalColors;
  fgCb = ptr->anso->fgCb;
  bgCb = ptr->anso->bgCb;
  fgColor = ptr->anso->fgColor;
  bgColor = ptr->anso->bgColor;
  colorPvExpStr.setRaw( ptr->anso->colorPvExpStr.rawString );
  xPvExpStr.setRaw( ptr->anso->xPvExpStr.rawString );
  yPvExpStr.setRaw( ptr->anso->yPvExpStr.rawString );
  anglePvExpStr.setRaw( ptr->anso->anglePvExpStr.rawString );

  return 1;

}

int aniSymbolClass::undoGroup (
  undoOpClass *opPtr
) {

  return 1;

}

int aniSymbolClass::undoRotate (
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

int aniSymbolClass::undoFlip (
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

void aniSymbolClass::updateColors (
  double colorValue
) {

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

void aniSymbolClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

int i;

  if ( max < ( ASYMBOL_K_MAX_PVS + 1 ) ) {
    *n = 0;
    return;
  }

  *n = ASYMBOL_K_MAX_PVS + 1;
  for ( i=0; i<ASYMBOL_K_MAX_PVS; i++ ) {
    pvs[i] = controlPvId[i];
  }
  pvs[i] = colorPvId; i++;
  pvs[i] = xPvId; i++;
  pvs[i] = yPvId; i++;
  pvs[i] = anglePvId; i++;

}

char *aniSymbolClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return colorPvExpStr.getRaw();
  }
  else if ( i == 1 ) {
    return xPvExpStr.getRaw();
  }
  else if ( i == 2 ) {
    return yPvExpStr.getRaw();
  }
  else if ( i == 3 ) {
    return anglePvExpStr.getRaw();
  }
  else if ( ( i > 3 ) && ( i < numPvs + 4 ) ) {
    return controlPvExpStr[i-4].getRaw();
  }

  return NULL;

}

void aniSymbolClass::replaceString (
  int i,
  int max,
  char *string
) {

int saveW, saveH, saveX, saveY, prevOr;
int status, resizeStat;

  if ( i == 0 ) {
    colorPvExpStr.setRaw( string );
  }
  else if ( i == 1 ) {
    xPvExpStr.setRaw( string );
  }
  else if ( i == 2 ) {
    yPvExpStr.setRaw( string );
  }
  else if ( i == 3 ) {
    anglePvExpStr.setRaw( string );
  }
  else if ( ( i > 3 ) && ( i < numPvs + 4 ) ) {
    controlPvExpStr[i-4].setRaw( string );
  }

}

// crawler functions may return blank pv names
char *aniSymbolClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return controlPvExpStr[0].getExpanded();

}

char *aniSymbolClass::crawlerGetNextPv ( void ) {

  crawlerPvIndex++;

  if ( crawlerPvIndex < ASYMBOL_K_MAX_PVS-3 ) {
    return controlPvExpStr[crawlerPvIndex].getExpanded();
  }
  else if ( crawlerPvIndex < ASYMBOL_K_MAX_PVS-2 ) {
    return colorPvExpStr.getExpanded();
  }
  else if ( crawlerPvIndex < ASYMBOL_K_MAX_PVS-1 ) {
    return xPvExpStr.getExpanded();
  }
  else if ( crawlerPvIndex < ASYMBOL_K_MAX_PVS ) {
    return yPvExpStr.getExpanded();
  }
  else if ( crawlerPvIndex == ASYMBOL_K_MAX_PVS ) {
    return anglePvExpStr.getExpanded();
  }
  else {
    return NULL;
  }

}
