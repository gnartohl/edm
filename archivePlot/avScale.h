//  for object: 458bb765_eab9_4d65_8fda_2ce55d2baec6
//  for lib: ?

#ifndef __avScale_h
#define __avScale_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "utility.h"
#include "pv_factory.h"
#include "cvtFast.h"

#define SCLC_MAJOR_VERSION 4
#define SCLC_MINOR_VERSION 0
#define SCLC_RELEASE 0

#ifdef __avScale_cc

#include "avScale.str"

static void sclc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sclc_value_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sclc_value_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void scloMonitorPvConnectState (
  ProcessVariable *pv,
  void *userArg );

static void scaleUpdate (
  ProcessVariable *pv,
  void *userArg );

static void sclc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sclc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sclc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sclc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void sclc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class scaleClass : public activeGraphicClass {

private:

friend void sclc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sclc_value_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sclc_value_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void scloMonitorPvConnectState (
  ProcessVariable *pv,
  void *userArg );

friend void scaleUpdate (
  ProcessVariable *pv,
  void *userArg );

friend void sclc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sclc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sclc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sclc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void sclc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

int init, active, activeMode, opComplete;

typedef struct editBufTag {
// edit buffer
  int bufLineColor;
  int bufScaleId;
  colorButtonClass lineCb;
  char minBufPvName[PV_Factory::MAX_PV_NAME+1];
  char maxBufPvName[PV_Factory::MAX_PV_NAME+1];
  char colorBufPvName[PV_Factory::MAX_PV_NAME+1];
  char modeBufPvName[PV_Factory::MAX_PV_NAME+1];
  char labelBufPvName[PV_Factory::MAX_PV_NAME+1];
  char updateBufPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

int bufX;
int bufY;
int bufW;
int bufH;

int scaleOfs, scaleLen, horizontal;

pvColorClass lineColor;

fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;

expStringClass minPvExpStr, maxPvExpStr, colorPvExpStr,
 modePvExpStr, labelPvExpStr, updatePvExpStr;

int scaleId;

int pvsExist;

pvConnectionClass connection;

int fontAscent, fontDescent, fontHeight, stringWidth,
 minTextLen, maxTextLen;

int bufferInvalid;

ProcessVariable *minPv, *maxPv, *colorPv, *modePv, *labelPv, *updatePv;
int minFieldType, maxFieldType, colorFieldType, modeFieldType,
 labelFieldType, updateFieldType;

int numLabTicks, numMajTicks, numMinTicks;
double min, max, adj_min, adj_max, label_tick, major_tick, minor_tick;
int colorIndex, mode, newMode, update;
char label[39+1], newLabel[39+1];
char format[15+1];

entryFormClass textEntry;
int teX, teY, teW, teH, teLargestH;
double bufMin, bufMax;
int entryState, editDialogIsActive;

int needConnectInit, needUpdate, needDraw, needErase, needModeUpdate;
int modeChange;

public:

static const int updateAllScales = 32000;
static const int updateAllGraphs = 32001;
static const int updateAll = 32002;
static const int updateXMarker = 32003;

static const int maxDataPoints = 500000;

static const int modeLinear = 0;
static const int modeLog = 1;
static const int modeHours = 3;
static const int modeMinutes = 4;
static const int modeSeconds = 5;
static const int modeDays = 6;

scaleClass ( void );

// copy constructor
scaleClass (
  const scaleClass *source );

~scaleClass ( void );

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

int drawXLinearScale (
  Widget w,
  gcClass *gc,
  int erase
);

int formatString (
  double value,
  char *string,
  int len
);

int drawXLinearAnnotation (
  Widget wdgt,
  gcClass *gc,
  int erase
);

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
