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
#include <sys/stat.h>
#include <setjmp.h>

#include <math.h>
#include "app_pkg.h"
#include "act_win.h"
#include "lookup.h"

#include "thread.h"
#include "crc.h"

#include "remFileOpen.h"

static int gFastRefresh = -1;
static const int gMaxExecutePasses = 7;

#define ADD_SCROLLED_WIN

#ifdef ADD_SCROLLED_WIN
static void b2ReleaseClip_cb( Widget, XtPointer, XtPointer );
#endif

void _edmDebug ( void ) {

int i;

  i = 1;

}

static void mon_pv (
  ProcessVariable *pv,
  void *userarg
) {

}

static void update_pv (
  ProcessVariable *pv,
  void *userarg
) {

}

static char *getPvValSync (
  char *name
) {


// get pv value synchronously

char *val = NULL;
char buf[255+1];
int len;
ProcessVariable *pvId = NULL;

  pvId = the_PV_Factory->create( name );
  if ( pvId ) {

    pvId->add_conn_state_callback( mon_pv, NULL );
    pvId->add_value_callback( update_pv, NULL );
    pend_io( 5.0 );
    pend_event( 0.0001 );

    if ( pvId->is_valid() ) {

      if ( pvId->get_type().type == ProcessVariable::Type::real ) {
        snprintf( buf, 255, "%-g", pvId->get_double() );
        len = strlen( buf );
      }
      else if ( pvId->get_type().type == ProcessVariable::Type::integer ) {
        snprintf( buf, 255, "%-d", pvId->get_int() );
        len = strlen( buf );
      }
      else {
        len = pvId->get_string( buf, 255 );
        len = strlen( buf );
      }

      if ( len ) {
        val = new char[len+1];
        strncpy( val, buf, len );
        val[len] = 0;
      }

    }

    pvId->remove_value_callback( mon_pv, NULL );
    pvId->remove_conn_state_callback( mon_pv, NULL );
    pvId->release();
    pvId = NULL;

  }

  return val;

}

static void showObjectDimensions (
  XtPointer client,
  XtIntervalId *id
) {

activeWindowClass *awo = (activeWindowClass *) client;
activeGraphicListPtr cur;
int doUpdate, distMode, x0, y0;

  if ( !awo->dimDialog ) {
    awo->dimDialog = new dimDialogClass;
    awo->dimDialog->create( awo );
  }

  if ( awo->viewDims ) {
    awo->dimDialog->popup();
  }
  else {
    awo->dimDialog->popdown();
    return;
  }

  awo->showDimBuf.objTopDist = 0;
  awo->showDimBuf.objBotDist = 0;
  awo->showDimBuf.objLeftDist = 0;
  awo->showDimBuf.objRightDist = 0;

  awo->numRefRects = 0;

  cur = awo->selectedHead->selFlink;
  if ( cur != awo->selectedHead ) { // use first selected object

    awo->numRefRects = 1;
    cur->node->getSelBoxDims( &(awo->refRect[0].x), &(awo->refRect[0].y),
     &(awo->refRect[0].w), &(awo->refRect[0].h) );

  }

  if ( awo->numRefRects == 1 ) {

    awo->showDimBuf.objX = awo->refRect[0].x;
    awo->showDimBuf.objY = awo->refRect[0].y;
    awo->showDimBuf.objW = awo->refRect[0].w;
    awo->showDimBuf.objH = awo->refRect[0].h;

    if ( awo->recordedRefRect ) {

      distMode = awo->dimDialog->getDistMode();

      switch ( distMode ) {
      case 0:
	x0 = awo->refRect[0].x;
	y0 = awo->refRect[0].y;
	break;
      case 1:
	x0 = awo->refRect[0].x + awo->refRect[0].w;
	y0 = awo->refRect[0].y;
	break;
      case 2:
	x0 = awo->refRect[0].x;
	y0 = awo->refRect[0].y + awo->refRect[0].h;
	break;
      case 3:
	x0 = awo->refRect[0].x + awo->refRect[0].w;
	y0 = awo->refRect[0].y + awo->refRect[0].h;
	break;
      default:
	x0 = awo->refRect[0].x;
	y0 = awo->refRect[0].y;
      }

      awo->showDimBuf.objTopDist = awo->refRect[0].y - awo->refRect[1].y;
      awo->showDimBuf.objBotDist = awo->refRect[0].y - awo->refRect[1].y -
       awo->refRect[1].h;
      awo->showDimBuf.objLeftDist = awo->refRect[0].x - awo->refRect[1].x;
      awo->showDimBuf.objRightDist = awo->refRect[0].x - awo->refRect[1].x -
       awo->refRect[1].w;

      awo->showDimBuf.objTopDist = y0 - awo->refRect[1].y;
      awo->showDimBuf.objBotDist = y0 - awo->refRect[1].y - awo->refRect[1].h;
      awo->showDimBuf.objLeftDist = x0 - awo->refRect[1].x;
      awo->showDimBuf.objRightDist = x0 - awo->refRect[1].x - awo->refRect[1].w;

    }

  }
  else {

    awo->showDimBuf.objX = 0;
    awo->showDimBuf.objY = 0;
    awo->showDimBuf.objW = 0;
    awo->showDimBuf.objH = 0;

  }

  if ( ( awo->state != AWC_MOVING_POINT ) &&
       ( awo->state != AWC_MOVING_CREATE_POINT ) &&
       ( awo->state != AWC_CREATING_POINTS ) &&
       ( awo->state != AWC_EDITING_POINTS ) ) {

    awo->showDimBuf.dist = 0;
    awo->showDimBuf.theta = 0;
    awo->showDimBuf.relTheta = 0;

  }

  doUpdate = 0;

  if ( awo->showDimBuf.init || ( awo->showDimBuf.x != awo->showDimBuf.prev_x ) ) {
    awo->showDimBuf.prev_x = awo->showDimBuf.x;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init || ( awo->showDimBuf.y != awo->showDimBuf.prev_y ) ) {
    awo->showDimBuf.prev_y = awo->showDimBuf.y;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init || ( awo->showDimBuf.dist != awo->showDimBuf.prev_dist ) ) {
    awo->showDimBuf.prev_dist = awo->showDimBuf.dist;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init || ( awo->showDimBuf.theta != awo->showDimBuf.prev_theta ) ) {
    awo->showDimBuf.prev_theta = awo->showDimBuf.theta;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init || ( awo->showDimBuf.relTheta != awo->showDimBuf.prev_relTheta ) ) {
    awo->showDimBuf.prev_relTheta = awo->showDimBuf.relTheta;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init || ( awo->showDimBuf.objX != awo->showDimBuf.prev_objX ) ) {
    awo->showDimBuf.prev_objX = awo->showDimBuf.objX;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init || ( awo->showDimBuf.objY != awo->showDimBuf.prev_objY ) ) {
    awo->showDimBuf.prev_objY = awo->showDimBuf.objY;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init || ( awo->showDimBuf.objW != awo->showDimBuf.prev_objW ) ) {
    awo->showDimBuf.prev_objW = awo->showDimBuf.objW;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init || ( awo->showDimBuf.objH != awo->showDimBuf.prev_objH ) ) {
    awo->showDimBuf.prev_objH = awo->showDimBuf.objH;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init ||
       ( awo->showDimBuf.objTopDist != awo->showDimBuf.prev_objTopDist ) ) {
    awo->showDimBuf.prev_objTopDist = awo->showDimBuf.objTopDist;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init ||
       ( awo->showDimBuf.objBotDist != awo->showDimBuf.prev_objBotDist ) ) {
    awo->showDimBuf.prev_objBotDist = awo->showDimBuf.objBotDist;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init ||
       ( awo->showDimBuf.objLeftDist != awo->showDimBuf.prev_objLeftDist ) ) {
    awo->showDimBuf.prev_objLeftDist = awo->showDimBuf.objLeftDist;
    doUpdate = 1;
  }

  if ( awo->showDimBuf.init ||
       ( awo->showDimBuf.objRightDist != awo->showDimBuf.prev_objRightDist ) ) {
    awo->showDimBuf.prev_objRightDist = awo->showDimBuf.objRightDist;
    doUpdate = 1;
  }

  awo->showDimBuf.init = 0;

  if ( doUpdate ) {

    awo->dimDialog->setX( awo->showDimBuf.x );
    awo->dimDialog->setY( awo->showDimBuf.y );
    awo->dimDialog->setLen( awo->showDimBuf.dist );
    awo->dimDialog->setAngle( awo->showDimBuf.theta );
    awo->dimDialog->setRelAngle( awo->showDimBuf.relTheta );
    awo->dimDialog->setObjX( awo->showDimBuf.objX );
    awo->dimDialog->setObjY( awo->showDimBuf.objY );
    awo->dimDialog->setObjW( awo->showDimBuf.objW );
    awo->dimDialog->setObjH( awo->showDimBuf.objH );
    awo->dimDialog->setObjTopDist( awo->showDimBuf.objTopDist );
    awo->dimDialog->setObjBotDist( awo->showDimBuf.objBotDist );
    awo->dimDialog->setObjLeftDist( awo->showDimBuf.objLeftDist );
    awo->dimDialog->setObjRightDist( awo->showDimBuf.objRightDist );

  }

  awo->showDimTimer = appAddTimeOut( awo->appCtx->appContext(),
   250, showObjectDimensions, awo );

}

static void setPointDimensions (
  activeWindowClass *awo,
  int x,
  int y
) {

double hyp1, theta1, hyp2, theta2, adiff;
int xx, yy, ww, hh;

  awo->showDimBuf.x = x;
  awo->showDimBuf.y = y;

  if ( awo->numRefPoints == 2 ) {

    xx = awo->refPoint[0].x - awo->refPoint[1].x;
    yy = awo->refPoint[1].y - awo->refPoint[0].y;
    ww = abs( xx );
    hh = abs( yy );
    hyp1 =
     sqrt( (double) xx * (double) xx +
           (double) yy * (double) yy );
    if ( fabs(hyp1) > 0.001 ) {
      theta1 = asin ( (double) yy / hyp1 ) * 57.29578;
      if ( awo->refPoint[0].x < awo->refPoint[1].x ) theta1 = 180 - theta1;
      if ( theta1 < 0 ) {
        theta1 = 360 + theta1;
      }
    }
    else {
      theta1 = 0;
    }

    xx = x - awo->refPoint[1].x;
    yy = awo->refPoint[1].y - y;
    ww = abs( xx );
    hh = abs( yy );
    hyp2 =
     sqrt( (double) xx * (double) xx +
           (double) yy * (double) yy );
    if ( fabs(hyp2) > 0.001 ) {
      theta2 = asin ( (double) yy / hyp2 ) * 57.29578;
      if ( x < awo->refPoint[1].x ) theta2 = 180 - theta2;
      if ( theta2 < 0 ) {
        theta2 = 360 + theta2;
      }
    }
    else {
      theta2 = 0;
    }

    adiff = theta2 - theta1;
    if ( adiff < 0 ) adiff = 360 + adiff;

    awo->showDimBuf.dist = hyp2;
    awo->showDimBuf.theta = theta2;
    awo->showDimBuf.relTheta = adiff;

  }
  else if ( awo->numRefPoints == 1 ) {

    xx = x - awo->refPoint[1].x;
    yy = awo->refPoint[1].y - y;
    ww = abs( xx );
    hh = abs( yy );
    hyp2 =
     sqrt( (double) xx * (double) xx +
           (double) yy * (double) yy );
    if ( fabs(hyp2) > 0.001 ) {
      theta2 = asin ( (double) yy / hyp2 ) * 57.29578;
      if ( x < awo->refPoint[1].x ) theta2 = 180 - theta2;
      if ( theta2 < 0 ) {
        theta2 = 360 + theta2;
      }
    }
    else {
      theta2 = 0;
    }

    awo->showDimBuf.dist = hyp2;
    awo->showDimBuf.theta = theta2;
    awo->showDimBuf.relTheta = 0.0;

  }

}

void printErrMsg (
  const char *fileName,
  int lineNum,
  const char *msg )
{

  fprintf( stderr, "==============================================================\n" );
  fprintf( stderr, "Internal error in %s at line %-d\nError Message: %s\n\n",
   fileName, lineNum, msg );
  fprintf( stderr, "Please contact the author of this software or else send \n" );
  fprintf( stderr, "the text of this message to tech-talk@aps.anl.gov\n" );
  fprintf( stderr, "==============================================================\n" );

}

static void extractName(
  char *fileName, 
  char *name ) {

int i, l, more;
char *gotOne;

  gotOne = strstr( fileName, "/" );
  if ( !gotOne ) {

    strncpy( name, fileName, 255 );

    // remove extension if .edl (default one)
    l = strlen( name );

    more = 1;
    for ( i=l-1; (i>=0) && more; i-- ) {

      if ( name[i] == '.' ) {

	more = 0;

        if ( l-i >= 4 ) {
          //if ( strcmp( &name[i], ".edl" ) == 0 ) {
          if ( strcmp( &name[i], activeWindowClass::defExt() ) == 0 ) {
	    name[i] = 0;
	  }
	}

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

  // remove extension if .edl (default one)
  l = strlen( name );

    more = 1;
    for ( i=l-1; (i>=0) && more; i-- ) {

      if ( name[i] == '.' ) {

	more = 0;

        if ( l-i >= 3 ) {
          //if ( strcmp( &name[i], ".edl" ) == 0 ) {
          if ( strcmp( &name[i], activeWindowClass::defExt() ) == 0 ) {
	    name[i] = 0;
	  }
	}

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

static void signal_handler (
  int sig
) {

  //fprintf( stderr, "Got signal: sig = %-d\n", sig );
  longjmp( g_jump_h, 1 );

}

static void acw_autosave (
  XtPointer client,
  XtIntervalId *id )
{

activeWindowClass *awo = (activeWindowClass *) client;
//int stat;
//char name[255+1], oldName[255+1];
//char str[31+1], *envPtr;

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

    fprintf( stderr, "Auto-save failed - received exception\n" );
    return;

  }
#endif

  //stat = sys_get_datetime_string( 31, str );
  //fprintf( stderr, "[%s] %s - autosave\n", str, awo->fileName );

  if ( strcmp( awo->startSignature, "edmActiveWindow" ) != 0 ) {
    fprintf( stderr, "Auto-save failed - bad initial signature\n" );
    return;
  }

  if ( strcmp( awo->endSignature, "wodniWevitcAmde" ) != 0 ) {
    fprintf( stderr, "Auto-save failed - bad ending signature\n" );
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

  envPtr = getenv( environment_str8 );
  if ( envPtr ) {
    strncpy( awo->autosaveName, envPtr, 255 );
    if ( envPtr[strlen(envPtr)] != '/' ) {
      Strncat( awo->autosaveName, "/", 255 );
    }
  }
  else {
    strncpy( awo->autosaveName, "/tmp/", 255 );
  }

  Strncat( awo->autosaveName, activeWindowClass_str1, 255 );

  if ( strcmp( awo->fileName, "" ) != 0 ) {
    extractName( awo->fileName, name );
    Strncat( awo->autosaveName, "_", 255 );
    Strncat( awo->autosaveName, name, 255 );
  }

  Strncat( awo->autosaveName, "_XXXXXX", 255 );

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
    awo->restoreTimer = appAddTimeOut( awo->appCtx->appContext(),
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

static void awc_do_save_new_path_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->confirm1.popdown();
  strncpy( awo->fileName, awo->newPath, 255 );
  awo->fileName[255] = 0;
  awo->save( awo->fileName );
  awo->setTitle();
  awo->state = awo->savedState;

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

    XtVaGetValues( awo->appCtx->fileSelectBoxWidgetId(),
     XmNpattern, &xmStr1,
     NULL );

    xmStr2 = NULL;

    n = 0;
    XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

    if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
      xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
      XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
    }

    awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top,
     "screensavefileselect", args, n );

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

static void awc_tedit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

}

static void awc_tedit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;
int num_selected;
activeGraphicListPtr curSel;

  awc_tedit_apply( w, client, call );
  awo->tef.popdown();

  awo->loadTemplate( awo->b2NoneSelectX, awo->b2NoneSelectY,
   awo->fileNameForSym );

  awo->operationComplete();
  awo->deleteTemplateMacros();

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

  awo->clear();
  awo->refresh();

}

static void awc_tedit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->tef.popdown();
  awo->deleteTemplateMacros();
  awo->operationComplete();

}

static int getCurReplaceIndex (
  activeWindowClass *awo
) {

bool more = true;
bool found = false;
int status;

  do {

    if ( awo->sarCurSel != awo->selectedHead ) {

      (awo->curReplaceIndex)++;

      char *str = awo->sarCurSel->node->getSearchString(
       awo->curReplaceIndex );

      if ( !str ) {

        awo->curReplaceIndex = -1;
        awo->sarCurSel = awo->sarCurSel->selFlink;

      }
      else if ( !blank(str) ) {

        //fprintf( stderr, "index = %-d, got [%s]\n", awo->curReplaceIndex, str );
        if ( awo->replaceOld ) {
          strncpy( awo->replaceOld, str, 10000 );
          awo->replaceOld[10000] = 0;
	}

        status = doSearchReplace( awo->sarCaseInsensivite, awo->sarUseRegExpr,
         awo->sar1, awo->sar2, 10000, awo->replaceOld, awo->replaceNew );

        if ( status == 0 ) {
	  found = true;
          more = false;
	}

      }

    }
    else {

      more = false;

    }

  } while ( more );

  if ( found ) {
    return 0;
  }

  return -1;

}

static void awc_editReplace_apply ( // this is used as a skip action
  Widget w,
  XtPointer client,
  XtPointer call ) {

activeWindowClass *awo = (activeWindowClass *) client;

  awo->efReplace.popdown();

  awo->seachStatus = getCurReplaceIndex( awo );

  if ( !awo->seachStatus ) {

    awo->efReplaceW = 300;
    awo->efReplaceH = 300;
    awo->efReplaceLargestH = 300;

    awo->efReplace.create( awo->top, awo->appCtx->ci.getColorMap(),
     &awo->appCtx->entryFormX,
     &awo->appCtx->entryFormY, &awo->efReplaceW, &awo->efReplaceH,
     &awo->efReplaceLargestH, activeWindowClass_str227,
     NULL, NULL, NULL );

    awo->efReplace.addTextField( (char *) activeWindowClass_str225, 45, awo->sar1, 255 );
    //Widget curw = (awo->efReplace.getCurItem())->activeW;
    //XtVaSetValues( curw, XmNsensitive, False, 0, NULL );
    awo->efReplace.addTextField( (char *) activeWindowClass_str226, 45, awo->sar2, 255 );
    //curw = (awo->efReplace.getCurItem())->activeW;
    //XtVaSetValues( curw, XmNsensitive, False, 0, NULL );
    awo->efReplace.addTextField( (char *) activeWindowClass_str228, 45, awo->replaceOld, 255 );
    //curw = (awo->efReplace.getCurItem())->activeW;
    //XtVaSetValues( curw, XmNsensitive, False, 0, NULL );
    awo->efReplace.addTextField( (char *) activeWindowClass_str229, 45, awo->replaceNew, 255 );
    awo->efReplace.finished( awc_editReplace_ok,
     awc_editReplace_apply,
     awc_editReplace_cancel, awo );
    XmString str = XmStringCreateLocalized( activeWindowClass_str231 ); // Skip
    Widget apply = awo->efReplace.getApplyWidget();
    XtVaSetValues( apply, XmNlabelString, str, 0, NULL );
    XmStringFree( str );

    awo->efReplace.popup();

  }
  else {

    awo->operationComplete();
    awo->clear();
    awo->refresh();

  }

}

static void awc_editReplace_ok (
  Widget w,
  XtPointer client,
  XtPointer call ) {

activeWindowClass *awo = (activeWindowClass *) client;

  awo->efReplace.popdown();

  if ( awo->sarCurSel->node ) {
    enableAccumulator();
    doAccSubs( awo->replaceNew, 10000 );
    incAccumulator();
    disableAccumulator();
    //fprintf( stderr, "set index %-d to [%s]\n", awo->curReplaceIndex, awo->replaceNew );
    awo->sarCurSel->node->replaceString( awo->curReplaceIndex, 10000, awo->replaceNew );
    awo->clear();
    awo->refresh();
    awo->setChanged();
  }

  awo->seachStatus = getCurReplaceIndex( awo );

  if ( !awo->seachStatus ) {

    awo->efReplaceW = 300;
    awo->efReplaceH = 300;
    awo->efReplaceLargestH = 300;

    awo->efReplace.create( awo->top, awo->appCtx->ci.getColorMap(),
     &awo->appCtx->entryFormX,
     &awo->appCtx->entryFormY, &awo->efReplaceW, &awo->efReplaceH,
     &awo->efReplaceLargestH, activeWindowClass_str227,
     NULL, NULL, NULL );

    awo->efReplace.addTextField( (char *) activeWindowClass_str225, 45, awo->sar1, 255 );
    //Widget curw = (awo->efReplace.getCurItem())->activeW;
    //XtVaSetValues( curw, XmNsensitive, False, 0, NULL );
    awo->efReplace.addTextField( (char *) activeWindowClass_str226, 45, awo->sar2, 255 );
    //curw = (awo->efReplace.getCurItem())->activeW;
    //XtVaSetValues( curw, XmNsensitive, False, 0, NULL );
    awo->efReplace.addTextField( (char *) activeWindowClass_str228, 45, awo->replaceOld, 255 );
    //curw = (awo->efReplace.getCurItem())->activeW;
    //XtVaSetValues( curw, XmNsensitive, False, 0, NULL );
    awo->efReplace.addTextField( (char *) activeWindowClass_str229, 45, awo->replaceNew, 255 );
    awo->efReplace.finished( awc_editReplace_ok,
     awc_editReplace_apply,
     awc_editReplace_cancel, awo );
    XmString str = XmStringCreateLocalized( activeWindowClass_str231 ); // Skip
    Widget apply = awo->efReplace.getApplyWidget();
    XtVaSetValues( apply, XmNlabelString, str, 0, NULL );
    XmStringFree( str );

    awo->efReplace.popup();

  }
  else {

    awo->operationComplete();
    awo->clear();
    awo->refresh();

  }

}

static void awc_editReplace_cancel (
  Widget w,
  XtPointer client,
  XtPointer call ) {

activeWindowClass *awo = (activeWindowClass *) client;

  awo->efReplace.popdown();

  awo->operationComplete();
  awo->clear();
  awo->refresh();

}

static void awc_editSaR_apply ( // used as replace all action
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->efSaR.popdown();

  awo->curReplaceIndex = -1;
  if ( !awo->replaceOld ) {
    awo->replaceOld = new char[10000+1];
    strcpy( awo->replaceOld, "" );
  }
  if ( !awo->replaceNew ) {
    awo->replaceNew = new char[10000+1];
    strcpy( awo->replaceNew, "" );
  }

  awo->seachStatus = getCurReplaceIndex( awo );

  if ( awo->seachStatus ) { // no match

    awo->efReplaceW = 300;
    awo->efReplaceH = 300;
    awo->efReplaceLargestH = 300;

    awo->efReplace.create( awo->top, awo->appCtx->ci.getColorMap(),
     &awo->appCtx->entryFormX,
     &awo->appCtx->entryFormY, &awo->efReplaceW, &awo->efReplaceH,
     &awo->efReplaceLargestH, activeWindowClass_str227,
     NULL, NULL, NULL );

      awo->efReplace.addLabel( activeWindowClass_str230 );
      awo->efReplace.finished( awc_editReplace_ok, awo );

    awo->efReplace.popup();

  }
  else {

    enableAccumulator();

    do {

      if ( awo->sarCurSel->node ) {
        doAccSubs( awo->replaceNew, 10000 );
        //fprintf( stderr, "set index %-d to [%s]\n", awo->curReplaceIndex, awo->replaceNew );
        awo->sarCurSel->node->replaceString( awo->curReplaceIndex, 10000, awo->replaceNew );
        awo->setChanged();
      }

      awo->seachStatus = getCurReplaceIndex( awo );

      incAccumulator();

    } while ( !awo->seachStatus );

    disableAccumulator();

    awo->clear();
    awo->refresh();
    awo->setChanged();
    awo->operationComplete();

  }

}

static void awc_editSaR_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->efSaR.popdown();

  awo->curReplaceIndex = -1;
  if ( !awo->replaceOld ) {
    awo->replaceOld = new char[10000+1];
    strcpy( awo->replaceOld, "" );
  }
  if ( !awo->replaceNew ) {
    awo->replaceNew = new char[10000+1];
    strcpy( awo->replaceNew, "" );
  }

  awo->seachStatus = getCurReplaceIndex( awo );

  awo->efReplaceW = 300;
  awo->efReplaceH = 300;
  awo->efReplaceLargestH = 300;

  awo->efReplace.create( awo->top, awo->appCtx->ci.getColorMap(),
   &awo->appCtx->entryFormX,
   &awo->appCtx->entryFormY, &awo->efReplaceW, &awo->efReplaceH,
   &awo->efReplaceLargestH, activeWindowClass_str227,
   NULL, NULL, NULL );

  if ( awo->seachStatus ) {
    awo->efReplace.addLabel( activeWindowClass_str230 );
    awo->efReplace.finished( awc_editReplace_ok, awo );
  }
  else {
    awo->efReplace.addTextField( (char *) activeWindowClass_str225, 45, awo->sar1, 255 );
    //Widget curw = (awo->efReplace.getCurItem())->activeW;
    //XtVaSetValues( curw, XmNsensitive, False, 0, NULL );
    awo->efReplace.addTextField( (char *) activeWindowClass_str226, 45, awo->sar2, 255 );
    //curw = (awo->efReplace.getCurItem())->activeW;
    //XtVaSetValues( curw, XmNsensitive, False, 0, NULL );
    awo->efReplace.addTextField( (char *) activeWindowClass_str228, 45, awo->replaceOld, 255 );
    //curw = (awo->efReplace.getCurItem())->activeW;
    //XtVaSetValues( curw, XmNsensitive, False, 0, NULL );
    awo->efReplace.addTextField( (char *) activeWindowClass_str229, 45, awo->replaceNew, 255 );
    awo->efReplace.finished( awc_editReplace_ok,
     awc_editReplace_apply,
     awc_editReplace_cancel, awo );
    XmString str = XmStringCreateLocalized( activeWindowClass_str231 );
    Widget apply = awo->efReplace.getApplyWidget();
    XtVaSetValues( apply, XmNlabelString, str, 0, NULL );
    XmStringFree( str );
  }

  awo->efReplace.popup();

}

static void awc_editSaR_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->efSaR.popdown();
  awo->operationComplete();

}

static void awc_editSetAcc_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->accVal = awo->bufAccVal;
  setAccumulator( awo->accVal );

}

static void awc_editSetAcc_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awc_editSetAcc_apply( w, client, call );
  awo->efSetAcc.popdown();
  awo->operationComplete();

}

static void awc_editSetAcc_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->efSetAcc.popdown();
  awo->operationComplete();

}

static void awc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

int i, n;
Arg args[4];

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

  strcpy( awo->templInfo, awo->bufTemplInfo );

  for ( i=0; i<AWC_MAXTMPLPARAMS; i++ ) {
    strcpy( awo->paramValue[i], awo->bufParamValue[i] );
  }

#ifndef ADD_SCROLLED_WIN
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
#else
  if ( !awo->appCtx->useScrollBars ) {

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

  }
  else {

    awo->reconfig();

  }
#endif

  awo->gridActive = awo->bufGridActive;

  awo->gridShow = awo->bufGridShow;

  if ( awo->oldGridSpacing != awo->gridSpacing ) {

    awo->oldGridSpacing = awo->gridSpacing;

  }

  awo->orthoMove = awo->bufOrthoMove;

  awo->orthogonal = awo->bufOrthogonal;

#ifdef ADD_SCROLLED_WIN
  awo->disableScroll = awo->bufDisableScroll;
#else
  awo->disableScroll = 0;
#endif

  awo->bgPixmapFlag = awo->bufBgPixmapFlag;

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

static void awc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  awo->ef1->popdownNoDestroy();

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

  awo->allSelectedAlignment = awo->defaultFm.currentFontAlignment();
  awo->allSelectedCtlAlignment = awo->defaultCtlFm.currentFontAlignment();
  awo->allSelectedBtnAlignment = awo->defaultBtnFm.currentFontAlignment();

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

static void awc_templateFileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

char *fName;
activeWindowClass *awo = (activeWindowClass *) client;
XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;
int i, stat, num_selected;
activeGraphicListPtr curSel;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    goto done;
  }

  strncpy( awo->fileNameForSym, fName, 255 );
  awo->fileNameForSym[255] = 0;

  awo->numTemplateMacros = 0;
  awo->templateMacros = NULL;
  awo->templateExpansions = NULL;

  stat = awo->getTemplateMacros();
  if ( !( stat & 1 ) ) {
    awo->operationComplete();
    goto done;
  }

  if ( awo->numTemplateMacros > 0 ) {

    awo->tef.create( awo->top, awo->appCtx->ci.getColorMap(),
     &awo->appCtx->entryFormX,
     &awo->appCtx->entryFormY, &awo->appCtx->entryFormW,
     &awo->appCtx->entryFormH, &awo->appCtx->largestH,
     activeWindowClass_str216, NULL, NULL, NULL );

    if ( !(awo->bufTemplInfo) ) {
      awo->bufTemplInfo = new char[AWC_MAXTEMPLINFO+1];
    }
    awo->tef.addReadonlyTextBox( "Info", 32, 10, awo->bufTemplInfo,
     AWC_MAXTEMPLINFO );

    awo->tef.addLabel( " " );
    awo->tef.addSeparator();
    awo->tef.addLabel( " " );

    for ( i=0; i<awo->numTemplateMacros; i++ ) {
      awo->tef.addTextField( awo->templateMacros[i],
       35, awo->templateExpansions[i], AWC_TMPLPARAMSIZE );
    }

    awo->tef.finished( awc_tedit_ok, awc_tedit_apply, awc_tedit_cancel, awo );
    awo->tef.popup();

  }
  else {

    awo->loadTemplate( awo->b2NoneSelectX, awo->b2NoneSelectY,
     awo->fileNameForSym );

    awo->operationComplete();
    awo->deleteTemplateMacros();

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

    awo->clear();
    awo->refresh();

  }

done:

  XtUnmanageChild( w ); // it's ok to unmanage a child  any number of times

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

    XQueryPointer( awo->d, XtWindow(awo->drawWidget), &root, &child,
     &rootX, &rootY, &winX, &winY, &mask );

    awo->confirm1.create( awo->top, "confirm", awo->b2PressXRoot, awo->b2PressYRoot, 2,
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

int stat;
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

  stat = awo->saveScheme( fileName );
  if ( !( stat & 1 ) ) {
    {
    char msg[255+1];
    sprintf( msg, activeWindowClass_str188, fileName );
    awo->appCtx->postMessage( msg );
    }
  }

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

typedef struct nameListTag {
  AVL_FIELDS(nameListTag)
  char *name;
} nameListType, *nameListPtr;

static int compare_nodes (
  void *node1,
  void *node2
) {

nameListPtr p1, p2;

  p1 = (nameListPtr) node1;
  p2 = (nameListPtr) node2;

  return ( strcmp( p1->name, p2->name ) );

}

static int compare_key (
  void *key,
  void *node
) {

nameListPtr p;
char *one;

  p = (nameListPtr) node;
  one = (char *) key;

  return ( strcmp( one, p->name ) );

}

static int copy_node (
  void *node1,
  void *node2
) {

nameListPtr p1, p2;

  p1 = (nameListPtr) node1;
  p2 = (nameListPtr) node2;

  *p1 = *p2;

  return 1;

}

static void awc_pvlistFileSelectOk_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

char *fName, *ptr, tmp[255+1], msg[255+1];
activeWindowClass *awo = (activeWindowClass *) client;
XmFileSelectionBoxCallbackStruct *cbs =
 (XmFileSelectionBoxCallbackStruct *) call;
activeGraphicListPtr cur;
int i, n, nPvs, stat, dup;
ProcessVariable *pvs[1000];
AVL_HANDLE pvNameTree;
nameListPtr curNameNode;
FILE *f;
int avlTreeCreated = 0;
int fileOpened = 0;

  if ( !XmStringGetLtoR( cbs->value, XmFONTLIST_DEFAULT_TAG, &fName ) ) {
    goto done;
  }

  if ( !*fName ) {
    XtFree( fName );
    goto done;
  }

  strncpy( tmp, fName, 255 );

  XtFree( fName );

  stat = avl_init_tree( compare_nodes, compare_key, copy_node,
   &pvNameTree );
  if ( !( stat & 1 ) ) {
    snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
    awo->appCtx->postMessage( msg );
    goto done;
  }

  avlTreeCreated = 1;

  cur = awo->head->blink;
  while ( cur != awo->head ) {

    for ( i=0; i<1000; i++ ) pvs[i] = 0;
    cur->node->getPvs( 1000, pvs, &n );
    for ( i=0; i<n; i++ ) {
      if ( pvs[i] ) {
        curNameNode = (nameListPtr) calloc( sizeof(nameListType), 1 );
        if ( !curNameNode ) {
          snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
          awo->appCtx->postMessage( msg );
          goto done;
        }
        curNameNode->name = (char *) pvs[i]->get_name();
        stat = avl_insert_node( pvNameTree, (void *) curNameNode, &dup );
        if ( !( stat & 1 ) ) {
          snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
          awo->appCtx->postMessage( msg );
          goto done;
        }
        if ( dup ) {
          free( curNameNode );
        }
      }
    }

    cur = cur->blink;

  }

  // fprintf( stderr, "awc_pvlistFileSelectOk_cb, file name = [%s]\n", tmp );

  ptr = strstr( tmp, "." );

  if ( !ptr ) {
    Strncat( tmp, ".pvlist", 255 );
  }

  f = fopen( tmp, "w" );
  if ( !f ) {
    strncpy( msg, activeWindowClass_str199, 255 );
    Strncat( msg, tmp, 255 );
    awo->appCtx->postMessage( msg );
    goto done;
  }

  fileOpened = 1;

  nPvs = 0;
  stat = avl_get_first( pvNameTree, (void **) &curNameNode );
  if ( !( stat & 1 ) ) {
    snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
    awo->appCtx->postMessage( msg );
    goto done;
  }
  while ( curNameNode ) {

    fprintf( f, "%s\n", curNameNode->name );
    nPvs++;

    stat = avl_get_next( pvNameTree, (void **) &curNameNode );
    if ( !( stat & 1 ) ) {
      snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
      awo->appCtx->postMessage( msg );
      goto done;
    }

  }

  fileOpened = 0;

  stat = fclose(f);
  if ( stat < 0 ) {
    strncpy( msg, activeWindowClass_str200, 255 );
    Strncat( msg, tmp, 255 );
    awo->appCtx->postMessage( msg );
    goto done;
  }

  //fprintf( stderr, "Num PVs = %-d\n", nPvs );

done:

  if ( fileOpened ) {

    fileOpened = 0;

    stat = fclose(f);
    if ( stat < 0 ) {
      strncpy( msg, activeWindowClass_str200, 255 );
      Strncat( msg, tmp, 255 );
      awo->appCtx->postMessage( msg );
    }

  }

  XtRemoveCallback( w, XmNcancelCallback,
   awc_fileSelectCancel_cb, (void *) awo );
  XtRemoveCallback( w, XmNokCallback,
   awc_fileSelectOk_cb, (void *) awo );

  awo->operationComplete();

  XtUnmanageChild( w ); // it's ok to unmanage a child again
  XtDestroyWidget( w );

  if ( avlTreeCreated ) {

    // delete tree
    curNameNode = NULL;
    stat = avl_get_first( pvNameTree, (void **) &curNameNode );
    if ( !( stat & 1 ) ) {
      snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
      awo->appCtx->postMessage( msg );
      return;
    }
    while ( curNameNode ) {

      stat = avl_delete_node( pvNameTree, (void **) &curNameNode );
      if ( !( stat & 1 ) ) {
        snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
        awo->appCtx->postMessage( msg );
        return;
      }

      free( curNameNode );
      curNameNode = NULL;

      stat = avl_get_first( pvNameTree, (void **) &curNameNode );
      if ( !( stat & 1 ) ) {
        snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
        awo->appCtx->postMessage( msg );
        return;
      }

    }

  }

}

