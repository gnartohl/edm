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

#ifndef __app_pkg_h
#define __app_pkg_h 1

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/FileSB.h>

#include "color_pkg.h"
#include "color_button.h"
#include "font_pkg.h"
#include "font_menu.h"
#include "gc_pkg.h"
#include "entry_form.h"
#include "confirm_dialog.h"
#include "scrolled_text.h"
#include "scrolled_list.h"
#include "msg_dialog.h"

#include "act_win.h"
#include "act_grf.h"
#include "bindings.h"
#include "ulBindings.h"
#include "scheme.h"
#include "process.h"

#include "thread.h"
#include "avl.h"

#include "app_pkg.str"
#include "environment.str"

typedef struct callbackBlockTag {
  struct callbackBlockTag *flink;
  void *ptr;
  class appContextClass *apco;
} callbackBlockType, *callbackBlockPtr;

typedef struct schemeListTag {
  AVL_FIELDS(schemeListTag)
  char *objName;
  char *fileName;
} schemeListType, *schemeListPtr;

typedef struct appDefExe_node_tag { /* locked queue node */
  void *flink;
  void *blink;
  class activeWindowClass *awObj;
  class activeGraphicClass *obj;
} APPDEFEXE_NODE_TYPE, *APPDEFEXE_NODE_PTR;

typedef struct appDefExe_que_tag { /* locked queue header */
  void *flink;
  void *blink;
  void *lock;
  class activeWindowClass *awObj;
  class activeGraphicClass *obj;
} APPDEFEXE_QUE_TYPE, *APPDEFEXE_QUE_PTR;

#define REMQHI( queue, buf, flag )\
  sys_remqh( (void *) (queue), (void **) (buf), (int) (flag) )

#define INSQTI( buf, queue, flag )\
  sys_insqt( (void *) (buf), (void *) (queue), (int) (flag) )

#define QUEWASEMP SYS_QUEWASEMP
#define ONEENTQUE SYS_ONEENTQUE

#define APPDEFEXE_QUEUE_SIZE 1000

typedef struct activeWindowListTag {
  struct activeWindowListTag *flink;
  struct activeWindowListTag *blink;
  activeWindowClass node;
  int requestDelete;
  int requestOpen;
  int requestActivate;
  int requestReactivate; // for multiplexors
  int requestClose;
  int requestRefresh;
  int requestPosition;
  int requestCascade;
  int requestImport;
  int x;
  int y;
} activeWindowListType, *activeWindowListPtr;

typedef struct macroListTag {
  struct macroListTag *flink;
  struct macroListTag *blink;
  char *macro;
  char *expansion;
} macroListType, *macroListPtr;

typedef struct fileListTag {
  struct fileListTag *flink;
  struct fileListTag *blink;
  char *file;
} fileListType, *fileListPtr;


class appContextClass {

private:

friend void setPath_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void continue_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void abort_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

#ifdef __epics__
friend void ctlPvUpdate (
  struct event_handler_args ast_args );
#endif

#ifdef GENERIC_PV
friend void ctlPvUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args );
#endif

friend void new_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void open_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void open_user_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void import_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void exit_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void view_pvList_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void view_msgBox_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void help_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void app_fileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void app_importSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend class activeWindowClass;

callbackBlockPtr callbackBlockHead, callbackBlockTail;

AVL_HANDLE schemeList, schemeSet;
int schemeListExists;

APPDEFEXE_QUE_TYPE appDefExeFreeQueue, appDefExeActiveQueue,
 appDefExeActiveNextQueue;
APPDEFEXE_NODE_TYPE appDefExeNodes[APPDEFEXE_QUEUE_SIZE+1];

Widget appTop, fileSelectBox, importSelectBox, mainWin, menuBar, filePullDown,
 fileCascade, newB, openB, exitB, viewPullDown, viewCascade, msgB, pvB,
 mainDrawingArea, pathPullDown, pathCascade, helpPullDown, helpCascade;
XtAppContext app;
Display *display;
char displayName[31+1];
XEvent event;

scrolledTextClass msgBox;
scrolledListClass pvList;

int executeCount;
int isActive;
int requestFlag;
int iconified;

THREAD_HANDLE threadHandle;

macroListPtr macroHead;
int numFiles;
fileListPtr fileHead;

char ctlPV[127+1];
char userLib[127+1];

#ifdef __epics__

chid ctlPvId;
evid ctlPvEventId;

#else

#ifdef GENERIC_PV

pvBindingClass pvObj;
char pvClassName[15+1];
pvClass *ctlPvId;
pvEventClass *ctlPvEventId;

