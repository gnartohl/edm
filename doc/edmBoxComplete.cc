//  for object: 2ed7d2e8-f439-11d2-8fed-00104b8742df
//  for lib: 3014b6ee-f439-11d2-ad99-00104b8742df

#define SMARTDRAW 1

#define __edmBox_cc 1

#include "edmBoxComplete.h"
#include "app_pkg.h"
#include "act_win.h"

static void eboMonitorPvConnectState (
  struct connection_handler_args arg )
{

edmBoxClass *ebo = (edmBoxClass *) ca_puser(arg.chid);

  ebo->actWin->appCtx->proc->lock();

  if ( !ebo->activeMode ) {
    ebo->actWin->appCtx->proc->unlock();
    return;
  }

  ebo->fieldType = ca_field_type(arg.chid);

  if ( arg.op == CA_OP_CONN_UP ) {

    ebo->needConnectInit = 1;

  }
  else { // lost connection

    ebo->active = 0;
    ebo->lineColor.setDisconnected();
    ebo->fillColor.setDisconnected();
    ebo->bufInvalidate();
    ebo->needDraw = 1;

  }

  ebo->actWin->addDefExeNode( ebo->aglPtr );

  ebo->actWin->appCtx->proc->unlock();

}

static void edmBoxInfoUpdate (
  struct event_handler_args ast_args )
{


edmBoxClass *ebo = (edmBoxClass *) ast_args.usr;
struct dbr_gr_double controlRec = *( (dbr_gr_double *) ast_args.dbr );

  ebo->actWin->appCtx->proc->lock();

  if ( !ebo->activeMode ) {
    ebo->actWin->appCtx->proc->unlock();
    return;
  }

  ebo->curValue = controlRec.value;

  if ( ebo->efReadMin.isNull() ) {
    ebo->readMin = controlRec.lower_disp_limit;
  }
  else {
    ebo->readMin = ebo->efReadMin.value();
  }

  if ( ebo->efReadMax.isNull() ) {
    ebo->readMax = controlRec.upper_disp_limit;
  }
  else {
    ebo->readMax = ebo->efReadMax.value();
  }

  ebo->needInfoInit = 1;
  ebo->actWin->addDefExeNode( ebo->aglPtr );

  ebo->actWin->appCtx->proc->unlock();

}

static void edmBoxUpdate (
  struct event_handler_args ast_args )
{

class edmBoxClass *ebo = (edmBoxClass *) ast_args.usr;

  ebo->actWin->appCtx->proc->lock();

  if ( !ebo->activeMode ) {
    ebo->actWin->appCtx->proc->unlock();
    return;
  }

  ebo->curValue = *( (double *) ast_args.dbr );

  ebo->needUpdate = 1;
  ebo->actWin->addDefExeNode( ebo->aglPtr );

  ebo->actWin->appCtx->proc->unlock();

}

static void edmBoxAlarmUpdate (
  struct event_handler_args ast_args )
{

edmBoxClass *ebo = (edmBoxClass *) ca_puser(ast_args.chid);
struct dbr_sts_float boxStatus;

  ebo->actWin->appCtx->proc->lock();

  if ( !ebo->activeMode ) {
    ebo->actWin->appCtx->proc->unlock();
    return;
  }

  ebo = (edmBoxClass *) ast_args.usr;

  boxStatus = *( (struct dbr_sts_float *) ast_args.dbr );
  ebo->lineColor.setStatus( boxStatus.status, boxStatus.severity );
  ebo->fillColor.setStatus( boxStatus.status, boxStatus.severity );

  ebo->needDraw = 1;
  ebo->bufInvalidate();
  ebo->actWin->addDefExeNode( ebo->aglPtr );

  ebo->actWin->appCtx->proc->unlock();

}

