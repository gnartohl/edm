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

#ifndef __meter_h
#define __meter_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "cadef.h"

#define METERC_K_COLORMODE_STATIC 0
#define METERC_K_COLORMODE_ALARM 1

#define METERC_MAJOR_VERSION 2
#define METERC_MINOR_VERSION 0
#define METERC_RELEASE 0

#define METERC_K_LITERAL 1
#define METERC_K_PV_LABEL 2
#define METERC_K_PV_NAME 3

#define METERC_K_MAX_GE_MIN 1
#define METERC_K_MAX_LT_MIN 2

#ifdef __meter_cc

#include "meter.str"

static char *dragName[] = {
  activeMeterClass_str1
};

static void meterc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void meterc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void meterc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void meterc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void meterc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void meter_infoUpdate (
  struct event_handler_args ast_args );

static void meter_readUpdate (
  struct event_handler_args ast_args );

static void meter_alarmUpdate (
  struct event_handler_args ast_args );

static void meter_monitor_read_connect_state (
  struct connection_handler_args arg );

#endif

class activeMeterClass : public activeGraphicClass {

private:

friend void meterc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void meterc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void meterc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void meterc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void meterc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void meter_controlUpdate (
  struct event_handler_args ast_args );

friend void meter_infoUpdate (
  struct event_handler_args ast_args );

friend void meter_readUpdate (
  struct event_handler_args ast_args );

friend void meter_alarmUpdate (
  struct event_handler_args ast_args );

friend void meter_monitor_control_connect_state (
  struct connection_handler_args arg );

friend void meter_monitor_read_connect_state (
  struct connection_handler_args arg );

int bufX, bufY, bufW, bufH;

int bufScaleLimitsFromDb, scaleLimitsFromDb;

int opComplete;

int minW;
int minH;

double controlV, curControlV, readV, curReadV, readMin, readMax;
int meterW, oldMeterW, bufInvalid, meterX, oldMeterX, originW, mode;
double meterOriginX;

int meterNeedleXorigin, meterNeedleYorigin;
int meterNeedleXend, meterNeedleYend;
int oldMeterNeedleXOrigin, oldMeterNeedleYOrigin;
int oldMeterNeedleXEnd, oldMeterNeedleYEnd;

fontMenuClass scaleFm, labelFm;
char scaleFontTag[63+1], labelFontTag[63+1];
XFontStruct *scaleFs, *labelFs;
int scaleFontAscent, scaleFontDescent, scaleFontHeight;
int labelFontAscent, labelFontDescent, labelFontHeight;

chid controlPvId, readPvId;
evid readEventId, alarmEventId;

expStringClass controlPvExpStr, readPvExpStr;
char bufControlPvName[activeGraphicClass::MAX_PV_NAME+1];
char bufReadPvName[activeGraphicClass::MAX_PV_NAME+1];

int controlExists, readExists;

int controlPvConnected, readPvConnected, active, activeMode, activeInitFlag;

int meterColorMode, fgColorMode, scaleColorMode;
pvColorClass meterColor, fgColor, bgColor;
pvColorClass tsColor, bsColor, labelColor, scaleColor;
colorButtonClass meterCb, fgCb, bgCb, tsCb, bsCb, labelCb, scaleCb;
char label[39+1];
int labelType;
int drawStaticFlag;
int showScale;
int useDisplayBg;

int bufMeterColorMode, bufFgColorMode, bufScaleColorMode;
int bufMajorIntervals, bufMinorIntervals;
int majorIntervals, minorIntervals;
int bufMeterColor, bufFgColor, bufBgColor;
int bufLabelColor, bufScaleColor;
int bufTsColor,bufBsColor;
char bufLabel[39+1];
char bufLiteralLabel[39+1],literalLabel[39+1];
int bufLabelType;
double bufMeterAngle, meterAngle;
char scaleFormat[15+1], bufScaleFormat[15+1];
int bufScalePrecision, scalePrecision;
double  bufScaleMin, scaleMin;
double  bufScaleMax, scaleMax;
int bufNeedleType,needleType;
int bufShadowMode,shadowMode;
int bufShowScale;
int bufUseDisplayBg;
char bufScaleFontTag[63+1],bufLabelFontTag[63+1];

int needErase, needDraw, needConnectInit, needRefresh, needInfoInit;

public:

activeMeterClass::activeMeterClass ( void );

activeMeterClass::activeMeterClass
 ( const activeMeterClass *source );

activeMeterClass::~activeMeterClass ( void ) {

  if ( name ) delete name;

}

char *activeMeterClass::objName ( void ) {

  return name;

}

int activeMeterClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeMeterClass::save (
  FILE *f );

int activeMeterClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeMeterClass::genericEdit ( void );

int activeMeterClass::edit ( void );

int activeMeterClass::editCreate ( void );

int activeMeterClass::draw ( void );

int activeMeterClass::erase ( void );

int activeMeterClass::drawActive ( void );

int activeMeterClass::eraseActive ( void );

int activeMeterClass::activate ( int pass, void *ptr );

int activeMeterClass::deactivate ( int pass );

void activeMeterClass::updateDimensions ( void );

void activeMeterClass::bufInvalidate ( void );

int activeMeterClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMeterClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeMeterClass::containsMacros ( void );

void activeMeterClass::btnUp (
  int x,
  int y,
  int meterState,
  int meterNumber );

void activeMeterClass::btnDown (
  int x,
  int y,
  int meterState,
  int meterNumber );

void activeMeterClass::btnDrag (
  int x,
  int y,
  int meterState,
  int meterNumber );

int activeMeterClass::getMeterActionRequest (
  int *up,
  int *down,
  int *drag );

int activeMeterClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int activeMeterClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

void activeMeterClass::executeDeferred ( void );

char *activeMeterClass::firstDragName ( void );

char *activeMeterClass::nextDragName ( void );

char *activeMeterClass::dragValue (
  int i );

void activeMeterClass::changeDisplayParams (
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

void activeMeterClass::changePvNames (
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

void *create_activeMeterClassPtr ( void );
void *clone_activeMeterClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
