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

#ifndef __related_display_h
#define __related_display_h 1

#include "act_grf.h"
#include "entry_form.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define RDC_ORIG_POS 0
#define RDC_BUTTON_POS 1
#define RDC_PARENT_OFS_POS 2

#define RDC_MAJOR_VERSION 4
#define RDC_MINOR_VERSION 4
#define RDC_RELEASE 0

typedef struct objAndIndexTag {
  void *obj;
  int index;
} objAndIndexType;

#ifdef __related_display_cc

#include "related_display.str"

static void doBlink (
  void *ptr
);

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void relDsp_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void relDsp_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void relDsp_color_value_update (
  ProcessVariable *pv,
  void *userarg );

static void rdc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void rdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class relatedDisplayClass : public activeGraphicClass {

public:

static const int NUMPVS = 4;
static const int maxDsps = 24;
static const int maxSymbolLen = 2550;

private:

friend void doBlink (
  void *ptr
);

friend void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id );

friend void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void relDsp_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void relDsp_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void relDsp_color_value_update (
  ProcessVariable *pv,
  void *userarg );

friend void openDisplay (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_update1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_apply1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_cancel1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void rdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

typedef struct bufTag {
  int bufUseFocus;
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufTopShadowColor;
  int bufBotShadowColor;
  int bufFgColor;
  int bufBgColor;
  int bufInvisible;
  int bufNoEdit;
  int bufCloseAction[maxDsps];
  int bufSetPostion[maxDsps];
  int bufAllowDups[maxDsps];
  int bufCascade[maxDsps];
  int bufPropagateMacros[maxDsps];
  char bufDisplayFileName[maxDsps][127+1];
  char bufSymbols[maxDsps][maxSymbolLen+1];
  int bufReplaceSymbols[maxDsps];
  char bufButtonLabel[127+1];
  char bufLabel[maxDsps][127+1];
  char bufFontTag[63+1];
  char bufColorPvName[PV_Factory::MAX_PV_NAME+1];
  char bufDestPvName[NUMPVS][PV_Factory::MAX_PV_NAME+1];
  char bufSource[NUMPVS][39+1];
  int bufOfsX;
  int bufOfsY;
  int bufButton3Popup;
  int bufIcon;
  int bufSwapButtons;
  char bufHelpCommand[255+1];
} bufType, *bufPtr;

colorButtonClass fgCb, bgCb, topShadowCb, botShadowCb;

int numDsps, dspIndex;

bufPtr buf;

entryListBase *pvEntry[NUMPVS], *valEntry[NUMPVS],
 *fileEntry[maxDsps], *labelEntry[maxDsps], *macrosEntry[maxDsps], *modeEntry[maxDsps],
 *propagateEntry[maxDsps], *positionEntry[maxDsps], *xOfsEntry[maxDsps], *yOfsEntry[maxDsps],
 *closeCurEntry[maxDsps], *dupsAllowedEntry[maxDsps];

activeWindowClass *aw;
int useFocus, needClose, needConnect, needUpdate, needRefresh;
int needToDrawUnconnected, needToEraseUnconnected;
XtIntervalId unconnectedTimer;

int topShadowColor;
int botShadowColor;
pvColorClass fgColor, bgColor;
int invisible, noEdit;

int closeAction[maxDsps];
int setPostion[maxDsps];
int allowDups[maxDsps];
int cascade[maxDsps];
int propagateMacros[maxDsps];

expStringClass displayFileName[maxDsps];

expStringClass symbolsExpStr[maxDsps];

int replaceSymbols[maxDsps]; // else append

expStringClass buttonLabel;

expStringClass label[maxDsps];

fontMenuClass fm;
char fontTag[63+1];
XmFontList fontList;
XFontStruct *fs;
int fontAscent, fontDescent, fontHeight;

ProcessVariable *colorPvId, *destPvId[NUMPVS];
int initialConnection[NUMPVS];

objAndIndexType objAndIndex[NUMPVS];

int opComplete[NUMPVS], singleOpComplete, colorExists, destExists[NUMPVS],
 atLeastOneExists, destType[NUMPVS];

pvConnectionClass connection;

expStringClass colorPvExpString;

expStringClass destPvExpString[NUMPVS];

expStringClass sourceExpString[NUMPVS];

int activeMode, active, init;

Widget popUpMenu, pullDownMenu, pb[maxDsps];

entryFormClass *ef1;

int posX, posY;

int ofsX, ofsY;

int button3Popup;

int icon;

int swapButtons;

expStringClass helpCommandExpString;
int helpItem, numMenuItems;

void setHelpItem ( void );

public:

relatedDisplayClass ( void );

relatedDisplayClass
 ( const relatedDisplayClass *source );

~relatedDisplayClass ( void );

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

int createSpecial (
  char *fname,
  activeWindowClass *_actWin );

void sendMsg (
  char *param );

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

int isRelatedDisplay ( void );

void augmentRelatedDisplayMacros (
  char *buf
);
 
int getNumRelatedDisplays ( void );

int getRelatedDisplayProperty (
  int index,
  char *key
);

char *getRelatedDisplayName (
  int index
);

char *getRelatedDisplayMacros (
  int index
);

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

void popupDisplay (
  int index );

void btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

#ifdef TRIUMF
#endif

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

void *create_relatedDisplayClassPtr ( void );
void *clone_relatedDisplayClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
