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

#define __symbol_cc 1

#include "symbol.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

#ifdef __epics__

static void symbol_monitor_control_connect_state (
  struct connection_handler_args arg )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(arg.chid);
activeSymbolClass *aso = (activeSymbolClass *) ptr->objPtr;

  if ( arg.op == CA_OP_CONN_UP ) {

    aso->needConnectInit = 1;
    aso->needConnect[ptr->index] = 1;

    aso->notControlPvConnected &= ptr->clrMask;


  }
  else {

    aso->notControlPvConnected |= ptr->setMask;
    aso->active = 0;
    aso->bufInvalidate();
    aso->needDraw = 1;

  }

  aso->actWin->appCtx->proc->lock();
  aso->actWin->addDefExeNode( aso->aglPtr );
  aso->actWin->appCtx->proc->unlock();

}

static void symbol_monitor_color_connect_state (
  struct connection_handler_args arg )
{

activeSymbolClass *aso = (activeSymbolClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    aso->needColorInit = 1;
    aso->colorPvConnected = 1;

  }
  else {

    aso->colorPvConnected = 0;
    aso->active = 0;
    aso->bufInvalidate();
    aso->needDraw = 1;

  }

  aso->actWin->appCtx->proc->lock();
  aso->actWin->addDefExeNode( aso->aglPtr );
  aso->actWin->appCtx->proc->unlock();

}

static void symbol_controlUpdate (
  struct event_handler_args ast_args )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) ca_puser(ast_args.chid);
activeSymbolClass *aso = (activeSymbolClass *) ptr->objPtr;

  if ( aso->active ) {

    if ( aso->binaryTruthTable ) {
      aso->controlVals[ptr->index] = *( (double *) ast_args.dbr );
      if ( aso->controlVals[ptr->index] != 0 )
        aso->iValue |= ptr->setMask;
      else
        aso->iValue &= ptr->clrMask;
      aso->curControlV = (double) aso->iValue;
    }
    else {
      aso->curControlV = *( (double *) ast_args.dbr );
    }

    aso->needRefresh = 1;
    aso->actWin->appCtx->proc->lock();
    aso->actWin->addDefExeNode( aso->aglPtr );
    aso->actWin->appCtx->proc->unlock();

  }

}

static void symbol_colorUpdate (
  struct event_handler_args ast_args )
{

activeSymbolClass *aso = (activeSymbolClass *) ca_puser(ast_args.chid);

  if ( aso->active ) {

    aso->curColorV = *( (double *) ast_args.dbr );

    aso->needColorRefresh = 1;
    aso->actWin->appCtx->proc->lock();
    aso->actWin->addDefExeNode( aso->aglPtr );
    aso->actWin->appCtx->proc->unlock();

  }

}

#endif

static void symbolSetItem (
  Widget w,
  XtPointer client,
  XtPointer call )
{

efSetItemCallbackDscPtr dsc = (efSetItemCallbackDscPtr) client;
entryFormClass *ef = (entryFormClass *) dsc->ef;
activeSymbolClass *aso = (activeSymbolClass *) dsc->obj;
int i;

  for ( i=0; i<aso->numStates; i++ ) {
    aso->elsvMin->setValue( aso->bufStateMinValue[ef->index] );
    aso->elsvMax->setValue( aso->bufStateMaxValue[ef->index] );
  }

}

