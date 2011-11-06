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

#define __entry_form_cc

#include "entry_form.h"
#include <Xm/SashP.h>

#include "thread.h"

static void toggleEntryDependency (
  Widget w,
  XtPointer client,
  XtPointer call
) {

class toggleEntry *tbo;
int i;

  tbo = (class toggleEntry *) client;

  if ( XmToggleButtonGetState( w ) ) {
    for ( i=0; i<tbo->numDepend; i++ ) {
      if ( tbo->dependList[i].entry ) {
        if ( tbo->dependList[i].sense ) {
          tbo->dependList[i].entry->enable();
        }
        else {
          tbo->dependList[i].entry->disable();
        }
      }
    }
  }
  else {
    for ( i=0; i<tbo->numDepend; i++ ) {
      if ( tbo->dependList[i].entry ) {
        if ( tbo->dependList[i].sense ) {
          tbo->dependList[i].entry->disable();
        }
        else {
          tbo->dependList[i].entry->enable();
        }
      }
    }
  }

}

static void textEntryDependency (
  Widget w,
  XtPointer client,
  XtPointer call
) {

class textEntry *teo;
char *buf;
int i;

  teo = (class textEntry *) client;

  buf = XmTextGetString( w );
  if ( !blank( buf ) ) {
    for ( i=0; i<teo->numDepend; i++ ) {
      if ( teo->dependList[i].entry ) {
        if ( teo->dependList[i].sense ) {
          teo->dependList[i].entry->enable();
	}
	else {
          teo->dependList[i].entry->disable();
	}
      }
    }
  }
  else {
    for ( i=0; i<teo->numDepend; i++ ) {
      if ( teo->dependList[i].entry ) {
        if ( teo->dependList[i].sense ) {
          teo->dependList[i].entry->disable();
	}
	else {
          teo->dependList[i].entry->enable();
	}
      }
    }
  }
  XtFree( buf );

}

static void optionEntryDependency (
  Widget w,
  XtPointer client,
  XtPointer call
) {

widgetListPtr curpb;
Widget curHistoryWidget;
class optionEntry *opto;
int i, n;

  opto = (class optionEntry *) client;

  XtVaGetValues( opto->activeW,
   XmNmenuHistory, (XtArgVal) &curHistoryWidget,
   NULL );

  if ( w != curHistoryWidget ) return;

  // disable/enable all
  n = 0;
  curpb = opto->head->flink;
  while ( curpb ) {

    for ( i=0; i<opto->optNumDepend[n]; i++ ) {
      if ( opto->optDependList[n][i].sense ) {
        opto->optDependList[n][i].entry->disable();
      }
      else {
        opto->optDependList[n][i].entry->enable();
      }
    }

    curpb = curpb->flink;
    n++;

  }

  // enable/disable any that satisfy condition
  n = 0;
  curpb = opto->head->flink;
  while ( curpb ) {

    if ( curpb->w == w ) {

      for ( i=0; i<opto->optNumDepend[n]; i++ ) {

        if ( opto->optDependList[n][i].entry ) {
          if ( opto->optDependList[n][i].sense ) {
            opto->optDependList[n][i].entry->enable();
	  }
	  else {
            opto->optDependList[n][i].entry->disable();
	  }
	}

      }

    }

    curpb = curpb->flink;
    n++;

  }

}

#if 0
// old version
static void optionEntryDependency (
  Widget w,
  XtPointer client,
  XtPointer call
) {

widgetListPtr curpb;
Widget curHistoryWidget;
class optionEntry *opto;
int i, n;

  opto = (class optionEntry *) client;

  XtVaGetValues( opto->activeW,
   XmNmenuHistory, (XtArgVal) &curHistoryWidget,
   NULL );

  if ( w != curHistoryWidget ) return;

  n = 0;
  curpb = opto->head->flink;
  while ( curpb ) {

    if ( curpb->w == w ) {

      for ( i=0; i<opto->optNumDepend[n]; i++ ) {

        if ( opto->optDependList[n][i].entry ) {
          if ( opto->optDependList[n][i].sense ) {
            opto->optDependList[n][i].entry->enable();
	  }
	  else {
            opto->optDependList[n][i].entry->disable();
	  }
	}

      }

    }
    else {

      for ( i=0; i<opto->optNumDepend[n]; i++ ) {

        if ( opto->optDependList[n][i].entry ) {
          if ( opto->optDependList[n][i].sense ) {
            opto->optDependList[n][i].entry->disable();
	  }
	  else {
            opto->optDependList[n][i].entry->enable();
	  }
	}

      }

    }

    curpb = curpb->flink;
    n++;

  }

}
#endif

static void efEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

XButtonEvent *be;
entryFormClass *efo = (entryFormClass *) client;
XmPushButtonCallbackStruct pbcb;
Time deltaTime;

  *continueToDispatch = False;

  if ( e->type == ButtonPress ) {

    be = (XButtonEvent *) e;

    if ( be->button != Button1 && be->button != Button2 && be->button != Button3 ) {
      *continueToDispatch = True;
      return;
    }

    if ( ( be->state & ShiftMask ) && ( be->state & ControlMask ) ) {

      // call the callback function associated with the OK button

      pbcb.reason = XmCR_ACTIVATE;
      pbcb.event = e;
      pbcb.click_count = 1;

      (*efo->okCb)( efo->pb_ok, efo->pbCallbackPtr,
       (XtPointer) &pbcb );

    }
    else if ( be->state & ShiftMask ) {

      // call the callback function associated with the Apply button

      pbcb.reason = XmCR_ACTIVATE;
      pbcb.event = e;
      pbcb.click_count = 1;

      XBell( efo->display, 25 );
      (*efo->applyCb)( efo->pb_apply, efo->pbCallbackPtr,
       (XtPointer) &pbcb );

    }
    else if ( be->state & ControlMask ) {

      // call the callback function associated with the Cancel button

      pbcb.reason = XmCR_ACTIVATE;
      pbcb.event = e;
      pbcb.click_count = 1;

      (*efo->cancelCb)( efo->pb_cancel, efo->pbCallbackPtr,
       (XtPointer) &pbcb );

    }
    else {

      *continueToDispatch = True;

    }

  }
  if ( e->type == ButtonRelease ) {

    be = (XButtonEvent *) e;

    deltaTime = be->time - efo->buttonClickTime;
    efo->buttonClickTime = be->time;

    if ( deltaTime < 250 ) {

      pbcb.reason = XmCR_ACTIVATE;
      pbcb.event = e;
      pbcb.click_count = 1;

      *continueToDispatch = False;

      if ( be->button == Button1 ) {
        (*efo->okCb)( efo->pb_ok, efo->pbCallbackPtr,
         (XtPointer) &pbcb );
      }
      else if ( be->button == Button2 ) {
        XBell( efo->display, 25 );
        (*efo->applyCb)( efo->pb_apply, efo->pbCallbackPtr,
         (XtPointer) &pbcb );
      }
      else if ( be->button == Button3 ) {
        (*efo->cancelCb)( efo->pb_cancel, efo->pbCallbackPtr,
         (XtPointer) &pbcb );
      }
      else {
        *continueToDispatch = True;
      }

    }
    else {

      *continueToDispatch = True;

    }

  }
  else {

    *continueToDispatch = True;

  }

}

static void embeddedEfPopup_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

embeddedEfEntry *eeo = (embeddedEfEntry *) client;

  eeo->ef.popup();

}

static void ef_increment_num_items (
  Widget w,
  XtPointer client,
  XtPointer call )
{

entryFormClass *eo;
char buf[127+1];
int n;
Arg args[2];
efSetItemCallbackDscPtr dsc;

  dsc = (efSetItemCallbackDscPtr) client;
  eo = (entryFormClass *) dsc->ef;

//   fprintf( stderr, "ef_increment_num_items\n" );

  if ( eo->numItems + 1 > eo->maxItems )
    eo->numItems = eo->maxItems;
  else {
    (eo->numItems)++;
  }

  sprintf( buf, "%-d", eo->numItems );
  n = 0;
  XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
  XtSetValues( eo->numItemsText, args, n );

}

static void ef_decrement_num_items (
  Widget w,
  XtPointer client,
  XtPointer call )
{

entryFormClass *eo;
char buf[127+1];
int n;
Arg args[2];
efSetItemCallbackDscPtr dsc;

  dsc = (efSetItemCallbackDscPtr) client;
  eo = (entryFormClass *) dsc->ef;

//   fprintf( stderr, "ef_decrement_num_items\n" );

  if ( eo->numItems - 1 < 1 )
    eo->numItems = 1;
  else {
    (eo->numItems)--;
  }

  sprintf( buf, "%-d", eo->numItems );
  n = 0;
  XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
  XtSetValues( eo->numItemsText, args, n );

}

static void ef_set_num_items (
  Widget w,
  XtPointer client,
  XtPointer call )
{

entryFormClass *eo;
efSetItemCallbackDscPtr dsc;
char buf[127+1];
char *ptr;
int i, blank, n;
Arg args[2];

  dsc = (efSetItemCallbackDscPtr) client;
  eo = (entryFormClass *) dsc->ef;

//   fprintf( stderr, "ef_set_num_items\n" );

  ptr = XmTextGetString( w );
  if ( ptr[0] == 0 )
    blank = 1;
  else
    blank = 0;
  //i = atol( ptr );
  i = strtol( ptr, NULL, 0 );
  XtFree( ptr );

  if ( blank ) return;

  if ( i > eo->maxItems ) {
    eo->numItems = eo->maxItems;
    sprintf( buf, "%-d", eo->numItems );
    n = 0;
    XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
    XtSetValues( eo->numItemsText, args, n );
  }
  else if ( i < 1 ) {
    eo->numItems = 1;
    sprintf( buf, "%-d", eo->numItems );
    n = 0;
    XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
    XtSetValues( eo->numItemsText, args, n );
  }
  else {
    eo->numItems = i;
  }

  // update itemNum if it is now out of range due to change in
  // permitted number of items

  if ( eo->numItems < ( eo->index + 1 ) ) {
    eo->index = eo->numItems - 1;
    sprintf( buf, "%-d", eo->index+1 );
    n = 0;
    XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
    XtSetValues( eo->itemNumText, args, n );
  }

}

static void ef_increment_item_num (
  Widget w,
  XtPointer client,
  XtPointer call )
{

entryFormClass *eo;
char buf[127+1];
int n;
Arg args[2];
efSetItemCallbackDscPtr dsc;

  dsc = (efSetItemCallbackDscPtr) client;
  eo = (entryFormClass *) dsc->ef;

// fprintf( stderr, "ef_increment_item_num\n" );

  if ( eo->index + 1 >= eo->numItems )
    eo->index = eo->numItems - 1;
  else {
    (eo->index)++;
  }

  sprintf( buf, "%-d", eo->index+1 );
  n = 0;
  XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
  XtSetValues( eo->itemNumText, args, n );

}

static void ef_decrement_item_num (
  Widget w,
  XtPointer client,
  XtPointer call )
{

entryFormClass *eo;
char buf[127+1];
int n;
Arg args[2];
efSetItemCallbackDscPtr dsc;

  dsc = (efSetItemCallbackDscPtr) client;
  eo = (entryFormClass *) dsc->ef;

// fprintf( stderr, "ef_decrement_item_num\n" );

  if ( eo->index - 1 < 0 )
    eo->index = 0;
  else {
    (eo->index)--;
  }

  sprintf( buf, "%-d", eo->index+1 );
  n = 0;
  XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
  XtSetValues( eo->itemNumText, args, n );

}

static void ef_set_item_num (
  Widget w,
  XtPointer client,
  XtPointer call )
{

entryFormClass *eo;
efSetItemCallbackDscPtr dsc;
char buf[127+1];
char *ptr;
int i, blank, n, updateOld;
Arg args[2];

  dsc = (efSetItemCallbackDscPtr) client;
  eo = (entryFormClass *) dsc->ef;

// fprintf( stderr, "ef_set_item_num\n" );

  updateOld = 0;

  ptr = XmTextGetString( w );
  if ( ptr[0] == 0 )
    blank = 1;
  else
    blank = 0;
  //i = atol( ptr );
  i = strtol( ptr, NULL, 0 );
  XtFree( ptr );

  if ( blank ) return;

  if ( i > eo->numItems ) {
    eo->index = eo->numItems - 1;
    sprintf( buf, "%-d", eo->index+1 );
    n = 0;
    XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
    XtSetValues( eo->itemNumText, args, n );
  }
  else if ( i < 1 ) {
    eo->index = 0;
    sprintf( buf, "%-d", eo->index+1 );
    n = 0;
    XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
    XtSetValues( eo->itemNumText, args, n );
  }
  else {
    updateOld = 1;
    eo->index = i - 1;
  }

  (eo->setItem_cb)( w, client, call );

  if ( updateOld ) {
    eo->oldIndex = eo->index;
  }

}

static void entryFormEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

entryFormClass *efo;
XConfigureEvent *ce;

  efo = (entryFormClass *) client;
  ce = (XConfigureEvent *) e;

  *continueToDispatch = False;

  if ( ce->type == ConfigureNotify ) {

    XWindowAttributes atts;
    XGetWindowAttributes(ce->display, ce->window, &atts); 

    /* another hack to (hopefully) avoid getting distracted
     * while the WM messes around reparenting/reconfiguring
     */
    if ( IsUnmapped == atts.map_state ) return;

    if ( *(efo->h) > *efo->largestH ) *efo->largestH = *(efo->h);
    if ( ce->height > *efo->largestH ) *efo->largestH = ce->height;

    if ( ( *(efo->w) != ce->width ) ||
         ( *(efo->h) != ce->height ) ) {

      *(efo->w) = ce->width;
      *(efo->h) = ce->height;

    }
    else {

      if ( ( abs( *(efo->x) - ce->x ) > 50 ) ||
           ( abs( *(efo->y) - ce->y ) > 50 ) ) {

        *(efo->x) = ce->x;
        *(efo->y) = ce->y;

      }

    }

  }
  else {

    *continueToDispatch = True;

  }

}

void entryListBase::addDependency (
  class entryListBase* entry
) {

  if ( numDepend < 9 ) {
    dependList[numDepend].entry = entry;
    dependList[numDepend].sense = 1;
    numDepend++;
  }

}

void entryListBase::addInvDependency (
  class entryListBase* entry
) {

  if ( numDepend < 9 ) {
    dependList[numDepend].entry = entry;
    dependList[numDepend].sense = 0;
    numDepend++;
  }

}

void entryListBase::addDependency (
  int i,
  class entryListBase* entry
) {

  if ( numDepend < 9 ) {
    dependList[numDepend].entry = entry;
    dependList[numDepend].sense = 1;
    numDepend++;
  }

}

void entryListBase::addInvDependency (
  int i,
  class entryListBase* entry
) {

  if ( numDepend < 9 ) {
    dependList[numDepend].entry = entry;
    dependList[numDepend].sense = 0;
    numDepend++;
  }

}

void entryListBase::enable ( void ) {

  if ( activeW ) {
    XtSetSensitive( activeW, True );
  }

}

void entryListBase::disable ( void ) {

  if ( activeW ) {
    XtSetSensitive( activeW, False );
  }

}

subFormWidget::subFormWidget ( void ) {

  wPtr = NULL;
  activeW = NULL;
  labelW = NULL;

}

