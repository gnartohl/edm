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

#define __mp_cc 1

#include "mp.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void mpc_sendMsg (
  XtPointer client,
  XtIntervalId *id )
{

activeMpClass *mpo = (activeMpClass *) client;

  if ( mpo->timerActive ) {
    mpo->msgPollTimer = XtAppAddTimeOut(
     mpo->actWin->appCtx->appContext(),
     1000, mpc_sendMsg, client );
  }
  else {
    return;
  }

  // mpo->actWin->appCtx->postDeferredExecutionQueue( mpo );

  //  This causes program to ultimately crash
  //  stat = g2a_send_void( &mpo->Player, COM_GET_POS, 0 );

}

static void mpc_readMsg (
  XtPointer client,
  int *fd,
  XtInputId *id )
{

activeMpClass *mpo = (activeMpClass *) client;
int stat;
PMP_HEADER msg;
PMP_DATA_A2G data;
/* for the msg.h package */
// MSG_STRUCT Msg;
Arg args[10];
int n, *iPtr;
PMP_DATA_VIDEO_INFOS *videoInfo;

  if ( mpo->inputDisabled ) {
    printf( "inputDisabled\n" );
    return;
  }

  stat = a2g_read_msg( &mpo->Player, &msg, &data );
  if ( stat != -1 ) {

#if 0
    {
      char buf[PMP_PRINT_MSG_BUF_SIZE];
      MESSAGE(("a2g_read_command: received msg: %s",
       pmp_print_msg( &msg, (const char *) &data, buf)));
      printf( "a2g_read_command: received msg: %s\n",
       pmp_print_msg( &msg, (const char *) &data, buf ) );
    }
#endif

    switch ( (A2G_COM) msg.type ) {

    case A2G_COM_VIDEO_INFOS:

      videoInfo = (PMP_DATA_VIDEO_INFOS *) &data;

#if 0
      printf( "width = %-d\n", videoInfo->width );
      printf( "height = %-d\n", videoInfo->height );
      printf( "bitrate = %-d\n", videoInfo->bitrate );
      printf( "vbv_buffer_size = %-d\n", videoInfo->vbv_buffer_size );
      printf( "framerate = %-g\n", videoInfo->framerate );
      printf( "pel_aspect_ratio = %-g\n", videoInfo->pel_aspect_ratio );
#endif

      n = 0;
      XtSetArg( args[n], XmNwidth, (short) videoInfo->width * 2 ); n++;
      XtSetArg( args[n], XmNheight, (short) videoInfo->height * 2 ); n++;
      XtSetValues( mpo->frame, args, n );

      break;

    case A2G_COM_AUDIO_DISABLED:
      mpo->playerHasQuit = 1;
      break;

    case A2G_COM_NUMBER_OF_FRAMES_PLAYED:
      if ( mpo->framesPlayedExists ) {
        iPtr = (int *) &data;

#ifdef __epics__
        stat = ca_put( DBR_LONG, mpo->framesPlayedPvId, iPtr );
#endif

      }
      break;

    default:
      break;

    }

  }
//    else {

//      printf( "message error, stat = %-d\n", stat );

//    }

}

static void mpc_putValue (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpClass *mpo = (activeMpClass *) client;
int i, stat, winId;
short value;

  for ( i=0; i<mpo->numStates; i++ ) {

    if ( w == mpo->pb[i] ) {

      value = (short) i;

      switch ( value ) {

      case 0: // stop
        stat = g2a_send_int( &mpo->Player, COM_STATE, 0, STATE_PAUSED );
	  //        stat = g2a_send_void( &mpo->Player, COM_STEP, 0 );
        break;

      case 1: // play
        if ( !mpo->fileOpen ) {

          stat = g2a_send_int( &mpo->Player, COM_ZOOM, 0, ZOOM_2X2 );

          winId = (int) XtWindow( mpo->frame );

          stat = g2a_send_int( &mpo->Player, COM_WINDOW_ID, 0, winId );

          stat = g2a_send_int( &mpo->Player, COM_WINDOW_POSITION_X, 0,
	   mpo->x );
          stat = g2a_send_int( &mpo->Player, COM_WINDOW_POSITION_Y, 0,
	   mpo->y + mpo->h );

          stat = g2a_send_int( &mpo->Player, COM_VIDEO_LAYOUT, 0,
           VIDEO_LAYOUT_TOP_LEFT );

          stat = g2a_send_int( &mpo->Player, COM_LOOP, 0, 1 );

          stat = g2a_send_string( &mpo->Player, COM_OPEN_FILE, 0,
           mpo->controlV );

          stat = g2a_send_int( &mpo->Player, COM_STATE, 0, STATE_PLAYING );

          mpo->fileOpen = 1;

	}
	else {

          stat = g2a_send_int( &mpo->Player, COM_STATE, 0, STATE_PLAYING );

	}

        break;

      case 2: // rewind
        stat = g2a_send_int( &mpo->Player, COM_SEEK, 0, 0 );
        break;

      }

      return;
    }

  }

}

