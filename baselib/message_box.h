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

#ifndef __message_box_h
#define __message_box_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "scrolled_text.h"

#ifdef __epics__
#include "cadef.h"
#endif

#ifndef __epics__
#define MAX_ENUM_STRING_SIZE 16
#endif

#define MESSAGEBOXC_MAJOR_VERSION 2
#define MESSAGEBOXC_MINOR_VERSION 1
#define MESSAGEBOXC_RELEASE 0

#ifdef __message_box_cc

#include "message_box.str"

static void messageboxc_flush_log_file (
  XtPointer client,
  XtIntervalId *id );

static void messageboxc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void messageboxc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void messageboxc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void messageboxc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void messageboxc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void messagebox_readUpdate (
  struct event_handler_args ast_args );

static void messagebox_monitor_read_connect_state (
  struct connection_handler_args arg );

#endif

class activeMessageBoxClass : public activeGraphicClass {

private:

friend void messageboxc_flush_log_file (
  XtPointer client,
  XtIntervalId *id );

friend void messageboxc_clear (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void messageboxc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void messageboxc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void messageboxc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void messageboxc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void messageboxc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void messagebox_readUpdate (
  struct event_handler_args ast_args );

friend void messagebox_monitor_read_connect_state (
  struct connection_handler_args arg );

int bufX, bufY, bufW, bufH;

int opComplete;

int minW;
int minH;

Widget frameWidget;

scrolledTextClass scrolledText;

char readV[39+1], curReadV[39+1];
char logFileName[127+1];
FILE *logFile;
int logFileOpen;
int fileSize;
int curFileSize;
int bufInvalid;

fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;

#ifdef __epics__
chid readPvId;
evid readEventId;
#endif

expStringClass readPvExpStr;
char bufReadPvName[39+1];
char bufLogFileName[127+1];
int bufFileSize;

int fileIsReadOnly, bufFileIsReadOnly;

int readExists, logFileExists;

int readPvConnected, firstReadUpdate, init, active, activeMode;

pvColorClass fgColor, bgColor, bg2Color, topShadowColor, botShadowColor;
colorButtonClass fgCb, bgCb, bg2Cb, topCb, botCb;
int size, bufSize;
int bufFgColor, bufBgColor, bufBg2Color, bufTopShadowColor,
 bufBotShadowColor;
char bufFontTag[63+1];

XtIntervalId flushTimer;
int flushTimerValue, bufFlushTimerValue;

int needConnectInit, needUpdate, needDraw;

public:

activeMessageBoxClass::activeMessageBoxClass ( void );

activeMessageBoxClass::activeMessageBoxClass
 ( const activeMessageBoxClass *source );

activeMessageBoxClass::~activeMessageBoxClass ( void ) {

  if ( name ) delete name;

}

char *activeMessageBoxClass::objName ( void ) {

  return name;

}

int activeMessageBoxClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeMessageBoxClass::save (
  FILE *f );

int activeMessageBoxClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeMessageBoxClass::genericEdit ( void );

int activeMessageBoxClass::edit ( void );

int activeMessageBoxClass::editCreate ( void );

int activeMessageBoxClass::draw ( void );

int activeMessageBoxClass::erase ( void );

int activeMessageBoxClass::drawActive ( void );

int activeMessageBoxClass::eraseActive ( void );

int activeMessageBoxClass::activate ( int pass, void *ptr );

int activeMessageBoxClass::deactivate ( int pass );

int activeMessageBoxClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMessageBoxClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMessageBoxClass::containsMacros ( void );

int activeMessageBoxClass::createWidgets ( void );

int activeMessageBoxClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeMessageBoxClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int activeMessageBoxClass::rotateLogFile ( void );

void activeMessageBoxClass::executeDeferred ( void );

void activeMessageBoxClass::changeDisplayParams (
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

void activeMessageBoxClass::changePvNames (
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

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMessageBoxClassPtr ( void );
void *clone_activeMessageBoxClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