subFormWidget::~subFormWidget ( void ) {

  if ( wPtr ) {
    delete wPtr;
    wPtr = NULL;
  }

}

toggleEntry::toggleEntry ( void ) {

  destination = NULL;
  arrayDsc.size = 0;
  arrayDsc.indexPtr = NULL;
  arrayDsc.destPtr = NULL;
  arrayDsc.valuePtr = NULL;

}

toggleEntry::~toggleEntry ( void ) {

}

void toggleEntry::cleanup ( void ) {

  removeDependencyCallbacks();

}

void toggleEntry::setValue ( int value ) {

int n;
Arg args[2];

  n = 0;
  XtSetArg( args[n], XmNset, (XtArgVal) value ); n++;
  XtSetValues( activeW, args, n );

}

void toggleEntry::addDependencyCallbacks ( void ) {

  if ( !haveCallback ) {
    haveCallback = 1;
    XtAddCallback( activeW, XmNvalueChangedCallback, toggleEntryDependency,
     this );
    toggleEntryDependency( activeW, this, NULL );
  }

}

void toggleEntry::removeDependencyCallbacks ( void ) {

  if ( haveCallback ) {
    haveCallback = 0;
    //XtRemoveCallback( activeW, XmNvalueChangedCallback,
    // toggleEntryDependency, this );
  }

  numDepend = 0;

}

textEntry::textEntry ( void ) { }

textEntry::~textEntry ( void ) {

}

void textEntry::cleanup ( void ) {

  removeDependencyCallbacks();

}

void textEntry::setValue ( int value ) {

char buf[127+1];
int n;
Arg args[2];

  sprintf( buf, "%-d", value );

  n = 0;
  XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
  XtSetValues( activeW, args, n );

}

void textEntry::setValue ( double value ) {

char buf[127+1];
int n;
Arg args[2];

  sprintf( buf, "%-g", value );

  n = 0;
  XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;
  XtSetValues( activeW, args, n );

}

void textEntry::setValue ( char *value ) {

int n;
Arg args[2];

  n = 0;
  XtSetArg( args[n], XmNvalue, (XtArgVal) value ); n++;
  XtSetValues( activeW, args, n );

}

void textEntry::addDependencyCallbacks ( void ) {

  if ( !haveCallback ) {
    haveCallback = 1;
    XtAddCallback( activeW, XmNvalueChangedCallback, textEntryDependency,
     this );
    textEntryDependency( activeW, this, NULL );
  }

}

void textEntry::removeDependencyCallbacks ( void ) {

  if ( haveCallback ) {
    haveCallback = 0;
    //XtRemoveCallback( activeW, XmNvalueChangedCallback,
    // textEntryDependency, this );
  }

  numDepend = 0;

}

colorButtonEntry::colorButtonEntry ( void )
{

  theCb = NULL;

}

colorButtonEntry::~colorButtonEntry ( void )
{

}

void colorButtonEntry::setValue ( int value ) {

char buf[127+1];
int n;
Arg args[2];

  sprintf( buf, "%-d", value );

  n = 0;
  XtSetArg( args[n], XmNvalue, (XtArgVal) buf ); n++;

}

void colorButtonEntry::enable ( void ) {

  if ( theCb ) theCb->enable();

}

void colorButtonEntry::disable ( void ) {

  if ( theCb ) theCb->disable();

}


fontMenuEntry::fontMenuEntry ( void )
{

  fmo = NULL;

}

fontMenuEntry::~fontMenuEntry ( void )
{

  if ( fmo ) fmo->destroyFontMenu();

}

optionEntry::optionEntry ( void ) {

int i;

//   fprintf( stderr, "optionEntry::optionEntry - new widgetListType\n" );

  head = new widgetListType;
  tail = head;
  tail->flink = NULL;

  numValues = 0;
  for ( i=0; i<10; i++ ) {
    optNumDepend[i] = 0;
    optHaveCallback[i] = 0;
  }

}

optionEntry::~optionEntry ( void ) {
  // walk list and delete items

widgetListPtr cur, next;

  cur = head->flink;
  while ( cur ) {
    next = cur->flink;
    delete[] cur->value;
    delete cur;
    cur = next;
  }

  delete head;

}

void optionEntry::cleanup ( void ) {

  removeDependencyCallbacks();

}

void optionEntry::setValue ( int value ) {

widgetListPtr curpb;
int item, n;
Arg args[2];

  item = 0;
  curpb = head->flink;
  while ( curpb ) {

    if ( item == value ) {
      n = 0;
      XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curpb->w ); n++;
      XtSetValues( activeW, args, n );
      return;
    }

    curpb = curpb->flink;
    item++;

  }

}

void optionEntry::setValue ( char *value ) {

widgetListPtr curpb;
int item, n;
Arg args[2];

  curpb = head->flink;
  while ( curpb ) {

    if ( strcmp( curpb->value, value ) == 0 ) {
      n = 0;
      XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curpb->w ); n++;
      XtSetValues( activeW, args, n );
      return;
    }

    curpb = curpb->flink;
    item++;

  }

}

void optionEntry::addDependency (
  int i,
  class entryListBase* entry
) {

  if ( i < 0 ) return;
  if ( i >= numValues ) return;

  if ( optNumDepend[i] < 9 ) {
    optDependList[i][optNumDepend[i]].entry = entry;
    optDependList[i][optNumDepend[i]].sense = 1;
    optNumDepend[i]++;
  }

}

void optionEntry::addInvDependency (
  int i,
  class entryListBase* entry
) {

  if ( i < 0 ) return;
  if ( i >= numValues ) return;

  if ( optNumDepend[i] < 9 ) {
    optDependList[i][optNumDepend[i]].entry = entry;
    optDependList[i][optNumDepend[i]].sense = 0;
    optNumDepend[i]++;
  }

}

void optionEntry::addDependencyCallbacks ( void ) {

int i;
widgetListPtr curpb;

  if ( numValues == 0 ) {
    fprintf( stderr,
     "optionEntry::addDependencyCallbacks - numValues not set\n" );
  }

  i = 0;
  curpb = head->flink;
  while ( curpb ) {

    if ( i <= numValues ) {
      if ( !optHaveCallback[i] ) {
        optHaveCallback[i] = 1;
        if ( curpb->w ) {
          XtAddCallback( curpb->w, XmNactivateCallback,
           optionEntryDependency, this );
          optionEntryDependency( curpb->w, this, NULL );
        }
      }
    }
    else {
      return;
    }

    curpb = curpb->flink;
    i++;

  }

}

void optionEntry::removeDependencyCallbacks ( void ) {

widgetListPtr curpb;
int i;

  i = 0;
  curpb = head->flink;
  while ( curpb ) {

    if ( i <= numValues ) {

      if ( optHaveCallback[i] ) {
        optHaveCallback[i] = 0;
        //XtRemoveCallback( curpb->w, XmNactivateCallback,
        // optionEntryDependency, this );
      }

    }

    curpb = curpb->flink;
    i++;

  }

  numValues = 0;
  for ( i=0; i<10; i++ ) {
    optNumDepend[i] = 0;
    optHaveCallback[i] = 0;
  }

}

void popdown_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

entryFormClass *eo;

  eo = (entryFormClass *) client;
  eo->popdown();

}

entryFormClass::entryFormClass ( void ) {

  // fprintf( stderr, "In entryFormClass::entryFormClass - new entryListBase\n" );

  itemHead = new entryListBase;
  itemTail = itemHead;
  itemTail->flink = NULL;

  index = 0;
  oldIndex = 0;
  maxItems = 1;
  numItems = 1;

  isPoppedUp = 0;

  firstItem = 1;
  firstColorButton = 1;
  leftAttachmentExists = 0;

  entryFontList = NULL;
  actionFontList = NULL;
  entryTag = NULL;
  actionTag = NULL;

  callbackData.op = 0;
  callbackData.command = 0;
  callbackPtr = NULL;
  clientCb = NULL;

  object_type = EF_K_RECTANGULAR;

  shell = NULL;

}

entryFormClass::~entryFormClass ( void ) {

entryListBase *cur, *next;

//  fprintf( stderr, "In entryFormClass::~entryFormClass\n" );

  if ( itemHead ) {

    cur = itemHead->flink;
    while ( cur ) {
      next = cur->flink;
//        fprintf( stderr, "entryFormClass::~entryFormClass - delete node\n" );
      delete cur;
      cur = next;
    }

//      fprintf( stderr, "entryFormClass::~entryFormClass - delete itemHead\n" );
    delete itemHead;
    itemHead = NULL;

  }

}

int entryFormClass::destroy ( void ) {

entryListBase *cur, *next;

// fprintf( stderr, "entryFormClass::destroy\n" );

  cur = itemHead->flink;
  while ( cur ) {
    cur->cleanup();
    cur = cur->flink;
  }

  if ( entryFontList ) XmFontListFree( entryFontList );
  if ( actionFontList ) XmFontListFree( actionFontList );

  if ( shell ) XtDestroyWidget( shell );

  if ( entryTag ) {
//     fprintf( stderr, "entryFormClass::destroy - delete entryTag\n" );
    delete[] entryTag;
  }

  if ( actionTag ) {
//     fprintf( stderr, "entryFormClass::destroy - delete actionTag\n" );
    delete[] actionTag;
  }

  if ( itemHead->flink ) {

    cur = itemHead->flink;
    while ( cur ) {
      next = cur->flink;
//       fprintf( stderr, "entryFormClass::destroy - delete node\n" );
      delete cur;
      cur = next;
    }

    itemHead->flink = NULL;
    itemTail = itemHead;

  }

  return 1;

}

entryListBase *entryFormClass::getCurItem ( void )
{

  return itemTail;

}

void entryFormClass::setMultiPointObjectType ( void )
{

  object_type = EF_K_MULTIPOINT;

}

int entryFormClass::objectIsMultiPoint ( void )
{

  if ( object_type == EF_K_MULTIPOINT )
    return 1;
  else
    return 0;

}

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
  const char *actionFontTag )
{

int stat;

  stat = create( top, _x, _y, _w, _h, _largestH, label, fi,
   entryFontTag, actionFontTag );

  XSetWindowColormap( display, XtWindow(shell), cmap );

  return stat;

}

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
  const char *actionFontTag )
{

XmString str;

   setItem_cb = NULL;

   x = _x;
   y = _y;
   w = _w;
   h = _h;
   largestH = _largestH;

  display = XtDisplay( top );

  buttonClickTime = 0;
  index = 0;
  oldIndex = 0;
  maxItems = 0;
  numItems = 0;

  firstItem = 1;
  firstArrayItem = 1;
  firstColorButton = 1;
  curWidgetIsLabel = 0;
  leftAttachmentExists = 0;

  strncpy( title, label, 31 );
  title[31] = 0;

  if ( fi ) {

    if ( entryFontTag ) {
//       fprintf( stderr, "entryFormClass::create - new char[strlen(entryFontTag)+1]\n" );
      entryTag = new char[strlen(entryFontTag)+1];
      strcpy( entryTag, entryFontTag );
      fi->getTextFontList( entryTag, &entryFontList );

}

    if ( actionFontTag ) {
//       fprintf( stderr, "entryFormClass::create - new char[strlen(actionFontTag)+1]\n" );
      actionTag = new char[strlen(actionFontTag)+1];
      strcpy( actionTag, actionFontTag );
      fi->getTextFontList( actionTag, &actionFontList );
    }

  }

  //shell = XtVaCreatePopupShell( label, xmDialogShellWidgetClass,
  shell = XtVaCreatePopupShell( "properties", xmDialogShellWidgetClass,
   top,
   XmNmappedWhenManaged, False,
   NULL );

  paneTop = XtVaCreateWidget( "paneTop", xmPanedWindowWidgetClass, shell,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   NULL );

  scrollWin = XtVaCreateWidget( "scrollwin", xmScrolledWindowWidgetClass, paneTop,
   XmNallowResize, True,
   XmNpaneMaximum, 10000,
   XmNscrollBarDisplayPolicy, XmAS_NEEDED,
   XmNscrollingPolicy, XmAUTOMATIC,
   NULL );

  pane = XtVaCreateWidget( "pane", xmPanedWindowWidgetClass, scrollWin,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   NULL );

  labelForm = XtVaCreateWidget( "labelform", xmFormWidgetClass, pane,
   XmNallowResize, True,
   NULL );

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  mainLabel = XtVaCreateManagedWidget( "mainlabel", xmLabelWidgetClass,
   labelForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  XtAddEventHandler( labelForm,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask, False,
   efEventHandler, (XtPointer) this );

  topForm = XtVaCreateWidget( "topform", xmFormWidgetClass, pane,
   XmNallowResize, True,
   XmNpaneMaximum, 10000,
   XmNmarginWidth, 50,
   NULL );

  curTopParent = topForm;

  XtAddEventHandler( topForm,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask, False,
   efEventHandler, (XtPointer) this );

  controlForm = XtVaCreateWidget( "controlform", xmFormWidgetClass, pane,
   XmNallowResize, True,
   XmNmarginWidth, 50,
   NULL );

  arrayForm = XtVaCreateWidget( "arrayform", xmFormWidgetClass, pane,
   XmNallowResize, True,
   XmNmarginWidth, 50,
   NULL );

  bottomForm = XtVaCreateWidget( "bottomform", xmFormWidgetClass, paneTop,
   XmNallowResize, True,
   XmNskipAdjust, True,
   NULL );

  XtAddEventHandler( bottomForm,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask, False,
   efEventHandler, (XtPointer) this );

  return 1;

}

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
  const char *actionFontTag )
{

int stat;

  stat = create( top, _x, _y, _w, _h, _largestH, fi,
   entryFontTag, actionFontTag );

  XSetWindowColormap( display, XtWindow(shell), cmap );

  return stat;

}

