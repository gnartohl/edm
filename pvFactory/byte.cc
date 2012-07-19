// -*- C++ -*-
// EDM Byte Widget
//
// Carl Lionberger
//

#include "byte.h"
#include "app_pkg.h"
#include "act_win.h"
#include "pv_factory.h"
#include "cvtFast.h"


edmByteClass::edmByteClass() : activeGraphicClass(), init(0), 
  is_executing(false), is_pvname_valid(false), valuePvId(0), bufInvalid(0),
  validFlag(false), value(0), lastval(0), dmask(0), lastsev(0), 
  theDir(BIGENDIAN),  nobt(16), shft(0), lineWidth(1), lineStyle(LineSolid), 
  theOutline(0)
{
   name = strdup(BYTE_CLASSNAME);
   checkBaseClassVersion( activeGraphicClass::MAJOR_VERSION, name );
}

edmByteClass::edmByteClass(edmByteClass *rhs)
{
    clone(rhs, BYTE_CLASSNAME);
    doAccSubs( pv_exp_str );
}

void edmByteClass::clone(const edmByteClass *rhs,
                               const char *classname)
{
    // This next line must always be included
    activeGraphicClass::clone((activeGraphicClass *)rhs);

    name = strdup(classname);

    is_pvname_valid = false;
    valuePvId = 0;
    pv_exp_str.setRaw(rhs->pv_exp_str.rawString);
    init = 0;
    is_executing = false;
    lineColor = rhs->lineColor;
    onColor = rhs->onColor;
    offColor = rhs->offColor;
    lineCb = rhs->lineCb;
    onColorCb = rhs->onColorCb;
    offColorCb = rhs->offColorCb;
    lineWidth = rhs->lineWidth;
    lineStyle = rhs->lineStyle;
    fgPixel = rhs->fgPixel;
    onPixel = rhs->onPixel;
    offPixel = rhs->offPixel;
    nobt = rhs->nobt;
    shft = rhs->shft;
    dmask = rhs->dmask;
    theDir = rhs->theDir;
    theOutline = 0;
}

edmByteClass::~edmByteClass()
{
    if (valuePvId)
    {
        valuePvId->remove_conn_state_callback(pv_callback, this);
        valuePvId->remove_value_callback(pv_callback, this);
        valuePvId->release();
        valuePvId = 0;
    }
    delete[] name;
    delete[] theOutline;
}

char *edmByteClass::objName()
{   return name; }

const char *edmByteClass::getRawPVName()
{
    char *s = pv_exp_str.getRaw();
    return s ? s : "";
}

const char *edmByteClass::getExpandedPVName()
{
    char *s = pv_exp_str.getExpanded();
    return s ? s : "";
}


// --------------------------------------------------------
// Load/save
// --------------------------------------------------------
int edmByteClass::save(FILE *f)
{

int major, minor, release, en, stat;

tagClass tag;

int zero = 0;
int one = 1;
int sixteen = 16;

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

int bigEndian = 0;
static char *endianEnumStr[2] = {
  "big",
  "little"
};
static int endianEnum[2] = {
  0,
  1
};

  major = BYTE_MAJOR;
  minor = BYTE_MINOR;
  release = BYTE_RELEASE;

  switch ( theDir ) {
  case BIGENDIAN:
    en = 0;
    break;
  case LITTLEENDIAN:
    en = 1;
    break;
  }

  tag.init();
  tag.loadW( "beginObjectProperties" );
  tag.loadW( "major", &major );
  tag.loadW( "minor", &minor );
  tag.loadW( "release", &release );
  tag.loadW( "x", &x );
  tag.loadW( "y", &y );
  tag.loadW( "w", &w );
  tag.loadW( "h", &h );
  tag.loadW( "controlPv", &pv_exp_str, emptyStr );
  tag.loadW( "lineColor", actWin->ci, &lineColor );
  tag.loadW( "onColor", actWin->ci, &onColor );
  tag.loadW( "offColor", actWin->ci, &offColor );
  tag.loadW( "lineWidth", &lineWidth, &one );
  tag.loadW( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &solid );
  tag.loadW( "endian", 2, endianEnumStr, endianEnum, &en, &bigEndian );
  tag.loadW( "numBits", &nobt, &sixteen );
  tag.loadW( "shift", &shft, &zero );
  tag.loadW( unknownTags );
  tag.loadW( "endObjectProperties" );
  tag.loadW( "" );

  stat = tag.writeTags( f );

  return stat;

}

