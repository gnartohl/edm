#ifndef __circle_gen_h
#define __circle_gen_h 1

#include "act_grf.h"
#include "entry_form.h"

//#ifdef __epics__
//#include "cadef.h"
//#endif

#include "pv.h"

#define ACC_K_COLORMODE_STATIC 0
#define ACC_K_COLORMODE_ALARM 1

#define ACC_MAJOR_VERSION 1
#define ACC_MINOR_VERSION 4
#define ACC_RELEASE 0

#ifdef __circle_gen_cc

static void acc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void acc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void acc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void acc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void acc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void acoMonitorAlarmPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

static void circleAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );
  
static void acoMonitorVisPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );
  
static void circleVisUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

#endif

class activeCircleClass : public activeGraphicClass {

private:

friend void acc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void acc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void acc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void acc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void acc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void acoMonitorAlarmPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void circleAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void acoMonitorVisPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args );

friend void circleVisUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );

int bufX, bufY, bufW, bufH;

pvColorClass lineColor;
unsigned int bufLineColor;
colorButtonClass lineCb;

int lineColorMode;
int bufLineColorMode;

int fill;
int bufFill;

pvColorClass fillColor;
unsigned int bufFillColor;
colorButtonClass fillCb;

int fillColorMode;
int bufFillColorMode;

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
char bufAlarmPvName[39+1];

expStringClass visPvExpStr;
char bufVisPvName[39+1];

int alarmPvExists, alarmPvConnected, visPvExists, visPvConnected;
int active, activeMode, init, opComplete;

int lineWidth, bufLineWidth;
int lineStyle, bufLineStyle;

int needVisConnectInit;
int needAlarmConnectInit;
int needDraw, needErase, needRefresh;

char pvClassName[15+1], pvUserClassName[15+1], pvOptionList[255+1];

int numPvTypes, pvNameIndex;

public:

activeCircleClass ( void );

// copy constructor
activeCircleClass
( const activeCircleClass *source );

~activeCircleClass ( void ) {

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

int createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int save (
  FILE *f );

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

int activate ( int pass, void *ptr );

int deactivate ( int pass );

void executeDeferred ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeCircleClassPtr ( void );
void *clone_activeCircleClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