int entryFormClass::create (
  Widget top,
  int *_x,
  int *_y,
  int *_w,
  int *_h,
  int *_largestH,
  fontInfoClass *fi,
  const char *entryFontTag,
  const char *actionFontTag )
{

int stat;

  stat = create ( top, _x, _y, _w, _h, _largestH, "No Label", fi, entryFontTag,
   actionFontTag );

  return stat;

}

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
  const char *actionFontTag )
{

int stat;

  stat = create( top, _x, _y, _w, _h, _largestH, label,
   _maxItems, _numItems, _setItem_cb, objPtr,
   fi, entryFontTag, actionFontTag );

  XSetWindowColormap( display, XtWindow(shell), cmap );

  return stat;

}

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
  const char *actionFontTag )
{

XmString str;
char buf[16];

   setItem_cb = _setItem_cb;
   setItemDsc.obj = objPtr;
   setItemDsc.ef = this;

   x = _x;
   y = _y;
   w = _w;
   h = _h;
   largestH = _largestH;

  display = XtDisplay( top );

  buttonClickTime = 0;
  index = 0;
  oldIndex = 0;
  maxItems = _maxItems;
  numItems = _numItems;

  firstItem = 1;
  firstArrayItem = 1;
  firstColorButton = 1;
  curWidgetIsLabel = 0;
  leftAttachmentExists = 0;

  strncpy( title, label, 31 );
  title[31] = 0;

  if ( fi ) {

    if ( entryFontTag ) {
//       fprintf( stderr, "entryFormClass::create - new char[strlen(entryFontTag)+1]\n" );
      entryTag = new char[strlen(entryFontTag)+1];
      strcpy( entryTag, entryFontTag );
      fi->getTextFontList( entryTag, &entryFontList );
    }

    if ( actionFontTag ) {
//       fprintf( stderr, "entryFormClass::create - new char[strlen(actionFontTag)+1]\n" );
      actionTag = new char[strlen(actionFontTag)+1];
      strcpy( actionTag, actionFontTag );
      fi->getTextFontList( actionTag, &actionFontList );
    }

  }

  //shell = XtVaCreatePopupShell( label, xmDialogShellWidgetClass,
  shell = XtVaCreatePopupShell( "properties", xmDialogShellWidgetClass,
   top,
   XmNmappedWhenManaged, False,
   NULL );

  paneTop = XtVaCreateWidget( "paneTop", xmPanedWindowWidgetClass, shell,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   NULL );

  scrollWin = XtVaCreateWidget( "scrollwin", xmScrolledWindowWidgetClass, paneTop,
   XmNallowResize, True,
   XmNpaneMaximum, 10000,
   XmNscrollBarDisplayPolicy, XmAS_NEEDED,
   XmNscrollingPolicy, XmAUTOMATIC,
   NULL );

  pane = XtVaCreateWidget( "pane", xmPanedWindowWidgetClass, scrollWin,
   XmNsashWidth, 1,
   XmNsashHeight, 1,
   NULL );

  labelForm = XtVaCreateWidget( "labelform", xmFormWidgetClass, pane,
   XmNallowResize, True,
   NULL );

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  mainLabel = XtVaCreateManagedWidget( "mainlabel", xmLabelWidgetClass,
   labelForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  XtAddEventHandler( labelForm,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask, False,
   efEventHandler, (XtPointer) this );

  topForm = XtVaCreateWidget( "topform", xmFormWidgetClass, pane,
   XmNallowResize, True,
   XmNpaneMaximum, 10000,
   XmNmarginWidth, 50,
   NULL );

  curTopParent = topForm;

  XtAddEventHandler( topForm,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask, False,
   efEventHandler, (XtPointer) this );

  controlForm = XtVaCreateWidget( "controlform", xmFormWidgetClass, pane,
   XmNallowResize, True,
   XmNmarginWidth, 50,
   NULL );

  if ( maxItems > 1 ) {

    // number of items

    numItemsArrowInc = XtVaCreateManagedWidget( "arrow", xmArrowButtonGadgetClass,
     controlForm,
     XmNtopOffset, 5,
     XmNarrowDirection, XmARROW_RIGHT,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     NULL );

    if ( setItem_cb && setItemDsc.obj ) {
      XtAddCallback( numItemsArrowInc, XmNactivateCallback,
      ef_increment_num_items, (void *) &setItemDsc );
    }

    sprintf( buf, "%-d", numItems );

    numItemsText = XtVaCreateManagedWidget( "numitemstext", xmTextFieldWidgetClass,
     controlForm,
     XmNcolumns, (short) 3,
     XmNvalue, buf,
     XmNmaxLength, (short) 10,
     XmNtopAttachment, XmATTACH_FORM,
  //    XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
  //    XmNtopWidget, numItemsArrowDec,
     XmNrightAttachment, XmATTACH_WIDGET,
     XmNrightWidget, numItemsArrowInc,
     //XmNfontList, entryFontList,
     NULL );

    if ( setItem_cb && setItemDsc.obj ) {
      XtAddCallback( numItemsText, XmNvalueChangedCallback,
       ef_set_num_items, (void *) &setItemDsc );
    }

    numItemsArrowDec = XtVaCreateManagedWidget( "arrow", xmArrowButtonGadgetClass,
     controlForm,
     XmNarrowDirection, XmARROW_LEFT,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, numItemsArrowInc,
     XmNrightAttachment, XmATTACH_WIDGET,
     XmNrightWidget, numItemsText,
     NULL );

    if ( setItem_cb && setItemDsc.obj ) {
      XtAddCallback( numItemsArrowDec, XmNactivateCallback,
       ef_decrement_num_items, (void *) &setItemDsc );
    }

    str = XmStringCreateLocalized( "Number of Items" );

    numItemsLabel = XtVaCreateManagedWidget( "numitemslabel", xmLabelWidgetClass,
     controlForm,
     XmNmarginTop, 7,
     XmNlabelString, str,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, numItemsText,
     XmNrightAttachment, XmATTACH_WIDGET,
     XmNrightWidget, numItemsArrowDec,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    // item number

    itemNumArrowInc = XtVaCreateManagedWidget( "arrow", xmArrowButtonGadgetClass,
     controlForm,
     XmNtopOffset, 5,
     XmNarrowDirection, XmARROW_RIGHT,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, numItemsText,
     XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNleftWidget, numItemsArrowInc,
     NULL );

    if ( setItem_cb && setItemDsc.obj ) {
      XtAddCallback( itemNumArrowInc, XmNactivateCallback,
      ef_increment_item_num, (void *) &setItemDsc );
    }

    itemNumText = XtVaCreateManagedWidget( "itemnumtext", xmTextFieldWidgetClass,
     controlForm,
     XmNcolumns, (short) 3,
     XmNvalue, "1",
     XmNmaxLength, (short) 10,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, numItemsText,
     XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNleftWidget, numItemsText,
     //XmNfontList, entryFontList,
     NULL );

    if ( setItem_cb && setItemDsc.obj ) {
      XtAddCallback( itemNumText, XmNvalueChangedCallback,
       ef_set_item_num, (void *) &setItemDsc );
    }

    itemNumArrowDec = XtVaCreateManagedWidget( "arrow", xmArrowButtonGadgetClass,
     controlForm,
     XmNarrowDirection, XmARROW_LEFT,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, itemNumArrowInc,
     XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNleftWidget, numItemsArrowDec,
     //XmNfontList, entryFontList,
     NULL );

    if ( setItem_cb && setItemDsc.obj ) {
      XtAddCallback( itemNumArrowDec, XmNactivateCallback,
       ef_decrement_item_num, (void *) &setItemDsc );
    }

    str = XmStringCreateLocalized( "Item Number" );

    itemNumLabel = XtVaCreateManagedWidget( "itemnumlabel", xmLabelWidgetClass,
     controlForm,
     XmNmarginTop, 7,
     XmNlabelString, str,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, numItemsText,
     XmNrightAttachment, XmATTACH_WIDGET,
     XmNrightWidget, itemNumArrowDec,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

  }

  arrayForm = XtVaCreateWidget( "arrayform", xmFormWidgetClass, pane,
   XmNallowResize, True,
   XmNmarginWidth, 50,
   NULL );

//    XtAddEventHandler( controlForm,
//     KeyPressMask|ButtonPressMask|ButtonReleaseMask, False,
//     efEventHandler, (XtPointer) this );

  XtAddEventHandler( arrayForm,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask, False,
   efEventHandler, (XtPointer) this );

  bottomForm = XtVaCreateWidget( "bottomform", xmFormWidgetClass, paneTop,
   XmNskipAdjust, True,
   NULL );

  XtAddEventHandler( bottomForm,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask, False,
   efEventHandler, (XtPointer) this );

  return 1;

}

int entryFormClass::addEmbeddedEf (
  char *label,
  entryFormClass **ef )
{

  return addEmbeddedEf( label, NULL, ef );

}

int entryFormClass::addEmbeddedEf (
  char *label,
  char *buttonLabel,
  entryFormClass **ef )
{

XmString str;

embeddedEfEntry *cur;

  cur = new embeddedEfEntry;

  // cur->ef = ef;
  *ef = &cur->ef;

  if ( buttonLabel ) {
    str = XmStringCreateLocalized( buttonLabel );
  }
  else {
    str = XmStringCreateLocalized( "" );
  }

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW = XtVaCreateManagedWidget( "embeddedformpb", xmPushButtonWidgetClass,
     topForm,
     XmNnavigationType, XmTAB_GROUP,
     //XmNwidth, 25,
     XmNheight, 25,
     XmNlabelString, str,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     NULL );

    curW = cur->activeW;
    curRW = cur->activeW;

  }
  else {

    cur->activeW = XtVaCreateManagedWidget( "embeddedformpb", xmPushButtonWidgetClass,
     topForm,
     XmNnavigationType, XmTAB_GROUP,
     //XmNwidth, 25,
     XmNheight, 25,
     XmNlabelString, str,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNleftWidget, curW,
     NULL );

    curW = cur->activeW;

  }

  XmStringFree( str );

  XtAddCallback( cur->activeW, XmNactivateCallback, embeddedEfPopup_cb,
   cur );

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addFontMenuGeneric (
  int includeAlignInfo,
  char *label,
  fontInfoClass *fi,
  fontMenuClass *fm,
  char *initFontTag )
{

XmString str;
Arg args[10];
int n;

fontMenuEntry *cur;

  cur = new fontMenuEntry;

  if ( firstItem ) {

    firstItem = 0;

    n = 0;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNrightAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNmarginWidth, (XtArgVal) 0 ); n++;

    cur->activeW = fm->createFontMenu( topForm, fi, args, n,
     includeAlignInfo );
    fm->setFontTag( initFontTag );

    curW = cur->activeW;
    curRW = cur->activeW;

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, (XtArgVal) curW ); n++;

    if ( leftAttachmentExists ) {

      XtSetArg( args[n], XmNleftAttachment,
       (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
      XtSetArg( args[n], XmNleftWidget, (XtArgVal) curRW ); n++;

    }
    else {

      XtSetArg( args[n], XmNrightAttachment,
       (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
      XtSetArg( args[n], XmNrightWidget, (XtArgVal) curRW ); n++;

    }

    XtSetArg( args[n], XmNmarginWidth, (XtArgVal) 0 ); n++;

    cur->activeW = fm->createFontMenu( topForm, fi, args, n,
     includeAlignInfo );
    fm->setFontTag( initFontTag );

    curW = cur->activeW;
    curRW = cur->activeW;

  }

  cur->fmo = fm;


  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );


  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addFontMenu (
  char *label,
  fontInfoClass *fi,
  fontMenuClass *fm,
  char *initFontTag )
{

  return addFontMenuGeneric( 1, label, fi, fm, initFontTag );

}

int entryFormClass::addFontMenuNoAlignInfo (
  char *label,
  fontInfoClass *fi,
  fontMenuClass *fm,
  char *initFontTag )
{

  return addFontMenuGeneric( 0, label, fi, fm, initFontTag );

}

void TextFieldToStringPW (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
XmTextVerifyCallbackStruct *xmv;

  teo = (class textEntry *) client;

  xmv = (XmTextVerifyCallbackStruct *) call;
  Strncat( teo->charDest, xmv->text->ptr, teo->maxLen );
  xmv->doit = False;

}

void TextBoxToString (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
char *buf;

  teo = (class textEntry *) client;

  buf = XmTextGetString( w );
  strncpy( teo->charDest, buf, teo->maxLen );
  XtFree( buf );


}

void TextFieldToString (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
char *buf;

  teo = (class textEntry *) client;

  buf = XmTextGetString( w );
  strncpy( teo->charDest, buf, teo->maxLen );
  XtFree( buf );

}

void TextFieldToInt (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
char *buf;
int *dest;
XmTextPosition pos;

  teo = (class textEntry *) client;
  dest = teo->destPtrI;

  buf = XmTextGetString( w );

  if ( blank(buf) || isLegalInteger( buf ) ) {
    *dest = strtol( buf, NULL, 0 );
    strncpy( teo->lastGoodNumeric, buf, 31 );
    teo->lastGoodNumeric[31] = 0;
  }
  else {
    pos = XmTextGetCursorPosition( w );
    XmTextSetString( w, teo->lastGoodNumeric );
    if ( pos > 0 ) pos -= 1;
    XmTextSetCursorPosition( w, pos );
  }

  XtFree( buf );

}

void TextFieldToEfInt (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
char *buf, *tk, *ctx;
efInt *dest;
int i;
XmTextPosition pos;

  teo = (class textEntry *) client;
  dest = teo->destPtrEfI;

  buf = XmTextGetString( w );

  if ( blank(buf) || isLegalInteger( buf ) ) {
    strncpy( teo->lastGoodNumeric, buf, 31 );
    teo->lastGoodNumeric[31] = 0;
    i = strtol( buf, NULL, 0 );
    dest->setValue( i );
    ctx = NULL;
    tk = strtok_r( buf, " \t\n", &ctx );
    if ( tk ) {
      dest->setNull( 0 );
    }
    else {
      dest->setNull( 1 );
    }
  }
  else {
    pos = XmTextGetCursorPosition( w );
    XmTextSetString( w, teo->lastGoodNumeric );
    if ( pos > 0 ) pos -= 1;
    XmTextSetCursorPosition( w, pos );
  }

  XtFree( buf );

}

void TextFieldToDouble (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
char *buf;
double *dest;
XmTextPosition pos;

  teo = (class textEntry *) client;
  dest = teo->destPtrD;

  buf = XmTextGetString( w );

  if ( blank(buf) || isLegalFloat( buf ) ) {
    *dest = atof( buf );
    strncpy( teo->lastGoodNumeric, buf, 31 );
    teo->lastGoodNumeric[31] = 0;
  }
  else {
    pos = XmTextGetCursorPosition( w );
    XmTextSetString( w, teo->lastGoodNumeric );
    if ( pos > 0 ) pos -= 1;
    XmTextSetCursorPosition( w, pos );
  }

  XtFree( buf );

}

void TextFieldToEfDouble (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
char *buf, *tk, *ctx;
efDouble *dest;
double d;
XmTextPosition pos;

  teo = (class textEntry *) client;
  dest = teo->destPtrEfD;

  buf = XmTextGetString( w );

  if ( blank(buf) || isLegalFloat( buf ) ) {
    strncpy( teo->lastGoodNumeric, buf, 31 );
    teo->lastGoodNumeric[31] = 0;
    d = atof( buf );
    dest->setValue( d );
    ctx = NULL;
    tk = strtok_r( buf, " \t\n", &ctx );
    if ( tk ) {
      dest->setNull( 0 );
    }
    else {
      dest->setNull( 1 );
    }
  }
  else {
    pos = XmTextGetCursorPosition( w );
    XmTextSetString( w, teo->lastGoodNumeric );
    if ( pos > 0 ) pos -= 1;
    XmTextSetCursorPosition( w, pos );
  }

  XtFree( buf );

}

void TextFieldToStringArray (
  Widget w,
  XtPointer client,
  XtPointer call )
{

efArrayCallbackDscPtr dsc;
char *buf, **destArray;
int i;

  dsc = (efArrayCallbackDscPtr) client;

  destArray = (char **) dsc->destPtr;
  i = *(dsc->indexPtr);

//   fprintf( stderr, "In TextFieldToStringArray, index = %-d\n", i );

  buf = XmTextGetString( w );
  strncpy( destArray[i], buf, dsc->size );
  XtFree( buf );

}

void TextFieldToIntArray (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
efArrayCallbackDscPtr dsc;
char *buf;
int *destArray;
int i, value;

  teo = (class textEntry *) client;
  dsc = &(teo->arrayDsc);

  destArray = (int *) dsc->destPtr;
  i = *(dsc->indexPtr);

//   fprintf( stderr, "In TextFieldToIntArray, index = %-d\n", i );

  buf = XmTextGetString( w );
  //value = atol( buf );
  value = strtol( buf, NULL, 0 );
  XtFree( buf );

  destArray[i] = value;

}

void TextFieldToEfIntArray (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
efArrayCallbackDscPtr dsc;
char *buf, *tk, *ctx;
efInt *destArray;
int i, value;

  teo = (class textEntry *) client;
  dsc = &(teo->arrayDsc);

  destArray = (efInt *) dsc->destPtr;
  i = *(dsc->indexPtr);

  buf = XmTextGetString( w );
  //value = atol( buf );
  value = strtol( buf, NULL, 0 );
  ctx = NULL;
  tk = strtok_r( buf, " \t\n", &ctx );
  if ( tk ) {
    destArray[i].setNull( 0 );
    destArray[i].setValue( value );
  }
  else {
    destArray[i].setNull( 1 );
  }
  XtFree( buf );

}

void TextFieldToDoubleArray (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
efArrayCallbackDscPtr dsc;
char *buf;
double *destArray;
int i;
double value;

  teo = (class textEntry *) client;
  dsc = &(teo->arrayDsc);

  destArray = (double *) dsc->destPtr;
  i = *(dsc->indexPtr);

//   fprintf( stderr, "In TextFieldToDoubleArray, index = %-d\n", i );

  buf = XmTextGetString( w );
  value = atof( buf );
  XtFree( buf );

  destArray[i] = value;

}

void TextFieldToEfDoubleArray (
  Widget w,
  XtPointer client,
  XtPointer call )
{

class textEntry *teo;
efArrayCallbackDscPtr dsc;
char *buf, *tk, *ctx;
efDouble *destArray;
int i;
double value;

  teo = (class textEntry *) client;
  dsc = &(teo->arrayDsc);

  destArray = (efDouble *) dsc->destPtr;
  i = *(dsc->indexPtr);

  buf = XmTextGetString( w );
  value = atof( buf );
  ctx = NULL;
  tk = strtok_r( buf, " \t\n", &ctx );
  if ( tk ) {
    destArray[i].setNull( 0 );
    destArray[i].setValue( value );
  }
  else {
    destArray[i].setNull( 1 );
  }

  XtFree( buf );

}

int entryFormClass::addTextField (
  char *label,
  int length,
  int *dest )
{

textEntry *cur;
XmString str;
char buf[127+1];

  leftAttachmentExists = 1;

  sprintf( buf, "%-d", *dest );

  cur = new textEntry;

  // textField widget

  cur->destPtrI = dest;
  strncpy( cur->lastGoodNumeric, buf, 31 );
  cur->lastGoodNumeric[31] = 0;

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curRW,
     //XmNfontList, entryFontList,
     NULL );

    curW = cur->activeW;
    curRW = cur->activeW;

  }

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, cur->labelW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, cur->labelW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, prevW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, prevW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }

  }

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, TextFieldToInt,
   cur );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addTextField (
  char *label,
  int length,
  efInt *dest )
{

textEntry *cur;
XmString str;
char buf[127+1];

  leftAttachmentExists = 1;

  if ( dest->isNull() )
    strcpy( buf, "" );
  else
    sprintf( buf, "%-d", dest->value() );

  cur = new textEntry;

  // textField widget

  cur->destPtrEfI = dest;
  strncpy( cur->lastGoodNumeric, buf, 31 );
  cur->lastGoodNumeric[31] = 0;

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curRW,
     //XmNfontList, entryFontList,
     NULL );

    curW = cur->activeW;
    curRW = cur->activeW;

  }

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, cur->labelW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, cur->labelW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, prevW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, prevW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }

  }

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, TextFieldToEfInt,
   cur );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addTextField (
  char *label,
  int length,
  double *dest )
{

textEntry *cur;
XmString str;
char buf[127+1];

  leftAttachmentExists = 1;

  sprintf( buf, "%-g", *dest );

//   fprintf( stderr, "entryFormClass::addTextField - new textEntry\n" );

  cur = new textEntry;

  // textField widget

  cur->destPtrD = dest;
  strncpy( cur->lastGoodNumeric, buf, 31 );
  cur->lastGoodNumeric[31] = 0;

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curRW,
     //XmNfontList, entryFontList,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, cur->labelW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, cur->labelW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, prevW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, prevW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }

  }

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, TextFieldToDouble,
   cur );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