// --------------------------------------------------------
// Load/save
// --------------------------------------------------------
int edmByteClass::old_save(FILE *f)
{
    // Version, bounding box
    fprintf(f, "%-d %-d %-d\n",
            BYTE_MAJOR, BYTE_MINOR, BYTE_RELEASE);
    fprintf(f, "%-d\n", x);
    fprintf(f, "%-d\n", y);
    fprintf(f, "%-d\n", w);
    fprintf(f, "%-d\n", h);

    actWin->ci->writeColorIndex( f, lineColor );
    actWin->ci->writeColorIndex( f, onColor );
    actWin->ci->writeColorIndex( f, offColor );
    //fprintf( f, "%-d\n", lineColor );
    //fprintf( f, "%-d\n", onColor );
    //fprintf( f, "%-d\n", offColor );

  writeStringToFile(f, (char *)getRawPVName());

  fprintf( f, "%-d\n", lineWidth );

  fprintf( f, "%-d\n", lineStyle );

  fprintf( f, "%d\n", theDir ); 
  fprintf( f, "%d\n", nobt );
  fprintf( f, "%d\n", shft ); 

  return 1;
}

int edmByteClass::createFromFile(FILE *f, char *filename,
                                       activeWindowClass *_actWin)
{

int major, minor, release, en, temp, stat;

tagClass tag;

int zero = 0;
int one = 1;
int sixteen = 16;

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

int bigEndian = 0;
static char *endianEnumStr[2] = {
  "big",
  "little"
};
static int endianEnum[2] = {
  0,
  1
};

  actWin = _actWin;

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
  tag.loadR( "controlPv", &pv_exp_str, emptyStr );
  tag.loadR( "lineColor", actWin->ci, &lineColor );
  tag.loadR( "onColor", actWin->ci, &onColor );
  tag.loadR( "offColor", actWin->ci, &offColor );
  tag.loadR( "lineWidth", &lineWidth, &one );
  tag.loadR( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &solid );
  tag.loadR( "endian", 2, endianEnumStr, endianEnum, &en, &bigEndian );
  tag.loadR( "numBits", &nobt, &sixteen );
  tag.loadR( "shift", &shft, &zero );
  tag.loadR( "endObjectProperties" );

  stat = tag.readTags( f, "endObjectProperties" );

  if ( !( stat & 1 ) ) {
    actWin->appCtx->postMessage( tag.errMsg() );
  }

  if ( major > BYTE_MAJOR ) {
    postIncompatable();
    return 0;
  }

  if ( major < 4 ) {
    postIncompatable();
    return 0;
  }

  this->initSelectBox(); // call after getting x,y,w,h

  if (actWin->ci->isRule(onColor))
  {
    onPixel = actWin->ci->getPixelByIndex(actWin->ci->evalRule(onColor,
                                                                         1.0));
    offPixel = actWin->ci->getPixelByIndex(actWin->ci->evalRule(onColor,
                                                                         0.0));
  }
  else
  {
    onPixel = actWin->ci->getPixelByIndex(onColor);
    offPixel = actWin->ci->getPixelByIndex(offColor);
  }

  temp = en;
  en = (temp < 0)?0:((temp > 1)?1:temp);
  theDir = (bdir)en;

  temp = nobt;
  nobt = (temp < 1)?1:((temp > 16)?16:temp);

  temp = shft;
  shft = (temp < 0)?0:((temp > 31)?31:temp);
  
  updateDimensions();

  return stat;

}