static void ebc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  ebo->actWin->setChanged();

  ebo->eraseSelectBoxCorners();
  ebo->erase();

  ebo->lineColorMode = ebo->bufLineColorMode;
  if ( ebo->lineColorMode == EBC_K_COLORMODE_ALARM )
    ebo->lineColor.setAlarmSensitive();
  else
    ebo->lineColor.setAlarmInsensitive();
  ebo->lineColor.setColorIndex( ebo->bufLineColor, ebo->actWin->ci );

  ebo->fill = ebo->bufFill;

  ebo->fillColorMode = ebo->bufFillColorMode;
  if ( ebo->fillColorMode == EBC_K_COLORMODE_ALARM )
    ebo->fillColor.setAlarmSensitive();
  else
    ebo->fillColor.setAlarmInsensitive();
  ebo->fillColor.setColorIndex( ebo->bufFillColor, ebo->actWin->ci );

  ebo->lineWidth = ebo->bufLineWidth;

  if ( ebo->bufLineStyle == 0 )
    ebo->lineStyle = LineSolid;
  else if ( ebo->bufLineStyle == 1 )
    ebo->lineStyle = LineOnOffDash;

  ebo->pvExpStr.setRaw( ebo->bufPvName );

  ebo->efReadMin = ebo->bufEfReadMin;
  ebo->efReadMax = ebo->bufEfReadMax;

  if ( ( ebo->efReadMin.isNull() ) && ( ebo->efReadMax.isNull() ) ) {
    ebo->readMin = 0;
    ebo->readMax = 10;
  }
  else{
    ebo->readMin = ebo->efReadMin.value();
    ebo->readMax = ebo->efReadMax.value();
  }

  strncpy( ebo->fontTag, ebo->fm.currentFontTag(), 63 );
  ebo->fontTag[63] = 0;

  ebo->actWin->fi->loadFontTag( ebo->fontTag );

  ebo->alignment = ebo->fm.currentFontAlignment();

  ebo->fs = ebo->actWin->fi->getXFontStruct( ebo->fontTag );

  strncpy( ebo->label, ebo->bufLabel, 63 );
  ebo->label[63] = 0;

  ebo->stringLength = strlen( ebo->label );

  ebo->updateFont( ebo->label, ebo->fontTag, &ebo->fs,
   &ebo->fontAscent, &ebo->fontDescent, &ebo->fontHeight,
   &ebo->stringWidth );

  ebo->x = ebo->bufX;
  ebo->sboxX = ebo->bufX;

  ebo->y = ebo->bufY;
  ebo->sboxY = ebo->bufY;

  ebo->w = ebo->bufW;
  ebo->sboxW = ebo->bufW;

  ebo->h = ebo->bufH;
  ebo->sboxH = ebo->bufH;

  if ( ebo->w < 5 ) {
    ebo->w = 5;
    ebo->sboxW = ebo->w;
  }

  ebo->boxH = ebo->h - ebo->fontHeight;
  if ( ebo->boxH < 5 ) {
    ebo->boxH = 5;
    ebo->h = ebo->boxH + ebo->fontHeight;
    ebo->sboxH = ebo->h;
  }

  if ( ebo->readMax > ebo->readMin ) {
    ebo->factorW = (double) ebo->w / ( ebo->readMax - ebo->readMin );
    ebo->factorH = (double) ebo->boxH / ( ebo->readMax - ebo->readMin );
  }
  else {
    ebo->factorW = 1;
    ebo->factorH = 1;
  }

  ebo->centerX = ebo->x + (int) ( ebo->w * 0.5 + 0.5 );
  ebo->centerY = ebo->y + (int) ( ebo->boxH * 0.5 + 0.5 );

  if ( ebo->alignment == XmALIGNMENT_BEGINNING )
    ebo->labelX = ebo->x;
  else if ( ebo->alignment == XmALIGNMENT_CENTER )
    ebo->labelX = ebo->x + ebo->w/2 - ebo->stringWidth/2;
  else if ( ebo->alignment == XmALIGNMENT_END )
    ebo->labelX = ebo->x + ebo->w - ebo->stringWidth;

  ebo->labelY = ebo->y + ebo->h;

}

static void ebc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  ebc_edit_update( w, client, call );
  ebo->refresh( ebo );

}

static void ebc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  ebc_edit_update( w, client, call );
  ebo->ef.popdown();
  ebo->operationComplete();

}

static void ebc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  ebo->ef.popdown();
  ebo->operationCancel();

}

static void ebc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  ebo->ef.popdown();
  ebo->operationCancel();
  ebo->erase();
  ebo->deleteRequest = 1;
  ebo->drawAll();

}