static void asc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSymbolClass *aso = (activeSymbolClass *) client;
int stat, resizeStat, i, saveW, saveH, saveX, saveY;

  aso->actWin->setChanged();

  aso->eraseSelectBoxCorners();
  aso->erase();

  strncpy( aso->id, aso->bufId, 31 );

  aso->x = aso->bufX;
  aso->sboxX = aso->bufX;

  aso->y = aso->bufY;
  aso->sboxY = aso->bufY;

  aso->numPvs = 0;
  for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) {
    aso->controlPvExpStr[i].setRaw( aso->bufControlPvName[i] );
    if ( !blank( aso->bufControlPvName[i] ) )
      (aso->numPvs)++;
    else
      break; /* pv entries on form must be contiguous */
  }

  aso->colorPvExpStr.setRaw( aso->bufColorPvName );

  strncpy( aso->symbolFileName, aso->bufSymbolFileName, 127 );

  aso->numStates = aso->ef.numItems;

  aso->useOriginalSize = aso->bufUseOriginalSize;

  aso->useOriginalColors = aso->bufUseOriginalColors;

  aso->binaryTruthTable = aso->bufBinaryTruthTable;

  aso->orientation = aso->bufOrientation;

  for ( i=0; i<aso->numStates; i++ ) {
    aso->stateMinValue[i] = aso->bufStateMinValue[i];
    aso->stateMaxValue[i] = aso->bufStateMaxValue[i];
  }

  if ( aso->useOriginalSize ) {
    stat = aso->readSymbolFile();
  }
  else {
    if ( ( aso->prevOr == OR_CW ) || ( aso->prevOr == OR_CCW ) ) {
      saveW = aso->h;
      saveH = aso->w;
    }
    else {
      saveW = aso->w;
      saveH = aso->h;
    }
    stat = aso->readSymbolFile();
    if ( ( saveW != aso->w ) || ( saveH != aso->h ) ) {
      resizeStat = aso->checkResizeSelectBoxAbs( -1, -1, saveW, saveH );
      if ( resizeStat & 1 ) {
        aso->resizeSelectBoxAbs( -1, -1, saveW, saveH );
        aso->resizeAbs( -1, -1, saveW, saveH );
      }
      else {
        aso->actWin->appCtx->postMessage(
         activeSymbolClass_str7 );
      }
    }
  }

  aso->prevOr = aso->orientation;

  if ( !( stat & 1 ) ) {
    aso->actWin->appCtx->postMessage( activeSymbolClass_str8 );
  }
  else {

    saveX = aso->x;
    saveY = aso->y;

    switch ( aso->orientation ) {

    case OR_CW:
      aso->rotate( aso->getXMid(), aso->getYMid(), '+' );
      aso->moveAbs( saveX, saveY );
      aso->resizeSelectBoxAbsFromUndo( aso->getX0(), aso->getY0(),
       aso->getW(), aso->getH() );
      break;

    case OR_CCW:
      aso->rotate( aso->getXMid(), aso->getYMid(), '-' );
      aso->moveAbs( saveX, saveY );
      aso->resizeSelectBoxAbsFromUndo( aso->getX0(), aso->getY0(),
       aso->getW(), aso->getH() );
      break;

    case OR_V:
      aso->flip( aso->getXMid(), aso->getYMid(), 'V' );
      aso->moveAbs( saveX, saveY );
      aso->resizeSelectBoxAbsFromUndo( aso->getX0(), aso->getY0(),
       aso->getW(), aso->getH() );
      break;

    case OR_H:
      aso->flip( aso->getXMid(), aso->getYMid(), 'H' );
      aso->moveAbs( saveX, saveY );
      aso->resizeSelectBoxAbsFromUndo( aso->getX0(), aso->getY0(),
       aso->getW(), aso->getH() );
      break;

    }

  }

}

static void asc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSymbolClass *aso = (activeSymbolClass *) client;

  asc_edit_update ( w, client, call );
  aso->refresh( aso );

}

static void asc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSymbolClass *aso = (activeSymbolClass *) client;

  asc_edit_update ( w, client, call );
  aso->ef.popdown();
  aso->operationComplete();

}

static void asc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSymbolClass *aso = (activeSymbolClass *) client;

  aso->ef.popdown();
  aso->operationCancel();

}

static void asc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSymbolClass *aso = (activeSymbolClass *) client;

  aso->ef.popdown();
  aso->operationCancel();
  aso->erase();
  aso->deleteRequest = 1;
  aso->drawAll();

}