int edmByteClass::old_createFromFile(FILE *f, char *filename,
                                       activeWindowClass *_actWin)
{
    int major, minor, release, temp;
    char tname[PV_Factory::MAX_PV_NAME+1];

    actWin = _actWin;

    // Version, bounding box
    fscanf(f, "%d %d %d\n", &major, &minor, &release); actWin->incLine();

    if ( major > BYTE_MAJOR ) {
      postIncompatable();
      return 0;
    }

    fscanf(f, "%d\n", &x); actWin->incLine();
    fscanf(f, "%d\n", &y); actWin->incLine();
    fscanf(f, "%d\n", &w); actWin->incLine();
    fscanf(f, "%d\n", &h); actWin->incLine();
    this->initSelectBox(); // call after getting x,y,w,h

    if ( ( major > 1 ) || ( ( major == 1 ) && ( minor > 0 ) ) )
    {
      actWin->ci->readColorIndex( f, &lineColor );
      actWin->incLine(); actWin->incLine();
      actWin->ci->readColorIndex( f, &onColor );
      actWin->incLine(); actWin->incLine();
      actWin->ci->readColorIndex( f, &offColor );
      actWin->incLine(); actWin->incLine();
    }
    else
    {
      fscanf( f, "%d\n", &lineColor ); actWin->incLine();
      fscanf( f, "%d\n", &onColor ); actWin->incLine();
      fscanf( f, "%d\n", &offColor ); actWin->incLine();
    }

    if (actWin->ci->isRule(onColor))
    {
       onPixel = actWin->ci->getPixelByIndex(actWin->ci->evalRule(onColor,
                                                                         1.0));
       offPixel = actWin->ci->getPixelByIndex(actWin->ci->evalRule(onColor,
                                                                         0.0));
    }
    else
    {
       onPixel = actWin->ci->getPixelByIndex(onColor);
       offPixel = actWin->ci->getPixelByIndex(offColor);
    }



  // PV Name
  readStringFromFile(tname, PV_Factory::MAX_PV_NAME+1, f); actWin->incLine();
  pv_exp_str.setRaw(tname);


  fscanf( f, "%d\n", &lineWidth ); actWin->incLine();

  fscanf( f, "%d\n", &lineStyle ); actWin->incLine();

  fscanf( f, "%d\n", &temp ); actWin->incLine();
  temp = (temp < 0)?0:((temp > 1)?1:temp);
  theDir = (bdir)temp;
  fscanf( f, "%d\n", &temp ); actWin->incLine();
  nobt = (temp < 1)?1:((temp > 16)?16:temp);
  fscanf( f, "%d\n", &temp ); actWin->incLine();
  shft = (temp < 0)?0:((temp > 31)?31:temp);
  
  updateDimensions();


  return 1;

}

// --------------------------------------------------------
// Edit Mode
// --------------------------------------------------------

// Idea of next two and helper methods:
// createInteractive -> editCreate -> genericEdit (delete on cancel)
// edit -> genericEdit (ignore changes on cancel)
int edmByteClass::createInteractive(activeWindowClass *aw_obj,
                                          int _x, int _y, int _w, int _h)
{   // required
    actWin = (activeWindowClass *) aw_obj;
    xOrigin = 0;
    yOrigin = 0;
    x = _x; y = _y; w = _w; h = _h;

    offPixel = actWin->ci->getPixelByIndex(actWin->defaultBgColor);
    offColor = actWin->defaultBgColor;
    onPixel = actWin->ci->getPixelByIndex(actWin->defaultTextFgColor);
    onColor = actWin->defaultTextFgColor;
    fgPixel = actWin->ci->getPixelByIndex(actWin->defaultTextFgColor);
    lineColor = actWin->fgColor;
    lineWidth = 1;
    lineStyle = 0;
    theOutline = 0;

    // initialize and draw some kind of default image for the user
    draw();
    editCreate();
    return 1;
}

int edmByteClass::edit()
{   // Popup property dialog, cancel -> no delete
    genericEdit();
    ef.finished(edit_ok, edit_apply, edit_cancel, this);
    actWin->currentEf = &ef;
    ef.popup();
    return 1;
}

int edmByteClass::editCreate()
{
    // Popup property dialog, cancel -> delete
    genericEdit();
    ef.finished(edit_ok, edit_apply, edit_cancel_delete, this);
    actWin->currentEf = NULL;
    ef.popup();
    return 1;
}

