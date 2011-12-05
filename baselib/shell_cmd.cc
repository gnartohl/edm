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

#define __shell_cmd_cc 1

#include "shell_cmd.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

typedef struct threadParamBlockTag {
  int multipleInstancesAllowed;
  char *cmd;
  float secondsToDelay;
} threadParamBlockType, *threadParamBlockPtr;

static char * const g_nullHost = "";

#ifdef __linux__
static void *shellCmdThread (
  THREAD_HANDLE h )
{
#endif

#ifdef darwin
static void *shellCmdThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __solaris__
static void *shellCmdThread (
  THREAD_HANDLE h )
{
#endif

#ifdef __osf__
static void shellCmdThread (
  THREAD_HANDLE h )
{
#endif

#ifdef HP_UX
static void *shellCmdThread (
  THREAD_HANDLE h )
{
#endif

int stat;
threadParamBlockPtr threadParamBlock =
 (threadParamBlockPtr) thread_get_app_data( h );

  if ( threadParamBlock->secondsToDelay > 0.0 ) {
    thread_delay( h, threadParamBlock->secondsToDelay );
  }

  stat = executeCmd( threadParamBlock->cmd );

  if ( threadParamBlock->multipleInstancesAllowed ) {
    stat = thread_request_free_ptr( (void *) threadParamBlock->cmd );
    stat = thread_request_free_ptr( (void *) threadParamBlock );
    stat = thread_detached_exit( h, NULL ); // this call deallocates h
  }
  else {
    stat = thread_request_free_ptr( (void *) threadParamBlock->cmd );
    stat = thread_request_free_ptr( (void *) threadParamBlock );
    stat = thread_exit( h, NULL ); // this requires a join
  }

#ifdef __linux__
  return (void *) NULL;
#endif

#ifdef darwin
  return (void *) NULL;
#endif
  
#ifdef __solaris__
  return (void *) NULL;
#endif

}

static void menu_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

int i;
shellCmdClass *shcmdo = (shellCmdClass *) client;

  for ( i=0; i<shcmdo->maxCmds; i++ ) {

    if ( w == shcmdo->pb[i] ) {

      shcmdo->cmdIndex = i;

      if ( shcmdo->usePassword ) {

        if ( !shcmdo->ef.formIsPoppedUp() ) {

          //pwFormX = actWin->xPos() + _x; got these from btnDown
          //pwFormY = actWin->yPos() + _y;
          shcmdo->pwFormW = 0;
          shcmdo->pwFormH = 0;
          shcmdo->pwFormMaxH = 600;

          shcmdo->ef.create( shcmdo->actWin->top,
           shcmdo->actWin->appCtx->ci.getColorMap(),
           &shcmdo->pwFormX, &shcmdo->pwFormY,
           &shcmdo->pwFormW, &shcmdo->pwFormH, &shcmdo->pwFormMaxH,
           "", NULL, NULL, NULL );

          strcpy( shcmdo->bufPw1, "" );

          shcmdo->ef.addPasswordField( shellCmdClass_str24, 35,
           shcmdo->bufPw1, 31 );

          shcmdo->ef.finished( pw_ok, pw_apply, pw_cancel, shcmdo );

          shcmdo->ef.popup();

	  return;

	}
	else {

	  return;

	}

      }
      else {

        shcmdo->actWin->appCtx->proc->lock();
        shcmdo->needExecute = 1;
        shcmdo->actWin->addDefExeNode( shcmdo->aglPtr );
        shcmdo->actWin->appCtx->proc->unlock();
        return;

      }

    }

  }

}

static void pw_ok (
  Widget w,
  XtPointer client,
  XtPointer call ) {

shellCmdClass *shcmdo = (shellCmdClass *) client;

  shcmdo->ef.popdown();

  shcmdo->actWin->appCtx->proc->lock();
  if ( strcmp( shcmdo->bufPw1, shcmdo->pw ) == 0 ) {
    shcmdo->needExecute = 1;
    shcmdo->actWin->addDefExeNode( shcmdo->aglPtr );
  }
  else {
    shcmdo->needWarning = 1;
    shcmdo->actWin->addDefExeNode( shcmdo->aglPtr );
  }
  shcmdo->actWin->appCtx->proc->unlock();

}

static void pw_apply (
  Widget w,
  XtPointer client,
  XtPointer call ) {

}

static void pw_cancel (
  Widget w,
  XtPointer client,
  XtPointer call ) {

shellCmdClass *shcmdo = (shellCmdClass *) client;

  shcmdo->ef.popdown();

}

static void shcmdc_executeCmd (
  XtPointer client,
  XtIntervalId *id )
{

shellCmdClass *shcmdo = (shellCmdClass *) client;
threadParamBlockPtr threadParamBlock;
int stat, i;
char buffer[2550+1];

  if ( shcmdo->numCmds != 1 ) return;

  if ( !blank( shcmdo->requiredHostName ) && !blank( shcmdo->hostName ) ) {
    if ( strcmp( shcmdo->requiredHostName, shcmdo->hostName ) != 0 ) {
      sprintf( buffer, shellCmdClass_str31, shcmdo->requiredHostName );
      shcmdo->actWin->appCtx->postMessage( buffer );
      return;
    }
  }

  i = 0; // this is called from an X timer, always use command index 0

  if ( !shcmdo->timerActive ) {
    return;
  }

  if ( shcmdo->timerActive && !shcmdo->oneShot ) {
    shcmdo->timer = appAddTimeOut(
     shcmdo->actWin->appCtx->appContext(),
     shcmdo->timerValue, shcmdc_executeCmd, client );
  }

  shcmdo->actWin->substituteSpecial( 2550,
   shcmdo->shellCommand[i].getExpanded(),
   buffer );

  if ( shcmdo->multipleInstancesAllowed ) {
    threadParamBlock =
     (threadParamBlockPtr) calloc( 1, sizeof(threadParamBlockType) );
    threadParamBlock->cmd = (char *) calloc( strlen(buffer)+1, 1 );
    strcpy( threadParamBlock->cmd, buffer );
    threadParamBlock->multipleInstancesAllowed =
     shcmdo->multipleInstancesAllowed;
    threadParamBlock->secondsToDelay = (float) shcmdo->threadSecondsToDelay;
    stat = thread_create_handle( &shcmdo->thread, threadParamBlock );
    stat = thread_create_proc( shcmdo->thread, shellCmdThread );
    stat = thread_detach( shcmdo->thread );
  }
  else {
    if ( shcmdo->thread ) {
      stat = thread_wait_til_complete_no_block( shcmdo->thread );
      if ( stat & 1 ) {
        thread_request_free_handle( shcmdo->thread );
        threadParamBlock =
         (threadParamBlockPtr) calloc( 1, sizeof(threadParamBlockType) );
        threadParamBlock->cmd = (char *) calloc( strlen(buffer)+1, 1 );
        strcpy( threadParamBlock->cmd, buffer );
        threadParamBlock->multipleInstancesAllowed =
         shcmdo->multipleInstancesAllowed;
        threadParamBlock->secondsToDelay =
         (float) shcmdo->threadSecondsToDelay;
        stat = thread_create_handle( &shcmdo->thread, threadParamBlock );
        stat = thread_create_proc( shcmdo->thread, shellCmdThread );
      }
    }
    else {
      threadParamBlock =
       (threadParamBlockPtr) calloc( 1, sizeof(threadParamBlockType) );
      threadParamBlock->cmd = (char *) calloc( strlen(buffer)+1, 1 );
      strcpy( threadParamBlock->cmd, buffer );
      threadParamBlock->multipleInstancesAllowed =
       shcmdo->multipleInstancesAllowed;
      threadParamBlock->secondsToDelay = (float) shcmdo->threadSecondsToDelay;
      stat = thread_create_handle( &shcmdo->thread, threadParamBlock );
      stat = thread_create_proc( shcmdo->thread, shellCmdThread );
    }
  }

}

static void shcmdc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

shellCmdClass *shcmdo = (shellCmdClass *) client;
int i;

  shcmdo->actWin->setChanged();

  shcmdo->eraseSelectBoxCorners();
  shcmdo->erase();

  strncpy( shcmdo->fontTag, shcmdo->fm.currentFontTag(), 63 );
  shcmdo->actWin->fi->loadFontTag( shcmdo->fontTag );
  shcmdo->actWin->drawGc.setFontTag( shcmdo->fontTag, shcmdo->actWin->fi );
  shcmdo->actWin->fi->getTextFontList( shcmdo->fontTag, &shcmdo->fontList );
  shcmdo->fs = shcmdo->actWin->fi->getXFontStruct( shcmdo->fontTag );

  shcmdo->topShadowColor = shcmdo->buf->bufTopShadowColor;
  shcmdo->botShadowColor = shcmdo->buf->bufBotShadowColor;

  shcmdo->fgColor.setColorIndex( shcmdo->buf->bufFgColor, shcmdo->actWin->ci );

  shcmdo->bgColor.setColorIndex( shcmdo->buf->bufBgColor, shcmdo->actWin->ci );

  shcmdo->invisible = shcmdo->buf->bufInvisible;

  shcmdo->closeAction = shcmdo->buf->bufCloseAction;

  shcmdo->x = shcmdo->buf->bufX;
  shcmdo->sboxX = shcmdo->buf->bufX;

  shcmdo->y = shcmdo->buf->bufY;
  shcmdo->sboxY = shcmdo->buf->bufY;

  shcmdo->w = shcmdo->buf->bufW;
  shcmdo->sboxW = shcmdo->buf->bufW;

  shcmdo->h = shcmdo->buf->bufH;
  shcmdo->sboxH = shcmdo->buf->bufH;

  shcmdo->buttonLabel.setRaw( shcmdo->buf->bufButtonLabel );
  shcmdo->shellCommand[0].setRaw( shcmdo->buf->bufShellCommand[0] );
  shcmdo->label[0].setRaw( shcmdo->buf->bufLabel[0] );

  shcmdo->numCmds = 0;
  if ( !blank(shcmdo->buf->bufShellCommand[0]) ) {
    (shcmdo->numCmds)++;
  }

  if ( shcmdo->numCmds > 0 ) {
    for ( i=1; i<shcmdo->maxCmds; i++ ) {
      if ( ( !blank(shcmdo->buf->bufShellCommand[i]) ) &&
	   ( !blank(shcmdo->buf->bufLabel[i]) ) ) {
        shcmdo->shellCommand[i].setRaw( shcmdo->buf->bufShellCommand[i] );
        shcmdo->label[i].setRaw( shcmdo->buf->bufLabel[i] );
        (shcmdo->numCmds)++;
      }
    }
  }

  for ( i=shcmdo->numCmds; i<shcmdo->maxCmds; i++ ) {
    shcmdo->shellCommand[i].setRaw( "" );
    shcmdo->label[i].setRaw( "" );
  }

  shcmdo->autoExecInterval = shcmdo->buf->bufAutoExecInterval;

  shcmdo->multipleInstancesAllowed = shcmdo->buf->bufMultipleInstancesAllowed;

  shcmdo->threadSecondsToDelay = shcmdo->buf->bufThreadSecondsToDelay;

  if ( blank(shcmdo->bufPw1) || blank(shcmdo->bufPw2) ) {
    if ( blank(shcmdo->pw) ) {
      shcmdo->usePassword = 0;
    }
    else {
      shcmdo->usePassword = 1;
    }
  }
  else if ( strcmp( shcmdo->bufPw1, shcmdo->bufPw2 ) != 0 ) {
    shcmdo->actWin->appCtx->postMessage( shellCmdClass_str25 );
    if ( blank(shcmdo->pw) ) {
      shcmdo->usePassword = 0;
    }
    else if ( strcmp( shcmdo->pw, "*" ) == 0 ) {
      strcpy( shcmdo->pw, "" );
      shcmdo->usePassword = 0;
    }
    else {
      shcmdo->usePassword = 1;
    }
  }
  else {
    strcpy( shcmdo->pw, shcmdo->bufPw2 );
    if ( strcmp( shcmdo->pw, "*" ) == 0 ) {
      strcpy( shcmdo->pw, "" );
      shcmdo->usePassword = 0;
    }
    else {
      shcmdo->usePassword = 1;
    }
  }

  shcmdo->lock = shcmdo->buf->bufLock;

  strncpy( shcmdo->requiredHostName, shcmdo->buf->bufRequiredHostName, 15 );
  shcmdo->requiredHostName[15] = 0;

  shcmdo->oneShot = shcmdo->buf->bufOneShot;

  shcmdo->swapButtons = shcmdo->buf->bufSwapButtons;

  shcmdo->includeHelpIcon = shcmdo->buf->bufIncludeHelpIcon;

  shcmdo->execCursor = shcmdo->buf->bufExecCursor;

  shcmdo->updateDimensions();

}

static void shcmdc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

shellCmdClass *shcmdo = (shellCmdClass *) client;

  shcmdc_edit_update( w, client, call );
  shcmdo->refresh( shcmdo );

}

