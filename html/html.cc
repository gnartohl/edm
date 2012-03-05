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

#define __html_cc 1

#include "html.h"
#include "app_pkg.h"
#include "act_win.h"
#include "thread.h"

static void unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

htmlClass *htmlo = (htmlClass *) client;

  if ( !htmlo->init ) {
    htmlo->needToDrawUnconnected = 1;
    htmlo->needRefresh = 1;
    htmlo->actWin->addDefExeNode( htmlo->aglPtr );
  }

  htmlo->unconnectedTimer = 0;

}

void _edmDebug( void );

static void htmlc_anchor (
  Widget w,
  XtPointer client,
  XmHTMLAnchorCallbackStruct *cbs
) {

htmlClass *htmlo = (htmlClass *) client;

  _edmDebug();

  if ( cbs->url_type == ANCHOR_FILE_LOCAL ) {
    htmlo->actWin->appCtx->proc->lock();
    //fprintf( stderr, "local\n" );
    //fprintf( stderr, "href = [%s]\n", cbs->href );
    cbs->visited = True;
    if ( htmlo->hrefFileName ) delete[] htmlo->hrefFileName;
    htmlo->hrefFileName = new char[strlen(cbs->href)+1];
    strcpy( htmlo->hrefFileName, cbs->href );
    htmlo->needLink = 1;
    htmlo->actWin->addDefExeNode( htmlo->aglPtr );
    htmlo->actWin->appCtx->proc->unlock();
  }
  else if ( cbs->url_type == ANCHOR_JUMP ) {
    cbs->doit = True;
    cbs->visited = True;
  }

}

static void htmlc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

htmlClass *htmlo = (htmlClass *) client;

  htmlo->actWin->setChanged();

  htmlo->eraseSelectBoxCorners();
  htmlo->erase();

  htmlo->fgColor.setColorIndex( htmlo->eBuf->bufFgColor, htmlo->actWin->ci );

  htmlo->bgColor.setColorIndex( htmlo->eBuf->bufBgColor, htmlo->actWin->ci );

  htmlo->alarmPvExpStr.setRaw( htmlo->eBuf->bufAlarmPvName );

  htmlo->visPvExpStr.setRaw( htmlo->eBuf->bufVisPvName );

  htmlo->contentPvExpStr.setRaw( htmlo->eBuf->bufContentPvName );

  if ( htmlo->eBuf->bufVisInverted )
    htmlo->visInverted = 0;
  else
    htmlo->visInverted = 1;

  strncpy( htmlo->minVisString, htmlo->eBuf->bufMinVisString, 39 );
  strncpy( htmlo->maxVisString, htmlo->eBuf->bufMaxVisString, 39 );

  if ( htmlo->bufValue ) {
    htmlo->value.setRaw( htmlo->bufValue );
  }

  htmlo->stringLength = strlen( htmlo->value.getRaw() );

  if ( htmlo->bufDocRoot ) {
    htmlo->docRoot.setRaw( htmlo->bufDocRoot );
  }

  htmlo->useFile = htmlo->eBuf->bufUseFile;

  htmlo->x = htmlo->eBuf->bufX;
  htmlo->sboxX = htmlo->eBuf->bufX;

  htmlo->y = htmlo->eBuf->bufY;
  htmlo->sboxY = htmlo->eBuf->bufY;

  htmlo->w = htmlo->eBuf->bufW;
  htmlo->sboxW = htmlo->eBuf->bufW;

  htmlo->h = htmlo->eBuf->bufH;
  htmlo->sboxH = htmlo->eBuf->bufH;

  htmlo->updateDimensions();

}

static void htmlc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

htmlClass *htmlo = (htmlClass *) client;

  htmlc_edit_update ( w, client, call );
  htmlo->refresh( htmlo );

}

static void htmlc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

htmlClass *htmlo = (htmlClass *) client;

  htmlc_edit_update ( w, client, call );

  if ( htmlo->bufValue ) {
    delete[] htmlo->bufValue;
    htmlo->bufValue = NULL;
  }

  if ( htmlo->bufDocRoot ) {
    delete[] htmlo->bufDocRoot;
    htmlo->bufDocRoot = NULL;
  }

  htmlo->ef.popdown();
  htmlo->operationComplete();

}

static void htmlc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

htmlClass *htmlo = (htmlClass *) client;

  if ( htmlo->bufValue ) {
    delete[] htmlo->bufValue;
    htmlo->bufValue = NULL;
  }

  if ( htmlo->bufDocRoot ) {
    delete[] htmlo->bufDocRoot;
    htmlo->bufDocRoot = NULL;
  }

  htmlo->ef.popdown();
  htmlo->operationCancel();

}

static void htmlc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

htmlClass *htmlo = (htmlClass *) client;

  if ( htmlo->bufValue ) {
    delete[] htmlo->bufValue;
    htmlo->bufValue = NULL;
  }

  if ( htmlo->bufDocRoot ) {
    delete[] htmlo->bufDocRoot;
    htmlo->bufDocRoot = NULL;
  }

  htmlo->ef.popdown();
  htmlo->operationCancel();
  htmlo->erase();
  htmlo->deleteRequest = 1;
  htmlo->drawAll();

}

void htmlClass::alarmPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

