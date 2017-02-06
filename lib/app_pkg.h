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
#include <Xm/SelectioB.h>
#include <Xm/MessageB.h>
#include <unordered_map>

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
#include "edmPrint.h"
#include "clipbd.h"

class pathListClass;
#include "path_list.h"

#include "thread.h"
#include "avl.h"

#include "pv_factory.h"
#include "cvtFast.h"

#include "app_pkg.str"
#include "environment.str"

#define MAX_LINE 1001
#define MAX_MACROS 101
#define MAX_NAME 101
#define MAX_DIR 206
#define MAX_FNAME 311

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
#define WINNAME_MAX 63

static const char *dummyWinName = "";

//////////////////////    start of class declaration: activeWindowListType

class activeWindowListType {

public:

  activeWindowListType() {
    winName = NULL;
  }

  ~activeWindowListType() {
    if ( winName ) delete[] winName;
  }

  int simpleMatch (
    char *pattern
  ) {

  int lp, ls, ends, begins, start;
  char buf[127+1], *ptr;

    // *<pattern> : winName ends in pattern
    // <pattern>* : winName begins with pattern
    // <pattern>  : winName exactly matches pattern

    if ( !pattern ) return 0;
    lp = strlen(pattern);
    if ( lp == 0 ) return 0; // empty pattern disallowed

    if ( !winName ) return 0;
    ls = strlen(winName);
    if ( ls == 0 ) return 0;

    if ( lp == ls ) {
      if ( strcmp( winName, pattern ) == 0 ) return 1;
    }

    ends = 0;
    begins = 0;

    if ( pattern[0] == '*' ) {
      ends = 1;
    }
    else if ( pattern[lp-1] == '*' ) {
      begins = 1;
    }
    else {
      return 0; // no match
    }

    if ( strcmp( pattern, "*" ) == 0 ) {
      return 1;
    }
    else {

      if ( begins ) {

        strncpy( buf, pattern, 127 );
        buf[lp-1] = 0;                    // discard last char
        ptr = strstr( winName, buf );
        if ( ptr == winName )
          return 1;
        else
          return 0;

      }
      else if ( ends ) {

        strncpy( buf, &pattern[1], 127 ); // discard first char
        start = ls - strlen(buf);
        if ( start < 0 ) return 0;
        ptr = strstr( &winName[start], buf );
        if ( ptr )
          return 1;
        else
          return 0;

      }

    }

    return 0;

  }

  void clearWinName ( void ) {
    if ( winName ) {
      strcpy( winName, "" );
    }
  }

  void setWinName (
    char *str
  ) {
    int l;
    if ( str && !winName ) { // winName may be set only once
      l = strlen(str);
      if ( l > WINNAME_MAX ) l = WINNAME_MAX;
      winName = new char[l+1];
      strncpy( winName, str, l );
      winName[l] = 0;
    }
  }

  const char *getWinName( void ) {
    if ( !winName ) {
      return dummyWinName;
    }
    else {
      return winName;
    }
  }

  activeWindowListType *flink;
  activeWindowListType *blink;
  activeWindowClass node;
  int requestDelete;
  int requestOpen;
  int requestActivate;
  int requestActivateClear;
  int requestReactivate; // for multiplexors
  int requestClose;
  int requestRefresh;
  int requestActiveRedraw;
  int requestPosition;
  int requestImport;
  int requestIconize;
  int requestConvertAndExit;
  int x;
  int y;
  char *winName;
  //char winName[WINNAME_MAX+1];

};

/////////// end of class declaration: activeWindowListType

typedef activeWindowListType *activeWindowListPtr;

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

// The following structure is for adding actions to an X application context
typedef struct actionsTag {
  void *key; // any unique address
  struct actionsTag *flink;
} actionsType, *actionsPtr;

//////////////////////    start of class declaration: appContextClass

