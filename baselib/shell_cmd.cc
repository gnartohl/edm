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

#ifdef __linux__
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

int stat;
threadParamBlockPtr threadParamBlock =
 (threadParamBlockPtr) thread_get_app_data( h );

  if ( threadParamBlock->secondsToDelay > 0.0 ) {
    thread_delay( h, threadParamBlock->secondsToDelay );
  }

  stat = system( threadParamBlock->cmd );

  if ( threadParamBlock->multipleInstancesAllowed ) {
    delete threadParamBlock->cmd;
    delete threadParamBlock;
    stat = thread_detached_exit( h, NULL ); // this call deallocates h
  }
  else {
    delete threadParamBlock->cmd;
    delete threadParamBlock;
    stat = thread_exit( h, NULL ); // this requires a join
  }

#ifdef __linux__
  return (void *) NULL;
#endif

#ifdef __solaris__
  return (void *) NULL;
#endif

}

static void shcmdc_executeCmd (
  XtPointer client,
  XtIntervalId *id )
{

shellCmdClass *shcmdo = (shellCmdClass *) client;
threadParamBlockPtr threadParamBlock;
int stat;

  if ( shcmdo->timerActive ) {
    shcmdo->timer = XtAppAddTimeOut(
     shcmdo->actWin->appCtx->appContext(),
     shcmdo->timerValue, shcmdc_executeCmd, client );
  }
  else {
    return;
  }

  shcmdo->actWin->substituteSpecial( 127, shcmdo->shellCommand.getExpanded(),
   shcmdo->bufShellCommand );

  if ( shcmdo->multipleInstancesAllowed ) {
    threadParamBlock = new threadParamBlockType;
    threadParamBlock->cmd = new (char)[strlen(shcmdo->bufShellCommand)+1];
    strcpy( threadParamBlock->cmd, shcmdo->bufShellCommand );
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
        stat = thread_destroy_handle( shcmdo->thread );
        threadParamBlock = new threadParamBlockType;
        threadParamBlock->cmd = new (char)[strlen(shcmdo->bufShellCommand)+1];
        strcpy( threadParamBlock->cmd, shcmdo->bufShellCommand );
        threadParamBlock->multipleInstancesAllowed =
         shcmdo->multipleInstancesAllowed;
        threadParamBlock->secondsToDelay = (float) shcmdo->threadSecondsToDelay;
        stat = thread_create_handle( &shcmdo->thread, threadParamBlock );
        stat = thread_create_proc( shcmdo->thread, shellCmdThread );
      }
    }
    else {
      threadParamBlock = new threadParamBlockType;
      threadParamBlock->cmd = new (char)[strlen(shcmdo->bufShellCommand)+1];
      strcpy( threadParamBlock->cmd, shcmdo->bufShellCommand );
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

  shcmdo->actWin->setChanged();

  shcmdo->eraseSelectBoxCorners();
  shcmdo->erase();

  strncpy( shcmdo->fontTag, shcmdo->fm.currentFontTag(), 63 );
  shcmdo->actWin->fi->loadFontTag( shcmdo->fontTag );
  shcmdo->actWin->drawGc.setFontTag( shcmdo->fontTag, shcmdo->actWin->fi );
  shcmdo->actWin->fi->getTextFontList( shcmdo->fontTag, &shcmdo->fontList );
  shcmdo->fs = shcmdo->actWin->fi->getXFontStruct( shcmdo->fontTag );

  shcmdo->topShadowColor = shcmdo->bufTopShadowColor;
  shcmdo->botShadowColor = shcmdo->bufBotShadowColor;

  shcmdo->fgColor.setColorIndex( shcmdo->bufFgColor, shcmdo->actWin->ci );

  shcmdo->bgColor.setColorIndex( shcmdo->bufBgColor, shcmdo->actWin->ci );

  shcmdo->invisible = shcmdo->bufInvisible;

  shcmdo->closeAction = shcmdo->bufCloseAction;

  shcmdo->x = shcmdo->bufX;
  shcmdo->sboxX = shcmdo->bufX;

  shcmdo->y = shcmdo->bufY;
  shcmdo->sboxY = shcmdo->bufY;

  shcmdo->w = shcmdo->bufW;
  shcmdo->sboxW = shcmdo->bufW;

  shcmdo->h = shcmdo->bufH;
  shcmdo->sboxH = shcmdo->bufH;

  shcmdo->shellCommand.setRaw( shcmdo->bufShellCommand );

  shcmdo->label.setRaw( shcmdo->bufLabel );
  // strncpy( shcmdo->label, shcmdo->bufLabel, 127 );

  shcmdo->autoExecInterval = shcmdo->bufAutoExecInterval;

  shcmdo->multipleInstancesAllowed = shcmdo->bufMultipleInstancesAllowed;

  shcmdo->threadSecondsToDelay = shcmdo->bufThreadSecondsToDelay;

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

  activeMode = 0;
  // strcpy( label, "" );
  invisible = 0;
  closeAction = 0;
  multipleInstancesAllowed = 0;
  threadSecondsToDelay = 0;
  autoExecInterval = 0;
  timerActive = 0;
  fontList = NULL;

}

