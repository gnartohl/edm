//  object: 2ed7d2e8-f439-11d2-8fed-00104b8742df
//     lib: 3014b6ee-f439-11d2-ad99-00104b8742df

// define SMARTDRAW if the widget may be part of an overlapping
// stack of widgets
//#define SMARTDRAW 1

#define __edmBoxComplete_cc 1

#include "edmBoxComplete.h"


//---------------------------------------------------------------------------

// default constructor

edmBoxClass::edmBoxClass ( void ) {

  // Allocate space for the component name and init
  name = new char[strlen("2ed7d2e8_f439_11d2_8fed_00104b8742df")+1];
  strcpy( name, "2ed7d2e8_f439_11d2_8fed_00104b8742df" );

  // Init class member data
  editBuf = NULL;
  pvExists = 0;
  active = 0;
  activeMode = 0;
  fill = 0;
  lineColorMode = COLORMODE_STATIC;
  fillColorMode = COLORMODE_STATIC;
  lineWidth = 1;
  lineStyle = LineSolid;
  readMin = 0;
  readMax = 10;
  precision = 1;
  efReadMin.setNull(1);
  efReadMax.setNull(1);
  strcpy( fontTag, "" );
  strcpy( label, "" );
  fs = NULL;
  labelX = 0;
  labelY = 0;
  alignment = XmALIGNMENT_BEGINNING;

  unconnectedTimer = 0;

  // This makes the blinking color magic work
  setBlinkFunction( (void *) doBlink );

}

//---------------------------------------------------------------------------

// copy constructor

edmBoxClass::edmBoxClass
 ( const edmBoxClass *source ) {

activeGraphicClass *ago = (activeGraphicClass *) this;

  // clone base class data
  ago->clone( (activeGraphicClass *) source );

  // Allocate space for the component name and init
  name = new char[strlen("2ed7d2e8_f439_11d2_8fed_00104b8742df")+1];
  strcpy( name, "2ed7d2e8_f439_11d2_8fed_00104b8742df" );

  editBuf = NULL;

  // copy edit mode data and initialize execute mode data
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
  precision = source->precision;
  efReadMin = source->efReadMin;
  efReadMax = source->efReadMax;
  efPrecision = source->efPrecision;

  unconnectedTimer = 0;

  // This makes the blinking color magic work
  setBlinkFunction( (void *) doBlink );

}

//---------------------------------------------------------------------------

// Destructor

edmBoxClass::~edmBoxClass ( void ) {

  // deallocate dynamic memory

  if ( name ) delete[] name;

  if ( unconnectedTimer ) {
    XtRemoveTimeOut( unconnectedTimer );
    unconnectedTimer = 0;
  }

  if ( editBuf ) delete editBuf;

  // disable blink function
  updateBlink( 0 );

}

//---------------------------------------------------------------------------

char *edmBoxClass::objName ( void ) {

  // Return component name

  return name;

}

//---------------------------------------------------------------------------

int edmBoxClass::createInteractive (
  activeWindowClass *aw_obj,
  int _x,
  int _y,
  int _w,
  int _h ) {

  // All widgets must copy active window object
  actWin = (activeWindowClass *) aw_obj;

  // Initialize edit mode data so an initial edit mode image
  // may be drawn

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

  boxH = h - fontHeight - fontHeight;
  if ( boxH < 5 ) {
    boxH = 5;
    h = boxH + fontHeight + fontHeight;
    sboxH = h;
  }

  boxY = y + fontHeight;

  if ( w < 5 ) {
    w = 5;
    sboxW = 5;
  }

  // Draw initial edit mode image
  this->draw();

  // create and popup the property dialog for a newly created object
  this->editCreate();

  return 1;

}

//---------------------------------------------------------------------------

int edmBoxClass::editCreate ( void ) {

  this->genericEdit();

  // Map dialog box form buttons to callbacks
  ef.finished( editOk, editApply, editCancelDelete, this );

  // Required by display engine
  actWin->currentEf = NULL;

  // Pop-up the dialog box
  ef.popup();

  return 1;

}

//---------------------------------------------------------------------------

int edmBoxClass::edit ( void ) {

  this->genericEdit();

  // Map dialog box form buttons to callbacks
  ef.finished( editOk, editApply, editCancel, this );

  // Required by display engine
  actWin->currentEf = &ef;

  // Pop-up the dialog box
  ef.popup();

  return 1;

}

//---------------------------------------------------------------------------

