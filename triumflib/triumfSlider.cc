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
//
//  TRIUMF additions by R.Keitel, Jan 2006
//  - slider must be selected (left-click on slider bar) before value can change
//  - if selected, slider bar changes to mslo->colorSelected
//  - if mouse leaves slider, color changes to mslo->colorOutofWindow
//  - a slider sensitivity selection widget is used to narrow slider range to sensitivity*(original range)
//    centred around current value. This allows higher precision sliding.
//  - if a slider with sensitivity < 100% is moved to left or right edge of widget and released,
//    the slider pans the range to center around current value
//  - if the PV of a slider with sensitivity < 100% is moved out of range by other events,
//    the slider bar changes to mslo->colorOutofRange. If the slider is reselected,
//    and is centred around the current value.
//  20060717 re-package as "Triumf Slider"
//    Note: I'll keep the ifdef TRIUMF in order to indicate the changes from the
//          Triumf Slider

#define __triumfSlider_cc 1

// uncomment this for getting TRIUMF functionality
// must be before including triumfSlider.h
#define TRIUMF 1
// #define DBG 1

#include "triumfSlider.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"


static int g_transInit = 1;
static XtTranslations g_parsedTrans;

static char g_dragTrans[] =
  "#override ~Ctrl~Shift<Btn2Down>: startDrag()\n\
   Ctrl~Shift<Btn2Down>: dummy()\n\
   Ctrl~Shift<Btn2Up>: selectActions()\n\
   Shift Ctrl<Btn2Down>: pvInfo()\n\
   Shift~Ctrl<Btn2Down>: dummy()\n\
   Shift~Ctrl<Btn2Up>: selectDrag()\n\
   <Btn3Up>: ChangeParams()";

static XtActionsRec g_dragActions[] = {
  { "startDrag", (XtActionProc) drag },
  { "pvInfo", (XtActionProc) pvInfo },
  { "dummy", (XtActionProc) dummy },
  { "selectActions", (XtActionProc) selectActions },
  { "ChangeParams", (XtActionProc) ChangeParams },
  { "selectDrag", (XtActionProc) selectDrag }
};

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;

  if ( !mslo->init ) {
    mslo->needToDrawUnconnected = 1;
    mslo->needDraw = 1;
    mslo->actWin->addDefExeNode( mslo->aglPtr );
  }

  mslo->unconnectedTimer = 0;

}

// original version of msloValueChangeCB
#ifndef TRIUMF
static void msloValueChangeCB (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int v, stat;
activeTriumfSliderClass *mslo;
double fvalue;

  // this only gets called when the left mouse button is clicked
  // on the scrollbar background or indicator

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  if ( !mslo->enabled || !mslo->active ) return;

  if ( mslo->increment == 0 ) return;

  if ( mslo->dragIndicator ) {
    mslo->dragIndicator = 0;
    return;
  }

  XmScaleGetValue( w, &v );

  if ( mslo->prevScaleV == -1 ) mslo->prevScaleV = v;

  if ( v > mslo->prevScaleV ) {

    if ( mslo->positive ) {
      fvalue = mslo->controlV + mslo->increment;
      if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
      if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
    }
    else {
      fvalue = mslo->controlV - mslo->increment;
      if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
      if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
    }

  }
  else {

    if ( mslo->positive ) {
      fvalue = mslo->controlV - mslo->increment;
      if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
      if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
    }
    else {
      fvalue = mslo->controlV + mslo->increment;
      if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
      if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
    }

  }

  mslo->prevScaleV = v;

  mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
   mslo->factor + 0.5 );

  XmScaleSetValue( w, mslo->controlX );

  mslo->oldControlV = mslo->oneControlV;

  mslo->eraseActiveControlText();

  mslo->actWin->appCtx->proc->lock();
  mslo->controlV = mslo->oneControlV = mslo->curControlV;
  mslo->actWin->appCtx->proc->unlock();

  mslo->controlV = fvalue;

  snprintf( mslo->controlValue, 14, mslo->controlFormat, mslo->controlV );

  stat = mslo->drawActiveControlText();

  if ( mslo->controlExists ) {
    if ( mslo->controlPvId ) {
      stat = mslo->controlPvId->put(
       XDisplayName(mslo->actWin->appCtx->displayName),
       fvalue );
      if ( !stat ) printf( activeTriumfSliderClass_str59 );
    }
  }

}
#endif


// TRIUMF version of msloValueChangeCB
#ifdef TRIUMF

static void msloValueChangeCB (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int v, stat = 0;
activeTriumfSliderClass *mslo;
XmScaleCallbackStruct *cbs;
XEvent *e;
double fvalue, df;
int at_limit = 0;

  // this only gets called when the left mouse button is clicked
  // on the scrollbar background or indicator
  // RK051213 not true: also called on left or right arrow
#ifdef DBG
  printf("triumfSlider: msloValueChangeCB\n");
#endif

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  if ( !mslo->enabled || !mslo->active ) return;

  if (!call) {
    return;
  }
  cbs = (XmScaleCallbackStruct *) call;
  e = cbs->event;
  if (!e) {
  	XmScaleGetValue( w, &v );
  	mslo->prevScaleV = v;
    return;
  }
  if ( e->type == ButtonRelease && activeTriumfSliderClass::selectedSlider != mslo) {
	return;
  }
  if ( (e->type == KeyPress || e->type == KeyRelease) && activeTriumfSliderClass::selectedSlider != mslo) {
	return;
  }
  // don't allow sliding if we have no write access
  if ( mslo->controlPvId ) {
    if ( !mslo->controlPvId->have_write_access() ) {
      return;
    }
  }
  ////
  mslo->changeSelectedSlider(mslo, 0); 
  if (mslo->outOfRange) {
    at_limit = 1;
  	updateSliderLimits(mslo);
	mslo->outOfRange = 0;
    mslo->eraseActive();
    snprintf( mslo->minValue, 14, "%-g", mslo->minFv );
    snprintf( mslo->maxValue, 14, "%-g", mslo->maxFv );
    mslo->drawActive();
  }
  if (mslo->newSelect) {
    mslo->newSelect = 0;
    XmScaleGetValue( w, &v );

    mslo->controlX = (int) ( ( mslo->controlV - mslo->minFv ) /
    mslo->factor + 0.5 );

    XmScaleSetValue( w, mslo->controlX );
    mslo->prevScaleV = mslo->controlX;
	return;
  }
  XmScaleGetValue( w, &v );

  if ( mslo->prevScaleV == -1) {
    mslo->prevScaleV = v;
  }
  // if we come here by dragging or selecting an out-of-range slider, we don't change the controlV
  if ( mslo->dragIndicator || mslo->newSelect) {
    mslo->dragIndicator = 0;
//  we cannot return here, as we potentially have to update the slider limits
//  return;
    fvalue = mslo->controlV;
  } else {
    // if we come here by increment/decrement an out-of-range slider, we do change the controlV

    if ( v > mslo->prevScaleV ) {

      if ( mslo->positive ) {
        fvalue = mslo->controlV + mslo->increment;
        if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
      }
      else {
        fvalue = mslo->controlV - mslo->increment;
        if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
      }

    }
    else if (v < mslo->prevScaleV) {

      if ( mslo->positive ) {
        fvalue = mslo->controlV - mslo->increment;
        if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
      }
      else {
        fvalue = mslo->controlV + mslo->increment;
        if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
      }
    }
	else {
		fvalue = mslo->controlV;
	}
  }
  df = fabs(mslo->maxFv - mslo->minFv) / 20.;
  if (fabs(fvalue - mslo->minFv) < df || fabs(fvalue - mslo->maxFv) < df) {
  	at_limit = 1;
    updateSliderLimits(mslo);
  }


  mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
     mslo->factor + 0.5 );

  XmScaleSetValue( w, mslo->controlX );
  mslo->prevScaleV = mslo->controlX;

  mslo->oldControlV = mslo->oneControlV;

  at_limit ? mslo->eraseActive() : mslo->eraseActiveControlText();

  mslo->actWin->appCtx->proc->lock();
  mslo->controlV = mslo->oneControlV = mslo->curControlV;
  mslo->actWin->appCtx->proc->unlock();

  mslo->controlV = fvalue;
  if ( mslo->ef.formIsPoppedUp() ) {
    mslo->valueEntry->setValue(fvalue);
  }

  snprintf( mslo->controlValue, 14, mslo->controlFormat, mslo->controlV );
  if (at_limit) {
    snprintf( mslo->minValue, 14, "%-g", mslo->minFv );
    snprintf( mslo->maxValue, 14, "%-g", mslo->maxFv );
  }
  stat =  (at_limit ? mslo->drawActive() : mslo->drawActiveControlText() );

  if ( mslo->controlExists ) {
    if ( mslo->controlPvId ) {
      stat = mslo->controlPvId->put(
       XDisplayName(mslo->actWin->appCtx->displayName),
       fvalue );
      if ( !stat ) printf( activeTriumfSliderClass_str59 );
    }
  }
}


#endif


static void msloIndicatorDragCB (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int v, stat;
activeTriumfSliderClass *mslo;
double fvalue;

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  if ( !mslo->enabled || !mslo->active ) return;

#ifdef TRIUMF
  if ( activeTriumfSliderClass::selectedSlider != mslo || mslo->outOfRange) {
  	XmScaleSetValue( w, mslo->controlX );
	return;
  }
#endif
  mslo->dragIndicator = 1;

  XmScaleGetValue( w, &v );

  fvalue = mslo->factor * (double) v + mslo->minFv;
  if ( mslo->positive ) {
    if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
    if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
  }
  else {
    if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
    if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
  }

  mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
    mslo->factor + 0.5 );

  XmScaleSetValue( w, mslo->controlX );
  mslo->prevScaleV = mslo->controlX;


  mslo->oldControlV = mslo->oneControlV;

  mslo->eraseActiveControlText();

  mslo->actWin->appCtx->proc->lock();
  mslo->controlV = mslo->oneControlV = mslo->curControlV;
  mslo->actWin->appCtx->proc->unlock();

  mslo->controlV = fvalue;

  snprintf( mslo->controlValue, 14, mslo->controlFormat, mslo->controlV );

  stat = mslo->drawActiveControlText();

  if ( mslo->controlExists ) {
    if ( mslo->controlPvId ) {
      stat = mslo->controlPvId->put(
       XDisplayName(mslo->actWin->appCtx->displayName),
       fvalue );
      if ( !stat ) printf( activeTriumfSliderClass_str59 );
    }
  }

}

static void calcIncRange (
  double minV,
  double maxV,
  char *strVal,
  double *incArray
) {

double max, lmin, lmax;
int i, j, start, end;
char tmpStr[255+1];

    if ( minV == 0 ) {
      lmin = 0;
    }
    else {
      lmin = rint( log10( fabs( minV ) ) );
    }

    if ( maxV == 0 ) {
      lmax = 0;
    }
    else {
      lmax = rint( log10( fabs( maxV ) ) );
    }

    max = 0;
    if ( lmin != 0 ) max = lmin;
    if ( lmax != 0 ) max = lmax;

    if ( lmin != 0 ) {
      if ( lmin > max ) max = lmin;
    }

    if ( lmax != 0 ) {
      if ( lmax > max ) max = lmax;
    }

    start = (int) max - 1;
    end = start - 5;
    strcpy( strVal, "---|" );
    incArray[0] = 0;
    for ( i=1,j=start; i<6; i++,j-- ) {
      sprintf( tmpStr, "10^%-d|", j );
      Strncat( strVal, tmpStr, 255 );
      incArray[i] = pow( 10, (double) j );
    }
    sprintf( tmpStr, "10^%-d", end );
    Strncat( strVal, tmpStr, 255 );
    incArray[6] = pow( 10, (double) j );

}