static void awc_pvlistFileSelectCancel_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo = (activeWindowClass *) client;

  // fprintf( stderr, "awc_pvlistFileSelectCancel_cb\n" );

  XtRemoveCallback( w, XmNcancelCallback,
   awc_fileSelectCancel_cb, (void *) awo );
  XtRemoveCallback( w, XmNokCallback,
   awc_fileSelectOk_cb, (void *) awo );

  awo->operationComplete();

  XtUnmanageChild( w );
  XtDestroyWidget( w );

}

static void awc_pvlistFileSelectKill_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

widgetAndPointerPtr wp = (widgetAndPointerPtr) client;

  // fprintf( stderr, "awc_pvlistFileSelectKill_cb\n" );

  awc_fileSelectCancel_cb( wp->w, wp->client, NULL );

}

static void awc_WMExit_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{
activeWindowClass *awo = (activeWindowClass *) client;

  if ( awo->mode == AWC_EDIT ) {
    awo->doClose = 1;
    awo->waiting = 0;
  }
  else {
    awo->doActiveClose = 1;
    awo->waiting = 0;
  }
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
    //fprintf( stderr, "selectScheme_cb, item = [%s]\n", item );
    strncpy( awo->curSchemeSet, item, 63 );
  }
  else {
    strncpy( awo->curSchemeSet, "", 63 );
    awo->loadComponentScheme( "default" );
    //awo->appCtx->displayScheme.loadDefault( &awo->appCtx->ci );
    //awo->setDisplayScheme( &awo->appCtx->displayScheme );
  }

}

static void b1ReleaseOneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo;
popupBlockPtr block;
long item;

  block = (popupBlockPtr) client;
  item = (long) block->ptr;
  awo = (activeWindowClass *) block->awo;

  switch ( item ) {

    case AWC_POPUP_EDIT_LINE_PROP:
      awo->state = AWC_EDITING;
      awo->currentEf = NULL;
      awo->cursor.set( XtWindow(awo->drawWidget), CURSOR_K_WAIT );
      awo->cursor.setColor( awo->ci->pix(awo->fgColor),
       awo->ci->pix(awo->bgColor) );
      awo->currentPointObject->setEditProperties();
      awo->undoObj.startNewUndoList( activeWindowClass_str189 );
      awo->currentPointObject->doEdit( &(awo->undoObj) );
      break;

    case AWC_POPUP_EDIT_LINE_SEG:
      awo->state = awo->savedState;
      awo->currentPointObject->setEditSegments();
      awo->undoObj.startNewUndoList( activeWindowClass_str189 );
      awo->currentPointObject->doEdit( &(awo->undoObj) );
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
long item;
int i, n, stat;
Arg args[10];
XmString xmStr1, xmStr2;
Atom wm_delete_window;
char *envPtr, text[255+1];

  block = (popupBlockPtr) client;
  item = (long) block->ptr;
  awo = (activeWindowClass *) block->awo;

  switch ( item ) {

    case AWC_POPUP_SHOW_MACROS:

      if ( awo->numMacros < 1 ) {

        snprintf( text, 255, "No Macros have been defined" );
        awo->appCtx->postMessage( text );

      }
      else {

        snprintf( text, 255, "Macros:" );
        awo->appCtx->postMessage( text );

        for ( i=0; i<awo->numMacros; i++ ) {

          snprintf( text, 255, "  %s=%s", awo->macros[i], awo->expansions[i] );
          text[255] = 0;
          awo->appCtx->postMessage( text );

	      }

      }

      snprintf( text, 255, " " );
      awo->appCtx->postMessage( text );

      break;


    case AWC_POPUP_FINDTOP:

      awo->appCtx->findTop();  // raise edm main window
      break;

    case AWC_POPUP_PRINT:

      XRaiseWindow( awo->d, XtWindow(awo->top) );

      processAllEvents( awo->appCtx->appContext(), awo->d );
      
      stat = awo->appCtx->epc.printDialog( awo->appCtx->displayName,
       awo->topWidgetId(),
       awo->appCtx->ci.getColorMap(),
       awo->b2PressXRoot, awo->b2PressYRoot );

      break;

    case AWC_POPUP_DUMP_PVLIST:

      awo->savedState = awo->state;
      awo->state = AWC_WAITING;

      envPtr = getenv( environment_str8 );
      if ( envPtr ) {
        strncpy( text, envPtr, 255 );
        if ( envPtr[strlen(envPtr)] != '/' ) {
          Strncat( text, "/", 255 );
        }
      }
      else {
        strncpy( text, "/tmp/", 255 );
      }

      xmStr1 = XmStringCreateLocalized( text );

      xmStr2 = XmStringCreateLocalized( "*.pvlist" );

      n = 0;
      XtSetArg( args[n], XmNdirectory, xmStr1 ); n++;
      XtSetArg( args[n], XmNpattern, xmStr2 ); n++;

      awo->pvlistFileSelectBox =
       XmCreateFileSelectionDialog( awo->top, "screendumpfileselect",
        args, n );

      XmStringFree( xmStr1 );
      XmStringFree( xmStr2 );

      XtAddCallback( awo->pvlistFileSelectBox, XmNcancelCallback,
       awc_pvlistFileSelectCancel_cb, (void *) awo );
      XtAddCallback( awo->pvlistFileSelectBox, XmNokCallback,
       awc_pvlistFileSelectOk_cb, (void *) awo );

      // -----------------------------------------------------

      awo->pvlistFileSelect.w = awo->pvlistFileSelectBox;
      awo->pvlistFileSelect.client = (void *) awo;

      wm_delete_window = XmInternAtom( XtDisplay(awo->top),
       "WM_DELETE_WINDOW", False );

      XmAddWMProtocolCallback( XtParent(awo->pvlistFileSelectBox),
       wm_delete_window, awc_pvlistFileSelectKill_cb, &awo->pvlistFileSelect );

      XtVaSetValues( XtParent(awo->pvlistFileSelectBox), XmNdeleteResponse,
       XmDO_NOTHING, NULL );

      // -----------------------------------------------------

      XtManageChild( awo->pvlistFileSelectBox );

      XSetWindowColormap( awo->d,
       XtWindow(XtParent(awo->pvlistFileSelectBox)),
       awo->appCtx->ci.getColorMap() );

      break;

    case AWC_POPUP_REFRESH:

      awo->clearActive();
      awo->refreshActive();
      break;

    case AWC_POPUP_OPEN_SELF:

      if ( awo->internalRelatedDisplay ) {
        activeWindowClass *topAwo = awo->actualTopObject();
        if ( topAwo->noEdit ) {
          awo->internalRelatedDisplay->sendMsg( "popupNoEdit" );
        }
        else {
          awo->internalRelatedDisplay->sendMsg( "popup" );
        }
      }
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

      XtVaGetValues( awo->appCtx->fileSelectBoxWidgetId(),
       XmNpattern, &xmStr1,
       NULL );

      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top,
       "screenopenfileselect", args, n );

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

      XtVaGetValues( awo->appCtx->fileSelectBoxWidgetId(),
       XmNpattern, &xmStr1,
       NULL );

      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top,
       "screenopenfileselect", args, n );

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

static void alignLeft (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel;
int deltaX, leftmost, stat;

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

}

static void alignRight (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel;
int deltaX, rightmost, stat;

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

}

static void alignTop (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel;
int deltaY, topmost, stat;

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

}

static void alignBot (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel;
int deltaY, botmost, stat;

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

}

static void alignSizeBoth (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel, topmostNode;
int topmost, width, height, stat;

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

}




static void alignSizeWidth (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel, topmostNode;
int topmost, width, stat;

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

}






static void alignSizeHeight (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel, leftmostNode;
int leftmost, height, stat;

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

}







static void distribVert (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel;
int i, n, minY, maxY, curY0, curY1, curX0, stat;
double space, totalSpace, resid, dY0;

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
        delete[] awo->list_array;
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

}


static void distribMidptVert (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel;
int i, n, minY, maxY, curY0, curY1, curX0, stat;
double space, dY0;

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
        delete[] awo->list_array;
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

}


static void distribHorz (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel;
int i, n, minX, maxX, curY0, curX0, curX1, stat;
double space, totalSpace, resid, dX0;

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
        delete[] awo->list_array;
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

}


static void distribMidptHorz (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel;
int i, n, minX, maxX, curY0, curX0, curX1, stat;
double space, dX0;

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
        delete[] awo->list_array;
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

}