class textEntry* entryFormClass::addTextFieldAccessible (
  char *label,
  int length,
  double *dest )
{

textEntry *cur;
XmString str;
char buf[127+1];

  leftAttachmentExists = 1;

  sprintf( buf, "%-g", *dest );

  // fprintf( stderr, "entryFormClass::addTextField - new textEntry\n" );

  cur = new textEntry;

  // textField widget

  cur->destPtrD = dest;
  strncpy( cur->lastGoodNumeric, buf, 31 );
  cur->lastGoodNumeric[31] = 0;

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curRW,
     //XmNfontList, entryFontList,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, cur->labelW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, cur->labelW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, prevW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, prevW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }

  }

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, TextFieldToDouble,
   cur );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return cur;

}

int entryFormClass::addTextField (
  char *label,
  int length,
  efDouble *dest )
{

textEntry *cur;
XmString str;
char buf[127+1];

  leftAttachmentExists = 1;

  if ( dest->isNull() )
    strcpy( buf, "" );
  else
    sprintf( buf, "%-g", dest->value() );

  cur = new textEntry;

  cur->destPtrEfD = dest;
  strncpy( cur->lastGoodNumeric, buf, 31 );
  cur->lastGoodNumeric[31] = 0;

  // textField widget

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curRW,
     //XmNfontList, entryFontList,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, cur->labelW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, cur->labelW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, prevW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, prevW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }

  }

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, TextFieldToEfDouble,
   cur );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addTextField (
  char *label,
  int length,
  char *dest,
  int stringSize )
{

textEntry *cur;
XmString str;

  leftAttachmentExists = 1;

  cur = new textEntry;

  // textField widget

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, dest,
     XmNmaxLength, stringSize,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, dest,
     XmNmaxLength, stringSize,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curRW,
     //XmNfontList, entryFontList,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }

  cur->charDest = dest;
  cur->maxLen = stringSize;

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, dest,
     XmNmaxLength, stringSize,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, cur->labelW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, cur->labelW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, dest,
     XmNmaxLength, stringSize,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, prevW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, prevW,
     //XmNfontList, entryFontList,
     NULL );

    prevW = cur->activeW;

  }

  cur->charDest = dest;
  cur->maxLen = stringSize;

  }

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, TextFieldToString,
   cur );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addGenericTextBox (
  int edit,
  char *label,
  int width,
  int height,
  char *dest,
  int stringSize )
{

textEntry *cur;
XmString str;
int n;
Arg args[20];
Widget scrolledTextWidget = NULL;

  leftAttachmentExists = 1;

  cur = new textEntry;

  // text widget

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    n = 0;
    XtSetArg( args[n], XmNrows, height ); n++;
    XtSetArg( args[n], XmNcolumns, width ); n++;
    if ( edit ) {
      XtSetArg( args[n], XmNeditable, True ); n++;
    }
    else {
      XtSetArg( args[n], XmNeditable, False ); n++;
    }
    XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
    if ( edit ) {
      XtSetArg( args[n], XmNcursorPositionVisible, True ); n++;
    }
    else {
      XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
    }
    XtSetArg( args[n], XmNmaxLength, stringSize ); n++;
    XtSetArg( args[n], XmNtopAttachment, XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNrightAttachment, XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNvalue, dest ); n++;
    XtSetArg( args[n], XmNwordWrap, False ); n++;

    scrolledTextWidget = cur->activeW =
     XmCreateScrolledText( topForm, "scrolledtext", args, n );

    curW = cur->activeW;
    curRW = cur->activeW;

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNrows, height ); n++;
    XtSetArg( args[n], XmNcolumns, width ); n++;
    if ( edit ) {
      XtSetArg( args[n], XmNeditable, True ); n++;
    }
    else {
      XtSetArg( args[n], XmNeditable, False ); n++;
    }
    XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
    if ( edit ) {
      XtSetArg( args[n], XmNcursorPositionVisible, True ); n++;
    }
    else {
      XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
    }
    XtSetArg( args[n], XmNmaxLength, stringSize ); n++;
    XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, curW ); n++;
    XtSetArg( args[n], XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNrightWidget, curRW ); n++;
    XtSetArg( args[n], XmNvalue, dest ); n++;
    XtSetArg( args[n], XmNwordWrap, False ); n++;

    scrolledTextWidget = cur->activeW =
     XmCreateScrolledText( topForm, "scrolledtext", args, n );

    curW = cur->activeW;
    curRW = cur->activeW;

  }

  cur->charDest = dest;
  cur->maxLen = stringSize;

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     NULL );

    XmStringFree( str );

    n = 0;
    XtSetArg( args[n], XmNrows, height ); n++;
    XtSetArg( args[n], XmNcolumns, width ); n++;
    if ( edit ) {
      XtSetArg( args[n], XmNeditable, True ); n++;
    }
    else {
      XtSetArg( args[n], XmNeditable, False ); n++;
    }
    XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
    if ( edit ) {
      XtSetArg( args[n], XmNcursorPositionVisible, True ); n++;
    }
    else {
      XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
    }
    XtSetArg( args[n], XmNmaxLength, stringSize ); n++;
    XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, cur->labelW ); n++;
    XtSetArg( args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, cur->labelW ); n++;
    XtSetArg( args[n], XmNmarginTop, 7 ); n++;
    XtSetArg( args[n], XmNvalue, dest ); n++;
    XtSetArg( args[n], XmNwordWrap, False ); n++;

    cur->activeW = XmCreateScrolledText( curTopParent, "scrolledtext", args, n );

    prevW = cur->activeW;

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNrows, height ); n++;
    XtSetArg( args[n], XmNcolumns, width ); n++;
    if ( edit ) {
      XtSetArg( args[n], XmNeditable, True ); n++;
    }
    else {
      XtSetArg( args[n], XmNeditable, False ); n++;
    }
    XtSetArg( args[n], XmNeditMode, XmMULTI_LINE_EDIT ); n++;
    if ( edit ) {
      XtSetArg( args[n], XmNcursorPositionVisible, True ); n++;
    }
    else {
      XtSetArg( args[n], XmNcursorPositionVisible, False ); n++;
    }
    XtSetArg( args[n], XmNmaxLength, stringSize ); n++;
    XtSetArg( args[n], XmNtopAttachment, XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, prevW ); n++;
    XtSetArg( args[n], XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, prevW ); n++;
    XtSetArg( args[n], XmNmarginTop, 7 ); n++;
    XtSetArg( args[n], XmNvalue, dest ); n++;
    XtSetArg( args[n], XmNwordWrap, False ); n++;

    cur->activeW = XmCreateScrolledText( curTopParent, "scrolledtext", args, n );

    prevW = cur->activeW;

  }

  cur->charDest = dest;
  cur->maxLen = stringSize;

  }

  if ( scrolledTextWidget ) XtManageChild( scrolledTextWidget );

  if ( edit ) XtAddCallback( cur->activeW, XmNvalueChangedCallback,
   TextBoxToString, cur );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addTextBox (
  char *label,
  int width,
  int height,
  char *dest,
  int stringSize )
{

  return addGenericTextBox( 1, label, width, height, dest, stringSize );

}

int entryFormClass::addReadonlyTextBox (
  char *label,
  int width,
  int height,
  char *dest,
  int stringSize )
{

  return addGenericTextBox( 0, label, width, height, dest, stringSize );

}

int entryFormClass::addPasswordField (
  char *label,
  int length,
  char *dest,
  int stringSize )
{

textEntry *cur;
XmString str;

  leftAttachmentExists = 1;

  cur = new textEntry;

  // textField widget

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, dest,
     XmNmaxLength, stringSize,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     XmNverifyBell, False,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, dest,
     XmNmaxLength, stringSize,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curRW,
     //XmNfontList, entryFontList,
     XmNverifyBell, False,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }

  cur->charDest = dest;
  cur->maxLen = stringSize;

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, dest,
     XmNmaxLength, stringSize,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, cur->labelW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, cur->labelW,
     //XmNfontList, entryFontList,
     XmNverifyBell, False,
     NULL );

    prevW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, dest,
     XmNmaxLength, stringSize,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, prevW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, prevW,
     //XmNfontList, entryFontList,
     XmNverifyBell, False,
     NULL );

    prevW = cur->activeW;

  }

  cur->charDest = dest;
  cur->maxLen = stringSize;

  }

  XtAddCallback( cur->activeW, XmNmodifyVerifyCallback, TextFieldToStringPW,
   cur );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addLockedField (
  char *label,
  int length,
  char *dest,
  int stringSize )
{

textEntry *cur;
XmString str;

  leftAttachmentExists = 1;

  cur = new textEntry;

  // textField widget

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, "<LOCKED>",
     XmNmaxLength, stringSize,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     XmNeditable, False,
     XmNcursorPositionVisible, False,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     topForm,
     XmNcolumns, (short) length,
     XmNvalue, "<LOCKED>",
     XmNmaxLength, stringSize,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curRW,
     //XmNfontList, entryFontList,
     XmNeditable, False,
     XmNcursorPositionVisible, False,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }

  cur->charDest = NULL;
  cur->maxLen = 0;

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, "<LOCKED>",
     XmNmaxLength, stringSize,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, cur->labelW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, cur->labelW,
     //XmNfontList, entryFontList,
     XmNeditable, False,
     XmNcursorPositionVisible, False,
     NULL );

    prevW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     curTopParent,
     XmNcolumns, (short) length,
     XmNvalue, "<LOCKED>",
     XmNmaxLength, stringSize,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, prevW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, prevW,
     //XmNfontList, entryFontList,
     XmNeditable, False,
     XmNcursorPositionVisible, False,
     NULL );

    prevW = cur->activeW;

  }

  cur->charDest = NULL;
  cur->maxLen = 0;

  }

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

void OptionToString (
  Widget w,
  XtPointer client,
  XtPointer call )
{

widgetListPtr cur;

  cur = (widgetListPtr) client;
  strncpy( (char *) cur->destination, cur->value, cur->size );

}

void OptionToInt (
  Widget w,
  XtPointer client,
  XtPointer call )
{

widgetListPtr cur;

  cur = (widgetListPtr) client;
  *( (int *) cur->destination ) = cur->entryNumber;

}

void OptionToStringArray (
  Widget w,
  XtPointer client,
  XtPointer call )
{

widgetListPtr curpb;
efArrayCallbackDscPtr dsc;
char *value, **destArray;
int i;

  curpb = (widgetListPtr) client;
  dsc = &curpb->arrayDsc;
  destArray = (char **) dsc->destPtr;
  i = *(dsc->indexPtr);
  value = (char *) dsc->valuePtr;

//   fprintf( stderr, "In OptionToStringArray\n" );
//   fprintf( stderr, "i = %-d\n", i );
//   fprintf( stderr, "value = [%s]\n", value );

  strncpy( destArray[i], value, dsc->size );

}