// default constructor
edmBoxClass::edmBoxClass ( void ) {

  name = new char[strlen("2ed7d2e8_f439_11d2_8fed_00104b8742df")+1];
  strcpy( name, "2ed7d2e8_f439_11d2_8fed_00104b8742df" );
  pvExists = 0;
  active = 0;
  activeMode = 0;
  fill = 0;
  lineColorMode = EBC_K_COLORMODE_STATIC;
  fillColorMode = EBC_K_COLORMODE_STATIC;
  lineWidth = 1;
  lineStyle = LineSolid;
  readMin = 0;
  readMax = 10;
  efReadMin.setNull(1);
  efReadMax.setNull(1);
  strcpy( fontTag, "" );
  strcpy( label, "" );
  fs = NULL;
  labelX = 0;
  labelY = 0;
  alignment = XmALIGNMENT_BEGINNING;

}

// copy constructor
edmBoxClass::edmBoxClass
 ( const edmBoxClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("2ed7d2e8_f439_11d2_8fed_00104b8742df")+1];
  strcpy( name, "2ed7d2e8_f439_11d2_8fed_00104b8742df" );

  lineColor.copy(source->lineColor);
  fillColor.copy(source->fillColor);
  lineCb = source->lineCb;
  fillCb = source->fillCb;
  fill = source->fill;
  lineColorMode = source->lineColorMode;
  fillColorMode = source->fillColorMode;
  strncpy( fontTag, source->fontTag, 63 );
  fontTag[63] = 0;
  strncpy( label, source->label, 63 );
  label[63] = 0;
  fs = actWin->fi->getXFontStruct( fontTag );
  stringLength = source->stringLength;
  fontAscent = source->fontAscent;
  fontDescent = source->fontDescent;
  fontHeight = source->fontHeight;
  stringWidth = source->stringWidth;
  labelX = source->labelX;
  labelY = source->labelY;
  alignment = source->alignment;
  lineWidth = source->lineWidth;
  lineStyle = source->lineStyle;

  pvExpStr.setRaw( source->pvExpStr.rawString );

  pvExists = 0;
  active = 0;
  activeMode = 0;

  readMin = source->readMin;
  readMax = source->readMax;
  efReadMin = source->efReadMin;
  efReadMax = source->efReadMax;

}

edmBoxClass::~edmBoxClass ( void ) {

  if ( name ) delete name;

}

char *edmBoxClass::objName ( void ) {

  return name;

}

