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

#define __symbolGen_cc 1

#include "symbolGen.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void symbol_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) clientData;
activeSymbolClass *aso = (activeSymbolClass *) ptr->objPtr;

  aso->actWin->appCtx->proc->lock();

  if ( !aso->activeMode ) {
    aso->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

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

  aso->actWin->addDefExeNode( aso->aglPtr );

  aso->actWin->appCtx->proc->unlock();

}

static void symbol_controlUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

objPlusIndexPtr ptr = (objPlusIndexPtr) clientData;

// How do we implement controlVals below?

activeSymbolClass *aso = (activeSymbolClass *) ptr->objPtr;

  aso->actWin->appCtx->proc->lock();

  if ( !aso->activeMode ) {
    aso->actWin->appCtx->proc->unlock();
    return;
  }

  if ( aso->binaryTruthTable ) {
    aso->controlVals[ptr->index] =
     *( (double *) classPtr->getValue( args ) );
    if ( aso->controlVals[ptr->index] != 0 )
      aso->iValue |= ptr->setMask;
    else
      aso->iValue &= ptr->clrMask;
    aso->curControlV = (double) aso->iValue;
  }
  else {
    aso->curControlV = *( (double *) classPtr->getValue( args ) );
  }

  aso->needRefresh = 1;

  aso->actWin->addDefExeNode( aso->aglPtr );

  aso->actWin->appCtx->proc->unlock();

}

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
int stat, resizeStat, i, saveW, saveH;

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

  strncpy( aso->symbolFileName, aso->bufSymbolFileName, 127 );

  aso->numStates = aso->ef.numItems;

  aso->useOriginalSize = aso->bufUseOriginalSize;

  aso->binaryTruthTable = aso->bufBinaryTruthTable;

  strncpy( aso->pvUserClassName, aso->actWin->pvObj.getPvName(aso->pvNameIndex),
    15);
  strncpy( aso->pvClassName, aso->actWin->pvObj.getPvClassName(aso->pvNameIndex),
    15);

  for ( i=0; i<aso->numStates; i++ ) {
    aso->stateMinValue[i] = aso->bufStateMinValue[i];
    aso->stateMaxValue[i] = aso->bufStateMaxValue[i];
  }

  if ( aso->useOriginalSize ) {
    stat = aso->readSymbolFile();
  }
  else {
    saveW = aso->w;
    saveH = aso->h;
    stat = aso->readSymbolFile();
    if ( ( saveW != aso->w ) || ( saveH != aso->h ) ) {
      resizeStat = aso->checkResizeSelectBoxAbs( -1, -1, saveW, saveH );
      if ( resizeStat & 1 ) {
        aso->resizeSelectBoxAbs( -1, -1, saveW, saveH );
        aso->resizeAbs( -1, -1, saveW, saveH );
      }
      else {
        aso->actWin->appCtx->postMessage(
         "Symbol resize underflow - using original size" );
      }
    }
  }

  if ( !( stat & 1 ) ) {
    aso->actWin->appCtx->postMessage( "Cannot read symbol file" );
  }

}

static void asc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSymbolClass *aso = (activeSymbolClass *) client;

  asc_edit_update( w, client, call );
  aso->refresh( aso );

}

static void asc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeSymbolClass *aso = (activeSymbolClass *) client;

  asc_edit_apply ( w, client, call );
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

  aso->erase();
  aso->deleteRequest = 1;
  aso->ef.popdown();
  aso->operationCancel();
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

  strcpy( pvClassName, "" );
  strcpy( pvUserClassName, "" );

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

  strncpy( pvClassName, source->pvClassName, 15 );
  strncpy( pvUserClassName, source->pvUserClassName, 15 );

  numStates = source->numStates;
  for ( i=0; i<numStates; i++ ) {
    stateMinValue[i] = source->stateMinValue[i];
    stateMaxValue[i] = source->stateMaxValue[i];
  }

  useOriginalSize = source->useOriginalSize;

  binaryTruthTable = source->binaryTruthTable;
  numPvs = source->numPvs;

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

  strncpy( pvUserClassName, actWin->defaultPvType, 15 );

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
    strncpy( title, "Unknown object", 31 );

  strncat( title, " Properties", 31 );

  strncpy( bufId, id, 31 );

  bufX = x;
  bufY = y;

  strncpy( bufSymbolFileName, symbolFileName, 127 );

  for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) {
    if ( controlPvExpStr[i].getRaw() )
      strncpy( bufControlPvName[i], controlPvExpStr[i].getRaw(), 39 );
    else
      strncpy( bufControlPvName[i], "", 39 );
  }

  for ( i=0; i<SYMBOL_K_NUM_STATES; i++ ) {
    bufStateMinValue[i] = stateMinValue[i];
    bufStateMaxValue[i] = stateMaxValue[i];
  }

  bufUseOriginalSize = useOriginalSize;

  bufBinaryTruthTable = binaryTruthTable;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, SYMBOL_K_NUM_STATES, numStates,
   symbolSetItem, (void *) this, NULL, NULL, NULL );

  ef.addTextField( "ID", 27, bufId, 31 );

  ef.addTextField( "X", 27, &bufX );
  ef.addTextField( "Y", 27, &bufY );
  ef.addTextField( "Symbol File", 27, bufSymbolFileName, 127 );
  ef.addToggle( "Preserve Original Size", &bufUseOriginalSize );

  ef.addToggle( "Binary Truth Table", &bufBinaryTruthTable );

  ef.addTextField( "PV Names", 27, bufControlPvName[0], 39 );
  for ( i=1; i<SYMBOL_K_MAX_PVS; i++ ) {
    ef.addTextField( " ", 27, bufControlPvName[i], 39 );
  }

  for ( i=0; i<SYMBOL_K_NUM_STATES; i++ ) {
    minPtr[i] = &bufStateMinValue[i];
    maxPtr[i] = &bufStateMaxValue[i];
  }

  ef.addTextFieldArray( ">=", 27, bufStateMinValue, &elsvMin );
  ef.addTextFieldArray( "<", 27, bufStateMaxValue, &elsvMax );

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

  f = actWin->openAny( symbolFileName, "r" );
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
    return stat; // memory leak here
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

    fgets( itemName, 127, f );

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
          printf( "Insufficient virtual memory - abort\n" );
          // numStates = 0;
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
            return stat; // memory leak here
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
  writeStringToFile( f, pvClassName );

  return 1;

}

int activeSymbolClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int stat, resizeStat, i, saveW, saveH;
int major, minor, release;
char string[39+1];
float val;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox();

  readStringFromFile( symbolFileName, 127, f ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    fscanf( f, "%d\n", &binaryTruthTable ); actWin->incLine();
    fscanf( f, "%d\n", &numPvs ); actWin->incLine();
  }
  else {
    binaryTruthTable = 0;
    numPvs = 1;
  }

  for ( i=0; i<numPvs; i++ ) {
    readStringFromFile( string, 39, f ); actWin->incLine();
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
    readStringFromFile( this->id, 31, f ); actWin->incLine();
  }
  else {
    strcpy( this->id, "" );
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {

    readStringFromFile( pvClassName, 15, f ); actWin->incLine();

    strncpy( pvUserClassName, actWin->pvObj.getNameFromClass( pvClassName ),
     15 );

  }

  saveW = w;
  saveH = h;

  stat = readSymbolFile();
  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( "Cannot read symbol file" );
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
           "Symbol resize underflow - using original size" );
        }
      }
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

      cur->node->activate( pass, (void *) cur );

      cur = cur->flink;

    }

  }

  switch ( pass ) {

  case 1:

    needErase = needDraw = needRefresh = needConnectInit = 0;
    for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) needConnect[i] = 0;
    aglPtr = ptr;
    iValue = 0; /* this get set via OR/AND operations */
    prevIndex = -1;
    init = 0;
    controlExists = 0;
    opComplete = 0;
    active = 0;

    actWin->appCtx->proc->lock();
    activeMode = 1;
    actWin->appCtx->proc->unlock();

    controlV = 1;

    if ( binaryTruthTable )
      notControlPvConnected = (int) pow(2,numPvs) - 1;
    else {
      numPvs = 1;
      notControlPvConnected = 1;
    }

    controlExists = 0;

    for ( i=0; i<numPvs; i++ ) {

   controlEventId[i] = NULL;

      if ( !controlPvExpStr[i].getExpanded() ||
            blank( controlPvExpStr[i].getExpanded() ) ) return 1;

    }

    controlExists = 1;

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


          controlPvId[i] = actWin->pvObj.createNew( pvClassName );
          if ( !controlPvId[i] ) {
            printf( "Cannot create %s object", pvClassName );
            // actWin->appCtx->postMessage( msg );
            opComplete = 1;
            return 1;
	  }
          controlPvId[i]->createEventId( &controlEventId[i] );
        
          stat = controlPvId[i]->searchAndConnect( &controlPvExpStr[i],
           symbol_monitor_control_connect_state, &argRec[i] );
          if ( stat != PV_E_SUCCESS ) {
            printf( "error from searchAndConect\n" );
            return 0;
          }

        }

      }
      else {

        init = 1;
        active = 1;
        index = 1;

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

    actWin->appCtx->proc->lock();

    active = 0;
    activeMode = 0;

    for ( i=0; i<numPvs; i++ ) {
      if ( controlExists ) {
        stat = controlPvId[i]->clearChannel();
        if ( stat != PV_E_SUCCESS )
          printf( "clearChannel failure\n" );

        stat = controlPvId[i]->destroyEventId( &controlEventId[i] );

        delete controlPvId[i];

        controlPvId[i] = NULL;

      }

    }

    actWin->appCtx->proc->unlock();

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
int stat, i, nci, nc[SYMBOL_K_MAX_PVS], nr, ne, nd;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  if ( !activeMode ) {
    actWin->remDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return;
  }

  v = curControlV;
  nci = needConnectInit; needConnectInit = 0;
  for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) {
    nc[i] = needConnect[i];
    needConnect[i] = 0;
  }
  nr = needRefresh; needRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  actWin->remDefExeNode( aglPtr );

  actWin->appCtx->proc->unlock();

//----------------------------------------------------------------------------

  if ( nci ) {

    if ( !notControlPvConnected ) {
      active = 1;
    }

    for ( i=0; i<SYMBOL_K_MAX_PVS; i++ ) {

      if ( nc[i] ) {

        if ( !controlEventId[i]->eventAdded() ) {

          stat = controlPvId[i]->addEvent( controlPvId[i]->pvrDouble(), 1,
           symbol_controlUpdate, (void *) &argRec[i], controlEventId[i], 
	   controlPvId[i]->pveValue() );
          if ( stat != PV_E_SUCCESS ) {
            printf( "addEvent failed\n" );
          }

        }

      }

    }

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

}

int activeSymbolClass::setProperty (
  char *prop,
  int *value )
{

int i, stat;

  if ( strcmp( prop, "value" ) == 0 ) {

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
