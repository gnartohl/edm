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

activeCircleClass::activeCircleClass ( void );

// copy constructor
activeCircleClass::activeCircleClass
( const activeCircleClass *source );

activeCircleClass::~activeCircleClass ( void ) {

  if ( name ) delete name;

}

char *activeCircleClass::objName ( void ) {

  return name;

}

int activeCircleClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int activeCircleClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int activeCircleClass::importFromXchFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int activeCircleClass::save (
  FILE *f );

int activeCircleClass::genericEdit ( void );

int activeCircleClass::edit ( void );

int activeCircleClass::editCreate ( void );

int activeCircleClass::draw ( void );

int activeCircleClass::erase ( void );

int activeCircleClass::drawActive ( void );

int activeCircleClass::eraseActive ( void );

int activeCircleClass::eraseUnconditional ( void );

int activeCircleClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeCircleClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int activeCircleClass::containsMacros ( void );

int activeCircleClass::activate ( int pass, void *ptr );

int activeCircleClass::deactivate ( int pass );

void activeCircleClass::executeDeferred ( void );

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
