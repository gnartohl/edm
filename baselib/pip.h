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

#ifndef __pip_h
#define __pip_h 1

#include "act_grf.h"
#include "entry_form.h"

#ifdef __epics__
#include "cadef.h"
#endif

#ifndef __epics__
#define MAX_ENUM_STRING_SIZE 16
#endif

#define PIPC_MAJOR_VERSION 1
#define PIPC_MINOR_VERSION 0
#define PIPC_RELEASE 0

#ifdef __pip_cc

#include "pip.str"

static void pipc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pipc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pipc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pipc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pipc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pip_readUpdate (
  struct event_handler_args ast_args );

static void pip_monitor_read_connect_state (
  struct connection_handler_args arg );

#endif

class activePipClass : public activeGraphicClass {

private:

friend void pipc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pipc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pipc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pipc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pipc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pip_readUpdate (
  struct event_handler_args ast_args );

friend void pip_monitor_read_connect_state (
  struct connection_handler_args arg );

int bufX, bufY, bufW, bufH;

int opComplete;

int minW;
int minH;

Widget *frameWidget;

char readV[39+1], curReadV[39+1];
char curFileName[127+1];
int bufInvalid;

#ifdef __epics__
chid readPvId;
evid readEventId;
#endif

expStringClass readPvExpStr, fileNameExpStr;
char bufReadPvName[activeGraphicClass::MAX_PV_NAME+1];
char bufFileName[127+1];

int readExists, fileExists;

int readPvConnected, init, active, activeMode;

pvColorClass fgColor, bgColor, topShadowColor, botShadowColor;
colorButtonClass fgCb, bgCb, topCb, botCb;
int bufFgColor, bufBgColor, bufTopShadowColor, bufBotShadowColor;

int needConnectInit, needUpdate, needDraw, needFileOpen;

activeWindowClass *aw;

int activateIsComplete;

public:

activePipClass ( void );

activePipClass
 ( const activePipClass *source );

~activePipClass ( void );

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

int activate ( int pass, void *ptr );

int deactivate ( int pass );

int preReactivate (
  int pass );

int reactivate (
  int pass,
  void *ptr );

int expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int containsMacros ( void );

int createPipWidgets ( void );

int checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void executeDeferred ( void );

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

void changePvNames (
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

int isWindowContainer ( void );

int activateComplete ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activePipClassPtr ( void );
void *clone_activePipClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