int edmBoxClass::createInteractive (
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
  fillColor.setColorIndex( actWin->defaultBgColor, actWin->ci );
  strcpy( fontTag, actWin->defaultCtlFontTag );
  alignment = actWin->defaultCtlAlignment;

  fs = actWin->fi->getXFontStruct( fontTag );
  updateFont( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  boxH = h - fontHeight;
  if ( boxH < 5 ) {
    boxH = 5;
    h = boxH + fontHeight;
    sboxH = h;
  }

  if ( w < 5 ) {
    w = 5;
    sboxW = 5;
  }

  this->draw();

  this->editCreate();

  return 1;

}

int edmBoxClass::genericEdit ( void ) {

char title[32], *ptr;

  ptr = actWin->obj.getNameFromClass( "2ed7d2e8_f439_11d2_8fed_00104b8742df" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, edmBoxComplete_str2, 31 );

  strncat( title, edmBoxComplete_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufLineColor = lineColor.pixelIndex();
  bufLineColorMode = lineColorMode;

  bufFillColor = fillColor.pixelIndex();
  bufFillColorMode = fillColorMode;

  bufFill = fill;
  bufLineWidth = lineWidth;
  bufLineStyle = lineStyle;

  if ( pvExpStr.getRaw() )
    strncpy( bufPvName, pvExpStr.getRaw(), 39 );
  else
    strncpy( bufPvName, "", 39 );

  bufEfReadMin = efReadMin;
  bufEfReadMax = efReadMax;

  strncpy( bufLabel, label, 63 );
  bufLabel[63] = 0;

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( edmBoxComplete_str4, 25, &bufX );
  ef.addTextField( edmBoxComplete_str5, 25, &bufY );
  ef.addTextField( edmBoxComplete_str6, 25, &bufW );
  ef.addTextField( edmBoxComplete_str7, 25, &bufH );
  ef.addOption( edmBoxComplete_str8, edmBoxComplete_str19,
   &bufLineWidth );
  ef.addOption( edmBoxComplete_str9, edmBoxComplete_str20,
   &bufLineStyle );
  ef.addColorButton( edmBoxComplete_str10, actWin->ci, &lineCb,
   &bufLineColor );
  ef.addToggle( edmBoxComplete_str11, &bufLineColorMode );
  ef.addToggle( edmBoxComplete_str12, &bufFill );
  ef.addColorButton( edmBoxComplete_str13, actWin->ci, &fillCb,
   &bufFillColor );
  ef.addToggle( edmBoxComplete_str11, &bufFillColorMode );
  ef.addTextField( edmBoxComplete_str14, 27, bufPvName, 39 );
  ef.addTextField( edmBoxComplete_str15, 27, &bufEfReadMin );
  ef.addTextField( edmBoxComplete_str16, 27, &bufEfReadMax );
  ef.addFontMenu( edmBoxComplete_str17, actWin->fi, &fm, fontTag );
  fm.setFontAlignment( alignment );
  ef.addTextField( edmBoxComplete_str18, 27, bufLabel, 63 );

  return 1;

}

int edmBoxClass::editCreate ( void ) {

  this->genericEdit();
  ef.finished( ebc_edit_ok, ebc_edit_apply, ebc_edit_cancel_delete, this );
  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int edmBoxClass::edit ( void ) {

  this->genericEdit();
  ef.finished( ebc_edit_ok, ebc_edit_apply, ebc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int edmBoxClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
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

  if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == EBC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    fscanf( f, "%d\n", &index ); actWin->incLine();
    fillColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == EBC_K_COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    fscanf( f, "%d %d %d\n", &r, &g, &b ); actWin->incLine();
    actWin->ci->setRGB( r, g, b, &pixel );
    index = actWin->ci->pixIndex( pixel );
    fillColor.setColorIndex( index, actWin->ci );

  }

  fscanf( f, "%d\n", &fillColorMode ); actWin->incLine();

  if ( fillColorMode == EBC_K_COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  readStringFromFile( oneName, 39, f ); actWin->incLine();
  pvExpStr.setRaw( oneName );

  fscanf( f, "%d\n", &lineWidth ); actWin->incLine();

  fscanf( f, "%d\n", &lineStyle ); actWin->incLine();

  efReadMin.read( f ); actWin->incLine();

  efReadMax.read( f ); actWin->incLine();

  if ( ( efReadMin.isNull() ) && ( efReadMax.isNull() ) ) {
    readMin = 0;
    readMax = 10;
  }
  else{
    readMin = efReadMin.value();
    readMax = efReadMax.value();
  }

  if ( ( major > 1 ) || ( minor > 3 ) ) {
    readStringFromFile( fontTag, 63, f ); actWin->incLine();
    readStringFromFile( label, 63, f ); actWin->incLine();
    actWin->fi->loadFontTag( fontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    stringLength = strlen( label );
    updateFont( label, fontTag, &fs,
     &fontAscent, &fontDescent, &fontHeight,
     &stringWidth );
  }
  else {
    strcpy( label, "" );
    stringLength = 0;
    strcpy( fontTag, actWin->defaultFontTag );
    fs = actWin->fi->getXFontStruct( fontTag );
    updateFont( label, fontTag, &fs,
     &fontAscent, &fontDescent, &fontHeight,
     &stringWidth );
  }

  if ( ( major > 1 ) || ( minor > 4 ) ) {
    fscanf( f, "%d\n", &alignment ); actWin->incLine();
  }
  else {
    alignment = XmALIGNMENT_BEGINNING;
  }

  if ( alignment == XmALIGNMENT_BEGINNING )
    labelX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    labelX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    labelX = x + w - stringWidth;
  labelY = y + h;

  boxH = h - fontHeight;
  if ( boxH < 5 ) {
    boxH = 5;
    h = boxH + fontHeight;
  }

  if ( readMax > readMin ) {
    factorW = (double) w / ( readMax - readMin );
    factorH = (double) boxH / ( readMax - readMin );
  }
  else {
    factorW = 1;
    factorH = 1;
  }

  centerX = x + (int) ( w * 0.5 + 0.5 );
  centerY = y + (int) ( boxH * 0.5 + 0.5 );

  curValue = 0;

  return 1;

}

int edmBoxClass::save (
  FILE *f )
{

int index, stat;

  fprintf( f, "%-d %-d %-d\n", EBC_MAJOR_VERSION, EBC_MINOR_VERSION,
   EBC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = lineColor.pixelIndex();
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", lineColorMode );

  fprintf( f, "%-d\n", fill );

  index = fillColor.pixelIndex();
  fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", fillColorMode );

  if ( pvExpStr.getRaw() )
    writeStringToFile( f, pvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  fprintf( f, "%-d\n", lineWidth );

  fprintf( f, "%-d\n", lineStyle );

  stat = efReadMin.write( f );
  stat = efReadMax.write( f );

  writeStringToFile( f, fontTag );

  writeStringToFile( f, label );

  fprintf( f, "%-d\n", alignment );

  return 1;

}

int edmBoxClass::drawActive ( void ) {

  if ( !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  if ( fill ) {
    actWin->executeGc.setFG( fillColor.getColor() );
    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), sideX, sideY, sideW, sideH );
  }

  actWin->executeGc.setFG( lineColor.getColor() );
  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), sideX, sideY, sideW, sideH );

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  if ( bufferInvalid ) {

    if ( strcmp( fontTag, "" ) != 0 ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
    }

    XDrawStrings( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), labelX, labelY, fontHeight,
     label, strlen(label) );

    bufferInvalid = 0;

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int edmBoxClass::eraseActive ( void ) {

  if ( !activeMode || !init ) return 1;

  if ( fill ) {
    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), sideX, sideY, sideW, sideH );
  }

  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), sideX, sideY, sideW, sideH );

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  if ( bufferInvalid ) {

    if ( strcmp( fontTag, "" ) != 0 ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
    }

    XDrawStrings( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), labelX, labelY, fontHeight,
     label, strlen(label) );

  }

  return 1;

}