int edmByteClass::genericEdit() // create Property Dialog
{
    char title[80+1], *ptr;
    // required
    ptr = actWin->obj.getNameFromClass(name);
    if (ptr)
    {
        strncpy(title, ptr, 80);
        title[80] = 0;
        Strncat(title, " Properties", 80);
    }
    else
        strncpy(title, "Unknown object Properties", 80);
   
    // Copy data member contents into edit buffers
  bufX = x;
  bufY = y;
  bufW = w;
  bufH = h;

  bufLineColor = lineColor;

  bufOnColor = onColor;
  bufOffColor = offColor;


  bufLineWidth = lineWidth;
  bufLineStyle = lineStyle;

  strncpy(bufPvName, getRawPVName(), PV_Factory::MAX_PV_NAME);

  bufTheDir = theDir;
  bufNobt = nobt;
  bufShft = shft;

    // create entry form dialog box
    ef.create(actWin->top, actWin->appCtx->ci.getColorMap(),
              &actWin->appCtx->entryFormX, &actWin->appCtx->entryFormY,
              &actWin->appCtx->entryFormW, &actWin->appCtx->entryFormH,
              &actWin->appCtx->largestH,
              title, NULL, NULL, NULL);

    // add dialog box entry fields
    ef.addTextField("X", 30, &bufX);
    ef.addTextField("Y", 30, &bufY);
    ef.addTextField("Width", 30, &bufW);
    ef.addTextField("Height", 30, &bufH);
    ef.addColorButton( "Line Color", actWin->ci, &lineCb, &bufLineColor );
    ef.addColorButton( "On Color/Rule", actWin->ci, &onColorCb, &bufOnColor );
    ef.addColorButton( "Off Color/Don't Care", actWin->ci, &offColorCb,
                        &bufOffColor );
    ef.addTextField("PV", 30, bufPvName, PV_Factory::MAX_PV_NAME);
    ef.addOption( "Line Thk", "0|1|2|3|4|5|6|7|8|9|10", &bufLineWidth );
    ef.addOption( "Line Style", "Solid|Dash", &bufLineStyle );
    ef.addOption( "Direction", "BigEndian|LittleEndian", (int *)&bufTheDir);
    ef.addTextField("Number of Bits", 30, &bufNobt);
    ef.addTextField("Shift", 30, &bufShft);

    return 1;
}

void edmByteClass::updateDimensions()
//
// Generates the outline of the byte widget, which is the segment list for an 
// XDrawSegments call.  It is also used to get coordinates for the 
// XFillRectangle calls for coloring the bits. 
//
// updateDimensions also sets up dmask and the alarm pixel values.
//
{
   float bitLen;

   dmask = 0;
   for (int i = 0, lmask = 1; i < nobt; i++, lmask <<=1)
   {
      dmask |= lmask;
   }

   minorPixel = actWin->ci->getPixelByIndex(
                    actWin->ci->getSpecialIndex(COLORINFO_K_MINOR));
   majorPixel = actWin->ci->getPixelByIndex(
                    actWin->ci->getSpecialIndex(COLORINFO_K_MAJOR));
   invalidPixel = actWin->ci->getPixelByIndex(
                    actWin->ci->getSpecialIndex(COLORINFO_K_INVALID));

   delete[] theOutline;

   theOutline = new XSegment[(nobt + 3) * 2];

   if (!theOutline) return;

   if (w > h)	// horizontal
   {
      bitLen = (float)w / nobt;
      for (int i=0, px = x; i <= nobt; i++, px = (int)(x + i * bitLen))
      {
         theOutline[i].x1 = px;
         theOutline[i].y1 = y;
         theOutline[i].x2 = px;
         theOutline[i].y2 = y + h;
      }
      theOutline[nobt + 1].x1 = x;
      theOutline[nobt + 1].y1 = y;
      theOutline[nobt + 1].x2 = x + w;
      theOutline[nobt + 1].y2 = y;
      theOutline[nobt + 2].x1 = x;
      theOutline[nobt + 2].y1 = y + h;
      theOutline[nobt + 2].x2= x + w;
      theOutline[nobt + 2].y2= y + h;
   }
   else // vertical
   {
      bitLen = (float)h / nobt;
      for (int i=0, py = y; i <= nobt; i++, py = (int)(y + i * bitLen))
      {
         theOutline[i].x1 = x;
         theOutline[i].y1 = py;
         theOutline[i].x2 = x + w;
         theOutline[i].y2 = py;
      }
      theOutline[nobt + 1].x1 = x;
      theOutline[nobt + 1].y1 = y;
      theOutline[nobt + 1].x2 = x;
      theOutline[nobt + 1].y2 = y + h;
      theOutline[nobt + 2].x1 = x + w;
      theOutline[nobt + 2].y1 = y;
      theOutline[nobt + 2].x2 = x + w;
      theOutline[nobt + 2].y2 = y + h;
   }
}