static void ChangeParams (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeTriumfSliderClass *mslo;
XButtonEvent *be;
char title[32], *ptr;
char strVal[255+1];

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

#ifdef TRIUMF  
  if ( activeTriumfSliderClass::selectedSlider != mslo) {
	return;
  }
#endif
#ifdef DBG
  printf("triumfSlider: ChangeParams\n");
#endif
  if ( !mslo->ef.formIsPoppedUp() ) {

    be = (XButtonEvent *) e;

#ifndef TRIUMF
    ptr = mslo->actWin->obj.getNameFromClass( "activeTriumfSliderClass" );
    if ( ptr )
      strncpy( title, ptr, 31 );
    else
      strncpy( title, activeTriumfSliderClass_str54, 31 );

    Strncat( title, activeTriumfSliderClass_str55, 31 );
#else
#ifdef DBG
    printf("triumfSlider: form not popped up\n");
#endif
    ptr = mslo->actWin->obj.getNameFromClass( "activeTriumfSliderClass" );
    if ( ptr )
      strncpy( title, ptr, 31 );
    else
      strncpy( title, activeTriumfSliderClass_str54, 31 );
    Strncat( title, ": ", 31 );
    Strncat( title, mslo->controlPvName.getExpanded(), 31 );
#endif
    mslo->bufIncrement = mslo->increment;
#ifdef TRIUMF
    // move the current value to the buffer, as slider might be out of range
    mslo->bufControlV = mslo->curControlV;
#else
    mslo->bufControlV = mslo->controlV;
#endif
    mslo->valueFormX = be->x_root;
    mslo->valueFormY = be->y_root;
    mslo->valueFormW = 0;
    mslo->valueFormH = 0;
    mslo->valueFormMaxH = 600;

    mslo->ef.create( mslo->actWin->top,
      mslo->actWin->appCtx->ci.getColorMap(),
      &mslo->valueFormX, &mslo->valueFormY,
      &mslo->valueFormW, &mslo->valueFormH, &mslo->valueFormMaxH,
      title, NULL, NULL, NULL );

#ifndef TRIUMF
    mslo->ef.addTextField( activeTriumfSliderClass_str57, 20,
     &mslo->bufControlV );
    mslo->ef.addTextField( activeTriumfSliderClass_str58, 20,
     &mslo->bufIncrement );
    calcIncRange( mslo->minFv, mslo->maxFv, strVal, mslo->incArray );
#else
    mslo->valueEntry = mslo->ef.addTextFieldAccessible( activeTriumfSliderClass_str57, 20,
     &mslo->bufControlV );
    mslo->incEntry = mslo->ef.addTextFieldAccessible( activeTriumfSliderClass_str58, 20,
     &mslo->bufIncrement );
    calcIncRange( mslo->scaleMin, mslo->scaleMax, strVal, mslo->incArray );
#endif

    mslo->incIndex = 0;
    mslo->ef.addOption( activeTriumfSliderClass_str58, strVal,
     &mslo->incIndex );

#ifdef TRIUMF
    mslo->ef.addOption( activeTriumfSliderClass_str93, activeTriumfSliderClass::rangeString,
     &mslo->rangeIndex );
#endif
    mslo->ef.finished( mslc_value_ok, mslc_value_apply, mslc_value_cancel,
     mslo );

    mslo->ef.popup();

  }

}

static void dummy (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

}

static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeTriumfSliderClass *mslo;
int stat;

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  if ( !mslo->enabled ) return;

  stat = mslo->startDrag( w, e );

}

static void selectDrag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeTriumfSliderClass *mslo;
int stat;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  if ( !mslo->enabled ) return;

  stat = mslo->selectDragValue( be );

}

static void selectActions (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeTriumfSliderClass *mslo;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  if ( !mslo->enabled ) return;

  mslo->doActions( be, be->x, be->y );

}

static void pvInfo (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

activeTriumfSliderClass *mslo;
XButtonEvent *be = (XButtonEvent *) e;

  XtVaGetValues( w, XmNuserData, &mslo, NULL );

  mslo->showPvInfo( be, be->x, be->y );

}

static void mslc_updateControl (
  XtPointer client,
  XtIntervalId *id )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;
int stat;
double fv;

  mslo->updateControlTimerActive = 0;
  mslo->updateControlTimer = 0;

  if ( !mslo->active || !mslo->init ) {
    mslo->updateControlTimerActive = 1;
    mslo->updateControlTimer = appAddTimeOut(
     mslo->actWin->appCtx->appContext(),
     500, mslc_updateControl, client );
    return;
  }

  mslo->oldControlV = mslo->oneControlV;

  mslo->eraseActiveControlText();

  mslo->actWin->appCtx->proc->lock();
  mslo->controlV = mslo->oneControlV = mslo->curControlV;
  mslo->actWin->appCtx->proc->unlock();

  if ( mslo->positive ) {

    if ( mslo->controlV < mslo->minFv )
      fv = mslo->minFv;
    else if ( mslo->controlV > mslo->maxFv )
      fv = mslo->maxFv;
    else
      fv = mslo->controlV;

  }
  else {

    if ( mslo->controlV > mslo->minFv )
      fv = mslo->minFv;
    else if ( mslo->controlV < mslo->maxFv )
      fv = mslo->maxFv;
    else
      fv = mslo->controlV;

  }

  mslo->controlX = (int) ( ( fv - mslo->minFv ) /
   mslo->factor + 0.5 );

  snprintf( mslo->controlValue, 14, mslo->controlFormat, mslo->controlV );

  stat = mslo->drawActiveControlText();

  XmScaleSetValue( mslo->scaleWidget, mslo->controlX );
  mslo->prevScaleV = mslo->controlX;

}

// Original verions of mslc_value_apply
#ifndef TRIUMF

static void mslc_value_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int stat;
double fvalue;
activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;

  fvalue = (double) mslo->bufControlV;

  if ( mslo->positive ) {
    if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
    if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
  }
  else {
    if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
    if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
  }

  mslo->controlV = fvalue;

  mslo->increment = mslo->bufIncrement;

  if ( ( mslo->incIndex > 0 ) && ( mslo->incIndex < 7 ) ) {
    mslo->increment = mslo->incArray[mslo->incIndex];
  }

  snprintf( mslo->incString, 31, mslo->controlFormat, mslo->increment );

  mslo->actWin->appCtx->proc->lock();
  mslo->curControlV = mslo->controlV;
  mslo->actWin->appCtx->proc->unlock();

// for EPICS support

  if ( mslo->controlExists ) {
    if ( mslo->controlPvId ) {
      stat = mslo->controlPvId->put(
       XDisplayName(mslo->actWin->appCtx->displayName),
       fvalue );
      if ( !stat ) printf( activeTriumfSliderClass_str3 );
      mslo->actWin->appCtx->proc->lock();
      mslo->actWin->addDefExeNode( mslo->aglPtr );
      mslo->actWin->appCtx->proc->unlock();
    }
  }

  mslo->needErase = 1;
  mslo->needDraw = 1;

}
#endif

// TRIUMF version of mslc_value_apply
#ifdef TRIUMF

static void mslc_value_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int stat;
double fvalue;
activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;

  mslo->changeSelectedSlider(mslo, 1);
  
#ifdef DBG
  printf("triumfSlider: mslc_value_apply - bufControlV %g controlV %g curControlV %g  minFv %g maxFv %g minFvOrg %g maxFvOrg %g\n",
        mslo->bufControlV, mslo->controlV, mslo->curControlV, mslo->minFv, mslo->maxFv, mslo->minFvOrg, mslo->maxFvOrg);
#endif
  fvalue = (double) mslo->bufControlV;

  if ( mslo->positive ) {
    if ( fvalue < mslo->minFvOrg ) fvalue = mslo->minFvOrg;
    if ( fvalue > mslo->maxFvOrg ) fvalue = mslo->maxFvOrg;
  }
  else {
    if ( fvalue > mslo->minFvOrg ) fvalue = mslo->minFvOrg;
    if ( fvalue < mslo->maxFvOrg ) fvalue = mslo->maxFvOrg;
  }

  mslo->controlV = fvalue;

  mslo->increment = mslo->bufIncrement;

  if ( ( mslo->incIndex > 0 ) && ( mslo->incIndex < 7 ) ) {
    mslo->increment = mslo->incArray[mslo->incIndex];
	// stuff selected value into text entry field
	mslo->bufIncrement = mslo->increment;
    mslo->incEntry->setValue(mslo->bufIncrement);
  }
  if ( ( mslo->rangeIndex >= 0 ) && ( mslo->rangeIndex < 6 ) ) {
    mslo->range = mslo->rangeArray[mslo->rangeIndex];
  }

  snprintf( mslo->incString, 31, mslo->controlFormat, mslo->increment );

  mslo->actWin->appCtx->proc->lock();
  mslo->curControlV = mslo->controlV;
  mslo->actWin->appCtx->proc->unlock();

  updateSliderLimits(mslo);

  mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
   mslo->factor + 0.5 );
  XmScaleSetValue( mslo->getScaleWidget(), mslo->controlX );
  mslo->prevScaleV = mslo->controlX;

// for EPICS support

  if ( mslo->controlExists ) {
    if ( mslo->controlPvId ) {
      stat = mslo->controlPvId->put(
       XDisplayName(mslo->actWin->appCtx->displayName),
       fvalue );
      if ( !stat ) printf( activeTriumfSliderClass_str3 );
      mslo->actWin->appCtx->proc->lock();
      mslo->actWin->addDefExeNode( mslo->aglPtr );
      mslo->actWin->appCtx->proc->unlock();
    }
  }
  mslo->needErase = 1;
  mslo->needDraw = 1;
  mslo->needCtlInfoInit = 1;
  mslo->needCtlInfoInitNoSave = 1;
}
#endif

static void mslc_value_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;

  mslc_value_apply ( w, client, call );
  mslo->ef.popdown();

}

static void mslc_value_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;

  mslo->ef.popdown();

}

static void mslc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;

  mslo->actWin->setChanged();

  mslo->eraseSelectBoxCorners();
  mslo->erase();

  
#ifdef DBG
  printf("triumfSlider: mslc_value_apply - bufControlV %g controlV %g curControlV %g  minFv %g maxFv %g minFvOrg %g maxFvOrg %g\n",
        mslo->bufControlV, mslo->controlV, mslo->curControlV, mslo->minFv, mslo->maxFv, mslo->minFvOrg, mslo->maxFvOrg);
#endif

  mslo->fgColor.setColorIndex( mslo->bufFgColor, mslo->actWin->ci );
  mslo->bgColor.setColorIndex( mslo->bufBgColor, mslo->actWin->ci );

  mslo->bgColorMode = mslo->bufBgColorMode;
  if ( mslo->bgColorMode == MSLC_K_COLORMODE_ALARM ) {
    mslo->bgColor.setAlarmSensitive();
  }
  else {
    mslo->bgColor.setAlarmInsensitive();
  }

  mslo->shadeColor = mslo->bufShadeColor;
  mslo->topColor = mslo->bufTopColor;
  mslo->botColor = mslo->bufBotColor;

  mslo->increment = mslo->bufIncrement;
  snprintf( mslo->incString, 31, mslo->controlFormat, mslo->increment );

  mslo->controlPvName.setRaw( mslo->eBuf->controlBufPvName );

  mslo->controlLabelName.setRaw( mslo->eBuf->controlBufLabelName );

  mslo->savedValuePvName.setRaw( mslo->eBuf->savedValueBufPvName );

  mslo->controlLabelType = mslo->bufControlLabelType;

  mslo->formatType = mslo->bufFormatType;

  mslo->limitsFromDb = mslo->bufLimitsFromDb;
  mslo->efPrecision = mslo->bufEfPrecision;
  mslo->efScaleMin = mslo->bufEfScaleMin;
  mslo->efScaleMax = mslo->bufEfScaleMax;

  mslo->minFv = mslo->scaleMin = mslo->efScaleMin.value();
  mslo->maxFv = mslo->scaleMax = mslo->efScaleMax.value();

  if ( mslo->efPrecision.isNull() )
    mslo->precision = 1;
  else
    mslo->precision = mslo->efPrecision.value();

  strncpy( mslo->fontTag, mslo->fm.currentFontTag(), 63 );
  mslo->actWin->fi->loadFontTag( mslo->fontTag );
  mslo->fs = mslo->actWin->fi->getXFontStruct( mslo->fontTag );

  mslo->showLimits = mslo->bufShowLimits;
  mslo->showLabel = mslo->bufShowLabel;
  mslo->showValue = mslo->bufShowValue;
  mslo->showSavedValue = mslo->bufShowSavedValue;

  mslo->orientation = mslo->bufOrientation;

  mslo->x = mslo->bufX;
  mslo->sboxX = mslo->bufX;

  mslo->y = mslo->bufY;
  mslo->sboxY = mslo->bufY;

  if ( mslo->bufW < mslo->minW ) mslo->bufW = mslo->minW;

  mslo->w = mslo->bufW;
  mslo->sboxW = mslo->bufW;

  if ( mslo->bufH < mslo->minH ) mslo->bufH = mslo->minH;

  mslo->h = mslo->bufH;
  mslo->sboxH = mslo->bufH;

  mslo->updateDimensions();

  // one more iteration
  if ( mslo->h < mslo->minH ) mslo->h = mslo->minH;
  mslo->sboxH = mslo->h;

  mslo->erase();
  mslo->draw();

}

static void mslc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;

  mslc_edit_update ( w, client, call );
  mslo->refresh( mslo );

}

static void mslc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;

  mslc_edit_update ( w, client, call );
  mslo->ef.popdown();
  mslo->operationComplete();

}

static void mslc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;

  mslo->ef.popdown();
  mslo->operationCancel();

}

static void mslc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) client;

  mslo->ef.popdown();
  mslo->operationCancel();
  mslo->erase();
  mslo->deleteRequest = 1;
  mslo->drawAll();

}


#ifdef TRIUMF

static void updateSliderLimits( 
  activeTriumfSliderClass *mslo )
{
  double newrange;

  // go back to original slider if range > 1
  if (mslo->range >= 1) {
    mslo->minFv = mslo->minFvOrg;
    mslo->maxFv = mslo->maxFvOrg;
  } else {
    // center around current value based on range
 
    newrange = (mslo->minFvOrg - mslo->maxFvOrg) * mslo->range;
    if (mslo->positive) {
      newrange = -newrange;
    }
    mslo->minFv = mslo->curControlV - newrange / 2.;
	if (mslo->minFv < mslo->minFvOrg) mslo->minFv = mslo->minFvOrg;
    mslo->maxFv = mslo->curControlV + newrange / 2.;
	if (mslo->maxFv > mslo->maxFvOrg) mslo->maxFv = mslo->maxFvOrg;
  }
  mslo->factor = ( mslo->maxFv - mslo->minFv ) / 100000;
  if ( mslo->factor == 0.0 ) mslo->factor = 1.0;
}

