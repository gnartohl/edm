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
#include "utility.h"
#include "keypad.h"
#include "msg_dialog.h"

#include "app_pkg.h"
#include "act_win.h"

#include "pv_factory.h"
#include "epics_pv_factory.h"
#include "cvtFast.h"

#define XYGC_K_MAX_TRACES 10

// scaling options
#define XYGC_K_FROM_PV 0
#define XYGC_K_USER_SPECIFIED 1
#define XYGC_K_AUTOSCALE 2

#define XYGC_K_SYMBOL_TYPE_NONE 0
#define XYGC_K_SYMBOL_TYPE_CIRCLE 1
#define XYGC_K_SYMBOL_TYPE_SQUARE 2
#define XYGC_K_SYMBOL_TYPE_DIAMOND 3

#define XYGC_K_UPDATE_ON_X_AND_Y 0
#define XYGC_K_UPDATE_ON_X_OR_Y 1
#define XYGC_K_UPDATE_ON_X 2
#define XYGC_K_UPDATE_ON_Y 3

#define XYGC_K_PLOT_STYLE_LINE 0
#define XYGC_K_PLOT_STYLE_POINT 1
#define XYGC_K_PLOT_STYLE_NEEDLE 2

#define XYGC_K_TRACE_INVALID 0
#define XYGC_K_TRACE_XY 1
#define XYGC_K_TRACE_CHRONOLOGICAL 2

#define XYGC_K_STATE_UNKNOWN 0
#define XYGC_K_STATE_INITIALIZING 1
#define XYGC_K_STATE_GOT_1ST_POINT 2
#define XYGC_K_STATE_GOT_2ND_POINT 3
#define XYGC_K_STATE_PLOTTING 4
#define XYGC_K_STATE_PAUSED 5

#define XYGC_K_PLOT_MODE_PLOT_N_STOP 0
#define XYGC_K_PLOT_MODE_PLOT_LAST_N 1

#define XYGC_K_RESET_MODE_IF_NOT_ZERO 0
#define XYGC_K_RESET_MODE_IF_ZERO 1

#define XYGC_K_AXIS_STYLE_LINEAR 0
#define XYGC_K_AXIS_STYLE_LOG10 1
#define XYGC_K_AXIS_STYLE_TIME 2
#define XYGC_K_AXIS_STYLE_TIME_LOG10 3
#define XYGC_K_AXIS_STYLE_STRIPCHART 4

#define XYGC_K_PVNAME 20
#define XYGC_K_LIT 21
#define XYGC_K_PVDESC 22

#define XYGC_K_FORMAT_FFLOAT 1
#define XYGC_K_FORMAT_GFLOAT 2
#define XYGC_K_FORMAT_EXPONENTIAL 3

#define XYGC_K_SCOPE_MODE 0
#define XYGC_K_PLOT_SORTED_X_MODE 1

#define XYGC_MAJOR_VERSION 1
#define XYGC_MINOR_VERSION 0
#define XYGC_RELEASE 0

#ifdef __xygraph_cc

static void updateTimerAction (
  XtPointer client,
  XtIntervalId *id );

static void setKpXMinDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void setKpXMaxDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void cancelKpXMin (
  Widget w,
  XtPointer client,
  XtPointer call );

static void cancelKpXMax (
  Widget w,
  XtPointer client,
  XtPointer call );

static void setKpY1MinDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void setKpY1MaxDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void cancelKpY1Min (
  Widget w,
  XtPointer client,
  XtPointer call );