int edmBoxClass::genericEdit ( void ) {

// Create the property dialog box so that user may edit widget attributes

char title[32], *ptr;

// Allocate the edit buffer, this holds values until the OK or Cancel button
// is pressed
  if ( !editBuf ) {
    editBuf = new editBufType;
  }

  // Build property dialog title - this will become "Box Properties"
  // Get the widget nickname from the component name
  ptr = actWin->obj.getNameFromClass( "2ed7d2e8_f439_11d2_8fed_00104b8742df" );
  if ( ptr )
    strncpy( title, ptr, 31 );
  else
    strncpy( title, edmBoxComplete_str2, 31 );

  Strncat( title, edmBoxComplete_str3, 31 );

  // Copy current property values into edit buffer; on OK or Apply
  // edit buffer values will be copied back into current property values

  editBuf->bufX = x;
  editBuf->bufY = y;
  editBuf->bufW = w;
  editBuf->bufH = h;

  editBuf->bufLineColor = lineColor.pixelIndex();
  editBuf->bufLineColorMode = lineColorMode;

  editBuf->bufFillColor = fillColor.pixelIndex();
  editBuf->bufFillColorMode = fillColorMode;

  editBuf->bufFill = fill;
  editBuf->bufLineWidth = lineWidth;
  editBuf->bufLineStyle = lineStyle;

  if ( pvExpStr.getRaw() )
    strncpy( editBuf->bufPvName, pvExpStr.getRaw(), PV_Factory::MAX_PV_NAME );
  else
    strcpy( editBuf->bufPvName, "" );

  editBuf->bufEfReadMin = efReadMin;
  editBuf->bufEfReadMax = efReadMax;
  editBuf->bufEfPrecision = efPrecision;

  strncpy( editBuf->bufLabel, label, 63 );
  editBuf->bufLabel[63] = 0;

  // Create the property dialog data entry form
  ef.create( actWin->top, actWin->appCtx->ci.getColorMap(),
   &actWin->appCtx->entryFormX,
   &actWin->appCtx->entryFormY, &actWin->appCtx->entryFormW,
   &actWin->appCtx->entryFormH, &actWin->appCtx->largestH,
   title, NULL, NULL, NULL );

  // Add fields to the form
  ef.addTextField( edmBoxComplete_str4, 30, &editBuf->bufX );
  ef.addTextField( edmBoxComplete_str5, 30, &editBuf->bufY );
  ef.addTextField( edmBoxComplete_str6, 30, &editBuf->bufW );
  ef.addTextField( edmBoxComplete_str7, 30, &editBuf->bufH );
  ef.addOption( edmBoxComplete_str8, edmBoxComplete_str19,
   &editBuf->bufLineWidth );
  ef.addOption( edmBoxComplete_str9, edmBoxComplete_str20,
   &editBuf->bufLineStyle );
  ef.addColorButton( edmBoxComplete_str10, actWin->ci, &lineCb,
   &editBuf->bufLineColor );
  ef.addToggle( edmBoxComplete_str11, &editBuf->bufLineColorMode );
  ef.addToggle( edmBoxComplete_str12, &editBuf->bufFill );
  ef.addColorButton( edmBoxComplete_str13, actWin->ci, &fillCb,
   &editBuf->bufFillColor );
  ef.addToggle( edmBoxComplete_str11, &editBuf->bufFillColorMode );
  ef.addTextField( edmBoxComplete_str14, 30, editBuf->bufPvName,
   PV_Factory::MAX_PV_NAME );
  ef.addTextField( edmBoxComplete_str15, 30, &editBuf->bufEfReadMin );
  ef.addTextField( edmBoxComplete_str16, 30, &editBuf->bufEfReadMax );
  ef.addTextField( edmBoxComplete_str23, 30, &editBuf->bufEfPrecision );
  ef.addFontMenu( edmBoxComplete_str17, actWin->fi, &fm, fontTag );
  fm.setFontAlignment( alignment );
  ef.addTextField( edmBoxComplete_str18, 30, editBuf->bufLabel, 63 );

  return 1;

}

//---------------------------------------------------------------------------

// called by editApply and editOK

void edmBoxClass::editUpdate (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  // Widget data has changed
  ebo->actWin->setChanged();

  // All widgets must include next two lines
  ebo->eraseSelectBoxCorners();
  ebo->erase();

  // Copy edit buffer into current property values

  ebo->lineColorMode = ebo->editBuf->bufLineColorMode;
  if ( ebo->lineColorMode == COLORMODE_ALARM )
    ebo->lineColor.setAlarmSensitive();
  else
    ebo->lineColor.setAlarmInsensitive();
  ebo->lineColor.setColorIndex( ebo->editBuf->bufLineColor, ebo->actWin->ci );

  ebo->fill = ebo->editBuf->bufFill;

  ebo->fillColorMode = ebo->editBuf->bufFillColorMode;
  if ( ebo->fillColorMode == COLORMODE_ALARM )
    ebo->fillColor.setAlarmSensitive();
  else
    ebo->fillColor.setAlarmInsensitive();
  ebo->fillColor.setColorIndex( ebo->editBuf->bufFillColor, ebo->actWin->ci );

  ebo->lineWidth = ebo->editBuf->bufLineWidth;

  if ( ebo->editBuf->bufLineStyle == 0 )
    ebo->lineStyle = LineSolid;
  else if ( ebo->editBuf->bufLineStyle == 1 )
    ebo->lineStyle = LineOnOffDash;

  ebo->pvExpStr.setRaw( ebo->editBuf->bufPvName );

  ebo->efReadMin = ebo->editBuf->bufEfReadMin;
  ebo->efReadMax = ebo->editBuf->bufEfReadMax;
  ebo->efPrecision = ebo->editBuf->bufEfPrecision;

  if ( ebo->efReadMin.isNull() ) {
    ebo->readMin = 0;
  }
  else{
    ebo->readMin = ebo->efReadMin.value();
  }

  if ( ebo->efReadMax.isNull() ) {
    ebo->readMax = 10;
  }
  else{
    ebo->readMax = ebo->efReadMax.value();
  }

  if ( ( ebo->efPrecision.isNull() ) ) {
    ebo->precision = 1;
  }
  else{
    ebo->precision = ebo->efPrecision.value();
  }

  strncpy( ebo->fontTag, ebo->fm.currentFontTag(), 63 );
  ebo->fontTag[63] = 0;

  ebo->actWin->fi->loadFontTag( ebo->fontTag );

  ebo->alignment = ebo->fm.currentFontAlignment();

  ebo->fs = ebo->actWin->fi->getXFontStruct( ebo->fontTag );

  strncpy( ebo->label, ebo->editBuf->bufLabel, 63 );
  ebo->label[63] = 0;

  ebo->stringLength = strlen( ebo->label );

  ebo->updateFont( ebo->label, ebo->fontTag, &ebo->fs,
   &ebo->fontAscent, &ebo->fontDescent, &ebo->fontHeight,
   &ebo->stringWidth );

  // copy into object dimensions and select box dimensions

  ebo->x = ebo->editBuf->bufX;
  ebo->sboxX = ebo->editBuf->bufX;

  ebo->y = ebo->editBuf->bufY;
  ebo->sboxY = ebo->editBuf->bufY;

  ebo->w = ebo->editBuf->bufW;
  ebo->sboxW = ebo->editBuf->bufW;

  ebo->h = ebo->editBuf->bufH;
  ebo->sboxH = ebo->editBuf->bufH;

  // Check values and do a few calculations

  if ( ebo->w < 5 ) {
    ebo->w = 5;
    ebo->sboxW = ebo->w;
  }

  ebo->boxH = ebo->h - ebo->fontHeight - ebo->fontHeight;
  if ( ebo->boxH < 5 ) {
    ebo->boxH = 5;
    ebo->h = ebo->boxH + ebo->fontHeight + ebo->fontHeight;
    ebo->sboxH = ebo->h;
  }

  ebo->boxY = ebo->y + ebo->fontHeight;

  if ( ebo->readMax > ebo->readMin ) {
    ebo->factorW = (double) ( ebo->w - 2 ) / ( ebo->readMax - ebo->readMin );
    ebo->factorH = (double) ebo->boxH / ( ebo->readMax - ebo->readMin );
  }
  else {
    ebo->factorW = 1;
    ebo->factorH = 1;
  }

  ebo->centerX = ebo->x + (int) ( ebo->w * 0.5 + 0.5 );
  ebo->centerY = ebo->boxY + (int) ( ebo->boxH * 0.5 + 0.5 );

  // updateDimensions should be called whenever the widget size
  // or text fonts might have changed
  ebo->updateDimensions();

}

