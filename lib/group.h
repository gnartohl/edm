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

#ifndef __group_h
#define __group_h 1

#include "act_grf.h"
#include "undo.h"
#include "entry_form.h"

#include "group.str"

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

#ifdef __epics__
#include "cadef.h"
#endif

class activeGroupClass : public activeGraphicClass {

private:

void * voidHead; // cast to activeGraphicListPtr at runtime
btnActionListPtr btnDownActionHead;
btnActionListPtr btnUpActionHead;
btnActionListPtr btnMotionActionHead;
btnActionListPtr btnFocusActionHead;
int depth;
undoClass undoObj;

public:

activeGroupClass::activeGroupClass ( void );

activeGroupClass::activeGroupClass
 ( const activeGroupClass *source );

activeGroupClass::~activeGroupClass ( void );

int activeGroupClass::createGroup (
  activeWindowClass *aw_obj );

int activeGroupClass::ungroup (
  void *curListNode );
 
int activeGroupClass::save (
  FILE *f );

int activeGroupClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeGroupClass::edit ( void );

void activeGroupClass::beginEdit ( void );

int activeGroupClass::checkEditStatus ( void );

int activeGroupClass::draw ( void );

int activeGroupClass::erase ( void );

int activeGroupClass::drawActive ( void );

int activeGroupClass::eraseActive ( void );

int activeGroupClass::activate (
  int pass,
  void *ptr );

int activeGroupClass::deactivate (
  int pass );

int activeGroupClass::preReactivate (
  int pass
);

int activeGroupClass::moveSelectBox (
  int _x,
  int _y );

int activeGroupClass::moveSelectBoxAbs (
  int _x,
  int _y );

int activeGroupClass::moveSelectBoxMidpointAbs (
  int _x,
  int _y );

int activeGroupClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGroupClass::resizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGroupClass::resizeSelectBoxAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGroupClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGroupClass::resizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGroupClass::move (
  int x,
  int y );

int activeGroupClass::moveAbs (
  int x,
  int y );

int activeGroupClass::moveMidpointAbs (
  int x,
  int y );

int activeGroupClass::rotate (
  int xOrigin,
  int yOrigin,
  char direction );

int activeGroupClass::flip (
  int xOrigin,
  int yOrigin,
  char direction );

int activeGroupClass::resize (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGroupClass::resizeAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGroupClass::resizeAbsFromUndo (
  int _x,
  int _y,
  int _w,
  int _h );

int activeGroupClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag );

int activeGroupClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

void activeGroupClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void activeGroupClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void activeGroupClass::btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void activeGroupClass::pointerIn (
  int x,
  int y,
  int buttonState );

void activeGroupClass::pointerOut (
  int x,
  int y,
  int buttonState );

activeGraphicClass *activeGroupClass::getTail ( void );

void activeGroupClass::updateGroup ( void );

int activeGroupClass::initDefExeNode (
  void *ptr );

int activeGroupClass::containsMacros ( void );

int activeGroupClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeGroupClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

void activeGroupClass::bufInvalidate ( void );

void activeGroupClass::setNextSelectedToEdit (
  activeGraphicClass *ptr );

void activeGroupClass::clearNextSelectedToEdit ( void );

void activeGroupClass::changeDisplayParams (
  unsigned int flag,
  char *fontTag,
  int alignment,
  char *ctlFontTag,
  int ctlAlignment,
  char *btnFontTag,
  int btnAlignment,
  int textFgColor,
  int fg1Color,
  int fg2Color,
  int offsetColor,
  int bgColor,
  int topShadowColor,
  int botShadowColor );

void activeGroupClass::changePvNames (
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
  char *alarmPvs[] );

void activeGroupClass::flushUndo ( void );

int activeGroupClass::addUndoCreateNode ( undoClass *_undoObj );

int activeGroupClass::addUndoMoveNode ( undoClass *_undoObj );

int activeGroupClass::addUndoResizeNode ( undoClass *_undoObj );

int activeGroupClass::addUndoCopyNode ( undoClass *_undoObj );

int activeGroupClass::addUndoCutNode ( undoClass *_undoObj );

int activeGroupClass::addUndoPasteNode ( undoClass *_undoObj );

int activeGroupClass::addUndoReorderNode ( undoClass *_undoObj );

int activeGroupClass::addUndoEditNode ( undoClass *_undoObj );

int activeGroupClass::addUndoGroupNode ( undoClass *_undoObj );

int activeGroupClass::addUndoRotateNode ( undoClass *_undoObj );

int activeGroupClass::addUndoFlipNode ( undoClass *_undoObj );

int activeGroupClass::undoCreate (
  undoOpClass *opPtr );

int activeGroupClass::undoMove (
  undoOpClass *opPtr,
  int x,
  int y );

int activeGroupClass::undoResize (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int activeGroupClass::undoCopy (
  undoOpClass *opPtr );

int activeGroupClass::undoCut (
  undoOpClass *opPtr );

int activeGroupClass::undoPaste (
  undoOpClass *opPtr );

int activeGroupClass::undoReorder (
  undoOpClass *opPtr );

int activeGroupClass::undoEdit (
  undoOpClass *opPtr );

int activeGroupClass::undoGroup (
  undoOpClass *opPtr );

int activeGroupClass::undoRotate (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int activeGroupClass::undoFlip (
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

};

#endif
