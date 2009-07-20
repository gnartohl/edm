//  for object: 2d80926b_bf54_4096_ab21_af7d725f15a2
//  for lib: ?

#define __archivePlot_cc 1

#include <stdio.h>
#include <string.h>

#include "archivePlot.h"
#include "app_pkg.h"
#include "act_win.h"

void _edmDebug ( void );

static void aploUpdateMsg (
  XtPointer client,
  XtIntervalId *id )
{

archivePlotClass *arplo = (archivePlotClass *) client;
int x0, y0;
double dx0, dy0;
char buf[127+1];
SYS_TIME_TYPE sysTime;

  arplo->msgTimer = 0;

  if ( arplo->msgDialogPoppedUp ) {
    arplo->msgDialog.popdown();
  }
  x0 = arplo->timerX - arplo->x;
  y0 = arplo->h - arplo->timerY + arplo->y + 1;
  dx0 = (double) x0 / (double) arplo->w *
   ( arplo->xMax - arplo->xMin ) + arplo->xMin;

  dy0 = (double) y0 / (double) arplo->h *
   ( arplo->yMax - arplo->yMin ) + arplo->yMin;

  if ( arplo->yMode == archivePlotClass::modeLog ) {
    dy0 = pow( 10.0, dy0 );
  }

  if ( arplo->xMode == arplo->modeHours ) {
    sysTime.cal_time = (int) ( dx0 * 3600.0 );
    memcpy( (void *) &sysTime.tm_time,
     (void *) localtime( &sysTime.cal_time ),
     sizeof( struct tm ) );
    sprintf( buf, "(%-d/%02d %-d:%02d:%02d,%-.9g)", sysTime.tm_time.tm_mon+1,
     sysTime.tm_time.tm_mday, sysTime.tm_time.tm_hour,
     sysTime.tm_time.tm_min, sysTime.tm_time.tm_sec, dy0 );
  }
  else {
    sprintf( buf, "(%-.9g,%-.9g)", dx0, dy0 );
  }

  arplo->msgDialog.popup( buf, arplo->timerMsgX+arplo->actWin->x+3,
   arplo->timerMsgY+arplo->actWin->y-25 );
  arplo->msgDialogPoppedUp = 1;

  arplo->actWin->appCtx->proc->lock();
  arplo->bufXMax = dx0;
  arplo->bufXMin = dx0 - 1;
  arplo->needXMarkerDrawCommand = 1;
  arplo->actWin->addDefExeNode( arplo->aglPtr );
  arplo->actWin->appCtx->proc->unlock();

}

static void arploMonitorPvConnectState (
  ProcessVariable *pv,
  void *userArg )
{

archivePlotClass *arplo = (archivePlotClass *) userArg;
char tmp[39+1];

  arplo->actWin->appCtx->proc->lock();

  if ( !arplo->activeMode ) {
    arplo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( pv->is_valid() ) {

    if ( pv == arplo->xMinPv ) {
      arplo->xMinFieldType = pv->get_type().type;
      arplo->bufXMin = pv->get_double();
    }
    else if ( pv == arplo->xMaxPv ) {
      arplo->xMaxFieldType = pv->get_type().type;
      arplo->bufXMax = pv->get_double();
    }
    else if ( pv == arplo->xModePv ) {
      arplo->xModeFieldType = pv->get_type().type;
      arplo->xMode = pv->get_int();
    }
    else if ( pv == arplo->yMinPv ) {
      arplo->yMinFieldType = pv->get_type().type;
      arplo->bufYMin = pv->get_double();
    }
    else if ( pv == arplo->yMaxPv ) {
      arplo->yMaxFieldType = pv->get_type().type;
      arplo->bufYMax = pv->get_double();
    }
    else if ( pv == arplo->yModePv ) {
      arplo->yModeFieldType = pv->get_type().type;
      arplo->yMode = pv->get_int();
    }
    else if ( pv == arplo->colorPv ) {
      arplo->colorFieldType = pv->get_type().type;
      arplo->colorIndex = pv->get_int();
    }
    else if ( pv == arplo->filePv ) {
      arplo->fileFieldType = pv->get_type().type;
      pv->get_string( arplo->file, 39 );
    }
    else if ( pv == arplo->updatePv ) {
      arplo->updateFieldType = pv->get_type().type;
      arplo->update = pv->get_int();
    }
    else if ( pv == arplo->archivePv ) {
      arplo->archiveFieldType = pv->get_type().type;
      pv->get_string( arplo->archiveName, 39 );
    }
    else if ( pv == arplo->startTimePv ) {
      arplo->startTimeFieldType = pv->get_type().type;
      pv->get_string( tmp, 39 );
      /* fprintf( stderr, "start time = %s\n", tmp ); */
    }
    else if ( pv == arplo->endTimePv ) {
      arplo->endTimeFieldType = pv->get_type().type;
      pv->get_string( tmp, 39 );
      /* fprintf( stderr, "end time = %s\n", tmp ); */
    }

  }
  else { // lost connection

    arplo->connection.setPvDisconnected( (void *) pv );
    arplo->active = 0;
    arplo->bgColor.setDisconnected();
    arplo->bufInvalidate();
    arplo->needDraw = 1;
    arplo->actWin->addDefExeNode( arplo->aglPtr );

  }

  arplo->actWin->appCtx->proc->unlock();

}

static void archivePlotUpdate (
  ProcessVariable *pv,
  void *userArg )
{

class archivePlotClass *arplo = (archivePlotClass *) userArg;
char tmp[39+1];
SYS_TIME_TYPE sysTime;
int stat;

  arplo->actWin->appCtx->proc->lock();

  if ( !arplo->activeMode ) {
    arplo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( pv == arplo->xMinPv ) {
    arplo->bufXMin = pv->get_double();
  }
  else if ( pv == arplo->xMaxPv ) {
    arplo->bufXMax = pv->get_double();
  }
  else if ( pv == arplo->xModePv ) {
    arplo->xMode = pv->get_int();
  }
  else if ( pv == arplo->colorPv ) {
    arplo->colorIndex = pv->get_int();
  }
  else if ( pv == arplo->yMinPv ) {
    arplo->bufYMin = pv->get_double();
  }
  else if ( pv == arplo->yMaxPv ) {
    arplo->bufYMax = pv->get_double();
  }
  else if ( pv == arplo->yModePv ) {
    arplo->yMode = pv->get_int();
  }
  else if ( pv == arplo->colorPv ) {
    arplo->colorIndex = pv->get_int();
  }
  else if ( pv == arplo->filePv ) {
    pv->get_string( arplo->file, 39 );
    //arplo->needFileUpdate = 1;
    //arplo->actWin->addDefExeNode( arplo->aglPtr );
  }
  else if ( pv == arplo->updatePv ) {
    arplo->update = pv->get_int();
    if ( ( arplo->update == arplo->graphId+1000 ) ||
         ( arplo->update == arplo->updateAllGraphs ) ||
         ( arplo->update == arplo->updateAll ) ) {
      arplo->needUpdate = 1;
      arplo->actWin->addDefExeNode( arplo->aglPtr );
    }
    else if ( arplo->update == arplo->graphId+31000 ) { // read archive
      arplo->needFileUpdate = 1;
      arplo->actWin->addDefExeNode( arplo->aglPtr );
    }
    else if ( arplo->update == -1 ) { // read archive
      arplo->abortRead = 1;
    }
    else if ( arplo->update == arplo->drawXMarker ) {
      arplo->needXMarkerDraw = 1;
      arplo->actWin->addDefExeNode( arplo->aglPtr );
    }
    else if ( arplo->update == arplo->eraseXMarker ) {
      arplo->needXMarkerErase = 1;
      arplo->actWin->addDefExeNode( arplo->aglPtr );
    }
  }
  else if ( pv == arplo->archivePv ) {
    pv->get_string( arplo->archiveName, 39 );
  }
  else if ( pv == arplo->startTimePv ) {
    pv->get_string( tmp, 39 );
    sysTime.cal_time = 0;
    stat = sys_cvt_string_to_time( tmp, strlen(tmp), &sysTime );

#if 0
    fprintf( stderr, "mon=%-d, day=%-d, yr=%-d, h=%-d, m=%-d, s=%-d\n\n",
	    sysTime.tm_time.tm_mon+1, sysTime.tm_time.tm_mday,
	    sysTime.tm_time.tm_year+1900,
	    sysTime.tm_time.tm_hour, sysTime.tm_time.tm_min,
	    sysTime.tm_time.tm_sec );

    fprintf( stderr, "seconds=%-d\n\n", sysTime.cal_time );
#endif

    {
      if ( stat & 1 ) {
        arplo->start_time_t = sysTime.cal_time;
      }
    }

  }
  else if ( pv == arplo->endTimePv ) {
    pv->get_string( tmp, 39 );
    sysTime.cal_time = 0;
    stat = sys_cvt_string_to_time( tmp, strlen(tmp), &sysTime );

    {
      if ( stat & 1 ) {
        arplo->end_time_t = sysTime.cal_time;
      }
    }

  }
  if ( !arplo->connection.pvsConnected() ) {
    arplo->connection.setPvConnected( (void *) pv );
    if ( arplo->connection.pvsConnected() ) {
      arplo->bgColor.setConnected();
      arplo->needConnectInit = 1;
      arplo->actWin->addDefExeNode( arplo->aglPtr );
    }
  }

  arplo->actWin->appCtx->proc->unlock();

}

static void arplc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

archivePlotClass *arplo = (archivePlotClass *) client;

  arplo->actWin->setChanged();

  arplo->eraseSelectBoxCorners();
  arplo->erase();

  arplo->xMinPvExpStr.setRaw( arplo->eBuf->xMinBufPvName );
  arplo->xMaxPvExpStr.setRaw( arplo->eBuf->xMaxBufPvName );
  arplo->xModePvExpStr.setRaw( arplo->eBuf->xModeBufPvName );
  arplo->yMinPvExpStr.setRaw( arplo->eBuf->yMinBufPvName );
  arplo->yMaxPvExpStr.setRaw( arplo->eBuf->yMaxBufPvName );
  arplo->yModePvExpStr.setRaw( arplo->eBuf->yModeBufPvName );
  arplo->colorPvExpStr.setRaw( arplo->eBuf->colorBufPvName );
  arplo->filePvExpStr.setRaw( arplo->eBuf->fileBufPvName );
  arplo->updatePvExpStr.setRaw( arplo->eBuf->updateBufPvName );
  arplo->archivePvExpStr.setRaw( arplo->eBuf->archiveBufPvName );
  arplo->startTimePvExpStr.setRaw( arplo->eBuf->startTimeBufPvName );
  arplo->endTimePvExpStr.setRaw( arplo->eBuf->endTimeBufPvName );

  arplo->lineColor.setColorIndex( arplo->eBuf->bufLineColor, arplo->actWin->ci );

  arplo->bgColor.setColorIndex( arplo->eBuf->bufBgColor, arplo->actWin->ci );

  arplo->graphId = arplo->eBuf->bufGraphId;

  arplo->x = arplo->bufX;
  arplo->sboxX = arplo->bufX;

  arplo->y = arplo->bufY;
  arplo->sboxY = arplo->bufY;

  arplo->w = arplo->bufW;
  arplo->sboxW = arplo->bufW;

  arplo->h = arplo->bufH;
  arplo->sboxH = arplo->bufH;

  if ( arplo->w < 5 ) {
    arplo->w = 5;
    arplo->sboxW = arplo->w;
  }

  if ( arplo->h < 5 ) {
    arplo->h = 5;
  }

}

static void arplc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

archivePlotClass *arplo = (archivePlotClass *) client;

  arplc_edit_update( w, client, call );
  arplo->refresh( arplo );

}

