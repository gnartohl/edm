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

#ifndef __xygraph_h
#define __xygraph_h 1

#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <math.h>

#include "ulBindings.h"
#include "act_grf.h"
#include "entry_form.h"

#include "cadef.h"

#define XYGC_K_MAX_PLOTS 10

#define XYGC_K_PVNAME 20
#define XYGC_K_LIT 21
#define XYGC_K_PVDESC 22

#define XYGC_K_FORMAT_NATURAL 0
#define XYGC_K_FORMAT_FLOAT 1
#define XYGC_K_FORMAT_EXPONENTIAL 2
#define XYGC_K_FORMAT_DECIMAL 3

#define XYGC_MAJOR_VERSION 2
#define XYGC_MINOR_VERSION 0
#define XYGC_RELEASE 0

#ifdef __xygraph_cc

static void xygo_monitor_plot_connect_state (
  struct connection_handler_args arg );

static void xygo_monitor_control_connect_state (
  struct connection_handler_args arg );

static void plotInfoUpdate (
  struct event_handler_args ast_args );

static void plotUpdate (
  struct event_handler_args ast_args );

static void controlUpdate (
  struct event_handler_args ast_args );

static void axygc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axygc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axygc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axygc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axygc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

typedef struct objPlusIndexTag {
  void *objPtr;
  int index;
  unsigned int setMask;
  unsigned int clrMask;
} objPlusIndexType, *objPlusIndexPtr;

class xyGraphClass : public activeGraphicClass {

private:

friend void xygo_monitor_plot_connect_state (
  struct connection_handler_args arg );

friend void xygo_monitor_control_connect_state (
  struct connection_handler_args arg );

friend void plotInfoUpdate (
  struct event_handler_args ast_args );

friend void plotUpdate (
  struct event_handler_args ast_args );

friend void controlUpdate (
  struct event_handler_args ast_args );

friend void axygc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axygc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axygc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axygc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axygc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

objPlusIndexType argRec[XYGC_K_MAX_PLOTS];

int bufX, bufY, bufW, bufH;

int formatType, bufFormatType;
char format[15+1];
int opComplete, activeMode, notPlotPvConnected[XYGC_K_MAX_PLOTS],
 ctlPvConnected, init, bufInvalid;
fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
int autoScale, bufAutoScale, precision;
efInt efPrecision, bufEfPrecision;
unsigned int bgColor, bufBgColor;
pvColorClass fgColor;
unsigned int bufFgColor;
pvColorClass plotColor[XYGC_K_MAX_PLOTS];
unsigned int bufPlotColor[XYGC_K_MAX_PLOTS];
colorButtonClass plotCb[XYGC_K_MAX_PLOTS], fgCb, bgCb;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

char plotTitle[79+1];
int labelType;
int border;
int xLabelTicks, xMajorTicks, xMinorTicks;
int yLabelTicks, yMajorTicks, yMinorTicks;

char bufPlotTitle[79+1];
int bufLabelType;
int bufBorder;
int bufXLabelTicks, bufXMajorTicks, bufXMinorTicks;
int bufYLabelTicks, bufYMajorTicks, bufYMinorTicks;

VPFUNC activateCallback, deactivateCallback;
int activateCallbackFlag, deactivateCallbackFlag,
 anyCallbackFlag;
int bufActivateCallbackFlag, bufDeactivateCallbackFlag;

int plotPvExists[XYGC_K_MAX_PLOTS], ctlPvExists;

chid pvId[XYGC_K_MAX_PLOTS], ctlPvId;
evid eventId[XYGC_K_MAX_PLOTS], ctlEventId;

expStringClass plotPvExpStr[XYGC_K_MAX_PLOTS], ctlPvExpStr;
char plotPvName[XYGC_K_MAX_PLOTS][39+1], bufPlotPvName[XYGC_K_MAX_PLOTS][39+1];
char ctlPvName[39+1], bufCtlPvName[39+1];

Widget plotWidget;

int needPlotConnect[XYGC_K_MAX_PLOTS], needPlotConnectInit,
 needPlotInfoInit[XYGC_K_MAX_PLOTS], needPlotInfoInit,
 needCtlConnectInit, needErase, needDraw, needRefresh, needUpdate;

public:

xyGraphClass::xyGraphClass ( void );

xyGraphClass::xyGraphClass
 ( const xyGraphClass *source );

xyGraphClass::~xyGraphClass ( void ) {

  if ( name ) delete name;

}

char *xyGraphClass::objName ( void ) {

  return name;

}

int xyGraphClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int xyGraphClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int xyGraphClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int xyGraphClass::save (
  FILE *f );

int xyGraphClass::genericEdit ( void );

int xyGraphClass::edit ( void );

int xyGraphClass::editCreate ( void );

int xyGraphClass::draw ( void );

int xyGraphClass::erase ( void );

int xyGraphClass::drawActive ( void );

int xyGraphClass::eraseActive ( void );

void xyGraphClass::bufInvalidate ( void );

int xyGraphClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int xyGraphClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int xyGraphClass::containsMacros ( void );

int xyGraphClass::activate (
  int pass,
  void *ptr );

int xyGraphClass::deactivate ( int pass );

void xyGraphClass::updateDimensions ( void );

void xyGraphClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

int xyGraphClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag );

void xyGraphClass::executeDeferred ( void );

int xyGraphClass::getProperty (
  char *prop,
  int bufSize,
  char *value );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_xyGraphClassPtr ( void );
void *clone_xyGraphClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
