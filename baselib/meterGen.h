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

#ifndef __meterGen_h
#define __meterGen_h 1

#include "act_grf.h"
#include "entry_form.h"

// #ifdef __epics__
// #include "cadef.h"
// #endif

#include "pv.h"

#define METERC_K_COLORMODE_STATIC 0
#define METERC_K_COLORMODE_ALARM 1

#define METERC_MAJOR_VERSION 1
#define METERC_MINOR_VERSION 3
#define METERC_RELEASE 0

#define METERC_K_LITERAL 1
#define METERC_K_PV_LABEL 2
#define METERC_K_PV_NAME 3

#define METERC_K_MAX_GE_MIN 1
#define METERC_K_MAX_LT_MIN 2

#ifdef __meterGen_cc

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
  pvClass *classPtr,
  void *clientData,
  void *args );

static void meter_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void meter_alarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void meter_monitor_read_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

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
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void meter_infoUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void meter_readUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void meter_alarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void meter_monitor_control_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void meter_monitor_read_connect_state (
  pvClass *classPtr,
  void *clientData,
  void *args );

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

// #ifdef __epics__
// chid controlPvId, readPvId;
// evid readEventId, alarmEventId;
// #endif

pvClass *controlPvId, *readPvId;
pvEventClass *readEventId, *alarmEventId;

expStringClass controlPvExpStr, readPvExpStr;
char bufControlPvName[39+1];
char bufReadPvName[39+1];

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
unsigned int bufMeterColor, bufFgColor, bufBgColor;
unsigned int bufLabelColor, bufScaleColor;
unsigned int bufTsColor,bufBsColor;
char bufLabel[39+1];
char bufLiteralLabel[39+1],literalLabel[39+1];
int bufLabelType;
double bufMeterAngle, meterAngle;
char scaleFormat[3+1], bufScaleFormat[3+1];
int bufScalePrecision, scalePrecision;
double  bufScaleMin, scaleMin;
double  bufScaleMax, scaleMax;
int bufNeedleType,needleType;
int bufShadowMode,shadowMode;
int bufShowScale;
int bufUseDisplayBg;
char bufScaleFontTag[63+1],bufLabelFontTag[63+1];

int needErase, needDraw, needConnectInit, needRefresh, needInfoInit;

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];
int numPvTypes, pvNameIndex;

public:

activeMeterClass ( void );

activeMeterClass
 ( const activeMeterClass *source );

~activeMeterClass ( void ) {

/*   printf( "In ~activeMeterClass\n" ); */

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

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate ( int pass, void *ptr );

int deactivate ( int pass );

void updateDimensions ( void );

void bufInvalidate ( void );

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
  int meterState,
  int meterNumber );

void btnDown (
  int x,
  int y,
  int meterState,
  int meterNumber );

void btnDrag (
  int x,
  int y,
  int meterState,
  int meterNumber );

int getMeterActionRequest (
  int *up,
  int *down,
  int *drag );

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