void OptionToIntArray (
  Widget w,
  XtPointer client,
  XtPointer call )
{

widgetListPtr curpb;
efArrayCallbackDscPtr dsc;
int i, *destArray;
long value;

  curpb = (widgetListPtr) client;
  dsc = &curpb->arrayDsc;
  destArray = (int *) dsc->destPtr;
  i = *(dsc->indexPtr);
  value = (long) dsc->valuePtr;

//   fprintf( stderr, "In OptionToIntArray\n" );
//   fprintf( stderr, "i = %-d\n", i );
//   fprintf( stderr, "old value = %-d\n", destArray[i] );
//   fprintf( stderr, "new value = %-d\n", value );

  destArray[i] = (int) value;

}

#if 1
int entryFormClass::addColorButton (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest )
{

XmString str, str1, str2;
Arg fArgs[15], bArgs[15], nbArgs[15], tArgs[15];
int fN, bN, nbN, tN;

colorButtonEntry *cur;

  cur = new colorButtonEntry;

  cur->theCb = cb;

  if ( curTopParent == topForm ) {

  //fprintf( stderr, "using topForm\n" );

  if ( firstItem ) {

    firstItem = 0;

    fN = 0;
    XtSetArg( fArgs[fN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); fN++;
    XtSetArg( fArgs[fN], XmNrightAttachment, (XtArgVal) XmATTACH_FORM ); fN++;

    bN = 0;
    XtSetArg( bArgs[bN], XmNnavigationType, XmTAB_GROUP ); bN++;
    XtSetArg( bArgs[bN], XmNwidth, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNheight, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNbackground, (XtArgVal) ci->pix(*dest) ); bN++;
    XtSetArg( bArgs[bN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); bN++;
    XtSetArg( bArgs[bN], XmNleftAttachment, (XtArgVal) XmATTACH_FORM ); bN++;

    str1 = XmStringCreateLocalized( "*" );

    if ( ci->isRule(*dest) ) {
      XtSetArg( bArgs[bN], XmNforeground,
       (XtArgVal) ci->labelPix(*dest) ); bN++;
      XtSetArg( bArgs[bN], XmNlabelString, (XtArgVal) str1 ); bN++;
    }

    str2 = XmStringCreateLocalized( ci->colorName(*dest) );

    nbN = 0;
    XtSetArg( nbArgs[nbN], XmNnavigationType, XmTAB_GROUP ); nbN++;
    //XtSetArg( nbArgs[nbN], XmNwidth, (XtArgVal) 130 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNheight, (XtArgVal) 25 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNrecomputeSize, (XtArgVal) 1 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNlabelString, (XtArgVal) str2 ); nbN++;

    tN = 0;

    cb->createWithRule( topForm, dest, ci, NULL, fArgs, fN, bArgs, bN,
     nbArgs, nbN, tArgs, tN );

    XmStringFree( str1 );
    XmStringFree( str2 );

    curW = cb->formWidget();
    curRW = cb->formWidget();

  }
  else {

    fN = 0;
    XtSetArg( fArgs[fN], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET ); fN++;
    XtSetArg( fArgs[fN], XmNtopWidget, (XtArgVal) curW ); fN++;
    XtSetArg( fArgs[fN], XmNleftAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); fN++;
    XtSetArg( fArgs[fN], XmNleftWidget, (XtArgVal) curW ); fN++;

    bN = 0;
    XtSetArg( bArgs[bN], XmNnavigationType, XmTAB_GROUP ); bN++;
    XtSetArg( bArgs[bN], XmNwidth, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNheight, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNbackground, (XtArgVal) ci->pix(*dest) ); bN++;
    XtSetArg( bArgs[bN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); bN++;
    XtSetArg( bArgs[bN], XmNleftAttachment, (XtArgVal) XmATTACH_FORM ); bN++;

    str1 = XmStringCreateLocalized( "*" );

    if ( ci->isRule(*dest) ) {
      XtSetArg( bArgs[bN], XmNforeground,
       (XtArgVal) ci->labelPix(*dest) ); bN++;
      XtSetArg( bArgs[bN], XmNlabelString, (XtArgVal) str1 ); bN++;
    }

    str2 = XmStringCreateLocalized( ci->colorName(*dest) );

    nbN = 0;
    XtSetArg( nbArgs[nbN], XmNnavigationType, XmTAB_GROUP ); nbN++;
    //XtSetArg( nbArgs[nbN], XmNwidth, (XtArgVal) 130 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNheight, (XtArgVal) 25 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNrecomputeSize, (XtArgVal) 1 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNlabelString, (XtArgVal) str2 ); nbN++;

    tN = 0;

    cb->createWithRule( topForm, dest, ci, NULL, fArgs, fN, bArgs, bN,
     nbArgs, nbN, tArgs, tN );

    XmStringFree( str1 );
    XmStringFree( str2 );

    curW = cb->formWidget();

  }

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  //fprintf( stderr, "using subForm\n" );

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    //fprintf( stderr, "first subFrom\n" );

    fN = 0;
    XtSetArg( fArgs[fN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); fN++;
    XtSetArg( fArgs[fN], XmNleftAttachment, (XtArgVal) XmATTACH_FORM ); fN++;

    bN = 0;
    XtSetArg( bArgs[bN], XmNnavigationType, XmTAB_GROUP ); bN++;
    XtSetArg( bArgs[bN], XmNwidth, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNheight, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNbackground, (XtArgVal) ci->pix(*dest) ); bN++;
    XtSetArg( bArgs[bN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); bN++;
    XtSetArg( bArgs[bN], XmNleftAttachment, (XtArgVal) XmATTACH_FORM ); bN++;

    str1 = XmStringCreateLocalized( "*" );

    if ( ci->isRule(*dest) ) {
      XtSetArg( bArgs[bN], XmNforeground,
       (XtArgVal) ci->labelPix(*dest) ); bN++;
      XtSetArg( bArgs[bN], XmNlabelString, (XtArgVal) str1 ); bN++;
    }

    str2 = XmStringCreateLocalized( ci->colorName(*dest) );

    nbN = 0;
    XtSetArg( nbArgs[nbN], XmNnavigationType, XmTAB_GROUP ); nbN++;
    //XtSetArg( nbArgs[nbN], XmNwidth, (XtArgVal) 130 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNheight, (XtArgVal) 25 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNrecomputeSize, (XtArgVal) 1 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNlabelString, (XtArgVal) str2 ); nbN++;

    tN = 0;

    cb->createWithRule( curTopParent, dest, ci, NULL, fArgs, fN, bArgs, bN,
     nbArgs, nbN, tArgs, tN );

    XmStringFree( str1 );
    XmStringFree( str2 );

    prevW = cb->formWidget();

  }
  else {

    //fprintf( stderr, "not first subFrom\n" );

    fN = 0;
    XtSetArg( fArgs[fN], XmNtopAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); fN++;
    XtSetArg( fArgs[fN], XmNtopWidget, (XtArgVal) prevW ); fN++;
    XtSetArg( fArgs[fN], XmNleftAttachment,
     (XtArgVal) XmATTACH_WIDGET ); fN++;
    XtSetArg( fArgs[fN], XmNleftWidget, (XtArgVal) prevW ); fN++;

    bN = 0;
    XtSetArg( bArgs[bN], XmNnavigationType, XmTAB_GROUP ); bN++;
    XtSetArg( bArgs[bN], XmNwidth, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNheight, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNbackground, (XtArgVal) ci->pix(*dest) ); bN++;
    XtSetArg( bArgs[bN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); bN++;
    XtSetArg( bArgs[bN], XmNleftAttachment, (XtArgVal) XmATTACH_FORM ); bN++;

    str1 = XmStringCreateLocalized( "*" );

    if ( ci->isRule(*dest) ) {
      XtSetArg( bArgs[bN], XmNforeground,
       (XtArgVal) ci->labelPix(*dest) ); bN++;
      XtSetArg( bArgs[bN], XmNlabelString, (XtArgVal) str1 ); bN++;
    }

    str2 = XmStringCreateLocalized( ci->colorName(*dest) );

    nbN = 0;
    XtSetArg( nbArgs[nbN], XmNnavigationType, XmTAB_GROUP ); nbN++;
    //XtSetArg( nbArgs[nbN], XmNwidth, (XtArgVal) 130 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNheight, (XtArgVal) 25 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNrecomputeSize, (XtArgVal) 1 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNlabelString, (XtArgVal) str2 ); nbN++;

    tN = 0;

    cb->createWithRule( curTopParent, dest, ci, NULL, fArgs, fN, bArgs, bN,
     nbArgs, nbN, tArgs, tN );

    XmStringFree( str1 );
    XmStringFree( str2 );

    prevW = cb->formWidget();

  }

#if 0
  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   curTopParent,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, prevW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, prevW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  prevW = cur->labelW;

  XmStringFree( str );
#endif

  }

  if ( firstColorButton ) {
    firstColorButton = 0;
    ci->setActiveWidget( cb->widget() );
    ci->setCurDestination( dest );
  }

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}
#endif

#if 0
int entryFormClass::addColorButton (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest )
{

XmString str;
Arg args[10];
int n;

colorButtonEntry *cur;

  cur = new colorButtonEntry;

  cur->theCb = cb;

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNwidth, (XtArgVal) 25 ); n++;
    XtSetArg( args[n], XmNheight, (XtArgVal) 25 ); n++;
    XtSetArg( args[n], XmNbackground, (XtArgVal) ci->pix(*dest) ); n++;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNrightAttachment, (XtArgVal) XmATTACH_FORM ); n++;

    cb->create( topForm, dest, ci, args, n );

    curW = cb->widget();
    curRW = cb->widget();

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNwidth, (XtArgVal) 25 ); n++;
    XtSetArg( args[n], XmNheight, (XtArgVal) 25 ); n++;
    XtSetArg( args[n], XmNbackground, (XtArgVal) ci->pix(*dest) ); n++;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, (XtArgVal) curW ); n++;
    XtSetArg( args[n], XmNleftAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, (XtArgVal) curW ); n++;

    cb->create( topForm, dest, ci, args, n );

    curW = cb->widget();

  }

  if ( firstColorButton ) {
    firstColorButton = 0;
    ci->setActiveWidget( curW );
    ci->setCurDestination( dest );
  }


  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNwidth, (XtArgVal) 25 ); n++;
    XtSetArg( args[n], XmNheight, (XtArgVal) 25 ); n++;
    XtSetArg( args[n], XmNbackground, (XtArgVal) ci->pix(*dest) ); n++;
    XtSetArg( args[n], XmNmarginTop, (XtArgVal) 7 ); n++;
    XtSetArg( args[n], XmNtopAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, (XtArgVal) cur->labelW ); n++;
    XtSetArg( args[n], XmNleftAttachment,
     (XtArgVal) XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, (XtArgVal) cur->labelW ); n++;

    cb->create( curTopParent, dest, ci, args, n );

    prevW = cb->widget();

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNwidth, (XtArgVal) 25 ); n++;
    XtSetArg( args[n], XmNheight, (XtArgVal) 25 ); n++;
    XtSetArg( args[n], XmNbackground, (XtArgVal) ci->pix(*dest) ); n++;
    XtSetArg( args[n], XmNmarginTop, (XtArgVal) 7 ); n++;
    XtSetArg( args[n], XmNtopAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, (XtArgVal) prevW ); n++;
    XtSetArg( args[n], XmNleftAttachment,
     (XtArgVal) XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, (XtArgVal) prevW ); n++;

    cb->create( curTopParent, dest, ci, args, n );

    prevW = cb->widget();

  }

  if ( firstColorButton ) {
    firstColorButton = 0;
    ci->setActiveWidget( prevW );
    ci->setCurDestination( dest );
  }

  }

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}
#endif

