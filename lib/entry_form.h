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

#ifndef __entry_form_h
#define __entry_form_h 1

#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/BulletinB.h>
#include <Xm/DrawingA.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleB.h>
#include <Xm/ArrowBG.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/DialogS.h>
#include <Xm/ScrolledW.h>
#include <Xm/PanedW.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/Separator.h>

#include "utility.h"
#include "color_pkg.h"
#include "color_button.h"
#include "font_pkg.h"
#include "font_menu.h"

#define EF_K_RECTANGULAR 1
#define EF_K_MULTIPOINT 2

typedef struct efWidgetAndPointerTag {
  Widget w;
  void *obj;
  XtPointer client;
} efWidgetAndPointerType, *efWidgetAndPointerPtr;

typedef struct efSetItemCallbackDscTag {
  void *obj;
  void *ef;
} efSetItemCallbackDscType, *efSetItemCallbackDscPtr;

// this structure is used to pass the address of an array and an address
// of an array index to an X callback function
typedef struct efArrayCallbackDscTag {
  int size;
  int *indexPtr;
  void *destPtr;
  void *valuePtr;
} efArrayCallbackDscType, *efArrayCallbackDscPtr;

typedef struct widgetListTag {
  struct widgetListTag *flink;
  Widget w;
  char *value;
  void *destination;
  int size;
  int entryNumber;
  efArrayCallbackDscType arrayDsc;
} widgetListType, *widgetListPtr;

class efInt {

private:

int val;
int null;

public:

efInt ( void ) {

  null = 1;
  val = 0;

}

~efInt ( void ) { }

int value ( void ) { return val; }

void setValue ( int v ) { val = v; null = 0; }

int isNull ( void ) { return null; }

void setNull ( int n ) { null = n; }

int write ( FILE *f ) {

int stat, v, n;

  v = value();
  n = isNull();
  stat = fprintf( f, "%-d %-d\n", v, n );

  if ( stat < 0 )
    return 0;
  else
    return 1;

}

int read ( FILE *f ) {

int stat, v, n;

  stat = fscanf( f, "%d %d\n", &v, &n );

  if ( stat < 0 ) return 0;

  setValue( v );
  setNull( n );

  return 1;

}

};

class efDouble {

private:

double val;
int null;

public:

efDouble ( void ) {

  null = 1;
  val = 0.0;

}

~efDouble ( void ) { }

double value ( void ) { return val; }

void setValue ( double v ) { val = v; null = 0; }

int isNull ( void ) { return null; }

void setNull ( int n ) { null = n; }

int write ( FILE *f ) {

int stat;
double v, n;

  v = value();
  n = isNull();
  stat = fprintf( f, "%-g %-g\n", v, n );

  if ( stat < 0 )
    return 0;
  else
    return 1;

}

int read ( FILE *f ) {

int stat;
double v, n;

  stat = fscanf( f, "%lg %lg\n", &v, &n );

  if ( stat < 0 ) return 0;

  setValue( v );
  setNull( (int) n );

  return 1;

}

};

class entryListBase;

typedef struct entryRecTag {
  entryListBase *entry;
  int sense;
} entryRecType, *entryRecPtr;

class entryListBase { // base class for all items that may be placed
                      // on a entry form

public:

efArrayCallbackDscType arrayDsc;
entryListBase *flink;
Widget labelW, activeW;

entryListBase ( void ) {

  haveCallback = 0;
  numDepend = 0;
  numValues = 0; // for multi-value widgets like option menus

}

virtual ~entryListBase ( void ) { }

virtual void cleanup ( void ) { }

virtual void setValue ( int value ) { }

virtual void setValue ( double value ) { }

virtual void setValue ( char *value ) { }

virtual void setNumValues ( int n ) {
  numValues = n;
}

virtual void addDependency ( class entryListBase* entry );

virtual void addInvDependency ( class entryListBase* entry );

virtual void addDependency ( int i, class entryListBase* entry );

virtual void addInvDependency ( int i, class entryListBase* entry );

virtual void addDependencyCallbacks ( void ) { }

virtual void removeDependencyCallbacks ( void ) { }

virtual void enable ( void );

virtual void disable ( void );

int numValues; // for multi-value widgets like option menus
int haveCallback;
int numDepend;
entryRecType dependList[10];

};


class subFormWidget : public entryListBase { // merely hold the pointer

public:

Widget *wPtr;

subFormWidget ( void );

virtual ~subFormWidget ( void );

};


class colorButtonEntry : public entryListBase {

public:

colorButtonEntry ( void );

virtual ~colorButtonEntry ( void );

void setValue ( int value );

void enable ( void );

void disable ( void );

colorButtonClass *theCb;

};


class fontMenuEntry : public entryListBase {

public:

fontMenuClass *fmo;

fontMenuEntry ( void );

virtual ~fontMenuEntry ( void );

};