int edmByteClass::draw()  // render the edit-mode image
{
    // required
    if (is_executing || deleteRequest)
        return 1;
    actWin->drawGc.saveFg();
    
    // don't bother coloring individual bits in edit mode
       
    actWin->drawGc.setFG( offPixel );
    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.normGC(), x, y, w, h );

    actWin->drawGc.setFG( actWin->ci->getPixelByIndex(lineColor) );
    actWin->drawGc.setLineWidth( lineWidth );
    actWin->drawGc.setLineStyle( lineStyle );

    updateDimensions();

    if (theOutline)
    {
       XDrawSegments(actWin->d, XtWindow(actWin->drawWidget),
          actWin->drawGc.normGC(), theOutline, nobt + 3);
    }
    else
    {
       XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
          actWin->drawGc.normGC(), x, y, w, h );
    }

    actWin->drawGc.setLineWidth( 1 );
    actWin->drawGc.setLineStyle( LineSolid );
 
    actWin->drawGc.restoreFg();
    return 1;
}

int edmByteClass::erase()  // erase edit-mode image
{
    // required
    if (is_executing || deleteRequest )
        return 1;
    XFillRectangle( actWin->d, XtWindow(actWin->drawWidget),
       actWin->drawGc.eraseGC(), x, y, w, h );

    actWin->drawGc.setLineWidth( lineWidth );
    actWin->drawGc.setLineStyle( lineStyle );

    XDrawRectangle( actWin->d, XtWindow(actWin->drawWidget),
     actWin->drawGc.eraseGC(), x, y, w, h );

    actWin->drawGc.setLineWidth( 1 );
    actWin->drawGc.setLineStyle( LineSolid );

    return 1;
}

int edmByteClass::eraseUnconditional ( void ) {

  if ( !enabled ) return 1;

    XFillRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

  return 1;

}



// Callbacks from property dialog
void edmByteClass::edit_update(Widget w, XtPointer client,XtPointer call)
{
    edmByteClass *me = (edmByteClass *) client;
    // required
    me->actWin->setChanged();
    me->eraseSelectBoxCorners();
    me->erase();

    me->lineColor = me->bufLineColor;
    me->onColor = me->bufOnColor;
    me->offColor = me->bufOffColor;

    if (me->actWin->ci->isRule(me->onColor))
    {
       me->onPixel = me->actWin->ci->getPixelByIndex(
                     me->actWin->ci->evalRule(me->onColor, 1.0));
       me->offPixel = me->actWin->ci->getPixelByIndex(
                      me->actWin->ci->evalRule(me->onColor, 0.0));
    }
    else
    {
       me->onPixel = me->actWin->ci->getPixelByIndex(me->onColor);
       me->offPixel = me->actWin->ci->getPixelByIndex(me->offColor);
    }
  
    me->lineWidth = me->bufLineWidth;
  
    if ( me->bufLineStyle == 0 )
      me->lineStyle = LineSolid;
    else if ( me->bufLineStyle == 1 )
      me->lineStyle = LineOnOffDash;
  
  
    me->pv_exp_str.setRaw( me->bufPvName );

    me->theDir = me->bufTheDir;
    me->nobt = (me->bufNobt < 1)?1:((me->bufNobt > 16)?16:me->bufNobt);
    me->shft = (me->bufShft < 0)?0:((me->bufShft > 31)?31:me->bufShft);

    me->x = me->bufX;
    me->sboxX = me->bufX;

    me->y = me->bufY;
    me->sboxY = me->bufY;

    me->w = me->bufW;
    me->sboxW = me->bufW;

    me->h = me->bufH;
    me->sboxH = me->bufH;

    me->updateDimensions();

  
}

