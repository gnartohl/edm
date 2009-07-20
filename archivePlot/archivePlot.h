//  for object: 2d80926b_bf54_4096_ab21_af7d725f15a2
//  for lib: ?

#ifndef __archivePlot_h
#define __archivePlot_h 1

#include <time.h>
#include <sys/time.h>
#include "sys_types.h"

#ifdef OLDARCHIVER

#include "BinArchive.h"
#include "ArchiveException.h"
//#include "ArchiveI.h"

#include "osiTime.h"

#else

// Base
#include <epicsVersion.h>
// Tools
#include <AutoPtr.h>
#include <BinaryTree.h>
#include <RegularExpression.h>
#include <epicsTimeHelper.h>
#include <ArgParser.h>
// Storage
#include <SpreadsheetReader.h>

#endif

#include "act_grf.h"
#include "entry_form.h"
#include "msg_dialog.h"
#include "utility.h"
#include "pv_factory.h"
#include "cvtFast.h"

#define ARPLC_MAJOR_VERSION 4
#define ARPLC_MINOR_VERSION 0
#define ARPLC_RELEASE 0

#ifdef __archivePlot_cc

#include "archivePlot.str"

static void aploUpdateMsg (
  XtPointer client,
  XtIntervalId *id );

static void arploMonitorPvConnectState (
  ProcessVariable *pv,
  void *userArg );

static void archivePlotUpdate (
  ProcessVariable *pv,
  void *userArg );

static void arplc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arplc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arplc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arplc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void arplc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class archivePlotClass : public activeGraphicClass {

private:

friend void aploUpdateMsg (
  XtPointer client,
  XtIntervalId *id );

friend void arplc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arplc_value_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arplc_value_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arploMonitorPvConnectState (
  ProcessVariable *pv,
  void *userArg );

friend void archivePlotUpdate (
  ProcessVariable *pv,
  void *userArg );

friend void arplc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arplc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arplc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arplc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void arplc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

int init, active, activeMode, opComplete;

typedef struct editBufTag {
// edit buffer
  int bufLineColor;
  int bufBgColor;
  int bufGraphId;
  colorButtonClass lineCb;
  colorButtonClass bgCb;
  char xMinBufPvName[PV_Factory::MAX_PV_NAME+1];
  char xMaxBufPvName[PV_Factory::MAX_PV_NAME+1];
  char xModeBufPvName[PV_Factory::MAX_PV_NAME+1];
  char yMinBufPvName[PV_Factory::MAX_PV_NAME+1];
  char yMaxBufPvName[PV_Factory::MAX_PV_NAME+1];
  char yModeBufPvName[PV_Factory::MAX_PV_NAME+1];
  char colorBufPvName[PV_Factory::MAX_PV_NAME+1];
  char fileBufPvName[PV_Factory::MAX_PV_NAME+1];
  char updateBufPvName[PV_Factory::MAX_PV_NAME+1];
  char archiveBufPvName[PV_Factory::MAX_PV_NAME+1];
  char startTimeBufPvName[PV_Factory::MAX_PV_NAME+1];
  char endTimeBufPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

int bufX;
int bufY;
int bufW;
int bufH;

pvColorClass lineColor;

pvColorClass bgColor;

char archiveName[39+1];

SYS_TIME_TYPE sysTime;
time_t start_time_t, end_time_t;

expStringClass xMinPvExpStr, xMaxPvExpStr, xModePvExpStr,
 yMinPvExpStr, yMaxPvExpStr, yModePvExpStr,
 colorPvExpStr, filePvExpStr, updatePvExpStr,
 archivePvExpStr, startTimePvExpStr, endTimePvExpStr;

int graphId;

int pvsExist;

pvConnectionClass connection;

int bufferInvalid;

ProcessVariable *xMinPv, *xMaxPv, *xModePv,
 *yMinPv, *yMaxPv, *yModePv,
 *colorPv, *filePv, *updatePv,
 *archivePv, *startTimePv, *endTimePv;

int xMinFieldType, xMaxFieldType, xModeFieldType,
 yMinFieldType, yMaxFieldType, yModeFieldType,
 colorFieldType, fileFieldType, updateFieldType,
 archiveFieldType, startTimeFieldType, endTimeFieldType;

double bufXMin, bufXMax, bufYMin, bufYMax;
double xMin, xMax, adj_xMin, adj_xMax, yMin, yMax, adj_yMin, adj_yMax;
int colorIndex, xMode, yMode, update;
char file[39+1];

int needConnectInit, needFileUpdate, needUpdate, needDraw, needErase,
 needXMarkerDraw, needXMarkerErase, needXMarkerDrawCommand,
 needXMarkerEraseCommand;
int markerDrawn;
double markerX, oldMarkerX;

#if 0
double *xarray, *yarray;
unsigned int *ixarray, *iyarray;
#endif

static const int maxDataPoints = 500000;
double xarray[maxDataPoints], yarray[maxDataPoints];
unsigned int ixarray[maxDataPoints], iyarray[maxDataPoints];
int numPoints;

int btn1Down, drawRegion, x0, y0, x1, y1, xx0, yy0, xx1, yy1,
 selectX0, selectX1, selectY0, selectY1;

static const int saveStackSize = 32;
double saveXMin[saveStackSize], saveXMax[saveStackSize],
 saveYMin[saveStackSize], saveYMax[saveStackSize];
int saveIndex;

msgDialogClass msgDialog;
int msgDialogPoppedUp;
XtIntervalId msgTimer;
int timerX, timerY;
int timerMsgX, timerMsgY;

int abortRead;

DbrType yType;

public:

static const int updateAllScales = 32000;
static const int updateAllGraphs = 32001;
static const int updateAll = 32002;
static const int drawXMarker = 32003;
static const int eraseXMarker = 32004;

static const int modeLinear = 0;
static const int modeLog = 1;
static const int modeHours = 3;
static const int modeMinutes = 4;
static const int modeSeconds = 5;
static const int modeDays = 6;

archivePlotClass ( void );

// copy constructor
archivePlotClass (
  const archivePlotClass *source );

~archivePlotClass ( void );

char *objName ( void );

int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int save (
  FILE *f );

int old_save (
  FILE *f );

int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int old_createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

void updateDimensions ( void );

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

int drawActive ( void );

int eraseActive ( void );

void bufInvalidate ( void );

int expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] );

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

int deactivate ( int pass );

void executeDeferred ( void );

void btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnDrag (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState,
  int buttonNumber );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

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

int readFile ( void );

int readArchive ( void );

void rescale ( void );

void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

};

extern "C" {

void *create_458bb765_eab9_4d65_8fda_2ce55d2baec6Ptr ( void );
void *clone_458bb765_eab9_4d65_8fda_2ce55d2baec6Ptr ( void * );

}

#endif