#endif

// for EPICS support

void activeTriumfSliderClass::monitorControlConnectState (
  ProcessVariable *pv,
  void *userarg )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) userarg;

  if ( pv->is_valid() ) {

    if ( mslo->limitsFromDb || mslo->efScaleMin.isNull() ) {
      mslo->scaleMin = pv->get_lower_disp_limit();
    }

    if ( mslo->limitsFromDb || mslo->efScaleMax.isNull() ) {
      mslo->scaleMax = pv->get_upper_disp_limit();
    }

    if ( mslo->limitsFromDb || mslo->efPrecision.isNull() ) {
      mslo->precision = pv->get_precision();
    }

    if ( mslo->formatType == MSLC_K_FORMAT_FLOAT ) {
      sprintf( mslo->controlFormat, "%%.%-df", mslo->precision );
    }
    else if ( mslo->formatType == MSLC_K_FORMAT_EXPONENTIAL ) {
      sprintf( mslo->controlFormat, "%%.%-de", mslo->precision );
    }
    else {
      sprintf( mslo->controlFormat, "%%.%-dg", mslo->precision );
    }
    // for changing the slider sensitivity, save the original limits in minFvOrg, maxFvOrg
    mslo->minFv = mslo->minFvOrg = mslo->scaleMin;
    mslo->maxFv = mslo->maxFvOrg = mslo->scaleMax;
    mslo->curControlV = pv->get_double();
	
	mslo->colorSelected = mslo->actWin->ci->colorIndexByAlias("triumf-slider-selected");
	mslo->colorOutofRange = mslo->actWin->ci->colorIndexByAlias("triumf-slider-outofrange");
	mslo->colorOutofWindow = mslo->actWin->ci->colorIndexByAlias("triumf-slider-outofwindow");
	if (mslo->colorSelected == 0 || mslo->colorOutofRange == 0 || mslo->colorOutofWindow == 0) {
      fprintf( stderr, activeTriumfSliderClass_str94 );
	}
    mslo->needCtlConnectInit = 1;
    mslo->needCtlInfoInit = 1;
#ifdef TRIUMF
  	mslo->needCtlInfoInitNoSave = 0;
#endif
  }
  else {

    mslo->controlPvConnected = 0;
    mslo->active = 0;
    mslo->fgColor.setDisconnected();
    mslo->bgColor.setDisconnected();
    mslo->bufInvalidate();
    mslo->needErase = 1;
    mslo->needDraw = 1;

  }

  mslo->actWin->appCtx->proc->lock();
  mslo->actWin->addDefExeNode( mslo->aglPtr );
  mslo->actWin->appCtx->proc->unlock();

}

void activeTriumfSliderClass::monitorControlLabelConnectState (
  ProcessVariable *pv,
  void *userarg )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) userarg;

  if ( pv->is_valid() ) {

    mslo->needCtlLabelConnectInit = 1;
    mslo->actWin->appCtx->proc->lock();
    mslo->actWin->addDefExeNode( mslo->aglPtr );
    mslo->actWin->appCtx->proc->unlock();

  }

}

void activeTriumfSliderClass::monitorSavedValueConnectState (
  ProcessVariable *pv,
  void *userarg )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) userarg;

  if ( pv->is_valid() ) {

    mslo->needSavedConnectInit = 1;

  }
  else {

    mslo->savedValuePvConnected = 0;

  }

  mslo->actWin->appCtx->proc->lock();
  mslo->actWin->addDefExeNode( mslo->aglPtr );
  mslo->actWin->appCtx->proc->unlock();

}

void activeTriumfSliderClass::controlLabelUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) userarg;

  pv->get_string( mslo->controlLabel, PV_Factory::MAX_PV_NAME );

  mslo->needCtlLabelInfoInit = 1;
  mslo->actWin->appCtx->proc->lock();
  mslo->actWin->addDefExeNode( mslo->aglPtr );
  mslo->actWin->appCtx->proc->unlock();

}

void activeTriumfSliderClass::controlUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) userarg;
int st, sev;

  mslo->curControlV = mslo->oneControlV = pv->get_double();

  st = pv->get_status();
  sev = pv->get_severity();

  if ( ( st != mslo->oldStat ) || ( sev != mslo->oldSev ) ) {
    mslo->oldStat = st;
    mslo->oldSev = sev;
    mslo->bgColor.setStatus( st, sev );
    mslo->bufInvalidate();
    mslo->needErase = 1;
    mslo->needDraw = 1;
    mslo->actWin->appCtx->proc->lock();
    mslo->actWin->addDefExeNode( mslo->aglPtr );
    mslo->actWin->appCtx->proc->unlock();
  }
#ifdef TRIUMF
  if (mslo->curControlV < mslo->minFv || mslo->curControlV > mslo->maxFv) {
    if (mslo->scrollBarWidget) {
	  if (mslo->keyRestore) {
	  	mslo->keyRestore = 0;
	  }
 	  mslo->outOfRange = 1;
	  // if a slider is moved out of range by an external event, we require a new selection
	  mslo->newSelect = 1;
	  if (mslo->shadeColor != mslo->colorOutofRange) {
	    mslo->shadeColorInRange = mslo->shadeColor;
      }
      mslo->shadeColor = mslo->colorOutofRange;
      XtVaSetValues( mslo->scrollBarWidget,
          XmNtroughColor, mslo->actWin->ci->pix(mslo->colorOutofRange),
          NULL );
    }
  } else {
    if (mslo->scrollBarWidget) {
      mslo->shadeColor = mslo->shadeColorInRange;
      XtVaSetValues( mslo->scrollBarWidget,
        XmNtroughColor, mslo->actWin->ci->pix(mslo->shadeColor),
        NULL );
    }
	mslo->outOfRange = 0;
  }
  if ( mslo->ef.formIsPoppedUp() ) {
    mslo->valueEntry->setValue(mslo->curControlV);
  }
#endif
  if ( !mslo->updateControlTimerActive ) {
    mslo->updateControlTimerActive = 1;
    mslo->updateControlTimerValue = 100;
    mslo->updateControlTimer = appAddTimeOut(
     mslo->actWin->appCtx->appContext(), mslo->updateControlTimerValue,
     mslc_updateControl, (void *) mslo );
  }

  // xtimer updates image

}

void activeTriumfSliderClass::savedValueUpdate (
  ProcessVariable *pv,
  void *userarg )
{

activeTriumfSliderClass *mslo = (activeTriumfSliderClass *) userarg;

  mslo->newSavedV = pv->get_double();

  mslo->needSavedRefresh = 1;
  mslo->actWin->appCtx->proc->lock();
  mslo->actWin->addDefExeNode( mslo->aglPtr );
  mslo->actWin->appCtx->proc->unlock();

}

