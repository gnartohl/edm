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

#ifndef __related_display_h
#define __related_display_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "cadef.h"

#define RDC_MAJOR_VERSION 2
#define RDC_MINOR_VERSION 2
#define RDC_RELEASE 0

typedef struct objAndIndexTag {
  void *obj;
  int index;
} objAndIndexType;

#ifdef __related_display_cc

#include "related_display.str"

static void test_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void relDsp_monitor_dest_connect_state (
  struct connection_handler_args arg );

static void rdc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_update1 (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_apply1 (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_cancel1 (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class relatedDisplayClass : public activeGraphicClass {

private:

friend void test_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void relDsp_monitor_dest_connect_state (
  struct connection_handler_args arg );

friend void openDisplay (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_update1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_apply1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_cancel1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static const int NUMPVS = 4;
static const int maxDsps = 8;

typedef struct bufTag {
  int bufUseFocus;
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufTopShadowColor;
  int bufBotShadowColor;
  int bufFgColor;
  int bufBgColor;
  int bufInvisible;
  int bufCloseAction[maxDsps];
  int bufSetPostion[maxDsps];
  int bufAllowDups[maxDsps];
  int bufCascade[maxDsps];
  int bufPropagateMacros[maxDsps];
  char bufDisplayFileName[maxDsps][127+1];;
  char bufSymbols[maxDsps][255+1];;
  int bufReplaceSymbols[maxDsps];
  char bufButtonLabel[127+1];;
  char bufLabel[maxDsps][127+1];;
  char bufFontTag[63+1];;
  char bufDestPvName[NUMPVS][39+1];
  char bufSource[NUMPVS][39+1];
} bufType, *bufPtr;

int numDsps, dspIndex;

bufPtr buf;

activeWindowClass *aw;
int useFocus, needClose;

int topShadowColor;
int botShadowColor;
pvColorClass fgColor, bgColor;
colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;
int invisible;

int closeAction[maxDsps];
int setPostion[maxDsps];
int allowDups[maxDsps];
int cascade[maxDsps];
int propagateMacros[maxDsps];

expStringClass displayFileName[maxDsps];

expStringClass symbolsExpStr[maxDsps];
char symbols[maxDsps][255+1];

int replaceSymbols[maxDsps]; // else append

expStringClass buttonLabel;

expStringClass label[maxDsps];

fontMenuClass fm;
char fontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

chid destPvId[NUMPVS];

objAndIndexType objAndIndex[NUMPVS];

int opComplete[NUMPVS], destExists[NUMPVS], destConnected[NUMPVS],
 destType[NUMPVS];

expStringClass destPvExpString[NUMPVS];

expStringClass sourceExpString[NUMPVS];

int activeMode;

Widget popUpMenu, pullDownMenu, pb[maxDsps];

entryFormClass *ef1;

int posX, posY;

public:

relatedDisplayClass::relatedDisplayClass ( void );

relatedDisplayClass::relatedDisplayClass
 ( const relatedDisplayClass *source );

relatedDisplayClass::~relatedDisplayClass ( void );

char *relatedDisplayClass::objName ( void ) {

  return name;

}

int relatedDisplayClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int relatedDisplayClass::save (
  FILE *f );

int relatedDisplayClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int relatedDisplayClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int relatedDisplayClass::genericEdit ( void );

int relatedDisplayClass::edit ( void );

int relatedDisplayClass::editCreate ( void );

int relatedDisplayClass::draw ( void );

int relatedDisplayClass::erase ( void );

int relatedDisplayClass::drawActive ( void );

int relatedDisplayClass::eraseActive ( void );

int relatedDisplayClass::activate (
  int pass,
  void *ptr );

int relatedDisplayClass::deactivate ( int pass );

void relatedDisplayClass::updateDimensions ( void );

int relatedDisplayClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int relatedDisplayClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int relatedDisplayClass::containsMacros ( void );


void relatedDisplayClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void relatedDisplayClass::popupDisplay (
  int index );

void relatedDisplayClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

int relatedDisplayClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

void relatedDisplayClass::changeDisplayParams (
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

void relatedDisplayClass::pointerIn (
  int _x,
  int _y,
  int buttonState );

void relatedDisplayClass::pointerOut (
  int _x,
  int _y,
  int buttonState );

 void relatedDisplayClass::executeDeferred ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_relatedDisplayClassPtr ( void );
void *clone_relatedDisplayClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