//---------------------------------------------------------------------------

// X Windows callback function for the apply button on the property dialog

void edmBoxClass::editApply (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  edmBoxClass::editUpdate( w, client, call );
  ebo->refresh( ebo );

}

//---------------------------------------------------------------------------

// X Windows callback function for the OK button on the property dialog

void edmBoxClass::editOk (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  edmBoxClass::editUpdate( w, client, call );
  delete ebo->editBuf;
  ebo->editBuf = NULL;
  ebo->ef.popdown();
  ebo->operationComplete();

}

//---------------------------------------------------------------------------

// X Windows callback function for the cancel button on the property dialog

void edmBoxClass::editCancel (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  delete ebo->editBuf;
  ebo->editBuf = NULL;
  ebo->ef.popdown();
  ebo->operationCancel();

}

//---------------------------------------------------------------------------

// X Windows callback function for the cancel button on the property dialog
// when the object is being created. If user cancels edit, object creation
// is being canceled thus deleteRequest must be set.

void edmBoxClass::editCancelDelete (
  Widget w,
  XtPointer client,
  XtPointer call )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  delete ebo->editBuf;
  ebo->editBuf = NULL;
  ebo->erase();
  ebo->deleteRequest = 1;
  ebo->ef.popdown();
  ebo->operationCancel();
  ebo->drawAll();

}

//---------------------------------------------------------------------------

int edmBoxClass::createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

  // Read widget properties from file

int major, minor, release, stat;

tagClass tag;

int zero = 0;
int one = 1;
static char *emptyStr = "";

int solid = LineSolid;
static char *styleEnumStr[2] = {
  "solid",
  "dash"
};
static int styleEnum[2] = {
  LineSolid,
  LineOnOffDash
};

