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

#ifndef __x_text_gen_h
#define __x_text_gen_h 1

#include "act_grf.h"
#include "entry_form.h"

// #ifdef __epics__
// #include "cadef.h"
// #endif

#include "pv.h"

#define AXTC_K_COLORMODE_STATIC 0
#define AXTC_K_COLORMODE_ALARM 1

#define AXTC_MAJOR_VERSION 1
#define AXTC_MINOR_VERSION 5
#define AXTC_RELEASE 0

#ifdef __x_text_gen_cc

static void axtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtoMonitorAlarmPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void xTextAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void axtoMonitorVisPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void xTextVisUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

#endif

class activeXTextClass : public activeGraphicClass {

private:

friend void axtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtoMonitorAlarmPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void xTextAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void axtoMonitorVisPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void xTextVisUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

int bufX, bufY, bufW, bufH;

pvColorClass fgColor;
unsigned int bufFgColor;
colorButtonClass fgCb;

int fgColorMode;
int bufFgColorMode;

pvColorClass bgColor;
unsigned int bufBgColor;
colorButtonClass bgCb;

int bgColorMode;
int bufBgColorMode;

int pvType;
pvValType pvValue, minVis, maxVis;
char minVisString[39+1], bufMinVisString[39+1];
char maxVisString[39+1], bufMaxVisString[39+1];

int prevVisibility, visibility, visInverted, bufVisInverted;

// #ifdef __epics__
// chid alarmPvId;
// evid alarmEventId;
// chid visPvId;
// evid visEventId;
// #endif

pvClass *visPvId, *alarmPvId;
pvEventClass *alarmEventId, *visEventId;

expStringClass alarmPvExpStr;
char bufAlarmPvName[127+1];

expStringClass visPvExpStr;
char bufVisPvName[127+1];

int alarmPvExists, alarmPvConnected, visPvExists, visPvConnected;
int active, activeMode, init, opComplete;

expStringClass value;
char bufValue[255+1];

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
int useDisplayBg, bufUseDisplayBg, alignment;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight, stringLength, stringWidth,
 stringY, stringX;
int autoSize, bufAutoSize;

int needVisConnectInit;
int needAlarmConnectInit;
int needDraw, needErase, needRefresh, needPropertyUpdate;

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];

int numPvTypes, pvNameIndex;

public:

activeXTextClass ( void );

activeXTextClass
 ( const activeXTextClass *source );

~activeXTextClass ( void ) {

/*   printf( "In ~activeXTextClass\n" ); */

  if ( name ) delete name;

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

int importFromXchFile (
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

int eraseUnconditional ( void );

int expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int containsMacros ( void );

int activate (
  int pass,
  void *ptr );

int deactivate (
  int pass );

void updateDimensions ( void );

void executeDeferred ( void );

int setProperty (
  char *prop,
  char *value );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeXTextClassPtr ( void );
void *clone_activeXTextClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