void edmByteClass::edit_ok(Widget w, XtPointer client, XtPointer call)
{
    edmByteClass *me = (edmByteClass *) client;
    edit_update(w, client, call);
    // required
    me->ef.popdown();
    me->operationComplete();
}

void edmByteClass::edit_apply(Widget w, XtPointer client, XtPointer call)
{
    edmByteClass *me = (edmByteClass *) client;
    edit_update(w, client, call);
    // required
    me->refresh(me);
}

void edmByteClass::edit_cancel(Widget w, XtPointer client,XtPointer call)
{
    edmByteClass *me = (edmByteClass *) client;
    // next two lines required
    me->ef.popdown();
    me->operationCancel();
}

void edmByteClass::edit_cancel_delete(Widget w, XtPointer client,
                                            XtPointer cal)
{
    edmByteClass *me = (edmByteClass *) client;
    // all lines required
    me->ef.popdown();
    me->operationCancel();
    me->erase();
    me->deleteRequest = 1;
    me->drawAll();
}

// --------------------------------------------------------
// GroupEdit
// --------------------------------------------------------
// 
void edmByteClass::changeDisplayParams(unsigned int flag,
                                             char *_fontTag,
                                             int _alignment,
                                             char *ctlFontTag,
                                             int ctlAlignment,
                                             char *btnFontTag,
                                             int btnAlignment,
                                             int textFgColor,
                                             int fg1Color,
                                             int fg2Color,
                                             int offsetColor,
                                             int bgColor,
                                             int topShadowColor,
                                             int botShadowColor)
{
}

void edmByteClass::changePvNames(int flag,
                                       int numCtlPvs,
                                       char *ctlPvs[],
                                       int numReadbackPvs,
                                       char *readbackPvs[],
                                       int numNullPvs,
                                       char *nullPvs[],
                                       int numVisPvs,
                                       char *visPvs[],
                                       int numAlarmPvs,
                                       char *alarmPvs[])
{
    if (flag & ACTGRF_READBACKPVS_MASK)
    {
        if (numReadbackPvs)
            pv_exp_str.setRaw(readbackPvs[0]);
    }

}

void edmByteClass::getPvs(int max,
			  ProcessVariable *pvs[],
			  int *n)
{
  if ( max < 1 ) {
    *n = 0;
    return;
  }

  *n = 1;
  pvs[0] = valuePvId;

}

char *edmByteClass::getSearchString (
  int i
) {

  if ( i == 0 ) {
    return pv_exp_str.getRaw();
  }

  return NULL;

}

void edmByteClass::replaceString (
  int i,
  int max,
  char *string
) {

  if ( i == 0 ) {
    pv_exp_str.setRaw( string );
  }

}    
// --------------------------------------------------------
// Macro support
// --------------------------------------------------------
int edmByteClass::containsMacros()
{   return pv_exp_str.containsPrimaryMacros(); }

int edmByteClass::expandTemplate (
  int numMacros,
  char *macros[],
  char *expansions[]
) {

expStringClass tmpStr;

  tmpStr.setRaw( pv_exp_str.getRaw() );
  tmpStr.expand1st( numMacros, macros, expansions );
  pv_exp_str.setRaw( tmpStr.getExpanded() );

  return 1;

}

int edmByteClass::expand1st(int numMacros, char *macros[],
                                  char *expansions[])
{   return pv_exp_str.expand1st(numMacros, macros, expansions); }

int edmByteClass::expand2nd(int numMacros, char *macros[],
                                  char *expansions[])
{   return pv_exp_str.expand2nd(numMacros, macros, expansions); }

// --------------------------------------------------------
// Execute
// --------------------------------------------------------
int edmByteClass::activate(int pass, void *ptr)
{
    switch (pass) // ... up to 6
    {
        case 1: // initialize
           aglPtr = ptr;

           is_executing = 1;
       
           is_executing = true;
           //is_pvname_valid = strcmp(getExpandedPVName(), "") != 0;
	   is_pvname_valid = !blankOrComment( (char *) getExpandedPVName() );

           init = 1;

            break;
        case 2: // connect to pv
            initEnable();
            if (valuePvId)
                fprintf( stderr,"byte::activate: pv already set!\n");
            if (is_pvname_valid)
            {
                valuePvId = the_PV_Factory->create(getExpandedPVName());
                if (valuePvId)
                {
                    valuePvId->add_conn_state_callback(pv_callback, this);
                    valuePvId->add_value_callback(pv_callback, this);
                }
            }
            break;
    }
    return 1;
}