static int left = XmALIGNMENT_BEGINNING;
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

  this->actWin = _actWin;

  tag.init();
  tag.loadR( "beginObjectProperties" );
  tag.loadR( "major", &major );
  tag.loadR( "minor", &minor );
  tag.loadR( "release", &release );
  tag.loadR( "x", &x );
  tag.loadR( "y", &y );
  tag.loadR( "w", &w );
  tag.loadR( "h", &h );
  tag.loadR( "controlPv", &pvExpStr, emptyStr );
  tag.loadR( "lineColor", actWin->ci, &lineColor );
  tag.loadR( "lineAlarm", &lineColorMode, &zero );
  tag.loadR( "fill", &fill, &zero );
  tag.loadR( "fillColor", actWin->ci, &fillColor );
  tag.loadR( "fillAlarm", &fillColorMode, &zero );
  tag.loadR( "lineWidth", &lineWidth, &one );
  tag.loadR( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &solid );
  tag.loadR( "min", &efReadMin );
  tag.loadR( "max", &efReadMax );
  tag.loadR( "precision", &efPrecision );
  tag.loadR( "font", 63, fontTag );
  tag.loadR( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
  tag.loadR( "label", 63, label, emptyStr );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  // If new object version is greater than current version then abort
  if ( major > MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  // If new object version is "old file format" then abort
  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  // initSelectBox must always be call after getting x,y,w,h
  this->initSelectBox(); // call after getting x,y,w,h

  // Process pv alarm information

  if ( lineColorMode == COLORMODE_ALARM )
    lineColor.setAlarmSensitive();
  else
    lineColor.setAlarmInsensitive();

  if ( fillColorMode == COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  // Process min, max, precision

  if ( efReadMin.isNull() ) {
    readMin = 0;
  }
  else{
    readMin = efReadMin.value();
  }

  if ( efReadMax.isNull() ) {
    readMax = 10;
  }
  else{
    readMax = efReadMax.value();
  }

  if ( ( efPrecision.isNull() ) ) {
    precision = 1;
  }
  else{
    precision = efPrecision.value();
  }

  // Make fonts available to X Server, get font struct and font metrics
  actWin->fi->loadFontTag( fontTag );
  fs = actWin->fi->getXFontStruct( fontTag );
  stringLength = strlen( label );
  updateFont( label, fontTag, &fs,
   &fontAscent, &fontDescent, &fontHeight,
   &stringWidth );

  // updateDimensions should be called whenever the widget size
  // or text fonts might have changed
  updateDimensions();

  // Do various calculations

  if ( readMax > readMin ) {
    factorW = (double) ( w - 2 ) / ( readMax - readMin );
    factorH = (double) boxH / ( readMax - readMin );
  }
  else {
    factorW = 1;
    factorH = 1;
  }

  centerX = x + (int) ( w * 0.5 + 0.5 );
  centerY = boxY + (int) ( boxH * 0.5 + 0.5 );

  curValue = 0;

  return stat;

}

//---------------------------------------------------------------------------

int edmBoxClass::old_createFromFile (
  FILE *f,
  char *name,
  activeWindowClass *_actWin )
{

int r, g, b, index;
int major, minor, release;
unsigned int pixel;
char oneName[PV_Factory::MAX_PV_NAME+1];

  this->actWin = _actWin;

  fscanf( f, "%d %d %d\n", &major, &minor, &release ); actWin->incLine();

  if ( major > MAJOR_VERSION ) {
    postIncompatable();
    return 0;
  }

  fscanf( f, "%d\n", &x ); actWin->incLine();
  fscanf( f, "%d\n", &y ); actWin->incLine();
  fscanf( f, "%d\n", &w ); actWin->incLine();
  fscanf( f, "%d\n", &h ); actWin->incLine();

  this->initSelectBox(); // call after getting x,y,w,h

  if ( ( major > 2 ) || ( ( major == 2 ) && ( minor > 0 ) ) ) {

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == COLORMODE_ALARM )
      lineColor.setAlarmSensitive();
    else
      lineColor.setAlarmInsensitive();

    fscanf( f, "%d\n", &fill ); actWin->incLine();

    actWin->ci->readColorIndex( f, &index );
    actWin->incLine(); actWin->incLine();
    fillColor.setColorIndex( index, actWin->ci );

  }
  else if ( major > 1 ) {

    fscanf( f, "%d\n", &index ); actWin->incLine();
    lineColor.setColorIndex( index, actWin->ci );

    fscanf( f, "%d\n", &lineColorMode ); actWin->incLine();

    if ( lineColorMode == COLORMODE_ALARM )
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

    if ( lineColorMode == COLORMODE_ALARM )
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

  if ( fillColorMode == COLORMODE_ALARM )
    fillColor.setAlarmSensitive();
  else
    fillColor.setAlarmInsensitive();

  readStringFromFile( oneName, PV_Factory::MAX_PV_NAME+1, f );
   actWin->incLine();
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
    readStringFromFile( fontTag, 63+1, f ); actWin->incLine();
    readStringFromFile( label, 63+1, f ); actWin->incLine();
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

  boxH = h - fontHeight - fontHeight;
  if ( boxH < 5 ) {
    boxH = 5;
    h = boxH + fontHeight + fontHeight;
  }

  boxY = y + fontHeight;

  if ( readMax > readMin ) {
    factorW = (double) ( w - 2 ) / ( readMax - readMin );
    factorH = (double) boxH / ( readMax - readMin );
  }
  else {
    factorW = 1;
    factorH = 1;
  }

  centerX = x + (int) ( w * 0.5 + 0.5 );
  centerY = boxY + (int) ( boxH * 0.5 + 0.5 );

  curValue = 0;

  return 1;

}

//---------------------------------------------------------------------------

int edmBoxClass::save (
  FILE *f )
{

// Save widget properties to file

int major, minor, release, stat;

tagClass tag;

int zero = 0;
int one = 1;
static char *emptyStr = "";

int solid = LineSolid;
static char *styleEnumStr[2] = {
  "solid",
  "dash"
};
static int styleEnum[2] = {
  LineSolid,
  LineOnOffDash
};

static int left = XmALIGNMENT_BEGINNING;
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

  major = MAJOR_VERSION;
  minor = MINOR_VERSION;
  release = RELEASE;

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "controlPv", &pvExpStr, emptyStr );
  tag.loadW( "lineColor", actWin->ci, &lineColor );
  tag.loadBoolW( "lineAlarm", &lineColorMode, &zero );
  tag.loadBoolW( "fill", &fill, &zero );
  tag.loadW( "fillColor", actWin->ci, &fillColor );
  tag.loadBoolW( "fillAlarm", &fillColorMode, &zero );
  tag.loadW( "lineWidth", &lineWidth, &one );
  tag.loadW( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &solid );
  tag.loadW( "min", &efReadMin );
  tag.loadW( "max", &efReadMax );
  tag.loadW( "precision", &efPrecision );
  tag.loadW( "font", fontTag );
  tag.loadW( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
  tag.loadW( "label", label, emptyStr );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

//---------------------------------------------------------------------------

int edmBoxClass::old_save (
  FILE *f )
{

int index, stat;

  fprintf( f, "%-d %-d %-d\n", MAJOR_VERSION, MINOR_VERSION, RELEASE );
  fprintf( f, "%-d\n", x );
  fprintf( f, "%-d\n", y );
  fprintf( f, "%-d\n", w );
  fprintf( f, "%-d\n", h );

  index = lineColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

  fprintf( f, "%-d\n", lineColorMode );

  fprintf( f, "%-d\n", fill );

  index = fillColor.pixelIndex();
  actWin->ci->writeColorIndex( f, index );
  //fprintf( f, "%-d\n", index );

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

//---------------------------------------------------------------------------

int edmBoxClass::draw ( void ) {

// Draw edit mode image

XRectangle xR = { x, y, w, h };
int clipStat;
int blink = 0;

  // if widget is being activated or has been deleted, return
  if ( activeMode || deleteRequest ) return 1;

  // Save foreground color
  actWin->drawGc.saveFg();

  // Set clipping region
  clipStat = actWin->drawGc.addNormXClipRectangle( xR );

  // If filled, fill in rectangle with specified color
  if ( fill ) {
    actWin->drawGc.setFG( fillColor.pixelIndex(), &blink );
    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.normGC(), x+1, boxY, w-2, boxH );
  }

  // Set line color, style, and width to specified values
  actWin->drawGc.setFG( lineColor.pixelIndex(), &blink );
  actWin->drawGc.setLineWidth( lineWidth );
  actWin->drawGc.setLineStyle( lineStyle );

  // Draw rectangle outline
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.normGC(), x+1, boxY, w-2, boxH );

  // Set graphic context font
  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  // Draw label text
  xDrawText( actWin->d, XtWindow(actWin->drawWidget),
   &actWin->drawGc, fs, labelX, labelY, alignment,
   label );

  // Remove clippling region
  if ( clipStat & 1 ) actWin->drawGc.removeNormXClipRectangle();

  // restore graphic context values
  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );
  actWin->drawGc.restoreFg();

  // This makes the blink magic work
  updateBlink( blink );

  return 1;

}

//---------------------------------------------------------------------------

int edmBoxClass::erase ( void ) {

// Draw edit mode image

// Colors are not set because the eraseGC is used; the eraseGC contains
// the color of the display background

XRectangle xR = { x, y, w, h };
int clipStat;

  // if widget is being activated or has been deleted, return
  if ( activeMode || deleteRequest ) return 1;

  // Set clipping region
  clipStat = actWin->drawGc.addEraseXClipRectangle( xR );

  // If filled, erase fill
  if ( fill ) {
    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x+1, boxY, w-2, boxH );
  }

  // Set line style and width to specified values
  actWin->drawGc.setLineWidth( lineWidth );
  actWin->drawGc.setLineStyle( lineStyle );

  // Erase rectangle outline
  XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
   actWin->drawGc.eraseGC(), x+1, boxY, w-2, boxH );

  // Set graphic context font
  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->drawGc.setFontTag( fontTag, actWin->fi );
  }

  // Erase label text
  xEraseText( actWin->d, XtWindow(actWin->drawWidget),
   &actWin->drawGc, fs, labelX, labelY, alignment,
   label );

  // Remove clippling region
  if ( clipStat & 1 ) actWin->drawGc.removeEraseXClipRectangle();

  // restore graphic context values
  actWin->drawGc.setLineWidth( 1 );
  actWin->drawGc.setLineStyle( LineSolid );

  return 1;

}