int entryFormClass::addColorButtonWithText (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest,
  int numCols,
  char *pvName )
{

XmString str;
Arg fArgs[10], bArgs[10], tArgs[15];
int fN, bN, tN;

colorButtonEntry *cur;
textEntry *te;

  cur = new colorButtonEntry;

  cur->theCb = cb;

  if ( firstItem ) {

    firstItem = 0;

    fN = 0;
    XtSetArg( fArgs[fN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); fN++;
    XtSetArg( fArgs[fN], XmNrightAttachment, (XtArgVal) XmATTACH_FORM ); fN++;

    bN = 0;
    XtSetArg( bArgs[bN], XmNnavigationType, XmTAB_GROUP ); bN++;
    XtSetArg( bArgs[bN], XmNwidth, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNheight, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNbackground, (XtArgVal) ci->pix(*dest) ); bN++;
    XtSetArg( bArgs[bN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); bN++;
    XtSetArg( bArgs[bN], XmNleftAttachment, (XtArgVal) XmATTACH_FORM ); bN++;

    tN = 0;
    XtSetArg( tArgs[tN], XmNnavigationType, XmTAB_GROUP ); tN++;
    XtSetArg( tArgs[tN], XmNhighlightThickness, 0 ); tN++;
    XtSetArg( tArgs[tN], XmNcolumns, (short) numCols ); tN++;
    XtSetArg( tArgs[tN], XmNpendingDelete, True ); tN++;
    //XtSetArg( tArgs[tN], XmNfontList, entryFontList ); tN++;

    if ( pvName )
      cb->createWithText( topForm, dest, ci, pvName, fArgs, fN, bArgs, bN,
       tArgs, tN );
    else
      cb->createWithText( topForm, dest, ci, "", fArgs, fN, bArgs, bN,
       tArgs, tN );

    te = new textEntry;
    te->charDest = cb->getPv();
    te->maxLen = cb->PvSize();
    XtAddCallback( cb->textWidget(), XmNvalueChangedCallback,
      TextFieldToString, te );

    curW = cb->formWidget();
    curRW = cb->formWidget();

  }
  else {

    fN = 0;
    XtSetArg( fArgs[fN], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET ); fN++;
    XtSetArg( fArgs[fN], XmNtopWidget, (XtArgVal) curW ); fN++;
    XtSetArg( fArgs[fN], XmNleftAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); fN++;
    XtSetArg( fArgs[fN], XmNleftWidget, (XtArgVal) curW ); fN++;

    bN = 0;
    XtSetArg( bArgs[bN], XmNnavigationType, XmTAB_GROUP ); bN++;
    XtSetArg( bArgs[bN], XmNwidth, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNheight, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNbackground, (XtArgVal) ci->pix(*dest) ); bN++;
    XtSetArg( bArgs[bN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); bN++;
    XtSetArg( bArgs[bN], XmNleftAttachment, (XtArgVal) XmATTACH_FORM ); bN++;

    tN = 0;
    XtSetArg( tArgs[tN], XmNnavigationType, XmTAB_GROUP ); tN++;
    XtSetArg( tArgs[tN], XmNhighlightThickness, 0 ); tN++;
    XtSetArg( tArgs[tN], XmNcolumns, (short) numCols ); tN++;
    XtSetArg( tArgs[tN], XmNvalue, cb->getPv() ); tN++;
    XtSetArg( tArgs[tN], XmNmaxLength, (short) cb->PvSize() ); tN++;
    XtSetArg( tArgs[tN], XmNpendingDelete, True ); tN++;
    //XtSetArg( tArgs[tN], XmNfontList, entryFontList ); tN++;

    if ( pvName )
      cb->createWithText( topForm, dest, ci, pvName, fArgs, fN, bArgs, bN,
       tArgs, tN );
    else
      cb->createWithText( topForm, dest, ci, "", fArgs, fN, bArgs, bN,
       tArgs, tN );

    te = new textEntry;
    te->charDest = cb->getPv();
    te->maxLen = cb->PvSize();
    XtAddCallback( cb->textWidget(), XmNvalueChangedCallback,
      TextFieldToString, te );

    curW = cb->formWidget();

  }

  if ( firstColorButton ) {
    firstColorButton = 0;
    ci->setActiveWidget( cb->widget() );
    ci->setCurDestination( dest );
  }

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addColorButtonWithRule (
  char *label,
  colorInfoClass *ci,
  colorButtonClass *cb,
  int *dest,
  int numCols,
  char *pvName )
{

XmString str, str1, str2;
Arg fArgs[15], bArgs[15], nbArgs[15], tArgs[15];
int fN, bN, nbN, tN;

colorButtonEntry *cur;
textEntry *te;

  cur = new colorButtonEntry;

  cur->theCb = cb;

  if ( firstItem ) {

    firstItem = 0;

    fN = 0;
    XtSetArg( fArgs[fN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); fN++;
    XtSetArg( fArgs[fN], XmNrightAttachment, (XtArgVal) XmATTACH_FORM ); fN++;

    bN = 0;
    XtSetArg( bArgs[bN], XmNnavigationType, XmTAB_GROUP ); bN++;
    XtSetArg( bArgs[bN], XmNwidth, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNheight, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNbackground, (XtArgVal) ci->pix(*dest) ); bN++;
    XtSetArg( bArgs[bN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); bN++;
    XtSetArg( bArgs[bN], XmNleftAttachment, (XtArgVal) XmATTACH_FORM ); bN++;

    str1 = XmStringCreateLocalized( "*" );

    if ( ci->isRule(*dest) ) {
      XtSetArg( bArgs[bN], XmNforeground,
       (XtArgVal) ci->labelPix(*dest) ); bN++;
      XtSetArg( bArgs[bN], XmNlabelString, (XtArgVal) str1 ); bN++;
    }

    str2 = XmStringCreateLocalized( ci->colorName(*dest) );

    nbN = 0;
    XtSetArg( nbArgs[nbN], XmNnavigationType, XmTAB_GROUP ); nbN++;
    //XtSetArg( nbArgs[nbN], XmNwidth, (XtArgVal) 130 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNheight, (XtArgVal) 25 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNrecomputeSize, (XtArgVal) 1 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNlabelString, (XtArgVal) str2 ); nbN++;

    tN = 0;
    XtSetArg( tArgs[tN], XmNnavigationType, XmTAB_GROUP ); tN++;
    XtSetArg( tArgs[tN], XmNhighlightThickness, 0 ); tN++;
    XtSetArg( tArgs[tN], XmNcolumns, (short) numCols ); tN++;
    XtSetArg( tArgs[tN], XmNpendingDelete, True ); tN++;
    //XtSetArg( tArgs[tN], XmNfontList, entryFontList ); tN++;

    cb->createWithRule( topForm, dest, ci, pvName, fArgs, fN, bArgs, bN,
     nbArgs, nbN, tArgs, tN );

    XmStringFree( str1 );
    XmStringFree( str2 );

    if ( pvName ) {
      te = new textEntry;
      te->charDest = cb->getPv();
      te->maxLen = cb->PvSize();
      XtAddCallback( cb->textWidget(), XmNvalueChangedCallback,
        TextFieldToString, te );
    }

    curW = cb->formWidget();
    curRW = cb->formWidget();

  }
  else {

    fN = 0;
    XtSetArg( fArgs[fN], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET ); fN++;
    XtSetArg( fArgs[fN], XmNtopWidget, (XtArgVal) curW ); fN++;
    XtSetArg( fArgs[fN], XmNleftAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); fN++;
    XtSetArg( fArgs[fN], XmNleftWidget, (XtArgVal) curW ); fN++;

    bN = 0;
    XtSetArg( bArgs[bN], XmNnavigationType, XmTAB_GROUP ); bN++;
    XtSetArg( bArgs[bN], XmNwidth, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNheight, (XtArgVal) 25 ); bN++;
    XtSetArg( bArgs[bN], XmNbackground, (XtArgVal) ci->pix(*dest) ); bN++;
    XtSetArg( bArgs[bN], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); bN++;
    XtSetArg( bArgs[bN], XmNleftAttachment, (XtArgVal) XmATTACH_FORM ); bN++;

    str1 = XmStringCreateLocalized( "*" );

    if ( ci->isRule(*dest) ) {
      XtSetArg( bArgs[bN], XmNforeground,
       (XtArgVal) ci->labelPix(*dest) ); bN++;
      XtSetArg( bArgs[bN], XmNlabelString, (XtArgVal) str1 ); bN++;
    }

    str2 = XmStringCreateLocalized( ci->colorName(*dest) );

    nbN = 0;
    XtSetArg( nbArgs[nbN], XmNnavigationType, XmTAB_GROUP ); nbN++;
    //XtSetArg( nbArgs[nbN], XmNwidth, (XtArgVal) 130 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNheight, (XtArgVal) 25 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNrecomputeSize, (XtArgVal) 1 ); nbN++;
    XtSetArg( nbArgs[nbN], XmNlabelString, (XtArgVal) str2 ); nbN++;

    tN = 0;
    XtSetArg( tArgs[tN], XmNnavigationType, XmTAB_GROUP ); tN++;
    XtSetArg( tArgs[tN], XmNhighlightThickness, 0 ); tN++;
    XtSetArg( tArgs[tN], XmNcolumns, (short) numCols ); tN++;
    XtSetArg( tArgs[tN], XmNvalue, cb->getPv() ); tN++;
    XtSetArg( tArgs[tN], XmNmaxLength, (short) cb->PvSize() ); tN++;
    XtSetArg( tArgs[tN], XmNpendingDelete, True ); tN++;
    //XtSetArg( tArgs[tN], XmNfontList, entryFontList ); tN++;

    cb->createWithRule( topForm, dest, ci, pvName, fArgs, fN, bArgs, bN,
     nbArgs, nbN, tArgs, tN );

    XmStringFree( str1 );
    XmStringFree( str2 );

    if ( pvName ) {
      te = new textEntry;
      te->charDest = cb->getPv();
      te->maxLen = cb->PvSize();
      XtAddCallback( cb->textWidget(), XmNvalueChangedCallback,
        TextFieldToString, te );
    }

    curW = cb->formWidget();

  }

  if ( firstColorButton ) {
    firstColorButton = 0;
    ci->setActiveWidget( cb->widget() );
    ci->setCurDestination( dest );
  }

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addOption (
  char *label,
  char *options,
  char *dest,
  int stringSize )
{

char *buf, *tk, *ctx;
optionEntry *cur;
XmString str;
Arg args[10];
int n;
Widget curHistoryWidget;
widgetListPtr curpb;

// init

  cur = new optionEntry;

  cur->pd = XmCreatePulldownMenu( curTopParent, "pulldown", NULL, 0 );

  buf = new char[strlen(options)+1];
  strcpy( buf, options );

  curHistoryWidget = NULL;


  // create all pushbuttons for the option menu

  n = 0;
  ctx = NULL;
  tk = strtok_r( buf, "|", &ctx );
  while ( tk ) {

    cur->numValues++;

    curpb = new widgetListType;
    curpb->destination = (void *) dest;
    curpb->entryNumber = n++;
    curpb->size = stringSize;
    curpb->value = new char[strlen(tk)+1];
    strcpy( curpb->value, tk );

    if ( entryTag )
      str = XmStringCreate( tk, entryTag );
    else
      str = XmStringCreateLocalized( tk );

    curpb->w = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
     cur->pd,
     XmNlabelString, str,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    if ( strcmp( dest, tk ) == 0 ) curHistoryWidget = curpb->w;

    XtAddCallback( curpb->w, XmNactivateCallback, OptionToString,
     (XtPointer) curpb );

    cur->tail->flink = curpb;
    cur->tail = curpb;
    cur->tail->flink = NULL;

    tk = strtok_r( NULL, "|", &ctx );

  }

  delete[] buf;

  // create the option menu

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNrightAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    cur->activeW = XmCreateOptionMenu( topForm, "menu", args, n );

    curW = cur->activeW;
    curRW = cur->activeW;

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, (XtArgVal) curW ); n++;
    XtSetArg( args[n], XmNleftAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, (XtArgVal) curRW ); n++;

    cur->activeW = XmCreateOptionMenu( topForm, "menu", args, n );

    curW = cur->activeW;

  }

  XtManageChild( curW );

  // create label

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNmarginTop, (XtArgVal) 7 ); n++;
    XtSetArg( args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, cur->labelW ); n++;
    XtSetArg( args[n], XmNleftAttachment, XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, cur->labelW ); n++;
    cur->activeW = XmCreateOptionMenu( curTopParent, "menu", args, n );

    prevW = cur->activeW;

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNmarginTop, (XtArgVal) 7 ); n++;
    XtSetArg( args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, prevW ); n++;
    XtSetArg( args[n], XmNleftAttachment, XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, prevW ); n++;
    cur->activeW = XmCreateOptionMenu( curTopParent, "menu", args, n );

    prevW = cur->activeW;

  }

  XtManageChild( prevW );

  }

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addOption (
  char *label,
  char *options,
  int *dest )
{

char *buf, *tk, *ctx;
optionEntry *cur;
XmString str;
Arg args[10];
int n;
Widget curHistoryWidget;
widgetListPtr curpb;

// init

  cur = new optionEntry;

  cur->pd = XmCreatePulldownMenu( curTopParent, "pulldown", NULL, 0 );

  buf = new char[strlen(options)+1];
  strcpy( buf, options );

  curHistoryWidget = NULL;


  // create all pushbuttons for the option menu

  n = 0;
  ctx = NULL;
  tk = strtok_r( buf, "|", &ctx );
  while ( tk ) {

    cur->numValues++;

    curpb = new widgetListType;
    curpb->destination = (void *) dest;
    curpb->entryNumber = n;
    curpb->size = sizeof(int);
    curpb->value = new char[strlen(tk)+1];
    strcpy( curpb->value, tk );

    if ( entryTag )
      str = XmStringCreate( tk, entryTag );
    else
      str = XmStringCreateLocalized( tk );

    curpb->w = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
     cur->pd,
     XmNlabelString, str,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    if ( *dest == n ) curHistoryWidget = curpb->w;

    XtAddCallback( curpb->w, XmNactivateCallback, OptionToInt,
     (XtPointer) curpb );

    cur->tail->flink = curpb;
    cur->tail = curpb;
    cur->tail->flink = NULL;

    tk = strtok_r( NULL, "|", &ctx );

    n++;

  }

  delete[] buf;


  // create the option menu

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNrightAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    cur->activeW = XmCreateOptionMenu( curTopParent, "menu", args, n );

    curW = cur->activeW;
    curRW = cur->activeW;

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, (XtArgVal) curW ); n++;
    XtSetArg( args[n], XmNleftAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, (XtArgVal) curRW ); n++;
    cur->activeW = XmCreateOptionMenu( curTopParent, "menu", args, n );

    curW = cur->activeW;

  }

  XtManageChild( curW );

  // create label

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   curTopParent,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    if ( entryTag )
      str = XmStringCreate( label, entryTag );
    else
      str = XmStringCreateLocalized( label );

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNmarginTop, (XtArgVal) 7 ); n++;
    XtSetArg( args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, cur->labelW ); n++;
    XtSetArg( args[n], XmNleftAttachment, XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, cur->labelW ); n++;
    cur->activeW = XmCreateOptionMenu( curTopParent, "menu", args, n );

    prevW = cur->activeW;

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNmarginTop, (XtArgVal) 7 ); n++;
    XtSetArg( args[n], XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, prevW ); n++;
    XtSetArg( args[n], XmNleftAttachment, XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, prevW ); n++;
    cur->activeW = XmCreateOptionMenu( curTopParent, "menu", args, n );

    prevW = cur->activeW;

  }

  XtManageChild( prevW );

  }

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addOptionArray (
  char *label,
  char *options,
  char **dest,
  int stringSize,
  entryListBase **obj )
{

char *buf, *tk, *ctx;
optionEntry *cur;
XmString str;
Arg args[10];
int n;
Widget curHistoryWidget;
widgetListPtr curpb;

// init

  cur = new optionEntry;
  *obj = cur;

  cur->pd = XmCreatePulldownMenu( arrayForm, "pulldown", NULL, 0 );

  buf = new char[strlen(options)+1];
  strcpy( buf, options );

  curHistoryWidget = NULL;


  // create all pushbuttons for the option menu

  n = 0;
  ctx = NULL;
  tk = strtok_r( buf, "|", &ctx );
  while ( tk ) {

    cur->numValues++;

    curpb = new widgetListType;
    curpb->destination = (void *) dest;
    curpb->entryNumber = n++;
    curpb->size = stringSize;
    curpb->value = new char[strlen(tk)+1];
    strcpy( curpb->value, tk );

    if ( entryTag )
      str = XmStringCreate( tk, entryTag );
    else
      str = XmStringCreateLocalized( tk );

    curpb->w = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
     cur->pd,
     XmNlabelString, str,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    if ( strcmp( dest[0], tk ) == 0 ) curHistoryWidget = curpb->w;

    curpb->arrayDsc.valuePtr = curpb->value;
    curpb->arrayDsc.indexPtr = &index;
    curpb->arrayDsc.destPtr = (void *) dest;
    curpb->arrayDsc.size = stringSize;

    XtAddCallback( curpb->w, XmNactivateCallback, OptionToStringArray,
     (XtPointer) curpb );

    cur->tail->flink = curpb;
    cur->tail = curpb;
    cur->tail->flink = NULL;

    tk = strtok_r( NULL, "|", &ctx );

  }

  delete[] buf;


  // create the option menu

  if ( firstArrayItem ) {

    firstArrayItem = 0;

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNrightAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    cur->activeW = XmCreateOptionMenu( arrayForm, "menu", args, n );

    curArrayW = cur->activeW;
    curArrayRW = cur->activeW;

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, (XtArgVal) curArrayW ); n++;
    XtSetArg( args[n], XmNleftAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, (XtArgVal) curArrayRW ); n++;
    cur->activeW = XmCreateOptionMenu( arrayForm, "menu", args, n );

    curArrayW = cur->activeW;

  }

  XtManageChild( curArrayW );

  // create label

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   arrayForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curArrayW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curArrayW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );


  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addOptionArray (
  char *label,
  char *options,
  int *dest,
  entryListBase **obj )
{

char *buf, *tk, *ctx;
optionEntry *cur;
XmString str;
Arg args[10];
int n;
Widget curHistoryWidget;
widgetListPtr curpb;

// init

  cur = new optionEntry;
  *obj = cur;

  cur->pd = XmCreatePulldownMenu( arrayForm, "pulldown", NULL, 0 );

  buf = new char[strlen(options)+1];
  strcpy( buf, options );

  curHistoryWidget = NULL;


  // create all pushbuttons for the option menu

  n = 0;
  ctx = NULL;
  tk = strtok_r( buf, "|", &ctx );
  while ( tk ) {

    cur->numValues++;

    curpb = new widgetListType;
    curpb->destination = (void *) dest;
    curpb->entryNumber = n;
    curpb->size = sizeof(int);
    curpb->value = new char[strlen(tk)+1];
    strcpy( curpb->value, tk );

    if ( entryTag )
      str = XmStringCreate( tk, entryTag );
    else
      str = XmStringCreateLocalized( tk );

    curpb->w = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
     cur->pd,
     XmNlabelString, str,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    if ( *dest == n ) curHistoryWidget = curpb->w;

    curpb->arrayDsc.valuePtr = (void *) curpb->entryNumber;
    curpb->arrayDsc.indexPtr = &index;
    curpb->arrayDsc.destPtr = (void *) dest;
    curpb->arrayDsc.size = sizeof(int);

    XtAddCallback( curpb->w, XmNactivateCallback, OptionToIntArray,
     (XtPointer) curpb );

    cur->tail->flink = curpb;
    cur->tail = curpb;
    cur->tail->flink = NULL;

    tk = strtok_r( NULL, "|", &ctx );

    n++;

  }

  delete[] buf;


  // create the option menu

  if ( firstArrayItem ) {

    firstArrayItem = 0;

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    XtSetArg( args[n], XmNrightAttachment, (XtArgVal) XmATTACH_FORM ); n++;
    cur->activeW = XmCreateOptionMenu( arrayForm, "menu", args, n );

    curArrayW = cur->activeW;
    curArrayRW = cur->activeW;

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNnavigationType, XmTAB_GROUP ); n++;
    XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->pd ); n++;
    XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget ); n++;
    XtSetArg( args[n], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET ); n++;
    XtSetArg( args[n], XmNtopWidget, (XtArgVal) curArrayW ); n++;
    XtSetArg( args[n], XmNleftAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( args[n], XmNleftWidget, (XtArgVal) curArrayRW ); n++;
    cur->activeW = XmCreateOptionMenu( arrayForm, "menu", args, n );

    curArrayW = cur->activeW;

  }

  XtManageChild( curArrayW );

  // create label

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   arrayForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curArrayW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curArrayW,
   XmNmarginTop, 7,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );


  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