static void shcmdc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

shellCmdClass *shcmdo = (shellCmdClass *) client;

  shcmdc_edit_update( w, client, call );
  shcmdo->ef.popdown();
  shcmdo->operationComplete();

}

static void shcmdc_edit_ok1 (
  Widget w,
  XtPointer client,
  XtPointer call )
{

shellCmdClass *shcmdo = (shellCmdClass *) client;

  shcmdo->ef1->popdownNoDestroy();

}

static void shcmdc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

shellCmdClass *shcmdo = (shellCmdClass *) client;

  shcmdo->ef.popdown();
  shcmdo->operationCancel();

}

static void shcmdc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

shellCmdClass *shcmdo = (shellCmdClass *) client;

  shcmdo->ef.popdown();
  shcmdo->operationCancel();
  shcmdo->erase();
  shcmdo->deleteRequest = 1;
  shcmdo->drawAll();

}

shellCmdClass::shellCmdClass ( void ) {

  name = new char[strlen("shellCmdClass")+1];
  strcpy( name, "shellCmdClass" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  activeMode = 0;
  invisible = 0;
  closeAction = 0;
  multipleInstancesAllowed = 0;
  threadSecondsToDelay = 0;
  autoExecInterval = 0;
  timerActive = 0;
  fontList = NULL;
  strcpy( pw, "" );
  usePassword = 0;
  lock = 0;
  oneShot = 0;
  swapButtons = 0;
  includeHelpIcon = 0;
  execCursor = 0;
  numCmds = 0;
  cmdIndex = 0;
  buf = NULL;
  strcpy( requiredHostName, "" );

}

shellCmdClass::~shellCmdClass ( void ) {

  if ( name ) delete[] name;
  if ( fontList ) XmFontListFree( fontList );
  if ( buf ) {
    delete buf;
    buf = NULL;
  }

}

// copy constructor
shellCmdClass::shellCmdClass
 ( const shellCmdClass *source ) {

activeGraphicClass *shcmdo = (activeGraphicClass *) this;
int i;

  shcmdo->clone( (activeGraphicClass *) source );

  name = new char[strlen("shellCmdClass")+1];
  strcpy( name, "shellCmdClass" );

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

  invisible = source->invisible;

  closeAction = source->closeAction;

  buttonLabel.copy( source->buttonLabel );

  for ( i=0; i<maxCmds; i++ ) {
    shellCommand[i].copy( source->shellCommand[i] );
    label[i].copy( source->label[i] );
  }

  autoExecInterval = source->autoExecInterval;

  multipleInstancesAllowed = source->multipleInstancesAllowed;

  threadSecondsToDelay = source->threadSecondsToDelay;

  strcpy( pw, source->pw );
  usePassword = source->usePassword;
  lock = source->lock;

  oneShot = source->oneShot;

  swapButtons = source->swapButtons;

  includeHelpIcon = source->includeHelpIcon;

  execCursor = source->execCursor;

  numCmds = source->numCmds;
  cmdIndex = 0;

  timerActive = 0;

  activeMode = 0;

  strncpy( requiredHostName, source->requiredHostName, 15 );
  requiredHostName[15] = 0;

  buf = NULL;

  doAccSubs( buttonLabel );
  doAccSubs( requiredHostName, 15 );
  for ( i=0; i<maxCmds; i++ ) {
    doAccSubs( shellCommand[i] );
    doAccSubs( label[i] );
  }

}

int shellCmdClass::createInteractive (
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

  strcpy( fontTag, actWin->defaultBtnFontTag );
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int shellCmdClass::save (
  FILE *f )
{

int stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

  major = SHCMDC_MAJOR_VERSION;
  minor = SHCMDC_MINOR_VERSION;
  release = SHCMDC_RELEASE;

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
  tag.loadW( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadW( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadW( "font", fontTag );
  tag.loadBoolW( "invisible", &invisible, &zero );
  tag.loadBoolW( "closeDisplay", &closeAction, &zero );
  tag.loadW( "buttonLabel", &buttonLabel, emptyStr );
  tag.loadW( "autoExecPeriod", &autoExecInterval, &dzero );
  tag.loadW( "initialDelay", &threadSecondsToDelay, &dzero );
  tag.loadW( "password", pw, emptyStr );
  tag.loadBoolW( "lock", &lock, &zero );
  tag.loadBoolW( "oneShot", &oneShot, &zero );
  tag.loadBoolW( "swapButtons", &swapButtons, &zero );
  tag.loadBoolW( "multipleInstances", &multipleInstancesAllowed,
   &zero );
  tag.loadW( "requiredHostName", requiredHostName, emptyStr );
  tag.loadW( "numCmds", &numCmds );
  tag.loadW( "commandLabel", label, numCmds, emptyStr );
  tag.loadW( "command", shellCommand, numCmds, emptyStr );
  tag.loadBoolW( "includeHelpIcon", &includeHelpIcon, &zero );
  tag.loadBoolW( "execCursor", &execCursor, &zero );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int shellCmdClass::old_save (
  FILE *f )
{

int i, index;
float val;

  fprintf( f, "%-d %-d %-d\n", SHCMDC_MAJOR_VERSION, SHCMDC_MINOR_VERSION,
   SHCMDC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = topShadowColor;
  actWin->ci->writeColorIndex( f, index );

  index = botShadowColor;
  actWin->ci->writeColorIndex( f, index );

  if ( shellCommand[0].getRaw() )
    writeStringToFile( f, shellCommand[0].getRaw() );
  else
    writeStringToFile( f, "" );

  if ( buttonLabel.getRaw() )
    writeStringToFile( f, buttonLabel.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", invisible );

  fprintf( f, "%-d\n", closeAction );

  val = autoExecInterval;
  fprintf( f, "%g\n", val );

  fprintf( f, "%-d\n", multipleInstancesAllowed );

  // ver 2.1.0
  fprintf( f, "%g\n", threadSecondsToDelay );

  // ver 2.2.0
  writeStringToFile( f, pw );
  fprintf( f, "%-d\n", lock );

  // ver 2.4.0
  if ( label[0].getRaw() )
    writeStringToFile( f, label[0].getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", numCmds );

  for ( i=1; i<numCmds; i++ ) {

    if ( shellCommand[i].getRaw() )
      writeStringToFile( f, shellCommand[i].getRaw() );
    else
      writeStringToFile( f, "" );

    if ( label[i].getRaw() )
      writeStringToFile( f, label[i].getRaw() );
    else
      writeStringToFile( f, "" );

  }

  // ver 2.5.0
  writeStringToFile( f, requiredHostName );

  return 1;

}

int shellCmdClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, n, stat, major, minor, release;

tagClass tag;

int zero = 0;
double dzero = 0;
char *emptyStr = "";

  this->actWin = _actWin;

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
  tag.loadR( "topShadowColor", actWin->ci, &topShadowColor );
  tag.loadR( "botShadowColor", actWin->ci, &botShadowColor );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "invisible", &invisible, &zero );
  tag.loadR( "closeDisplay", &closeAction, &zero );
  tag.loadR( "buttonLabel", &buttonLabel, emptyStr );
  tag.loadR( "autoExecPeriod", &autoExecInterval, &dzero );
  tag.loadR( "initialDelay", &threadSecondsToDelay, &dzero );
  tag.loadR( "password", 31, pw, emptyStr );
  tag.loadR( "lock", &lock, &zero );
  tag.loadR( "oneShot", &oneShot, &zero );
  tag.loadR( "swapButtons", &swapButtons, &zero );
  tag.loadR( "multipleInstances", &multipleInstancesAllowed, &zero );
  tag.loadR( "requiredHostName", 15, requiredHostName, emptyStr );
  tag.loadR( "numCmds", &numCmds, &zero );
  tag.loadR( "commandLabel", maxCmds, label, &n, emptyStr );
  tag.loadR( "command", maxCmds, shellCommand, &n, emptyStr );
  tag.loadR( "includeHelpIcon", &includeHelpIcon, &zero );
  tag.loadR( "execCursor", &execCursor, &zero );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > SHCMDC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( blank(pw) ) {
    usePassword = 0;
  }
  else {
    usePassword = 1;
  }

  for ( i=numCmds; i<maxCmds; i++ ) {
    shellCommand[i].setRaw( "" );
    label[i].setRaw( "" );
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return stat;

}

int shellCmdClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int i, r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[2550+1];
float val;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > SHCMDC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  swapButtons = 0;
  includeHelpIcon = 0;
  execCursor = 0;

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 2 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    topShadowColor = index;

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    botShadowColor = index;

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    topShadowColor = index;

    fscanf( f, "%d\n", &index ); actWin->incLine();
    botShadowColor = index;

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    bgColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    topShadowColor = actWin->ci->pixIndex( pixel );

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    botShadowColor = actWin->ci->pixIndex( pixel );

  }

  readStringFromFile( oneName, 2550+1, f ); actWin->incLine();
  shellCommand[0].setRaw( oneName );

  readStringFromFile( oneName, 127+1, f ); actWin->incLine();
  buttonLabel.setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  fscanf( f, "%d\n", &invisible ); actWin->incLine();
  fscanf( f, "%d\n", &closeAction ); actWin->incLine();

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 0 ) ) ) {
    fscanf( f, "%g\n", &val ); actWin->incLine();
    autoExecInterval = (double) val;
  }
  else {
    autoExecInterval = 0.0;
  }

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 1 ) ) ) {
    fscanf( f, "%d\n", &multipleInstancesAllowed ); actWin->incLine();
  }
  else {
    multipleInstancesAllowed = 1;
  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {
    fscanf( f, "%g\n", &val ); actWin->incLine();
    threadSecondsToDelay = (double) val;
  }
  else {
    threadSecondsToDelay = 0;
  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 1 ) ) ) {
    readStringFromFile( pw, 31+1, f ); actWin->incLine();
    if ( blank(pw) ) {
      usePassword = 0;
    }
    else {
      usePassword = 1;
    }
    fscanf( f, "%d\n", &lock );
  }
  else {
    strcpy( pw, "" );
    usePassword = 0;
    lock = 0;
  }

  // after v 2.3 menu label 0, numCmds, and then the array data
  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 3 ) ) ) {

    readStringFromFile( oneName, 127+1, f ); actWin->incLine();
    label[0].setRaw( oneName );

    fscanf( f, "%d\n", &numCmds ); actWin->incLine();

    for ( i=1; i<numCmds; i++ ) {

      readStringFromFile( oneName, 2550+1, f ); actWin->incLine();
      shellCommand[i].setRaw( oneName );

      readStringFromFile( oneName, 127+1, f ); actWin->incLine();
      label[i].setRaw( oneName );

    }

  }
  else {

    numCmds = 1;
    if ( blank( shellCommand[0].getRaw() ) ) numCmds = 0;

  }

  for ( i=numCmds; i<maxCmds; i++ ) {
    shellCommand[i].setRaw( "" );
    label[i].setRaw( "" );
  }

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 4 ) ) ) {
    readStringFromFile( requiredHostName, 15+1, f );
  }
  else {
    strcpy( requiredHostName, "" );
  }

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  fs = actWin->fi->getXFontStruct( fontTag );
  actWin->fi->getTextFontList( fontTag, &fontList );

  updateDimensions();

  return 1;

}

int shellCmdClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int fgR, fgG, fgB, bgR, bgG, bgB, more, index;
unsigned int pixel;
char *tk, *gotData, *context, buffer[2550+1];

  fgR = 0xffff;
  fgG = 0xffff;
  fgB = 0xffff;

  bgR = 0xffff;
  bgG = 0xffff;
  bgB = 0xffff;

  this->actWin = _actWin;

  strcpy( fontTag, actWin->defaultBtnFontTag );

  topShadowColor = actWin->defaultTopShadowColor;
  botShadowColor = actWin->defaultBotShadowColor;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  swapButtons = 0;
  includeHelpIcon = 0;
  execCursor = 0;

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buffer, 2550, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( shellCmdClass_str1 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buffer, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( shellCmdClass_str1 );
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
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "fgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        fgR = atol( tk );

      }
            
      else if ( strcmp( tk, "fggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        fgG = atol( tk );

      }
            
      else if ( strcmp( tk, "fgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        fgB = atol( tk );

      }
            
      else if ( strcmp( tk, "bgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        bgR = atol( tk );

      }
            
      else if ( strcmp( tk, "bggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        bgG = atol( tk );

      }
            
      else if ( strcmp( tk, "bgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        bgB = atol( tk );

      }
            
      else if ( strcmp( tk, "closecurrent" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        closeAction = atol( tk );

      }
            
      else if ( strcmp( tk, "invisible" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        invisible = atol( tk );

      }
            
      else if ( strcmp( tk, "font" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        strncpy( fontTag, tk, 63 );

      }

      else if ( strcmp( tk, "command" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        shellCommand[0].setRaw( tk );

      }

      else if ( strcmp( tk, "label" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        buttonLabel.setRaw( tk );

      }

    }

  } while ( more );

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->ci->setRGB( fgR, fgG, fgB, &pixel );
  index = actWin->ci->pixIndex( pixel );
  fgColor.setColorIndex( index, actWin->ci );

  actWin->ci->setRGB( bgR, bgG, bgB, &pixel );
  index = actWin->ci->pixIndex( pixel );
  bgColor.setColorIndex( index, actWin->ci );

  actWin->fi->loadFontTag( fontTag );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );
  fs = actWin->fi->getXFontStruct( fontTag );

  updateDimensions();

  return 1;

}

int shellCmdClass::genericEdit ( void ) {

int i;
char title[32], *ptr, *envPtr, saveLock = 0;

  buf = new bufType;

  envPtr = getenv( "EDMSUPERVISORMODE" );
  if ( envPtr ) {
    if ( strcmp( envPtr, "TRUE" ) == 0 ) {
      if ( lock ) {
        actWin->appCtx->postMessage( shellCmdClass_str26 );
      }
      saveLock = lock;
      lock = 0;
    }
  }

  ptr = actWin->obj.getNameFromClass( "shellCmdClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, shellCmdClass_str2, 31 );

  Strncat( title, shellCmdClass_str3, 31 );

  buf->bufX = x;
  buf->bufY = y;
  buf->bufW = w;
  buf->bufH = h;

  strncpy( buf->bufFontTag, fontTag, 63 );

  buf->bufTopShadowColor = topShadowColor;
  buf->bufBotShadowColor = botShadowColor;

  buf->bufFgColor = fgColor.pixelIndex();

  buf->bufBgColor = bgColor.pixelIndex();

  for ( i=0; i<maxCmds; i++ ) {
    if ( shellCommand[i].getRaw() )
      strncpy( buf->bufShellCommand[i], shellCommand[i].getRaw(), 2550 );
    else
      strncpy( buf->bufShellCommand[i], "", 2550 );
    if ( label[i].getRaw() )
      strncpy( buf->bufLabel[i], label[i].getRaw(), 127 );
    else
      strncpy( buf->bufLabel[i], "", 127 );
  }
  for ( i=numCmds; i<maxCmds; i++ ) {
    strncpy( buf->bufShellCommand[i], "", 2550 );
    strncpy( buf->bufLabel[i], "", 127 );
  }

  if ( buttonLabel.getRaw() )
    strncpy( buf->bufButtonLabel, buttonLabel.getRaw(), 127 );
  else
    strncpy( buf->bufButtonLabel, "", 127 );

  buf->bufInvisible = invisible;

  buf->bufCloseAction = closeAction;

  buf->bufAutoExecInterval = autoExecInterval;

  buf->bufMultipleInstancesAllowed = multipleInstancesAllowed;

  buf->bufThreadSecondsToDelay = threadSecondsToDelay;

  strcpy( bufPw1, "" );
  strcpy( bufPw2, "" );

  if ( envPtr ) {
    if ( strcmp( envPtr, "TRUE" ) == 0 ) {
      buf->bufLock = saveLock;
    }
  }
  else {
    buf->bufLock = lock;
  }

  strncpy( buf->bufRequiredHostName, requiredHostName, 15 );
  buf->bufRequiredHostName[15] = 0;

  buf->bufOneShot = oneShot;

  buf->bufSwapButtons = swapButtons;

  buf->bufIncludeHelpIcon = includeHelpIcon;

  buf->bufExecCursor = execCursor;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( shellCmdClass_str4, 35, &buf->bufX );
  ef.addTextField( shellCmdClass_str5, 35, &buf->bufY );
  ef.addTextField( shellCmdClass_str6, 35, &buf->bufW );
  ef.addTextField( shellCmdClass_str7, 35, &buf->bufH );

  if ( !lock ) {
    ef.addTextField( shellCmdClass_str14, 35, buf->bufShellCommand[0], 2550 );
  }
  else {
    ef.addLockedField( shellCmdClass_str14, 35, buf->bufShellCommand[0], 2550 );
  }

  ef.addTextField( shellCmdClass_str13, 35, buf->bufLabel[0], 127 );

  ef.addEmbeddedEf( shellCmdClass_str22, "...", &ef1 );

  ef1->create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  for ( i=1; i<maxCmds; i++ ) {
    ef1->beginSubForm();
    ef1->addTextField( shellCmdClass_str13, 35, buf->bufLabel[i], 127 );
    ef1->addLabel( shellCmdClass_str23 );
    if ( !lock ) {
      ef1->addTextField( shellCmdClass_str14, 35, buf->bufShellCommand[i],
       2550 );
    }
    else {
      ef1->addLockedField( shellCmdClass_str14, 35, buf->bufShellCommand[i],
       2550 );
    }
    ef1->endSubForm();
  }

  ef1->finished( shcmdc_edit_ok1, this );

  ef.addTextField( shellCmdClass_str21, 35, buf->bufButtonLabel, 127 );

  ef.addTextField( shellCmdClass_str30, 35, buf->bufRequiredHostName, 15 );

  if ( !lock ) {
    ef.addPasswordField( shellCmdClass_str24, 35, bufPw1, 31 );
    ef.addPasswordField( shellCmdClass_str27, 35, bufPw2, 31 );
    ef.addToggle( shellCmdClass_str28, &buf->bufLock );
  }
  else {
    ef.addLockedField( shellCmdClass_str24, 35, bufPw1, 31 );
    ef.addLockedField( shellCmdClass_str27, 35, bufPw2, 31 );
  }

  ef.addToggle( shellCmdClass_str15, &buf->bufInvisible );
  ef.addToggle( shellCmdClass_str16, &buf->bufCloseAction );
  ef.addToggle( shellCmdClass_str17, &buf->bufMultipleInstancesAllowed );
  ef.addTextField( shellCmdClass_str20, 35, &buf->bufThreadSecondsToDelay );
  ef.addTextField( shellCmdClass_str18, 35, &buf->bufAutoExecInterval );
  ef.addToggle( shellCmdClass_str33, &buf->bufOneShot );
  ef.addToggle( shellCmdClass_str34, &buf->bufSwapButtons );
  ef.addToggle( shellCmdClass_str35, &buf->bufIncludeHelpIcon );
  ef.addToggle( shellCmdClass_str36, &buf->bufExecCursor );

  ef.addColorButton( shellCmdClass_str8, actWin->ci, &fgCb, &buf->bufFgColor );
  ef.addColorButton( shellCmdClass_str9, actWin->ci, &bgCb, &buf->bufBgColor );
  ef.addColorButton( shellCmdClass_str10, actWin->ci, &topShadowCb,
   &buf->bufTopShadowColor );
  ef.addColorButton( shellCmdClass_str11, actWin->ci, &botShadowCb,
   &buf->bufBotShadowColor );

  ef.addFontMenu( shellCmdClass_str12, actWin->fi, &fm, fontTag );
  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  if ( envPtr ) {
    if ( strcmp( envPtr, "TRUE" ) == 0 ) {
      lock = saveLock;
    }
  }

  return 1;

}

int shellCmdClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( shcmdc_edit_ok, shcmdc_edit_apply, shcmdc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int shellCmdClass::edit ( void ) {

int stat;

  stat = this->genericEdit();
  ef.finished( shcmdc_edit_ok, shcmdc_edit_apply, shcmdc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int shellCmdClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int shellCmdClass::eraseActive ( void ) {

  if ( !enabled || !activeMode || invisible ) return 1;

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  return 1;

}

int shellCmdClass::draw ( void ) {

int tX, tY;
XRectangle xR = { x, y, w, h };

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( bgColor.pixelColor() );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, x, y+h );

   actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x, y+h, x+w, y+h );

   XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
    actWin->drawGc.normGC(), x+w, y, x+w, y+h );

  actWin->drawGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+w-2, y+2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+2, x+2, y+h-2 );

  actWin->drawGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  if ( fs ) {

    actWin->drawGc.addNormXClipRectangle( xR );

    actWin->drawGc.setFG( fgColor.pixelColor() );
    actWin->drawGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    if ( buttonLabel.getRaw() )
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, buttonLabel.getRaw() );
    else
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, "" );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int shellCmdClass::drawActive ( void ) {

int tX, tY;
char string[127+1];
XRectangle xR = { x, y, w, h };

  if ( !enabled || !activeMode || invisible ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( bgColor.getColor() );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( buttonLabel.getExpanded() )
    strncpy( string, buttonLabel.getExpanded(), 127 );
  else
    strncpy( string, "", 127 );

  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x, y+h );

  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y+h, x+w, y+h );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w, y, x+w, y+h );

  // top
  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+w-2, y+2 );

  // left
  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+2, y+h-2 );

  // bottom
  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  // right
  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  if ( fs ) {

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.setFG( fgColor.getColor() );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->executeWidget, drawable(actWin->executeWidget),
     &actWin->executeGc, fs, tX, tY, XmALIGNMENT_CENTER, string );

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int shellCmdClass::activate (
  int pass,
  void *ptr )
{

int i, n;
Arg args[5];
XmString str;

  switch ( pass ) {

  case 1:

    thread = NULL;
    activeMode = 1;
    aglPtr = ptr;
    needExecute = needWarning = opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      opComplete = 1;

      hostName = getenv( "HOSTNAME" );
      if ( !hostName ) hostName = g_nullHost;

      initEnable();

      if ( numCmds == 1 ) {
        cmdIndex = 0;
        if ( oneShot || ( autoExecInterval > 0.5 ) ) {
          timerValue = (int) ( autoExecInterval * 1000.0 );
          timer = appAddTimeOut( actWin->appCtx->appContext(),
           0, shcmdc_executeCmd, this );
          timerActive = 1;
        }
      }

      n = 0;
      XtSetArg( args[n], XmNpopupEnabled, (XtArgVal) False ); n++;
      popUpMenu = XmCreatePopupMenu( actWin->topWidgetId(), "", args, n );

      pullDownMenu = XmCreatePulldownMenu( popUpMenu, "", NULL, 0 );

      for ( i=0; i<numCmds; i++ ) {

        if ( label[i].getExpanded() ) {
           str = XmStringCreateLocalized( label[i].getExpanded() );
        }
        else {
          str = XmStringCreateLocalized( " " );
        }
        pb[i] = XtVaCreateManagedWidget( "", xmPushButtonWidgetClass,
         popUpMenu,
         XmNlabelString, str,
         NULL );
        XmStringFree( str );

        XtAddCallback( pb[i], XmNactivateCallback, menu_cb,
         (XtPointer) this );

      }

    }

    break;

  case 3:
  case 4:
  case 5:
  case 6:

    break;

  }

  return 1;

}

int shellCmdClass::deactivate (
  int pass )
{

int stat;

  if ( pass == 1 ) {

    activeMode = 0;

    if ( ef.formIsPoppedUp() ) ef.popdown();

    XtDestroyWidget( popUpMenu );

    if ( timerActive ) {
      XtRemoveTimeOut( timer );
      timerActive = 0;
    }
    if ( thread ) {
      if ( !multipleInstancesAllowed ) {
        stat = thread_detach( thread );
        thread = NULL;
      }
    }

  }

  return 1;

}

void shellCmdClass::updateDimensions ( void )
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

int shellCmdClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i;
expStringClass tmpStr;

  for ( i=0; i<numCmds; i++ ) {

    tmpStr.setRaw( shellCommand[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    shellCommand[i].setRaw( tmpStr.getExpanded() );

    tmpStr.setRaw( label[i].getRaw() );
    tmpStr.expand1st( numMacros, macros, expansions );
    label[i].setRaw( tmpStr.getExpanded() );

  }

  tmpStr.setRaw( buttonLabel.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  buttonLabel.setRaw( tmpStr.getExpanded() );

  return 1;

}

int shellCmdClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i, stat;

  for ( i=0; i<numCmds; i++ ) {
    stat = shellCommand[i].expand1st( numMacros, macros, expansions );
    stat = label[i].expand1st( numMacros, macros, expansions );
  }
  stat = buttonLabel.expand1st( numMacros, macros, expansions );

  return stat;

}

int shellCmdClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int i, stat;

  for ( i=0; i<numCmds; i++ ) {
    stat = shellCommand[i].expand2nd( numMacros, macros, expansions );
    stat = label[i].expand2nd( numMacros, macros, expansions );
  }
  stat = buttonLabel.expand2nd( numMacros, macros, expansions );

  return stat;

}

int shellCmdClass::containsMacros ( void ) {

int i;

  for ( i=0; i<numCmds; i++ ) {
    if ( shellCommand[i].containsPrimaryMacros() ) return 1;
    if ( label[i].containsPrimaryMacros() ) return 1;
  }
  if ( buttonLabel.containsPrimaryMacros() ) return 1;

  return 0;

}

void shellCmdClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( swapButtons ) {
    if ( buttonNumber == 1 ) {
      buttonNumber = 3;
    }
    else if ( buttonNumber == 3 ) {
      buttonNumber = 1;
    }
  }

  if ( buttonNumber != 1 ) return;

  if ( numCmds < 2 ) return;

  XmMenuPosition( popUpMenu, be );
  XtManageChild( popUpMenu );

}

void shellCmdClass::executeCmd ( void ) {

int stat;
threadParamBlockPtr threadParamBlock;
char buffer[2550+1];

  if ( !blank( requiredHostName ) && !blank( hostName ) ) {
    if ( strcmp( requiredHostName, hostName ) != 0 ) {
      sprintf( buffer, shellCmdClass_str32, requiredHostName, hostName );
      actWin->appCtx->postMessage( buffer );
      return;
    }
  }

  actWin->substituteSpecial( 2550, shellCommand[cmdIndex].getExpanded(),
   buffer );

  if ( multipleInstancesAllowed ) {
    threadParamBlock =
     (threadParamBlockPtr) calloc( 1, sizeof(threadParamBlockType) );
    threadParamBlock->cmd = (char *) calloc( strlen(buffer)+1, 1 );
    strcpy( threadParamBlock->cmd, buffer );
    threadParamBlock->multipleInstancesAllowed =
     multipleInstancesAllowed;
    threadParamBlock->secondsToDelay = (float) threadSecondsToDelay;
    stat = thread_create_handle( &thread, threadParamBlock );
    stat = thread_create_proc( thread, shellCmdThread );
    stat = thread_detach( thread );

  }
  else {

    if ( thread ) {

      stat = thread_wait_til_complete_no_block( thread );
      if ( !( stat & 1 ) ) {
        actWin->appCtx->postMessage( shellCmdClass_str19 );
      }
      else {
        thread_request_free_handle( thread );
        threadParamBlock =
         (threadParamBlockPtr) calloc( 1, sizeof(threadParamBlockType) );
        threadParamBlock->cmd = (char *) calloc( strlen(buffer)+1, 1 );
        strcpy( threadParamBlock->cmd, buffer );
        threadParamBlock->multipleInstancesAllowed =
         multipleInstancesAllowed;
        threadParamBlock->secondsToDelay = (float) threadSecondsToDelay;
        stat = thread_create_handle( &thread, threadParamBlock );
        stat = thread_create_proc( thread, shellCmdThread );
      }

    }
    else {

      threadParamBlock =
       (threadParamBlockPtr) calloc( 1, sizeof(threadParamBlockType) );
      threadParamBlock->cmd = (char *) calloc( strlen(buffer)+1, 1 );
      strcpy( threadParamBlock->cmd, buffer );
      threadParamBlock->multipleInstancesAllowed =
       multipleInstancesAllowed;
      threadParamBlock->secondsToDelay = (float) threadSecondsToDelay;
      stat = thread_create_handle( &thread, threadParamBlock );
      stat = thread_create_proc( thread, shellCmdThread );

    }

  }

}

void shellCmdClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  if ( !enabled ) {
    *action = 0;
    return;
  }

  if ( swapButtons ) {
    if ( buttonNumber == 1 ) {
      buttonNumber = 3;
    }
    else if ( buttonNumber == 3 ) {
      buttonNumber = 1;
    }
  }

  if ( buttonNumber != 1 ) return;

  if ( numCmds < 1 ) return;

  pwFormX = be->x_root; // we may use these later
  pwFormY = be->y_root;

  if ( numCmds == 1 ) {

    cmdIndex = 0;

    if ( usePassword ) {

      if ( !ef.formIsPoppedUp() ) {

        pwFormW = 0;
        pwFormH = 0;
        pwFormMaxH = 600;

        ef.create( actWin->top,
         actWin->appCtx->ci.getColorMap(),
         &pwFormX, &pwFormY,
         &pwFormW, &pwFormH, &pwFormMaxH,
         "", NULL, NULL, NULL );

        strcpy( bufPw1, "" );

        ef.addPasswordField( shellCmdClass_str24, 35, bufPw1, 31 );

        ef.finished( pw_ok, pw_apply, pw_cancel, this );

        ef.popup();

        *action = 0; // close screen via actWin->closeDeferred
        return;

      }
      else {

        *action = 0;
        return;

      }

    }

    executeCmd();

    if ( actWin->isEmbedded ) {
      *action = 0;
    }
    else {
      *action = closeAction;
    }

  }
  else {

    *action = 0;

  }

}