//---------------------------------------------------------------------------

int edmBoxClass::checkResizeSelectBox (
  int _x,
  int _y,
  int _w,
  int _h ) {

  // Constrain minimum widget area size, input parameters are
  // delta values

int tmpw, tmph, tmpBoxH, ret_stat;

  ret_stat = 1;

  tmpw = sboxW;
  tmph = sboxH;

  tmpw += _w;
  tmph += _h;

  tmpBoxH = tmph - fontHeight - fontHeight;
  if ( tmpBoxH < 5 ) ret_stat = 0;

  if ( tmpw < 5 ) ret_stat = 0;

  return ret_stat;

}

//---------------------------------------------------------------------------

int edmBoxClass::checkResizeSelectBoxAbs (
  int _x,
  int _y,
  int _w,
  int _h ) {

  // Constrain minimum widget area size, input parameters are
  // absolute values

int tmpw, tmph, tmpBoxH, ret_stat;

  ret_stat = 1;

  tmpw = _w;
  tmph = _h;

  if ( tmph != -1 ) {
    tmpBoxH = tmph - fontHeight - fontHeight;
    if ( tmpBoxH < 5 ) ret_stat = 0;
  }

  if ( tmpw != -1 ) {
    if ( tmpw < 5 ) ret_stat = 0;
  }

  return ret_stat;

}

//---------------------------------------------------------------------------

