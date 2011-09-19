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

#ifndef __freeze_button_h
#define __freeze_button_h 1

#include "act_grf.h"
#include "entry_form.h"

#define FBTC_MAJOR_VERSION 4
#define FBTC_MINOR_VERSION 0
#define FBTC_RELEASE 0

#ifdef __freeze_button_cc

#include "freeze_button.str"

static void doBlink(void* ptr);

static void fbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void fbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void fbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void fbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void fbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class activeFreezeButtonClass : public activeGraphicClass {

private:

friend void doBlink (void* ptr);
    
friend void fbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void fbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void fbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void fbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void fbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

int opComplete;

int bufX, bufY, bufW, bufH;

int _3D, buf3D, fgColor, bufFgColor, bgColor, bufBgColor, frozenBgColor,bufFrozenBgColor,
 topShadowColor, bufTopShadowColor, botShadowColor, bufBotShadowColor;
colorButtonClass fgCb, bgCb, frozenBgCb, topShadowCb, botShadowCb;
int activeMode;

char label[31+1], bufLabel[31+1], frozenLabel[31+1], bufFrozenLabel[31+1];

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

public:

activeFreezeButtonClass ( void );

activeFreezeButtonClass
 ( const activeFreezeButtonClass *source );

~activeFreezeButtonClass ( void ) {

  if ( name ) delete[] name;
  updateBlink( 0 );

}

char *objName ( void ) {

  return name;

}

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int save (
  FILE *f );

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate (
  int pass,
  void *ptr );

int deactivate ( void );

void updateDimensions ( void );

void btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

void changeDisplayParams (
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

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeFreezeButtonClassPtr ( void );
void *clone_activeFreezeButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
