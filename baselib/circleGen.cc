#define __circle_gen_cc 1

#include "circleGen.h"
#include "app_pkg.h"
#include "act_win.h"

#include "thread.h"

static void acoMonitorAlarmPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

  activeCircleClass *aco = (activeCircleClass *) clientData;

  aco->actWin->appCtx->proc->lock();

  if ( !aco->activeMode ) {
    aco->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    aco->needAlarmConnectInit = 1;

  }
  else { // lost connection

    aco->alarmPvConnected = 0;
    aco->active = 0;
    aco->lineColor.setDisconnected();
    aco->fillColor.setDisconnected();
    aco->bufInvalidate();
    aco->needDraw = 1;

  }

  aco->actWin->addDefExeNode( aco->aglPtr );

  aco->actWin->appCtx->proc->unlock();

}

static void circleAlarmUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

class activeCircleClass *aco = (activeCircleClass *) clientData;

  aco->actWin->appCtx->proc->lock();

  if ( !aco->activeMode ) {
    aco->actWin->appCtx->proc->unlock();
    return;
  }

  aco->lineColor.setStatus( classPtr->getStatus( args ),
   classPtr->getSeverity( args ) );
  aco->fillColor.setStatus( classPtr->getStatus( args ),
   classPtr->getSeverity( args ) );

  aco->bufInvalidate();
  aco->needRefresh = 1;

  aco->actWin->addDefExeNode( aco->aglPtr );

  aco->actWin->appCtx->proc->unlock();

}

static void acoMonitorVisPvConnectState (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

  activeCircleClass *aco = (activeCircleClass *) clientData;

  aco->actWin->appCtx->proc->lock();

  if ( !aco->activeMode ) {
    aco->actWin->appCtx->proc->unlock();
    return;
  }

  if ( classPtr->getOp( args ) == classPtr->pvkOpConnUp() ) {

    aco->needVisConnectInit = 1;

  }
  else { // lost connection

    aco->visPvConnected = 0;
    aco->active = 0;
    aco->lineColor.setDisconnected();
    aco->fillColor.setDisconnected();
    aco->bufInvalidate();
    aco->needDraw = 1;

  }

  aco->actWin->addDefExeNode( aco->aglPtr );

  aco->actWin->appCtx->proc->unlock();

}

static void circleVisUpdate (
  pvClass *classPtr,
  void *clientData,
  void *args )
{

pvValType pvV;
class activeCircleClass *aco = (activeCircleClass *) clientData;

  aco->actWin->appCtx->proc->lock();

  if ( !aco->activeMode ) {
    aco->actWin->appCtx->proc->unlock();
    return;
  }

  pvV.d = *( (double *) classPtr->getValue( args ) );

  if ( ( pvV.d >= aco->minVis.d ) && ( pvV.d < aco->maxVis.d ) )
    aco->visibility = 1 ^ aco->visInverted;
  else
    aco->visibility = 0 ^ aco->visInverted;

  if ( aco->visibility ) {

    aco->needRefresh = 1;

  }
  else {

    aco->needErase = 1;
    aco->needRefresh = 1;

  }

  aco->actWin->addDefExeNode( aco->aglPtr );

  aco->actWin->appCtx->proc->unlock();

}


static void acc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

  activeCircleClass *aco = (activeCircleClass *) client;

  aco->actWin->setChanged();

  aco->eraseSelectBoxCorners();
  aco->erase();

  aco->fill = aco->bufFill;

  aco->lineColorMode = aco->bufLineColorMode;
  if ( aco->lineColorMode == ACC_K_COLORMODE_ALARM )
    aco->lineColor.setAlarmSensitive();
  else
    aco->lineColor.setAlarmInsensitive();
  aco->lineColor.setColor( aco->bufLineColor, aco->actWin->ci );

  aco->fillColorMode = aco->bufFillColorMode;
  if ( aco->fillColorMode == ACC_K_COLORMODE_ALARM )
    aco->fillColor.setAlarmSensitive();
  else
    aco->fillColor.setAlarmInsensitive();
  aco->fillColor.setColor( aco->bufFillColor, aco->actWin->ci );

  aco->lineWidth = aco->bufLineWidth;

  if ( aco->bufLineStyle == 0 )
    aco->lineStyle = LineSolid;
  else if ( aco->bufLineStyle == 1 )
    aco->lineStyle = LineOnOffDash;

  aco->alarmPvExpStr.setRaw( aco->bufAlarmPvName );

  aco->visPvExpStr.setRaw( aco->bufVisPvName );

  strncpy( aco->pvUserClassName, aco->actWin->pvObj.getPvName(aco->pvNameIndex), 15);
  strncpy( aco->pvClassName, aco->actWin->pvObj.getPvClassName(aco->pvNameIndex), 15);

  if ( aco->bufVisInverted )
    aco->visInverted = 0;
  else
    aco->visInverted = 1;

  strncpy( aco->minVisString, aco->bufMinVisString, 39 );
  strncpy( aco->maxVisString, aco->bufMaxVisString, 39 );

  aco->x = aco->bufX;
  aco->sboxX = aco->bufX;

  aco->y = aco->bufY;
  aco->sboxY = aco->bufY;

  aco->w = aco->bufW;
  aco->sboxW = aco->bufW;

  aco->h = aco->bufH;
  aco->sboxH = aco->bufH;

}