static void arplc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

archivePlotClass *arplo = (archivePlotClass *) client;

  arplc_edit_update( w, client, call );
  arplo->ef.popdown();
  arplo->operationComplete();

}

static void arplc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

archivePlotClass *arplo = (archivePlotClass *) client;

  arplo->ef.popdown();
  arplo->operationCancel();

}

static void arplc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

archivePlotClass *arplo = (archivePlotClass *) client;

  arplo->ef.popdown();
  arplo->operationCancel();
  arplo->erase();
  arplo->deleteRequest = 1;
  arplo->drawAll();

}

// default constructor
archivePlotClass::archivePlotClass ( void ) {

  name = new char[strlen("2d80926b_bf54_4096_ab21_af7d725f15a2")+1];
  strcpy( name, "2d80926b_bf54_4096_ab21_af7d725f15a2" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  activeMode = 0;
  connection.setMaxPvs( 12 );
  xMin = 0.0;
  xMax = 10.0;
  xMode = modeLinear;
  adj_xMin = xMin;
  adj_xMax = xMax;
  yMin = 0.0;
  yMax = 10.0;
  yMode = modeLinear;
  adj_yMin = yMin;
  adj_yMax = yMax;
  strcpy( file, "" );
  numPoints = 0;
  saveXMin[0] = xMin;
  saveXMax[0] = xMax;
  saveYMin[0] = yMin;
  saveYMax[0] = yMax;
  saveIndex = -1;
  graphId = 0;
  msgDialogPoppedUp = 0;
  eBuf = NULL;

#if 0
  xarray = new double[maxDataPoints];
  yarray = new double[maxDataPoints];
  ixarray = new unsigned int[maxDataPoints];
  iyarray = new unsigned int[maxDataPoints];
#endif

}

// copy constructor
archivePlotClass::archivePlotClass
 ( const archivePlotClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("2d80926b_bf54_4096_ab21_af7d725f15a2")+1];
  strcpy( name, "2d80926b_bf54_4096_ab21_af7d725f15a2" );
  activeMode = 0;
  connection.setMaxPvs( 12 );
  xMin = 0.0;
  xMax = 10.0;
  xMode = modeLinear;
  adj_xMin = xMin;
  adj_xMax = xMax;
  yMin = 0.0;
  yMax = 10.0;
  yMode = modeLinear;
  adj_yMin = yMin;
  adj_yMax = yMax;
  lineColor.copy(source->lineColor);
  bgColor.copy(source->bgColor);
  xMinPvExpStr.setRaw( source->xMinPvExpStr.rawString );
  xMaxPvExpStr.setRaw( source->xMaxPvExpStr.rawString );
  xModePvExpStr.setRaw( source->xModePvExpStr.rawString );
  yMinPvExpStr.setRaw( source->yMinPvExpStr.rawString );
  yMaxPvExpStr.setRaw( source->yMaxPvExpStr.rawString );
  yModePvExpStr.setRaw( source->yModePvExpStr.rawString );
  colorPvExpStr.setRaw( source->colorPvExpStr.rawString );
  filePvExpStr.setRaw( source->filePvExpStr.rawString );
  updatePvExpStr.setRaw( source->updatePvExpStr.rawString );
  archivePvExpStr.setRaw( source->archivePvExpStr.rawString );
  startTimePvExpStr.setRaw( source->startTimePvExpStr.rawString );
  endTimePvExpStr.setRaw( source->endTimePvExpStr.rawString );
  graphId = source->graphId;
  msgDialogPoppedUp = 0;

  numPoints = 0;
  saveXMin[0] = xMin;
  saveXMax[0] = xMax;
  saveYMin[0] = yMin;
  saveYMax[0] = yMax;
  saveIndex = -1;
  eBuf = NULL;

#if 0
  xarray = new double[maxDataPoints];
  yarray = new double[maxDataPoints];
  ixarray = new unsigned int[maxDataPoints];
  iyarray = new unsigned int[maxDataPoints];
#endif

}

archivePlotClass::~archivePlotClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

#if 0
  delete xarray;
  xarray = NULL;
  delete yarray;
  yarray = NULL;
  delete ixarray;
  ixarray = NULL;
  delete iyarray;
  iyarray = NULL;
#endif

}

char *archivePlotClass::objName ( void ) {

  return name;

}

int archivePlotClass::createInteractive (
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

  lineColor.setColorIndex( actWin->defaultFg1Color, actWin->ci );

  bgColor.setColorIndex( actWin->defaultBgColor, actWin->ci );

  this->draw();

  this->editCreate();

  return 1;

}

int archivePlotClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "2d80926b_bf54_4096_ab21_af7d725f15a2" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, archivePlot_str2, 31 );

  Strncat( title, archivePlot_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  eBuf->bufLineColor = lineColor.pixelIndex();

  eBuf->bufBgColor = bgColor.pixelIndex();

  if ( xMinPvExpStr.getRaw() )
    strncpy( eBuf->xMinBufPvName, xMinPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->xMinBufPvName, "" );

 if ( xMaxPvExpStr.getRaw() )
   strncpy( eBuf->xMaxBufPvName, xMaxPvExpStr.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->xMaxBufPvName, "" );

 if ( xModePvExpStr.getRaw() )
   strncpy( eBuf->xModeBufPvName, xModePvExpStr.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->xModeBufPvName, "" );

  if ( yMinPvExpStr.getRaw() )
    strncpy( eBuf->yMinBufPvName, yMinPvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->yMinBufPvName, "" );

 if ( yMaxPvExpStr.getRaw() )
   strncpy( eBuf->yMaxBufPvName, yMaxPvExpStr.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->yMaxBufPvName, "" );

 if ( yModePvExpStr.getRaw() )
   strncpy( eBuf->yModeBufPvName, yModePvExpStr.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->yModeBufPvName, "" );

 if ( colorPvExpStr.getRaw() )
   strncpy( eBuf->colorBufPvName, colorPvExpStr.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->colorBufPvName, "" );

 if ( filePvExpStr.getRaw() )
   strncpy( eBuf->fileBufPvName, filePvExpStr.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->fileBufPvName, "" );

 if ( updatePvExpStr.getRaw() )
   strncpy( eBuf->updateBufPvName, updatePvExpStr.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->updateBufPvName, "" );

  if ( archivePvExpStr.getRaw() )
    strncpy( eBuf->archiveBufPvName, archivePvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->archiveBufPvName, "" );

  if ( startTimePvExpStr.getRaw() )
    strncpy( eBuf->startTimeBufPvName, startTimePvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->startTimeBufPvName, "" );

  if ( endTimePvExpStr.getRaw() )
    strncpy( eBuf->endTimeBufPvName, endTimePvExpStr.getRaw(),
     PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->endTimeBufPvName, "" );

  eBuf->bufGraphId = graphId;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( archivePlot_str4, 30, &bufX );

  ef.addTextField( archivePlot_str5, 30, &bufY );
  ef.addTextField( archivePlot_str6, 30, &bufW );
  ef.addTextField( archivePlot_str7, 30, &bufH );

  ef.addTextField( archivePlot_str19, 30, &eBuf->bufGraphId );

  ef.addTextField( archivePlot_str20, 30, eBuf->archiveBufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( archivePlot_str21, 30, eBuf->startTimeBufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( archivePlot_str22, 30, eBuf->endTimeBufPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( archivePlot_str15, 30, eBuf->fileBufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( archivePlot_str10, 30, eBuf->xModeBufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( archivePlot_str8, 30, eBuf->xMinBufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( archivePlot_str9, 30, eBuf->xMaxBufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( archivePlot_str13, 30, eBuf->yModeBufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( archivePlot_str11, 30, eBuf->yMinBufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( archivePlot_str12, 30, eBuf->yMaxBufPvName,
   PV_Factory::MAX_PV_NAME );

  ef.addTextField( archivePlot_str14, 30, eBuf->colorBufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( archivePlot_str16, 30, eBuf->updateBufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addColorButton( archivePlot_str17, actWin->ci, &eBuf->lineCb,
   &eBuf->bufLineColor );
  ef.addColorButton( archivePlot_str18, actWin->ci, &eBuf->bgCb,
   &eBuf->bufBgColor );

  return 1;

}

int archivePlotClass::editCreate ( void ) {

  this->genericEdit();

  ef.finished( arplc_edit_ok, arplc_edit_apply, arplc_edit_cancel_delete, this );

  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int archivePlotClass::edit ( void ) {

  this->genericEdit();
  ef.finished( arplc_edit_ok, arplc_edit_apply, arplc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int archivePlotClass::createFromFile (
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
  tag.loadR( "id", &graphId, &zero );
  tag.loadR( "fgColor", actWin->ci, &lineColor );
  tag.loadR( "bgColor", actWin->ci, &bgColor );
  tag.loadR( "filePv", &filePvExpStr, emptyStr );
  tag.loadR( "xMinPv", &xMinPvExpStr, emptyStr );
  tag.loadR( "xMaxPv", &xMaxPvExpStr, emptyStr );
  tag.loadR( "xModePv", &xModePvExpStr, emptyStr );
  tag.loadR( "yMinPv", &yMinPvExpStr, emptyStr );
  tag.loadR( "yMaxPv", &yMaxPvExpStr, emptyStr );
  tag.loadR( "yModePv", &yModePvExpStr, emptyStr );
  tag.loadR( "colorPv", &colorPvExpStr, emptyStr );
  tag.loadR( "updatePv", &updatePvExpStr, emptyStr );
  tag.loadR( "archivePv", &archivePvExpStr, emptyStr );
  tag.loadR( "startTimePv", &startTimePvExpStr, emptyStr );
  tag.loadR( "endTimePv", &endTimePvExpStr, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > ARPLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  return stat;

}

int archivePlotClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int index;
int major, minor, release;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > ARPLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 2 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &index ); actWin->incLine();
    bgColor.setColorIndex( index, actWin->ci );

  }

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  filePvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  xMinPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  xMaxPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  xModePvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  yMinPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  yMaxPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  yModePvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  colorPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  updatePvExpStr.setRaw( oneName );

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 0 ) ) ) {

    readStringFromFile( oneName, 39+1, f ); actWin->incLine();
    archivePvExpStr.setRaw( oneName );

    readStringFromFile( oneName, 39+1, f ); actWin->incLine();
    startTimePvExpStr.setRaw( oneName );

    readStringFromFile( oneName, 39+1, f ); actWin->incLine();
    endTimePvExpStr.setRaw( oneName );

  }
  else {

    archivePvExpStr.setRaw( "" );

    startTimePvExpStr.setRaw( "" );

    endTimePvExpStr.setRaw( "" );

  }

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 1 ) ) ) {
    fscanf( f, "%d\n", &graphId ); actWin->incLine();
  }
  else {
    graphId = 0;
  }

  return 1;

}

int archivePlotClass::save (
  FILE *f )
{

int stat, major, minor, release;
tagClass tag;
int zero = 0;
char *emptyStr = "";

  major = ARPLC_MAJOR_VERSION;
  minor = ARPLC_MINOR_VERSION;
  release = ARPLC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "id", &graphId, &zero );
  tag.loadW( "fgColor", actWin->ci, &lineColor );
  tag.loadW( "bgColor", actWin->ci, &bgColor );
  tag.loadW( "filePv", &filePvExpStr, emptyStr );
  tag.loadW( "xMinPv", &xMinPvExpStr, emptyStr );
  tag.loadW( "xMaxPv", &xMaxPvExpStr, emptyStr );
  tag.loadW( "xModePv", &xModePvExpStr, emptyStr );
  tag.loadW( "yMinPv", &yMinPvExpStr, emptyStr );
  tag.loadW( "yMaxPv", &yMaxPvExpStr, emptyStr );
  tag.loadW( "yModePv", &yModePvExpStr, emptyStr );
  tag.loadW( "colorPv", &colorPvExpStr, emptyStr );
  tag.loadW( "updatePv", &updatePvExpStr, emptyStr );
  tag.loadW( "archivePv", &archivePvExpStr, emptyStr );
  tag.loadW( "startTimePv", &startTimePvExpStr, emptyStr );
  tag.loadW( "endTimePv", &endTimePvExpStr, emptyStr );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int archivePlotClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", ARPLC_MAJOR_VERSION, ARPLC_MINOR_VERSION,
   ARPLC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = lineColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  index = bgColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  if ( filePvExpStr.getRaw() )
    writeStringToFile( f, filePvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( xMinPvExpStr.getRaw() )
    writeStringToFile( f, xMinPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( xMaxPvExpStr.getRaw() )
    writeStringToFile( f, xMaxPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( xModePvExpStr.getRaw() )
    writeStringToFile( f,  xModePvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( yMinPvExpStr.getRaw() )
    writeStringToFile( f, yMinPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( yMaxPvExpStr.getRaw() )
    writeStringToFile( f, yMaxPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( yModePvExpStr.getRaw() )
    writeStringToFile( f,  yModePvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( colorPvExpStr.getRaw() )
    writeStringToFile( f, colorPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( updatePvExpStr.getRaw() )
    writeStringToFile( f, updatePvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1.1
  if ( archivePvExpStr.getRaw() )
    writeStringToFile( f, archivePvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( startTimePvExpStr.getRaw() )
    writeStringToFile( f, startTimePvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( endTimePvExpStr.getRaw() )
    writeStringToFile( f, endTimePvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  // version 1.2
  fprintf( f, "%-d\n", graphId );

  return 1;

}

int archivePlotClass::drawActive ( void ) {

int i;
unsigned int _x0, _y0;
XRectangle xR = { x, y, w, h };
int clipStat;

  if ( !enabled || !activeMode || !init ) return 1;

  /* fprintf( stderr, "apc drawActive\n" ); */

  rescale();

  actWin->executeGc.saveFg();

  // bg
  actWin->executeGc.setFG( bgColor.pixelColor() );
  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );
  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h );

  actWin->executeGc.setFG( lineColor.getColor() );
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  clipStat = actWin->executeGc.addNormXClipRectangle( xR );

  i = 0;
  _x0 = ixarray[i];
  _y0 = iyarray[i];

  if ( yType != DBR_TIME_ENUM ) {

    for ( i=1; i<numPoints; i++ ) {
      if ( ( _x0 <= 0xffff ) &&
           ( _y0 <= 0xffff ) &&
           ( ixarray[i] <= 0xffff ) &&
           ( iyarray[i] <= 0xffff ) ) {
        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), _x0, _y0,
         ixarray[i], iyarray[i] );
      }
      _x0 = ixarray[i];
      _y0 = iyarray[i]; 
    }

  }
  else {

    for ( i=1; i<numPoints; i++ ) {
      if ( ( _x0 <= 0xffff ) &&
           ( _y0 <= 0xffff ) &&
           ( ixarray[i] <= 0xffff ) &&
           ( iyarray[i] <= 0xffff ) ) {
        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), _x0, _y0,
         ixarray[i], _y0 );
        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.normGC(), ixarray[i], _y0,
         ixarray[i], iyarray[i] );
      }
      _x0 = ixarray[i];
      _y0 = iyarray[i]; 
    }

  }

  if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();

  actWin->executeGc.restoreFg();

  return 1;

}

int archivePlotClass::eraseActive ( void ) {

  if ( !enabled || !activeMode || !init ) return 1;

  /* fprintf( stderr, "eraseActive\n" ); */

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  return 1;

}

void archivePlotClass::bufInvalidate ( void )
{

  bufferInvalid = 1;

}

int archivePlotClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( xMinPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  xMinPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( xMaxPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  xMaxPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( xModePvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  xModePvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( yMinPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  yMinPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( yMaxPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  yMaxPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( yModePvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  yModePvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( colorPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  colorPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( filePvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  filePvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( updatePvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  updatePvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( archivePvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  archivePvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( startTimePvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  startTimePvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( endTimePvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  endTimePvExpStr.setRaw( tmpStr.getExpanded() );

  return 1;

}

int archivePlotClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = xMinPvExpStr.expand1st( numMacros, macros, expansions );
  stat = xMaxPvExpStr.expand1st( numMacros, macros, expansions );
  stat = xModePvExpStr.expand1st( numMacros, macros, expansions );
  stat = yMinPvExpStr.expand1st( numMacros, macros, expansions );
  stat = yMaxPvExpStr.expand1st( numMacros, macros, expansions );
  stat = yModePvExpStr.expand1st( numMacros, macros, expansions );
  stat = colorPvExpStr.expand1st( numMacros, macros, expansions );
  stat = filePvExpStr.expand1st( numMacros, macros, expansions );
  stat = updatePvExpStr.expand1st( numMacros, macros, expansions );
  stat = archivePvExpStr.expand1st( numMacros, macros, expansions );
  stat = startTimePvExpStr.expand1st( numMacros, macros, expansions );
  stat = endTimePvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int archivePlotClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = xMinPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = xMaxPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = xModePvExpStr.expand2nd( numMacros, macros, expansions );
  stat = yMinPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = yMaxPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = yModePvExpStr.expand2nd( numMacros, macros, expansions );
  stat = colorPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = filePvExpStr.expand2nd( numMacros, macros, expansions );
  stat = updatePvExpStr.expand2nd( numMacros, macros, expansions );
  stat = archivePvExpStr.expand2nd( numMacros, macros, expansions );
  stat = startTimePvExpStr.expand2nd( numMacros, macros, expansions );
  stat = endTimePvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int archivePlotClass::containsMacros ( void ) {

  if ( xMinPvExpStr.containsPrimaryMacros() ) return 1;
  if ( xMaxPvExpStr.containsPrimaryMacros() ) return 1;
  if ( xModePvExpStr.containsPrimaryMacros() ) return 1;
  if ( yMinPvExpStr.containsPrimaryMacros() ) return 1;
  if ( yMaxPvExpStr.containsPrimaryMacros() ) return 1;
  if ( yModePvExpStr.containsPrimaryMacros() ) return 1;
  if ( colorPvExpStr.containsPrimaryMacros() ) return 1;
  if ( filePvExpStr.containsPrimaryMacros() ) return 1;
  if ( updatePvExpStr.containsPrimaryMacros() ) return 1;
  if ( archivePvExpStr.containsPrimaryMacros() ) return 1;
  if ( startTimePvExpStr.containsPrimaryMacros() ) return 1;
  if ( endTimePvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int archivePlotClass::activate (
  int pass,
  void *ptr )
{

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2: // connect to pv

    if ( !opComplete ) {

      msgDialog.create( actWin->executeWidgetId() );
      msgDialogPoppedUp = 0;
      msgTimer = 0;

      markerDrawn = 0;

      connection.init();

      initEnable();

      drawRegion = 0;
      btn1Down = 0;

      bufX = x;
      bufY = y;
      bufW = w;
      bufH = h;

      if ( xMax <= xMin ) xMax = xMin + 1.0;
      adj_xMin = xMin;
      adj_xMax = xMax;

      if ( yMax <= yMin ) yMax = yMin + 1.0;
      adj_yMin = yMin;
      adj_yMax = yMax;

      aglPtr = ptr;
      needConnectInit = needFileUpdate = needUpdate = needDraw = needErase =
       needXMarkerDraw = needXMarkerErase = needXMarkerDrawCommand =
       needXMarkerEraseCommand = 0;
      init = 0;
      active = 0;
      activeMode = 1;
      bufferInvalid = 1;
      pvsExist = 1;
      abortRead = 0;

      xMinPv = xMaxPv = xModePv = yMinPv = yMaxPv = yModePv =
       colorPv = filePv = updatePv = NULL;

      if ( !xMinPvExpStr.getExpanded() ||
         // ( strcmp( xMinPvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( xMinPvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !xMaxPvExpStr.getExpanded() ||
         // ( strcmp( xMaxPvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( xMaxPvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !xModePvExpStr.getExpanded() ||
         // ( strcmp( xModePvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( xModePvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !yMinPvExpStr.getExpanded() ||
         // ( strcmp( yMinPvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( yMinPvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !yMaxPvExpStr.getExpanded() ||
         // ( strcmp( yMaxPvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( yMaxPvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !yModePvExpStr.getExpanded() ||
         // ( strcmp( yModePvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( yModePvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !colorPvExpStr.getExpanded() ||
         // ( strcmp( colorPvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( colorPvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !filePvExpStr.getExpanded() ||
         // ( strcmp( filePvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( filePvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !updatePvExpStr.getExpanded() ||
         // ( strcmp( updatePvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( updatePvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !archivePvExpStr.getExpanded() ||
         // ( strcmp( archivePvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( archivePvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !startTimePvExpStr.getExpanded() ||
         // ( strcmp( startTimePvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( startTimePvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !endTimePvExpStr.getExpanded() ||
         // ( strcmp( endTimePvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( endTimePvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !pvsExist ) { // disable active mode behavior
        activeMode = 0;
        opComplete = 1;
        break;
      }

      bgColor.setConnectSensitive();

      xMinPv = the_PV_Factory->create( xMinPvExpStr.getExpanded() );
      if ( xMinPv ) {
        if ( xMinPv->is_valid() ) {
          arploMonitorPvConnectState( xMinPv, this );
          archivePlotUpdate( xMinPv, this );
	}
        xMinPv->add_conn_state_callback( arploMonitorPvConnectState, this );
        xMinPv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      xMaxPv = the_PV_Factory->create( xMaxPvExpStr.getExpanded() );
      if ( xMaxPv ) {
        if ( xMaxPv->is_valid() ) {
          arploMonitorPvConnectState( xMaxPv, this );
          archivePlotUpdate( xMaxPv, this );
	}
        xMaxPv->add_conn_state_callback( arploMonitorPvConnectState, this );
        xMaxPv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      xModePv = the_PV_Factory->create( xModePvExpStr.getExpanded() );
      if ( xModePv ) {
        if ( xModePv->is_valid() ) {
          arploMonitorPvConnectState( xModePv, this );
          archivePlotUpdate( xModePv, this );
	}
        xModePv->add_conn_state_callback( arploMonitorPvConnectState, this );
        xModePv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      yMinPv = the_PV_Factory->create( yMinPvExpStr.getExpanded() );
      if ( yMinPv ) {
        if ( yMinPv->is_valid() ) {
          arploMonitorPvConnectState( yMinPv, this );
          archivePlotUpdate( yMinPv, this );
	}
        yMinPv->add_conn_state_callback( arploMonitorPvConnectState, this );
        yMinPv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      yMaxPv = the_PV_Factory->create( yMaxPvExpStr.getExpanded() );
      if ( yMaxPv ) {
        if ( yMaxPv->is_valid() ) {
          arploMonitorPvConnectState( yMaxPv, this );
          archivePlotUpdate( yMaxPv, this );
	}
        yMaxPv->add_conn_state_callback( arploMonitorPvConnectState, this );
        yMaxPv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      yModePv = the_PV_Factory->create( yModePvExpStr.getExpanded() );
      if ( yModePv ) {
        if ( yModePv->is_valid() ) {
          arploMonitorPvConnectState( yModePv, this );
          archivePlotUpdate( yModePv, this );
	}
        yModePv->add_conn_state_callback( arploMonitorPvConnectState, this );
        yModePv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      colorPv = the_PV_Factory->create( colorPvExpStr.getExpanded() );
      if ( colorPv ) {
        if ( colorPv->is_valid() ) {
          arploMonitorPvConnectState( colorPv, this );
          archivePlotUpdate( colorPv, this );
	}
        colorPv->add_conn_state_callback( arploMonitorPvConnectState, this );
        colorPv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      filePv = the_PV_Factory->create( filePvExpStr.getExpanded() );
      if ( filePv ) {
        if ( filePv->is_valid() ) {
          arploMonitorPvConnectState( filePv, this );
          archivePlotUpdate( filePv, this );
	}
        filePv->add_conn_state_callback( arploMonitorPvConnectState, this );
        filePv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      updatePv = the_PV_Factory->create( updatePvExpStr.getExpanded() );
      if ( updatePv ) {
        if ( updatePv->is_valid() ) {
          arploMonitorPvConnectState( updatePv, this );
          archivePlotUpdate( updatePv, this );
	}
        updatePv->add_conn_state_callback( arploMonitorPvConnectState, this );
        updatePv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      archivePv = the_PV_Factory->create( archivePvExpStr.getExpanded() );
      if ( archivePv ) {
        if ( archivePv->is_valid() ) {
          arploMonitorPvConnectState( archivePv, this );
          archivePlotUpdate( archivePv, this );
	}
        archivePv->add_conn_state_callback( arploMonitorPvConnectState, this );
        archivePv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      startTimePv = the_PV_Factory->create( startTimePvExpStr.getExpanded() );
      if ( startTimePv ) {
        if ( startTimePv->is_valid() ) {
          arploMonitorPvConnectState( startTimePv, this );
          archivePlotUpdate( startTimePv, this );
	}
        startTimePv->add_conn_state_callback( arploMonitorPvConnectState,
         this );
        startTimePv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      endTimePv = the_PV_Factory->create( endTimePvExpStr.getExpanded() );
      if ( endTimePv ) {
        if ( endTimePv->is_valid() ) {
          arploMonitorPvConnectState( endTimePv, this );
          archivePlotUpdate( endTimePv, this );
	}
        endTimePv->add_conn_state_callback( arploMonitorPvConnectState, this );
        endTimePv->add_value_callback( archivePlotUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

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

int archivePlotClass::deactivate (
  int pass )
{

  if ( pass == 1 ) {

    activeMode = 0;

    msgDialog.destroy(); 
    msgTimer = 0;

    x = bufX;
    y = bufY;
    w = bufW;
    h = bufH;

    if( xMinPv ) {
      xMinPv->remove_conn_state_callback( arploMonitorPvConnectState, this );
      xMinPv->remove_value_callback( archivePlotUpdate, this );
      xMinPv->release();
      xMinPv = NULL;
    }

    if( xMaxPv ) {
      xMaxPv->remove_conn_state_callback( arploMonitorPvConnectState, this );
      xMaxPv->remove_value_callback( archivePlotUpdate, this );
      xMaxPv->release();
      xMaxPv = NULL;
    }

    if( xModePv ) {
      xModePv->remove_conn_state_callback( arploMonitorPvConnectState, this );
      xModePv->remove_value_callback( archivePlotUpdate, this );
      xModePv->release();
      xModePv = NULL;
    }

    if( yMinPv ) {
      yMinPv->remove_conn_state_callback( arploMonitorPvConnectState, this );
      yMinPv->remove_value_callback( archivePlotUpdate, this );
      yMinPv->release();
      yMinPv = NULL;
    }

    if( yMaxPv ) {
      yMaxPv->remove_conn_state_callback( arploMonitorPvConnectState, this );
      yMaxPv->remove_value_callback( archivePlotUpdate, this );
      yMaxPv->release();
      yMaxPv = NULL;
    }

    if( yModePv ) {
      yModePv->remove_conn_state_callback( arploMonitorPvConnectState, this );
      yModePv->remove_value_callback( archivePlotUpdate, this );
      yModePv->release();
      yModePv = NULL;
    }

    if( colorPv ) {
      colorPv->remove_conn_state_callback( arploMonitorPvConnectState, this );
      colorPv->remove_value_callback( archivePlotUpdate, this );
      colorPv->release();
      colorPv = NULL;
    }

    if( filePv ) {
      filePv->remove_conn_state_callback( arploMonitorPvConnectState, this );
      filePv->remove_value_callback( archivePlotUpdate, this );
      filePv->release();
      filePv = NULL;
    }

    if( updatePv ) {
      updatePv->remove_conn_state_callback( arploMonitorPvConnectState, this );
      updatePv->remove_value_callback( archivePlotUpdate, this );
      updatePv->release();
      updatePv = NULL;
    }

    if( archivePv ) {
      archivePv->remove_conn_state_callback( arploMonitorPvConnectState,
       this );
      archivePv->remove_value_callback( archivePlotUpdate, this );
      archivePv->release();
      archivePv = NULL;
    }

    if( startTimePv ) {
      startTimePv->remove_conn_state_callback( arploMonitorPvConnectState,
       this );
      startTimePv->remove_value_callback( archivePlotUpdate, this );
      startTimePv->release();
      startTimePv = NULL;
    }

    if( endTimePv ) {
      endTimePv->remove_conn_state_callback( arploMonitorPvConnectState,
       this );
      endTimePv->remove_value_callback( archivePlotUpdate, this );
      endTimePv->release();
      endTimePv = NULL;
    }

  }

  return 1;

}

int archivePlotClass::draw ( void ) {

  if ( activeMode || deleteRequest ) return 1;

  /* fprintf( stderr, "draw\n" ); */

  actWin->drawGc.saveFg();

  // bg
  actWin->drawGc.setFG( bgColor.pixelColor() );
  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, h );

  // data
  actWin->drawGc.setFG( lineColor.pixelColor() );
  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  actWin->drawGc.restoreFg();

  return 1;

}

int archivePlotClass::erase ( void ) {

  if ( activeMode || deleteRequest ) return 1;

  /* fprintf( stderr, "erase\n" ); */

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int archivePlotClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  ret_stat = 1;

  tmpw = sboxW;
  tmph = sboxH;

  tmpw += _w;
  tmph += _h;

  if ( tmph < 5 ) ret_stat = 0;

  if ( tmpw < 5 ) ret_stat = 0;

  return ret_stat;

}

int archivePlotClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, ret_stat;

  ret_stat = 1;

  tmpw = _w;
  tmph = _h;

  if ( tmph != -1 ) {
    if ( tmph < 5 ) ret_stat = 0;
  }

  if ( tmpw != -1 ) {
    if ( tmpw < 5 ) ret_stat = 0;
  }

  return ret_stat;

}

void archivePlotClass::updateDimensions ( void ) {

  if ( h < 5 ) {
    h = 5;
    sboxH = h;
  }

  if ( w < 5 ) {
    w = 5;
    sboxW = 5;
  }

}

void archivePlotClass::btnUp (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

double dx0, dy0, dx1, dy1;

  *action = 0;

  if ( !enabled ) return;

  if ( msgTimer ) {
    XtRemoveTimeOut( msgTimer );
    msgTimer = 0;
  }

  if ( msgDialogPoppedUp ) {
    msgDialog.popdown();
    msgDialogPoppedUp = 0;
    needXMarkerEraseCommand = 1;
    actWin->addDefExeNode( aglPtr );
    return;
  }

  if ( ( buttonNumber != 1 ) || ( buttonState & ShiftMask ) ) return;

  if ( !btn1Down ) return;

  btn1Down = 0;

  //fprintf( stderr, "btnUp\n" );

  drawRegion = 0;

  selectX1 = be->x - x;
  selectY1 = h - be->y + y;

  if ( ( selectX0 == selectX1 ) && ( selectY0 == selectY1 ) ) {
    if ( saveIndex <= 0 ) {
      saveIndex = 0;
      XBell( actWin->d, 50 );
    }
    else {
      saveIndex--;
    }
    /* fprintf( stderr, "saveIndex = %-d\n", saveIndex ); */
    /* fprintf( stderr, "saveXMin = %-g\n", saveXMin[saveIndex] ); */
    /* fprintf( stderr, "saveXMax = %-g\n", saveXMax[saveIndex] ); */
    /* fprintf( stderr, "saveYMin = %-g\n", saveYMin[saveIndex] ); */
    /* fprintf( stderr, "saveYMax = %-g\n\n", saveYMax[saveIndex] ); */
    if ( connection.pvsConnected() ) {
      xMinPv->put( saveXMin[saveIndex] );
      xMaxPv->put( saveXMax[saveIndex] );
      yMinPv->put( saveYMin[saveIndex] );
      yMaxPv->put( saveYMax[saveIndex] );
      xModePv->put( modeHours );
      updatePv->put( graphId );
    }
    return;
  }

  // erase old

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( lineColor.pixelColor() );
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.xorGC(), x+xx0, y+h-yy1, xx1-xx0, yy1-yy0 );

  actWin->executeGc.restoreFg();

  if ( buttonState & ControlMask ) return; // abort on ctrl-key release

  x0 = selectX0;
  x1 = selectX1;
  if ( x0 > x1 ) {
    x0 = selectX1;
    x1 = selectX0;
  }
  if ( x0 < 0 ) x0 = 0;
  if ( x0 > w ) x0 = w;
  if ( x1 < 0 ) x1 = 0;
  if ( x1 > w ) x1 = w;
  if ( x0 == x1 ) x1 = x0 + 1;

  y0 = selectY0;
  y1 = selectY1;
  if ( y0 > y1 ) {
    y0 = selectY1;
    y1 = selectY0;
  }
  if ( y0 < 0 ) y0 = 0;
  if ( y0 > h ) y0 = h;
  if ( y1 < 0 ) y1 = 0;
  if ( y1 > h ) y1 = h;
  if ( y0 == y1 ) y1 = y0 + 1;

  /* fprintf( stderr, "%-d,%-d      %-d,%-d\n", x0, y0, x1, y1 ); */

  dx0 = (double) x0 / (double) w * ( xMax - xMin ) + xMin;
  dy0 = (double) y0 / (double) h * ( yMax - yMin ) + yMin;

  dx1 = (double) x1 / (double) w * ( xMax - xMin ) + xMin;
  dy1 = (double) y1 / (double) h * ( yMax - yMin ) + yMin;

  /* fprintf( stderr, "%-g,%-g      %-g,%-g\n\n", dx0, dy0, dx1, dy1 ); */

  xMin = dx0;
  xMax = dx1;
  yMin = dy0;
  yMax = dy1;

  saveIndex++;
  if ( saveIndex >= saveStackSize ) {
    saveIndex = saveStackSize-1;
    XBell( actWin->d, 50 );
    return;
  }
  saveXMin[saveIndex] = xMin;
  saveXMax[saveIndex] = xMax;
  saveYMin[saveIndex] = yMin;
  saveYMax[saveIndex] = yMax;

  if ( connection.pvsConnected() ) {
    xMinPv->put( xMin );
    xMaxPv->put( xMax );
    yMinPv->put( yMin );
    yMaxPv->put( yMax );
    xModePv->put( modeHours );
    updatePv->put( graphId );
  }

}

void archivePlotClass::btnDown (
  XButtonEvent *be,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( ( buttonNumber == 1 ) &&
      ( buttonState & ShiftMask ) &&
      !( buttonState & ControlMask ) ) {
    if ( msgTimer ) {
      XtRemoveTimeOut( msgTimer );
      msgTimer = 0;
    }
    timerX = be->x;
    timerY = be->y;
    timerMsgX = _x;
    timerMsgY = _y;
    msgTimer = appAddTimeOut( actWin->appCtx->appContext(), 250,
     aploUpdateMsg, this );
    return;
  }

  if ( msgDialogPoppedUp ) {
    msgDialog.popdown();
    msgDialogPoppedUp = 0;
    needXMarkerEraseCommand = 1;
    actWin->addDefExeNode( aglPtr );
    return;
  }

  if ( msgTimer ) {
    XtRemoveTimeOut( msgTimer );
    msgTimer = 0;
  }

  if ( buttonNumber == 3 ) {
    saveIndex = 0;
    if ( connection.pvsConnected() ) {
      xMinPv->put( saveXMin[saveIndex] );
      xMaxPv->put( saveXMax[saveIndex] );
      yMinPv->put( saveYMin[saveIndex] );
      yMaxPv->put( saveYMax[saveIndex] );
      xModePv->put( modeHours );
      updatePv->put( graphId );
    }
    return;
  }

  if ( ( buttonNumber != 1 ) || ( buttonState & ShiftMask ) ) return;

  //fprintf( stderr, "btnDown\n" );

  btn1Down = 1;

  drawRegion = 1;

  selectX0 = be->x - x;
  selectY0 = h - be->y + y;

}


void archivePlotClass::btnDrag (
  XMotionEvent *me,
  int _x,
  int _y,
  int buttonState,
  int buttonNumber )
{

  if ( !enabled ) return;

  if ( ( buttonNumber == 1 ) &&
      ( buttonState & ShiftMask ) &&
      !( buttonState & ControlMask ) ) {
    if ( msgTimer ) {
      XtRemoveTimeOut( msgTimer );
      msgTimer = 0;
    }
    timerX = me->x;
    timerY = me->y;
    timerMsgX = _x;
    timerMsgY = _y;
    msgTimer = appAddTimeOut( actWin->appCtx->appContext(), 250,
     aploUpdateMsg, this );
    return;
  }

  if ( ( buttonNumber != 1 ) || ( buttonState & ShiftMask ) ) return;

  if ( !btn1Down ) return;

  //fprintf( stderr, "btnDrag\n" );

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( lineColor.pixelColor() );
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  if ( drawRegion > 1 ) {

    // erase old

    XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.xorGC(), x+xx0, y+h-yy1, xx1-xx0, yy1-yy0 );

  }

  drawRegion = 2;

  x0 = selectX0;
  x1 = me->x - x;

  y0 = selectY0;
  y1 = h - me->y + y;

  // draw new

  xx0 = x0;
  xx1 = x1;
  if ( xx0 > xx1 ) {
    xx0 = x1;
    xx1 = x0;
  }

  yy0 = y0;
  yy1 = y1;
  if ( yy0 > yy1 ) {
    yy0 = y1;
    yy1 = y0;
  }

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.xorGC(), x+xx0, y+h-yy1, xx1-xx0, yy1-yy0 );

  actWin->executeGc.restoreFg();

}

int archivePlotClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 1;
  *down = 1;
  *up = 1;
  *focus = 0;

  return 1;

}

void archivePlotClass::executeDeferred ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;

int nc, nfu, nu, nd, ne, nmd, nmdc, nme, nmec;
int ix, iy0, iy1;
double xRange;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  nc = needConnectInit; needConnectInit = 0;
  nfu = needFileUpdate; needFileUpdate = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  ne = needErase; needErase = 0;
  nmd = needXMarkerDraw; needXMarkerDraw = 0;
  nmdc = needXMarkerDrawCommand; needXMarkerDrawCommand = 0;
  nme = needXMarkerErase; needXMarkerErase = 0;
  nmec = needXMarkerEraseCommand; needXMarkerEraseCommand = 0;
  actWin->remDefExeNode( aglPtr );

  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;


//----------------------------------------------------------------------------

  if ( nc ) {

    /* fprintf( stderr, "needConnectInit\n" ); */

    init = 1; // OK to draw in active state now

    rescale();

    drawActive();

  }


//----------------------------------------------------------------------------

  if ( nfu ) {

    // fprintf( stderr, "needFileUpdate\n" );

    readFile();
    readArchive();

    if ( connection.pvsConnected() ) {
      xMinPv->put( xMin );
      xMaxPv->put( xMax );
      yMinPv->put( yMin );
      yMaxPv->put( yMax );
      xModePv->put( modeHours );
      updatePv->put( graphId );
    }

  }


//----------------------------------------------------------------------------

  if ( nu ) {

    xMin = bufXMin;
    xMax = bufXMax;
    yMin = bufYMin;
    yMax = bufYMax;

    /* fprintf( stderr, "needUpdate\n" ); */
    /* fprintf( stderr, "xMin = %-g\n", xMin ); */
    /* fprintf( stderr, "xMax = %-g\n", xMax ); */
    /* fprintf( stderr, "yMin = %-g\n", yMin ); */
    /* fprintf( stderr, "yMax = %-g\n", yMax ); */

    eraseActive();

    if ( xMax <= xMin ) xMax = xMin + 1.0;

    if ( yMax <= yMin ) yMax = yMin + 1.0;

    rescale();

    if ( connection.pvsConnected() ) {
      updatePv->put( 0 );
    }

    drawActive();

  }


//----------------------------------------------------------------------------

  if ( nd ) {

    drawActive();

  }


//----------------------------------------------------------------------------

  if ( nmdc ) {

    //fprintf( stderr, "need x marker draw command, x=%-g, y=%-g\n",
    // bufXMin, bufXMax );

    if ( connection.pvsConnected() ) {
      xMinPv->put( bufXMin );
      xMaxPv->put( bufXMax );
      updatePv->put( drawXMarker );
    }

  }

  if ( nmec ) {

    /* fprintf( stderr, "need x marker erase command\n" ); */

    if ( connection.pvsConnected() ) {
      updatePv->put( eraseXMarker );
    }

  }

  if ( nmd ) {

    //fprintf( stderr, "1 need x marker draw, bufXMax = %-g\n", bufXMax );

    actWin->executeGc.setFG( lineColor.pixelColor() );
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineOnOffDash );

    xRange = xMax - xMin;
    if ( bufXMin != bufXMax ) {
      markerX = bufXMax;
    }

    //fprintf( stderr, "2 need x marker draw, markerX = %-g\n", markerX );

    clipStat = actWin->executeGc.addXorXClipRectangle( xR );

    if ( markerDrawn ) {

      /* fprintf( stderr, "erase old\n" ); */

      if ( ( oldMarkerX >= xMin ) && ( oldMarkerX <= xMax ) ) {

        ix = x + (int) ( ( oldMarkerX - xMin ) / xRange * w );
        iy0 = y;
        iy1 = y + h;
        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.xorGC(), ix, iy0, ix, iy1 );

      }

    }

    if ( ( markerX >= xMin ) && ( markerX <= xMax ) ) {

      ix = x + (int) ( ( markerX - xMin ) / xRange * w );
      iy0 = y;
      iy1 = y + h;
      XDrawLine( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.xorGC(), ix, iy0, ix, iy1 );

      markerDrawn = 1;

    }

    if ( clipStat & 1 ) actWin->executeGc.removeXorXClipRectangle();

    actWin->executeGc.setLineStyle( LineSolid );
    actWin->executeGc.restoreFg();

    oldMarkerX = markerX;

    if ( connection.pvsConnected() ) {
      updatePv->put( 0 );
    }

  }

  if ( nme ) {

    if ( !markerDrawn ) {
      /* fprintf( stderr, "no erase was performed\n" ); */
    }
    else {

      /* fprintf( stderr, "need x marker erase, x = %-g\n", oldMarkerX ); */

      if ( ( oldMarkerX >= xMin ) && ( oldMarkerX <= xMax ) ) {

        actWin->executeGc.setFG( lineColor.pixelColor() );
        actWin->executeGc.setLineWidth( 1 );
        actWin->executeGc.setLineStyle( LineOnOffDash );

        xRange = xMax - xMin;
        ix = x + (int) ( ( oldMarkerX - xMin ) / xRange * w );
        iy0 = y;
        iy1 = y + h;

        clipStat = actWin->executeGc.addXorXClipRectangle( xR );

        XDrawLine( actWin->d, drawable(actWin->executeWidget),
         actWin->executeGc.xorGC(), ix, iy0, ix, iy1 );

        if ( clipStat & 1 ) actWin->executeGc.removeXorXClipRectangle();

        actWin->executeGc.setLineStyle( LineSolid );
        actWin->executeGc.restoreFg();

      }

    }

    markerDrawn = 0;

    if ( connection.pvsConnected() ) {
      updatePv->put( 0 );
    }

  }

}

void archivePlotClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_FG1COLOR_MASK )
    lineColor.setColorIndex( _fg1Color, actWin->ci );

  if ( _flag & ACTGRF_BGCOLOR_MASK )
    bgColor.setColorIndex( _bgColor, actWin->ci );

}

void archivePlotClass::changePvNames (
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

}

int archivePlotClass::readFile ( void ) {

FILE *f;
char line[127+1], *tk, *context, *result;
int n;

/* fprintf( stderr, "readFile\n" ); */

  numPoints = 0;

  if ( blank( file ) ) return 0;

  f = fopen( file, "r" );
  if ( !f ) return 0;

  n = 0;
  result = fgets( line, 127, f );
  if ( !result ) return 0;

  context = NULL;
  tk = strtok_r( line, " ", &context );
  if ( !tk ) return 0;
  if ( strcmp( tk, "?" ) == 0 ) {
    xarray[n] = (double) n;
  }
  else {
    xarray[n] = atof( tk );
  }
  xMin = xMax = xarray[n];

  tk = strtok_r( NULL, " ", &context );
  if ( !tk ) return 0;
  yarray[n] = atof( tk );
  yMin = yMax = yarray[n];

  n = 1;

  do {

    if ( n < maxDataPoints ) {

      result = fgets( line, 127, f );
      if ( result ) {

        tk = strtok_r( line, " ", &context );
        if ( !tk ) return 0;
        if ( strcmp( tk, "?" ) == 0 ) {
          xarray[n] = (double) n;
        }
        else {
          xarray[n] = atof( tk );
        }

        if ( xarray[n] > xMax ) xMax = xarray[n];
        if ( xarray[n] < xMin ) xMin = xarray[n];

        tk = strtok_r( NULL, " ", &context );
        if ( !tk ) return 0;
        yarray[n] = atof( tk );

        if ( yarray[n] > yMax ) yMax = yarray[n];
        if ( yarray[n] < yMin ) yMin = yarray[n];

        n++;

      }

    }

  } while ( result && ( n < maxDataPoints ) );

  fclose( f );

  numPoints = n;

  if ( xMax <= xMin ) xMax = xMin + 1.0;
  if ( yMax <= yMin ) yMax = yMin + 1.0;

  saveIndex = 0;
  saveXMin[saveIndex] = xMin;
  saveXMax[saveIndex] = xMax;
  saveYMin[saveIndex] = yMin;
  saveYMax[saveIndex] = yMax;

  return 1;

}


#ifdef OLDARCHIVER

int archivePlotClass::readArchive ( void ) {

int n;
bool result;
double tt0, val;
char msg[80];
ValueIterator values;
osiTime start( start_time_t, 0 );
osiTime end( end_time_t, 0 );

  // fprintf( stderr, "readArchive\n" );
  // fprintf( stderr, "archive is [%s]\n", archiveName );

  try {

  Archive archiveObj( new BinArchive( archiveName ) );
  ChannelIterator channel( archiveObj );

  archiveObj.findChannelByName ( (const stdString) file, channel );

  if ( !channel ) return 0;

  //fprintf( stderr, "channel is [%s]\n", file );

  ValueIterator values( archiveObj );

  // tt0 = (double) start / 3600.0;
  tt0 = 0.0;

  result = channel->getValueBeforeTime( start, values );
  if ( !result ) {

    result = channel->getValueAfterTime( start, values );
    if ( !result ) {
      sprintf( msg, "No data time for specified date range" );
      actWin->appCtx->postMessage( msg );
      return 1;
    }

  }

  yType = values->getType();

  n = -1;

  if ( (double) values->getTime() < (double) end ) {

    n = 0;

    xarray[n] = (double) values->getTime() / 3600.0 - tt0;
    xMin = xMax = xarray[n];

    if ( yMode == modeLog ) {
      val = values->getDouble();
      if ( val <= 0 ) val = 1;
      yarray[n] = log10( val );
      yMin = yMax = yarray[n];
    }
    else {
      yarray[n] = values->getDouble();
      yMin = yMax = yarray[n];
    }

#if 0
    if ( debugMode() ) {
      fprintf( stderr, "before: time(hr) = %-15.9g, value = %-g\n",
       (double) values->getTime() / 3600.0 - tt0,
       values->getDouble() );
    }
#endif

    ++values;

    if ( msgDialogPoppedUp ) {
      msgDialog.popdown();
    }
    msgDialog.popup( "Reading Archive...", actWin->x+x+w/2, actWin->y+y+h/2 );
    msgDialogPoppedUp = 1;

    abortRead = 0;
    while ( values && ( (double) values->getTime() < (double) end ) &&
            !abortRead ) {

      processAllEvents( actWin->appCtx->appContext(), actWin->display() );
      pend_io( 0.01 );
      pend_event( 0.00001 );

#if 0
      if ( debugMode() ) {
        fprintf( stderr, "time(hr) = %-15.9g, value = %-g\n",
         (double) values->getTime() / 3600.0 - tt0,
         values->getDouble() );
      }
#endif

      if ( n < maxDataPoints-1 ) {

        n++;

        if ( !( n % 100 ) ) {
          sprintf( msg, "Reading Archive... (%-d)", n );
          msgDialog.popup( msg, actWin->x+x+w/2, actWin->y+y+h/2 );
          msgDialogPoppedUp = 1;
	}

        xarray[n] = (double) values->getTime() / 3600.0 - tt0;
        if ( xarray[n] > xMax ) xMax = xarray[n];
        if ( xarray[n] < xMin ) xMin = xarray[n];

        if ( yMode == modeLog ) {
          val = values->getDouble();
          if ( val <= 0 ) val = 1;
          yarray[n] = log10( val );
	}
	else {
          yarray[n] = values->getDouble();
	}

        if ( yarray[n] > yMax ) yMax = yarray[n];
        if ( yarray[n] < yMin ) yMin = yarray[n];

      }
      else {

        sprintf( msg, "Array size exceeded (%-d points), data set truncated",
         maxDataPoints );
        actWin->appCtx->postMessage( msg );
        ++values; // do this for code below
        break;

      }

      ++values;

    }

  }

  if ( !abortRead ) {

    // one more
    result = channel->getValueAfterTime( end, values );
    if ( result ) {

      if ( n < maxDataPoints-1 ) {

        n++;

        xarray[n] = (double) values->getTime() / 3600.0 - tt0;
        if ( xarray[n] > xMax ) xMax = xarray[n];
        if ( xarray[n] < xMin ) xMin = xarray[n];

        if ( yMode == modeLog ) {
          val = values->getDouble();
          if ( val <= 0 ) val = 1;
          yarray[n] = log10( val );
        }
        else {
          yarray[n] = values->getDouble();
        }

        if ( yarray[n] > yMax ) yMax = yarray[n];
        if ( yarray[n] < yMin ) yMin = yarray[n];

#if 0
        if ( debugMode() ) {
          fprintf( stderr, "after: time(hr) = %-15.9g, value = %-g\n",
           (double) values->getTime() / 3600.0 - tt0,
           values->getDouble() );
        }
#endif

      }
      else {

        sprintf( msg, "Array size exceeded (%-d points), data set truncated",
         maxDataPoints );
        actWin->appCtx->postMessage( msg );

      }

    }

  }

  numPoints = n + 1;

  if ( xMax <= xMin ) xMax = xMin + 1.0;
  if ( yMax <= yMin ) yMax = yMin + 1.0;

  saveIndex = 0;
  saveXMin[saveIndex] = xMin;
  saveXMax[saveIndex] = xMax;
  saveYMin[saveIndex] = yMin;
  saveYMax[saveIndex] = yMax;

  if ( msgDialogPoppedUp ) {
    msgDialog.popdown();
    msgDialogPoppedUp = 0;
  }

  if ( numPoints == 0 ) {
    sprintf( msg, "No data time for specified date range" );
    actWin->appCtx->postMessage( msg );
  }
  else if ( numPoints == 1 ) {
    sprintf( msg, "Only one data point for specified date range" );
    actWin->appCtx->postMessage( msg );
  }

  return 1;

  }
  catch (const ArchiveException &e) {

    if ( msgDialogPoppedUp ) {
      msgDialog.popdown();
      msgDialogPoppedUp = 0;
    }

    numPoints = 0;
    sprintf( msg, "%s\n", e.what() );
    actWin->appCtx->postMessage( msg );
    return 0;

  }

}

#else

int readValues (
  IndexFile &indexf,
  stdVector<stdString> names,
  epicsTime *start,
  epicsTime *end,
  ReaderFactory::How how,
  double delta,
  int max,
  double *x,
  double *y,
  int *n,
  DbrType *yType,
  int *truncated
) {

SpreadsheetReader sheet(indexf, how, delta);
bool ok = sheet.find(names, start);
const RawValue::Data *value;
double dVal;
bool status;
epicsTimeStamp stamp;

  *n = 0;
  *yType = 0;
  *truncated = 0;

  if ( !sheet.getChannelFound(0) ) {
    fprintf(stderr, "Warning: channel '%s' not in archive\n",
     names[0].c_str());
    return 0;
  }

  if ( ok ) { // got at least one value
    *yType = sheet.getType(0);
  }

  while ( ok && ( *n < max-1 ) ) {

    if ( end && sheet.getTime() >= *end ) break;

    stamp = sheet.getTime();

    value = sheet.getValue(0);
    if ( value ) {

      if ( !RawValue::isInfo(value) ) {

        status = RawValue::getDouble(
         sheet.getType(0), sheet.getCount(0),
         value, dVal, 0 );

        x[*n] = (double) stamp.secPastEpoch + stamp.nsec/1e9;
	y[*n] = dVal;
	(*n)++;

      }

    }

    ok = sheet.next();

  }

  if ( *n >= max-1 ) {
    *n = max-1;
    *truncated = 1;
  }

  return 1;

}


int archivePlotClass::readArchive ( void ) {

int i, n, truncated;
bool result;
double val;
char msg[80];
stdVector<stdString> names;
ReaderFactory::How how = ReaderFactory::Raw;
IndexFile indexf(50);
double delta = 0.0;
epicsTimeStamp stamp;

  // fprintf( stderr, "readArchive\n" );
  // fprintf( stderr, "archive is [%s]\n", archiveName );

  epicsTimeFromTime_t( &stamp, start_time_t );
  //fprintf( stderr, "start = %-d\n", stamp.secPastEpoch );
  epicsTime tStart( stamp );

  epicsTimeFromTime_t( &stamp, end_time_t );
  //fprintf( stderr, "start = %-d\n", stamp.secPastEpoch );
  epicsTime tEnd( stamp );

  //try {

    if ( !indexf.open( archiveName ) ) return 0;

    names.push_back( (const stdString) file );

    if ( msgDialogPoppedUp ) {
      msgDialog.popdown();
    }
    msgDialog.popup( "Reading Archive...", actWin->x+x+w/2, actWin->y+y+h/2 );
    msgDialogPoppedUp = 1;

    result = readValues( indexf, names, &tStart, &tEnd, how, delta,
     maxDataPoints, xarray, yarray, &n, &yType, &truncated );

    if ( !result ) {
      indexf.close();
      return 0;
    }

    if ( truncated ) {
      sprintf( msg, "Array size exceeded (%-d points), data set truncated",
       maxDataPoints );
      actWin->appCtx->postMessage( msg );
    }


    xarray[0] = xarray[0] / 3600.0;
    xMin = xMax = xarray[0];

    if ( yMode == modeLog ) {
      val = yarray[0];
      if ( val <= 0 ) val = 1;
      yarray[0] = log10( val );
    }
    yMin = yMax = yarray[0];

    for ( i=1; i<n; i++ ) {

      //fprintf( stderr, "%16.11lg     %16.11lg\n", xarray[i], yarray[i] );

      xarray[i] = xarray[i] / 3600.0;
      if ( xarray[i] > xMax ) xMax = xarray[i];
      if ( xarray[i] < xMin ) xMin = xarray[i];

      if ( yMode == modeLog ) {
        val = yarray[i];
        if ( val <= 0 ) val = 1;
        yarray[i] = log10( val );
      }

      if ( yarray[i] > yMax ) yMax = yarray[i];
      if ( yarray[i] < yMin ) yMin = yarray[i];

    }

  //}
  //catch (const ArchiveException &e) {

  //  if ( msgDialogPoppedUp ) {
  //    msgDialog.popdown();
  //    msgDialogPoppedUp = 0;
  //  }

  //  numPoints = 0;
  //  sprintf( msg, "%s\n", e.what() );
  //  actWin->appCtx->postMessage( msg );
  //  return 0;

  //}

  numPoints = n;

  //fprintf( stderr, "numPoints = %-d\n", numPoints );

  if ( xMax <= xMin ) xMax = xMin + 1.0;
  if ( yMax <= yMin ) yMax = yMin + 1.0;

  saveIndex = 0;
  saveXMin[saveIndex] = xMin;
  saveXMax[saveIndex] = xMax;
  saveYMin[saveIndex] = yMin;
  saveYMax[saveIndex] = yMax;

  if ( msgDialogPoppedUp ) {
    msgDialog.popdown();
    msgDialogPoppedUp = 0;
  }

  if ( numPoints == 0 ) {
    sprintf( msg, "No data time for specified date range" );
    actWin->appCtx->postMessage( msg );
  }
  else if ( numPoints == 1 ) {
    sprintf( msg, "Only one data point for specified date range" );
    actWin->appCtx->postMessage( msg );
  }

  indexf.close();

  return 1;

}

#endif


void archivePlotClass::rescale ( void ) {

double xRange, yRange;
int i;

  if ( !activeMode || !init ) return;

  /* fprintf( stderr, "rescale\n" ); */
  /* fprintf( stderr, "xMin = %-g\n", xMin ); */
  /* fprintf( stderr, "xMax = %-g\n", xMax ); */
  /* fprintf( stderr, "yMin = %-g\n", yMin ); */
  /* fprintf( stderr, "yMax = %-g\n", yMax ); */

  xRange = xMax - xMin;
  yRange = yMax - yMin;

  for ( i=0; i<numPoints; i++ ) {

    ixarray[i] = x + (int) ( ( xarray[i] - xMin ) / xRange * w );
    iyarray[i] = y + h - (int) ( ( yarray[i] - yMin ) / yRange * h );

  }

}

void archivePlotClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 12 ) {
    *n = 0;
    return;
  }

  *n = 12;
  pvs[0] = xMinPv;
  pvs[1] = xMaxPv;
  pvs[2] = xModePv;
  pvs[3] = yMinPv;
  pvs[4] = yMaxPv;
  pvs[5] = yModePv;
  pvs[6] = colorPv;
  pvs[7] = filePv;
  pvs[8] = updatePv;
  pvs[9] = archivePv;
  pvs[10] = startTimePv;
  pvs[11] = endTimePv;

}

extern "C" {

void *create_2d80926b_bf54_4096_ab21_af7d725f15a2Ptr ( void ) {

archivePlotClass *ptr;

  ptr = new archivePlotClass;
  return (void *) ptr;

}

void *clone_2d80926b_bf54_4096_ab21_af7d725f15a2Ptr (
  void *_srcPtr )
{

archivePlotClass *ptr, *srcPtr;

  srcPtr = (archivePlotClass *) _srcPtr;

  ptr = new archivePlotClass( srcPtr );

  return (void *) ptr;

}

}
