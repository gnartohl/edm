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

#ifndef __exit_button_h
#define __exit_button_h 1

#include "act_grf.h"
#include "entry_form.h"

#define EBTC_MAJOR_VERSION 2
#define EBTC_MINOR_VERSION 0
#define EBTC_RELEASE 0

#ifdef __exit_button_cc

#include "exit_button.str"

static void ebtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ebtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ebtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ebtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ebtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class activeExitButtonClass : public activeGraphicClass {

private:

friend void ebtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ebtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ebtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ebtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ebtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

int opComplete;

int bufX, bufY, bufW, bufH;

int fgColor, bufFgColor, bgColor, bufBgColor,
 topShadowColor, bufTopShadowColor, botShadowColor, bufBotShadowColor;
colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;
int _3D, buf3D, invisible, bufInvisible, activeMode, iconify, bufIconify,
 exitProgram, bufExitProgram;

char label[31+1], bufLabel[31+1];

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

public:

activeExitButtonClass::activeExitButtonClass ( void );

activeExitButtonClass::activeExitButtonClass
 ( const activeExitButtonClass *source );

activeExitButtonClass::~activeExitButtonClass ( void ) {

  if ( name ) delete name;

}

char *activeExitButtonClass::objName ( void ) {

  return name;

}

int activeExitButtonClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeExitButtonClass::save (
  FILE *f );

int activeExitButtonClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeExitButtonClass::genericEdit ( void );

int activeExitButtonClass::edit ( void );

int activeExitButtonClass::editCreate ( void );

int activeExitButtonClass::draw ( void );

int activeExitButtonClass::erase ( void );

int activeExitButtonClass::drawActive ( void );

int activeExitButtonClass::eraseActive ( void );

int activeExitButtonClass::activate (
  int pass,
  void *ptr );

int activeExitButtonClass::deactivate ( void );

void activeExitButtonClass::updateDimensions ( void );

void activeExitButtonClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void activeExitButtonClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

int activeExitButtonClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

void activeExitButtonClass::changeDisplayParams (
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

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeExitButtonClassPtr ( void );
void *clone_activeExitButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