shellCmdClass::~shellCmdClass ( void ) {

  if ( name ) delete name;
  if ( fontList ) XmFontListFree( fontList );

}

// copy constructor
shellCmdClass::shellCmdClass
 ( const shellCmdClass *source ) {

activeGraphicClass *shcmdo = (activeGraphicClass *) this;

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

  shellCommand.copy( source->shellCommand );

  label.copy( source->label );
  // strncpy( label, source->label, 127 );

  autoExecInterval = source->autoExecInterval;

  multipleInstancesAllowed = source->multipleInstancesAllowed;

  threadSecondsToDelay = source->threadSecondsToDelay;

  timerActive = 0;

  activeMode = 0;

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

int index;
float val;

  fprintf( f, "%-d %-d %-d\n", SHCMDC_MAJOR_VERSION, SHCMDC_MINOR_VERSION,
   SHCMDC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  fprintf( f, "%-d\n", index );

  index = bgColor.pixelIndex();
  fprintf( f, "%-d\n", index );

  index = topShadowColor;
  fprintf( f, "%-d\n", index );

  index = botShadowColor;
  fprintf( f, "%-d\n", index );

  if ( shellCommand.getRaw() )
    writeStringToFile( f, shellCommand.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( label.getRaw() )
    writeStringToFile( f, label.getRaw() );
  else
    writeStringToFile( f, "" );

  //if ( label )
  //  writeStringToFile( f, label );
  //else
  //  writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", invisible );

  fprintf( f, "%-d\n", closeAction );

  val = autoExecInterval;
  fprintf( f, "%g\n", val );

  fprintf( f, "%-d\n", multipleInstancesAllowed );

  // ver 2.1.0
  fprintf( f, "%g\n", threadSecondsToDelay );

  return 1;

}

int shellCmdClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[127+1];
float val;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( major > 1 ) {

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

  readStringFromFile( oneName, 127, f ); actWin->incLine();
  shellCommand.setRaw( oneName );

  readStringFromFile( oneName, 127, f ); actWin->incLine();
  label.setRaw( oneName );
  // strncpy( label, oneName, 127 );

  readStringFromFile( fontTag, 63, f ); actWin->incLine();

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
char *tk, *gotData, *context, buf[255+1];

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

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( shellCmdClass_str1 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
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

        shellCommand.setRaw( tk );

      }

      else if ( strcmp( tk, "label" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( shellCmdClass_str1 );
          return 0;
        }

        label.setRaw( tk );
        // strncpy( label, tk, 127 );

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

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "shellCmdClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, shellCmdClass_str2, 31 );

  strncat( title, shellCmdClass_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  strncpy( bufFontTag, fontTag, 63 );

  bufTopShadowColor = topShadowColor;
  bufBotShadowColor = botShadowColor;

  bufFgColor = fgColor.pixelIndex();

  bufBgColor = bgColor.pixelIndex();

  if ( shellCommand.getRaw() )
    strncpy( bufShellCommand, shellCommand.getRaw(), 127 );
  else
    strncpy( bufShellCommand, "", 127 );

  if ( label.getRaw() )
    strncpy( bufLabel, label.getRaw(), 127 );
  else
    strncpy( bufLabel, "", 127 );

  //if ( label )
  //  strncpy( bufLabel, label, 127 );
  //else
  //  strncpy( bufLabel, "", 127 );

  bufInvisible = invisible;

  bufCloseAction = closeAction;

  bufAutoExecInterval = autoExecInterval;

  bufMultipleInstancesAllowed = multipleInstancesAllowed;

  bufThreadSecondsToDelay = threadSecondsToDelay;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( shellCmdClass_str4, 30, &bufX );
  ef.addTextField( shellCmdClass_str5, 30, &bufY );
  ef.addTextField( shellCmdClass_str6, 30, &bufW );
  ef.addTextField( shellCmdClass_str7, 30, &bufH );

  ef.addTextField( shellCmdClass_str14, 30, bufShellCommand, 127 );
  ef.addTextField( shellCmdClass_str13, 30, bufLabel, 127 );
  ef.addToggle( shellCmdClass_str15, &bufInvisible );
  ef.addToggle( shellCmdClass_str16, &bufCloseAction );
  ef.addToggle( shellCmdClass_str17, &bufMultipleInstancesAllowed );
  ef.addTextField( shellCmdClass_str20, 30, &bufThreadSecondsToDelay );
  ef.addTextField( shellCmdClass_str18, 30, &bufAutoExecInterval );

  ef.addColorButton( shellCmdClass_str8, actWin->ci, &fgCb, &bufFgColor );
  ef.addColorButton( shellCmdClass_str9, actWin->ci, &bgCb, &bufBgColor );
  ef.addColorButton( shellCmdClass_str10, actWin->ci, &topShadowCb,
   &bufTopShadowColor );
  ef.addColorButton( shellCmdClass_str11, actWin->ci, &botShadowCb,
   &bufBotShadowColor );

  ef.addFontMenu( shellCmdClass_str12, actWin->fi, &fm, fontTag );
  XtUnmanageChild( fm.alignWidget() ); // no alignment info

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

  this->genericEdit();
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

  if ( !activeMode || invisible ) return 1;

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
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

    if ( label.getRaw() )
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, label.getRaw() );
    else
      drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
       XmALIGNMENT_CENTER, "" );

    // drawText( actWin->drawWidget, &actWin->drawGc, fs, tX, tY,
    //  XmALIGNMENT_CENTER, label );

    actWin->drawGc.removeNormXClipRectangle();

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int shellCmdClass::drawActive ( void ) {

int tX, tY;
char string[39+1];
XRectangle xR = { x, y, w, h };

  if ( !activeMode || invisible ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( bgColor.getColor() );

  XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  if ( label.getExpanded() )
    strncpy( string, label.getExpanded(), 39 );
  else
    strncpy( string, "", 39 );

  // strncpy( string, label, 39 );

  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x+w, y );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, x, y+h );

  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y+h, x+w, y+h );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w, y, x+w, y+h );

  // top
  actWin->executeGc.setFG( actWin->ci->pix(topShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+w-1, y+1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+w-2, y+2 );

  // left
  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+1, x+1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+2, x+2, y+h-2 );

  // bottom
  actWin->executeGc.setFG( actWin->ci->pix(botShadowColor) );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+1, y+h-1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+2, y+h-2, x+w-2, y+h-2 );

  // right
  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-1, y+1, x+w-1, y+h-1 );

  XDrawLine( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x+w-2, y+2, x+w-2, y+h-2 );

  if ( fs ) {

    actWin->executeGc.addNormXClipRectangle( xR );

    actWin->executeGc.setFG( fgColor.getColor() );
    actWin->executeGc.setFontTag( fontTag, actWin->fi );

    tX = x + w/2;
    tY = y + h/2 - fontAscent/2;

    drawText( actWin->executeWidget, &actWin->executeGc, fs, tX, tY,
     XmALIGNMENT_CENTER, string );

    actWin->executeGc.removeNormXClipRectangle();

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int shellCmdClass::activate (
  int pass,
  void *ptr )
{

  switch ( pass ) {

  case 1:

    thread = NULL;
    activeMode = 1;
    aglPtr = ptr;

    if ( autoExecInterval > 0.5 ) {
      timerValue = (int) ( autoExecInterval * 1000.0 );
      timer = XtAppAddTimeOut( actWin->appCtx->appContext(),
       timerValue, shcmdc_executeCmd, this );
      timerActive = 1;
    }

    break;

  case 2:
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
    if ( timerActive ) {
      XtRemoveTimeOut( timer );
      timerActive = 0;
    }
    if ( thread ) {
      if ( !multipleInstancesAllowed ) {
        stat = thread_detach( thread );
        stat = thread_destroy_handle( thread );
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

int shellCmdClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = shellCommand.expand1st( numMacros, macros, expansions );
  stat = label.expand1st( numMacros, macros, expansions );

  return stat;

}

int shellCmdClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = shellCommand.expand2nd( numMacros, macros, expansions );
  stat = label.expand2nd( numMacros, macros, expansions );

  return stat;

}

int shellCmdClass::containsMacros ( void ) {

  if ( shellCommand.containsPrimaryMacros() ) return 1;
  if ( label.containsPrimaryMacros() ) return 1;

  return 0;

}

void shellCmdClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

}

void shellCmdClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

int stat;
threadParamBlockPtr threadParamBlock;

  actWin->substituteSpecial( 127, shellCommand.getExpanded(),
   bufShellCommand );

  //printf( "shellCmdClass::btnDown, shellCommand = [%s]\n",
  // bufShellCommand );

  if ( multipleInstancesAllowed ) {

    threadParamBlock = new threadParamBlockType;
    threadParamBlock->cmd = new (char)[strlen(bufShellCommand)+1];
    strcpy( threadParamBlock->cmd, bufShellCommand );
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
        stat = thread_destroy_handle( thread );
        threadParamBlock = new threadParamBlockType;
        threadParamBlock->cmd = new (char)[strlen(bufShellCommand)+1];
        strcpy( threadParamBlock->cmd, bufShellCommand );
        threadParamBlock->multipleInstancesAllowed =
         multipleInstancesAllowed;
        threadParamBlock->secondsToDelay = (float) threadSecondsToDelay;
        stat = thread_create_handle( &thread, threadParamBlock );
        stat = thread_create_proc( thread, shellCmdThread );
      }

    }
    else {

      threadParamBlock = new threadParamBlockType;
      threadParamBlock->cmd = new (char)[strlen(bufShellCommand)+1];
      strcpy( threadParamBlock->cmd, bufShellCommand );
      threadParamBlock->multipleInstancesAllowed =
       multipleInstancesAllowed;
      threadParamBlock->secondsToDelay = (float) threadSecondsToDelay;
      stat = thread_create_handle( &thread, threadParamBlock );
      stat = thread_create_proc( thread, shellCmdThread );

    }

  }

  *action = closeAction;

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

  if ( !blank( shellCommand.getRaw() ) )
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