activeSymbolClass::activeSymbolClass ( void ) {

activeGraphicListPtr head;
int i;

  name = new char[strlen("activeSymbolClass")+1];
  strcpy( name, "activeSymbolClass" );

  for ( i=0; i<SYMBOL_K_NUM_STATES; i++ ) {

    head = new activeGraphicListType;
    head->flink = head;
    head->blink = head;

    voidHead[i] = (void *) head;

  }

  activeMode = 0;
  numStates = 2;
  index = 0;
  controlV = 0.0;
  for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) {
    controlVals[i] = 0.0;
  }
  iValue = 0;
  strcpy( symbolFileName, "" );

  useOriginalSize = 0;
  binaryTruthTable = 0;
  orientation = 0;
  numPvs = 1;

  for ( i=0; i<SYMBOL_K_NUM_STATES; i++ ) {
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

}

activeSymbolClass::~activeSymbolClass ( void ) {

//   printf( "In activeSymbolClass::~activeSymbolClass\n" );

activeGraphicListPtr head;
activeGraphicListPtr cur, next;
btnActionListPtr curBtnAction, nextBtnAction;
int i;

  for ( i=0; i<SYMBOL_K_NUM_STATES; i++ ) {

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
activeSymbolClass::activeSymbolClass
 ( const activeSymbolClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;
activeGraphicListPtr head, cur, curSource, sourceHead;
int i;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeSymbolClass")+1];
  strcpy( name, "activeSymbolClass" );

  for ( i=0; i<SYMBOL_K_NUM_STATES; i++ ) {

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
  for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) {
    controlVals[i] = 0.0;
    controlPvExpStr[i].setRaw( source->controlPvExpStr[i].rawString );
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
  bufFgColor = source->bufFgColor;
  bufBgColor = source->bufBgColor;
  colorPvExpStr.setRaw( source->colorPvExpStr.rawString );

}

int activeSymbolClass::createInteractive (
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
  bufFgColor = actWin->defaultTextFgColor;
  bufBgColor = actWin->defaultBgColor;

  this->editCreate();

  return 1;

}

int activeSymbolClass::genericEdit ( void )
{

int i;
char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeSymbolClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeSymbolClass_str9, 31 );

  strncat( title, activeSymbolClass_str10, 31 );

  strncpy( bufId, id, 31 );

  bufX = x;
  bufY = y;

  strncpy( bufSymbolFileName, symbolFileName, 127 );

  if ( colorPvExpStr.getRaw() )
    strncpy( bufColorPvName, colorPvExpStr.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufColorPvName, "" );

  for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) {
    if ( controlPvExpStr[i].getRaw() )
      strncpy( bufControlPvName[i], controlPvExpStr[i].getRaw(),
       activeGraphicClass::MAX_PV_NAME );
    else
      strcpy( bufControlPvName[i], "" );
  }

  for ( i=0; i<SYMBOL_K_NUM_STATES; i++ ) {
    bufStateMinValue[i] = stateMinValue[i];
    bufStateMaxValue[i] = stateMaxValue[i];
  }

  bufUseOriginalSize = useOriginalSize;

  bufBinaryTruthTable = binaryTruthTable;

  prevOr = bufOrientation = orientation;

  bufUseOriginalColors = useOriginalColors;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, SYMBOL_K_NUM_STATES, numStates,
   symbolSetItem, (void *) this, NULL, NULL, NULL );

  //ef.addTextField( activeSymbolClass_str11, 30, bufId, 31 );

  ef.addTextField( activeSymbolClass_str12, 30, &bufX );
  ef.addTextField( activeSymbolClass_str13, 30, &bufY );
  ef.addTextField( activeSymbolClass_str14, 30, bufSymbolFileName, 127 );
  ef.addTextField( activeSymbolClass_str29, 30, bufColorPvName,
   activeGraphicClass::MAX_PV_NAME );

  ef.addTextField( activeSymbolClass_str17, 30, bufControlPvName[0],
   activeGraphicClass::MAX_PV_NAME );

  for ( i=1; i<SYMBOL_K_MAX_PVS; i++ ) {
    ef.addTextField( " ", 30, bufControlPvName[i],
     activeGraphicClass::MAX_PV_NAME );
  }

  ef.addToggle( activeSymbolClass_str16, &bufBinaryTruthTable );

  ef.addOption( activeSymbolClass_str33, activeSymbolClass_str34,
   &bufOrientation );

  ef.addToggle( activeSymbolClass_str15, &bufUseOriginalSize );

  ef.addToggle( activeSymbolClass_str30, &bufUseOriginalColors );

  ef.addColorButton(activeSymbolClass_str31, actWin->ci, &fgCb, &bufFgColor );

  ef.addColorButton(activeSymbolClass_str32, actWin->ci, &bgCb, &bufBgColor );

  for ( i=0; i<SYMBOL_K_NUM_STATES; i++ ) {
    minPtr[i] = &bufStateMinValue[i];
    maxPtr[i] = &bufStateMaxValue[i];
  }

  ef.addTextFieldArray( activeSymbolClass_str18, 30, bufStateMinValue, &elsvMin );
  ef.addTextFieldArray( activeSymbolClass_str19, 30, bufStateMaxValue, &elsvMax );

  return 1;

}

