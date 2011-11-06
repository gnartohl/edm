//  for object: 458bb765_eab9_4d65_8fda_2ce55d2baec6
//  for lib: ?

#define __avScale_cc 1

#include "avScale.h"
#include "app_pkg.h"
#include "act_win.h"

static void sclc_value_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scaleClass *sclo = (scaleClass *) client;

  sclo->min = sclo->bufMin;
  sclo->max = sclo->bufMax;
  sclo->needUpdate = 1;
  sclo->actWin->addDefExeNode( sclo->aglPtr );

}

static void sclc_value_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scaleClass *sclo = (scaleClass *) client;

  sclc_value_edit_apply ( w, client, call );
  sclo->textEntry.popdown();
  sclo->editDialogIsActive = 0;

}

static void sclc_value_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scaleClass *sclo = (scaleClass *) client;

  sclo->textEntry.popdown();
  sclo->editDialogIsActive = 0;

}


static void scloMonitorPvConnectState (
  ProcessVariable *pv,
  void *userArg )
{

scaleClass *sclo = (scaleClass *) userArg;

  sclo->actWin->appCtx->proc->lock();

  if ( !sclo->activeMode ) {
    sclo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( pv->is_valid() ) {

    if ( pv == sclo->minPv ) {
      sclo->minFieldType = pv->get_type().type;
      sclo->bufMin = pv->get_double();
    }
    else if ( pv == sclo->maxPv ) {
      sclo->maxFieldType = pv->get_type().type;
      sclo->bufMax = pv->get_double();
    }
    else if ( pv == sclo->colorPv ) {
      sclo->colorFieldType = pv->get_type().type;
      sclo->colorIndex = pv->get_int();
    }
    else if ( pv == sclo->modePv ) {
      sclo->modeFieldType = pv->get_type().type;
      sclo->mode = pv->get_int();
    }
    else if ( pv == sclo->labelPv ) {
      sclo->labelFieldType = pv->get_type().type;
      pv->get_string( sclo->newLabel, 39 );
    }
    else if ( pv == sclo->updatePv ) {
      sclo->updateFieldType = pv->get_type().type;
      sclo->update = pv->get_int();
    }

  }
  else { // lost connection

    sclo->connection.setPvDisconnected( (void *) pv );
    sclo->active = 0;
    sclo->lineColor.setDisconnected();
    sclo->bufInvalidate();
    sclo->needDraw = 1;
    sclo->actWin->addDefExeNode( sclo->aglPtr );

  }

  sclo->actWin->appCtx->proc->unlock();

}

static void scaleUpdate (
  ProcessVariable *pv,
  void *userArg )
{

class scaleClass *sclo = (scaleClass *) userArg;

  sclo->actWin->appCtx->proc->lock();

  if ( !sclo->activeMode ) {
    sclo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( pv == sclo->minPv ) {
    if ( sclo->mode == scaleClass::modeLog ) {
      sclo->bufMin = pow( 10.0, pv->get_double() );
    }
    else {
      sclo->bufMin = pv->get_double();
    }
  }
  else if ( pv == sclo->maxPv ) {
    if ( sclo->mode == scaleClass::modeLog ) {
      sclo->bufMax = pow( 10.0, pv->get_double() );
    }
    else {
      sclo->bufMax = pv->get_double();
    }
  }
  else if ( pv == sclo->colorPv ) {
    sclo->colorIndex = pv->get_int();
  }
  else if ( pv == sclo->modePv ) {
    sclo->newMode = pv->get_int();
    sclo->needModeUpdate = 1;
    sclo->actWin->addDefExeNode( sclo->aglPtr );
  }
  else if ( pv == sclo->labelPv ) {
    pv->get_string( sclo->newLabel, 39 );
  }
  else if ( pv == sclo->updatePv ) {
    sclo->update = pv->get_int();
    if ( ( sclo->update == sclo->scaleId ) ||
         ( sclo->update == sclo->updateAllScales ) ||
         ( sclo->update == sclo->updateAll ) ) {
      sclo->needUpdate = 1;
      sclo->actWin->addDefExeNode( sclo->aglPtr );
      /* fprintf( stderr, "got scale update, min=%-g, max=%-g\n", sclo->min, sclo->max ); */
    }
  }

  if ( !sclo->connection.pvsConnected() ) {
    sclo->connection.setPvConnected( (void *) pv );
    if ( sclo->connection.pvsConnected() ) {
      sclo->lineColor.setConnected();
      sclo->needConnectInit = 1;
      sclo->actWin->addDefExeNode( sclo->aglPtr );
    }
  }

  sclo->actWin->appCtx->proc->unlock();

}

static void sclc_edit_update (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scaleClass *sclo = (scaleClass *) client;

  sclo->actWin->setChanged();

  sclo->eraseSelectBoxCorners();
  sclo->erase();

  sclo->minPvExpStr.setRaw( sclo->eBuf->minBufPvName );
  sclo->maxPvExpStr.setRaw( sclo->eBuf->maxBufPvName );
  sclo->colorPvExpStr.setRaw( sclo->eBuf->colorBufPvName );
  sclo->modePvExpStr.setRaw( sclo->eBuf->modeBufPvName );
  sclo->labelPvExpStr.setRaw( sclo->eBuf->labelBufPvName );
  sclo->updatePvExpStr.setRaw( sclo->eBuf->updateBufPvName );

  sclo->lineColor.setColorIndex( sclo->eBuf->bufLineColor, sclo->actWin->ci );

  sclo->scaleId = sclo->eBuf->bufScaleId;

  strncpy( sclo->fontTag, sclo->fm.currentFontTag(), 63 );
  sclo->fontTag[63] = 0;

  sclo->actWin->fi->loadFontTag( sclo->fontTag );

  sclo->fs = sclo->actWin->fi->getXFontStruct( sclo->fontTag );

  sclo->updateFont( " ", sclo->fontTag, &sclo->fs,
   &sclo->fontAscent, &sclo->fontDescent, &sclo->fontHeight,
   &sclo->stringWidth );

  sclo->x = sclo->bufX;
  sclo->sboxX = sclo->bufX;

  sclo->y = sclo->bufY;
  sclo->sboxY = sclo->bufY;

  sclo->w = sclo->bufW;
  sclo->sboxW = sclo->bufW;

  sclo->h = sclo->bufH;
  sclo->sboxH = sclo->bufH;

  if ( sclo->w < 5 ) {
    sclo->w = 5;
    sclo->sboxW = sclo->w;
  }

  if ( sclo->h < 5 ) {
    sclo->h = 5;
  }

  if ( sclo->w >= sclo->h )
    sclo->horizontal = 1;
  else
    sclo->horizontal = 0;

}

static void sclc_edit_apply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scaleClass *sclo = (scaleClass *) client;

  sclc_edit_update( w, client, call );
  sclo->refresh( sclo );

}

static void sclc_edit_ok (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scaleClass *sclo = (scaleClass *) client;

  sclc_edit_update( w, client, call );
  sclo->ef.popdown();
  sclo->operationComplete();

}

static void sclc_edit_cancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scaleClass *sclo = (scaleClass *) client;

  sclo->ef.popdown();
  sclo->operationCancel();

}