void edmBoxClass::updateDimensions ( void ) {

  // Update values of position and size of widget internal details

  boxY = y + fontHeight;
  labelY = y + h - fontHeight;
  textValueY = y;

  if ( alignment == XmALIGNMENT_BEGINNING )
    textValueX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    textValueX = x + w/2;
  else if ( alignment == XmALIGNMENT_END )
    textValueX = x + w;

  if ( alignment == XmALIGNMENT_BEGINNING )
    labelX = x;
  else if ( alignment == XmALIGNMENT_CENTER )
    labelX = x + w/2;
  else if ( alignment == XmALIGNMENT_END )
    labelX = x + w;

  boxH = h - fontHeight - fontHeight;
  if ( boxH < 5 ) {
    boxH = 5;
    h = boxH + fontHeight + fontHeight;
    sboxH = h;
  }

  if ( w < 5 ) {
    w = 5;
    sboxW = 5;
  }

}

//---------------------------------------------------------------------------

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

  // Depending on flag bits set, update various display properties; widgets
  // ignore properties that are not applicable

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

    updateDimensions();

  }

}

//---------------------------------------------------------------------------

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

  // Depending on flag bits set, update various process variable
  // names; widgets ignore pv names that are not applicable

  if ( flag & ACTGRF_CTLPVS_MASK ) {
    if ( numCtlPvs ) {
      pvExpStr.setRaw( ctlPvs[0] );
    }
  }

}

//---------------------------------------------------------------------------

int edmBoxClass::drawActive ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;
char string[31+1];
int ascent, descent, height, width;
int blink = 0;

  if ( !init ) {
    if ( needToDrawUnconnected ) {
      actWin->executeGc.saveFg();
      actWin->executeGc.setFG( lineColor.getDisconnectedIndex(), &blink );
      actWin->executeGc.setLineWidth( 1 );
      actWin->executeGc.setLineStyle( LineSolid );
      XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
      actWin->executeGc.restoreFg();
      needToEraseUnconnected = 1;
      updateBlink( blink );
    }
  }
  else if ( needToEraseUnconnected ) {
    actWin->executeGc.setLineWidth( 1 );
    actWin->executeGc.setLineStyle( LineSolid );
    XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );
    needToEraseUnconnected = 0;
  }

  if ( !enabled || !activeMode || !init ) return 1;

  actWin->executeGc.saveFg();

  clipStat = actWin->executeGc.addNormXClipRectangle( xR );

  if ( fill ) {
    actWin->executeGc.setFG( fillColor.getIndex(), &blink  );
    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), valueBoxX, valueBoxY, valueBoxW, valueBoxH );
  }

  actWin->executeGc.setFG( lineColor.getIndex(), &blink  );
  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.normGC(), valueBoxX, valueBoxY, valueBoxW, valueBoxH );

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  snprintf( string, 31, format, value );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  updateFont( string, fontTag, &fs,
   &ascent, &descent, &height, &width );

  xDrawText( actWin->d, XtWindow(actWin->executeWidget),
   &actWin->executeGc, fs, textValueX, textValueY, alignment,
   string );

  prevValue = value;

  if ( bufferInvalid ) {

    xDrawText( actWin->d, XtWindow(actWin->executeWidget),
     &actWin->executeGc, fs, labelX, labelY, alignment,
     label );

    bufferInvalid = 0;

  }

  if ( clipStat & 1 ) actWin->executeGc.removeNormXClipRectangle();

  actWin->executeGc.restoreFg();

  updateBlink( blink );

  return 1;

}

//---------------------------------------------------------------------------

int edmBoxClass::eraseActive ( void ) {

XRectangle xR = { x, y, w, h };
int clipStat;
char string[31+1];

  if ( !enabled || !activeMode || !init ) return 1;

  clipStat = actWin->executeGc.addEraseXClipRectangle( xR );

  if ( fill ) {
    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), valueBoxX, valueBoxY, valueBoxW, valueBoxH );
  }

  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), valueBoxX, valueBoxY, valueBoxW, valueBoxH );

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  snprintf( string, 31, format, prevValue );

  if ( strcmp( fontTag, "" ) != 0 ) {
    actWin->executeGc.setFontTag( fontTag, actWin->fi );
  }

  xEraseText( actWin->d, XtWindow(actWin->executeWidget),
   &actWin->executeGc, fs, textValueX, textValueY, alignment,
   string );

  if ( bufferInvalid ) {

    xEraseText( actWin->d, XtWindow(actWin->executeWidget),
     &actWin->executeGc, fs, labelX, labelY, alignment,
     label );

  }

  if ( clipStat & 1 ) actWin->executeGc.removeEraseXClipRectangle();

  return 1;

}

//---------------------------------------------------------------------------

void edmBoxClass::bufInvalidate ( void )
{

  bufferInvalid = 1;

}

//---------------------------------------------------------------------------

int edmBoxClass::expand1st (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = pvExpStr.expand1st( numMacros, macros, expansions );

  return stat;

}

//---------------------------------------------------------------------------

int edmBoxClass::expand2nd (
  int numMacros,
  char *macros[],
  char *expansions[] )
{

int stat;

  stat = pvExpStr.expand2nd( numMacros, macros, expansions );

  return stat;

}

//---------------------------------------------------------------------------

int edmBoxClass::containsMacros ( void ) {

  if ( pvExpStr.containsPrimaryMacros() ) return 1;

  return 0;

}

//---------------------------------------------------------------------------