int activeSymbolClass::edit ( void )
{

  this->genericEdit();
  ef.finished( asc_edit_ok, asc_edit_apply, asc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeSymbolClass::editCreate ( void )
{

  this->genericEdit();
  ef.finished( asc_edit_ok, asc_edit_apply, asc_edit_cancel_delete,
   this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeSymbolClass::readSymbolFile ( void )
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

  for ( i=0; i<SYMBOL_K_NUM_STATES; i++ ) {

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
    fclose( f );
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
        fclose( f );
        actWin->setLine( saveLine );
        return 0;
      }
      numStates = i+1;
      fclose( f );
      actWin->setLine( saveLine );
      return 1;
    }

    tk = strtok( itemName, " \t\n" );
    if ( strcmp( tk, "activeGroupClass" ) != 0 ) {
      // numStates = 0;
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
        // numStates = 0;
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
          printf( activeSymbolClass_str20 );
          // numStates = 0;
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
          printf( activeSymbolClass_str21 );
          // numStates = 0;
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

int activeSymbolClass::save (
 FILE *f )
{

int i;

  switch ( orientation ) {

  case OR_CW:
    rotate( getXMid(), getYMid(), '-' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  case OR_CCW:
    rotate( getXMid(), getYMid(), '+' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  }

  fprintf( f, "%-d %-d %-d\n", ASC_MAJOR_VERSION, ASC_MINOR_VERSION,
   ASC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

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

  switch ( orientation ) {

  case OR_CW:
    rotate( getXMid(), getYMid(), '+' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  case OR_CCW:
    rotate( getXMid(), getYMid(), '-' );
    resizeSelectBoxAbsFromUndo( getX0(), getY0(),
     getW(), getH() );
    break;

  }

  // version 1.5.0
  if ( colorPvExpStr.getRaw() )
    writeStringToFile( f, colorPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1.6.0
  fprintf( f, "%-d\n", useOriginalColors );
  fprintf( f, "%-d\n", bufFgColor );
  fprintf( f, "%-d\n", bufBgColor );

  return 1;

}

int activeSymbolClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int stat, resizeStat, i, saveW, saveH;
int major, minor, release;
char string[activeGraphicClass::MAX_PV_NAME+1];
float val;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

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
    readStringFromFile( string, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    controlPvExpStr[i].setRaw( string );
  }

  fscanf( f, "%d\n", &numStates ); actWin->incLine();

  if ( numStates < 1 ) numStates = 1;
  if ( numStates > SYMBOL_K_NUM_STATES ) numStates = SYMBOL_K_NUM_STATES;

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
    readStringFromFile( string, activeGraphicClass::MAX_PV_NAME+1, f );
     actWin->incLine();
    colorPvExpStr.setRaw( string );
  }

  if ( ( major > 1 ) || ( minor > 5 ) ) {
    fscanf( f, "%d\n", &useOriginalColors ); actWin->incLine();
    fscanf( f, "%d\n", &bufFgColor ); actWin->incLine();
    fscanf( f, "%d\n", &bufBgColor ); actWin->incLine();
  }
  else {
    useOriginalColors = 1;
    bufFgColor = actWin->defaultTextFgColor;
    bufBgColor = actWin->defaultBgColor;
  }

  saveW = w;
  saveH = h;

  stat = readSymbolFile();
  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( activeSymbolClass_str22 );
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
           activeSymbolClass_str23 );
        }
      }
    }

    switch ( orientation ) {

    case OR_CW:
      rotate( getXMid(), getYMid(), '+' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    case OR_CCW:
      rotate( getXMid(), getYMid(), '-' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    case OR_V:
      flip( getXMid(), getYMid(), 'V' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    case OR_H:
      flip( getXMid(), getYMid(), 'H' );
      resizeSelectBoxAbsFromUndo( getX0(), getY0(),
       getW(), getH() );
      break;

    }

  }

  return 1;

}

int activeSymbolClass::erase ( void ) {

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

int activeSymbolClass::eraseActive ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;

  if ( !init || !activeMode || ( numStates < 1 ) ) return 1;

  if ( ( prevIndex >= 0 ) && ( prevIndex < numStates ) ) {

    head = (activeGraphicListPtr) voidHead[prevIndex];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->eraseActive();

      cur = cur->flink;

    }

  }

  prevIndex = index;

  return 1;

}

int activeSymbolClass::draw ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;

  if ( activeMode || deleteRequest ) return 1;

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

int activeSymbolClass::drawActive ( void ) {

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

  return 1;

}

int activeSymbolClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag )
{

  *up = 1;
  *down = 1;
  *drag = 0;

  return 1;

}

void activeSymbolClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

  // inc index on button down, dec index on shift button down

}

void activeSymbolClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

}

int activeSymbolClass::activate (
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
	 "", 0, "", 0, "", 0, bufFgColor, bufFgColor, 0, 0, bufBgColor,
         0, 0 );
      }
      cur->node->activate( pass, (void *) cur );
      cur = cur->flink;

    }

  }

  switch ( pass ) {

  case 1:

    needErase = needDraw = needRefresh = needConnectInit =
     needColorInit = needColorRefresh = 0;
    for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) needConnect[i] = 0;
    aglPtr = ptr;
    iValue = 0; /* this get set via OR/AND operations */
    prevIndex = -1;
    init = 0;
    controlExists = 0;
    opComplete = 0;
    active = 0;
    activeMode = 1;
    controlV = 1;

    if ( binaryTruthTable )
      notControlPvConnected = (int) pow(2,numPvs) - 1;
    else {
      numPvs = 1;
      notControlPvConnected = 1;
    }

    controlExists = 1;
    for ( i=0; i<numPvs; i++ ) {

#ifdef __epics__
      controlEventId[i] = 0;
#endif

      if ( !controlPvExpStr[i].getExpanded() ||
           blank( controlPvExpStr[i].getExpanded() ) ) controlExists = 0;

    }

    if ( blank( colorPvExpStr.getExpanded() ) ) {
      colorExists = 0;
    }
    else {
      colorExists = 1;
    }

