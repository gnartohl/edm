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
  is_executing(false), is_pvname_valid(false), valuePvId(0), bufInvalid(true),
  lastval(0), theDir(BIGENDIAN),  nobt(16),shft(0), lineWidth(1), 
  lineStyle(LineSolid), theOutline(0)
{
   name = strdup(BYTE_CLASSNAME);
}

edmByteClass::edmByteClass(edmByteClass *rhs)
{
    clone(rhs, BYTE_CLASSNAME);
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
    onPixel = rhs->onPixel;
    offPixel = rhs->offPixel;
    nobt = rhs->nobt;
    shft = rhs->shft;
    theDir = rhs->theDir;
    theOutline = 0;
}

edmByteClass::~edmByteClass()
{
    if (valuePvId)
    {
        valuePvId->remove_conn_state_callback(pv_conn_state_callback, this);
        valuePvId->remove_value_callback(pv_value_callback, this);
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
    // Version, bounding box
    fprintf(f, "%-d %-d %-d\n",
            BYTE_MAJOR, BYTE_MINOR, BYTE_RELEASE);
    fprintf(f, "%-d\n", x);
    fprintf(f, "%-d\n", y);
    fprintf(f, "%-d\n", w);
    fprintf(f, "%-d\n", h);

    fprintf( f, "%-d\n", lineColor );
    fprintf( f, "%-d\n", onColor );
    fprintf( f, "%-d\n", offColor );

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
    int major, minor, release, temp;
    char tname[40];

    actWin = _actWin;
    // Version, bounding box
    fscanf(f, "%d %d %d\n", &major, &minor, &release); actWin->incLine();
    fscanf(f, "%d\n", &x); actWin->incLine();
    fscanf(f, "%d\n", &y); actWin->incLine();
    fscanf(f, "%d\n", &w); actWin->incLine();
    fscanf(f, "%d\n", &h); actWin->incLine();
    this->initSelectBox(); // call after getting x,y,w,h

    fscanf( f, "%d\n", &lineColor ); actWin->incLine();
    fscanf( f, "%d\n", &onColor ); actWin->incLine();
    fscanf( f, "%d\n", &offColor ); actWin->incLine();
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
  readStringFromFile(tname, 39, f); actWin->incLine();
  pv_exp_str.setRaw(tname);


  fscanf( f, "%d\n", &lineWidth ); actWin->incLine();

  fscanf( f, "%d\n", &lineStyle ); actWin->incLine();

  fscanf( f, "%d\n", &temp ); actWin->incLine();
  temp = (temp < 0)?0:((temp > 1)?1:temp);
  theDir = (bdir)temp;
  fscanf( f, "%d\n", &temp ); actWin->incLine();
  nobt = (temp < 1)?1:((temp > 16)?16:temp);
  fscanf( f, "%d\n", &temp ); actWin->incLine();
  shft = (temp < 0)?0:((temp > 15)?15:temp);
  
  makeOutline();

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

    offPixel = actWin->ci->getPixelByIndex(actWin->bgColor);
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
    char title[80], *ptr;
    // required
    ptr = actWin->obj.getNameFromClass(name);
    if (ptr)
    {
        strncpy(title, ptr, 80);
        strncat(title, " Properties", 80);
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

  strncpy(bufPvName, getRawPVName(), 39);

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
    ef.addTextField("PV", 30, bufPvName, 39);
    ef.addOption( "Line Thk", "0|1|2|3|4|5|6|7|8|9|10", &bufLineWidth );
    ef.addOption( "Line Style", "Solid|Dash", &bufLineStyle );
    ef.addOption( "Direction", "BigEndian|LittleEndian", (int *)&bufTheDir);
    ef.addTextField("Number of Bits", 30, &bufNobt);
    ef.addTextField("Shift", 30, &bufShft);

    return 1;
}

int edmByteClass::makeOutline()
//
// The outline of the byte widget is the segment list for an XDrawSegments 
// call.  It is also used to get coordinates for the XFillRectangle calls 
// for coloring the bits. 
//
{
   float bitLen;

   delete[] theOutline;

   theOutline = new XSegment[(nobt + 3) * 2];

   if (!theOutline) return 0;

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
   
   return 1;
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

    makeOutline();

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

    XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
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
    me->shft = (me->bufShft < 0)?0:((me->bufShft > 15)?15:me->bufShft);

    me->x = me->bufX;
    me->sboxX = me->bufX;

    me->y = me->bufY;
    me->sboxY = me->bufY;

    me->w = me->bufW;
    me->sboxW = me->bufW;

    me->h = me->bufH;
    me->sboxH = me->bufH;

    me->makeOutline();
  
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
    
// --------------------------------------------------------
// Macro support
// --------------------------------------------------------
int edmByteClass::containsMacros()
{   return pv_exp_str.containsPrimaryMacros(); }


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
           is_pvname_valid = strcmp(getExpandedPVName(), "") != 0;

           init = 1;

            break;
        case 2: // connect to pv
            if (valuePvId)
                printf("textentry::activate: pv already set!\n");
            if (is_pvname_valid)
            {
                valuePvId = the_PV_Factory->create(getExpandedPVName());
                if (valuePvId)
                {
                    valuePvId->add_conn_state_callback(pv_conn_state_callback, this);
                    valuePvId->add_value_callback(pv_value_callback, this);
                }
            }
            bufInvalidate();
            if (!valuePvId)
                drawActive();

            break;
    }
    return 1;
}

void edmByteClass::bufInvalidate(void)
{
   bufInvalid = true;
}

int edmByteClass::deactivate(int pass)
{
  is_executing = 0;
  if ( pass == 1 ) {
            if (valuePvId)
            {
                valuePvId->remove_conn_state_callback(pv_conn_state_callback, this);
                valuePvId->remove_value_callback(pv_value_callback, this);
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

  if (i < nobt)
     current = (value & mask)?1:0;
  else
     current = previous?0:1; 	// save checking for nobt in next line.

  if (current != previous)
  {
     actWin->executeGc.setFG( previous?onPixel:offPixel );

     if (w > h)
        XFillRectangle(actWin->d, XtWindow(actWin->executeWidget),
                    actWin->executeGc.normGC(), 
                    theOutline[lastseg].x1, 
                    theOutline[lastseg].y1,
                    theOutline[i].x1 - theOutline[lastseg].x1, h);
     else
        XFillRectangle(actWin->d, XtWindow(actWin->executeWidget),
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
   if (bufInvalid)
   {
      bufInvalid = false;
      drawActiveFull();
   }
   else
      drawActiveBits();

   return 1;
}

inline void edmByteClass::innerDrawBits(int value, int i, int mask)
{
    actWin->executeGc.setFG( (value & mask)?onPixel:offPixel );

    if (w > h)
    {
       XFillRectangle(actWin->d, XtWindow(actWin->executeWidget),
                   actWin->executeGc.normGC(),
                   theOutline[i].x1, theOutline[i].y1,
                   theOutline[i+1].x1 - theOutline[i].x1, h);
       actWin->executeGc.setFG(actWin->ci->getPixelByIndex(lineColor) );
       XDrawRectangle(actWin->d, XtWindow(actWin->executeWidget),
                   actWin->executeGc.normGC(),
                   theOutline[i].x1, theOutline[i].y1,
                   theOutline[i+1].x1 - theOutline[i].x1, h);
    }
    else
    {
       XFillRectangle(actWin->d, XtWindow(actWin->executeWidget),
                   actWin->executeGc.normGC(),
                   theOutline[i].x1, theOutline[i].y1,
                   w, theOutline[i+1].y1 - theOutline[i].y1);
       actWin->executeGc.setFG(actWin->ci->getPixelByIndex(lineColor) );
       XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
                   actWin->executeGc.normGC(), 
                   theOutline[i].x1, theOutline[i].y1,
                   w, theOutline[i+1].y1 - theOutline[i].y1);
    }
}


int edmByteClass::drawActiveBits()
{
  unsigned int value;
  if ( !init || !is_executing ) return 1;

  actWin->executeGc.saveFg();

  if (!theOutline)
  {
     actWin->executeGc.setFG(offPixel);
     XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h );
     actWin->executeGc.setFG( actWin->ci->getPixelByIndex(lineColor) );
     XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
  }
  else if (valuePvId && valuePvId->is_valid())
  {
     value = valuePvId->get_int();
     int i = 0;
     int mask;
     actWin->executeGc.setLineWidth( lineWidth );
     actWin->executeGc.setLineStyle( lineStyle );
     if (theDir == LITTLEENDIAN)
     {
        for (i = 0, mask = 1 << shft; i <= nobt; i++, mask <<= 1)
        {
           if ((lastval ^ value) & mask)
              innerDrawBits(value, i, mask);
        }
     }
     else  // BIGENDIAN
     {
        for (i = 0, mask= 1 << (shft + nobt -1); i <= nobt; i++, mask >>= 1)
        {
           if ((lastval ^ value) & mask)
              innerDrawBits(value, i, mask);
        }
     }
     lastval = value;
  }
 
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

   
  actWin->executeGc.restoreFg();
  return 1;
}

int edmByteClass::drawActiveFull()
{
  unsigned int value;
  if ( !init || !is_executing ) return 1;

  actWin->executeGc.saveFg();

  if (!theOutline)
  {
     actWin->executeGc.setFG(offPixel);
     XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.normGC(), x, y, w, h );
  }
  else if (valuePvId && valuePvId->is_valid())
  {
     lastval = value;
     value = valuePvId->get_int();
     int previous = 0;
     int lastseg = 0;
     int i = 0;
     int mask;
     if (theDir == LITTLEENDIAN)
     {
        mask = 1 << shft;
        previous = (value & mask)?1:0;
        mask <<= 1;
        for (i = 1; i <= nobt; i++, mask <<= 1)
        {
           innerDrawFull(value, i, mask, previous, lastseg);
        }
     }
     else  // BIGENDIAN
     {
        mask = 1 << shft + nobt -1;
        previous = (value & mask)?1:0;
        mask >>= 1;
        for (i = 1; i <= nobt; i++, mask >>= 1)
        {
           innerDrawFull(value, i, mask, previous, lastseg);
        }
     }
  }
 
  actWin->executeGc.setFG( actWin->ci->getPixelByIndex(lineColor) );
  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  if (!theOutline)
  {
     XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
       actWin->executeGc.normGC(), x, y, w, h );
  }
  else
  {
       XDrawSegments(actWin->d, XtWindow(actWin->executeWidget),
          actWin->executeGc.normGC(), theOutline, nobt + 3);
  }
  
  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

   
  actWin->executeGc.restoreFg();
  return 1;
}

int edmByteClass::eraseActive()
{
      if ( !init || !is_executing ) return 1;

  XFillRectangle( actWin->d, XtWindow(actWin->executeWidget),
     actWin->executeGc.eraseGC(), x, y, w, h );

  actWin->executeGc.setLineWidth( lineWidth );
  actWin->executeGc.setLineStyle( lineStyle );

  XDrawRectangle( actWin->d, XtWindow(actWin->executeWidget),
   actWin->executeGc.eraseGC(), x, y, w, h );

  actWin->executeGc.setLineWidth( 1 );
  actWin->executeGc.setLineStyle( LineSolid );

    return 1;
}

void edmByteClass::pv_conn_state_callback(ProcessVariable *pv, void *userarg)
{
    edmByteClass *me = (edmByteClass *)userarg;
    me->actWin->appCtx->proc->lock();
    if (me->is_executing)
    {
        me->bufInvalidate();
        me->actWin->addDefExeNode(me->aglPtr);
    }
    me->actWin->appCtx->proc->unlock();
}

void edmByteClass::pv_value_callback(ProcessVariable *pv, void *userarg)
{
    edmByteClass *me = (edmByteClass *)userarg;
    me->actWin->appCtx->proc->lock();
    if (me->is_executing)
    {
        //me->bufInvalidate();
        me->actWin->addDefExeNode(me->aglPtr);
    }
    me->actWin->appCtx->proc->unlock();
}


void edmByteClass::executeDeferred()
{   // Called as a result of addDefExeNode
    if (actWin->isIconified)
        return;

    actWin->appCtx->proc->lock();
    actWin->remDefExeNode(aglPtr);
    actWin->appCtx->proc->unlock();

    if (is_executing)
       drawActive();
}

// Drag & drop support
char *edmByteClass::firstDragName()
{   return "PV"; }

char *edmByteClass::nextDragName()
{   return NULL; }

char *edmByteClass::dragValue(int i)
{   return (char *)getExpandedPVName(); }