activeTriumfSliderClass::activeTriumfSliderClass ( void ) {

  name = new char[strlen("activeTriumfSliderClass")+1];
  strcpy( name, "activeTriumfSliderClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  deleteRequest = 0;
  selected = 0;
  positive = 1;
  outOfRange = 0;
  rangeIndex = 0;
  range = 1.;
  strcpy( id, "" );

  scaleMin = 0;
  scaleMax = 10;
#ifdef TRIUMF
  // storage for original values
  minFvOrg = 0;
  maxFvOrg = 0;
  keyRestore = 0;
#endif
  limitsFromDb = 1;
  efScaleMin.setNull(1);
  efScaleMax.setNull(1);
  efPrecision.setNull(1);
  formatType = MSLC_K_FORMAT_FLOAT;
  precision = 1;

  controlLabelType = MSLC_K_PV_NAME;

  showLimits = 0;
  showLabel = 0;
  showValue = 0;
  showSavedValue = 0;

  orientation = MSLC_K_HORIZONTAL;

  limitsH = 0;
  labelH = 0;
  midVertScaleY = 0;
  midVertScaleY1 = 0;
  midVertScaleY2 = 0;

  keySensitive = 0;

  frameWidget = NULL;
  scaleWidget = NULL;
  scrollBarWidget = NULL;

  unconnectedTimer = 0;

  eBuf = NULL;

}

// copy constructor
activeTriumfSliderClass::activeTriumfSliderClass
 ( const activeTriumfSliderClass *source ) {

activeGraphicClass *mslo = (activeGraphicClass *) this;

  mslo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeTriumfSliderClass")+1];
  strcpy( name, "activeTriumfSliderClass" );

  deleteRequest = 0;

  bgColor.copy( source->bgColor );
  fgColor.copy( source->fgColor );

  bgColorMode = source->bgColorMode;

  shadeColor = source->shadeColor;
  topColor = source->topColor;
  botColor = source->botColor;

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  shadeCb = source->shadeCb;
  topCb = source->topCb;
  botCb = source->botCb;

  controlPvName.copy( source->controlPvName );
  controlLabelName.copy( source->controlLabelName );
  savedValuePvName.copy( source->savedValuePvName );

  strncpy( fontTag, source->fontTag, 63 );

  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  strcpy( controlValue, "0.0" );
  strcpy( savedValue, "0.0" );
  strcpy( controlLabel, "" );

  controlLabelType = source->controlLabelType;

  increment = source->increment;

  positive = source->positive;

  limitsFromDb = source->limitsFromDb;
  scaleMin = source->scaleMin;
  scaleMax = source->scaleMax;
#ifdef TRIUMF
  minFvOrg = source->minFvOrg;
  maxFvOrg = source->maxFvOrg;
#endif
  precision = source->precision;

  efScaleMin = source->efScaleMin;
  efScaleMax = source->efScaleMax;
  efPrecision = source->efPrecision;

  formatType = source->formatType ;

  showLimits = source->showLimits;
  showLabel = source->showLabel;
  showValue = source->showValue;
  showSavedValue = source->showSavedValue;

  orientation = source->orientation;

  keySensitive = source->keySensitive;

  savedV = source->savedV;

  frameWidget = NULL;
  scaleWidget = NULL;
  scrollBarWidget = NULL;

  unconnectedTimer = 0;

  eBuf = NULL;

  doAccSubs( controlPvName );
  doAccSubs( savedValuePvName );
  doAccSubs( controlLabelName );

}

activeTriumfSliderClass::~activeTriumfSliderClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

int activeTriumfSliderClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = aw_obj;
  xOrigin = 0;
  yOrigin = 0;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  increment = 0.0;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  bgColorMode = 0;

  shadeColor =actWin->defaultOffsetColor;
  topColor = actWin->defaultTopShadowColor;
  botColor = actWin->defaultBotShadowColor;

  strcpy( controlValue, "0.0" );
  strcpy( controlLabel, "" );

  controlLabelType = MSLC_K_PV_NAME;

  strcpy( fontTag, actWin->defaultCtlFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();
  if ( h < minH ) h = minH;
  if ( w < minW ) w = minW;

  this->draw();

  this->editCreate();

  return 1;

}

int activeTriumfSliderClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

int labelTypeLiteral = 0;
static char *labelEnumStr[3] = {
  "literal",
  "pvLabel",
  "pvName"
};
static int labelEnum[3] = {
  0,
  1,
  2
};

int formatTypeFfloat = 0;
static char *formatTypeEnumStr[3] = {
  "ffloat",
  "exponential",
  "gfloat"
};
static int formatTypeEnum[3] = {
  0,
  1,
  2
};

int horz = 0;
static char *orienTypeEnumStr[2] = {
  "horizontal",
  "vertical"
};
static int orienTypeEnum[2] = {
  0,
  1
};

  major = MSLC_MAJOR_VERSION;
  minor = MSLC_MINOR_VERSION;
  release = MSLC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "fgColor", actWin->ci, &fgColor );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadW( "bgAlarm", &bgColorMode, &zero );
  tag.loadW( "2ndBgColor", actWin->ci, &shadeColor );
  tag.loadW( "topShadowColor", actWin->ci, &topColor );
  tag.loadW( "botShadowColor", actWin->ci, &botColor );
  tag.loadW( "increment", &increment, &dzero );
  tag.loadW( "controlPv", &controlPvName, emptyStr );
  tag.loadW( "controlLabel", &controlLabelName, emptyStr );
  tag.loadW( "controlLabelType", 3, labelEnumStr, labelEnum, &controlLabelType,
   &labelTypeLiteral );
  tag.loadW( "font", fontTag );
  tag.loadW( "displayFormat", 3, formatTypeEnumStr, formatTypeEnum,
   &formatType, &formatTypeFfloat );
  tag.loadBoolW( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadW( "precision", &efPrecision );
  tag.loadW( "scaleMin", &efScaleMin );
  tag.loadW( "scaleMax", &efScaleMax );
  tag.loadBoolW( "showLimits", &showLimits, &zero );
  tag.loadBoolW( "showLabel", &showLabel, &zero );
  tag.loadBoolW( "showValue", &showValue, &zero );
  tag.loadW( "orientation", 2, orienTypeEnumStr, orienTypeEnum,
   &orientation, &horz );
  tag.loadW( "savedValuePv", &savedValuePvName, emptyStr );
  tag.loadBoolW( "showSavedValue", &showSavedValue, &zero );
  tag.loadR( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int activeTriumfSliderClass::old_save (
  FILE *f )
{

int index, stat;

  fprintf( f, "%-d %-d %-d\n", MSLC_MAJOR_VERSION, MSLC_MINOR_VERSION,
   MSLC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  actWin->ci->writeColorIndex( f, shadeColor );
  //fprintf( f, "%-d\n", shadeColor );

  actWin->ci->writeColorIndex( f, topColor );
  //fprintf( f, "%-d\n", topColor );

  actWin->ci->writeColorIndex( f, botColor );
  //fprintf( f, "%-d\n", botColor );

  fprintf( f, "%-g\n", increment );

  if ( controlPvName.getRaw() )
    writeStringToFile( f, controlPvName.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( controlLabelName.getRaw() )
    writeStringToFile( f, controlLabelName.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", controlLabelType );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", bgColorMode );

  fprintf( f, "%-d\n", limitsFromDb );
  stat = efPrecision.write( f );
  stat = efScaleMin.write( f );
  stat = efScaleMax.write( f );

  fprintf( f, "%-d\n", formatType );

  fprintf( f, "%-d\n", showLimits );
  fprintf( f, "%-d\n", showLabel );
  fprintf( f, "%-d\n", showValue );

  fprintf( f, "%-d\n", orientation );

  return 1;

}

int activeTriumfSliderClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

int labelTypeLiteral = 0;
static char *labelEnumStr[3] = {
  "literal",
  "pvLabel",
  "pvName"
};
static int labelEnum[3] = {
  0,
  1,
  2
};

int formatTypeFfloat = 0;
static char *formatTypeEnumStr[3] = {
  "ffloat",
  "exponential",
  "gfloat"
};
static int formatTypeEnum[3] = {
  0,
  1,
  2
};

int horz = 0;
static char *orienTypeEnumStr[2] = {
  "horizontal",
  "vertical"
};
static int orienTypeEnum[2] = {
  0,
  1
};

  actWin = _actWin;

  tag.init();
  tag.loadR( "beginObjectProperties" );
  tag.loadR( unknownTags );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );
  tag.loadR( "x", &x );
  tag.loadR( "y", &y );
  tag.loadR( "w", &w );
  tag.loadR( "h", &h );
  tag.loadR( "fgColor", actWin->ci, &fgColor );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "bgAlarm", &bgColorMode, &zero );
  tag.loadR( "2ndBgColor", actWin->ci, &shadeColor );
  tag.loadR( "topShadowColor", actWin->ci, &topColor );
  tag.loadR( "botShadowColor", actWin->ci, &botColor );
  tag.loadR( "increment", &increment, &dzero );
  tag.loadR( "controlPv", &controlPvName, emptyStr );
  tag.loadR( "controlLabel", &controlLabelName, emptyStr );
  tag.loadR( "controlLabelType", 3, labelEnumStr, labelEnum,
   &controlLabelType, &labelTypeLiteral );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "displayFormat", 3, formatTypeEnumStr, formatTypeEnum,
   &formatType, &formatTypeFfloat );
  tag.loadR( "limitsFromDb", &limitsFromDb, &zero );
  tag.loadR( "precision", &efPrecision );
  tag.loadR( "scaleMin", &efScaleMin );
  tag.loadR( "scaleMax", &efScaleMax );
  tag.loadR( "showLimits", &showLimits, &zero );
  tag.loadR( "showLabel", &showLabel, &zero );
  tag.loadR( "showValue", &showValue, &zero );
  tag.loadR( "orientation", 2, orienTypeEnumStr, orienTypeEnum, &orientation,
   &horz );
  tag.loadR( "savedValuePv", &savedValuePvName, emptyStr );
  tag.loadR( "showSavedValue", &showSavedValue, &zero );
  tag.loadW( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > MSLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( bgColorMode == MSLC_K_COLORMODE_ALARM ) {
    bgColor.setAlarmSensitive();
  }
  else {
    bgColor.setAlarmInsensitive();
  }

  if ( limitsFromDb || efPrecision.isNull() )
    precision = 1;
  else
    precision = efPrecision.value();

  if ( ( limitsFromDb || efScaleMin.isNull() ) &&
       ( limitsFromDb || efScaleMax.isNull() ) ) {
    minFv = scaleMin = 0;
    maxFv = scaleMax = 10;
  }
  else{
    minFv = scaleMin = efScaleMin.value();
    maxFv = scaleMax = efScaleMax.value();
  }

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  controlX = 0;

  strcpy( controlValue, "0.0" );
  strcpy( controlLabel, "" );

  curControlV = oneControlV = controlV = 0.0;

  return stat;

}

int activeTriumfSliderClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int stat, index;
int major, minor, release;
char oneName[PV_Factory::MAX_PV_NAME+1];
float val;

  actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > MSLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 0 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    shadeColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    topColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    botColor = index;

  }
  else {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &shadeColor ); actWin->incLine();

    fscanf( f, "%d\n", &topColor ); actWin->incLine();

    fscanf( f, "%d\n", &botColor ); actWin->incLine();

  }

  fscanf( f, "%g\n", &val ); actWin->incLine();
  increment = (double) val;

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlPvName.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  controlLabelName.setRaw( oneName );

  fscanf( f, "%d\n", &controlLabelType ); actWin->incLine();

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  fscanf( f, "%d\n", &bgColorMode ); actWin->incLine();

  if ( bgColorMode == MSLC_K_COLORMODE_ALARM ) {
    bgColor.setAlarmSensitive();
  }
  else {
    bgColor.setAlarmInsensitive();
  }

  fscanf( f, "%d\n", &limitsFromDb ); actWin->incLine();

  stat = efPrecision.read( f ); actWin->incLine();

  efScaleMin.read( f ); actWin->incLine();

  efScaleMax.read( f ); actWin->incLine();

  fscanf( f, "%d\n", &formatType ); actWin->incLine();

  fscanf( f, "%d\n", &showLimits ); actWin->incLine();
  fscanf( f, "%d\n", &showLabel ); actWin->incLine();
  fscanf( f, "%d\n", &showValue ); actWin->incLine();

  fscanf( f, "%d\n", &orientation ); actWin->incLine();

  if ( limitsFromDb || efPrecision.isNull() )
    precision = 1;
  else
    precision = efPrecision.value();

  if ( ( limitsFromDb || efScaleMin.isNull() ) &&
       ( limitsFromDb || efScaleMax.isNull() ) ) {
    minFv = scaleMin = 0;
    maxFv = scaleMax = 10;
  }
  else{
    minFv = scaleMin = efScaleMin.value();
    maxFv = scaleMax = efScaleMax.value();
  }

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  controlX = 0;

  strcpy( controlValue, "0.0" );
  strcpy( controlLabel, "" );

  curControlV = oneControlV = controlV = 0.0;

  return 1;

}

int activeTriumfSliderClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "activeTriumfSliderClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeTriumfSliderClass_str10, 31 );

  Strncat( title, activeTriumfSliderClass_str11, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;
  bufFgColor = fgColor.pixelIndex();
  bufBgColor = bgColor.pixelIndex();
  bufBgColorMode = bgColorMode;
  bufShadeColor = shadeColor;
  bufTopColor = topColor;
  bufBotColor = botColor;
  bufIncrement = increment;
  strncpy( bufFontTag, fontTag, 63 );

  bufShowLimits = showLimits;
  bufShowLabel = showLabel;
  bufShowValue = showValue;
  bufShowSavedValue = showSavedValue;

  bufOrientation = orientation;

  if ( controlPvName.getRaw() )
    strncpy( eBuf->controlBufPvName, controlPvName.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->controlBufPvName, "" );

  if ( controlLabelName.getRaw() )
    strncpy( eBuf->controlBufLabelName, controlLabelName.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->controlBufLabelName, "" );

  if ( savedValuePvName.getRaw() )
    strncpy( eBuf->savedValueBufPvName, savedValuePvName.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->savedValueBufPvName, "" );

  bufControlLabelType = controlLabelType;

  bufLimitsFromDb = limitsFromDb;
  bufEfPrecision = efPrecision;
  bufEfScaleMin = efScaleMin;
  bufEfScaleMax = efScaleMax;

  bufFormatType = formatType;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activeTriumfSliderClass_str19, 35, &bufX );
  ef.addTextField( activeTriumfSliderClass_str20, 35, &bufY );
  ef.addTextField( activeTriumfSliderClass_str21, 35, &bufW );
  ef.addTextField( activeTriumfSliderClass_str22, 35, &bufH );

  ef.addTextField( activeTriumfSliderClass_str36, 35, eBuf->controlBufPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeTriumfSliderClass_str48, 35, eBuf->savedValueBufPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( activeTriumfSliderClass_str37, 35, eBuf->controlBufLabelName,
   PV_Factory::MAX_PV_NAME );
  labelEntry = ef.getCurItem();
  ef.addOption( activeTriumfSliderClass_str38, activeTriumfSliderClass_str39,
   &bufControlLabelType );
  labelTypeEntry = ef.getCurItem();
  labelTypeEntry->setNumValues( 3 );
  labelTypeEntry->addInvDependency( 2, labelEntry );
  labelTypeEntry->addDependencyCallbacks();

  ef.addToggle( activeTriumfSliderClass_str86, &bufShowLimits );
  ef.addToggle( activeTriumfSliderClass_str87, &bufShowLabel );
  ef.addToggle( activeTriumfSliderClass_str88, &bufShowValue );
  ef.addToggle( activeTriumfSliderClass_str92, &bufShowSavedValue );

  ef.addOption( activeTriumfSliderClass_str89,
   activeTriumfSliderClass_str90, &bufOrientation );

  ef.addTextField( activeTriumfSliderClass_str28, 35, &bufIncrement );

  ef.addToggle( activeTriumfSliderClass_str29, &bufLimitsFromDb );
  limitsFromDbEntry = ef.getCurItem();
  ef.addOption( activeTriumfSliderClass_str30, activeTriumfSliderClass_str35,
   &bufFormatType );
  ef.addTextField( activeTriumfSliderClass_str31, 35, &bufEfPrecision );
  scalePrecEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scalePrecEntry );
  ef.addTextField( activeTriumfSliderClass_str32, 35, &bufEfScaleMin );
  scaleMinEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scaleMinEntry );
  ef.addTextField( activeTriumfSliderClass_str33, 35, &bufEfScaleMax );
  scaleMaxEntry = ef.getCurItem();
  limitsFromDbEntry->addInvDependency( scaleMaxEntry );
  limitsFromDbEntry->addDependencyCallbacks();

  ef.addColorButton( activeTriumfSliderClass_str24, actWin->ci, &fgCb,
   &bufFgColor );
  ef.addColorButton( activeTriumfSliderClass_str26, actWin->ci, &bgCb,
   &bufBgColor );
  ef.addToggle( activeTriumfSliderClass_str25, &bufBgColorMode );

  ef.addColorButton( activeTriumfSliderClass_str27, actWin->ci, &shadeCb,
   &bufShadeColor );
  ef.addColorButton( activeTriumfSliderClass_str40, actWin->ci, &topCb,
   &bufTopColor );
  ef.addColorButton( activeTriumfSliderClass_str46, actWin->ci, &botCb,
   &bufBotColor );

  ef.addFontMenu( activeTriumfSliderClass_str23, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeTriumfSliderClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( mslc_edit_ok, mslc_edit_apply, mslc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeTriumfSliderClass::edit ( void ) {

  this->genericEdit();
  ef.finished( mslc_edit_ok, mslc_edit_apply, mslc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeTriumfSliderClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeTriumfSliderClass::eraseActive ( void ) {

  if ( !enabled || !active || !init ) return 1;

  actWin->executeGc.saveFg();
  actWin->executeGc.setFG( bgColor.getColor() );

  XDrawRectangle( actWin->d, XtWindow(frameWidget),
   actWin->executeGc.normGC(), 0, 0, w, h );

  XFillRectangle( actWin->d, XtWindow(frameWidget),
   actWin->executeGc.normGC(), 0, 0, w, h );

  actWin->executeGc.restoreFg();

  return 1;

}

int activeTriumfSliderClass::draw ( void ) {

int tX, tY;

  if ( deleteRequest ) return 1;

  if ( orientation == MSLC_K_HORIZONTAL ) {
    scaleX = 1;
    scaleW = w - 2;
    scaleY = labelH + limitsH + 1;
    scaleH = h - scaleY - 2;
  }
  else {
    if ( showLimits || showValue ) {
      scaleX = (int) ( 0.6 * (double) w );
      scaleW = w - scaleX - 2;
      if ( scaleW < 14 ) {
        scaleW = 14;
        scaleX = w - scaleW - 2;
      }
    }
    else {
      scaleX = 1;
      scaleW = w - 2;
    }
    scaleY = labelH + 1;
    scaleH = h - scaleY - 2;
    midVertScaleY = scaleH/2 + scaleY -
     (int) ( (double) fontHeight * 0.5 );
    midVertScaleY1 = scaleH/3 + scaleY -
     (int) ( (double) fontHeight * 0.5 );
    midVertScaleY2 = 2*scaleH/3 + scaleY -
     (int) ( (double) fontHeight * 0.5 );
  }

  actWin->drawGc.saveFg();
  actWin->executeGc.saveBg();
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  actWin->drawGc.setFG( bgColor.pixelColor() );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( actWin->ci->pix(shadeColor) );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+scaleX+2, y+scaleY+2, scaleW-4, scaleH-4 );

  actWin->drawGc.setFG( fgColor.pixelColor() );

  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  if ( showLimits ) {

    if ( orientation == MSLC_K_HORIZONTAL ) {

      tX = 2;
      tY = labelH;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, x+tX, y+tY,
       XmALIGNMENT_BEGINNING, (char *) "0.0" );

      tX = w - 2;
      tY = labelH;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_END, (char *) "10.0" );

    }
    else {

      tX = scaleX;
      tY = h - 2 - limitsH;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_END, (char *) "0.0" );

      tX = scaleX;
      tY = scaleY;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_END, (char *) "10.0" );

    }

  }

  if ( showValue ) {

    if ( orientation == MSLC_K_HORIZONTAL ) {

      tY = labelH;

      if ( showSavedValue ) {
        tX = w/3;
      }
      else {
        tX = w/2;
      }
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_CENTER, "0.0" );

      if ( showSavedValue ) {
        tX = 2*w/3;
        actWin->drawGc.setBG( fgColor.pixelColor() );
        actWin->drawGc.setFG( bgColor.pixelColor() );
        drawImageText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
         XmALIGNMENT_CENTER, "0.0" );
        actWin->drawGc.setBG( bgColor.pixelColor() );
        actWin->drawGc.setFG( fgColor.pixelColor() );
      }

    }
    else {

      tX = scaleX;

      if ( showSavedValue ) {
        tY = midVertScaleY1;
      }
      else {
        tY = midVertScaleY;
      }
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_END, "0.0" );

      if ( showSavedValue ) {
        tY = midVertScaleY2;
        actWin->drawGc.setBG( fgColor.pixelColor() );
        actWin->drawGc.setFG( bgColor.pixelColor() );
        drawImageText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
         XmALIGNMENT_END, "0.0" );
        actWin->drawGc.setBG( bgColor.pixelColor() );
        actWin->drawGc.setFG( fgColor.pixelColor() );
      }

    }

  }

  if ( showLabel ) {

    if ( orientation == MSLC_K_HORIZONTAL ) {
      tX = 2;
      tY = 0;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_BEGINNING, "Label" );
    }
    else {
      tX = w - 2;
      tY = 0;
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX+x, tY+y,
       XmALIGNMENT_END, "Label" );
    }

  }

  actWin->drawGc.restoreFg();
  actWin->drawGc.restoreBg();

  return 1;

}

