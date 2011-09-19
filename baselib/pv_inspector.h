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

#ifndef __pv_inspector_h
#define __pv_inspector_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define PIC_ORIG_POS 0
#define PIC_BUTTON_POS 1
#define PIC_PARENT_OFS_POS 2

#define PIC_MAJOR_VERSION 4
#define PIC_MINOR_VERSION 1
#define PIC_RELEASE 0

typedef struct objAndIndexTag {
  void *obj;
  int index;
} objAndIndexType;

#ifdef __pv_inspector_cc

#include "pv_inspector.str"

static void rtypeUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void pioUpdateValue (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pioGrabUpdate (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pioSetSelection (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pioSetValueChanged (
  Widget w,
  XtPointer client,
  XtPointer call );

static void dropTransferProc (
  Widget w,
  XtPointer clientData,
  Atom *selType,
  Atom *type,
  XtPointer value,
  unsigned long *length,
  int format );

static void handleDrop (
  Widget w,
  XtPointer client,
  XtPointer call );

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void monitor_pv_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void monitor_rtype_pv_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void pv_update (
  ProcessVariable *pv,
  void *userarg );

static void rtype_pv_update (
  ProcessVariable *pv,
  void *userarg );

static void pic_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pic_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pic_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pic_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pic_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void pic_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class pvInspectorClass : public activeGraphicClass {

public:

static const int NUMPVS = 4;
static const int maxDsps = 20;

private:

friend void rtypeUnconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void pioUpdateValue (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pioGrabUpdate (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pioSetSelection (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pioSetValueChanged (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void dropTransferProc (
  Widget w,
  XtPointer clientData,
  Atom *selType,
  Atom *type,
  XtPointer value,
  unsigned long *length,
  int format );

friend void handleDrop (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void monitor_pv_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void monitor_rtype_pv_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void pv_update (
  ProcessVariable *pv,
  void *userarg );

friend void rtype_pv_update (
  ProcessVariable *pv,
  void *userarg );

friend void openDisplay (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pic_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pic_edit_update1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pic_edit_apply1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pic_edit_cancel1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pic_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pic_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pic_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pic_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void pic_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

typedef struct bufTag {
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufTopShadowColor;
  int bufBotShadowColor;
  int bufFgColor;
  int bufBgColor;
  int bufNoEdit;
  int bufSetPostion[maxDsps];
  int bufAllowDups[maxDsps];
  char bufDisplayFileName[maxDsps][127+1];
  char bufButtonLabel[127+1];
  char bufLabel[maxDsps][127+1];
  char bufFontTag[63+1];
  int bufOfsX;
  int bufOfsY;
  int bufUseRtype[maxDsps];
  int bufUseType[maxDsps];
  int bufUseSpecType[maxDsps];
  int bufUseDim[maxDsps];
  char bufDisplayFileExt[maxDsps][15+1];
} bufType, *bufPtr;

int numDsps, dspIndex;

bufPtr buf;

activeWindowClass *aw;
int needClose, needResolvePvName, needConnect,
 needRtypeConnect, needTimeout, needRtypeTimeout;

int topShadowColor;
int botShadowColor;
pvColorClass fgColor, bgColor;
colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;
int noEdit;

int setPostion[maxDsps];
int allowDups[maxDsps];
int propagateMacros[maxDsps];

expStringClass displayFileName[maxDsps];
expStringClass displayFileExt[maxDsps];

expStringClass symbolsExpStr[maxDsps];
char symbols[maxDsps][255+1];

int replaceSymbols[maxDsps]; // else append

expStringClass buttonLabel;

expStringClass label[maxDsps];

fontMenuClass fm;
char fontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

int opComplete;

int activeMode;

Widget popUpMenu, pullDownMenu, pb[maxDsps];

entryFormClass *ef1;

int posX, posY;

int ofsX, ofsY;

int useAnyRtype;
int useRtype[maxDsps];
int useType[maxDsps];
int useSpecType[maxDsps];
int useDim[maxDsps];
Widget tf_widget;
int widget_value_changed;
char entryValue[PV_Factory::MAX_PV_NAME+1];
char rtypeFieldName[PV_Factory::MAX_PV_NAME+1];
char rtype[63+1];
int grabUpdate;
ProcessVariable *pvId, *rtypePvId;
int resolvingName;
XtIntervalId unconnectedTimer;
XtIntervalId rtypeUnconnectedTimer;
msgDialogClass msgDialog;
int msgDialogPoppedUp;
int pvType;
int pvSpecificType;
int isVector;
int pvConnected;
int rtypePvConnected;
int displayOpen;

public:

pvInspectorClass ( void );

pvInspectorClass
 ( const pvInspectorClass *source );

~pvInspectorClass ( void );

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

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int draw ( void );

int erase ( void );

int drawActive ( void );

int eraseActive ( void );

int activate (
  int pass,
  void *ptr );

int deactivate ( int pass );

void updateDimensions ( void );

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

void btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

char *vectorId (
  int isVector );

char *pvSpecificTypeName (
  int pvSpecificTypeNum );

char *pvTypeName (
  int pvTypeNum );

void popupDisplay (
  int index );

void btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

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

void pointerIn (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState );

void pointerOut (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState );

void mousePointerIn (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState );

void mousePointerOut (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState );

 void executeDeferred ( void );

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_pvInspectorClassPtr ( void );
void *clone_pvInspectorClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
