//  for object: 2ed7d2e8-f439-11d2-8fed-00104b8742df
//  for lib: 3014b6ee-f439-11d2-ad99-00104b8742df

#ifndef __edmBox_obj_h
#define __edmBox_obj_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "cadef.h"

#define EBC_K_COLORMODE_STATIC 0
#define EBC_K_COLORMODE_ALARM 1

#define EBC_MAJOR_VERSION 2
#define EBC_MINOR_VERSION 1
#define EBC_RELEASE 0

#ifdef __edmBox_cc

#include "edmBoxComplete.str"

static char *dragName[] = {
  edmBoxComplete_str1,
};

static void ebc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ebc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ebc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ebc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ebc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void edmBoxAlarmUpdate (
  struct event_handler_args ast_args );

static void eboMonitorPvConnectState (
  struct connection_handler_args arg );

static void edmBoxUpdate (
  struct event_handler_args ast_args );

static void edmBoxInfoUpdate (
  struct event_handler_args ast_args );

#endif

class edmBoxClass : public activeGraphicClass {

private:

friend void ebc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ebc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ebc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ebc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ebc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void edmBoxAlarmUpdate (
  struct event_handler_args ast_args );

friend void eboMonitorPvConnectState (
  struct connection_handler_args arg );

friend void edmBoxInfoUpdate (
  struct event_handler_args ast_args );

friend void edmBoxUpdate (
  struct event_handler_args ast_args );

int init, active, activeMode, opComplete, pvExists, pointerMotionDetected;

int bufX, bufY, bufW, bufH;

int lineWidth, bufLineWidth;
int lineStyle, bufLineStyle;

pvColorClass lineColor;
int bufLineColor;
colorButtonClass lineCb;

int lineColorMode;
int bufLineColorMode;

int fill;
int bufFill;

pvColorClass fillColor;
int bufFillColor;
colorButtonClass fillCb;

int fillColorMode;
int bufFillColorMode;

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;

char label[63+1], bufLabel[63+1];

char bufPvName[activeGraphicClass::MAX_PV_NAME+1];
expStringClass pvExpStr;

int fontAscent, fontDescent, fontHeight, stringLength, stringWidth,
 labelX, labelY;

int bufferInvalid, boxH, alignment;

chid pvId;
int fieldType;
evid eventId, alarmEventId, infoEventId;

double value, curValue, readMin, readMax, factorW, factorH;
int centerX, centerY, sideW, sideH, sideX, sideY;
efDouble efReadMin, efReadMax, bufEfReadMin, bufEfReadMax;

int needConnectInit, needInfoInit, needUpdate, needDraw, needErase;

public:

edmBoxClass ( void );

// copy constructor
edmBoxClass (
  const edmBoxClass *source );

~edmBoxClass ( void );

char *objName ( void );

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
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnDown (
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

char *firstDragName ( void );

char *nextDragName ( void );

char *dragValue (
  int i );

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

};

extern "C" {

void *create_2ed7d2e8_f439_11d2_8fed_00104b8742dfPtr ( void );
void *clone_2ed7d2e8_f439_11d2_8fed_00104b8742dfPtr ( void * );

}

#endif