int activeTriumfSliderClass::eraseActiveControlText ( void ) {

int tX, tY;

  if ( !enabled || !active || !init || !showValue ) return 1;

  if ( fs && controlExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.saveBg();

    actWin->executeGc.setFG( bgColor.getColor() );

    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    if ( orientation == MSLC_K_HORIZONTAL ) {

      tY = labelH;

      if ( showSavedValue ) {
        tX = w/3;
      }
      else {
        tX = w/2;
      }

      drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_CENTER, controlValue );

      if ( showSavedValue ) {

        actWin->executeGc.setBG( fgColor.pixelColor() );
        actWin->executeGc.setFG( fgColor.pixelColor() );

        tX = 2*w/3;
        drawImageText( frameWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_CENTER, savedValue );

        actWin->executeGc.setBG( bgColor.pixelColor() );
        actWin->executeGc.setFG( bgColor.pixelColor() );

      }

    }
    else {

      tX = scaleX;

      if ( showSavedValue ) {
        tY = midVertScaleY1;
      }
      else {
        tY = midVertScaleY;
      }
      drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
       XmALIGNMENT_END, controlValue );

      if ( showSavedValue ) {

        actWin->executeGc.setBG( fgColor.pixelColor() );
        actWin->executeGc.setFG( fgColor.pixelColor() );

        tY = midVertScaleY2;
        drawImageText( frameWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_END, savedValue );

        actWin->executeGc.setBG( bgColor.pixelColor() );
        actWin->executeGc.setFG( bgColor.pixelColor() );

      }

    }

    actWin->executeGc.restoreFg();
    actWin->executeGc.restoreBg();

  }

  return 1;

}

int activeTriumfSliderClass::drawActiveControlText ( void ) {

int tX, tY;

  if ( !enabled || !active || !init || !showValue ) return 1;

  if ( fs && controlExists ) {

    actWin->executeGc.saveFg();
    actWin->executeGc.saveBg();

    actWin->executeGc.setFG( fgColor.getColor() );

    if ( fs ) {

      actWin->executeGc.setFontTag( fontTag, actWin->fi );

      if ( orientation == MSLC_K_HORIZONTAL ) {

        tY = labelH;

        if ( showSavedValue ) {
          tX = w/3;
        }
        else {
          tX = w/2;
        }

        drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_CENTER, controlValue );

        if ( showSavedValue ) {

          actWin->executeGc.setBG( fgColor.pixelColor() );
          actWin->executeGc.setFG( bgColor.pixelColor() );

          tX = 2*w/3;
          drawImageText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_CENTER, savedValue );

          actWin->executeGc.setBG( bgColor.pixelColor() );
          actWin->executeGc.setFG( fgColor.pixelColor() );

	}

      }
      else {

        tX = scaleX;

        if ( showSavedValue ) {
          tY = midVertScaleY1;
        }
        else {
          tY = midVertScaleY;
        }
        drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_END, controlValue );

        if ( showSavedValue ) {

          actWin->executeGc.setBG( fgColor.pixelColor() );
          actWin->executeGc.setFG( bgColor.pixelColor() );

          tY = midVertScaleY2;
          drawImageText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_END, savedValue );

          actWin->executeGc.setBG( bgColor.pixelColor() );
          actWin->executeGc.setFG( fgColor.pixelColor() );

        }

      }

    }

    actWin->executeGc.restoreFg();

  }

  return 1;

}

int activeTriumfSliderClass::drawActive ( void ) {

int tX, tY;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( bgColor.getDisconnected() );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
  }

  if ( !enabled || !active || !init ) return 1;

  XtVaSetValues( frameWidget,
   XmNbackground, bgColor.getColor(),
   NULL );
  XtVaSetValues( scaleWidget,
   XmNbackground, bgColor.getColor(),
   NULL );

  actWin->executeGc.saveFg();
  actWin->executeGc.saveBg();

  actWin->executeGc.setFG( fgColor.getColor() );

  if ( fs ) {

    if ( controlExists ) {

      actWin->executeGc.setFontTag( fontTag, actWin->fi );

      if ( showLimits ) {

        if ( orientation == MSLC_K_HORIZONTAL ) {

          tX = 2;
          tY = labelH;
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_BEGINNING, minValue );

          tX = w - 2;
          tY = labelH;
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_END, maxValue );

        }
        else {

          tX = scaleX;
          tY = h - 2 - limitsH;
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_END, minValue );

          tX = scaleX;
          tY = scaleY;
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_END, maxValue );

        }

      }

      if ( showValue ) {

        if ( orientation == MSLC_K_HORIZONTAL ) {

          tY = labelH;

          if ( showSavedValue ) {
            tX = w/3;
          }
          else {
            tX = w/2;
          }

          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_CENTER, controlValue );

          if ( showSavedValue ) {

            actWin->executeGc.setBG( fgColor.pixelColor() );
            actWin->executeGc.setFG( bgColor.pixelColor() );

            tX = 2*w/3;
            drawImageText( frameWidget, &actWin->executeGc, fs, tX, tY,
             XmALIGNMENT_CENTER, savedValue );

            actWin->executeGc.setBG( bgColor.pixelColor() );
            actWin->executeGc.setFG( fgColor.pixelColor() );
          }

        }
        else {

          tX = scaleX;

          if ( showSavedValue ) {
            tY = midVertScaleY1;
          }
          else {
            tY = midVertScaleY;
          }
          drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
           XmALIGNMENT_END, controlValue );

          if ( showSavedValue ) {

            actWin->executeGc.setBG( fgColor.pixelColor() );
            actWin->executeGc.setFG( bgColor.pixelColor() );

            tY = midVertScaleY2;
            drawImageText( frameWidget, &actWin->executeGc, fs, tX, tY,
             XmALIGNMENT_END, savedValue );

            actWin->executeGc.setBG( bgColor.pixelColor() );
            actWin->executeGc.setFG( fgColor.pixelColor() );

          }

        }

      }

    }

    if ( showLabel && controlLabelExists ) {

      if ( orientation == MSLC_K_HORIZONTAL ) {
        tX = 2;
        tY = 0;
        drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_BEGINNING, controlLabel );
      }
      else {
        tX = w - 2;
        tY = 0;
        drawText( frameWidget, &actWin->executeGc, fs, tX, tY,
         XmALIGNMENT_END, controlLabel );
      }

    }

  }

  actWin->executeGc.restoreFg();
  actWin->executeGc.restoreBg();

  return 1;

}

void activeTriumfSliderClass::bufInvalidate ( void ) {

  bufInvalid = 1;

}

static void scrollBarEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

activeTriumfSliderClass *mslo;
XButtonEvent *be;
XKeyEvent *ke;
KeySym key;
char keyBuf[20];
const int keyBufSize = 20;
XComposeStatus compose;
int b2Op, charCount, stat, v;
double mult, fvalue;
#ifdef TRIUMF
int at_limit = 0;
double df; 
#endif

  *continueToDispatch = True;

  mslo = (activeTriumfSliderClass *) client;

  if ( !mslo->active ) return;

  if ( e->type == EnterNotify ) {

    *continueToDispatch = False;

    if ( mslo->controlPvId ) {
      if ( !mslo->controlPvId->have_write_access() ) {
        mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
         CURSOR_K_NO );
#ifdef TRIUMF
         // this should probably also be added to the non-TRIUMF version
		 return;
#endif
      }
      else {
        mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
         CURSOR_K_DEFAULT );
        XmProcessTraversal( mslo->scaleWidget, XmTRAVERSE_CURRENT );
        mslo->keySensitive = 1;
      }
    }
#ifdef TRIUMF
    if (mslo == activeTriumfSliderClass::selectedSlider) {
      if (mslo->outOfRange) {
        XtVaSetValues( mslo->scrollBarWidget,
          XmNtroughColor, mslo->actWin->ci->pix(mslo->colorOutofRange),
          NULL );
        } else {
        XtVaSetValues( mslo->scrollBarWidget,
          XmNtroughColor, mslo->actWin->ci->pix(mslo->colorSelected),
          NULL );
      }
    } else {
        XtVaSetValues( mslo->scrollBarWidget,
          XmNtroughColor, mslo->actWin->ci->pix(mslo->shadeColor),
          NULL );
    }
#endif
  }
  else if ( e->type == LeaveNotify ) {

    *continueToDispatch = False;

    mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
     CURSOR_K_DEFAULT );
    mslo->keySensitive = 0;
#ifdef TRIUMF
    if (mslo == activeTriumfSliderClass::selectedSlider) {
      if (mslo->outOfRange) {
        XtVaSetValues( mslo->scrollBarWidget,
          XmNtroughColor, mslo->actWin->ci->pix(mslo->colorOutofRange),
          NULL );
      } else {
        XtVaSetValues( mslo->scrollBarWidget,
          XmNtroughColor, mslo->actWin->ci->pix(mslo->colorOutofWindow),
          NULL );
      }
    }
#endif
  }

  // allow Button2 operations when no write access
  b2Op = 0;
  if ( ( e->type == ButtonPress ) || ( e->type == ButtonRelease ) ) {
    be = (XButtonEvent *) e;
    if ( be->button == Button2 ) {
      b2Op = 1;
    }
  }

  if ( mslo->controlPvId ) {
    if ( !mslo->controlPvId->have_write_access() && !b2Op ) {
      *continueToDispatch = False;
      return;
    }
  }

  if ( e->type == ButtonPress ) {

    be = (XButtonEvent *) e;

    if ( be->state & ControlMask ) {
      mult = 10.0;
    }
    else {
      mult = 1.0;
    }
    switch ( be->button ) {

//========== B4 Press ========================================

#if 0
    case Button4:

      XmScaleGetValue( mslo->scaleWidget, &v );

      if ( mslo->positive ) {
        fvalue = mslo->controlV + mslo->increment * mult;
        if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
      }
      else {
        fvalue = mslo->controlV - mslo->increment * mult;
        if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
      }

      mslo->prevScaleV = v;

      mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
       mslo->factor + 0.5 );

      XmScaleSetValue( mslo->scaleWidget, mslo->controlX );

      mslo->oldControlV = mslo->oneControlV;

      mslo->eraseActiveControlText();

      mslo->actWin->appCtx->proc->lock();
      mslo->controlV = mslo->oneControlV = mslo->curControlV;
      mslo->actWin->appCtx->proc->unlock();

      mslo->controlV = fvalue;

      snprintf( mslo->controlValue, 14, mslo->controlFormat, mslo->controlV );

      stat = mslo->drawActiveControlText();

      if ( mslo->controlExists ) {
        if ( mslo->controlPvId ) {
          stat = mslo->controlPvId->put(
           XDisplayName(mslo->actWin->appCtx->displayName),
           fvalue );
          if ( !stat ) printf( activeTriumfSliderClass_str59 );
        }
      }

      break;
#endif

//========== B4 Press ========================================

//========== B5 Press ========================================

#if 0
    case Button5:

      XmScaleGetValue( mslo->scaleWidget, &v );

      if ( mslo->positive ) {
        fvalue = mslo->controlV - mslo->increment * mult;
        if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
      }
      else {
        fvalue = mslo->controlV + mslo->increment * mult;
        if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
      }

      mslo->prevScaleV = v;

      mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
       mslo->factor + 0.5 );

      XmScaleSetValue( mslo->scaleWidget, mslo->controlX );

      mslo->oldControlV = mslo->oneControlV;

      mslo->eraseActiveControlText();

      mslo->actWin->appCtx->proc->lock();
      mslo->controlV = mslo->oneControlV = mslo->curControlV;
      mslo->actWin->appCtx->proc->unlock();

      mslo->controlV = fvalue;

      snprintf( mslo->controlValue, 14, mslo->controlFormat, mslo->controlV );

      stat = mslo->drawActiveControlText();

      if ( mslo->controlExists ) {
        if ( mslo->controlPvId ) {
          stat = mslo->controlPvId->put(
           XDisplayName(mslo->actWin->appCtx->displayName),
           fvalue );
          if ( !stat ) printf( activeTriumfSliderClass_str59 );
        }
      }

      break;