static void sclc_edit_cancel_delete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

scaleClass *sclo = (scaleClass *) client;

  sclo->ef.popdown();
  sclo->operationCancel();
  sclo->erase();
  sclo->deleteRequest = 1;
  sclo->drawAll();

}

// default constructor
scaleClass::scaleClass ( void ) {

  name = new char[strlen("458bb765_eab9_4d65_8fda_2ce55d2baec6")+1];
  strcpy( name, "458bb765_eab9_4d65_8fda_2ce55d2baec6" );
  checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
  activeMode = 0;
  scaleOfs = scaleLen = 0;
  horizontal = 1;
  connection.setMaxPvs( 6 );
  numMajTicks = 2;
  numMinTicks = 0;
  min = 0.0;
  max = 10.0;
  adj_min = min;
  adj_max = max;
  label_tick = ( max - min ) / 2;
  major_tick = label_tick / 2;
  minor_tick = label_tick / 2;
  strcpy( format, "g" );
  strcpy( label, "" );
  mode = modeLinear;
  scaleId = 0;

  numLabTicks = 1;
  numMajTicks = 1;
  numMinTicks = 1;

  eBuf = NULL;

}

// copy constructor
scaleClass::scaleClass
 ( const scaleClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  ago->clone( (activeGraphicClass *) source );

  name = new char[strlen("458bb765_eab9_4d65_8fda_2ce55d2baec6")+1];
  strcpy( name, "458bb765_eab9_4d65_8fda_2ce55d2baec6" );
  activeMode = 0;
  scaleOfs = scaleLen = 0;
  connection.setMaxPvs( 6 );
  numMajTicks = 2;
  numMinTicks = 0;
  min = 0.0;
  max = 10.0;
  adj_min = min;
  adj_max = max;
  label_tick = ( max - min ) / 2;
  major_tick = label_tick / 2;
  minor_tick = label_tick / 2;
  strcpy( format, "g" );
  strcpy( label, "" );
  mode = modeLinear;

  strcpy( fontTag, source->fontTag );

  actWin->fi->loadFontTag( fontTag );

  fs = actWin->fi->getXFontStruct( fontTag );

  updateFont( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  horizontal = source->horizontal;
  lineColor.copy(source->lineColor);
  minPvExpStr.setRaw( source->minPvExpStr.rawString );
  maxPvExpStr.setRaw( source->maxPvExpStr.rawString );
  colorPvExpStr.setRaw( source->colorPvExpStr.rawString );
  modePvExpStr.setRaw( source->modePvExpStr.rawString );
  labelPvExpStr.setRaw( source->labelPvExpStr.rawString );
  updatePvExpStr.setRaw( source->updatePvExpStr.rawString );
  scaleId = source->scaleId;

  numLabTicks = 1;
  numMajTicks = 1;
  numMinTicks = 1;

  eBuf = NULL;

}

scaleClass::~scaleClass ( void ) {

  if ( name ) delete[] name;

  if ( eBuf ) delete eBuf;

}

char *scaleClass::objName ( void ) {

  return name;

}

int scaleClass::createInteractive (
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

  strcpy( fontTag, actWin->defaultCtlFontTag );

  actWin->fi->loadFontTag( fontTag );

  fs = actWin->fi->getXFontStruct( fontTag );

  updateFont( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  this->draw();

  this->editCreate();

  return 1;

}

int scaleClass::genericEdit ( void ) {

char title[32], *ptr;

  if ( !eBuf ) {
    eBuf = new editBufType;
  }

  ptr = actWin->obj.getNameFromClass( "458bb765_eab9_4d65_8fda_2ce55d2baec6" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, scale_str2, 31 );

  Strncat( title, scale_str3, 31 );

  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  eBuf->bufLineColor = lineColor.pixelIndex();

  eBuf->bufScaleId = scaleId;

  if ( minPvExpStr.getRaw() )
    strncpy( eBuf->minBufPvName, minPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->minBufPvName, "" );

 if ( maxPvExpStr.getRaw() )
   strncpy( eBuf->maxBufPvName, maxPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->maxBufPvName, "" );

 if ( colorPvExpStr.getRaw() )
   strncpy( eBuf->colorBufPvName, colorPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->colorBufPvName, "" );

 if ( modePvExpStr.getRaw() )
   strncpy( eBuf->modeBufPvName, modePvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->modeBufPvName, "" );

 if ( labelPvExpStr.getRaw() )
   strncpy( eBuf->labelBufPvName, labelPvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->labelBufPvName, "" );

 if ( updatePvExpStr.getRaw() )
   strncpy( eBuf->updateBufPvName, updatePvExpStr.getRaw(),
    PV_Factory::MAX_PV_NAME );
  else
    strcpy( eBuf->updateBufPvName, "" );

  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  ef.addTextField( scale_str4, 35, &bufX );
  ef.addTextField( scale_str5, 35, &bufY );
  ef.addTextField( scale_str6, 35, &bufW );
  ef.addTextField( scale_str7, 35, &bufH );
  ef.addTextField( scale_str16, 35, &eBuf->bufScaleId );
  ef.addTextField( scale_str8, 35, eBuf->minBufPvName, PV_Factory::MAX_PV_NAME );
  ef.addTextField( scale_str9, 35, eBuf->maxBufPvName, PV_Factory::MAX_PV_NAME );
  ef.addTextField( scale_str10, 35, eBuf->colorBufPvName, PV_Factory::MAX_PV_NAME );
  ef.addTextField( scale_str11, 35, eBuf->modeBufPvName, PV_Factory::MAX_PV_NAME );
  ef.addTextField( scale_str12, 35, eBuf->labelBufPvName, PV_Factory::MAX_PV_NAME );
  ef.addTextField( scale_str13, 35, eBuf->updateBufPvName, PV_Factory::MAX_PV_NAME );
  ef.addColorButton( scale_str14, actWin->ci, &eBuf->lineCb,
   &eBuf->bufLineColor );
  ef.addFontMenu( scale_str15, actWin->fi, &fm, fontTag );

  XtUnmanageChild( fm.alignWidget() ); // no alignment info

  return 1;

}

int scaleClass::editCreate ( void ) {

  this->genericEdit();

  ef.finished( sclc_edit_ok, sclc_edit_apply, sclc_edit_cancel_delete, this );

  actWin->currentEf = NULL;
  ef.popup();

  return 1;

}

int scaleClass::edit ( void ) {

  this->genericEdit();
  ef.finished( sclc_edit_ok, sclc_edit_apply, sclc_edit_cancel, this );
  actWin->currentEf = &ef;
  ef.popup();

  return 1;

}

int scaleClass::createFromFile (
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
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "id", &scaleId, &zero );
  tag.loadR( "fgColor", actWin->ci, &lineColor );
  tag.loadR( "minPv", &minPvExpStr, emptyStr );
  tag.loadR( "maxPv", &maxPvExpStr, emptyStr );
  tag.loadR( "colorPv", &colorPvExpStr, emptyStr );
  tag.loadR( "modePv", &modePvExpStr, emptyStr );
  tag.loadR( "labelPv", &labelPvExpStr, emptyStr );
  tag.loadR( "updatePv", &updatePvExpStr, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > SCLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if ( w >= h )
    horizontal = 1;
  else
    horizontal = 0;

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  updateFont( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  return stat;

}

int scaleClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int index;
int major, minor, release;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > SCLC_MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( w >= h )
    horizontal = 1;
  else
    horizontal = 0;

  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 1 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

  }
  else {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

  }

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  minPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  maxPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  colorPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  modePvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  labelPvExpStr.setRaw( oneName );

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
  updatePvExpStr.setRaw( oneName );

  readStringFromFile( fontTag, 63+1, f ); actWin->incLine();

  // version 1.1
  if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 0 ) ) ) {
    fscanf( f, "%d\n", &scaleId ); actWin->incLine();
  }
  else {
    scaleId = 0;
  }

  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  updateFont( " ", fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  return 1;

}