#ifdef __epics__
    colorEventId = 0;
#endif

    break;

  case 2:

    if ( !opComplete ) {

      opStat = 1;

      if ( controlExists ) {

        for ( i=0; i<numPvs; i++ ) {

          argRec[i].objPtr = (void *) this;
          argRec[i].index = i;
          argRec[i].setMask = (unsigned int) 1 << i;
          argRec[i].clrMask = ~(argRec[i].setMask);

#ifdef __epics__
          stat = ca_search_and_connect( controlPvExpStr[i].getExpanded(),
           &controlPvId[i], symbol_monitor_control_connect_state, &argRec[i] );
          if ( stat != ECA_NORMAL ) {
            printf( activeSymbolClass_str24 );
            opStat = 0;
          }
#endif

        }

      }
      else {

        init = 1;
        active = 1;
        index = 1;

      }

      if ( colorExists ) {

#ifdef __epics__
        stat = ca_search_and_connect( colorPvExpStr.getExpanded(),
         &colorPvId, symbol_monitor_color_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activeSymbolClass_str24 );
          opStat = 0;
        }
#endif

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

int activeSymbolClass::deactivate (
  int pass
) {

int i, stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while ( cur != head ) {

      cur->node->deactivate( pass );

      cur = cur->flink;

    }

  }

  if ( pass == 1 ) {

  active = 0;
  activeMode = 0;

#ifdef __epics__

  for ( i=0; i<numPvs; i++ ) {

    if ( controlEventId[i] ) {
      stat = ca_clear_event( controlEventId[i] );
      if ( stat != ECA_NORMAL )
        printf( activeSymbolClass_str25 );
      controlEventId[i] = 0;
    }

    if ( controlExists ) {
      stat = ca_clear_channel( controlPvId[i] );
      if ( stat != ECA_NORMAL )
        printf( activeSymbolClass_str26 );
    }

  }

  if ( colorEventId ) {
    stat = ca_clear_event( colorEventId );
    if ( stat != ECA_NORMAL )
      printf( activeSymbolClass_str25 );
    colorEventId = 0;
  }

  if ( colorExists ) {
    stat = ca_clear_channel( colorPvId );
    if ( stat != ECA_NORMAL )
      printf( activeSymbolClass_str26 );
  }

#endif

  }

  return 1;

}

