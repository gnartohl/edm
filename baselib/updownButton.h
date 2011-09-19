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

#ifndef __updownButton_h
#define __updownButton_h 1

#include "act_grf.h"
#include "entry_form.h"
#include "keypad.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define UDBTC_MAJOR_VERSION 4
#define UDBTC_MINOR_VERSION 0
#define UDBTC_RELEASE 0

#ifdef __updownButton_cc

#include "updownButton.str"

static void doBlink (
  void *ptr
);

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void udbtoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void udbtc_visUpdate (
  ProcessVariable *pv,
  void *userarg );

static void udbtc_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void udbtc_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

static void udbtc_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

static void udbtc_saveUpdate (
  ProcessVariable *pv,
  void *userarg );

static void udbtc_decrement (
  XtPointer client,
  XtIntervalId *id );

static void udbtc_increment (
  XtPointer client,
  XtIntervalId *id );

static char *dragName[] = {
  activeUpdownButtonClass_str8,
  activeUpdownButtonClass_str25,
  activeUpdownButtonClass_str29
};

static void udbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

static void udbtc_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void udbtc_monitor_save_connect_state (
  ProcessVariable *pv,
  void *userarg );

#endif

class activeUpdownButtonClass : public activeGraphicClass {

private:

friend void doBlink (
  void *ptr
);

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void udbtoCancelKp (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtoSetKpDoubleValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_monitor_vis_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void udbtc_visUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void udbtc_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void udbtc_colorUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void udbtc_controlUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void udbtc_saveUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void udbtc_decrement (
  XtPointer client,
  XtIntervalId *id );

friend void udbtc_increment (
  XtPointer client,
  XtIntervalId *id );

friend void udbtc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void udbtc_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void udbtc_monitor_save_connect_state (
  ProcessVariable *pv,
  void *userarg );

int opComplete;

int minW;
int minH;

typedef struct editBufTag {
// edit buffer
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufFgColor;
  int bufBgColor;
  int bufTopShadowColor;
  int bufBotShadowColor;
  int buf3D;
  int bufInvisible;
  double bufRate;
  int bufLimitsFromDb;
  efDouble bufEfScaleMin;
  efDouble bufEfScaleMax;
  int bufVisInverted;
  colorButtonClass fgCb;
  colorButtonClass bgCb;
  colorButtonClass topShadowCb;
  colorButtonClass botShadowCb;
  char bufLabel[39+1];
  char bufDestPvName[PV_Factory::MAX_PV_NAME+1];
  char bufSavePvName[PV_Factory::MAX_PV_NAME+1];
  char bufFine[39+1];
  char bufCoarse[39+1];
  char bufVisPvName[PV_Factory::MAX_PV_NAME+1];
  char bufMinVisString[39+1];
  char bufMaxVisString[39+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
} editBufType, *editBufPtr;

editBufPtr eBuf;

entryListBase *limitsFromDbEntry, *minEntry, *maxEntry;

entryListBase *invisPvEntry, *visInvEntry, *minVisEntry, *maxVisEntry;

int destType, saveType;

pvColorClass fgColor, bgColor;
int topShadowColor;
int botShadowColor;
expStringClass label;
int _3D, invisible;

fontMenuClass fm;
char fontTag[63+1];
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

static const int destPvConnection = 1;
static const int visPvConnection = 2;
static const int colorPvConnection = 3;

pvConnectionClass connection;

ProcessVariable *destPvId, *savePvId;

expStringClass destPvExpString;

expStringClass savePvExpString;

expStringClass fineExpString;

expStringClass coarseExpString;

double rate;

int destExists, saveExists, buttonPressed;

int destPvConnected, savePvConnected, active, activeMode, init;

int incrementTimerActive, incrementTimerValue;
XtIntervalId incrementTimer;
double controlV, curControlV, curSaveV, coarse, fine;

int needConnectInit, needSaveConnectInit, needCtlInfoInit, needRefresh,
 needErase, needDraw, needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;
int initialConnection, initialSavedValueConnection, initialVisConnection,
 initialColorConnection;

int widgetsCreated;
Widget popUpMenu, pullDownMenu, pbCoarse, pbFine, pbRate, pbValue,
 pbSave, pbRestore;

double kpDouble;
keypadClass kp;
int keyPadOpen, kpDest;

int isSaved;

static const int kpCoarseDest = 1;
static const int kpFineDest = 2;
static const int kpRateDest = 3;
static const int kpValueDest = 4;

int limitsFromDb;
double scaleMin, scaleMax, minDv, maxDv;
efDouble efScaleMin, efScaleMax;

ProcessVariable *visPvId;
expStringClass visPvExpString;
int visExists;
double visValue, curVisValue, minVis, maxVis;
char minVisString[39+1];
char maxVisString[39+1];
int prevVisibility, visibility, visInverted;
int needVisConnectInit, needVisInit, needVisUpdate;

ProcessVariable *colorPvId;
expStringClass colorPvExpString;
int colorExists;
double colorValue, curColorValue;
int needColorConnectInit, needColorInit, needColorUpdate;

int rootX, rootY;

public:

activeUpdownButtonClass ( void );

activeUpdownButtonClass
 ( const activeUpdownButtonClass *source );

~activeUpdownButtonClass ( void );

char *objName ( void ) {

  return name;

}

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

int importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate ( int pass, void *ptr );

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

void executeDeferred ( void );

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

void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

char *crawlerGetFirstPv ( void );

char *crawlerGetNextPv ( void );

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeUpdownButtonClassPtr ( void );
void *clone_activeUpdownButtonClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