void toggleToInt (
  Widget w,
  XtPointer client,
  XtPointer call )
{

toggleEntry *cur = (toggleEntry *) client;
XmToggleButtonCallbackStruct *toggleCallback =
 (XmToggleButtonCallbackStruct *) call;

  if ( toggleCallback->set )
    cur->value = 1;
  else
    cur->value = 0;

  *( (int *) cur->destination ) = cur->value;

}

void toggleToIntArray (
  Widget w,
  XtPointer client,
  XtPointer call )
{

efArrayCallbackDscPtr dsc;
int i, *destArray;
toggleEntry *cur = (toggleEntry *) client;
XmToggleButtonCallbackStruct *toggleCallback =
 (XmToggleButtonCallbackStruct *) call;

  cur = (toggleEntry *) client;
  dsc = &cur->arrayDsc;
  destArray = (int *) dsc->destPtr;
  i = *(dsc->indexPtr);

  if ( toggleCallback->set )
    cur->value = 1;
  else
    cur->value = 0;

  destArray[i] = cur->value;

}

int entryFormClass::addToggle (
  char *label,
  int *dest )
{

toggleEntry *cur;
XmString str;
Arg args[5];
int n;

  cur = new toggleEntry;

  if ( *dest )
    cur->value = 1;
  else
    cur->value = 0;

  // togglebutton widget

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  if ( curTopParent  == topForm ) {

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "toggle", xmToggleButtonWidgetClass,
     curTopParent,
     XmNnavigationType, XmTAB_GROUP,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     XmNlabelString, str,
     //XmNfontList, entryFontList,
     XmNindicatorOn, XmINDICATOR_CROSS_BOX,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "toggle", xmToggleButtonWidgetClass,
     curTopParent,
     XmNnavigationType, XmTAB_GROUP,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNleftWidget, curRW,
     XmNlabelString, str,
     //XmNfontList, entryFontList,
     XmNindicatorOn, XmINDICATOR_CROSS_BOX,
     NULL );

    curW = cur->activeW;

  }

  XmStringFree( str );

  }
  else {

  if ( firstSubFormChild ) {

    firstSubFormChild = 0;

    cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
     curTopParent,
     XmNlabelString, str,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

    XmStringFree( str );

    cur->activeW =  XtVaCreateManagedWidget( "toggle", xmToggleButtonWidgetClass,
     curTopParent,
     XmNnavigationType, XmTAB_GROUP,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, cur->labelW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, cur->labelW,
     XmNlabelString, str,
     //XmNfontList, entryFontList,
     XmNindicatorOn, XmINDICATOR_CROSS_BOX,
     NULL );

    prevW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "toggle", xmToggleButtonWidgetClass,
     curTopParent,
     XmNnavigationType, XmTAB_GROUP,
     XmNmarginTop, 7,
     XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNtopWidget, prevW,
     XmNleftAttachment, XmATTACH_WIDGET,
     XmNleftWidget, prevW,
     XmNlabelString, str,
     //XmNfontList, entryFontList,
     XmNindicatorOn, XmINDICATOR_CROSS_BOX,
     NULL );

    prevW = cur->activeW;

  }

  }

  n = 0;
  if ( cur->value ) {
    XtSetArg( args[n], XmNset, True ); n++;
  }
  else {
    XtSetArg( args[n], XmNset, False ); n++;
  }

  XtSetValues( cur->activeW, args, n );

  cur->destination = (void *) dest;

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, toggleToInt,
   cur );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addToggleArray (
  char *label,
  int *dest,
  entryListBase **obj )
{

toggleEntry *cur;
XmString str;
Arg args[5];
int n;

  cur = new toggleEntry;

  if ( dest[0] )
    cur->value = 1;
  else
    cur->value = 0;

  // togglebutton widget

  if ( firstItem ) {

    firstItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "toggle", xmToggleButtonWidgetClass,
     topForm,
     XmNnavigationType, XmTAB_GROUP,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     XmNindicatorOn, XmINDICATOR_CROSS_BOX,
     NULL );

     curW = cur->activeW;
     curRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "toggle", xmToggleButtonWidgetClass,
     topForm,
     XmNnavigationType, XmTAB_GROUP,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curRW,
     XmNindicatorOn, XmINDICATOR_CROSS_BOX,
     NULL );

    curW = cur->activeW;

  }

  n = 0;
  if ( cur->value ) {
    XtSetArg( args[n], XmNset, True ); n++;
  }
  else {
    XtSetArg( args[n], XmNset, False ); n++;
  }

  XtSetValues( cur->activeW, args, n );

  cur->arrayDsc.indexPtr = &index;
  cur->arrayDsc.destPtr = (void *) dest;
  cur->arrayDsc.size = sizeof(int);

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, toggleToIntArray,
   cur );

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

static void kill_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

efWidgetAndPointerPtr wp = (efWidgetAndPointerPtr) client;
entryFormClass *eo = (entryFormClass *) wp->obj;
XtCallbackProc cb = eo->cancelCb;

  (*cb)( wp->w, wp->client, NULL );

}

int entryFormClass::finished (
  XtCallbackProc ok_cb,
  XtCallbackProc apply_cb,
  XtCallbackProc cancel_cb,
  XtPointer ptr )
{

XmString str;
int num;
Widget *children;

  okCb = ok_cb;
  applyCb = apply_cb;
  cancelCb = cancel_cb;
  pbCallbackPtr = ptr;

  if ( actionTag )
    str = XmStringCreate( "Cancel", actionTag );
  else
    str = XmStringCreateLocalized( "Cancel" );

  pb_cancel = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass, bottomForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNbottomAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   //XmNfontList, actionFontList,
   NULL );

  XmStringFree( str );

  this->wp.w = pb_cancel;
  this->wp.obj = this;
  this->wp.client = ptr;
  XtAddCallback( pb_cancel, XmNactivateCallback, kill_cb, &this->wp );

  Atom wm_delete_window = XmInternAtom( XtDisplay(shell),
   "WM_DELETE_WINDOW", False );

  XmAddWMProtocolCallback( shell, wm_delete_window, kill_cb, &this->wp );

  XtVaSetValues( shell, XmNdeleteResponse, XmDO_NOTHING, NULL );

//   XtAddCallback( pb_cancel, XmNactivateCallback, cancel_cb, ptr );

  if ( actionTag )
    str = XmStringCreate( "Apply", actionTag );
  else
    str = XmStringCreateLocalized( "Apply" );

  pb_apply = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass,  bottomForm,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, pb_cancel,
   XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNbottomWidget, pb_cancel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, pb_cancel,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   //XmNfontList, actionFontList,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb_apply, XmNactivateCallback, apply_cb, ptr );



  if ( actionTag )
    str = XmStringCreate( "OK", actionTag );
  else
    str = XmStringCreateLocalized( "OK" );

  pb_ok = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass, bottomForm,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, pb_apply,
   XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNbottomWidget, pb_apply,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, pb_apply,
   XmNleftAttachment, XmATTACH_NONE,
   XmNshowAsDefault, True,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   //XmNfontList, actionFontList,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb_ok, XmNactivateCallback, ok_cb, ptr );
//   XtAddCallback( pb_ok, XmNactivateCallback, popdown_cb, this );


  XtManageChild( labelForm );
  if ( !firstItem ) XtManageChild( topForm );
  if ( maxItems > 1 ) XtManageChild( controlForm );
  if ( maxItems > 0 ) XtManageChild( arrayForm );
  XtManageChild( bottomForm );
  XtManageChild( pane );
  XtManageChild( scrollWin );
  XtManageChild( paneTop );

  // remove pane sashes from tab traversal
  XtVaGetValues( pane,
   XmNchildren, &children,
   XmNnumChildren, &num,
   NULL );

  while ( num-- > 0 ) {
    if ( XmIsSash( children[num] ) ) {
      XtVaSetValues( children[num],
       XmNtraversalOn, False,
       NULL );
    }
  }

  return 1;

}

void entryFormClass::ok_callback (
  Widget w,
  XtPointer client,
  XtPointer call
) {

  entryFormClass *ef = (entryFormClass *) client;
  client = ef->callbackPtr;
  ef->callbackData.command = entryFormClass::OK;
  (ef->clientCb)( w, client, XtPointer (&ef->callbackData) );

}

void entryFormClass::apply_callback (
  Widget w,
  XtPointer client,
  XtPointer call
) {

  entryFormClass *ef = (entryFormClass *) client;
  client = ef->callbackPtr;
  ef->callbackData.command = entryFormClass::APPLY;
  (ef->clientCb)( w, client, XtPointer (&ef->callbackData) );

}

void entryFormClass::cancel_callback (
  Widget w,
  XtPointer client,
  XtPointer call
) {

  entryFormClass *ef = (entryFormClass *) client;
  client = ef->callbackPtr;
  ef->callbackData.command = entryFormClass::CANCEL;
  (ef->clientCb)( w, client, XtPointer (&ef->callbackData) );

}

int entryFormClass::finished (
  int operationType,
  XtCallbackProc cb,
  XtPointer ptr )
{

XmString str;
int num;
Widget *children;
XtCallbackProc ok_cb, apply_cb, cancel_cb;

  callbackPtr = ptr;
  clientCb = cb;
  ptr = this;
  callbackData.op = operationType;
  ok_cb = entryFormClass::ok_callback;
  apply_cb = entryFormClass::apply_callback;
  cancel_cb = entryFormClass::cancel_callback;

  okCb = ok_cb;
  applyCb = apply_cb;
  cancelCb = cancel_cb;
  pbCallbackPtr = ptr;

  if ( actionTag )
    str = XmStringCreate( "Cancel", actionTag );
  else
    str = XmStringCreateLocalized( "Cancel" );

  pb_cancel = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass, bottomForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNbottomAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  this->wp.w = pb_cancel;
  this->wp.obj = this;
  this->wp.client = ptr;
  XtAddCallback( pb_cancel, XmNactivateCallback, kill_cb, &this->wp );

  Atom wm_delete_window = XmInternAtom( XtDisplay(shell),
   "WM_DELETE_WINDOW", False );

  XmAddWMProtocolCallback( shell, wm_delete_window, kill_cb, &this->wp );

  XtVaSetValues( shell, XmNdeleteResponse, XmDO_NOTHING, NULL );

  if ( actionTag )
    str = XmStringCreate( "Apply", actionTag );
  else
    str = XmStringCreateLocalized( "Apply" );

  pb_apply = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass,  bottomForm,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, pb_cancel,
   XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNbottomWidget, pb_cancel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, pb_cancel,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb_apply, XmNactivateCallback, apply_cb, ptr );

  if ( actionTag )
    str = XmStringCreate( "OK", actionTag );
  else
    str = XmStringCreateLocalized( "OK" );

  pb_ok = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass, bottomForm,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, pb_apply,
   XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNbottomWidget, pb_apply,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, pb_apply,
   XmNleftAttachment, XmATTACH_NONE,
   XmNshowAsDefault, True,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb_ok, XmNactivateCallback, ok_cb, ptr );

  XtManageChild( labelForm );
  if ( !firstItem ) XtManageChild( topForm );
  if ( maxItems > 1 ) XtManageChild( controlForm );
  if ( maxItems > 0 ) XtManageChild( arrayForm );
  XtManageChild( bottomForm );
  XtManageChild( pane );
  XtManageChild( scrollWin );
  XtManageChild( paneTop );

  // remove pane sashes from tab traversal
  XtVaGetValues( pane,
   XmNchildren, &children,
   XmNnumChildren, &num,
   NULL );

  while ( num-- > 0 ) {
    if ( XmIsSash( children[num] ) ) {
      XtVaSetValues( children[num],
       XmNtraversalOn, False,
       NULL );
    }
  }

  return 1;

}

int entryFormClass::finished (
  XtCallbackProc close_cb,
  XtPointer ptr )
{

XmString str;
int num;
Widget *children;

  okCb = close_cb;
  applyCb = close_cb;
  cancelCb = close_cb;
  pbCallbackPtr = ptr;

  if ( actionTag )
    str = XmStringCreate( "Close", actionTag );
  else
    str = XmStringCreateLocalized( "Close" );

  pb_ok = XtVaCreateManagedWidget( "pb", xmPushButtonGadgetClass, bottomForm,
   XmNtopAttachment, XmATTACH_FORM,
   XmNbottomAttachment, XmATTACH_FORM,
   XmNrightAttachment, XmATTACH_FORM,
   XmNshowAsDefault, True,
   XmNdefaultButtonShadowThickness, 1,
   XmNlabelString, str,
   //XmNfontList, actionFontList,
   NULL );

  XmStringFree( str );

  XtAddCallback( pb_ok, XmNactivateCallback, close_cb, ptr );

  Atom wm_delete_window = XmInternAtom( XtDisplay(shell),
   "WM_DELETE_WINDOW", False );

  XmAddWMProtocolCallback( shell, wm_delete_window, close_cb, ptr );

  XtVaSetValues( shell, XmNdeleteResponse, XmDO_NOTHING, NULL );

  XtManageChild( labelForm );
  if ( !firstItem ) XtManageChild( topForm );
  if ( maxItems > 1 ) XtManageChild( controlForm );
  if ( maxItems > 0 ) XtManageChild( arrayForm );
  XtManageChild( bottomForm );
  XtManageChild( pane );
  XtManageChild( scrollWin );
  XtManageChild( paneTop );

  // remove pane sashes from tab traversal
  XtVaGetValues( pane,
   XmNchildren, &children,
   XmNnumChildren, &num,
   NULL );

  while ( num-- > 0 ) {
    if ( XmIsSash( children[num] ) ) {
      XtVaSetValues( children[num],
       XmNtraversalOn, False,
       NULL );
    }
  }

  return 1;

}