int activeSymbolClass::moveSelectBox (
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

int activeSymbolClass::moveSelectBoxAbs (
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

int activeSymbolClass::moveSelectBoxMidpointAbs (
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

int activeSymbolClass::checkResizeSelectBox (
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

int activeSymbolClass::resizeSelectBox (
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

int activeSymbolClass::checkResizeSelectBoxAbs (
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

int activeSymbolClass::resizeSelectBoxAbs (
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

int activeSymbolClass::resizeSelectBoxAbsFromUndo (
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

int activeSymbolClass::move (
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

int activeSymbolClass::moveAbs (
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

int activeSymbolClass::moveMidpointAbs (
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

int activeSymbolClass::rotate (
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

int activeSymbolClass::flip (
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

int activeSymbolClass::resize (
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

int activeSymbolClass::resizeAbs (
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

int activeSymbolClass::resizeAbsFromUndo (
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

void activeSymbolClass::updateGroup ( void ) { // for paste operation

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

int activeSymbolClass::expand1st (
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

int activeSymbolClass::expand2nd (
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

int activeSymbolClass::containsMacros ( void ) {

activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

  if ( deleteRequest ) return 1;

  for ( i=0; i<numPvs; i++ ) {

    if ( controlPvExpStr[i].containsPrimaryMacros() ) return 1;

  }

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

void activeSymbolClass::executeDeferred ( void ) {

double v;
int stat, i, nci, nc[SYMBOL_K_MAX_PVS], nr, ne, nd, ncolori, ncr;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  v = curControlV;
  nci = needConnectInit; needConnectInit = 0;
  for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) {
    nc[i] = needConnect[i];
    needConnect[i] = 0;
  }
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

    for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) {

      if ( nc[i] ) {

        if ( !controlEventId[i] ) {

          stat = ca_add_masked_array_event( DBR_DOUBLE, 1,
           controlPvId[i], symbol_controlUpdate, (void *) &argRec[i],
           (float) 0.0, (float) 0.0, (float) 0.0, &controlEventId[i],
           DBE_VALUE );
          if ( stat != ECA_NORMAL )
            printf( activeSymbolClass_str27 );

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

    stat = ca_add_masked_array_event( DBR_DOUBLE, 1,
     colorPvId, symbol_colorUpdate, (void *) this,
     (float) 0.0, (float) 0.0, (float) 0.0, &colorEventId,
     DBE_VALUE );
    if ( stat != ECA_NORMAL )
      printf( activeSymbolClass_str27 );

  }

#endif

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

    if ( index != prevIndex ) {
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

int activeSymbolClass::setProperty (
  char *prop,
  int *value )
{

int i, stat;

  if ( strcmp( prop, activeSymbolClass_str28 ) == 0 ) {

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

char *activeSymbolClass::firstDragName ( void ) {

  dragIndex = 0;

  if ( binaryTruthTable ) {
    return dragNameTruthTable[dragIndex];
  }
  else {
    return dragName[dragIndex];
  }

}

char *activeSymbolClass::nextDragName ( void ) {

  if ( binaryTruthTable ) {
    if ( dragIndex < (int) (sizeof(dragNameTruthTable)/sizeof(char *)) - 1 ) {
      dragIndex++;
      return dragNameTruthTable[dragIndex];
    }
    else {
      return NULL;
    }
  }
  else {
    return NULL;
  }

}

char *activeSymbolClass::dragValue (
  int i ) {

  if ( i < 0 ) i = 0;
  if ( i > 4 ) i = 0;
  return controlPvExpStr[i].getExpanded();

}

void activeSymbolClass::changePvNames (
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

void activeSymbolClass::flushUndo ( void ) {

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

int activeSymbolClass::addUndoCreateNode (
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

int activeSymbolClass::addUndoMoveNode (
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

int activeSymbolClass::addUndoResizeNode (
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

int activeSymbolClass::addUndoCopyNode (
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

int activeSymbolClass::addUndoCutNode (
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

int activeSymbolClass::addUndoPasteNode (
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

int activeSymbolClass::addUndoReorderNode (
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

int activeSymbolClass::addUndoEditNode (
  undoClass *_undoObj )
{

int stat;
activeGraphicListPtr head;
activeGraphicListPtr cur;
int i;

 return 1;

  stat = _undoObj->addEditNode( this, NULL );
  if ( !( stat & 1 ) ) return stat;

  undoObj.startNewUndoList( "" );

  for ( i=0; i<numStates; i++ ) {

    head = (activeGraphicListPtr) voidHead[i];

    cur = head->flink;
    while( cur != head ) {

      stat = cur->node->addUndoEditNode( &(this->undoObj) );
      if ( !( stat & 1 ) ) return stat;

      cur = cur->flink;

    }

  }

  return 1;

}

int activeSymbolClass::addUndoGroupNode (
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

int activeSymbolClass::addUndoRotateNode (
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

int activeSymbolClass::addUndoFlipNode (
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

int activeSymbolClass::undoCreate (
  undoOpClass *opPtr
) {

  return 1;

}

int activeSymbolClass::undoMove (
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

int activeSymbolClass::undoResize (
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

int activeSymbolClass::undoCopy (
  undoOpClass *opPtr
) {

  return 1;

}

int activeSymbolClass::undoCut (
  undoOpClass *opPtr
) {

  return 1;

}

int activeSymbolClass::undoPaste (
  undoOpClass *opPtr
) {

  return 1;

}

int activeSymbolClass::undoReorder (
  undoOpClass *opPtr
) {

  return 1;

}

int activeSymbolClass::undoEdit (
  undoOpClass *opPtr
) {

  return 1;

}

int activeSymbolClass::undoGroup (
  undoOpClass *opPtr
) {

  return 1;

}

int activeSymbolClass::undoRotate (
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

int activeSymbolClass::undoFlip (
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

void activeSymbolClass::updateColors (
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