class textEntry : public entryListBase {

public:

char *charDest;
int maxLen;
char lastGoodNumeric[31+1];
double *destPtrD;
int *destPtrI;
efDouble *destPtrEfD;
efInt *destPtrEfI;

textEntry ( void );

virtual ~textEntry ( void );

void cleanup ( void );

void setValue ( int value );

void setValue ( double value );

void setValue ( char *value );

void addDependencyCallbacks ( void );

void removeDependencyCallbacks ( void );

};


class optionEntry : public entryListBase {

public:

Widget pd;
widgetListPtr head, tail;

optionEntry ( void );

virtual ~optionEntry ( void );

void cleanup ( void );

void setValue ( int value );

void setValue ( char *value );

void addDependency (
  int i,
  class entryListBase* entry
);

void addInvDependency (
  int i,
  class entryListBase* entry
);

void addDependencyCallbacks ( void );

void removeDependencyCallbacks ( void );

int optHaveCallback[10];
int optNumDepend[10];
entryRecType optDependList[10][10];

};


class toggleEntry : public entryListBase {

public:

int value;
void *destination;

toggleEntry ( void );

virtual ~toggleEntry ( void );

void cleanup ( void );

void setValue ( int value );

void addDependencyCallbacks ( void );

void removeDependencyCallbacks ( void );

};

#ifdef __entry_form_cc

static void textEntryDependency (
  Widget w,
  XtPointer client,
  XtPointer call );

static void toggleEntryDependency (
  Widget w,
  XtPointer client,
  XtPointer call );

static void optionEntryDependency (
  Widget w,
  XtPointer client,
  XtPointer call );

static void kill_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void efEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

