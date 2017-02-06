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

#ifndef __include_widget_h
#define __include_widget_h 1

#include <list>
#include <string>
#include "act_grf.h"
#include "entry_form.h"

// the following defines btnActionListType & btnActionListPtr
#include "btnActionListType.h"

#include "pv_factory.h"
#include "cvtFast.h"

#define IW_ORIG_POS 0
#define IW_BUTTON_POS 1
#define IW_PARENT_OFS_POS 2

#define IW_MAJOR_VERSION 4
#define IW_MINOR_VERSION 4
#define IW_RELEASE 0

using namespace std;


typedef struct objAndIndexTag {
  void *obj;
  int index;
} objAndIndexType;

#ifdef __include_widget_cc

#include "include_widget.str"

static void doBlink (
  void *ptr
);

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void incW_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void incW_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

static void incW_color_value_update (
  ProcessVariable *pv,
  void *userarg );

static void iw_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

static void iw_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

static void iw_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

static void iw_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

static void iw_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

static void iw_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

//////////////// start class declaration: includeWidgetClass

class includeWidgetClass : public activeGraphicClass {

public:

static const int NUMPVS = 4;
static const int maxSymbolLen = 2550;

private:

friend void doBlink (
  void *ptr
);

friend void incW_monitor_dest_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void incW_monitor_color_connect_state (
  ProcessVariable *pv,
  void *userarg );

friend void incW_color_value_update (
  ProcessVariable *pv,
  void *userarg );

friend void openDisplay (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void iw_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void iw_edit_update1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void iw_edit_apply1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void iw_edit_cancel1 (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void iw_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void iw_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void iw_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void iw_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void iw_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call );

btnActionListPtr btnDownActionHead;
btnActionListPtr btnUpActionHead;
btnActionListPtr btnMotionActionHead;
btnActionListPtr btnFocusActionHead;

typedef struct bufTag {
  int bufX;
  int bufY;
  int bufW;
  int bufH;
  int bufFgColor;
  colorButtonClass fgCb;
  int bufDrawFrame;
  char bufIncludeFileName[127+1];
  char bufSymbols[maxSymbolLen+1];
  char bufHelpCommand[255+1];
} bufType, *bufPtr;

bufPtr buf;

void *voidHead; // cast to activeGraphicListPtr at runtime
void * curCrawlerNode; // cast to activeGraphicListPtr at runtime
int curCrawlerState;

enum CrawlerStates { GETTING_FIRST_CRAWLER_PV, GETTING_NEXT_CRAWLER_PV,
 NO_MORE_CRAWLER_PVS };

typedef struct relatedDisplayNodeList {
  void *ptr; // cast to activeGraphicListPtr at runtime
  int first;  // index of first related screen
  int last;   // index of last related screen
  struct relatedDisplayNodeList *flink;
  struct relatedDisplayNodeList *blink;
} RelatedDisplayNodeType, *RelatedDisplayNodePtr;
RelatedDisplayNodePtr relatedDisplayNodeHead;

 entryListBase  *fileEntry, *macrosEntry, *drawFrameEntry;

activeWindowClass *aw;
int  needClose, needConnect, needUpdate, needRefresh;
int needToDrawUnconnected, needToEraseUnconnected;

pvColorClass fgColor;

 int drawFrame;

static int debug;    // other debug
static int debugm;   // debug macros
static int debugr;   // debug recursive

// for recursive include blocking
std::list<string> parentList;
static string repeatFileName;
static int includeLevel;
static int doubleLevel;
static int existLevel;

char includeFileName[127+1];

expStringClass symbolsExpStr;

//?????
ProcessVariable *colorPvId, *destPvId[NUMPVS];
int initialConnection[NUMPVS];

objAndIndexType objAndIndex[NUMPVS];

 int opComplete1, op2Complete1, opComplete[NUMPVS], singleOpComplete, colorExists, destExists[NUMPVS],
 atLeastOneExists, destType[NUMPVS];

pvConnectionClass connection;

expStringClass colorPvExpString;

expStringClass destPvExpString[NUMPVS];

expStringClass sourceExpString[NUMPVS];

int activeMode, active, init;

Widget popUpMenu, pullDownMenu, pb;

entryFormClass *ef1;

int posX, posY;

int ofsX, ofsY;

int icon;

int swapButtons;

expStringClass helpCommandExpString;
int helpItem, numMenuItems;

void setHelpItem ( void );

public:

includeWidgetClass ( void );

includeWidgetClass
 ( const includeWidgetClass *source );

~includeWidgetClass ( void );

char *objName ( void ) {

  return name;

}

int isIncludeWidget ( void ) { return 1; }

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

//
int createFromFile (
  FILE *fptr,
  char *name,
  activeWindowClass *actWin,
  std::list<string>& parentList);

int createSpecial (
  char *fname,
  activeWindowClass *_actWin );

int genericEdit ( void );

int edit ( void );

int editCreate ( void );

int readIncludeFile( std::list<string> parentList);

int draw ( void );

int erase ( void );

activeGraphicClass *enclosingObject (
  int _x,
  int _y );

int doSmartDrawAllActive ( void );

int drawActiveIfIntersects (
  int x0,
  int y0,
  int x1,
  int y1 );

int smartDrawCount ( void );

void resetSmartDrawCount ( void );

void bufInvalidate ( void );

int drawActive ( void );

int eraseActive ( void );

int readSymbolFile ( void );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag );

int getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus );

void btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action );

void btnDrag (
  XMotionEvent *me,
  int x,
  int y,
  int buttonState,
  int buttonNumber );

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

void checkMouseOver (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState );

int activateBeforePreReexecuteComplete ( void );

int preReactivate (
  int pass,
  int *numSubObjects );

int reactivate (
  int pass,
  void *ptr,
  int *numSubObjects );

int activateComplete ( void );
 
int activate (
  int pass,
  void *ptr,
  int *numSubObjects );

int deactivate (
  int pass,
  int *numSubObjects );

void updateGroup ( void );
 
int moveSelectBox (
  int _x,
  int _y );

int moveSelectBoxAbs (
  int _x,
  int _y );

int moveSelectBoxMidpointAbs (
  int _x,
  int _y );

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

int move (
  int x,
  int y );

int moveAbs (
  int x,
  int y );

int moveMidpointAbs (
  int x,
  int y );

int getIncludeWidgetProperty (
  char *key
);

char *getIncludeFileName ();

char *getIncludeWidgetMacros ();

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

void getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n );

int showPvInfo (
  XButtonEvent *be,
  int x,
  int y );

void changeWidgetParams (
  unsigned int flag,
  int alignment,
  int ctlAlignment,
  int btnAlignment,
  int textFgColor,
  int fg1Color,
  int fg2Color,
  int offsetColor,
  int bgColor,
  int topShadowColor,
  int botShadowColor );

int initDefExeNode (
  void *ptr );

void executeDeferred ( void );

int startDrag (
  XButtonEvent *be,
  int x,
  int y );

int selectDragValue (
  XButtonEvent *be );

char *firstDragName (
  int x,
  int y
);

char *nextDragName (
  int x,
  int y
);

char *dragValue (
  int x,
  int y,
  int i
);

int atLeastOneDragPv (
  int x,
  int y
);

void initEnable ( void );

void enable ( void );

void disable ( void );

char *crawlerGetFirstPv ( void );

char *crawlerGetNextPv ( void );

std::list<string>& getParentList();

int isRelatedDisplay ( void );

int getNumRelatedDisplays ( void );

int getRelatedDisplayProperty (
  int index,
  char *name
);

char *getRelatedDisplayName (
  int index
);

char *getRelatedDisplayMacros (
  int index
);

char *getSearchString (
  int i
);

void replaceString (
  int i,
  int max,
  char *string
);

int makeNewMacroSet(
  char *macrodef, 
  int numMacros,
  char **macros,
  char **values, 
  int *numNewMacros, 
  int *numNewAlloc, 
  char **newMacros,
  char **newValues,
  int max_macros
);

};

#ifdef __cplusplus
extern "C" {
#endif

void *create_includeWidgetClassPtr ( void );
void *clone_includeWidgetClassPtr ( void * );

#ifdef __cplusplus
}
#endif

#endif