int scaleClass::save (
  FILE *f )
{


int stat, major, minor, release;
tagClass tag;

int zero = 0;
char *emptyStr = "";

  major = SCLC_MAJOR_VERSION;
  minor = SCLC_MINOR_VERSION;
  release = SCLC_RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "font", fontTag );
  tag.loadW( "id", &scaleId, &zero );
  tag.loadW( "fgColor", actWin->ci, &lineColor );
  tag.loadW( "minPv", &minPvExpStr, emptyStr );
  tag.loadW( "maxPv", &maxPvExpStr, emptyStr );
  tag.loadW( "colorPv", &colorPvExpStr, emptyStr );
  tag.loadW( "modePv", &modePvExpStr, emptyStr );
  tag.loadW( "labelPv", &labelPvExpStr, emptyStr );
  tag.loadW( "updatePv", &updatePvExpStr, emptyStr );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

int scaleClass::old_save (
  FILE *f )
{

int index;

  fprintf( f, "%-d %-d %-d\n", SCLC_MAJOR_VERSION, SCLC_MINOR_VERSION,
   SCLC_RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = lineColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  if ( minPvExpStr.getRaw() )
    writeStringToFile( f, minPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( maxPvExpStr.getRaw() )
    writeStringToFile( f, maxPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( colorPvExpStr.getRaw() )
    writeStringToFile( f, colorPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( modePvExpStr.getRaw() )
    writeStringToFile( f,  modePvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( labelPvExpStr.getRaw() )
    writeStringToFile( f, labelPvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  if ( updatePvExpStr.getRaw() )
    writeStringToFile( f, updatePvExpStr.getRaw() );
  else
    writeStringToFile( f, "" );

  writeStringToFile( f, fontTag );

  fprintf( f, "%-d\n", scaleId );

  return 1;

}

int scaleClass::drawActive ( void ) {

int inc, stringX, stringY, stat, l;
 unsigned int i;
char fullName[127+1];

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( lineColor.getColor() );
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setFontTag( fontTag, actWin->fi );

  if ( horizontal ) {

    drawXLinearScale( actWin->executeWidget, drawable(actWin->executeWidget),
     &actWin->executeGc, 0 );
    drawXLinearAnnotation( actWin->executeWidget, drawable(actWin->executeWidget),
     &actWin->executeGc, 0 );

    if ( strcmp( fontTag, "" ) != 0 ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
    }
    drawText( actWin->executeWidget, drawable(actWin->executeWidget),
     &actWin->executeGc, fs,
     x+w/2, (int) (y+h-1.1*fontHeight), XmALIGNMENT_CENTER, label );

  }
  else {

    if ( ( mode == modeLinear ) || ( mode == modeHours ) ) {
      ::drawYLinearScale( actWin->d, drawable(actWin->executeWidget),
       &actWin->executeGc, 1, x+w, y+scaleOfs+scaleLen, scaleLen, adj_min,
       adj_max, numLabTicks, numMajTicks, numMinTicks,
       lineColor.getColor(), actWin->executeGc.getBaseBG(),
       0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 0 );
    }
    else if ( mode == modeLog ) {
      ::drawYLog10Scale( actWin->d, drawable(actWin->executeWidget),
       &actWin->executeGc, 1, x+w, y+scaleOfs+scaleLen, scaleLen, adj_min,
       adj_max, numLabTicks, numMajTicks, numMinTicks,
       lineColor.getColor(), actWin->executeGc.getBaseBG(),
       0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 0 );
    }

    stat = actWin->fi->getFontName( fontTag, 90.0, fullName, 127 );

    //fprintf( stderr, "fullName = [%s]\n", fullName );

    actWin->executeGc.setNativeFont( fullName, actWin->fi );

    l = XTextWidth( fs, label, strlen(label) );

    stringX = x + (int) ( 1.0 * (double) fontHeight );
    stringY = y + scaleOfs + scaleLen/2 + l/2;

    //fprintf( stderr, "stringX = %-d\n", stringX );
    //fprintf( stderr, "stringY = %-d\n", stringY );
    //fprintf( stderr, "label = [%s]\n", label );

    for ( i=0; i<strlen(label); i++ ) {

      XDrawString( XtDisplay(actWin->executeWidget),
       drawable(actWin->executeWidget), actWin->executeGc.normGC(),
       (int) stringX, stringY, &label[i], 1 );

      inc = 2 + XTextWidth( fs, &label[i], 1 );
      stringY -= inc;

    }

  }

  actWin->executeGc.restoreFg();

  return 1;

}

int scaleClass::eraseActive ( void ) {

int inc, stringX, stringY, stat, l;
 unsigned int i;
char fullName[127+1];

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  actWin->executeGc.setFG( lineColor.getColor() );
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );
  actWin->executeGc.setFontTag( fontTag, actWin->fi );

  if ( horizontal ) {

    drawXLinearScale( actWin->executeWidget, drawable(actWin->executeWidget),
     &actWin->executeGc, 1 );
    drawXLinearAnnotation( actWin->executeWidget, drawable(actWin->executeWidget),
     &actWin->executeGc, 1 );

    if ( strcmp( fontTag, "" ) != 0 ) {
      actWin->executeGc.setFontTag( fontTag, actWin->fi );
    }
    eraseText( actWin->executeWidget, drawable(actWin->executeWidget),
     &actWin->executeGc, fs,
     x+w/2, (int) (y+h-1.1*fontHeight), XmALIGNMENT_CENTER, label );

  }
  else {

   if ( ( mode == modeLinear ) || ( mode == modeHours ) ) {
      ::drawYLinearScale( actWin->d, drawable(actWin->executeWidget),
       &actWin->executeGc, 1, x+w, y+scaleOfs+scaleLen, scaleLen, adj_min,
       adj_max, numLabTicks, numMajTicks, numMinTicks,
       lineColor.getColor(), actWin->executeGc.getBaseBG(),
       0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 1 );
    }
    else if ( mode == modeLog ) {
      ::drawYLog10Scale( actWin->d, drawable(actWin->executeWidget),
       &actWin->executeGc, 1, x+w, y+scaleOfs+scaleLen, scaleLen, adj_min,
       adj_max, numLabTicks, numMajTicks, numMinTicks,
       lineColor.getColor(), actWin->executeGc.getBaseBG(),
       0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 1 );
    }

    stat = actWin->fi->getFontName( fontTag, 90.0, fullName, 127 );
    actWin->executeGc.setNativeFont( fullName, actWin->fi );

    l = XTextWidth( fs, label, strlen(label) );

    stringX = x + (int) ( 1.0 * (double) fontHeight );
    stringY = y + scaleOfs + scaleLen/2 + l/2;

    for ( i=0; i<strlen(label); i++ ) {

      XDrawString( XtDisplay(actWin->executeWidget),
       drawable(actWin->executeWidget), actWin->executeGc.eraseGC(),
       (int) stringX, stringY, &label[i], 1 );

      inc = 2 + XTextWidth( fs, &label[i], 1 );
      stringY -= inc;

    }

  }

  actWin->executeGc.restoreFg();

  return 1;

}

void scaleClass::bufInvalidate ( void )
{

  bufferInvalid = 1;

}

int scaleClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

expStringClass tmpStr;

  tmpStr.setRaw( minPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  minPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( maxPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  maxPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( colorPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  colorPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( modePvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  modePvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( labelPvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  labelPvExpStr.setRaw( tmpStr.getExpanded() );

  tmpStr.setRaw( updatePvExpStr.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  updatePvExpStr.setRaw( tmpStr.getExpanded() );

  return 1;

}

int scaleClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = minPvExpStr.expand1st( numMacros, macros, expansions );
  stat = maxPvExpStr.expand1st( numMacros, macros, expansions );
  stat = colorPvExpStr.expand1st( numMacros, macros, expansions );
  stat = modePvExpStr.expand1st( numMacros, macros, expansions );
  stat = labelPvExpStr.expand1st( numMacros, macros, expansions );
  stat = updatePvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

int scaleClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = minPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = maxPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = colorPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = modePvExpStr.expand2nd( numMacros, macros, expansions );
  stat = labelPvExpStr.expand2nd( numMacros, macros, expansions );
  stat = updatePvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

int scaleClass::containsMacros ( void ) {

  if ( minPvExpStr.containsPrimaryMacros() ) return 1;
  if ( maxPvExpStr.containsPrimaryMacros() ) return 1;
  if ( colorPvExpStr.containsPrimaryMacros() ) return 1;
  if ( modePvExpStr.containsPrimaryMacros() ) return 1;
  if ( labelPvExpStr.containsPrimaryMacros() ) return 1;
  if ( updatePvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

int scaleClass::activate (
  int pass,
  void *ptr )
{

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2: // connect to pv

    if ( !opComplete ) {

      connection.init();
      initEnable();

      bufX = x;
      bufY = y;
      bufW = w;
      bufH = h;

      if ( max <= min ) max = min + 1.0;
      adj_min = min;
      adj_max = max;
      label_tick = ( max - min ) / 2;
      major_tick = label_tick / 2;
      minor_tick = label_tick / 2;
      numLabTicks = 1;
      numMajTicks = 1;
      numMinTicks = 1;
      strcpy( format, "g" );

      aglPtr = ptr;
      needConnectInit = needUpdate = needModeUpdate = needDraw = needErase = 0;
      init = 0;
      active = 0;
      activeMode = 1;
      bufferInvalid = 1;
      pvsExist = 1;

      minPv = maxPv = colorPv = modePv = labelPv = updatePv = NULL;

      editDialogIsActive = 0;

      if ( !minPvExpStr.getExpanded() ||
         // ( strcmp( minPvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( minPvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !maxPvExpStr.getExpanded() ||
         // ( strcmp( maxPvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( maxPvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !colorPvExpStr.getExpanded() ||
         // ( strcmp( colorPvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( colorPvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !modePvExpStr.getExpanded() ||
         // ( strcmp( modePvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( modePvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !labelPvExpStr.getExpanded() ||
         // ( strcmp( labelPvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( labelPvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !updatePvExpStr.getExpanded() ||
         // ( strcmp( updatePvExpStr.getExpanded(), "" ) == 0 ) ) {
	 blankOrComment( updatePvExpStr.getExpanded() ) ) {
        pvsExist = 0;
      }
      connection.addPv();

      if ( !pvsExist ) { // disable active mode behavior
        activeMode = 0;
        opComplete = 1;
        break;
      }

      lineColor.setConnectSensitive();

      minPv = the_PV_Factory->create( minPvExpStr.getExpanded() );
      if ( minPv ) {
        if ( minPv->is_valid() ) {
          scloMonitorPvConnectState( minPv, this );
          scaleUpdate( minPv, this );
	}
        minPv->add_conn_state_callback( scloMonitorPvConnectState, this );
        minPv->add_value_callback( scaleUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      maxPv = the_PV_Factory->create( maxPvExpStr.getExpanded() );
      if ( maxPv ) {
        if ( maxPv->is_valid() ) {
          scloMonitorPvConnectState( maxPv, this );
          scaleUpdate( maxPv, this );
	}
        maxPv->add_conn_state_callback( scloMonitorPvConnectState, this );
        maxPv->add_value_callback( scaleUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      colorPv = the_PV_Factory->create( colorPvExpStr.getExpanded() );
      if ( colorPv ) {
        if ( colorPv->is_valid() ) {
          scloMonitorPvConnectState( colorPv, this );
          scaleUpdate( colorPv, this );
	}
        colorPv->add_conn_state_callback( scloMonitorPvConnectState, this );
        colorPv->add_value_callback( scaleUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      modePv = the_PV_Factory->create( modePvExpStr.getExpanded() );
      if ( modePv ) {
        if ( modePv->is_valid() ) {
          scloMonitorPvConnectState( modePv, this );
          scaleUpdate( modePv, this );
	}
        modePv->add_conn_state_callback( scloMonitorPvConnectState, this );
        modePv->add_value_callback( scaleUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      labelPv = the_PV_Factory->create( labelPvExpStr.getExpanded() );
      if ( labelPv ) {
        if ( labelPv->is_valid() ) {
          scloMonitorPvConnectState( labelPv, this );
          scaleUpdate( labelPv, this );
	}
        labelPv->add_conn_state_callback( scloMonitorPvConnectState, this );
        labelPv->add_value_callback( scaleUpdate, this );
      }
      else {
        activeMode = 0;
        opComplete = 1;
        break;
      }

      updatePv = the_PV_Factory->create( updatePvExpStr.getExpanded() );
      if ( updatePv ) {
        if ( updatePv->is_valid() ) {
          scloMonitorPvConnectState( updatePv, this );
          scaleUpdate( updatePv, this );
	}
        updatePv->add_conn_state_callback( scloMonitorPvConnectState, this );
        updatePv->add_value_callback( scaleUpdate, this );
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

int scaleClass::deactivate (
  int pass )
{

  if ( pass == 1 ) {

    if ( textEntry.formIsPoppedUp() ) {
      textEntry.popdown();
      editDialogIsActive = 0;
    }

    activeMode = 0;

    x = bufX;
    y = bufY;
    w = bufW;
    h = bufH;

    if( minPv ) {
      minPv->remove_conn_state_callback( scloMonitorPvConnectState, this );
      minPv->remove_value_callback( scaleUpdate, this );
      minPv->release();
      minPv = NULL;
    }

    if( maxPv ) {
      maxPv->remove_conn_state_callback( scloMonitorPvConnectState, this );
      maxPv->remove_value_callback( scaleUpdate, this );
      maxPv->release();
      maxPv = NULL;
    }

    if( colorPv ) {
      colorPv->remove_conn_state_callback( scloMonitorPvConnectState, this );
      colorPv->remove_value_callback( scaleUpdate, this );
      colorPv->release();
      colorPv = NULL;
    }

    if( modePv ) {
      modePv->remove_conn_state_callback( scloMonitorPvConnectState, this );
      modePv->remove_value_callback( scaleUpdate, this );
      modePv->release();
      modePv = NULL;
    }

    if( labelPv ) {
      labelPv->remove_conn_state_callback( scloMonitorPvConnectState, this );
      labelPv->remove_value_callback( scaleUpdate, this );
      labelPv->release();
      labelPv = NULL;
    }

    if( updatePv ) {
      updatePv->remove_conn_state_callback( scloMonitorPvConnectState, this );
      updatePv->remove_value_callback( scaleUpdate, this );
      updatePv->release();
      updatePv = NULL;
    }

  }

  return 1;

}

int scaleClass::draw ( void ) {

int stringX, stringY, inc, stat, l;
unsigned int i;
char fullName[127+1];

  if ( activeMode || deleteRequest ) return 1;

  actWin->drawGc.saveFg();

  actWin->drawGc.setFG( lineColor.pixelColor() );
  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.setFontTag( fontTag, actWin->fi );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  if ( horizontal ) {

    scaleLen = w;
    scaleOfs = 0;
    drawXLinearScale( actWin->drawWidget, XtWindow(actWin->drawWidget), &actWin->drawGc, 0 );
    drawText( actWin->drawWidget, XtWindow(actWin->drawWidget), &actWin->drawGc, fs,
     x+w/2, (int) (y+h-1.1*fontHeight), XmALIGNMENT_CENTER, label );

  }
  else {

    scaleLen = h;
    scaleOfs = 0;

    if ( ( mode == modeLinear ) || ( mode == modeHours ) ) {
      ::drawYLinearScale( actWin->d, XtWindow(actWin->drawWidget),
       &actWin->drawGc, 1, x+w, y+scaleOfs+scaleLen, scaleLen, adj_min,
       adj_max, numLabTicks, numMajTicks, numMinTicks,
       lineColor.getColor(), actWin->drawGc.getBaseBG(),
       0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 0 );
    }
    else if ( mode == modeLog ) {
      ::drawYLog10Scale( actWin->d, XtWindow(actWin->drawWidget),
       &actWin->drawGc, 1, x+w, y+scaleOfs+scaleLen, scaleLen, adj_min,
       adj_max, numLabTicks, numMajTicks, numMinTicks,
       lineColor.getColor(), actWin->drawGc.getBaseBG(),
       0, 0, 0, 0, 0, actWin->fi, fontTag, fs, 1, 0, 0, 0 );
    }

    stat = actWin->fi->getFontName( fontTag, 90.0, fullName, 127 );
    actWin->drawGc.setNativeFont( fullName, actWin->fi );

    l = XTextWidth( fs, label, strlen(label) );

    stringX = x + (int) ( 1.0 * (double) fontHeight );
    stringY = y + scaleOfs + scaleLen/2 + l/2;

    for ( i=0; i<strlen(label); i++ ) {

      XDrawString( XtDisplay(actWin->drawWidget),
       XtWindow(actWin->drawWidget), actWin->drawGc.normGC(),
       (int) stringX, stringY, &label[i], 1 );

      inc = 2 + XTextWidth( fs, &label[i], 1 );
      stringY -= inc;

    }

  }

  actWin->drawGc.restoreFg();

  return 1;

}

int scaleClass::erase ( void ) {

  if ( activeMode || deleteRequest ) return 1;

  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x, y, w, h );

  return 1;

}

int scaleClass::checkResizeSelectBox (
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

int scaleClass::checkResizeSelectBoxAbs (
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

void scaleClass::updateDimensions ( void ) {

  if ( h < 5 ) {
    h = 5;
    sboxH = h;
  }

  if ( w < 5 ) {
    w = 5;
    sboxW = 5;
  }

}

void scaleClass::btnUp (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( buttonState & ShiftMask ) {
  }
  else {
  }

}

void scaleClass::btnDown (
  XButtonEvent *be,
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  *action = 0;

  if ( !enabled ) return;

  if ( buttonNumber != 1 ) return;

  if ( editDialogIsActive ) return;

  teX = be->x_root;
  teY = be->y_root;
  teW = w;
  teH = h;
  teLargestH = 600;

  bufMin = adj_min;
  bufMax = adj_max;

  if ( mode == scaleClass::modeLog ) {
    bufMin = pow( 10.0, adj_min );
    bufMax = pow( 10.0, adj_max );
  }

  textEntry.create( actWin->top, &teX, &teY, &teW, &teH, &teLargestH, "",
  NULL, NULL, NULL );
  textEntry.addTextField( "Min", 10, &bufMin );
  textEntry.addTextField( "Max", 10, &bufMax );
  textEntry.finished( sclc_value_edit_ok, sclc_value_edit_apply,
   sclc_value_edit_cancel, this );
  textEntry.popup();
  editDialogIsActive = 1;

}


void scaleClass::btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

  if ( !enabled ) return;

}

int scaleClass::getButtonActionRequest (
  int *up,
  int *down,
  int *drag,
  int *focus )
{

  *drag = 0;
  *down = 1;
  *up = 1;
  *focus = 0;

  return 1;

}

void scaleClass::executeDeferred ( void ) {

int nc, nu, nmu, nd, ne, stat, l,
 newScaleOfs;
char buf[31+1];

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  nc = needConnectInit; needConnectInit = 0;
  nu = needUpdate; needUpdate = 0;
  nmu = needModeUpdate; needModeUpdate = 0;
  nd = needDraw; needDraw = 0;
  ne = needErase; needErase = 0;
  actWin->remDefExeNode( aglPtr );

  actWin->appCtx->proc->unlock();

  if ( !activeMode ) return;


//----------------------------------------------------------------------------

  if ( nc ) {

    //mode = newMode;
    strcpy( label, newLabel );

    if ( max <= min ) max = min + 1.0;

    if ( ( mode == modeLinear ) || ( mode == modeHours ) ) {
      stat = get_scale_params1( min, max, &adj_min, &adj_max,
       &numLabTicks, &numMajTicks, &numMinTicks, format );
    }
    else if ( mode == modeLog ) {
      stat = get_log10_scale_params1( min, max, &adj_min, &adj_max,
       &numLabTicks, &numMajTicks, &numMinTicks, format );
    }

    label_tick = ( adj_max - adj_min ) / numLabTicks;
    major_tick = label_tick / numMajTicks;
    minor_tick = major_tick / numMinTicks;

    if ( !init ) {

      if ( horizontal ) {

        stat = formatString( adj_min, buf, 31 );
        scaleOfs = XTextWidth( fs, buf, strlen(buf) );

        stat = formatString( adj_max, buf, 31 );
        l = XTextWidth( fs, buf, strlen(buf) );
        if ( l > scaleOfs ) scaleOfs = l;

        scaleOfs = scaleOfs / 2 + 6;
        scaleLen = w;
        x = x - scaleOfs;
        w = w + scaleOfs + scaleOfs;

      }
      else {
        scaleOfs = fontHeight;
        scaleLen = h;
        y = y - scaleOfs;
        h = h + scaleOfs + scaleOfs;
      }

      init = 1; // OK to draw in active state now

    }

    drawActive();

  }


//----------------------------------------------------------------------------

  if ( nmu ) {

    eraseActive();

    mode = newMode;

    min = bufMin;
    max = bufMax;
    //min = 0;
    //max = 1;

    if ( max <= min ) max = min + 1.0;

    if ( ( mode == modeLinear ) || ( mode == modeHours ) ) {
      stat = get_scale_params1( min, max, &adj_min, &adj_max,
       &numLabTicks, &numMajTicks, &numMinTicks, format );
    }
    else if ( mode == modeLog ) {
      stat = get_log10_scale_params1( min, max, &adj_min, &adj_max,
       &numLabTicks, &numMajTicks, &numMinTicks, format );
    }

    label_tick = ( adj_max - adj_min ) / numLabTicks;
    major_tick = label_tick / numMajTicks;
    minor_tick = major_tick / numMinTicks;

    //fprintf( stderr, "\n\nscale %-d, min=%-g, max=%-g, adj_min=%-g, adj_max=%-g\n\n",
    //   scaleId, min, max, adj_min, adj_max );

    minPv->put( adj_min );
    maxPv->put( adj_max );
    updatePv->put( scaleId+1000 );

    if ( horizontal ) {

      stat = formatString( adj_min, buf, 31 );
      newScaleOfs = XTextWidth( fs, buf, strlen(buf) );

      stat = formatString( adj_max, buf, 31 );
      l = XTextWidth( fs, buf, strlen(buf) );
      if ( l > newScaleOfs ) newScaleOfs = l;

      newScaleOfs = newScaleOfs / 2 + 6;

      if ( newScaleOfs != scaleOfs ) {
        x = x + scaleOfs;
        w = w - scaleOfs - scaleOfs;
        scaleOfs = newScaleOfs;
        x = x - scaleOfs;
        w = w + scaleOfs + scaleOfs;
      }

    }
    else {
    }

    drawActive();

  }


//----------------------------------------------------------------------------

  if ( nu ) {

    eraseActive();

    //mode = newMode;
    strcpy( label, newLabel );

    if ( mode == scaleClass::modeLog ) {
      if ( bufMin <= 0 ) bufMin = 1;
      if ( bufMax <= 0 ) bufMax = 1;
      if ( bufMax <= bufMin ) bufMax = bufMin + 1;
      min = log10( bufMin );
      max = log10( bufMax );
    }
    else {
      if ( max <= min ) max = min + 1.0;
      min = bufMin;
      max = bufMax;
    }

    if ( ( mode == modeLinear ) || ( mode == modeHours ) ) {
      stat = get_scale_params1( min, max, &adj_min, &adj_max,
       &numLabTicks, &numMajTicks, &numMinTicks, format );
    }
    else if ( mode == modeLog ) {
      stat = get_log10_scale_params1( min, max, &adj_min, &adj_max,
       &numLabTicks, &numMajTicks, &numMinTicks, format );
    }

    label_tick = ( adj_max - adj_min ) / numLabTicks;
    major_tick = label_tick / numMajTicks;
    minor_tick = major_tick / numMinTicks;

    //fprintf( stderr, "\n\nscale %-d, min=%-g, max=%-g, adj_min=%-g, adj_max=%-g\n\n",
    //   scaleId, min, max, adj_min, adj_max );

    minPv->put( adj_min );
    maxPv->put( adj_max );

    if ( modeChange ) {
      modeChange = 0;
      updatePv->put( scaleId+31000 ); // force graph to reread data
    }
    else {
      updatePv->put( scaleId+1000 );
    }

    if ( horizontal ) {

      stat = formatString( adj_min, buf, 31 );
      newScaleOfs = XTextWidth( fs, buf, strlen(buf) );

      stat = formatString( adj_max, buf, 31 );
      l = XTextWidth( fs, buf, strlen(buf) );
      if ( l > newScaleOfs ) newScaleOfs = l;

      newScaleOfs = newScaleOfs / 2 + 6;

      if ( newScaleOfs != scaleOfs ) {
        x = x + scaleOfs;
        w = w - scaleOfs - scaleOfs;
        scaleOfs = newScaleOfs;
        x = x - scaleOfs;
        w = w + scaleOfs + scaleOfs;
      }

    }
    else {
    }

    drawActive();

  }


//----------------------------------------------------------------------------

  if ( nd ) {

    drawActive();

  }

}

void scaleClass::changeDisplayParams (
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

  if ( _flag & ACTGRF_CTLFONTTAG_MASK ) {

    strcpy( fontTag, _ctlFontTag );

    actWin->fi->loadFontTag( fontTag );

    fs = actWin->fi->getXFontStruct( fontTag );

    updateFont( " ", fontTag, &fs,
     &fontAscent, &fontDescent, &fontHeight,
     &stringWidth );

  }

}

void scaleClass::changePvNames (
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

int scaleClass::drawXLinearScale (
  Widget wdgt,
  Drawable dr,
  gcClass *gc,
  int erase
) {

int x0, y0, x1, y1;
int label_tick_height, major_tick_height, minor_tick_height;
double dx, inc, fact, ofs;

  fact = scaleLen / ( adj_max - adj_min );
  ofs = fact * adj_min * -1.0;

  /* draw axis */
  x0 = x + scaleOfs;
  y0 = y;
  x1 = x0 + scaleLen;
  y1 = y0;
  if ( erase )
    XDrawLine( actWin->d, dr, gc->eraseGC(), x0, y0, x1, y1 );
  else
    XDrawLine( actWin->d, dr, gc->normGC(), x0, y0, x1, y1 );

  label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
  major_tick_height = (int) ( 0.7 * ( double) label_tick_height );
  minor_tick_height = (int) ( ( double) major_tick_height * 0.5 );

  /* draw label ticks */

  if ( label_tick > 0.0 ) {

    x0 = x + scaleOfs;
    x1 = x0;
    y0 = y;
    y1 = y0 + label_tick_height;

    dx = adj_min;
    inc = label_tick;

    while ( dx < ( adj_max + inc * 0.5 ) ) {

      if ( erase )
        XDrawLine( actWin->d, dr, gc->eraseGC(), x0, y0, x1, y1 );
      else
        XDrawLine( actWin->d, dr, gc->normGC(), x0, y0, x1, y1 );

      dx += inc;
      x0 = x1 = (int) ( dx * fact + ofs + 0.5 ) + x + scaleOfs;

    }

    dx = adj_max;
    x0 = x1 = (int) ( dx * fact + ofs + 0.5 ) + x + scaleOfs;
    if ( erase )
      XDrawLine( actWin->d, dr, gc->eraseGC(), x0, y0, x1, y1 );
    else
      XDrawLine( actWin->d, dr, gc->normGC(), x0, y0, x1, y1 );

  }

  /* draw major ticks */

  if ( major_tick > 0.0 ) {

    x0 = x + scaleOfs;
    x1 = x0;
    y0 = y;
    y1 = y0 + major_tick_height;

    dx = adj_min;
    inc = major_tick;

    while ( dx < ( adj_max + inc * 0.5 ) ) {

      if ( erase )
        XDrawLine( actWin->d, dr, gc->eraseGC(), x0, y0, x1, y1 );
      else
        XDrawLine( actWin->d, dr, gc->normGC(), x0, y0, x1, y1 );

      dx += inc;
      x0 = x1 = (int) ( dx * fact + ofs + 0.5 ) + x + scaleOfs;

    }

    dx = adj_max;
    x0 = x1 = (int) ( dx * fact + ofs + 0.5 ) + x + scaleOfs;
    if ( erase )
      XDrawLine( actWin->d, dr, gc->eraseGC(), x0, y0, x1, y1 );
    else
      XDrawLine( actWin->d, dr, gc->normGC(), x0, y0, x1, y1 );

  }

  /* draw minor ticks */

  if ( minor_tick > 0.0 ) {

    x0 = x + scaleOfs;
    x1 = x0;
    y0 = y;
    y1 = y0 + minor_tick_height;

    dx = adj_min;
    inc = minor_tick;

    while ( dx < ( adj_max + inc * 0.5 ) ) {

      if ( erase )
        XDrawLine( actWin->d, dr, gc->eraseGC(), x0, y0, x1, y1 );
      else
        XDrawLine( actWin->d, dr, gc->normGC(), x0, y0, x1, y1 );

      dx += inc;
      x0 = x1 = (int) ( dx * fact + ofs + 0.5 ) + x + scaleOfs;

    }

    dx = adj_max;
    x0 = x1 = (int) ( dx * fact + ofs + 0.5 ) + x + scaleOfs;
    if ( erase )
      XDrawLine( actWin->d, dr, gc->eraseGC(), x0, y0, x1, y1 );
    else
      XDrawLine( actWin->d, dr, gc->normGC(), x0, y0, x1, y1 );

  }

  return 1;

}

int scaleClass::drawXLinearAnnotation (
  Widget wdgt,
  Drawable dr,
  gcClass *gc,
  int erase
) {

int x0, y0, x1, y1, stat;
int label_tick_height;
double dx, inc, fact, ofs;
char value[31+1];
SYS_TIME_TYPE sysTime;

  fact = scaleLen / ( adj_max - adj_min );
  ofs = fact * adj_min * -1.0;

  x0 = x + scaleOfs;
  x1 = x0 + scaleLen;

  /* draw annotation */

  if ( label_tick > 0.0 ) {

    label_tick_height = (int) ( 0.8 * (double) abs( fontHeight - 2 ) );
    x0 = x + scaleOfs;
    x1 = x0;
    y0 = y;
    y1 = y0 + (int) ( 1.2 * label_tick_height );

    dx = adj_min;
    inc = label_tick;

    while ( dx < ( adj_max - inc * 0.5 ) ) {

      if ( mode == modeHours ) {

        sysTime.cal_time = (int) ( dx * 3600.0 );
        memcpy( (void *) &sysTime.tm_time,
         (void *) localtime( &sysTime.cal_time ),
         sizeof( struct tm ) );

        sprintf( value, "%-d:%02d:%02d",
         sysTime.tm_time.tm_hour, sysTime.tm_time.tm_min,
         sysTime.tm_time.tm_sec );

        if ( erase )
          stat = eraseText( wdgt, dr, gc, fs, x0, y1, XmALIGNMENT_CENTER, value );
        else
          stat = drawText( wdgt, dr, gc, fs, x0, y1, XmALIGNMENT_CENTER, value );

        sprintf( value, "%-d/%02d", sysTime.tm_time.tm_mon+1,
         sysTime.tm_time.tm_mday );

        if ( erase )
          stat = eraseText( wdgt, dr, gc, fs, x0, y1+fontHeight,
           XmALIGNMENT_CENTER, value );
        else
          stat = drawText( wdgt, dr, gc, fs, x0, y1+fontHeight,
           XmALIGNMENT_CENTER, value );

      }
      else {

        formatString( dx, value, 31 );
        if ( erase )
          stat = eraseText( wdgt, dr, gc, fs, x0, y1, XmALIGNMENT_CENTER, value );
        else
          stat = drawText( wdgt, dr, gc, fs, x0, y1, XmALIGNMENT_CENTER, value );

      }

      dx += inc;
      x0 = x1 = (int) ( dx * fact + ofs + 0.5 ) + x + scaleOfs;

    }

    dx = adj_max;
    x0 = x1 = (int) ( dx * fact + ofs + 0.5 ) + x + scaleOfs;

    if ( mode == modeHours ) {

      sysTime.cal_time = (int) ( dx * 3600.0 );
      memcpy( (void *) &sysTime.tm_time,
       (void *) localtime( &sysTime.cal_time ),
       sizeof( struct tm ) );

      sprintf( value, "%-d:%02d:%02d",
       sysTime.tm_time.tm_hour, sysTime.tm_time.tm_min,
       sysTime.tm_time.tm_sec );

      if ( erase )
        stat = eraseText( wdgt, dr, gc, fs, x0, y1, XmALIGNMENT_CENTER, value );
      else
        stat = drawText( wdgt, dr, gc, fs, x0, y1, XmALIGNMENT_CENTER, value );

      sprintf( value, "%-d/%02d", sysTime.tm_time.tm_mon+1,
       sysTime.tm_time.tm_mday );

      if ( erase )
        stat = eraseText( wdgt, dr, gc, fs, x0, y1+fontHeight,
         XmALIGNMENT_CENTER, value );
      else
        stat = drawText( wdgt, dr, gc, fs, x0, y1+fontHeight,
         XmALIGNMENT_CENTER, value );

    }
    else {

      formatString( dx, value, 31 );

      if ( erase )
        stat = eraseText( wdgt, dr, gc, fs, x0, y1, XmALIGNMENT_CENTER, value );
      else
        stat = drawText( wdgt, dr, gc, fs, x0, y1, XmALIGNMENT_CENTER, value );

    }

  }

  return 1;

}

int scaleClass::formatString (
  double value,
  char *string,
  int len
) {

char buf[128];

  if ( !string ) return 0;
  if ( len < 1 ) return 0;

  sprintf( buf, "%-g", value );

  if ( strlen(buf) > 8 ) {
    sprintf( buf, "%-3g", value );
  }

  strncpy( string, buf, len );

  return 1;

}

void scaleClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 6 ) {
    *n = 0;
    return;
  }

  *n = 6;
  pvs[0] = minPv;
  pvs[1] = maxPv;
  pvs[2] = colorPv;
  pvs[3] = modePv;
  pvs[4] = labelPv;
  pvs[5] = updatePv;

}

extern "C" {

void *create_458bb765_eab9_4d65_8fda_2ce55d2baec6Ptr ( void ) {

scaleClass *ptr;

  ptr = new scaleClass;
  return (void *) ptr;

}

void *clone_458bb765_eab9_4d65_8fda_2ce55d2baec6Ptr (
  void *_srcPtr )
{

scaleClass *ptr, *srcPtr;

  srcPtr = (scaleClass *) _srcPtr;

  ptr = new scaleClass( srcPtr );

  return (void *) ptr;

}

}
