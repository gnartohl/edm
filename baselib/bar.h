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

#ifndef __bar_h
#define __bar_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "cadef.h"

#define BARC_K_COLORMODE_STATIC 0
#define BARC_K_COLORMODE_ALARM 1

#define BARC_MAJOR_VERSION 2
#define BARC_MINOR_VERSION 1
#define BARC_RELEASE 0

#define BARC_K_PV_NAME 0
#define BARC_K_LITERAL 1

#define BARC_K_MAX_GE_MIN 1
#define BARC_K_MAX_LT_MIN 2

#define BARC_K_LIMITS_FROM_DB 1
#define BARC_K_LIMITS_FROM_FORM 0

#ifdef __bar_cc

#include "bar.str"

static char *dragName[] = {
  activeBarClass_str1,
  activeBarClass_str2,
};

static void barc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void barc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#if 0
static void bar_controlUpdate (
  struct event_handler_args ast_args );
#endif

static void bar_infoUpdate (
  struct event_handler_args ast_args );

static void bar_readUpdate (
  struct event_handler_args ast_args );

static void bar_nullUpdate (
  struct event_handler_args ast_args );

static void bar_alarmUpdate (
  struct event_handler_args ast_args );

#if 0
static void bar_monitor_control_connect_state (
  struct connection_handler_args arg );
#endif

static void bar_monitor_read_connect_state (
  struct connection_handler_args arg );

static void bar_monitor_null_connect_state (
  struct connection_handler_args arg );

#endif

class activeBarClass : public activeGraphicClass {

private:

friend void barc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void barc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void bar_controlUpdate (
  struct event_handler_args ast_args );

friend void bar_infoUpdate (
  struct event_handler_args ast_args );

friend void bar_readUpdate (
  struct event_handler_args ast_args );

friend void bar_nullUpdate (
  struct event_handler_args ast_args );

friend void bar_alarmUpdate (
  struct event_handler_args ast_args );

friend void bar_monitor_control_connect_state (
  struct connection_handler_args arg );

friend void bar_monitor_read_connect_state (
  struct connection_handler_args arg );

friend void bar_monitor_null_connect_state (
  struct connection_handler_args arg );

int horizontal, bufHorizontal;

int bufX, bufY, bufW, bufH;

int opComplete;

int minW, minVertW;
int minH, minVertH;

double controlV, curControlV, readV, curReadV, curNullV;
int barY, oldBarY, barH, oldBarH, barW, oldBarW, bufInvalid, barX, oldBarX,
 originW, originH, mode, barAreaX, barAreaW, barAreaY, barAreaH, barStrLen, barMaxW, barMaxH,
 aboveBarOrigin, oldAboveBarOrigin, zeroCrossover;
double barOriginX, barOriginY, factorLt, factorGe;
efDouble efBarOriginX;

fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

chid controlPvId, readPvId, nullPvId;
evid controlEventId, readEventId, nullEventId, alarmEventId;

expStringClass controlPvExpStr, readPvExpStr, nullPvExpStr;
char bufControlPvName[activeGraphicClass::MAX_PV_NAME+1];
char bufReadPvName[activeGraphicClass::MAX_PV_NAME+1];
char bufNullPvName[activeGraphicClass::MAX_PV_NAME+1];

unsigned char pvNotConnectedMask;

int controlExists, readExists, nullExists;

int init, active, activeMode;

int barColorMode, fgColorMode;
pvColorClass barColor, fgColor, bgColor;
colorButtonClass barCb, fgCb, bgCb;
char label[39+1];
int labelType;
int border;
int showScale;
int labelTicks, majorTicks, minorTicks;
char scaleFormat[15+1];
int limitsFromDb;
double readMin, readMax;
efDouble efReadMin, efReadMax;
int precision;
efInt efPrecision;

int bufBarColorMode, bufFgColorMode;
int bufBarColor, bufFgColor, bufBgColor;
char bufLabel[39+1];
int bufLabelType;
int bufBorder;
int bufShowScale;
int bufLabelTicks, bufMajorTicks, bufMinorTicks;
char bufFontTag[63+1];
char bufScaleFormat[15+1];
int bufLimitsFromDb;
efDouble bufEfReadMin, bufEfReadMax;
efInt bufEfPrecision;
efDouble bufEfBarOriginX;

int needErase, needDraw, needFullDraw, needDrawCheck, needConnectInit,
 needRefresh, needInfoInit;

public:

activeBarClass::activeBarClass ( void );

activeBarClass::activeBarClass
 ( const activeBarClass *source );

activeBarClass::~activeBarClass ( void );

char *activeBarClass::objName ( void ) {

  return name;

}

int activeBarClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeBarClass::save (
  FILE *f );

int activeBarClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeBarClass::genericEdit ( void );

int activeBarClass::edit ( void );

int activeBarClass::editCreate ( void );

int activeBarClass::draw ( void );

int activeBarClass::erase ( void );

int activeBarClass::drawActive ( void );

int activeBarClass::eraseActive ( void );

int activeBarClass::activate ( int pass, void *ptr );

int activeBarClass::deactivate ( int pass );

void activeBarClass::updateDimensions ( void );

void activeBarClass::bufInvalidate ( void );

int activeBarClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeBarClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeBarClass::containsMacros ( void );

void activeBarClass::btnUp (
  int x,
  int y,
  int barState,
  int barNumber );

void activeBarClass::btnDown (
  int x,
  int y,
  int barState,
  int barNumber );

void activeBarClass::btnDrag (
  int x,
  int y,
  int barState,
  int barNumber );

int activeBarClass::getBarActionRequest (
  int *up,
  int *down,
  int *drag );

int activeBarClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeBarClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void activeBarClass::drawScale (
  Widget widget,
  gcClass *gc );

void activeBarClass::drawHorzScale (
  Widget widget,
  gcClass *gc );

void activeBarClass::drawVertScale (
  Widget widget,
  gcClass *gc );

void activeBarClass::updateScaleInfo ( void );

void activeBarClass::updateHorzScaleInfo ( void );

void activeBarClass::updateVertScaleInfo ( void );

void activeBarClass::updateBar ( void );

void activeBarClass::executeDeferred ( void );

char *activeBarClass::firstDragName ( void );

char *activeBarClass::nextDragName ( void );

char *activeBarClass::dragValue (
  int i );

void activeBarClass::changeDisplayParams (
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

void activeBarClass::changePvNames (
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

void *create_activeBarClassPtr ( void );
void *clone_activeBarClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