htmlClass *htmlo = (htmlClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    htmlo->connection.setPvDisconnected( (void *) htmlo->alarmPvConnection );
    htmlo->fgColor.setDisconnected();
    htmlo->bgColor.setDisconnected();

    htmlo->actWin->appCtx->proc->lock();
    htmlo->needRefresh = 1;
    htmlo->actWin->addDefExeNode( htmlo->aglPtr );
    htmlo->actWin->appCtx->proc->unlock();

  }

}

void htmlClass::alarmPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

htmlClass *htmlo = (htmlClass *) userarg;

  if ( !htmlo->connection.pvsConnected() ) {

    htmlo->connection.setPvConnected( (void *) alarmPvConnection );

    if ( htmlo->connection.pvsConnected() ) {
      htmlo->actWin->appCtx->proc->lock();
      htmlo->needConnectInit = 1;
      htmlo->actWin->addDefExeNode( htmlo->aglPtr );
      htmlo->actWin->appCtx->proc->unlock();
    }

  }
  else {

    htmlo->actWin->appCtx->proc->lock();
    htmlo->needAlarmUpdate = 1;
    htmlo->actWin->addDefExeNode( htmlo->aglPtr );
    htmlo->actWin->appCtx->proc->unlock();

  }

}

void htmlClass::visPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

htmlClass *htmlo = (htmlClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    htmlo->connection.setPvDisconnected( (void *) htmlo->visPvConnection );
    htmlo->fgColor.setDisconnected();
    htmlo->bgColor.setDisconnected();

    htmlo->actWin->appCtx->proc->lock();
    htmlo->needRefresh = 1;
    htmlo->actWin->addDefExeNode( htmlo->aglPtr );
    htmlo->actWin->appCtx->proc->unlock();

  }

}

void htmlClass::visPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

htmlClass *htmlo = (htmlClass *) userarg;

  if ( !htmlo->connection.pvsConnected() ) {

    htmlo->connection.setPvConnected( (void *) visPvConnection );

    if ( htmlo->connection.pvsConnected() ) {
      htmlo->actWin->appCtx->proc->lock();
      htmlo->needConnectInit = 1;
      htmlo->actWin->addDefExeNode( htmlo->aglPtr );
      htmlo->actWin->appCtx->proc->unlock();
    }

  }
  else {

    htmlo->actWin->appCtx->proc->lock();
    htmlo->needVisUpdate = 1;
    htmlo->actWin->addDefExeNode( htmlo->aglPtr );
    htmlo->actWin->appCtx->proc->unlock();

    }

}

void htmlClass::contentPvConnectStateCallback (
  ProcessVariable *pv,
  void *userarg
) {

htmlClass *htmlo = (htmlClass *) userarg;

  if ( pv->is_valid() ) {

  }
  else { // lost connection

    htmlo->connection.setPvDisconnected( (void *) htmlo->contentPvConnection );
    htmlo->fgColor.setDisconnected();
    htmlo->bgColor.setDisconnected();

    htmlo->actWin->appCtx->proc->lock();
    htmlo->needRefresh = 1;
    htmlo->actWin->addDefExeNode( htmlo->aglPtr );
    htmlo->actWin->appCtx->proc->unlock();

  }

}

void htmlClass::contentPvValueCallback (
  ProcessVariable *pv,
  void *userarg
) {

htmlClass *htmlo = (htmlClass *) userarg;

  if ( !htmlo->connection.pvsConnected() ) {

    htmlo->connection.setPvConnected( (void *) contentPvConnection );

    if ( htmlo->connection.pvsConnected() ) {
      htmlo->actWin->appCtx->proc->lock();
      htmlo->needConnectInit = 1;
      htmlo->actWin->addDefExeNode( htmlo->aglPtr );
      htmlo->actWin->appCtx->proc->unlock();
    }

  }
  else {

    htmlo->actWin->appCtx->proc->lock();
    htmlo->needContentUpdate = 1;
    htmlo->actWin->addDefExeNode( htmlo->aglPtr );
    htmlo->actWin->appCtx->proc->unlock();

  }

}

htmlClass::htmlClass ( void ) {

  name = new char[strlen("html")+1];
  strcpy( name, "html" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );

  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  visPvExists = alarmPvExists = contentPvExists = 0;
  activeMode = 0;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  connection.setMaxPvs( 2 );
  unconnectedTimer = 0;
  bufValue = NULL;
  fileContents = NULL;
  hrefFileName = NULL;
  bufDocRoot = NULL;
  useFile = 0;
  eBuf = NULL;

}

// copy constructor
htmlClass::htmlClass
 ( const htmlClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("html")+1];
  strcpy( name, "html" );

  fgColor.copy(source->fgColor);
  bgColor.copy(source->bgColor);
  visInverted = source->visInverted;

  alarmPvExpStr.setRaw( source->alarmPvExpStr.rawString );
  visPvExpStr.setRaw( source->visPvExpStr.rawString );
  contentPvExpStr.setRaw( source->contentPvExpStr.rawString );

  visibility = 0;
  prevVisibility = -1;
  visPvExists = alarmPvExists = contentPvExists = 0;
  activeMode = 0;

  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );

  value.copy( source->value );

  stringLength = source->stringLength;
  bufValue = NULL;
  fileContents = NULL;
  hrefFileName = NULL;

  useFile = source->useFile;

  bufDocRoot = NULL;
  docRoot.copy( source->docRoot );

  connection.setMaxPvs( 2 );

  unconnectedTimer = 0;

  eBuf = NULL;

}

