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
#define EBC_MINOR_VERSION 0
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

fontMenuClass fm;
char fontTag[63+1], bufFontTag[63+1];
XFontStruct *fs;

char label[63+1], bufLabel[63+1];

char bufPvName[39+1];
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

edmBoxClass::edmBoxClass ( void );

// copy constructor
edmBoxClass::edmBoxClass (
  const edmBoxClass *source );

edmBoxClass::~edmBoxClass ( void );

char *edmBoxClass::objName ( void );

int edmBoxClass::createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int edmBoxClass::save (
  FILE *f );

int edmBoxClass::createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int edmBoxClass::genericEdit ( void );

int edmBoxClass::edit ( void );

int edmBoxClass::editCreate ( void );

int edmBoxClass::draw ( void );

int edmBoxClass::erase ( void );

void edmBoxClass::updateDimensions ( void );

int edmBoxClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h );

int edmBoxClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h );

int edmBoxClass::drawActive ( void );

int edmBoxClass::eraseActive ( void );

void edmBoxClass::bufInvalidate ( void );

int edmBoxClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

int edmBoxClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

int edmBoxClass::containsMacros ( void );

int edmBoxClass::activate (
  int pass,
  void *ptr );

int edmBoxClass::deactivate ( int pass );

void edmBoxClass::executeDeferred ( void );

void edmBoxClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void edmBoxClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void edmBoxClass::btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

int edmBoxClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

char *edmBoxClass::firstDragName ( void );

char *edmBoxClass::nextDragName ( void );

char *edmBoxClass::dragValue (
  int i );

void edmBoxClass::changeDisplayParams (
  unsigned int flag,
  char *fontTag,
  int alignment,
  char *ctlFontTag,
  int ctlAlignment,
  char *btnFontTag,
  int btnAlignment,
  unsigned int textFgColor,
  unsigned int fg1Color,
  unsigned int fg2Color,
  unsigned int offsetColor,
  unsigned int bgColor,
  unsigned int topShadowColor,
  unsigned int botShadowColor );

void edmBoxClass::changePvNames (
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
