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

#define __pip_cc 1

#include "pip.h"
#include <sys/stat.h>
#include <unistd.h>
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void pipc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;

  pipo->actWin->setChanged();

  pipo->eraseSelectBoxCorners();
  pipo->erase();

  pipo->fgColor.setColorIndex(
   pipo->bufFgColor, pipo->actWin->ci );

  pipo->bgColor.setColorIndex(
   pipo->bufBgColor, pipo->actWin->ci );

  pipo->topShadowColor.setColorIndex(
   pipo->bufTopShadowColor, pipo->actWin->ci );

  pipo->botShadowColor.setColorIndex(
   pipo->bufBotShadowColor, pipo->actWin->ci );

  pipo->readPvExpStr.setRaw( pipo->bufReadPvName );

  pipo->fileNameExpStr.setRaw( pipo->bufFileName );

  pipo->x = pipo->bufX;
  pipo->sboxX = pipo->bufX;

  pipo->y = pipo->bufY;
  pipo->sboxY = pipo->bufY;

  pipo->w = pipo->bufW;
  pipo->sboxW = pipo->bufW;

  pipo->h = pipo->bufH;
  pipo->sboxH = pipo->bufH;

  if ( pipo->h < pipo->minH ) {
    pipo->h = pipo->minH;
    pipo->sboxH = pipo->minH;
  }

}

static void pipc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;

  pipc_edit_update ( w, client, call );
  pipo->refresh( pipo );

}

static void pipc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;

  pipc_edit_update ( w, client, call );
  pipo->ef.popdown();
  pipo->operationComplete();

}

static void pipc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;

  pipo->ef.popdown();
  pipo->operationCancel();

}

static void pipc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activePipClass *pipo = (activePipClass *) client;

  pipo->ef.popdown();
  pipo->operationCancel();
  pipo->erase();
  pipo->deleteRequest = 1;
  pipo->drawAll();

}

#ifdef __epics__

static void pip_monitor_read_connect_state (
  struct connection_handler_args arg )
{

activePipClass *pipo =
 (activePipClass *) ca_puser(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    pipo->needConnectInit = 1;

  }
  else {

    pipo->readPvConnected = 0;
    pipo->active = 0;
    pipo->fgColor.setDisconnected();
    pipo->needDraw = 1;

  }

  pipo->actWin->appCtx->proc->lock();
  pipo->actWin->addDefExeNode( pipo->aglPtr );
  pipo->actWin->appCtx->proc->unlock();

}

static void pip_readUpdate (
  struct event_handler_args ast_args )
{

char *str;
activePipClass *pipo = (activePipClass *) ast_args.usr;

  if ( pipo->active ) {

    str = (char *) ast_args.dbr;
    if ( str ) strncpy( pipo->curReadV, str, 39 );

    pipo->actWin->appCtx->proc->lock();
    pipo->needUpdate = 1;
    pipo->actWin->addDefExeNode( pipo->aglPtr );
    pipo->actWin->appCtx->proc->unlock();

  }

}

#endif

activePipClass::activePipClass ( void ) {

  name = new char[strlen("activePipClass")+1];
  strcpy( name, "activePipClass" );
  minW = 50;
  minH = 50;
  activeMode = 0;
  frameWidget = NULL;
  aw = NULL;
  strcpy( curFileName, "" );
  readPvId = 0;
  readEventId = 0;
  activateIsComplete = 0;

}

// copy constructor
activePipClass::activePipClass
 ( const activePipClass *source ) {

activeGraphicClass *pipo = (activeGraphicClass *) this;

  pipo->clone( (activeGraphicClass *) source );

  name = new char[strlen("activePipClass")+1];
  strcpy( name, "activePipClass" );

  fgCb = source->fgCb;
  bgCb = source->bgCb;
  topCb = source->topCb;
  botCb = source->botCb;

  fgColor.copy( source->fgColor );
  bgColor.copy( source->bgColor );
  topShadowColor.copy( source->topShadowColor );
  botShadowColor.copy( source->botShadowColor );

  readPvExpStr.copy( source->readPvExpStr );
  fileNameExpStr.copy( source->fileNameExpStr );

  minW = 50;
  minH = 50;
  frameWidget = NULL;
  aw = NULL;
  strcpy( curFileName, "" );
  readPvId = 0;
  readEventId = 0;
  activateIsComplete = 0;

}

