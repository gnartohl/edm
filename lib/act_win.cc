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

#define __act_win_cc 1

#include <signal.h>
#include <unistd.h>
#include <setjmp.h>

#include <math.h>
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"
#include "crc.h"

void _edmDebug ( void ) {

int i;

  i = 1;

}

void printErrMsg (
  const char *fileName,
  int lineNum,
  const char *msg )
{

  printf( "==============================================================\n" );
  printf( "Internal error in %s at line %-d\nError Message: %s\n\n",
   fileName, lineNum, msg );
  printf( "Please contact the author of this software or else send \n" );
  printf( "the text of this message to tech-talk@aps.anl.gov\n" );
  printf( "==============================================================\n" );

}

static void extractName(
  char *fileName, 
  char *name ) {

int i, l;
char *gotOne;

  gotOne = strstr( fileName, "/" );
  if ( !gotOne ) {

    strncpy( name, fileName, 255 );

    // remove extension
    l = strlen( name );

    for ( i=0; i<l; i++ ) {
      if ( name[i] == '.' ) {
        name[i] = 0;
        return;
      }
    }

    return;

  }

  // remove directory
  l = strlen( fileName );

  for ( i=l-1; i>0; i-- ) {

    if ( fileName[i] == '/' ) {

      strncpy( name, &fileName[i+1], 255 );
      break;

    }

  }

  // remove extension
  l = strlen( name );

  for ( i=0; i<l; i++ ) {
    if ( name[i] == '.' ) {
      name[i] = 0;
      return;
    }
  }

}

// for quick sort routines
static int qsort_compare_x_func (
  void const *node1,
  void const *node2
) {

activeGraphicListPtr p1, p2;

  p1 = (activeGraphicListPtr) node1;
  p2 = (activeGraphicListPtr) node2;

  if ( p1->node->getX0() > p2->node->getX0() )
    return 1;
  else if ( p1->node->getX0() < p2->node->getX0() )
    return -1;
  else
    return 0;

}

static int qsort_compare_y_func (
  void const *node1,
  void const *node2
) {

activeGraphicListPtr p1, p2;

  p1 = (activeGraphicListPtr) node1;
  p2 = (activeGraphicListPtr) node2;

  if ( p1->node->getY0() > p2->node->getY0() )
    return 1;
  else if ( p1->node->getY0() < p2->node->getY0() )
    return -1;
  else
    return 0;

}

static void acw_restoreTitle (
  XtPointer client,
  XtIntervalId *id )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->restoreTimer = 0;
  strcpy( awo->title, awo->restoreTitle );
  awo->expStrTitle.setRaw( awo->title );
  awo->setTitle();
  XFlush( awo->d );

}

static jmp_buf g_jump_h;

void signal_handler (
  int sig
) {

  //printf( "Got signal: sig = %-d\n", sig );
  longjmp( g_jump_h, 1 );

}

static void acw_autosave (
  XtPointer client,
  XtIntervalId *id )
{

activeWindowClass *awo = (activeWindowClass *) client;
//int stat;
//char name[255+1], oldName[255+1];
//char str[31+1];

//struct sigaction sa, oldsa, dummysa;

  awo->autosaveTimer = 0;

#if 0
  stat = setjmp( g_jump_h );
  if ( !stat ) {

    sa.sa_handler = signal_handler;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = 0;

    stat = sigaction( SIGILL, &sa, &oldsa );
    stat = sigaction( SIGSEGV, &sa, &dummysa );

  }
  else {

    printf( "Auto-save failed - received exception\n" );
    return;

  }
#endif

  //stat = sys_get_datetime_string( 31, str );
  //printf( "[%s] %s - autosave\n", str, awo->fileName );

  if ( strcmp( awo->startSignature, "edmActiveWindow" ) != 0 ) {
    printf( "Auto-save failed - bad initial signature\n" );
    return;
  }

  if ( strcmp( awo->endSignature, "wodniWevitcAmde" ) != 0 ) {
    printf( "Auto-save failed - bad ending signature\n" );
    return;
  }

  awo->doAutoSave = 1;
  awo->appCtx->postDeferredExecutionQueue( awo );

  return;

#if 0
  if ( !awo->changeSinceAutoSave ) return;

  awo->changeSinceAutoSave = 0;

  strncpy( oldName, awo->autosaveName, 255 );
  oldName[255] = 0;

  strncpy( awo->autosaveName, activeWindowClass_str1, 255 );

  if ( strcmp( awo->fileName, "" ) != 0 ) {
    extractName( awo->fileName, name );
    strncat( awo->autosaveName, "_", 255 );
    strncat( awo->autosaveName, name, 255 );
  }

  strncat( awo->autosaveName, "_XXXXXX", 255 );

  mkstemp( awo->autosaveName );

  awo->saveNoChange( awo->autosaveName );

  if ( awo->mode != AWC_EXECUTE ) {

    strcpy( awo->restoreTitle, awo->title );
    strcpy( awo->title, activeWindowClass_str2 );
    awo->expStrTitle.setRaw( awo->title );
    awo->setTitleUsingTitle();
    XFlush( awo->d );

    if ( awo->restoreTimer ) {
      XtRemoveTimeOut( awo->restoreTimer );
      awo->restoreTimer = 0;
    }
    awo->restoreTimer = XtAppAddTimeOut( awo->appCtx->appContext(),
     3000, acw_restoreTitle, awo );

  }

  // now delete previous autosave file
  if ( strcmp( oldName, "" ) != 0 ) {
    stat = unlink( oldName );
  }

#endif

}

static void awc_dont_save_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->confirm1.popdown();
  awo->state = awo->savedState;

}

static void awc_do_save_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->confirm1.popdown();
  awo->save( awo->fileName );
  awo->state = awo->savedState;

}

static void awc_do_save_and_exit_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->confirm1.popdown();
  awo->save( awo->fileName );
  awo->state = awo->savedState;

  if ( awo->autosaveTimer ) {
    XtRemoveTimeOut( awo->autosaveTimer );
    awo->autosaveTimer = 0;
  }
  if ( awo->restoreTimer ) {
    XtRemoveTimeOut( awo->restoreTimer );
    awo->restoreTimer = 0;
  }

  //mark active window for delege
  awo->appCtx->removeActiveWindow( awo );

  XtUnmanageChild( awo->drawWidget );

}

static void awc_continue_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->confirm.popdown();
  awo->state = awo->savedState;

}

static void awc_save_and_exit_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;
int n;
Arg args[10];
XmString xmStr1, xmStr2;
Atom wm_delete_window;

  awo->confirm.popdown();

  if ( strcmp( awo->fileName, "" ) == 0 ) { // do save as ...

    awo->exit_after_save = 1;

    xmStr1 = XmStringCreateLocalized( "*.edl" );
    xmStr2 = NULL;

    n = 0;
    XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

    if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
      xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
      XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
    }

    awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top, "", args,
     n );

    XmStringFree( xmStr1 );
    if ( xmStr2 ) XmStringFree( xmStr2 );

    XtAddCallback( awo->fileSelectBox, XmNcancelCallback,
     awc_saveFileSelectCancel_cb, (void *) awo );
    XtAddCallback( awo->fileSelectBox, XmNokCallback,
     awc_saveFileSelectOk_cb, (void *) awo );

    // -----------------------------------------------------

    awo->wpFileSelect.w = awo->fileSelectBox;
    awo->wpFileSelect.client = (void *) awo;

    wm_delete_window = XmInternAtom( XtDisplay(awo->top),
     "WM_DELETE_WINDOW", False );

    XmAddWMProtocolCallback( XtParent(awo->fileSelectBox),
     wm_delete_window, awc_saveFileSelectKill_cb, &awo->wpFileSelect );

    XtVaSetValues( XtParent(awo->fileSelectBox), XmNdeleteResponse,
     XmDO_NOTHING, NULL );

    // -----------------------------------------------------

    XtManageChild( awo->fileSelectBox );

    XSetWindowColormap( awo->d,
     XtWindow(XtParent(awo->fileSelectBox)),
     awo->appCtx->ci.getColorMap() );

  }
  else {

    awo->save( awo->fileName );

    awo->state = awo->savedState;

    if ( awo->autosaveTimer ) {
      XtRemoveTimeOut( awo->autosaveTimer );
      awo->autosaveTimer = 0;
    }
    if ( awo->restoreTimer ) {
      XtRemoveTimeOut( awo->restoreTimer );
      awo->restoreTimer = 0;
    }

    //mark active window for delege
    awo->appCtx->removeActiveWindow( awo );

    XtUnmanageChild( awo->drawWidget );

  }

}

static void awc_abort_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->confirm.popdown();

  awo->state = awo->savedState;

  if ( awo->autosaveTimer ) {
    XtRemoveTimeOut( awo->autosaveTimer );
    awo->autosaveTimer = 0;
  }
  if ( awo->restoreTimer ) {
    XtRemoveTimeOut( awo->restoreTimer );
    awo->restoreTimer = 0;
  }

  //mark active window for delege
  awo->appCtx->removeActiveWindow( awo );

  XtUnmanageChild( awo->drawWidget );

}

static void awc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;
int n;
Arg args[3];

  strncpy( awo->defaultFontTag, awo->defaultFm.currentFontTag(), 127 );
  strncpy( awo->defaultCtlFontTag, awo->defaultCtlFm.currentFontTag(),
   127 );
  strncpy( awo->defaultBtnFontTag, awo->defaultBtnFm.currentFontTag(),
   127 );

  awo->defaultAlignment = awo->defaultFm.currentFontAlignment();
  awo->defaultCtlAlignment = awo->defaultCtlFm.currentFontAlignment();
  awo->defaultBtnAlignment = awo->defaultBtnFm.currentFontAlignment();

  awo->fgColor = awo->bufFgColor;
  awo->bgColor = awo->bufBgColor;
  awo->defaultTextFgColor = awo->bufDefaultTextFgColor;
  awo->defaultFg1Color = awo->bufDefaultFg1Color;
  awo->defaultFg2Color = awo->bufDefaultFg2Color;
  awo->defaultBgColor = awo->bufDefaultBgColor;
  awo->defaultTopShadowColor = awo->bufDefaultTopShadowColor;
  awo->defaultBotShadowColor = awo->bufDefaultBotShadowColor;
  awo->defaultOffsetColor = awo->bufDefaultOffsetColor;

  awo->drawGc.setBaseBG( awo->ci->pix(awo->bgColor) );

  awo->cursor.setColor( awo->ci->pix(awo->fgColor),
   awo->ci->pix(awo->bgColor) );

  strncpy( awo->id, awo->bufId, 31 );

  awo->x = awo->bufX;
  awo->y = awo->bufY;
  awo->w = awo->bufW;
  awo->h = awo->bufH;

  strncpy( awo->title, awo->bufTitle, 127 );
  awo->expStrTitle.setRaw( awo->title );

  strncpy( awo->defaultPvType, awo->bufDefaultPvType, 15 );

  awo->gridSpacing = awo->bufGridSpacing;

   n = 0;
   XtSetArg( args[n], XmNx, (XtArgVal) awo->x ); n++;
   XtSetValues( awo->drawWidget, args, n );

   n = 0;
   XtSetArg( args[n], XmNy, (XtArgVal) awo->y ); n++;
   XtSetValues( awo->drawWidget, args, n );

   n = 0;
   XtSetArg( args[n], XmNwidth, (XtArgVal) awo->w ); n++;
   XtSetValues( awo->top, args, n );

   n = 0;
   XtSetArg( args[n], XmNheight, (XtArgVal) awo->h ); n++;
   XtSetValues( awo->top, args, n );


  strncpy( awo->gridActiveStr, awo->bufGridActiveStr, 7 );
  strncpy( awo->gridShowStr, awo->bufGridShowStr, 7 );

  if ( strcmp( awo->gridActiveStr, activeWindowClass_str3 ) == 0 )
    awo->gridActive = 1;
  else
    awo->gridActive = 0;

  if ( strcmp( awo->gridShowStr, activeWindowClass_str3 ) == 0 )
    awo->gridShow = 1;
  else
    awo->gridShow = 0;

  if ( awo->oldGridSpacing != awo->gridSpacing ) {

    awo->oldGridSpacing = awo->gridSpacing;

  }

  awo->orthoMove = awo->bufOrthoMove;

  awo->orthogonal = awo->bufOrthogonal;

  awo->activateCallbackFlag = awo->bufActivateCallbackFlag;
  awo->deactivateCallbackFlag = awo->bufDeactivateCallbackFlag;

  awo->clear();
  awo->refresh();

  awo->setChanged();

}

static void awc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awc_edit_apply( w, client, call );
  awo->ef.popdown();
  awo->operationComplete();

}

static void awc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->refresh();
  awo->ef.popdown();
  awo->operationComplete();

}

static void awc_change_dsp_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;
activeGraphicListPtr curSel;
unsigned int flag = 0;
char strObjType[31+1], schemeFile[255+1];
int stat, success;

  if ( awo->allSelectedFontTagFlag ) flag |= ACTGRF_FONTTAG_MASK;
  if ( awo->allSelectedAlignmentFlag ) flag |= ACTGRF_ALIGNMENT_MASK;
  if ( awo->allSelectedCtlFontTagFlag ) flag |= ACTGRF_CTLFONTTAG_MASK;
  if ( awo->allSelectedCtlAlignmentFlag ) flag |= ACTGRF_CTLALIGNMENT_MASK;
  if ( awo->allSelectedBtnFontTagFlag ) flag |= ACTGRF_BTNFONTTAG_MASK;
  if ( awo->allSelectedBtnAlignmentFlag ) flag |= ACTGRF_BTNALIGNMENT_MASK;
  if ( awo->allSelectedTextFgColorFlag ) flag |= ACTGRF_TEXTFGCOLOR_MASK;
  if ( awo->allSelectedFg1ColorFlag ) flag |= ACTGRF_FG1COLOR_MASK;
  if ( awo->allSelectedFg2ColorFlag ) flag |= ACTGRF_FG2COLOR_MASK;
  if ( awo->allSelectedOffsetColorFlag ) flag |= ACTGRF_OFFSETCOLOR_MASK;
  if ( awo->allSelectedBgColorFlag ) flag |= ACTGRF_BGCOLOR_MASK;
  if ( awo->allSelectedTopShadowColorFlag ) flag |= ACTGRF_TOPSHADOWCOLOR_MASK;
  if ( awo->allSelectedBotShadowColorFlag ) flag |= ACTGRF_BOTSHADOWCOLOR_MASK;

  strncpy( awo->allSelectedFontTag, awo->defaultFm.currentFontTag(), 63 );
  strncpy( awo->allSelectedCtlFontTag, awo->defaultCtlFm.currentFontTag(),
   63 );
  strncpy( awo->allSelectedBtnFontTag, awo->defaultBtnFm.currentFontTag(),
   63 );

  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    if ( awo->useComponentScheme ) {

      curSel->node->getObjType( 31, strObjType );

      if ( strcmp( strObjType, "" ) == 0 ) { // don't have type, try all

        success = 0;

        if ( !success ) { // try Controls
          if ( awo->appCtx->schemeExists ( awo->curSchemeSet,
           curSel->node->objName(), global_str5 ) ) {
            awo->appCtx->getScheme( awo->curSchemeSet, curSel->node->objName(),
             global_str5, schemeFile, 255 );
            if ( strcmp( schemeFile, "" ) != 0 ) {
              success = awo->loadComponentScheme( schemeFile );
            }
	  }
	}

        if ( !success ) { // try Monitors
          if ( awo->appCtx->schemeExists ( awo->curSchemeSet,
           curSel->node->objName(), global_str2 ) ) {
            awo->appCtx->getScheme( awo->curSchemeSet, curSel->node->objName(),
             global_str2, schemeFile, 255 );
            if ( strcmp( schemeFile, "" ) != 0 ) {
              success = awo->loadComponentScheme( schemeFile );
	    }
	  }
	}

        if ( !success ) { // try Graphics
          if ( awo->appCtx->schemeExists ( awo->curSchemeSet,
           curSel->node->objName(), global_str3 ) ) {
            awo->appCtx->getScheme( awo->curSchemeSet, curSel->node->objName(),
             global_str3, schemeFile, 255 );
            if ( strcmp( schemeFile, "" ) != 0 ) {
              success = awo->loadComponentScheme( schemeFile );
	    }
	  }
	}

        if ( !success ) { // reload default
          success = awo->loadComponentScheme( "default" );
	}

      }
      else {

        awo->appCtx->getScheme( awo->curSchemeSet, curSel->node->objName(),
         strObjType, schemeFile, 255 );
        if ( strcmp( schemeFile, "" ) != 0 ) {
          stat = awo->loadComponentScheme( schemeFile );
          if ( !( stat & 1 ) ) {
            awo->loadComponentScheme( "default" );
          }
        }

      }

    }

    curSel->node->changeDisplayParams(
     flag,
     awo->allSelectedFontTag,
     awo->allSelectedAlignment,
     awo->allSelectedCtlFontTag,
     awo->allSelectedCtlAlignment,
     awo->allSelectedBtnFontTag,
     awo->allSelectedBtnAlignment,
     awo->allSelectedTextFgColor,
     awo->allSelectedFg1Color,
     awo->allSelectedFg2Color,
     awo->allSelectedOffsetColor,
     awo->allSelectedBgColor,
     awo->allSelectedTopShadowColor,
     awo->allSelectedBotShadowColor );

    curSel = curSel->selFlink;

  }

  curSel = awo->selectedHead->selFlink;
  if ( curSel ) curSel->node->drawAll();

  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    curSel->node->drawSelectBoxCorners();

    curSel = curSel->selFlink;

  }

  awo->clear();
  awo->refresh();

  awo->setChanged();

}

static void awc_change_dsp_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awc_change_dsp_edit_apply( w, client, call );
  awo->ef.popdown();
  awo->operationComplete();

}

static void awc_change_dsp_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->refresh();
  awo->ef.popdown();
  awo->operationComplete();

}

static void awc_change_pv_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;
activeGraphicListPtr curSel;
unsigned int flag = 0;
char *pCtl[1], *pReadback[1], *pNull[1], *pVis[1], *pAlarm[1];

  if ( awo->allSelectedCtlPvNameFlag ) flag |= ACTGRF_CTLPVS_MASK;
  if ( awo->allSelectedReadbackPvNameFlag ) flag |= ACTGRF_READBACKPVS_MASK;
  if ( awo->allSelectedNullPvNameFlag ) flag |= ACTGRF_NULLPVS_MASK;
  if ( awo->allSelectedVisPvNameFlag ) flag |= ACTGRF_VISPVS_MASK;
  if ( awo->allSelectedAlarmPvNameFlag ) flag |= ACTGRF_ALARMPVS_MASK;

  pCtl[0] = awo->allSelectedCtlPvName[0];
  pReadback[0] = awo->allSelectedReadbackPvName[0];
  pNull[0] = awo->allSelectedNullPvName[0];
  pVis[0] = awo->allSelectedVisPvName[0];
  pAlarm[0] = awo->allSelectedAlarmPvName[0];

  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    curSel->node->changePvNames(
     flag,
     1, pCtl,
     1, pReadback,
     1, pNull,
     1, pVis,
     1, pAlarm );

    curSel = curSel->selFlink;

  }

  curSel = awo->selectedHead->selFlink;
  if ( curSel ) curSel->node->drawAll();

  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    curSel->node->drawSelectBoxCorners();

    curSel = curSel->selFlink;

  }

  awo->clear();
  awo->refresh();

  awo->setChanged();

}

static void awc_change_pv_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awc_change_pv_edit_apply( w, client, call );
  awo->ef.popdown();
  awo->operationComplete();

}

static void awc_change_pv_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->refresh();
  awo->ef.popdown();
  awo->operationComplete();

}

static void awc_fileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

char *fName;
activeWindowClass *awo = (activeWindowClass *) client;
XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;
activeWindowListPtr cur;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    goto done;
  }

  cur = new activeWindowListType;
  awo->appCtx->addActiveWindow( cur );

  cur->node.create( awo->appCtx, NULL, 0, 0, 0, 0, awo->appCtx->numMacros,
   awo->appCtx->macros, awo->appCtx->expansions );
  cur->node.realize();
  cur->node.setGraphicEnvironment( &awo->appCtx->ci, &awo->appCtx->fi );

  cur->node.storeFileName( fName );

  XtFree( fName );

  if ( awo->appCtx->executeOnOpen || ( awo->mode == AWC_EXECUTE ) ) {
    awo->appCtx->openActivateActiveWindow( &cur->node );
  }
  else {
    awo->appCtx->openEditActiveWindow( &cur->node );
  }

done:

  XtRemoveCallback( w, XmNcancelCallback,
   awc_fileSelectCancel_cb, (void *) awo );
  XtRemoveCallback( w, XmNokCallback,
   awc_fileSelectOk_cb, (void *) awo );

  awo->operationComplete();

  XtUnmanageChild( w ); // it's ok to unmanage a child again
  XtDestroyWidget( w );

}

static void awc_fileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  XtRemoveCallback( w, XmNcancelCallback,
   awc_fileSelectCancel_cb, (void *) awo );
  XtRemoveCallback( w, XmNokCallback,
   awc_fileSelectOk_cb, (void *) awo );

  awo->operationComplete();

  XtUnmanageChild( w );
  XtDestroyWidget( w );

}

static void awc_fileSelectKill_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

widgetAndPointerPtr wp = (widgetAndPointerPtr) client;

  awc_fileSelectCancel_cb( wp->w, wp->client, NULL );

}

static void awc_saveFileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

char *fName;
activeWindowClass *awo = (activeWindowClass *) client;
XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;

Window root, child;
int rootX, rootY, winX, winY;
unsigned int mask;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    strcpy( awo->fileName, "" );
    awo->operationComplete();
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    strcpy( awo->fileName, "" );
    awo->operationComplete();
    goto done;
  }

  strncpy( awo->fileName, fName, sizeof(awo->fileName)-1 );
  XtFree( fName );
  XtUnmanageChild( w );

  awo->setTitle();

  if ( awo->edlFileExists( awo->fileName ) ) {

    XQueryPointer( awo->d, XtWindow(awo->top), &root, &child,
     &rootX, &rootY, &winX, &winY, &mask );

    awo->confirm1.create( awo->top, awo->b2PressXRoot, awo->b2PressYRoot, 2,
     activeWindowClass_str4, NULL, NULL );

    awo->confirm1.addButton( activeWindowClass_str5, awc_dont_save_cb,
     (void *) awo );

    if ( awo->exit_after_save ) {
      awo->confirm1.addButton( activeWindowClass_str3, awc_do_save_and_exit_cb,
       (void *) awo );
    }
    else {
      awo->confirm1.addButton( activeWindowClass_str3, awc_do_save_cb,
       (void *) awo );
    }

    awo->confirm1.finished();
    awo->confirm1.popup();

    XSetWindowColormap( awo->d, XtWindow(awo->confirm1.top()),
     awo->appCtx->ci.getColorMap() );

  }
  else {

    awo->save( awo->fileName );
    awo->operationComplete();

    if ( awo->exit_after_save ) {

      if ( awo->autosaveTimer ) {
        XtRemoveTimeOut( awo->autosaveTimer );
        awo->autosaveTimer = 0;
      }
      if ( awo->restoreTimer ) {
        XtRemoveTimeOut( awo->restoreTimer );
        awo->restoreTimer = 0;
      }

      //mark active window for delege
      awo->appCtx->removeActiveWindow( awo );

      XtUnmanageChild( awo->drawWidget );

    }

  }

done:

  awo->exit_after_save = 0;

  XtRemoveCallback( w, XmNcancelCallback,
   awc_saveFileSelectCancel_cb, (void *) awo );
  XtRemoveCallback( w, XmNokCallback,
   awc_saveFileSelectOk_cb, (void *) awo );

  XtUnmanageChild( w );
  XtDestroyWidget( w );

}

static void awc_saveFileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  XtRemoveCallback( w, XmNcancelCallback,
   awc_saveFileSelectCancel_cb, (void *) awo );
  XtRemoveCallback( w, XmNokCallback,
   awc_saveFileSelectOk_cb, (void *) awo );

  awo->operationComplete();

  XtUnmanageChild( w );
  XtDestroyWidget( w );

}

static void awc_saveFileSelectKill_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

widgetAndPointerPtr wp = (widgetAndPointerPtr) client;

  awc_saveFileSelectCancel_cb( wp->w, wp->client, NULL );

}

static void awc_loadSchemeSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

char *fName;
activeWindowClass *awo = (activeWindowClass *) client;
XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;

char fileName[127+1];

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    strcpy( awo->fileName, "" );
    awo->operationComplete();
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    awo->operationComplete();
    goto done;
  }

  strncpy( fileName, fName, 127 );
  XtFree( fName );
  XtUnmanageChild( w );

  awo->loadScheme( fileName );
  awo->operationComplete();

done:

  XtRemoveCallback( w, XmNcancelCallback,
   awc_loadSchemeSelectCancel_cb, (void *) awo );
  XtRemoveCallback( w, XmNokCallback,
   awc_loadSchemeSelectOk_cb, (void *) awo );

  XtUnmanageChild( w );
  XtDestroyWidget( w );

}

static void awc_loadSchemeSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  XtRemoveCallback( w, XmNcancelCallback,
   awc_loadSchemeSelectCancel_cb, (void *) awo );
  XtRemoveCallback( w, XmNokCallback,
   awc_loadSchemeSelectOk_cb, (void *) awo );

  awo->operationComplete();

  XtUnmanageChild( w );
  XtDestroyWidget( w );

}

static void awc_loadSchemeSelectKill_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

widgetAndPointerPtr wp = (widgetAndPointerPtr) client;

  awc_loadSchemeSelectCancel_cb( wp->w, wp->client, NULL );

}

static void awc_saveSchemeSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

char *fName;
activeWindowClass *awo = (activeWindowClass *) client;
XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;

char fileName[127+1];

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    strcpy( awo->fileName, "" );
    awo->operationComplete();
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    awo->operationComplete();
    goto done;
  }

  strncpy( fileName, fName, 127 );
  XtFree( fName );
  XtUnmanageChild( w );

  awo->saveScheme( fileName );
  awo->operationComplete();

done:

  XtRemoveCallback( w, XmNcancelCallback,
   awc_saveSchemeSelectCancel_cb, (void *) awo );
  XtRemoveCallback( w, XmNokCallback,
   awc_saveSchemeSelectOk_cb, (void *) awo );

  XtUnmanageChild( w );
  XtDestroyWidget( w );

}

static void awc_saveSchemeSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  XtRemoveCallback( w, XmNcancelCallback,
   awc_saveSchemeSelectCancel_cb, (void *) awo );
  XtRemoveCallback( w, XmNokCallback,
   awc_saveSchemeSelectOk_cb, (void *) awo );

  awo->operationComplete();

  XtUnmanageChild( w );
  XtDestroyWidget( w );

}

static void awc_saveSchemeSelectKill_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

widgetAndPointerPtr wp = (widgetAndPointerPtr) client;

  awc_saveSchemeSelectCancel_cb( wp->w, wp->client, NULL );

}

static void awc_WMExit_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{
activeWindowClass *awo = (activeWindowClass *) client;

  awo->doClose = 1;
  awo->appCtx->postDeferredExecutionQueue( awo );

}

static void selectScheme_cb (
   Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo;
popupBlockPtr block;
char *item;

  block = (popupBlockPtr) client;
  item = (char *) block->ptr;
  awo = (activeWindowClass *) block->awo;

  if ( item ) {
    //printf( "selectScheme_cb, item = [%s]\n", item );
    strncpy( awo->curSchemeSet, item, 63 );
  }
  else {
    strncpy( awo->curSchemeSet, "", 63 );
    awo->appCtx->displayScheme.loadDefault( &awo->appCtx->ci );
    awo->setDisplayScheme( &awo->appCtx->displayScheme );
  }

}

static void b1ReleaseOneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo;
popupBlockPtr block;
int item;

  block = (popupBlockPtr) client;
  item = (int) block->ptr;
  awo = (activeWindowClass *) block->awo;

  switch ( item ) {

    case AWC_POPUP_EDIT_LINE_PROP:
      awo->state = AWC_EDITING;
      awo->currentEf = NULL;
      awo->cursor.set( XtWindow(awo->drawWidget), CURSOR_K_WAIT );
      awo->cursor.setColor( awo->ci->pix(awo->fgColor),
       awo->ci->pix(awo->bgColor) );
      awo->currentPointObject->setEditProperties();
      awo->currentPointObject->doEdit();
      break;

    case AWC_POPUP_EDIT_LINE_SEG:
      awo->state = awo->savedState;
      awo->currentPointObject->setEditSegments();
      awo->currentPointObject->doEdit();
      break;

  }

}

static void b2ReleaseExecute_cb (
   Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo;
popupBlockPtr block;
int item;
int n;
Arg args[10];
XmString xmStr1, xmStr2;
Atom wm_delete_window;

  block = (popupBlockPtr) client;
  item = (int) block->ptr;
  awo = (activeWindowClass *) block->awo;

  switch ( item ) {

    case AWC_POPUP_FINDTOP:

      awo->appCtx->findTop();  // raise edm main window
      break;

    case AWC_POPUP_REFRESH:

      awo->clearActive();
      awo->refreshActive();

      break;

    case AWC_POPUP_EDIT:

      awo->returnToEdit( 0 );
      break;

    case AWC_POPUP_TOGGLE_TITLE:

      if ( awo->showName )
        awo->showName = 0;
      else
        awo->showName = 1;
      awo->setTitle();
      break;

    case AWC_POPUP_OPEN:

      awo->savedState = awo->state;
      awo->state = AWC_WAITING;

      xmStr1 = XmStringCreateLocalized( "*.edl" );
      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top, "", args,
       n );

      XmStringFree( xmStr1 );
      if ( xmStr2 ) XmStringFree( xmStr2 );

      XtAddCallback( awo->fileSelectBox, XmNcancelCallback,
       awc_fileSelectCancel_cb, (void *) awo );
      XtAddCallback( awo->fileSelectBox, XmNokCallback,
       awc_fileSelectOk_cb, (void *) awo );

      // -----------------------------------------------------

      awo->wpFileSelect.w = awo->fileSelectBox;
      awo->wpFileSelect.client = (void *) awo;

      wm_delete_window = XmInternAtom( XtDisplay(awo->top),
       "WM_DELETE_WINDOW", False );

      XmAddWMProtocolCallback( XtParent(awo->fileSelectBox),
       wm_delete_window, awc_fileSelectKill_cb, &awo->wpFileSelect );

      XtVaSetValues( XtParent(awo->fileSelectBox), XmNdeleteResponse,
       XmDO_NOTHING, NULL );

      // -----------------------------------------------------

      XtManageChild( awo->fileSelectBox );

      XSetWindowColormap( awo->d,
       XtWindow(XtParent(awo->fileSelectBox)),
       awo->appCtx->ci.getColorMap() );

      break;

    case AWC_POPUP_OPEN_USER:

      awo->savedState = awo->state;
      awo->state = AWC_WAITING;

      xmStr1 = XmStringCreateLocalized( "*.edl" );
      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top, "", args,
       n );

      XmStringFree( xmStr1 );
      if ( xmStr2 ) XmStringFree( xmStr2 );

      XtAddCallback( awo->fileSelectBox, XmNcancelCallback,
       awc_fileSelectCancel_cb, (void *) awo );
      XtAddCallback( awo->fileSelectBox, XmNokCallback,
       awc_fileSelectOk_cb, (void *) awo );

      // -----------------------------------------------------

      awo->wpFileSelect.w = awo->fileSelectBox;
      awo->wpFileSelect.client = (void *) awo;

      wm_delete_window = XmInternAtom( XtDisplay(awo->top),
       "WM_DELETE_WINDOW", False );

      XmAddWMProtocolCallback( XtParent(awo->fileSelectBox),
       wm_delete_window, awc_fileSelectKill_cb, &awo->wpFileSelect );

      XtVaSetValues( XtParent(awo->fileSelectBox), XmNdeleteResponse,
       XmDO_NOTHING, NULL );

      // -----------------------------------------------------

      XtManageChild( awo->fileSelectBox );

      XSetWindowColormap( awo->d,
       XtWindow(XtParent(awo->fileSelectBox)),
       awo->appCtx->ci.getColorMap() );

      break;

    case AWC_POPUP_CLOSE:

      awo->returnToEdit( 1 );
      break;

    case AWC_POPUP_LOWER:

      XLowerWindow( awo->d, XtWindow(awo->top) );
      break;

  }

}

static int isContained(
  activeGraphicClass *node1,
  activeGraphicClass *node2 )
{

// return 1 if node1 is contained in node2

  if ( node1->getX0() < node2->getX0() ) return 0;

  if ( node1->getX1() > node2->getX1() ) return 0;

  if ( node1->getY0() < node2->getY0() ) return 0;

  if ( node1->getY1() > node2->getY1() ) return 0;

  return 1;

}

static void raise (
  activeWindowClass *awo )
{

activeGraphicListPtr curSel;

  curSel = awo->selectedHead->selFlink;

  if ( curSel == awo->selectedHead ) { // none selected
    return;
  }

  awo->setChanged();

  while ( curSel != awo->selectedHead ) {

    // remove
    curSel->blink->flink = curSel->flink;
    curSel->flink->blink = curSel->blink;

    // insert at tail
    curSel->blink = awo->head->blink;
    awo->head->blink->flink = curSel;
    awo->head->blink = curSel;
    curSel->flink = awo->head;

    curSel->node->eraseSelectBoxCorners();

    curSel = curSel->selFlink;

  }

  curSel = awo->selectedHead->selFlink;
  if ( curSel ) curSel->node->drawAll();

  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    curSel->node->drawSelectBoxCorners();

    curSel = curSel->selFlink;

  }

}

static void lower (
  activeWindowClass *awo )
{

activeGraphicListPtr curSel;

  // remove node and insert at head
  curSel = awo->selectedHead->selFlink;

  if ( curSel == awo->selectedHead ) { // none selected
    return;
  }

  awo->setChanged();

  while ( curSel != awo->selectedHead ) {

    // remove
    curSel->blink->flink = curSel->flink;
    curSel->flink->blink = curSel->blink;

    // insert at tail
    curSel->flink = awo->head->flink;
    awo->head->flink->blink = curSel;
    awo->head->flink = curSel;
    curSel->blink = awo->head;

    curSel->node->eraseSelectBoxCorners();

    curSel = curSel->selFlink;

  }

  curSel = awo->selectedHead->selFlink;
  if ( curSel ) curSel->node->drawAll();

  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    curSel->node->drawSelectBoxCorners();

    curSel = curSel->selFlink;

  }

}

static void rotate (
  activeWindowClass *awo,
  char direction )
{

int stat, minX, maxX, minY, maxY, xOrigin, yOrigin;
activeGraphicListPtr curSel;

  curSel = awo->selectedHead->selFlink;
  if ( curSel == awo->selectedHead ) return;

  awo->undoObj.startNewUndoList( activeWindowClass_str181 );
  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {
    stat = curSel->node->addUndoRotateNode( &(awo->undoObj) );
    curSel = curSel->selFlink;
  }

  awo->setChanged();

  // find origin
  curSel = awo->selectedHead->selFlink;
  minX = curSel->node->getX0();
  maxX = curSel->node->getX1();
  minY = curSel->node->getY0();
  maxY = curSel->node->getY1();
  while ( curSel != awo->selectedHead ) {

    if ( curSel->node->getX0() < minX )
      minX = curSel->node->getX0();

    if ( curSel->node->getX1() > maxX )
      maxX = curSel->node->getX1();

    if ( curSel->node->getY0() < minY )
      minY = curSel->node->getY0();

    if ( curSel->node->getY1() > maxY )
      maxY = curSel->node->getY1();

    curSel = curSel->selFlink;

  }

  xOrigin = (int) ( ( (double) minX + (double) maxX + 0.5 ) / 2.0 );
  yOrigin = (int) ( ( (double) minY + (double) maxY + 0.5 ) / 2.0 );

  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    curSel->node->eraseSelectBoxCorners();
    curSel->node->erase();

    curSel->node->rotate( xOrigin, yOrigin, direction );
    curSel->node->updateDimensions();

    curSel->node->resizeSelectBoxAbsFromUndo( curSel->node->getX0(),
     curSel->node->getY0(), curSel->node->getW(),
     curSel->node->getH() );

    curSel = curSel->selFlink;

  }

  awo->refresh();

}

static void flip (
  activeWindowClass *awo,
  char direction )
{

int stat, minX, maxX, minY, maxY, xOrigin, yOrigin;
activeGraphicListPtr curSel;

  curSel = awo->selectedHead->selFlink;
  if ( curSel == awo->selectedHead ) return;

  awo->undoObj.startNewUndoList( activeWindowClass_str182 );
  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {
    stat = curSel->node->addUndoFlipNode( &(awo->undoObj ));
    curSel = curSel->selFlink;
  }

  awo->setChanged();

  // find origin
  curSel = awo->selectedHead->selFlink;
  minX = curSel->node->getX0();
  maxX = curSel->node->getX1();
  minY = curSel->node->getY0();
  maxY = curSel->node->getY1();
  while ( curSel != awo->selectedHead ) {

    if ( curSel->node->getX0() < minX )
      minX = curSel->node->getX0();

    if ( curSel->node->getX1() > maxX )
      maxX = curSel->node->getX1();

    if ( curSel->node->getY0() < minY )
      minY = curSel->node->getY0();

    if ( curSel->node->getY1() > maxY )
      maxY = curSel->node->getY1();

    curSel = curSel->selFlink;

  }

  xOrigin = (int) ( ( (double) minX + (double) maxX + 0.5 ) / 2.0 );
  yOrigin = (int) ( ( (double) minY + (double) maxY + 0.5 ) / 2.0 );

  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    curSel->node->eraseSelectBoxCorners();
    curSel->node->erase();

    curSel->node->flip( xOrigin, yOrigin, direction );
    curSel->node->updateDimensions();

    curSel->node->resizeSelectBoxAbsFromUndo( curSel->node->getX0(),
     curSel->node->getY0(), curSel->node->getW(),
     curSel->node->getH() );

    curSel = curSel->selFlink;

  }

  awo->refresh();

}

static void cut (
  activeWindowClass *awo )
{

activeGraphicListPtr curCut, nextCut, curSel;

  // remove nodes and put on cut list; if cur list is not
  // empty the delete all nodes

  curSel = awo->selectedHead->selFlink;
  if ( curSel == awo->selectedHead ) return;

  awo->undoObj.flush();

  awo->setChanged();

  // empty cut list
  curCut = awo->appCtx->cutHead1->flink;
  while ( curCut != awo->appCtx->cutHead1 ) {
    nextCut = curCut->flink;
    delete curCut->node;
    delete curCut;
    curCut = nextCut;
  }

  awo->appCtx->cutHead1->flink = awo->appCtx->cutHead1;
  awo->appCtx->cutHead1->blink = awo->appCtx->cutHead1;

  // remove nodes off select list
  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    curSel->node->eraseSelectBoxCorners();
    curSel->node->erase();

    // deselect
    curSel->node->deselect();

    // remove
    curSel->blink->flink = curSel->flink;
    curSel->flink->blink = curSel->blink;

    // insert into cut list
    curSel->blink = awo->appCtx->cutHead1->blink;
    awo->appCtx->cutHead1->blink->flink = curSel;
    awo->appCtx->cutHead1->blink = curSel;
    curSel->flink = awo->appCtx->cutHead1;

    curSel = curSel->selFlink;

  }

  // make selected list empty
  awo->selectedHead->selFlink = awo->selectedHead;
  awo->selectedHead->selBlink = awo->selectedHead;

  awo->state = AWC_NONE_SELECTED;
  awo->updateMasterSelection();

}

static void copy (
  activeWindowClass *awo )
{

activeGraphicListPtr curCut, nextCut, curSel, newOne;

  // copy nodes to cut list; if cur cut list is not
  // empty the delete all nodes

  curSel = awo->selectedHead->selFlink;
  if ( curSel == awo->selectedHead ) return;

  // empty cut list
  curCut = awo->appCtx->cutHead1->flink;
  while ( curCut != awo->appCtx->cutHead1 ) {
    nextCut = curCut->flink;
    delete curCut->node;
    delete curCut;
    curCut = nextCut;
  }

  awo->appCtx->cutHead1->flink = awo->appCtx->cutHead1;
  awo->appCtx->cutHead1->blink = awo->appCtx->cutHead1;

  // copy selected nodes to cut list
  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    curSel->node->eraseSelectBoxCorners();

    // deselect
    curSel->node->deselect();

    newOne = new activeGraphicListType;
    newOne->defExeFlink = NULL;
    newOne->defExeBlink = NULL;
    newOne->node = awo->obj.clone( curSel->node->objName(), curSel->node );

    // copy to cut list
    newOne->blink = awo->appCtx->cutHead1->blink;
    awo->appCtx->cutHead1->blink->flink = newOne;
    awo->appCtx->cutHead1->blink = newOne;
    newOne->flink = awo->appCtx->cutHead1;

    curSel = curSel->selFlink;

  }

  // make selected list empty
  awo->selectedHead->selFlink = awo->selectedHead;
  awo->selectedHead->selBlink = awo->selectedHead;

  awo->state = AWC_NONE_SELECTED;
  awo->updateMasterSelection();

}

static int undo (
  activeWindowClass *awo )
{

int stat;

  stat = awo->undoObj.performUndo();
  awo->clear();
  awo->refresh();
  awo->updateMasterSelection();

  return stat;

}

static void paste (
  int x,
  int y,
  int item,
  activeWindowClass *awo )
{

int num_selected;
activeGraphicListPtr curCut, curSel, newOne;

int newX, newY, locMinX, locMinY, locDeltaX, locDeltaY;

  // empty select buffer then copy all nodes from cut buffer
  // and place them into main buffer and select buffer

  curCut = awo->appCtx->cutHead1->flink;
  if ( curCut == awo->appCtx->cutHead1 ) return;

  awo->setChanged();

  // deselect all currently selected
  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {

    // deselect
    curSel->node->deselect();
    curSel->node->eraseSelectBoxCorners();

    curSel = curSel->selFlink;

  }

  // make selected list empty
  awo->selectedHead->selFlink = awo->selectedHead;
  awo->selectedHead->selBlink = awo->selectedHead;

  locMinX = 1000000;
  locMinY = 1000000;
  locDeltaX = 0;
  locDeltaY = 0;

  if ( item != AWC_POPUP_PASTE_IN_PLACE ) {

    curCut = awo->appCtx->cutHead1->flink;
    while ( curCut != awo->appCtx->cutHead1 ) {

      if ( curCut->node->getX0() < locMinX ) {
        locMinX = curCut->node->getX0();
      }
      if ( curCut->node->getY0() < locMinY ) {
        locMinY = curCut->node->getY0();
      }

      curCut = curCut->flink;

    }

    locDeltaX = x - locMinX;
    locDeltaY = y - locMinY;

  }

  curCut = awo->appCtx->cutHead1->blink;
  while ( curCut != awo->appCtx->cutHead1 ) {

    newOne = new activeGraphicListType;
    newOne->defExeFlink = NULL;
    newOne->defExeBlink = NULL;

    // before performing the paste, make sure active window pointer of
    //  object in cut list points to the current active window (in
    //  case this object was cut or copied from another window)
    curCut->node->actWin = awo;
    curCut->node->updateGroup(); // for groups

    newOne->node = awo->obj.clone( curCut->node->objName(), curCut->node );
    newOne->node->actWin = awo;
    newOne->node->updateGroup(); // for groups
    newOne->node->move( locDeltaX, locDeltaY );
    newOne->node->moveSelectBox( locDeltaX, locDeltaY );

    /* do this in case the grid is active */

    newX = newOne->node->getX0();
    newY = newOne->node->getY0();
    awo->filterPosition( &newX, &newY, newX, newY );

    newOne->node->moveAbs( newX, newY );
    newOne->node->moveSelectBoxAbs( newX, newY );

    // main buffer
    newOne->blink = awo->head->blink;
    awo->head->blink->flink = newOne;
    awo->head->blink = newOne;
    newOne->flink = awo->head;

    // select buffer
    newOne->selBlink = awo->selectedHead->selBlink;
    awo->selectedHead->selBlink->selFlink = newOne;
    awo->selectedHead->selBlink = newOne;
    newOne->selFlink = awo->selectedHead;

    newOne->node->setSelected();

    curCut = curCut->blink;

  }

  // determine new state
  num_selected = 0;

  curSel = awo->selectedHead->selFlink;
  while ( ( curSel != awo->selectedHead ) &&
          ( num_selected < 2 ) ) {

    num_selected++;
    curSel = curSel->selFlink;

  }

  if ( num_selected == 0 ) {
    awo->state = AWC_NONE_SELECTED;
    awo->updateMasterSelection();
  }
  else if ( num_selected == 1 ) {
    awo->state = AWC_ONE_SELECTED;
    awo->useFirstSelectedAsReference = 1;
    awo->updateMasterSelection();
  }
  else {
    awo->state = AWC_MANY_SELECTED;
    awo->updateMasterSelection();
  }

  awo->refresh();

  return;

}

static void b2ReleaseNoneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo;
popupBlockPtr block;
int stat, item, wasSelected, num_selected;
activeGraphicListPtr curSel;
activeGraphicListPtr symHead, cur1, cur2, curGroup, next1, next2;
int n;
Arg args[10];
XmString xmStr1, xmStr2;

Window root, child;
int rootX, rootY, winX, winY;
unsigned int mask;
int symbolStateCount, invisRect, contained, autoMakePossible, y0, y1, yMid,
 sortValue, i, ii, first, last;
char msg[79+1];

struct {
  activeGraphicListPtr listElement;
  int ySortValue;
  int processed;
} nodeArray[SYMBOL_K_NUM_STATES], tmp;

Atom wm_delete_window;

  block = (popupBlockPtr) client;
  item = (int) block->ptr;
  awo = (activeWindowClass *) block->awo;

  switch ( item ) {

    case AWC_POPUP_SELECT_ALL:

      cur1 = awo->head->blink;
      while ( cur1 != awo->head ) {

          wasSelected = cur1->node->isSelected();
          if ( !wasSelected ) {
            num_selected++;
            cur1->node->setSelected();
            //cur1->node->drawSelectBoxCorners();
            cur1->selBlink = awo->selectedHead->selBlink;
            awo->selectedHead->selBlink->selFlink = cur1;
            awo->selectedHead->selBlink = cur1;
            cur1->selFlink = awo->selectedHead;
          }

        cur1 = cur1->blink;

      }

      // determine new state
      num_selected = 0;

      curSel = awo->selectedHead->selFlink;
      while ( ( curSel != awo->selectedHead ) &&
              ( num_selected < 2 ) ) {

        num_selected++;
        curSel = curSel->selFlink;

      }

      if ( num_selected == 0 ) {
        awo->state = AWC_NONE_SELECTED;
        awo->updateMasterSelection();
      }
      else if ( num_selected == 1 ) {
        awo->state = AWC_ONE_SELECTED;
        awo->useFirstSelectedAsReference = 1;
        awo->updateMasterSelection();
      }
      else {
        awo->state = AWC_MANY_SELECTED;
        awo->updateMasterSelection();
      }

      awo->refresh();

      break;

    case AWC_POPUP_MAKESYMBOL:

      // first, examine each symbol. If not an invisible rectangle, then
      // find an invisible rectangle that contains the previously
      // examined symbol. If there is no containing rectangle abort
      // the process and inform user. If the symbol was an invisible
      // rectangle, then make sure that no other invisible rectangle
      // contains it. If so, abort the process and inform user.

      symbolStateCount = 0;
      autoMakePossible = 1;

      cur1 = awo->head->flink;
      while ( cur1 != awo->head ) {

        if ( strcmp( cur1->node->objName(), "activeGroupClass" ) == 0 ) {
          strcpy( msg, activeWindowClass_str6 );
          autoMakePossible = 0;
          break;
        }

        if ( strcmp( cur1->node->objName(), "activeRectangleClass" ) == 0 ) {
          if ( cur1->node->isInvisible() ) {
            invisRect = 1;
            symbolStateCount++;
            if ( symbolStateCount >= SYMBOL_K_NUM_STATES ) {
              strcpy( msg, activeWindowClass_str7 );
              autoMakePossible = 0;
              break;
            }
          }
          else {
            invisRect = 0;
          }
        }
        else {
          invisRect = 0;
        }

        if ( !invisRect ) {

          // find an invisible rectangle containing cur1->node

          contained = 0;
          cur2 = awo->head->flink;
          while ( cur2 != awo->head ) {

            if ( strcmp( cur2->node->objName(), "activeRectangleClass" ) == 0 ) {
              if ( cur2->node->isInvisible() ) {
                if ( isContained( cur1->node, cur2->node ) ) {
                  contained = 1;
                  break;
                }
                invisRect = 1;
              }
            }

            cur2 = cur2->flink;

          }

          if ( !contained ) {
            strcpy( msg,
            activeWindowClass_str8 );
            autoMakePossible = 0;
            break;
          }

        }
        else {

          // make sure no other invisible rectangle contains cur1->node

          contained = 0;
          cur2 = awo->head->flink;
          while ( cur2 != awo->head ) {

            if ( cur1 == cur2 ) {
              cur2 = cur2->flink;
              continue;
            }

            if ( strcmp( cur2->node->objName(), "activeRectangleClass" ) == 0 ) {
              if ( cur2->node->isInvisible() ) {
                if ( isContained( cur1->node, cur2->node ) ) {
                  contained = 1;
                  break;
                }
                invisRect = 1;
              }
            }

            cur2 = cur2->flink;

          }

          if ( contained ) {
            strcpy( msg,
            activeWindowClass_str9 );
            autoMakePossible = 0;
            break;
          }

        }

        cur1 = cur1->flink;

      }

      if ( !autoMakePossible ) {
        awo->appCtx->postMessage( activeWindowClass_str10 );
        awo->appCtx->postMessage( msg );
      }

      if ( autoMakePossible ) {

        // deselect all currently selected
        curSel = awo->selectedHead->selFlink;
        while ( curSel != awo->selectedHead ) {

          // deselect
          curSel->node->deselect();
          curSel->node->eraseSelectBoxCorners();

          curSel = curSel->selFlink;

        }

        // make selected list empty
        awo->selectedHead->selFlink = awo->selectedHead;
        awo->selectedHead->selBlink = awo->selectedHead;

        // remove all objects from main list and place on symbol list
        symHead = new activeGraphicListType;
        symHead->flink = symHead;
        symHead->blink = symHead;
        symHead->defExeFlink = NULL;
        symHead->defExeBlink = NULL;

        cur1 = awo->head->flink;
        while ( cur1 != awo->head ) {

          next1 = cur1->flink;

          // unlink
          cur1->blink->flink = cur1->flink;
          cur1->flink->blink = cur1->blink;

          // link
          cur1->blink = symHead->blink;
          symHead->blink->flink = cur1;
          symHead->blink = cur1;
          cur1->flink = symHead;

          cur1 = next1;

        }

        // traverse symHead list iterively; find invisible rectange:
        // select and place on main and select lists; traverse symHead
        // list again and find all non-invisible rectangles contained
        // in current invisible rectange: select and place on main and
        // select lists; when all contained objects have be found,
        // create group. Continue in like manner until symHead list is empty.

        cur1 = symHead->flink;
        while ( cur1 != symHead ) {

          if ( strcmp( cur1->node->objName(), "activeRectangleClass" ) == 0 ) {

            if ( cur1->node->isInvisible() ) {

              // unlink
              cur1->blink->flink = cur1->flink;
              cur1->flink->blink = cur1->blink;

              // link into main list
              cur1->flink = awo->head->flink;
              awo->head->flink->blink = cur1;
              awo->head->flink = cur1;
              cur1->blink = awo->head;

              // link into select list
              cur1->selFlink = awo->selectedHead->selFlink;
              awo->selectedHead->selFlink->selBlink = cur1;
              awo->selectedHead->selFlink = cur1;
              cur1->selBlink = awo->selectedHead;

              cur1->node->setSelected();

              // traverse list again and put all non-invisible rectangles
              // on list and set selected

              cur2 = symHead->flink;
              while ( cur2 != symHead ) {

                next2 = cur2->flink;

                if ( ( strcmp( cur2->node->objName(), "activeRectangleClass" ) != 0 ) ||
                     !cur2->node->isInvisible() ) {

                  if ( isContained( cur2->node, cur1->node ) ) {

                    // unlink
                    cur2->blink->flink = cur2->flink;
                    cur2->flink->blink = cur2->blink;

                    // link into main
                    cur2->flink = awo->head->flink;
                    awo->head->flink->blink = cur2;
                    awo->head->flink = cur2;
                    cur2->blink = awo->head;

                    // link into select list
                    cur2->selFlink = awo->selectedHead->selFlink;
                    awo->selectedHead->selFlink->selBlink = cur2;
                    awo->selectedHead->selFlink = cur2;
                    cur2->selBlink = awo->selectedHead;

                    cur2->node->setSelected();

                  }

                }

                cur2 = next2;

	      }

              // create group of all selected objects
              curGroup = new activeGraphicListType;
              curGroup->defExeFlink = NULL;
              curGroup->defExeBlink = NULL;
              curGroup->node = new activeGroupClass;
              stat = curGroup->node->createGroup( awo );

              // link into main list
              curGroup->blink = awo->head->blink;
              awo->head->blink->flink = curGroup;
              awo->head->blink = curGroup;
              curGroup->flink = awo->head;

              cur1 = symHead; // start from beginning again

            }

          }

          cur1 = cur1->flink;

        }

        delete symHead;

        // now do 2-D sort of nodes left-to-right, up-to-down; we will not have
        // many groups so can use an inefficient sort here

        // copy to array
        n = 0;
        cur1 = awo->head->flink;
        while ( cur1 != awo->head ) {

          nodeArray[n].listElement = cur1;
          nodeArray[n].ySortValue = 0;
          nodeArray[n].processed = 0;
          n++;

          cur1 = cur1->flink;

        }

        // set ySortValue for all nodes in same row to common value
        sortValue = 1;
        for ( i=0; i<n-1; i++ ) {

          if ( !nodeArray[i].processed ) {

            nodeArray[i].processed = 1;
            y0 = nodeArray[i].listElement->node->getY0();
            y1 = nodeArray[i].listElement->node->getY1();
            nodeArray[i].ySortValue =
             nodeArray[i].listElement->node->getYMid();

            for ( ii=i+1; ii<n; ii++ ) {

              if ( !nodeArray[ii].processed ) {
                yMid = nodeArray[ii].listElement->node->getYMid();
                if ( ( yMid > y0 ) && ( yMid < y1 ) ) {
                  nodeArray[ii].ySortValue = nodeArray[i].ySortValue;
                  nodeArray[ii].processed = 1;
                }
              }

            }

          }

        }

        // do exchange sort by ySortValue
        for ( i=1; i<n; i++ ) {
          for ( ii=i; ii>0; ii-- ) {
            if ( nodeArray[ii].ySortValue < nodeArray[ii-1].ySortValue ) {
              tmp = nodeArray[ii];
              nodeArray[ii] = nodeArray[ii-1];
              nodeArray[ii-1] = tmp;
            }
          }
        }

        // do exchange sort by X0 for each ySortValue group
        first = 0;
        while ( first < n ) {

          sortValue = nodeArray[first].ySortValue;
          last = first;
          i = first + 1;
          while ( ( nodeArray[i].ySortValue == sortValue ) &&
                  ( i < n ) ) {
            last = i;
            i++;
          }

          for ( i=first+1; i<=last; i++ ) {
            for ( ii=i; ii>first; ii-- ) {
              if ( nodeArray[ii].listElement->node->getX0() <
                   nodeArray[ii-1].listElement->node->getX0() ) {
                tmp = nodeArray[ii];
                nodeArray[ii] = nodeArray[ii-1];
                nodeArray[ii-1] = tmp;
              }
            }
          }

          first = last + 1;

        }

        // now empty and rebuild the main list
        cur1 = awo->head->flink;
        while ( cur1 != awo->head ) {

          next1 = cur1->flink;

          // unlink
          cur1->blink->flink = cur1->flink;
          cur1->flink->blink = cur1->blink;

          cur1 = next1;

        }

        for ( i=0; i<n; i++ ) {

          // link into main list
          nodeArray[i].listElement->blink = awo->head->blink;
          awo->head->blink->flink = nodeArray[i].listElement;
          awo->head->blink = nodeArray[i].listElement;
          nodeArray[i].listElement->flink = awo->head;

        }

        awo->refresh();
        awo->setChanged();

        sprintf( msg, activeWindowClass_str11, n );
        awo->appCtx->postMessage( msg );

      }

      break;

    case AWC_POPUP_CLOSE:

      if ( awo->change ) {

        awo->savedState = awo->state;
        awo->state = AWC_WAITING;

        XQueryPointer( awo->d, XtWindow(awo->top), &root, &child,
         &rootX, &rootY, &winX, &winY, &mask );

        awo->confirm.create( awo->top, awo->b2PressXRoot, awo->b2PressYRoot, 3,
         activeWindowClass_str12, NULL, NULL );
        awo->confirm.addButton( activeWindowClass_str13, awc_continue_cb,
         (void *) awo );
        awo->confirm.addButton( activeWindowClass_str14, awc_abort_cb,
         (void *) awo );
        awo->confirm.addButton( activeWindowClass_str15, awc_save_and_exit_cb,
         (void *) awo );
        awo->confirm.finished();
        awo->confirm.popup();
        XSetWindowColormap( awo->d, XtWindow(awo->confirm.top()),
         awo->appCtx->ci.getColorMap() );

      }
      else {

        if ( awo->autosaveTimer ) {
          XtRemoveTimeOut( awo->autosaveTimer );
          awo->autosaveTimer = 0;
        }
        if ( awo->restoreTimer ) {
          XtRemoveTimeOut( awo->restoreTimer );
          awo->restoreTimer = 0;
        }

        //mark active window for delege
        awo->appCtx->removeActiveWindow( awo );

        XtUnmanageChild( awo->drawWidget );

      }

      break;

    case AWC_POPUP_EXECUTE:

      awo->appCtx->activateActiveWindow( awo );
      break;

    case AWC_POPUP_UNDO:

      stat = undo( awo );
      if ( !( stat & 1 ) ) XBell( awo->d, 50 );
      break;

    case AWC_POPUP_REFRESH:

      awo->clear();
      awo->refresh();

      break;

    case AWC_POPUP_FINDTOP:

      awo->appCtx->findTop();  // raise edm main window
      break;

    case AWC_POPUP_OUTLIERS:

      n = 0;
      cur1 = awo->head->flink;
      while ( cur1 != awo->head ) {

        if (
             ( ( cur1->node->getX0()+6 > awo->w ) &&
               ( cur1->node->getX1() > awo->w ) ) ||
             ( ( cur1->node->getX1()-6 < 0 ) &&
               ( cur1->node->getX0() < 0 ) ) ||
             ( ( cur1->node->getY0()+6 > awo->h ) &&
               ( cur1->node->getY1() > awo->h ) ) ||
             ( ( cur1->node->getY1()-6 < 0 ) &&
               ( cur1->node->getY0() < 0 ) )
          ) {

          n++;

          // link into select list
          cur1->selFlink = awo->selectedHead->selFlink;
          awo->selectedHead->selFlink->selBlink = cur1;
          awo->selectedHead->selFlink = cur1;
          cur1->selBlink = awo->selectedHead;

          cur1->node->setSelected();

          awo->state = AWC_ONE_SELECTED;
          awo->useFirstSelectedAsReference = 1;

          awo->updateMasterSelection();

          awo->cursor.set( XtWindow(awo->drawWidget), CURSOR_K_WAIT );
          awo->cursor.setColor( awo->ci->pix(awo->fgColor),
           awo->ci->pix(awo->bgColor) );

          awo->savedState = awo->state;
          awo->state = AWC_EDITING;
          awo->currentEf = NULL;
          cur1->node->doEdit();

          break;

	}

        cur1 = cur1->flink;

      }

      if ( n == 0 ) awo->appCtx->postMessage( activeWindowClass_str16 );

      break;

    case AWC_POPUP_LOAD_SCHEME:

      awo->savedState = awo->state;
      awo->state = AWC_WAITING;

      xmStr1 = XmStringCreateLocalized( "*.scheme" );
      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->colorPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->colorPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->schemeSelectBox = XmCreateFileSelectionDialog( awo->top, "", args,
       n );

      XmStringFree( xmStr1 );
      if ( xmStr2 ) XmStringFree( xmStr2 );

      XtAddCallback( awo->schemeSelectBox, XmNcancelCallback,
       awc_loadSchemeSelectCancel_cb, (void *) awo );
      XtAddCallback( awo->schemeSelectBox, XmNokCallback,
       awc_loadSchemeSelectOk_cb, (void *) awo );

      // -----------------------------------------------------

      awo->wpSchemeSelect.w = awo->schemeSelectBox;
      awo->wpSchemeSelect.client = (void *) awo;

      wm_delete_window = XmInternAtom( XtDisplay(awo->top),
       "WM_DELETE_WINDOW", False );

      XmAddWMProtocolCallback( XtParent(awo->schemeSelectBox),
       wm_delete_window, awc_loadSchemeSelectKill_cb,
       &awo->wpSchemeSelect );

      XtVaSetValues( XtParent(awo->schemeSelectBox), XmNdeleteResponse,
       XmDO_NOTHING, NULL );

      // -----------------------------------------------------

      XtManageChild( awo->schemeSelectBox );

      XSetWindowColormap( awo->d,
       XtWindow(XtParent(awo->schemeSelectBox)),
       awo->appCtx->ci.getColorMap() );

      break;

    case AWC_POPUP_SAVE_SCHEME:

      awo->savedState = awo->state;
      awo->state = AWC_WAITING;

      xmStr1 = XmStringCreateLocalized( "*.scheme" );
      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->colorPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->colorPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->schemeSelectBox = XmCreateFileSelectionDialog( awo->top, "", args,
       n );

      XmStringFree( xmStr1 );
      if ( xmStr2 ) XmStringFree( xmStr2 );

      XtAddCallback( awo->schemeSelectBox, XmNcancelCallback,
       awc_saveSchemeSelectCancel_cb, (void *) awo );
      XtAddCallback( awo->schemeSelectBox, XmNokCallback,
       awc_saveSchemeSelectOk_cb, (void *) awo );

      // -----------------------------------------------------

      awo->wpSchemeSelect.w = awo->schemeSelectBox;
      awo->wpSchemeSelect.client = (void *) awo;

      wm_delete_window = XmInternAtom( XtDisplay(awo->top),
       "WM_DELETE_WINDOW", False );

      XmAddWMProtocolCallback( XtParent(awo->schemeSelectBox),
       wm_delete_window, awc_saveSchemeSelectKill_cb,
       &awo->wpSchemeSelect );

      XtVaSetValues( XtParent(awo->schemeSelectBox), XmNdeleteResponse,
       XmDO_NOTHING, NULL );

      // -----------------------------------------------------

      XtManageChild( awo->schemeSelectBox );

      XSetWindowColormap( awo->d,
       XtWindow(XtParent(awo->schemeSelectBox)),
       awo->appCtx->ci.getColorMap() );

      break;

    case AWC_POPUP_SAVE_AS:

      awo->savedState = awo->state;
      awo->state = AWC_WAITING;

      xmStr1 = XmStringCreateLocalized( "*.edl" );
      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top, "", args,
       n );

      XmStringFree( xmStr1 );
      if ( xmStr2 ) XmStringFree( xmStr2 );

      XtAddCallback( awo->fileSelectBox, XmNcancelCallback,
       awc_saveFileSelectCancel_cb, (void *) awo );
      XtAddCallback( awo->fileSelectBox, XmNokCallback,
       awc_saveFileSelectOk_cb, (void *) awo );

      // -----------------------------------------------------

      awo->wpFileSelect.w = awo->fileSelectBox;
      awo->wpFileSelect.client = (void *) awo;

      wm_delete_window = XmInternAtom( XtDisplay(awo->top),
       "WM_DELETE_WINDOW", False );

      XmAddWMProtocolCallback( XtParent(awo->fileSelectBox),
       wm_delete_window, awc_saveFileSelectKill_cb, &awo->wpFileSelect );

      XtVaSetValues( XtParent(awo->fileSelectBox), XmNdeleteResponse,
       XmDO_NOTHING, NULL );

      // -----------------------------------------------------

      XtManageChild( awo->fileSelectBox );

      XSetWindowColormap( awo->d,
       XtWindow(XtParent(awo->fileSelectBox)),
       awo->appCtx->ci.getColorMap() );

      break;

    case AWC_POPUP_SAVE:

      if ( strcmp( awo->fileName, "" ) == 0 ) { // do save as ...

        awo->savedState = awo->state;
        awo->state = AWC_WAITING;

        xmStr1 = XmStringCreateLocalized( "*.edl" );
        xmStr2 = NULL;

        n = 0;
        XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

        if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
          xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
          XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
        }

        awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top, "", args,
         n );

        XmStringFree( xmStr1 );
        if ( xmStr2 ) XmStringFree( xmStr2 );

        XtAddCallback( awo->fileSelectBox, XmNcancelCallback,
         awc_saveFileSelectCancel_cb, (void *) awo );
        XtAddCallback( awo->fileSelectBox, XmNokCallback,
         awc_saveFileSelectOk_cb, (void *) awo );

        // -----------------------------------------------------

        awo->wpFileSelect.w = awo->fileSelectBox;
        awo->wpFileSelect.client = (void *) awo;

        wm_delete_window = XmInternAtom( XtDisplay(awo->top),
         "WM_DELETE_WINDOW", False );

        XmAddWMProtocolCallback( XtParent(awo->fileSelectBox),
         wm_delete_window, awc_saveFileSelectKill_cb, &awo->wpFileSelect );

        XtVaSetValues( XtParent(awo->fileSelectBox), XmNdeleteResponse,
         XmDO_NOTHING, NULL );

        // -----------------------------------------------------

        XtManageChild( awo->fileSelectBox );

        XSetWindowColormap( awo->d,
         XtWindow(XtParent(awo->fileSelectBox)),
         awo->appCtx->ci.getColorMap() );

      }
      else {

        awo->save( awo->fileName );

      }

      break;

    case AWC_POPUP_OPEN:

      awo->savedState = awo->state;
      awo->state = AWC_WAITING;

      xmStr1 = XmStringCreateLocalized( "*.edl" );
      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top, "", args,
       n );

      XmStringFree( xmStr1 );
      if ( xmStr2 ) XmStringFree( xmStr2 );

      XtAddCallback( awo->fileSelectBox, XmNcancelCallback,
       awc_fileSelectCancel_cb, (void *) awo );
      XtAddCallback( awo->fileSelectBox, XmNokCallback,
       awc_fileSelectOk_cb, (void *) awo );

      // -----------------------------------------------------

      awo->wpFileSelect.w = awo->fileSelectBox;
      awo->wpFileSelect.client = (void *) awo;

      wm_delete_window = XmInternAtom( XtDisplay(awo->top),
       "WM_DELETE_WINDOW", False );

      XmAddWMProtocolCallback( XtParent(awo->fileSelectBox),
       wm_delete_window, awc_fileSelectKill_cb, &awo->wpFileSelect );

      XtVaSetValues( XtParent(awo->fileSelectBox), XmNdeleteResponse,
       XmDO_NOTHING, NULL );

      // -----------------------------------------------------

      XtManageChild( awo->fileSelectBox );

      XSetWindowColormap( awo->d,
       XtWindow(XtParent(awo->fileSelectBox)),
       awo->appCtx->ci.getColorMap() );

      break;

    case AWC_POPUP_OPEN_USER:

      awo->savedState = awo->state;
      awo->state = AWC_WAITING;

      xmStr1 = XmStringCreateLocalized( "*.edl" );
      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top, "", args,
       n );

      XmStringFree( xmStr1 );
      if ( xmStr2 ) XmStringFree( xmStr2 );

      XtAddCallback( awo->fileSelectBox, XmNcancelCallback,
       awc_fileSelectCancel_cb, (void *) awo );
      XtAddCallback( awo->fileSelectBox, XmNokCallback,
       awc_fileSelectOk_cb, (void *) awo );

      // -----------------------------------------------------

      awo->wpFileSelect.w = awo->fileSelectBox;
      awo->wpFileSelect.client = (void *) awo;

      wm_delete_window = XmInternAtom( XtDisplay(awo->top),
       "WM_DELETE_WINDOW", False );

      XmAddWMProtocolCallback( XtParent(awo->fileSelectBox),
       wm_delete_window, awc_fileSelectKill_cb, &awo->wpFileSelect );

      XtVaSetValues( XtParent(awo->fileSelectBox), XmNdeleteResponse,
       XmDO_NOTHING, NULL );

      // -----------------------------------------------------

      XtManageChild( awo->fileSelectBox );

      XSetWindowColormap( awo->d,
       XtWindow(XtParent(awo->fileSelectBox)),
       awo->appCtx->ci.getColorMap() );

      break;

    case AWC_POPUP_PASTE_IN_PLACE:
    case AWC_POPUP_PASTE:

      paste( awo->b2PressX, awo->b2PressY, item, awo );
      awo->refresh();
      break;

#if 0
      // empty select buffer then copy all nodes from cut buffer
      // and place them into main buffer and select buffer

      curCut = awo->appCtx->cutHead1->flink;
      if ( curCut == awo->appCtx->cutHead1 ) break; // nothing to paste

      awo->setChanged();

      // deselect all currently selected
      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        // deselect
        curSel->node->deselect();
        curSel->node->eraseSelectBoxCorners();

        curSel = curSel->selFlink;

      }

      // make selected list empty
      awo->selectedHead->selFlink = awo->selectedHead;
      awo->selectedHead->selBlink = awo->selectedHead;

      locMinX = 1000000;
      locMinY = 1000000;
      locDeltaX = 0;
      locDeltaY = 0;

      if ( item != AWC_POPUP_PASTE_IN_PLACE ) {

      curCut = awo->appCtx->cutHead1->flink;
      while ( curCut != awo->appCtx->cutHead1 ) {

        if ( curCut->node->getX0() < locMinX ) {
          locMinX = curCut->node->getX0();
        }
        if ( curCut->node->getY0() < locMinY ) {
          locMinY = curCut->node->getY0();
        }

        curCut = curCut->flink;

      }

      locDeltaX = awo->b2PressX - locMinX;
      locDeltaY = awo->b2PressY - locMinY;

      }

//        curCut = awo->appCtx->cutHead1->flink;
//        while ( curCut != awo->appCtx->cutHead1 ) {

      curCut = awo->appCtx->cutHead1->blink;
      while ( curCut != awo->appCtx->cutHead1 ) {

        newOne = new activeGraphicListType;
        newOne->defExeFlink = NULL;
        newOne->defExeBlink = NULL;

        // before performing the paste, make sure active window pointer of
        //  object in cut list points to the current active window (in
        //  case this object was cut or copied from another window)
        curCut->node->actWin = awo;
        curCut->node->updateGroup(); // for groups

        newOne->node = awo->obj.clone( curCut->node->objName(), curCut->node );
        newOne->node->actWin = awo;
        newOne->node->updateGroup(); // for groups
        newOne->node->move( locDeltaX, locDeltaY );
        newOne->node->moveSelectBox( locDeltaX, locDeltaY );

        /* do this in case the grid is active */

        newX = newOne->node->getX0();
        newY = newOne->node->getY0();
        awo->filterPosition( &newX, &newY, newX, newY );

        newOne->node->moveAbs( newX, newY );
        newOne->node->moveSelectBoxAbs( newX, newY );

        /* */

        // main buffer
//          newOne->flink = awo->head->flink;
//          awo->head->flink->blink = newOne;
//          awo->head->flink = newOne;
//          newOne->blink = awo->head;

        newOne->blink = awo->head->blink;
        awo->head->blink->flink = newOne;
        awo->head->blink = newOne;
        newOne->flink = awo->head;

        // select buffer
        newOne->selBlink = awo->selectedHead->selBlink;
        awo->selectedHead->selBlink->selFlink = newOne;
        awo->selectedHead->selBlink = newOne;
        newOne->selFlink = awo->selectedHead;

        newOne->node->setSelected();

//          curCut = curCut->flink;

        curCut = curCut->blink;

      }

      // determine new state
      num_selected = 0;

      curSel = awo->selectedHead->selFlink;
      while ( ( curSel != awo->selectedHead ) &&
              ( num_selected < 2 ) ) {

        num_selected++;
        curSel = curSel->selFlink;

      }

      if ( num_selected == 0 ) {
        awo->state = AWC_NONE_SELECTED;
        awo->updateMasterSelection();
      }
      else if ( num_selected == 1 ) {
        awo->state = AWC_ONE_SELECTED;
        awo->useFirstSelectedAsReference = 1;
        awo->updateMasterSelection();
      }
      else {
        awo->state = AWC_MANY_SELECTED;
	awo->updateMasterSelection();
      }
#endif

      awo->refresh();

      break;

    case AWC_POPUP_PROPERTIES: // properties

      awo->savedState = awo->state;
      
      awo->state = AWC_EDITING;
      awo->currentEf = NULL;
      strncpy( awo->bufId, awo->id, 31 );
      awo->bufX = awo->x;
      awo->bufY = awo->y;
      awo->bufW = awo->w;
      awo->bufH = awo->h;
      strncpy( awo->bufTitle, awo->title, 127 );
      strncpy( awo->bufDefaultPvType, awo->defaultPvType, 15 );

      awo->bufOrthoMove = awo->orthoMove;
      awo->bufOrthogonal = awo->orthogonal;
      awo->bufGridSpacing = awo->gridSpacing;
      strcpy( awo->bufGridActiveStr, awo->gridActiveStr );
      strcpy( awo->bufGridShowStr, awo->gridShowStr );
      awo->bufActivateCallbackFlag = awo->activateCallbackFlag;
      awo->bufDeactivateCallbackFlag = awo->deactivateCallbackFlag;

      awo->bufFgColor = awo->fgColor;
      awo->bufBgColor = awo->bgColor;
      awo->bufDefaultTextFgColor = awo->defaultTextFgColor;
      awo->bufDefaultFg1Color = awo->defaultFg1Color;
      awo->bufDefaultFg2Color = awo->defaultFg2Color;
      awo->bufDefaultBgColor = awo->defaultBgColor;
      awo->bufDefaultTopShadowColor = awo->defaultTopShadowColor;
      awo->bufDefaultBotShadowColor = awo->defaultBotShadowColor;
      awo->bufDefaultOffsetColor = awo->defaultOffsetColor;

      awo->ef.create( awo->top, awo->appCtx->ci.getColorMap(),
       &awo->appCtx->entryFormX,
       &awo->appCtx->entryFormY, &awo->appCtx->entryFormW,
       &awo->appCtx->entryFormH, &awo->appCtx->largestH,
       activeWindowClass_str17, NULL, NULL, NULL );

      awo->ef.addTextField( activeWindowClass_str18, 30, awo->bufId, 31 );

      awo->ef.addTextField( activeWindowClass_str19, 30, &awo->bufX );
      awo->ef.addTextField( activeWindowClass_str20, 30, &awo->bufY );
      awo->ef.addTextField( activeWindowClass_str21, 30, &awo->bufW );
      awo->ef.addTextField( activeWindowClass_str22, 30, &awo->bufH );

      awo->ef.addTextField( activeWindowClass_str23, 30, awo->bufTitle, 127 );

      awo->ef.addTextField( activeWindowClass_str24, 30,
       awo->bufDefaultPvType, 15 );

      awo->ef.addColorButton( activeWindowClass_str25, awo->ci, &awo->fgCb,
       &awo->bufFgColor );
      awo->ef.addColorButton( activeWindowClass_str26, awo->ci, &awo->bgCb,
       &awo->bufBgColor );
      awo->ef.addOption( activeWindowClass_str27, activeWindowClass_str28,
       awo->bufGridShowStr, 8 );
      awo->ef.addOption( activeWindowClass_str29, activeWindowClass_str28,
       awo->bufGridActiveStr, 8 );
      awo->ef.addTextField( activeWindowClass_str30, 30,
       &awo->bufGridSpacing );
      awo->ef.addOption( activeWindowClass_str168, activeWindowClass_str32,
       &awo->bufOrthoMove );
      awo->ef.addOption( activeWindowClass_str31, activeWindowClass_str32,
       &awo->bufOrthogonal );
      awo->ef.addColorButton( activeWindowClass_str33, awo->ci, &awo->defaultTextFgCb,
       &awo->bufDefaultTextFgColor );
      awo->ef.addColorButton( activeWindowClass_str34, awo->ci, &awo->defaultFg1Cb,
       &awo->bufDefaultFg1Color );
      awo->ef.addColorButton( activeWindowClass_str35, awo->ci, &awo->defaultFg2Cb,
       &awo->bufDefaultFg2Color );
      awo->ef.addColorButton( activeWindowClass_str36, awo->ci, &awo->defaultBgCb,
       &awo->bufDefaultBgColor );
      awo->ef.addColorButton( activeWindowClass_str37, awo->ci,
       &awo->defaultOffsetCb, &awo->bufDefaultOffsetColor );
      awo->ef.addColorButton( activeWindowClass_str38, awo->ci,
       &awo->defaultTopShadowCb, &awo->bufDefaultTopShadowColor );
      awo->ef.addColorButton( activeWindowClass_str39, awo->ci,
       &awo->defaultBotShadowCb, &awo->bufDefaultBotShadowColor );
      awo->ef.addFontMenu( activeWindowClass_str183, awo->fi, &awo->defaultFm,
       awo->defaultFontTag );
      awo->defaultFm.setFontAlignment( awo->defaultAlignment );
      awo->ef.addFontMenu( activeWindowClass_str40, awo->fi, &awo->defaultCtlFm,
       awo->defaultCtlFontTag );
      awo->defaultCtlFm.setFontAlignment( awo->defaultCtlAlignment );
      awo->ef.addFontMenu( activeWindowClass_str41, awo->fi, &awo->defaultBtnFm,
       awo->defaultBtnFontTag );
      awo->defaultBtnFm.setFontAlignment( awo->defaultBtnAlignment );
      awo->ef.addToggle( activeWindowClass_str42, &awo->bufActivateCallbackFlag );
      awo->ef.addToggle( activeWindowClass_str43,
       &awo->bufDeactivateCallbackFlag );
      awo->ef.finished( awc_edit_ok, awc_edit_apply, awc_edit_cancel, awo );
      awo->ef.popup();

      break;

    case AWC_POPUP_HELP:

      awo->openExecuteSysFile( "helpMain" );
      break;

  }

}

static void b2ReleaseOneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo;
popupBlockPtr block;
int item, stat, num_selected;
activeGraphicListPtr cur, curSel, nextSel;

  block = (popupBlockPtr) client;
  item = (int) block->ptr;
  awo = (activeWindowClass *) block->awo;

  switch ( item ) {

    case AWC_POPUP_DESELECT:

      // deselect all
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        cur->node->deselect();
        cur->node->drawSelectBoxCorners(); // erase via xor gc
        cur = cur->selFlink;
      }
      // make list empty
      awo->selectedHead->selFlink = awo->selectedHead;
      awo->selectedHead->selBlink = awo->selectedHead;
      awo->state = AWC_NONE_SELECTED;
      awo->updateMasterSelection();

      awo->refresh();

      break;

    case AWC_POPUP_RAISE: // raise

      raise( awo );
      break;

#if 0
      // remove node and insert at tail

      awo->setChanged();

      curSel = awo->selectedHead->selFlink;
      if ( curSel != awo->selectedHead ) {

        // remove
        curSel->blink->flink = curSel->flink;
        curSel->flink->blink = curSel->blink;

        // insert at tail
        curSel->blink = awo->head->blink;
        awo->head->blink->flink = curSel;
        awo->head->blink = curSel;
        curSel->flink = awo->head;

        curSel->node->eraseSelectBoxCorners();
         curSel->node->drawAll();
        curSel->node->drawSelectBoxCorners();  // redraw

      }
      else {

        printErrMsg( __FILE__, __LINE__, "Inconsistent select state" );
        //printf( "Klingons decloaking! (1)\n" );

      }

      break;
#endif

    case AWC_POPUP_LOWER: // lower

      lower( awo );
      break;

#if 0
      // remove node and insert at head

      awo->setChanged();

      curSel = awo->selectedHead->selFlink;
      if ( curSel != awo->selectedHead ) {

        // remove
        curSel->blink->flink = curSel->flink;
        curSel->flink->blink = curSel->blink;

        // insert at tail
        curSel->flink = awo->head->flink;
        awo->head->flink->blink = curSel;
        awo->head->flink = curSel;
        curSel->blink = awo->head;

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();
        curSel->node->drawAll();
        curSel->node->drawSelectBoxCorners();  // redraw

      }
      else {

        printErrMsg( __FILE__, __LINE__, "Inconsistent select state" );
        //printf( "Klingons decloaking! (2)\n" );

      }

      break;
#endif

    case AWC_POPUP_CUT:

      cut( awo );
      awo->refresh();
      break;

#if 0
      // remove nodes and put on cut list; if cur list is not
      // empty the delete all nodes

      awo->undoObj.flush();

      awo->setChanged();

      // empty cut list
      curCut = awo->appCtx->cutHead1->flink;
      while ( curCut != awo->appCtx->cutHead1 ) {
        nextCut = curCut->flink;
        delete curCut->node;
        delete curCut;
        curCut = nextCut;
      }

      awo->appCtx->cutHead1->flink = awo->appCtx->cutHead1;
      awo->appCtx->cutHead1->blink = awo->appCtx->cutHead1;

      // remove nodes off main list
      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        // deselect
        curSel->node->deselect();

        // remove
        curSel->blink->flink = curSel->flink;
        curSel->flink->blink = curSel->blink;

        // insert into cut list
        curSel->blink = awo->appCtx->cutHead1->blink;
        awo->appCtx->cutHead1->blink->flink = curSel;
        awo->appCtx->cutHead1->blink = curSel;
        curSel->flink = awo->appCtx->cutHead1;

        curSel = curSel->selFlink;

      }

      // make selected list empty
      awo->selectedHead->selFlink = awo->selectedHead;
      awo->selectedHead->selBlink = awo->selectedHead;

      awo->state = AWC_NONE_SELECTED;
      awo->updateMasterSelection();

      awo->refresh();

      break;
#endif

    case AWC_POPUP_COPY:

      copy( awo );
      awo->refresh();
      break;

#if 0
      // copy nodes to cut list; if cur cut list is not
      // empty the delete all nodes

      // empty cut list
      curCut = awo->appCtx->cutHead1->flink;
      while ( curCut != awo->appCtx->cutHead1 ) {
        nextCut = curCut->flink;
        delete curCut->node;
        delete curCut;
        curCut = nextCut;
      }

      awo->appCtx->cutHead1->flink = awo->appCtx->cutHead1;
      awo->appCtx->cutHead1->blink = awo->appCtx->cutHead1;

      // copy selected nodes to cut list
      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();

        // deselect
        curSel->node->deselect();

        newOne = new activeGraphicListType;
        newOne->defExeFlink = NULL;
        newOne->defExeBlink = NULL;
        newOne->node = awo->obj.clone( curSel->node->objName(), curSel->node );

        // copy to cut list
        newOne->blink = awo->appCtx->cutHead1->blink;
        awo->appCtx->cutHead1->blink->flink = newOne;
        awo->appCtx->cutHead1->blink = newOne;
        newOne->flink = awo->appCtx->cutHead1;

        curSel = curSel->selFlink;

      }

      // make selected list empty
      awo->selectedHead->selFlink = awo->selectedHead;
      awo->selectedHead->selBlink = awo->selectedHead;

      awo->state = AWC_NONE_SELECTED;
      awo->updateMasterSelection();

      awo->refresh();

      break;
#endif

    case AWC_POPUP_GROUP:

      awo->undoObj.flush();

      awo->setChanged();

      cur = new activeGraphicListType;
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;
      cur->node = new activeGroupClass;
      stat = cur->node->createGroup( awo );

      // link into main list
      cur->blink = awo->head->blink;
      awo->head->blink->flink = cur;
      awo->head->blink = cur;
      cur->flink = awo->head;

      // link into selected list (which has been emptied by the
      // createGroup operation
      cur->selBlink = awo->selectedHead->selBlink;
      awo->selectedHead->selBlink->selFlink = cur;
      awo->selectedHead->selBlink = cur;
      cur->selFlink = awo->selectedHead;

      cur->node->setSelected();

      awo->state = AWC_ONE_SELECTED;
      awo->useFirstSelectedAsReference = 1;
      awo->updateMasterSelection();

      awo->refresh();

      break;

    case AWC_POPUP_UNGROUP:

      awo->undoObj.flush();

      awo->setChanged();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        nextSel = curSel->selFlink;

        curSel->node->ungroup( (void *) curSel );

        curSel = nextSel;

      }

      // determine new state
      num_selected = 0;

      curSel = awo->selectedHead->selFlink;
      while ( ( curSel != awo->selectedHead ) &&
              ( num_selected < 2 ) ) {

        num_selected++;
        curSel = curSel->selFlink;

      }

      if ( num_selected == 0 ) {
        awo->state = AWC_NONE_SELECTED;
        awo->updateMasterSelection();
      }
      else if ( num_selected == 1 ) {
        awo->state = AWC_ONE_SELECTED;
        awo->useFirstSelectedAsReference = 1;
        awo->updateMasterSelection();
      }
      else {
        awo->state = AWC_MANY_SELECTED;
        awo->updateMasterSelection();
      }

      awo->refresh();

      break;

    case AWC_POPUP_ROTATE_CW:

      rotate( awo, '+' );
      break;

    case AWC_POPUP_ROTATE_CCW:

      rotate( awo, '-' );
      break;

    case AWC_POPUP_FLIP_H:

      flip( awo, 'H' );
      break;

    case AWC_POPUP_FLIP_V:

      flip( awo, 'V' );
      break;

    case AWC_POPUP_UNDO:

      stat = undo( awo );
      if ( !( stat & 1 ) ) XBell( awo->d, 50 );
      break;

    case AWC_POPUP_REFRESH:

      awo->clear();
      awo->refresh();

      break;

  }

}

static void b2ReleaseManySelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo;
popupBlockPtr block;
int i, item, deltaX, deltaY, leftmost, rightmost, topmost, botmost, n,
 curY0, curY1, curX0, curX1, minY, maxY, minX, maxX, midX, midY,
 stat, num_selected, width, height;
double space, totalSpace, dY0, dX0, resid;
activeGraphicListPtr cur, curSel, nextSel, topmostNode, leftmostNode;

  block = (popupBlockPtr) client;
  item = (int) block->ptr;
  awo = (activeWindowClass *) block->awo;

  switch ( item ) {

    case AWC_POPUP_DESELECT:

      // deselect all
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        cur->node->deselect();
        cur->node->drawSelectBoxCorners(); // erase via xor gc
        cur = cur->selFlink;
      }
      // make list empty
      awo->selectedHead->selFlink = awo->selectedHead;
      awo->selectedHead->selBlink = awo->selectedHead;
      awo->state = AWC_NONE_SELECTED;
      awo->updateMasterSelection();

      awo->refresh();

      break;

    case AWC_POPUP_GROUP:

      awo->undoObj.flush();

      awo->setChanged();

      cur = new activeGraphicListType;
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;
      cur->node = new activeGroupClass;
      stat = cur->node->createGroup( awo );

      // link into main list
      cur->blink = awo->head->blink;
      awo->head->blink->flink = cur;
      awo->head->blink = cur;
      cur->flink = awo->head;

      // link into selected list (which has been emptied by the
      // createGroup operation
      cur->selBlink = awo->selectedHead->selBlink;
      awo->selectedHead->selBlink->selFlink = cur;
      awo->selectedHead->selBlink = cur;
      cur->selFlink = awo->selectedHead;

      cur->node->setSelected();

      awo->state = AWC_ONE_SELECTED;
      awo->useFirstSelectedAsReference = 1;
      awo->updateMasterSelection();

      awo->refresh();

      break;

    case AWC_POPUP_UNGROUP:

      awo->undoObj.flush();

      awo->setChanged();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        nextSel = curSel->selFlink;

        curSel->node->ungroup( (void *) curSel );

        curSel = nextSel;

      }

      // determine new state
      num_selected = 0;

      curSel = awo->selectedHead->selFlink;
      while ( ( curSel != awo->selectedHead ) &&
              ( num_selected < 2 ) ) {

        num_selected++;
        curSel = curSel->selFlink;

      }

      if ( num_selected == 0 ) {
        awo->state = AWC_NONE_SELECTED;
        awo->updateMasterSelection();
      }
      else if ( num_selected == 1 ) {
        awo->state = AWC_ONE_SELECTED;
        awo->useFirstSelectedAsReference = 1;
        awo->updateMasterSelection();
      }
      else {
        awo->state = AWC_MANY_SELECTED;
        awo->updateMasterSelection();
      }

      awo->refresh();

      break;

    case AWC_POPUP_RAISE:

      raise( awo );
      break;

#if 0
      // remove node and insert at tail

      awo->setChanged();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        // remove
        curSel->blink->flink = curSel->flink;
        curSel->flink->blink = curSel->blink;

        // insert at tail
        curSel->blink = awo->head->blink;
        awo->head->blink->flink = curSel;
        awo->head->blink = curSel;
        curSel->flink = awo->head;

        curSel->node->eraseSelectBoxCorners();

        curSel = curSel->selFlink;

      }

      curSel = awo->selectedHead->selFlink;
      if ( curSel ) curSel->node->drawAll();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->drawSelectBoxCorners();

        curSel = curSel->selFlink;

      }

      break;
#endif

    case AWC_POPUP_ROTATE_CW:

      rotate( awo, '+' );
      break;

    case AWC_POPUP_ROTATE_CCW:

      rotate( awo, '-' );
      break;

    case AWC_POPUP_FLIP_H:

      flip( awo, 'H' );
      break;

    case AWC_POPUP_FLIP_V:

      flip( awo, 'V' );
      break;

    case AWC_POPUP_LOWER: // lower

      lower( awo );
      break;

#if 0
      // remove node and insert at head

      awo->setChanged();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        // remove
        curSel->blink->flink = curSel->flink;
        curSel->flink->blink = curSel->blink;

        // insert at tail
        curSel->flink = awo->head->flink;
        awo->head->flink->blink = curSel;
        awo->head->flink = curSel;
        curSel->blink = awo->head;

        curSel->node->eraseSelectBoxCorners();

        curSel = curSel->selFlink;

      }

      curSel = awo->selectedHead->selFlink;
      if ( curSel ) curSel->node->drawAll();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->drawSelectBoxCorners();

        curSel = curSel->selFlink;

      }

      break;
#endif

    case AWC_POPUP_CUT:

      cut( awo );
      awo->refresh();
      break;

#if 0
      // remove nodes and put on cut list; if cur list is not
      // empty the delete all nodes

      awo->undoObj.flush();

      awo->setChanged();

      // empty cut list
      curCut = awo->appCtx->cutHead1->flink;
      while ( curCut != awo->appCtx->cutHead1 ) {
        nextCut = curCut->flink;
        delete curCut->node;
        delete curCut;
        curCut = nextCut;
      }

      awo->appCtx->cutHead1->flink = awo->appCtx->cutHead1;
      awo->appCtx->cutHead1->blink = awo->appCtx->cutHead1;

      // remove nodes off main list
      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        // deselect
        curSel->node->deselect();

        // remove
        curSel->blink->flink = curSel->flink;
        curSel->flink->blink = curSel->blink;

        // insert into cut list
        curSel->blink = awo->appCtx->cutHead1->blink;
        awo->appCtx->cutHead1->blink->flink = curSel;
        awo->appCtx->cutHead1->blink = curSel;
        curSel->flink = awo->appCtx->cutHead1;

        curSel = curSel->selFlink;

      }

      // make selected list empty
      awo->selectedHead->selFlink = awo->selectedHead;
      awo->selectedHead->selBlink = awo->selectedHead;

      awo->state = AWC_NONE_SELECTED;
      awo->updateMasterSelection();

      awo->refresh();

      break;
#endif

    case AWC_POPUP_COPY:

      copy( awo );
      awo->refresh();
      break;

#if 0
      // copy nodes to cut list; if cur cut list is not
      // empty the delete all nodes

      // empty cut list
      curCut = awo->appCtx->cutHead1->flink;
      while ( curCut != awo->appCtx->cutHead1 ) {
        nextCut = curCut->flink;
        delete curCut->node;
        delete curCut;
        curCut = nextCut;
      }

      awo->appCtx->cutHead1->flink = awo->appCtx->cutHead1;
      awo->appCtx->cutHead1->blink = awo->appCtx->cutHead1;

      // copy nodes to cut list
      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();

        // deselect
        curSel->node->deselect();

        newOne = new activeGraphicListType;
        newOne->defExeFlink = NULL;
        newOne->defExeBlink = NULL;
        newOne->node = awo->obj.clone( curSel->node->objName(), curSel->node );

        // copy to cut list
        newOne->blink = awo->appCtx->cutHead1->blink;
        awo->appCtx->cutHead1->blink->flink = newOne;
        awo->appCtx->cutHead1->blink = newOne;
        newOne->flink = awo->appCtx->cutHead1;

        curSel = curSel->selFlink;

      }

      // make selected list empty
      awo->selectedHead->selFlink = awo->selectedHead;
      awo->selectedHead->selBlink = awo->selectedHead;

      awo->state = AWC_NONE_SELECTED;
      awo->updateMasterSelection();

      awo->refresh();

      break;
#endif

    case AWC_POPUP_ALIGN_LEFT:

      awo->undoObj.startNewUndoList( activeWindowClass_str172 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      curSel = awo->selectedHead->selFlink;
      leftmost = curSel->node->getX0();
      while ( curSel != awo->selectedHead ) {

        if ( curSel->node->getX0() < leftmost )
          leftmost = curSel->node->getX0();

        curSel = curSel->selFlink;

      }

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        deltaX = leftmost - curSel->node->getX0();
        curSel->node->move( deltaX, 0 );
        curSel->node->moveSelectBox( deltaX, 0 );

        curSel = curSel->selFlink;

      }

      awo->refresh();


      break;

    case AWC_POPUP_ALIGN_RIGHT:

      awo->undoObj.startNewUndoList( activeWindowClass_str172 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      curSel = awo->selectedHead->selFlink;
      rightmost = curSel->node->getX1();
      while ( curSel != awo->selectedHead ) {

        if ( curSel->node->getX1() > rightmost )
          rightmost = curSel->node->getX1();

        curSel = curSel->selFlink;

      }

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        deltaX = rightmost - curSel->node->getX1();
        curSel->node->move( deltaX, 0 );
        curSel->node->moveSelectBox( deltaX, 0 );

        curSel = curSel->selFlink;

      }

      awo->refresh();

      break;

    case AWC_POPUP_ALIGN_TOP:

      awo->undoObj.startNewUndoList( activeWindowClass_str172 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      curSel = awo->selectedHead->selFlink;
      topmost = curSel->node->getY0();
      while ( curSel != awo->selectedHead ) {

        if ( curSel->node->getY0() < topmost )
          topmost = curSel->node->getY0();

        curSel = curSel->selFlink;

      }

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        deltaY = topmost - curSel->node->getY0();
        curSel->node->move( 0, deltaY );
        curSel->node->moveSelectBox( 0, deltaY );

        curSel = curSel->selFlink;

      }

      awo->refresh();

      break;

    case AWC_POPUP_ALIGN_BOTTOM:

      awo->undoObj.startNewUndoList( activeWindowClass_str172 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      curSel = awo->selectedHead->selFlink;
      botmost = curSel->node->getY1();
      while ( curSel != awo->selectedHead ) {

        if ( curSel->node->getY1() > botmost )
          botmost = curSel->node->getY1();

        curSel = curSel->selFlink;

      }

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        deltaY = botmost - curSel->node->getY1();
        curSel->node->move( 0, deltaY );
        curSel->node->moveSelectBox( 0, deltaY );

        curSel = curSel->selFlink;

      }

      awo->refresh();

      break;

    case AWC_POPUP_ALIGN_CENTER:

      awo->undoObj.startNewUndoList( activeWindowClass_str175 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      if ( awo->useFirstSelectedAsReference ) {

        // use the first selected node as the x axis value

        curSel = awo->selectedHead->selFlink;
        topmostNode = curSel;

      }
      else {

        // use the topmost node as the x axis value

        curSel = awo->selectedHead->selFlink;
        topmost = curSel->node->getY0();
        topmostNode = curSel;
        while ( curSel != awo->selectedHead ) {

          if ( curSel->node->getY0() < topmost ) {
            topmost = curSel->node->getY0();
            topmostNode = curSel;
          }

          curSel = curSel->selFlink;

        }

      }

      midX = topmostNode->node->getXMid();
      midY = topmostNode->node->getYMid();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        curSel->node->moveMidpointAbs( midX, midY );
        curSel->node->moveSelectBoxMidpointAbs( midX, midY );

        curSel = curSel->selFlink;

      }

      awo->refresh();

      break;

    case AWC_POPUP_ALIGN_CENTER_VERT:

      awo->undoObj.startNewUndoList( activeWindowClass_str175 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      if ( awo->useFirstSelectedAsReference ) {

        // use the first selected node as the x axis value

        curSel = awo->selectedHead->selFlink;
        topmostNode = curSel;

      }
      else {

        // use the topmost node as the x axis value

        curSel = awo->selectedHead->selFlink;
        topmost = curSel->node->getY0();
        topmostNode = curSel;
        while ( curSel != awo->selectedHead ) {

          if ( curSel->node->getY0() < topmost ) {
            topmost = curSel->node->getY0();
            topmostNode = curSel;
          }

          curSel = curSel->selFlink;

        }

      }

      midX = topmostNode->node->getXMid();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        midY = curSel->node->getYMid();
        curSel->node->moveMidpointAbs( midX, midY );
        curSel->node->moveSelectBoxMidpointAbs( midX, midY );

        curSel = curSel->selFlink;

      }

      awo->refresh();

      break;

    case AWC_POPUP_ALIGN_CENTER_HORZ:

      awo->undoObj.startNewUndoList( activeWindowClass_str175 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      if ( awo->useFirstSelectedAsReference ) {

        // use the first selected node as the y axis value

        curSel = awo->selectedHead->selFlink;
        leftmostNode = curSel;

      }
      else {

        // use the leftmost node as the y axis value

        curSel = awo->selectedHead->selFlink;
        leftmost = curSel->node->getX0();
        leftmostNode = curSel;
        while ( curSel != awo->selectedHead ) {

          if ( curSel->node->getX0() < leftmost ) {
            leftmost = curSel->node->getX0();
            leftmostNode = curSel;
          }

          curSel = curSel->selFlink;

        }

      }

      midY = leftmostNode->node->getYMid();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        midX = curSel->node->getXMid();
        curSel->node->moveMidpointAbs( midX, midY );
        curSel->node->moveSelectBoxMidpointAbs( midX, midY );

        curSel = curSel->selFlink;

      }

      awo->refresh();

      break;


    case AWC_POPUP_ALIGN_SIZE:

      awo->undoObj.startNewUndoList( activeWindowClass_str173 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoResizeNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      if ( awo->useFirstSelectedAsReference ) {

        // use the first selected node as the width & height value

        curSel = awo->selectedHead->selFlink;
        topmostNode = curSel;

      }
      else {

        // use the topmost node as the width & height value

        curSel = awo->selectedHead->selFlink;
        topmost = curSel->node->getY0();
        topmostNode = curSel;
        while ( curSel != awo->selectedHead ) {

          if ( curSel->node->getY0() < topmost ) {
            topmost = curSel->node->getY0();
            topmostNode = curSel;
          }

          curSel = curSel->selFlink;

        }

      }

      width = topmostNode->node->getW();
      height = topmostNode->node->getH();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        curSel->node->resizeAbs( -1, -1, width, height );
        curSel->node->resizeSelectBoxAbs( -1, -1, width, height );

        curSel = curSel->selFlink;

      }

      awo->refresh();

      break;

    case AWC_POPUP_ALIGN_SIZE_HORZ:

      awo->undoObj.startNewUndoList( activeWindowClass_str173 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoResizeNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      if ( awo->useFirstSelectedAsReference ) {

        // use the first selected node as the width value

        curSel = awo->selectedHead->selFlink;
        topmostNode = curSel;

      }
      else {

        // use the topmost node as the width value

        curSel = awo->selectedHead->selFlink;
        topmost = curSel->node->getY0();
        topmostNode = curSel;
        while ( curSel != awo->selectedHead ) {

          if ( curSel->node->getY0() < topmost ) {
            topmost = curSel->node->getY0();
            topmostNode = curSel;
          }

          curSel = curSel->selFlink;

        }

      }

      width = topmostNode->node->getW();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        curSel->node->resizeAbs( -1, -1, width, -1 );
        curSel->node->resizeSelectBoxAbs( -1, -1, width, -1 );

        curSel = curSel->selFlink;

      }

      awo->refresh();

      break;

    case AWC_POPUP_ALIGN_SIZE_VERT:

      awo->undoObj.startNewUndoList( activeWindowClass_str173 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoResizeNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      if ( awo->useFirstSelectedAsReference ) {

        // use the first selected node as the height value

        curSel = awo->selectedHead->selFlink;
        leftmostNode = curSel;

      }
      else {

        // use the leftmost node as the height value

        curSel = awo->selectedHead->selFlink;
        leftmost = curSel->node->getX0();
        leftmostNode = curSel;
        while ( curSel != awo->selectedHead ) {

          if ( curSel->node->getX0() < leftmost ) {
            leftmost = curSel->node->getX0();
            leftmostNode = curSel;
          }

          curSel = curSel->selFlink;

        }

      }

      height = leftmostNode->node->getH();

      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        curSel->node->eraseSelectBoxCorners();
        curSel->node->erase();

        curSel->node->resizeAbs( -1, -1, -1, height );
        curSel->node->resizeSelectBoxAbs( -1, -1, -1, height );

        curSel = curSel->selFlink;

      }

      awo->refresh();

      break;


    case AWC_POPUP_DISTRIBUTE_VERTICALLY:

      awo->undoObj.startNewUndoList( activeWindowClass_str174 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      totalSpace = 0.0;
      n = 0;
      curSel = awo->selectedHead->selFlink;
      minY = curSel->node->getY0();
      maxY = curSel->node->getY1();
      while ( curSel != awo->selectedHead ) {

        totalSpace += curSel->node->getY1() - curSel->node->getY0();

        if ( curSel->node->getY0() < minY ) minY = curSel->node->getY0();
        if ( curSel->node->getY1() > maxY ) maxY = curSel->node->getY1();

        n++;

        curSel = curSel->selFlink;

      }

      if ( n > awo->list_array_size ) {
        delete awo->list_array;
        awo->list_array_size = n;
        awo->list_array = new activeGraphicListType[n];
        awo->list_array->defExeFlink = NULL;
        awo->list_array->defExeBlink = NULL;
      }

      i = 0;
      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        if ( i < n ) {
          awo->list_array[i] = *curSel;
          i++;
        }

        curSel = curSel->selFlink;

      }

      qsort( (void *) awo->list_array, n,
       sizeof( activeGraphicListType ), qsort_compare_y_func );

      if ( n >= 2 ) {
        space = ( (double) maxY - (double) minY - totalSpace ) /
         ( (double) n - 1.0 );
      }
      else {
        space = 0.0;
      }

      curY0 = (awo->list_array[0]).node->getY0();
      curY1 = (awo->list_array[0]).node->getY1();
      resid = 0.0;

      for ( i=1; i<n-1; i++ ) {

        curX0 = (awo->list_array[i]).node->getX0();
        dY0 = (double) curY1 + space;
        curY0 = (int) rint(dY0);
        resid = resid + dY0 - curY0;
        if ( resid > 1.0 ) {
          curY0 += 1;
          resid -= 1.0;
        }
        else if ( resid < -1.0 ) {
          curY0 -= 1;
          resid += 1.0;
        }

        (awo->list_array[i]).node->moveAbs( curX0, curY0 );
        (awo->list_array[i]).node->moveSelectBoxAbs( curX0, curY0 );
        curY1 = (awo->list_array[i]).node->getY1();

      }

      awo->clear();
      awo->refresh();

      break;

    case AWC_POPUP_DISTRIBUTE_MIDPT_VERTICALLY:

      awo->undoObj.startNewUndoList( activeWindowClass_str174 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      n = 0;
      curSel = awo->selectedHead->selFlink;
      minY = curSel->node->getYMid();
      maxY = curSel->node->getYMid();
      while ( curSel != awo->selectedHead ) {

        if ( curSel->node->getYMid() < minY ) minY = curSel->node->getYMid();
        if ( curSel->node->getYMid() > maxY ) maxY = curSel->node->getYMid();

        n++;

        curSel = curSel->selFlink;

      }

      if ( n > awo->list_array_size ) {
        delete awo->list_array;
        awo->list_array_size = n;
        awo->list_array = new activeGraphicListType[n];
        awo->list_array->defExeFlink = NULL;
        awo->list_array->defExeBlink = NULL;
      }

      i = 0;
      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        if ( i < n ) {
          awo->list_array[i] = *curSel;
          i++;
        }

        curSel = curSel->selFlink;

      }

      qsort( (void *) awo->list_array, n,
       sizeof( activeGraphicListType ), qsort_compare_y_func );

      if ( n >= 2 ) {
        space = ( (double) maxY - (double) minY ) / ( (double) n - 1 );
      }
      else {
        space = 0.0;
      }

      curY1 = (awo->list_array[0]).node->getYMid();

      for ( i=1; i<n-1; i++ ) {

        curX0 = (awo->list_array[i]).node->getXMid();
        dY0 = (double) curY1 + space * (double) i;
        curY0 = (int) rint(dY0);

        (awo->list_array[i]).node->moveMidpointAbs( curX0, curY0 );
        (awo->list_array[i]).node->moveSelectBoxMidpointAbs( curX0, curY0 );

      }

      awo->clear();
      awo->refresh();

      break;

    case AWC_POPUP_DISTRIBUTE_HORIZONTALLY:

      awo->undoObj.startNewUndoList( activeWindowClass_str174 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      totalSpace = 0.0;
      n = 0;
      curSel = awo->selectedHead->selFlink;
      minX = curSel->node->getX0();
      maxX = curSel->node->getX1();
      while ( curSel != awo->selectedHead ) {

        totalSpace += curSel->node->getX1() - curSel->node->getX0();

        if ( curSel->node->getX0() < minX ) minX = curSel->node->getX0();
        if ( curSel->node->getX1() > maxX ) maxX = curSel->node->getX1();

        n++;

        curSel = curSel->selFlink;

      }

      if ( n > awo->list_array_size ) {
        delete awo->list_array;
        awo->list_array_size = n;
        awo->list_array = new activeGraphicListType[n];
        awo->list_array->defExeFlink = NULL;
        awo->list_array->defExeBlink = NULL;
      }

      i = 0;
      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        if ( i < n ) {
          awo->list_array[i] = *curSel;
          i++;
        }

        curSel = curSel->selFlink;

      }

      qsort( (void *) awo->list_array, n,
       sizeof( activeGraphicListType ), qsort_compare_x_func );

      if ( n >= 2 ) {
        space = ( (double) maxX - (double) minX - totalSpace ) /
         ( (double) n - 1.0 );
      }
      else {
        space = 0.0;
      }

      curX0 = (awo->list_array[0]).node->getX0();
      curX1 = (awo->list_array[0]).node->getX1();
      resid = 0.0;

      for ( i=1; i<n-1; i++ ) {

        curY0 = (awo->list_array[i]).node->getY0();
        dX0 = (double) curX1 + space;
        curX0 = (int) rint(dX0);
        resid = resid + dX0 - curX0;
        if ( resid > 1.0 ) {
          curX0 += 1;
          resid -= 1.0;
        }
        else if ( resid < -1.0 ) {
          curX0 -= 1;
          resid += 1.0;
        }

        (awo->list_array[i]).node->moveAbs( curX0, curY0 );
        (awo->list_array[i]).node->moveSelectBoxAbs( curX0, curY0 );
        curX1 = (awo->list_array[i]).node->getX1();

      }

      awo->clear();
      awo->refresh();

      break;

    case AWC_POPUP_DISTRIBUTE_MIDPT_HORIZONTALLY:

      awo->undoObj.startNewUndoList( activeWindowClass_str174 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      n = 0;
      curSel = awo->selectedHead->selFlink;
      minX = curSel->node->getXMid();
      maxX = curSel->node->getXMid();
      while ( curSel != awo->selectedHead ) {

        if ( curSel->node->getXMid() < minX ) minX = curSel->node->getXMid();
        if ( curSel->node->getXMid() > maxX ) maxX = curSel->node->getXMid();

        n++;

        curSel = curSel->selFlink;

      }

      if ( n > awo->list_array_size ) {
        delete awo->list_array;
        awo->list_array_size = n;
        awo->list_array = new activeGraphicListType[n];
        awo->list_array->defExeFlink = NULL;
        awo->list_array->defExeBlink = NULL;
      }

      i = 0;
      curSel = awo->selectedHead->selFlink;
      while ( curSel != awo->selectedHead ) {

        if ( i < n ) {
          awo->list_array[i] = *curSel;
          i++;
        }

        curSel = curSel->selFlink;

      }

      qsort( (void *) awo->list_array, n,
       sizeof( activeGraphicListType ), qsort_compare_x_func );

      if ( n >= 2 ) {
        space = ( (double) maxX - (double) minX ) / ( (double) n - 1 );
      }
      else {
        space = 0;
      }

      curX1 = (awo->list_array[0]).node->getXMid();

      for ( i=1; i<n-1; i++ ) {

        curY0 = (awo->list_array[i]).node->getYMid();
        dX0 = (double) curX1 + space * (double) i;
        curX0 = (int) rint(dX0);

        (awo->list_array[i]).node->moveMidpointAbs( curX0, curY0 );
        (awo->list_array[i]).node->moveSelectBoxMidpointAbs( curX0, curY0 );

      }

      awo->clear();
      awo->refresh();

      break;

    case AWC_POPUP_CHANGE_DSP_PARAMS:

      awo->savedState = awo->state;
      
      awo->state = AWC_EDITING;
      awo->currentEf = NULL;

      awo->ef.create( awo->top, awo->appCtx->ci.getColorMap(),
       &awo->appCtx->entryFormX,
       &awo->appCtx->entryFormY, &awo->appCtx->entryFormW,
       &awo->appCtx->entryFormH, &awo->appCtx->largestH,
       activeWindowClass_str44, NULL, NULL, NULL );

      //awo->ef.addTextField( activeWindowClass_str45, 30, awo->bufTitle, 127 );

      awo->ef.addFontMenu( activeWindowClass_str46, awo->fi, &awo->defaultFm,
       awo->allSelectedFontTag );
      awo->defaultFm.setFontAlignment( awo->allSelectedAlignment );
      awo->ef.addToggle( activeWindowClass_str47,
       &awo->allSelectedFontTagFlag );
      awo->ef.addToggle( activeWindowClass_str48,
       &awo->allSelectedAlignmentFlag );

      awo->ef.addFontMenu( activeWindowClass_str49, awo->fi,
       &awo->defaultCtlFm, awo->allSelectedCtlFontTag );
      awo->defaultCtlFm.setFontAlignment( awo->allSelectedCtlAlignment );
      awo->ef.addToggle( activeWindowClass_str50,
       &awo->allSelectedCtlFontTagFlag );
      awo->ef.addToggle( activeWindowClass_str51,
       &awo->allSelectedCtlAlignmentFlag );

      awo->ef.addFontMenu( activeWindowClass_str52, awo->fi,
       &awo->defaultBtnFm, awo->allSelectedBtnFontTag );
      awo->defaultBtnFm.setFontAlignment( awo->allSelectedBtnAlignment );
      awo->ef.addToggle( activeWindowClass_str53,
       &awo->allSelectedBtnFontTagFlag );
      awo->ef.addToggle( activeWindowClass_str54,
       &awo->allSelectedBtnAlignmentFlag );

      awo->ef.addColorButton( activeWindowClass_str55, awo->ci,
       &awo->defaultTextFgCb, &awo->allSelectedTextFgColor );
      awo->ef.addToggle( activeWindowClass_str56,
       &awo->allSelectedTextFgColorFlag );

      awo->ef.addColorButton( activeWindowClass_str57, awo->ci,
       &awo->defaultFg1Cb, &awo->allSelectedFg1Color );
      awo->ef.addToggle( activeWindowClass_str58,
       &awo->allSelectedFg1ColorFlag );

      awo->ef.addColorButton( activeWindowClass_str59, awo->ci,
       &awo->defaultFg2Cb, &awo->allSelectedFg2Color );
      awo->ef.addToggle( activeWindowClass_str60,
       &awo->allSelectedFg2ColorFlag );

      awo->ef.addColorButton( activeWindowClass_str61, awo->ci,
       &awo->defaultBgCb, &awo->allSelectedBgColor );
      awo->ef.addToggle( activeWindowClass_str62,
       &awo->allSelectedBgColorFlag );

      awo->ef.addColorButton( activeWindowClass_str63, awo->ci,
       &awo->defaultOffsetCb, &awo->allSelectedOffsetColor );
      awo->ef.addToggle( activeWindowClass_str64,
       &awo->allSelectedOffsetColorFlag );

      awo->ef.addColorButton( activeWindowClass_str65, awo->ci,
       &awo->defaultTopShadowCb, &awo->allSelectedTopShadowColor );
      awo->ef.addToggle( activeWindowClass_str66,
       &awo->allSelectedTopShadowColorFlag );

      awo->ef.addColorButton( activeWindowClass_str67, awo->ci,
       &awo->defaultBotShadowCb, &awo->allSelectedBotShadowColor );
      awo->ef.addToggle( activeWindowClass_str68,
       &awo->allSelectedBotShadowColorFlag );

      awo->ef.addToggle( activeWindowClass_str187,
       &awo->useComponentScheme );

      awo->ef.finished( awc_change_dsp_edit_ok, awc_change_dsp_edit_apply,
       awc_change_dsp_edit_cancel, awo );
      awo->ef.popup();

      break;

    case AWC_POPUP_CHANGE_PV_NAMES:

      awo->savedState = awo->state;
      
      awo->state = AWC_EDITING;
      awo->currentEf = NULL;

      awo->ef.create( awo->top, awo->appCtx->ci.getColorMap(),
       &awo->appCtx->entryFormX,
       &awo->appCtx->entryFormY, &awo->appCtx->entryFormW,
       &awo->appCtx->entryFormH, &awo->appCtx->largestH,
       activeWindowClass_str69, NULL, NULL, NULL );

      awo->ef.addTextField( activeWindowClass_str70, 30, awo->allSelectedCtlPvName[0], 127 );
      awo->ef.addToggle( activeWindowClass_str71, &awo->allSelectedCtlPvNameFlag );

      awo->ef.addTextField( activeWindowClass_str72, 30, awo->allSelectedReadbackPvName[0],
       127 );
      awo->ef.addToggle( activeWindowClass_str73, &awo->allSelectedReadbackPvNameFlag );

      awo->ef.addTextField( activeWindowClass_str74, 30, awo->allSelectedNullPvName[0], 127 );
      awo->ef.addToggle( activeWindowClass_str75, &awo->allSelectedNullPvNameFlag );

      awo->ef.addTextField( activeWindowClass_str76, 30, awo->allSelectedVisPvName[0],
       127 );
      awo->ef.addToggle( activeWindowClass_str77, &awo->allSelectedVisPvNameFlag );

      awo->ef.addTextField( activeWindowClass_str78, 30, awo->allSelectedAlarmPvName[0], 127 );
      awo->ef.addToggle( activeWindowClass_str79, &awo->allSelectedAlarmPvNameFlag );

      awo->ef.finished( awc_change_pv_edit_ok, awc_change_pv_edit_apply,
       awc_change_pv_edit_cancel, awo );
      awo->ef.popup();

      break;

    case AWC_POPUP_UNDO:

      stat = undo( awo );
      if ( !( stat & 1 ) ) XBell( awo->d, 50 );
      break;

    case AWC_POPUP_REFRESH:

      awo->clear();
      awo->refresh();

      break;

  }

}

static void createPopup_cb (
   Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo;
objNameListPtr curObjNameNode;
activeGraphicListPtr cur;
int stat;
char schemeFile[255+1];

  // find object name

  awo = (activeWindowClass *) client;

  curObjNameNode = awo->objNameHead->flink;
  while ( curObjNameNode != awo->objNameHead ) {
    if ( curObjNameNode->w == w ) {

      awo->appCtx->getScheme( awo->curSchemeSet, curObjNameNode->objName,
       curObjNameNode->objType, schemeFile, 255 );
      if ( strcmp( schemeFile, "" ) != 0 ) {
        //printf( "load new scheme [%s]\n", schemeFile );
        stat = awo->loadComponentScheme( schemeFile );
        if ( !( stat & 1 ) ) {
          awo->loadComponentScheme( "default" );
        }
      }

      // create object

      cur = new activeGraphicListType;
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;
      cur->node = awo->obj.createNew( curObjNameNode->objName );

      cur->node->setObjType( curObjNameNode->objType );

      cur->blink = awo->head->blink;
      awo->head->blink->flink = cur;
      awo->head->blink = cur;
      cur->flink = awo->head;

      if ( cur->node->objName() ) {

        awo->savedState = awo->state;
        awo->state = AWC_EDITING;
        awo->currentEf = NULL;

        stat = cur->node->createInteractive( awo, awo->startx,
         awo->starty, awo->width, awo->height );

        cur->node->initSelectBox();

      }

      break;

    }
    curObjNameNode = curObjNameNode->flink;
  }

}

static void topWinEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

XConfigureEvent *ce;
activeWindowClass *awo;
int check_origin;

  awo = (activeWindowClass *) client;

  *continueToDispatch = True;

  if ( e->type == ConfigureNotify ) {

    ce = (XConfigureEvent *) e;

    check_origin = 0;

    if ( ( ce->width > 1 ) && ( ce->height > 1 ) ) {

      if ( ( awo->w != ce->width ) ||
           ( awo->h != ce->height ) ) {

        awo->w = ce->width;
        awo->h = ce->height;

      }
      else {

        check_origin = 1;

      }

    }

    if ( check_origin ) {

      if ( ( ce->x != 0 ) && ( ce->y != 0 ) &&
           ( awo->w > 0 ) && ( awo->h > 0 ) &&
           ( awo->x != 0 ) && ( awo->y != 0 ) ) {

        if ( ( awo->x != ce->x ) ||
             ( awo->y != ce->y ) ) {

          awo->x = ce->x;
          awo->y = ce->y;

        }

      }

    }

  } // end of ConfigureNotify

}

static void drawWinEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

XExposeEvent *expe;
XMotionEvent *me;
XButtonEvent *be;
XKeyEvent *ke;
XConfigureEvent *ce;
activeWindowClass *awo;
int width, height, deltax, deltay, deltaw, deltah, operation, ok, stat,
 x0, y0, selectMustEnclose, num_selected, prev_num_selected, rawX, rawY;
activeGraphicListPtr cur, curSel, editNode, start;
int gotSelection, wasSelected, totalSelected, operationPerformed;
KeySym key;
char keyBuf[20];
const int keyBufSize = 20;
XComposeStatus compose;
int charCount, xInc, yInc, foundNewOneToEdit, isMultiPoint, doAgain;
double xScaleFactor, yScaleFactor, newX, newW, newY, newH;
 char msg[31+1];

Window root, child;
int rootX, rootY, winX, winY;
unsigned int mask;

  awo = (activeWindowClass *) client;

  *continueToDispatch = False;

  if ( awo->mode != AWC_EDIT ) return;

  awo->oldState = awo->state;

  if ( e->type != MotionNotify ) {
    if ( awo->msgDialogPoppedUp ) {
      awo->msgDialog.popdown();
      awo->msgDialogPoppedUp = 0;
    }
  }

  if ( e->type == ConfigureNotify ) {

    ce = (XConfigureEvent *) e;

    if ( ( awo->w != ce->width ) ||
         ( awo->h != ce->height ) ) {

      awo->w = ce->width;
      awo->h = ce->height;

    }

    if ( ( awo->x != ce->x ) ||
         ( awo->y != ce->y ) ) {

        awo->x = ce->x;
        awo->y = ce->y;

    }

  }
  else if ( e->type == Expose ) {

    expe = (XExposeEvent *) e;

      awo->refresh( expe->x, expe->y, expe->width, expe->height );

  }
  else if ( e->type == KeyPress ) {

    if ( awo->state == AWC_START_DEFINE_REGION ) goto done;
    if ( awo->state == AWC_DEFINE_REGION ) goto done;
    if ( awo->state == AWC_EDITING ) goto done;
    if ( awo->state == AWC_START_DEFINE_SELECT_REGION ) goto done;
    if ( awo->state == AWC_DEFINE_SELECT_REGION ) goto done;
    if ( awo->state == AWC_EDITING_POINTS ) goto done;
    if ( awo->state == AWC_MOVING_POINT ) goto done;
    if ( awo->state == AWC_CHOOSING_LINE_OP ) goto done;
    if ( awo->state == AWC_WAITING ) goto done;

    ke = (XKeyEvent *) e;

    xInc = 0;
    yInc = 0;

    charCount = XLookupString( ke, keyBuf, keyBufSize, &key, &compose );

    if ( key == XK_M ) {
      awo->orthoMove = 1;
    }
    else if ( key == XK_m ) {
      awo->orthoMove = 0;
    }
    else if ( key == XK_L ) {
      awo->orthogonal = 1;
    }
    else if ( key == XK_l ) {
      awo->orthogonal = 0;
    }
    else if ( key == XK_G ) {
      awo->gridShow = 1;
      strcpy( awo->gridShowStr, activeWindowClass_str3 ); // Yes
      awo->clear();
      awo->refresh();
    }
    else if ( key == XK_g ) {
      awo->gridShow = 0;
      strcpy( awo->gridShowStr, activeWindowClass_str5 ); // No
      awo->clear();
      awo->refresh();
    }
    else if ( key == XK_S ) {
      awo->gridActive = 1;
      strcpy( awo->gridActiveStr, activeWindowClass_str3 ); // Yes
    }
    else if ( key == XK_s ) {
      awo->gridActive = 0;
      strcpy( awo->gridActiveStr, activeWindowClass_str5 ); // No
    }
    else if ( key == XK_x ) {
      cut( awo );
      awo->refresh();
    }
    else if ( key == XK_c ) {
      copy( awo );
      awo->refresh();
    }
    else if ( key == XK_v ) {
      XQueryPointer( awo->d, XtWindow(awo->top), &root, &child,
       &rootX, &rootY, &winX, &winY, &mask );
      paste( winX, winY, AWC_POPUP_PASTE, awo );
      awo->refresh();
    }
    else if ( key == XK_V ) {
      XQueryPointer( awo->d, XtWindow(awo->top), &root, &child,
       &rootX, &rootY, &winX, &winY, &mask );
      paste( winX, winY, AWC_POPUP_PASTE_IN_PLACE, awo );
      awo->refresh();
    }
    else if ( key == XK_u ) {
      raise( awo );
    }
    else if ( key == XK_d ) {
      lower( awo );
    }
    else if ( key == XK_z ) {
      stat = undo( awo );
      if ( !( stat & 1 ) ) XBell( awo->d, 50 );
    }

    if ( ( awo->state == AWC_ONE_SELECTED ) ||
	 ( awo->state == AWC_MANY_SELECTED ) ) {

      if ( ( key == XK_Left ) ||
           ( key == XK_Right ) ||
           ( key == XK_Up ) ||
	   ( key == XK_Down ) ) {

        XQueryPointer( awo->d, XtWindow(awo->top), &root, &child,
         &rootX, &rootY, &winX, &winY, &mask );

        cur = awo->selectedHead->selFlink;
        operation = 0;
        while ( ( cur != awo->selectedHead ) && !operation ) {
          operation = cur->node->getSelectBoxOperation( winX, winY );
          cur = cur->selFlink;
        }

        if ( operation ) {

          cur = awo->selectedHead->selFlink;
          while ( cur != awo->selectedHead ) {
            cur->node->erase();
            cur = cur->selFlink;
          }

          cur = awo->selectedHead->selFlink;
          while ( cur != awo->selectedHead ) {
            cur->node->drawSelectBoxCorners();
            cur->node->drawSelectBox();
            cur = cur->selFlink;
          }

          awo->savedState = awo->state;
          awo->usingArrowKeys = 1;
          awo->startx = winX;
          awo->starty = winY;
          awo->oldx = winX;
          awo->oldy = winY;

          if ( operation == AGC_MOVE_OP ) {

            awo->undoObj.startNewUndoList( activeWindowClass_str170 );
            cur = awo->selectedHead->selFlink;
            while ( cur != awo->selectedHead ) {
              stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
              cur = cur->selFlink;
            }

            awo->state = AWC_MOVE;

	  }
          else {

            awo->undoObj.startNewUndoList( activeWindowClass_str171 );
            cur = awo->selectedHead->selFlink;
            while ( cur != awo->selectedHead ) {
              stat = cur->node->addUndoResizeNode( &(awo->undoObj) );
              cur = cur->selFlink;
            }

            if ( operation == AGC_LEFT_OP ) {

              awo->state = AWC_RESIZE_LEFT;

            }
            else if ( operation == AGC_TOP_OP ) {

              awo->state = AWC_RESIZE_TOP;

            }
            else if ( operation == AGC_BOTTOM_OP ) {

              awo->state = AWC_RESIZE_BOTTOM;

            }
            else if ( operation == AGC_RIGHT_OP ) {

              awo->state = AWC_RESIZE_RIGHT;

            }
            else if ( operation == AGC_LEFT_TOP_OP ) {

              awo->state = AWC_RESIZE_LEFT_TOP;

            }
            else if ( operation == AGC_LEFT_BOTTOM_OP ) {

              awo->state = AWC_RESIZE_LEFT_BOTTOM;

            }
            else if ( operation == AGC_RIGHT_TOP_OP ) {

              awo->state = AWC_RESIZE_RIGHT_TOP;

            }
            else if ( operation == AGC_RIGHT_BOTTOM_OP ) {

              awo->state = AWC_RESIZE_RIGHT_BOTTOM;

            }

          }

	}

      }

    }

    if ( key == XK_Left ) {
      xInc = -1;
    }
    else if ( key == XK_Right ) {
      xInc = 1;
    }
    else if ( key == XK_Up ) {
      yInc = -1;
    }
    else if ( key == XK_Down ) {
      yInc = 1;
    }
    else {
      goto done;
    }

    switch ( awo->state ) {

      case AWC_MOVE:

        awo->usingArrowKeys = 1;

        awo->setChanged();

        deltax = xInc;
        deltay = yInc;

        cur = awo->selectedHead->selFlink;
        while ( cur != awo->selectedHead ) {
          deltax = xInc;
          deltay = yInc;
          cur->node->drawSelectBox(); //erase via xor gc
          cur->node->moveSelectBox( deltax, deltay );
          cur->node->drawSelectBox();
          cur = cur->selFlink;
        }
        awo->oldx += xInc;
        awo->oldy += yInc;

        break;

      case AWC_RESIZE_LEFT:

        awo->usingArrowKeys = 1;

        awo->setChanged();

        xScaleFactor = 1.0 + (double ) ( awo->startx - awo->oldx - xInc ) /
         (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

        ok = 1;
        cur = awo->selectedHead->selFlink;
        while ( cur != awo->selectedHead ) {

          newX = awo->masterSelectX0 - awo->startx + awo->oldx + xInc +
           (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
           * xScaleFactor + 0.5 );

          newW =
           (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

          stat = cur->node->checkResizeSelectBoxAbs( (int) newX, -1,
           (int) newW, -1 );

          if ( stat == 0 ) ok = 0;

          cur = cur->selFlink;

        }

        if ( ok ) {

          cur = awo->selectedHead->selFlink;
          while ( cur != awo->selectedHead ) {

            newX = awo->masterSelectX0 - awo->startx + awo->oldx + xInc +
             (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
             * xScaleFactor + 0.5 );

            newW =
             (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

            cur->node->drawSelectBox(); //erase via xor gc
            cur->node->resizeSelectBoxAbs( (int) newX, -1, (int) newW,
             -1 );
            cur->node->drawSelectBox();

            cur = cur->selFlink;
          }

          awo->oldx += xInc;

        }

        break;

      case AWC_RESIZE_TOP:

        awo->usingArrowKeys = 1;

        awo->setChanged();

        yScaleFactor = 1.0 + (double) ( awo->starty - awo->oldy - yInc ) /
         (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

        ok = 1;
        cur = awo->selectedHead->selFlink;
        while ( cur != awo->selectedHead ) {

          newY = awo->masterSelectY0 - awo->starty + awo->oldy + yInc +
           (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
           * yScaleFactor + 0.5 );

          newH =
           (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

          stat = cur->node->checkResizeSelectBoxAbs( -1, (int) newY, -1,
           (int) newH );

          if ( stat == 0 ) ok = 0;

          cur = cur->selFlink;

        }

        if ( ok ) {

          cur = awo->selectedHead->selFlink;
          while ( cur != awo->selectedHead ) {

            newY = awo->masterSelectY0 - awo->starty + awo->oldy + yInc +
             (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
             * yScaleFactor + 0.5 );

            newH =
             (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

            cur->node->drawSelectBox(); //erase via xor gc
            cur->node->resizeSelectBoxAbs( -1, (int) newY, -1, (int) newH );
            cur->node->drawSelectBox();

            cur = cur->selFlink;

          }

          awo->oldy += yInc;

        }

        break;

      case AWC_RESIZE_BOTTOM:

        awo->usingArrowKeys = 1;

        awo->setChanged();

        yScaleFactor = 1.0 + (double ) ( awo->oldy + yInc - awo->starty ) /
         (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

        ok = 1;
        cur = awo->selectedHead->selFlink;
        while ( cur != awo->selectedHead ) {

          newY = awo->masterSelectY0 +
           (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
           * yScaleFactor + 0.5 );

          newH =
           (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

          stat = cur->node->checkResizeSelectBoxAbs( -1, (int) newY,
           -1, (int) newH );

          if ( stat == 0 ) ok = 0;

          cur = cur->selFlink;

        }

        if ( ok ) {

          cur = awo->selectedHead->selFlink;
          while ( cur != awo->selectedHead ) {

            newY = awo->masterSelectY0 +
             (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
             * yScaleFactor + 0.5 );

            newH =
             (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

            cur->node->drawSelectBox(); //erase via xor gc
            cur->node->resizeSelectBoxAbs( -1, (int) newY, -1, (int) newH );
            cur->node->drawSelectBox();

            cur = cur->selFlink;

          }

          awo->oldy += yInc;

        }

        break;

      case AWC_RESIZE_RIGHT:

        awo->usingArrowKeys = 1;

        awo->setChanged();

        xScaleFactor = 1.0 + (double ) ( awo->oldx + xInc - awo->startx ) /
         (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

        ok = 1;
        cur = awo->selectedHead->selFlink;
        while ( cur != awo->selectedHead ) {

          newX = awo->masterSelectX0 +
           (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
           * xScaleFactor + 0.5 );

          newW =
           (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

          stat = cur->node->checkResizeSelectBoxAbs( (int) newX, -1,
           (int) newW, -1 );

          if ( stat == 0 ) ok = 0;

          cur = cur->selFlink;

        }

        if ( ok ) {

          cur = awo->selectedHead->selFlink;
          while ( cur != awo->selectedHead ) {

            newX = awo->masterSelectX0 +
             (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
             * xScaleFactor + 0.5 );

            newW =
             (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

            cur->node->drawSelectBox(); //erase via xor gc
            cur->node->resizeSelectBoxAbs( (int) newX, -1, (int) newW,
             -1 );
            cur->node->drawSelectBox();
            cur = cur->selFlink;

          }

          awo->oldx += xInc;

        }

        break;

      case AWC_RESIZE_LEFT_TOP:

        awo->usingArrowKeys = 1;

        awo->setChanged();

        xScaleFactor = 1.0 + (double ) ( awo->startx - awo->oldx - xInc ) /
         (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

        yScaleFactor = 1.0 + (double) ( awo->starty - awo->oldy - yInc ) /
         (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

        ok = 1;
        cur = awo->selectedHead->selFlink;
        while ( cur != awo->selectedHead ) {

          newX = awo->masterSelectX0 - awo->startx + awo->oldx + xInc +
           (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
           * xScaleFactor + 0.5 );

          newW =
           (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

          newY = awo->masterSelectY0 - awo->starty + awo->oldy + yInc +
           (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
           * yScaleFactor + 0.5 );

          newH =
           (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

          stat = cur->node->checkResizeSelectBoxAbs( (int) newX, (int) newY,
           (int) newW, (int) newH );

          if ( stat == 0 ) ok = 0;

          cur = cur->selFlink;

        }

        if ( ok ) {

          cur = awo->selectedHead->selFlink;
          while ( cur != awo->selectedHead ) {

            newX = awo->masterSelectX0 - awo->startx + awo->oldx + xInc +
             (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
             * xScaleFactor + 0.5 );

            newW =
             (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

            newY = awo->masterSelectY0 - awo->starty + awo->oldy + yInc +
             (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
             * yScaleFactor + 0.5 );

            newH =
             (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

            cur->node->drawSelectBox(); //erase via xor gc
            cur->node->resizeSelectBoxAbs( (int) newX, (int) newY, 
             (int) newW, (int) newH );
            cur->node->drawSelectBox();

            cur = cur->selFlink;

          }

          awo->oldx += xInc;
          awo->oldy += yInc;

        }

        break;

      case AWC_RESIZE_LEFT_BOTTOM:

        awo->usingArrowKeys = 1;

        awo->setChanged();

        xScaleFactor = 1.0 + (double ) ( awo->startx - awo->oldx - xInc ) /
         (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

        yScaleFactor = 1.0 + (double ) ( awo->oldy + yInc - awo->starty ) /
         (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

        ok = 1;
        cur = awo->selectedHead->selFlink;
        while ( cur != awo->selectedHead ) {

          newX = awo->masterSelectX0 - awo->startx + awo->oldx + xInc +
           (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
           * xScaleFactor + 0.5 );

          newW =
           (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

          newY = awo->masterSelectY0 +
           (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
           * yScaleFactor + 0.5 );

          newH =
           (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

          stat = cur->node->checkResizeSelectBoxAbs( (int) newX, (int) newY,
           (int) newW, (int) newH );

          if ( stat == 0 ) ok = 0;

          cur = cur->selFlink;

        }

        if ( ok ) {

          cur = awo->selectedHead->selFlink;
          while ( cur != awo->selectedHead ) {

            newX = awo->masterSelectX0 - awo->startx + awo->oldx + xInc +
             (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
             * xScaleFactor + 0.5 );

            newW =
             (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

            newY = awo->masterSelectY0 +
             (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
             * yScaleFactor + 0.5 );

            newH =
             (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

            cur->node->drawSelectBox(); //erase via xor gc
            cur->node->resizeSelectBoxAbs( (int) newX, (int) newY,
             (int) newW, (int) newH );
            cur->node->drawSelectBox();

            cur = cur->selFlink;

          }

          awo->oldx += xInc;
          awo->oldy += yInc;

        }

        break;

      case AWC_RESIZE_RIGHT_TOP:

        awo->usingArrowKeys = 1;

        awo->setChanged();

        xScaleFactor = 1.0 + (double ) ( awo->oldx + xInc - awo->startx ) /
         (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

        yScaleFactor = 1.0 + (double) ( awo->starty - awo->oldy - yInc ) /
         (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

        ok = 1;
        cur = awo->selectedHead->selFlink;
        while ( cur != awo->selectedHead ) {

          newX = awo->masterSelectX0 +
           (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
           * xScaleFactor + 0.5 );

          newW =
           (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

          newY = awo->masterSelectY0 - awo->starty + awo->oldy + yInc +
           (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
           * yScaleFactor + 0.5 );

          newH =
           (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

          stat = cur->node->checkResizeSelectBoxAbs( (int) newX, (int) newY,
           (int) newW, (int) newH );

          if ( stat == 0 ) ok = 0;

          cur = cur->selFlink;

        }

        if ( ok ) {

          cur = awo->selectedHead->selFlink;
          while ( cur != awo->selectedHead ) {

            newX = awo->masterSelectX0 +
             (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
             * xScaleFactor + 0.5 );

            newW =
             (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

            newY = awo->masterSelectY0 - awo->starty + awo->oldy + yInc +
             (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
             * yScaleFactor + 0.5 );

            newH =
             (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

            cur->node->drawSelectBox(); //erase via xor gc
            cur->node->resizeSelectBoxAbs( (int) newX, (int) newY,
             (int) newW, (int) newH );
            cur->node->drawSelectBox();

            cur = cur->selFlink;

          }

          awo->oldx += xInc;
          awo->oldy += yInc;

        }

        break;

      case AWC_RESIZE_RIGHT_BOTTOM:

        awo->usingArrowKeys = 1;

        awo->setChanged();

        xScaleFactor = 1.0 + (double ) ( awo->oldx + xInc - awo->startx ) /
         (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

        yScaleFactor = 1.0 + (double ) ( awo->oldy + yInc - awo->starty ) /
         (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

        ok = 1;
        cur = awo->selectedHead->selFlink;
        while ( cur != awo->selectedHead ) {

          newX = awo->masterSelectX0 +
           (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
           * xScaleFactor + 0.5 );

          newW =
           (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

          newY = awo->masterSelectY0 +
           (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
           * yScaleFactor + 0.5 );

          newH =
           (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

          stat = cur->node->checkResizeSelectBoxAbs( (int) newX, (int) newY,
           (int) newW, (int) newH );

          if ( stat == 0 ) ok = 0;

          cur = cur->selFlink;

        }

        if ( ok ) {

          cur = awo->selectedHead->selFlink;
          while ( cur != awo->selectedHead ) {

            newX = awo->masterSelectX0 +
             (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
             * xScaleFactor + 0.5 );

            newW =
             (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

            newY = awo->masterSelectY0 +
             (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
             * yScaleFactor + 0.5 );

            newH =
             (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

            cur->node->drawSelectBox(); //erase via xor gc
            cur->node->resizeSelectBoxAbs( (int) newX, (int) newY,
             (int) newW, (int) newH );
            cur->node->drawSelectBox();

            cur = cur->selFlink;

          }

          awo->oldx += xInc;
          awo->oldy += yInc;

        }

        break;

    }

  }
  else if ( e->type == ButtonPress ) {

    if ( awo->state == AWC_WAITING ) goto done;

    mask = ShiftMask & ControlMask;

    be = (XButtonEvent *) e;

    awo->deltaTime = be->time - awo->buttonClickTime;
    awo->buttonClickTime = be->time;

    awo->buttonPressX = be->x;
    awo->buttonPressY = be->y;

    switch ( be->button ) {

      case Button1:

        if ( ( be->state & ShiftMask ) || ( awo->deltaTime < 250 ) ) {

//========== Shift B1 Press or double click ========================

          if ( awo->state == AWC_EDITING_POINTS ) {

	    // this is, again, a result of the ugly clash between
	    // rectangular objects and multi-point objects; this time
	    // we set state back to AWC_EDITING in case we are editing
	    // a group of objects; if the next item in the group is a
	    // multipoint object, state will get reset to AWC_EDITING_POINTS
            awo->state = AWC_EDITING;

            if ( !( be->state & ShiftMask ) ) {
              awo->currentPointObject->removeLastPoint();
	    }

            awo->currentPointObject->lineEditComplete();

          }

//========== Shift B1 Press ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B1 Press ===================================

//========== Ctrl B1 Press ===================================

        }
	else {

//========== B1 Press ===================================

          if ( awo->state == AWC_EDITING_POINTS ) {

            awo->filterPosition( &be->x, &be->y, be->x, be->y );

            stat = awo->currentPointObject->addPoint( be->x, be->y );

          }
          else if ( awo->state == AWC_CHOOSING_LINE_OP ) {

	    // we can only get here if the user clicks outside the
	    // b1OneSelectPopup (line edit mode) menu, so just restore
	    // saved state
            awo->state = awo->savedState;

	  }
          else if ( awo->state == AWC_EDITING ) {

            if ( awo->currentEf ) {

              isMultiPoint = awo->currentEf->objectIsMultiPoint();

              awo->currentEf->popdown();
              awo->currentEf = NULL;

              if ( isMultiPoint ) {
                awo->currentPointObject->lineEditCancel();
              }
              else {
                awo->currentObject->operationCancel();  // restore saved state
              }

              // This code is a bit tricky. The intent here is as follows:
              // If an item was being edited and the user selects another
              // item, then that item immediately is edited. Note that the
              // button release code will take some redundant actions.
              // Modifications of this or the corresponding button release
              // code could result in some strange side-effects.

              // ===========================================================
	      // First, deselect all nodes

              cur = awo->head->blink;
              while ( cur != awo->head ) {

                if ( cur->node->deleteRequest ) {
                  cur = cur->blink;
                  continue;
                }

                wasSelected = cur->node->isSelected();
                if ( wasSelected ) {

                  cur->node->drawSelectBoxCorners(); // erase via xor gc
                  cur->node->deselect();

                  // unlink
                  if ( cur->selBlink ) {
                    cur->selBlink->selFlink = cur->selFlink;
		  }
		  else {
                    printf( "%s at x=%-d, y=%-d : selBlink is null (1)\n",
		     cur->node->objName(), cur->node->getX0(),
		     cur->node->getY0() );
		  }
                  if ( cur->selFlink ) {
                    cur->selFlink->selBlink = cur->selBlink;
		  }
		  else {
                    printf( "%s at x=%-d, y=%-d : selFlink is null (2)\n",
		     cur->node->objName(), cur->node->getX0(),
		     cur->node->getY0() );
		  }

                }

                cur = cur->blink;

              }

              // ===========================================================
	      // Next, look for a new selection to edit

              foundNewOneToEdit = 0;
              editNode = NULL;

              // if another item is selected, edit this one
              cur = awo->head->blink;
              while ( cur != awo->head ) {

                if ( cur->node->deleteRequest ) {
                  cur = cur->blink;
                  continue;
                }

                gotSelection = cur->node->select( be->x, be->y );
                if ( gotSelection ) {

                  foundNewOneToEdit = 1;
                  editNode = cur;
                  cur->node->drawSelectBoxCorners();
                  cur->selBlink = awo->selectedHead->selBlink;
                  awo->selectedHead->selBlink->selFlink = cur;
                  awo->selectedHead->selBlink = cur;
                  cur->selFlink = awo->selectedHead;
                  awo->savedState = awo->state;

                  awo->state = AWC_EDITING;
                  if ( cur->node->isMultiPointObject() )
                    cur->node->setEditProperties();

                  break;

                }

                cur = cur->blink;

              }

              // ===========================================================

              if ( awo->state == AWC_EDITING ) {

                if ( editNode ) {
                  awo->updateEditSelectionPointers();
                  awo->cursor.set( XtWindow(awo->drawWidget), CURSOR_K_WAIT );
                  awo->cursor.setColor( awo->ci->pix(awo->fgColor),
                   awo->ci->pix(awo->bgColor) );
                  editNode->node->doEdit();
                }

              }
              else {

                // determine new state
                num_selected = 0;

                curSel = awo->selectedHead->selFlink;
                while ( ( curSel != awo->selectedHead ) &&
                        ( num_selected < 2 ) ) {

                  num_selected++;
                  curSel = curSel->selFlink;

                }

                if ( num_selected == 0 ) {
                  awo->state = AWC_NONE_SELECTED;
                  awo->updateMasterSelection();
                  awo->useFirstSelectedAsReference = 0;
                }
                else if ( num_selected == 1 ) {
                  awo->useFirstSelectedAsReference = 1;
                  awo->state = AWC_ONE_SELECTED;
                  awo->updateMasterSelection();
                }
                else {
                  printErrMsg( __FILE__, __LINE__,
                   "Inconsistent select state" );
                  //printf( "Klingons decloaking! (3)\n" );
                  awo->state = AWC_MANY_SELECTED;
                  awo->updateMasterSelection();
                }

              }

            }

          }

        }

//========== B1 Press ===================================

        break;

      case Button2:

        if ( be->state & ShiftMask ) {

//========== Shift B2 Press ===================================

          if ( awo->state == AWC_EDITING_POINTS ) {

            stat = awo->currentPointObject->removeLastPoint();

          }

//========== Shift B2 Press ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B2 Press ===================================

//========== Ctrl B2 Press ===================================

        }
	else {

//========== B2 Press ===================================

          awo->b2PressX = be->x;
          awo->b2PressY = be->y;
          awo->b2PressXRoot = be->x_root;
          awo->b2PressYRoot = be->y_root;

          if ( awo->state == AWC_EDITING_POINTS ) {

            awo->currentPoint =
              awo->currentPointObject->selectPoint( be->x, be->y );

            if ( awo->currentPoint ) awo->state = AWC_MOVING_POINT;

          }

//========== B2 Press ===================================

        }

        break;

      case Button3:

        if ( be->state & ShiftMask ) {

//========== Shift B3 Press ===================================

//========== Shift B3 Press ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B3 Press ===================================

//========== Ctrl B3 Press ===================================

        }
        else {

//========== B3 Press ===================================

//========== B3 Press ===================================

//           break;

        }

        break;

    }

  }
  else if ( e->type == ButtonRelease ) {

    if ( awo->state == AWC_WAITING ) goto done;

    be = (XButtonEvent *) e;

    switch ( be->button ) {

      case Button1:

        if ( be->state & ShiftMask ) {

//========== Shift B1 Release ===================================

          switch ( awo->state ) {

            case AWC_EDITING_POINTS:
              break;

            case AWC_NONE_SELECTED:

              cur = awo->head->blink;
              while ( cur != awo->head ) {
                gotSelection = cur->node->select( be->x, be->y );
                if ( gotSelection ) {
                  cur->node->drawSelectBoxCorners();
                  cur->selBlink = awo->selectedHead->selBlink;
                  awo->selectedHead->selBlink->selFlink = cur;
                  awo->selectedHead->selBlink = cur;
                  cur->selFlink = awo->selectedHead;
                  awo->state = AWC_ONE_SELECTED;
                  awo->updateMasterSelection();
                  awo->useFirstSelectedAsReference = 1;
                  break;
                }
                cur = cur->blink;
              }

              break;

            case AWC_ONE_SELECTED:
            case AWC_MANY_SELECTED:

              cur = awo->head->blink;
              while ( cur != awo->head ) {

                wasSelected = cur->node->isSelected();

                gotSelection = cur->node->select( be->x, be->y );

                if ( gotSelection ) {

                  if ( wasSelected ) { // deselect

                    cur->node->drawSelectBoxCorners(); // erase via xor gc
                    cur->node->deselect();

		    // unlink
                    if ( cur->selBlink ) {
                      cur->selBlink->selFlink = cur->selFlink;
		    }
		    else {
                      printf( "%s at x=%-d, y=%-d : selBlink is null (3)\n",
		       cur->node->objName(), cur->node->getX0(),
		       cur->node->getY0() );
		    }
                    if ( cur->selFlink ) {
                      cur->selFlink->selBlink = cur->selBlink;
		    }
		    else {
                      printf( "%s at x=%-d, y=%-d : selFlink is null (4)\n",
		       cur->node->objName(), cur->node->getX0(),
		       cur->node->getY0() );
		    }

		    // determine new state
                    num_selected = 0;

                    curSel = awo->selectedHead->selFlink;
                    while ( ( curSel != awo->selectedHead ) &&
                            ( num_selected < 2 ) ) {

                      num_selected++;
                      curSel = curSel->selFlink;

                    }

                    if ( num_selected == 0 ) {
                      awo->state = AWC_NONE_SELECTED;
                      awo->updateMasterSelection();
                      awo->useFirstSelectedAsReference = 0;
                    }
                    else if ( num_selected == 1 ) {
                      awo->useFirstSelectedAsReference = 1;
                      awo->state = AWC_ONE_SELECTED;
                      awo->updateMasterSelection();
                    }
                    else {
                      awo->state = AWC_MANY_SELECTED;
                      awo->updateMasterSelection();
                    }

                    break;

                  }
                  else {

                    cur->node->drawSelectBoxCorners();
                    cur->selBlink = awo->selectedHead->selBlink;
                    awo->selectedHead->selBlink->selFlink = cur;
                    awo->selectedHead->selBlink = cur;
                    cur->selFlink = awo->selectedHead;
                    awo->state = AWC_MANY_SELECTED;
                    awo->updateMasterSelection();

                    break;

                  }

                }

                cur = cur->blink;

              }

              break;

          }

//========== Shift B1 Release ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B1 Release ===================================

          switch ( awo->state ) {

            case AWC_MOVE:

              awo->setChanged();

              awo->state = awo->savedState;

              deltax = awo->oldx - awo->startx;
              deltay = awo->oldy - awo->starty;

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {
                cur->node->eraseSelectBox();
                cur->node->erase();
                cur->node->move( deltax, deltay );
                cur->node->snapToGrid();
                cur = cur->selFlink;
              }

              awo->updateMasterSelection();

              awo->refresh();

              break;

            case AWC_ONE_SELECTED:

              start = NULL;
              cur = awo->head->blink;
              while ( cur != awo->head ) {

                wasSelected = cur->node->isSelected();
                if ( wasSelected ) {

                  cur->node->drawSelectBoxCorners(); // erase via xor gc
                  cur->node->deselect();
                  // unlink
                  if ( cur->selBlink ) {
                    cur->selBlink->selFlink = cur->selFlink;
		  }
		  else {
                    printf( "%s at x=%-d, y=%-d : selBlink is null (5)\n",
		     cur->node->objName(), cur->node->getX0(),
		     cur->node->getY0() );
		  }
                  if ( cur->selFlink ) {
                    cur->selFlink->selBlink = cur->selBlink;
		  }
		  else {
                    printf( "%s at x=%-d, y=%-d : selFlink is null (6)\n",
		     cur->node->objName(), cur->node->getX0(),
		     cur->node->getY0() );
		  }

                  start = cur;
                  break;

		}

                cur = cur->blink;

	      }

              if ( start ) {
                cur = start->blink;
	      }
	      else {
                cur = awo->head->blink;
	      }

              while ( cur != awo->head ) {
                gotSelection = cur->node->select( be->x, be->y );
                if ( gotSelection ) {
                  cur->node->drawSelectBoxCorners();
                  cur->selBlink = awo->selectedHead->selBlink;
                  awo->selectedHead->selBlink->selFlink = cur;
                  awo->selectedHead->selBlink = cur;
                  cur->selFlink = awo->selectedHead;
                  awo->state = AWC_ONE_SELECTED;
                  awo->useFirstSelectedAsReference = 1;
                  awo->updateMasterSelection();
                  break;
                }
                cur = cur->blink;
              }

              // determine new state
              num_selected = 0;

              curSel = awo->selectedHead->selFlink;
              while ( ( curSel != awo->selectedHead ) &&
                      ( num_selected < 2 ) ) {

                num_selected++;
                curSel = curSel->selFlink;

              }

              if ( num_selected == 0 ) {

                // try it once more using the entire list

                cur = awo->head->blink;
                while ( cur != awo->head ) {
                  gotSelection = cur->node->select( be->x, be->y );
                  if ( gotSelection ) {
                    cur->node->drawSelectBoxCorners();
                    cur->selBlink = awo->selectedHead->selBlink;
                    awo->selectedHead->selBlink->selFlink = cur;
                    awo->selectedHead->selBlink = cur;
                    cur->selFlink = awo->selectedHead;
                    awo->state = AWC_ONE_SELECTED;
                    awo->useFirstSelectedAsReference = 1;
                    awo->updateMasterSelection();
                    break;
                  }
                  cur = cur->blink;
                }

	      }

              // determine new state
              num_selected = 0;

              curSel = awo->selectedHead->selFlink;
              while ( ( curSel != awo->selectedHead ) &&
                      ( num_selected < 2 ) ) {

                num_selected++;
                curSel = curSel->selFlink;

              }

              if ( num_selected == 0 ) {
                awo->state = AWC_NONE_SELECTED;
                awo->updateMasterSelection();
                awo->useFirstSelectedAsReference = 0;
              }
              else if ( num_selected == 1 ) {
                awo->useFirstSelectedAsReference = 1;
                awo->state = AWC_ONE_SELECTED;
                awo->updateMasterSelection();
              }
              else {
                  printErrMsg( __FILE__, __LINE__,
                   "Inconsistent select state" );
                  //printf( "Klingons decloaking! (4)\n" );
                awo->state = AWC_MANY_SELECTED;
                awo->updateMasterSelection();
              }

              break;

	  }

//========== Ctrl B1 Release ===================================

        }
        else {

//========== B1 Release ===================================

          if ( awo->deltaTime < 250 ) break; // switch ( be->button ) ...

          switch ( awo->state ) {

	    case AWC_EDITING_POINTS:
              break;

            case AWC_START_DEFINE_REGION:

              awo->state = AWC_NONE_SELECTED;
              awo->updateMasterSelection();
              break;

            case AWC_DEFINE_REGION:

              // do this in case user clicks outside
              // popup menu to cancel operation
              awo->state = AWC_NONE_SELECTED;
              awo->updateMasterSelection();

              awo->width = awo->oldx - awo->startx;
              awo->height = awo->oldy - awo->starty;

              if ( awo->width < 0 ) {
                x0 = awo->oldx;
                awo->width = abs( awo->width );
              }
              else {
                x0 = awo->startx;
	      }

              if ( awo->height < 0 ) {
                y0 = awo->oldy;
                awo->height = abs( awo->height );
              }
              else {
                y0 = awo->starty;
	      }

              if ( ( awo->width > 0 ) && ( awo->height > 0 ) ) {

                // erase region definition box

                awo->drawGc.saveFg();
                awo->drawGc.setFG( awo->ci->pix(awo->fgColor) );
                XDrawRectangle( awo->d, XtWindow(awo->drawWidget),
                 awo->drawGc.xorGC(), x0, y0, awo->width,
                 awo->height );
                awo->drawGc.restoreFg();

              }

              awo->startx = x0;
              awo->starty = y0;

              if ( ( awo->width > 5 ) && ( awo->height > 5 ) ) {

                XmMenuPosition( awo->b1NoneSelectPopup, be );
                XtManageChild( awo->b1NoneSelectPopup );

                XSetWindowColormap( awo->d,
                 XtWindow(XtParent(awo->b1NoneSelectPopup)),
                 awo->appCtx->ci.getColorMap() );

              }

              break;

            case AWC_MOVE:

              awo->setChanged();

              awo->state = awo->savedState;

              deltax = awo->oldx - awo->startx;
              deltay = awo->oldy - awo->starty;

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {
                cur->node->eraseSelectBox();
                cur->node->erase();
                cur->node->move( deltax, deltay );
                cur->node->snapToGrid();
                cur = cur->selFlink;
              }

              awo->updateMasterSelection();

              awo->refresh();

              break;

            case AWC_RESIZE_LEFT:

              awo->setChanged();

              awo->state = awo->savedState;

              deltax = awo->oldx - awo->startx;
	      deltay = 0;
              deltaw = awo->startx - awo->oldx;

              xScaleFactor = 1.0 + (double ) deltaw /
               (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

              yScaleFactor = 1.0;

              awo->drawAfterResizeAbs( awo, deltax, xScaleFactor, deltay,
               yScaleFactor );

              awo->updateMasterSelection();

              break;

            case AWC_RESIZE_TOP:

              awo->setChanged();

              awo->state = awo->savedState;

              deltay = awo->oldy - awo->starty;
	      deltax = 0;
              deltah = awo->starty - awo->oldy;

              yScaleFactor = 1.0 + (double) deltah /
               (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

              xScaleFactor = 1.0;

              awo->drawAfterResizeAbs( awo, deltax, xScaleFactor, deltay,
               yScaleFactor );

              awo->updateMasterSelection();

              break;

            case AWC_RESIZE_BOTTOM:

              awo->setChanged();

              awo->state = awo->savedState;

              deltax = 0;
	      deltay = 0;
              deltah = awo->oldy - awo->starty;

              xScaleFactor = 1.0;

              yScaleFactor = 1.0 + (double ) deltah /
               (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

              awo->drawAfterResizeAbs( awo, deltax, xScaleFactor, deltay,
               yScaleFactor );

              awo->updateMasterSelection();

              break;

            case AWC_RESIZE_RIGHT:

              awo->setChanged();

              awo->state = awo->savedState;

              deltax = 0;
	      deltay = 0;
              deltaw = awo->oldx - awo->startx;

              xScaleFactor = 1.0 + (double ) deltaw /
               (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

              yScaleFactor = 1.0;

              awo->drawAfterResizeAbs( awo, deltax, xScaleFactor, deltay,
               yScaleFactor );

              awo->updateMasterSelection();

              break;

            case AWC_RESIZE_LEFT_TOP:

              awo->setChanged();

              awo->state = awo->savedState;

              deltax = awo->oldx - awo->startx;
              deltaw = awo->startx - awo->oldx;

              xScaleFactor = 1.0 + (double ) deltaw /
               (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

              deltay = awo->oldy - awo->starty;
              deltah = awo->starty - awo->oldy;

              yScaleFactor = 1.0 + (double) deltah /
               (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

              awo->drawAfterResizeAbs( awo, deltax, xScaleFactor, deltay,
               yScaleFactor );

              awo->updateMasterSelection();

              break;

            case AWC_RESIZE_LEFT_BOTTOM:

              awo->setChanged();

              awo->state = awo->savedState;

              deltax = awo->oldx - awo->startx;
              deltaw = awo->startx - awo->oldx;
              xScaleFactor = 1.0 + (double ) deltaw /
               (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

	      deltay = 0;
              deltah = awo->oldy - awo->starty;
              yScaleFactor = 1.0 + (double ) deltah /
               (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

              awo->drawAfterResizeAbs( awo, deltax, xScaleFactor, deltay,
               yScaleFactor );

              awo->updateMasterSelection();

              break;

            case AWC_RESIZE_RIGHT_TOP:

              awo->setChanged();

              awo->state = awo->savedState;

              deltax = 0;
              deltaw = awo->oldx - awo->startx;
              xScaleFactor = 1.0 + (double ) deltaw /
               (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

              deltay = awo->oldy - awo->starty;
              deltah = awo->starty - awo->oldy;
              yScaleFactor = 1.0 + (double) deltah /
               (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

              awo->drawAfterResizeAbs( awo, deltax, xScaleFactor, deltay,
               yScaleFactor );

              awo->updateMasterSelection();

              break;

            case AWC_RESIZE_RIGHT_BOTTOM:

              awo->setChanged();

              awo->state = awo->savedState;

              deltax = 0;
              deltaw = awo->oldx - awo->startx;
              xScaleFactor = 1.0 + (double ) deltaw /
               (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

	      deltay = 0;
              deltah = awo->oldy - awo->starty;
              yScaleFactor = 1.0 + (double ) deltah /
               (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

              awo->drawAfterResizeAbs( awo, deltax, xScaleFactor, deltay,
               yScaleFactor );

              awo->updateMasterSelection();

              break;

            case AWC_NONE_SELECTED:

              cur = awo->head->blink;
              while ( cur != awo->head ) {
                gotSelection = cur->node->select( be->x, be->y );
                if ( gotSelection ) {
                  cur->node->drawSelectBoxCorners();
                  cur->selBlink = awo->selectedHead->selBlink;
                  awo->selectedHead->selBlink->selFlink = cur;
                  awo->selectedHead->selBlink = cur;
                  cur->selFlink = awo->selectedHead;
                  awo->state = AWC_ONE_SELECTED;
                  awo->useFirstSelectedAsReference = 1;
                  awo->updateMasterSelection();
                  break;
                }
                cur = cur->blink;
              }

              break;

            case AWC_ONE_SELECTED:

              operationPerformed = 0;

              cur = awo->head->blink;
              while ( cur != awo->head ) {

                wasSelected = cur->node->isSelected();
                gotSelection = cur->node->select( be->x, be->y );
                if ( gotSelection && wasSelected ) {

                  if ( cur->node->isMultiPointObject() ) {

                    awo->savedState = awo->state;
                    awo->currentPointObject = cur->node;
                    XmMenuPosition( awo->b1OneSelectPopup, be );
                    XtManageChild( awo->b1OneSelectPopup );
                    XSetWindowColormap( awo->d,
                     XtWindow(XtParent(awo->b1OneSelectPopup)),
                    awo->appCtx->ci.getColorMap() );
                    awo->state = AWC_CHOOSING_LINE_OP;
                    operationPerformed = 1;
                    break; // out of while loop

                  }
		  else {

                    awo->savedState = awo->state;
                    awo->state = AWC_EDITING;
                    awo->currentEf = NULL;
                    awo->cursor.set( XtWindow(awo->drawWidget),
                     CURSOR_K_WAIT );
                    awo->cursor.setColor( awo->ci->pix(awo->fgColor),
                     awo->ci->pix(awo->bgColor) );
                    cur->node->doEdit();
                    operationPerformed = 1;
                    break; // out of while loop

		  }

                }
                else if ( gotSelection && !wasSelected ) {
                  cur->node->deselect();
                }

                cur = cur->blink;

              }

              if ( !operationPerformed ) {

                totalSelected = 0;

                cur = awo->head->blink;
                while ( cur != awo->head ) {

                  wasSelected = cur->node->isSelected();
                  gotSelection = cur->node->select( be->x, be->y );
                  if ( gotSelection && !wasSelected && !totalSelected ) {

                    cur->node->drawSelectBoxCorners();
                    cur->selBlink = awo->selectedHead->selBlink;
                    awo->selectedHead->selBlink->selFlink = cur;
                    awo->selectedHead->selBlink = cur;
                    cur->selFlink = awo->selectedHead;
                    totalSelected++;

                  }
                  else if ( gotSelection && !wasSelected && totalSelected ) {

                    cur->node->deselect();

                  }
                  else if ( !gotSelection && wasSelected ) {

                    cur->node->drawSelectBoxCorners(); // erase via xor gc
                    cur->node->deselect();
                    // unlink
                    if ( cur->selBlink ) {
                      cur->selBlink->selFlink = cur->selFlink;
		    }
		    else {
                      printf( "%s at x=%-d, y=%-d : selBlink is null (7)\n",
		       cur->node->objName(), cur->node->getX0(),
		       cur->node->getY0() );
		    }
                    if ( cur->selFlink ) {
                      cur->selFlink->selBlink = cur->selBlink;
		    }
		    else {
                      printf( "%s at x=%-d, y=%-d : selFlink is null (8)\n",
		       cur->node->objName(), cur->node->getX0(),
		       cur->node->getY0() );
		    }

                  }

                  cur = cur->blink;

                }

                // determine new state
                num_selected = 0;

                curSel = awo->selectedHead->selFlink;
                while ( ( curSel != awo->selectedHead ) &&
                        ( num_selected < 2 ) ) {

                  num_selected++;
                  curSel = curSel->selFlink;

                }

                if ( num_selected == 0 ) {
                  awo->state = AWC_NONE_SELECTED;
                  awo->updateMasterSelection();
                  awo->useFirstSelectedAsReference = 0;
                }
                else if ( num_selected == 1 ) {
                  awo->useFirstSelectedAsReference = 1;
                  awo->state = AWC_ONE_SELECTED;
                  awo->updateMasterSelection();
                }
                else {
                  printErrMsg( __FILE__, __LINE__,
                   "Inconsistent select state" );
                  //printf( "Klingons decloaking! (5)\n" );
                  awo->state = AWC_MANY_SELECTED;
                  awo->updateMasterSelection();
                }

              }

              break;

	    case AWC_MANY_SELECTED:

              operationPerformed = 0;

              cur = awo->head->blink;
              while ( cur != awo->head ) {

                wasSelected = cur->node->isSelected();
                gotSelection = cur->node->select( be->x, be->y );
                if ( gotSelection && wasSelected ) {

		  // edit first node in select list
                  awo->savedState = awo->state;
                  awo->state = AWC_EDITING;
                  awo->currentEf = NULL;
                  awo->cursor.set( XtWindow(awo->drawWidget),
                   CURSOR_K_WAIT );
                  awo->cursor.setColor( awo->ci->pix(awo->fgColor),
                   awo->ci->pix(awo->bgColor) );
                  // next line is for multiline objects
                  awo->selectedHead->selFlink->node->setEditProperties();
                  awo->selectedHead->selFlink->node->doEdit();
                  if ( awo->selectedHead->selFlink->node->isMultiPointObject()
                  ) {
                    awo->currentPointObject =
                     awo->selectedHead->selFlink->node;
                  }
                  operationPerformed = 1;
                  break; // out of while loop

                }
                else if ( gotSelection && !wasSelected ) {
                  cur->node->deselect();
		}

                cur = cur->blink;

              }

              if ( !operationPerformed ) {

	        // deselect all

                cur = awo->head->flink;       // do this because above may
                while ( cur != awo->head ) {  // have selected more
                  cur->node->deselect();     // have selected several
                  cur = cur->flink;
		}

                cur = awo->selectedHead->selFlink;
                while ( cur != awo->selectedHead ) {
                  cur->node->drawSelectBoxCorners(); // erase via xor gc
                  cur = cur->selFlink;
                }
                // make list empty
                awo->selectedHead->selFlink = awo->selectedHead;
                awo->selectedHead->selBlink = awo->selectedHead;
                awo->state = AWC_NONE_SELECTED;
                awo->updateMasterSelection();

                cur = awo->head->blink;
                while ( cur != awo->head ) {
                  gotSelection = cur->node->select( be->x, be->y );
                  if ( gotSelection ) {
                    cur->node->drawSelectBoxCorners();
                    cur->selBlink = awo->selectedHead->selBlink;
                    awo->selectedHead->selBlink->selFlink = cur;
                    awo->selectedHead->selBlink = cur;
                    cur->selFlink = awo->selectedHead;
                    awo->state = AWC_ONE_SELECTED;
                    awo->useFirstSelectedAsReference = 1;
                    awo->updateMasterSelection();
                    break;
                  }
                  cur = cur->blink;
                }

              }

              break;

          }

//========== B1 Release ===================================

        }

        break;

      case Button2:


        if ( be->state & ShiftMask ) {

//========== Shift B2 Release ===================================

//========== Shift B2 Release ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B2 Release ===================================

//========== Ctrl B2 Release ===================================

        }
        else {

//========== B2 Release ===================================

          prev_num_selected = 0;
          cur = awo->head->blink;
          while ( cur != awo->head ) {
            if ( cur->node->isSelected() ) {
	      prev_num_selected++;
	    }
	    cur = cur->blink;
	  }

          do {

          doAgain = 0;

          switch ( awo->state ) {

	    case AWC_MOVING_POINT:

              awo->state = AWC_EDITING_POINTS;
              break;

	    case AWC_EDITING_POINTS:

              break;

	    case AWC_NONE_SELECTED:

              XmMenuPosition( awo->b2NoneSelectPopup, be );
              XtManageChild( awo->b2NoneSelectPopup );

              XSetWindowColormap( awo->d,
               XtWindow(XtParent(awo->b2NoneSelectPopup)),
               awo->appCtx->ci.getColorMap() );

              break;

	    case AWC_ONE_SELECTED:

              XmMenuPosition( awo->b2OneSelectPopup, be );
              XtManageChild( awo->b2OneSelectPopup );

              XSetWindowColormap( awo->d,
               XtWindow(XtParent(awo->b2OneSelectPopup)),
               awo->appCtx->ci.getColorMap() );

              break;

	    case AWC_MANY_SELECTED:

              XmMenuPosition( awo->b2ManySelectPopup, be );
              XtManageChild( awo->b2ManySelectPopup );

              XSetWindowColormap( awo->d,
               XtWindow(XtParent(awo->b2ManySelectPopup)),
               awo->appCtx->ci.getColorMap() );

              break;

            case AWC_START_DEFINE_SELECT_REGION:

              if ( prev_num_selected == 0 ) {
                awo->state = AWC_NONE_SELECTED;
                awo->updateMasterSelection();
		doAgain = 1;
	      }
              else if ( prev_num_selected == 1 ) {
                awo->state = AWC_ONE_SELECTED;
                awo->useFirstSelectedAsReference = 1;
                awo->updateMasterSelection();
                doAgain = 1;
	      }
              else {
                awo->state = AWC_MANY_SELECTED;
                awo->updateMasterSelection();
                doAgain = 1;
	      }

              break;

            case AWC_DEFINE_SELECT_REGION:

              awo->width = awo->oldx - awo->startx;
              awo->height = awo->oldy - awo->starty;

              if ( ( awo->width < 0 ) && ( awo->height < 0 ) ) {
                selectMustEnclose = False;
              }
              else {
                selectMustEnclose = True;
              }

              if ( awo->width < 0 ) {
                x0 = awo->oldx;
                awo->width = abs( awo->width );
              }
              else {
                x0 = awo->startx;
	      }

              if ( awo->height < 0 ) {
                y0 = awo->oldy;
                awo->height = abs( awo->height );
              }
              else {
                y0 = awo->starty;
	      }

              if ( ( awo->width > 0 ) && ( awo->height > 0 ) ) {

                // erase region definition box
                awo->drawGc.saveFg();
                awo->drawGc.setFG( awo->ci->pix(awo->fgColor) );
                XDrawRectangle( awo->d, XtWindow(awo->drawWidget),
                 awo->drawGc.xorGC(), x0, y0, awo->width,
                 awo->height );
		awo->drawGc.restoreFg();

              }

              if ( ( awo->width < 3 ) && ( awo->height < 3 ) ) {
                awo->state = awo->savedState;
                doAgain = 1;
                break; // out of switch statement
              }

              awo->startx = x0;
              awo->starty = y0;

              awo->state = awo->savedState;  // if below fails
              num_selected = 0;

              if ( ( awo->width > 0 ) && ( awo->height > 0 ) ) {

                cur = awo->head->blink;
                while ( cur != awo->head ) {

                  if ( selectMustEnclose ) {

                    wasSelected = cur->node->isSelected();
                    gotSelection = cur->node->selectEnclosed( awo->startx,
                     awo->starty, awo->width, awo->height );
                    if ( !wasSelected ) {
                      if ( gotSelection ) {
                        num_selected++;
                        cur->node->drawSelectBoxCorners();
                        cur->selBlink = awo->selectedHead->selBlink;
                        awo->selectedHead->selBlink->selFlink = cur;
                        awo->selectedHead->selBlink = cur;
                        cur->selFlink = awo->selectedHead;
                      }
                    }
                    else {
                      if ( gotSelection ) {
                        cur->node->drawSelectBoxCorners(); // erase via xor gc
                        cur->node->deselect();
		        // unlink
                        if ( cur->selBlink ) {
                          cur->selBlink->selFlink = cur->selFlink;
		        }
		        else {
                          printf(
                           "%s at x=%-d, y=%-d : selBlink is null (9)\n",
		           cur->node->objName(), cur->node->getX0(),
		           cur->node->getY0() );
		        }
                        if ( cur->selFlink ) {
                          cur->selFlink->selBlink = cur->selBlink;
		        }
		        else {
                          printf(
                           "%s at x=%-d, y=%-d : selFlink is null (A)\n",
		           cur->node->objName(), cur->node->getX0(),
		           cur->node->getY0() );
		        }
                      }
		      else {
                        num_selected++;
		      }
                    }

                  }
                  else {

                    wasSelected = cur->node->isSelected();
                    gotSelection = cur->node->selectTouching( awo->startx,
                     awo->starty, awo->width, awo->height );
                    if ( !wasSelected ) {
                      if ( gotSelection ) {
                        num_selected++;
                        cur->node->drawSelectBoxCorners();
                        cur->selBlink = awo->selectedHead->selBlink;
                        awo->selectedHead->selBlink->selFlink = cur;
                        awo->selectedHead->selBlink = cur;
                        cur->selFlink = awo->selectedHead;
                      }
                    }
                    else {
                      if ( gotSelection ) {
                        cur->node->drawSelectBoxCorners(); // erase via xor gc
                        cur->node->deselect();
		        // unlink
                        if ( cur->selBlink ) {
                          cur->selBlink->selFlink = cur->selFlink;
		        }
		        else {
                          printf(
                           "%s at x=%-d, y=%-d : selBlink is null (B)\n",
		           cur->node->objName(), cur->node->getX0(),
		           cur->node->getY0() );
		        }
                        if ( cur->selFlink ) {
                          cur->selFlink->selBlink = cur->selBlink;
		        }
		        else {
                          printf(
                           "%s at x=%-d, y=%-d : selFlink is null (C)\n",
		           cur->node->objName(), cur->node->getX0(),
		           cur->node->getY0() );
		        }
                      }
		      else {
                        num_selected++;
		      }
                    }

                  }

                  cur = cur->blink;

                }

                if ( num_selected == 0 ) {
                  awo->state = AWC_NONE_SELECTED;
                  awo->updateMasterSelection();
		  // doAgain = 1;
		}
                else if ( num_selected == 1 ) {
                  awo->state = AWC_ONE_SELECTED;
                  awo->useFirstSelectedAsReference = 1;
                  awo->updateMasterSelection();
                  // if ( prev_num_selected == num_selected ) doAgain = 1;
		}
                else {
                  awo->state = AWC_MANY_SELECTED;
                  awo->updateMasterSelection();
                  // if ( prev_num_selected == num_selected ) doAgain = 1;
		}

              }

              break;

          }

	  } while ( doAgain );

//========== B2 Release ===================================

          break;

        }

        break;

      case Button3:

        if ( be->state & ShiftMask ) {

//========== Shift B3 Release ===================================

//========== Shift B3 Release ===================================

          break;

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B3 Release ===================================

//========== Ctrl B3 Release ===================================

          break;

        }
        else {

//========== B3 Release ===================================

//========== B3 Release ===================================

          break;

        }

        break;

    }

  }

  else if ( e->type == MotionNotify ) {

    if ( awo->state == AWC_WAITING ) goto done;

    me = (XMotionEvent *) e;

    if ( awo->appCtx->viewXy ) {

      if ( !awo->msgDialogCreated ) {
        awo->msgDialog.create( awo->top );
        awo->msgDialogCreated = 1;
        awo->msgDialogPoppedUp = 0;
      }

      sprintf( msg, "(%-d,%-d)", me->x, me->y );

      awo->msgDialog.popup( msg, awo->x+awo->w/2, awo->y+awo->h/2 );
      awo->msgDialogPoppedUp = 1;

    }
    else {

      if ( awo->msgDialogPoppedUp ) {
        awo->msgDialog.popdown();
        awo->msgDialogPoppedUp = 0;
      }

    }

    // on first motion event after a button press, use the button press x,y
    if ( awo->buttonPressX != -1 ) {
      me->x = awo->buttonPressX;
      awo->buttonPressX = -1;
    }
    if ( awo->buttonPressY != -1 ) {
      me->y = awo->buttonPressY;
      awo->buttonPressY = -1;
    }

    rawX = me->x;
    rawY = me->y;

    if ( awo->state == AWC_MOVE ) {
      awo->filterPosition( &me->x, &me->y, awo->startx, awo->starty );
    }
    else {
      awo->filterPosition( &me->x, &me->y, me->x, me->y );
    }

    if ( me->state & Button1Mask ) {

      if ( me->state & ShiftMask ) {

//======== Shift B1 motion ============


//======== Shift B1 motion ============

      }
      else if ( me->state & ControlMask ) {

//======== Ctrl B1 motion ============

        switch ( awo->state ) {

	  case AWC_ONE_SELECTED:
	  case AWC_MANY_SELECTED:

            cur = awo->selectedHead->selFlink;
            operation = 0;
            while ( ( cur != awo->selectedHead ) && !operation ) {
              operation = cur->node->getSelectBoxOperation( rawX, rawY );
              cur = cur->selFlink;
            }

            if ( operation ) {

              // force operation to be a move
              operation = AGC_MOVE_OP;

              awo->undoObj.startNewUndoList( activeWindowClass_str170 );
              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {
                stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
                cur = cur->selFlink;
              }

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {
                cur->node->erase();
                cur = cur->selFlink;
              }

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {
                cur->node->drawSelectBoxCorners();
                cur->node->drawSelectBox();
                cur = cur->selFlink;
              }

              awo->savedState = awo->state;
              awo->state = AWC_MOVE;
              awo->usingArrowKeys = 0;
              awo->startx = me->x;
              awo->starty = me->y;
              awo->oldx = me->x;
              awo->oldy = me->y;

            }

            break;

          case AWC_MOVE:

            if ( !awo->usingArrowKeys ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {
                deltax = me->x - awo->oldx;
                deltay = me->y - awo->oldy;
                cur->node->drawSelectBox(); //erase via xor gc
                cur->node->moveSelectBox( deltax, deltay );
                cur->node->drawSelectBox();
                cur = cur->selFlink;
              }
              awo->oldx = me->x;
              awo->oldy = me->y;

            }

            break;

	}

//======== Ctrl B1 motion ============

      }
      else {

//======== B1 motion ============

        switch ( awo->state ) {

	  case AWC_MOVING_POINT:
            break;

	  case AWC_EDITING_POINTS:
            break;

          case AWC_NONE_SELECTED:

            awo->state = AWC_START_DEFINE_REGION;
            awo->startx = me->x;
            awo->starty = me->y;
            break;

          case AWC_START_DEFINE_REGION:

            width = me->x - awo->startx;
            height = me->y - awo->starty;

            if ( width < 0 ) {
              x0 = me->x;
              width = abs( width );
            }
            else {
              x0 = awo->startx;
            }

            if ( height < 0 ) {
              y0 = me->y;
              height = abs( height );
            }
            else {
              y0 = awo->starty;
            }

            if ( ( width > 0 ) && ( height > 0 ) ) {

              awo->state = AWC_DEFINE_REGION;

              awo->drawGc.saveFg();
              awo->drawGc.setFG( awo->ci->pix(awo->fgColor) );
              XDrawRectangle( awo->d, XtWindow(awo->drawWidget),
               awo->drawGc.xorGC(), x0, y0, width, height );
              awo->drawGc.restoreFg();

            }

            awo->oldx = me->x;
            awo->oldy = me->y;

            break;

          case AWC_DEFINE_REGION:

            width = awo->oldx - awo->startx;
            height = awo->oldy - awo->starty;

            if ( width < 0 ) {
              x0 = awo->oldx;
              width = abs( width );
            }
            else {
              x0 = awo->startx;
            }

            if ( height < 0 ) {
              y0 = awo->oldy;
              height = abs( height );
            }
            else {
              y0 = awo->starty;
            }

            if ( ( width > 0 ) && ( height > 0 ) ) {

              awo->drawGc.saveFg();
              awo->drawGc.setFG( awo->ci->pix(awo->fgColor) );
              XDrawRectangle( awo->d, XtWindow(awo->drawWidget),
               awo->drawGc.xorGC(), x0, y0, width, height );
              awo->drawGc.restoreFg();

            }

            width = me->x - awo->startx;
            height = me->y - awo->starty;

            if ( width < 0 ) {
              x0 = me->x;
              width = abs( width );
            }
            else {
              x0 = awo->startx;
            }

            if ( height < 0 ) {
              y0 = me->y;
              height = abs( height );
            }
            else {
              y0 = awo->starty;
            }

            if ( ( width > 0 ) && ( height > 0 ) ) {

              awo->drawGc.saveFg();
              awo->drawGc.setFG( awo->ci->pix(awo->fgColor) );
              XDrawRectangle( awo->d, XtWindow(awo->drawWidget),
               awo->drawGc.xorGC(), x0, y0, width, height );
              awo->drawGc.restoreFg();

            }

            awo->oldx = me->x;
            awo->oldy = me->y;

            break;

	  case AWC_ONE_SELECTED:
	  case AWC_MANY_SELECTED:

            cur = awo->selectedHead->selFlink;
            operation = 0;
            while ( ( cur != awo->selectedHead ) && !operation ) {
              operation = cur->node->getSelectBoxOperation( rawX, rawY );
              cur = cur->selFlink;
            }

            if ( operation ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {
                cur->node->erase();
                cur = cur->selFlink;
              }

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {
                cur->node->drawSelectBoxCorners();
                cur->node->drawSelectBox();
                cur = cur->selFlink;
              }

              awo->savedState = awo->state;
              awo->usingArrowKeys = 0;
              awo->startx = me->x;
              awo->starty = me->y;
              awo->oldx = me->x;
              awo->oldy = me->y;

              if ( operation == AGC_MOVE_OP ) {

                awo->undoObj.startNewUndoList( activeWindowClass_str170 );
                cur = awo->selectedHead->selFlink;
                while ( cur != awo->selectedHead ) {
                  stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
                  cur = cur->selFlink;
                }

                awo->state = AWC_MOVE;

              }
              else {

                awo->undoObj.startNewUndoList( activeWindowClass_str171 );
                cur = awo->selectedHead->selFlink;
                while ( cur != awo->selectedHead ) {
                  stat = cur->node->addUndoResizeNode( &(awo->undoObj) );
                  cur = cur->selFlink;
                }

                if ( operation == AGC_LEFT_OP ) {

                  awo->state = AWC_RESIZE_LEFT;

                }
                else if ( operation == AGC_TOP_OP ) {

                  awo->state = AWC_RESIZE_TOP;

                }
                else if ( operation == AGC_BOTTOM_OP ) {

                  awo->state = AWC_RESIZE_BOTTOM;

                }
                else if ( operation == AGC_RIGHT_OP ) {

                  awo->state = AWC_RESIZE_RIGHT;

                }
                else if ( operation == AGC_LEFT_TOP_OP ) {

                  awo->state = AWC_RESIZE_LEFT_TOP;

                }
                else if ( operation == AGC_LEFT_BOTTOM_OP ) {

                  awo->state = AWC_RESIZE_LEFT_BOTTOM;

                }
                else if ( operation == AGC_RIGHT_TOP_OP ) {

                  awo->state = AWC_RESIZE_RIGHT_TOP;

                }
                else if ( operation == AGC_RIGHT_BOTTOM_OP ) {

                  awo->state = AWC_RESIZE_RIGHT_BOTTOM;

                }

              }

	    }

            break;

          case AWC_MOVE:

            if ( !awo->usingArrowKeys ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {
                deltax = me->x - awo->oldx;
                deltay = me->y - awo->oldy;
                cur->node->drawSelectBox(); //erase via xor gc
                cur->node->moveSelectBox( deltax, deltay );
                cur->node->drawSelectBox();
                cur = cur->selFlink;
              }
              awo->oldx = me->x;
              awo->oldy = me->y;

            }

            break;

          case AWC_RESIZE_LEFT:

            if ( !awo->usingArrowKeys ) {

            xScaleFactor = 1.0 + (double ) ( awo->startx - me->x ) /
             (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

            ok = 1;
            cur = awo->selectedHead->selFlink;
            while ( cur != awo->selectedHead ) {

              newX = awo->masterSelectX0 - awo->startx + me->x +
               (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
               * xScaleFactor + 0.5 );

              newW =
               (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

              stat = cur->node->checkResizeSelectBoxAbs( (int) newX, -1,
               (int) newW, -1 );

              if ( stat == 0 ) ok = 0;

              cur = cur->selFlink;

            }

            if ( ok ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {

                newX = awo->masterSelectX0 - awo->startx + me->x +
                 (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
                 * xScaleFactor + 0.5 );

                newW =
                 (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

                cur->node->drawSelectBox(); //erase via xor gc
                cur->node->resizeSelectBoxAbs( (int) newX, -1, (int) newW,
                 -1 );
                cur->node->drawSelectBox();

                cur = cur->selFlink;

              }

              awo->oldx = me->x;
              awo->oldy = me->y;

            }

	    }

            break;

          case AWC_RESIZE_TOP:

            if ( !awo->usingArrowKeys ) {

            yScaleFactor = 1.0 + (double) ( awo->starty - me->y ) /
             (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

            ok = 1;
            cur = awo->selectedHead->selFlink;
            while ( cur != awo->selectedHead ) {

              newY = awo->masterSelectY0 - awo->starty + me->y +
               (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
               * yScaleFactor + 0.5 );

              newH =
               (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

              stat = cur->node->checkResizeSelectBoxAbs( -1, (int) newY, -1,
               (int) newH );

              if ( stat == 0 ) ok = 0;

              cur = cur->selFlink;

            }

            if ( ok ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {

                newY = awo->masterSelectY0 - awo->starty + me->y +
                 (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
                 * yScaleFactor + 0.5 );

                newH =
                 (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

                cur->node->drawSelectBox(); //erase via xor gc
                cur->node->resizeSelectBoxAbs( -1, (int) newY, -1,
                 (int) newH );
                cur->node->drawSelectBox();

                cur = cur->selFlink;

              }

              awo->oldx = me->x;
              awo->oldy = me->y;

            }

	    }

            break;

          case AWC_RESIZE_BOTTOM:

            if ( !awo->usingArrowKeys ) {

            yScaleFactor = 1.0 + (double ) ( me->y - awo->starty ) /
             (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

            ok = 1;
            cur = awo->selectedHead->selFlink;
            while ( cur != awo->selectedHead ) {

              newY = awo->masterSelectY0 +
               (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
               * yScaleFactor + 0.5 );

              newH =
               (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

              stat = cur->node->checkResizeSelectBoxAbs( -1, (int) newY,
               -1, (int) newH );

              if ( stat == 0 ) ok = 0;

              cur = cur->selFlink;

            }

            if ( ok ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {

                newY = awo->masterSelectY0 +
                 (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
                 * yScaleFactor + 0.5 );

                newH =
                 (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

                cur->node->drawSelectBox(); //erase via xor gc
                cur->node->resizeSelectBoxAbs( -1, (int) newY, -1,
                 (int) newH );
                cur->node->drawSelectBox();

                cur = cur->selFlink;

              }

              awo->oldx = me->x;
              awo->oldy = me->y;

            }

	    }

            break;

          case AWC_RESIZE_RIGHT:

            if ( !awo->usingArrowKeys ) {

            xScaleFactor = 1.0 + (double ) ( me->x - awo->startx ) /
             (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

            ok = 1;
            cur = awo->selectedHead->selFlink;
            while ( cur != awo->selectedHead ) {

              newX = awo->masterSelectX0 +
               (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
               * xScaleFactor + 0.5 );

              newW =
               (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

              stat = cur->node->checkResizeSelectBoxAbs( (int) newX, -1,
               (int) newW, -1 );

              if ( stat == 0 ) ok = 0;

              cur = cur->selFlink;

            }

            if ( ok ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {

                newX = awo->masterSelectX0 +
                 (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
                 * xScaleFactor + 0.5 );

                newW =
                 (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

                cur->node->drawSelectBox(); //erase via xor gc
                cur->node->resizeSelectBoxAbs( (int) newX, -1, (int) newW,
                 -1 );
                cur->node->drawSelectBox();
                cur = cur->selFlink;

              }

              awo->oldx = me->x;
              awo->oldy = me->y;

            }

	    }

            break;

          case AWC_RESIZE_LEFT_TOP:

            if ( !awo->usingArrowKeys ) {

            xScaleFactor = 1.0 + (double ) ( awo->startx - me->x ) /
             (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

            yScaleFactor = 1.0 + (double) ( awo->starty - me->y ) /
             (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

            ok = 1;
            cur = awo->selectedHead->selFlink;
            while ( cur != awo->selectedHead ) {

              newX = awo->masterSelectX0 - awo->startx + me->x +
               (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
               * xScaleFactor + 0.5 );

              newW =
               (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

              newY = awo->masterSelectY0 - awo->starty + me->y +
               (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
               * yScaleFactor + 0.5 );

              newH =
               (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

              stat = cur->node->checkResizeSelectBoxAbs( (int) newX,
               (int) newY, (int) newW, (int) newH );

              if ( stat == 0 ) ok = 0;

              cur = cur->selFlink;

            }

            if ( ok ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {

                newX = awo->masterSelectX0 - awo->startx + me->x +
                 (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
                 * xScaleFactor + 0.5 );

                newW =
                 (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

                newY = awo->masterSelectY0 - awo->starty + me->y +
                 (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
                 * yScaleFactor + 0.5 );

                newH =
                 (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

                cur->node->drawSelectBox(); //erase via xor gc
                cur->node->resizeSelectBoxAbs( (int) newX, (int) newY,
                 (int) newW, (int) newH );
                cur->node->drawSelectBox();

                cur = cur->selFlink;

              }

              awo->oldx = me->x;
              awo->oldy = me->y;

            }

	    }

            break;

          case AWC_RESIZE_LEFT_BOTTOM:

            if ( !awo->usingArrowKeys ) {

            xScaleFactor = 1.0 + (double ) ( awo->startx - me->x ) /
             (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

            yScaleFactor = 1.0 + (double ) ( me->y - awo->starty ) /
             (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

            ok = 1;
            cur = awo->selectedHead->selFlink;
            while ( cur != awo->selectedHead ) {


              newX = awo->masterSelectX0 - awo->startx + me->x +
               (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
               * xScaleFactor + 0.5 );

              newW =
               (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

              newY = awo->masterSelectY0 +
               (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
               * yScaleFactor + 0.5 );

              newH =
               (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

              stat = cur->node->checkResizeSelectBoxAbs( (int) newX,
               (int) newY, (int) newW, (int) newH );

              if ( stat == 0 ) ok = 0;

              cur = cur->selFlink;
            }

            if ( ok ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {

                newX = awo->masterSelectX0 - awo->startx + me->x +
                 (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
                 * xScaleFactor + 0.5 );

                newW =
                 (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

                newY = awo->masterSelectY0 +
                 (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
                 * yScaleFactor + 0.5 );

                newH =
                 (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

                cur->node->drawSelectBox(); //erase via xor gc
                cur->node->resizeSelectBoxAbs( (int) newX,
                 (int) newY, (int) newW, (int) newH );
                cur->node->drawSelectBox();

                cur = cur->selFlink;

              }

              awo->oldx = me->x;
              awo->oldy = me->y;

            }

	    }

            break;

          case AWC_RESIZE_RIGHT_TOP:

            if ( !awo->usingArrowKeys ) {

            xScaleFactor = 1.0 + (double ) ( me->x - awo->startx ) /
             (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

            yScaleFactor = 1.0 + (double) ( awo->starty - me->y ) /
             (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

            ok = 1;
            cur = awo->selectedHead->selFlink;
            while ( cur != awo->selectedHead ) {

              newX = awo->masterSelectX0 +
               (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
               * xScaleFactor + 0.5 );

              newW =
               (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

              newY = awo->masterSelectY0 - awo->starty + me->y +
               (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
               * yScaleFactor + 0.5 );

              newH =
               (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

              stat = cur->node->checkResizeSelectBoxAbs( (int) newX,
               (int) newY, (int) newW, (int) newH );

              if ( stat == 0 ) ok = 0;

              cur = cur->selFlink;

            }

            if ( ok ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {

                newX = awo->masterSelectX0 +
                 (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
                 * xScaleFactor + 0.5 );

                newW =
                 (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

                newY = awo->masterSelectY0 - awo->starty + me->y +
                 (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
                 * yScaleFactor + 0.5 );

                newH =
                 (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

                cur->node->drawSelectBox(); //erase via xor gc
                cur->node->resizeSelectBoxAbs( (int) newX, (int) newY,
                 (int) newW, (int) newH );
                cur->node->drawSelectBox();

                cur = cur->selFlink;

              }

              awo->oldx = me->x;
              awo->oldy = me->y;

            }

	    }

            break;

          case AWC_RESIZE_RIGHT_BOTTOM:

            if ( !awo->usingArrowKeys ) {

            xScaleFactor = 1.0 + (double ) ( me->x - awo->startx ) /
             (double) ( awo->masterSelectX1 - awo->masterSelectX0 );

            yScaleFactor = 1.0 + (double ) ( me->y - awo->starty ) /
             (double) ( awo->masterSelectY1 - awo->masterSelectY0 );

            ok = 1;
            cur = awo->selectedHead->selFlink;
            while ( cur != awo->selectedHead ) {

              newX = awo->masterSelectX0 +
               (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
               * xScaleFactor + 0.5 );

              newW =
               (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

              newY = awo->masterSelectY0 +
               (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
               * yScaleFactor + 0.5 );

              newH =
               (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

              stat = cur->node->checkResizeSelectBoxAbs( (int) newX,
               (int) newY, (int) newW, (int) newH );

              if ( stat == 0 ) ok = 0;

              cur = cur->selFlink;

            }

            if ( ok ) {

              cur = awo->selectedHead->selFlink;
              while ( cur != awo->selectedHead ) {

                newX = awo->masterSelectX0 +
                 (int) ( (double) ( cur->node->getX0() - awo->masterSelectX0 )
                 * xScaleFactor + 0.5 );

                newW =
                 (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

                newY = awo->masterSelectY0 +
                 (int) ( (double) ( cur->node->getY0() - awo->masterSelectY0 )
                 * yScaleFactor + 0.5 );

                newH =
                 (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

                cur->node->drawSelectBox(); //erase via xor gc
                cur->node->resizeSelectBoxAbs( (int) newX,
                 (int) newY, (int) newW, (int) newH );
                cur->node->drawSelectBox();

                cur = cur->selFlink;

              }

              awo->oldx = me->x;
              awo->oldy = me->y;

            }

	    }

            break;

          default:

            break;

        }

      }

//======== B1 motion ============

    }
    else if ( me->state & Button2Mask ) {

      if ( me->state & ShiftMask ) {

//======== Shift B2 motion ============


//======== Shift B2 motion ============

      }
      else if ( me->state & ControlMask ) {

//======== Ctrl B2 motion ============


//======== Ctrl B2 motion ============

      }
      else {

//======== B2 motion ============

        switch ( awo->state ) {

	  case AWC_MOVING_POINT:

            stat = awo->currentPointObject->movePoint( awo->currentPoint,
             me->x, me->y );
            break;

	  case AWC_EDITING_POINTS:
            break;

          case AWC_NONE_SELECTED:
          case AWC_ONE_SELECTED:
          case AWC_MANY_SELECTED:

            if ( awo->state == AWC_NONE_SELECTED )
              awo->useFirstSelectedAsReference = 0;
            else if ( awo->state == AWC_ONE_SELECTED )
              awo->useFirstSelectedAsReference = 1;

            awo->savedState = awo->state;

            awo->state = AWC_START_DEFINE_SELECT_REGION;
            awo->startx = me->x;
            awo->starty = me->y;
            break;

          case AWC_START_DEFINE_SELECT_REGION:

            width = me->x - awo->startx;
            height = me->y - awo->starty;

            if ( width < 0 ) {
              x0 = me->x;
              width = abs( width );
            }
            else {
              x0 = awo->startx;
            }

            if ( height < 0 ) {
              y0 = me->y;
              height = abs( height );
            }
            else {
              y0 = awo->starty;
            }

            if ( ( width > 0 ) && ( height > 0 ) ) {

              awo->state = AWC_DEFINE_SELECT_REGION;
              awo->drawGc.saveFg();
              awo->drawGc.setFG( awo->ci->pix(awo->fgColor) );
              XDrawRectangle( awo->d, XtWindow(awo->drawWidget),
               awo->drawGc.xorGC(), x0, y0, width, height );
              awo->drawGc.restoreFg();

            }

            awo->oldx = me->x;
            awo->oldy = me->y;

            break;

          case AWC_DEFINE_SELECT_REGION:

            width = awo->oldx - awo->startx;
            height = awo->oldy - awo->starty;

            if ( width < 0 ) {
              x0 = awo->oldx;
              width = abs( width );
            }
            else {
              x0 = awo->startx;
            }

            if ( height < 0 ) {
              y0 = awo->oldy;
              height = abs( height );
            }
            else {
              y0 = awo->starty;
            }

            if ( ( width > 0 ) && ( height > 0 ) ) {

              awo->drawGc.saveFg();
              awo->drawGc.setFG( awo->ci->pix(awo->fgColor) );
              XDrawRectangle( awo->d, XtWindow(awo->drawWidget),
               awo->drawGc.xorGC(), x0, y0, width, height );
              awo->drawGc.restoreFg();

            }

            width = me->x - awo->startx;
            height = me->y - awo->starty;

            if ( width < 0 ) {
              x0 = me->x;
              width = abs( width );
            }
            else {
              x0 = awo->startx;
            }

            if ( height < 0 ) {
              y0 = me->y;
              height = abs( height );
            }
            else {
              y0 = awo->starty;
            }

            if ( ( width > 0 ) && ( height > 0 ) ) {

              awo->drawGc.saveFg();
              awo->drawGc.setFG( awo->ci->pix(awo->fgColor) );
              XDrawRectangle( awo->d, XtWindow(awo->drawWidget),
               awo->drawGc.xorGC(), x0, y0, width, height );
              awo->drawGc.restoreFg();

            }

            awo->oldx = me->x;
            awo->oldy = me->y;

            break;

        }

//======== B2 motion ============

      }

    }
    else if ( me->state & Button3Mask ) {

      if ( me->state & ShiftMask ) {

//======== Shift B3 motion ============


//======== Shift B3 motion ============

      }
      else if ( me->state & ControlMask ) {

//======== Ctrl B3 motion ============


//======== Ctrl B3 motion ============

      }
      else {

//======== B3 motion ============


//======== B3 motion ============

      }

    }

  }
  else {

  }

done:

  return;

}

void activeWinEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

XExposeEvent *expe;
XMotionEvent *me;
XButtonEvent *be;
XConfigureEvent *ce;
activeWindowClass *awo;
unsigned int mask;
btnActionListPtr curBtn;
int action, foundAction, numOut, numIn, buttonNum;
activeGraphicListPtr cur;

  awo = (activeWindowClass *) client;

  if ( awo->mode != AWC_EXECUTE ) goto done;

  foundAction = 0;

  awo->oldState = awo->state;

  *continueToDispatch = False;

  if ( e->type == ConfigureNotify ) {

    ce = (XConfigureEvent *) e;

    if ( ( awo->w != ce->width ) ||
         ( awo->h != ce->height ) ) {

      awo->setChanged();

      awo->w = ce->width;
      awo->h = ce->height;

    }

    if ( ( awo->x != ce->x ) ||
         ( awo->y != ce->y ) ) {

        awo->setChanged();

        awo->x = ce->x;
        awo->y = ce->y;

    }

  }
  else if ( e->type == Expose ) {

    expe = (XExposeEvent *) e;

      awo->refreshActive( expe->x, expe->y, expe->width, expe->height );

  }
  else if ( e->type == ButtonPress ) {

    if ( awo->state == AWC_WAITING ) goto done;

    mask = ShiftMask & ControlMask;

    be = (XButtonEvent *) e;

    if ( ( be->button == 2 ) && !( be->state & ShiftMask ) ) {

      cur = awo->head->blink;
      while ( cur != awo->head ) {

        if ( ( be->x > cur->node->getX0() ) &&
             ( be->x < cur->node->getX1() ) &&
             ( be->y > cur->node->getY0() ) &&
             ( be->y < cur->node->getY1() ) ) {

          // only the highest object may participate
          if ( cur->node->dragValue( cur->node->getCurrentDragIndex() ) ) {
            cur->node->startDrag( be->x, be->y );
            foundAction = 1;
	  }
          break; // out of while loop

	}

        cur = cur->blink;

      }

    }
    else if ( ( be->button == 2 ) && ( be->state & ShiftMask ) ) {
      // do nothing
    }
    else {

      curBtn = awo->btnDownActionHead->flink;
      while ( curBtn != awo->btnDownActionHead ) {

        if ( ( be->x > curBtn->node->getX0() ) &&
             ( be->x < curBtn->node->getX1() ) &&
             ( be->y > curBtn->node->getY0() ) &&
             ( be->y < curBtn->node->getY1() ) ) {

          foundAction = 1;
          action = 0;

          if ( curBtn->pressed == 0 ) curBtn->pressed = 1;
          curBtn->node->btnDown( be->x, be->y, be->state, be->button,
           &action );

          if ( action == 1 ) { /* close window */
            awo->returnToEdit( 1 );
            goto done;
          }

        }

        curBtn = curBtn->flink;

      }

    }

    if ( !foundAction ) {

    switch ( be->button ) {

      case Button1:

        if ( be->state & ShiftMask ) {

//========== Shift B1 Press ===================================

//========== Shift B1 Press ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B1 Press ===================================

//========== Ctrl B1 Press ===================================

        }
	else {

//========== B1 Press ===================================

//========== B1 Press ===================================

        }

        break;

      case Button2:

        if ( be->state & ShiftMask ) {

//========== Shift B2 Press ===================================

//========== Shift B2 Press ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B2 Press ===================================

//========== Ctrl B2 Press ===================================

        }
	else {

//========== B2 Press ===================================

//========== B2 Press ===================================

        }

        break;

      case Button3:

        if ( be->state & ShiftMask ) {

//========== Shift B3 Press ===================================

//========== Shift B3 Press ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B3 Press ===================================

//========== Ctrl B3 Press ===================================

        }
        else {

//========== B3 Press ===================================

//========== B3 Press ===================================

        }

        break;

    } // end switch ( be->button )

    } // if ( !foundAction )

  }
  else if ( e->type == ButtonRelease ) {

    if ( awo->state == AWC_WAITING ) goto done;

    be = (XButtonEvent *) e;

    if ( ( be->button == 2 ) && ( be->state & ShiftMask ) ) {

      cur = awo->head->blink;
      while ( cur != awo->head ) {

        if ( ( be->x > cur->node->getX0() ) &&
             ( be->x < cur->node->getX1() ) &&
             ( be->y > cur->node->getY0() ) &&
             ( be->y < cur->node->getY1() ) ) {

          foundAction = 1;

          cur->node->selectDragValue( be->x, be->y );
          break; // out of while loop

	}

        cur = cur->blink;

      }

    }
    else {

      curBtn = awo->btnDownActionHead->flink;
      while ( curBtn != awo->btnDownActionHead ) {
        if ( curBtn->pressed == 1 ) {
          foundAction = 1;
          curBtn->pressed = 0;
          curBtn->node->btnUp( be->x, be->y, be->state, be->button, &action );
          if ( action == 1 ) { /* close window */
            awo->returnToEdit( 1 );
            goto done;
          }
        }

        curBtn = curBtn->flink;

      }

    }

    if ( !foundAction ) {

    switch ( be->button ) {

      case Button1:

        if ( be->state & ShiftMask ) {

//========== Shift B1 Release ===================================

//========== Shift B1 Release ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B1 Release ===================================

//========== Ctrl B1 Release ===================================

        }
        else {

//========== B1 Release ===================================


//========== B1 Release ===================================

        }

        break;

      case Button2:


        if ( be->state & ShiftMask ) {

//========== Shift B2 Release ===================================

//========== Shift B2 Release ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B2 Release ===================================

//========== Ctrl B2 Release ===================================

        }
        else {

//========== B2 Release ===================================

          XmMenuPosition( awo->b2ExecutePopup, be );
          XtManageChild( awo->b2ExecutePopup );

          XSetWindowColormap( awo->d,
           XtWindow(XtParent(awo->b2ExecutePopup)),
           awo->appCtx->ci.getColorMap() );

//========== B2 Release ===================================

        }

        break;

      case Button3:

        if ( be->state & ShiftMask ) {

//========== Shift B3 Release ===================================

//========== Shift B3 Release ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B3 Release ===================================

//========== Ctrl B3 Release ===================================

        }
        else {

//========== B3 Release ===================================

//========== B3 Release ===================================

        }

        break;

    } // end switch ( be->button )

    } // if ( !foundAction )

  }
  else if ( e->type == MotionNotify ) {

    if ( awo->state == AWC_WAITING ) goto done;

    me = (XMotionEvent *) e;

    if ( ( me->state != 0 ) && !( me->state & Button2Mask ) ) {

      curBtn = awo->btnMotionActionHead->flink;
      while ( curBtn != awo->btnMotionActionHead ) {

        if ( ( me->x > curBtn->node->getX0() ) &&
             ( me->x < curBtn->node->getX1() ) &&
             ( me->y > curBtn->node->getY0() ) &&
             ( me->y < curBtn->node->getY1() ) ) {

          foundAction = 1;
          if ( me->state & Button1Mask ) {
            buttonNum = 1;
	  }
	  else if ( me->state & Button3Mask ) {
            buttonNum = 3;
	  }
          else {
            buttonNum = 0;
	  }
          if ( buttonNum ) curBtn->node->btnDrag( me->x, me->y,
           me->state, buttonNum );

        }

        curBtn = curBtn->flink;

      }

    }

    numIn = numOut = 0;

    curBtn = awo->btnFocusActionHead->flink;
    while ( curBtn != awo->btnFocusActionHead ) {

      if ( ( me->x > curBtn->node->getX0() ) &&
           ( me->x < curBtn->node->getX1() ) &&
           ( me->y > curBtn->node->getY0() ) &&
           ( me->y < curBtn->node->getY1() ) ) {

        if ( curBtn->in != 1 ) {
          curBtn->in = 1;
          curBtn->node->pointerIn( me->x, me->y, me->state );
          numIn++;
          foundAction = 1;
        }

      }
      else {

        if ( curBtn->in == 1 ) {
          curBtn->in = 0;
          curBtn->node->pointerOut( me->x, me->y, me->state );
          numOut++;
          foundAction = 1;
	}

      }

      curBtn = curBtn->flink;

    }

    if ( numIn ) {
      awo->showActive = 1;
    }
    else if ( numOut ) {
      awo->showActive = 0;
      awo->cursor.set( XtWindow(awo->executeWidget), CURSOR_K_DEFAULT );
    }

    if ( !foundAction ) {

    if ( me->state & Button1Mask ) {

      if ( me->state & ShiftMask ) {

//======== Shift B1 motion ============

//======== Shift B1 motion ============

      }
      else if ( me->state & ControlMask ) {

//======== Ctrl B1 motion ============

//======== Ctrl B1 motion ============

      }
      else {

//======== B1 motion ============

//======== B1 motion ============

      }

    }
    else if ( me->state & Button2Mask ) {

      if ( me->state & ShiftMask ) {

//======== Shift B2 motion ============

//======== Shift B2 motion ============

      }
      else if ( me->state & ControlMask ) {

//======== Ctrl B2 motion ============

//======== Ctrl B2 motion ============

      }
      else {

//======== B2 motion ============

//======== B2 motion ============

      }

    }
    else if ( me->state & Button3Mask ) {

      if ( me->state & ShiftMask ) {

//======== Shift B3 motion ============

//======== Shift B3 motion ============

      }
      else if ( me->state & ControlMask ) {

//======== Ctrl B3 motion ============

//======== Ctrl B3 motion ============

      }
      else {

//======== B3 motion ============

//======== B3 motion ============

      }

    }

    } // if ( !foundAction )

  }
  else {

//     printf( "unknown\n" );

  }

done:

  return;

}

activeWindowClass::activeWindowClass ( void ) {

  strcpy( startSignature, "edmActiveWindow" );
  strcpy( endSignature, "wodniWevitcAmde" );

  strcpy( defaultPvType, "" );

  dragItemIndex = 0;
  major = 0;
  minor = 0;
  release = 0;
  fileLineNumber = 0;
  dragPopup = NULL;
  buttonPressX = buttonPressY = -1;
  list_array_size = 0;
  list_array = NULL;
  noRefresh = 0;
  exit_after_save = 0;
  orthogonal = 0;
  orthoMove = 0;
  masterSelectX0 = masterSelectY0 = masterSelectX1 = masterSelectY1 = 0;
  isIconified = False;
  autosaveTimer = 0;
  doAutoSave = 0;
  doClose = 0;
  restoreTimer = 0;

  commentHead = new commentLinesType;
  commentHead->line = NULL;
  commentTail = commentHead;
  commentTail->flink = NULL;

  head = new activeGraphicListType;
  head->flink = head;
  head->blink = head;

  cutHead = new activeGraphicListType;
  cutHead->flink = cutHead;
  cutHead->blink = cutHead;

  selectedHead = new activeGraphicListType;
  selectedHead->selFlink = selectedHead;
  selectedHead->selBlink = selectedHead;

  defExeHead = new activeGraphicListType;
  defExeHead->defExeFlink = defExeHead;
  defExeHead->defExeBlink = defExeHead;

  enterActionHead = new btnActionListType;
  enterActionHead->flink = enterActionHead;
  enterActionHead->blink = enterActionHead;

  btnDownActionHead = new btnActionListType;
  btnDownActionHead->flink = btnDownActionHead;
  btnDownActionHead->blink = btnDownActionHead;

  btnUpActionHead = new btnActionListType;
  btnUpActionHead->flink = btnUpActionHead;
  btnUpActionHead->blink = btnUpActionHead;

  btnMotionActionHead = new btnActionListType;
  btnMotionActionHead->flink = btnMotionActionHead;
  btnMotionActionHead->blink = btnMotionActionHead;

  btnFocusActionHead = new btnActionListType;
  btnFocusActionHead->flink = btnFocusActionHead;
  btnFocusActionHead->blink = btnFocusActionHead;

  popupBlockHead = new popupBlockListType;
  popupBlockHead->flink = popupBlockHead;
  popupBlockHead->blink = popupBlockHead;

  objNameHead = new objNameListType;
  objNameHead->flink = objNameHead;
  objNameHead->blink = objNameHead;

  eventHead = new eventListType;
  eventHead->flink = eventHead;
  eventHead->blink = eventHead;

  limEventHead = new eventListType;
  limEventHead->flink = limEventHead;
  limEventHead->blink = limEventHead;

  pollHead = new pollListType;
  pollHead->flink = pollHead;
  pollHead->blink = pollHead;

  strcpy( id, "" );
  strcpy( title, "" );
  strcpy( autosaveName, "" );
  showName = 0;
  numMacros = 0;

  activateCallbackFlag = deactivateCallbackFlag = 0;

  setSchemePd = NULL;
  setSchemeCb = NULL;
  chPd = NULL;
  grPd = NULL;
  grCb = NULL;
  mnPd = NULL;
  mnCb = NULL;
  ctlPd = NULL;
  ctlCb = NULL;
  alignPd = NULL;
  alignCb = NULL;
  centerPd = NULL;
  centerCb = NULL;
  distributePd = NULL;
  distributeCb = NULL;
  sizePd = NULL;
  sizeCb = NULL;
  orientPd1 = NULL;
  orientPdM = NULL;
  orientCb1 = NULL;
  orientCbM = NULL;
  editPd1 = NULL;
  editPdM = NULL;
  editCb1 = NULL;
  editCbM = NULL;
  b1NoneSelectPopup = NULL;
  b1OneSelectPopup = NULL;
  b2NoneSelectPopup = NULL;
  b2OneSelectPopup = NULL;
  b2ManySelectPopup = NULL;
  b2ExecutePopup = NULL;
  drawWidget = NULL;
  top = NULL;

  useComponentScheme = 0;
  allSelectedTextFgColorFlag = 1;
  allSelectedFg1ColorFlag = 1;
  allSelectedFg2ColorFlag = 1;
  allSelectedBgColorFlag = 1;
  allSelectedOffsetColorFlag = 1;
  allSelectedTopShadowColorFlag = 1;
  allSelectedBotShadowColorFlag = 1;
  allSelectedFontTagFlag = 1;
  allSelectedAlignmentFlag = 1;
  allSelectedCtlFontTagFlag = 1;
  allSelectedCtlAlignmentFlag = 1;
  allSelectedBtnFontTagFlag = 1;
  allSelectedBtnAlignmentFlag = 1;

  strcpy( allSelectedCtlPvName[0], "" );
  strcpy( allSelectedReadbackPvName[0], "" );
  strcpy( allSelectedNullPvName[0], "" );
  strcpy( allSelectedVisPvName[0], "" );
  strcpy( allSelectedAlarmPvName[0], "" );

  allSelectedCtlPvNameFlag = 1;
  allSelectedReadbackPvNameFlag = 1;
  allSelectedNullPvNameFlag = 1;
  allSelectedVisPvNameFlag = 1;
  allSelectedAlarmPvNameFlag = 1;

  versionStackPtr = 0;

  buttonClickTime = 0;
  deltaTime = 0;

  msgDialogCreated = 0;
  msgDialogPoppedUp = 0;

  strcpy( curSchemeSet, "" );

}

int activeWindowClass::pushVersion ( void ) {

  if ( versionStackPtr > 9 ) return 0; // overflow

  versionStack[versionStackPtr][0] = major;
  versionStack[versionStackPtr][1] = minor;
  versionStack[versionStackPtr][2] = release;

  versionStackPtr++;

  return 1;

}

int activeWindowClass::popVersion ( void ) {

  if ( versionStackPtr == 0 ) return 0; // underflow

  versionStackPtr--;

  major = versionStack[versionStackPtr][0];
  minor = versionStack[versionStackPtr][1];
  release = versionStack[versionStackPtr][2];

  return 1;

}

activeWindowClass::~activeWindowClass ( void ) {

int i, stat;
popupBlockListPtr curPopupBlock, nextPopupBlock;
activeGraphicListPtr curCut, nextCut, cur, next;
objNameListPtr curObjName, nextObjName;
commentLinesPtr commentCur, commentNext;

  if ( dragPopup ) {
    XtDestroyWidget( dragPopup );
    dragPopup = NULL;
  }

  if ( autosaveTimer ) {
    XtRemoveTimeOut( autosaveTimer );
    autosaveTimer = 0;
  }
  if ( restoreTimer ) {
    XtRemoveTimeOut( restoreTimer );
    restoreTimer = 0;
  }

  commentCur = commentHead->flink;
  while ( commentCur ) {
    commentNext = commentCur->flink;
    if ( commentCur->line ) delete commentCur->line;
    delete commentCur;
    commentCur = commentNext;
  }
  commentTail = commentHead;
  commentTail->flink = NULL;
  delete commentHead;

  if ( ef.formIsPoppedUp() ) ef.popdown();

  if ( strcmp( autosaveName, "" ) != 0 ) {
    stat = unlink( autosaveName );
  }

 if ( list_array_size > 0 ) delete list_array;

  // empty cut list
  curCut = cutHead->flink;
  while ( curCut != cutHead ) {
    nextCut = curCut->flink;
    delete curCut->node;
    delete curCut;
    curCut = nextCut;
  }

  if ( cutHead ) delete cutHead;

  // empty main list
  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  if ( head ) delete head;

  if ( selectedHead ) delete selectedHead;

  if ( defExeHead ) delete defExeHead;

  if ( enterActionHead ) delete enterActionHead;

  if ( btnDownActionHead ) delete btnDownActionHead;

  if ( btnUpActionHead ) delete btnUpActionHead;

  if ( btnMotionActionHead ) delete btnMotionActionHead;

  if ( btnFocusActionHead ) delete btnFocusActionHead;

  curPopupBlock = popupBlockHead->flink;
  while ( curPopupBlock != popupBlockHead ) {
    nextPopupBlock = curPopupBlock->flink;
    XtRemoveCallback( curPopupBlock->block.w, XmNactivateCallback,
     b2ReleaseNoneSelect_cb, (XtPointer) &curPopupBlock->block );
    XtDestroyWidget( curPopupBlock->block.w );
    delete curPopupBlock;
    curPopupBlock = nextPopupBlock;
  }

  if ( popupBlockHead ) delete popupBlockHead;

  curObjName = objNameHead->flink;
  while ( curObjName != objNameHead ) {
    nextObjName = curObjName->flink;
    XtRemoveCallback( curObjName->w, XmNactivateCallback, createPopup_cb,
     (XtPointer) this );
    XtDestroyWidget( curObjName->w );
    delete curObjName->objType;
    delete curObjName;
    curObjName = nextObjName;
  }

  if ( objNameHead ) delete objNameHead;

  if ( eventHead ) delete eventHead;

  if ( limEventHead ) delete limEventHead;

  if ( pollHead ) delete pollHead;

  for ( i=0; i<numMacros; i++ ) {
    delete macros[i];
    delete expansions[i];
  }

  if ( numMacros > 0 ) {
    delete macros;
    delete expansions;
  }

  if ( top ) {
    XtRemoveEventHandler( top, StructureNotifyMask, False,
     topWinEventHandler, (XtPointer) this );
  }

  if ( drawWidget ) {
    XtRemoveEventHandler( drawWidget,
     KeyPressMask|ButtonPressMask|ButtonReleaseMask|Button1MotionMask|
     Button2MotionMask|Button3MotionMask|ExposureMask, False,
     drawWinEventHandler, (XtPointer) this );
  }

  if ( setSchemePd ) XtDestroyWidget( setSchemePd );
  if ( setSchemeCb ) XtDestroyWidget( setSchemeCb );
  if ( chPd ) XtDestroyWidget( chPd );
  if ( grPd ) XtDestroyWidget( grPd );
  if ( grCb ) XtDestroyWidget( grCb );
  if ( mnPd ) XtDestroyWidget( mnPd );
  if ( mnCb ) XtDestroyWidget( mnCb );
  if ( ctlPd ) XtDestroyWidget( ctlPd );
  if ( ctlCb ) XtDestroyWidget( ctlCb );
  if ( alignPd ) XtDestroyWidget( alignPd );
  if ( alignCb ) XtDestroyWidget( alignCb );
  if ( distributePd ) XtDestroyWidget( distributePd );
  if ( distributeCb ) XtDestroyWidget( distributeCb );
  if ( centerPd ) XtDestroyWidget( centerPd );
  if ( centerCb ) XtDestroyWidget( centerCb );
  if ( sizePd ) XtDestroyWidget( sizePd );
  if ( sizeCb ) XtDestroyWidget( sizeCb );
  if ( orientPd1 ) XtDestroyWidget( orientPd1 );
  if ( orientPdM ) XtDestroyWidget( orientPdM );
  if ( orientCb1 ) XtDestroyWidget( orientCb1 );
  if ( orientCbM ) XtDestroyWidget( orientCbM );
  if ( editPd1 ) XtDestroyWidget( editPd1 );
  if ( editPdM ) XtDestroyWidget( editPdM );
  if ( editCb1 ) XtDestroyWidget( editCb1 );
  if ( editCbM ) XtDestroyWidget( editCbM );

  if ( b1OneSelectPopup ) XtDestroyWidget( b1OneSelectPopup );
  if ( b1NoneSelectPopup ) XtDestroyWidget( b1NoneSelectPopup );
  if ( b2NoneSelectPopup ) XtDestroyWidget( b2NoneSelectPopup );
  if ( b2OneSelectPopup ) XtDestroyWidget( b2OneSelectPopup );
  if ( b2ManySelectPopup ) XtDestroyWidget( b2ManySelectPopup );
  if ( b2ExecutePopup ) XtDestroyWidget( b2ExecutePopup );

  if ( drawWidget ) XtDestroyWidget( drawWidget );

  if ( msgDialogCreated ) msgDialog.destroy();

  if ( top ) XtDestroyWidget( top );

}

void activeWindowClass::updateAllSelectedDisplayInfo ( void ) {

  allSelectedTextFgColor = defaultTextFgColor;
  allSelectedFg1Color = defaultFg1Color;
  allSelectedFg2Color = defaultFg2Color;
  allSelectedBgColor = defaultBgColor;
  allSelectedOffsetColor = defaultOffsetColor;
  allSelectedTopShadowColor = defaultTopShadowColor;
  allSelectedBotShadowColor = defaultBotShadowColor;
  strcpy( allSelectedFontTag, defaultFontTag );
  allSelectedAlignment = defaultAlignment;
  strcpy( allSelectedCtlFontTag, defaultCtlFontTag );
  allSelectedCtlAlignment = defaultCtlAlignment;
  strcpy( allSelectedBtnFontTag, defaultBtnFontTag );
  allSelectedBtnAlignment = defaultBtnAlignment;

}

void activeWindowClass::expandTitle (
  int phase,
  int nMac,
  char **mac,
  char **exp
) {

  if ( phase == 1 )
    expStrTitle.expand1st( nMac, mac, exp );
  else
    expStrTitle.expand2nd( nMac, mac, exp );

}

void activeWindowClass::setTitle ( void ) {

XTextProperty xtext;
char *cptr;
char *none = activeWindowClass_str83;

  if ( showName || ( mode == AWC_EDIT ) ) {

    if ( strcmp( fileName, "" ) == 0 ) {
      cptr = none;
    }
    else {
      cptr = fileName;
    }

  }
  else {

    if ( !expStrTitle.getExpanded() ) {
      if ( strcmp( fileName, "" ) == 0 ) {
        cptr = none;
      }
      else {
        cptr = fileName;
      }
    }
    else if ( strcmp( expStrTitle.getExpanded(), "" ) == 0 ) {
      if ( strcmp( fileName, "" ) == 0 ) {
        cptr = none;
      }
      else {
        cptr = fileName;
      }
    }
    else {
      cptr = expStrTitle.getExpanded();
    }

  }

  XStringListToTextProperty( &cptr, 1, &xtext );
  XSetWMName( d, XtWindow(top), &xtext );
  XSetWMIconName( d, XtWindow(top), &xtext );

  XFree( xtext.value );

}

void activeWindowClass::setTitleUsingTitle ( void ) {

XTextProperty xtext;
char *cptr;
char *none = activeWindowClass_str83;

  if (  !expStrTitle.getExpanded() ) {
    if ( strcmp( fileName, "" ) == 0 ) {
      cptr = none;
    }
    else {
      cptr = fileName;
    }
  }
  else if ( strcmp( expStrTitle.getExpanded(), "" ) == 0 ) {
    if ( strcmp( fileName, "" ) == 0 ) {
      cptr = none;
    }
    else {
      cptr = fileName;
    }
  }
  else {
    cptr = expStrTitle.getExpanded();
  }

  XStringListToTextProperty( &cptr, 1, &xtext );
  XSetWMName( d, XtWindow(top), &xtext );
  XSetWMIconName( d, XtWindow(top), &xtext );

  XFree( xtext.value );

}

void activeWindowClass::filterPosition (
  int *_x,
  int *_y,
  int oldX,
  int oldY )
{

int newX, newY;

  if ( orthoMove ) {
    if ( ( *_x != oldX ) || ( *_y != oldY ) ) {
      if ( abs( *_x - oldX ) >= abs( *_y - oldY ) ) {
        *_y = oldY;
      }
      else {
        *_x = oldX;
      }
    }
  }

  if ( !gridActive ) return;

  newX = ( *_x + gridSpacing/2 ) / gridSpacing;
  newX *= gridSpacing;

  newY = ( *_y + gridSpacing/2 ) / gridSpacing;
  newY *= gridSpacing;

  *_x = newX;
  *_y = newY;

}

int activeWindowClass::drawAfterResizeAbs (
  activeWindowClass *actWin,
  int deltaX,
  double xScaleFactor,
  int deltaY,
  double yScaleFactor )

{

activeGraphicListPtr cur;
double newX, newY, newW, newH;

  cur = this->selectedHead->selFlink;
  while ( cur != this->selectedHead ) {
    cur->node->eraseSelectBox();
    cur = cur->selFlink;
  }

  // erase selected
  cur = this->selectedHead->selFlink;
  while ( cur != this->selectedHead ) {

    cur->node->erase();

    newX = actWin->masterSelectX0 + deltaX +
     (int) ( (double) ( cur->node->getX0() - actWin->masterSelectX0 )
     * xScaleFactor + 0.5 );

    newW =
     (int) ( (double) cur->node->getW() * xScaleFactor + 0.5 );

    newY = actWin->masterSelectY0 + deltaY +
     (int) ( (double) ( cur->node->getY0() - actWin->masterSelectY0 )
     * yScaleFactor + 0.5 );

    newH =
     (int) ( (double) cur->node->getH() * yScaleFactor + 0.5 );

    cur->node->resizeAbs( (int) newX, (int) newY, (int) newW, (int) newH );

    cur->node->snapSizeToGrid();

    cur = cur->selFlink;

  }

  if ( actWin->gridShow ) actWin->displayGrid();

  // draw everything
  cur = this->head->flink;
  while ( cur != this->head ) {
    cur->node->draw();
    cur = cur->flink;
  }

  cur = this->head->flink;
  while ( cur != this->head ) {
    if ( cur->node->isSelected() ) cur->node->drawSelectBoxCorners();
    cur = cur->flink;
  }

  return 1;

}

int activeWindowClass::drawAfterResize (
  activeWindowClass *actWin,
  int deltax,
  int deltay,
  int deltaw,
  int deltah )

{

activeGraphicListPtr cur;

  cur = this->selectedHead->selFlink;
  while ( cur != this->selectedHead ) {
    cur->node->eraseSelectBox();
    cur = cur->selFlink;
  }

  // erase selected
  cur = this->selectedHead->selFlink;
  while ( cur != this->selectedHead ) {
    cur->node->erase();
    cur->node->resize( deltax, deltay, deltaw, deltah );
    cur->node->snapSizeToGrid();
    cur = cur->selFlink;
  }

  if ( actWin->gridShow ) actWin->displayGrid();

  // draw everything
  cur = this->head->flink;
  while ( cur != this->head ) {
    cur->node->draw();
    cur = cur->flink;
  }

  cur = this->head->flink;
  while ( cur != this->head ) {
    if ( cur->node->isSelected() ) cur->node->drawSelectBoxCorners();
    cur = cur->flink;
  }

  return 1;

}

int activeWindowClass::createAutoPopup (
  appContextClass *ctx,
  Widget parent,
  int OneX,
  int OneY,
  int OneW,
  int OneH,
  int _numMacros,
  char **_macros,
  char **_expansions ) {

  return genericCreate(
   ctx,
   parent,
   OneX,
   OneY,
   OneW,
   OneH,
   0,
   ctx->noEdit,
   0,
   _numMacros,
   _macros,
   _expansions );

}

int activeWindowClass::create (
  appContextClass *ctx,
  Widget parent,
  int OneX,
  int OneY,
  int OneW,
  int OneH,
  int _numMacros,
  char **_macros,
  char **_expansions ) {

  return genericCreate(
   ctx,
   parent,
   OneX,
   OneY,
   OneW,
   OneH,
   1,
   ctx->noEdit,
   1,
   _numMacros,
   _macros,
   _expansions );

}

int activeWindowClass::createNoEdit (
  appContextClass *ctx,
  Widget parent,
  int OneX,
  int OneY,
  int OneW,
  int OneH,
  int _numMacros,
  char **_macros,
  char **_expansions ) {

  return genericCreate(
   ctx,
   parent,
   OneX,
   OneY,
   OneW,
   OneH,
   1,
   1,
   1,
   _numMacros,
   _macros,
   _expansions );

}

int activeWindowClass::genericCreate (
  appContextClass *ctx,
  Widget parent,
  int OneX,
  int OneY,
  int OneW,
  int OneH,
  int windowDecorations,
  int _noEdit,
  int closeAllowed,
  int _numMacros,
  char **_macros,
  char **_expansions ) {

XmString str;
Widget pb;
objNameListPtr curObjNameNode;
char *oneObjName, *menuName;
popupBlockListPtr curBlockListNode;
int i, l, n, wPix, bPix;
Arg args[3];
unsigned int crc = 0;
Atom wm_delete_window;

  appCtx = ctx;

  autosaveTimer = 0;
  doAutoSave = 0;
  doClose = 0;
  restoreTimer = 0;

  change = 0;
  changeSinceAutoSave = 0;
  exit_after_save = 0;

  this->numMacros = _numMacros;

  if ( _numMacros ) {

    l = _numMacros;
    this->macros = new (char *)[l];
    this->expansions = new (char *)[l];

    for ( i=0; i<_numMacros; i++ ) {

      l = strlen(_macros[i]) + 1;
      this->macros[i] = new char[l];
      strcpy( this->macros[i], _macros[i] );
      crc = updateCRC( crc, this->macros[i], l-1 );

      l = strlen(_expansions[i]) + 1;
      this->expansions[i] = new char[l];
      strcpy( this->expansions[i], _expansions[i] );
      crc = updateCRC( crc, this->expansions[i], l-1 );

    }

  }
  else {

    this->macros = NULL;
    this->expansions = NULL;

  }

  this->crc = crc;

  state = AWC_NONE_SELECTED;
  updateMasterSelection();

  currentEf = NULL;

  oldx = -1;
  oldy = -1;

  useFirstSelectedAsReference = 0;

  d = appCtx->getDisplay();

  x = OneX;
  y = OneY;
  w = OneW;
  h = OneH;

  gridActive = 0;
  strcpy( gridActiveStr, activeWindowClass_str84 );
  gridShow = 0;
  strcpy( gridShowStr, activeWindowClass_str84 );
  gridSpacing = 10;
  oldGridSpacing = gridSpacing;
  orthogonal = 0;
  orthoMove = 0;
  strcpy( windowControlName, "" );
  strcpy( windowIdName, "" );
  strcpy( title, "" );
  showName = 0;
  strcpy( fileName, "" );
  ruler = 0;
  strcpy( rulerUnits, activeWindowClass_str85 );

  wPix = appCtx->ci.pixIndex( WhitePixel( d, DefaultScreen(d) ) );
  bPix = appCtx->ci.pixIndex( BlackPixel( d, DefaultScreen(d) ) );

  bgColor = wPix;
  defaultBgColor = wPix;

  fgColor = bPix; // for grid
  defaultTextFgColor = bPix; // for grid
  defaultFg1Color = bPix; // for grid
  defaultFg2Color = wPix; // for grid
  defaultTopShadowColor = wPix;
  defaultBotShadowColor = bPix;
  defaultOffsetColor = bPix;

  strcpy( defaultFontTag, "" );
  defaultAlignment = XmALIGNMENT_BEGINNING;
  strcpy( defaultCtlFontTag, "" );
  defaultCtlAlignment = XmALIGNMENT_BEGINNING;
  strcpy( defaultBtnFontTag, "" );
  defaultBtnAlignment = XmALIGNMENT_BEGINNING;

  updateAllSelectedDisplayInfo();

  mode = AWC_EDIT;

  if ( !parent ) {

    top = XtVaAppCreateShell( "", "", topLevelShellWidgetClass,
     d,
     XmNmappedWhenManaged, False,
     XmNmwmDecorations, windowDecorations,
     NULL );

    drawWidget = XtVaCreateManagedWidget( "", xmDrawingAreaWidgetClass,
     top,
     XmNwidth, w,
     XmNheight, h,
     XmNmappedWhenManaged, False,
     NULL );

  }
  else {

    top = parent;

    drawWidget = XtVaCreateManagedWidget( "", xmDrawingAreaWidgetClass,
     top,
     XmNx, x,
     XmNy, y,
     XmNwidth, w,
     XmNheight, h,
     XmNmappedWhenManaged, False,
     NULL );

  }

  executeWidget = drawWidget;

  // This handles close on the window manager.

  wm_delete_window = XmInternAtom( XtDisplay(top), "WM_DELETE_WINDOW",
   False );

  XmAddWMProtocolCallback( top, wm_delete_window, awc_WMExit_cb,
    (XtPointer) this );

  XtVaSetValues( top, XmNdeleteResponse, XmDO_NOTHING, NULL );

  XtAddEventHandler( top,
    StructureNotifyMask, False,
   topWinEventHandler, (XtPointer) this );

  XtAddEventHandler( drawWidget,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask|Button1MotionMask|
    Button2MotionMask|Button3MotionMask|ExposureMask, False,
   drawWinEventHandler, (XtPointer) this );

  // create drawing popup menus

//===================================================================

  n = 0;
  XtSetArg( args[n], XmNmenuPost, (XtArgVal) "<Btn5Down>;" ); n++;
  b1OneSelectPopup = XmCreatePopupMenu( top, "", args, n );

  chPd = XmCreatePulldownMenu( b1OneSelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str86 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b1OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_EDIT_LINE_PROP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b1ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );

  str = XmStringCreateLocalized( activeWindowClass_str87 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b1OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_EDIT_LINE_SEG;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b1ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );

//===================================================================

  n = 0;
  XtSetArg( args[n], XmNmenuPost, (XtArgVal) "<Btn5Down>;" ); n++;
  b1NoneSelectPopup = XmCreatePopupMenu( top, "", args, n );

  grPd = XmCreatePulldownMenu( b1NoneSelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( global_str3 );

  grCb = XtVaCreateManagedWidget( global_str3,
   xmCascadeButtonWidgetClass,
   b1NoneSelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, grPd,
   NULL );

  XmStringFree( str );

  oneObjName = obj.firstObjName( global_str3 );
  while ( oneObjName ) {

    menuName = obj.getNameFromClass( oneObjName );

    str = XmStringCreateLocalized( menuName );

    pb = XtVaCreateManagedWidget( "",
     xmPushButtonWidgetClass,
     grPd,
     XmNlabelString, str,
     NULL );

    XmStringFree( str );

    XtAddCallback( pb, XmNactivateCallback, createPopup_cb,
     (XtPointer) this );

    curObjNameNode = new objNameListType;
    curObjNameNode->w = pb;
    curObjNameNode->objName = oneObjName;
    curObjNameNode->objType = new char[strlen(global_str3)+1];
    strcpy( curObjNameNode->objType, global_str3 );

    curObjNameNode->blink = objNameHead->blink;
    objNameHead->blink->flink = curObjNameNode;
    objNameHead->blink = curObjNameNode;
    curObjNameNode->flink = objNameHead;

    oneObjName = obj.nextObjName( global_str3 );

  }

  mnPd = XmCreatePulldownMenu( b1NoneSelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( global_str2 );

  mnCb = XtVaCreateManagedWidget( global_str2,
   xmCascadeButtonWidgetClass,
   b1NoneSelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, mnPd,
   NULL );

  XmStringFree( str );

  oneObjName = obj.firstObjName( global_str2 );
  while ( oneObjName ) {

    menuName = obj.getNameFromClass( oneObjName );

    str = XmStringCreateLocalized( menuName );

    pb = XtVaCreateManagedWidget( "",
     xmPushButtonWidgetClass,
     mnPd,
     XmNlabelString, str,
     NULL );

    XmStringFree( str );

    XtAddCallback( pb, XmNactivateCallback, createPopup_cb,
     (XtPointer) this );

    curObjNameNode = new objNameListType;
    curObjNameNode->w = pb;
    curObjNameNode->objName = oneObjName;
    curObjNameNode->objType = new char[strlen(global_str2)+1];
    strcpy( curObjNameNode->objType, global_str2 );

    curObjNameNode->blink = objNameHead->blink;
    objNameHead->blink->flink = curObjNameNode;
    objNameHead->blink = curObjNameNode;
    curObjNameNode->flink = objNameHead;

    oneObjName = obj.nextObjName( global_str2 );

  }

  ctlPd = XmCreatePulldownMenu( b1NoneSelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( global_str5 );

  ctlCb = XtVaCreateManagedWidget( global_str5,
   xmCascadeButtonWidgetClass,
   b1NoneSelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, ctlPd,
   NULL );

  XmStringFree( str );

  oneObjName = obj.firstObjName( global_str5 );
  while ( oneObjName ) {

    menuName = obj.getNameFromClass( oneObjName );

    str = XmStringCreateLocalized( menuName );

    pb = XtVaCreateManagedWidget( "",
     xmPushButtonWidgetClass,
     ctlPd,
     XmNlabelString, str,
     NULL );

    XmStringFree( str );

    XtAddCallback( pb, XmNactivateCallback, createPopup_cb,
     (XtPointer) this );

    curObjNameNode = new objNameListType;
    curObjNameNode->w = pb;
    curObjNameNode->objName = oneObjName;
    curObjNameNode->objType = new char[strlen(global_str5)+1];
    strcpy( curObjNameNode->objType, global_str5 );

    curObjNameNode->blink = objNameHead->blink;
    objNameHead->blink->flink = curObjNameNode;
    objNameHead->blink = curObjNameNode;
    curObjNameNode->flink = objNameHead;

    oneObjName = obj.nextObjName( global_str5 );

  }

//===================================================================

  n = 0;
  XtSetArg( args[n], XmNmenuPost, (XtArgVal) "<Btn5Down>;" ); n++;
  b2NoneSelectPopup = XmCreatePopupMenu( top, "", args, n );

  str = XmStringCreateLocalized( activeWindowClass_str92 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_EXECUTE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  setSchemePd = NULL;
  if ( appCtx->numSchemeSets ) {

    setSchemePd = XmCreatePulldownMenu( b2NoneSelectPopup, "", NULL, 0 );

    str = XmStringCreateLocalized( activeWindowClass_str186 );

    setSchemeCb = XtVaCreateManagedWidget( "Select Scheme Set",
     xmCascadeButtonWidgetClass,
     b2NoneSelectPopup,
     XmNlabelString, str,
     XmNsubMenuId, setSchemePd,
     NULL );

    XmStringFree( str );

    str = XmStringCreateLocalized( "None" );

    pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
     setSchemePd,
     XmNlabelString, str,
     NULL );

    XmStringFree( str );

    curBlockListNode = new popupBlockListType;
    curBlockListNode->block.w = pb;
    curBlockListNode->block.ptr = (void *) NULL;
    curBlockListNode->block.awo = this;

    curBlockListNode->blink = popupBlockHead->blink;
    popupBlockHead->blink->flink = curBlockListNode;
    popupBlockHead->blink = curBlockListNode;
    curBlockListNode->flink = popupBlockHead;

    XtAddCallback( pb, XmNactivateCallback, selectScheme_cb,
     (XtPointer) &curBlockListNode->block );

    for ( i=0; i<appCtx->numSchemeSets; i++ ) {

      str = XmStringCreateLocalized( appCtx->schemeSetList[i] );

      pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
       setSchemePd,
       XmNlabelString, str,
       NULL );

      XmStringFree( str );

      curBlockListNode = new popupBlockListType;
      curBlockListNode->block.w = pb;
      curBlockListNode->block.ptr = (void *) appCtx->schemeSetList[i];
      curBlockListNode->block.awo = this;

      curBlockListNode->blink = popupBlockHead->blink;
      popupBlockHead->blink->flink = curBlockListNode;
      popupBlockHead->blink = curBlockListNode;
      curBlockListNode->flink = popupBlockHead;

      XtAddCallback( pb, XmNactivateCallback, selectScheme_cb,
       (XtPointer) &curBlockListNode->block );

    }

  }


  str = XmStringCreateLocalized( activeWindowClass_str93 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SAVE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str94 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SAVE_AS;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str185 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SELECT_ALL;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str95 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_PASTE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str167 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_PASTE_IN_PLACE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str96 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_PROPERTIES;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str97 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_CLOSE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


   str = XmStringCreateLocalized( activeWindowClass_str98 );

   pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
    b2NoneSelectPopup,
    XmNlabelString, str,
    NULL );

   XmStringFree( str );

   curBlockListNode = new popupBlockListType;
   curBlockListNode->block.w = pb;
   curBlockListNode->block.ptr = (void *) AWC_POPUP_OPEN;
   curBlockListNode->block.awo = this;

   curBlockListNode->blink = popupBlockHead->blink;
   popupBlockHead->blink->flink = curBlockListNode;
   popupBlockHead->blink = curBlockListNode;
   curBlockListNode->flink = popupBlockHead;

   XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
    (XtPointer) &curBlockListNode->block );


#if 0
   str = XmStringCreateLocalized( activeWindowClass_str99 );

   pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
    b2NoneSelectPopup,
    XmNlabelString, str,
    NULL );

   XmStringFree( str );

   curBlockListNode = new popupBlockListType;
   curBlockListNode->block.w = pb;
   curBlockListNode->block.ptr = (void *) AWC_POPUP_OPEN_USER;
   curBlockListNode->block.awo = this;

   curBlockListNode->blink = popupBlockHead->blink;
   popupBlockHead->blink->flink = curBlockListNode;
   popupBlockHead->blink = curBlockListNode;
   curBlockListNode->flink = popupBlockHead;

   XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
    (XtPointer) &curBlockListNode->block );
#endif

  str = XmStringCreateLocalized( activeWindowClass_str100 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_LOAD_SCHEME;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str101 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SAVE_SCHEME;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str102 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_MAKESYMBOL;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str103 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_OUTLIERS;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str104 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_FINDTOP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str169 );

  undoPb1 = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   XmNsensitive, 0,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = undoPb1;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_UNDO;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( undoPb1, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str105 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_REFRESH;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str184 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_HELP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );

//===================================================================

  n = 0;
  XtSetArg( args[n], XmNmenuPost, (XtArgVal) "<Btn5Down>;" ); n++;
  b2OneSelectPopup = XmCreatePopupMenu( top, "", args, n );

  str = XmStringCreateLocalized( activeWindowClass_str106 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_RAISE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str107 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_LOWER;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str108 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_COPY;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str109 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_CUT;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str110 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_PASTE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str167 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_PASTE_IN_PLACE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str111 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_GROUP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str112 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_UNGROUP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  orientPd1 = XmCreatePulldownMenu( b2OneSelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str176 );

  orientCb1 = XtVaCreateManagedWidget( activeWindowClass_str176,
   xmCascadeButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, orientPd1,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str177 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   orientPd1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ROTATE_CW;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );

  str = XmStringCreateLocalized( activeWindowClass_str178 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   orientPd1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ROTATE_CCW;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );

  str = XmStringCreateLocalized( activeWindowClass_str179 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   orientPd1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_FLIP_H;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );

  str = XmStringCreateLocalized( activeWindowClass_str180 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   orientPd1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_FLIP_V;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  editPd1 = XmCreatePulldownMenu( b2OneSelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str140 );

  editCb1 = XtVaCreateManagedWidget( activeWindowClass_str140,
   xmCascadeButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, editPd1,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str141 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   editPd1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_CHANGE_DSP_PARAMS;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );

  str = XmStringCreateLocalized( activeWindowClass_str142 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   editPd1,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_CHANGE_PV_NAMES;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str143 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_DESELECT;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str169 );

  undoPb2 = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   XmNsensitive, 0,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = undoPb2;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_UNDO;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( undoPb2, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str114 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_REFRESH;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str184 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_HELP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );

//===================================================================
 
  n = 0;
  XtSetArg( args[n], XmNmenuPost, (XtArgVal) "<Btn5Down>;" ); n++;
  b2ManySelectPopup = XmCreatePopupMenu( top, "", args, n );

  str = XmStringCreateLocalized( activeWindowClass_str115 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_RAISE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str116 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_LOWER;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str117 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_COPY;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str118 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_CUT;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str119 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_PASTE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str167 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_PASTE_IN_PLACE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str120 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_GROUP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str121 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_UNGROUP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  orientPdM = XmCreatePulldownMenu( b2ManySelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str176 );

  orientCbM = XtVaCreateManagedWidget( activeWindowClass_str176,
   xmCascadeButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, orientPdM,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str177 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   orientPdM,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ROTATE_CW;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );

  str = XmStringCreateLocalized( activeWindowClass_str178 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   orientPdM,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ROTATE_CCW;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );

  str = XmStringCreateLocalized( activeWindowClass_str179 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   orientPdM,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_FLIP_H;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );

  str = XmStringCreateLocalized( activeWindowClass_str180 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   orientPdM,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_FLIP_V;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  alignPd = XmCreatePulldownMenu( b2ManySelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str122 );

  alignCb = XtVaCreateManagedWidget( activeWindowClass_str122,
   xmCascadeButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, alignPd,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str123 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   alignPd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ALIGN_LEFT;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str124 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   alignPd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ALIGN_RIGHT;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str125 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   alignPd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ALIGN_TOP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str126 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   alignPd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ALIGN_BOTTOM;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );

  centerPd = XmCreatePulldownMenu( b2ManySelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str127 );

  centerCb = XtVaCreateManagedWidget( activeWindowClass_str127,
   xmCascadeButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, centerPd,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str128 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   centerPd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ALIGN_CENTER_VERT;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str129 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   centerPd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ALIGN_CENTER_HORZ;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str130 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   centerPd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ALIGN_CENTER;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  sizePd = XmCreatePulldownMenu( b2ManySelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str131 );

  sizeCb = XtVaCreateManagedWidget( activeWindowClass_str131,
   xmCascadeButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, sizePd,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str132 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   sizePd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ALIGN_SIZE_VERT;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str133 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   sizePd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ALIGN_SIZE_HORZ;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str134 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   sizePd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_ALIGN_SIZE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  distributePd = XmCreatePulldownMenu( b2ManySelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str135 );

  distributeCb = XtVaCreateManagedWidget( activeWindowClass_str135,
   xmCascadeButtonWidgetClass, b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, distributePd,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str136 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   distributePd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_DISTRIBUTE_VERTICALLY;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str137 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   distributePd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_DISTRIBUTE_HORIZONTALLY;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str138 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   distributePd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_DISTRIBUTE_MIDPT_VERTICALLY;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  str = XmStringCreateLocalized( activeWindowClass_str139 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   distributePd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr =
   (void *) AWC_POPUP_DISTRIBUTE_MIDPT_HORIZONTALLY;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  editPdM = XmCreatePulldownMenu( b2ManySelectPopup, "", NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str140 );

  editCbM = XtVaCreateManagedWidget( activeWindowClass_str140,
   xmCascadeButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, editPdM,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str141 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   editPdM,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_CHANGE_DSP_PARAMS;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );

  str = XmStringCreateLocalized( activeWindowClass_str142 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   editPdM,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_CHANGE_PV_NAMES;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str143 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_DESELECT;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str169 );

  undoPb3 = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsensitive, 0,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = undoPb3;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_UNDO;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( undoPb3, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str144 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_REFRESH;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str184 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_HELP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );

//===================================================================

  n = 0;
  XtSetArg( args[n], XmNmenuPost, (XtArgVal) "<Btn5Down>;" ); n++;
  b2ExecutePopup = XmCreatePopupMenu( top, "", args, n );

  if ( !_noEdit && closeAllowed ) {

    str = XmStringCreateLocalized( activeWindowClass_str145 );

    pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
     b2ExecutePopup,
     XmNlabelString, str,
     NULL );

    XmStringFree( str );

    curBlockListNode = new popupBlockListType;
    curBlockListNode->block.w = pb;
    curBlockListNode->block.ptr = (void *) AWC_POPUP_EDIT;
    curBlockListNode->block.awo = this;
  
    curBlockListNode->blink = popupBlockHead->blink;
    popupBlockHead->blink->flink = curBlockListNode;
    popupBlockHead->blink = curBlockListNode;
    curBlockListNode->flink = popupBlockHead;

    XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
     (XtPointer) &curBlockListNode->block );

  }


  str = XmStringCreateLocalized( activeWindowClass_str146 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ExecutePopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_OPEN;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
   (XtPointer) &curBlockListNode->block );


#if 0
  str = XmStringCreateLocalized( activeWindowClass_str147 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ExecutePopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_OPEN_USER;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
   (XtPointer) &curBlockListNode->block );
#endif


  if ( closeAllowed ) {

    str = XmStringCreateLocalized( activeWindowClass_str148 );

    pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
     b2ExecutePopup,
     XmNlabelString, str,
     NULL );

    XmStringFree( str );

    curBlockListNode = new popupBlockListType;
    curBlockListNode->block.w = pb;
    curBlockListNode->block.ptr = (void *) AWC_POPUP_CLOSE;
    curBlockListNode->block.awo = this;

    curBlockListNode->blink = popupBlockHead->blink;
    popupBlockHead->blink->flink = curBlockListNode;
    popupBlockHead->blink = curBlockListNode;
    curBlockListNode->flink = popupBlockHead;

    XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
     (XtPointer) &curBlockListNode->block );

  }
  else {

    str = XmStringCreateLocalized( activeWindowClass_str149 );

    pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
     b2ExecutePopup,
     XmNlabelString, str,
     NULL );

    XmStringFree( str );

    curBlockListNode = new popupBlockListType;
    curBlockListNode->block.w = pb;
    curBlockListNode->block.ptr = (void *) AWC_POPUP_LOWER;
    curBlockListNode->block.awo = this;

    curBlockListNode->blink = popupBlockHead->blink;
    popupBlockHead->blink->flink = curBlockListNode;
    popupBlockHead->blink = curBlockListNode;
    curBlockListNode->flink = popupBlockHead;

    XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
     (XtPointer) &curBlockListNode->block );

  }

  str = XmStringCreateLocalized( activeWindowClass_str150 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ExecutePopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_TOGGLE_TITLE;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str151 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ExecutePopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_FINDTOP;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str152 );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   b2ExecutePopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_REFRESH;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
   (XtPointer) &curBlockListNode->block );


//===================================================================

  return 1;

}

void activeWindowClass::realize ( void ) {

  XtRealizeWidget( top );
  XSetWindowColormap( d, XtWindow(top), appCtx->ci.getColorMap() );
  XtMapWidget( drawWidget );
  setTitle();

}

int activeWindowClass::setGraphicEnvironment (
  colorInfoClass *OneCi,
  fontInfoClass *OneFi ) {

  ci = OneCi;
  fi = OneFi;

  strncpy( defaultFontTag, fi->defaultFont(), 127 );
  strncpy( defaultCtlFontTag, fi->defaultFont(), 127 );
  strncpy( defaultBtnFontTag, fi->defaultFont(), 127 );

  drawGc.create( drawWidget );

  fgColor = ci->pixIndex( BlackPixel( d, DefaultScreen(d) ) );
  bgColor = ci->pixIndex( WhitePixel( d, DefaultScreen(d) ) );

  drawGc.setFG( ci->pix(fgColor) );
  drawGc.setBG( ci->pix(bgColor) );
  drawGc.setBaseBG( ci->pix(bgColor) );

  executeGc.create( executeWidget );
  executeGc.setBaseBG( drawGc.getBaseBG() );

  cursor.create( d, XtWindow(top), ci->getColorMap() );
  cursor.set( XtWindow(drawWidget), CURSOR_K_CROSSHAIR );
  cursor.setColor( ci->pix(fgColor), ci->pix(bgColor) );

  return 1;

}

Display *activeWindowClass::display ( void ) {

  return d;

}

Widget activeWindowClass::topWidgetId ( void ) {

  return top;

}

Widget activeWindowClass::drawWidgetId ( void ) {

  return drawWidget;

}

Widget activeWindowClass::executeWidgetId ( void ) {

  return executeWidget;

}

int activeWindowClass::changed ( void ) {

  return change;

}

void activeWindowClass::setChanged ( void ) {

//char str[31+1];
//int stat;

  change = 1;

  if ( !changeSinceAutoSave ) {
    changeSinceAutoSave = 1;
    //stat = sys_get_datetime_string( 31, str );
    if ( autosaveTimer ) {
      XtRemoveTimeOut( autosaveTimer );
      autosaveTimer = 0;
    }
    autosaveTimer = XtAppAddTimeOut( appCtx->appContext(),
     300000, acw_autosave, this );
     //30000, acw_autosave, this );
    //printf( "[%s] %s - add autosave timer\n", str, fileName );
  }

}

void activeWindowClass::setUnchanged ( void ) {

  change = 0;
  changeSinceAutoSave = 0;

}

int activeWindowClass::genericLoadScheme (
  char *fName,
  int includeDisplayProperties )
{

char *gotOne, oneFileName[255+1];
displaySchemeClass scheme;
int stat;

  gotOne = strstr( fName, "/" );

  if ( gotOne ) {
    strncpy( oneFileName, fName, 255 );
  }
  else {
    strncpy( oneFileName, appCtx->colorPath, 255 );
    strncat( oneFileName, fName, 255 );
  }

  if ( strlen(oneFileName) > strlen(".scheme") ) {
    if ( strcmp( &oneFileName[strlen(oneFileName)-strlen(".scheme")], ".scheme" ) != 0 ) {
      strncat( oneFileName, ".scheme", 255 );
    }
  }
  else {
    strncat( oneFileName, ".scheme", 255 );
  }

  stat = scheme.load( ci, oneFileName );
  if ( !( stat & 1 ) ) return stat;

  strncpy( defaultPvType, scheme.getPvType(), 15 );

  strncpy( defaultFontTag, scheme.getFont(), 127 );
  if ( strcmp( defaultFontTag, "" ) != 0 ) {
    stat = defaultFm.setFontTag( defaultFontTag );
  }

  defaultAlignment = scheme.getAlignment();
  if ( defaultAlignment != 0 ) {
    stat = defaultFm.setFontAlignment( defaultAlignment );
  }

  strncpy( defaultCtlFontTag, scheme.getCtlFont(), 127 );
  if ( strcmp( defaultCtlFontTag, "" ) != 0 ) {
    stat = defaultCtlFm.setFontTag( defaultCtlFontTag );
  }

  defaultCtlAlignment = scheme.getCtlAlignment();
  if ( defaultCtlAlignment != 0 ) {
    stat = defaultCtlFm.setFontAlignment( defaultCtlAlignment );
  }

  strncpy( defaultBtnFontTag, scheme.getBtnFont(), 127 );
  if ( strcmp( defaultBtnFontTag, "" ) != 0 ) {
    stat = defaultBtnFm.setFontTag( defaultBtnFontTag );
  }

  defaultBtnAlignment = scheme.getBtnAlignment();
  if ( defaultBtnAlignment != 0 ) {
    stat = defaultBtnFm.setFontAlignment( defaultBtnAlignment );
  }

  if ( includeDisplayProperties ) {
    fgColor = scheme.getFg();
    bgColor = scheme.getBg();
  }

  defaultTextFgColor = scheme.getDefTextFg();

  defaultFg1Color = scheme.getDefFg1();

  defaultFg2Color = scheme.getDefFg2();

  defaultBgColor = scheme.getDefBg();

  defaultTopShadowColor = scheme.getTopShadow();

  defaultBotShadowColor = scheme.getBotShadow();

  defaultOffsetColor = scheme.getOffset();

  if ( includeDisplayProperties ) {
    drawGc.setFG( ci->pix(fgColor) );
    drawGc.setBG( ci->pix(bgColor) );
    drawGc.setBaseBG( ci->pix(bgColor) );
    executeGc.setBaseBG( ci->pix(bgColor) );
    cursor.setColor( ci->pix(fgColor), ci->pix(bgColor) );
  }

  setChanged();

  updateAllSelectedDisplayInfo();

  return 1;

}

int activeWindowClass::loadScheme (
  char *fName )
{

  return genericLoadScheme( fName, 1 );

}

int activeWindowClass::loadComponentScheme (
  char *fName )
{

  return genericLoadScheme( fName, 0 );

}

int activeWindowClass::saveScheme (
  char *fName ) {

char *gotOne, oneFileName[255+1];
displaySchemeClass scheme;
int stat;

  gotOne = strstr( fName, "/" );

  if ( gotOne ) {
    strncpy( oneFileName, fName, 255 );
  }
  else {
    strncpy( oneFileName, appCtx->colorPath, 255 );
    strncat( oneFileName, fName, 255 );
  }

  if ( strlen(oneFileName) > strlen(".scheme") ) {
    if ( strcmp( &oneFileName[strlen(oneFileName)-strlen(".scheme")], ".scheme" ) != 0 ) {
      strncat( oneFileName, ".scheme", 255 );
    }
  }
  else {
    strncat( oneFileName, ".scheme", 255 );
  }

  scheme.setPvType( defaultPvType );
  scheme.setFg( fgColor );
  scheme.setBg( bgColor );
  scheme.setDefTextFg( defaultTextFgColor );
  scheme.setDefFg1( defaultFg1Color );
  scheme.setDefFg2( defaultFg2Color );
  scheme.setDefBg( defaultBgColor );
  scheme.setOffset( defaultOffsetColor );
  scheme.setTopShadow( defaultTopShadowColor );
  scheme.setBotShadow( defaultBotShadowColor );
  scheme.setFont( defaultFontTag );
  scheme.setCtlFont( defaultCtlFontTag );
  scheme.setBtnFont( defaultBtnFontTag );
  scheme.setAlignment( defaultAlignment );
  scheme.setCtlAlignment( defaultCtlAlignment );
  scheme.setBtnAlignment( defaultBtnAlignment );

  stat = scheme.save( ci, oneFileName );

  return stat;

}

int activeWindowClass::renameToBackupFile (
  char *fname )
{

int stat;
char tmp[511+1];

  strncpy( tmp, fname, 510 ); // leave room for ~
  strncat( tmp, "~", 511 );

  if ( fileExists( tmp ) ) {
    stat = unlink( tmp );
    if ( stat ) return 2; // error
  }
  if ( fileExists( fname ) ) {
    stat = rename( fname, tmp );
    if ( stat ) return 4; // error
  }

  return 1; // success

}

int activeWindowClass::save (
  char *fName ) {

int stat;

  stat = genericSave( fName, 1, 1, 1 );

  return stat;

}

int activeWindowClass::saveNoChange (
  char *fName ){

int stat;

  stat = genericSave( fName, 0, 0, 0 );

  return stat;

}

int activeWindowClass::genericSave (
  char *fName,
  int resetChangeFlag,
  int appendExtensionFlag,
  int backupFlag ) {

FILE *f;
char *gotOne, oneFileName[255+1];
activeGraphicListPtr cur;
char msg[79+1];
int stat;

  if ( resetChangeFlag ) { // this is a save initiated by the user
    if ( restoreTimer ) { // title hold the string "Auto Save"
      XtRemoveTimeOut( restoreTimer );
      restoreTimer = 0;
      if ( strcmp( title, activeWindowClass_str153 ) == 0 ) {
        strcpy( title, restoreTitle );
        setTitle();
        XFlush( d );
      }
    }
  }

  gotOne = strstr( fName, "/" );

  if ( gotOne ) {
    strncpy( oneFileName, fName, 255 );
  }
  else {
    strncpy( oneFileName, appCtx->curPath, 255 );
    strncat( oneFileName, fName, 255 );
  }

  if ( appendExtensionFlag ) {

    if ( strlen(oneFileName) > strlen(".edl") ) {
      if (
       strcmp( &oneFileName[strlen(oneFileName)-strlen(".edl")], ".edl" ) !=
       0 ) {
        strncat( oneFileName, ".edl", 255 );
      }
    }
    else {
      strncat( oneFileName, ".edl", 255 );
    }

  }

  if ( backupFlag ) {

    stat = renameToBackupFile( oneFileName );
    if ( !( stat & 1 ) ) {
      sprintf( msg, activeWindowClass_str154, oneFileName );
      appCtx->postMessage( msg );
    }

  }

  f = fopen( oneFileName, "w" );
  if ( !f ) {
    sprintf( msg, activeWindowClass_str155, oneFileName );
    appCtx->postMessage( msg );
    return 0;
  }

  this->saveWin( f );
  fprintf( f, "<<<E~O~D>>>\n" );

  cur = head->flink;
  while ( cur != head ) {
    if ( !cur->node->deleteRequest ) {
      fprintf( f, "%s\n", cur->node->objName() );
      cur->node->save( f );
      fprintf( f, "<<<E~O~D>>>\n" );
    }
    cur = cur->flink;
  }

  fclose( f );

  if ( resetChangeFlag ) this->setUnchanged();

  return 1;

}

int activeWindowClass::loadCascade ( void ) {

FILE *f;
activeWindowListPtr curWin;
activeGraphicListPtr cur, next;
char *gotOne, name[63+1];
int stat, l, goodXY, n, maxX, maxY;
char msg[79+1];
Arg args[5];

  initLine();

  // empty main list
  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  head->flink = head;
  head->blink = head;

  // empty cut list
  cur = cutHead->flink;
  while ( cur != cutHead ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  cutHead->flink = cutHead;
  cutHead->blink = cutHead;

  // set select list empty

  selectedHead->selFlink = selectedHead;
  selectedHead->selBlink = selectedHead;

  // read in file
  f = this->openAny( this->fileName, "r" );
  if ( !f ) {
    sprintf( msg, activeWindowClass_str156, this->fileName );
    appCtx->postMessage( msg );
    return 0;
  }

  this->setUnchanged();

  this->loadWin( f );

  stat = readUntilEndOfData( f ); // for forward compatibility
  if ( !( stat & 1 ) ) return stat; // memory leak here

  maxX = XDisplayWidth( d, DefaultScreen(d) ) - 20;
  maxY = XDisplayHeight( d, DefaultScreen(d) ) - 20;

  do {

    goodXY = 1;

    curWin = appCtx->head->flink;
    while ( curWin != appCtx->head ) {
      if ( this != &curWin->node ) {
        if ( strcmp( this->name, curWin->node.name ) == 0 ) {
          if ( ( x == curWin->node.x ) && ( y == curWin->node.y ) ) {
            x += 20;
            if ( x > maxX ) x = 20;
            y += 20;
            if ( y > maxY ) y = 20;
            goodXY = 0;
	  }
        }
      }
      curWin = curWin->flink;
    }

  } while ( !goodXY );

  n = 0;
  XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
  XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
  XtSetValues( this->drawWidget, args, n );
  
  while ( !feof(f) ) {

    gotOne = fgets( name, 63, f ); incLine();

    if ( gotOne ) {

      l = strlen(name);
      if ( l > 63 ) l = 63;
      name[l-1] = 0;  // discard \n

      cur = new activeGraphicListType;
      if ( !cur ) {
        fclose( f );
        appCtx->postMessage(
         activeWindowClass_str157 );
        return 0;
      }
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;

      cur->node = obj.createNew( name );

      if ( cur->node ) {

        stat = cur->node->createFromFile( f, name, this );
        if ( !( stat & 1 ) ) return stat; // memory leak here

        stat = readUntilEndOfData( f ); // for forward compatibility
        if ( !( stat & 1 ) ) return stat; // memory leak here

        cur->blink = head->blink;
        head->blink->flink = cur;
        head->blink = cur;
        cur->flink = head;

      }
      else {
        fclose( f );
        sprintf( msg, activeWindowClass_str158, line(),
         name );
        appCtx->postMessage( msg );
        return 0;
      }

    }

  }

  fclose( f );

  showName = 0;

  setTitle();

  exit_after_save = 0;

  return 1;

}

int activeWindowClass::loadCascade (
  int x,
  int y ) {

FILE *f;
activeWindowListPtr curWin;
activeGraphicListPtr cur, next;
char *gotOne, name[63+1];
int stat, l, goodXY, n, maxX, maxY;
char msg[79+1];
Arg args[5];

  initLine();

  // empty main list
  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  head->flink = head;
  head->blink = head;

  // empty cut list
  cur = cutHead->flink;
  while ( cur != cutHead ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  cutHead->flink = cutHead;
  cutHead->blink = cutHead;

  // set select list empty

  selectedHead->selFlink = selectedHead;
  selectedHead->selBlink = selectedHead;

  // read in file
  f = openAny( this->fileName, "r" );
  if ( !f ) {
    sprintf( msg, activeWindowClass_str156, this->fileName );
    appCtx->postMessage( msg );
    return 0;
  }

  this->setUnchanged();

  this->loadWin( f, x, y );

  stat = readUntilEndOfData( f ); // for forward compatibility
  if ( !( stat & 1 ) ) return stat; // memory leak here

  maxX = XDisplayWidth( d, DefaultScreen(d) ) - 20;
  maxY = XDisplayHeight( d, DefaultScreen(d) ) - 20;

  do {

    goodXY = 1;

    curWin = appCtx->head->flink;
    while ( curWin != appCtx->head ) {
      if ( this != &curWin->node ) {
        if ( strcmp( this->name, curWin->node.name ) == 0 ) {
          if ( ( x == curWin->node.x ) && ( y == curWin->node.y ) ) {
            x += 20;
            if ( x > maxX ) x = 20;
            y += 20;
            if ( y > maxY ) y = 20;
            goodXY = 0;
	  }
        }
      }
      curWin = curWin->flink;
    }

  } while ( !goodXY );

  n = 0;
  XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
  XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
  XtSetValues( this->drawWidget, args, n );

  while ( !feof(f) ) {

    gotOne = fgets( name, 63, f ); incLine();

    if ( gotOne ) {

      l = strlen(name);
      if ( l > 63 ) l = 63;
      name[l-1] = 0;  // discard \n

      cur = new activeGraphicListType;
      if ( !cur ) {
        fclose( f );
        appCtx->postMessage(
         activeWindowClass_str157 );
        return 0;
      }
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;

      cur->node = obj.createNew( name );

      if ( cur->node ) {

        stat = cur->node->createFromFile( f, name, this );
        if ( !( stat & 1 ) ) return stat; // memory leak here

        stat = readUntilEndOfData( f ); // for forward compatibility
        if ( !( stat & 1 ) ) return stat; // memory leak here

        cur->blink = head->blink;
        head->blink->flink = cur;
        head->blink = cur;
        cur->flink = head;

      }
      else {
        fclose( f );
        sprintf( msg, activeWindowClass_str158, line(),
         name );
        appCtx->postMessage( msg );
        return 0;
      }

    }

  }

  fclose( f );

  showName = 0;

  setTitle();

  exit_after_save = 0;

  return 1;

}

int activeWindowClass::load ( void ) {

FILE *f;
activeGraphicListPtr cur, next;
char *gotOne, name[63+1];
int stat, l;
char msg[79+1];

  initLine();

  // empty main list
  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  head->flink = head;
  head->blink = head;

  // empty cut list
  cur = cutHead->flink;
  while ( cur != cutHead ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  cutHead->flink = cutHead;
  cutHead->blink = cutHead;

  // set select list empty

  selectedHead->selFlink = selectedHead;
  selectedHead->selBlink = selectedHead;

  // read in file
  f = this->openAny( this->fileName, "r" );
  if ( !f ) {
    sprintf( msg, activeWindowClass_str156, this->fileName );
    appCtx->postMessage( msg );
    return 0;
  }

  this->setUnchanged();

  this->loadWin( f );

  stat = readUntilEndOfData( f ); // for forward compatibility
  if ( !( stat & 1 ) ) return stat; // memory leak here

  while ( !feof(f) ) {

    gotOne = fgets( name, 63, f ); incLine();

    if ( gotOne ) {

      l = strlen(name);
      if ( l > 63 ) l = 63;
      name[l-1] = 0;  // discard \n

      cur = new activeGraphicListType;
      if ( !cur ) {
        fclose( f );
        appCtx->postMessage( activeWindowClass_str157 );
        return 0;
      }
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;

      cur->node = obj.createNew( name );

      if ( cur->node ) {

        stat = cur->node->createFromFile( f, name, this );
        if ( !( stat & 1 ) ) return stat; // memory leak here

        stat = readUntilEndOfData( f ); // for forward compatibility
        if ( !( stat & 1 ) ) return stat; // memory leak here

        cur->blink = head->blink;
        head->blink->flink = cur;
        head->blink = cur;
        cur->flink = head;

      }
      else {
        fclose( f );
        sprintf( msg, activeWindowClass_str158, line(),
         name );
        appCtx->postMessage( msg );
        return 0;
      }

    }

  }

  fclose( f );

  showName = 0;

  setTitle();

  exit_after_save = 0;

  return 1;

}

int activeWindowClass::load (
  int x,
  int y ) {

FILE *f;
activeGraphicListPtr cur, next;
char *gotOne, name[63+1];
int stat, l;
char msg[79+1];

  initLine();

  // empty main list
  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  head->flink = head;
  head->blink = head;

  // empty cut list
  cur = cutHead->flink;
  while ( cur != cutHead ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  cutHead->flink = cutHead;
  cutHead->blink = cutHead;

  // set select list empty

  selectedHead->selFlink = selectedHead;
  selectedHead->selBlink = selectedHead;

  // read in file
  f = openAny( this->fileName, "r" );
  if ( !f ) {
    sprintf( msg, activeWindowClass_str156, this->fileName );
    appCtx->postMessage( msg );
    return 0;
  }

  this->setUnchanged();

  this->loadWin( f, x, y );

  stat = readUntilEndOfData( f ); // for forward compatibility
  if ( !( stat & 1 ) ) return stat; // memory leak here

  while ( !feof(f) ) {

    gotOne = fgets( name, 63, f ); incLine();

    if ( gotOne ) {

      l = strlen(name);
      if ( l > 63 ) l = 63;
      name[l-1] = 0;  // discard \n

      cur = new activeGraphicListType;
      if ( !cur ) {
        fclose( f );
        appCtx->postMessage( activeWindowClass_str157 );
        return 0;
      }
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;

      cur->node = obj.createNew( name );

      if ( cur->node ) {

        stat = cur->node->createFromFile( f, name, this );
        if ( !( stat & 1 ) ) return stat; // memory leak here

        stat = readUntilEndOfData( f ); // for forward compatibility
        if ( !( stat & 1 ) ) return stat; // memory leak here

        cur->blink = head->blink;
        head->blink->flink = cur;
        head->blink = cur;
        cur->flink = head;

      }
      else {
        fclose( f );
        sprintf( msg, activeWindowClass_str158, line(),
         name );
        appCtx->postMessage( msg );
        return 0;
      }

    }

  }

  fclose( f );

  showName = 0;

  setTitle();

  exit_after_save = 0;

  return 1;

}

int activeWindowClass::import ( void ) {

FILE *f;
activeGraphicListPtr cur, next;
char *gotOne, name[63+1];
int stat, l;
char msg[79+1];

  // empty main list
  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  head->flink = head;
  head->blink = head;

  // empty cut list
  cur = cutHead->flink;
  while ( cur != cutHead ) {
    next = cur->flink;
    delete cur->node;
    delete cur;
    cur = next;
  }

  cutHead->flink = cutHead;
  cutHead->blink = cutHead;

  // set select list empty

  selectedHead->selFlink = selectedHead;
  selectedHead->selBlink = selectedHead;

  this->setDisplayScheme( &appCtx->displayScheme );

  // read in file
  f = this->openExchangeFile( this->fileName, "r" );
  if ( !f ) {
    sprintf( msg, activeWindowClass_str156, this->fileName );
    appCtx->postMessage( msg );
    return 0;
  }

  this->setChanged();

  this->importWin( f );

  while ( !feof(f) ) {

    gotOne = getNextDataString( name, 63, f );

    if ( gotOne ) {

      l = strlen(name);
      if ( l > 63 ) l = 63;
      name[l-1] = 0;  // discard \n

      cur = new activeGraphicListType;
      if ( !cur ) {
        fclose( f );
        appCtx->postMessage( activeWindowClass_str159 );
        return 0;
      }
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;

      cur->node = obj.createNew( name );

      if ( cur->node ) {

        stat = cur->node->importFromXchFile( f, name, this );
        if ( !( stat & 1 ) ) return stat; // memory leak here

        cur->blink = head->blink;
        head->blink->flink = cur;
        head->blink = cur;
        cur->flink = head;

      }
      else {
        fclose( f );
        sprintf( msg, activeWindowClass_str160, name );
        appCtx->postMessage( msg );
        return 0;
      }

    }

  }

  fclose( f );

  // change file extension to .edl
  l = strlen(this->fileName);
  if ( l > 4 ) {
    if ( strcmp( &this->fileName[l-4], ".xch" ) == 0 ) {
      strcpy( &this->fileName[l-4], ".edl" );
    }
  }

  showName = 0;

  setTitle();

  exit_after_save = 0;

  return 1;

}

int activeWindowClass::refreshGrid ( void ) {

  if ( gridShow ) displayGrid();

  return 1;

}

int activeWindowClass::clear ( void ) {

  setTitle();

  XClearWindow( d, XtWindow(drawWidget) );

  if ( gridShow ) displayGrid();

  return 1;

}

int activeWindowClass::refresh (
  int _x,
  int _y,
  int _w,
  int _h )
{

XRectangle xR = { _x, _y, _w, _h };
activeGraphicListPtr cur, next;
int needDelete = 0;

  if ( noRefresh ) return 1;

  setTitle();

  if ( gridShow ) {
    drawGc.addNormXClipRectangle( xR );
    displayGrid( _x, _y, _w, _h );
    drawGc.removeNormXClipRectangle();
  }

  cur = head->flink;
  if ( cur != head ) {
    needDelete = cur->node->refresh( _x, _y, _w, _h );
  }

  if ( needDelete ) {
    cur = head->flink;
    while ( cur != head ) {
      next = cur->flink;
      if ( cur->node->deleteRequest ) {
        cur->blink->flink = cur->flink;
        cur->flink->blink = cur->blink;
        delete cur->node;
        delete cur;
      }
      cur = next;
    }
  }

  return 1;

}

int activeWindowClass::refresh ( void ) {

activeGraphicListPtr cur, next;
int needDelete = 0;

  if ( noRefresh ) return 1;

  setTitle();

  if ( gridShow ) {
    displayGrid();
  }

  cur = head->flink;
  if ( cur != head ) { // if list is not empty
    needDelete = cur->node->refresh();
  }

  if ( needDelete ) {
    cur = head->flink;
    while ( cur != head ) {
      next = cur->flink;
      if ( cur->node->deleteRequest ) {
        cur->blink->flink = cur->flink;
        cur->flink->blink = cur->blink;
        delete cur->node;
        delete cur;
      }
      cur = next;
    }
  }

  return 1;

}

int activeWindowClass::createWidgets ( void ) {

int stat;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    stat = cur->node->createWidgets();

    cur = cur->flink;

  }

  return 1;

}

int activeWindowClass::executeMux ( void ) {

int pass, opStat, stat, nTries, btnUp, btnDown, btnDrag, btnFocus;
activeGraphicListPtr cur;
btnActionListPtr curBtn;

  // each pass must complete successfully in approx 10 seconds

  for ( pass=1; pass<7; pass++ ) {

    nTries = 200;
    do {

      opStat = 1;

      cur = head->flink;
      while ( cur != head ) {

        if ( cur->node->isMux() ) {
          stat = cur->node->activate( pass, (void *) cur );
          if ( !( stat & 1 ) ) opStat = stat;
        }

        cur = cur->flink;

      }

      if ( !( opStat & 1 ) ) {

        processAllEvents( appCtx->appContext(), d );

      }

      nTries--;

    } while ( nTries && !( opStat & 1 ) );

    processAllEvents( appCtx->appContext(), d );

  }

  // get list of button action requests
  cur = head->flink;
  while ( cur != head ) {

    if ( cur->node->isMux() ) {

      btnUp = btnDown = 0;
      stat = cur->node->getButtonActionRequest( &btnUp, &btnDown, &btnDrag,
       &btnFocus );
      if ( btnUp ) {
        curBtn = new btnActionListType;
        curBtn->node = cur->node;
        curBtn->blink = btnUpActionHead->blink;
        btnUpActionHead->blink->flink = curBtn;
        btnUpActionHead->blink = curBtn;
        curBtn->flink = btnUpActionHead;
      }
      if ( btnDown ) {
        curBtn = new btnActionListType;
        curBtn->node = cur->node;
        if ( btnUp )
          curBtn->pressed = 0;
        else
          curBtn->pressed = -1;
        curBtn->blink = btnDownActionHead->blink;
        btnDownActionHead->blink->flink = curBtn;
        btnDownActionHead->blink = curBtn;
        curBtn->flink = btnDownActionHead;
      }
      if ( btnDrag ) {
        curBtn = new btnActionListType;
        curBtn->node = cur->node;
        curBtn->blink = btnMotionActionHead->blink;
        btnMotionActionHead->blink->flink = curBtn;
        btnMotionActionHead->blink = curBtn;
        curBtn->flink = btnMotionActionHead;
      }
      if ( btnFocus ) {
        curBtn = new btnActionListType;
        curBtn->node = cur->node;
        curBtn->in = -1; // unknown
        curBtn->blink = btnFocusActionHead->blink;
        btnFocusActionHead->blink->flink = curBtn;
        btnFocusActionHead->blink = curBtn;
        curBtn->flink = btnFocusActionHead;
      }

    }

    cur = cur->flink;

  }

  return 1;

}

int activeWindowClass::initDefExeNode (
  void *ptr )
{

activeGraphicListPtr node = (activeGraphicListPtr) ptr;

  node->defExeFlink = node->defExeBlink = NULL;

  return 1;

}

int activeWindowClass::addDefExeNode (
  void *ptr )
{

activeGraphicListPtr node = (activeGraphicListPtr) ptr;

  if ( node->defExeFlink ) return 2; // already in list

  node->defExeBlink = defExeHead->defExeBlink;
  defExeHead->defExeBlink->defExeFlink = node;
  defExeHead->defExeBlink = node;
  node->defExeFlink = defExeHead;

  return 1;

}

int activeWindowClass::remDefExeNode (
  void *ptr )
{

activeGraphicListPtr node = (activeGraphicListPtr) ptr;

  if ( !node->defExeFlink ) return 4; // not in list

  // unlink
  node->defExeBlink->defExeFlink = node->defExeFlink;
  node->defExeFlink->defExeBlink = node->defExeBlink;
  node->defExeFlink = node->defExeBlink = NULL;

  return 1;

}

int activeWindowClass::execute ( void ) {

int pass, opStat, stat, nTries, btnUp, btnDown, btnDrag, btnFocus;
activeGraphicListPtr cur, cur1;
btnActionListPtr curBtn;
int numMuxMacros;
char **muxMacro, **muxExpansion;
char callbackName[63+1];

  if ( activateCallbackFlag ) {
    strncpy( callbackName, id, 63 );
    strncat( callbackName, "Activate", 63 );
    activateCallback = appCtx->userLibObject.getFunc( callbackName );
    if ( activateCallback ) {
      (*activateCallback)( this );
    }
  }

  this->clear();

  isIconified = False;

  showActive = 0;

  appCtx->proc->lock();

  defExeHead->defExeFlink = defExeHead;
  defExeHead->defExeBlink = defExeHead;

  cur = head->flink;
  while ( cur != head ) {
    cur->node->initDefExeNode( cur );
    cur = cur->flink;
  }

  appCtx->proc->unlock();

  cursor.set( XtWindow(executeWidget), CURSOR_K_DEFAULT );
  cursor.setColor( ci->pix(fgColor), ci->pix(bgColor) );

  XtRemoveEventHandler( drawWidget,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask|Button1MotionMask|
    Button2MotionMask|Button3MotionMask|ExposureMask, False,
   drawWinEventHandler, (XtPointer) this );

  executeGc.setBaseBG( drawGc.getBaseBG() );

  expandTitle( 1, numMacros, macros, expansions );

  cur = head->flink;
  while ( cur != head ) {

    stat = cur->node->expand1st( numMacros, macros, expansions );

    cur = cur->flink;

  }

  mode = AWC_EXECUTE;
  waiting = 0; // for deferred screen close action

  // activate mux controls
  executeMux();

  // now activate all non-mux controls

  cur = head->flink;
  while ( cur != head ) {

    if ( cur->node->isMux() ) {

      stat = cur->node->getMacros( &numMuxMacros, &muxMacro, &muxExpansion );

      if ( numMuxMacros > 0 ) {

        expandTitle( 2, numMacros, macros, expansions );

        cur1 = head->flink;
        while ( cur1 != head ) {

          stat = cur1->node->expand2nd( numMuxMacros, muxMacro, muxExpansion );

          cur1 = cur1->flink;

        }

      }

    }

    cur = cur->flink;

  }

  // each pass must complete successfully in approx 10 seconds

  for ( pass=1; pass<7; pass++ ) {

    nTries = 200;
    do {

      opStat = 1;

      cur = head->flink;
      while ( cur != head ) {

        if ( !cur->node->isMux() ) {
          stat = cur->node->activate( pass, (void *) cur );
          if ( !( stat & 1 ) ) opStat = stat;
        }

        cur = cur->flink;

      }

      if ( !( opStat & 1 ) ) {

        processAllEvents( appCtx->appContext(), d );

      }

      nTries--;

    } while ( nTries && !( opStat & 1 ) );

    processAllEvents( appCtx->appContext(), d );

  }

  // get list of button action requests
  cur = head->flink;
  while ( cur != head ) {

    if ( !cur->node->isMux() ) {

      btnUp = btnDown = 0;
      stat = cur->node->getButtonActionRequest( &btnUp, &btnDown, &btnDrag,
       &btnFocus );
      if ( btnUp ) {
        curBtn = new btnActionListType;
        curBtn->node = cur->node;
        curBtn->blink = btnUpActionHead->blink;
        btnUpActionHead->blink->flink = curBtn;
        btnUpActionHead->blink = curBtn;
        curBtn->flink = btnUpActionHead;
      }
      if ( btnDown ) {
        curBtn = new btnActionListType;
        curBtn->node = cur->node;
        if ( btnUp )
          curBtn->pressed = 0;
        else
          curBtn->pressed = -1;
        curBtn->blink = btnDownActionHead->blink;
        btnDownActionHead->blink->flink = curBtn;
        btnDownActionHead->blink = curBtn;
        curBtn->flink = btnDownActionHead;
      }
      if ( btnDrag ) {
        curBtn = new btnActionListType;
        curBtn->node = cur->node;
        curBtn->blink = btnMotionActionHead->blink;
        btnMotionActionHead->blink->flink = curBtn;
        btnMotionActionHead->blink = curBtn;
        curBtn->flink = btnMotionActionHead;
      }
      if ( btnFocus ) {
        curBtn = new btnActionListType;
        curBtn->node = cur->node;
        curBtn->in = -1; // unknown
        curBtn->blink = btnFocusActionHead->blink;
        btnFocusActionHead->blink->flink = curBtn;
        btnFocusActionHead->blink = curBtn;
        curBtn->flink = btnFocusActionHead;
      }

    }

    cur = cur->flink;

  }

  XRaiseWindow( d, XtWindow(top) );

  setTitle();

  if ( gridShow ) {
    clearActive();
  }

  processAllEvents( appCtx->appContext(), d );

  XtAddEventHandler( executeWidget,
   ButtonPressMask|ButtonReleaseMask|PointerMotionMask|
   ExposureMask, False, activeWinEventHandler, (XtPointer) this );

  refreshActive();

  return 1;

}

int activeWindowClass::reexecute ( void ) { // for multiplexor

int pass, opStat, stat, nTries;
activeGraphicListPtr cur, cur1;

int numMuxMacros;
char **muxMacro, **muxExpansion;

  if ( mode == AWC_EXECUTE ) return 1;

  isIconified = False;

  expandTitle( 1, numMacros, macros, expansions );

  cur = head->flink;
  while ( cur != head ) {

    if ( !cur->node->isMux() && cur->node->containsMacros() ) {
      stat = cur->node->expand1st( numMacros, macros, expansions );
    }

    cur = cur->flink;

  }

  mode = AWC_EXECUTE;
  waiting = 0; // for deferred screen close action

  // now reactivate all non-mux controls

  cur = head->flink;
  while ( cur != head ) {

    if ( cur->node->isMux() ) {

      stat = cur->node->getMacros( &numMuxMacros, &muxMacro, &muxExpansion );

      expandTitle( 2, numMuxMacros, muxMacro, muxExpansion );

      if ( numMuxMacros > 0 ) {

        cur1 = head->flink;
        while ( cur1 != head ) {

          if (!cur1->node->isMux() && cur1->node->containsMacros()) {
            stat = cur1->node->expand2nd(numMuxMacros, muxMacro, muxExpansion);
          }

          cur1 = cur1->flink;

        }

      }

    }

    cur = cur->flink;

  }

  // each pass must complete successfully in approx 10 seconds

  for ( pass=1; pass<7; pass++ ) {

    nTries = 200;
    do {

      opStat = 1;

      cur = head->flink;
      while ( cur != head ) {

        if ( !cur->node->isMux() && cur->node->containsMacros() ) {
          stat = cur->node->activate( pass, (void *) cur );
          if ( !( stat & 1 ) ) opStat = stat;
        }

        cur = cur->flink;

      }

      if ( !( opStat & 1 ) ) {

        processAllEvents( appCtx->appContext(), d );

      }

      nTries--;

    } while ( nTries && !( opStat & 1 ) );

    processAllEvents( appCtx->appContext(), d );

  }

  setTitle();

  refreshActive();

  return 1;

}

int activeWindowClass::returnToEdit (
  int closeFlag ) {

activeGraphicListPtr cur;
btnActionListPtr curBtn, nextBtn;

Window root, child;
int rootX, rootY, winX, winY, pass;
unsigned int mask;
char callbackName[63+1];

  mode = AWC_EDIT;

  cursor.set( XtWindow(drawWidget), CURSOR_K_CROSSHAIR );
  cursor.setColor( ci->pix(fgColor), ci->pix(bgColor) );

  XtRemoveEventHandler( executeWidget,
   ButtonPressMask|ButtonReleaseMask|Button1MotionMask|
   Button2MotionMask|Button3MotionMask|PointerMotionHintMask|ExposureMask,
   False, activeWinEventHandler, (XtPointer) this );

  curBtn = btnDownActionHead->flink;
  while ( curBtn != btnDownActionHead ) {
    nextBtn = curBtn->flink;
    delete curBtn;
    curBtn = nextBtn;
  }

  btnDownActionHead->flink = btnDownActionHead;
  btnDownActionHead->blink = btnDownActionHead;

  curBtn = btnUpActionHead->flink;
  while ( curBtn != btnUpActionHead ) {
    nextBtn = curBtn->flink;
    delete curBtn;
    curBtn = nextBtn;
  }

  btnUpActionHead->flink = btnUpActionHead;
  btnUpActionHead->blink = btnUpActionHead;

  curBtn = btnMotionActionHead->flink;
  while ( curBtn != btnMotionActionHead ) {
    nextBtn = curBtn->flink;
    delete curBtn;
    curBtn = nextBtn;
  }

  btnMotionActionHead->flink = btnMotionActionHead;
  btnMotionActionHead->blink = btnMotionActionHead;

  curBtn = btnFocusActionHead->flink;
  while ( curBtn != btnFocusActionHead ) {
    nextBtn = curBtn->flink;
    delete curBtn;
    curBtn = nextBtn;
  }

  btnFocusActionHead->flink = btnFocusActionHead;
  btnFocusActionHead->blink = btnFocusActionHead;

  cur = head->flink;
  while ( cur != head ) {

    for ( pass=1; pass<=2; pass++ ) {
      cur->node->deactivate( pass );
    }
    cur->node->deselect();

    cur = cur->flink;

  }

  // make selected list empty
  selectedHead->selFlink = selectedHead;
  selectedHead->selBlink = selectedHead;

  state = AWC_NONE_SELECTED;
  updateMasterSelection();

  if ( closeFlag ) {

    appCtx->proc->lock(); // uncommented
    defExeHead->defExeFlink = defExeHead;
    defExeHead->defExeBlink = defExeHead;
    appCtx->proc->unlock(); // uncommented

    if ( change ) {

      savedState = state;
      state = AWC_WAITING;

      XQueryPointer( d, XtWindow(top), &root, &child,
       &rootX, &rootY, &winX, &winY, &mask );

      confirm.create( top, rootX, rootY, 2,
       activeWindowClass_str161, NULL, NULL );
      confirm.addButton( activeWindowClass_str162, awc_abort_cb,
       (void *) this );
      confirm.addButton( activeWindowClass_str163, awc_continue_cb,
       (void *) this );
      confirm.finished();
      confirm.popup();
      XSetWindowColormap( d, XtWindow(confirm.top()),
       appCtx->ci.getColorMap() );

    }
    else {

      if ( autosaveTimer ) {
        XtRemoveTimeOut( autosaveTimer );
        autosaveTimer = 0;
      }
      if ( restoreTimer ) {
        XtRemoveTimeOut( restoreTimer );
        restoreTimer = 0;
      }

      //mark active window for delege
      appCtx->removeActiveWindow( this );

      XtUnmanageChild( executeWidget );

      if ( deactivateCallbackFlag ) {
        strncpy( callbackName, id, 63 );
        strncat( callbackName, "Deactivate", 63 );
        deactivateCallback = appCtx->userLibObject.getFunc(
         callbackName );
        if ( deactivateCallback ) {
          (*deactivateCallback)( this );
        }
      }

      return 1;

    }

  }
  else {

    processAllEvents( appCtx->appContext(), d );

    this->appCtx->deiconifyMainWindow();

  }

  XtAddEventHandler( drawWidget,
   KeyPressMask|ButtonPressMask|ButtonReleaseMask|Button1MotionMask|
    Button2MotionMask|Button3MotionMask|ExposureMask, False,
   drawWinEventHandler, (XtPointer) this );

  this->clear();
  this->refresh();

  if ( deactivateCallbackFlag ) {
    strncpy( callbackName, id, 63 );
    strncat( callbackName, "Deactivate", 63 );
    deactivateCallback = appCtx->userLibObject.getFunc(
     callbackName );
    if ( deactivateCallback ) {
      (*deactivateCallback)( this );
    }
  }

  return 1;

}

int activeWindowClass::preReexecute ( void )
{

activeGraphicListPtr cur;

  if ( mode == AWC_EDIT ) return 1;

  mode = AWC_EDIT;

  cur = head->flink;
  while ( cur != head ) {

    if ( !cur->node->isMux() && cur->node->containsMacros() ) {
      cur->node->bufInvalidate();
      cur->node->eraseActive();
      cur->node->preReactivate(1);
      cur->node->preReactivate(2);
    }

    cur = cur->flink;

  }

  return 1;

}

int activeWindowClass::clearActive ( void ) {

  XClearWindow( d, XtWindow(executeWidget) );

  return 1;

}

int activeWindowClass::requestActiveRefresh ( void ) {

  appCtx->refreshActiveWindow( this );

  return 1;

}

int activeWindowClass::refreshActive (
  int _x,
  int _y,
  int _w,
  int _h )
{

activeGraphicListPtr cur;

  if ( noRefresh ) return 1;

  cur = head->flink;
  if ( cur != head ) {
    cur->node->refreshActive( _x, _y, _w, _h );
  }

  return 1;

}

int activeWindowClass::refreshActive ( void ) {

activeGraphicListPtr cur;

  if ( noRefresh ) return 1;

  cur = head->flink;
  if ( cur != head ) {
    cur->node->refreshActive();
  }

  return 1;

}

int activeWindowClass::saveWin (
  FILE *f ) {

int stat, index;
commentLinesPtr commentCur;

#if 0
int r, g, b;
#endif

  commentCur = commentHead->flink;
  while ( commentCur ) {
    if ( commentCur->line ) fprintf( f, "%s", commentCur->line );
    commentCur = commentCur->flink;
  }

  fprintf( f, "%-d %-d %-d\n", AWC_MAJOR_VERSION, AWC_MINOR_VERSION,
   AWC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );
  stat = writeStringToFile( f, defaultFontTag );
  fprintf( f, "%-d\n", defaultAlignment );
  stat = writeStringToFile( f, defaultCtlFontTag );
  fprintf( f, "%-d\n", defaultCtlAlignment );

#if 0

  ci->getRGB( fgColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  ci->getRGB( bgColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  ci->getRGB( defaultTextFgColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  ci->getRGB( defaultFg1Color, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  ci->getRGB( defaultFg2Color, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  ci->getRGB( defaultBgColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  ci->getRGB( defaultTopShadowColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  ci->getRGB( defaultBotShadowColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );
  ci->getRGB( defaultOffsetColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

#endif

#if 1

  // major version >= 3

  index = fgColor;
  fprintf( f, "%-d\n", index );
  index = bgColor;
  fprintf( f, "%-d\n", index );
  index = defaultTextFgColor;
  fprintf( f, "%-d\n", index );
  index = defaultFg1Color;
  fprintf( f, "%-d\n", index );
  index = defaultFg2Color;
  fprintf( f, "%-d\n", index );
  index = defaultBgColor;
  fprintf( f, "%-d\n", index );
  index = defaultTopShadowColor;
  fprintf( f, "%-d\n", index );
  index = defaultBotShadowColor;
  fprintf( f, "%-d\n", index );
  index = defaultOffsetColor;
  fprintf( f, "%-d\n", index );

#endif

  stat = writeStringToFile( f, title );

  // version 1.5.0
  fprintf( f, "%-d\n", gridShow );
  fprintf( f, "%-d\n", gridActive );
  fprintf( f, "%-d\n", gridSpacing );
  fprintf( f, "%-d\n", orthogonal );

  // version 1.6.0
  stat = writeStringToFile( f, defaultPvType );

  // version 1.7.0
  stat = writeStringToFile( f, this->id );
  fprintf( f, "%-d\n", this->activateCallbackFlag );
  fprintf( f, "%-d\n", this->deactivateCallbackFlag );

  // version 2.0.1
  stat = writeStringToFile( f, defaultBtnFontTag );
  fprintf( f, "%-d\n", defaultBtnAlignment );

  return 1;

}

void activeWindowClass::readCommentsAndVersion (
  FILE *f
) {

char oneLine[255+1], buf[255+1], *tk;
commentLinesPtr commentCur;
int numComments = 0, moreComments = 1;

  do {

    readStringFromFile( oneLine, 255, f ); incLine();

    strcpy( buf, oneLine );

    tk = strtok( buf, " \t\n" );

    if ( !tk || ( tk[0] == '#' ) ) {

      numComments++;
      commentCur = new commentLinesType;
      commentCur->line = new (char)[strlen(oneLine)+4];
      strcpy( commentCur->line, oneLine );
      strcat( commentCur->line, "\n" );
      commentTail->flink = commentCur;
      commentTail = commentCur;
      commentTail->flink = NULL;

    }
    else {
      moreComments = 0;
    }

  } while ( moreComments );

  if ( !numComments ) {
    commentTail = commentHead;
    commentTail->flink = NULL;
  }

  sscanf( oneLine, "%d %d %d\n", &major, &minor, &release );

}

void activeWindowClass::discardCommentsAndVersion (
  FILE *f,
  int *_major,
  int *_minor,
  int *_release
) {

char oneLine[255+1], buf[255+1], *tk;
int moreComments = 1;

  do {

    // don't inc line here
    readStringFromFile( oneLine, 255, f );

    strcpy( buf, oneLine );

    tk = strtok( buf, " \t\n" );

    if ( !tk || ( tk[0] == '#' ) ) {
      // do nothing
    }
    else {
      moreComments = 0;
    }

  } while ( moreComments );

  sscanf( oneLine, "%d %d %d\n", _major, _minor, _release );

}

int activeWindowClass::loadWin (
  FILE *f ) {

  // if this is changed then activeWindowClass::discardWinLoadData
  // must be likewise changed

int stat;
int r, g, b, n, index;
Arg args[5];
unsigned int pixel;

  readCommentsAndVersion( f );

  fscanf( f, "%d\n", &x ); incLine();
  fscanf( f, "%d\n", &y ); incLine();
  fscanf( f, "%d\n", &w ); incLine();
  fscanf( f, "%d\n", &h ); incLine();

   n = 0;
   XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
   XtSetValues( drawWidget, args, n );

   n = 0;
   XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
   XtSetValues( drawWidget, args, n );

   n = 0;
   XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
   XtSetValues( top, args, n );

   n = 0;
   XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
   XtSetValues( top, args, n );

   readStringFromFile( defaultFontTag, 63, f ); incLine();

  if ( strcmp( defaultFontTag, "" ) != 0 ) {
    stat = defaultFm.setFontTag( defaultFontTag );
  }

  fscanf( f, "%d\n", &defaultAlignment ); incLine();

  if ( defaultAlignment != 0 ) {
    stat = defaultFm.setFontAlignment( defaultAlignment );
  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {

    readStringFromFile( defaultCtlFontTag, 63, f ); incLine();

    if ( strcmp( defaultCtlFontTag, "" ) != 0 ) {
      stat = defaultCtlFm.setFontTag( defaultCtlFontTag );
    }

    fscanf( f, "%d\n", &defaultCtlAlignment ); incLine();

    if ( defaultCtlAlignment != 0 ) {
      stat = defaultCtlFm.setFontAlignment( defaultCtlAlignment );
    }

  }
  else {

    if ( strcmp( defaultFontTag, "" ) != 0 ) {
      stat = defaultCtlFm.setFontTag( defaultFontTag );
    }

    if ( defaultAlignment != 0 ) {
      stat = defaultCtlFm.setFontAlignment( defaultAlignment );
    }

  }

  if ( major >= 3 ) {

    fscanf( f, "%d\n", &index ); incLine();
    fgColor = index;

    fscanf( f, "%d\n", &index ); incLine();
    bgColor = index;

    drawGc.setBaseBG( ci->pix(bgColor) );

    fscanf( f, "%d\n", &index ); incLine();
    defaultTextFgColor = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultFg1Color = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultFg2Color = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultBgColor = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultTopShadowColor = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultBotShadowColor = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultOffsetColor = index;

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &fgColor );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &bgColor );

    drawGc.setBaseBG( ci->pix(bgColor) );

    if ( ( major > 1 ) || ( minor > 2 ) ) {
      fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
      if ( ( major < 2 ) && ( minor < 4 ) ) {
        r *= 256;
        g *= 256;
        b *= 256;
      }
      ci->setRGB( r, g, b, &pixel );
    }
    else {
      ci->setRGB( r, g, b, &pixel );
    }

    stat = ci->getIndex( pixel, &defaultTextFgColor );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &defaultFg1Color );

    if ( ( major > 1 ) || ( minor > 2 ) ) {
      fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
      if ( ( major < 2 ) && ( minor < 4 ) ) {
        r *= 256;
        g *= 256;
        b *= 256;
      }
      ci->setRGB( r, g, b, &pixel );
    }
    else {
      ci->setRGB( r, g, b, &pixel );
    }

    stat = ci->getIndex( pixel, &defaultFg2Color );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &defaultBgColor );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &defaultTopShadowColor );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &defaultBotShadowColor );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &defaultOffsetColor );

  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( title, 127, f ); incLine();
  }
  else {
    strcpy( title, "" );
  }
  expStrTitle.setRaw( title );

  if ( ( major > 1 ) || ( minor > 4 ) ) {

    fscanf( f, "%d\n", &gridShow ); incLine();
    if ( gridShow )
      strcpy( gridShowStr, activeWindowClass_str3 );
    else
      strcpy( gridShowStr, activeWindowClass_str5 );

    fscanf( f, "%d\n", &gridActive ); incLine();
    if ( gridActive )
      strcpy( gridActiveStr, activeWindowClass_str3 );
    else
      strcpy( gridActiveStr, activeWindowClass_str5 );

    fscanf( f, "%d\n", &gridSpacing ); incLine();

    fscanf( f, "%d\n", &orthogonal ); incLine();

  }
  else {

    gridShow = 0;
    strcpy( gridShowStr, activeWindowClass_str5 );
    gridActive = 0;
    strcpy( gridActiveStr, activeWindowClass_str5 );
    gridSpacing = 10;
    orthogonal = 0;

  }

  if ( ( major > 1 ) || ( minor > 5 ) ) {
    readStringFromFile( defaultPvType, 15, f ); incLine();
  }
  else {
    strcpy( defaultPvType, "epics" );
  }

  if ( ( major > 1 ) || ( minor > 6 ) ) {
    readStringFromFile( this->id, 31, f ); incLine();
    fscanf( f, "%d\n", &this->activateCallbackFlag ); incLine();
    fscanf( f, "%d\n", &this->deactivateCallbackFlag ); incLine();
  }
  else {
    strcpy( this->id, "" );
    activateCallbackFlag = 0;
    deactivateCallbackFlag = 0;
  }

  if ( ( major > 2 ) ||
       ( ( major == 2 ) && ( minor > 0 ) ) ||
       ( ( major == 2 ) && ( minor == 0 ) && ( release > 0 ) ) ) {

    readStringFromFile( defaultBtnFontTag, 63, f ); incLine();

    if ( strcmp( defaultBtnFontTag, "" ) != 0 ) {
      stat = defaultBtnFm.setFontTag( defaultBtnFontTag );
    }

    fscanf( f, "%d\n", &defaultBtnAlignment ); incLine();

    if ( defaultBtnAlignment != 0 ) {
      stat = defaultBtnFm.setFontAlignment( defaultBtnAlignment );
    }

  }
  else {

    if ( strcmp( defaultFontTag, "" ) != 0 ) {
      stat = defaultBtnFm.setFontTag( defaultFontTag );
    }

    if ( defaultAlignment != 0 ) {
      stat = defaultBtnFm.setFontAlignment( defaultAlignment );
    }

  }

  updateAllSelectedDisplayInfo();

  return 1;

}

int activeWindowClass::loadWin (
  FILE *f,
  int _x,
  int _y ) {

  // if this is changed then activeWindowClass::discardWinLoadData
  // must be likewise changed

int stat;
int r, g, b, n, discard, index;
Arg args[5];
unsigned int pixel;

  readCommentsAndVersion( f );

  fscanf( f, "%d\n", &discard ); incLine(); // discard x
  fscanf( f, "%d\n", &discard ); incLine(); // discard y

  x = _x;
  y = _y;

  fscanf( f, "%d\n", &w ); incLine();
  fscanf( f, "%d\n", &h ); incLine();

   n = 0;
   XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
   XtSetValues( drawWidget, args, n );

   n = 0;
   XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
   XtSetValues( drawWidget, args, n );

   n = 0;
   XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
   XtSetValues( top, args, n );

   n = 0;
   XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
   XtSetValues( top, args, n );

   readStringFromFile( defaultFontTag, 63, f ); incLine();

  if ( strcmp( defaultFontTag, "" ) != 0 ) {
    stat = defaultFm.setFontTag( defaultFontTag );
  }

  fscanf( f, "%d\n", &defaultAlignment ); incLine();

  if ( defaultAlignment != 0 ) {
    stat = defaultFm.setFontAlignment( defaultAlignment );
  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {

    readStringFromFile( defaultCtlFontTag, 63, f ); incLine();

    if ( strcmp( defaultCtlFontTag, "" ) != 0 ) {
      stat = defaultCtlFm.setFontTag( defaultCtlFontTag );
    }

    fscanf( f, "%d\n", &defaultCtlAlignment ); incLine();

    if ( defaultCtlAlignment != 0 ) {
      stat = defaultCtlFm.setFontAlignment( defaultCtlAlignment );
    }

  }
  else {

    if ( strcmp( defaultFontTag, "" ) != 0 ) {
      stat = defaultCtlFm.setFontTag( defaultFontTag );
    }

    if ( defaultAlignment != 0 ) {
      stat = defaultCtlFm.setFontAlignment( defaultAlignment );
    }

  }

  if ( major >= 3 ) {

    fscanf( f, "%d\n", &index ); incLine();
    fgColor = index;

    fscanf( f, "%d\n", &index ); incLine();
    bgColor = index;

    drawGc.setBaseBG( ci->pix(bgColor) );

    fscanf( f, "%d\n", &index ); incLine();
    defaultTextFgColor = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultFg1Color = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultFg2Color = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultBgColor = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultTopShadowColor = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultBotShadowColor = index;

    fscanf( f, "%d\n", &index ); incLine();
    defaultOffsetColor = index;

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &fgColor );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &bgColor );

    drawGc.setBaseBG( ci->pix(bgColor) );

    if ( ( major > 1 ) || ( minor > 2 ) ) {
      fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
      if ( ( major < 2 ) && ( minor < 4 ) ) {
        r *= 256;
        g *= 256;
        b *= 256;
      }
      ci->setRGB( r, g, b, &pixel );
      stat = ci->getIndex( pixel, &defaultTextFgColor );
    }
    else {
      ci->setRGB( r, g, b, &pixel );
      stat = ci->getIndex( pixel, &defaultTextFgColor );
    }

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &defaultFg1Color );

    if ( ( major > 1 ) || ( minor > 2 ) ) {
      fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
      if ( ( major < 2 ) && ( minor < 4 ) ) {
        r *= 256;
        g *= 256;
        b *= 256;
      }
      ci->setRGB( r, g, b, &pixel );
      stat = ci->getIndex( pixel, &defaultFg2Color );
    }
    else {
      ci->setRGB( r, g, b, &pixel );
      stat = ci->getIndex( pixel, &defaultFg2Color );
    }

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &defaultBgColor );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &defaultTopShadowColor );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &defaultBotShadowColor );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    stat = ci->getIndex( pixel, &defaultOffsetColor );

  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( title, 127, f ); incLine();
  }
  else {
    strcpy( title, "" );
  }
  expStrTitle.setRaw( title );

  if ( ( major > 1 ) || ( minor > 4 ) ) {

    fscanf( f, "%d\n", &gridShow ); incLine();
    if ( gridShow )
      strcpy( gridShowStr, activeWindowClass_str3 );
    else
      strcpy( gridShowStr, activeWindowClass_str5 );

    fscanf( f, "%d\n", &gridActive ); incLine();
    if ( gridActive )
      strcpy( gridActiveStr, activeWindowClass_str3 );
    else
      strcpy( gridActiveStr, activeWindowClass_str5 );

    fscanf( f, "%d\n", &gridSpacing ); incLine();

    fscanf( f, "%d\n", &orthogonal ); incLine();

  }
  else {

    gridShow = 0;
    strcpy( gridShowStr, activeWindowClass_str5 );
    gridActive = 0;
    strcpy( gridActiveStr, activeWindowClass_str5 );
    gridSpacing = 10;
    orthogonal = 0;

  }

  if ( ( major > 1 ) || ( minor > 5 ) ) {
    readStringFromFile( defaultPvType, 15, f ); incLine();
  }
  else {
    strcpy( defaultPvType, "epics" );
  }

  if ( ( major > 1 ) || ( minor > 6 ) ) {
    readStringFromFile( this->id, 31, f ); incLine();
    fscanf( f, "%d\n", &this->activateCallbackFlag ); incLine();
    fscanf( f, "%d\n", &this->deactivateCallbackFlag ); incLine();
  }
  else {
    strcpy( this->id, "" );
    activateCallbackFlag = 0;
    deactivateCallbackFlag = 0;
  }

  if ( ( major > 2 ) ||
       ( ( major == 2 ) && ( minor > 0 ) ) ||
       ( ( major == 2 ) && ( minor == 0 ) && ( release > 0 ) ) ) {

    readStringFromFile( defaultBtnFontTag, 63, f ); incLine();

    if ( strcmp( defaultBtnFontTag, "" ) != 0 ) {
      stat = defaultBtnFm.setFontTag( defaultBtnFontTag );
    }

    fscanf( f, "%d\n", &defaultBtnAlignment ); incLine();

    if ( defaultBtnAlignment != 0 ) {
      stat = defaultBtnFm.setFontAlignment( defaultBtnAlignment );
    }

  }
  else {

    if ( strcmp( defaultFontTag, "" ) != 0 ) {
      stat = defaultBtnFm.setFontTag( defaultFontTag );
    }

    if ( defaultAlignment != 0 ) {
      stat = defaultBtnFm.setFontAlignment( defaultAlignment );
    }

  }

  updateAllSelectedDisplayInfo();

  return 1;

}

int activeWindowClass::importWin (
  FILE *f ) {

int r, g, b, n;
Arg args[5];
char buf[255+1], *gotData;
int more, stat;
unsigned int pixel;

char *tk, *context;

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;

  gotData = getNextDataString( buf, 255, f );
  if ( !gotData ) {
    appCtx->postMessage( activeWindowClass_str164 );
    return 0;
  }

  context = NULL;

  tk = strtok_r( buf, " \t\n", &context );
  if ( !tk ) {
    appCtx->postMessage( activeWindowClass_str164 );
    return 0;
  }

  if ( strcmp( tk, "Display" ) != 0 ) {
    appCtx->postMessage( activeWindowClass_str164 );
    return 0;
  }

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      appCtx->postMessage( activeWindowClass_str164 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      appCtx->postMessage( activeWindowClass_str164 );
      return 0;
    }

    if ( strcmp( tk, "<eod>" ) == 0 ) {

      more = 0;

    }
    else {

      more = 1;

      if ( strcmp( tk, "x" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          appCtx->postMessage( activeWindowClass_str164 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          appCtx->postMessage( activeWindowClass_str164 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          appCtx->postMessage( activeWindowClass_str164 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          appCtx->postMessage( activeWindowClass_str164 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "title" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          appCtx->postMessage( activeWindowClass_str164 );
          return 0;
        }

        strncpy( title, tk, 127 );
        expStrTitle.setRaw( title );

      }
            
      else if ( strcmp( tk, "red" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          appCtx->postMessage( activeWindowClass_str164 );
          return 0;
        }

        r = atol( tk );

      }
            
      else if ( strcmp( tk, "green" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          appCtx->postMessage( activeWindowClass_str164 );
          return 0;
        }

        g = atol( tk );

      }
            
      else if ( strcmp( tk, "blue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          appCtx->postMessage( activeWindowClass_str164 );
          return 0;
        }

        b = atol( tk );

      }
            
    }

  } while ( more );

  ci->setRGB( r, g, b, &pixel );
  stat = ci->getIndex( pixel, &bgColor );

  n = 0;
  XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
  XtSetValues( drawWidget, args, n );

  n = 0;
  XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
  XtSetValues( drawWidget, args, n );

  n = 0;
  XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
  XtSetValues( top, args, n );

  n = 0;
  XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
  XtSetValues( top, args, n );

  drawGc.setBaseBG( ci->pix(bgColor) );

  strcpy( gridShowStr, activeWindowClass_str5 );

  strcpy( gridActiveStr, activeWindowClass_str5 );

  fscanf( f, "%d\n", &gridSpacing );

  fscanf( f, "%d\n", &orthogonal );

  return 1;

}

// used by activeSymbolClass::readSymbolFile
int activeWindowClass::discardWinLoadData (
  FILE *f,
  int *_major,
  int *_minor,
  int *_release ) {

int i, r, g, b, index;
char s[127+1];

  // don't inc line here

  pushVersion();

  // discardCommentsAndVersion( f, _major, _minor, _release );
  readCommentsAndVersion( f );

  *_major = major;
  *_minor = minor;
  *_release = release;

  fscanf( f, "%d\n", &i );
  fscanf( f, "%d\n", &i );
  fscanf( f, "%d\n", &i );
  fscanf( f, "%d\n", &i );

  readStringFromFile( s, 63, f );

  fscanf( f, "%d\n",&i );

  if ( ( *_major > 1 ) || ( *_minor > 2 ) ) {
    readStringFromFile( s, 63, f );
    fscanf( f, "%d\n",&i );
  }

  if ( major >= 3 ) {

    fscanf( f, "%d\n", &index ); incLine();

    fscanf( f, "%d\n", &index ); incLine();

    fscanf( f, "%d\n", &index ); incLine();

    fscanf( f, "%d\n", &index ); incLine();

    fscanf( f, "%d\n", &index ); incLine();

    fscanf( f, "%d\n", &index ); incLine();

    fscanf( f, "%d\n", &index ); incLine();

    fscanf( f, "%d\n", &index ); incLine();

    fscanf( f, "%d\n", &index ); incLine();

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b );

    fscanf( f, "%d %d %d\n", &r, &g, &b );

    if ( ( *_major > 1 ) || ( *_minor > 2 ) ) {
      fscanf( f, "%d %d %d\n", &r, &g, &b );
    }

    fscanf( f, "%d %d %d\n", &r, &g, &b );

    if ( ( *_major > 1 ) || ( *_minor > 2 ) ) {
      fscanf( f, "%d %d %d\n", &r, &g, &b );
    }

    fscanf( f, "%d %d %d\n", &r, &g, &b );

    fscanf( f, "%d %d %d\n", &r, &g, &b );

    fscanf( f, "%d %d %d\n", &r, &g, &b );

    fscanf( f, "%d %d %d\n", &r, &g, &b );

  }

  if ( ( *_major > 1 ) || ( *_minor > 1 ) ) {
    readStringFromFile( s, 127, f );
  }

  if ( ( *_major > 1 ) || ( *_minor > 4 ) ) {

    fscanf( f, "%d\n", &i );

    fscanf( f, "%d\n", &i );

    fscanf( f, "%d\n", &i );

    fscanf( f, "%d\n", &i );

  }

  if ( ( *_major > 1 ) || ( *_minor > 5 ) ) {
    readStringFromFile( defaultPvType, 15, f );
  }
  else {
    strcpy( defaultPvType, "epics" );
  }

  if ( ( *_major > 1 ) || ( *_minor > 6 ) ) {
    readStringFromFile( this->id, 31, f );
    fscanf( f, "%d\n", &this->activateCallbackFlag );
    fscanf( f, "%d\n", &this->deactivateCallbackFlag );
  }
  else {
    strcpy( this->id, "" );
    activateCallbackFlag = 0;
    deactivateCallbackFlag = 0;
  }

  popVersion();

  return 1;

}

void activeWindowClass::displayGrid ( void ) {

int x, y;

  drawGc.saveFg();
  drawGc.setFG( ci->pix(fgColor) );

  for ( y=0; y<=h; y+=gridSpacing ) {
    for ( x=0; x<=w; x+=gridSpacing ) {
      XDrawPoint( d, XtWindow(drawWidget), drawGc.normGC(),
       x, y );
    }
  }

  drawGc.restoreFg();

}

void activeWindowClass::displayGrid (
  int _x,
  int _y,
  int _w,
  int _h )
{

int x, y, x0, x1, y0, y1;

  x0 = _x;
  y0 = _y;
  x1 = _x + _w;
  y1 = _y + _h;

  drawGc.saveFg();
  drawGc.setFG( ci->pix(fgColor) );

  for ( y=0; y<=h; y+=gridSpacing ) {
    for ( x=0; x<=w; x+=gridSpacing ) {
      if ( ( x >= x0 ) && ( x <= x1 ) && ( y >= y0 ) && ( y <= y1 ) ) {
        XDrawPoint( d, XtWindow(drawWidget), drawGc.normGC(), x, y );
      }
    }
  }

  drawGc.restoreFg();

}

int activeWindowClass::fileExists (
  char *fname )
{

FILE *f;
int result;

  f = fopen( fname, "r" );
  if ( f ) {
    result = 1;
    fclose( f );
  }
  else
    result = 0;

  return result;

}

int activeWindowClass::edlFileExists (
  char *fName )
{

FILE *f;
char *gotOne, oneFileName[255+1];
int result;

  gotOne = strstr( fName, "/" );

  if ( gotOne ) {
    strncpy( oneFileName, fName, 255 );
  }
  else {
    strncpy( oneFileName, appCtx->curPath, 255 );
    strncat( oneFileName, fName, 255 );
  }

  if ( strlen(oneFileName) > strlen(".edl") ) {
    if ( strcmp( &oneFileName[strlen(oneFileName)-strlen(".edl")], ".edl" ) !=
      0 ) {
      strncat( oneFileName, ".edl", 255 );
    }
  }
  else {
    strncat( oneFileName, ".edl", 255 );
  }

  f = fopen( oneFileName, "r" );
  if ( f ) {
    result = 1;
    fclose( f );
  }
  else
    result = 0;

  return result;

}

void activeWindowClass::lineEditBegin ( void )
{

  cursor.set( XtWindow(drawWidget), CURSOR_K_TINYCROSSHAIR );
  cursor.setColor( ci->pix(fgColor), ci->pix(bgColor) );
  state = AWC_EDITING_POINTS;

}

void activeWindowClass::operationComplete ( void )
{

  cursor.set( XtWindow(drawWidget), CURSOR_K_CROSSHAIR );
  cursor.setColor( ci->pix(fgColor), ci->pix(bgColor) );
  state = savedState;

}

void activeWindowClass::setDisplayScheme (
  displaySchemeClass *displayScheme )
{

int stat;

  if ( !displayScheme->isLoaded() ) return;

  strncpy( defaultPvType, displayScheme->getPvType(), 15 );

  strncpy( defaultFontTag, displayScheme->getFont(), 127 );
  if ( strcmp( defaultFontTag, "" ) != 0 ) {
    stat = defaultFm.setFontTag( defaultFontTag );
  }

  defaultAlignment = displayScheme->getAlignment();
  if ( defaultAlignment != 0 ) {
    stat = defaultFm.setFontAlignment( defaultAlignment );
  }

  strncpy( defaultCtlFontTag, displayScheme->getCtlFont(), 127 );
  if ( strcmp( defaultCtlFontTag, "" ) != 0 ) {
    stat = defaultCtlFm.setFontTag( defaultCtlFontTag );
  }

  defaultCtlAlignment = displayScheme->getCtlAlignment();
  if ( defaultCtlAlignment != 0 ) {
    stat = defaultCtlFm.setFontAlignment( defaultCtlAlignment );
  }

  strncpy( defaultBtnFontTag, displayScheme->getBtnFont(), 127 );
  if ( strcmp( defaultBtnFontTag, "" ) != 0 ) {
    stat = defaultBtnFm.setFontTag( defaultBtnFontTag );
  }

  defaultBtnAlignment = displayScheme->getBtnAlignment();
  if ( defaultBtnAlignment != 0 ) {
    stat = defaultBtnFm.setFontAlignment( defaultBtnAlignment );
  }

  fgColor = displayScheme->getFg();

  bgColor = displayScheme->getBg();

  defaultTextFgColor = displayScheme->getDefTextFg();

  defaultFg1Color = displayScheme->getDefFg1();

  defaultFg2Color = displayScheme->getDefFg2();

  defaultBgColor = displayScheme->getDefBg();

  defaultTopShadowColor = displayScheme->getTopShadow();

  defaultBotShadowColor = displayScheme->getBotShadow();

  defaultOffsetColor = displayScheme->getOffset();

  drawGc.setFG( ci->pix(fgColor) );
  drawGc.setBG( ci->pix(bgColor) );
  drawGc.setBaseBG( ci->pix(bgColor) );
  executeGc.setBaseBG( ci->pix(bgColor) );
  cursor.setColor( ci->pix(fgColor), ci->pix(bgColor) );

  updateAllSelectedDisplayInfo();

}

void activeWindowClass::updateEditSelectionPointers ( void ) {

activeGraphicListPtr cur, next;

  // first clear all nextSelectedToEdit pointers
  cur = head->flink;
  while ( cur != head ) {
    cur->node->clearNextSelectedToEdit();
    cur = cur->flink;
  }

  // then set all nextSelectedToEdit pointers for all nodes in select list
  cur = selectedHead->selFlink;
  while ( cur != selectedHead ) {
    next = cur->selFlink;
    if ( next != selectedHead ) {
      cur->node->setNextSelectedToEdit( next->node );
    }
    else {
      cur->node->clearNextSelectedToEdit();
    }
    cur = next;
  }

}

void activeWindowClass::updateMasterSelection ( void ) {

activeGraphicListPtr cur;
int x0, y0, x1, y1;

  // update nextSelectedToEdit pointers also
  updateEditSelectionPointers();

  // now, get master selection coordinates

  cur = selectedHead->selFlink;

  if ( cur != selectedHead ) {

    masterSelectX0 = cur->node->getX0();
    masterSelectY0 = cur->node->getY0();
    masterSelectX1 = cur->node->getX1();
    masterSelectY1 = cur->node->getY1();

    cur = cur->selFlink;

  }
  else {

    masterSelectX0 = 0;
    masterSelectY0 = 0;
    masterSelectX1 = 0;
    masterSelectY1 = 0;

  }

  while ( cur != selectedHead ) {

    x0 = cur->node->getX0();
    if ( x0 < masterSelectX0 ) masterSelectX0 = x0;

    y0 = cur->node->getY0();
    if ( y0 < masterSelectY0 ) masterSelectY0 = y0;

    x1 = cur->node->getX1();
    if ( x1 > masterSelectX1 ) masterSelectX1 = x1;

    y1 = cur->node->getY1();
    if ( y1 > masterSelectY1 ) masterSelectY1 = y1;

    cur = cur->selFlink;

  }

  if ( masterSelectX1 == masterSelectX0 ) masterSelectX1 = masterSelectX0 + 1;
  if ( masterSelectY1 == masterSelectY0 ) masterSelectY1 = masterSelectY0 + 1;

}

void activeWindowClass::processObjects ( void )
{

activeGraphicListPtr cur, next;

  appCtx->proc->lock();
  cur = defExeHead->defExeFlink;
  appCtx->proc->unlock();

  if ( !cur ) return;

  while ( cur != defExeHead ) {

    appCtx->proc->lock();
    next = cur->defExeFlink;
    appCtx->proc->unlock();

    cur->node->executeDeferred();

    cur = next;

  }

}

char *activeWindowClass::idName( void )
{

  return id;

}

int activeWindowClass::setProperty (
  char *id,
  char *property,
  char *value )
{

int stat;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    if ( strcmp( id, cur->node->idName() ) == 0 ) {
      stat = cur->node->setProperty( property, value );
      return stat;
    }

    cur = cur->flink;

  }

  return 0; // not found

}

int activeWindowClass::setProperty (
  char *id,
  char *property,
  double *value )
{

int stat;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    if ( strcmp( id, cur->node->idName() ) == 0 ) {
      stat = cur->node->setProperty( property, value );
      return stat;
    }

    cur = cur->flink;

  }

  return 0; // not found

}

int activeWindowClass::setProperty (
  char *id,
  char *property,
  int *value )
{

int stat;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    if ( strcmp( id, cur->node->idName() ) == 0 ) {
      stat = cur->node->setProperty( property, value );
      return stat;
    }

    cur = cur->flink;

  }

  return 0; // not found

}

int activeWindowClass::getProperty (
  char *id,
  char *property,
  int bufSize,
  char *value )
{

int stat;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    if ( strcmp( id, cur->node->idName() ) == 0 ) {
      stat = cur->node->getProperty( property, bufSize, value );
      return stat;
    }

    cur = cur->flink;

  }

  return 0; // not found

}

int activeWindowClass::getProperty (
  char *id,
  char *property,
  double *value )
{

int stat;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    if ( strcmp( id, cur->node->idName() ) == 0 ) {
      stat = cur->node->getProperty( property, value );
      return stat;
    }

    cur = cur->flink;

  }

  return 0; // not found

}

int activeWindowClass::getProperty (
  char *id,
  char *property,
  int *value )
{

int stat;
activeGraphicListPtr cur;

  cur = head->flink;
  while ( cur != head ) {

    if ( strcmp( id, cur->node->idName() ) == 0 ) {
      stat = cur->node->getProperty( property, value );
      return stat;
    }

    cur = cur->flink;

  }

  return 0; // not found

}

void activeWindowClass::storeFileName (
  char *inName )
{

  strncpy( fileName, inName, 255 );
  getFileName( name, inName, 127 );
  getFilePrefix( prefix, inName, 127 );
  getFilePostfix( postfix, inName, 127 );

}

FILE *activeWindowClass::openAny (
  char *name,
  char *mode )
{

char buf[255+1];
FILE *f;
int i;

  for ( i=0; i<appCtx->numPaths; i++ ) {

    appCtx->expandFileName( i, buf, name, ".edl", 255 );

    if ( strcmp( buf, "" ) != 0 ) {
      f = fopen( buf, mode );
      if ( f ) {
        strncpy( fileName, buf, 255 ); // update fileName
        return f;
      }
    }

  }

  return NULL;

}

FILE *activeWindowClass::openExchangeFile (
  char *name,
  char *mode )
{

char buf[255+1];
FILE *f;
int i;

  for ( i=0; i<appCtx->numPaths; i++ ) {

    appCtx->expandFileName( i, buf, name, ".xch", 255 );
    if ( strcmp( buf, "" ) != 0 ) {
      f = fopen( buf, mode );
      if ( f ) {
        strncpy( fileName, buf, 255 ); // update fileName
        return f;
      }
    }

  }

  return NULL;

}

void activeWindowClass::executeFromDeferredQueue( void )
{

  if ( doAutoSave ) {

    int stat;
    char name[255+1], oldName[255+1];

    doAutoSave = 0;

    if ( !changeSinceAutoSave ) return;

    changeSinceAutoSave = 0;

    strncpy( oldName, autosaveName, 255 );
    oldName[255] = 0;

    strncpy( autosaveName, activeWindowClass_str1, 255 );

    if ( strcmp( fileName, "" ) != 0 ) {
      extractName( fileName, name );
      strncat( autosaveName, "_", 255 );
      strncat( autosaveName, name, 255 );
    }

    strncat( autosaveName, "_XXXXXX", 255 );

    mkstemp( autosaveName );

    saveNoChange( autosaveName );

    if ( mode != AWC_EXECUTE ) {

      strcpy( restoreTitle, title );
      strcpy( title, activeWindowClass_str2 );
      expStrTitle.setRaw( title );
      setTitleUsingTitle();
      XFlush( d );

      if ( restoreTimer ) {
        XtRemoveTimeOut( restoreTimer );
        restoreTimer = 0;
      }
      restoreTimer = XtAppAddTimeOut( appCtx->appContext(),
       3000, acw_restoreTitle, this );

    }

    // now delete previous autosave file
    if ( strcmp( oldName, "" ) != 0 ) {
      stat = unlink( oldName );
    }

  }


  if ( doClose ) {

    if ( mode != AWC_EDIT ) {

      if ( waiting == 0 ) {
        doClose = 0;
        returnToEdit( 1 );
      }
      else if ( waiting < 0 ) { // wait, don't close
        appCtx->postDeferredExecutionNextQueue( this );
      }
      else {                    // close after n cycles
        waiting--;
        appCtx->postDeferredExecutionNextQueue( this );
      }

    }
    else {

      doClose = 0;

      if ( change ) {

        savedState = state;
        state = AWC_WAITING;

        confirm.create( top, x, y, 3,
         activeWindowClass_str161, NULL, NULL );
        confirm.addButton( activeWindowClass_str163, awc_continue_cb,
         (void *) this );
        confirm.addButton( activeWindowClass_str165, awc_abort_cb,
         (void *) this );
        confirm.addButton( activeWindowClass_str166, awc_save_and_exit_cb,
         (void *) this );
        confirm.finished();
        confirm.popup();
        XSetWindowColormap( d, XtWindow(confirm.top()),
         appCtx->ci.getColorMap() );

      }
      else {

        if ( autosaveTimer ) {
          XtRemoveTimeOut( autosaveTimer );
          autosaveTimer = 0;
        }
        if ( restoreTimer ) {
          XtRemoveTimeOut( restoreTimer );
          restoreTimer = 0;
        }

        //mark active window for delege
        appCtx->removeActiveWindow( this );

        XtUnmanageChild( drawWidget );

      }

    }

  }

}

int activeWindowClass::readUntilEndOfData (
  FILE *f ) {

char buf[1023+1];
char *ptr;

  if ( major < 2 ) {
    return 1; // success
  }

  while ( 1 ) {

    ptr = fgets( buf, 1023, f ); incLine();
    if ( !ptr ) {
      return 0; // error
    }

    if ( strcmp( buf, "E\002O\002D\n" ) == 0 ) {
      return 1; // success
    }

    if ( strcmp( buf, "<<<E~O~D>>>\n" ) == 0 ) {
      return 1; // success
    }

  }

  // execution cannot get here

}

int activeWindowClass::readUntilEndOfData (
  FILE *f,
  int _major,
  int _minor,
  int _release ) {

char buf[1023+1];
char *ptr;

  // don't inc line here

  if ( _major < 2 ) {
    return 1; // success
  }

  while ( 1 ) {

    ptr = fgets( buf, 1023, f );
    if ( !ptr ) {
      return 0; // error
    }

    if ( strcmp( buf, "E\002O\002D\n" ) == 0 ) {
      return 1; // success
    }

    if ( strcmp( buf, "<<<E~O~D>>>\n" ) == 0 ) {
      return 1; // success
    }

  }

  // execution cannot get here

}

void activeWindowClass::initLine ( void ) {

  fileLineNumber = 0;

}

void activeWindowClass::incLine ( void ) {

  fileLineNumber++;

}

int activeWindowClass::line ( void ) {

  return fileLineNumber;

}

void activeWindowClass::setLine (
  int _line ) {

  fileLineNumber = _line;

}

void activeWindowClass::substituteSpecial (
  int max,
  char *bufIn,
  char *bufOut )
{

char param[1023+1], dspName[127+1], *envPtr;
int i, len, iIn, iOut, p0, p1, more, state, winid;

  state = 1; // copying

  p0 = iIn = iOut = 0;
  more = 1;
  while ( more ) {

    switch ( state ) {

    case 1: // copying

      if ( bufIn[iIn] == 0 ) {
        bufOut[iOut] = bufIn[iIn];
        more = 0;
        break;
      }
      else if ( bufIn[iIn] == '<' ) {
        state = 2; // finding param
	p0 = iIn;
	break;
      }
      bufOut[iOut] = bufIn[iIn];
      if ( iOut < max ) iOut++;
      break;

    case 2: // finding param

      if ( bufIn[iIn] == 0 ) {
        bufOut[iOut] = bufIn[iIn];
        more = 0;
        break;
      }
      else if ( bufIn[iIn] == '>' ) {
	p1 = iIn;
	len = p1 - p0 + 1;
        if ( len > 1023 ) len = 1023;
	strncpy( param, &bufIn[p0], len );
	param[len] = 0;

        if ( strcmp( param, "<WINID>" ) == 0 ) {
          winid = (int) XtWindow( this->top );
          sprintf( param, "%-d", winid );
          bufOut[iOut] = 0;
          strncat( bufOut, param, max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	}
        else if ( strcmp( param, "<TITLE>" ) == 0 ) {
          bufOut[iOut] = 0;
          if ( expStrTitle.getExpanded() ) {
            if ( !blank( expStrTitle.getExpanded() ) ) {
              strncat( bufOut, expStrTitle.getExpanded(), max );
	    }
	    else {
              strncat( bufOut, activeWindowClass_str83, max );
	    }
	  }
          else {
            strncat( bufOut, activeWindowClass_str83, max );
	  }
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	}
        else if ( strcmp( param, "<PROJDIR>" ) == 0 ) {
          bufOut[iOut] = 0;
          strncat( bufOut, appCtx->dataFilePrefix[0], max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	}
        else if ( strcmp( param, "<HELPDIR>" ) == 0 ) {
          bufOut[iOut] = 0;
          envPtr = getenv( environment_str5 );
          if ( envPtr ) {
            strncat( bufOut, envPtr, max );
            iOut = strlen( bufOut );
            if ( iOut >= max ) iOut = max - 1;
	  }
	}
        else if ( strcmp( param, "<DSPNAME>" ) == 0 ) {
          bufOut[iOut] = 0;
          strncpy( dspName, XDisplayName(""), 127 );
          strncat( bufOut, dspName, max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	}
        else if ( strcmp( param, "<DSPID>" ) == 0 ) {
          bufOut[iOut] = 0;
          strncpy( dspName, XDisplayName(""), 127 );
          for ( i=0; i<(int) strlen(dspName); i++ ) {
            if ( dspName[i] == '.' ) dspName[i] = '-';
	  }
          strncat( bufOut, dspName, max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	}

        state = 1; //copying
      }
      break;

    }

    iIn++;

    if ( iIn >= max ) more = 0;

  }

}

static void dragMenuCb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

dragPopupBlockPtr ptr = (dragPopupBlockPtr) client;
activeGraphicClass *ago = (activeGraphicClass *) ptr->ago;
int num = ptr->num;

  ago->setCurrentDragIndex( num );

}

void activeWindowClass::popupDragBegin (
   char *label )
{

Arg args[5];
int n;
XmString str;
Widget labelW, sepW;

  if ( dragPopup ) {
    XtDestroyWidget( dragPopup );
    dragPopup = NULL;
  }

  n = 0;
  XtSetArg( args[n], XmNmenuPost, (XtArgVal) "<Btn5Down>;" ); n++;
  dragPopup = XmCreatePopupMenu( top, "", args, n );

  str = XmStringCreateLocalized( label );

  labelW = XtVaCreateManagedWidget( "", xmLabelWidgetClass,
   dragPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  sepW = XtVaCreateManagedWidget( "", xmSeparatorWidgetClass,
   dragPopup,
   NULL );

  dragItemIndex = 0;

}

void activeWindowClass::popupDragBegin ( void ) {

Arg args[5];
int n;

  if ( dragPopup ) {
    XtDestroyWidget( dragPopup );
    dragPopup = NULL;
  }

  n = 0;
  XtSetArg( args[n], XmNmenuPost, (XtArgVal) "<Btn5Down>;" ); n++;
  dragPopup = XmCreatePopupMenu( top, "", args, n );

  dragItemIndex = 0;

}

void activeWindowClass::popupDragAddItem (
  void *actGrfPtr,
  char *item )
{

XmString str;
Widget pb;

  str = XmStringCreateLocalized( item );

  pb = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
   dragPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  dragPopupBlock[dragItemIndex].w = pb;
  dragPopupBlock[dragItemIndex].num = dragItemIndex;
  dragPopupBlock[dragItemIndex].ago = actGrfPtr;

  XtAddCallback( pb, XmNactivateCallback, dragMenuCb,
   (XtPointer) &dragPopupBlock[dragItemIndex] );

  if ( dragItemIndex < 9 ) dragItemIndex++;

}

void activeWindowClass::popupDragFinish (
  int _x,
  int _y )
{

XButtonPressedEvent be;

  memset( (char *) &be, 0, sizeof(be) );
  be.type = ButtonPress;
  be.x_root = x + _x;
  be.y_root = y + _y;

  XmMenuPosition( dragPopup, &be );
  XtManageChild( dragPopup );

}

void activeWindowClass::enableBuffering ( void ) {

}

void activeWindowClass::disableBuffering ( void ) {

}

void activeWindowClass::setUndoText (
  char *string )
{

XmString str;

  if ( !string ) {

    str = XmStringCreateLocalized( activeWindowClass_str169 ); // "undo"
    XtVaSetValues( undoPb1,
     XmNlabelString, str,
     XmNsensitive, 0,
     NULL );
    XtVaSetValues( undoPb2,
     XmNlabelString, str,
     XmNsensitive, 0,
     NULL );
    XtVaSetValues( undoPb3,
     XmNlabelString, str,
     XmNsensitive, 0,
     NULL );
    XmStringFree( str );

  }
  else {

    str = XmStringCreateLocalized( string );
    XtVaSetValues( undoPb1,
     XmNlabelString, str,
     XmNsensitive, 1,
     NULL );
    XtVaSetValues( undoPb2,
     XmNlabelString, str,
     XmNsensitive, 1,
     NULL );
    XtVaSetValues( undoPb3,
     XmNlabelString, str,
     XmNsensitive, 1,
     NULL );
    XmStringFree( str );

  }

}

void activeWindowClass::closeDeferred (
  int cycles )
{

  waiting = 1;
  doClose = 1;
  appCtx->postDeferredExecutionQueue( this );

}

int activeWindowClass::checkPoint (
  FILE *fptr )
{

int i;

  if ( fptr ) {
    fprintf( fptr, "name=%s\tx=%-d\ty=%-d\ti=%-d\n", fileName, x, y,
     isIconified );
    fprintf( fptr, "num=%-d\n", numMacros );
    for ( i=0; i<numMacros; i++ ) {
      fprintf( fptr, "%s=%s\n", macros[i], expansions[i] );
    }
    fprintf( fptr, "\n" );
  }
  else {
    printf( "name=%s\tx=%-d\ty=%-d\ti=%-d\n", fileName, x, y,
     isIconified );
    printf( "num=%-d\n", numMacros );
    for ( i=0; i<numMacros; i++ ) {
      printf( "%s=%s\n", macros[i], expansions[i] );
    }
    printf( "\n" );
  }

  return 1;

}

void activeWindowClass::openExecuteSysFile (
  char *fName )
{

activeWindowListPtr cur;
char buf[255+1], *envPtr;
int i, numMacros;
char *sysValues[5], *ptr;

char *sysMacros[] = {
  "help"
};

  if ( !*fName ) {
    return;
  }

  // is help file already open?
  cur = appCtx->head->flink;
  while ( cur != appCtx->head ) {
    if ( strcmp( fName, cur->node.name ) == 0 ) {
      // deiconify
      XMapWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
      // raise
      XRaiseWindow( cur->node.d, XtWindow(cur->node.topWidgetId()) );
      return; // display is already open; don't open another instance
    }
    cur = cur->flink;
  }

  envPtr = getenv( environment_str5 );
  if ( envPtr ) {

    strncpy( buf, envPtr, 255 );

    if ( buf[strlen(buf)-1] != '/' ) {
      strncat( buf, "/", 255 );
    }

  }
  else {

    strcpy( buf, "/etc/" );

  }

  // build system macros

  numMacros = 0;

  ptr = new char[strlen(buf)+1];
  strcpy( ptr, buf );
  sysValues[0] = ptr;

  numMacros++;

  // ============

  strncat( buf, fName, 255 );
  strncat( buf, ".edl", 255 );

  cur = new activeWindowListType;
  appCtx->addActiveWindow( cur );

  cur->node.createNoEdit( appCtx, NULL, 0, 0, 0, 0, numMacros,
   sysMacros, sysValues );

  for ( i=0; i<numMacros; i++ ) {
    delete sysValues[i];
  }

  cur->node.realize();
  cur->node.setGraphicEnvironment( &appCtx->ci, &appCtx->fi );

  cur->node.storeFileName( buf );

  appCtx->openActivateActiveWindow( &cur->node );

}