#ifdef __epics__

static void mpc_monitor_control_connect_state (
  struct connection_handler_args arg )
{

activeMpClass *mpo = (activeMpClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    mpo->needCtlConnectInit = 1;

  }
  else {

    mpo->controlPvConnected = 0;
    mpo->fgColor.setDisconnected();
    mpo->notActive |= 1;
    mpo->needDraw = 1;

  }

  mpo->actWin->appCtx->proc->lock();
  mpo->actWin->addDefExeNode( mpo->aglPtr );
  mpo->actWin->appCtx->proc->unlock();

}

static void mpc_monitor_framesPlayed_connect_state (
  struct connection_handler_args arg )
{

activeMpClass *mpo = (activeMpClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    mpo->needFrPlConnectInit = 1;

  }
  else {

    mpo->framesPlayedPvConnected = 0;
    mpo->fgColor.setDisconnected();
    mpo->notActive |= 2;
    mpo->needDraw = 1;

  }

  mpo->actWin->appCtx->proc->lock();
  mpo->actWin->addDefExeNode( mpo->aglPtr );
  mpo->actWin->appCtx->proc->unlock();

}

static void mpc_controlUpdate (
  struct event_handler_args ast_args )
{

activeMpClass *mpo = (activeMpClass *) ast_args.usr;

  strncpy( mpo->curControlV, (char *) ast_args.dbr, 39 );

  mpo->needDraw = 1;
  mpo->actWin->appCtx->proc->lock();
  mpo->actWin->addDefExeNode( mpo->aglPtr );
  mpo->actWin->appCtx->proc->unlock();

  mpo->fileOpen = 0;

}

#endif

static void mpc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpClass *mpo = (activeMpClass *) client;

  mpo->actWin->setChanged();

  mpo->eraseSelectBoxCorners();
  mpo->erase();

  strncpy( mpo->fontTag, mpo->fm.currentFontTag(), 63 );
  mpo->actWin->fi->loadFontTag( mpo->fontTag );
  mpo->actWin->drawGc.setFontTag( mpo->fontTag, mpo->actWin->fi );
  mpo->actWin->fi->getTextFontList( mpo->fontTag, &mpo->fontList );
  mpo->fs = mpo->actWin->fi->getXFontStruct( mpo->fontTag );

  mpo->topShadowColor = mpo->bufTopShadowColor;
  mpo->botShadowColor = mpo->bufBotShadowColor;

  mpo->fgColor.setColor( mpo->bufFgColor, mpo->actWin->ci );

  mpo->bgColor.setColor( mpo->bufBgColor, mpo->actWin->ci );

  mpo->x = mpo->bufX;
  mpo->sboxX = mpo->bufX;

  mpo->y = mpo->bufY;
  mpo->sboxY = mpo->bufY;

  mpo->w = mpo->bufW;
  mpo->sboxW = mpo->bufW;

  mpo->h = mpo->bufH;
  mpo->sboxH = mpo->bufH;

  mpo->controlPvExpStr.setRaw( mpo->bufControlPvName );

  mpo->framesPlayedPvExpStr.setRaw( mpo->bufframesPlayedPvName );

  mpo->updateDimensions();

}

static void mpc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpClass *mpo = (activeMpClass *) client;

  mpc_edit_update ( w, client, call );
  mpo->refresh( mpo );

}

static void mpc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpClass *mpo = (activeMpClass *) client;

  mpc_edit_update ( w, client, call );
  mpo->ef.popdown();
  mpo->operationComplete();

}