int edmByteClass::deactivate(int pass)
{
  is_executing = 0;
  if ( pass == 1 ) {
            if (valuePvId)
            {
                valuePvId->remove_conn_state_callback(pv_callback, this);
                valuePvId->remove_value_callback(pv_callback, this);
                valuePvId->release();
                valuePvId = 0;
            }
   }

  return 1;
}

inline void edmByteClass::innerDrawFull(int value, int i, int mask, 
                                  int &previous, int &lastseg)
{
  int current;

  if ( !enabled ) return;

  if (i < nobt)
     current = (value & mask)?1:0;
  else
     current = previous?0:1; // save checking for nobt in next line.

  if (current != previous)
  {
     actWin->executeGc.setFG( previous?fgPixel:offPixel );

     if (w > h)
        XFillRectangle(actWin->d, drawable(actWin->executeWidget),
                    actWin->executeGc.normGC(), 
                    theOutline[lastseg].x1, 
                    theOutline[lastseg].y1,
                    theOutline[i].x1 - theOutline[lastseg].x1, h);
     else
        XFillRectangle(actWin->d, drawable(actWin->executeWidget),
                    actWin->executeGc.normGC(), 
                    theOutline[lastseg].x1, 
                    theOutline[lastseg].y1,
                    w, theOutline[i].y1 - theOutline[lastseg].y1);
     previous = current;
     lastseg = i;
   } 
}

int edmByteClass::drawActive()
{
   if (is_executing && enabled)
   {
      if (valuePvId->is_valid())
      {
         unsigned int severity;
         // logic for non-invalid alarm colors would go here.  
         severity = valuePvId->get_severity();
         switch(severity)
         {
             case NO_ALARM:  
                fgPixel = onPixel;
                break;
             case MINOR_ALARM:
                fgPixel = minorPixel;
                break;
             case MAJOR_ALARM:
                fgPixel = majorPixel;
                break;
             default:
                fgPixel = invalidPixel;
         }
         if (!validFlag || lastsev != severity) 
         {
            validFlag = true;
            lastsev = severity;
            bufInvalidate();
         }
      }
      else if (validFlag)
      {
         validFlag = false;
         bufInvalidate();
         fgPixel = invalidPixel;
      }
   }
   if (bufInvalid)
   {
      drawActiveFull();
   }
   else
   {
      drawActiveBits();
   }
   return 1;
}

inline void edmByteClass::innerDrawBits(int value, int i, int mask)
{

  if ( !enabled ) return;

    actWin->executeGc.setFG( (value & mask)?fgPixel:offPixel );

    if (w > h)
    {
       XFillRectangle(actWin->d, drawable(actWin->executeWidget),
                   actWin->executeGc.normGC(),
                   theOutline[i].x1, theOutline[i].y1,
                   theOutline[i+1].x1 - theOutline[i].x1, h);
       actWin->executeGc.setFG(actWin->ci->getPixelByIndex(lineColor) );
       XDrawRectangle(actWin->d, drawable(actWin->executeWidget),
                   actWin->executeGc.normGC(),
                   theOutline[i].x1, theOutline[i].y1,
                   theOutline[i+1].x1 - theOutline[i].x1, h);
    }
    else
    {
       XFillRectangle(actWin->d, drawable(actWin->executeWidget),
                   actWin->executeGc.normGC(),
                   theOutline[i].x1, theOutline[i].y1,
                   w, theOutline[i+1].y1 - theOutline[i].y1);
       actWin->executeGc.setFG(actWin->ci->getPixelByIndex(lineColor) );
       XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
                   actWin->executeGc.normGC(), 
                   theOutline[i].x1, theOutline[i].y1,
                   w, theOutline[i+1].y1 - theOutline[i].y1);
    }
}