htmlClass::~htmlClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

  if ( bufValue ) delete[] bufValue;

  if ( bufDocRoot ) delete[] bufDocRoot;

  if ( hrefFileName ) delete[] hrefFileName;

  if ( fileContents ) delete[] fileContents;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

}

int htmlClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

int stat = 1;

  actWin = (activeWindowClass *) aw_obj;
  xOrigin = 0;
  yOrigin = 0;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  updateDimensions();

  this->draw();

  this->editCreate();

  return stat;

}

int htmlClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  if ( !bufValue ) {
    bufValue = new char[htmlClass::MAX_TEXT_LEN+1];
  }

  if ( !bufDocRoot ) {
    bufDocRoot = new char[htmlClass::MAX_FILENAME_LEN+1];
  }

  ptr = actWin->obj.getNameFromClass( "html" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, htmlClass_str4, 31 );

  Strncat( title, htmlClass_str5, 31 );

  eBuf->bufX = x;
  eBuf->bufY = y;
  eBuf->bufW = w;
  eBuf->bufH = h;

  eBuf->bufFgColor = fgColor.pixelIndex();

  eBuf->bufBgColor = bgColor.pixelIndex();

  if ( alarmPvExpStr.getRaw() )
    strncpy( eBuf->bufAlarmPvName, alarmPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufAlarmPvName, "" );

  if ( visPvExpStr.getRaw() )
    strncpy( eBuf->bufVisPvName, visPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufVisPvName, "" );

  if ( contentPvExpStr.getRaw() )
    strncpy( eBuf->bufContentPvName, contentPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->bufContentPvName, "" );

  if ( visInverted )
    eBuf->bufVisInverted = 0;
  else
    eBuf->bufVisInverted = 1;

  strncpy( eBuf->bufMinVisString, minVisString, 39 );
  strncpy( eBuf->bufMaxVisString, maxVisString, 39 );

  if ( value.getRaw() )
    strncpy( bufValue, value.getRaw(), htmlClass::MAX_TEXT_LEN );
  else
    strncpy( bufValue, "", htmlClass::MAX_TEXT_LEN );

  if ( docRoot.getRaw() )
    strncpy( bufDocRoot, docRoot.getRaw(), htmlClass::MAX_FILENAME_LEN );
  else
    strncpy( bufDocRoot, "", htmlClass::MAX_FILENAME_LEN );

  eBuf->bufUseFile = useFile;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( htmlClass_str7, 35, &eBuf->bufX );
  ef.addTextField( htmlClass_str8, 35, &eBuf->bufY );
  ef.addTextField( htmlClass_str9, 35, &eBuf->bufW );
  ef.addTextField( htmlClass_str10, 35, &eBuf->bufH );
  ef.addTextBox( htmlClass_str23, 32, 10, bufValue,
   htmlClass::MAX_TEXT_LEN );
  ef.addToggle( htmlClass_str11, &eBuf->bufUseFile );
  ef.addTextField( htmlClass_str12, 35, bufDocRoot, MAX_FILENAME_LEN );
  ef.addTextField( htmlClass_str25, 35, eBuf->bufContentPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addColorButton( htmlClass_str13, actWin->ci, &eBuf->fgCb, &eBuf->bufFgColor );
  ef.addColorButton( htmlClass_str16, actWin->ci, &eBuf->bgCb, &eBuf->bufBgColor );
  ef.addTextField( htmlClass_str18, 35, eBuf->bufAlarmPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( htmlClass_str19, 35, eBuf->bufVisPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addOption( " ", htmlClass_str20, &eBuf->bufVisInverted );
  ef.addTextField( htmlClass_str21, 35, eBuf->bufMinVisString, 39 );
  ef.addTextField( htmlClass_str22, 35, eBuf->bufMaxVisString, 39 );

  return 1;

}

int htmlClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( htmlc_edit_ok, htmlc_edit_apply, htmlc_edit_cancel_delete,
   this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int htmlClass::edit ( void ) {

  this->genericEdit();
  ef.finished( htmlc_edit_ok, htmlc_edit_apply, htmlc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int htmlClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
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
  tag.loadR( "alarmPv", &alarmPvExpStr, emptyStr );
  tag.loadR( "visPv", &visPvExpStr, emptyStr );
  tag.loadR( "visInvert", &visInverted, &zero );
  tag.loadR( "visMin", 39, minVisString, emptyStr );
  tag.loadR( "visMax", 39, maxVisString, emptyStr );
  tag.loadR( "htmlText", &value, emptyStr );
  tag.loadR( "docRoot", &docRoot );
  tag.loadR( "useFile", &useFile, &zero );
  tag.loadR( "contentPv", &contentPvExpStr, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > HTMLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox();

  return stat;

}

int htmlClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int index;
int major, minor, release;
char oneValue[htmlClass::MAX_TEXT_LEN+1],
 onePv[PV_Factory::MAX_PV_NAME+1];
int stat = 1;

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > HTMLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  fgColor.setColorIndex( index, actWin->ci );

  actWin->ci->readColorIndex( f, &index );
  actWin->incLine(); actWin->incLine();
  bgColor.setColorIndex( index, actWin->ci );

  readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  alarmPvExpStr.setRaw( onePv );

  readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  visPvExpStr.setRaw( onePv );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  readStringFromFile( minVisString, 39+1, f ); actWin->incLine();
  readStringFromFile( maxVisString, 39+1, f ); actWin->incLine();

  readStringFromFile( oneValue, htmlClass::MAX_TEXT_LEN+1, f );
   actWin->incLine();
  value.setRaw( oneValue );

  if ( value.getRaw() )
    stringLength = strlen( value.getRaw() );
  else
    stringLength = 0;

  fscanf( f, "%d\n", &useFile ); actWin->incLine();

  readStringFromFile( oneValue, htmlClass::MAX_FILENAME_LEN+1, f );
   actWin->incLine();
  docRoot.setRaw( oneValue );


  if ( ( ( major == 1 ) && ( minor > 0 ) ) || ( major > 1 ) ) {

    readStringFromFile( onePv, PV_Factory::MAX_PV_NAME+1, f );
     actWin->incLine();
    contentPvExpStr.setRaw( onePv );

  }

  return stat;

}

int htmlClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, more, index;
unsigned int pixel;
int stat = 1;
char *tk, *gotData, *context,
 oneValue[htmlClass::MAX_TEXT_LEN+1], buf[255+1];

  r = 0xffff;
  g = 0xffff;
  b = 0xffff;

  this->actWin = _actWin;

  fgColor.setColorIndex( actWin->defaultTextFgColor, actWin->ci );
  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    buf[255] = 0;
    if ( !gotData ) {
      actWin->appCtx->postMessage( htmlClass_str24 );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( htmlClass_str24 );
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
          actWin->appCtx->postMessage( htmlClass_str24 );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( htmlClass_str24 );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( htmlClass_str24 );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( htmlClass_str24 );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "value" ) == 0 ) {

        tk = strtok_r( NULL, "\"", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( htmlClass_str24 );
          return 0;
        }

        strncpy( oneValue, tk, htmlClass::MAX_TEXT_LEN );
        oneValue[htmlClass::MAX_TEXT_LEN] = 0;

      }
            
      else if ( strcmp( tk, "red" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( htmlClass_str24 );
          return 0;
        }

        r = atol( tk );

      }
            
      else if ( strcmp( tk, "green" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( htmlClass_str24 );
          return 0;
        }

        g = atol( tk );

      }
            
      else if ( strcmp( tk, "blue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( htmlClass_str24 );
          return 0;
        }

        b = atol( tk );

      }
            
    }

  } while ( more );

  actWin->ci->setRGB( r, g, b, &pixel );
  index = actWin->ci->pixIndex( pixel );
  fgColor.setColorIndex( index, actWin->ci );

  alarmPvExpStr.setRaw( "" );

  visPvExpStr.setRaw( "" );

  visInverted = 0;

  strcpy( minVisString, "1" );
  strcpy( maxVisString, "1" );

  value.setRaw( oneValue );

  if ( value.getRaw() )
    stringLength = strlen( value.getRaw() );
  else
    stringLength = 0;

  this->initSelectBox(); // call after getting x,y,w,h

  return stat;

}