static void cancelKpY1Max (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xMonitorConnection (
  struct connection_handler_args arg );

static void yMonitorConnection (
  struct connection_handler_args arg );

static void xInfoUpdate (
  struct event_handler_args arg );

static void yInfoUpdate (
  struct event_handler_args arg );

static void xValueUpdate (
  struct event_handler_args arg );

static void yValueUpdate (
  struct event_handler_args arg );

static void yValueWithTimeUpdate (
  struct event_handler_args arg );

static void resetMonitorConnection (
  struct connection_handler_args arg );

static void resetValueUpdate (
  struct event_handler_args arg );

static void axygc_edit_ok_trace (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axygc_edit_ok_axis (
  Widget w,
  XtPointer client,
  XtPointer call );

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

class xyGraphClass : public activeGraphicClass {

private:

typedef struct editBufTag {
// edit buffer
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  char bufGraphTitle[127+1];
  char bufXLabel[127+1];
  char bufYLabel[127+1];
  int bufPlotMode;
  int bufResetMode;
  int bufPlotStyle[XYGC_K_MAX_TRACES];
  int bufPlotSymbolType[XYGC_K_MAX_TRACES];
  int bufPlotUpdateMode[XYGC_K_MAX_TRACES];
  int bufPlotColor[XYGC_K_MAX_TRACES];
  char bufXPvName[XYGC_K_MAX_TRACES][activeGraphicClass::MAX_PV_NAME+1];
  char bufYPvName[XYGC_K_MAX_TRACES][activeGraphicClass::MAX_PV_NAME+1];
  int bufLineThk[XYGC_K_MAX_TRACES];
  int bufLineStyle[XYGC_K_MAX_TRACES];
  char bufTrigPvName[activeGraphicClass::MAX_PV_NAME+1];
  char bufResetPvName[activeGraphicClass::MAX_PV_NAME+1];
  int bufOpMode[XYGC_K_MAX_TRACES];
  int bufY2Scale[XYGC_K_MAX_TRACES];
  int bufCount;
  int bufXAxis;
  int bufXAxisStyle;
  int bufXAxisSource;
  int bufXAxisTimeFormat;
  efDouble bufXMin;
  efDouble bufXMax;

  efInt bufXNumLabelIntervals;
  int bufXLabelGrid;
  efInt bufXNumMajorPerLabel;
  int bufXMajorGrid;
  efInt bufXNumMinorPerMajor;
  int bufXMinorGrid;
  efInt bufXAnnotationPrecision;
  int bufXAnnotationFormat;

  int bufY1Axis;
  int bufY1AxisStyle;
  int bufY1AxisSource;
  efDouble bufY1Min;
  efDouble bufY1Max;

  efInt bufY1NumLabelIntervals;
  int bufY1LabelGrid;
  efInt bufY1NumMajorPerLabel;
  int bufY1MajorGrid;
  efInt bufY1NumMinorPerMajor;
  int bufY1MinorGrid;
  efInt bufY1AnnotationPrecision;
  int bufY1AnnotationFormat;

  int bufY2Axis;
  int bufY2AxisStyle;
  int bufY2AxisSource;
  efDouble bufY2Min;
  efDouble bufY2Max;

  efInt bufY2NumLabelIntervals;
  int bufY2LabelGrid;
  efInt bufY2NumMajorPerLabel;
  int bufY2MajorGrid;
  efInt bufY2NumMinorPerMajor;
  int bufY2MinorGrid;
  efInt bufY2AnnotationPrecision;
  int bufY2AnnotationFormat;

  int bufFgColor;
  int bufBgColor;
  int bufGridColor;
  int bufFormatType;
  int bufBorder;
  int bufXFormatType;
  efInt bufXPrecision;
  int bufY1FormatType;
  efInt bufY1Precision;
  int bufY2FormatType;
  efInt bufY2Precision;

  int bufUpdateTimerValue;

} editBufType, *editBufPtr;

typedef struct objPlusIndexTag {
  void *objPtr;
  int index;
} objPlusIndexType, *objPlusIndexPtr;

friend void updateTimerAction (
  XtPointer client,
  XtIntervalId *id );

friend void setKpXMinDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void setKpXMaxDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void cancelKpXMin (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void cancelKpXMax (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void setKpY1MinDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void setKpY1MaxDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void cancelKpY1Min (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void cancelKpY1Max (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xMonitorConnection (
  struct connection_handler_args arg );

friend void yMonitorConnection (
  struct connection_handler_args arg );

friend void xInfoUpdate (
  struct event_handler_args arg );

friend void yInfoUpdate (
  struct event_handler_args arg );

friend void xValueUpdate (
  struct event_handler_args arg );

friend void yValueUpdate (
  struct event_handler_args arg );

friend void yValueWithTimeUpdate (
  struct event_handler_args arg );

friend void resetMonitorConnection (
  struct connection_handler_args arg );

friend void resetValueUpdate (
  struct event_handler_args arg );

friend void axygc_edit_ok_trace (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axygc_edit_ok_axis (
  Widget w,
  XtPointer client,
  XtPointer call );

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

int bufX, bufY, bufW, bufH;

Pixmap pixmap;

pvConnectionClass connection;

keypadClass kp;

double kpXMin, kpXMax;
efDouble kpXMinEfDouble, kpXMaxEfDouble;
int kpCancelMinX, kpCancelMaxX;
int xMinX0, xMinX1, xMinY0, xMinY1;
int xMaxX0, xMaxX1, xMaxY0, xMaxY1;

double kpY1Min, kpY1Max;
efDouble kpY1MinEfDouble, kpY1MaxEfDouble;
int kpCancelMinY1, kpCancelMaxY1;
int y1MinX0, y1MinX1, y1MinY0, y1MinY1;
int y1MaxX0, y1MaxX1, y1MaxY0, y1MaxY1;

expStringClass graphTitle, xLabel, yLabel;

int numTraces;
int plotAreaX, plotAreaY, plotAreaW, plotAreaH;

objPlusIndexType argRec[XYGC_K_MAX_TRACES];

//ProcessVariable *xPv[XYGC_K_MAX_TRACES], *yPv[XYGC_K_MAX_TRACES];
chid xPv[XYGC_K_MAX_TRACES], yPv[XYGC_K_MAX_TRACES];
evid xEv[XYGC_K_MAX_TRACES], yEv[XYGC_K_MAX_TRACES];

int plotColor[XYGC_K_MAX_TRACES];
int lineThk[XYGC_K_MAX_TRACES];
int lineStyle[XYGC_K_MAX_TRACES];
int plotSymbolType[XYGC_K_MAX_TRACES];
int plotUpdateMode[XYGC_K_MAX_TRACES];
int opMode[XYGC_K_MAX_TRACES];
int y2Scale[XYGC_K_MAX_TRACES];

expStringClass xPvExpStr[XYGC_K_MAX_TRACES], yPvExpStr[XYGC_K_MAX_TRACES];

//const ProcessVariable::Type xPvType[XYGC_K_MAX_TRACES];
//const ProcessVariable::Type yPvType[XYGC_K_MAX_TRACES];
int xPvType[XYGC_K_MAX_TRACES], yPvType[XYGC_K_MAX_TRACES];

int traceType[XYGC_K_MAX_TRACES]; // XYGC_K_TRACE_XY or XYGC_K_TRACE_CHRONOLOGICAL

int plotState[XYGC_K_MAX_TRACES];

typedef struct plotInfoTag {
  int n;
  double firstDX;
  double lastDX;
  short firstX;
  short lastX;
  short firstY;
  short lastY;
  short minY;
  short maxY;
} plotInfoType, *plotInfoPtr;

plotInfoPtr plotInfo[XYGC_K_MAX_TRACES];
int plotInfoSize[XYGC_K_MAX_TRACES], plotInfoHead[XYGC_K_MAX_TRACES],
 plotInfoTail[XYGC_K_MAX_TRACES];

void *xPvData[XYGC_K_MAX_TRACES];
void *yPvData[XYGC_K_MAX_TRACES];
double xPvCurValue[XYGC_K_MAX_TRACES], yPvCurValue[XYGC_K_MAX_TRACES];
double xPvMin[XYGC_K_MAX_TRACES], xPvMax[XYGC_K_MAX_TRACES];
double yPvMin[XYGC_K_MAX_TRACES], yPvMax[XYGC_K_MAX_TRACES];
int xPvCount[XYGC_K_MAX_TRACES], yPvCount[XYGC_K_MAX_TRACES];
int xPvSize[XYGC_K_MAX_TRACES], yPvSize[XYGC_K_MAX_TRACES];

XPoint *plotBuf[XYGC_K_MAX_TRACES];
int plotBufSize[XYGC_K_MAX_TRACES];

int traceIsDrawn[XYGC_K_MAX_TRACES];

double dbXMin[XYGC_K_MAX_TRACES], dbXMax[XYGC_K_MAX_TRACES];
int dbXPrec[XYGC_K_MAX_TRACES], xArrayNeedInit[XYGC_K_MAX_TRACES],
 xArrayNeedUpdate[XYGC_K_MAX_TRACES], xArrayGotValue[XYGC_K_MAX_TRACES];

double dbYMin[XYGC_K_MAX_TRACES], dbYMax[XYGC_K_MAX_TRACES];
int dbYPrec[XYGC_K_MAX_TRACES], yArrayNeedInit[XYGC_K_MAX_TRACES],
 yArrayNeedUpdate[XYGC_K_MAX_TRACES], yArrayGotValue[XYGC_K_MAX_TRACES];

int arrayHead[XYGC_K_MAX_TRACES], arrayTail[XYGC_K_MAX_TRACES],
 arrayNumPoints[XYGC_K_MAX_TRACES], curNpts[XYGC_K_MAX_TRACES];

int effectiveCount[XYGC_K_MAX_TRACES], totalCount[XYGC_K_MAX_TRACES];

//ProcessVariable *trigPv, *resetPv;
chid resetPv, trigPv;
evid resetEv, trigEv;
expStringClass trigPvExpStr, resetPvExpStr;

int count, bufferScrollSize, plotStyle[XYGC_K_MAX_TRACES], plotMode, resetMode;
int firstTimeSample, curSec, curNsec;

int xAxis, xAxisStyle, xAxisSource, xAxisTimeFormat;
efDouble xMin, xMax;

int y1Axis, y1AxisStyle, y1AxisSource;
efDouble y1Min, y1Max;

int y2Axis, y2AxisStyle, y2AxisSource;
efDouble y2Min, y2Max;

double curXMin, curXMax, curY1Min, curY1Max, curY2Min, curY2Max;
int curXPrec, curY1Prec, curY2Prec;

int curXNumLabelTicks, curXMajorsPerLabel, curXMinorsPerMajor;
int curY1NumLabelTicks, curY1MajorsPerLabel, curY1MinorsPerMajor;
int curY2NumLabelTicks, curY2MajorsPerLabel, curY2MinorsPerMajor;

colorButtonClass plotCb[XYGC_K_MAX_TRACES];
colorButtonClass fgCb, bgCb, gridCb;

int fgColor;
int bgColor;
int gridColor;

int xFormatType;
efInt xPrecision;
char xFormat[15+1];

int y1FormatType;
efInt y1Precision;
char y1Format[15+1];

int y2FormatType;
efInt y2Precision;
char y2Format[15+1];

fontMenuClass fm;
char fontTag[63+1];

int border;

int opComplete, active, activeMode, init, bufInvalid;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

efInt xNumLabelIntervals;
int xLabelGrid;
efInt xNumMajorPerLabel;
int xMajorGrid;
efInt xNumMinorPerMajor;
int xMinorGrid;
efInt xAnnotationPrecision;
int xAnnotationFormat;

efInt y1NumLabelIntervals;
int y1LabelGrid;
efInt y1NumMajorPerLabel;
int y1MajorGrid;
efInt y1NumMinorPerMajor;
int y1MinorGrid;
efInt y1AnnotationPrecision;
int y1AnnotationFormat;

efInt y2NumLabelIntervals;
int y2LabelGrid;
efInt y2NumMajorPerLabel;
int y2MajorGrid;
efInt y2NumMinorPerMajor;
int y2MinorGrid;
efInt y2AnnotationPrecision;
int y2AnnotationFormat;

int xPvExists[XYGC_K_MAX_TRACES], yPvExists[XYGC_K_MAX_TRACES],
 trigPvExists, resetPvExists;

double xFactor[XYGC_K_MAX_TRACES], xOffset[XYGC_K_MAX_TRACES];
double y1Factor[XYGC_K_MAX_TRACES], y1Offset[XYGC_K_MAX_TRACES];
double y2Factor[XYGC_K_MAX_TRACES], y2Offset[XYGC_K_MAX_TRACES];

Widget plotWidget;

int needConnect, needInit, needRefresh, needUpdate, needErase, needDraw,
 needResetConnect, needReset, needTrigConnect, needTrig, needXRescale,
 needY1Rescale, needY2Rescale, needBufferScroll, needVectorUpdate,
 needRealUpdate, needBoxRescale;

int numBufferScrolls;

int updateTimerActive, updateTimerValue;
XtIntervalId updateTimer;

double xRescaleValue, y1RescaleValue, y2RescaleValue;

int needThisbufScroll[XYGC_K_MAX_TRACES];

entryFormClass *efTrace, *efAxis;

editBufPtr eBuf;

msgDialogClass msgDialog;
int msgDialogPopedUp;

int firstBoxRescale, doingBoxRescale;
double savedXMin, savedXMax, savedYMin, savedYMax;
double boxXMin, boxXMax, boxYMin, boxYMax;
int savedXMinNullState, savedXMaxNullState, savedYMinNullState,
 savedYMaxNullState;
int rescaleBoxX0, rescaleBoxY0, rescaleBoxX1, rescaleBoxY1,
 oldRescaleBoxW, oldRescaleBoxH;

public:

xyGraphClass::xyGraphClass ( void );

xyGraphClass::xyGraphClass
 ( const xyGraphClass *source );

xyGraphClass::~xyGraphClass ( void );

static void xyGraphClass::plotPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg );

static void xyGraphClass::plotUpdate (
  ProcessVariable *pv,
  void *userarg );

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

void xyGraphClass::regenBuffer ( void );

void xyGraphClass::genChronoVector (
  int trace,
  int *rescale
);

void xyGraphClass::genXyVector (
  int trace,
  int *rescale
);

int xyGraphClass::fullRefresh ( void );

int xyGraphClass::draw ( void );

int xyGraphClass::erase ( void );

int xyGraphClass::drawActiveOne (
  int i // trace
);

int xyGraphClass::drawActive ( void );

int xyGraphClass::altDrawActiveOne (
  int i // trace
);

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

void xyGraphClass::btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

void xyGraphClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

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

void xyGraphClass::initPlotInfo (
  int trace
);

void xyGraphClass::addPoint (
  double x,
  short x,
  short y,
  int trace
);

int xyGraphClass::fillPlotArray (
  int trace
);

void xyGraphClass::drawBorder ( void );

void xyGraphClass::drawXScale ( void );

void xyGraphClass::drawY1Scale ( void );

void xyGraphClass::drawGrid ( void );

void xyGraphClass::drawTitle ( void );

void xyGraphClass::drawXlabel ( void );

void xyGraphClass::drawYlabel ( void );

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