void shellCmdClass::pointerIn (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState )
{

  if ( !enabled ) return;

  activeGraphicClass::pointerIn( me, me->x, me->y, buttonState );

  if ( execCursor ) {

    if ( includeHelpIcon ) {
      actWin->cursor.set( XtWindow(actWin->executeWidget),
       CURSOR_K_RUN_WITH_HELP );
    }
    else {
      actWin->cursor.set( XtWindow(actWin->executeWidget),
       CURSOR_K_RUN );
    }

  }
  else {

    if ( includeHelpIcon ) {
      actWin->cursor.set( XtWindow(actWin->executeWidget),
       CURSOR_K_PNTR_WITH_HELP );
    }
    else {
      actWin->cursor.set( XtWindow(actWin->executeWidget),
       CURSOR_K_DEFAULT );
    }

  }

}

int shellCmdClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;
  *down = 1;
  *up = 1;

  if ( numCmds > 0 )
    *focus = 1;
  else
    *focus = 0;

  return 1;

}

void shellCmdClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topShadowColor = _topShadowColor;

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botShadowColor = _botShadowColor;

  if ( _flag & ACTGRF_BTNFONTTAG_MASK ) {
    strncpy( fontTag, _btnFontTag, 63 );
    fontTag[63] = 0;
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    updateDimensions();
  }

}

