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

#ifndef __x_text_dsp_obj_h
#define __x_text_dsp_obj_h 1

#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <math.h>

#include "ulBindings.h"
#include "act_grf.h"
#include "entry_form.h"
#include "keypad.h"
#include "calpad.h"
#include "fileSelect.h"

#include "pv_factory.h"
#include "cvtFast.h"

// max size of char array
#define XTDC_K_MAX 255

#define XTDC_K_FORMAT_NATURAL 0
#define XTDC_K_FORMAT_FLOAT 1
#define XTDC_K_FORMAT_GFLOAT 2
#define XTDC_K_FORMAT_EXPONENTIAL 3
#define XTDC_K_FORMAT_DECIMAL 4
#define XTDC_K_FORMAT_HEX 5
#define XTDC_K_FORMAT_STRING 6

#define XTDC_K_COLORMODE_STATIC 0
#define XTDC_K_COLORMODE_ALARM 1

#define XTDC_K_FILE_FULL_PATH 0
#define XTDC_K_FILE_NAME_EXT 1
#define XTDC_K_FILE_NAME 2

#define XTDC_MAJOR_VERSION 4
#define XTDC_MINOR_VERSION 7
#define XTDC_RELEASE 0

#ifdef __x_text_dsp_obj_cc

#include "x_text_dsp_obj.str"

static void eventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

static void dropTransferProc (
  Widget w,
  XtPointer clientData,
  Atom *selType,
  Atom *type,
  XtPointer value,
  unsigned long *length,
  XtPointer format );

static void handleDrop (
  Widget w,
  XtPointer client,
  XtPointer call );

static void doBlink (
  void *ptr
);

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static char *dragName[] = {
  activeXTextDspClass_str1,
  activeXTextDspClass_str2
};