class appContextClass {

private:

friend void setPath_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void selectPath_cb (
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

friend void dont_shutdown_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void do_shutdown_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void shutdown_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ctlPvMonitorConnection (
  ProcessVariable *pv,
  void *userarg );

friend void ctlPvUpdate (
  ProcessVariable *pv,
  void *userarg );

friend void new_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void open_from_path_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void open_cb (
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

friend void renderImages_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void checkpointPid_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void viewFontMapping_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void viewEnv_cb (
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

friend void app_fileSelectFromPathOk_cb (
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

Widget appTop, fileSelectFromPathBox, fileSelectBox, importSelectBox,
 mainWin, menuBar, filePullDown,
 fileCascade, newB, openB, exitB, viewPullDown, viewCascade, msgB, pvB,
 mainDrawingArea, pathPullDown, pathCascade, helpPullDown, helpCascade;
XtAppContext app;
Display *display;
char colormode[7+1]; // index (default) or rgb
int privColorMap;
int exitOnLastClose, atLeastOneOpen;

// for screen configuration save/load
int screenAdd;
int screenAddAll;
char *confOk;
int confOkCount;
char cfgName[MAX_FNAME];

XEvent event;

pathListClass pathList;

scrolledTextClass msgBox;
scrolledListClass pvList;

int executeCount;
int isActive;
int requestFlag;
int iconified;
int firstOpen;

THREAD_HANDLE threadHandle;

char ctlPV[127+1];
char userLib[127+1];

ProcessVariable *ctlPvId;
int initialConnection;

confirmDialogClass confirm;
int local;

msgDialogClass msgDialog;
int msgDialogOpenCount;

THREAD_LOCK_HANDLE actionsLock;
actionsPtr actHead, actTail;

int useStdErrFlag;
char *errMsgPrefix;

public:

macroListPtr macroHead;
int numFiles;
fileListPtr fileHead;

char displayName[63+1];

int numSchemeSets;
char **schemeSetList;
displaySchemeClass displayScheme;

int numPaths;
char **dataFilePrefix;
char curPath[127+1];

char colorPath[127+1];

int viewXy;
Widget viewXyB;

int renderImagesFlag;
Widget renderImagesB;

Widget checkpointPidB;

Widget viewFontMappingB;

Widget viewEnvB;

ulBindingClass userLibObject;
int exitFlag;
int objDelFlag;
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

int displayH, displayW;

int entryFormX;
int entryFormY;
int entryFormW;
int entryFormH;
int largestH;

int shutdownFlag;
int saveContextOnExit;
int primaryServer;
int oneInstance;
FILE *shutdownFilePtr;

int reloadFlag;

edmPrintClass epc;

clipBdClass clipBd;

int convertOnly;

// group visibility info
int haveGroupVisInfo;
expStringClass curGroupVisPvExpStr;
int curGroupVisInverted;
char curGroupMinVisString[39+1];
char curGroupMaxVisString[39+1];

int syncOnce;

GC ddgc; // for drag and drop
XFontStruct *ddFixedFont; // for drag and drop

int useScrollBars;

appContextClass (
  void );

~appContextClass (
  void );

void closeDownAppCtx ( void );

int getFilePaths ( void );

void expandFileName (
  int index,
  char *expandedName,
  char *inName,
  char *ext,
  int maxSize );

void expandFileName (
  int index,
  char *expandedName,
  char *inName,
  int maxSize );

void buildSchemeList ( void );

void destroySchemeList ( void );

void getScheme (
  char *schemeSetName,
  char *objName,
  char *objType,
  char *schemeFileName,
  int maxLen );

int schemeExists (
  char *schemeSetName,
  char *objName,
  char *objType );

int setProperty (
  char *winId,
  char *id,
  char *property,
  char *value );

int setProperty (
  char *winId,
  char *id,
  char *property,
  double *value );

int setProperty (
  char *winId,
  char *id,
  char *property,
  int *value );

int getProperty (
  char *winId,
  char *id,
  char *property,
  int bufSize,
  char *value );

int getProperty (
  char *winId,
  char *id,
  char *property,
  double *value );

int getProperty (
  char *winId,
  char *id,
  char *property,
  int *value );

int initDeferredExecutionQueue ( void );

void termDeferredExecutionQueue ( void );

void removeAllDeferredExecutionQueueNode (
  class activeWindowClass *awo );

void processDeferredExecutionQueue ( void );

void postDeferredExecutionQueue (
  class activeGraphicClass *ptr );

void postDeferredExecutionQueue (
  class activeWindowClass *ptr );

void postDeferredExecutionNextQueue (
  class activeGraphicClass *ptr );

void postDeferredExecutionNextQueue (
  class activeWindowClass *ptr );

Display *getDisplay ( void ) {

  return display;

}

THREAD_HANDLE getThreadHandle ( void ) {

  return threadHandle;

}

void createMainWindow ( void );

void addActiveWindow (
  activeWindowListPtr node );

int refreshActiveWindow (
  activeWindowClass *activeWindowNode );

int smartDrawAllActive (
  activeWindowClass *activeWindowNode );

int removeActiveWindow (
  activeWindowClass *activeWindowNode );

int openEditActiveWindow (
  activeWindowClass *activeWindowNode );

int openActivateActiveWindow (
  activeWindowClass *activeWindowNode );

int openActivateActiveWindow (
  activeWindowClass *activeWindowNode,
  int x,
  int y );

int openActivateIconifiedActiveWindow (
  activeWindowClass *activeWindowNode,
  int x,
  int y );

int activateActiveWindow (
  activeWindowClass *activeWindowNode );

int reactivateActiveWindow (
  activeWindowClass *activeWindowNode );

int getParams(
  int argc,
  char **argv );

int startApplication (
  int argc,
  char **argv,
  int _primaryServer );

int startApplication (
  int argc,
  char **argv,
  int _primaryServer,
  int _oneInstance,
  int _convertOnly );

void openInitialFiles ( void );

void openFiles (
  char *list );

void controlWinNames (
  char *cmd,
  char *list );

int addActWin (
  char *name,
  int x,
  int y,
  int numMacs,
  char **syms,
  char **exps );

void applicationLoop ( void );

XtAppContext appContext ( void );

Widget fileSelectFromPathBoxWidgetId ( void );

Widget fileSelectBoxWidgetId ( void );

Widget importSelectBoxWidgetId ( void );

void setErrMsgPrefix (
  char *prefix
);

void useStdErr (
  int flag
);

void postMessage (
  char *msg );

void raiseMessageWindow ( void );

void iconifyMainWindow ( void );

void deiconifyMainWindow ( void );

void lock ( void );

void unlock ( void );

void reopenUserLib ( void );

void xSynchronize (
  int onoff );

void exitProgram ( void );

void findTop ( void );

void postNote ( 
  char *msg );

void closeNote ( void );

int numScreens ( void );

void performShutdown (
  FILE *f );

int getShutdownFlag ( void );

void reloadAll ( void );

void requestSelectedReload ( void );

void reloadSelected ( void );

void refreshAll ( void );

int renderImages ( void );

void setRenderImages (
  int flag
);

int openCheckPointScreen (
  char *screenName,
  int x,
  int y,
  int icon,
  int noEdit,
  int numCheckPointMacros,
  char *checkPointMacros
);

int okToExit ( void );

void addActions (
  XtActionsRec *actions, // actions must be a unique static address
  Cardinal n
);

void showEnv ( void );

Widget apptop ( void );

int screenAlreadyUp(unordered_map<string, string>& sigs, 
  char *edlname, 
  char **macros, 
  char **values, 
  int nmacros);
  
void closeAllButHead();
int screenConfigOk(FILE *fp);
void getScreenSignatures(unordered_map<string, string> &sigs);
void setScreenAdd(int add) { screenAdd = add; }
int getScreenAdd(void) { return screenAdd; }
void setScreenAddAll(int add) { screenAddAll = add; }
int getScreenAddAll(void) { return screenAddAll; }
char *getConfOk(void) { return confOk; }
int getCfgDirectory(char *pfilter, char *pmsg);
void setCfgName(char *fname) { strcpy(cfgName, fname); }
char *getCfgName(void) { return cfgName; }
int writeConfig(char *fname);
char *checkCfgName(char *fname);
};
/////////////////// end of class declaration: appContextClass
#endif