#endif

//========== B5 Press ========================================

    }

#ifdef TRIUMF
    if (( activeTriumfSliderClass::selectedSlider != mslo || mslo->outOfRange) && be->button != Button1) {
      mslo->buttonPressed = 0;
	  return;
    }
#endif
    mslo->buttonPressed = 1;

  }
  else if ( e->type == ButtonRelease ) {
#ifdef TRIUMF
    if ( activeTriumfSliderClass::selectedSlider != mslo) {
      mslo->buttonPressed = 0;
	  return;
    }
#endif

    be = (XButtonEvent *) e;

    mslo->buttonPressed = 0;

    if ( mslo->frameWidget ) {
      if ( mslo->needUnmap && mslo->isMapped ) {
        XtUnmapWidget( mslo->frameWidget );
        mslo->isMapped = 0;
      }
    }

  }
  else if ( e->type == KeyPress ) {

    ke = (XKeyEvent *) e;

    charCount = XLookupString( ke, keyBuf, keyBufSize, &key, &compose );

    if ( !mslo->keySensitive ) {

      if ( key != XK_Tab ) {
        *continueToDispatch = False;
      }
      return;

    }

    if ( ke->state & ControlMask ) {
      mult = 10.0;
    }
    else {
      mult = 1.0;
    }

    if ( key == XK_Down ) key = XK_Left;
    if ( key == XK_Up ) key = XK_Right;

    if ( ( key == XK_Left ) ||
         ( key == XK_Right ) ) {

#ifdef TRIUMF
      if ( activeTriumfSliderClass::selectedSlider != mslo || mslo->outOfRange) {
	    return;
      }
#endif	  
      *continueToDispatch = False;

      XmScaleGetValue( mslo->scaleWidget, &v );

      if ( key == XK_Left ) {

        if ( mslo->positive ) {
          fvalue = mslo->controlV - mslo->increment * mult;
          if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
          if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
        }
        else {
          fvalue = mslo->controlV + mslo->increment * mult;
          if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
          if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
        }

      }
      else { // key == XK_Right

        if ( mslo->positive ) {
          fvalue = mslo->controlV + mslo->increment * mult;
          if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
          if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
        }
        else {
          fvalue = mslo->controlV - mslo->increment * mult;
          if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
          if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
        }

      }
#ifdef TRIUMF
      df = 2*mslo->increment;
      if (fabs(fvalue - mslo->minFv) < df || fabs(fvalue - mslo->maxFv) < df) {
  	    at_limit = 1;
        updateSliderLimits(mslo);
      }
#endif
 //     mslo->prevScaleV = v;
      mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
       mslo->factor + 0.5 );

      XmScaleSetValue( mslo->scaleWidget, mslo->controlX );
      mslo->prevScaleV = mslo->controlX;

      mslo->oldControlV = mslo->oneControlV;

#ifdef TRIUMF
      at_limit ? mslo->eraseActive() : mslo->eraseActiveControlText();
#else
      mslo->eraseActiveControlText();
#endif	  

      mslo->actWin->appCtx->proc->lock();
      mslo->controlV = mslo->oneControlV = mslo->curControlV;
      mslo->actWin->appCtx->proc->unlock();

      mslo->controlV = fvalue;

      snprintf( mslo->controlValue, 14, mslo->controlFormat, mslo->controlV );
#ifdef TRIUMF
      if (at_limit) {
        snprintf( mslo->minValue, 14, "%-g", mslo->minFv );
        snprintf( mslo->maxValue, 14, "%-g", mslo->maxFv );
      }
      stat =  (at_limit ? mslo->drawActive() : mslo->drawActiveControlText() );
#else
      stat = mslo->drawActiveControlText();
#endif
      if ( mslo->controlExists ) {
        if ( mslo->controlPvId ) {
          stat = mslo->controlPvId->put(
           XDisplayName(mslo->actWin->appCtx->displayName),
           fvalue );
          if ( !stat ) printf( activeTriumfSliderClass_str59 );
        }
      }

    }
    else if ( key == XK_S ) {

      *continueToDispatch = False;

      mslo->savedV = mslo->controlV;

      if ( mslo->savedValueExists ) {
        if ( mslo->savedValuePvId ) {
          stat = mslo->savedValuePvId->put(
           XDisplayName(mslo->actWin->appCtx->displayName),
           mslo->savedV );
          if ( !stat ) printf( activeTriumfSliderClass_str59 );
        }
      }
      else {
        mslo->newSavedV = mslo->savedV;
        mslo->needSavedRefresh = 1;
        mslo->actWin->appCtx->proc->lock();
        mslo->actWin->addDefExeNode( mslo->aglPtr );
        mslo->actWin->appCtx->proc->unlock();
      }

    }
    else if ( key == XK_R ) {

      *continueToDispatch = False;

#ifndef TRIUMF
      mslo->controlV = mslo->savedV;
#else
      mslo->curControlV = mslo->savedV;
      mslo->controlV = mslo->savedV;
	  mslo->keyRestore = 1;
#endif 
     if ( mslo->controlExists ) {
        if ( mslo->controlPvId ) {
          stat = mslo->controlPvId->put(
           XDisplayName(mslo->actWin->appCtx->displayName),
           mslo->controlV );
          if ( !stat ) printf( activeTriumfSliderClass_str59 );
        }
      }

    }

  }

}

static void triumfSliderEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

XButtonEvent *be;
activeTriumfSliderClass *mslo;
int stat, b2Op;
char title[32], *ptr, strVal[255+1];

#if 0
// wheel mouse support (btn 4, 5)
int v;
double fvalue, mult;
#endif

  *continueToDispatch = True;

  mslo = (activeTriumfSliderClass *) client;

  if ( !mslo->enabled || !mslo->active ) return;

  if ( e->type == EnterNotify ) {

    if ( mslo->controlPvId ) {
      if ( !mslo->controlPvId->have_write_access() ) {
        mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
         CURSOR_K_NO );
      }
      else {
        mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
         CURSOR_K_DEFAULT );
      }
    }

  }
  else if ( e->type == LeaveNotify ) {

    mslo->actWin->cursor.set( XtWindow(mslo->actWin->executeWidget),
     CURSOR_K_DEFAULT );
  }
#ifndef TRIUMF
  ptr = mslo->actWin->obj.getNameFromClass( "activeTriumfSliderClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activeTriumfSliderClass_str54, 31 );

  Strncat( title, activeTriumfSliderClass_str55, 31 );
#else
  strncpy( title, "Slider: ", 31 );
  Strncat( title, mslo->controlPvName.getExpanded(), 31 );
#endif
  if ( e->type == Expose ) {

    mslo->bufInvalidate();
    stat = mslo->drawActive();
    return;

  }

  // allow Button2 operations when no write access
  b2Op = 0;
  if ( ( e->type == ButtonPress ) || ( e->type == ButtonRelease ) ) {
    be = (XButtonEvent *) e;
    if ( be->button == Button2 ) {
      b2Op = 1;
    }
  }

  if ( mslo->controlPvId ) {
    if ( !mslo->controlPvId->have_write_access() && !b2Op ) {
      *continueToDispatch = False;
      return;
    }
  }

  if ( e->type == ButtonPress ) {

    be = (XButtonEvent *) e;

#if 0
// wheel mouse support (btn 4, 5)
    if ( be->state & ControlMask ) {
      mult = 10.0;
    }
    else {
      mult = 1.0;
    }
#endif

    switch ( be->button ) {

//========== B2 Press ========================================

    case Button2:

      if ( !( be->state & ( ControlMask | ShiftMask ) ) ) {
        stat = mslo->startDrag( w, e );
      }
      else if ( ( be->state & ShiftMask ) &&
                ( be->state & ControlMask ) ) {
        stat = mslo->showPvInfo( be, be->x, be->y );
      }

      break;

//========== B2 Press ========================================

//========== B3 Press ========================================

    case Button3:
#ifdef TRIUMF
#ifdef DBG
      printf("triumfSlider: button 3\n");
#endif
      if ( activeTriumfSliderClass::selectedSlider != mslo) {
	    break;
      }
#endif
      if ( !mslo->ef.formIsPoppedUp() ) {

        mslo->bufIncrement = mslo->increment;
#ifdef TRIUMF
        // move the current value to the buffer, as slider might be out of range
        mslo->bufControlV = mslo->curControlV;
#else
        mslo->bufControlV = mslo->controlV;
#endif
        mslo->valueFormX = be->x_root;
        mslo->valueFormY = be->y_root;
        mslo->valueFormW = 0;
        mslo->valueFormH = 0;
        mslo->valueFormMaxH = 600;

        mslo->ef.create( mslo->actWin->top,
          mslo->actWin->appCtx->ci.getColorMap(),
          &mslo->valueFormX, &mslo->valueFormY,
          &mslo->valueFormW, &mslo->valueFormH, &mslo->valueFormMaxH,
          title, NULL, NULL, NULL );

#ifndef TRIUMF
        mslo->ef.addTextField( activeTriumfSliderClass_str57, 20,
          &mslo->bufControlV );
        mslo->ef.addTextField( activeTriumfSliderClass_str58, 20,
          &mslo->bufIncrement );
        calcIncRange( mslo->minFv, mslo->maxFv, strVal, mslo->incArray );
#else
        mslo->valueEntry = mslo->ef.addTextFieldAccessible( activeTriumfSliderClass_str57, 20,
          &mslo->bufControlV );
        mslo->incEntry = mslo->ef.addTextFieldAccessible( activeTriumfSliderClass_str58, 20,
          &mslo->bufIncrement );
        calcIncRange( mslo->scaleMin, mslo->scaleMax, strVal, mslo->incArray );
#endif

        mslo->incIndex = 0;
        mslo->ef.addOption( activeTriumfSliderClass_str58, strVal,
          &mslo->incIndex );

#ifdef TRIUMF
        mslo->ef.addOption( activeTriumfSliderClass_str93, activeTriumfSliderClass::rangeString,
          &mslo->rangeIndex );
#endif
        mslo->ef.finished( mslc_value_ok, mslc_value_apply, mslc_value_cancel,
          mslo );

        mslo->ef.popup();

      }

      break;

//========== B3 Press ========================================

//========== B4 Press ========================================

#if 0
    case Button4:

      XmScaleGetValue( mslo->scaleWidget, &v );

      if ( mslo->positive ) {
        fvalue = mslo->controlV + mslo->increment * mult;
        if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
      }
      else {
        fvalue = mslo->controlV - mslo->increment * mult;
        if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
      }

      mslo->prevScaleV = v;

      mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
       mslo->factor + 0.5 );

      XmScaleSetValue( mslo->scaleWidget, mslo->controlX );

      mslo->oldControlV = mslo->oneControlV;

      mslo->eraseActiveControlText();

      mslo->actWin->appCtx->proc->lock();
      mslo->controlV = mslo->oneControlV = mslo->curControlV;
      mslo->actWin->appCtx->proc->unlock();

      mslo->controlV = fvalue;

      snprintf( mslo->controlValue, 14, mslo->controlFormat, mslo->controlV );

      stat = mslo->drawActiveControlText();

      if ( mslo->controlExists ) {
        if ( mslo->controlPvId ) {
          stat = mslo->controlPvId->put(
           XDisplayName(mslo->actWin->appCtx->displayName),
           fvalue );
          if ( !stat ) printf( activeTriumfSliderClass_str59 );
        }
      }

      break;
#endif

//========== B4 Press ========================================

//========== B5 Press ========================================

#if 0
    case Button5:

      XmScaleGetValue( mslo->scaleWidget, &v );

      if ( mslo->positive ) {
        fvalue = mslo->controlV - mslo->increment * mult;
        if ( fvalue < mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue > mslo->maxFv ) fvalue = mslo->maxFv;
      }
      else {
        fvalue = mslo->controlV + mslo->increment * mult;
        if ( fvalue > mslo->minFv ) fvalue = mslo->minFv;
        if ( fvalue < mslo->maxFv ) fvalue = mslo->maxFv;
      }

     mslo->prevScaleV = v;

      mslo->controlX = (int) ( ( fvalue - mslo->minFv ) /
       mslo->factor + 0.5 );

      XmScaleSetValue( mslo->scaleWidget, mslo->controlX );

      mslo->oldControlV = mslo->oneControlV;

      at_limit ? mslo->eraseActive() : mslo->eraseActiveControlText();

      mslo->actWin->appCtx->proc->lock();
      mslo->controlV = mslo->oneControlV = mslo->curControlV;
      mslo->actWin->appCtx->proc->unlock();

      mslo->controlV = fvalue;

      snprintf( mslo->controlValue, 14, mslo->controlFormat, mslo->controlV );
      stat =  mslo->drawActiveControlText();

      if ( mslo->controlExists ) {
        if ( mslo->controlPvId ) {
          stat = mslo->controlPvId->put(
           XDisplayName(mslo->actWin->appCtx->displayName),
           fvalue );
          if ( !stat ) printf( activeTriumfSliderClass_str59 );
        }
      }

      break;
#endif

//========== B5 Press ========================================

    }

  }
  if ( e->type == ButtonRelease ) {

//========== Any B Release ========================================

    be = (XButtonEvent *) e;

    switch ( be->button ) {

    case Button2:

      if ( ( be->state & ShiftMask ) &&
           !( be->state & ControlMask ) ) {
        stat = mslo->selectDragValue( be );
      }
      else if ( !( be->state & ShiftMask ) &&
                ( be->state & ControlMask ) ) {
        mslo->doActions( be, be->x, be->y );
      }

      break;

    }

//========== Any B Release ========================================

  }

}