void edmBoxClass::bufInvalidate ( void )
{

  bufferInvalid = 1;

}

int edmBoxClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = pvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int edmBoxClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = pvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int edmBoxClass::containsMacros ( void ) {

  if ( pvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int edmBoxClass::activate (
  int pass,
  void *ptr )
{

int stat;

  switch ( pass ) {

  case 1: // initialize

    aglPtr = ptr;
    needConnectInit = needInfoInit = needUpdate = needDraw = 0;
    init = 0;
    opComplete = 0;
    eventId = alarmEventId = infoEventId = 0;
    active = 0;
    activeMode = 1;
    bufferInvalid = 0;
    pointerMotionDetected = 0;

    if ( !pvExpStr.getExpanded() ||
       ( strcmp( pvExpStr.getExpanded(), "" ) == 0 ) ) {
      pvExists = 0;
    }
    else {
      pvExists = 1;
      lineColor.setConnectSensitive();
      fillColor.setConnectSensitive();
    }

    break;

  case 2: // connect to pv

    if ( !opComplete ) {

      if ( pvExists ) {
        stat = ca_search_and_connect( pvExpStr.getExpanded(), &pvId,
         eboMonitorPvConnectState, this );
        if ( stat != ECA_NORMAL ) {
          printf( edmBoxComplete_str21 );
          return 0;
        }
      }
      else {
        active = 1;
        init = 1;
        sideW = w;
        sideX = x;
        sideH = boxH;
        sideY = y;
        stat = drawActive();
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

int edmBoxClass::deactivate (
  int pass )
{

int stat;

  if ( pass == 1 ) {

  activeMode = 0;

  if ( eventId ) {
    stat = ca_clear_event( eventId );
    if ( stat != ECA_NORMAL )
      printf( edmBoxComplete_str22 );
  }

  if ( pvExists ) {
    stat = ca_clear_channel( pvId );
    if ( stat != ECA_NORMAL )
      printf( edmBoxComplete_str23 );
  }

  }

  return 1;

}

int edmBoxClass::draw ( void ) {

  if ( activeMode || deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  if ( fill ) {
    actWin->drawGc.setFG( fillColor.pixelColor() );
    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x, y, w, boxH );
  }

  actWin->drawGc.setFG( lineColor.pixelColor() );
  actWin->drawGc.setLineWidth( lineWidth );
  actWin->drawGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x, y, w, boxH );

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  XDrawStrings( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), labelX, labelY, fontHeight,
   label, strlen(label) );

  actWin->drawGc.restoreFg();

  return 1;

}

int edmBoxClass::erase ( void ) {

  if ( activeMode || deleteRequest ) return 1;

  if ( fill ) {
    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x, y, w, boxH );
  }

  actWin->drawGc.setLineWidth( lineWidth );
  actWin->drawGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, boxH );

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  XDrawStrings( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), labelX, labelY, fontHeight,
   label, strlen(label) );

  return 1;

}

int edmBoxClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, tmpBoxH, ret_stat;

  ret_stat = 1;

  tmpw = sboxW;
  tmph = sboxH;

  tmpw += _w;
  tmph += _h;

  tmpBoxH = tmph - fontHeight;
  if ( tmpBoxH < 5 ) ret_stat = 0;

  if ( tmpw < 5 ) ret_stat = 0;

  return ret_stat;

}

int edmBoxClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

int tmpw, tmph, tmpBoxH, ret_stat;

  ret_stat = 1;

  tmpw = _w;
  tmph = _h;

  if ( tmph != -1 ) {
    tmpBoxH = tmph - fontHeight;
    if ( tmpBoxH < 5 ) ret_stat = 0;
  }

  if ( tmpw != -1 ) {
    if ( tmpw < 5 ) ret_stat = 0;
  }

  return ret_stat;

}

void edmBoxClass::updateDimensions ( void ) {

  if ( alignment == XmALIGNMENT_BEGINNING )
    labelX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    labelX = x + w/2 - stringWidth/2;
  else if ( alignment == XmALIGNMENT_END )
    labelX = x + w - stringWidth;

  labelY = y + h;

  boxH = h - fontHeight;
  if ( boxH < 5 ) {
    boxH = 5;
    h = boxH + fontHeight;
    sboxH = h;
  }

  if ( w < 5 ) {
    w = 5;
    sboxW = 5;
  }

}

void edmBoxClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

double v;
int stat;

  *action = 0;

  if ( pvExists && !pointerMotionDetected ) {

    if ( buttonState & ShiftMask ) {
      v = curValue - 10.0;
      if ( v < readMin ) v = readMin;
    }
    else {
      v = curValue + 10.0;
      if ( v > readMax ) v = readMax;
    }

#ifdef __epics__

    stat = ca_put( DBR_DOUBLE, pvId, &v );

#endif

  }

}

void edmBoxClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  pointerMotionDetected = 0;

}


void edmBoxClass::btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

double v;
int stat;

 pointerMotionDetected = 1;

  if ( pvExists ) {

    if ( buttonState & ShiftMask ) {
      v = curValue - 1.0;
      if ( v < readMin ) v = readMin;
    }
    else {
      v = curValue + 1.0;
      if ( v > readMax ) v = readMax;
    }

#ifdef __epics__

    stat = ca_put( DBR_DOUBLE, pvId, &v );

#endif

  }

}

int edmBoxClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 1;
  *down = 1;
  *up = 1;

  if ( pvExists )
    *focus = 1;
  else
    *focus = 0;

  return 1;

}

