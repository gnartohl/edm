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

#ifndef __related_display_gen_h
#define __related_display_gen_h 1

#include "act_grf.h"
#include "entry_form.h"

// #ifdef __epics__
// #include "cadef.h"
// #endif

#include "pv.h"

#ifndef __epics__
#define MAX_ENUM_STRING_SIZE 16
#endif

#define RDC_MAJOR_VERSION 1
#define RDC_MINOR_VERSION 8
#define RDC_RELEASE 0

typedef struct objAndIndexTag {
  void *obj;
  int index;
} objAndIndexType;

#ifdef __related_display_gen_cc

static void relDsp_monitor_dest_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

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

friend void relDsp_monitor_dest_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void openDisplay (
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

int bufX, bufY, bufW, bufH;

unsigned int topShadowColor, bufTopShadowColor;
unsigned int botShadowColor, bufBotShadowColor;
unsigned int bufFgColor, bufBgColor;
pvColorClass fgColor, bgColor;
colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;
int invisible, bufInvisible;
int closeAction, bufCloseAction;
int setPostion, bufSetPostion;
int allowDups, bufAllowDups;
int cascade, bufCascade;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

char bufDisplayFileName[127+1];
char displayFileName[127+1];

char bufSymbols[255+1];
char symbols[255+1];

int replaceSymbols, bufReplaceSymbols; // else append

char bufLabel[127+1];
char label[127+1];

#define NUMPVS 4

pvClass *destPvId[NUMPVS];

objAndIndexType objAndIndex[NUMPVS];

int opComplete[NUMPVS], destExists[NUMPVS], destConnected[NUMPVS],
 destType[NUMPVS];

expStringClass destPvExpString[NUMPVS];
char bufDestPvName[NUMPVS][39+1];

expStringClass sourceExpString[NUMPVS];
char bufSource[NUMPVS][39+1];

int activeMode;

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];
int numPvTypes, pvNameIndex;

public:

relatedDisplayClass ( void );

relatedDisplayClass
 ( const relatedDisplayClass *source );

~relatedDisplayClass ( void );

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

int importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

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

int deactivate ( int pass );

void updateDimensions ( void );

int expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int containsMacros ( void );

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