void shellCmdClass::executeDeferred ( void ) {

  int ne, nw;

  actWin->appCtx->proc->lock();
  ne = needExecute; needExecute = 0;
  nw = needWarning; needWarning = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

  if ( ne ) {

    executeCmd();
    if ( !actWin->isEmbedded ) {
      if ( closeAction ) actWin->closeDeferred( 2 );
    }

  }

  if ( nw ) {

    actWin->appCtx->postMessage( shellCmdClass_str29 );

  }

}

char *shellCmdClass::getSearchString (
  int i
) {

int num = 1 + 1 + maxCmds + maxCmds;
int ii, selector, index;

  if ( i == 0 ) {
    return buttonLabel.getRaw();
  }
  else if ( i == 1 ) {
    return requiredHostName;
  }
  else if ( ( i > 1 ) && ( i < num ) ) {
    ii = i - 2;
    selector = ii % 2;
    index = ii / 2;
    if ( selector == 0 ) {
      return shellCommand[index].getRaw();
    }
    else if ( selector == 1 ) {
      return label[index].getRaw();
    }
  }

  return NULL;

}

void shellCmdClass::replaceString (
  int i,
  int max,
  char *string
) {

int num = 1 + 1 + maxCmds + maxCmds;
int ii, selector, index;

  if ( i == 0 ) {
    buttonLabel.setRaw( string );
  }
  else if ( i == 1 ) {
    int l = max;
    if ( 15 < max ) l = 15;
    strncpy( requiredHostName, string, l );
    requiredHostName[l] = 0;
  }
  else if ( ( i > 1 ) && ( i < num ) ) {
    ii = i - 2;
    selector = ii % 2;
    index = ii / 2;
    if ( selector == 0 ) {
      shellCommand[index].setRaw( string );
    }
    else if ( selector == 1 ) {
      label[index].setRaw( string );
    }
  }

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_shellCmdClassPtr ( void ) {

shellCmdClass *ptr;

  ptr = new shellCmdClass;
  return (void *) ptr;

}

void *clone_shellCmdClassPtr (
  void *_srcPtr )
{

shellCmdClass *ptr, *srcPtr;

  srcPtr = (shellCmdClass *) _srcPtr;

  ptr = new shellCmdClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