static void acc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCircleClass *aco = (activeCircleClass *) client;

  acc_edit_update( w, client, call );
  aco->refresh( aco );

}

static void acc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCircleClass *aco = (activeCircleClass *) client;

  acc_edit_apply( w, client, call );
  aco->ef.popdown();
  aco->operationComplete();

}

static void acc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCircleClass *aco = (activeCircleClass *) client;

  aco->ef.popdown();
  aco->operationCancel();

}

static void acc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

activeCircleClass *aco = (activeCircleClass *) client;

  aco->ef.popdown();
  aco->operationCancel();
  aco->erase();
  aco->deleteRequest = 1;
  aco->drawAll();

}

activeCircleClass::activeCircleClass ( void ) {

  name = new char[strlen("activeCircleClass")+1];
  strcpy( name, "activeCircleClass" );
  visibility = 0;
  prevVisibility = -1;
  visInverted = 0;
  visPvConnected = alarmPvConnected = 0;
  visPvExists = alarmPvExists = 0;
  active = 0;
  activeMode = 0;
  fill = 0;
  lineColorMode = ACC_K_COLORMODE_STATIC;
  fillColorMode = ACC_K_COLORMODE_STATIC;
  lineWidth = 1;
  lineStyle = LineSolid;
  strcpy( minVisString, "" );
  strcpy( maxVisString, "" );
  strcpy( pvClassName, "" );
  strcpy( pvUserClassName, "" );

}

// copy constructor
activeCircleClass::activeCircleClass
( const activeCircleClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("activeCircleClass")+1];
  strcpy( name, "activeCircleClass" );

  lineColor.copy(source->lineColor);
  fillColor.copy(source->fillColor);
  lineCb = source->lineCb;
  fillCb = source->fillCb;
  fill = source->fill;
  lineColorMode = source->lineColorMode;
  fillColorMode = source->fillColorMode;
  visInverted = source->visInverted;

  alarmPvExpStr.setRaw( source->alarmPvExpStr.rawString );
  visPvExpStr.setRaw( source->visPvExpStr.rawString );

  visibility = 0;
  prevVisibility = -1;
  visPvConnected = alarmPvConnected = 0;
  visPvExists = alarmPvExists = 0;
  active = 0;
  activeMode = 0;

  strncpy( minVisString, source->minVisString, 39 );
  strncpy( maxVisString, source->maxVisString, 39 );
  
  strncpy( pvClassName, source->pvClassName, 15 );
  strncpy( pvUserClassName, source->pvUserClassName, 15 );

  lineWidth = source->lineWidth;
  lineStyle = source->lineStyle;

}

int activeCircleClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

//   printf( "In activeCircleClass::createInteractive\n" );

  actWin = (activeWindowClass *) aw_obj;
  x = _x;
  y = _y;
  w = _w;
  h = _h;

  lineColor.setColor( actWin->defaultFg1Color, actWin->ci );
  fillColor.setColor( actWin->defaultBgColor, actWin->ci );

  this->draw();

  strncpy( pvUserClassName, actWin->defaultPvType, 15 );

  this->editCreate();

  return 1;

}

int activeCircleClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "activeCircleClass" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, "Unknown object", 31 );

  strncat( title, " Properties", 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufLineColor = lineColor.pixelColor();
  bufLineColorMode = lineColorMode;

  bufFillColor = fillColor.pixelColor();
  bufFillColorMode = fillColorMode;

  bufFill = fill;
  bufLineWidth = lineWidth;
  bufLineStyle = lineStyle;

  if ( alarmPvExpStr.getRaw() )
    strncpy( bufAlarmPvName, alarmPvExpStr.getRaw(), 39 );
  else
    strncpy( bufAlarmPvName, "", 39 );

  if ( visPvExpStr.getRaw() )
    strncpy( bufVisPvName, visPvExpStr.getRaw(), 39 );
  else
    strncpy( bufVisPvName, "", 39 );

  if ( visInverted )
    bufVisInverted = 0;
  else
    bufVisInverted = 1;

  strncpy( bufMinVisString, minVisString, 39 );
  strncpy( bufMaxVisString, maxVisString, 39 );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( "X", 25, &bufX );
  ef.addTextField( "Y", 25, &bufY );
  ef.addTextField( "Width", 25, &bufW );
  ef.addTextField( "Height", 25, &bufH );
  ef.addOption( "Line Thk", "0|1|2|3|4|5|6|7|8|9|10", &bufLineWidth );
  ef.addOption( "Line Style", "Solid|Dash", &bufLineStyle );
  ef.addColorButton( "Line Color", actWin->ci, &lineCb, &bufLineColor );
  ef.addToggle( "Alarm Sensitive", &bufLineColorMode );
  ef.addToggle( "Fill", &bufFill );
  ef.addColorButton( "Fill Color", actWin->ci, &fillCb, &bufFillColor );
  ef.addToggle( "Alarm Sensitive", &bufFillColorMode );
  ef.addTextField( "Alarm PV", 27, bufAlarmPvName, 39 );
  ef.addTextField( "Visability PV", 27, bufVisPvName, 39 );
  ef.addOption( " ", "Not Visible if|Visible if", &bufVisInverted );
  ef.addTextField( ">=", 27, bufMinVisString, 39 );
  ef.addTextField( "and <", 27, bufMaxVisString, 39 );

  actWin->pvObj.getOptionMenuList( pvOptionList, 255, &numPvTypes );
  if ( numPvTypes == 1 ) {
    pvNameIndex= 0;
  }
  else {
    // printf( "pvUserClassName = [%s]\n", pvUserClassName );
    pvNameIndex = actWin->pvObj.getNameNum( pvUserClassName );
    if ( pvNameIndex < 0 ) pvNameIndex = 0;
    // printf( "pvOptionList = [%s]\n", pvOptionList );
    // printf( "pvNameIndex = %-d\n", pvNameIndex );
    ef.addOption( "PV Type", pvOptionList, &pvNameIndex );
  }

  return 1;

}


int activeCircleClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( acc_edit_ok, acc_edit_apply, acc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int activeCircleClass::edit ( void ) {

  this->genericEdit();
  ef.finished( acc_edit_ok, acc_edit_apply, acc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int activeCircleClass::createFromFile (
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
  if ( ( major < 2 ) && ( minor < 3 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  lineColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

  if ( lineColorMode == ACC_K_COLORMODE_ALARM )
    lineColor.setAlarmSensitive();
  else
    lineColor.setAlarmInsensitive();

  fscanf( f, "%d\n", &fill ); actWin->incLine();

  fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
  if ( ( major < 2 ) && ( minor < 3 ) ) {
    r *= 256;
    g *= 256;
    b *= 256;
  }
  actWin->ci->setRGB( r, g, b, &pixel );
  fillColor.setColor( pixel, actWin->ci );

  fscanf( f, "%d\n", &fillColorMode ); actWin->incLine();

  if ( fillColorMode == ACC_K_COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  alarmPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  visPvExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &visInverted ); actWin->incLine();

  if ( ( major > 1 ) || ( minor > 1 ) ) {
    readStringFromFile( minVisString, 39, f ); actWin->incLine();
    readStringFromFile( maxVisString, 39, f ); actWin->incLine();
  }
  else {
    strcpy( minVisString, "1" );
    strcpy( maxVisString, "1" );
  }

  if ( ( major > 1 ) || ( minor > 0 ) ) {
    fscanf( f, "%d\n", &lineWidth ); actWin->incLine();
    fscanf( f, "%d\n", &lineStyle ); actWin->incLine();
  }
  else {
    lineWidth = 1;
    lineStyle = LineSolid;
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {

    readStringFromFile( pvClassName, 15, f ); actWin->incLine();

    strncpy( pvUserClassName, actWin->pvObj.getNameFromClass( pvClassName ),
     15 );

  }

//  curValue = value = 0;

  return 1;

}

int activeCircleClass::importFromXchFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin ) {

int fgR, fgG, fgB, bgR, bgG, bgB, more;
unsigned int pixel;
char *tk, *gotData, *context, buf[255+1];

  fgR = 0xffff;
  fgG = 0xffff;
  fgB = 0xffff;

  bgR = 0xffff;
  bgG = 0xffff;
  bgB = 0xffff;

  this->actWin = _actWin;

  lineColor.setColor( actWin->defaultFg1Color, actWin->ci );
  fillColor.setColor( actWin->defaultBgColor, actWin->ci );

  // continue until tag is <eod>

  do {

    gotData = getNextDataString( buf, 255, f );
    if ( !gotData ) {
      actWin->appCtx->postMessage( "import file syntax error" );
      return 0;
    }

    context = NULL;

    tk = strtok_r( buf, " \t\n", &context );
    if ( !tk ) {
      actWin->appCtx->postMessage( "import file syntax error" );
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
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        x = atol( tk );

      }
      else if ( strcmp( tk, "y" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        y = atol( tk );

      }
      else if ( strcmp( tk, "w" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        w = atol( tk );

      }
      else if ( strcmp( tk, "h" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        h = atol( tk );

      }
            
      else if ( strcmp( tk, "fgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        fgR = atol( tk );

      }
            
      else if ( strcmp( tk, "fggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        fgG = atol( tk );

      }
            
      else if ( strcmp( tk, "fgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        fgB = atol( tk );

      }
            
      else if ( strcmp( tk, "bgred" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        bgR = atol( tk );

      }
            
      else if ( strcmp( tk, "bggreen" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        bgG = atol( tk );

      }
            
      else if ( strcmp( tk, "bgblue" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        bgB = atol( tk );

      }
            
      else if ( strcmp( tk, "linewidth" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

       lineWidth  = atol( tk );

      }
            
      else if ( strcmp( tk, "fill" ) == 0 ) {

        tk = strtok_r( NULL, "\"\n", &context );
        if ( !tk ) {
          actWin->appCtx->postMessage( "import file syntax error" );
          return 0;
        }

        fill = atol( tk );

      }
            
    }

  } while ( more );

  this->initSelectBox(); // call after getting x,y,w,h

  actWin->ci->setRGB( fgR, fgG, fgB, &pixel );
  lineColor.setColor( pixel, actWin->ci );
  lineColor.setAlarmInsensitive();

  actWin->ci->setRGB( bgR, bgG, bgB, &pixel );
  fillColor.setColor( pixel, actWin->ci );
  fillColor.setAlarmSensitive();

  return 1;

}

int activeCircleClass::save (
  FILE *f )
{

int r, g, b;

  fprintf( f, "%-d %-d %-d\n", ACC_MAJOR_VERSION, ACC_MINOR_VERSION,
   ACC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  actWin->ci->getRGB( lineColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", lineColorMode );

  fprintf( f, "%-d\n", fill );

  actWin->ci->getRGB( fillColor.pixelColor(), &r, &g, &b );
  fprintf( f, "%-d %-d %-d\n", r, g, b );

  fprintf( f, "%-d\n", fillColorMode );

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

  fprintf( f, "%-d\n", lineWidth );

  fprintf( f, "%-d\n", lineStyle );

  writeStringToFile( f, pvClassName );

  return 1;

}

int activeCircleClass::drawActive ( void )
{

  if ( !activeMode || !visibility ) return 1;

  prevVisibility = visibility;

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.saveFg();

  if ( fill ) {
    actWin->executeGc.setFG( fillColor.getColor() );
    XFillArc( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h, 0, 23040 );
  }

  actWin->executeGc.setFG( lineColor.getColor() );
  XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), x, y, w, h, 0, 23040 );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.restoreFg();

  return 1;

}

int activeCircleClass::eraseUnconditional ( void )
{

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );

  if ( fill ) {
    XFillArc( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );
  }

  XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  XFillArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  return 1;

}

int activeCircleClass::eraseActive ( void )
{

  if ( !activeMode ) return 1;

  if ( prevVisibility == 0 ) {
    prevVisibility = visibility;
    return 1;
  }

  prevVisibility = visibility;

  actWin->executeGc.setLineStyle( lineStyle );
  actWin->executeGc.setLineWidth( lineWidth );

  if ( fill ) {
    XFillArc( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );
  }

  XDrawArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  XFillArc( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h, 0, 23040 );

  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setLineWidth( 1 );

  return 1;

}

int activeCircleClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand1st( numMacros, macros, expansions );
  stat = visPvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int activeCircleClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = alarmPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = visPvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int activeCircleClass::containsMacros ( void ) {

  if ( alarmPvExpStr.containsPrimaryMacros() ) return 1;
  if ( visPvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int activeCircleClass::activate (
  int pass,
  void *ptr )
{

int stat;

  switch ( pass ) {

  case 1: // initialize

    needVisConnectInit = 0;
    needAlarmConnectInit = 0;
    needErase = needDraw = needRefresh = 0;
    aglPtr = ptr;
    opComplete = 0;
    visEventId = NULL;
    alarmEventId = NULL;
    
    alarmPvConnected = visPvConnected = 0;
    active = 0;

    actWin->appCtx->proc->lock();
    activeMode = 1;
    actWin->appCtx->proc->unlock();

    prevVisibility = -1;

    init = 1;
    active = 1;

    if ( !alarmPvExpStr.getExpanded() ||
         ( strcmp( alarmPvExpStr.getExpanded(), "" ) == 0 ) ) {
      alarmPvExists = 0;
    }
    else {
      alarmPvExists = 1;
      lineColor.setConnectSensitive();
      fillColor.setConnectSensitive();
      init = 0;
      active = 0;
    }

    if ( !visPvExpStr.getExpanded() ||
         ( strcmp( visPvExpStr.getExpanded(), "" ) == 0 ) ) {
      visPvExists = 0;
      visibility = 1;
    }
    else {
      visPvExists = 1;
      visibility = 0;
      lineColor.setConnectSensitive();
      fillColor.setConnectSensitive();
      init = 0;
      active = 0;
    }

    break;

  case 2: // connect to pv's

    if ( !opComplete ) {

      if ( alarmPvExists ) {
	
          // printf( "pvNameIndex = %-d\n", pvNameIndex );
          // printf( "pv class name = [%s]\n", pvClassName );
          // printf( "pvOptionList = [%s]\n", pvOptionList );

          alarmPvId = actWin->pvObj.createNew( pvClassName );
          if ( !alarmPvId ) {
            printf( "Cannot create %s object", pvClassName );
            // actWin->appCtx->postMessage( msg );
            opComplete = 1;
            return 1;
	  }
          alarmPvId->createEventId( &alarmEventId );

        stat = alarmPvId->searchAndConnect( &alarmPvExpStr,
         acoMonitorAlarmPvConnectState, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect %s\n", 
	   alarmPvExpStr.getExpanded() );
          return 0;
        }
      }

      if ( visPvExists ) {

          // printf( "pvNameIndex = %-d\n", pvNameIndex );
          // printf( "pv class name = [%s]\n", pvClassName );
          // printf( "pvOptionList = [%s]\n", pvOptionList );

          visPvId = actWin->pvObj.createNew( pvClassName );
          if ( !visPvId ) {
            printf( "Cannot create %s object", pvClassName );
            // actWin->appCtx->postMessage( msg );
            opComplete = 1;
            return 1;
	  }
          visPvId->createEventId( &visEventId );
        
        stat = visPvId->searchAndConnect( &visPvExpStr,
         acoMonitorVisPvConnectState, this );
        if ( stat != PV_E_SUCCESS ) {
          printf( "error from searchAndConnect %s\n", 
	   visPvExpStr.getExpanded() );
          return 0;
        }
      }

      opComplete = 1;
      this->bufInvalidate();

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

int activeCircleClass::deactivate (
  int pass ) {

int stat;

  if ( pass == 1 ) {

    actWin->appCtx->proc->lock();

    activeMode = 0;

    if ( alarmPvExists ) {
      stat = alarmPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = alarmPvId->destroyEventId( &alarmEventId );

      delete alarmPvId;

      alarmPvId = NULL;

    }

    if ( visPvExists ) {
      stat = visPvId->clearChannel();
      if ( stat != PV_E_SUCCESS )
        printf( "clearChannel failure\n" );

      stat = visPvId->destroyEventId( &alarmEventId );

      delete visPvId;

      visPvId = NULL;

    }

    actWin->appCtx->proc->unlock();

  }

  return 1;

}

int activeCircleClass::draw ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineStyle( lineStyle );
  actWin->drawGc.setLineWidth( lineWidth );
  actWin->drawGc.saveFg();

  if ( fill ) {
    actWin->drawGc.setFG( fillColor.pixelColor() );
    XFillArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.normGC(),
     x, y, w, h, 0, 23040 );
  }

  actWin->drawGc.setFG( lineColor.pixelColor() );
  XDrawArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.normGC(),
   x, y, w, h, 0, 23040 );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.restoreFg();

  return 1;

}

int activeCircleClass::erase ( void ) {

  if ( activeMode ) return 1;
  if ( deleteRequest ) return 1;

  actWin->drawGc.setLineStyle( lineStyle );
  actWin->drawGc.setLineWidth( lineWidth );

  if ( fill ) {
    XFillArc( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x, y, w, h, 0, 23040 );
  }

  XDrawArc( actWin->d, XtWindow(actWin->drawWidget), actWin->drawGc.eraseGC(),
   x, y, w, h, 0, 23040 );

  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setLineWidth( 1 );

  return 1;

}

void activeCircleClass::executeDeferred ( void ) {

int stat, nvc, nac, ne, nd, nr;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  if ( !activeMode ) {
    actWin->remDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return;
  }

  nvc = needVisConnectInit; needVisConnectInit = 0;
  nac = needAlarmConnectInit; needAlarmConnectInit = 0;
  ne = needErase; needErase = 0;
  nd = needDraw; needDraw = 0;
  nr = needRefresh; needRefresh = 0;
  actWin->remDefExeNode( aglPtr );
  actWin->appCtx->proc->unlock();

  if ( nvc ) {

    if ( ( visPvId->getType() == visPvId->pvrEnum() ) ||
         ( visPvId->getType() == visPvId->pvrLong() ) ||
         ( visPvId->getType() == visPvId->pvrFloat() ) ||
         ( visPvId->getType() == visPvId->pvrDouble() ) ) {

      visPvConnected = 1;

      minVis.d = (double) atof( minVisString );
      maxVis.d = (double) atof( maxVisString );

      if ( ( visPvConnected || !visPvExists ) &&
           ( alarmPvConnected || !alarmPvExists ) ) {

        active = 1;
        lineColor.setConnected();
        fillColor.setConnected();
        bufInvalidate();

        if ( init ) {
          eraseUnconditional();
	}

        init = 1;

        actWin->requestActiveRefresh();

      }
      

    // pvrDouble tells channel access to send the value of the pv
    // as a double to the event callback
    if ( !visEventId->eventAdded() ) {
      stat = visPvId->addEvent( visPvId->pvrDouble(), 1,
       circleVisUpdate, (void *) this, visEventId, visPvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }

    }
    else { // force a draw in the non-active state

      active = 0;
      lineColor.setDisconnected();
      fillColor.setDisconnected();
      bufInvalidate();
      drawActive();

    }

  }

  if ( nac ) {

    alarmPvConnected = 1;

    if ( ( visPvConnected || !visPvExists ) &&
         ( alarmPvConnected || !alarmPvExists ) ) {

      active = 1;
      lineColor.setConnected();
      fillColor.setConnected();
      bufInvalidate();

      if ( init ) {
        eraseUnconditional();
      }

      init = 1;

      actWin->requestActiveRefresh();

    }

    // if ( !alarmEventId ) {
    
    // pvrDouble tells channel access to send the value of the pv
    // as a double to the event callback
    if ( !alarmEventId->eventAdded() ) {
      stat = alarmPvId->addEvent( alarmPvId->pvrDouble(), 1,
       circleAlarmUpdate, (void *) this, alarmEventId, alarmPvId->pveValue() );
      if ( stat != PV_E_SUCCESS ) {
        printf( "addEvent failed\n" );
      }
    }

  }

  if ( ne ) {
    eraseActive();
  }

  if ( nd ) {
//      drawActive();
    stat = smartDrawAllActive();
  }

  if ( nr ) {
//      actWin->requestActiveRefresh();
    stat = smartDrawAllActive();
  }
}

#ifdef __cplusplus
extern "C" {
#endif

void *create_activeCircleClassPtr ( void ) {

activeCircleClass *ptr;

  ptr = new activeCircleClass;
  return (void *) ptr;

}

void *clone_activeCircleClassPtr (
  void *_srcPtr )
{

activeCircleClass *ptr, *srcPtr;

  srcPtr = (activeCircleClass *) _srcPtr;

  ptr = new activeCircleClass( srcPtr );

  return (void *) ptr;

}

#ifdef __cplusplus
}
#endif