static void distrib2D (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, next, curSel, twoDimHead1, twoDimHead2;
int i, n, incY, curMidY, minY, maxY, topY, bottomY, incX, minX, maxX,
 midX=0, curMidX, leftX, rightX, nCols, nRows, maxRows, listEmpty, stat;

      awo->undoObj.startNewUndoList( activeWindowClass_str174 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      // init 2 lists

      twoDimHead1 = new activeGraphicListType;
      twoDimHead1->flink = twoDimHead1;
      twoDimHead1->blink = twoDimHead1;

      twoDimHead2 = new activeGraphicListType;
      twoDimHead2->flink = twoDimHead2;
      twoDimHead2->blink = twoDimHead2;

      // count nodes as n and find geometrical extent
      n = 0;
      curSel = awo->selectedHead->selFlink;
      minX = curSel->node->getX0();
      maxX = curSel->node->getX1();
      minY = curSel->node->getY0();
      maxY = curSel->node->getY1();
      leftX = curSel->node->getXMid();
      rightX = curSel->node->getXMid();
      topY = curSel->node->getYMid();
      bottomY = curSel->node->getYMid();
      while ( curSel != awo->selectedHead ) {

        if ( curSel->node->getX0() < minX ) {
          minX = curSel->node->getX0();
          leftX = curSel->node->getXMid();
	}
        if ( curSel->node->getX1() > maxX ) {
          maxX = curSel->node->getX1();
          rightX = curSel->node->getXMid();
	}
        if ( curSel->node->getY0() < minY ) {
          minY = curSel->node->getY0();
          topY = curSel->node->getYMid();
	}
        if ( curSel->node->getY1() > maxY ) {
          maxY = curSel->node->getY1();
          bottomY = curSel->node->getYMid();
	}

        n++;

        cur = new activeGraphicListType;
        cur->node = curSel->node;
        // insert at tail
        cur->blink = twoDimHead1->blink;
        twoDimHead1->blink->flink = cur;
        twoDimHead1->blink = cur;
        cur->flink = twoDimHead1;

        curSel = curSel->selFlink;

      }

      // find num rows, cols
      nCols = maxRows = 0;
      cur = twoDimHead1->flink;
      listEmpty = 0;

      while ( !listEmpty ) {

        // find left most
        cur = twoDimHead1->flink;
        if (  cur != twoDimHead1 ) {
          minX = cur->node->getX0();
          midX = cur->node->getXMid();
        }
        cur = cur->flink;
        while ( cur != twoDimHead1 ) {
          if (  cur->node->getX0() <  minX ) {
            minX = cur->node->getX0();
            midX = cur->node->getXMid();
	  }
          cur = cur->flink;
        }

        nCols++;
        nRows = 0;
        // now find all nodes for this column; a node is in this
        // column if [X0,X1] contains midX from above; count the
        // rows and update max rows
        cur = twoDimHead1->flink;
        while (  cur != twoDimHead1 ) {
          next = cur->flink;
          if ( ( cur->node->getX0() <= midX ) &&
               ( cur->node->getX1() >= midX ) ) {
            nRows++;
            // remove node from cur list
            cur->blink->flink = cur->flink;
            cur->flink->blink = cur->blink;
            // insert this node to other list
            cur->blink = twoDimHead2->blink;
            twoDimHead2->blink->flink = cur;
            twoDimHead2->blink = cur;
            cur->flink = twoDimHead2;
	  }
          cur = next;
	}
        if ( nRows > maxRows ) maxRows = nRows;

        // are there more rows on current list?
        listEmpty = ( twoDimHead1->flink == twoDimHead1 );

      }

      if ( nCols > 1 ) {
        incX = ( rightX - leftX ) / ( nCols - 1 );
      }
      else {
        incX = 1;
      }

      if ( maxRows > 1 ) {
        incY = ( bottomY - topY ) / ( maxRows - 1 );
      }
      else {
        incY = 1;
      }

      // if necessary, reallocate work array
      if ( maxRows > awo->list_array_size ) {
        delete[] awo->list_array;
        awo->list_array_size = maxRows;
        awo->list_array = new activeGraphicListType[maxRows];
        awo->list_array->defExeFlink = NULL;
        awo->list_array->defExeBlink = NULL;
      }

      // now, pull out each column, sort and then adjust node position
      // (note that all nodes have been moved to the 2nd list)

      curMidX = leftX;
      nCols = 0;
      cur = twoDimHead2->flink;
      listEmpty = 0;

      while ( !listEmpty ) {

        // find left most
        cur = twoDimHead2->flink;
        if (  cur != twoDimHead2 ) {
          minX = cur->node->getX0();
          midX = cur->node->getXMid();
        }
        cur = cur->flink;
        while ( cur != twoDimHead2 ) {
          if (  cur->node->getX0() <  minX ) {
            minX = cur->node->getX0();
            midX = cur->node->getXMid();
	  }
          cur = cur->flink;
        }

        nRows = 0;
        // now find all nodes for this column; a node is in this
        // column if [X0,X1] contains midX from above;
        cur = twoDimHead2->flink;
        while (  cur != twoDimHead2 ) {

          next = cur->flink;

          if ( ( cur->node->getX0() <= midX ) &&
               ( cur->node->getX1() >= midX ) ) {

            // adjust x postion
            cur->node->moveMidpointAbs( curMidX, cur->node->getYMid() );
            cur->node->moveSelectBoxMidpointAbs(
             curMidX, cur->node->getYMid() );

            awo->list_array[nRows] = *cur;

            // remove node from cur list
            cur->blink->flink = cur->flink;
            cur->flink->blink = cur->blink;
            // insert this node to other list
            cur->blink = twoDimHead1->blink;
            twoDimHead1->blink->flink = cur;
            twoDimHead1->blink = cur;
            cur->flink = twoDimHead1;

            nRows++;

	  }

          cur = next;

	}

        // sort the array and adjust y postion
        qsort( (void *) awo->list_array, nRows,
         sizeof( activeGraphicListType ), qsort_compare_y_func );

        curMidY = topY;
        for ( i=0; i<nRows; i++ ) {
          awo->list_array[i].node->moveMidpointAbs(
           awo->list_array[i].node->getXMid(), curMidY );
          awo->list_array[i].node->moveSelectBoxMidpointAbs(
           awo->list_array[i].node->getXMid(), curMidY );
          curMidY += incY;
	}

        nCols++;
        curMidX += incX;

        // are there more rows on current list?
        listEmpty = ( twoDimHead2->flink == twoDimHead2 );

      }

      // Discard head and list nodes
      // (note that all nodes have been moved back to the 1st list)
      cur = twoDimHead1->flink;
      while (  cur != twoDimHead1 ) {
        next = cur->flink;
        delete cur;
        cur = next;
      }
      delete twoDimHead1;
      delete twoDimHead2;

      // finally, update the display
      awo->clear();
      awo->refresh();

}












static void alignCenter (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel, topmostNode;
int topmost, midX, midY, stat;

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

}

static void alignCenterVert (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel, topmostNode;
int topmost, midX, midY, stat;

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

}

static void alignCenterHorz (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel, leftmostNode;
int leftmost, midX, midY, stat;

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

}



















static void do_selectAll (
  activeWindowClass *awo )
{

activeGraphicListPtr cur, curSel;
int num_selected, wasSelected;

  cur = awo->head->blink;
  while ( cur != awo->head ) {

    if ( !cur->node->hidden ) {

      wasSelected = cur->node->isSelected();
      if ( !wasSelected ) {
        num_selected++;
        cur->node->setSelected();
        //cur->node->drawSelectBoxCorners();
        cur->selBlink = awo->selectedHead->selBlink;
        awo->selectedHead->selBlink->selFlink = cur;
        awo->selectedHead->selBlink = cur;
        cur->selFlink = awo->selectedHead;
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

}

static void do_deselect (
  activeWindowClass *awo )
{

activeGraphicListPtr cur;

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

}

static void do_group (
  activeWindowClass *awo )
{

activeGraphicListPtr cur;
int stat;

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

}

static void do_ungroup (
  activeWindowClass *awo )
{

activeGraphicListPtr curSel, nextSel;
int num_selected;

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

static void delete_items (
  activeWindowClass *awo
) {

activeGraphicListPtr curSel, nextSel;

  // remove nodes and put on cut list; if cur list is not
  // empty then delete all nodes

  // fprintf(stderr, "delete_items\n");
  curSel = awo->selectedHead->selFlink;
  if ( curSel == awo->selectedHead ) return;

  awo->undoObj.flush();

  awo->setChanged();

  // remove nodes off select list
  curSel = awo->selectedHead->selFlink;
  while ( curSel != awo->selectedHead ) {
    nextSel = curSel->selFlink;
    // if on the blink list, remove
    if ( curSel->node->blink() ) {
      awo->ci->removeFromBlinkList( (void *) curSel->node,
       curSel->node->blinkFunction() );
      curSel->node->setNotBlink();
    }

    curSel->node->eraseSelectBoxCorners();
    curSel->node->erase();

    // deselect
    curSel->node->deselect();

    // remove from list
    curSel->blink->flink = curSel->flink;
    curSel->flink->blink = curSel->blink;
    // delete
    delete curSel->node;
    delete curSel;

    curSel = nextSel;

  }

  // make selected list empty
  awo->selectedHead->selFlink = awo->selectedHead;
  awo->selectedHead->selBlink = awo->selectedHead;

  awo->state = AWC_NONE_SELECTED;
  awo->updateMasterSelection();

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

    // if on the blink list, remove
    if ( curSel->node->blink() ) {
      awo->ci->removeFromBlinkList( (void *) curSel->node,
       curSel->node->blinkFunction() );
      curSel->node->setNotBlink();
    }

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
  awo->setChanged();
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

  // Don't allow a paste operation where x or y is off the page
  // but always allow a "paste in place" operation
  if ( item != AWC_POPUP_PASTE_IN_PLACE ) {
    if ( ( x < 0 ) || ( y < 0 ) || ( x >= awo->w ) || ( y >= awo->h ) ) {
      XBell( awo->d, 50 );
      return;
    }
  }

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

  enableAccumulator();

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

  incAccumulator();
  disableAccumulator();

  return;

}

static void b2ReleaseNoneSelect_cb (
   Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo;
popupBlockPtr block;
int stat;
long item;
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
char msg[79+1], text[255+1];

struct {
  activeGraphicListPtr listElement;
  int ySortValue;
  int processed;
} nodeArray[SYMBOL_K_NUM_STATES], tmp;

Atom wm_delete_window;

int efSetAccW = 300;
int efSetAccH = 300;
int efSetAccLargestH = 300;

  block = (popupBlockPtr) client;
  item = (long) block->ptr;
  awo = (activeWindowClass *) block->awo;

  switch ( item ) {

    case AWC_POPUP_SET_PASTE_INDEX:

      awo->savedState = awo->state;

      awo->state = AWC_WAITING;
      awo->currentEf = NULL;

      awo->efSetAcc.create( awo->top, awo->appCtx->ci.getColorMap(),
       &awo->appCtx->entryFormX,
       &awo->appCtx->entryFormY, &efSetAccW, &efSetAccH,
       &efSetAccLargestH, activeWindowClass_str222,
       NULL, NULL, NULL );

      awo->bufAccVal = getAccumulator();
      awo->efSetAcc.addTextField( activeWindowClass_str223, 25, &awo->bufAccVal );
      awo->efSetAcc.finished( awc_editSetAcc_ok, awo );

      awo->efSetAcc.popup();

      break;

    case AWC_POPUP_SELECT_ALL:

      do_selectAll( awo );
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
          if ( n < SYMBOL_K_NUM_STATES-1 ) n++;
            
          cur1 = cur1->flink;

        }

        // set ySortValue for all nodes in same row to common value
        sortValue = 1;
        for ( i=0; i<n; i++ ) {

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

        XQueryPointer( awo->d, XtWindow(awo->drawWidget), &root, &child,
         &rootX, &rootY, &winX, &winY, &mask );

        awo->confirm.create( awo->top, "confirm", awo->b2PressXRoot, awo->b2PressYRoot, 3,
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

    case AWC_POPUP_PRINT:

      XRaiseWindow( awo->d, XtWindow(awo->top) );

      processAllEvents( awo->appCtx->appContext(), awo->d );

      stat = awo->appCtx->epc.printDialog( awo->appCtx->displayName,
       awo->topWidgetId(),
       awo->appCtx->ci.getColorMap(),
       awo->b2PressXRoot, awo->b2PressYRoot );

      break;

    case AWC_POPUP_REFRESH:

      awo->clear();
      awo->refresh();

      break;

    case AWC_POPUP_SHOW_MACROS:

      if ( awo->numMacros < 1 ) {

        snprintf( text, 255, "No Macros have been defined" );
        awo->appCtx->postMessage( text );

      }
      else {

        snprintf( text, 255, "Macros:" );
        awo->appCtx->postMessage( text );

        for ( i=0; i<awo->numMacros; i++ ) {

          snprintf( text, 255, "  %s=%s", awo->macros[i], awo->expansions[i] );
          text[255] = 0;
          awo->appCtx->postMessage( text );

	}

      }

      snprintf( text, 255, " " );
      awo->appCtx->postMessage( text );

      break;

    case AWC_POPUP_FINDTOP:

      awo->appCtx->findTop();  // raise edm main window
      break;

    case AWC_POPUP_OUTLIERS:

      n = 0;
      cur1 = awo->head->flink;
      while ( cur1 != awo->head ) {

        if (
	     ( !cur1->node->hidden ) &&
             ( ( ( cur1->node->getX0()+6 > awo->w ) &&
                 ( cur1->node->getX1() > awo->w ) ) ||
               ( ( cur1->node->getX1()-6 < 0 ) &&
                 ( cur1->node->getX0() < 0 ) ) ||
               ( ( cur1->node->getY0()+6 > awo->h ) &&
                 ( cur1->node->getY1() > awo->h ) ) ||
               ( ( cur1->node->getY1()-6 < 0 ) &&
                 ( cur1->node->getY0() < 0 ) ) )
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
          awo->undoObj.startNewUndoList( activeWindowClass_str189 );
          cur1->node->doEdit( &(awo->undoObj) );

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

      awo->schemeSelectBox = XmCreateFileSelectionDialog( awo->top,
       "screenloadschemefileselect", args, n );

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

      awo->schemeSelectBox = XmCreateFileSelectionDialog( awo->top,
       "screensaveschemefileselect", args, n );

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

      XtVaGetValues( awo->appCtx->fileSelectBoxWidgetId(),
       XmNpattern, &xmStr1,
       NULL );

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      xmStr2 = NULL;
      if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top,
       "screensavefileselect", args, n );

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

        XtVaGetValues( awo->appCtx->fileSelectBoxWidgetId(),
         XmNpattern, &xmStr1,
         NULL );

        xmStr2 = NULL;

        n = 0;
        XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

        if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
          xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
          XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
        }

        awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top,
         "screensavefileselect", args, n );

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

    case AWC_POPUP_SAVE_TO_PATH:

      {
        char name[255+1], saveMsg[255+1];
        awo->savedState = awo->state;
        awo->state = AWC_WAITING;
        extractName( awo->fileName, name );
	strncpy( awo->newPath, awo->appCtx->curPath, 255 );
        awo->newPath[255] = 0;
        Strncat( awo->newPath, name, 255 );
        //Strncat( awo->newPath, ".edl", 255 );
        Strncat( awo->newPath, activeWindowClass::defExt(), 255 );
        strcpy( saveMsg, activeWindowClass_str197 );
        Strncat( saveMsg, awo->newPath, 255 );
        Strncat( saveMsg, "?", 255 );
        awo->confirm1.create( awo->top, "confirm", awo->b2PressXRoot, awo->b2PressYRoot,
         2, saveMsg, NULL, NULL );
        awo->confirm1.addButton( activeWindowClass_str5, awc_dont_save_cb,
         (void *) awo );
        awo->confirm1.addButton( activeWindowClass_str3,
         awc_do_save_new_path_cb, (void *) awo );
        awo->confirm1.finished();
        awo->confirm1.popup();
      }

      break;

    case AWC_POPUP_INSERT_TEMPLATE:

      awo->savedState = awo->state;
      awo->state = AWC_WAITING;

      if ( !awo->templateFileSelectBox ) {

        xmStr1 = xmStr2 = NULL;

        n = 0;
        XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

        if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
          xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
          XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
        }

        awo->templateFileSelectBox = XmCreateFileSelectionDialog( awo->top,
         "templateopenfileselect", args, n );

        XmStringFree( xmStr1 );
        if ( xmStr2 ) XmStringFree( xmStr2 );

        XtAddCallback( awo->templateFileSelectBox, XmNcancelCallback,
         awc_fileSelectCancel_cb, (void *) awo );
        XtAddCallback( awo->templateFileSelectBox, XmNokCallback,
         awc_templateFileSelectOk_cb, (void *) awo );

        wm_delete_window = XmInternAtom( XtDisplay(awo->top),
         "WM_DELETE_WINDOW", False );

        XmAddWMProtocolCallback( XtParent(awo->templateFileSelectBox),
         wm_delete_window, awc_fileSelectKill_cb, &awo->wpFileSelect );

        XtVaSetValues( XtParent(awo->templateFileSelectBox), XmNdeleteResponse,
         XmDO_NOTHING, NULL );

        XSetWindowColormap( awo->d,
         XtWindow(XtParent(awo->templateFileSelectBox)),
         awo->appCtx->ci.getColorMap() );

      }

      awo->wpFileSelect.w = awo->templateFileSelectBox;
      awo->wpFileSelect.client = (void *) awo;

      XtManageChild( awo->templateFileSelectBox );

      break;

    case AWC_POPUP_OPEN:

      awo->savedState = awo->state;
      awo->state = AWC_WAITING;

      XtVaGetValues( awo->appCtx->fileSelectBoxWidgetId(),
       XmNpattern, &xmStr1,
       NULL );

      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top,
       "screenopenfileselect", args, n );

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

      XtVaGetValues( awo->appCtx->fileSelectBoxWidgetId(),
       XmNpattern, &xmStr1,
       NULL );

      xmStr2 = NULL;

      n = 0;
      XtSetArg( args[n], XmNpattern, xmStr1 ); n++;

      if ( strcmp( awo->appCtx->curPath, "" ) != 0 ) {
        xmStr2 = XmStringCreateLocalized( awo->appCtx->curPath );
        XtSetArg( args[n], XmNdirectory, xmStr2 ); n++;
      }

      awo->fileSelectBox = XmCreateFileSelectionDialog( awo->top,
       "screenopenfileselect", args, n );

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
#ifdef ADD_SCROLLED_WIN
      awo->bufDisableScroll = awo->disableScroll;
#endif
      awo->bufGridSpacing = awo->gridSpacing;
      awo->bufGridActive = awo->gridActive;
      awo->bufGridShow = awo->gridShow;
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
      awo->bufBgPixmapFlag = awo->bgPixmapFlag;

      awo->ef.create( awo->top, awo->appCtx->ci.getColorMap(),
       &awo->appCtx->entryFormX,
       &awo->appCtx->entryFormY, &awo->appCtx->entryFormW,
       &awo->appCtx->entryFormH, &awo->appCtx->largestH,
       activeWindowClass_str17, NULL, NULL, NULL );

      awo->ef.addTextField( activeWindowClass_str18, 35, awo->bufId, 31 );

      awo->ef.addTextField( activeWindowClass_str19, 35, &awo->bufX );
      awo->ef.addTextField( activeWindowClass_str20, 35, &awo->bufY );
      awo->ef.addTextField( activeWindowClass_str21, 35, &awo->bufW );
      awo->ef.addTextField( activeWindowClass_str22, 35, &awo->bufH );

      awo->ef.addTextField( activeWindowClass_str23, 35, awo->bufTitle, 127 );

      awo->ef.addTextField( activeWindowClass_str24, 35,
       awo->bufDefaultPvType, 15 );

      awo->ef.addColorButton( activeWindowClass_str25, awo->ci, &awo->fgCb,
       &awo->bufFgColor );
      awo->ef.addColorButton( activeWindowClass_str26, awo->ci, &awo->bgCb,
       &awo->bufBgColor );
      awo->ef.addToggle( activeWindowClass_str27, &awo->bufGridShow );
      awo->ef.addToggle( activeWindowClass_str29, &awo->bufGridActive );
      awo->ef.addTextField( activeWindowClass_str30, 35,
       &awo->bufGridSpacing );
      awo->ef.addToggle( activeWindowClass_str168, &awo->bufOrthoMove );
      awo->ef.addToggle( activeWindowClass_str31, &awo->bufOrthogonal );
#ifdef ADD_SCROLLED_WIN
      if ( awo->appCtx->useScrollBars ) {
        awo->ef.addToggle( activeWindowClass_str203, &awo->bufDisableScroll );
      }
#endif
      awo->ef.addOption( activeWindowClass_str212, activeWindowClass_str213,
       &awo->bufBgPixmapFlag );

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

      awo->ef.addEmbeddedEf(activeWindowClass_str218 , "...", &awo->ef1 );

      //-----------------------------------------------------------------------

      awo->ef1->create( awo->top, awo->appCtx->ci.getColorMap(),
       &awo->appCtx->entryFormX,
       &awo->appCtx->entryFormY, &awo->appCtx->entryFormW,
       &awo->appCtx->entryFormH, &awo->appCtx->largestH,
       activeWindowClass_str216, NULL, NULL, NULL );

      if ( !(awo->bufTemplInfo) ) {
        awo->bufTemplInfo = new char[AWC_MAXTEMPLINFO+1];
      }
      strcpy( awo->bufTemplInfo, awo->templInfo );
      awo->ef1->addTextBox( "Info", 32, 10, awo->bufTemplInfo,
       AWC_MAXTEMPLINFO );

      awo->ef1->addLabel( " " );
      awo->ef1->addSeparator();
      awo->ef1->addLabel( " " );

      for ( i=0; i<AWC_MAXTMPLPARAMS; i++ ) {
        //awo->ef1->beginLeftSubForm();
        char paramName[15+1];
        snprintf( paramName, 15, "    %s%-d", activeWindowClass_str219, i+1 );
        strcpy( awo->bufParamValue[i], awo->paramValue[i] );
        awo->ef1->addTextField( paramName, 35, awo->bufParamValue[i],
         AWC_MAXTMPLPARAMS );
        //awo->ef1->endSubForm();
      }

      awo->ef1->finished( awc_edit_ok1, awo );

      //-----------------------------------------------------------------------

      awo->ef.finished( awc_edit_ok, awc_edit_apply, awc_edit_cancel, awo );
      awo->ef.popup();

      break;

    case AWC_POPUP_TOGGLE_VIEW_DIMS:

      if ( !awo->dimDialog ) {
        awo->dimDialog = new dimDialogClass;
        awo->dimDialog->create( awo );
      }

      if ( awo->viewDims ) {
        awo->viewDims = 0;
        XtRemoveTimeOut( awo->showDimTimer );
        awo->showDimTimer = 0;
        awo->dimDialog->popdown();
      }
      else {
        awo->viewDims = 1;
	awo->dimDialog->popup();
        awo->showDimBuf.init = 1;
        awo->showDimBuf.x = 0;
        awo->showDimBuf.y = 0;
        awo->showDimBuf.dist = 0;
        awo->showDimBuf.theta = 0;
        awo->showDimBuf.relTheta = 0;
        awo->showDimBuf.objX = 0;
        awo->showDimBuf.objY = 0;
        awo->showDimBuf.objW = 0;
        awo->showDimBuf.objH = 0;
        awo->showDimBuf.objTopDist = 0;
        awo->showDimBuf.objBotDist = 0;
        awo->showDimBuf.objLeftDist = 0;
        awo->showDimBuf.objRightDist = 0;
        awo->showDimTimer = appAddTimeOut( awo->appCtx->appContext(),
         250, showObjectDimensions, awo );
      }

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
int stat;
long item;
activeGraphicListPtr curSel;

  block = (popupBlockPtr) client;
  item = (long) block->ptr;
  awo = (activeWindowClass *) block->awo;

  switch ( item ) {

    case AWC_POPUP_COPY_GROUP_INFO:

      curSel = awo->selectedHead->selFlink;
      if ( curSel != awo->selectedHead ) {

        stat = curSel->node->getGroupVisInfo(
         &awo->appCtx->curGroupVisPvExpStr,
         &awo->appCtx->curGroupVisInverted, 40,
         awo->appCtx->curGroupMinVisString,
         awo->appCtx->curGroupMaxVisString );
	if ( stat & 1 ) {
          awo->appCtx->haveGroupVisInfo = 1;
	  //fprintf( stderr, "pv = [%s]\n", awo->appCtx->curGroupVisPvExpStr.getRaw() );
	  //fprintf( stderr, "inv = %-d\n", awo->appCtx->curGroupVisInverted );
	  //fprintf( stderr, "min = [%s]\n", awo->appCtx->curGroupMinVisString );
	  //fprintf( stderr, "max = [%s]\n", awo->appCtx->curGroupMaxVisString );
	}
	else {
	  XBell( awo->d, 50 );
	}

      }
      else {

        printErrMsg( __FILE__, __LINE__, "Inconsistent select state" );

      }

      break;

    case AWC_POPUP_PASTE_GROUP_INFO:

      curSel = awo->selectedHead->selFlink;
      if ( curSel != awo->selectedHead ) {

        if ( awo->appCtx->haveGroupVisInfo ) {

          stat = curSel->node->putGroupVisInfo(
           &awo->appCtx->curGroupVisPvExpStr,
           awo->appCtx->curGroupVisInverted, 40,
           awo->appCtx->curGroupMinVisString,
           awo->appCtx->curGroupMaxVisString );
	  if ( !( stat & 1 ) ) {
	    XBell( awo->d, 50 );
	  }

	}
	else {

          XBell( awo->d, 50 );

        }

      }
      else {

        printErrMsg( __FILE__, __LINE__, "Inconsistent select state" );

      }

      break;

    case AWC_POPUP_RECORD_DIMS:

      curSel = awo->selectedHead->selFlink;
      if ( curSel != awo->selectedHead ) {
        awo->recordedRefRect = 1;
        curSel->node->getSelBoxDims( &(awo->refRect[1].x),
         &(awo->refRect[1].y), &(awo->refRect[1].w), &(awo->refRect[1].h) );
      }

      break;

    case AWC_POPUP_DESELECT:

      do_deselect( awo );
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
        //fprintf( stderr, "Klingons decloaking! (1)\n" );

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
        //fprintf( stderr, "Klingons decloaking! (2)\n" );

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

      do_group( awo );
      break;

    case AWC_POPUP_UNGROUP:

      do_ungroup( awo );
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
int stat;
long item;
XmString str;
Widget apply;

  block = (popupBlockPtr) client;
  item = (long) block->ptr;
  awo = (activeWindowClass *) block->awo;

  switch ( item ) {

    case AWC_POPUP_SAR:

      awo->savedState = awo->state;

      awo->state = AWC_WAITING;
      awo->currentEf = NULL;

      awo->sarCurSel = awo->selectedHead->selFlink;

      awo->efSaRW = 300;
      awo->efSaRH = 300;
      awo->efSaRLargestH = 300;

      awo->efSaR.create( awo->top, awo->appCtx->ci.getColorMap(),
       &awo->appCtx->entryFormX,
       &awo->appCtx->entryFormY, &awo->efSaRW, &awo->efSaRH,
       &awo->efSaRLargestH, activeWindowClass_str224,
       NULL, NULL, NULL );

      if ( !awo->sar1 ) {
        awo->sar1 = new char[255+1];
        strcpy( awo->sar1, "" );
      }
      if ( !awo->sar2 ) {
        awo->sar2 = new char[255+1];
        strcpy( awo->sar2, "" );
      }
      awo->efSaR.addTextField( (char *) activeWindowClass_str225, 45, awo->sar1, 255 );
      awo->efSaR.addTextField( (char *) activeWindowClass_str226, 45, awo->sar2, 255 );
      awo->efSaR.addToggle( (char *) activeWindowClass_str233, &awo->sarCaseInsensivite );
      awo->efSaR.addToggle( (char *) activeWindowClass_str234, &awo->sarUseRegExpr );
      awo->efSaR.finished( awc_editSaR_ok, awc_editSaR_apply, awc_editSaR_cancel, awo );
      str = XmStringCreateLocalized( activeWindowClass_str232 ); // All
      apply = awo->efSaR.getApplyWidget();
      XtVaSetValues( apply, XmNlabelString, str, 0, NULL );
      XmStringFree( str );

      awo->efSaR.popup();

      break;

    case AWC_POPUP_DESELECT:

      do_deselect( awo );
      break;

    case AWC_POPUP_GROUP:

      do_group( awo );
      break;

    case AWC_POPUP_UNGROUP:

      do_ungroup( awo );
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

      alignLeft( awo );
      break;

    case AWC_POPUP_ALIGN_RIGHT:

      alignRight( awo );
      break;

    case AWC_POPUP_ALIGN_TOP:

      alignTop( awo );
      break;

    case AWC_POPUP_ALIGN_BOTTOM:

      alignBot( awo );
      break;

    case AWC_POPUP_ALIGN_CENTER:

      alignCenter( awo );
      break;

    case AWC_POPUP_ALIGN_CENTER_VERT:

      alignCenterVert( awo );
      break;

    case AWC_POPUP_ALIGN_CENTER_HORZ:

      alignCenterHorz( awo );
      break;

      break;


    case AWC_POPUP_ALIGN_SIZE:

      alignSizeBoth( awo );
      break;

    case AWC_POPUP_ALIGN_SIZE_HORZ:

      alignSizeWidth( awo );
      break;

    case AWC_POPUP_ALIGN_SIZE_VERT:

      alignSizeHeight( awo );
      break;

    case AWC_POPUP_DISTRIBUTE_VERTICALLY:

      distribVert( awo );
      break;

#if 0
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
        delete[] awo->list_array;
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
#endif

    case AWC_POPUP_DISTRIBUTE_MIDPT_VERTICALLY:

      distribMidptVert( awo );
      break;

#if 0
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
        delete[] awo->list_array;
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
#endif

    case AWC_POPUP_DISTRIBUTE_HORIZONTALLY:

      distribHorz( awo );
      break;

#if 0
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
        delete[] awo->list_array;
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
#endif

    case AWC_POPUP_DISTRIBUTE_MIDPT_HORIZONTALLY:

      distribMidptHorz( awo );
      break;

#if 0
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
        delete[] awo->list_array;
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
#endif

    case AWC_POPUP_DISTRIBUTE_MIDPT_BOTH:

      distrib2D( awo );
      break;

#if 0
      awo->undoObj.startNewUndoList( activeWindowClass_str174 );
      cur = awo->selectedHead->selFlink;
      while ( cur != awo->selectedHead ) {
        stat = cur->node->addUndoMoveNode( &(awo->undoObj) );
        cur = cur->selFlink;
      }

      awo->setChanged();

      // init 2 lists

      twoDimHead1 = new activeGraphicListType;
      twoDimHead1->flink = twoDimHead1;
      twoDimHead1->blink = twoDimHead1;

      twoDimHead2 = new activeGraphicListType;
      twoDimHead2->flink = twoDimHead2;
      twoDimHead2->blink = twoDimHead2;

      // count nodes as n and find geometrical extent
      n = 0;
      curSel = awo->selectedHead->selFlink;
      minX = curSel->node->getX0();
      maxX = curSel->node->getX1();
      minY = curSel->node->getY0();
      maxY = curSel->node->getY1();
      leftX = curSel->node->getXMid();
      rightX = curSel->node->getXMid();
      topY = curSel->node->getYMid();
      bottomY = curSel->node->getYMid();
      while ( curSel != awo->selectedHead ) {

        if ( curSel->node->getX0() < minX ) {
          minX = curSel->node->getX0();
          leftX = curSel->node->getXMid();
	}
        if ( curSel->node->getX1() > maxX ) {
          maxX = curSel->node->getX1();
          rightX = curSel->node->getXMid();
	}
        if ( curSel->node->getY0() < minY ) {
          minY = curSel->node->getY0();
          topY = curSel->node->getYMid();
	}
        if ( curSel->node->getY1() > maxY ) {
          maxY = curSel->node->getY1();
          bottomY = curSel->node->getYMid();
	}

        n++;

        cur = new activeGraphicListType;
        cur->node = curSel->node;
        // insert at tail
        cur->blink = twoDimHead1->blink;
        twoDimHead1->blink->flink = cur;
        twoDimHead1->blink = cur;
        cur->flink = twoDimHead1;

        curSel = curSel->selFlink;

      }

      // find num rows, cols
      nCols = maxRows = 0;
      cur = twoDimHead1->flink;
      listEmpty = 0;

      while ( !listEmpty ) {

        // find left most
        cur = twoDimHead1->flink;
        if (  cur != twoDimHead1 ) {
          minX = cur->node->getX0();
          midX = cur->node->getXMid();
        }
        cur = cur->flink;
        while ( cur != twoDimHead1 ) {
          if (  cur->node->getX0() <  minX ) {
            minX = cur->node->getX0();
            midX = cur->node->getXMid();
	  }
          cur = cur->flink;
        }

        nCols++;
        nRows = 0;
        // now find all nodes for this column; a node is in this
        // column if [X0,X1] contains midX from above; count the
        // rows and update max rows
        cur = twoDimHead1->flink;
        while (  cur != twoDimHead1 ) {
          next = cur->flink;
          if ( ( cur->node->getX0() <= midX ) &&
               ( cur->node->getX1() >= midX ) ) {
            nRows++;
            // remove node from cur list
            cur->blink->flink = cur->flink;
            cur->flink->blink = cur->blink;
            // insert this node to other list
            cur->blink = twoDimHead2->blink;
            twoDimHead2->blink->flink = cur;
            twoDimHead2->blink = cur;
            cur->flink = twoDimHead2;
	  }
          cur = next;
	}
        if ( nRows > maxRows ) maxRows = nRows;

        // are there more rows on current list?
        listEmpty = ( twoDimHead1->flink == twoDimHead1 );

      }

      if ( nCols > 1 ) {
        incX = ( rightX - leftX ) / ( nCols - 1 );
      }
      else {
        incX = 1;
      }

      if ( maxRows > 1 ) {
        incY = ( bottomY - topY ) / ( maxRows - 1 );
      }
      else {
        incY = 1;
      }

      // if necessary, reallocate work array
      if ( maxRows > awo->list_array_size ) {
        delete[] awo->list_array;
        awo->list_array_size = maxRows;
        awo->list_array = new activeGraphicListType[maxRows];
        awo->list_array->defExeFlink = NULL;
        awo->list_array->defExeBlink = NULL;
      }

      // now, pull out each column, sort and then adjust node position
      // (note that all nodes have been moved to the 2nd list)

      curMidX = leftX;
      nCols = 0;
      cur = twoDimHead2->flink;
      listEmpty = 0;

      while ( !listEmpty ) {

        // find left most
        cur = twoDimHead2->flink;
        if (  cur != twoDimHead2 ) {
          minX = cur->node->getX0();
          midX = cur->node->getXMid();
        }
        cur = cur->flink;
        while ( cur != twoDimHead2 ) {
          if (  cur->node->getX0() <  minX ) {
            minX = cur->node->getX0();
            midX = cur->node->getXMid();
	  }
          cur = cur->flink;
        }

        nRows = 0;
        // now find all nodes for this column; a node is in this
        // column if [X0,X1] contains midX from above;
        cur = twoDimHead2->flink;
        while (  cur != twoDimHead2 ) {

          next = cur->flink;

          if ( ( cur->node->getX0() <= midX ) &&
               ( cur->node->getX1() >= midX ) ) {

            // adjust x postion
            cur->node->moveMidpointAbs( curMidX, cur->node->getYMid() );
            cur->node->moveSelectBoxMidpointAbs(
             curMidX, cur->node->getYMid() );

            awo->list_array[nRows] = *cur;

            // remove node from cur list
            cur->blink->flink = cur->flink;
            cur->flink->blink = cur->blink;
            // insert this node to other list
            cur->blink = twoDimHead1->blink;
            twoDimHead1->blink->flink = cur;
            twoDimHead1->blink = cur;
            cur->flink = twoDimHead1;

            nRows++;

	  }

          cur = next;

	}

        // sort the array and adjust y postion
        qsort( (void *) awo->list_array, nRows,
         sizeof( activeGraphicListType ), qsort_compare_y_func );

        curMidY = topY;
        for ( i=0; i<nRows; i++ ) {
          awo->list_array[i].node->moveMidpointAbs(
           awo->list_array[i].node->getXMid(), curMidY );
          awo->list_array[i].node->moveSelectBoxMidpointAbs(
           awo->list_array[i].node->getXMid(), curMidY );
          curMidY += incY;
	}

        nCols++;
        curMidX += incX;

        // are there more rows on current list?
        listEmpty = ( twoDimHead2->flink == twoDimHead2 );

      }

      // Discard head and list nodes
      // (note that all nodes have been moved back to the 1st list)
      cur = twoDimHead1->flink;
      while (  cur != twoDimHead1 ) {
        next = cur->flink;
        delete cur;
        cur = next;
      }
      delete twoDimHead1;
      delete twoDimHead2;

      // finally, update the display
      awo->clear();
      awo->refresh();

      break;





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
#endif

    case AWC_POPUP_CHANGE_DSP_PARAMS:

      awo->savedState = awo->state;
      
      awo->state = AWC_EDITING;
      awo->currentEf = NULL;

      awo->ef.create( awo->top, awo->appCtx->ci.getColorMap(),
       &awo->appCtx->entryFormX,
       &awo->appCtx->entryFormY, &awo->appCtx->entryFormW,
       &awo->appCtx->entryFormH, &awo->appCtx->largestH,
       activeWindowClass_str44, NULL, NULL, NULL );

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

      awo->ef.addTextField( activeWindowClass_str70, 35, awo->allSelectedCtlPvName[0], PV_Factory::MAX_PV_NAME );
      awo->ef.addToggle( activeWindowClass_str71, &awo->allSelectedCtlPvNameFlag );

      awo->ef.addTextField( activeWindowClass_str72, 35, awo->allSelectedReadbackPvName[0],
       PV_Factory::MAX_PV_NAME );
      awo->ef.addToggle( activeWindowClass_str73, &awo->allSelectedReadbackPvNameFlag );

      awo->ef.addTextField( activeWindowClass_str74, 35, awo->allSelectedNullPvName[0], PV_Factory::MAX_PV_NAME );
      awo->ef.addToggle( activeWindowClass_str75, &awo->allSelectedNullPvNameFlag );

      awo->ef.addTextField( activeWindowClass_str76, 35, awo->allSelectedVisPvName[0],
       PV_Factory::MAX_PV_NAME );
      awo->ef.addToggle( activeWindowClass_str77, &awo->allSelectedVisPvNameFlag );

      awo->ef.addTextField( activeWindowClass_str78, 35, awo->allSelectedAlarmPvName[0], PV_Factory::MAX_PV_NAME );
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
static void action_cb (
   Widget w,
  XtPointer client,
  XtPointer call )
{

activeWindowClass *awo;
popupBlockPtr block;
long item;

  block = (popupBlockPtr) client;
  item = (long) block->ptr;
  awo = (activeWindowClass *) block->awo;

  awo->pvAction->executeAction( item );

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
        //fprintf( stderr, "load new scheme [%s]\n", schemeFile );
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

  awo = (activeWindowClass *) client;

  *continueToDispatch = True;

  if ( e->type == MapNotify ) {

    if ( awo->mode == AWC_EDIT ) {
      awo->refresh();
    }
    else {
      awo->refreshActive();
    }

  }
  else if ( e->type == ConfigureNotify ) {

    //if ( !(awo->isEmbedded) ) fprintf( stderr, "ConfigureNotify - [%s]\n",
    //  awo->fileNameForSym );

    ce = (XConfigureEvent *) e;

    if ( !ce->send_event ) {

#ifdef ADD_SCROLLED_WIN
      // This is a special case - scroll bars are in use but they are
      // disabled for this screen
      if ( awo->appCtx->useScrollBars ) {

        if ( awo->disableScroll ) {

          XtVaSetValues(awo->drawWidget,
           XmNwidth, (Dimension)ce->width,
           XmNheight, (Dimension)ce->height,
	   NULL);

          if ( awo->scroll ) {
            XtVaSetValues(awo->scroll,
             XmNwidth, (Dimension)ce->width,
             XmNheight, (Dimension)ce->height,
             NULL);
          }

          XtVaSetValues(awo->top,
           XmNwidth, (Dimension)ce->width,
           XmNheight, (Dimension)ce->height,
           NULL);

        }

      }
#endif

      return;

    }

    if ( ( awo->w != ce->width ) ||
         ( awo->h != ce->height ) ) {

      //printf( "  resize\n" );

#ifdef ADD_SCROLLED_WIN
      if ( awo->appCtx->useScrollBars && !awo->disableScroll ) {
        if (ce->width > awo->w ) {
          awo->w = ce->width;
        }
      }
      else {
        awo->w = ce->width;
      }

      if ( awo->appCtx->useScrollBars && !awo->disableScroll ) {
        if ( ce->height > awo->h ) {
          awo->h = ce->height;
        }
      }
      else {
        awo->h = ce->height;
      }

      if ( awo->appCtx->useScrollBars && !awo->disableScroll ) {

        XtVaSetValues(awo->drawWidget,
         XmNwidth, (Dimension)awo->w,
         XmNheight, (Dimension)awo->h,
         NULL);

      }
      else {

        if ( awo->scroll ) {
          XtVaSetValues(awo->scroll,
           XmNwidth, (Dimension)awo->w,
           XmNheight, (Dimension)awo->h,
           NULL);
	}

        XtVaSetValues(awo->drawWidget,
         XmNwidth, (Dimension)awo->w,
         XmNheight, (Dimension)awo->h,
         NULL);

      }

#else
        awo->w = ce->width;
        awo->h = ce->height;
#endif

    }

    if ( ( awo->x != ce->x ) ||
         ( awo->y != ce->y ) ) {

      //printf( "  move\n" );

      awo->x = ce->x;
      awo->y = ce->y;

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

Boolean  nothingDone = False;

  awo = (activeWindowClass *) client;

  *continueToDispatch = False;

  if ( awo->mode != AWC_EDIT ) return;

  awo->oldState = awo->state;

  if ( e->type != MotionNotify ) {
    if ( !awo->appCtx->viewXy || ( e->type != Expose ) ) {
      if ( awo->msgDialogPoppedUp ) {
        awo->msgDialog.popdown();
        awo->msgDialogPoppedUp = 0;
      }
    }
  }

  if ( e->type == MapNotify ) {

    awo->refresh();

  }
  else if ( e->type == ConfigureNotify ) {

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
  else if ( e->type == KeyRelease ) {

    ke = (XKeyEvent *) e;

    charCount = XLookupString( ke, keyBuf, keyBufSize, &key, &compose );

    if ( ( key == XK_Control_L ) || ( key == XK_Control_R ) ) {
      awo->ctlKeyPressed = 0;
    }

  }
  else if ( e->type == KeyPress ) {

    // The following supports wheel mice on older versions
    // of the Exceed X server which can't map wheel events to the
    // (standard) Button4/5 but only to Key events.
    if ( ((XKeyEvent*)e)->state &
	 ( Mod1Mask | Mod3Mask | Mod4Mask | Mod5Mask ) ) {
      *continueToDispatch = True;
      return;
    }

    if ( awo->state == AWC_START_DEFINE_REGION ) goto done;
    if ( awo->state == AWC_DEFINE_REGION ) goto done;
    if ( awo->state == AWC_EDITING ) goto done;
    if ( awo->state == AWC_START_DEFINE_SELECT_REGION ) goto done;
    if ( awo->state == AWC_DEFINE_SELECT_REGION ) goto done;
    if ( awo->state == AWC_EDITING_POINTS ) goto done;
    if ( awo->state == AWC_CREATING_POINTS ) goto done;
    //if ( awo->state == AWC_MOVING_POINT ) goto done;
    //if ( awo->state == AWC_MOVING_CREATE_POINT ) goto done;
    if ( awo->state == AWC_CHOOSING_LINE_OP ) goto done;
    if ( awo->state == AWC_WAITING ) goto done;

    ke = (XKeyEvent *) e;

    xInc = 0;
    yInc = 0;

    charCount = XLookupString( ke, keyBuf, keyBufSize, &key, &compose );




    if ( ( awo->state == AWC_MOVING_POINT ) ||
         ( awo->state == AWC_MOVING_CREATE_POINT ) ) {

      xInc = yInc = 0;

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

      awo->usingArrowKeys = 1;

      stat = awo->currentPointObject->movePointRel( awo->currentPoint,
       xInc, yInc );

      if ( awo->viewDims ) {
        setPointDimensions( awo, awo->currentPoint->x,
         awo->currentPoint->y );
      }

      goto done;

    }




    if ( ( key == XK_Control_L ) || ( key == XK_Control_R ) ) {
      awo->ctlKeyPressed = 1;
    }

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
      awo->clear();
      awo->refresh();
    }
    else if ( key == XK_g ) {
      awo->gridShow = 0;
      awo->clear();
      awo->refresh();
    }
    else if ( key == XK_S ) {
      awo->gridActive = 1;
    }
    else if ( key == XK_s ) {
      awo->gridActive = 0;
    }
    else if ( key == XK_x ) {
      cut( awo );
      awo->refresh();
    }
    else if ( key == XK_Delete) {
      delete_items ( awo );
      awo->refresh();
    }
    else if ( key == XK_c ) {
      copy( awo );
      awo->refresh();
    }
    else if ( key == XK_v ) {
      XQueryPointer( awo->d, XtWindow(awo->drawWidget), &root, &child,
       &rootX, &rootY, &winX, &winY, &mask );
      paste( winX, winY, AWC_POPUP_PASTE, awo );
      awo->refresh();
    }
    else if ( key == XK_V ) {
      XQueryPointer( awo->d, XtWindow(awo->drawWidget), &root, &child,
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
    else if ( key == XK_period ) {
      if ( awo->state == AWC_NONE_SELECTED ) {
        do_selectAll( awo );
      }
      else if ( ( awo->state == AWC_ONE_SELECTED ) ||
	   ( awo->state == AWC_MANY_SELECTED ) ) {
        do_deselect( awo );
      }
    }
    else if ( key == XK_bracketleft ) {
      if ( ( awo->state == AWC_ONE_SELECTED ) ||
	   ( awo->state == AWC_MANY_SELECTED ) ) {
        do_group( awo );
      }
    }
    else if ( key == XK_bracketright ) {
      if ( ( awo->state == AWC_ONE_SELECTED ) ||
	   ( awo->state == AWC_MANY_SELECTED ) ) {
        do_ungroup( awo );
      }
    }
    else if ( key == XK_R ) {
      curSel = awo->selectedHead->selFlink;
      if ( curSel != awo->selectedHead ) {
        awo->recordedRefRect = 1;
        curSel->node->getSelBoxDims( &(awo->refRect[1].x),
         &(awo->refRect[1].y), &(awo->refRect[1].w), &(awo->refRect[1].h) );
      }
    }
    else if ( key == XK_r ) {
      awo->recordedRefRect = 0;
    }
    else if ( key == XK_plus ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        alignCenter( awo );
      }
    }
    else if ( key == XK_minus ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        alignCenterHorz( awo );
      }
    }
    else if ( key == XK_bar ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        alignCenterVert( awo );
      }
    }
    else if ( key == XK_b ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        alignSizeBoth( awo );
      }
    }
    else if ( key == XK_h ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        alignSizeHeight( awo );
      }
    }
    else if ( key == XK_w ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        alignSizeWidth( awo );
      }
    }
    else if ( key == XK_2 ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        distrib2D( awo );
      }
    }
    else if ( key == XK_e ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        distribVert( awo );
      }
    }
    else if ( key == XK_E ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        distribMidptVert( awo );
      }
    }
    else if ( key == XK_f ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        distribHorz( awo );
      }
    }
    else if ( key == XK_F ) {
      if ( awo->state == AWC_MANY_SELECTED ) {
        distribMidptHorz( awo );
      }
    }
    else {
      nothingDone = True;
    }

    if ( ( awo->state == AWC_ONE_SELECTED ) ||
	 ( awo->state == AWC_MANY_SELECTED ) ) {

      nothingDone = False;

      if ( ( key == XK_Left ) ||
           ( key == XK_Right ) ||
           ( key == XK_Up ) ||
	   ( key == XK_Down ) ) {

        XQueryPointer( awo->d, XtWindow(awo->drawWidget), &root, &child,
         &rootX, &rootY, &winX, &winY, &mask );

        cur = awo->selectedHead->selFlink;
        operation = 0;
        while ( ( cur != awo->selectedHead ) && !operation ) {
          operation = cur->node->getSelectBoxOperation( awo->ctlKeyPressed,
           winX, winY );
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
	else {

	  // ----------------------------------------------------------
	  // no operation so if more than one selected do align
	  if ( awo->state == AWC_MANY_SELECTED ) {

            if ( key == XK_Left ) {
	      alignLeft( awo );
	    }
            else if ( key == XK_Right ) {
	      alignRight( awo );
	    }
            else if ( key == XK_Up ) {
	      alignTop( awo );
	    }
            else if ( key == XK_Down ) {
	      alignBot( awo );
	    }

	  }
	  // ----------------------------------------------------------

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

    if ( Button4 == ((XButtonEvent*)e)->button ||
         Button5 == ((XButtonEvent*)e)->button ) {
      nothingDone = True;
      goto done;
    }

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

          if ( ( awo->state == AWC_EDITING_POINTS ) ||
               ( awo->state == AWC_CREATING_POINTS ) ) {

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

          if ( ( awo->state == AWC_EDITING_POINTS ) ||
               ( awo->state == AWC_CREATING_POINTS ) ) {

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
                    fprintf( stderr, "%s at x=%-d, y=%-d : selBlink is null (1)\n",
		     cur->node->objName(), cur->node->getX0(),
		     cur->node->getY0() );
		  }
                  if ( cur->selFlink ) {
                    cur->selFlink->selBlink = cur->selBlink;
		  }
		  else {
                    fprintf( stderr, "%s at x=%-d, y=%-d : selFlink is null (2)\n",
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
                  awo->undoObj.startNewUndoList( activeWindowClass_str189 );
                  editNode->node->doEdit( &(awo->undoObj) );
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
                  //fprintf( stderr, "Klingons decloaking! (3)\n" );
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

//========== Shift B2 Press ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B2 Press ===================================

          if ( ( awo->state == AWC_EDITING_POINTS ) ||
               ( awo->state == AWC_CREATING_POINTS ) ) {

            stat = awo->currentPointObject->removeLastPoint();

          }
	  else {

            cur = awo->head->blink;
            while ( cur != awo->head ) {

              if ( ( be->x > cur->node->getX0() ) &&
                   ( be->x < cur->node->getX1() ) &&
                   ( be->y > cur->node->getY0() ) &&
                   ( be->y < cur->node->getY1() ) ) {

                // only highest object (with a non-blank pv) may participate
                if ( cur->node->atLeastOneDragPv( be->x, be->y ) ) {
                  stat = cur->node->startDrag( be, be->x, be->y );
                  if ( stat ) {
                    break; // out of while loop
                  }
                }

              }

              cur = cur->blink;

            }

	  }

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
	  else if ( awo->state == AWC_CREATING_POINTS ) {

            awo->currentPoint =
              awo->currentPointObject->selectPoint( be->x, be->y );

            if ( awo->currentPoint ) {
              awo->state = AWC_MOVING_CREATE_POINT;
	    }

          }

//========== B2 Press ===================================

        }

        break;

      case Button3:

        if ( be->state & ShiftMask ) {

//========== Shift B3 Press ===================================

          if ( awo->state == AWC_EDITING_POINTS ) {

            awo->currentPointObject->lineEditCancel();
            stat = awo->undoObj.performUndo();
            awo->setChanged();
            awo->clear();
            awo->refresh();
            awo->updateMasterSelection();

	  }

//========== Shift B3 Press ===================================

        }
        else if ( be->state & ControlMask ) {

//========== Ctrl B3 Press ===================================

          if ( ( awo->state == AWC_EDITING_POINTS ) ||
               ( awo->state == AWC_CREATING_POINTS ) ) {

            //awo->filterPosition( &be->x, &be->y, be->x, be->y );

            stat = awo->currentPointObject->removePoint( be->x, be->y );

          }

//========== Ctrl B3 Press ===================================

        }
        else {

//========== B3 Press ===================================

          if ( ( awo->state == AWC_EDITING_POINTS ) ||
               ( awo->state == AWC_CREATING_POINTS ) ) {

            //awo->filterPosition( &be->x, &be->y, be->x, be->y );

            stat = awo->currentPointObject->insertPoint( be->x, be->y );

          }

//========== B3 Press ===================================

//           break;

        }

        break;

    }

  }
  else if ( e->type == ButtonRelease ) {

    if ( Button4 == ((XButtonEvent*)e)->button ||
         Button5 == ((XButtonEvent*)e)->button ) {
      nothingDone = True;
      goto done;
    }

    if ( awo->state == AWC_WAITING ) goto done;

    be = (XButtonEvent *) e;

    switch ( be->button ) {

      case Button1:

        if ( be->state & ShiftMask ) {

//========== Shift B1 Release ===================================

          switch ( awo->state ) {

            case AWC_EDITING_POINTS:
            case AWC_CREATING_POINTS:
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
                      fprintf( stderr, "%s at x=%-d, y=%-d : selBlink is null (3)\n",
		       cur->node->objName(), cur->node->getX0(),
		       cur->node->getY0() );
		    }
                    if ( cur->selFlink ) {
                      cur->selFlink->selBlink = cur->selBlink;
		    }
		    else {
                      fprintf( stderr, "%s at x=%-d, y=%-d : selFlink is null (4)\n",
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
                    fprintf( stderr, "%s at x=%-d, y=%-d : selBlink is null (5)\n",
		     cur->node->objName(), cur->node->getX0(),
		     cur->node->getY0() );
		  }
                  if ( cur->selFlink ) {
                    cur->selFlink->selBlink = cur->selBlink;
		  }
		  else {
                    fprintf( stderr, "%s at x=%-d, y=%-d : selFlink is null (6)\n",
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
                awo->showSelectionObject();
              }
              else {
                  printErrMsg( __FILE__, __LINE__,
                   "Inconsistent select state" );
                  //fprintf( stderr, "Klingons decloaking! (4)\n" );
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
            case AWC_CREATING_POINTS:
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
                    awo->undoObj.startNewUndoList( activeWindowClass_str189 );
                    cur->node->doEdit( &(awo->undoObj) );
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
                      fprintf( stderr, "%s at x=%-d, y=%-d : selBlink is null (7)\n",
		       cur->node->objName(), cur->node->getX0(),
		       cur->node->getY0() );
		    }
                    if ( cur->selFlink ) {
                      cur->selFlink->selBlink = cur->selBlink;
		    }
		    else {
                      fprintf( stderr, "%s at x=%-d, y=%-d : selFlink is null (8)\n",
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
                  //fprintf( stderr, "Klingons decloaking! (5)\n" );
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
                  awo->undoObj.startNewUndoList( activeWindowClass_str189 );
                  awo->selectedHead->selFlink->node->doEdit( &(awo->undoObj) );
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

          if ( ( awo->state == AWC_CREATING_POINTS ) ||
               ( awo->state == AWC_EDITING_POINTS ) ) {
            // do nothing
          }
          else {

            cur = awo->head->blink;
            while ( cur != awo->head ) {

              if ( ( be->x > cur->node->getX0() ) &&
                   ( be->x < cur->node->getX1() ) &&
                   ( be->y > cur->node->getY0() ) &&
                   ( be->y < cur->node->getY1() ) ) {

                if ( cur->node->atLeastOneDragPv( be->x, be->y ) ) {
                  cur->node->selectDragValue( be );
                  break; // out of while loop
                }

              }

              cur = cur->blink;

            }

          }

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
              awo->usingArrowKeys = 0;
              if ( awo->currentPointObject ) {
                awo->currentPointObject->deselectAllPoints();
	      }
              break;

	    case AWC_MOVING_CREATE_POINT:

              awo->state = AWC_CREATING_POINTS;
              awo->usingArrowKeys = 0;
              if ( awo->currentPointObject ) {
                awo->currentPointObject->deselectAllPoints();
	      }

              break;

	    case AWC_EDITING_POINTS:
            case AWC_CREATING_POINTS:
              break;

	    case AWC_NONE_SELECTED:

              awo->b2NoneSelectX = be->x;
              awo->b2NoneSelectY = be->y;
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

//              awo->width = awo->oldx - awo->startx;
//              awo->height = awo->oldy - awo->starty;
//
//              if ( ( awo->width < 0 ) && ( awo->height < 0 ) ) {
//                selectMustEnclose = False;
//              }
//              else {
//                selectMustEnclose = True;
//              }
//
//              if ( awo->width < 0 ) {
//                x0 = awo->oldx;
//                awo->width = abs( awo->width );
//              }
//              else {
//                x0 = awo->startx;
//	      }
//
//              if ( awo->height < 0 ) {
//                y0 = awo->oldy;
//                awo->height = abs( awo->height );
//              }
//              else {
//                y0 = awo->starty;
//	      }
//
//              if ( ( awo->width > 0 ) && ( awo->height > 0 ) ) {
//
//                // erase region definition box
//                awo->drawGc.saveFg();
//                awo->drawGc.setFG( awo->ci->pix(awo->fgColor) );
//                XDrawRectangle( awo->d, XtWindow(awo->drawWidget),
//                 awo->drawGc.xorGC(), x0, y0, awo->width,
//                 awo->height );
//		awo->drawGc.restoreFg();
//
//              }
//
//              if ( ( awo->width < 3 ) && ( awo->height < 3 ) ) {
//                awo->state = awo->savedState;
//                doAgain = 1;
//                break; // out of switch statement
//              }
//
//              awo->startx = x0;
//              awo->starty = y0;
//
//              awo->state = awo->savedState;  // if below fails
//              num_selected = 0;
//
//              if ( ( awo->width > 0 ) && ( awo->height > 0 ) ) {
//
//                cur = awo->head->blink;
//                while ( cur != awo->head ) {
//
//                  if ( selectMustEnclose ) {
//
//                    wasSelected = cur->node->isSelected();
//                    gotSelection = cur->node->selectEnclosed( awo->startx,
//                     awo->starty, awo->width, awo->height );
//                    if ( !wasSelected ) {
//                      if ( gotSelection ) {
//                        num_selected++;
//                        cur->node->drawSelectBoxCorners();
//                        cur->selBlink = awo->selectedHead->selBlink;
//                        awo->selectedHead->selBlink->selFlink = cur;
//                        awo->selectedHead->selBlink = cur;
//                        cur->selFlink = awo->selectedHead;
//                      }
//                    }
//                    else {
//                      if ( gotSelection ) {
//                        cur->node->drawSelectBoxCorners(); // erase via xor gc
//                        cur->node->deselect();
//		        // unlink
//                        if ( cur->selBlink ) {
//                          cur->selBlink->selFlink = cur->selFlink;
//		        }
//		        else {
//                          fprintf( stderr,
//                           "%s at x=%-d, y=%-d : selBlink is null (9)\n",
//		           cur->node->objName(), cur->node->getX0(),
//		           cur->node->getY0() );
//		        }
//                        if ( cur->selFlink ) {
//                          cur->selFlink->selBlink = cur->selBlink;
//		        }
//		        else {
//                          fprintf( stderr,
//                           "%s at x=%-d, y=%-d : selFlink is null (A)\n",
//		           cur->node->objName(), cur->node->getX0(),
//		           cur->node->getY0() );
//		        }
//                      }
//		      else {
//                        num_selected++;
//		      }
//                    }
//
//                  }
//                  else {
//
//                    wasSelected = cur->node->isSelected();
//                    gotSelection = cur->node->selectTouching( awo->startx,
//                     awo->starty, awo->width, awo->height );
//                    if ( !wasSelected ) {
//                      if ( gotSelection ) {
//                        num_selected++;
//                        cur->node->drawSelectBoxCorners();
//                        cur->selBlink = awo->selectedHead->selBlink;
//                        awo->selectedHead->selBlink->selFlink = cur;
//                        awo->selectedHead->selBlink = cur;
//                        cur->selFlink = awo->selectedHead;
//                      }
//                    }
//                    else {
//                      if ( gotSelection ) {
//                        cur->node->drawSelectBoxCorners(); // erase via xor gc
//                        cur->node->deselect();
//		        // unlink
//                        if ( cur->selBlink ) {
//                          cur->selBlink->selFlink = cur->selFlink;
//		        }
//		        else {
//                          fprintf( stderr,
//                           "%s at x=%-d, y=%-d : selBlink is null (B)\n",
//		           cur->node->objName(), cur->node->getX0(),
//		           cur->node->getY0() );
//		        }
//                        if ( cur->selFlink ) {
//                          cur->selFlink->selBlink = cur->selBlink;
//		        }
//		        else {
//                          fprintf( stderr,
//                           "%s at x=%-d, y=%-d : selFlink is null (C)\n",
//		           cur->node->objName(), cur->node->getX0(),
//		           cur->node->getY0() );
//		        }
//                      }
//		      else {
//                        num_selected++;
//		      }
//                    }
//
//                  }
//
//                  cur = cur->blink;
//
//                }
//
//                if ( num_selected == 0 ) {
//                  awo->state = AWC_NONE_SELECTED;
//                  awo->updateMasterSelection();
//		  // doAgain = 1;
//		}
//                else if ( num_selected == 1 ) {
//                  awo->state = AWC_ONE_SELECTED;
//                  awo->useFirstSelectedAsReference = 1;
//                  awo->updateMasterSelection();
//                  // if ( prev_num_selected == num_selected ) doAgain = 1;
//		}
//                else {
//                  awo->state = AWC_MANY_SELECTED;
//                  awo->updateMasterSelection();
//                  // if ( prev_num_selected == num_selected ) doAgain = 1;
//		}
//
//              }
//
//              break;
//
//          }
//
//	  } while ( doAgain );

// new code which uses EDMELECTOR env var

              awo->width = awo->oldx - awo->startx;
              awo->height = awo->oldy - awo->starty;
              char *selectOr = getenv( environment_str41 );      // EDMELECTOR
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
                    if ( !wasSelected) {
                      if ( gotSelection ) {
                        num_selected++;
                        awo->select(cur);
                      }
                    }
                    else {
                      if ( gotSelection ) {
                        if (!selectOr) {
                          awo->unselect(cur);
                        }
                      }
                      else {
                        if (selectOr) {
                            awo->unselect(cur);
                        } else {
                            num_selected++;
                        }
                      }
                    }

                  }
                  else {
                    wasSelected = cur->node->isSelected();
                    gotSelection = cur->node->selectTouching( awo->startx,
                      awo->starty, awo->width, awo->height );
                    if ( !wasSelected) {
                      if ( gotSelection ) {
                        num_selected++;
                        awo->select(cur);
                      }
                    }
                    else {
                      if ( gotSelection ) {
                        if (!selectOr) {
                          awo->unselect(cur);
                        }
                      }
                      else {
                        if (selectOr) {
                           awo->unselect(cur);
                        } else {
                            num_selected++;
                        }
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

    if ( !awo->usingArrowKeys ) {
      if ( awo->viewDims ) {
        setPointDimensions( awo, (int) me->x, (int) me->y );
      }
    }

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
	  case AWC_MOVING_CREATE_POINT:
            break;

	  case AWC_EDITING_POINTS:
          case AWC_CREATING_POINTS:
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
	  case AWC_MOVING_CREATE_POINT:

	    if ( !awo->usingArrowKeys ) {
              stat = awo->currentPointObject->movePoint( awo->currentPoint,
               me->x, me->y );
	    }
            break;

	  case AWC_EDITING_POINTS:
          case AWC_CREATING_POINTS:
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

    nothingDone = True;

  }

done:

  if ( nothingDone ) *continueToDispatch = True;

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
int action, foundAction, foundPvAction, numOut, numIn, buttonNum;
activeGraphicListPtr cur;
activeGraphicClass *ptr;
Window root, child;
int rootX, rootY, winX, winY, di;

Boolean nothingDone = False;

  awo = (activeWindowClass *) client;

  if ( awo->mode != AWC_EXECUTE ) goto done;

  foundAction = 0;

  awo->oldState = awo->state;

  *continueToDispatch = False;

  if ( e->type == MapNotify ) {

    awo->refreshActive();

  }
  else if ( e->type == ConfigureNotify ) {

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

    if ( Button4 == ((XButtonEvent*)e)->button ||
         Button5 == ((XButtonEvent*)e)->button ) {
      nothingDone = True;
      goto done;
    }

    if ( awo->state == AWC_WAITING ) goto done;

    mask = ShiftMask & ControlMask;

    be = (XButtonEvent *) e;

    XQueryPointer( awo->d, XtWindow(awo->executeWidget), &root, &child,
     &rootX, &rootY, &winX, &winY, &mask );

    if ( ( be->button == 2 ) &&
         !( be->state & ShiftMask ) &&
         !( be->state & ControlMask ) ) {

      cur = awo->head->blink;
      while ( cur != awo->head ) {

        if ( ( be->x > cur->node->getX0() ) &&
             ( be->x < cur->node->getX1() ) &&
             ( be->y > cur->node->getY0() ) &&
             ( be->y < cur->node->getY1() ) ) {

          // only the highest object (with a non-blank pv) may participate
          if ( cur->node->atLeastOneDragPv( be->x, be->y ) ) {
            action = cur->node->startDrag( be, be->x, be->y );
            if ( action ) {
              foundAction = 1;
              break; // out of while loop
	          }
	        }

	      }

        cur = cur->blink;

      }

    }
    else if ( ( be->button == 2 ) &&
         ( be->state & ShiftMask ) &&
         ( be->state & ControlMask ) ) {

      cur = awo->head->blink;
      while ( cur != awo->head ) {

        if ( ( be->x > cur->node->getX0() ) &&
             ( be->x < cur->node->getX1() ) &&
             ( be->y > cur->node->getY0() ) &&
             ( be->y < cur->node->getY1() ) ) {

          if ( cur->node->atLeastOneDragPv( be->x, be->y ) ) {
            action = cur->node->showPvInfo( be, be->x, be->y );
            if ( action ) {
              foundAction = 1;
              break; // out of while loop
	          }
	        }

	      }

        cur = cur->blink;

      }

    }
    else if ( ( be->button == 2 ) && ( be->state & ShiftMask ) ) {
      // do nothing
    }
    else if ( ( be->button == 2 ) && ( be->state & ControlMask ) ) {
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
          curBtn->node->btnDown( be, winX, winY, be->state, be->button,
           &action );
	        awo->btnDownX = be->x;
	        awo->btnDownY = be->y;

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

    if ( Button4 == ((XButtonEvent*)e)->button ||
         Button5 == ((XButtonEvent*)e)->button ) {
      nothingDone = True;
      goto done;
    }

    if ( awo->state == AWC_WAITING ) goto done;

    be = (XButtonEvent *) e;

    XQueryPointer( awo->d, XtWindow(awo->executeWidget), &root, &child,
     &rootX, &rootY, &winX, &winY, &mask );

    if ( ( be->button == 2 ) &&
         ( be->state & ShiftMask ) &&
         !( be->state & ControlMask ) ) {

      cur = awo->head->blink;
      while ( cur != awo->head ) {

        if ( ( be->x > cur->node->getX0() ) &&
             ( be->x < cur->node->getX1() ) &&
             ( be->y > cur->node->getY0() ) &&
             ( be->y < cur->node->getY1() ) ) {

          foundAction = 1;

          if ( cur->node->atLeastOneDragPv( be->x, be->y ) ) {
            cur->node->selectDragValue( be );
            break; // out of while loop
	  }

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
          curBtn->node->btnUp( be, awo->btnDownX, awo->btnDownY, be->state,
           be->button, &action );
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

        if ( ( be->state & ShiftMask ) && !( be->state & ControlMask ) ) {

//========== Shift B2 Release ===================================

//========== Shift B2 Release ===================================

        }
        else if ( !( be->state & ShiftMask ) && ( be->state & ControlMask ) ) {

//========== Ctrl B2 Release ===================================

          foundPvAction = 0;

          cur = awo->head->blink;
          while ( cur != awo->head ) {

            if ( ( be->x > cur->node->getX0() ) &&
                 ( be->x < cur->node->getX1() ) &&
                 ( be->y > cur->node->getY0() ) &&
                 ( be->y < cur->node->getY1() ) ) {

              if ( cur->node->atLeastOneDragPv( be->x, be->y ) ) {

                di = cur->node->getCurrentDragIndex();

                if ( cur->node->dragValue( be->x, be->y, di ) ) {

                  if ( !blankOrComment( cur->node->dragValue( be->x, be->y, di ) ) ) {

                    if ( awo->pvAction->numActions() ) {

                      foundPvAction = 1;

                      awo->pvAction->setInfo( cur->node->dragValue( be->x, be->y, di ),
                       XDisplayName(awo->appCtx->displayName) );

                      XmMenuPosition( awo->actionPopup, be );
                      XtManageChild( awo->actionPopup );

                      XSetWindowColormap( awo->d,
                       XtWindow(XtParent(awo->actionPopup)),
                       awo->appCtx->ci.getColorMap() );

                    }

                  }

                }

                break; // out of while loop

              }

            }

            cur = cur->blink;

          }

//========== Ctrl B2 Release ===================================

        }
        else if ( ( be->state & ShiftMask ) && ( be->state & ControlMask ) ) {

//========== Shift-Ctrl B2 Release =============================

//========== Shift-Ctrl B2 Release =============================

	}
        else {

//========== B2 Release ===================================

          awo->b2PressX = winX;
          awo->b2PressY = winY;
          awo->b2PressXRoot = be->x_root;
          awo->b2PressYRoot = be->y_root;

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

    XQueryPointer( awo->d, XtWindow(awo->executeWidget), &root, &child,
     &rootX, &rootY, &winX, &winY, &mask );

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
          if ( buttonNum ) {
            curBtn->node->btnDrag( me, winX, winY, me->state, buttonNum );
	  }

        }

        curBtn = curBtn->flink;

      }

    }

    /* mouse pointer in and out - processed for all objects */

    curBtn = awo->btnFocusActionHead->blink;
    while ( curBtn != awo->btnFocusActionHead ) {

      curBtn->node->checkMouseOver( me, winX, winY, me->state );

      curBtn = curBtn->blink;

    }

    /* pointer in and out - processed for only the top object, in */
    /* addition, this can make the pointer icon change */

    numIn = numOut = 0;

    if ( awo->highlightedObject ) {

      if ( awo->highlightedObject ==
           awo->highlightedObject->enclosingObject( me->x, me->y ) ) {

        // still highlighted

	// But let another, overlapping, higher object steal focus

        ptr = NULL;
        curBtn = awo->btnFocusActionHead->blink;
        while ( ( curBtn != awo->btnFocusActionHead ) &&
                ( ptr == NULL ) ) {

          ptr = curBtn->node->enclosingObject( me->x, me->y );
          if ( ptr ) {
            awo->highlightedObject->pointerOut( me, winX, winY, me->state );
            awo->highlightedObject = NULL;
            numOut++;
            foundAction = 1;
            awo->cursor.set( XtWindow(awo->executeWidget), CURSOR_K_DEFAULT );
	          break;
          }

          curBtn = curBtn->blink;

        }

      }
      else {

        awo->highlightedObject->pointerOut( me, winX, winY, me->state );
        awo->highlightedObject = NULL;
        numOut++;
        foundAction = 1;
        awo->cursor.set( XtWindow(awo->executeWidget), CURSOR_K_DEFAULT );

      }

    }

    if ( !(awo->highlightedObject) ) {

      curBtn = awo->btnFocusActionHead->blink;
      while ( curBtn != awo->btnFocusActionHead ) {

        ptr = curBtn->node->enclosingObject( me->x, me->y );
        if ( ptr ) {
          ptr->pointerIn( me, winX, winY, me->state );
          awo->highlightedObject = ptr;
          numIn++;
          foundAction = 1;
	        break;
        }

        curBtn = curBtn->blink;

      }

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

    nothingDone = True;

  }

done:

  if ( nothingDone ) *continueToDispatch = True;

  return;

}

activeWindowClass::activeWindowClass ( void ) : unknownTags() {

char *str;
int i;

  templateFileSelectBox = NULL;

  curReplaceIndex = -1;
  replaceOld = NULL;
  replaceNew = NULL;
  sar1 = NULL;
  sar2 = NULL;
  sarCaseInsensivite = 1;
  sarUseRegExpr = 0;
  accVal = bufAccVal = 0;

  numRefPoints = numRefRects = recordedRefRect = 0;
  showDimTimer = 0;
  dimDialog = NULL;
  viewDims = 0;

  strcpy( templInfo, "" );
  bufTemplInfo = NULL;

  for ( i=0; i<AWC_MAXTMPLPARAMS; i++ ) {
    strcpy( paramValue[i], "" );
  }

  invalidBgColor = 0;
  invalidFile = 0;

  usePixmap = -1; // -1 means unknown, will be determined on execute
  bgPixmap = (Pixmap) NULL;
  pixmapW = pixmapH = -1;
  needCopy = 0;
  pixmapX0 = pixmapX1 = pixmapY0 = pixmapY1 = 0;
  bgPixmapFlag = 0;

  windowState = AWC_INIT;

  str = getenv( environment_str20 );
  if ( str ) {
    clearEpicsPvTypeDefault = 1;
  }
  else {
    clearEpicsPvTypeDefault = 0;
  }

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
  disableScroll = 0;
  orthoMove = 0;
  masterSelectX0 = masterSelectY0 = masterSelectX1 = masterSelectY1 = 0;
  isIconified = False;
  autosaveTimer = 0;
  doAutoSave = 0;
  doClose = 0;
  doActiveClose = 0;
  waiting = 0;
  restoreTimer = 0;

  commentHead = new commentLinesType;
  commentHead->line = NULL;
  commentTail = commentHead;
  commentTail->flink = NULL;

  pvDefHead = new pvDefType;
  pvDefHead->def = NULL;
  pvDefTail = pvDefHead;
  pvDefTail->flink = NULL;
  forceLocalPvs = 0;

  internalRelatedDisplay = NULL;

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
  numMacros = actualNumMacros = 0;
  macros = NULL;
  expansions = NULL;

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
  actionPopup = NULL;
  drawWidget = NULL;
  top = NULL;

  useComponentScheme = 0;
  allSelectedTextFgColorFlag = 0;
  allSelectedFg1ColorFlag = 0;
  allSelectedFg2ColorFlag = 0;
  allSelectedBgColorFlag = 0;
  allSelectedOffsetColorFlag = 0;
  allSelectedTopShadowColorFlag = 0;
  allSelectedBotShadowColorFlag = 0;
  allSelectedFontTagFlag = 0;
  allSelectedAlignmentFlag = 0;
  allSelectedCtlFontTagFlag = 0;
  allSelectedCtlAlignmentFlag = 0;
  allSelectedBtnFontTagFlag = 0;
  allSelectedBtnAlignmentFlag = 0;

  strcpy( allSelectedCtlPvName[0], "" );
  strcpy( allSelectedReadbackPvName[0], "" );
  strcpy( allSelectedNullPvName[0], "" );
  strcpy( allSelectedVisPvName[0], "" );
  strcpy( allSelectedAlarmPvName[0], "" );

  allSelectedCtlPvNameFlag = 0;
  allSelectedReadbackPvNameFlag = 0;
  allSelectedNullPvNameFlag = 0;
  allSelectedVisPvNameFlag = 0;
  allSelectedAlarmPvNameFlag = 0;

  versionStackPtr = 0;

  buttonClickTime = 0;
  deltaTime = 0;

  msgDialogCreated = 0;
  msgDialogPoppedUp = 0;

  objNameDialogCreated = 0;
  objNameDialogPoppedUp = 0;

  strcpy( curSchemeSet, "" );

  noRaise = 0;

  noEdit = 0;
  closeAllowed = 0;

  stale = 0;
  modTime = 0;

  numChildren = 0;
  parent = NULL;
  isEmbedded = 0;
  embSetSize = 0;
  embSizeOfs = 0;
  widgetToDeallocate = NULL;

  loadFailure = 0;

  haveComments = 0;
  strcpy( fileNameAndRev, "" );
  strcpy( fileRev, "" );

  btnDownX = btnDownY = 0;

  mode = AWC_EDIT;

  pvAction = new pvActionClass;

  ctlKeyPressed = 0;

  b2NoneSelectX = b2NoneSelectY = 0;

  reloadRequestFlag = 0;

  frozen = false;

}

int activeWindowClass::getRandFile (
  char *outStr,
  int outStrMaxLen
) {

char *envPtr, name[255+1];
int fd;

  if ( strcmp( fileName, "" ) != 0 ) {
    extractName( fileName, name );
  }
  else {
    strcpy( name, "edm_dump_pvs" );
  }

  envPtr = getenv( environment_str154 ); // EDMPVDUMP
  if ( envPtr ) {
    snprintf( outStr, outStrMaxLen, "%s/%s_XXXXXX", envPtr, name );
  }
  else {
    snprintf( outStr, outStrMaxLen, "/tmp/%s_XXXXXX", name );
  }
  outStr[outStrMaxLen] = 0;

  fd = mkstemp( outStr );
  outStr[outStrMaxLen] = 0;

  if ( debugMode() ) {
    fprintf( stderr, "PV list dump file is [%s], fd = %-d\n", outStr, fd );
  }

  return fd;

}

void activeWindowClass::dumpPvList ( void ) {

char fname[255+1], *envPtr, *ptr, msg[255+1];
activeGraphicListPtr cur;
int i, n, nPvs, stat, dup, fd;
ProcessVariable *pvs[1000];
AVL_HANDLE pvNameTree;
nameListPtr curNameNode;
FILE *f;
int avlTreeCreated = 0;
int fileOpened = 0;

  envPtr = getenv( environment_str154 ); // EDMPVDUMP
  if ( !envPtr ) return;

  fd = getRandFile( fname, 255 );
  if ( !fd ) return;

  fileOpened = 1;

  snprintf( msg, 255, "edl file name: %s\n", this->fileName );
  msg[255] = 0;
  write( fd, msg, strlen(msg) );

  stat = avl_init_tree( compare_nodes, compare_key, copy_node,
   &pvNameTree );
  if ( !( stat & 1 ) ) {
    snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
    appCtx->postMessage( msg );
    goto done;
  }

  avlTreeCreated = 1;

  cur = head->blink;
  while ( cur != head ) {

    for( i=0; i<1000; i++ ) pvs[i] = 0;
    cur->node->getPvs( 1000, pvs, &n );
    for ( i=0; i<n; i++ ) {
      if ( pvs[i] && pvs[i]->is_epics() && pvs[i]->get_name() ) {
        curNameNode = (nameListPtr) calloc( sizeof(nameListType), 1 );
        if ( !curNameNode ) {
          snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
          appCtx->postMessage( msg );
          goto done;
        }
        curNameNode->name = (char *) pvs[i]->get_name();
        stat = avl_insert_node( pvNameTree, (void *) curNameNode, &dup );
        if ( !( stat & 1 ) ) {
          snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
          appCtx->postMessage( msg );
          goto done;
        }
        if ( dup ) {
          free( curNameNode );
        }
      }
    }

    cur = cur->blink;

  }

  nPvs = 0;
  stat = avl_get_first( pvNameTree, (void **) &curNameNode );
  if ( !( stat & 1 ) ) {
    snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
    appCtx->postMessage( msg );
    goto done;
  }
  while ( curNameNode ) {

    snprintf( msg, 255, "%s\n", curNameNode->name );
    msg[255] = 0;
    write( fd, msg, strlen(msg) );

    nPvs++;

    stat = avl_get_next( pvNameTree, (void **) &curNameNode );
    if ( !( stat & 1 ) ) {
      snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
      appCtx->postMessage( msg );
      goto done;
    }

  }

  fileOpened = 0;

  close( fd );

done:

  if ( fileOpened ) {

    fileOpened = 0;

    close( fd );

  }

  if ( avlTreeCreated ) {

    // delete tree
    curNameNode = NULL;
    stat = avl_get_first( pvNameTree, (void **) &curNameNode );
    if ( !( stat & 1 ) ) {
      snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
      appCtx->postMessage( msg );
      return;
    }
    while ( curNameNode ) {

      stat = avl_delete_node( pvNameTree, (void **) &curNameNode );
      if ( !( stat & 1 ) ) {
        snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
        appCtx->postMessage( msg );
        return;
      }

      free( curNameNode );
      curNameNode = NULL;

      stat = avl_get_first( pvNameTree, (void **) &curNameNode );
      if ( !( stat & 1 ) ) {
        snprintf( msg, 255, activeWindowClass_str198, __LINE__, __FILE__ );
        appCtx->postMessage( msg );
        return;
      }

    }

  }

}

void activeWindowClass::initCopy ( void ) {

  pixmapX0 = w;
  pixmapY0 = h;
  pixmapX1 = 0;
  pixmapY1 = 0;

}

void activeWindowClass::updateCopyRegion (
  int _x0,
  int _y0,
  int _w,
  int _h
) {

int _x1, _y1;

  _x1 = _x0 + _w;
  _y1 = _y0 + _h;

  if ( pixmapX0 > _x0 ) pixmapX0 = _x0;
  if ( pixmapX1 < _x1 ) pixmapX1 = _x1;
  if ( pixmapY0 > _y0 ) pixmapY0 = _y0;
  if ( pixmapY1 < _y1 ) pixmapY1 = _y1;

}

void activeWindowClass::doCopy ( void ) {

  if ( mode == AWC_EDIT ) {
    needCopy = 0;
    return;
  }

  if ( needCopy ) {

    //printf( "full copy\n" );
    //printf( "[ %-d, %-d, %-d, %-d ]\n",
    // pixmapX0, pixmapX1, pixmapY0, pixmapY1 );

    needCopy = 0;
    needFullCopy = 0;

    if ( bgPixmap ) {
      //printf( "do copy\n" );
      XCopyArea( d, bgPixmap,
       XtWindow(executeWidget), executeGc.normGC(),
       0, 0, w, h, 0, 0 );
      initCopy();
    }

  }

}

void activeWindowClass::doMinCopy ( void ) {

  if ( mode == AWC_EDIT ) {
    needCopy = 0;
    needFullCopy = 0;
    return;
  }

  if ( needFullCopy ) {
    doCopy();
    return;
  }

  pixmapX0 -= 10;
  if ( pixmapX0 < 0 ) pixmapX0 = 0;

  pixmapX1 += 10;
  if ( pixmapX1 > w ) pixmapX1 = w;

  pixmapY0 -= 10;
  if ( pixmapY0 < 0 ) pixmapY0 = 0;

  pixmapY1 += 10;
  if ( pixmapY1 > h ) pixmapY1 = h;

  int pixW = pixmapX1 - pixmapX0 + 1;
  int pixH = pixmapY1 - pixmapY0 + 1;

  if ( pixW < 1 ) return;
  if ( pixH < 1 ) return;

  if ( needCopy ) {

    needCopy = 0;

    if ( bgPixmap ) {
      //printf( "do copy\n" );
      XCopyArea( d, bgPixmap,
       XtWindow(executeWidget), executeGc.normGC(),
       pixmapX0, pixmapY0, pixW, pixH, pixmapX0, pixmapY0 );
      initCopy();
    }

  }

}

Drawable activeWindowClass::drawable (
  Widget w
) {

  if ( bgPixmap ) {
    return (Drawable) bgPixmap;
  }
  else {
    return (Drawable) XtWindow(executeWidget);
  }

}

int activeWindowClass::okToDeactivate ( void ) {

activeGraphicListPtr cur, next;

  if ( loadFailure ) return 1;

  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    if ( cur->node ) {
      if ( !cur->node->activateComplete() ) return 0;
    }
    cur = next;
  }

  if ( windowState == AWC_COMPLETE_EXECUTE ) {
    return 1;
  }

  return 0;

}

int activeWindowClass::okToPreReexecute ( void ) {

activeGraphicListPtr cur, next;

  if ( loadFailure ) return 1;

  cur = head->flink;
  while ( cur != head ) {
    next = cur->flink;
    if ( cur->node ) {
      if ( !cur->node->activateBeforePreReexecuteComplete() ) return 0;
    }
    cur = next;
  }

  if ( windowState == AWC_COMPLETE_EXECUTE ) {
    return 1;
  }

  return 0;

}

void activeWindowClass::getModTime (
  char *oneFileName
) {

struct stat fileStat;
int status;

  if ( strcmp( oneFileName, "" ) != 0 ) {

    status = stat( oneFileName, &fileStat );

    modTime = fileStat.st_mtime;
    stale = 0;

  }
  else {

    stale = 1; // no file name

  }

}

void activeWindowClass::checkModTime (
  char *oneFileName
) {

struct stat fileStat;
int status;

  if ( strcmp( oneFileName, "" ) != 0 ) {

    status = stat( oneFileName, &fileStat );

    if ( modTime < fileStat.st_mtime ) {
      stale = 1;
    }
    else {
      stale = 0;
    }

  }
  else {

    stale = 0; // new file, not yet saved

  }

}

int activeWindowClass::pushVersion ( void ) {

  if ( versionStackPtr > 9 ) return 0; // overflow

  versionStack[versionStackPtr][0] = major;
  versionStack[versionStackPtr][1] = minor;
  versionStack[versionStackPtr][2] = release;
  versionStack[versionStackPtr][3] = haveComments;

  versionStackPtr++;

  return 1;

}

int activeWindowClass::popVersion ( void ) {

  if ( versionStackPtr == 0 ) return 0; // underflow

  versionStackPtr--;

  major = versionStack[versionStackPtr][0];
  minor = versionStack[versionStackPtr][1];
  release = versionStack[versionStackPtr][2];
  haveComments = versionStack[versionStackPtr][3];

  return 1;

}

activeWindowClass::~activeWindowClass ( void ) {

int i, stat;
popupBlockListPtr curPopupBlock, nextPopupBlock;
activeGraphicListPtr curCut, nextCut, cur, next;
objNameListPtr curObjName, nextObjName;
commentLinesPtr commentCur, commentNext;
pvDefPtr pvDefCur, pvDefNext;

  //if ( !isEmbedded ) fprintf( stderr, "Destroy - [%s]\n", fileNameForSym );

  windowState = AWC_TERMINATED;

  if ( templateFileSelectBox ) {

    XtRemoveCallback( templateFileSelectBox, XmNcancelCallback,
     awc_fileSelectCancel_cb, (void *) this );
    XtRemoveCallback( templateFileSelectBox, XmNokCallback,
     awc_templateFileSelectOk_cb, (void *) this );

    XtUnmanageChild( templateFileSelectBox ); // it's ok to unmanage a child any number of times
    XtDestroyWidget( templateFileSelectBox );

    templateFileSelectBox = NULL;

  }

  if ( sar1 ) {
    delete[] sar1;
    sar1 = NULL;
  }

  if ( sar2 ) {
    delete[] sar2;
    sar2 = NULL;
  }

  if ( replaceOld ) {
    delete[] replaceOld;
    replaceOld = NULL;
  }

  if ( replaceNew ) {
    delete[] replaceNew;
    replaceNew = NULL;
  }

  if ( bufTemplInfo ) {
    delete[] bufTemplInfo;
    bufTemplInfo = NULL;
  }

  if ( top ) XtUnmapWidget( top );  //??????? XtUnmapWidget

  if ( dimDialog ) {
    if ( showDimTimer ) {
      XtRemoveTimeOut( showDimTimer );
      showDimTimer = 0;
    }
    viewDims = 0;
    dimDialog->destroy();
    delete dimDialog;
    dimDialog = NULL;
  }

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
    if ( commentCur->line ) delete[] commentCur->line;
    delete commentCur;
    commentCur = commentNext;
  }
  commentTail = commentHead;
  commentTail->flink = NULL;
  delete commentHead;

  pvDefCur = pvDefHead->flink;
  while ( pvDefCur ) {
    pvDefNext = pvDefCur->flink;
    if ( pvDefCur->def ) delete[] pvDefCur->def;
    delete pvDefCur;
    pvDefCur = pvDefNext;
  }
  pvDefTail = pvDefHead;
  pvDefTail->flink = NULL;
  delete pvDefHead;

  if ( ef.formIsPoppedUp() ) ef.popdown();

  if ( strcmp( autosaveName, "" ) != 0 ) {
    stat = unlink( autosaveName );
  }

 if ( list_array_size > 0 ) delete[] list_array;

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
    delete[] curObjName->objType;
    delete curObjName;
    curObjName = nextObjName;
  }

  if ( objNameHead ) delete objNameHead;

  if ( eventHead ) delete eventHead;

  if ( limEventHead ) delete limEventHead;

  if ( pollHead ) delete pollHead;

  for ( i=0; i<actualNumMacros; i++ ) {
    delete[] macros[i];
    delete[] expansions[i];
  }

  if ( macros ) {
    delete[] macros;
    macros = NULL;
  }

  if ( expansions ) {
    delete[] expansions;
    expansions = NULL;
  }

  if ( top ) {
    XtRemoveEventHandler( top, StructureNotifyMask, False,
     topWinEventHandler, (XtPointer) this );
  }

  if ( drawWidget ) {
    XtRemoveEventHandler( drawWidget,
     KeyPressMask|KeyReleaseMask|ButtonPressMask|PointerMotionMask|
     ButtonReleaseMask|Button1MotionMask|
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
  if ( actionPopup ) XtDestroyWidget( actionPopup );

  if ( drawWidget ) XtDestroyWidget( drawWidget );

  if ( msgDialogCreated ) msgDialog.destroy();

  if ( objNameDialogCreated ) objNameDialog.destroy();

#if 1
  if ( top ) XtDestroyWidget( top );
#endif

  // need to deallocate widget address used for top if this was an
  // embedded window
  if ( widgetToDeallocate ) {
    delete widgetToDeallocate;
    widgetToDeallocate = NULL;
  }

  delete pvAction;

  if ( bgPixmap ) {
    XFreePixmap( d, bgPixmap );
    bgPixmap = (Pixmap) NULL;
    pixmapW = pixmapH = -1;
  }

}

void activeWindowClass::select(activeGraphicListPtr cur) {
    cur->node->drawSelectBoxCorners();
    cur->selBlink = selectedHead->selBlink;
    selectedHead->selBlink->selFlink = cur;
    selectedHead->selBlink = cur;
    cur->selFlink = selectedHead;
}

void activeWindowClass::unselect(activeGraphicListPtr cur) {
    cur->node->drawSelectBoxCorners(); // erase via xor gc
    cur->node->deselect();
// unlink
    if ( cur->selBlink ) {
      cur->selBlink->selFlink = cur->selFlink;
    }
    else {
      fprintf( stderr,
       "%s at x=%-d, y=%-d : selBlink is null (B)\n",
       cur->node->objName(), cur->node->getX0(),
       cur->node->getY0() );
    }
    if ( cur->selFlink ) {
      cur->selFlink->selBlink = cur->selBlink;
    }
    else {
      fprintf( stderr,
       "%s at x=%-d, y=%-d : selFlink is null (C)\n",
       cur->node->objName(), cur->node->getX0(),
       cur->node->getY0() );
    }
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
char t[255+1];

  strncpy( fileNameAndRev, fileName, 255 );
  fileNameAndRev[255] = 0;
  if ( !blank(fileRev) ) {
    Strncat( fileNameAndRev, " (", 287 );
    Strncat( fileNameAndRev, fileRev, 287 );
    Strncat( fileNameAndRev, ")", 287 );
  }

  if ( showName || ( mode == AWC_EDIT ) ) {

    if ( strcmp( fileName, "" ) == 0 ) {
      cptr = none;
    }
    else {
      cptr = fileNameAndRev;
    }

  }
  else {

    if ( !expStrTitle.getExpanded() ) {
      if ( strcmp( fileName, "" ) == 0 ) {
        cptr = none;
      }
      else {
        cptr = fileNameAndRev;
      }
    }
    else if ( strcmp( expStrTitle.getExpanded(), "" ) == 0 ) {
      if ( strcmp( fileName, "" ) == 0 ) {
        cptr = none;
      }
      else {
        cptr = fileNameAndRev;
      }
    }
    else {
      //cptr = expStrTitle.getExpanded();
      strncpy( t, expStrTitle.getExpanded(), 255 );
      t[255] = 0;
      if ( invalidFile ) {
        Strncat( t, " (", 255 );
        Strncat( t, activeWindowClass_str214, 255 );
        Strncat( t, ")", 255 );
      }
      cptr = t;
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

  strncpy( fileNameAndRev, fileName, 255 );
  fileNameAndRev[255] = 0;
  if ( !blank(fileRev) ) {
    Strncat( fileNameAndRev, " (", 287 );
    Strncat( fileNameAndRev, fileRev, 287 );
    Strncat( fileNameAndRev, ")", 287 );
  }

  if ( !expStrTitle.getExpanded() ) {
    if ( strcmp( fileName, "" ) == 0 ) {
      cptr = none;
    }
    else {
      cptr = fileNameAndRev;
    }
  }
  else if ( strcmp( expStrTitle.getExpanded(), "" ) == 0 ) {
    if ( strcmp( fileName, "" ) == 0 ) {
      cptr = none;
    }
    else {
      cptr = fileNameAndRev;
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
   0,
   0,
   NULL,
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
   0,
   0,
   NULL,
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

  _edmDebug();

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
   0,
   0,
   NULL,
   _numMacros,
   _macros,
   _expansions );

}

int activeWindowClass::createEmbedded (
  appContextClass *ctx,
  Widget *parent,
  int OneX,
  int OneY,
  int OneW,
  int OneH,
  int _embeddedX,
  int _embeddedY,
  int _embCenter,
  int _embSetSize,
  int _embSizeOfs,
  int _embNoScroll,
  int _numMacros,
  char **_macros,
  char **_expansions ) {

int stat;

  stat = genericCreate(
   ctx,
   *parent,
   OneX,
   OneY,
   OneW,
   OneH,
   1,      // yes: windowDecorations
   1,      // yes: no edit
   0,      //  no: close allowed
   1,      // yes: embedded
   _embNoScroll,
   parent, // need to deallocate parent
   _numMacros,
   _macros,
   _expansions );

  embeddedX = _embeddedX;
  embeddedY = _embeddedY;
  embeddedW = w;
  embeddedH = h;
  embCenter = _embCenter;
  embSetSize = _embSetSize;
  embSizeOfs = _embSizeOfs;

  return stat;

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
  int _closeAllowed,
  int _isEmbedded,
  int _noScroll,
  Widget *_widgetToDeallocate,
  int _numMacros,
  char **_macros,
  char **_expansions ) {

int i, l, wPix, bPix;
Atom wm_delete_window;
char tmp[10];

  appCtx = ctx;

  autosaveTimer = 0;
  doAutoSave = 0;
  doClose = 0;
  doActiveClose = 0;
  waiting = 0;
  restoreTimer = 0;

  change = 0;
  changeSinceAutoSave = 0;
  exit_after_save = 0;

  noEdit = _noEdit;
  closeAllowed = _closeAllowed;
  isEmbedded = _isEmbedded;
  embNoScroll = _noScroll;
  widgetToDeallocate = _widgetToDeallocate;

  this->numMacros = _numMacros;

  actualNumMacros = _numMacros + 2;

  this->macros = new char *[actualNumMacros];
  this->expansions = new char *[actualNumMacros];

  crc = 0;
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

  this->crc = crc;

  // autocreate

  this->macros[actualNumMacros-2] = new char[strlen("!W")+1];
  strcpy( this->macros[actualNumMacros-2], "!W" );

  snprintf( tmp, 9, "%-lx", (unsigned long) this );

  this->expansions[actualNumMacros-2] = new char[strlen((char *) tmp)+1];
  strcpy( this->expansions[actualNumMacros-2], (char *) tmp );

  this->macros[actualNumMacros-1] = new char[strlen("!A")+1];
  strcpy( this->macros[actualNumMacros-1], "!A" );

  snprintf( tmp, 9, "%-lx", (unsigned long) this->appCtx );

  this->expansions[actualNumMacros-1] = new char[strlen((char *) tmp)+1];
  strcpy( this->expansions[actualNumMacros-1], (char *) tmp );

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
  gridShow = 0;
  gridSpacing = 10;
  oldGridSpacing = gridSpacing;
  orthogonal = 0;
  disableScroll = 0;
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
 
  embBg = bPix;

#ifndef ADD_SCROLLED_WIN
  if ( !parent ) {

    top = XtVaCreatePopupShell( "edm", topLevelShellWidgetClass,
     appCtx->apptop(),
     XmNmappedWhenManaged, False,
     XmNmwmDecorations, windowDecorations,
     XmNresizePolicy, XmRESIZE_NONE,
     NULL );

    drawWidget = XtVaCreateManagedWidget( "screen", xmDrawingAreaWidgetClass,
     top,
     XmNwidth, w,
     XmNheight, h,
     XmNmappedWhenManaged, False,
     XmNresizePolicy, XmRESIZE_NONE,
     NULL );

  }
  else {

    top = parent;

    if ( isEmbedded ) {

      XtVaGetValues( parent,
      XmNbackground, &embBg,
      NULL );

    }

    drawWidget = XtVaCreateWidget( "screen", xmDrawingAreaWidgetClass,
     top,
     XmNx, x,
     XmNy, y,
     XmNwidth, w,
     XmNheight, h,
     XmNmappedWhenManaged, False,
     XmNresizePolicy, XmRESIZE_NONE,
     XmNbackground, embBg,
     NULL );

  }

  scroll = 0;

#else
  if ( appCtx->useScrollBars ) {

    if ( !parent ) {

      //top = XtVaAppCreateShell( "edm", "edm", topLevelShellWidgetClass,
      // d,
      // XmNmappedWhenManaged, False,
      // XmNmwmDecorations, windowDecorations,
      // NULL );

      top = XtVaCreatePopupShell( "edm", topLevelShellWidgetClass,
       appCtx->apptop(),
       XmNmappedWhenManaged, False,
       XmNmwmDecorations, windowDecorations,
       NULL );

    }
    else {

      top = parent;

      if ( isEmbedded ) {

        XtVaGetValues( parent,
        XmNbackground, &embBg,
        NULL );

      }

    }

    {

      Dimension maxW = WidthOfScreen(XtScreen(top));
      Dimension maxH = HeightOfScreen(XtScreen(top));

      scroll = parent ? 0 : XtVaCreateManagedWidget( "screenscroll", xmScrolledWindowWidgetClass,
       top, 
       XmNwidth, w > maxW ? maxW : w,
       XmNheight, h > maxH ? maxH : h,
       /* avoid getting scrollbars just because of the viewport shadows
        * so we eliminate them. Ideally, all of this should happen in the
        * resource files but edm is not written properly :-O
        */
       XmNshadowThickness,0,
       XmNscrollingPolicy, XmAUTOMATIC,
       XmNscrollBarDisplayPolicy, XmAS_NEEDED,
       XmNbackground, embBg,
       NULL );

      drawWidget = XtVaCreateManagedWidget( "screen", xmDrawingAreaWidgetClass,
       scroll ? scroll : top,
       XmNresizePolicy, XmRESIZE_NONE,
       XmNx, parent ? x : 0,
       XmNy, parent ? y : 0,
       XmNwidth, w,
       XmNheight, h,
       XmNmappedWhenManaged, False,
       XmNbackground, embBg,
       NULL );

    }

  }
  else {

    if ( !parent ) {

      top = XtVaCreatePopupShell( "edm", topLevelShellWidgetClass,
       appCtx->apptop(),
       XmNmappedWhenManaged, False,
       XmNmwmDecorations, windowDecorations,
       XmNresizePolicy, XmRESIZE_NONE,
       NULL );

      drawWidget = XtVaCreateManagedWidget( "screen", xmDrawingAreaWidgetClass,
       top,
       XmNwidth, w,
       XmNheight, h,
       XmNmappedWhenManaged, False,
       XmNresizePolicy, XmRESIZE_NONE,
       NULL );

    }
    else {

      top = parent;

      if ( isEmbedded ) {

        XtVaGetValues( parent,
        XmNbackground, &embBg,
        NULL );

      }

      drawWidget = XtVaCreateWidget( "screen", xmDrawingAreaWidgetClass,
       top,
       XmNx, x,
       XmNy, y,
       XmNwidth, w,
       XmNheight, h,
       XmNmappedWhenManaged, False,
       XmNresizePolicy, XmRESIZE_NONE,
       XmNbackground, embBg,
       NULL );

    }

    scroll = 0;

  }
#endif

  executeWidget = drawWidget;

  if ( !parent ) {

    // This handles close on the window manager.

    wm_delete_window = XmInternAtom( XtDisplay(top), "WM_DELETE_WINDOW",
     False );

    XmAddWMProtocolCallback( top, wm_delete_window, awc_WMExit_cb,
     (XtPointer) this );

    XtVaSetValues( top, XmNdeleteResponse, XmDO_NOTHING, NULL );

    XtAddEventHandler( top,
     StructureNotifyMask, False,
     topWinEventHandler, (XtPointer) this );

  }

  XtAddEventHandler( drawWidget,
   KeyPressMask|KeyReleaseMask|ButtonPressMask|PointerMotionMask|
   ButtonReleaseMask|Button1MotionMask|
   Button2MotionMask|Button3MotionMask|ExposureMask, False,
   drawWinEventHandler, (XtPointer) this );

  windowState = AWC_COMPLETE_DEACTIVATE;

  return 1;

}

int activeWindowClass::createNodeForCrawler (
  appContextClass *ctx,
  char *filename
) {

tagClass tag;
FILE *f;
char objName[63+1], defName[255+1], tagName[255+1], val[4095+1],
 *gotOne;
int isCompound, stat, l;
objBindingClass obj;
activeGraphicListPtr cur;

  loadFailure = 1;
  tag.initLine();

  f = openAny( filename, "r" );
  if ( !f ) {
    fprintf( stderr, activeWindowClass_str208, filename );
    return( 1000 );
  }

#if 0
  head = new activeGraphicListType;
  head->flink = head;
  head->blink = head;
#endif

  readCommentsAndVersion( f );

  if ( major < 4 ) {

    fprintf( stderr, activeWindowClass_str206, this->fileName, major );
    fileClose( f );
    return( 1000 );

    stat = readUntilEndOfData( f ); // for forward compatibility
    if ( !( stat & 1 ) ) return stat; // memory leak here

    while ( !feof(f) ) {

      gotOne = fgets( objName, 63, f ); incLine();

      if ( gotOne ) {

        l = strlen(objName);
        if ( l > 63 ) l = 63;
        objName[l-1] = 0;  // discard \n

        cur = new activeGraphicListType;
        if ( !cur ) {
          fileClose( f );
          fprintf( stderr, activeWindowClass_str207 );
          return 1000;
        }
        cur->defExeFlink = NULL;
        cur->defExeBlink = NULL;

        cur->node = obj.createNew( objName );

        if ( cur->node ) {

          stat = cur->node->old_createFromFile( f, objName, this );
          if ( !( stat & 1 ) ) return stat; // memory leak here
          if (stat < 0) return stat;   // infinite recursion

          stat = readUntilEndOfData( f ); // for forward compatibility
          if ( !( stat & 1 ) ) return stat; // memory leak here

          cur->blink = head->blink;
          head->blink->flink = cur;
          head->blink = cur;
          cur->flink = head;

        }
        else {

          fileClose( f );
          fprintf( stderr, activeWindowClass_str209, line(),
           objName );
          return 1000;

        }

      }

    }

    //fileClose( f );
    //return 1000;

  }
  else {

  // read file and process each leading keyword
  tag.init();
  tag.loadR( "object", 63, objName );
  tag.loadR( "pvdef", 255, defName );
  tag.loadR( "forceLocalPvs" );

  gotOne = tag.getName( tagName, 255, f );

  while ( gotOne ) {

    //fprintf( stderr, "name = [%s]\n", tagName );

    if ( strcmp( tagName, "object" ) == 0 ) {

      tag.getValue( val, 4095, f, &isCompound );
      tag.decode( tagName, val, isCompound );

      // ==============================================================
      // Create object

      //fprintf( stderr, "objName = [%s]\n", objName );

      cur = new activeGraphicListType;
      if ( !cur ) {
        fileClose( f );
        fprintf( stderr, activeWindowClass_str207 );
        return 1000;
      }
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;

      cur->node = obj.createNew( objName );

      if ( cur->node ) {

        stat = cur->node->createFromFile( f, objName, this );
        if ( !( stat & 1 ) ) return stat;

        cur->blink = head->blink;
        head->blink->flink = cur;
        head->blink = cur;
        cur->flink = head;

      }
      else {

        // Discard all content up to "endObjectProperties"

        fprintf( stderr, activeWindowClass_str209, tag.line(),
         objName );

        tag.init();
        tag.loadR( "endObjectProperties", 63, objName );
        stat = tag.readTags( f, "endObjectProperties" );

        // Start looking for leading keywords again
        tag.init();
        tag.loadR( "object", 63, objName );
        tag.loadR( "pvdef", 255, defName );
        tag.loadR( "forceLocalPvs" );

      }

    }

    gotOne = tag.getName( tagName, 255, f );

  }

  }

  fileClose( f );

  loadFailure = 0;

  return 1;

}

void activeWindowClass::map ( void ) {

  XtMapWidget( drawWidget );
  if ( isEmbedded ) XtMapWidget( top );

}

void activeWindowClass::realize ( void ) {

  realize ( (int) 1 );

}

void activeWindowClass::realizeNoMap ( void ) {

  realize ( (int) 0 );

}

void activeWindowClass::realize (
  int doMap ) {

XmString str;
Widget pb;
objNameListPtr curObjNameNode;
char *oneObjName, *menuName;
popupBlockListPtr curBlockListNode;
int i, n;
Arg args[3];

  XtManageChild( drawWidget );
  if ( isEmbedded ) XtManageChild( top );
  XtRealizeWidget( top );
  XSetWindowColormap( d, XtWindow(top), appCtx->ci.getColorMap() );
  if ( doMap ) {
    XtMapWidget( drawWidget );
    if ( isEmbedded ) XtMapWidget( top );
  }
  setTitle();

  // create drawing popup menus

//===================================================================

  n = 0;
  XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
  b1OneSelectPopup = XmCreatePopupMenu( top, "b1oneselectmenu", args, n );

  chPd = XmCreatePulldownMenu( b1OneSelectPopup, "b1oneselectpulldown",
   NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str86 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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
  XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
  b1NoneSelectPopup = XmCreatePopupMenu( top, "b1noneselectmenu", args, n );

  grPd = XmCreatePulldownMenu( b1NoneSelectPopup, "b1noneselectpulldown",
   NULL, 0 );

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

    pb = XtVaCreateManagedWidget( "pb",
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

  mnPd = XmCreatePulldownMenu( b1NoneSelectPopup, "b1noneselectpulldown",
   NULL, 0 );

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

    pb = XtVaCreateManagedWidget( "pb",
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

  ctlPd = XmCreatePulldownMenu( b1NoneSelectPopup, "b1noneselectpulldown",
   NULL, 0 );

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

    pb = XtVaCreateManagedWidget( "pb",
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
  XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
  b2NoneSelectPopup = XmCreatePopupMenu( top, "b2noneselectmenu", args, n );

  str = XmStringCreateLocalized( activeWindowClass_str92 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

    setSchemePd = XmCreatePulldownMenu( b2NoneSelectPopup,
     "b2noneselectpulldown", NULL, 0 );

    str = XmStringCreateLocalized( activeWindowClass_str186 );

    setSchemeCb = XtVaCreateManagedWidget( "Select Scheme Set",
     xmCascadeButtonWidgetClass,
     b2NoneSelectPopup,
     XmNlabelString, str,
     XmNsubMenuId, setSchemePd,
     NULL );

    XmStringFree( str );

    str = XmStringCreateLocalized( "None" );

    pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

      pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

      if ( i == 0 ) {
        // init scheme set with first one
        strncpy( curSchemeSet, appCtx->schemeSetList[i], 63 );
      }

    }

  }


  str = XmStringCreateLocalized( activeWindowClass_str93 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str196 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SAVE_TO_PATH;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str94 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str222 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SET_PASTE_INDEX;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str96 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str220 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_TOGGLE_VIEW_DIMS;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str97 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

   pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


   str = XmStringCreateLocalized( activeWindowClass_str215 );

   pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
    b2NoneSelectPopup,
    XmNlabelString, str,
    NULL );

   XmStringFree( str );

   curBlockListNode = new popupBlockListType;
   curBlockListNode->block.w = pb;
   curBlockListNode->block.ptr = (void *) AWC_POPUP_INSERT_TEMPLATE;
   curBlockListNode->block.awo = this;

   curBlockListNode->blink = popupBlockHead->blink;
   popupBlockHead->blink->flink = curBlockListNode;
   popupBlockHead->blink = curBlockListNode;
   curBlockListNode->flink = popupBlockHead;

   XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
    (XtPointer) &curBlockListNode->block );


#if 0
   str = XmStringCreateLocalized( activeWindowClass_str99 );

   pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str210 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SHOW_MACROS;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str104 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  undoPb1 = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str192 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2NoneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  if ( !appCtx->epc.printStatusOK() ) {
    XtVaSetValues( pb, XmNsensitive, 0, NULL );
  }

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_PRINT;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str105 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

#ifdef ADD_SCROLLED_WIN
  {

    if ( appCtx->useScrollBars ) {

      str = XmStringCreateLocalized( activeWindowClass_str202 );
      pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
       b2NoneSelectPopup,
       XmNlabelString, str,
       NULL);

      XmStringFree(str);
      XtAddCallback( pb, XmNactivateCallback, b2ReleaseClip_cb, (XtPointer)this);

    }

  }
#endif


  str = XmStringCreateLocalized( activeWindowClass_str184 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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
  XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
  b2OneSelectPopup = XmCreatePopupMenu( top, "b2oneselectmenu", args, n );


  str = XmStringCreateLocalized( activeWindowClass_str92 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2OneSelectPopup,
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


  str = XmStringCreateLocalized( activeWindowClass_str106 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str222 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SET_PASTE_INDEX;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str111 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str194 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_COPY_GROUP_INFO;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str195 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_PASTE_GROUP_INFO;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  orientPd1 = XmCreatePulldownMenu( b2OneSelectPopup, "b2oneselectpulldown",
   NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str176 );

  orientCb1 = XtVaCreateManagedWidget( activeWindowClass_str176,
   xmCascadeButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, orientPd1,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str177 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  editPd1 = XmCreatePulldownMenu( b2OneSelectPopup, "b2oneselectpulldown",
   NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str140 );

  editCb1 = XtVaCreateManagedWidget( activeWindowClass_str140,
   xmCascadeButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, editPd1,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str141 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str224 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SAR;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str221 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2OneSelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_RECORD_DIMS;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseOneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str143 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  undoPb2 = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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
  XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
  b2ManySelectPopup = XmCreatePopupMenu( top, "b2manyselectmenu", args, n );


  str = XmStringCreateLocalized( activeWindowClass_str92 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2ManySelectPopup,
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


  str = XmStringCreateLocalized( activeWindowClass_str115 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str222 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SET_PASTE_INDEX;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseNoneSelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str120 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  orientPdM = XmCreatePulldownMenu( b2ManySelectPopup, "b2manyselectpulldown",
   NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str176 );

  orientCbM = XtVaCreateManagedWidget( activeWindowClass_str176,
   xmCascadeButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, orientPdM,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str177 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  alignPd = XmCreatePulldownMenu( b2ManySelectPopup, "b2manyselectpulldown",
   NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str122 );

  alignCb = XtVaCreateManagedWidget( activeWindowClass_str122,
   xmCascadeButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, alignPd,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str123 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  centerPd = XmCreatePulldownMenu( b2ManySelectPopup, "b2manyselectpulldown",
   NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str127 );

  centerCb = XtVaCreateManagedWidget( activeWindowClass_str127,
   xmCascadeButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, centerPd,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str128 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  sizePd = XmCreatePulldownMenu( b2ManySelectPopup, "b2manyselectpulldown",
   NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str131 );

  sizeCb = XtVaCreateManagedWidget( activeWindowClass_str131,
   xmCascadeButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, sizePd,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str132 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  distributePd = XmCreatePulldownMenu( b2ManySelectPopup,
   "b2manyselectpulldown", NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str135 );

  distributeCb = XtVaCreateManagedWidget( activeWindowClass_str135,
   xmCascadeButtonWidgetClass, b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, distributePd,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str136 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str190 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   distributePd,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr =
   (void *) AWC_POPUP_DISTRIBUTE_MIDPT_BOTH;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );



  editPdM = XmCreatePulldownMenu( b2ManySelectPopup, "b2manyselectpulldown",
   NULL, 0 );

  str = XmStringCreateLocalized( activeWindowClass_str140 );

  editCbM = XtVaCreateManagedWidget( activeWindowClass_str140,
   xmCascadeButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   XmNsubMenuId, editPdM,
   NULL );

  XmStringFree( str );

  str = XmStringCreateLocalized( activeWindowClass_str141 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  str = XmStringCreateLocalized( activeWindowClass_str224 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2ManySelectPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SAR;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseManySelect_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str143 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  undoPb3 = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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
  XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
  b2ExecutePopup = XmCreatePopupMenu( top, "b2executemenu", args, n );

  if ( !noEdit && closeAllowed ) {

    str = XmStringCreateLocalized( activeWindowClass_str145 );

    pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

    pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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
  else if ( !isEmbedded ) {

    str = XmStringCreateLocalized( activeWindowClass_str149 );

    pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  if ( !isEmbedded ) {

    str = XmStringCreateLocalized( activeWindowClass_str150 );

    pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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

  }


  str = XmStringCreateLocalized( activeWindowClass_str210 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2ExecutePopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_SHOW_MACROS;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str151 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  if ( !isEmbedded ) {

    str = XmStringCreateLocalized( activeWindowClass_str192 );

    pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
     b2ExecutePopup,
     XmNlabelString, str,
     NULL );

    XmStringFree( str );

    if ( !appCtx->epc.printStatusOK() ) {
      XtVaSetValues( pb, XmNsensitive, 0, NULL );
    }

    curBlockListNode = new popupBlockListType;
    curBlockListNode->block.w = pb;
    curBlockListNode->block.ptr = (void *) AWC_POPUP_PRINT;
    curBlockListNode->block.awo = this;

    curBlockListNode->blink = popupBlockHead->blink;
    popupBlockHead->blink->flink = curBlockListNode;
    popupBlockHead->blink = curBlockListNode;
    curBlockListNode->flink = popupBlockHead;

    XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
     (XtPointer) &curBlockListNode->block );

  }


  str = XmStringCreateLocalized( activeWindowClass_str201 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2ExecutePopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_DUMP_PVLIST;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
   (XtPointer) &curBlockListNode->block );


  str = XmStringCreateLocalized( activeWindowClass_str152 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
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


  if ( isEmbedded ) {

  str = XmStringCreateLocalized( activeWindowClass_str204 );

  pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
   b2ExecutePopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  curBlockListNode = new popupBlockListType;
  curBlockListNode->block.w = pb;
  curBlockListNode->block.ptr = (void *) AWC_POPUP_OPEN_SELF;
  curBlockListNode->block.awo = this;

  curBlockListNode->blink = popupBlockHead->blink;
  popupBlockHead->blink->flink = curBlockListNode;
  popupBlockHead->blink = curBlockListNode;
  curBlockListNode->flink = popupBlockHead;

  XtAddCallback( pb, XmNactivateCallback, b2ReleaseExecute_cb,
   (XtPointer) &curBlockListNode->block );

  }

//===================================================================

  n = 0;
  XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
  actionPopup = XmCreatePopupMenu( top, "actionmenu", args, n );


  for ( i=0; i<pvAction->numActions(); i++ ) {

    str = XmStringCreateLocalized( pvAction->getActionName( i ) );

    pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
     actionPopup,
     XmNlabelString, str,
     NULL );

    XmStringFree( str );

    curBlockListNode = new popupBlockListType;
    curBlockListNode->block.w = pb;
    curBlockListNode->block.ptr = (void *) i;
    curBlockListNode->block.awo = this;

    curBlockListNode->blink = popupBlockHead->blink;
    popupBlockHead->blink->flink = curBlockListNode;
    popupBlockHead->blink = curBlockListNode;
    curBlockListNode->flink = popupBlockHead;

    XtAddCallback( pb, XmNactivateCallback, action_cb,
     (XtPointer) &curBlockListNode->block );

  }

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
  drawGc.setCI( ci );

  fgColor = ci->pixIndex( BlackPixel( d, DefaultScreen(d) ) );
  bgColor = ci->pixIndex( WhitePixel( d, DefaultScreen(d) ) );

  drawGc.setFG( ci->pix(fgColor) );
  drawGc.setBG( ci->pix(bgColor) );
  drawGc.setBaseBG( ci->pix(bgColor) );

  if ( isEmbedded ) {
    drawGc.setBG( embBg );
    drawGc.setBaseBG( embBg );
  }

  executeGc.create( executeWidget );
  executeGc.setCI( ci );
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

Widget activeWindowClass::actualTopWidgetId ( void ) {

activeWindowClass *aw, *prevAw;

  prevAw = NULL;
  aw = this;
  while ( aw ) {
    prevAw = aw;
    aw = aw->parent;
  }

  if ( prevAw ) {
    return prevAw->top;
  }

  return (Widget) NULL;

}

activeWindowClass *activeWindowClass::actualTopObject ( void ) {

activeWindowClass *aw, *prevAw;

  prevAw = NULL;
  aw = this;
  while ( aw ) {
    prevAw = aw;
    aw = aw->parent;
  }

  if ( prevAw ) {
    return prevAw;
  }

  return this;

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

  if ( noEdit ) {
    change = 0;
    changeSinceAutoSave = 0;
    return;
  }

  change = 1;

  if ( !changeSinceAutoSave ) {
    changeSinceAutoSave = 1;
    //stat = sys_get_datetime_string( 31, str );
    if ( autosaveTimer ) {
      XtRemoveTimeOut( autosaveTimer );
      autosaveTimer = 0;
    }
    autosaveTimer = appAddTimeOut( appCtx->appContext(),
     300000, acw_autosave, this );
     //30000, acw_autosave, this );
    //fprintf( stderr, "[%s] %s - add autosave timer\n", str, fileName );
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
    Strncat( oneFileName, fName, 255 );
  }

  if ( strlen(oneFileName) > strlen(".scheme") ) {
    if ( strcmp( &oneFileName[strlen(oneFileName)-strlen(".scheme")], ".scheme" ) != 0 ) {
      Strncat( oneFileName, ".scheme", 255 );
    }
  }
  else {
    Strncat( oneFileName, ".scheme", 255 );
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
    Strncat( oneFileName, fName, 255 );
  }

  if ( strlen(oneFileName) > strlen(".scheme") ) {
    if ( strcmp( &oneFileName[strlen(oneFileName)-strlen(".scheme")], ".scheme" ) != 0 ) {
      Strncat( oneFileName, ".scheme", 255 );
    }
  }
  else {
    Strncat( oneFileName, ".scheme", 255 );
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

int stat, found, min, max, num, count;
char tmp[530+1], spec[520+1], name[511+1], ext[511+1], verstr[10+1],
 *tk, *ctx, *nonInt;

char *envPtr;
int maxver = 1;

  envPtr = getenv( environment_str18 );

  if ( envPtr ) {

    if ( !strcasecmp( envPtr, activeWindowClass_str211 ) ) {
      maxver = -1;
    }
    else {

      strncpy( tmp, envPtr, 511 );
      tmp[511] = 0;
      num = strtol( tmp, &nonInt, 10 );
      if ( !nonInt || !strcmp( nonInt, "" ) ) {
	maxver = num;
      }

      if ( maxver < 1 ) maxver = 1;

    }

  }

  strncpy( spec, fname, 511 );
  Strncat( spec, "-*", 520 );
  min = max = count = 0;
  getFirstFileNameExt( spec, 511, name, 511, ext, &found );
  while ( found ) {
    ctx = NULL;
    tk = strtok_r( ext, "-", &ctx );
    tk = strtok_r( NULL, "-", &ctx );
    if ( tk ) {
      num = strtol( tk, &nonInt, 10 );
      if ( !nonInt || !strcmp( nonInt, "" ) ) {
        count++;
        if ( min ) {
          if ( min > num ) min = num;
	}
	else {
	  min = num;
	}
	if ( max < num ) max = num;
      }
    }
    getNextFileNameExt( spec, 511, name, 511, ext, &found );
  }

  if ( maxver != 1 ) {

    if ( ( maxver != -1 ) && count >= maxver ) {

      strncpy( tmp, fname, 510 ); // leave room for version info
      tmp[510] = 0;
      snprintf( verstr, 10, "-%-d", min ); 
      Strncat( tmp, verstr, 530 );

      if ( fileExists( tmp ) ) {
        stat = unlink( tmp );
        if ( stat ) return 2; // error
      }

    }

    num = max+1;
    strncpy( tmp, fname, 510 ); // leave room for version info
    tmp[510] = 0;
    snprintf( verstr, 10, "-%-d", num ); 
    Strncat( tmp, verstr, 530 );

    if ( fileExists( fname ) ) {
      stat = rename( fname, tmp );
      if ( stat ) return 4; // error
    }

  }
  else {

    strncpy( tmp, fname, 510 ); // leave room for ~
    Strncat( tmp, "~", 511 );

    if ( fileExists( tmp ) ) {
      stat = unlink( tmp );
      if ( stat ) return 2; // error
    }
    if ( fileExists( fname ) ) {
      stat = rename( fname, tmp );
      if ( stat ) return 4; // error
    }

  }

  return 1; // success

}

int activeWindowClass::save (
  char *fName ) {

int stat;

  storeFileName( fName );
  stat = genericSave( fName, 1, 1, 1 );

  return stat;

}

int activeWindowClass::saveNoChange (
  char *fName ){

int stat;

  stat = genericSave( fName, 0, 0, 0 );

  return stat;

}

int activeWindowClass::old_genericSave (
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
    Strncat( oneFileName, fName, 255 );
  }

  if ( appendExtensionFlag ) {

    //if ( strlen(oneFileName) > strlen(".edl") ) {
    if ( strlen(oneFileName) > strlen(activeWindowClass::defExt()) ) {
      if (
        //strcmp( &oneFileName[strlen(oneFileName)-strlen(".edl")],
        // ".edl" ) != 0 ) {
        strcmp( &oneFileName[strlen(oneFileName)-strlen(activeWindowClass::defExt())],
         activeWindowClass::defExt() ) != 0 ) {
        //Strncat( oneFileName, ".edl", 255 );
        Strncat( oneFileName, activeWindowClass::defExt(), 255 );
      }
    }
    else {
      //Strncat( oneFileName, ".edl", 255 );
      Strncat( oneFileName, activeWindowClass::defExt(), 255 );
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

  this->old_saveWin( f );
  fprintf( f, "<<<E~O~D>>>\n" );

  cur = head->flink;
  while ( cur != head ) {
    if ( !cur->node->deleteRequest ) {
      if ( strcmp( cur->node->getCreateParam(), "" ) == 0 ) {
        fprintf( f, "%s\n", cur->node->objName() );
      }
      else {
        fprintf( f, "%s:%s\n", cur->node->objName(),
         cur->node->getCreateParam() );
      }
      cur->node->old_save( f );
      fprintf( f, "<<<E~O~D>>>\n" );
    }
    cur = cur->flink;
  }

  fclose( f );

  if ( resetChangeFlag ) this->setUnchanged();

  return 1;

}

int activeWindowClass::genericSave (
  char *fName,
  int resetChangeFlag,
  int appendExtensionFlag,
  int backupFlag ) {

FILE *f;
char *gotOne, oneFileName[255+1], fullName[255+1], *description;
activeGraphicListPtr cur;
char msg[79+1];
int stat;
tagClass tag;

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
    Strncat( oneFileName, fName, 255 );
  }

  if ( appendExtensionFlag ) {

    //if ( strlen(oneFileName) > strlen(".edl") ) {
    if ( strlen(oneFileName) > strlen(activeWindowClass::defExt()) ) {
      if (
	//strcmp( &oneFileName[strlen(oneFileName)-strlen(".edl")],
        // ".edl" ) != 0 ) {
        strcmp( &oneFileName[strlen(oneFileName)-strlen(activeWindowClass::defExt())],
         activeWindowClass::defExt() ) != 0 ) {
        //Strncat( oneFileName, ".edl", 255 );
        Strncat( oneFileName, activeWindowClass::defExt(), 255 );
      }
    }
    else {
      //Strncat( oneFileName, ".edl", 255 );
      Strncat( oneFileName, activeWindowClass::defExt(), 255 );
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

  cur = head->flink;
  while ( cur != head ) {
    if ( !cur->node->deleteRequest && !cur->node->hidden ) {
      if ( strcmp( cur->node->getCreateParam(), "" ) == 0 ) {
        strncpy( fullName, cur->node->objName(), 255 );
        description = obj.getNameFromClass( fullName );
        fprintf( f, "# (%s)\n", description );
        fprintf( f, "object %s\n", cur->node->objName() );
        if ( tag.genDoc() ) {
          fprintf( stderr, "# (%s)\n", description );
          fprintf( stderr, "object %s\n", cur->node->objName() );
	      }
      }
      else {
        strncpy( fullName, cur->node->objName(), 255 );
        Strncat( fullName, ":", 255 );
        Strncat( fullName, cur->node->getCreateParam(), 255 );
        description = obj.getNameFromClass( fullName );
        if ( !description ) {
          strcpy( description, "Unknown object" );
	      }
        fprintf( f, "# (%s)\n", description );
        fprintf( f, "object %s:%s\n", cur->node->objName(),
         cur->node->getCreateParam() );
        if ( tag.genDoc() ) {
          fprintf( stderr, "# (%s)\n", description );
          fprintf( stderr, "object %s:%s\n", cur->node->objName(),
           cur->node->getCreateParam() );
	      }
        Strncat( fullName, cur->node->getCreateParam(), 255 );
      }
      cur->node->save( f );
    }
    cur = cur->flink;
  }

  fclose( f );

  if ( resetChangeFlag ) this->setUnchanged();

  return 1;

}

int activeWindowClass::old_loadGeneric (
  int x,
  int y,
  int setPosition ) {

FILE *f;
activeGraphicListPtr cur, next;
char *gotOne, name[63+1];
int stat, l;
char msg[79+1];
Widget clipWidget, hsbWidget, vsbWidget;

  loadFailure = 1;

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

  if ( setPosition ) {
    stat = this->old_loadWin( f, x, y );
  }
  else {
    stat = this->old_loadWin( f );
  }
  if ( !( stat & 1 ) ) return stat; // memory leak here

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
        fileClose( f );
        appCtx->postMessage( activeWindowClass_str157 );
        return 0;
      }
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;

      cur->node = obj.createNew( name );

      if ( cur->node ) {

	fprintf( stderr, "call old_createFromFile\n" );
        stat = cur->node->old_createFromFile( f, name, this );
        if ( !( stat & 1 ) ) return stat; // memory leak here

        stat = readUntilEndOfData( f ); // for forward compatibility
        if ( !( stat & 1 ) ) return stat; // memory leak here

        cur->blink = head->blink;
        head->blink->flink = cur;
        head->blink = cur;
        cur->flink = head;

      }
      else {
        fileClose( f );
        sprintf( msg, activeWindowClass_str158, line(),
         name );
        appCtx->postMessage( msg );
        return 0;
      }

    }

  }

  fileClose( f );

  if ( scroll ) {

    XtVaSetValues( scroll,
     XmNtopShadowColor, ci->pix(defaultTopShadowColor),
     XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
     XmNborderColor, ci->pix(bgColor),
     XmNhighlightColor, ci->pix(bgColor),
     XmNforeground, ci->pix(bgColor),
     XmNbackground, ci->pix(bgColor),
     NULL );

    XtVaGetValues( scroll,
     XmNclipWindow, &clipWidget,
     XmNhorizontalScrollBar, &hsbWidget,
     XmNverticalScrollBar, &vsbWidget,
     NULL );

    if ( clipWidget ) {
      XtVaSetValues( clipWidget,
        XmNtopShadowColor, ci->pix(defaultTopShadowColor),
        XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
        XmNborderColor, ci->pix(bgColor),
        XmNhighlightColor, ci->pix(bgColor),
        XmNforeground, ci->pix(bgColor),
        XmNbackground, ci->pix(bgColor),
       NULL );
    }

    if ( hsbWidget ) {
      XtVaSetValues( hsbWidget,
        XmNtopShadowColor, ci->pix(defaultTopShadowColor),
        XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
        XmNborderColor, ci->pix(bgColor),
        XmNhighlightColor, ci->pix(bgColor),
        XmNforeground, ci->pix(bgColor),
        XmNbackground, ci->pix(bgColor),
        XmNtroughColor, ci->pix(bgColor),
        NULL );
    }

    if ( vsbWidget ) {
      XtVaSetValues( vsbWidget,
        XmNtopShadowColor, ci->pix(defaultTopShadowColor),
        XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
        XmNborderColor, ci->pix(bgColor),
        XmNhighlightColor, ci->pix(bgColor),
        XmNforeground, ci->pix(bgColor),
        XmNbackground, ci->pix(bgColor),
        XmNtroughColor, ci->pix(bgColor),
        NULL );
    }

  }

  showName = 0;

  setTitle();

  exit_after_save = 0;

  loadFailure = 0;

  return 1;

}

int activeWindowClass::old_load ( void ) {

  return old_loadGeneric( 0, 0, 0 );

}

int activeWindowClass::old_load (
  int x,
  int y ) {

  return old_loadGeneric( x, y, 1 );

}

int activeWindowClass::loadDummy (
  int x,
  int y,
  int setPosition ) {

FILE *f = NULL;
activeGraphicListPtr cur, next;
int stat;
Widget clipWidget, hsbWidget, vsbWidget;

tagClass tag;

  loadFailure = 1;

  tag.initLine();

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

  stat = this->loadWinDummy( f, x, y, setPosition );

  this->setUnchanged();

  if ( scroll ) {

    XtVaSetValues( scroll,
     XmNtopShadowColor, ci->pix(defaultTopShadowColor),
     XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
     XmNborderColor, ci->pix(bgColor),
     XmNhighlightColor, ci->pix(bgColor),
     XmNforeground, ci->pix(bgColor),
     XmNbackground, ci->pix(bgColor),
     NULL );

    XtVaGetValues( scroll,
     XmNclipWindow, &clipWidget,
     XmNhorizontalScrollBar, &hsbWidget,
     XmNverticalScrollBar, &vsbWidget,
     NULL );

    if ( clipWidget ) {
      XtVaSetValues( clipWidget,
        XmNtopShadowColor, ci->pix(defaultTopShadowColor),
        XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
        XmNborderColor, ci->pix(bgColor),
        XmNhighlightColor, ci->pix(bgColor),
        XmNforeground, ci->pix(bgColor),
        XmNbackground, ci->pix(bgColor),
       NULL );
    }

    if ( hsbWidget ) {
      XtVaSetValues( hsbWidget,
        XmNtopShadowColor, ci->pix(defaultTopShadowColor),
        XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
        XmNborderColor, ci->pix(bgColor),
        XmNhighlightColor, ci->pix(bgColor),
        XmNforeground, ci->pix(bgColor),
        XmNbackground, ci->pix(bgColor),
        XmNtroughColor, ci->pix(bgColor),
        NULL );
    }

    if ( vsbWidget ) {
      XtVaSetValues( vsbWidget,
        XmNtopShadowColor, ci->pix(defaultTopShadowColor),
        XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
        XmNborderColor, ci->pix(bgColor),
        XmNhighlightColor, ci->pix(bgColor),
        XmNforeground, ci->pix(bgColor),
        XmNbackground, ci->pix(bgColor),
        XmNtroughColor, ci->pix(bgColor),
        NULL );
    }

  }

  showName = 0;

  setTitle();

  exit_after_save = 0;

  loadFailure = 0;

  return 1;

}

void activeWindowClass::deleteTemplateMacros ( void ) {

int i;

  for ( i=0; i<numTemplateMacros; i++ ) {
    if ( templateMacros[i] ) delete[] templateMacros[i];
    if ( templateExpansions[i] ) delete[] templateExpansions[i];
  }
  delete templateMacros;
  templateMacros = NULL;
  delete templateExpansions;
  templateExpansions = NULL;

}

int activeWindowClass::getTemplateMacros ( void ) {

tagClass tag;
FILE *f;
int i, ii, n, winMajor, winMinor, winRelease;
char saveParams[AWC_MAXTMPLPARAMS][AWC_TMPLPARAMSIZE+1];

  numTemplateMacros = 0;

  f = this->openAnyTemplate( fileNameForSym, "r" );
  if ( !f ) {
    return 0;
  }

  // save template params
  for ( i=0; i<AWC_MAXTMPLPARAMS; i++ ) {
    strcpy( saveParams[i], paramValue[i] );
  }

  // the next func reads template params into bufParamValue
  discardWinLoadData( f, &winMajor, &winMinor, &winRelease );

  // copy buf
  for ( i=0; i<AWC_MAXTMPLPARAMS; i++ ) {
    strcpy( paramValue[i], bufParamValue[i] );
  }

  fclose( f );

  n = 0;
  for ( i=0; i<AWC_MAXTMPLPARAMS; i++ ) {
    if ( !blank(paramValue[i]) ) {
      n++;
    }
  }

  numTemplateMacros = n;
  templateMacros = (char **) calloc( n, sizeof( char *) );
  templateExpansions = (char **) calloc( n, sizeof( char *) );

  for ( i=0, ii=0; i<AWC_MAXTMPLPARAMS; i++ ) {
    if ( !blank(paramValue[i]) ) {
      if ( ii < n ) {
        templateMacros[ii] = new char[strlen(paramValue[i])+1];
        strcpy( templateMacros[ii], paramValue[i] );
        templateExpansions[ii] = new char[AWC_TMPLPARAMSIZE+1];
        strcpy( templateExpansions[ii], "" );
      }
      ii++;
    }
  }

  // restore template params
  for ( i=0; i<AWC_MAXTMPLPARAMS; i++ ) {
    strcpy( paramValue[i], saveParams[i] );
  }

  return 1;

}

int activeWindowClass::loadTemplate (
  int x,
  int y,
  char *fname ) {

FILE *f;
activeGraphicListPtr cur;
char *gotOne, tagName[255+1], objName[63+1], val[4095+1],
 defName[255+1];
int stat;
char msg[79+1];
int winMajor, winMinor, winRelease;

int isCompound;
tagClass tag;

  tag.initLine();

  // set select list empty

  selectedHead->selFlink = selectedHead;
  selectedHead->selBlink = selectedHead;

  // read in file
  f = this->openAnyTemplate( fname, "r" );
  if ( !f ) {
    sprintf( msg, activeWindowClass_str156, this->fileName );
    appCtx->postMessage( msg );
    return 0;
  }

  this->setChanged();

  discardWinLoadData( f, &winMajor, &winMinor, &winRelease );

  if ( winMajor > AWC_MAJOR_VERSION ) {
    appCtx->postMessage( activeWindowClass_str191 );
    return 0;
  }

  if ( winMajor < 4 ) {

    appCtx->postMessage( activeWindowClass_str191 );
    return 0;

  }
  else {

    // read file and process each leading keyword
    tag.init();
    tag.loadR( "object", 63, objName );
    tag.loadR( "pvdef", 255, defName );
    tag.loadR( "forceLocalPvs" );

    gotOne = tag.getName( tagName, 255, f );

    while ( gotOne ) {

      //fprintf( stderr, "name = [%s]\n", tagName );

      if ( strcmp( tagName, "object" ) == 0 ) {

        tag.getValue( val, 4095, f, &isCompound );
        tag.decode( tagName, val, isCompound );

        // ==============================================================
        // Create object

        //fprintf( stderr, "objName = [%s]\n", objName );

        cur = new activeGraphicListType;
        if ( !cur ) {
          fileClose( f );
          appCtx->postMessage(
           activeWindowClass_str157 );
          return 0;
        }
        cur->defExeFlink = NULL;
        cur->defExeBlink = NULL;

        cur->node = obj.createNew( objName );

        if ( cur->node ) {

          stat = cur->node->createFromFile( f, objName, this );
          if ( !( stat & 1 ) ) {
            return stat; // memory leak here
	        }

          cur->node->move( x, y );
          cur->node->moveSelectBox( x, y );

	        cur->node->expandTemplate( numTemplateMacros,
	        templateMacros, templateExpansions );

          cur->blink = head->blink;
          head->blink->flink = cur;
          head->blink = cur;
          cur->flink = head;

	  // select
          cur->node->setSelected();
          cur->selBlink = selectedHead->selBlink;
          selectedHead->selBlink->selFlink = cur;
          selectedHead->selBlink = cur;
          cur->selFlink = selectedHead;

        }
        else {

	  // Discard all content up to "endObjectProperties"

          sprintf( msg, activeWindowClass_str158, tag.line(),
           objName );
          appCtx->postMessage( msg );

          tag.init();
          tag.loadR( "endObjectProperties", 63, objName );
          stat = tag.readTags( f, "endObjectProperties" );

	  // Start looking for leading keywords again
          tag.init();
          tag.loadR( "object", 63, objName );
          tag.loadR( "pvdef", 255, defName );
          tag.loadR( "forceLocalPvs" );

        }

        // ===================================

        gotOne = tag.getName( tagName, 255, f );

      }
      else if ( strcmp( tagName, "pvdef" ) == 0 ) {

	// discard these

        tag.getValue( val, 4095, f, &isCompound );
        tag.decode( tagName, val, isCompound );

        gotOne = tag.getName( tagName, 255, f );

      }
      else if ( strcmp( tagName, "forceLocalPvs" ) == 0 ) {

	// ignore this

        gotOne = tag.getName( tagName, 255, f );

      }
      else {

        fprintf( stderr, "Unknown tag name: [%s]\n", tagName );
        gotOne = NULL;

      }

    }

  }

  fileClose( f );

  return 1;

}

int activeWindowClass::loadGeneric (
  int x,
  int y,
  int setPosition ) {

FILE *f;
activeGraphicListPtr cur, next;
char *gotOne, name[63+1], tagName[255+1], objName[63+1], val[4095+1],
 defName[255+1];
int stat, l;
char msg[79+1];
Widget clipWidget, hsbWidget, vsbWidget;
pvDefPtr pvDefCur;

int isCompound;
tagClass tag;

  loadFailure = 1;

  tag.initLine();

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
    if ( isEmbedded ) {
      return loadDummy( x, y, setPosition );
    }
    return 0;
  }

  this->setUnchanged();

  readCommentsAndVersion( f );

  if ( major > AWC_MAJOR_VERSION ) {
    appCtx->postMessage( activeWindowClass_str191 );
    if ( isEmbedded ) {
      return loadDummy( x, y, setPosition );
    }
    return 0;
  }

  if ( major < 4 ) {

    if ( setPosition ) {
      stat = this->old_loadWin( f, x, y );
    }
    else {
      stat = this->old_loadWin( f );
    }
    if ( !( stat & 1 ) ) {
      if ( isEmbedded ) {
        return loadDummy( x, y, setPosition );
      }
      return stat; // memory leak here
    }

    stat = readUntilEndOfData( f ); // for forward compatibility
    if ( !( stat & 1 ) ) {
      if ( isEmbedded ) {
        return loadDummy( x, y, setPosition );
      }
      return stat; // memory leak here
    }

    while ( !feof(f) ) {

      gotOne = fgets( name, 63, f ); incLine();

      if ( gotOne ) {

        l = strlen(name);
        if ( l > 63 ) l = 63;
        name[l-1] = 0;  // discard \n

        cur = new activeGraphicListType;
        if ( !cur ) {
          fileClose( f );
          appCtx->postMessage( activeWindowClass_str157 );
          if ( isEmbedded ) {
            return loadDummy( x, y, setPosition );
          }
          return 0;
        }
        cur->defExeFlink = NULL;
        cur->defExeBlink = NULL;

        cur->node = obj.createNew( name );

        if ( cur->node ) {

          stat = cur->node->old_createFromFile( f, name, this );
          if ( !( stat & 1 ) ) {
            if ( isEmbedded ) {
              return loadDummy( x, y, setPosition );
            }
            return stat; // memory leak here
	  }

          stat = readUntilEndOfData( f ); // for forward compatibility
          if ( !( stat & 1 ) ) {
            if ( isEmbedded ) {
              return loadDummy( x, y, setPosition );
            }
            return stat; // memory leak here
	  }

          cur->blink = head->blink;
          head->blink->flink = cur;
          head->blink = cur;
          cur->flink = head;

        }
        else {
          fileClose( f );
          sprintf( msg, activeWindowClass_str158, line(),
           name );
          appCtx->postMessage( msg );
          if ( isEmbedded ) {
            return loadDummy( x, y, setPosition );
          }
          return 0;
        }

      }

    }

  }
  else {

    if ( setPosition ) {
      stat = this->loadWin( f, x, y );
    }
    else {
      stat = this->loadWin( f );
    }
    if ( !( stat & 1 ) ) {
      if ( isEmbedded ) {
        return loadDummy( x, y, setPosition );
      }
      return stat; // memory leak here
    }

    if ( isEmbedded ) {

      // Create internal, hidden related display object

      strncpy( tagName, prefix, 255 );
      Strncat( tagName, displayName, 255 );
      Strncat( tagName, postfix, 255 );

      cur = new activeGraphicListType;
      if ( !cur ) {
        fileClose( f );
        appCtx->postMessage(
         activeWindowClass_str157 );
        if ( isEmbedded ) {
          return loadDummy( x, y, setPosition );
        }
        return 0;
      }
      cur->defExeFlink = NULL;
      cur->defExeBlink = NULL;

      cur->node = obj.createNew( "relatedDisplayClass" );

      if ( cur->node ) {

        // tagName is related display filename
        stat = cur->node->createSpecial( tagName, this );
        if ( stat & 1 ) { // else memory leak here
          internalRelatedDisplay = cur->node;
          cur->blink = head->blink;
          head->blink->flink = cur;
          head->blink = cur;
          cur->flink = head;
        }

        cur->node->hidden = 1; // make hidden

      }

    }

    if ( invalidFile ) {
      bgColor = invalidBgColor;
      drawGc.setBaseBG( ci->pix(bgColor) );
    }

    // read file and process each leading keyword
    tag.init();
    tag.loadR( "object", 63, objName );
    tag.loadR( "pvdef", 255, defName );
    tag.loadR( "forceLocalPvs" );

    gotOne = tag.getName( tagName, 255, f );

    while ( gotOne ) {

      //fprintf( stderr, "name = [%s]\n", tagName );

      if ( strcmp( tagName, "object" ) == 0 ) {

        tag.getValue( val, 4095, f, &isCompound );
        tag.decode( tagName, val, isCompound );

        // ==============================================================
        // Create object

        //fprintf( stderr, "objName = [%s]\n", objName );

        cur = new activeGraphicListType;
        if ( !cur ) {
          fileClose( f );
          appCtx->postMessage(
           activeWindowClass_str157 );
          if ( isEmbedded ) {
            return loadDummy( x, y, setPosition );
          }
          return 0;
        }
        cur->defExeFlink = NULL;
        cur->defExeBlink = NULL;

        cur->node = obj.createNew( objName );

        if ( cur->node ) {

          stat = cur->node->createFromFile( f, objName, this );
          if ( !( stat & 1 ) ) {
            if ( isEmbedded ) {
              return loadDummy( x, y, setPosition );
            }
            return stat; // memory leak here
	  }

          cur->blink = head->blink;
          head->blink->flink = cur;
          head->blink = cur;
          cur->flink = head;

        }
        else {

          //fileClose( f );

	  // Discard all content up to "endObjectProperties"

          sprintf( msg, activeWindowClass_str158, tag.line(),
           objName );
          appCtx->postMessage( msg );

          tag.init();
          tag.loadR( "endObjectProperties", 63, objName );
          stat = tag.readTags( f, "endObjectProperties" );

	  // Start looking for leading keywords again
          tag.init();
          tag.loadR( "object", 63, objName );
          tag.loadR( "pvdef", 255, defName );
          tag.loadR( "forceLocalPvs" );

        }

        // ===================================

        gotOne = tag.getName( tagName, 255, f );

      }
      else if ( strcmp( tagName, "pvdef" ) == 0 ) {

        tag.getValue( val, 4095, f, &isCompound );
        tag.decode( tagName, val, isCompound );

	//fprintf( stderr, "pv = [%s]\n", defName );
        pvDefCur = new pvDefType;
        pvDefCur->def = new char[strlen(defName)+1];
        strcpy( pvDefCur->def, defName );
        pvDefTail->flink = pvDefCur;
        pvDefTail = pvDefCur;
        pvDefTail->flink = NULL;

        gotOne = tag.getName( tagName, 255, f );

      }
      else if ( strcmp( tagName, "forceLocalPvs" ) == 0 ) {

	forceLocalPvs = 1;
	//fprintf( stderr, "force local pvs\n" );

        gotOne = tag.getName( tagName, 255, f );

      }
      else {

        fprintf( stderr, "Unknown tag name: [%s]\n", tagName );
        gotOne = NULL;

      }

    }

  }

  fileClose( f );

  if ( scroll ) {

    XtVaSetValues( scroll,
     XmNtopShadowColor, ci->pix(defaultTopShadowColor),
     XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
     XmNborderColor, ci->pix(bgColor),
     XmNhighlightColor, ci->pix(bgColor),
     XmNforeground, ci->pix(bgColor),
     XmNbackground, ci->pix(bgColor),
     NULL );

    XtVaGetValues( scroll,
     XmNclipWindow, &clipWidget,
     XmNhorizontalScrollBar, &hsbWidget,
     XmNverticalScrollBar, &vsbWidget,
     NULL );

    if ( clipWidget ) {
      XtVaSetValues( clipWidget,
        XmNtopShadowColor, ci->pix(defaultTopShadowColor),
        XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
        XmNborderColor, ci->pix(bgColor),
        XmNhighlightColor, ci->pix(bgColor),
        XmNforeground, ci->pix(bgColor),
        XmNbackground, ci->pix(bgColor),
       NULL );
    }

    if ( hsbWidget ) {
      XtVaSetValues( hsbWidget,
        XmNtopShadowColor, ci->pix(defaultTopShadowColor),
        XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
        XmNborderColor, ci->pix(bgColor),
        XmNhighlightColor, ci->pix(bgColor),
        XmNforeground, ci->pix(bgColor),
        XmNbackground, ci->pix(bgColor),
        XmNtroughColor, ci->pix(bgColor),
        NULL );
    }

    if ( vsbWidget ) {
      XtVaSetValues( vsbWidget,
        XmNtopShadowColor, ci->pix(defaultTopShadowColor),
        XmNbottomShadowColor, ci->pix(defaultBotShadowColor),
        XmNborderColor, ci->pix(bgColor),
        XmNhighlightColor, ci->pix(bgColor),
        XmNforeground, ci->pix(bgColor),
        XmNbackground, ci->pix(bgColor),
        XmNtroughColor, ci->pix(bgColor),
        NULL );
    }

  }

  showName = 0;

  setTitle();

  exit_after_save = 0;

  loadFailure = 0;

  return 1;

}

int activeWindowClass::load ( void ) {

  return loadGeneric( 0, 0, 0 );

}

int activeWindowClass::load (
  int x,
  int y ) {

  return loadGeneric( x, y, 1 );

}

int activeWindowClass::import ( void ) {

FILE *f;
activeGraphicListPtr cur, next;
char *gotOne, name[63+1];
int stat, l;
char msg[79+1];

  loadFailure = 1;

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

  // change file extension to .edl (default one)
  l = strlen(this->fileName);
  if ( l > 4 ) {
    if ( strcmp( &this->fileName[l-4], ".xch" ) == 0 ) {
      //strcpy( &this->fileName[l-4], ".edl" );
      strcpy( &this->fileName[l-4], activeWindowClass::defExt() );
    }
  }

  showName = 0;

  setTitle();

  exit_after_save = 0;

  loadFailure = 0;

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

short sx = _x;
short sy = _y;
short sw = _w;
short sh = _h;
XRectangle xR = { sx, sy, sw, sh };
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

int activeWindowClass::executeMux ( void ) {

int pass, opStat, stat, nTries, btnUp, btnDown, btnDrag, btnFocus;
activeGraphicListPtr cur;
btnActionListPtr curBtn;

  // each pass must complete successfully in approx 10 seconds

  for ( pass=1; pass<gMaxExecutePasses; pass++ ) {

    nTries = 200;
    do {

      opStat = 1;

      cur = head->flink;
      while ( cur != head ) {

        if ( cur->node->isMux() ) {
          cur->node->initEnable();
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

int pass, opStat, stat, nTries, btnUp, btnDown, btnDrag, btnFocus, cnt,
 numSubObjects;
activeGraphicListPtr cur, cur1;
btnActionListPtr curBtn;
int numMuxMacros;
char **muxMacro, **muxExpansion;
char callbackName[63+1];
pvDefPtr pvDefCur;
char *envPtr;

  if ( dimDialog ) {
    viewDims = 0;
    if ( dimDialog->dialogIsPoppedUp() ) dimDialog->popdown();
  }

  initCopy();

  windowState = AWC_START_EXECUTE;

  if ( bgPixmapFlag == AWC_BGPIXMAP_NEVER ) {
    usePixmap = 0;
  }
  else if ( bgPixmapFlag == AWC_BGPIXMAP_ALWAYS ) {
    usePixmap = 1;
  }
  else {
    usePixmap = -1;
  }

  if ( usePixmap == -1 ) {
    envPtr = getenv( environment_str23 );
    if ( envPtr ) {
      usePixmap = 1;
    }
    else {
      usePixmap = 0;
    }
  }

  if ( usePixmap ) {

    needFullCopy = 0;

    if ( bgPixmap ) {
      if ( ( w != pixmapW ) || ( h != pixmapH ) ) {
        XFreePixmap( d, bgPixmap );
        bgPixmap = (Pixmap) NULL;
        pixmapW = pixmapH = -1;
      }
    }

    if ( !bgPixmap ) {
      if ( ( w > 0 ) && ( h > 0 ) ) {
        int screen_num, depth;
        Display *d = appCtx->getDisplay();
        screen_num = DefaultScreen( d );
        depth = DefaultDepth( d, screen_num );
        bgPixmap = XCreatePixmap( d, XtWindow(executeWidget),
        w, h, depth );
        pixmapW = w;
        pixmapH = h;
      }
    }

    if ( bgPixmap ) {
      executeGc.saveFg();
      executeGc.setFG( ci->pix(bgColor) );
      executeGc.setLineWidth(1);
      executeGc.setLineStyle( LineSolid );
      XDrawRectangle( d, bgPixmap,
       executeGc.normGC(), 0, 0,
       w, h );
      XFillRectangle( d, bgPixmap,
       executeGc.normGC(), 0, 0,
       w, h );
      executeGc.restoreFg();
    }

  }
  else {

    if ( bgPixmap ) {
      XFreePixmap( d, bgPixmap );
      bgPixmap = (Pixmap) NULL;
      pixmapW = pixmapH = -1;
    }

  }

  if ( diagnosticMode() ) {
    char diagBuf[255+1];
    snprintf( diagBuf, 255, "execute [%s]\n", fileName );
    logDiagnostic( diagBuf );
  }

  // process local pv definitions
  pvDefCur = pvDefHead->flink;
  while ( pvDefCur ) {
    //fprintf( stderr, "execute - create pv = [%s]\n", pvDefCur->def );
    pvDefCur->id = the_PV_Factory->create( pvDefCur->def );
    pvDefCur = pvDefCur->flink;
  }

  if ( blank(defaultPvType) ||
       ( clearEpicsPvTypeDefault &&
         ( strcmp( defaultPvType, "EPICS" ) == 0 ) ) ) {
    the_PV_Factory->clear_default_pv_type();
  }
  else {
    the_PV_Factory->set_default_pv_type( defaultPvType );
  }

  if ( forceLocalPvs ) {
    the_PV_Factory->set_default_pv_type( "LOC" );
  }

  btnDownX = btnDownY = 0;
  highlightedObject = NULL;

  if ( activateCallbackFlag ) {
    strncpy( callbackName, id, 63 );
    Strncat( callbackName, "Activate", 63 );
    activateCallback = appCtx->userLibObject.getFunc( callbackName );
    if ( activateCallback ) {
      (*activateCallback)( this );
    }
  }

  this->clear();

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
   KeyPressMask|KeyReleaseMask|ButtonPressMask|PointerMotionMask|
   ButtonReleaseMask|Button1MotionMask|
   Button2MotionMask|Button3MotionMask|ExposureMask, False,
   drawWinEventHandler, (XtPointer) this );

  executeGc.setBaseBG( drawGc.getBaseBG() );

  expandTitle( 1, actualNumMacros, macros, expansions );

  cur = head->flink;
  while ( cur != head ) {

    stat = cur->node->expand1st( actualNumMacros, macros, expansions );

    cur = cur->flink;

  }

  frozen = false;

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

        expandTitle( 2, actualNumMacros, macros, expansions );

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

  for ( pass=1; pass<gMaxExecutePasses; pass++ ) {

    nTries = 200;
    do {

      opStat = 1;

      cnt = 0;
      cur = head->flink;
      while ( cur != head ) {

        if ( !cur->node->isMux() ) {
          cur->node->initEnable();
          stat = cur->node->activate( pass, (void *) cur, &numSubObjects );
          if ( !( stat & 1 ) ) opStat = stat;
          cnt += numSubObjects;
          if ( cnt >= NUM_PER_PENDIO ) {
            pend_io( 5.0 );
            pend_event( 0.01 );
            //processAllEvents( appCtx->appContext(), d );
            cnt = 0;
	  }
        }

        cur = cur->flink;

      }

      nTries--;

    } while ( nTries && !( opStat & 1 ) );

    pend_io( 5.0 );
    pend_event( 0.01 );
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

  if ( noRaise ) {
    noRaise = 0;
  }
  else {
    XRaiseWindow( d, XtWindow(top) );
    isIconified = False;
  }

  setTitle();

  if ( gridShow ) {
    clearActive();
  }

  processAllEvents( appCtx->appContext(), d );

  XtAddEventHandler( executeWidget,
   ButtonPressMask|ButtonReleaseMask|PointerMotionMask|
   ExposureMask, False, activeWinEventHandler, (XtPointer) this );

  refreshActive();

  windowState = AWC_COMPLETE_EXECUTE;

  dumpPvList();

  return 1;

}

int activeWindowClass::reexecute ( void ) { // for multiplexor

int pass, opStat, stat, nTries, cnt, numSubObjects;
activeGraphicListPtr cur, cur1;
pvDefPtr pvDefCur;

int numMuxMacros;
char **muxMacro, **muxExpansion;

  windowState = AWC_START_EXECUTE;

  if ( diagnosticMode() ) {
    char diagBuf[255+1];
    snprintf( diagBuf, 255, "reexecute [%s]\n", fileName );
    logDiagnostic( diagBuf );
  }

  // process local pv definitions
  pvDefCur = pvDefHead->flink;
  while ( pvDefCur ) {
    //fprintf( stderr, "reexecute - create pv = [%s]\n", pvDefCur->def );
    pvDefCur = pvDefCur->flink;
  }

  if ( blank(defaultPvType) ||
       ( clearEpicsPvTypeDefault &&
         ( strcmp( defaultPvType, "EPICS" ) == 0 ) ) ) {
    the_PV_Factory->clear_default_pv_type();
  }
  else {
    the_PV_Factory->set_default_pv_type( defaultPvType );
  }

  if ( forceLocalPvs ) {
    the_PV_Factory->set_default_pv_type( "LOC" );
  }

  if ( mode == AWC_EXECUTE ) return 1;

  btnDownX = btnDownY = 0;
  highlightedObject = NULL;

  isIconified = False;

  expandTitle( 1, actualNumMacros, macros, expansions );

  cur = head->flink;
  while ( cur != head ) {

    if ( !cur->node->isMux() && cur->node->containsMacros() ) {
      stat = cur->node->expand1st( actualNumMacros, macros, expansions );
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

      if ( numMuxMacros > 0 ) {

        expandTitle( 2, numMuxMacros, muxMacro, muxExpansion );

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

  for ( pass=1; pass<gMaxExecutePasses; pass++ ) {

    nTries = 200;
    do {

      opStat = 1;

      cnt = 0;
      cur = head->flink;
      while ( cur != head ) {

        if ( !cur->node->isMux() && cur->node->containsMacros() ) {
          cur->node->initEnable();
          stat = cur->node->reactivate( pass, (void *) cur, &numSubObjects );
          if ( !( stat & 1 ) ) opStat = stat;
          cnt += numSubObjects;
          if ( cnt >= NUM_PER_PENDIO ) {
            pend_io( 5.0 );
            pend_event( 0.01 );
            //processAllEvents( appCtx->appContext(), d );
            cnt = 0;
	  }
        }

        cur = cur->flink;

      }

      nTries--;

    } while ( nTries && !( opStat & 1 ) );

    processAllEvents( appCtx->appContext(), d );

  }

  setTitle();

  refreshActive();

  windowState = AWC_COMPLETE_EXECUTE;

  dumpPvList();

  return 1;

}

int activeWindowClass::returnToEdit (
  int closeFlag ) {

activeGraphicListPtr cur;
btnActionListPtr curBtn, nextBtn;

Window root, child;
int rootX, rootY, winX, winY, pass, cnt, numSubObjects;
unsigned int mask;
char callbackName[63+1];
pvDefPtr pvDefCur;

  if ( !okToDeactivate() ) {
    appCtx->postMessage( activeWindowClass_str193 );
    return 0;
  }

  frozen = false;

  windowState = AWC_START_DEACTIVATE;

  if ( diagnosticMode() ) {
    char diagBuf[255+1];
    snprintf( diagBuf, 255, "returnToEdit [%s]\n", fileName );
    logDiagnostic( diagBuf );
  }

  // process local pv definitions
  pvDefCur = pvDefHead->flink;
  while ( pvDefCur ) {
    //fprintf( stderr, "returnToEdit - release pv = [%s]\n", pvDefCur->def );
    pvDefCur->id->release();
    pvDefCur = pvDefCur->flink;
  }

  mode = AWC_EDIT;

  highlightedObject = NULL;

#if 0
#ifdef ADD_SCROLLED_WIN

  if ( appCtx->useScrollBars ) {

    /* if they resized the toplevel window in execute mode, revert
     * the change now...
     */
    if ( scroll ) {

      Dimension currW, currH, tmpD;
      Position currX, currY;
      Widget clip;

      XtVaGetValues( scroll, XmNclipWindow, &clip, 0 );

      if ( !clip ) {

        XtWarning("activeWindowClass::returnToEdit(): no clipWindow found");

      }
      else {

        XtVaGetValues(
         clip,
         XmNwidth, &currW,
         XmNheight, &currH,
         XmNx, &currX,
         XmNy, &currY,
         0 );

        if ( currW > w && currH > h ) {
          XtVaSetValues( top, XmNwidth, (Dimension)w, XmNheight, (Dimension)h, 0 );
        }
        else if ( currW > w ) {
          XtVaGetValues( top, XmNwidth, &tmpD, 0);
          XtVaSetValues( top, XmNwidth, (Dimension)(w + tmpD - currW ), 0);
        }
        else if ( currH > h ) {
          XtVaGetValues( top, XmNheight, &tmpD, 0);
          XtVaSetValues( top, XmNheight, (Dimension)(h + tmpD - currH ), 0);
        }

      }

    }

  }

#endif
#endif

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

  cnt = 0;
  cur = head->flink;
  while ( cur != head ) {

    for ( pass=1; pass<=2; pass++ ) {
      cur->node->deactivate( pass, &numSubObjects );
    }
    cur->node->deselect();

    cur = cur->flink;

    cnt += numSubObjects;
    if ( cnt >= NUM_PER_PENDIO ) {
      pend_io( 5.0 );
      pend_event( 0.01 );
      //processAllEvents( appCtx->appContext(), d );
      cnt = 0;
    }

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

      XQueryPointer( d, XtWindow(executeWidget), &root, &child,
       &rootX, &rootY, &winX, &winY, &mask );

      confirm.create( top, "confirm", rootX, rootY, 2,
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
        Strncat( callbackName, "Deactivate", 63 );
        deactivateCallback = appCtx->userLibObject.getFunc(
         callbackName );
        if ( deactivateCallback ) {
          (*deactivateCallback)( this );
        }
      }

      windowState = AWC_COMPLETE_DEACTIVATE;

      return 1;

    }

  }
  else {

    processAllEvents( appCtx->appContext(), d );

    this->appCtx->deiconifyMainWindow();

  }

  XtAddEventHandler( drawWidget,
   KeyPressMask|KeyReleaseMask|ButtonPressMask|PointerMotionMask|
   ButtonReleaseMask|Button1MotionMask|
   Button2MotionMask|Button3MotionMask|ExposureMask, False,
   drawWinEventHandler, (XtPointer) this );

  this->clear();
  this->refresh();

  if ( deactivateCallbackFlag ) {
    strncpy( callbackName, id, 63 );
    Strncat( callbackName, "Deactivate", 63 );
    deactivateCallback = appCtx->userLibObject.getFunc(
     callbackName );
    if ( deactivateCallback ) {
      (*deactivateCallback)( this );
    }
  }

  windowState = AWC_COMPLETE_DEACTIVATE;

  return 1;

}

int activeWindowClass::preReexecute ( void )
{

activeGraphicListPtr cur;
int numSubObjects, cnt;

  if ( mode == AWC_EDIT ) return 1;

  if ( !okToPreReexecute() ) {
    appCtx->postMessage( activeWindowClass_str193 );
    return 0;
  }

  windowState = AWC_START_DEACTIVATE;

  if ( diagnosticMode() ) {
    char diagBuf[255+1];
    snprintf( diagBuf, 255, "preReexecute [%s]\n", fileName );
    logDiagnostic( diagBuf );
  }

  mode = AWC_EDIT;

  cnt = 0;
  cur = head->flink;
  while ( cur != head ) {

    if ( !cur->node->isMux() && cur->node->containsMacros() ) {
      cur->node->bufInvalidate();
      cur->node->eraseActive();
      cur->node->preReactivate( 1, &numSubObjects);
      cur->node->preReactivate( 2, &numSubObjects);
      cnt += numSubObjects;
      if ( cnt >= NUM_PER_PENDIO ) {
        pend_io( 5.0 );
        pend_event( 0.01 );
        //processAllEvents( appCtx->appContext(), d );
        cnt = 0;
      }
    }

    cur = cur->flink;

  }

  windowState = AWC_COMPLETE_DEACTIVATE;

  return 1;

}

int activeWindowClass::clearActive ( void ) {

  if ( bgPixmap ) {

    executeGc.setLineWidth(1);
    executeGc.setLineStyle( LineSolid );

    XDrawRectangle( d, bgPixmap,
     executeGc.eraseGC(), 0, 0,
     w, h );

    XFillRectangle( d, bgPixmap,
     executeGc.eraseGC(), 0, 0,
     w, h );

    needCopy = 1;
    needFullCopy = 1;
    doCopy();

  }
  else {

    XClearWindow( d, XtWindow(executeWidget) );

  }

  return 1;

}

int activeWindowClass::requestSmartDrawAllActive ( void ) {

  if ( bgPixmap ) {
    needCopy = 1;
    smartDrawAllActive();
    return 1;
  }

  appCtx->smartDrawAllActive( this );

  return 1;

}

int activeWindowClass::smartDrawAllActive ( void ) {

activeGraphicListPtr cur;
int n = 0;
char *envPtr;

  if ( gFastRefresh == -1 ) {
    envPtr = getenv( environment_str21 );
    if ( envPtr ) {
      gFastRefresh = 1;
    }
    else {
      gFastRefresh = 0;
    }
  }

  if ( gFastRefresh ) {

    cur = head->flink;
    while ( cur != head ) {
      n += cur->node->smartDrawCount();
      cur = cur->flink;
    }

    if ( ( n < 1 ) || ( n > 1000 ) ) {

      cur = head->flink;
      while ( cur != head ) {
        if ( cur->node->smartDrawCount() ) {
          cur->node->resetSmartDrawCount();
        }
        cur = cur->flink;
      }
      requestActiveRefresh();
      return 1;

    }

  }

  cur = head->flink;
  while ( cur != head ) {
    if ( cur->node->smartDrawCount() ) {
      cur->node->doSmartDrawAllActive();
    }
    cur = cur->flink;
  }

  needCopy = 1;

  return 1;

}

int activeWindowClass::requestActiveRefresh ( void ) {

  if ( bgPixmap ) {
    refreshActive();
    return 1;
  }

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

  if ( noRefresh ) {
    needCopy = 1;
    return 1;
  }

  cur = head->flink;
  if ( cur != head ) {
    cur->node->refreshActive( _x, _y, _w, _h );
  }

  needFullCopy = 1;
  needCopy = 1;

  return 1;

}

int activeWindowClass::refreshActive ( void ) {

activeGraphicListPtr cur;

  if ( noRefresh ) {
    needCopy = 1;
    return 1;
  }

  cur = head->flink;
  if ( cur != head ) {
    cur->node->refreshActive();
  }

  needFullCopy = 1;
  needCopy = 1;

  return 1;

}

int activeWindowClass::old_saveWin (
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

#if 0

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

#if 1

  // version >= 3.1.0

  index = fgColor;
  ci->writeColorIndex( f, index );
  index = bgColor;
  ci->writeColorIndex( f, index );
  index = defaultTextFgColor;
  ci->writeColorIndex( f, index );
  index = defaultFg1Color;
  ci->writeColorIndex( f, index );
  index = defaultFg2Color;
  ci->writeColorIndex( f, index );
  index = defaultBgColor;
  ci->writeColorIndex( f, index );
  index = defaultTopShadowColor;
  ci->writeColorIndex( f, index );
  index = defaultBotShadowColor;
  ci->writeColorIndex( f, index );
  index = defaultOffsetColor;
  ci->writeColorIndex( f, index );

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

int activeWindowClass::saveWin (
  FILE *f ) {

int stat;
commentLinesPtr commentCur;
tagClass tag;

static int zero = 0;
static int ten = 10;

static int left = XmALIGNMENT_BEGINNING;
static char *emptyStr = "";
static char *alignEnumStr[3] = {
  "left",
  "center",
  "right"
};
static int alignEnum[3] = {
  XmALIGNMENT_BEGINNING,
  XmALIGNMENT_CENTER,
  XmALIGNMENT_END
};

static int perEnvVar = 0;
static char *pixmapEnumStr[3] = {
  "perEnvVar",
  "never",
  "always"
};
static int pixmapEnum[3] = {
  0,
  1,
  2
};

char *commentFile;
FILE *cf;
char str[255+1], *strPtr;

  if ( !haveComments ) {
    commentFile = getenv( environment_str22 );
    if ( commentFile ) {
      cf = fopen( commentFile, "r" );
      if ( cf ) {
        fprintf( f, "# <<<edm-generated-comments>>>\n" );
        fprintf( f, "#\n" );
        do {
          strPtr = fgets( str, 255, cf );
          str[255] = 0;
          if ( strPtr ) fprintf( f, "%s", str );
          if ( !strstr( str, "\n" ) ) fprintf( f, "\n" );
        } while ( strPtr );
        fclose( cf );
      }
    }
  }

  commentCur = commentHead->flink;
  while ( commentCur ) {
    if ( commentCur->line ) fprintf( f, "%s", commentCur->line );
    commentCur = commentCur->flink;
  }

  fprintf( f, "%-d %-d %-d\n", AWC_MAJOR_VERSION, AWC_MINOR_VERSION,
   AWC_RELEASE );

  major = AWC_MAJOR_VERSION;
  minor = AWC_MINOR_VERSION;
  release = AWC_RELEASE;

  tag.init();
  tag.loadW( "beginScreenProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "font", defaultFontTag );
  tag.loadW( "fontAlign", 3, alignEnumStr, alignEnum,
   &defaultAlignment, &left );
  tag.loadW( "ctlFont", defaultCtlFontTag );
  tag.loadW( "ctlFontAlign", 3, alignEnumStr, alignEnum,
   &defaultCtlAlignment, &left );
  tag.loadW( "btnFont", defaultBtnFontTag );
  tag.loadW( "btnFontAlign", 3, alignEnumStr, alignEnum,
   &defaultBtnAlignment, &left );
  tag.loadW( "fgColor", &appCtx->ci, &fgColor );
  tag.loadW( "bgColor", &appCtx->ci, &bgColor );
  tag.loadW( "textColor", &appCtx->ci, &defaultTextFgColor );
  tag.loadW( "ctlFgColor1", &appCtx->ci, &defaultFg1Color );
  tag.loadW( "ctlFgColor2", &appCtx->ci, &defaultFg2Color );
  tag.loadW( "ctlBgColor1", &appCtx->ci, &defaultBgColor );
  tag.loadW( "ctlBgColor2", &appCtx->ci, &defaultOffsetColor );
  tag.loadW( "topShadowColor", &appCtx->ci, &defaultTopShadowColor );
  tag.loadW( "botShadowColor", &appCtx->ci, &defaultBotShadowColor );
  tag.loadW( "title", title, emptyStr );
  tag.loadBoolW( "showGrid", &gridShow, &zero );
  tag.loadBoolW( "snapToGrid", &gridActive, &zero );
  tag.loadW( "gridSize", &gridSpacing, &ten );
  tag.loadBoolW( "orthoLineDraw", &orthogonal, &zero );
  tag.loadW( "pvType", defaultPvType, emptyStr );
  tag.loadBoolW( "disableScroll", &disableScroll, &zero );
  tag.loadW( "pixmapFlag", 3, pixmapEnumStr, pixmapEnum,
   &bgPixmapFlag, &perEnvVar );
  tag.loadW( "templateParams", AWC_TMPLPARAMSIZE+1, (char *) paramValue,
   AWC_MAXTMPLPARAMS, emptyStr );
  tag.loadComplexW( "templateInfo", (char *) templInfo, emptyStr );
  tag.loadW( unknownTags );
  tag.loadW( "endScreenProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

void activeWindowClass::readCommentsAndVersionGeneric (
  FILE *f,
  int isSymbolFile
) {

char oneLine[255+1], buf[255+1], buf2[255+1], *tk, *context, *context2;
commentLinesPtr commentCur;
int numComments = 0, moreComments = 1, checkForRev = 1,
 checkForEdmComments = 1;

  haveComments = 0;
  strcpy( fileNameAndRev, fileName );

  do {

    readStringFromFile( oneLine, 255+1, f ); incLine();

    strcpy( buf, oneLine );

    context = NULL;
    tk = strtok_r( buf, " \t\n", &context );

    if ( !tk || ( tk[0] == '#' ) ) {

      if ( !isSymbolFile ) {

        // check for cvs/rcs revision info
        if ( tk && ( tk[0] == '#' ) ) {

          if ( checkForEdmComments ) {

            strcpy( buf2, oneLine );

            context2 = NULL;
            tk = strtok_r( buf2, " \t\n#", &context2 );

            if ( tk ) {

              if ( strcmp( tk, "<<<edm-generated-comments>>>" ) == 0 ) {

                checkForEdmComments = 0;
                haveComments = 1;

	      }

	    }

	  }

          if ( checkForRev ) {

            strcpy( buf2, oneLine );

            context2 = NULL;
            tk = strtok_r( buf2, " \t\n#", &context2 );

            if ( tk ) {

              if ( strcmp( tk, "$InvalidBgColor:" ) == 0 ) {

		invalidFile = 1;
                invalidBgColor = 0;

                checkForRev = 0; // use first rev found, don't check any more

                tk = strtok_r( NULL, " \t\n#", &context2 );
                if ( tk ) {
                  char *nonInt;
                  invalidBgColor = strtol( tk, &nonInt, 10 );
                  Strncat( fileNameAndRev, " (", 287 );
                  Strncat( fileNameAndRev, activeWindowClass_str214, 287 );
                  Strncat( fileNameAndRev, ")", 287 );
                  strncpy( fileRev, activeWindowClass_str214, 31 );
                  fileRev[31] = 0;
	        }

              }
              else if ( strcmp( tk, "$Revision:" ) == 0 ) {

                checkForRev = 0; // use first rev found, don't check any more

                tk = strtok_r( NULL, " \t\n#", &context2 );
                if ( tk ) {
                  Strncat( fileNameAndRev, " (", 287 );
                  Strncat( fileNameAndRev, tk, 287 );
                  Strncat( fileNameAndRev, ")", 287 );
                  strncpy( fileRev, tk, 31 );
                  fileRev[31] = 0;
	        }

              }

	    }

	  }

        }

      }

      if ( !isSymbolFile ) {

        numComments++;
        commentCur = new commentLinesType;
        commentCur->line = new char[strlen(oneLine)+4];
        strcpy( commentCur->line, oneLine );
        strcat( commentCur->line, "\n" );
        commentTail->flink = commentCur;
        commentTail = commentCur;
        commentTail->flink = NULL;

      }

    }
    else {
      moreComments = 0;
    }

  } while ( moreComments );

  if ( !isSymbolFile ) {
    if ( !numComments ) {
      commentTail = commentHead;
      commentTail->flink = NULL;
    }
  }

  sscanf( oneLine, "%d %d %d\n", &major, &minor, &release );

}

void activeWindowClass::readCommentsAndVersion (
  FILE *f
) {

int isSymbolFile = 0;

  readCommentsAndVersionGeneric( f, isSymbolFile );

}

void activeWindowClass::readSymbolCommentsAndVersion (
  FILE *f
) {

int isSymbolFile = 1;

  readCommentsAndVersionGeneric( f, isSymbolFile );

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
    readStringFromFile( oneLine, 255+1, f );

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

int activeWindowClass::loadWinDummy (
  FILE *f,
  int _x,
  int _y,
  int setPosition ) {

  // if this is changed then activeWindowClass::discardWinLoadData
  // must be likewise changed

int stat, retStat = 1;
int fileX, fileY, n, tmpVal;
Arg args[5];

tagClass tag;

  x = 0;
  y = 0;
  strcpy( defaultFontTag, "" );
  strcpy( defaultCtlFontTag, "" );
  strcpy( defaultBtnFontTag, "" );

  strcpy( this->id, "" );
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;

  major = 4;
  minor = 0;
  release = 0;
  fileX = 0;
  fileY = 0;
  w = 50;
  h = 50;
  defaultAlignment = 0;
  defaultCtlAlignment = 0;
  defaultBtnAlignment = 0;
  fgColor = 0;
  bgColor = 0;
  defaultTextFgColor = 0;
  defaultFg1Color = 0;
  defaultFg2Color = 0;
  defaultBgColor = 0;
  defaultOffsetColor = 0;
  defaultTopShadowColor = 0;
  defaultBotShadowColor = 0;
  strcpy( title, "" );
  gridShow = 0;
  gridActive = 0;
  gridSpacing = 0;
  orthogonal = 0;
  strcpy( defaultPvType, "" );

  if ( setPosition ) {
    x = _x;
    y = _y;
  }
  else {
    x = fileX;
    y = fileY;
  }

  if ( !intersects( x, y, x+w, y+h, 0, 0,
   XDisplayWidth( d, DefaultScreen(d) ),
   XDisplayHeight( d, DefaultScreen(d) ) ) ) {

//    appCtx->postMessage(
//    "Screen location is out of display bounds - setting location to (50,50)" );

    x = y = 50;

  }

#ifdef ADD_SCROLLED_WIN
  if ( isEmbedded ) {

      n = 0;
      XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
      XtSetValues( drawWidget, args, n );

      n = 0;
      XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
      XtSetValues( drawWidget, args, n );

  }
#else
  n = 0;
  XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
  XtSetValues( drawWidget, args, n );

  n = 0;
  XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
  XtSetValues( drawWidget, args, n );
#endif

  if ( isEmbedded ) {

    if ( embCenter && !embSetSize ) {

      if ( embeddedH > h ) {

        tmpVal = y + ( embeddedH - h ) / 2;
        n = 0;
        XtSetArg( args[n], XmNy, (XtArgVal) tmpVal ); n++;
        XtSetValues( drawWidget, args, n );

      }

      if ( embeddedW > w ) {

        tmpVal = x + ( embeddedW - w ) / 2;
        n = 0;
        XtSetArg( args[n], XmNx, (XtArgVal) tmpVal ); n++;
        XtSetValues( drawWidget, args, n );

      }

    }

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
    XtSetValues( drawWidget, args, n );

    n = 0;
    XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
    XtSetValues( drawWidget, args, n );

  }

  if ( isEmbedded ) {

    if ( embSetSize ) {

      if ( w+embSizeOfs <= embeddedW ) {

         n = 0;
         XtSetArg( args[n], XmNwidth, (XtArgVal) w+embSizeOfs ); n++;
         XtSetValues( top, args, n );

      }

      if ( h+embSizeOfs <= embeddedH ) {

         n = 0;
         XtSetArg( args[n], XmNheight, (XtArgVal) h+embSizeOfs ); n++;
         XtSetValues( top, args, n );

      }

    }

  }
  else {

#ifndef ADD_SCROLLED_WIN
    n = 0;
    XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
    XtSetValues( top, args, n );

    n = 0;
    XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
    XtSetValues( top, args, n );
#else
    if ( !appCtx->useScrollBars ) {

      n = 0;
      XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
      XtSetValues( top, args, n );

      n = 0;
      XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
      XtSetValues( top, args, n );

    }
    else {

      reconfig();

    }
#endif

  }

  if ( strcmp( defaultFontTag, "" ) != 0 ) {
    stat = defaultFm.setFontTag( defaultFontTag );
  }

  stat = defaultFm.setFontAlignment( defaultAlignment );

  if ( strcmp( defaultCtlFontTag, "" ) != 0 ) {
    stat = defaultCtlFm.setFontTag( defaultCtlFontTag );
  }

  stat = defaultCtlFm.setFontAlignment( defaultCtlAlignment );

  if ( strcmp( defaultBtnFontTag, "" ) != 0 ) {
    stat = defaultBtnFm.setFontTag( defaultBtnFontTag );
  }

  stat = defaultBtnFm.setFontAlignment( defaultBtnAlignment );

  drawGc.setBaseBG( ci->pix(bgColor) );

  expStrTitle.setRaw( title );

  updateAllSelectedDisplayInfo();

  return retStat;

}

int activeWindowClass::loadWinGeneric (
  FILE *f,
  int _x,
  int _y,
  int setPosition ) {

  // if this is changed then activeWindowClass::discardWinLoadData
  // must be likewise changed

int stat, retStat = 1;
int fileX, fileY, n, tmpVal;
Arg args[5];

tagClass tag;

static int zero = 0;
static int ten = 10;

static int left = XmALIGNMENT_BEGINNING;
static char *emptyStr = "";
static char *alignEnumStr[3] = {
  "left",
  "center",
  "right"
};
static int alignEnum[3] = {
  XmALIGNMENT_BEGINNING,
  XmALIGNMENT_CENTER,
  XmALIGNMENT_END
};

static int perEnvVar = 0;
static char *pixmapEnumStr[3] = {
  "perEnvVar",
  "never",
  "always"
};
static int pixmapEnum[3] = {
  0,
  1,
  2
};

#if 0
  readCommentsAndVersion( f );

  if ( major > AWC_MAJOR_VERSION ) {
    appCtx->postMessage( activeWindowClass_str191 );
    return 0;
  }

  if ( major < 4 ) {
    appCtx->postMessage( activeWindowClass_str191 );
    return 0;
  }
#endif

  x = 0;
  y = 0;
  w = 100;
  h = 150;
  strcpy( defaultFontTag, "" );
  strcpy( defaultCtlFontTag, "" );
  strcpy( defaultBtnFontTag, "" );

  strcpy( this->id, "" );
  activateCallbackFlag = 0;
  deactivateCallbackFlag = 0;

  tag.init();
  tag.loadR( "beginScreenProperties" );
  tag.loadR( unknownTags );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );
  tag.loadR( "x", &fileX );
  tag.loadR( "y", &fileY );
  tag.loadR( "w", &w );
  tag.loadR( "h", &h );
  tag.loadR( "font", 63, defaultFontTag );
  tag.loadR( "fontAlign", 3, alignEnumStr, alignEnum,
   &defaultAlignment, &left );
  tag.loadR( "ctlFont", 63, defaultCtlFontTag );
  tag.loadR( "ctlFontAlign", 3, alignEnumStr, alignEnum,
   &defaultCtlAlignment, &left );
  tag.loadR( "btnFont", 63, defaultBtnFontTag );
  tag.loadR( "btnFontAlign", 3, alignEnumStr, alignEnum,
   &defaultBtnAlignment, &left );
  tag.loadR( "fgColor", &appCtx->ci, &fgColor );
  tag.loadR( "bgColor", &appCtx->ci, &bgColor );
  tag.loadR( "textColor", &appCtx->ci, &defaultTextFgColor );
  tag.loadR( "ctlFgColor1", &appCtx->ci, &defaultFg1Color );
  tag.loadR( "ctlFgColor2", &appCtx->ci, &defaultFg2Color );
  tag.loadR( "ctlBgColor1", &appCtx->ci, &defaultBgColor );
  tag.loadR( "ctlBgColor2", &appCtx->ci, &defaultOffsetColor );
  tag.loadR( "topShadowColor", &appCtx->ci, &defaultTopShadowColor );
  tag.loadR( "botShadowColor", &appCtx->ci, &defaultBotShadowColor );
  tag.loadR( "title", 127, title, emptyStr );
  tag.loadR( "showGrid", &gridShow, &zero );
  tag.loadR( "snapToGrid", &gridActive, &zero );
  tag.loadR( "gridSize", &gridSpacing, &ten );
  tag.loadR( "orthoLineDraw", &orthogonal, &zero );
  tag.loadR( "pvType", 15, defaultPvType, emptyStr );
  tag.loadR( "disableScroll", &disableScroll, &zero );
  tag.loadR( "pixmapFlag", 3, pixmapEnumStr, pixmapEnum,
   &bgPixmapFlag, &perEnvVar );
  tag.loadR( "templateParams", AWC_MAXTMPLPARAMS, AWC_TMPLPARAMSIZE+1,
   (char *) paramValue, &numParamValues, emptyStr );
  tag.loadR( "templateInfo", AWC_MAXTEMPLINFO, (char *) templInfo,
   emptyStr );
  tag.loadR( "endScreenProperties" );

  stat = tag.readTags( f, "endScreenProperties" );

  if ( !( stat & 1 ) ) {
    retStat = stat;
    appCtx->postMessage( tag.errMsg() );
  }

  if ( disableScroll ) {
  }

  if ( strcmp( defaultPvType, "epics" ) == 0 ) {
    strcpy( defaultPvType, "EPICS" );
  }

  if ( setPosition ) {
    x = _x;
    y = _y;
  }
  else {
    x = fileX;
    y = fileY;
  }

  if ( !intersects( x, y, x+w, y+h, 0, 0,
   XDisplayWidth( d, DefaultScreen(d) ),
   XDisplayHeight( d, DefaultScreen(d) ) ) ) {

//    appCtx->postMessage(
//    "Screen location is out of display bounds - setting location to (50,50)" );

    x = y = 50;

  }

#if 0
  fprintf( stderr, "embCenter = %-d\n", embCenter );
  fprintf( stderr, "embSetSize = %-d\n", embSetSize );
  fprintf( stderr, "embSizeOfs = %-d\n", embSizeOfs );
  fprintf( stderr, "embeddedW = %-d\n", embeddedW );
  fprintf( stderr, "embeddedH = %-d\n", embeddedH );
  fprintf( stderr, "w = %-d\n", w );
  fprintf( stderr, "h = %-d\n", h );
#endif

#ifdef ADD_SCROLLED_WIN
  if ( isEmbedded ) {

      n = 0;
      XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
      XtSetValues( drawWidget, args, n );

      n = 0;
      XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
      XtSetValues( drawWidget, args, n );

  }
#else
  n = 0;
  XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
  XtSetValues( drawWidget, args, n );

  n = 0;
  XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
  XtSetValues( drawWidget, args, n );
#endif

  if ( isEmbedded ) {

    if ( embCenter && !embSetSize ) {

      if ( embeddedH > h ) {

        tmpVal = y + ( embeddedH - h ) / 2;
        n = 0;
        XtSetArg( args[n], XmNy, (XtArgVal) tmpVal ); n++;
        XtSetValues( drawWidget, args, n );

      }

      if ( embeddedW > w ) {

        tmpVal = x + ( embeddedW - w ) / 2;
        n = 0;
        XtSetArg( args[n], XmNx, (XtArgVal) tmpVal ); n++;
        XtSetValues( drawWidget, args, n );

      }

    }

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
    XtSetValues( drawWidget, args, n );

    n = 0;
    XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
    XtSetValues( drawWidget, args, n );

  }

  if ( isEmbedded ) {

    if ( embSetSize ) {

      if ( w+embSizeOfs <= embeddedW ) {

         n = 0;
         XtSetArg( args[n], XmNwidth, (XtArgVal) w+embSizeOfs ); n++;
         XtSetValues( top, args, n );

      }

      if ( h+embSizeOfs <= embeddedH ) {

         n = 0;
         XtSetArg( args[n], XmNheight, (XtArgVal) h+embSizeOfs ); n++;
         XtSetValues( top, args, n );

      }

    }

  }
  else {

#ifndef ADD_SCROLLED_WIN
    n = 0;
    XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
    XtSetValues( top, args, n );

    n = 0;
    XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
    XtSetValues( top, args, n );
#else
    if ( !appCtx->useScrollBars ) {

      n = 0;
      XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
      XtSetValues( top, args, n );

      n = 0;
      XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
      XtSetValues( top, args, n );

    }
    else {

      reconfig();

    }
#endif

  }

  if ( strcmp( defaultFontTag, "" ) != 0 ) {
    stat = defaultFm.setFontTag( defaultFontTag );
  }

  stat = defaultFm.setFontAlignment( defaultAlignment );

  if ( strcmp( defaultCtlFontTag, "" ) != 0 ) {
    stat = defaultCtlFm.setFontTag( defaultCtlFontTag );
  }

  stat = defaultCtlFm.setFontAlignment( defaultCtlAlignment );

  if ( strcmp( defaultBtnFontTag, "" ) != 0 ) {
    stat = defaultBtnFm.setFontTag( defaultBtnFontTag );
  }

  stat = defaultBtnFm.setFontAlignment( defaultBtnAlignment );

  drawGc.setBaseBG( ci->pix(bgColor) );

  expStrTitle.setRaw( title );

  updateAllSelectedDisplayInfo();

  return retStat;

}

int activeWindowClass::loadWin (
  FILE *f,
  int _x,
  int _y ) {

  return loadWinGeneric( f, _x, _y, 1 );

}

int activeWindowClass::loadWin (
  FILE *f
) {

  return loadWinGeneric( f, 0, 0, 0 );

}

int activeWindowClass::old_loadWinGeneric (
  FILE *f,
  int _x,
  int _y,
  int setPosition ) {

  // if this is changed then activeWindowClass::old_discardWinLoadData
  // must be likewise changed

int stat, tmpVal;
int r, g, b, n, index;
Arg args[5];
unsigned int pixel;

#if 0
  readCommentsAndVersion( f );

  if ( major > AWC_MAJOR_VERSION ) {
    appCtx->postMessage( activeWindowClass_str191 );
    return 0;
  }
#endif

  fscanf( f, "%d\n", &x ); incLine();
  fscanf( f, "%d\n", &y ); incLine();

  if ( setPosition ) {
    x = _x;
    y = _y;
  }

  fscanf( f, "%d\n", &w ); incLine();
  fscanf( f, "%d\n", &h ); incLine();

#if 0
  if ( isEmbedded ) {
    w = embeddedW;
    h = embeddedH;
  }
#endif

  if ( !intersects( x, y, x+w, y+h, 0, 0,
   XDisplayWidth( d, DefaultScreen(d) ),
   XDisplayHeight( d, DefaultScreen(d) ) ) ) {

//  appCtx->postMessage(
//  "Screen location is out of display bounds - setting location to (50,50)" );

    x = y = 50;

  }

#ifdef ADD_SCROLLED_WIN
  if ( isEmbedded ) {

      n = 0;
      XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
      XtSetValues( drawWidget, args, n );

      n = 0;
      XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
      XtSetValues( drawWidget, args, n );

  }
#else
  n = 0;
  XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
  XtSetValues( drawWidget, args, n );

  n = 0;
  XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
  XtSetValues( drawWidget, args, n );
#endif

  if ( isEmbedded ) {

    if ( embCenter ) {

      if ( embeddedH > h ) {

        tmpVal = y + ( embeddedH - h ) / 2;
        n = 0;
        XtSetArg( args[n], XmNy, (XtArgVal) tmpVal ); n++;
        XtSetValues( drawWidget, args, n );

      }

      if ( embeddedW > w ) {

        tmpVal = x + ( embeddedW - w ) / 2;
        n = 0;
        XtSetArg( args[n], XmNx, (XtArgVal) tmpVal ); n++;
        XtSetValues( drawWidget, args, n );

      }

    }
    else {

      n = 0;
      XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
      XtSetValues( drawWidget, args, n );

      n = 0;
      XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
      XtSetValues( drawWidget, args, n );

    }

  }
  else {

    n = 0;
    XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
    XtSetValues( drawWidget, args, n );

    n = 0;
    XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
    XtSetValues( drawWidget, args, n );

  }

  if ( isEmbedded ) {

    if ( embSetSize ) {

       n = 0;
       XtSetArg( args[n], XmNwidth, (XtArgVal) w+embSizeOfs ); n++;
       XtSetValues( top, args, n );

       n = 0;
       XtSetArg( args[n], XmNheight, (XtArgVal) h+embSizeOfs ); n++;
       XtSetValues( top, args, n );

    }
    else {

      n = 0;
      XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
      XtSetValues( top, args, n );

      n = 0;
      XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
      XtSetValues( top, args, n );

    }

  }
  else {

#ifndef ADD_SCROLLED_WIN
    n = 0;
    XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
    XtSetValues( top, args, n );

    n = 0;
    XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
    XtSetValues( top, args, n );
#else
    if ( !appCtx->useScrollBars ) {

      n = 0;
      XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
      XtSetValues( top, args, n );

      n = 0;
      XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;
      XtSetValues( top, args, n );

    }
    else {

      reconfig();

    }
#endif

  }

  readStringFromFile( defaultFontTag, 63+1, f ); incLine();

  if ( strcmp( defaultFontTag, "" ) != 0 ) {
    stat = defaultFm.setFontTag( defaultFontTag );
  }

  fscanf( f, "%d\n", &defaultAlignment ); incLine();

  if ( defaultAlignment != 0 ) {
    stat = defaultFm.setFontAlignment( defaultAlignment );
  }

  if ( ( major > 1 ) || ( minor > 2 ) ) {

    readStringFromFile( defaultCtlFontTag, 63+1, f ); incLine();

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

  if ( ( major > 3 ) || ( ( major == 3 ) && ( minor > 0 ) ) ) {

    ci->readColorIndex( f, &index );
    incLine(); incLine();
    fgColor = index;

    ci->readColorIndex( f, &index );
    incLine(); incLine();
    bgColor = index;

    drawGc.setBaseBG( ci->pix(bgColor) );

    ci->readColorIndex( f, &index );
    incLine(); incLine();
    defaultTextFgColor = index;

    ci->readColorIndex( f, &index );
    incLine(); incLine();
    defaultFg1Color = index;

    ci->readColorIndex( f, &index );
    incLine(); incLine();
    defaultFg2Color = index;

    ci->readColorIndex( f, &index );
    incLine(); incLine();
    defaultBgColor = index;

    ci->readColorIndex( f, &index );
    incLine(); incLine();
    defaultTopShadowColor = index;

    ci->readColorIndex( f, &index );
    incLine(); incLine();
    defaultBotShadowColor = index;

    ci->readColorIndex( f, &index );
    incLine(); incLine();
    defaultOffsetColor = index;

  }
  else if ( ( major == 3 ) && ( minor == 0 ) ) {

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
    fgColor = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    bgColor = ci->pixIndex( pixel );

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
    defaultTextFgColor = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultFg1Color = ci->pixIndex( pixel );

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
    defaultFg2Color = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultBgColor = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultTopShadowColor = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultBotShadowColor = ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); incLine();
    if ( ( major < 2 ) && ( minor < 4 ) ) {
      r *= 256;
      g *= 256;
      b *= 256;
    }
    ci->setRGB( r, g, b, &pixel );
    defaultOffsetColor = ci->pixIndex( pixel );

  }

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( title, 127+1, f ); incLine();
  }
  else {
    strcpy( title, "" );
  }
  expStrTitle.setRaw( title );

  if ( ( major > 1 ) || ( minor > 4 ) ) {

    fscanf( f, "%d\n", &gridShow ); incLine();

    fscanf( f, "%d\n", &gridActive ); incLine();

    fscanf( f, "%d\n", &gridSpacing ); incLine();

    fscanf( f, "%d\n", &orthogonal ); incLine();

  }
  else {

    gridShow = 0;
    gridActive = 0;
    gridSpacing = 10;
    orthogonal = 0;

  }

  if ( ( major > 1 ) || ( minor > 5 ) ) {
    readStringFromFile( defaultPvType, 15+1, f ); incLine();
  }
  else {
    strcpy( defaultPvType, "" );
  }

  if ( strcmp( defaultPvType, "epics" ) == 0 ) {
    strcpy( defaultPvType, "EPICS" );
  }

  if ( ( major > 1 ) || ( minor > 6 ) ) {
    readStringFromFile( this->id, 31+1, f ); incLine();
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

    readStringFromFile( defaultBtnFontTag, 63+1, f ); incLine();

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

int activeWindowClass::old_loadWin (
  FILE *f ) {

  return old_loadWinGeneric( f, 0, 0, 0 );

}

int activeWindowClass::old_loadWin (
  FILE *f,
  int _x,
  int _y ) {

  return old_loadWinGeneric( f, _x, _y, 1 );

}

int activeWindowClass::importWin (
  FILE *f ) {

int r, g, b;
char buf[255+1], *gotData;
int more;
unsigned int pixel;

char *tk, *context;

int n;
Arg args[5];

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
  bgColor = ci->pixIndex( pixel );

#ifndef ADD_SCROLLED_WIN
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
#else
  if ( appCtx->useScrollBars ) {

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

  }
  else {

    reconfig();

  }
#endif

  drawGc.setBaseBG( ci->pix(bgColor) );

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

  // read until end of data marker

char junk[255+1];
int stat;
tagClass tag;
int i, r, g, b, index;
char s[127+1];
unknownTagList junkTags;
char *emptyStr = "";

  // don't inc line here

  pushVersion();

  readSymbolCommentsAndVersion( f );

  if ( major > AWC_MAJOR_VERSION ) {
    appCtx->postMessage( activeWindowClass_str191 );
    return 0;
  }

  if ( major < 4 ) {

    *_major = major;
    *_minor = minor;
    *_release = release;

    fscanf( f, "%d\n", &i );
    fscanf( f, "%d\n", &i );
    fscanf( f, "%d\n", &i );
    fscanf( f, "%d\n", &i );

    readStringFromFile( s, 63+1, f );

    fscanf( f, "%d\n",&i );

    if ( ( major > 1 ) || ( minor > 2 ) ) {
      readStringFromFile( s, 63+1, f );
      fscanf( f, "%d\n",&i );
    }

    if ( ( major > 3 ) || ( ( major == 3 ) && ( minor > 0 ) ) ) {

      ci->readColorIndex( f, &index );
      incLine(); incLine();

      ci->readColorIndex( f, &index );
      incLine(); incLine();

      ci->readColorIndex( f, &index );
      incLine(); incLine();

      ci->readColorIndex( f, &index );
      incLine(); incLine();

      ci->readColorIndex( f, &index );
      incLine(); incLine();

      ci->readColorIndex( f, &index );
      incLine(); incLine();

      ci->readColorIndex( f, &index );
      incLine(); incLine();

      ci->readColorIndex( f, &index );
      incLine(); incLine();

      ci->readColorIndex( f, &index );
      incLine(); incLine();

    }
    else if ( ( major == 3 ) && ( minor == 0 ) ) {

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

      if ( ( major > 1 ) || ( minor > 2 ) ) {
        fscanf( f, "%d %d %d\n", &r, &g, &b );
      }

      fscanf( f, "%d %d %d\n", &r, &g, &b );

      if ( ( major > 1 ) || ( minor > 2 ) ) {
        fscanf( f, "%d %d %d\n", &r, &g, &b );
      }

      fscanf( f, "%d %d %d\n", &r, &g, &b );

      fscanf( f, "%d %d %d\n", &r, &g, &b );

      fscanf( f, "%d %d %d\n", &r, &g, &b );

      fscanf( f, "%d %d %d\n", &r, &g, &b );

    }

    if ( ( major > 1 ) || ( minor > 1 ) ) {
      readStringFromFile( s, 127+1, f );
    }

    if ( ( major > 1 ) || ( minor > 4 ) ) {

      fscanf( f, "%d\n", &i );

      fscanf( f, "%d\n", &i );

      fscanf( f, "%d\n", &i );

      fscanf( f, "%d\n", &i );

    }

    if ( ( major > 1 ) || ( minor > 5 ) ) {
      readStringFromFile( s, 15+1, f );
    }

    if ( ( major > 1 ) || ( minor > 6 ) ) {
      readStringFromFile( s, 31+1, f );
      fscanf( f, "%d\n", &i );
      fscanf( f, "%d\n", &i );
    }

    if ( ( major > 2 ) ||
         ( ( major == 2 ) && ( minor > 0 ) ) ||
         ( ( major == 2 ) && ( minor == 0 ) && ( release > 0 ) ) ) {
  
      readStringFromFile( s, 63+1, f );

      fscanf( f, "%d\n", &i );

    }

  }
  else {

  *_major = major;
  *_minor = minor;
  *_release = release;

  tag.init();
  tag.loadR( "beginScreenProperties" );
  tag.loadR( junkTags );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );
  tag.loadR( "x", 255, junk );
  tag.loadR( "y", 255, junk );
  tag.loadR( "w", 255, junk );
  tag.loadR( "h", 255, junk );
  tag.loadR( "font", 255, junk );
  tag.loadR( "fontAlign", 255, junk );
  tag.loadR( "ctlFont", 255, junk );
  tag.loadR( "ctlFontAlign", 255, junk );
  tag.loadR( "btnFont", 255, junk );
  tag.loadR( "btnFontAlign", 255, junk );
  tag.loadR( "fgColor", 255, junk );
  tag.loadR( "bgColor", 255, junk );
  tag.loadR( "textColor", 255, junk );
  tag.loadR( "ctlFgColor1", 255, junk );
  tag.loadR( "ctlFgColor2", 255, junk );
  tag.loadR( "ctlBgColor1", 255, junk );
  tag.loadR( "ctlBgColor2", 255, junk );
  tag.loadR( "topShadowColor", 255, junk );
  tag.loadR( "botShadowColor", 255, junk );
  tag.loadR( "title", 255, junk );
  tag.loadR( "showGrid", 255, junk );
  tag.loadR( "snapToGrid", 255, junk );
  tag.loadR( "gridSize", 255, junk );
  tag.loadR( "orthoLineDraw", 255, junk );
  tag.loadR( "pvType", 255, junk );
  tag.loadR( "disableScroll", 255, junk );
  tag.loadR( "templateParams", AWC_MAXTMPLPARAMS, AWC_TMPLPARAMSIZE+1,
   (char *) bufParamValue, &bufNumParamValues, emptyStr );
  if ( !bufTemplInfo ) {
    bufTemplInfo = new char[AWC_MAXTEMPLINFO+1];
  }
  tag.loadR( "templateInfo", AWC_MAXTEMPLINFO, (char *) bufTemplInfo,
   emptyStr );

  tag.loadR( "endScreenProperties" );

  stat = tag.readTags( f, "endScreenProperties" );

  if ( !( stat & 1 ) ) {
    appCtx->postMessage( tag.errMsg() );
  }

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

//f = fopen( fname, "r" );
  f = fileOpen( fname, "r" );
  if ( f ) {
    result = 1;
    fileClose( f );
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
    Strncat( oneFileName, fName, 255 );
  }

  //if ( strlen(oneFileName) > strlen(".edl") ) {
  if ( strlen(oneFileName) > strlen(activeWindowClass::defExt()) ) {
    //if ( strcmp( &oneFileName[strlen(oneFileName)-strlen(".edl")],
    //   ".edl" ) != 0 ) {
    if ( strcmp( &oneFileName[strlen(oneFileName)-strlen(activeWindowClass::defExt())],
       activeWindowClass::defExt() ) != 0 ) {
      //Strncat( oneFileName, ".edl", 255 );
      Strncat( oneFileName, activeWindowClass::defExt(), 255 );
    }
  }
  else {
    //Strncat( oneFileName, ".edl", 255 );
    Strncat( oneFileName, activeWindowClass::defExt(), 255 );
  }

  //f = fopen( oneFileName, "r" );
  f = fileOpen( oneFileName, "r" );
  if ( f ) {
    result = 1;
    fileClose( f );
  }
  else
    result = 0;

  return result;

}

void activeWindowClass::lineEditBegin ( void )
{

  setChanged();
  cursor.set( XtWindow(drawWidget), CURSOR_K_TINYCROSSHAIR );
  cursor.setColor( ci->pix(fgColor), ci->pix(bgColor) );
  state = AWC_EDITING_POINTS;

}

void activeWindowClass::lineCreateBegin ( void )
{

  setChanged();
  cursor.set( XtWindow(drawWidget), CURSOR_K_TINYCROSSHAIR );
  cursor.setColor( ci->pix(fgColor), ci->pix(bgColor) );
  state = AWC_CREATING_POINTS;

}

void activeWindowClass::operationComplete ( void )
{

  if ( state == AWC_EDITING ) {

    if ( undoObj.listEmpty() ) {
      undoObj.discard();
    }
  }

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



// !!!!!! notice coupling with activeWindowClass::showSelectionObject()

  if ( objNameDialogPoppedUp ) {
    objNameDialog.popdown();
    objNameDialogPoppedUp = 0;
  }


}

void activeWindowClass::showSelectionObject ( void ) {

// !!!!!! notice coupling with activeWindowClass::updateMasterSelection()

activeGraphicListPtr cur;
int x1=0, y1=0;
char buf[31+1];
int num_selected;

  num_selected = 0;
  cur = selectedHead->selFlink;
  while ( ( cur != selectedHead ) &&
          ( num_selected < 2 ) ) {

    if ( cur->node->objName() ) {
      if ( obj.getNameFromClass( cur->node->objName() ) ) {
        strncpy( buf, obj.getNameFromClass( cur->node->objName() ), 31 );
      }
      else {
        strcpy( buf, "?" );
      }
    }
    else {
      strcpy( buf, "?" );
    }

    x1 = cur->node->getX1();
    y1 = cur->node->getY1();

    num_selected++;
    cur = cur->selFlink;

  }

  if ( top ) {

    if ( !objNameDialogCreated ) {
      objNameDialog.create( top );
      objNameDialogCreated = 1;
      objNameDialogPoppedUp = 0;
    }

    if ( objNameDialogPoppedUp ) {
      objNameDialog.popdown();
    }

    if ( num_selected == 1 ) {
      objNameDialog.popup( buf, x+x1, y+y1 );
      objNameDialogPoppedUp = 1;
    }

  }

}

int activeWindowClass::processObjects ( void )
{

activeGraphicListPtr cur, next;
int workToDo = 0;

  if( !( this->frozen ) ) {

    appCtx->proc->lock();
    cur = defExeHead->defExeFlink;
    appCtx->proc->unlock();

    if ( !cur ) {
      return 0;
    }

    if ( cur != defExeHead ) {
      needCopy = 1;
      workToDo = 1;
    }

    while ( cur != defExeHead ) {

      if ( pixmapX0 > cur->node->getX0() ) pixmapX0 = cur->node->getX0();
      if ( pixmapX1 < cur->node->getX1() ) pixmapX1 = cur->node->getX1();
      if ( pixmapY0 > cur->node->getY0() ) pixmapY0 = cur->node->getY0();
      if ( pixmapY1 < cur->node->getY1() ) pixmapY1 = cur->node->getY1();

      appCtx->proc->lock();
      next = cur->defExeFlink;
      appCtx->proc->unlock();

      cur->node->executeDeferred();

      cur = next;

    }

  }

  return workToDo;

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

char nameWithSubs[1024+1];

  this->substituteSpecial( 1024, inName, nameWithSubs );

  strncpy( fileName, nameWithSubs, 255 );
  fileName[255] = 0;
  getFileName( displayName, nameWithSubs, 127 );
  displayName[127] = 0;
  getFilePrefix( prefix, nameWithSubs, 127 );
  prefix[127] = 0;
  getFilePostfix( postfix, nameWithSubs, 127 );
  postfix[127] = 0;

  strncpy( fileNameForSym, nameWithSubs, 255 );
  fileNameForSym[255] = 0;
  getFileName( displayNameForSym, nameWithSubs, 127 );
  displayNameForSym[127] = 0;
  getFilePrefix( prefixForSym, nameWithSubs, 127 );
  prefixForSym[127] = 0;
  getFilePostfix( postfixForSym, nameWithSubs, 127 );
  postfixForSym[127] = 0;
  
  //strncpy( fileName, inName, 255 );
  //getFileName( displayName, inName, 127 );
  //getFilePrefix( prefix, inName, 127 );
  //getFilePostfix( postfix, inName, 127 );

  //strncpy( fileNameForSym, inName, 255 );
  //getFileName( displayNameForSym, inName, 127 );
  //getFilePrefix( prefixForSym, inName, 127 );
  //getFilePostfix( postfixForSym, inName, 127 );

}

// The following function is necessary because of rules associated with
// opening related displays. For related displays, info like prefix is
// implicit and thus omitted so related display may be specified without
// a path (causing edm to seach through the paths given in EDMDATAFILES).

// When the special internal symbol specified by <FILEPREFIX> is translated
// however we want a value and thus no value is take to mean ./

void activeWindowClass::storeFileNameForSymbols (
  char *inName )
{

char nameWithSubs[1024+1];

  this->substituteSpecial( 1024, inName, nameWithSubs );

  strncpy( fileNameForSym, nameWithSubs, 255 );
  fileNameForSym[255] = 0;
  getFileName( displayNameForSym, nameWithSubs, 127 );
  displayNameForSym[127] = 0;
  getFilePrefix( prefixForSym, nameWithSubs, 127 );
  prefixForSym[127] = 0;
  getFilePostfix( postfixForSym, nameWithSubs, 127 );
  postfixForSym[127] = 0;

  //strncpy( fileNameForSym, inName, 255 );
  //getFileName( displayNameForSym, inName, 127 );
  //getFilePrefix( prefixForSym, inName, 127 );
  //getFilePostfix( postfixForSym, inName, 127 );

}


FILE *activeWindowClass::openAny (
  char *name,
  char *mode )
{

char buf[255+1];
FILE *f;
int i;

  //printf( "standard ext is [%s]\n", activeWindowClass::stdExt() );
  //printf( "default ext is [%s]\n", activeWindowClass::defExt() );
  //printf( "default search mask is [%s]\n", activeWindowClass::defMask() );

  for ( i=0; i<appCtx->numPaths; i++ ) {

    //appCtx->expandFileName( i, buf, name, ".edl", 255 );
    appCtx->expandFileName( i, buf, name, activeWindowClass::defExt(), 255 );

    if ( strcmp( buf, "" ) != 0 ) {
      //f = fopen( buf, mode );
      f = fileOpen( buf, mode );
      if ( f ) {
        strncpy( fileName, buf, 255 ); // update fileName
        storeFileNameForSymbols( buf ); // update int sym file name components
        return f;
      }
    }

  }

  return NULL;

}

FILE *activeWindowClass::openAnyTemplate (
  char *name,
  char *mode )
{

char buf[255+1];
FILE *f;
int i;

  for ( i=0; i<appCtx->numPaths; i++ ) {

    //appCtx->expandFileName( i, buf, name, ".edl", 255 );
    appCtx->expandFileName( i, buf, name, activeWindowClass::defExt(), 255 );

    if ( strcmp( buf, "" ) != 0 ) {
      f = fileOpen( buf, mode );
      if ( f ) {
        return f;
      }
    }

  }

  return NULL;

}

FILE *activeWindowClass::openAnyTemplateParam (
  char *name,
  char *mode )
{

char buf[255+1];
FILE *f;
int i;

  for ( i=0; i<appCtx->numPaths; i++ ) {

    appCtx->expandFileName( i, buf, name, ".tmpl", 255 );

    if ( strcmp( buf, "" ) != 0 ) {
      f = fileOpen( buf, mode );
      if ( f ) {
        return f;
      }
    }

  }

  return NULL;

}

FILE *activeWindowClass::openAnySymFile (
  char *name,
  char *mode )
{

char buf[255+1];
FILE *f;
int i;

  for ( i=0; i<appCtx->numPaths; i++ ) {

    //appCtx->expandFileName( i, buf, name, ".edl", 255 );
    appCtx->expandFileName( i, buf, name, activeWindowClass::defExt(), 255 );

    if ( strcmp( buf, "" ) != 0 ) {
      //f = fopen( buf, mode );
      f = fileOpen( buf, mode );
      if ( f ) {
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
      //f = fopen( buf, mode );
      f = fileOpen( buf, mode );
      if ( f ) {
        strncpy( fileName, buf, 255 ); // update fileName
        storeFileNameForSymbols( buf ); // update int sym file name components
        return f;
      }
    }

  }

  return NULL;

}

FILE *activeWindowClass::openAnyGenericFile (
  char *name,
  char *mode,
  char *fullName,
  int max )
{

FILE *f;
int i;

  for ( i=0; i<appCtx->numPaths; i++ ) {

    appCtx->expandFileName( i, fullName, name, max );

    if ( strcmp( fullName, "" ) != 0 ) {
      //f = fopen( fullName, mode );
      f = fileOpen( fullName, mode );
      if ( f ) {
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
    char name[255+1], oldName[255+1], *envPtr;

    doAutoSave = 0;

    if ( !changeSinceAutoSave ) return;

    changeSinceAutoSave = 0;

    strncpy( oldName, autosaveName, 255 );
    oldName[255] = 0;

    envPtr = getenv( environment_str8 );
    if ( envPtr ) {
      strncpy( autosaveName, envPtr, 255 );
      if ( envPtr[strlen(envPtr)] != '/' ) {
        Strncat( autosaveName, "/", 255 );
      }
    }
    else {
      strncpy( autosaveName, "/tmp/", 255 );
    }

    Strncat( autosaveName, activeWindowClass_str1, 255 );

    if ( strcmp( fileName, "" ) != 0 ) {
      extractName( fileName, name );
      Strncat( autosaveName, "_", 255 );
      Strncat( autosaveName, name, 255 );
    }

    Strncat( autosaveName, "_XXXXXX", 255 );

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
      restoreTimer = appAddTimeOut( appCtx->appContext(),
       3000, acw_restoreTitle, this );

    }

    // now delete previous autosave file
    if ( strcmp( oldName, "" ) != 0 ) {
      stat = unlink( oldName );
    }

  }


  if ( doActiveClose ) {

    if ( mode != AWC_EDIT ) {

      if ( waiting == 0 ) {
        doActiveClose = 0;
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

        doActiveClose = 0;

    }

  }
  else if ( doClose ) {

    if ( mode == AWC_EDIT ) {

      if ( waiting == 0 ) {

        doClose = 0;

        if ( change ) {

          savedState = state;
          state = AWC_WAITING;

          confirm.create( top, "confirm", x, y, 3,
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

    }

  }
  else {

    doActiveClose = 0;
    doClose = 0;

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

char param[1023+1], tmp[1023+1], dspName[127+1], *envPtr, *ptr, *pvVal;
int i, len, iIn, iOut, p0, p1, more, state, winid, isEnvVar, isPvVal;

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
          Strncat( bufOut, param, max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	      }
        else if ( strcmp( param, "<FILESPEC>" ) == 0 ) {
          bufOut[iOut] = 0;
          Strncat( bufOut, fileNameForSym, max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	      }
        else if ( strcmp( param, "<FILEPREFIX>" ) == 0 ) {
          bufOut[iOut] = 0;
          Strncat( bufOut, prefixForSym, max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	      }
        else if ( strcmp( param, "<FILENAME>" ) == 0 ) {
          bufOut[iOut] = 0;
          Strncat( bufOut, displayNameForSym, max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	      }
        else if ( strcmp( param, "<FILEPOSTFIX>" ) == 0 ) {
          bufOut[iOut] = 0;
          Strncat( bufOut, postfixForSym, max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	      }
        else if ( strcmp( param, "<TITLE>" ) == 0 ) {
          bufOut[iOut] = 0;
          if ( expStrTitle.getExpanded() ) {
            if ( !blank( expStrTitle.getExpanded() ) ) {
              Strncat( bufOut, expStrTitle.getExpanded(), max );
	          }
	          else {
              Strncat( bufOut, activeWindowClass_str83, max );
	          }
	        }
          else {
            Strncat( bufOut, activeWindowClass_str83, max );
	        }
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	      }
        else if ( strcmp( param, "<PROJDIR>" ) == 0 ) {
          bufOut[iOut] = 0;
          Strncat( bufOut, appCtx->dataFilePrefix[0], max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	      }
        else if ( strcmp( param, "<HELPDIR>" ) == 0 ) {
          bufOut[iOut] = 0;
          envPtr = getenv( environment_str5 );
          if ( envPtr ) {
            Strncat( bufOut, envPtr, max );
            iOut = strlen( bufOut );
            if ( iOut >= max ) iOut = max - 1;
	        }
	      }
        else if ( strcmp( param, "<DSPNAME>" ) == 0 ) {
          bufOut[iOut] = 0;
          strncpy( dspName, XDisplayName(appCtx->displayName), 127 );
          Strncat( bufOut, dspName, max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	      }
        else if ( strcmp( param, "<DSPID>" ) == 0 ) {
          bufOut[iOut] = 0;
          strncpy( dspName, XDisplayName(appCtx->displayName), 127 );
          for ( i=0; i<(int) strlen(dspName); i++ ) {
            if ( dspName[i] == '.' ) dspName[i] = '-';
	        }
          Strncat( bufOut, dspName, max );
          iOut = strlen( bufOut );
          if ( iOut >= max ) iOut = max - 1;
	      }
        else if ( strncmp( param, "<val:", 5 ) == 0 ) {

          isPvVal = 1;
          strncpy( tmp, param, 1023 );
          tmp[1023] = 0;

          ptr = strstr( tmp, ">" );
          if ( ptr ) {
            *ptr = 0;
            pvVal = NULL;
            pvVal = getPvValSync( &tmp[5] );
            if ( pvVal ) {
              bufOut[iOut] = 0;
              Strncat( bufOut, pvVal, max );
              delete[] pvVal;
              pvVal = NULL;
              iOut = strlen( bufOut );
              if ( iOut >= max ) iOut = max - 1;
            }
            else {
              isPvVal = 0;
            }
          }
          else {
            isPvVal = 0;
          }

          if ( !isPvVal ) {

            bufOut[iOut] = 0;
            Strncat( bufOut, param, max );
            iOut = strlen( bufOut );
            if ( iOut >= max ) iOut = max - 1;

          }

	      }
        else { // maybe an env var

          if ( strncmp( param, "<env:", 5 ) == 0 ) {

            isEnvVar = 1;
            strncpy( tmp, param, 1023 );
            tmp[1023] = 0;

            ptr = strstr( tmp, ">" );
            if ( ptr ) {
              *ptr = 0;
              envPtr = getenv( &tmp[5] );
              if ( envPtr ) {
                bufOut[iOut] = 0;
                Strncat( bufOut, envPtr, max );
                iOut = strlen( bufOut );
                if ( iOut >= max ) iOut = max - 1;
	            }
              else {
                isEnvVar = 0;
	            }
	          }
	          else {
              isEnvVar = 0;
	          }

	        }
	        else {
    
            isEnvVar = 0;

	        }

          if ( !isEnvVar ) {

            bufOut[iOut] = 0;
            Strncat( bufOut, param, max );
            iOut = strlen( bufOut );
            if ( iOut >= max ) iOut = max - 1;

	        }

	      }

        state = 1; //copying
      }
      break;

    }

    iIn++;

    if ( iIn >= max ) more = 0;

  }

  bufOut[max] = 0;

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
  XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
  dragPopup = XmCreatePopupMenu( top, "dragmenu", args, n );

  str = XmStringCreateLocalized( label );

  labelW = XtVaCreateManagedWidget( "draglabel", xmLabelWidgetClass,
   dragPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  sepW = XtVaCreateManagedWidget( "dragsep", xmSeparatorWidgetClass,
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
  XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
  dragPopup = XmCreatePopupMenu( top, "dragmenu", args, n );

  dragItemIndex = 0;

}

void activeWindowClass::popupDragAddItem (
  void *actGrfPtr,
  char *item )
{

XmString str;
Widget pb;

  str = XmStringCreateLocalized( item );

  pb = XtVaCreateManagedWidget( "dragpb", xmPushButtonWidgetClass,
   dragPopup,
   XmNlabelString, str,
   NULL );

  XmStringFree( str );

  dragPopupBlock[dragItemIndex].w = pb;
  dragPopupBlock[dragItemIndex].num = dragItemIndex;
  dragPopupBlock[dragItemIndex].ago = actGrfPtr;

  XtAddCallback( pb, XmNactivateCallback, dragMenuCb,
   (XtPointer) &dragPopupBlock[dragItemIndex] );

  if ( dragItemIndex < MAX_DRAG_ITEMS-1 ) dragItemIndex++;

}

void activeWindowClass::popupDragFinish (
  XButtonEvent *be )
{

  XmMenuPosition( dragPopup, be );
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

  waiting = cycles;
  doActiveClose = 1;
  appCtx->postDeferredExecutionQueue( this );

}

void activeWindowClass::closeAnyDeferred (
  int cycles )
{

  if ( mode == AWC_EDIT ) {
    waiting = 0;
    doClose = 1;
  }
  else {
    waiting = cycles;
    doActiveClose = 1;
  }

  appCtx->postDeferredExecutionQueue( this );

}

int activeWindowClass::checkPoint (
  int primaryServer,
  FILE *fptr )
{

int i;

  if ( fptr ) {
    //fprintf( fptr, "    name=%s\n", fileName );
    //fprintf( fptr, "    x=%-d\n", x );
    //fprintf( fptr, "    y=%-d\n", y );
    //fprintf( fptr, "    icon=%-d\n", isIconified );
    //fprintf( fptr, "    noEdit=%-d\n", noEdit );
    //fprintf( fptr, "    macros {\n" );
    //fprintf( fptr, "      num=%-d\n", numMacros );
    //for ( i=0; i<numMacros; i++ ) {
    //  fprintf( fptr, "      %s=%s\n", macros[i], expansions[i] );
    //}
    //fprintf( fptr, "    }\n" );
    fprintf( fptr, "%s\n", fileName );
    fprintf( fptr, "%-d\n", x-4 );
    fprintf( fptr, "%-d\n", y-25 );
    fprintf( fptr, "%-d\n", isIconified );
    fprintf( fptr, "%-d\n", noEdit );
    //fprintf( fptr, "    macros {\n" );
    fprintf( fptr, "%-d\n", numMacros );
    for ( i=0; i<numMacros; i++ ) {
      fprintf( fptr, "%s=%s\n", macros[i], expansions[i] );
    }
    //fprintf( fptr, "    }\n" );
  }
  else {
    fprintf( stderr, "name=%s\tx=%-d\ty=%-d\ti=%-d\n", fileName, x, y,
     isIconified );
    fprintf( stderr, "num=%-d\n", numMacros );
    for ( i=0; i<numMacros; i++ ) {
      fprintf( stderr, "%s=%s\n", macros[i], expansions[i] );
    }
  }

  return 1;

}

void activeWindowClass::openExecuteSysFile (
  char *fName )
{

activeWindowListPtr cur;
char buf[255+1], *envPtr;
int i, numSysMacros;
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
    if ( strcmp( fName, cur->node.displayName ) == 0 ) {
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
      Strncat( buf, "/", 255 );
    }

  }
  else {

    strcpy( buf, "/etc/edm/" );

  }

  // build system macros

  numSysMacros = 0;

  ptr = new char[strlen(buf)+1];
  strcpy( ptr, buf );
  sysValues[0] = ptr;

  numSysMacros++;

  // ============

  Strncat( buf, fName, 255 );
  //Strncat( buf, ".edl", 255 );
  Strncat( buf, activeWindowClass::defExt(), 255 );

  cur = new activeWindowListType;
  appCtx->addActiveWindow( cur );

  cur->node.createNoEdit( appCtx, NULL, 0, 0, 0, 0, numSysMacros,
   sysMacros, sysValues );

  for ( i=0; i<numSysMacros; i++ ) {
    delete[] sysValues[i];
  }

  cur->node.realize();
  cur->node.setGraphicEnvironment( &appCtx->ci, &appCtx->fi );

  cur->node.storeFileName( buf );

  appCtx->openActivateActiveWindow( &cur->node );

}

void activeWindowClass::requestReload ( void ) {

  reloadRequestFlag = 1;
  appCtx->requestSelectedReload();

}

void activeWindowClass::reloadSelf ( void ) {

activeGraphicListPtr curCut, nextCut, cur, next;
commentLinesPtr commentCur, commentNext;
pvDefPtr pvDefCur, pvDefNext;

  if ( mode == AWC_EXECUTE ) {
    clearActive();
  }
  else {
    clear();
  }

  if ( ef.formIsPoppedUp() ) ef.popdown();

  if ( autosaveTimer ) {
    XtRemoveTimeOut( autosaveTimer );
    autosaveTimer = 0;
  }
  if ( restoreTimer ) {
    XtRemoveTimeOut( restoreTimer );
    restoreTimer = 0;
  }

  // flush undo list
  undoObj.flush();

  // init some items

  autosaveTimer = 0;
  doAutoSave = 0;
  doClose = 0;
  doActiveClose = 0;
  waiting = 0;
  restoreTimer = 0;
  change = 0;
  changeSinceAutoSave = 0;
  exit_after_save = 0;
  state = AWC_NONE_SELECTED;
  updateMasterSelection();
  currentEf = NULL;
  oldx = -1;
  oldy = -1;
  useFirstSelectedAsReference = 0;
  gridActive = 0;
  gridShow = 0;
  strcpy( id, "" );
  strcpy( title, "" );
  strcpy( autosaveName, "" );
  showName = 0;

  defExeHead->defExeFlink = defExeHead;
  defExeHead->defExeBlink = defExeHead;

  enterActionHead->flink = enterActionHead;
  enterActionHead->blink = enterActionHead;

  btnDownActionHead->flink = btnDownActionHead;
  btnDownActionHead->blink = btnDownActionHead;

  btnUpActionHead->flink = btnUpActionHead;
  btnUpActionHead->blink = btnUpActionHead;

  btnMotionActionHead->flink = btnMotionActionHead;
  btnMotionActionHead->blink = btnMotionActionHead;

  btnFocusActionHead->flink = btnFocusActionHead;
  btnFocusActionHead->blink = btnFocusActionHead;

  // empty cut list
  curCut = cutHead->flink;
  while ( curCut != cutHead ) {
    nextCut = curCut->flink;
    delete curCut->node;
    delete curCut;
    curCut = nextCut;
  }
  cutHead->flink = cutHead;
  cutHead->blink = cutHead;

  // delete those things created when the file is loaded

  commentCur = commentHead->flink;
  while ( commentCur ) {
    commentNext = commentCur->flink;
    if ( commentCur->line ) delete[] commentCur->line;
    delete commentCur;
    commentCur = commentNext;
  }
  commentTail = commentHead;
  commentTail->flink = NULL;

  pvDefCur = pvDefHead->flink;
  while ( pvDefCur ) {
    pvDefNext = pvDefCur->flink;
    if ( pvDefCur->def ) delete[] pvDefCur->def;
    delete pvDefCur;
    pvDefCur = pvDefNext;
  }
  pvDefTail = pvDefHead;
  pvDefTail->flink = NULL;

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

  selectedHead->selFlink = selectedHead;
  selectedHead->selBlink = selectedHead;

}

int activeWindowClass::isExecuteMode ( void ) {

  return ( mode == AWC_EXECUTE );

}

int activeWindowClass::xPos ( void ) {

int embeddedParentX, actualEmbeddedX, actualX, actualY;

  if ( isEmbedded && parent ) {

    short drawWinX, drawWinY;
    XtVaGetValues( top,
     XmNx, &drawWinX,
     XmNy, &drawWinY,
     NULL );
    embeddedParentX = (int) drawWinX;

    XtVaGetValues( drawWidget,
     XmNx, &drawWinX,
     XmNy, &drawWinY,
     NULL );
    actualEmbeddedX = (int) drawWinX;

    return parent->xPos() + embeddedParentX + actualEmbeddedX;

  }
  else {

    getDrawWinPos( &actualX, &actualY );
    return x + actualX;

  }

}

int activeWindowClass::yPos ( void ) {

int embeddedParentY, actualEmbeddedY, actualX, actualY;

  if ( isEmbedded && parent ) {

    short drawWinX, drawWinY;
    XtVaGetValues( top,
     XmNx, &drawWinX,
     XmNy, &drawWinY,
     NULL );
    embeddedParentY = (int) drawWinY;

    XtVaGetValues( drawWidget,
     XmNx, &drawWinX,
     XmNy, &drawWinY,
     NULL );
    actualEmbeddedY = (int) drawWinY;

    return parent->yPos() + embeddedParentY + actualEmbeddedY;

  }
  else {

    getDrawWinPos( &actualX, &actualY );
    return y + actualY;

  }

}

void activeWindowClass::move (
  int newX,
  int newY
) {

int n;
Arg args[10];

  n = 0;
  XtSetArg( args[n], XmNx, (XtArgVal) newX ); n++;
  XtSetArg( args[n], XmNy, (XtArgVal) newY ); n++;

  if ( scroll ) {
    XtSetValues( scroll, args, n );
  }
  else {
    XtSetValues( drawWidget, args, n );
  }

}

void activeWindowClass::getDrawWinPos (
  int *curX,
  int *curY
) {

  if ( scroll ) {
    short drawWinX, drawWinY;
    XtVaGetValues( drawWidget,
     XmNx, &drawWinX,
     XmNy, &drawWinY,
     NULL );
    *curX = (int) drawWinX;
    *curY = (int) drawWinY;
  }
  else {
    *curX = 0;
    *curY = 0;
  }

}

int activeWindowClass::sameAncestorName (
  char *name
) {

activeWindowClass *aw;

  aw = this;
  while ( aw ) {

    if ( strcmp( name, aw->displayName ) == 0 ) return 1;

    aw = aw->parent;

  }

  return 0;

}

void activeWindowClass::freeze ( bool flag) {

  this->frozen = flag;

}

bool activeWindowClass::is_frozen(void){

  return this->frozen;

}

#ifdef ADD_SCROLLED_WIN

void activeWindowClass::reconfig ( void ) {

Arg args[5];
Cardinal n;

Dimension maxW = WidthOfScreen( XtScreen(top) ), scrollw;
Dimension maxH = HeightOfScreen( XtScreen(top) ), scrollh;

  n = 0;
  XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
  XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
  XtSetArg( args[n], XmNwidth, (XtArgVal) w ); n++;
  XtSetArg( args[n], XmNheight, (XtArgVal) h ); n++;

  if ( !scroll ) {

    XtSetValues( drawWidget, args, 2 );
    XtSetValues( top, args+2, 2 );

  }
  else {

    XtSetValues( top, args, n );

    XtSetArg( args[0], XmNx, (XtArgVal) 0 );
    XtSetArg( args[1], XmNy, (XtArgVal) 0 );
    XtSetValues( drawWidget, args, n );

    n = 0;
    XtSetArg( args[n], XmNwidth, (XtArgVal)&scrollw ); n++;
    XtSetArg( args[n], XmNheight, (XtArgVal)&scrollh ); n++;
    XtGetValues(top, args, n);

    if ( scrollw > maxW ) scrollw = maxW;
    if ( scrollh > maxH ) scrollh = maxH;
    XtSetArg( args[0], XmNwidth, (XtArgVal)scrollw );
    XtSetArg( args[1], XmNheight, (XtArgVal)scrollh );
    XtSetValues( top, args, n );

  }

}

static void b2ReleaseClip_cb (
  Widget w,
  XtPointer client_data,
  XtPointer call_data
) {

Dimension newW, newH;
activeWindowClass *awc = (activeWindowClass*)client_data;
Widget clip = 0;
Widget scroll = awc->scrollWidgetId();

  awc->setChanged();

  if ( scroll )
    XtVaGetValues( scroll, XmNclipWindow, &clip, NULL );

  if ( !clip ) {
    XtWarning("b2ReleaseClip_cb(): no clipWindow found");
    return;
  }

  XtVaGetValues(
   clip,
   XmNwidth, &newW,
   XmNheight, &newH,
   NULL );

  XtVaSetValues( awc->drawWidget, XmNwidth, newW, XmNheight, newH, NULL );

  XtVaSetValues( awc->top,XmNwidth, newW, XmNheight, newH, NULL );

  awc->w = newW;
  awc->h = newH;

}

#endif