static void xtdoCancelStr (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetCpValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetFsValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoRestoreValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetKpIntValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void dummy (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void selectActions (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void pvInfo (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

static void xtdo_access_security_change (
  ProcessVariable *pv,
  void *userarg );

static void xtdo_monitor_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void xtdo_monitor_sval_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void xtdo_monitor_fg_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void XtextDspUpdate (
  ProcessVariable *pv,
  void *userarg );

static void XtextDspSvalUpdate (
  ProcessVariable *pv,
  void *userarg );

static void XtextDspFgUpdate (
  ProcessVariable *pv,
  void *userarg );

static void XtextDspBgUpdate (
  ProcessVariable *pv,
  void *userarg );

static void axtdc_value_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_value_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void axtdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToStringA (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToStringLF (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToIntA (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToIntLF (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToDoubleA (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoTextFieldToDoubleLF (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoGrabUpdate (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetSelection (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoSetValueChanged (
  Widget w,
  XtPointer client,
  XtPointer call );

static void xtdoModVerify (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class activeXTextDspClass : public activeGraphicClass {

private:

friend void eventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

friend void dropTransferProc (
  Widget w,
  XtPointer clientData,
  Atom *selType,
  Atom *type,
  XtPointer value,
  unsigned long *length,
  XtPointer format );

friend void handleDrop (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void doBlink (
  void *ptr
);

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void xtdoCancelStr (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetCpValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetFsValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoRestoreValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetKpIntValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void dummy (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void selectActions (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void pvInfo (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams );

friend void xtdo_access_security_change (
  ProcessVariable *pv,
  void *userarg );

friend void xtdo_monitor_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void xtdo_monitor_sval_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void xtdo_monitor_fg_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void XtextDspUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void XtextDspSvalUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void XtextDspFgUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void XtextDspBgUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void axtdc_value_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_value_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void axtdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToStringA (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToStringLF (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToIntA (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToIntLF (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToDoubleA (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoTextFieldToDoubleLF (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoGrabUpdate (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetSelection (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoSetValueChanged (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void xtdoModVerify (
  Widget w,
  XtPointer client,
  XtPointer call );

static const int pvConnection = 1;
static const int svalPvConnection = 2;
static const int fgPvConnection = 3;
pvConnectionClass connection;

typedef struct editBufTag {
// edit buffer
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufNumDecimals;
  int bufFormatType;
  int bufColorMode;
  int bufBgColorMode;
  int bufSmartRefresh;
  char bufFontTag[63+1];
  int bufUseDisplayBg;
  int bufAutoHeight;
  int bufLimitsFromDb;
  int bufChangeValOnLoseFocus;
  int bufFastUpdate;
  int bufAutoSelect;
  int bufUpdatePvOnDrop;
  int bufUseHexPrefix;
  efInt bufEfPrecision;
  char bufFieldLenInfo[7+1];
  int bufClipToDspLimits;
  int bufBgColor;
  int bufFgColor;
  int bufSvalColor;
  colorButtonClass fgCb;
  colorButtonClass bgCb;
  colorButtonClass svalCb;
  int bufChangeCallbackFlag;
  int bufActivateCallbackFlag;
  int bufDeactivateCallbackFlag;
  int bufNullDetectMode;
  char bufPvName[PV_Factory::MAX_PV_NAME+1];
  char bufSvalPvName[PV_Factory::MAX_PV_NAME+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
  char bufDefDir[XTDC_K_MAX+1];
  char bufPattern[XTDC_K_MAX+1];
  int bufIsWidget;
  int bufEditable;
  int bufIsDate;
  int bufIsFile;
  int bufFileComponent;
  int bufDateAsFileName;
  int bufUseKp;
  int bufShowUnits;
  int bufUseAlarmBorder;
  int bufInputFocusUpdatesAllowed;
  int bufIsPassword;
  int bufCharacterMode;
  int bufNoExecuteClipMask;
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *nullPvEntry, *nullCondEntry, *nullColorEntry;

entryListBase *limitsFromDbEntry, *precisionEntry;

entryListBase *editableEntry, *keypadEntry;

entryListBase *isWidgetEntry, *charModeEntry, *inFocUpdEntry, *chgValOnFocEntry,
 *autoSelEntry, *updPvOnDropEntry, *isPwEntry;

entryListBase *dateEntry, *cvtDateToFileEntry;

entryListBase *fileEntry, *returnEntry, *defDirEntry, *patEntry;

entryListBase *chgCbEntry;

 entryListBase *useDspBgEntry, *bgColorEntry, *bgColorModeEntry;

int numDecimals, formatType, colorMode, bgColorMode,
 pvType, pvCount, svalPvType, noSval, svalPvCount;
char format[15+1];
int opComplete, pvExistCheck, activeMode, init, noSvalYet;
int smartRefresh;
double dvalue, curDoubleValue, curSvalValue;
char curValue[XTDC_K_MAX+1], value[XTDC_K_MAX+1], bfrValue[XTDC_K_MAX+1];
fontMenuClass fm;
char fontTag[63+1];
int useDisplayBg, alignment, autoHeight;
int limitsFromDb;
int changeValOnLoseFocus;
int fastUpdate;
int autoSelect;
int updatePvOnDrop;
int useHexPrefix;
int precision;
efInt efPrecision;
char fieldLenInfo[7+1];
int clipToDspLimits;
double upperLim, lowerLim;
pvColorClass bgColor;
pvColorClass fgColor;
colorButtonClass fgCb, bgCb, svalCb;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight, stringLength, stringWidth,
 stringY, stringX, bufInvalid;

IPFUNC changeCallback;

VPFUNC activateCallback, deactivateCallback;
int changeCallbackFlag, activateCallbackFlag, deactivateCallbackFlag,
 anyCallbackFlag;

int pvExists, svalPvExists, fgPvExists;

// nullDetectMode: 0=null when saved value pv equals cur vale
//                 1=null when saved value pv is 0
int nullDetectMode;

ProcessVariable *pvId, *svalPvId, *fgPvId;

int fgPvValue;
int oldChangeResult;

int pvIndex;
expStringClass pvExpStr, svalPvExpStr, fgPvExpStr;
char pvName[PV_Factory::MAX_PV_NAME+1];

expStringClass defDir, pattern;

int numStates;

int isWidget;
int handlerInstalled;
int editable;
entryFormClass textEntry;
int teX, teY, teW, teH, teLargestH;
char entryValue[XTDC_K_MAX+1];
int entryState, editDialogIsActive;
int isDate;
int isFile;
int fileComponent;
int dateAsFileName;

Widget tf_widget;
int widget_value_changed;

int needConnectInit, needInfoInit, needErase, needDraw, needRefresh,
 needUpdate, deferredCount, needToDrawUnconnected, needToEraseUnconnected,
 needFgPvPut, needAccessSecurityCheck, initialConnection;
XtIntervalId unconnectedTimer;

keypadClass kp;
int kpInt;
double kpDouble;
int useKp;
calpadClass cp;
fselectClass fsel;

int grabUpdate;
int focusIn, focusOut, cursorIn, cursorOut;
int needInitialValue;

int showUnits;
char units[MAX_UNITS_SIZE+1];

// only used for alarm border
short prevAlarmSeverity;

int useAlarmBorder;

int newPositioning;

int oldStat, oldSev;

int inputFocusUpdatesAllowed;

int isPassword;
char pwValue[255+1];
int pwLength;

int characterMode;

int noExecuteClipMask;

int writeDisabled;

public:

activeXTextDspClass ( void );

activeXTextDspClass
 ( const activeXTextDspClass *source );

~activeXTextDspClass ( void );

char *objName ( void ) {

  return name;

}

int putValueWithClip (
  char *val
);

int putValueWithClip (
  double val
);

int putValueWithClip (
  int val
);

int minStringSize( void );

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

int old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int save (
  FILE *f );

int old_save (
  FILE *f );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

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

void updateDimensions ( void );

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

void pointerIn (
  int _x,
  int _y,
  int buttonState );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

void executeDeferred ( void );

int getProperty (
  char *prop,
  int bufSize,
  char *value );

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

void map ( void );

void unmap ( void );

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

char *crawlerGetFirstPv ( void );

char *crawlerGetNextPv ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeXTextDspClassPtr ( void );
void *clone_activeXTextDspClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