#ifdef TRIUMF

void activeTriumfSliderClass::changeSelectedSlider(activeTriumfSliderClass *mslo, int fromPopup)
{
  if (activeTriumfSliderClass::selectedSlider != mslo) {
    // if we have a selected slider, we reset it to the unselected color
    if (activeTriumfSliderClass::selectedSlider) {
      activeTriumfSliderClass::selectedSlider->shadeColor = activeTriumfSliderClass::selectedSlider->shadeColorSaved;
      activeTriumfSliderClass::selectedSlider->shadeColorInRange = activeTriumfSliderClass::selectedSlider->shadeColorSaved;
      XtVaSetValues( activeTriumfSliderClass::selectedSlider->scrollBarWidget,
           XmNtroughColor, activeTriumfSliderClass::selectedSlider->actWin->ci->pix(activeTriumfSliderClass::selectedSlider->shadeColor),
           NULL );
	}
	// .. then set the newly selected slider to the "selected color"
	if (mslo) {
      if (fromPopup) {
        mslo->shadeColor = mslo->colorOutofWindow;
        mslo->shadeColorInRange = mslo->colorOutofWindow;
      } else {
        mslo->shadeColor = mslo->colorSelected;
        mslo->shadeColorInRange = mslo->colorSelected;
      }
      activeTriumfSliderClass::selectedSlider = mslo;
      XtVaSetValues( mslo->scrollBarWidget,
         XmNtroughColor, mslo->actWin->ci->pix(mslo->shadeColor),
         NULL );
	  newSelect = 1;
    }
  } else if (mslo->newSelect) {
      mslo->shadeColor = mslo->colorSelected;
      XtVaSetValues( mslo->scrollBarWidget,
         XmNtroughColor, mslo->actWin->ci->pix(mslo->shadeColor),
         NULL );
  }
}
#endif



int activeTriumfSliderClass::activate (
  int pass,
  void *ptr )
{

int opStat;

#ifdef TRIUMF
      shadeColorSaved = shadeColor;
      shadeColorInRange = shadeColor;
	  newSelect = 0;
#endif  
  switch ( pass ) {

  case 1:

    opComplete = 0;
    break;

  case 2:

    if ( !opComplete ) {

      opComplete = 1;

      initEnable();
      isMapped = 0;
      buttonPressed = 0;
      needUnmap = 0;

      oldStat = -1;
      oldSev = -1;
      prevScaleV = -1;
      dragIndicator = 0;
      controlPvId = controlLabelPvId = savedValuePvId = 0;

      strcpy( controlValue, "" );
      strcpy( incString, "" );
      strcpy( savedValue, "" );
      activeMode = 1;
      init = 0;
      active = 0;
      aglPtr = ptr;
      curControlV = oneControlV = 0.0;
      controlV = 0.0;
      savedV = 0.0;
      needCtlConnectInit = needCtlInfoInit = needCtlRefresh =
      needCtlLabelConnectInit = needCtlLabelInfoInit =
      needSavedConnectInit = needSavedRefresh =
      needErase = needDraw = 0;
#ifdef TRIUMF
  	  needCtlInfoInitNoSave = 0;
#endif
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      oldControlV = 0;
      updateControlTimerActive = 0;
      updateControlTimer = 0;
      incrementTimerActive = 0;

      controlPvConnected = 0;

      if ( !controlPvName.getExpanded() ||
         // ( strcmp( controlPvName.getExpanded(), "" ) == 0 ) ) {
        blankOrComment( controlPvName.getExpanded() ) ) {
        controlExists = 0;
      }
      else {
        controlExists = 1;
        fgColor.setConnectSensitive();
        bgColor.setConnectSensitive();
      }

      savedValuePvConnected = 0;

      if ( !savedValuePvName.getExpanded() ||
	 // ( strcmp( savedValuePvName.getExpanded(), "" ) == 0 ) ) {
         blankOrComment( savedValuePvName.getExpanded() ) ) {
        savedValueExists = 0;
      }
      else {
        savedValueExists = 1;
      }

      if ( !controlLabelName.getExpanded() ||
         // ( strcmp( controlLabelName.getExpanded(), "" ) == 0 ) ) {
        blankOrComment( controlLabelName.getExpanded() ) ) {
        controlLabelExists = 0;
      }
      else {
        controlLabelExists = 1;
      }

      if ( controlLabelType == MSLC_K_PV_NAME ) {
        controlLabelExists = 1;
        strncpy( controlLabel, controlPvName.getExpanded(),
         PV_Factory::MAX_PV_NAME );
      }
      else {
        strncpy( controlLabel, controlLabelName.getExpanded(),
         PV_Factory::MAX_PV_NAME );
      }

    }

    break;

  case 3:

    opComplete = 0;
    break;

  case 4:

    if ( !opComplete ) {

      opStat = 1;

      if ( controlExists ) {
        controlPvId = the_PV_Factory->create( controlPvName.getExpanded() );
        if ( controlPvId ) {
          controlPvId->add_conn_state_callback(
           monitorControlConnectState, this );
          controlPvId->add_value_callback(
           controlUpdate, this );
        }
      }

      if ( savedValueExists ) {
        savedValuePvId = the_PV_Factory->create(
         savedValuePvName.getExpanded() );
        if ( savedValuePvId ) {
          savedValuePvId->add_conn_state_callback(
           monitorSavedValueConnectState, this );
          savedValuePvId->add_value_callback(
           savedValueUpdate, this );
        }
      }

      if ( controlLabelExists && ( controlLabelType == MSLC_K_LABEL )  ) {
        controlLabelPvId =
         the_PV_Factory->create( controlLabelName.getExpanded() );
        if ( controlLabelPvId ) {
          controlLabelPvId->add_conn_state_callback(
           monitorControlLabelConnectState, this );
          controlLabelPvId->add_value_callback(
           controlLabelUpdate, this );
	}

      }

      if ( opStat & 1 ) {

        //updateControlTimerActive = 1;
        //updateControlTimerValue = 100;
        //updateControlTimer = appAddTimeOut( actWin->appCtx->appContext(),
        // updateControlTimerValue, mslc_updateControl, (void *) this );

        opComplete = 1;

      }

      return opStat;

    }

    break;

  case 5:
  case 6:

    break;

  }

  return 1;

}

int activeTriumfSliderClass::deactivate (
  int pass
) {

  active = activeMode = 0;

  switch ( pass ) {

  case 1:

// for EPICS support

    if ( controlPvId ) {
      controlPvId->remove_conn_state_callback(
       monitorControlConnectState, this );
      controlPvId->remove_value_callback(
       controlUpdate, this );
      controlPvId->release();
      controlPvId = 0;
    }

    if ( savedValuePvId ) {
      savedValuePvId->remove_conn_state_callback(
       monitorSavedValueConnectState, this );
       savedValuePvId->remove_value_callback( savedValueUpdate, this );
       savedValuePvId->release();
       savedValuePvId = 0;
    }

    if ( controlLabelPvId ) {
      controlLabelPvId->remove_conn_state_callback(
       monitorControlLabelConnectState, this );
      controlLabelPvId->remove_value_callback(
       controlLabelUpdate, this );
      controlLabelPvId->release();
      controlLabelPvId = 0;
    }

    if ( ef.formIsPoppedUp() ) {
      ef.popdown();
    }

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    if ( updateControlTimerActive ) {
      updateControlTimerActive = 0;
      if ( updateControlTimer ) {
        XtRemoveTimeOut( updateControlTimer );
        updateControlTimer = 0;
      }
    }

    if ( frameWidget ) {
      XtRemoveEventHandler( frameWidget,
       ButtonPressMask|ExposureMask|EnterWindowMask|LeaveWindowMask, False,
       triumfSliderEventHandler, (XtPointer) this );
    }

    if ( scrollBarWidget ) {
      XtRemoveEventHandler( scrollBarWidget,
       KeyPressMask|ButtonPressMask|ButtonReleaseMask|EnterWindowMask|LeaveWindowMask, False,
       scrollBarEventHandler, (XtPointer) this );
    }

    if ( scaleWidget ) {
      XtRemoveCallback( scaleWidget, XmNvalueChangedCallback,
       msloValueChangeCB, (XtPointer) this );
      XtRemoveCallback( scaleWidget, XmNdragCallback,
       msloIndicatorDragCB, (XtPointer) this );
    }

    if ( frameWidget ) {
      if ( scaleWidget ) {
        XtUnmanageChild( scaleWidget );
        XtDestroyWidget( scaleWidget );
        scaleWidget = NULL;
        scrollBarWidget = NULL;
      }
      XtUnmanageChild( frameWidget );
      XtDestroyWidget( frameWidget );
      frameWidget = NULL;
    }
#ifdef TRIUMF
    if (activeTriumfSliderClass::selectedSlider == this) {
      activeTriumfSliderClass::selectedSlider = NULL;
      shadeColor = shadeColorSaved;
    }
	range = 1.; // restore full slider range
#endif
    break;

  case 2:

    break;

  }

  return 1;

}

int activeTriumfSliderClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  tmpw = sboxW;
  tmph = sboxH;

  ret_stat = 1;

  tmpw += _w;
  if ( tmpw < minW ) {
    ret_stat = 0;
  }

  tmph += _h;
  if ( tmph < minH ) {
    ret_stat = 0;
  }

  return ret_stat;

}

int activeTriumfSliderClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  ret_stat = 1;

  if ( _w != -1 ) {
    tmpw = _w;
    if ( tmpw < minW ) {
      ret_stat = 0;
    }
  }

  if ( _h != -1 ) {
    tmph = _h;
    if ( tmph < minH ) {
      ret_stat = 0;
    }
  }

  return ret_stat;

}

void activeTriumfSliderClass::updateDimensions ( void )
{

  if ( fs ) {
    fontAscent = fs->ascent;
    fontDescent = fs->descent;
    fontHeight = fontAscent + fontDescent;
  }
  else {
    fontAscent = 10;
    fontDescent = 5;
    fontHeight = fontAscent + fontDescent;
  }

  if ( showLimits || showValue ) {
    limitsH = fontHeight;
  }
  else {
    limitsH = 0;
  }

  if ( showLabel ) {
    labelH = fontHeight;
  }
  else {
    labelH = 0;
  }

  if ( orientation == MSLC_K_HORIZONTAL ) {
    minW = 50;
    minH = 14 + limitsH + labelH;
  }
  else {
    minW = 14;
    minH = 50;
  }

  // dummy values at this point
  minFv = 0.0;
  maxFv = 10.0;
  positive = 1;

  factor = ( maxFv - minFv ) / 100000;
  if ( factor == 0.0 ) factor = 1.0;

  midVertScaleY = scaleH/2 + scaleY - (int) ( (double) fontHeight * 0.5 );
  midVertScaleY1 = scaleH/3 + scaleY - (int) ( (double) fontHeight * 0.5 );
  midVertScaleY2 = 2*scaleH/3 + scaleY - (int) ( (double) fontHeight * 0.5 );

}

int activeTriumfSliderClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( controlPvName.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  controlPvName.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( savedValuePvName.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  savedValuePvName.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( controlLabelName.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  controlLabelName.setRaw( tmpStr.getExpanded() );

  return 1;

}

int activeTriumfSliderClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

  int retStat, stat;

  retStat = 1;

  stat = controlPvName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = savedValuePvName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = controlLabelName.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeTriumfSliderClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

  int retStat, stat;

  retStat = 1;

  stat = controlPvName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = savedValuePvName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = controlLabelName.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activeTriumfSliderClass::containsMacros ( void ) {

  if ( controlPvName.containsPrimaryMacros() ) return 1;

  if ( savedValuePvName.containsPrimaryMacros() ) return 1;

  if ( controlLabelName.containsPrimaryMacros() ) return 1;

  return 0;

}

void activeTriumfSliderClass::executeDeferred ( void ) {

int stat, ncc, nci, ncins, ncr, nclc, ncli, nsc, nsr, ne, nd, i;
unsigned char orien, pd;
double cv, fv;
WidgetList children;
Cardinal numChildren;


  if ( actWin->isIconified ) return;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  ncc = needCtlConnectInit; needCtlConnectInit = 0;
  nci = needCtlInfoInit; needCtlInfoInit = 0;
#ifdef TRIUMF
  ncins = needCtlInfoInitNoSave; needCtlInfoInitNoSave = 0;
#endif
  ncr = needCtlRefresh; needCtlRefresh = 0;
  nclc = needCtlLabelConnectInit; needCtlLabelConnectInit = 0;
  ncli = needCtlLabelInfoInit; needCtlLabelInfoInit = 0;
  nsc = needSavedConnectInit; needSavedConnectInit = 0;
  nsr = needSavedRefresh; needSavedRefresh = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  cv = curControlV;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

#ifdef DBG
  printf("triumfSlider: executeDeferred\nncc%d nci%d ncins%d ncr%d nclc%d ncli%d nsc%d nsr%d ne%d nd%d\n",
        ncc, nci, ncins, ncr, nclc, ncli, nsc, nsr, ne, nd);
#endif

  if ( ncc ) {

    controlPvConnected = 1;

    if ( !frameWidget ) {

      frameWidget = XtVaCreateManagedWidget( "",
       xmDrawingAreaWidgetClass,
       actWin->executeWidgetId(),
       XmNx, x,
       XmNy, y,
       XmNwidth, w,
       XmNheight, h,
       XmNmarginHeight, 0,
       XmNmarginWidth, 0,
       XmNresizePolicy, XmRESIZE_NONE,
       XmNbackground, bgColor.pixelColor(),
       XmNmappedWhenManaged, False,
       NULL );

      if ( frameWidget ) {

        XtAddEventHandler( frameWidget,
         ButtonPressMask|ButtonReleaseMask|ExposureMask|
         EnterWindowMask|LeaveWindowMask, False,
         triumfSliderEventHandler, (XtPointer) this );

        if ( orientation == MSLC_K_HORIZONTAL ) {
          scaleX = 1;
          scaleW = w - 2;
          scaleY = labelH + limitsH + 1;
          scaleH = h - scaleY - 2;
        }
        else {
          if ( showLimits || showValue ) {
            scaleX = (int) ( 0.6 * (double) w );
            scaleW = w - scaleX - 2;
            if ( scaleW < 14 ) {
              scaleW = 14;
              scaleX = w - scaleW - 2;
            }
          }
          else {
            scaleX = 1;
            scaleW = w - 2;
          }
          scaleY = labelH + 1;
          scaleH = h - scaleY - 2;
          midVertScaleY = scaleH/2 + scaleY -
           (int) ( (double) fontHeight * 0.5 );
          midVertScaleY1 = scaleH/3 + scaleY -
           (int) ( (double) fontHeight * 0.5 );
          midVertScaleY2 = 2*scaleH/3 + scaleY -
           (int) ( (double) fontHeight * 0.5 );
        }

        if ( g_transInit ) {
          g_transInit = 0;
          g_parsedTrans = XtParseTranslationTable( g_dragTrans );
        }
        actWin->appCtx->addActions( g_dragActions, XtNumber(g_dragActions) );

        if ( orientation == MSLC_K_HORIZONTAL ) {
          orien = XmHORIZONTAL;
          pd = XmMAX_ON_RIGHT;
        }
        else {
          orien = XmVERTICAL;
          pd = XmMAX_ON_TOP;
        }

        scaleWidget = XtVaCreateManagedWidget(
         "", xmScaleWidgetClass,
         frameWidget,
         XmNx, scaleX,
         XmNy, scaleY,
         XmNwidth, scaleW,
         XmNheight, scaleH,
         XmNscaleWidth, scaleW,
         XmNscaleHeight, scaleH,
         XmNorientation, orien,
         XmNprocessingDirection, pd,
         XmNscaleMultiple, 1,
         XmNminimum, 0,
         XmNmaximum, 100000,
         XmNnavigationType, XmTAB_GROUP,
         XmNtraversalOn, True,
         XmNhighlightOnEnter, False,
         XmNuserData, this,
         XmNforeground, fgColor.getColor(),
         XmNbackground, bgColor.pixelColor(),
         XmNtopShadowColor, actWin->ci->pix(topColor),
         XmNbottomShadowColor, actWin->ci->pix(botColor),
         NULL );

        XtVaGetValues( scaleWidget,
         XmNnumChildren, &numChildren,
         XmNchildren, &children,
         NULL );

        scrollBarWidget = NULL;
        for ( i=0; i<(int)numChildren; i++ ) {
          if ( XtClass( children[i] ) == xmScrollBarWidgetClass) {
            scrollBarWidget = children[i];
            XtVaSetValues( children[i],
             XmNuserData, this,
             NULL );
            XtOverrideTranslations( children[i], g_parsedTrans );
          }
        }

        if ( scrollBarWidget ) {

          XtVaSetValues( scrollBarWidget,
           XmNforeground, fgColor.getColor(),
           XmNbackground, bgColor.pixelColor(),
           XmNtroughColor, actWin->ci->pix(shadeColor),
           XmNtopShadowColor, actWin->ci->pix(topColor),
           XmNbottomShadowColor, actWin->ci->pix(botColor),
           XmNinitialDelay, 500,
           XmNrepeatDelay, 1,
           NULL );

          XtAddEventHandler( scrollBarWidget,
           KeyPressMask|ButtonPressMask|ButtonReleaseMask|EnterWindowMask|LeaveWindowMask,
           False, scrollBarEventHandler, (XtPointer) this );

        }

        XtAddCallback( scaleWidget, XmNvalueChangedCallback,
         msloValueChangeCB, (XtPointer) this );

        XtAddCallback( scaleWidget, XmNdragCallback,
         msloIndicatorDragCB, (XtPointer) this );

        XtManageChild( frameWidget );

	if ( enabled ) {
          XtMapWidget( frameWidget );
          isMapped = 1;
	}
        else {
          isMapped = 0;
	}

      }

    }

  }

//----------------------------------------------------------------------------

  if ( nci ) {

    controlV = cv;

    snprintf( minValue, 14, "%-g", minFv );

    snprintf( maxValue, 14, "%-g", maxFv );

    if ( maxFv > minFv )
      positive = 1;
    else
      positive = 0;

    snprintf( controlValue, 14, controlFormat, controlV );

    factor = ( maxFv - minFv ) / 100000;
    if ( factor == 0.0 ) factor = 1.0;

    controlX = (int) ( ( controlV - minFv ) /
     factor + 0.5 );

    snprintf( incString, 31, controlFormat, increment );

    active = 1;
    init = 1;

    if ( !savedValueExists && !ncins) {
      savedV = controlV;
      snprintf( savedValue, 14, controlFormat, savedV );
    }

    fgColor.setConnected();
    bgColor.setConnected();

    bufInvalidate();

    stat = eraseActive();
    stat = drawActive();

    bufInvalidate();

  }

//----------------------------------------------------------------------------

  if ( ncr ) {

    eraseActiveControlText();

    if ( positive ) {

      if ( controlV < minFv )
        fv = minFv;
      else if ( controlV > maxFv )
        fv = maxFv;
      else
        fv = controlV;

    }
    else {

      if ( controlV > minFv )
        fv = minFv;
      else if ( controlV < maxFv )
        fv = maxFv;
      else
        fv = controlV;

    }

    controlX = (int) ( ( fv - minFv ) /
     factor + 0.5 );

    snprintf( controlValue, 14, controlFormat, controlV );

    stat = drawActiveControlText();

  }

//----------------------------------------------------------------------------

  if ( nsc ) {

    savedValuePvConnected = 1;

  }

//----------------------------------------------------------------------------

  if ( nsr ) {

    eraseActiveControlText();

    savedV = newSavedV;

    snprintf( savedValue, 14, controlFormat, savedV );

    stat = drawActiveControlText();

  }

//----------------------------------------------------------------------------

  if ( nclc ) {

    ; // do nothing

  }

//----------------------------------------------------------------------------

  if ( ncli ) {

    if ( active ) {
      stat = eraseActive();
      stat = drawActive();
    }

    bufInvalidate();

  }

//----------------------------------------------------------------------------

  if ( ne ) {

    eraseActive();

  }

//----------------------------------------------------------------------------

  if ( nd ) {

    drawActive();

  }

//----------------------------------------------------------------------------

}

int activeTriumfSliderClass::getProperty (
  char *prop,
  double *_value )
{

  if ( strcmp( prop, activeTriumfSliderClass_str85 ) == 0 ) {

    *_value = controlV;
    return 1;

  }

  return 0;

}

char *activeTriumfSliderClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *activeTriumfSliderClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *activeTriumfSliderClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    switch ( i ) {
    case 0:
      return controlPvName.getExpanded();
      break;
    case 1:
      return savedValuePvName.getExpanded();
      break;
    }

  }
  else {

    switch ( i ) {
    case 0:
      return controlPvName.getRaw();
      break;
    case 1:
      return savedValuePvName.getRaw();
      break;
    }

  }

  return NULL;

}

void activeTriumfSliderClass::changeDisplayParams (
  unsigned int _flag,
  char *_fontTag,
  int _alignment,
  char *_ctlFontTag,
  int _ctlAlignment,
  char *_btnFontTag,
  int _btnAlignment,
  int _textFgColor,
  int _fg1Color,
  int _fg2Color,
  int _offsetColor,
  int _bgColor,
  int _topShadowColor,
  int _botShadowColor )
{

  if ( _flag & ACTGRF_TEXTFGCOLOR_MASK )
    fgColor.setColorIndex( _textFgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_OFFSETCOLOR_MASK )
    shadeColor = _offsetColor;

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topColor = _topShadowColor;

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botColor = _botShadowColor;

  if ( _flag & ACTGRF_CTLFONTTAG_MASK ) {

    strcpy( fontTag, _ctlFontTag );
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );

    updateDimensions();
    if ( h < minH ) h = minH;
    if ( w < minW ) w = minW;

  }

}

void activeTriumfSliderClass::changePvNames (
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
  char *alarmPvs[] )
{

  if ( flag & ACTGRF_CTLPVS_MASK ) {
    if ( numCtlPvs ) {
      controlPvName.setRaw( ctlPvs[0] );
    }
  }

}

void activeTriumfSliderClass::map ( void ) {

  needUnmap = 0;

  if ( frameWidget ) {
    if ( !isMapped ) {
      XtMapWidget( frameWidget );
      isMapped = 1;
    }
  }

}

void _edmDebug( void );

void activeTriumfSliderClass::unmap ( void ) {

  if ( buttonPressed ) {
    needUnmap = 1;
    return;
  }
  else {
    needUnmap = 0;
  }

  if ( frameWidget ) {
    if ( isMapped ) {
      XtUnmapWidget( frameWidget );
      isMapped = 0;
    }
  }

}

void activeTriumfSliderClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 2 ) {
    *n = 0;
    return;
  }

  *n = 2;
  pvs[0] = controlPvId;
  pvs[1] = savedValuePvId;

}

char *activeTriumfSliderClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return controlPvName.getRaw();
  }
  else if ( i == 1 ) {
    return savedValuePvName.getRaw();
  }
  else if ( i == 2 ) {
    return controlLabelName.getRaw();
  }

  return NULL;

}

void activeTriumfSliderClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    controlPvName.setRaw( string );
  }
  else if ( i == 1 ) {
    savedValuePvName.setRaw( string );
  }
  else if ( i == 2 ) {
    controlLabelName.setRaw( string );
  }

}

// crawler functions may return blank pv names
char *activeTriumfSliderClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;
  return controlPvName.getExpanded();

}

char *activeTriumfSliderClass::crawlerGetNextPv ( void ) {

int max;

  if ( controlLabelType != MSLC_K_LITERAL ) { // label name is a pv
    max = 2;
  }
  else {
    max = 1;
  }

  if ( crawlerPvIndex >= max ) return NULL;

  crawlerPvIndex++;

  if ( crawlerPvIndex == 1 ) {
    return savedValuePvName.getExpanded();
  }
  else if ( crawlerPvIndex == 2 ) {
    return controlLabelName.getExpanded();
  }

  return NULL;

}


activeTriumfSliderClass* activeTriumfSliderClass::selectedSlider = NULL;
char* activeTriumfSliderClass::rangeString = "100%|10%|1%|.1%|.01%|.001%|.0001%";
double activeTriumfSliderClass::rangeArray[7] = { 1., 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001 };


#ifdef __cplusplus
extern "C" {
#endif

void *create_activeTriumfSliderClassPtr ( void ) {

activeTriumfSliderClass *ptr;

  ptr = new activeTriumfSliderClass;
  return (void *) ptr;

}

void *clone_activeTriumfSliderClassPtr (
  void *_srcPtr )
{

activeTriumfSliderClass *ptr, *srcPtr;

  srcPtr = (activeTriumfSliderClass *) _srcPtr;

  ptr = new activeTriumfSliderClass( srcPtr );

  return (void *) ptr;

}


#ifdef __cplusplus
}
#endif