int edmByteClass::drawActiveBits()
{
  if ( !enabled || !init || !is_executing ) return 1;

  actWin->executeGc.saveFg();

  if (!theOutline)
  {
     actWin->executeGc.setFG(offPixel);
     XFillRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h );
     actWin->executeGc.setFG( actWin->ci->getPixelByIndex(lineColor) );
     XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
  }
  else if (validFlag)
  {
     int i = 0;
     int mask;
     actWin->executeGc.setLineWidth( lineWidth );
     actWin->executeGc.setLineStyle( lineStyle );
     if (theDir == LITTLEENDIAN)
     {
        for (i = 0, mask = 1; i < nobt; i++, mask <<= 1)
        {
           if ((lastval ^ value) & mask)
              innerDrawBits(value, i, mask);
        }
     }
     else  // BIGENDIAN
     {
        for (i = 0, mask= 1 << (nobt -1); i < nobt; i++, mask >>= 1)
        {
           if ((lastval ^ value) & mask)
              innerDrawBits(value, i, mask);
        }
     }
  }
 
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

   
  actWin->executeGc.restoreFg();
  return 1;
}

int edmByteClass::drawActiveFull()
{
  if ( !enabled || !init || !is_executing ) return 1;

  actWin->executeGc.saveFg();

  if (!theOutline)
  {
     actWin->executeGc.setFG(0);
     XFillRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h );
     bufValidate();
  }
  else if (validFlag)
  {
     int previous = 0;
     int lastseg = 0;
     int i = 0;
     int mask;
     if (theDir == LITTLEENDIAN)
     {
        mask = 1;
        previous = (value & mask)?1:0;
        mask <<= 1;
        for (i = 1; i <= nobt; i++, mask <<= 1)
        {
           innerDrawFull(value, i, mask, previous, lastseg);
        }
     }
     else  // BIGENDIAN
     {
        mask = 1 << (nobt -1);
        previous = (value & mask)?1:0;
        mask >>= 1;
        for (i = 1; i <= nobt; i++, mask >>= 1)
        {
           innerDrawFull(value, i, mask, previous, lastseg);
        }
     }
     bufValidate();
  }
  else // disconnected
  {
    actWin->drawGc.setFG( 0 );	// white
    XFillRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
    bufValidate();
  }
 
  actWin->executeGc.setFG( actWin->ci->getPixelByIndex(lineColor) );
  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  if (!theOutline)
  {
     XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
  }
  else
  {
       XDrawSegments(actWin->d, drawable(actWin->executeWidget),
          actWin->executeGc.normGC(), theOutline, nobt + 3);
  }
  
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

   
  actWin->executeGc.restoreFg();
  return 1;
}

int edmByteClass::eraseActive()
{
      if ( !enabled || !init || !is_executing ) return 1;

  XFillRectangle( actWin->d, drawable(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, drawable(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

    return 1;
}

void edmByteClass::pv_callback(ProcessVariable *pv, void *userarg)
{
    edmByteClass *me = (edmByteClass *)userarg;
    me->actWin->appCtx->proc->lock();
    if (me->is_executing)
    {
       me->actWin->addDefExeNode(me->aglPtr);
    }
    me->actWin->appCtx->proc->unlock();
}


void edmByteClass::executeDeferred()
{   // Called as a result of addDefExeNode

    if (is_executing && valuePvId->is_valid())
    {
       lastval = value;
       value = ((valuePvId->get_int() >> shft) & dmask);
       if (!actWin->isIconified)
	 drawActive();

       actWin->appCtx->proc->lock();
       actWin->remDefExeNode(aglPtr);
       actWin->appCtx->proc->unlock();
    }
}

// Drag & drop support
char *edmByteClass::firstDragName()
{   
   if ( !enabled ) return NULL; 
   return "PV";
}

char *edmByteClass::nextDragName()
{
   return NULL;
}

char *edmByteClass::dragValue(int i)
{
   if ( !enabled ) return NULL; 

   if ( actWin->mode == AWC_EXECUTE ) {

      return (char *)getExpandedPVName();

   }
   else {

      return (char *)getRawPVName();

   }

}

// crawler functions may return blank pv names
char *edmByteClass::crawlerGetFirstPv ( void ) {

  crawlerPvIndex = 0;

  return pv_exp_str.getExpanded();

}

char *edmByteClass::crawlerGetNextPv ( void ) {

  return NULL;

}