void edmBoxClass::executeDeferred ( void ) {

int stat, nc, ni, nu, nd;
double value;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  if ( !activeMode ) {
    actWin->remDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return;
  }

  value = curValue;
  nc = needConnectInit; needConnectInit = 0;
  ni = needInfoInit; needInfoInit = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  actWin->remDefExeNode( aglPtr );

  actWin->appCtx->proc->unlock();


//----------------------------------------------------------------------------

  if ( nc ) {

    // require process variable to be numeric
    if ( ( fieldType == DBR_ENUM ) ||
         ( fieldType == DBR_INT ) ||
         ( fieldType == DBR_LONG ) ||
         ( fieldType == DBR_FLOAT ) ||
         ( fieldType == DBR_DOUBLE ) ) {

      if ( !infoEventId ) {
        stat = ca_get_callback( DBR_GR_DOUBLE, pvId,
         edmBoxInfoUpdate, (void *) this );
        if ( stat != ECA_NORMAL ) {
          printf( edmBoxComplete_str24 );
        }
      }

    }
    else { // force a draw in the non-active state & post error message

      actWin->appCtx->postMessage(
       edmBoxComplete_str25 );

      active = 0;
      init = 1;
      sideW = w;
      sideX = x;
      sideH = boxH;
      sideY = y;
      lineColor.setDisconnected();
      fillColor.setDisconnected();
#if SMARTDRAW
      smartDrawAllActive();
#else
      drawActive();
#endif

    }

  }


//----------------------------------------------------------------------------

  if ( ni ) {

    stat = eraseActive();

    if ( readMax > readMin ) {
      factorW = (double) w / ( readMax - readMin );
      factorH = (double) boxH / ( readMax - readMin );
    }
    else {
      factorW = 1;
      factorH = 1;
    }

    centerX = x + (int) ( w * 0.5 + 0.5 );
    centerY = y + (int) ( boxH * 0.5 + 0.5 );

    if ( value > 0.0 ) {
      sideW = (int) ( value * factorW + 0.5 );
      sideX = centerX - (int) ( (double) sideW * 0.5 + 0.5 );
      sideH = (int) ( value * factorH + 0.5 );
      sideY = centerY - (int) ( (double) sideH * 0.5 + 0.5 );
    }
    else {
      sideW = 1;
      sideX = centerX;
      sideH = 1;
      sideY = centerY;
    }

    if ( !eventId ) {
      stat = ca_add_masked_array_event( DBR_DOUBLE, 1, pvId,
       edmBoxUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &eventId, DBE_VALUE );
      if ( stat != ECA_NORMAL ) {
        printf( edmBoxComplete_str26 );
      }
    }

    if ( !alarmEventId ) {
      stat = ca_add_masked_array_event( DBR_STS_DOUBLE, 1, pvId,
       edmBoxAlarmUpdate, (void *) this, (float) 0.0, (float) 0.0,
       (float) 0.0, &alarmEventId, DBE_ALARM );
      if ( stat != ECA_NORMAL ) {
        printf( edmBoxComplete_str26 );
      }
    }

    init = 1;
    active = 1;
    lineColor.setConnected();
    fillColor.setConnected();

    bufInvalidate();

#if SMARTDRAW
    smartDrawAllActive();
#else
    drawActive();
#endif

  }


//----------------------------------------------------------------------------

  if ( nu ) {

    eraseActive();

    if ( value > 0.0 ) {
      sideW = (int) ( value * factorW + 0.5 );
      sideX = centerX - (int) ( (double) sideW * 0.5 + 0.5 );
      sideH = (int) ( value * factorH + 0.5 );
      sideY = centerY - (int) ( (double) sideH * 0.5 + 0.5 );
    }
    else {
      sideW = 1;
      sideX = centerX;
      sideH = 1;
      sideY = centerY;
    }

#if SMARTDRAW
    smartDrawAllActive();
#else
    drawActive();
#endif

  }


//----------------------------------------------------------------------------

  if ( nd ) {

#if SMARTDRAW
    smartDrawAllActive();
#else
    drawActive();
#endif

  }

}

char *edmBoxClass::firstDragName ( void ) {

  return dragName[dragIndex];

}

char *edmBoxClass::nextDragName ( void ) {

  return NULL;

}

char *edmBoxClass::dragValue (
  int i ) {

  return pvExpStr.getExpanded();

}

void edmBoxClass::changeDisplayParams (
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
    fillColor.setColorIndex( _bgColor, actWin->ci );

  if ( _flag & ACTGRF_CTLFONTTAG_MASK ) {

    strcpy( fontTag, _ctlFontTag );
    alignment = _ctlAlignment;

    fs = actWin->fi->getXFontStruct( fontTag );
    updateFont( " ", fontTag, &fs,
     &fontAscent, &fontDescent, &fontHeight,
     &stringWidth );

    boxH = h - fontHeight;
    if ( boxH < 5 ) {
      boxH = 5;
      h = boxH + fontHeight;
      sboxH = h;
    }

    if ( w < 5 ) {
      w = 5;
      sboxW = 5;
    }

  }

}

void edmBoxClass::changePvNames (
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
      pvExpStr.setRaw( ctlPvs[0] );
    }
  }

}

extern "C" {

void *create_2ed7d2e8_f439_11d2_8fed_00104b8742dfPtr ( void ) {

edmBoxClass *ptr;

  ptr = new edmBoxClass;
  return (void *) ptr;

}

void *clone_2ed7d2e8_f439_11d2_8fed_00104b8742dfPtr (
  void *_srcPtr )
{

edmBoxClass *ptr, *srcPtr;

  srcPtr = (edmBoxClass *) _srcPtr;

  ptr = new edmBoxClass( srcPtr );

  return (void *) ptr;

}

}