static void embeddedEfPopup_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ef_increment_num_items (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ef_decrement_num_items (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ef_set_num_items (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ef_increment_item_num (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ef_decrement_item_num (
  Widget w,
  XtPointer client,
  XtPointer call );

static void ef_set_item_num (
  Widget w,
  XtPointer client,
  XtPointer call );

#endif

class entryFormClass {

public:

static const int OK = 1;
static const int APPLY = 2;
static const int CANCEL = 3;

static const int CREATE = 10;
static const int EDIT = 11;

typedef struct callbackDataTag {
  int op;
  int command;
} callbackDataType, *callbackDataPtr;

callbackDataType callbackData;
XtPointer callbackPtr;
XtCallbackProc clientCb;

private:

static void ok_callback (
  Widget w,
  XtPointer client,
  XtPointer call );

static void apply_callback (
  Widget w,
  XtPointer client,
  XtPointer call );

static void cancel_callback (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void textEntryDependency (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void toggleEntryEntryDependency (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void optionEntryDependency (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void kill_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void efEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch );

friend void ef_increment_num_items (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ef_decrement_num_items (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ef_set_num_items (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ef_increment_item_num (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ef_decrement_item_num (
  Widget w,
  XtPointer client,
  XtPointer call );

friend void ef_set_item_num (
  Widget w,
  XtPointer client,
  XtPointer call );

Display *display;

Widget shell, paneTop, scrollWin, pane, labelForm, mainLabel,
 topForm, controlForm, arrayForm, bottomForm, pb_ok,
 pb_cancel, pb_apply, itemNumArrowInc, itemNumArrowDec, itemNumText,
 itemNumLabel, numItemsArrowInc, numItemsArrowDec, numItemsText,
 numItemsLabel, *subForm, prevW, curTopParent;
int firstSubFormChild;

entryListBase *itemHead, *itemTail;

XtCallbackProc setItem_cb;

XtCallbackProc okCb, applyCb, cancelCb;
XtPointer pbCallbackPtr;

int object_type; // type of object to which this form applies
                 // when used as a property form
                 // may be EF_K_RECTANGULAR or EF_K_MULTIPOINT

int firstItem, firstArrayItem, curWidgetIsLabel, leftAttachmentExists;
int firstColorButton;
Widget curW, curRW, curArrayW, curArrayRW;
XmFontList entryFontList, actionFontList;
char *entryTag, *actionTag;
Time buttonClickTime;

efSetItemCallbackDscType setItemDsc;

int isPoppedUp;

char title[31+1];

public:

int *x, *y, *w, *h, *largestH;
int index, oldIndex, maxItems, numItems;

efWidgetAndPointerType wp;

entryFormClass ( void );

~entryFormClass ( void );

int destroy ( void );

entryListBase *getCurItem ( void );

void setMultiPointObjectType ( void );

int objectIsMultiPoint ( void );

int create (
  Widget top,
  int *_x,
  int *_y,
  int *_w,
  int *_h,
  int *_largestH,
  fontInfoClass *fi,
  const char *entryFontTag,
  const char *actionFontTag );

int create (
  Widget top,
  Colormap cmap,
  int *_x,
  int *_y,
  int *_w,
  int *_h,
  int *_largestH,
  fontInfoClass *fi,
  const char *entryFontTag,
  const char *actionFontTag );

int create (
  Widget top,
  int *_x,
  int *_y,
  int *_w,
  int *_h,
  int *_largestH,
  char *label,
  fontInfoClass *fi,
  const char *entryFontTag,
  const char *actionFontTag );

int create (
  Widget top,
  Colormap cmap,
  int *_x,
  int *_y,
  int *_w,
  int *_h,
  int *_largestH,
  char *label,
  fontInfoClass *fi,
  const char *entryFontTag,
  const char *actionFontTag );

int create (
  Widget top,
  int *_x,
  int *_y,
  int *_w,
  int *_h,
  int *_largestH,
  char *label,
  int _maxItems,
  int _numItems,
  XtCallbackProc _setItem_cb,
  void *objPtr,
  fontInfoClass *fi,
  const char *entryFontTag,
  const char *actionFontTag );

int create (
  Widget top,
  Colormap cmap,
  int *_x,
  int *_y,
  int *_w,
  int *_h,
  int *_largestH,
  char *label,
  int _maxItems,
  int _numItems,
  XtCallbackProc _setItem_cb,
  void *objPtr,
  fontInfoClass *fi,
  const char *entryFontTag,
  const char *actionFontTag );

int addColorButton (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest );

int addColorButtonWithText (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest,
  int numCols,
  char *pvName );

int addColorButtonWithRule (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest,
  int numCols,
  char *pvName );

int addEmbeddedEf (
  char *label,
  entryFormClass **ef );

int addEmbeddedEf (
  char *label,
  char *buttonLabel,
  entryFormClass **ef );

int addFontMenuGeneric (
  int includeAlignInfo,
  char *label,
  fontInfoClass *fi,
  fontMenuClass *fm,
  char *initFontTag );

int addFontMenu (
  char *label,
  fontInfoClass *fi,
  fontMenuClass *fm,
  char *initFontTag );

int addFontMenuNoAlignInfo (
  char *label,
  fontInfoClass *fi,
  fontMenuClass *fm,
  char *initFontTag );

int addTextField (
  char *label,
  int length,
  int *dest );

int addTextField (
  char *label,
  int length,
  efInt *dest );

int addTextField (
  char *label,
  int length,
  double *dest );

class textEntry* addTextFieldAccessible (
  char *label,
  int length,
  double *dest );

int addTextField (
  char *label,
  int length,
  efDouble *dest );

int addTextField (
  char *label,
  int length,
  char *dest,
  int stringSize );

int addGenericTextBox (
  int edit,
  char *label,
  int width,
  int height,
  char *dest,
  int stringSize );

int addTextBox (
  char *label,
  int width,
  int height,
  char *dest,
  int stringSize );

int addReadonlyTextBox (
  char *label,
  int width,
  int height,
  char *dest,
  int stringSize );

int addPasswordField (
  char *label,
  int length,
  char *dest,
  int stringSize );

int addLockedField (
  char *label,
  int length,
  char *dest,
  int stringSize );

int addOption (
  char *label,
  char *options,
  char *dest,
  int stringSize );

int addOption (
  char *label,
  char *options,
  int *dest );

int addToggle (
  char *label,
  int *dest );

int addLabel (
  char *label );

int addSeparator ( void );

// arrays

int addColorButtonArray (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest,
  entryListBase *obj );

int addFontMenuArray (
  char *label,
  fontInfoClass *fi,
  fontMenuClass *fm,
  char *initFontTag,
  entryListBase *obj );

int addTextFieldArray (
  char *label,
  int length,
  int *dest,
  entryListBase **obj );

int addTextFieldArray (
  char *label,
  int length,
  efInt *dest,
  entryListBase **obj );

int addTextFieldArray (
  char *label,
  int length,
  double *dest,
  entryListBase **obj );

int addTextFieldArray (
  char *label,
  int length,
  efDouble *dest,
  entryListBase **obj );

int addTextFieldArray (
  char *label,
  int length,
  char **dest,
  int stringSize,
  entryListBase **obj );

int addOptionArray (
  char *label,
  char *options,
  char **dest,
  int stringSize,
  entryListBase **obj );

int addOptionArray (
  char *label,
  char *options,
  int *dest,
  entryListBase **obj );

int addToggleArray (
  char *label,
  int *dest,
  entryListBase **obj );

int finished (
  XtCallbackProc ok_cb,
  XtCallbackProc apply_cb,
  XtCallbackProc cancel_cb,
  XtPointer ptr );

int finished (
  int operationType,
  XtCallbackProc cb,
  XtPointer ptr );

int finished (
  XtCallbackProc close_cb,
  XtPointer ptr );

int beginSubForm ( void );

int beginLeftSubForm ( void );

int endSubForm ( void );

int popup ( void );

int popdown ( void );

int popdownNoDestroy ( void );

int formIsPoppedUp ( void );

Widget getOkWidget ( void ) {
  return pb_ok;
}

Widget getApplyWidget ( void ) {
  return pb_apply;
}

Widget getCancelWidget ( void ) {
  return pb_cancel;
}

};

class embeddedEfEntry : public entryListBase {

private:

friend void embeddedEfPopup_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

public:

entryFormClass ef;

embeddedEfEntry ( void ) {

}

virtual ~embeddedEfEntry ( void ) {

  ef.destroy();

}

};

#endif