activePipClass::~activePipClass ( void ) {

  if ( name ) delete name;

}


int activePipClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  actWin = (activeWindowClass *) aw_obj;

  if ( _w < minW )
    w = minW;
  else
    w = _w;

  if ( _h < minH )
    h = minH;
  else
    h = _h;

  x = _x;
  y = _y;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  topShadowColor.setColorIndex( actWin->defaultTopShadowColor, actWin->ci );
  botShadowColor.setColorIndex( actWin->defaultBotShadowColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int activePipClass::save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", PIPC_MAJOR_VERSION,
   PIPC_MINOR_VERSION, PIPC_RELEASE );

  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = topShadowColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = botShadowColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  if ( readPvExpStr.getRaw() )
    writeStringToFile( f, readPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( fileNameExpStr.getRaw() )
    writeStringToFile( f, fileNameExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  return 1;

}

int activePipClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int index;
int major, minor, release;
char oneName[activeGraphicClass::MAX_PV_NAME+1];
char oneFileName[127+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  if ( major > PIPC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }
  fscanf( f, "%d\n", &x );
  fscanf( f, "%d\n", &y );
  fscanf( f, "%d\n", &w );
  fscanf( f, "%d\n", &h );

  this->initSelectBox();

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  fgColor.setColorIndex( index, actWin->ci );

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  bgColor.setColorIndex( index, actWin->ci );

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  topShadowColor.setColorIndex( index, actWin->ci );

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  botShadowColor.setColorIndex( index, actWin->ci );

  readStringFromFile( oneName, activeGraphicClass::MAX_PV_NAME+1, f );
  readPvExpStr.setRaw( oneName );

  readStringFromFile( oneFileName, 127, f );
  fileNameExpStr.setRaw( oneFileName );

  return 1;

}

int activePipClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activePipClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, activePipClass_str4, 31 );

  Strncat( title, activePipClass_str5, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufFgColor = fgColor.pixelIndex();
  bufBgColor = bgColor.pixelIndex();
  bufTopShadowColor = topShadowColor.pixelIndex();
  bufBotShadowColor = botShadowColor.pixelIndex();

  if ( readPvExpStr.getRaw() )
    strncpy( bufReadPvName, readPvExpStr.getRaw(),
     activeGraphicClass::MAX_PV_NAME );
  else
    strcpy( bufReadPvName, "" );

  if ( fileNameExpStr.getRaw() )
    strncpy( bufFileName, fileNameExpStr.getRaw(), 127 );
  else
    strcpy( bufFileName, "" );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( activePipClass_str6, 35, &bufX );
  ef.addTextField( activePipClass_str7, 35, &bufY );
  ef.addTextField( activePipClass_str8, 35, &bufW );
  ef.addTextField( activePipClass_str9, 35, &bufH );
  ef.addTextField( activePipClass_str11, 35, bufReadPvName,
   activeGraphicClass::MAX_PV_NAME );
  ef.addTextField( activePipClass_str12, 35, bufFileName, 127 );
  ef.addColorButton( activePipClass_str16, actWin->ci, &fgCb, &bufFgColor );
  ef.addColorButton( activePipClass_str18, actWin->ci, &bgCb, &bufBgColor );
  ef.addColorButton( activePipClass_str19, actWin->ci, &topCb,
   &bufTopShadowColor );
  ef.addColorButton( activePipClass_str20, actWin->ci, &botCb,
   &bufBotShadowColor );

  return 1;

}

int activePipClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( pipc_edit_ok, pipc_edit_apply, pipc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activePipClass::edit ( void ) {

  this->genericEdit();
  ef.finished( pipc_edit_ok, pipc_edit_apply, pipc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activePipClass::erase ( void ) {

  if ( deleteRequest ) return 1;

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int activePipClass::eraseActive ( void ) {

  return 1;

}

int activePipClass::draw ( void ) {

  if ( deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( bgColor.pixelColor() );
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.setFG( fgColor.pixelColor() );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  actWin->drawGc.restoreFg();

  return 1;

}

int activePipClass::drawActive ( void ) {

  if ( !activeMode || !init ) return 1;

  if ( aw ) {
    if ( aw->loadFailure ) {
      aw = NULL;
      frameWidget = NULL;
    }
    else {
      if ( frameWidget ) {
        if ( *frameWidget ) XtMapWidget( *frameWidget );
      }
    }
  }

  return 1;

}

int activePipClass::activate (
  int pass,
  void *ptr )
{

int stat;

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      opComplete = 1;

      aglPtr = ptr;
      needConnectInit = needUpdate = needDraw = needFileOpen = 0;
      activateIsComplete = 0;

#ifdef __epics__
      readPvId = 0;
      readEventId = 0;
#endif

      readPvConnected = active = init = 0;
      activeMode = 1;

      if ( !readPvExpStr.getExpanded() ||
            blank( readPvExpStr.getExpanded() ) ) {
        readExists = 0;
      }
      else {
        readExists = 1;
        fgColor.setConnectSensitive();
      }

      if ( readExists || !fileNameExpStr.getExpanded() ||
            blank( fileNameExpStr.getExpanded() ) ) {
        fileExists = 0;
      }
      else {
        fileExists = 1;
      }

      if ( fileExists ) {
        needFileOpen = 1;
        actWin->addDefExeNode( aglPtr );
      }
      else {
        activateIsComplete = 1;
      }

#ifdef __epics__

      if ( readExists ) {
        stat = ca_search_and_connect( readPvExpStr.getExpanded(), &readPvId,
         pip_monitor_read_connect_state, this );
        if ( stat != ECA_NORMAL ) {
          printf( activePipClass_str22 );
        }
      }

#endif

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

int activePipClass::deactivate (
  int pass
) {

int stat, okToClose;
activeWindowListPtr cur;

  if ( pass == 1 ) {

    active = 0;
    activeMode = 0;

    if ( aw ) {
      if ( aw->loadFailure ) {
        aw = NULL;
        frameWidget = NULL;
      }
    }

    if ( frameWidget ) {
      if ( *frameWidget ) XtUnmapWidget( *frameWidget );
    }

    if ( aw ) {

      okToClose = 0;
      // make sure the window was successfully opened
      cur = actWin->appCtx->head->flink;
      while ( cur != actWin->appCtx->head ) {

        if ( &cur->node == aw ) {
          okToClose = 1;
          break;
	}
        cur = cur->flink;
      }

      if ( okToClose ) {
        aw->returnToEdit( 1 );
      }

      aw = NULL;

    }

    if ( frameWidget ) {
      frameWidget = NULL;
    }

#ifdef __epics__

    if ( readPvId ) {
      stat = ca_clear_channel( readPvId );
      readPvId = NULL;
      if ( stat != ECA_NORMAL )
        printf( activePipClass_str23 );
    }

#endif

  }

  return 1;

}

int activePipClass::preReactivate (
  int pass ) {

int stat, okToClose;
activeWindowListPtr cur;

  if ( pass == 1 ) {

    active = 0;
    activeMode = 0;

    if ( aw ) {
      if ( aw->loadFailure ) {
        aw = NULL;
        frameWidget = NULL;
      }
    }

    if ( aw ) {

      okToClose = 0;
      // make sure the window was successfully opened
      cur = actWin->appCtx->head->flink;
      while ( cur != actWin->appCtx->head ) {
        if ( &cur->node == aw ) {
          okToClose = 1;
          break;
        }
        cur = cur->flink;
      }

      if ( okToClose ) {
        aw->returnToEdit( 1 );
      }

      aw = NULL;

    }

#ifdef __epics__

    if ( readPvId ) {
      stat = ca_clear_channel( readPvId );
      if ( stat != ECA_NORMAL )
        printf( activePipClass_str23 );
    }

#endif

  }

  return 1;

}

int activePipClass::reactivate (
  int pass,
  void *ptr ) {

int status;

  switch ( pass ) {

  case 1:

    opComplete = 0;

    break;

  case 2:

    if ( !opComplete ) {

      opComplete = 1;

      aglPtr = ptr;
      needConnectInit = needUpdate = needDraw = needFileOpen = 0;


#ifdef __epics__
      readPvId = 0;
      readEventId = 0;
#endif

      readPvConnected = active = init = 0;
      activeMode = 1;

      if ( !readPvExpStr.getExpanded() ||
            blank( readPvExpStr.getExpanded() ) ) {
        readExists = 0;
      }
      else {
        readExists = 1;
        fgColor.setConnectSensitive();
      }

      if ( !fileNameExpStr.getExpanded() ||
            blank( fileNameExpStr.getExpanded() ) ) {
        fileExists = 0;
      }
      else {
        fileExists = 1;
      }

      if ( fileExists ) {
        needFileOpen = 1;
        actWin->addDefExeNode( aglPtr );
      }
      else {
        activateIsComplete = 1;
      }

#ifdef __epics__

      if ( readExists ) {
        status = ca_search_and_connect( readPvExpStr.getExpanded(), &readPvId,
         pip_monitor_read_connect_state, this );
        if ( status != ECA_NORMAL ) {
          printf( activePipClass_str22 );
        }
      }

#endif

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

int activePipClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = readPvExpStr.expand1st( numMacros, macros, expansions );
  stat = fileNameExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activePipClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat;

  retStat = 1;

  stat = readPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  stat = fileNameExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int activePipClass::containsMacros ( void ) {

  if ( readPvExpStr.containsPrimaryMacros() ) return 1;

  if ( fileExists && fileNameExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int activePipClass::createPipWidgets ( void ) {

  frameWidget = new Widget;
  *frameWidget = NULL;

  *frameWidget = XtVaCreateManagedWidget( "", xmBulletinBoardWidgetClass,
   actWin->executeWidgetId(),
   XmNx, x,
   XmNy, y,
   XmNwidth, w,
   XmNheight, h,
   XmNresizePolicy, XmRESIZE_NONE,
   XmNmarginWidth, 0,
   XmNmarginHeight, 0,
   //XmNshadowThickness, 2,
   //XmNshadowType, XmSHADOW_ETCHED_OUT,
   //XmNtopShadowColor, topShadowColor.pixelColor(),
   //XmNbottomShadowColor, botShadowColor.pixelColor(),
   XmNbackground, bgColor.pixelColor(),
   XmNmappedWhenManaged, False,
   NULL );

  if ( !(*frameWidget) ) {
    printf( activePipClass_str24 );
    frameWidget = NULL;
    return 0;
  }

  return 1;

}

int activePipClass::checkResizeSelectBox (
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

int activePipClass::checkResizeSelectBoxAbs (
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

void activePipClass::executeDeferred ( void ) {

char v[39+1];
int stat, nc, nu, nd, nfo, okToClose;
activeWindowListPtr cur;

//----------------------------------------------------------------------------

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  nfo = needFileOpen; needFileOpen = 0;
  strncpy( v, curReadV, 39 );
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

//----------------------------------------------------------------------------

#ifdef __epics__

  if ( nc ) {

    readPvConnected = 1;
    active = 1;
    init = 1;

    if ( !readEventId ) {

      stat = ca_add_masked_array_event( DBR_STRING, 1, readPvId,
       pip_readUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &readEventId, DBE_VALUE );
      if ( stat != ECA_NORMAL )
        printf( activePipClass_str25 );

    }

    fgColor.setConnected();
    drawActive();

  }

#endif

  if ( nu ) {

    strncpy( readV, v, 39 );
    //printf( "readV = [%s]\n", readV );

    if ( !blank( readV ) ) {

      // close old

      if ( frameWidget ) {
        if ( *frameWidget ) XtUnmapWidget( *frameWidget );
      }

      if ( aw ) {

        okToClose = 0;
        // make sure the window was successfully opened
        cur = actWin->appCtx->head->flink;
        while ( cur != actWin->appCtx->head ) {
          if ( &cur->node == aw ) {
            okToClose = 1;
            break;
          }
          cur = cur->flink;
        }

        if ( okToClose ) {
          aw->returnToEdit( 1 );
        }

        aw = NULL;

      }

      if ( frameWidget ) {
        frameWidget = NULL;
      }

      // prevent possible mutual recursion
      if ( actWin->sameAncestorName( readV ) ) {

        actWin->appCtx->postMessage( activePipClass_str26 );
        activateIsComplete = 1;

      }
      else {

        // open new

        if ( !frameWidget ) {
          createPipWidgets();
        }

        if ( !aw ) {

          //printf( "1) Open file %s\n", readV );

          strncpy( curFileName, readV, 127 );
          curFileName[127] = 0;

          cur = new activeWindowListType;
          actWin->appCtx->addActiveWindow( cur );

          cur->node.createEmbedded( actWin->appCtx, frameWidget, 0, 0, w, h,
           x, y, actWin->numMacros, actWin->macros, actWin->expansions );

          cur->node.realize();

          cur->node.setGraphicEnvironment( &cur->node.appCtx->ci,
           &cur->node.appCtx->fi );

          cur->node.storeFileName( readV );

          actWin->appCtx->openActivateActiveWindow( &cur->node, 0, 0 );

          aw = &cur->node;

          aw->parent = actWin;
          (actWin->numChildren)++;

          drawActive();

        }

      }

    }

  }

//----------------------------------------------------------------------------

  if ( nd ) {
    drawActive();
  }

//----------------------------------------------------------------------------

  if ( nfo && fileExists ) {

    //printf( "2) Open file %s\n", fileNameExpStr.getExpanded() );

    strncpy( curFileName, fileNameExpStr.getExpanded(), 127 );
    curFileName[127] = 0;

    // prevent possible mutual recursion
    if ( actWin->sameAncestorName( curFileName ) ) {

      actWin->appCtx->postMessage( activePipClass_str26 );
      activateIsComplete = 1;

    }
    else {

      if ( !frameWidget ) {
        createPipWidgets();
      }

      if ( !aw ) {

        cur = new activeWindowListType;
        actWin->appCtx->addActiveWindow( cur );

        cur->node.createEmbedded( actWin->appCtx, frameWidget, 0, 0, w, h,
         x, y, actWin->numMacros, actWin->macros, actWin->expansions );

        cur->node.realize();

        cur->node.setGraphicEnvironment( &cur->node.appCtx->ci,
         &cur->node.appCtx->fi );

        cur->node.storeFileName( fileNameExpStr.getExpanded() );

        actWin->appCtx->openActivateActiveWindow( &cur->node, 0, 0 );

        aw = &cur->node;

        aw->parent = actWin;
        (actWin->numChildren)++;

        activateIsComplete = 1;

      }

    }

  }

//----------------------------------------------------------------------------

}

void activePipClass::changeDisplayParams (
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
    fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  if ( _flag & ACTGRF_TOPSHADOWCOLOR_MASK )
    topShadowColor.setColorIndex( actWin->defaultTopShadowColor, actWin->ci );

  if ( _flag & ACTGRF_BOTSHADOWCOLOR_MASK )
    botShadowColor.setColorIndex( actWin->defaultBotShadowColor, actWin->ci );

}

void activePipClass::changePvNames (
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

  if ( flag & ACTGRF_READBACKPVS_MASK ) {
    if ( numReadbackPvs ) {
      readPvExpStr.setRaw( readbackPvs[0] );
    }
  }

}

int activePipClass::isWindowContainer ( void ) {

  return 1;

}

int activePipClass::activateComplete ( void ) {

int flag;

  if ( aw ) {
    if ( aw->loadFailure ) {
      activateIsComplete = 1;
    }
  }

  if ( !activateIsComplete ) return 0;

  if ( aw ) {
    if ( aw->isExecuteMode() || aw->loadFailure ) {
      flag = aw->okToDeactivate();
    }
    else {
      flag = 0;
    }
  }
  else {
    flag = 1;
  }

  return flag;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activePipClassPtr ( void ) {

activePipClass *ptr;

  ptr = new activePipClass;
  return (void *) ptr;

}

void *clone_activePipClassPtr (
  void *_srcPtr )
{

activePipClass *ptr, *srcPtr;

  srcPtr = (activePipClass *) _srcPtr;

  ptr = new activePipClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