static void mpc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpClass *mpo = (activeMpClass *) client;

  mpo->ef.popdown();
  mpo->operationCancel();

}

static void mpc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeMpClass *mpo = (activeMpClass *) client;

  mpo->erase();
  mpo->deleteRequest = 1;
  mpo->ef.popdown();
  mpo->operationCancel();
  mpo->drawAll();

}

activeMpClass::activeMpClass ( void ) {

int i;

  name = new char[strlen("activeMpClass")+1];
  strcpy( name, "activeMpClass" );

  for ( i=0; i<MPC_NUM_STATES; i++ ) {
    pb[i] = NULL;
  }

  activeMode = 0;
  widgetsCreated = 0;
  fileOpen = 0;
  inputDisabled = 1;
  timerActive = 0;
  fontList = NULL;

}

activeMpClass::~activeMpClass ( void ) {

  if ( name ) delete name;
  if ( fontList ) XmFontListFree( fontList );

}

// copy constructor
activeMpClass::activeMpClass
 ( const activeMpClass *source ) {

int i;
activeGraphicClass *mpo = (activeGraphicClass *) this;

  mpo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeMpClass")+1];
  strcpy( name, "activeMpClass" );

  for ( i=0; i<MPC_NUM_STATES; i++ ) {
    pb[i] = NULL;
  }

  strncpy( fontTag, source->fontTag, 63 );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;

  topShadowColor = source->topShadowColor;
  botShadowColor = source->botShadowColor;
  topShadowCb = source->topShadowCb;
  botShadowCb = source->botShadowCb;

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);
  fgCb = source->fgCb;
  bgCb = source->bgCb;

  controlPvExpStr.setRaw( source->controlPvExpStr.rawString );

  framesPlayedPvExpStr.setRaw( source->framesPlayedPvExpStr.rawString );

  widgetsCreated = 0;
  activeMode = 0;
  fileOpen = 0;
  inputDisabled = 1;

}

int activeMpClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  strcpy( fontTag, actWin->defaultFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColor( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColor( actWin->defaultBgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int activeMpClass::save (
  FILE *f )
{

int r, g, b;

  fprintf( f, "%-d %-d %-d\n", MPC_MAJOR_VERSION, MPC_MINOR_VERSION,
   MPC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getRGB( fgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( bgColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( topShadowColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  actWin->ci->getRGB( botShadowColor, &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  if ( controlPvExpStr.getRaw() )
    writeStringToFile( f, controlPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( framesPlayedPvExpStr.getRaw() )
    writeStringToFile( f, framesPlayedPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  return 1;

}

int activeMpClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b;
int major, minor, release;
unsigned int pixel;
char oneName[39+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  actWin->ci->setRGB( r, g, b, &pixel );
  fgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  actWin->ci->setRGB( r, g, b, &pixel );
  bgColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  actWin->ci->setRGB( r, g, b, &topShadowColor );

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  actWin->ci->setRGB( r, g, b, &botShadowColor );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  controlPvExpStr.setRaw( oneName );

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    readStringFromFile( oneName, 39, f ); actWin->incLine();
    framesPlayedPvExpStr.setRaw( oneName );
  }
  else {
    framesPlayedPvExpStr.setRaw( "" );
  }

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return 1;

}

int activeMpClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeMpClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  strncat( title, " Properties", 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  strncpy( bufFontTag, fontTag, 63 );

  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;

  bufFgColor = fgColor.pixelColor();

  bufBgColor = bgColor.pixelColor();

  if ( controlPvExpStr.getRaw() )
    strncpy( bufControlPvName, controlPvExpStr.getRaw(), 39 );
  else
    strncpy( bufControlPvName, "", 39 );

  if ( framesPlayedPvExpStr.getRaw() )
    strncpy( bufframesPlayedPvName, framesPlayedPvExpStr.getRaw(), 39 );
  else
    strncpy( bufframesPlayedPvName, "", 39 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "X", 27, &bufX );
  ef.addTextField( "Y", 27, &bufY );
  ef.addTextField( "Width", 27, &bufW );
  ef.addTextField( "Height", 27, &bufH );
  ef.addColorButton( "FG Color", actWin->ci, &fgCb, &bufFgColor );
  ef.addOption( "FG Color Mode", "Static|Alarm", &bufFgColorMode );
  ef.addColorButton( "BG Color", actWin->ci, &bgCb, &bufBgColor );
//   ef.addOption( "BG Color Mode", "Static|Alarm", &bufBgColorMode );
  ef.addColorButton( "Top Shadow", actWin->ci, &topShadowCb,
   &bufTopShadowColor );
  ef.addColorButton( "Bottom Shadow", actWin->ci, &botShadowCb,
   &bufBotShadowColor );
  ef.addFontMenu( "Font", actWin->fi, &fm, fontTag );
  ef.addTextField( "Filename PV", 27, bufControlPvName, 39 );
  ef.addTextField( "Frames Played PV", 27, bufframesPlayedPvName, 39 );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int activeMpClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( mpc_edit_ok, mpc_edit_apply, mpc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeMpClass::edit ( void ) {

  this->genericEdit();
  ef.finished( mpc_edit_ok, mpc_edit_apply, mpc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeMpClass::erase ( void ) {

  if ( deleteRequest || activeMode ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activeMpClass::eraseActive ( void ) {

  return 1;

}

int activeMpClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };

  actWin->drawGc.saveFg();

  if ( deleteRequest || activeMode ) return 1;

  actWin->drawGc.setFG( bgColor.pixelColor() );
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
	      XmALIGNMENT_CENTER, "MP" );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int activeMpClass::drawActive ( void ) {

Arg args[10];
int n;
// Widget w;

  if ( !activeMode || !widgetsCreated ) return 1;

// set colors

//   w = XmOptionButtonGadget( optionMenu );
//   n = 0;
//   XtSetArg( args[n], XmNbackground, bgColor.getColor() ); n++;
//   XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;
//   XtSetValues( w, args, n );

  n = 0;
  XtSetArg( args[n], XmNforeground, fgColor.getColor() ); n++;
  XtSetValues( optionMenu, args, n );

  return 1;

}

int activeMpClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand1st( numMacros, macros, expansions );
  stat = framesPlayedPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeMpClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = controlPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = framesPlayedPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeMpClass::containsMacros ( void ) {

  return controlPvExpStr.containsPrimaryMacros();
  return framesPlayedPvExpStr.containsPrimaryMacros();

}

int activeMpClass::createWidgets ( void ) {

  return 1;

}

int activeMpClass::activate (
  int pass,
  void *ptr )
{

int i, stat, opStat;
XmString str;
Arg args[10];
int n;

const char *mtvp_argv[] = {
  "mtvp",
  NULL
};

  switch ( pass ) {

  case 1:

    needCtlConnectInit = needFrPlConnectInit = needDraw = 0;
    aglPtr = ptr;
    opComplete = 0;

#ifdef __epics__
    controlEventId = 0;
#endif

    controlPvConnected = 0;
    framesPlayedPvConnected = 0;
    notActive = 0;
    activeMode = 1;
    widgetsCreated = 0;
    fileOpen = 0;
    optionMenu = (Widget) NULL;
    frame = (Widget) NULL;

    if ( strcmp( controlPvExpStr.getRaw(), "" ) != 0 ) {
      controlExists = 1;
      notActive |= 1;
    }
    else
      controlExists = 0;

    if ( strcmp( framesPlayedPvExpStr.getRaw(), "" ) != 0 ) {
      framesPlayedExists = 1;
      notActive |= 2;
    }
    else
      framesPlayedExists = 0;

    break;

  case 2:

    if ( !widgetsCreated ) {

      pulldownMenu = XmCreatePulldownMenu( actWin->executeWidgetId(), "",
       NULL, 0 );

      char *stateString[] = { "Stop", "Play", "Rewind" };

      numStates = sizeof(stateString) / sizeof(char *);

      for ( i=0; i<numStates; i++ ) {

        str = XmStringCreate( stateString[i], fontTag );

        pb[i] = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         pulldownMenu,
         XmNlabelString, str,
         XmNfontList, fontList,
         NULL );

        XmStringFree( str );

        n = 0;
        XtSetArg( args[n], XmNwidth, w-42 ); n++;
        XtSetArg( args[n], XmNheight, h-14 ); n++;
        XtSetValues( pb[i], args, n );

        XtAddCallback( pb[i], XmNactivateCallback, mpc_putValue,
         (XtPointer) this );

      }

      curHistoryWidget = pb[0];

      n = 0;
      XtSetArg( args[n], XmNsubMenuId, (XtArgVal) pulldownMenu ); n++;
      XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curHistoryWidget );
       n++;
      XtSetArg( args[n], XmNx, (XtArgVal) x ); n++;
      XtSetArg( args[n], XmNy, (XtArgVal) y ); n++;
      XtSetArg( args[n], XmNbackground,
       (XtArgVal) actWin->executeGc.getBaseBG() ); n++;
      XtSetArg( args[n], XmNhighlightColor,
       (XtArgVal) actWin->executeGc.getBaseBG() ); n++;
      XtSetArg( args[n], XmNhighlightPixmap, (XtArgVal) None ); n++;
      XtSetArg( args[n], XmNtopShadowColor, (XtArgVal) topShadowColor );
       n++;
      XtSetArg( args[n], XmNbottomShadowColor, (XtArgVal) botShadowColor );
       n++;
      optionMenu = XmCreateOptionMenu( actWin->executeWidgetId(), "",
       args, n );

      XtManageChild( optionMenu );

      frame = XtVaCreateManagedWidget( "", xmFrameWidgetClass,
       actWin->executeWidgetId(),
       XmNx, x,
       XmNy, y+h+10,
//         XmNwidth, 170,
//         XmNheight, 125,
       XmNwidth, 330,
       XmNheight, 240,
       XmNmarginWidth, 0,
       XmNmarginHeight, 0,
//         XmNtopShadowColor, shadeColor,
//         XmNbottomShadowColor, BlackPixel( actWin->display(),
//          DefaultScreen(actWin->display()) ),
//         XmNshadowType, XmSHADOW_ETCHED_OUT,
       XmNmappedWhenManaged, False,
       NULL );

      XtMapWidget( frame );

      widgetsCreated = 1;

    }

    if ( !opComplete ) {

      opStat = 1;

#ifdef __epics__

      if ( controlExists ) {
        stat = ca_search_and_connect( controlPvExpStr.getExpanded(),
         &controlPvId, mpc_monitor_control_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( "error from ca_search\n" );
          opStat = 0;
        }
      }

      if ( framesPlayedExists ) {
        stat = ca_search_and_connect( framesPlayedPvExpStr.getExpanded(),
         &framesPlayedPvId, mpc_monitor_framesPlayed_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( "error from ca_search\n" );
          opStat = 0;
        }
      }

      if ( !( opStat & 1 ) ) opComplete = 1;

#endif

      stat = g2a_init( &Player, mtvp_argv, "edm" );
      if ( stat == -1 ) {
        actWin->appCtx->postMessage( "Cannot start mtvp" );
      }
      else {

        /* add the io callback */
        msgInputId = XtAppAddInput( actWin->appCtx->appContext(),
         Player.receive_channel.msg_pipe[PMP_FD_READ],
         (XtPointer) XtInputReadMask, mpc_readMsg, (void *) this );
        inputDisabled = 0;

        /* add the timer callback */
        msgPollTimer = XtAppAddTimeOut( actWin->appCtx->appContext(),
         1000, mpc_sendMsg, (void *) this );
        timerActive = 1;

      }

      return opStat;

    }

    break;

  case 3:
  case 4:

    break;

  case 5:

    opComplete = 0;
    break;

  case 6:

    if ( !opComplete ) {
      opComplete = 1;
    }

    break;

  }

  return 1;

}

int activeMpClass::deactivate (
  int pass
) {

int stat;

  activeMode = 0;

  switch ( pass ) {

  case 1:

    playerHasQuit = 0;

    timerActive = 0;
    XtRemoveTimeOut( msgPollTimer );

    inputDisabled = 1;
    XtRemoveInput( msgInputId );

    g2a_send_void( &Player, COM_QUIT, 0 );
    g2a_cleanup(&Player);

//      for ( i=0; i<10; i++ ) {

//        do {
//          result = XtAppPending( actWin->appCtx->appContext() );
//          if ( result ) XtAppProcessEvent( actWin->appCtx->appContext(),
//           result );

//          if ( playerHasQuit ) {
//            printf( "playerHasQuit is true\n" );
//            g2a_cleanup( &Player );
//            break;
//          }

//        } while ( result );

//        if ( playerHasQuit ) break;

//      }

#ifdef __epics__

    if ( controlEventId ) {
      stat = ca_clear_event( controlEventId );
      if ( stat != ECA_NORMAL )
        printf( "ca_clear_event failure\n" );
      controlEventId = 0;
    }

    if ( controlExists ) {
      stat = ca_clear_channel( controlPvId );
      if ( stat != ECA_NORMAL )
        printf( "ca_clear_channel failure\n" );
    }

    if ( framesPlayedExists ) {
      stat = ca_clear_channel( framesPlayedPvId );
      if ( stat != ECA_NORMAL )
        printf( "ca_clear_channel failure\n" );
    }

#endif

    break;

  case 2:

    widgetsCreated = 0;
    if ( optionMenu ) {
      XtUnmapWidget( optionMenu );
      XtDestroyWidget( optionMenu );
      XtDestroyWidget( pulldownMenu );
    }

    if ( frame ) {
      XtUnmapWidget( frame );
      XtDestroyWidget( frame );
    }

    break;

  }

  return 1;

}

void activeMpClass::updateDimensions ( void )
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

}

void activeMpClass::executeFromDeferredQueue ( void ) {

int stat;

//  printf( "activeMpClass::executeFromDeferredQueue\n" );
  stat = g2a_send_void( &Player, COM_GET_NUMBER_OF_FRAMES_PLAYED, 0 );

}

void activeMpClass::executeDeferred ( void ) {

int stat, ncc, nfc, nd;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  ncc = needCtlConnectInit; needCtlConnectInit = 0;
  nfc = needFrPlConnectInit; needFrPlConnectInit = 0;
  nd = needDraw; needDraw = 0;
  strncpy( controlV, curControlV, 39 );
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

#ifdef __epics__

  if ( ncc ) {

    stat = ca_add_masked_array_event( DBR_STRING, 1, controlPvId,
     mpc_controlUpdate, (void *) this, (float) 0.0, (float) 0.0,
    (float) 0.0, &controlEventId, DBE_VALUE );
    if ( stat != ECA_NORMAL ) {
      printf( "ca_add_masked_array_event failed\n" );
    }

    controlPvConnected = 1;
    notActive &= ~( (unsigned int) 1 );

    if ( !notActive ) {
      fgColor.setConnected();
      drawActive();
    }

  }

  if ( nfc ) {

    framesPlayedPvConnected = 1;
    notActive &= ~( (unsigned int) 2 );

    if ( !notActive ) {
      fgColor.setConnected();
      drawActive();
    }

  }

#endif

  if ( nd ) {

    drawActive();

  }

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeMpClassPtr ( void ) {

activeMpClass *ptr;

  ptr = new activeMpClass;
  return (void *) ptr;

}

void *clone_activeMpClassPtr (
  void *_srcPtr )
{

activeMpClass *ptr, *srcPtr;

  srcPtr = (activeMpClass *) _srcPtr;

  ptr = new activeMpClass( srcPtr );

  return (void *) ptr;

}

typedef struct libRecTag {
  char *className;
  char *typeName;
  char *text;
} libRecType, *libRecPtr;

static int libRecIndex = 0;

static libRecType libRec[] = {
  { "activeMpClass", "Controls", "MP" }
};

int firstRegRecord (
  char **className,
  char **typeName,
  char **text )
{

  libRecIndex = 0;

  *className = libRec[libRecIndex].className;
  *typeName = libRec[libRecIndex].typeName;
  *text = libRec[libRecIndex].text;

  return 0;

}

int nextRegRecord (
  char **className,
  char **typeName,
  char **text )
{

int max = sizeof(libRec) / sizeof(libRecType) - 1;

  if ( libRecIndex >= max ) return -1; //no more 
  libRecIndex++;

  *className = libRec[libRecIndex].className;
  *typeName = libRec[libRecIndex].typeName;
  *text = libRec[libRecIndex].text;

  return 0;

}

#ifdef __cplusplus
}
#endif