int htmlClass::save (
  FILE *f )
{

int major, minor, release, stat;

tagClass tag;

int zero = 0;
char *emptyStr = "";

  major = HTMLC_MAJOR_VERSION;
  minor = HTMLC_MINOR_VERSION;
  release = HTMLC_RELEASE;

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
  tag.loadW( "alarmPv", &alarmPvExpStr, emptyStr );
  tag.loadW( "visPv", &visPvExpStr, emptyStr );
  tag.loadBoolW( "visInvert", &visInverted, &zero );
  tag.loadW( "visMin", minVisString, emptyStr );
  tag.loadW( "visMax", maxVisString, emptyStr );
  tag.loadComplexW( "htmlText", &value, emptyStr );
  tag.loadW( "docRoot", &docRoot, emptyStr );
  tag.loadBoolW( "useFile", &useFile, &zero );
  tag.loadW( "contentPv", &contentPvExpStr, emptyStr );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;


int index;

  fprintf( f, "%-d %-d %-d\n", HTMLC_MAJOR_VERSION, HTMLC_MINOR_VERSION,
   HTMLC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  if ( alarmPvExpStr.getRaw() )
    writeStringToFile( f, alarmPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( visPvExpStr.getRaw() )
    writeStringToFile( f, visPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  if ( value.getRaw() )
    writeStringToFile( f, value.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", useFile );

  if ( docRoot.getRaw() )
    writeStringToFile( f, docRoot.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1-1-0

  if ( contentPvExpStr.getRaw() )
    writeStringToFile( f, contentPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  return 1;

}

int htmlClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", HTMLC_MAJOR_VERSION, HTMLC_MINOR_VERSION,
   HTMLC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = fgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );

  if ( alarmPvExpStr.getRaw() )
    writeStringToFile( f, alarmPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( visPvExpStr.getRaw() )
    writeStringToFile( f, visPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", visInverted );
  writeStringToFile( f, minVisString );
  writeStringToFile( f, maxVisString );

  if ( value.getRaw() )
    writeStringToFile( f, value.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", useFile );

  if ( docRoot.getRaw() )
    writeStringToFile( f, docRoot.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1-1-0

  if ( contentPvExpStr.getRaw() )
    writeStringToFile( f, contentPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  return 1;

}

int htmlClass::drawActive ( void ) {

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      needToEraseUnconnected = 1;
    }
  }
  else if ( needToEraseUnconnected ) {
    needToEraseUnconnected = 0;
  }

  if ( !enabled || !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  if ( !widgetsMapped ) {
    XtMapWidget( bulBrd );
    widgetsMapped = 1;
  }

  return 1;

}

int htmlClass::eraseUnconditional ( void ) {

  if ( !enabled || !activeMode ) return 1;

  if ( widgetsMapped ) {
    XtUnmapWidget( bulBrd );
    widgetsMapped = 0;
  }

  return 1;

}

int htmlClass::eraseActive ( void ) {

  if ( !enabled || !activeMode ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  if ( widgetsMapped ) {
    XtUnmapWidget( bulBrd );
    widgetsMapped = 0;
  }

  return 1;

}

int htmlClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( alarmPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  alarmPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( visPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  visPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( contentPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  contentPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( value.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  value.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( docRoot.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  docRoot.setRaw( tmpStr.getExpanded() );

  return 1;

}

int htmlClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat;

  retStat = 1; // success

  stat = alarmPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = contentPvExpStr.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = value.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = docRoot.expand1st( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int htmlClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat, retStat;

  retStat = 1; // success

  stat = alarmPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = visPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = contentPvExpStr.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = value.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;
  stat = docRoot.expand2nd( numMacros, macros, expansions );
  if ( !( stat & 1 ) ) retStat = stat;

  return retStat;

}

int htmlClass::containsMacros ( void ) {

  if ( alarmPvExpStr.containsPrimaryMacros() ) return 1;
  if ( visPvExpStr.containsPrimaryMacros() ) return 1;
  if ( contentPvExpStr.containsPrimaryMacros() ) return 1;
  if ( value.containsPrimaryMacros() ) return 1;
  if ( docRoot.containsPrimaryMacros() ) return 1;

  return 0;

}

int htmlClass::activate (
  int pass,
  void *ptr )
{

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

      connection.init();
      initEnable();

      aglPtr = ptr;

      curFgColorIndex = -1;
      curBgColorIndex = -1;
      curStatus = -1;
      curSeverity = -1;
      prevVisibility = -1;
      visibility = 0;
      prevFgVisibility = -1;
      fgVisibility = 0;
      prevBgVisibility = -1;
      bgVisibility = 0;

      needConnectInit = needAlarmUpdate = needVisUpdate = needRefresh =
        needPropertyUpdate = needLink = needOpenFile = needContentUpdate = 0;
      needToEraseUnconnected = 0;
      needToDrawUnconnected = 0;
      unconnectedTimer = 0;

      bulBrd = (Widget) NULL;
      htmlBox = (Widget) NULL;

      bulBrd = XtVaCreateWidget( "", xmBulletinBoardWidgetClass,
       actWin->executeWidgetId(),
       XmNx, x,
       XmNy, y,
       XmNwidth, w,
       XmNheight, h,
       XmNbackground, (XtArgVal) bgColor.getColor(),
       XmNforeground, (XtArgVal) fgColor.getColor(),
       XmNmarginHeight, 0,
       XmNmarginWidth, 0,
       XmNmappedWhenManaged, False,
       NULL );

      htmlBox = XtVaCreateManagedWidget( "html", xmHTMLWidgetClass,
       bulBrd,
       XmNwidth, w,
       XmNheight, h,
       XmNmarginHeight, 0,
       XmNmarginWidth, 0,
       XmNx, 0,
       XmNy, 0,
       XmNbackground, (XtArgVal) bgColor.getColor(),
       XmNforeground, (XtArgVal) fgColor.getColor(),
       NULL);

      if ( useFile ) {
        needOpenFile = 1;
        actWin->addDefExeNode( aglPtr );
      }
      else {
        XmHTMLTextSetString( htmlBox, value.getExpanded() );
      }

      XtAddCallback( htmlBox, XmNactivateCallback,
       (XtCallbackProc) htmlc_anchor, this );

      XtManageChild( bulBrd );

      if ( !enabled ) {
        XtUnmapWidget( bulBrd );
      }

      widgetsCreated = 1;
      widgetsMapped = 0;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      stringLength = strlen( value.getExpanded() );

      alarmPvId = visPvId = contentPvId = 0;

      pvType = -1;

      init = 1; // this stays true if there are no pvs

      if ( !alarmPvExpStr.getExpanded() ||
           // ( strcmp( alarmPvExpStr.getExpanded(), "" ) == 0 ) ) {
	   blankOrComment( alarmPvExpStr.getExpanded() ) ) {
        alarmPvExists = 0;
        fgVisibility = bgVisibility = 1;
      }
      else {
        connection.addPv();
        alarmPvExists = 1;
        fgColor.setConnectSensitive();
        bgColor.setConnectSensitive();
        init = 0;
      }

      if ( !visPvExpStr.getExpanded() ||
           // ( strcmp( visPvExpStr.getExpanded(), "" ) == 0 ) ) {
	   blankOrComment( visPvExpStr.getExpanded() ) ) {
        visPvExists = 0;
        visibility = 1;
      }
      else {
        connection.addPv();
        visPvExists = 1;
        visibility = 0;
        fgVisibility = bgVisibility = 1;
        fgColor.setConnectSensitive();
        bgColor.setConnectSensitive();
        init = 0;
      }

      if ( !contentPvExpStr.getExpanded() ||
           // ( strcmp( contentPvExpStr.getExpanded(), "" ) == 0 ) ) {
	   blankOrComment( contentPvExpStr.getExpanded() ) ) {
        contentPvExists = 0;
        fgVisibility = bgVisibility = 1;
      }
      else {
        connection.addPv();
        contentPvExists = 1;
        fgColor.setConnectSensitive();
        bgColor.setConnectSensitive();
        init = 0;
      }

      if ( alarmPvExists ) {
        alarmPvId = the_PV_Factory->create( alarmPvExpStr.getExpanded() );
        if ( alarmPvId ) {
          if ( alarmPvId->is_valid() ) {
            alarmPvConnectStateCallback( alarmPvId, this );
            alarmPvValueCallback( alarmPvId, this );
	  }
          alarmPvId->add_conn_state_callback( alarmPvConnectStateCallback,
           this );
          alarmPvId->add_value_callback( alarmPvValueCallback, this );
	}
      }

      if ( visPvExists ) {
        visPvId = the_PV_Factory->create( visPvExpStr.getExpanded() );
        if ( visPvId ) {
          if ( visPvId->is_valid() ) {
            visPvConnectStateCallback( visPvId, this );
            visPvValueCallback( visPvId, this );
          }
          visPvId->add_conn_state_callback( visPvConnectStateCallback, this );
          visPvId->add_value_callback( visPvValueCallback, this );
	}
      }

      if ( contentPvExists ) {
        contentPvId = the_PV_Factory->create( contentPvExpStr.getExpanded() );
        if ( contentPvId ) {
          if ( contentPvId->is_valid() ) {
            contentPvConnectStateCallback( contentPvId, this );
            contentPvValueCallback( contentPvId, this );
	  }
          contentPvId->add_conn_state_callback( contentPvConnectStateCallback,
           this );
          contentPvId->add_value_callback( contentPvValueCallback, this );
	}
      }

      activeMode = 1;

      opComplete = 1;

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

int htmlClass::deactivate (
  int pass )
{

  if ( pass == 1 ) {

    activeMode = 0;

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    if ( value.getRaw() )
      stringLength = strlen( value.getRaw() );
    else
      stringLength = 0;

    if ( alarmPvId ) {
      alarmPvId->remove_conn_state_callback( alarmPvConnectStateCallback,
       this );
      alarmPvId->remove_value_callback( alarmPvValueCallback, this );
      alarmPvId->release();
      alarmPvId = 0;
    }

    if ( visPvId ) {
      visPvId->remove_conn_state_callback( visPvConnectStateCallback, this );
      visPvId->remove_value_callback( visPvValueCallback, this );
      visPvId->release();
      visPvId = 0;
    }

    if ( contentPvId ) {
      contentPvId->remove_conn_state_callback( contentPvConnectStateCallback,
       this );
      contentPvId->remove_value_callback( contentPvValueCallback, this );
      contentPvId->release();
      contentPvId = 0;
    }

  }
  else if ( pass == 2 ) {

    if ( widgetsCreated ) {
      if ( bulBrd ) {
        if ( widgetsMapped ) {
          XtUnmapWidget( bulBrd );
          widgetsMapped = 0;
        }
        XtDestroyWidget( htmlBox );
        htmlBox = NULL;
        XtDestroyWidget( bulBrd );
        bulBrd = NULL;
      }
      widgetsCreated = 0;
    }

  }

  return 1;

}

int htmlClass::draw ( void ) {

  if ( activeMode ) return 1;
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

int htmlClass::erase ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

void htmlClass::updateDimensions ( void )
{

  if ( activeMode ) {
    if ( value.getExpanded() )
      stringLength = strlen( value.getExpanded() );
    else
      stringLength = 0;
  }
  else {
    if ( value.getRaw() )
      stringLength = strlen( value.getRaw() );
    else
      stringLength = 0;
  }

}

void htmlClass::executeDeferred ( void ) {

int stat, nc, nau, nvu, nr, npu, nl, nof, ncu, index, change;
pvValType pvV;

char tmp[contentSize+1];

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();
  nc = needConnectInit; needConnectInit = 0;
  nau = needAlarmUpdate; needAlarmUpdate = 0;
  nvu = needVisUpdate; needVisUpdate = 0;
  ncu = needContentUpdate; needContentUpdate = 0;
  nr = needRefresh; needRefresh = 0;
  npu = needPropertyUpdate; needPropertyUpdate = 0;
  nl = needLink; needLink = 0;
  nof = needOpenFile; needOpenFile = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;

  if ( nc ) {

    minVis.d = (double) atof( minVisString );
    maxVis.d = (double) atof( maxVisString );

    fgColor.setConnected();
    bgColor.setConnected();

    if ( alarmPvExists ) {

      curStatus = alarmPvId->get_status();
      curSeverity = alarmPvId->get_severity();

      fgColor.setStatus( curStatus, curSeverity );
      bgColor.setStatus( curStatus, curSeverity );

      curFgColorIndex = actWin->ci->evalRule( fgColor.pixelIndex(),
       alarmPvId->get_double() );
      fgColor.changeIndex( curFgColorIndex, actWin->ci );

      curBgColorIndex = actWin->ci->evalRule( bgColor.pixelIndex(),
       alarmPvId->get_double() );
      bgColor.changeIndex( curBgColorIndex, actWin->ci );

      if ( !visPvExists ) {

        if ( actWin->ci->isInvisible( curFgColorIndex ) ) {
          prevFgVisibility = fgVisibility = 0;
        }
        else {
          prevFgVisibility = fgVisibility = 1;
        }

        if ( actWin->ci->isInvisible( curBgColorIndex ) ) {
          prevBgVisibility = bgVisibility = 0;
        }
        else {
          prevBgVisibility = bgVisibility = 1;
        }

      }

    }

    if ( visPvExists ) {

      pvV.d = visPvId->get_double();
      if ( ( pvV.d >= minVis.d ) && ( pvV.d < maxVis.d ) )
        visibility = 1 ^ visInverted;
      else
        visibility = 0 ^ visInverted;

      prevVisibility = visibility;

    }

    if ( contentPvExists ) {
       ncu = 1;
    }

    init = 1;

    eraseUnconditional();
    stat = smartDrawAllActive();

  }

  if ( nau ) {

    change = 0;

    if ( curStatus != alarmPvId->get_status() ) {
      curStatus = alarmPvId->get_status();
      change = 1;
    }

    if ( curSeverity != alarmPvId->get_severity() ) {
      curSeverity = alarmPvId->get_severity();
      change = 1;
    }

    if ( change ) {
      fgColor.setStatus( curStatus, curSeverity );
      bgColor.setStatus( curStatus, curSeverity );
    }

    index = actWin->ci->evalRule( fgColor.pixelIndex(),
     alarmPvId->get_double() );

    if ( curFgColorIndex != index ) {
      curFgColorIndex = index;
      change = 1;
    }

    index = actWin->ci->evalRule( bgColor.pixelIndex(),
     alarmPvId->get_double() );

    if ( curBgColorIndex != index ) {
      curBgColorIndex = index;
      change = 1;
    }

    if ( change ) {

      if ( !visPvExists ) {

        if ( actWin->ci->isInvisible( curFgColorIndex ) ) {
          fgVisibility = 0;
        }
        else {
          fgVisibility = 1;
        }

        if ( actWin->ci->isInvisible( curBgColorIndex ) ) {
          bgVisibility = 0;
        }
        else {
          bgVisibility = 1;
        }

      }

      fgColor.changeIndex( curFgColorIndex, actWin->ci );
      bgColor.changeIndex( curBgColorIndex, actWin->ci );
      if ( ( prevFgVisibility != fgVisibility ) ||
	   ( prevBgVisibility != bgVisibility ) ) {
        prevFgVisibility = fgVisibility;
        prevBgVisibility = bgVisibility;
        eraseUnconditional();
      }
      smartDrawAllActive();

    }

  }

  if ( nvu ) {

    pvV.d = visPvId->get_double();
    if ( ( pvV.d >= minVis.d ) && ( pvV.d < maxVis.d ) )
      visibility = 1 ^ visInverted;
    else
      visibility = 0 ^ visInverted;

    if ( prevVisibility != visibility ) {
      if ( !visibility ) eraseUnconditional();
      stat = smartDrawAllActive();
    }

  }

  if ( ncu ) {

    contentPvId->get_string( tmp, htmlClass::contentSize );
    contentStringExpStr.setRaw( tmp );
    contentStringExpStr.expand1st( actWin->numMacros, actWin->macros, actWin->expansions );

    if ( useFile ) {
      loadFile( contentStringExpStr.getExpanded() );
      if ( fileContents ) {
        XmHTMLTextSetString( htmlBox, fileContents );
        delete[] fileContents;
        fileContents = NULL;
      }
    }
    else {
      XmHTMLTextSetString( htmlBox, contentStringExpStr.getExpanded() );
    }

  }

  if ( nr ) {
    stat = smartDrawAllActive();
  }

  if ( npu ) {

    eraseActive();

    if ( bufValue ) {
      value.setRaw( bufValue );
      stringLength = strlen( bufValue );
      delete[] bufValue;
      bufValue = NULL;
    }

    updateDimensions();

    stat = smartDrawAllActive();

  }

  if ( nof ) {

    if ( value.getExpanded() ) {
      loadFile( value.getExpanded() );
      if ( fileContents ) {
        XmHTMLTextSetString( htmlBox, fileContents );
        delete[] fileContents;
        fileContents = NULL;
      }
    }

    //stat = smartDrawAllActive();

  }

  if ( nl ) {

    if ( hrefFileName ) {
      loadFile( hrefFileName );
      if ( fileContents ) {
        XmHTMLTextSetString( htmlBox, fileContents );
        delete[] fileContents;
        fileContents = NULL;
      }
    }

    //stat = smartDrawAllActive();

  }

}

char *htmlClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

char *htmlClass::nextDragName ( void ) {

  if ( !enabled ) return NULL;

  if ( dragIndex < (int) ( sizeof(dragName) / sizeof(char *) ) - 1 ) {
    dragIndex++;
    return dragName[dragIndex];
  }
  else {
    return NULL;
  }

}

char *htmlClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    switch ( i ) {

    case 0:
      return alarmPvExpStr.getExpanded();

    case 1:
      return visPvExpStr.getExpanded();

    case 2:
      return contentPvExpStr.getExpanded();

    }

  }
  else {

    switch ( i ) {

    case 0:
      return alarmPvExpStr.getRaw();

    case 1:
      return visPvExpStr.getRaw();

    case 2:
      return contentPvExpStr.getRaw();

    }

  }

  return (char *) NULL;

}

void htmlClass::changeDisplayParams (
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

}

void htmlClass::changePvNames (
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
      contentPvExpStr.setRaw( visPvs[0] );
    }
  }

  if ( flag & ACTGRF_VISPVS_MASK ) {
    if ( numVisPvs ) {
      visPvExpStr.setRaw( visPvs[0] );
    }
  }

  if ( flag & ACTGRF_ALARMPVS_MASK ) {
    if ( numAlarmPvs ) {
      alarmPvExpStr.setRaw( alarmPvs[0] );
    }
  }

}

void htmlClass::updateColors (
  double colorValue )
{

int index, change;

  change = 0;

  if ( alarmPvId ) {

    if ( curStatus != alarmPvId->get_status() ) {
      curStatus = alarmPvId->get_status();
      change = 1;
    }

    if ( curSeverity != alarmPvId->get_severity() ) {
      curSeverity = alarmPvId->get_severity();
      change = 1;
    }

  }

  index = actWin->ci->evalRule( fgColor.pixelIndex(),
   colorValue );

  if ( curFgColorIndex != index ) {
    curFgColorIndex = index;
    change = 1;
  }

  index = actWin->ci->evalRule( bgColor.pixelIndex(),
   colorValue );

  if ( curBgColorIndex != index ) {
    curBgColorIndex = index;
    change = 1;
  }

  if ( change ) {

    if ( actWin->ci->isInvisible( curFgColorIndex ) ) {
      fgVisibility = 0;
    }
    else {
      fgVisibility = 1;
    }

    if ( actWin->ci->isInvisible( curBgColorIndex ) ) {
      bgVisibility = 0;
    }
    else {
      bgVisibility = 1;
    }

    fgColor.changeIndex( curFgColorIndex, actWin->ci );
    bgColor.changeIndex( curBgColorIndex, actWin->ci );
    if ( ( prevFgVisibility != fgVisibility ) ||
       ( prevBgVisibility != bgVisibility ) ) {
      prevFgVisibility = fgVisibility;
      prevBgVisibility = bgVisibility;
    }

  }

}

void htmlClass::loadFile (
  char *name
) {

int f, status, absolute;
struct stat buf;
char tmp[MAX_FILENAME_LEN+1], tmp1[MAX_FILENAME_LEN+1], *tk;

  absolute = 0;
  strncpy( tmp1, name, MAX_FILENAME_LEN );
  tmp1[MAX_FILENAME_LEN] = 0;
  tk = strtok( tmp1, " \t\n" );
  if ( tk ) {
    if ( tk[0] == '/' ) {
      absolute = 1;
    }
  }

  if ( !absolute ) {

    strncpy( tmp1, docRoot.getExpanded(), MAX_FILENAME_LEN );
    tmp1[MAX_FILENAME_LEN] = 0;
    tk = strtok( tmp1, " \t\n" );
    if ( tk ) {
      strncpy( tmp, tk, MAX_FILENAME_LEN );
      tmp[MAX_FILENAME_LEN] = 0;
    }
    else {
      strncpy( tmp, "", MAX_FILENAME_LEN );
      tmp[MAX_FILENAME_LEN] = 0;
    }

    //fprintf( stderr, "tmp=[%s]\n", tmp );

    if ( !blank(tmp) ) {
      if ( tmp[strlen(tmp)-1] != '/' ) {
        Strncat( tmp, "/", MAX_FILENAME_LEN );
        tmp[MAX_FILENAME_LEN] = 0;
      }
    }

    Strncat( tmp, name, MAX_FILENAME_LEN );
    tmp[MAX_FILENAME_LEN] = 0;

  }
  else {

    strncpy( tmp, name, MAX_FILENAME_LEN );
    tmp[MAX_FILENAME_LEN] = 0;

  }

  //fprintf( stderr, "name=%s\n", tmp );

  status = stat( tmp, &buf );
  //fprintf( stderr, "status = %-d\n", status );

  //if ( !status ) {
  //  fprintf( stderr, "size=%-ld\n", buf.st_size );
  //}

  f = open( tmp, O_RDONLY, 0777 );
  if ( f != -1 ) {
    if ( fileContents ) delete[] fileContents;
    fileContents = new char[buf.st_size+10];
    read( f, fileContents, buf.st_size );
    fileContents[buf.st_size] = 0;
    close( f );
  }

}

void htmlClass::map ( void ) {

  if ( bulBrd ) {
    XtMapWidget( bulBrd );
    widgetsMapped = 1;
  }

}

void htmlClass::unmap ( void ) {

  if ( bulBrd ) {
    XtUnmapWidget( bulBrd );
    widgetsMapped = 0;
  }

}

void htmlClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 3 ) {
    *n = 0;
    return;
  }

  *n = 3;
  pvs[0] = alarmPvId;
  pvs[1] = visPvId;
  pvs[2] = contentPvId;

}

#ifdef __cplusplus
extern "C" {
#endif

void *create_htmlPtr ( void ) {

htmlClass *ptr;

  ptr = new htmlClass;
  return (void *) ptr;

}

void *clone_htmlPtr (
  void *_srcPtr )
{

htmlClass *ptr, *srcPtr;

  srcPtr = (htmlClass *) _srcPtr;

  ptr = new htmlClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