#endif

#endif

confirmDialogClass confirm;
int local;

msgDialogClass msgDialog;

public:

int numSchemeSets;
char **schemeSetList;
displaySchemeClass displayScheme;

int numPaths;
char **dataFilePrefix;
char curPath[127+1];

char colorPath[127+1];

int viewXy;
Widget viewXyB;

ulBindingClass userLibObject;
int exitFlag;
int watchdog;
processClass *proc;
int iconTestCount;

activeGraphicListPtr cutHead1;

fontInfoClass fi;
colorInfoClass ci;

int executeOnOpen;
int noEdit;
int numMacros;
char **macros;
char **expansions;

int usingControlPV;

activeWindowListPtr head;

int entryFormX;
int entryFormY;
int entryFormW;
int entryFormH;
int largestH;

appContextClass::appContextClass (
  void );

appContextClass::~appContextClass (
  void );

void appContextClass::getFilePaths ( void );

void appContextClass::expandFileName (
  int index,
  char *expandedName,
  char *inName,
  char *ext,
  int maxSize );

void appContextClass::buildSchemeList ( void );

void appContextClass::destroySchemeList ( void );

void appContextClass::getScheme (
  char *schemeSetName,
  char *objName,
  char *objType,
  char *schemeFileName,
  int maxLen );

int appContextClass::setProperty (
  char *winId,
  char *id,
  char *property,
  char *value );

int appContextClass::setProperty (
  char *winId,
  char *id,
  char *property,
  double *value );

int appContextClass::setProperty (
  char *winId,
  char *id,
  char *property,
  int *value );

int appContextClass::getProperty (
  char *winId,
  char *id,
  char *property,
  int bufSize,
  char *value );

int appContextClass::getProperty (
  char *winId,
  char *id,
  char *property,
  double *value );

int appContextClass::getProperty (
  char *winId,
  char *id,
  char *property,
  int *value );

int appContextClass::initDeferredExecutionQueue ( void );

void appContextClass::removeAllDeferredExecutionQueueNode (
  class activeWindowClass *awo );

void appContextClass::processDeferredExecutionQueue ( void );

void appContextClass::postDeferredExecutionQueue (
  class activeGraphicClass *ptr );

void appContextClass::postDeferredExecutionQueue (
  class activeWindowClass *ptr );

void appContextClass::postDeferredExecutionNextQueue (
  class activeGraphicClass *ptr );

void appContextClass::postDeferredExecutionNextQueue (
  class activeWindowClass *ptr );

Display *appContextClass::getDisplay ( void ) {

  return display;

}

THREAD_HANDLE appContextClass::getThreadHandle ( void ) {

  return threadHandle;

}

void appContextClass::createMainWindow ( void );

void appContextClass::addActiveWindow (
  activeWindowListPtr node );

int appContextClass::refreshActiveWindow (
  activeWindowClass *activeWindowNode );

int appContextClass::removeActiveWindow (
  activeWindowClass *activeWindowNode );

int appContextClass::openEditActiveWindow (
  activeWindowClass *activeWindowNode );

int appContextClass::openActivateActiveWindow (
  activeWindowClass *activeWindowNode );

int appContextClass::openActivateActiveWindow (
  activeWindowClass *activeWindowNode,
  int x,
  int y );

int appContextClass::openActivateCascadeActiveWindow (
  activeWindowClass *activeWindowNode );

int appContextClass::openActivateCascadeActiveWindow (
  activeWindowClass *activeWindowNode,
  int x,
  int y );

int appContextClass::activateActiveWindow (
  activeWindowClass *activeWindowNode );

int appContextClass::reactivateActiveWindow (
  activeWindowClass *activeWindowNode );

int appContextClass::getParams(
  int argc,
  char **argv );

int appContextClass::startApplication (
  int argc,
  char **argv );

void appContextClass::applicationLoop ( void );

XtAppContext appContextClass::appContext ( void );

Widget appContextClass::fileSelectBoxWidgetId ( void );

Widget appContextClass::importSelectBoxWidgetId ( void );

void appContextClass::postMessage (
  char *msg );

void appContextClass::iconifyMainWindow ( void );

void appContextClass::deiconifyMainWindow ( void );

void appContextClass::lock ( void );

void appContextClass::unlock ( void );

void appContextClass::reopenUserLib ( void );

void appContextClass::xSynchronize (
  int onoff );

void appContextClass::exitProgram ( void );

void appContextClass::findTop ( void );

void appContextClass::postNote ( 
  char *msg );

void appContextClass::closeNote ( void );

};

#endif