int edmBoxClass::activate (
  int pass,
  void *ptr )
{

int stat;

  switch ( pass ) {

  case 1: // initialize

    opComplete = 0;

    break;

  case 2: // connect to pv

    if ( !opComplete ) {

      initEnable();
      aglPtr = ptr;
      needConnectInit = needUpdate = needDraw = 0;
      init = 0;
      active = 0;
      activeMode = 1;
      bufferInvalid = 0;
      pointerMotionDetected = 0;
      pvId = NULL;
      oldStat = oldSev = -1;
      value = prevValue = 0;

      needToDrawUnconnected = 0;
      needToEraseUnconnected = 0;
      unconnectedTimer = 0;

      if ( !unconnectedTimer ) {
        unconnectedTimer = appAddTimeOut( actWin->appCtx->appContext(),
         2000, unconnectedTimeout, this );
      }

      if ( !pvExpStr.getExpanded() ||
	 // ( strcmp( pvExpStr.getExpanded(), "" ) == 0 ) ) {
         blankOrComment( pvExpStr.getExpanded() ) ) {
        pvExists = 0;
      }
      else {
        pvExists = 1;
        lineColor.setConnectSensitive();
        fillColor.setConnectSensitive();
      }

      if ( pvExists ) {
	pvId = the_PV_Factory->create( pvExpStr.getExpanded() );
	if ( pvId ) {
	  pvId->add_conn_state_callback( monitorPvConnectState, this );
          pvId->add_value_callback( pvUpdate, this );
	}
	else {
          printf( edmBoxComplete_str21 );
        }
      }
      else {
        active = 1;
        init = 1;
        valueBoxW = w-2;
        valueBoxX = x+1;
        valueBoxH = boxH;
        valueBoxY = boxY;
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

//---------------------------------------------------------------------------

int edmBoxClass::deactivate (
  int pass )
{

  if ( pass == 1 ) {

    if ( unconnectedTimer ) {
      XtRemoveTimeOut( unconnectedTimer );
      unconnectedTimer = 0;
    }

    activeMode = 0;

    if ( pvId ) {
      pvId->remove_conn_state_callback( monitorPvConnectState, this );
      pvId->remove_value_callback( pvUpdate, this );
      pvId->release();
      pvId = NULL;
    }

  }

  return 1;

}

//---------------------------------------------------------------------------

void edmBoxClass::btnUp (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

double v, dInc;

  *action = 0;

  if ( !enabled ) return;

  dInc = 10.0;

  // wheel
  if ( buttonNumber == 4 ) {
    buttonNumber = 1;
    dInc = 1.0;
  }

  if ( buttonNumber == 5 ) {
    buttonNumber = 1;
    buttonState |= ShiftMask;
    dInc = 1.0;
  }
  // wheel

  if ( buttonNumber != 1 ) return;

  if ( pvExists && !pointerMotionDetected ) {

    if ( buttonState & ShiftMask ) {
      v = curValue - dInc;
      if ( v < readMin ) v = readMin;
    }
    else {
      v = curValue + dInc;
      if ( v > readMax ) v = readMax;
    }

    pvId->put( v );

  }

}

//---------------------------------------------------------------------------

void edmBoxClass::btnDown (
  int x,
  int y,
  int buttonState,
  int buttonNumber,
  int *action )
{

  if ( !enabled ) return;
  if ( buttonNumber == 4 ) buttonNumber = 1;
  if ( buttonNumber == 5 ) buttonNumber = 1;
  if ( buttonNumber != 1 ) return;

  pointerMotionDetected = 0;

}

//---------------------------------------------------------------------------

void edmBoxClass::btnDrag (
  int x,
  int y,
  int buttonState,
  int buttonNumber )
{

double v;

  if ( !enabled ) return;
  if ( buttonNumber != 1 ) return;

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

    pvId->put( v );

  }

}

//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------

void edmBoxClass::executeDeferred ( void ) {

int stat, nc, nu, nd;

  if ( actWin->isIconified ) return;

  actWin->appCtx->proc->lock();

  if ( !activeMode ) {
    actWin->remDefExeNode( aglPtr );
    actWin->appCtx->proc->unlock();
    return;
  }

  value = curValue;
  nc = needConnectInit; needConnectInit = 0;
  nu = needUpdate; needUpdate = 0;
  nd = needDraw; needDraw = 0;
  actWin->remDefExeNode( aglPtr );

  actWin->appCtx->proc->unlock();


//--------------

  if ( nc ) {

    // require process variable to be numeric
    if ( ( fieldType == ProcessVariable::Type::real ) ||
         ( fieldType == ProcessVariable::Type::integer ) ) {

      snprintf( format, 15, "%%.%-df", precision );

      stat = eraseActive();

      if ( readMax > readMin ) {
        factorW = (double) ( w - 2 ) / ( readMax - readMin );
        factorH = (double) boxH / ( readMax - readMin );
      }
      else {
        factorW = 1;
        factorH = 1;
      }

      centerX = x + (int) ( w * 0.5 + 0.5 );
      centerY = boxY + (int) ( boxH * 0.5 + 0.5 );

      if ( value > 0.0 ) {
        valueBoxW = (int) ( value * factorW + 0.5 );
        valueBoxX = centerX - (int) ( (double) valueBoxW * 0.5 + 0.5 );
        valueBoxH = (int) ( value * factorH + 0.5 );
        valueBoxY = centerY - (int) ( (double) valueBoxH * 0.5 + 0.5 );
      }
      else {
        valueBoxW = 1;
        valueBoxX = centerX;
        valueBoxH = 1;
        valueBoxY = centerY;
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
    else { // force a draw in the non-active state & post error message

      actWin->appCtx->postMessage(
       edmBoxComplete_str22 );

      active = 0;
      init = 1;
      valueBoxW = w-2;
      valueBoxX = x+1;
      valueBoxH = boxH;
      valueBoxY = boxY;
      lineColor.setDisconnected();
      fillColor.setDisconnected();

#if SMARTDRAW
      smartDrawAllActive();
#else
      drawActive();
#endif

    }

  }


//--------------

  if ( nu ) {

    eraseActive();

    if ( value > 0.0 ) {
      valueBoxW = (int) ( value * factorW + 0.5 );
      valueBoxX = centerX - (int) ( (double) valueBoxW * 0.5 + 0.5 );
      valueBoxH = (int) ( value * factorH + 0.5 );
      valueBoxY = centerY - (int) ( (double) valueBoxH * 0.5 + 0.5 );
    }
    else {
      valueBoxW = 1;
      valueBoxX = centerX;
      valueBoxH = 1;
      valueBoxY = centerY;
    }

#if SMARTDRAW
    smartDrawAllActive();
#else
    drawActive();
#endif

  }


//--------------

  if ( nd ) {

#if SMARTDRAW
    smartDrawAllActive();
#else
    drawActive();
#endif

  }

}

//---------------------------------------------------------------------------

char *edmBoxClass::firstDragName ( void ) {

  if ( !enabled ) return NULL;

  dragIndex = 0;
  return dragName[dragIndex];

}

//---------------------------------------------------------------------------

char *edmBoxClass::nextDragName ( void ) {

  return NULL;

}

//---------------------------------------------------------------------------

char *edmBoxClass::dragValue (
  int i ) {

  if ( !enabled ) return NULL;

  if ( actWin->mode == AWC_EXECUTE ) {

    return pvExpStr.getExpanded();

  }
  else {

    return pvExpStr.getRaw();

  }

}

//---------------------------------------------------------------------------

// X windows makes client code do blinking if color depth is
// anything other than 8 bits

void edmBoxClass::doBlink (
  void *ptr
) {

edmBoxClass *ebo = (edmBoxClass *) ptr;

  if ( !ebo->activeMode ) {
    if ( ebo->isSelected() ) ebo->drawSelectBoxCorners(); // erase via xor
#if SMARTDRAW
    ebo->smartDrawAll();
#else
      ebo->draw();
#endif
    if ( ebo->isSelected() ) ebo->drawSelectBoxCorners();
  }
  else {
    ebo->actWin->appCtx->proc->lock();
    ebo->bufInvalidate();
    ebo->needUpdate = 1;
    ebo->actWin->addDefExeNode( ebo->aglPtr );
    ebo->actWin->appCtx->proc->unlock();
  }

}

//---------------------------------------------------------------------------

void edmBoxClass::unconnectedTimeout (
  XtPointer client,
  XtIntervalId *id )
{

edmBoxClass *ebo = (edmBoxClass *) client;

  if ( !ebo->init ) {
    ebo->actWin->appCtx->proc->lock();
    ebo->bufInvalidate();
    ebo->needToDrawUnconnected = 1;
    ebo->needUpdate = 1;
    ebo->actWin->addDefExeNode( ebo->aglPtr );
    ebo->actWin->appCtx->proc->unlock();
  }

  ebo->unconnectedTimer = 0;

}


//---------------------------------------------------------------------------

void edmBoxClass::monitorPvConnectState (
  ProcessVariable *pv,
  void *userarg )
{

edmBoxClass *ebo = (edmBoxClass *) userarg;

  ebo->actWin->appCtx->proc->lock();

  if ( !ebo->activeMode ) {
    ebo->actWin->appCtx->proc->unlock();
    return;
  }

  if ( pv->is_valid() ) {

    ebo->fieldType = (int) pv->get_type().type;

    ebo->curValue = pv->get_double();

    if ( ebo->efReadMin.isNull() ) {
      ebo->readMin = pv->get_lower_disp_limit();
    }
    else {
      ebo->readMin = ebo->efReadMin.value();
    }

    if ( ebo->efReadMax.isNull() ) {
      ebo->readMax = pv->get_upper_disp_limit();
    }
    else {
      ebo->readMax = ebo->efReadMax.value();
    }

    if ( ebo->efPrecision.isNull() ) {
      ebo->precision = pv->get_precision();
    }
    else {
      ebo->precision = ebo->efPrecision.value();
    }

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

//---------------------------------------------------------------------------

void edmBoxClass::pvUpdate (
  ProcessVariable *pv,
  void *userarg )
{

class edmBoxClass *ebo = (edmBoxClass *) userarg;
int st, sev;

  if ( !ebo->activeMode ) return;

  ebo->actWin->appCtx->proc->lock();

  ebo->curValue = pv->get_double();
  ebo->needUpdate = 1;

  st = pv->get_status();
  sev = pv->get_severity();
  if ( ( st != ebo->oldStat ) || ( sev != ebo->oldSev ) ) {
    ebo->oldStat = st;
    ebo->oldSev = sev;
    ebo->lineColor.setStatus( st, sev );
    ebo->fillColor.setStatus( st, sev );
    ebo->needDraw = 1;
    ebo->bufInvalidate();
  }

  ebo->actWin->addDefExeNode( ebo->aglPtr );

  ebo->actWin->appCtx->proc->unlock();

}

void edmBoxClass::getPvs (
  int max,
  ProcessVariable *pvs[],
  int *n ) {

  if ( max < 1 ) {
    *n = 0;
    return;
  }

  *n = 1;
  pvs[0] = pvId;

}

//---------------------------------------------------------------------------

// "C" compatible class factory functions

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
