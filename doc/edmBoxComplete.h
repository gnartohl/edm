//  component: 2ed7d2e8-f439-11d2-8fed-00104b8742df
// shared lib: 3014b6ee-f439-11d2-ad99-00104b8742df

#ifndef __edmBoxComplete_h
#define __edmBoxComplete_h 1

#include "act_grf.h"
#include "app_pkg.h"
#include "act_win.h"
#include "pv_factory.h"

#ifdef __edmBoxComplete_cc

#include "edmBoxComplete.str"

static char *dragName[] = {
  edmBoxComplete_str1
};

#endif

class edmBoxClass : public activeGraphicClass {

public:

// default constructor
edmBoxClass ( void );

// copy constructor
edmBoxClass (
  const edmBoxClass *source );

// destructor
~edmBoxClass ( void );

// returns component name
char *objName ( void );

// Called when user creates object interactively
int createInteractive (
  activeWindowClass *aw_obj,
  int x,
  int y,
  int w,
  int h );

int editCreate ( void );

int edit ( void );

// called by editCreate and edit
int genericEdit ( void );

// Called when file is opened
int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

int old_createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin );

// Called when file is saved
int save (
  FILE *f );

int old_save (
  FILE *f );

// Called when widget is resized; also called by widget
// when size or font might have changed
void updateDimensions ( void );

// Called when edit mode image needs to be drawn
int draw ( void );

// Called when edit mode image needs to be erased
int erase ( void );

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

// Called when execute mode image needs to be drawn
int drawActive ( void );

// Called when execute mode image needs to be erased
int eraseActive ( void );

// Called on expose event and called by widget code to indicate that
// the image buffer is invalid and that all detail should be redrawn
// at next "draw" command
void bufInvalidate ( void );

// Called before widget is activated; all expStringClass objects
// need to be processed
int expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] );

// Called when widget is activated and mux widget has changed a symbol; all
// expStringClass objects need to be processed
int expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] );

// Called to discover if any expStringClass objects contain macro references
int containsMacros ( void );

// Called when widget is activated
int activate (
  int pass,
  void *ptr );

// Called when widget is deactivated
int deactivate ( int pass );

// Called periodically so widget may do X Windows I/O to update image
// (and perform other event related activities)
void executeDeferred ( void );

// Called on button-up action
void btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

// Called on button-down action
void btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

// Called on button-drag action
void btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber );

// Called to discover in what actions widget has interest
int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

// Called to get name of first drag item (this widget has only one)
char *firstDragName ( void );

// Called to get name of next drag item (this widget has only one)
char *nextDragName ( void );

// Called to get value of ith drag item
char *dragValue (
  int i );

// Called to get run-time pv info
void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

private:

static const int COLORMODE_STATIC = 0; // color is not alarm sensitive
static const int COLORMODE_ALARM = 1;  // color is alarm sensitive
static const int MAJOR_VERSION = 4;    // Widget major version number
static const int MINOR_VERSION = 0;    // Widget minor version number
static const int RELEASE = 0;          // Widget release number

// Called by X when user presses OK button on property dialog form
static void editOk (
  Widget w,
  XtPointer client,
  XtPointer call );

// Called by X when user presses Apply button on property dialog form
static void editApply (
  Widget w,
  XtPointer client,
  XtPointer call );

// Called by editOk and editApply to copy edit buffer values into current
// property values
static void editUpdate (
  Widget w,
  XtPointer client,
  XtPointer call );

// Called by X when user presses Cancel button on property dialog form
static void editCancel (
  Widget w,
  XtPointer client,
  XtPointer call );

// Called by X when user presses Cancel button on property dialog form and
// widget is being created
static void editCancelDelete (
  Widget w,
  XtPointer client,
  XtPointer call );

// X windows makes client code do blinking if color depth is
// anything other than 8 bits
static void doBlink (
  void *ptr
);

// Called if widget does not connect within a given time interval
static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

// Called when a process variable connects or disconnects
static void monitorPvConnectState (
  ProcessVariable *pv,
  void *userarg );

// Called when the value of a process variable changes
static void pvUpdate (
  ProcessVariable *pv,
  void *userarg );

// user selectable attributes
int lineStyle;
int lineWidth;
pvColorClass lineColor;
colorButtonClass lineCb;
int lineColorMode;
int fill;
pvColorClass fillColor;
colorButtonClass fillCb;
int fillColorMode;
fontMenuClass fm;
char fontTag[63+1];
char label[63+1];
expStringClass pvExpStr;
int alignment;
efDouble efReadMin, efReadMax;
double readMin, readMax;
efInt efPrecision;
int precision;

typedef struct editBufTag {
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufLineWidth;
  int bufLineStyle;
  int bufLineColor;
  int bufLineColorMode;
  int bufFill;
  int bufFillColor;
  int bufFillColorMode;
  char bufFontTag[63+1];
  char bufLabel[63+1];
  char bufPvName[PV_Factory::MAX_PV_NAME+1];
  efDouble bufEfReadMin;
  efDouble bufEfReadMax;
  efInt bufEfPrecision;
} editBufType, *editBufPtr;

editBufPtr editBuf; // edit buffer for user selectable attributes

// Text related parameters

// Font metrics
int fontAscent;
int fontDescent;
int fontHeight;
int stringLength;
int stringWidth;

int labelX;         // x location of label
int labelY;         // y location of label
int textValueY;     // x location of text value
int textValueX;     // y location of text value
XFontStruct *fs;    // X Windows font struct
char format[15+1];  // holds format used to display text value

// Misc
int centerX;    // x cood of center of value box
int centerY;    // y cood of center of value box
int boxY;       // y cood of top-left of value box display area
int boxH;       // height of value box display area

// execute mode variables
int init;                  // 1 if pv has connected at least once
                           //  since activation
int active;                // 1 if pv is currently connected
int activeMode;            // 1 if widget is in execute mode
int opComplete;            // used by activate method
int pvExists;              // 1 if pv name is not blank
int pointerMotionDetected; // used by button processing methods
int bufferInvalid;         // 1 means that all details need to be drawn/erased
                           //  at next opportunity

// Process variable related information
ProcessVariable *pvId;
 int fieldType;             // ProcessVariable::Type::real
                            // ProcessVariable::Type::integer
                            // ProcessVariable::Type::enumerated
                            // ProcessVariable::Type::text
int oldStat;                // previous alarm status value
int oldSev;                 // previous alarm severity value

// Misc
double value;        // current pv value
double curValue;     // copy of current pv value
double prevValue;    // previous pv value
double factorW;      // for scaling
double factorH;      // for scaling
int valueBoxW;       // current value box width
int valueBoxH;       // current value box height
int valueBoxX;       // current value box x coord
int valueBoxY;       // current value box y coord

// Flags set in event functions to request deferred processing
int needConnectInit;
int needUpdate;
int needDraw;
int needErase;
int needToDrawUnconnected;
int needToEraseUnconnected;

// X Windows timer used to sense that pv has not connected in timely manner
int unconnectedTimer;

};

// Class factory functions
extern "C" {
void *create_2ed7d2e8_f439_11d2_8fed_00104b8742dfPtr ( void );
void *clone_2ed7d2e8_f439_11d2_8fed_00104b8742dfPtr ( void * );
}

#endif