int entryFormClass::popup ( void ) {

Arg args[5];
int n;
short paneW = 0, paneH = 0, botH = 0;
XTextProperty xtext;
char *pTitle;

 if ( ( *x != 0 ) ) {
   n = 0;
   XtSetArg( args[n], XmNx, (XtArgVal) *x ); n++;
   XtSetValues( shell, args, n );
 }

 if ( ( *y != 0 ) ) {
   n = 0;
   XtSetArg( args[n], XmNy, (XtArgVal) *y ); n++;
   XtSetValues( shell, args, n );
 }

  n = 0;
  XtSetArg( args[n], XmNheight, (XtArgVal) &botH ); n++;
  XtGetValues( bottomForm, args, n );

  n = 0;
  XtSetArg( args[n], XmNwidth, (XtArgVal) &paneW ); n++;
  XtSetArg( args[n], XmNheight, (XtArgVal) &paneH ); n++;
  XtGetValues( pane, args, n );

  paneW += 10;

  n = 0;
  paneH += botH + 25;
  if ( paneH + 25 > *largestH ) {
    paneH = *largestH - 25;
    paneW += 40;
  }
  XtSetArg( args[n], XmNheight, (XtArgVal) paneH ); n++;
  XtSetValues( shell, args, n );

  n = 0;
  XtSetArg( args[n], XmNwidth, (XtArgVal) paneW ); n++;
  XtSetValues( shell, args, n );

  XtAddEventHandler( shell, StructureNotifyMask, False,
   entryFormEventHandler, (XtPointer) this );

  XtPopup( shell, XtGrabNone );

  pTitle = title;
  XStringListToTextProperty( &pTitle, 1, &xtext );
  XSetWMName( display, XtWindow(shell), &xtext );
  XSetWMIconName( display, XtWindow(shell), &xtext );
  XFree( xtext.value );

  isPoppedUp = 1;

  return 1;

}

int entryFormClass::popdown ( void ) {

  XtPopdown( shell );

  isPoppedUp = 0;

  this->destroy();

  return 1;

}

int entryFormClass::popdownNoDestroy ( void ) {

  XtRemoveEventHandler( shell, StructureNotifyMask, False,
   entryFormEventHandler, (XtPointer) this );

  XtPopdown( shell );

  //  isPoppedUp = 0;

  return 1;

}

int entryFormClass::addTextFieldArray (
  char *label,
  int length,
  char **dest,
  int stringSize,
  entryListBase **obj )
{

textEntry *cur;
XmString str;

  cur = new textEntry;
  *obj = cur;

  // textField widget


  if ( firstArrayItem ) {

    firstArrayItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     arrayForm,
     XmNcolumns, (short) length,
     XmNvalue, dest[0],
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curArrayW = cur->activeW;
     curArrayRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     arrayForm,
     XmNcolumns, (short) length,
     XmNvalue, dest[0],
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curArrayW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curArrayRW,
     //XmNfontList, entryFontList,
     NULL );

     curArrayW = cur->activeW;
     curArrayRW = cur->activeW;

  }

  cur->arrayDsc.indexPtr = &index;
  cur->arrayDsc.destPtr = (void *) dest;
  cur->arrayDsc.size = stringSize;

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, TextFieldToStringArray,
   &(cur->arrayDsc) );

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   arrayForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curArrayW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curArrayW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );


  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addTextFieldArray (
  char *label,
  int length,
  int *dest,
  entryListBase **obj )
{

textEntry *cur;
XmString str;
char buf[127+1];

  sprintf( buf, "%-d", dest[0] );

//   fprintf( stderr, "entryFormClass::addTextFieldArray - new textEntry\n" );

  cur = new textEntry;
  *obj = cur;

  // textField widget

  if ( firstArrayItem ) {

    firstArrayItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     arrayForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curArrayW = cur->activeW;
     curArrayRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     arrayForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curArrayW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curArrayRW,
     //XmNfontList, entryFontList,
     NULL );

     curArrayW = cur->activeW;
     curArrayRW = cur->activeW;

  }

  cur->arrayDsc.indexPtr = &index;
  cur->arrayDsc.destPtr = (void *) dest;
  cur->arrayDsc.size = sizeof(int);

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, TextFieldToIntArray,
   cur );
   //&(cur->arrayDsc) );

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   arrayForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curArrayW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curArrayW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addTextFieldArray (
  char *label,
  int length,
  efInt *dest,
  entryListBase **obj )
{

textEntry *cur;
XmString str;
char buf[127+1];

  if ( dest[0].isNull() )
    strcpy( buf, "" );
  else
    sprintf( buf, "%-d", dest[0].value() );

//   fprintf( stderr, "entryFormClass::addTextFieldArray - new textEntry\n" );

  cur = new textEntry;
  *obj = cur;

  // textField widget

  if ( firstArrayItem ) {

    firstArrayItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     arrayForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curArrayW = cur->activeW;
     curArrayRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     arrayForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curArrayW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curArrayRW,
     //XmNfontList, entryFontList,
     NULL );

     curArrayW = cur->activeW;
     curArrayRW = cur->activeW;

  }

  cur->arrayDsc.indexPtr = &index;
  cur->arrayDsc.destPtr = (void *) dest;
  cur->arrayDsc.size = sizeof(int);

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, TextFieldToEfIntArray,
   cur );
   //&(cur->arrayDsc) );

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   arrayForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curArrayW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curArrayW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addTextFieldArray (
  char *label,
  int length,
  double *dest,
  entryListBase **obj )
{

textEntry *cur;
XmString str;
char buf[127+1];

  sprintf( buf, "%-g", dest[0] );

//   fprintf( stderr, "entryFormClass::addTextFieldArray - new textEntry\n" );

  cur = new textEntry;
  *obj = cur;

  // textField widget

  if ( firstArrayItem ) {

    firstArrayItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     arrayForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curArrayW = cur->activeW;
     curArrayRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     arrayForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curArrayW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curArrayRW,
     //XmNfontList, entryFontList,
     NULL );

     curArrayW = cur->activeW;
     curArrayRW = cur->activeW;

  }

  cur->arrayDsc.indexPtr = &index;
  cur->arrayDsc.destPtr = (void *) dest;
  cur->arrayDsc.size = sizeof(double);

  XtAddCallback( cur->activeW, XmNvalueChangedCallback, TextFieldToDoubleArray,
   cur );
   //&(cur->arrayDsc) );

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   arrayForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curArrayW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curArrayW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addTextFieldArray (
  char *label,
  int length,
  efDouble *dest,
  entryListBase **obj )
{

textEntry *cur;
XmString str;
char buf[127+1];

  if ( dest[0].isNull() )
    strcpy( buf, "" );
  else
    sprintf( buf, "%-g", dest[0].value() );

  cur = new textEntry;
  *obj = cur;

  // textField widget

  if ( firstArrayItem ) {

    firstArrayItem = 0;

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     arrayForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_FORM,
     XmNrightAttachment, XmATTACH_FORM,
     //XmNfontList, entryFontList,
     NULL );

     curArrayW = cur->activeW;
     curArrayRW = cur->activeW;

  }
  else {

    cur->activeW =  XtVaCreateManagedWidget( "text", xmTextFieldWidgetClass,
     arrayForm,
     XmNcolumns, (short) length,
     XmNvalue, buf,
     XmNmaxLength, length,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curArrayW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curArrayRW,
     //XmNfontList, entryFontList,
     NULL );

     curArrayW = cur->activeW;
     curArrayRW = cur->activeW;

  }

  cur->arrayDsc.indexPtr = &index;
  cur->arrayDsc.destPtr = (void *) dest;
  cur->arrayDsc.size = sizeof(int);

  XtAddCallback( cur->activeW, XmNvalueChangedCallback,
   TextFieldToEfDoubleArray,
   cur );
   //&(cur->arrayDsc) );

  if ( entryTag )
    str = XmStringCreate( label, entryTag );
  else
    str = XmStringCreateLocalized( label );

  cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
   arrayForm,
   XmNlabelString, str,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, curArrayW,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, curArrayW,
   //XmNfontList, entryFontList,
   NULL );

  XmStringFree( str );

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addSeparator ( void ) {

textEntry *cur;

  cur = new textEntry;
  cur->activeW = NULL;

  // separator widget

  if ( curTopParent  == topForm ) {

    if ( firstItem ) {

      firstItem = 0;

      cur->labelW = XtVaCreateManagedWidget( "sep", xmSeparatorWidgetClass,
       topForm,
       XmNmarginTop, 7,
       XmNtopAttachment, XmATTACH_FORM,
       XmNrightAttachment, XmATTACH_FORM,
       XmNleftAttachment, XmATTACH_FORM,
       NULL );

      curW = cur->labelW;
      curRW = cur->labelW;

    }
    else {

      cur->labelW = XtVaCreateManagedWidget( "sep", xmSeparatorWidgetClass,
       topForm,
       XmNmarginTop, 7,
       XmNtopAttachment, XmATTACH_WIDGET,
       XmNtopWidget, curW,
       XmNrightAttachment, XmATTACH_FORM,
       XmNleftAttachment, XmATTACH_FORM,
       NULL );

      curW = cur->labelW;

    }

  }
  else {

    if ( firstSubFormChild ) {

      firstSubFormChild = 0;

      cur->labelW = XtVaCreateManagedWidget( "sep", xmSeparatorWidgetClass,
       curTopParent,
       XmNmarginTop, 7,
       XmNmarginBottom, 7,
       XmNtopAttachment, XmATTACH_FORM,
       XmNbottomAttachment, XmATTACH_FORM,
       XmNleftAttachment, XmATTACH_FORM,
       XmNorientation, XmVERTICAL,
       NULL );

      prevW = cur->labelW;

    }
    else {

      cur->labelW = XtVaCreateManagedWidget( "sep", xmSeparatorWidgetClass,
       curTopParent,
       XmNmarginTop, 7,
       XmNmarginBottom, 7,
       XmNmarginLeft, 4,
       XmNtopAttachment, XmATTACH_FORM,
       XmNbottomAttachment, XmATTACH_FORM,
       XmNleftAttachment, XmATTACH_WIDGET,
       XmNleftWidget, prevW,
       XmNorientation, XmVERTICAL,
       NULL );

      prevW = cur->labelW;

    }

  }

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::addLabel (
  char *label )
{

textEntry *cur;
XmString str;

  cur = new textEntry;
  cur->activeW = NULL;

  // label widget

  if ( curTopParent  == topForm ) {

    if ( firstItem ) {

      firstItem = 0;

      if ( entryTag )
        str = XmStringCreate( label, entryTag );
      else
        str = XmStringCreateLocalized( label );

      cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
       topForm,
       XmNlabelString, str,
       XmNmarginTop, 7,
       XmNtopAttachment, XmATTACH_FORM,
       XmNleftAttachment, XmATTACH_FORM,
       //XmNfontList, entryFontList,
       NULL );

      XmStringFree( str );

      curW = cur->labelW;
      curRW = cur->labelW;

    }
    else {

      if ( entryTag )
        str = XmStringCreate( label, entryTag );
      else
        str = XmStringCreateLocalized( label );

      cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
       topForm,
       XmNlabelString, str,
       XmNmarginTop, 7,
       XmNtopAttachment, XmATTACH_WIDGET,
       XmNtopWidget, curW,
       XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
       XmNrightWidget, curW,
       //XmNfontList, entryFontList,
       NULL );

      XmStringFree( str );

      curW = cur->labelW;

    }

  }
  else {

    if ( firstSubFormChild ) {

      firstSubFormChild = 0;

      if ( entryTag )
        str = XmStringCreate( label, entryTag );
      else
        str = XmStringCreateLocalized( label );

      cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
       curTopParent,
       XmNlabelString, str,
       XmNmarginTop, 7,
       XmNtopAttachment, XmATTACH_FORM,
       XmNleftAttachment, XmATTACH_FORM,
       //XmNfontList, entryFontList,
       NULL );

      XmStringFree( str );

      prevW = cur->labelW;

    }
    else {

      if ( entryTag )
        str = XmStringCreate( label, entryTag );
      else
        str = XmStringCreateLocalized( label );

      cur->labelW = XtVaCreateManagedWidget( "label", xmLabelWidgetClass,
       curTopParent,
       XmNlabelString, str,
       XmNmarginTop, 7,
       XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
       XmNtopWidget, prevW,
       XmNleftAttachment, XmATTACH_WIDGET,
       XmNleftWidget, prevW,
       //XmNfontList, entryFontList,
       NULL );

      XmStringFree( str );

      prevW = cur->labelW;

    }

  }

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}

int entryFormClass::formIsPoppedUp ( void ) {

  return isPoppedUp;

}

int entryFormClass::beginSubForm ( void ) {

  subForm = new Widget;

  if ( firstItem ) {

    firstItem = 0;

    *subForm = XtVaCreateWidget( "subform", xmFormWidgetClass, topForm,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     NULL );
    curW = *subForm;
    curRW = *subForm;

  }
  else {

    *subForm = XtVaCreateWidget( "subform", xmFormWidgetClass, topForm,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
     XmNrightWidget, curRW,
     NULL );
    curW = *subForm;
    //curRW = *subForm;

  }

  XtAddEventHandler( *subForm,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask, False,
   efEventHandler, (XtPointer) this );

  curTopParent = *subForm;

  firstSubFormChild = 1;

  return 1;

}

int entryFormClass::beginLeftSubForm ( void ) {

  subForm = new Widget;

  if ( firstItem ) {

    firstItem = 0;

    *subForm = XtVaCreateWidget( "subform", xmFormWidgetClass, topForm,
     XmNtopAttachment, XmATTACH_FORM,
     XmNleftAttachment, XmATTACH_FORM,
     NULL );
    curW = *subForm;
    curRW = *subForm;

  }
  else {

    *subForm = XtVaCreateWidget( "subform", xmFormWidgetClass, topForm,
     XmNtopAttachment, XmATTACH_WIDGET,
     XmNtopWidget, curW,
     XmNleftAttachment, XmATTACH_FORM,
     NULL );
    curW = *subForm;
    //curRW = *subForm;

  }

  XtAddEventHandler( *subForm,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask, False,
   efEventHandler, (XtPointer) this );

  curTopParent = *subForm;

  firstSubFormChild = 1;

  return 1;

}

int entryFormClass::endSubForm ( void ) {

subFormWidget *cur;

  curTopParent = topForm;
  XtManageChild( *subForm );

  cur = new subFormWidget;
  cur->wPtr = subForm;
  cur->activeW = *subForm;
  cur->labelW = NULL;

  itemTail->flink = cur;
  itemTail = cur;
  itemTail->flink = NULL;

  return 1;

}
