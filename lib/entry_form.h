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

efInt::efInt ( void ) {

  null = 1;
  val = 0;

}

efInt::~efInt ( void ) { }

int efInt::value ( void ) { return val; }

void efInt::setValue ( int v ) { val = v; null = 0; }

int efInt::isNull ( void ) { return null; }

void efInt::setNull ( int n ) { null = n; }

int efInt::write ( FILE *f ) {

int stat, v, n;

  v = value();
  n = isNull();
  stat = fprintf( f, "%-d %-d\n", v, n );

  if ( stat < 0 )
    return 0;
  else
    return 1;

}

int efInt::read ( FILE *f ) {

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

efDouble::efDouble ( void ) {

  null = 1;
  val = 0.0;

}

efDouble::~efDouble ( void ) { }

double efDouble::value ( void ) { return val; }

void efDouble::setValue ( double v ) { val = v; null = 0; }

int efDouble::isNull ( void ) { return null; }

void efDouble::setNull ( int n ) { null = n; }

int efDouble::write ( FILE *f ) {

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

int efDouble::read ( FILE *f ) {

int stat;
double v, n;

  stat = fscanf( f, "%lg %lg\n", &v, &n );

  if ( stat < 0 ) return 0;

  setValue( v );
  setNull( (int) n );

  return 1;

}

};

class entryListBase { // base class for all items that may be placed
                      // on a entry form

public:

efArrayCallbackDscType arrayDsc;
entryListBase *flink;
Widget labelW, activeW;

entryListBase::entryListBase ( void ) { }

virtual entryListBase::~entryListBase ( void ) { }

virtual void entryListBase::setValue ( int value ) { }

virtual void entryListBase::setValue ( double value ) { }

virtual void entryListBase::setValue ( char *value ) { }

};


class colorButtonEntry : public entryListBase {

public:

colorButtonEntry::colorButtonEntry ( void );

virtual colorButtonEntry::~colorButtonEntry ( void );

void colorButtonEntry::setValue ( int value );

};


class fontMenuEntry : public entryListBase {

public:

fontMenuClass *fmo;

fontMenuEntry::fontMenuEntry ( void );

virtual fontMenuEntry::~fontMenuEntry ( void );

};


class textEntry : public entryListBase {

public:

char *charDest;
int maxLen;

textEntry::textEntry ( void );

virtual textEntry::~textEntry ( void );

void textEntry::setValue ( int value );

void textEntry::setValue ( double value );

void textEntry::setValue ( char *value );

};


class optionEntry : public entryListBase {

public:

Widget pd;
widgetListPtr head, tail;

optionEntry::optionEntry ( void );

virtual optionEntry::~optionEntry ( void );

void optionEntry::setValue ( int value );

void optionEntry::setValue ( char *value );

};


class toggleEntry : public entryListBase {

public:

int value;
void *destination;
efArrayCallbackDscType arrayDsc;

toggleEntry::toggleEntry ( void );

virtual toggleEntry::~toggleEntry ( void );

void toggleEntry::setValue ( int value );

};

#ifdef __entry_form_cc

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

private:

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

Widget shell, scrollWin, pane, labelForm, mainLabel, topForm, controlForm,
 arrayForm, bottomForm, pb_ok,
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

int firstItem, firstArrayItem, curWidgetIsLabel;
int firstColorButton;
Widget curW, curRW, curArrayW, curArrayRW;
XmFontList entryFontList, actionFontList;
char *entryTag, *actionTag;
Time buttonClickTime;

efSetItemCallbackDscType setItemDsc;

int isPoppedUp;

public:

int *x, *y, *w, *h, *largestH;
int index, oldIndex, maxItems, numItems;

efWidgetAndPointerType wp;

entryFormClass::entryFormClass ( void );

entryFormClass::~entryFormClass ( void );

int entryFormClass::destroy ( void );

void entryFormClass::setMultiPointObjectType ( void );

int entryFormClass::objectIsMultiPoint ( void );

int entryFormClass::create (
  Widget top,
  int *_x,
  int *_y,
  int *_w,
  int *_h,
  int *_largestH,
  fontInfoClass *fi,
  const char *entryFontTag,
  const char *actionFontTag );

int entryFormClass::create (
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

int entryFormClass::create (
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

int entryFormClass::create (
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

int entryFormClass::create (
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

int entryFormClass::create (
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

int entryFormClass::addColorButton (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest );

int entryFormClass::addColorButtonWithText (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest,
  int numCols,
  char *pvName );

int entryFormClass::addEmbeddedEf (
  char *label,
  entryFormClass *ef );

int entryFormClass::addFontMenu (
  char *label,
  fontInfoClass *fi,
  fontMenuClass *fm,
  char *initFontTag );

int entryFormClass::addTextField (
  char *label,
  int length,
  int *dest );

int entryFormClass::addTextField (
  char *label,
  int length,
  efInt *dest );

int entryFormClass::addTextField (
  char *label,
  int length,
  double *dest );

int entryFormClass::addTextField (
  char *label,
  int length,
  efDouble *dest );

int entryFormClass::addTextField (
  char *label,
  int length,
  char *dest,
  int stringSize );

int entryFormClass::addOption (
  char *label,
  char *options,
  char *dest,
  int stringSize );

int entryFormClass::addOption (
  char *label,
  char *options,
  int *dest );

int entryFormClass::addToggle (
  char *label,
  int *dest );

int entryFormClass::addLabel (
  char *label );

int entryFormClass::addSeparator ( void );

// arrays

int entryFormClass::addColorButtonArray (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest,
  entryListBase *obj );

int entryFormClass::addFontMenuArray (
  char *label,
  fontInfoClass *fi,
  fontMenuClass *fm,
  char *initFontTag,
  entryListBase *obj );

int entryFormClass::addTextFieldArray (
  char *label,
  int length,
  int *dest,
  entryListBase **obj );

int entryFormClass::addTextFieldArray (
  char *label,
  int length,
  efInt *dest,
  entryListBase **obj );

int entryFormClass::addTextFieldArray (
  char *label,
  int length,
  double *dest,
  entryListBase **obj );

int entryFormClass::addTextFieldArray (
  char *label,
  int length,
  efDouble *dest,
  entryListBase **obj );

int entryFormClass::addTextFieldArray (
  char *label,
  int length,
  char **dest,
  int stringSize,
  entryListBase **obj );

int entryFormClass::addOptionArray (
  char *label,
  char *options,
  char **dest,
  int stringSize,
  entryListBase **obj );

int entryFormClass::addOptionArray (
  char *label,
  char *options,
  int *dest,
  entryListBase **obj );

int entryFormClass::addToggleArray (
  char *label,
  int *dest,
  entryListBase **obj );

int entryFormClass::finished (
  XtCallbackProc ok_cb,
  XtCallbackProc apply_cb,
  XtCallbackProc cancel_cb,
  XtPointer ptr );

int entryFormClass::beginSubForm ( void );

int entryFormClass::endSubForm ( void );

int entryFormClass::popup ( void );

int entryFormClass::popdown ( void );

int entryFormClass::popdownNoDestroy ( void );

int entryFormClass::formIsPoppedUp ( void );

};

class embeddedEfEntry : public entryListBase {

private:

friend void embeddedEfPopup_cb (
  Widget w,
  XtPointer client,
  XtPointer call );

public:

entryFormClass *ef;

embeddedEfEntry::embeddedEfEntry ( void ) {

}

virtual embeddedEfEntry::~embeddedEfEntry ( void ) {

  ef->destroy();

}

};

#endif
