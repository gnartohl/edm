#define __color_pkg_cc 1

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

#include <math.h>
#include "color_pkg.h"
#include "color_button.h"
#include "utility.h"
#include "thread.h"
#include "remFileOpen.h"

static int showRGB = 0;

static void doCiBlink (
  void *ptr
) {

simpleButtonPtr sbp = (simpleButtonPtr) ptr;

  drawSimpleButton( sbp );

}

static void drawSimpleButton (
  simpleButtonPtr sbp
) {

int stat, blink = 0;

  sbp->cio->gc.setFG( sbp->colorIndex, &blink );
  XFillRectangle( XtDisplay(sbp->wgt), XtWindow(sbp->wgt),
   sbp->cio->gc.normGC(), sbp->x, sbp->y, 20, 20 );

  if ( sbp->cio->isRule( sbp->colorIndex ) ) {
    sbp->cio->gc.setFG( sbp->cio->labelPix(sbp->colorIndex) );
    XFillArc( XtDisplay(sbp->wgt), XtWindow(sbp->wgt),
     sbp->cio->gc.normGC(), sbp->x+7, sbp->y+7, 6, 6, 0, 23040 );
  }

  if ( sbp->colorIndex == sbp->cio->curIndex ) {
    sbp->cio->gc.setFG(
     BlackPixel( XtDisplay(sbp->wgt), DefaultScreen(XtDisplay(sbp->wgt)) ) );
    XDrawRectangle( XtDisplay(sbp->wgt), XtWindow(sbp->wgt),
    sbp->cio->gc.normGC(), sbp->x-2, sbp->y-2, 23, 23 );
  }

  if ( blink ) {
    if ( !sbp->blink ) {
      stat = sbp->cio->addToBlinkList( (void *) sbp, (void *) doCiBlink );
      sbp->blink = 1;
    }
  }
  else {
    if ( sbp->blink ) {
      stat = sbp->cio->removeFromBlinkList( (void *) sbp, (void *) doCiBlink );
      sbp->blink = 0;
    }
  }

}

void showColorName (
  XtPointer client,
  XtIntervalId *id )
{

showNameBlockPtr block = (showNameBlockPtr) client;
colorInfoClass *cio = (colorInfoClass *) block->ptr;
int x, y, i;
Window root, child;
int rootX, rootY, winX, winY;
unsigned int mask;

  if ( !cio->showNameTimerActive ) return;

  cio->showNameTimerActive = 0;

  XQueryPointer( cio->display, XtWindow(cio->shell), &root, &child,
   &rootX, &rootY, &winX, &winY, &mask );

  x = rootX + 10;
  y = rootY + 10;
  i = block->i;

  cio->msgDialog.popup( cio->colorName(i), x, y );

}

void doColorBlink (
  XtPointer client,
  XtIntervalId *id )
{

colorInfoClass *cio = (colorInfoClass *) client;
int i;

  if ( !cio->incrementTimerActive ) return;

  cio->incrementTimer = appAddTimeOut( cio->appCtx,
   cio->incrementTimerValue, doColorBlink, client );

  if ( cio->blink ) {
    cio->blink = 0;
    for ( i=0; i<cio->num_blinking_colors; i++ ) {
      XStoreColor( cio->display, cio->cmap, &cio->offBlinkingXColor[i] );
    }
  }
  else {
    cio->blink = 1;
    for ( i=0; i<cio->num_blinking_colors; i++ ) {
      XStoreColor( cio->display, cio->cmap, &cio->blinkingXColor[i] );
    }
  }

  XFlush( cio->display );

}

typedef void (*vfunc)( void *ptr );

void toggleColorBlink (
  XtPointer client,
  XtIntervalId *id )
{

colorInfoClass *cio = (colorInfoClass *) client;
blinkNodePtr cur;
int stat;
vfunc vf;

  if ( !cio->incrementTimerActive ) return;

  cio->incrementTimer = appAddTimeOut( cio->appCtx,
   cio->incrementTimerValue, toggleColorBlink, client );

  if ( cio->blink ) {
    cio->blink = 0;
  }
  else {
    cio->blink = 1;
  }

  // update request lists; these are lists of request to be added/removed
  // to/from the blink list.
  cio->addAllToBlinkList();
  cio->removeAllFromBlinkList();

  stat = avl_get_first( cio->blinkH, (void **) &cur );
  if ( !( stat & 1 ) ) return;

  while ( cur ) {

    //fprintf( stderr, "obj = %-d\n", (int) cur->obj );

    vf = (vfunc) cur->func;
    if ( vf ) {
      (*vf)( cur->obj );
    }
    else {
      fprintf( stderr, colorInfoClass_str35 );
    }

    stat = avl_get_next( cio->blinkH, (void **) &cur );
    if ( !( stat & 1 ) ) return;

  }

}

void colorShellEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

colorInfoClass *cio;
int i;

  cio = (colorInfoClass *) client;

  *continueToDispatch = False;

  if ( e->type == UnmapNotify ) {

    for ( i=0; i<cio->max_colors+cio->num_blinking_colors; i++ ) {
      if ( cio->simpleColorButtons[i].blink ) {
        cio->removeFromBlinkList( (void *) &cio->simpleColorButtons[i],
         (void *) doCiBlink );
        cio->simpleColorButtons[i].blink = 0;
      }
    }

    if ( cio->showNameTimerActive ) {
      cio->showNameTimerActive = 0;
      XtRemoveTimeOut( cio->showNameTimer );
    }
    cio->msgDialog.popdown();
    cio->curPaletteRow = -1;
    cio->curPaletteCol = -1;

  }

}

void colorFormEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

XMotionEvent *me;
XExposeEvent *expe;
XButtonEvent *be;
XCrossingEvent *ce;
colorInfoClass *cio;
int stat, x, y, pos, i, r, c, ncols, nrows, remainder, count=0;
unsigned int bg;
int *dest;
int red, green, blue;
colorCachePtr cur;

  cio = (colorInfoClass *) client;

  *continueToDispatch = False;

  if ( ( e->type == Expose ) || ( e->type == ConfigureNotify ) ) {

    if ( e->type == Expose ) {
      expe = (XExposeEvent *) e;
      count = expe->count;
    }
    else if ( e->type == ConfigureNotify ) {
      count = 0;
    }

    if ( !count ) {

      ncols = cio->num_color_cols;
      nrows = (cio->max_colors+cio->num_blinking_colors) / ncols;
      remainder = (cio->max_colors+cio->num_blinking_colors) % ncols;

      pos = 0;
      for ( r=0; r<nrows; r++ ) {

        for ( c=0; c<ncols; c++ ) {

          x = c*5 + c*20 + 5;
          y = r*5 + r*20 + 5;

          stat = avl_get_match( cio->colorCacheByPosH, (void *) &pos,
           (void **) &cur );
          if ( !( stat & 1 ) ) {
            i = 0;
          }
          else if ( !cur ) {
            i = 0;
          }
          else {
            //i = cur->index;
            i = pos;
	  }

          drawSimpleButton( &cio->simpleColorButtons[i] );
#if 0
          cio->gc.setFG( cio->colors[i] );
          XFillRectangle( cio->display, XtWindow(cio->form), cio->gc.normGC(),
           x, y, 20, 20 );

          if ( cio->isRule( i ) ) {
            cio->gc.setFG( cio->labelPix(i) );
            XFillArc( cio->display, XtWindow(cio->form),
             cio->gc.normGC(), x+7, y+7, 6, 6, 0, 23040 );
	  }

          if ( i == cio->curIndex ) {
            cio->gc.setFG(
             BlackPixel( cio->display, DefaultScreen(cio->display) ) );
            XDrawRectangle( cio->display, XtWindow(cio->form),
             cio->gc.normGC(), x-2, y-2, 23, 23 );
          }
#endif

          pos++;

        }

      }

      if ( remainder ) {

        r = nrows;

        for ( c=0; c<remainder; c++ ) {

          x = c*5 + c*20 + 5;
          y = r*5 + r*20 + 5;

          stat = avl_get_match( cio->colorCacheByPosH, (void *) &pos,
           (void **) &cur );
          if ( !( stat & 1 ) ) {
            i = 0;
          }
          else if ( !cur ) {
            i = 0;
          }
          else {
            //i = cur->index;
            i = pos;
	  }

          drawSimpleButton( &cio->simpleColorButtons[i] );
#if 0
          cio->gc.setFG( cio->colors[i] );
          XFillRectangle( cio->display, XtWindow(cio->form), cio->gc.normGC(),
           x, y, 20, 20 );

          if ( cio->isRule( i ) ) {
            cio->gc.setFG( cio->labelPix(i) );
            XFillArc( cio->display, XtWindow(cio->form),
             cio->gc.normGC(), x+7, y+7, 6, 6, 0, 23040 );
	  }

          if ( i == cio->curIndex ) {
            cio->gc.setFG(
             BlackPixel( cio->display, DefaultScreen(cio->display) ) );
            XDrawRectangle( cio->display, XtWindow(cio->form),
             cio->gc.normGC(), x-2, y-2, 23, 23 );
          }
#endif

          pos++;

        }

      }

    }

  }
  else if ( e->type == LeaveNotify ) {

    ce = (XCrossingEvent *) e;

    if ( cio->showNameTimerActive ) {
      cio->showNameTimerActive = 0;
      XtRemoveTimeOut( cio->showNameTimer );
    }
    cio->msgDialog.popdown();
    cio->curPaletteRow = -1;
    cio->curPaletteCol = -1;

  }
  else if ( e->type == MotionNotify ) {

    me = (XMotionEvent *) e;

    ncols = cio->num_color_cols;
    nrows = (cio->max_colors+cio->num_blinking_colors) / ncols;
    remainder = (cio->max_colors+cio->num_blinking_colors) % ncols;
    if ( remainder ) nrows++;

    r = me->y / 25;
    if ( r > nrows-1 ) r = nrows-1;
    c = me->x / 25;
    if ( c > ncols-1 ) c = ncols-1;

    pos = r * ncols + c;
    if ( pos > cio->numColors-1 ) pos = cio->numColors-1;

    stat = avl_get_match( cio->colorCacheByPosH, (void *) &pos,
     (void **) &cur );
    if ( !( stat & 1 ) ) {
      i = 0;
    }
    else if ( !cur ) {
      i = 0;
    }
    else {
      i = cur->index;
    }

    if ( ( r != cio->curPaletteRow ) || ( c != cio->curPaletteCol ) ) {
      cio->msgDialog.popdown();
      if ( cio->showNameTimerActive ) {
        cio->showNameTimerActive = 0;
        XtRemoveTimeOut( cio->showNameTimer );
      }
      cio->showNameBlock.x = me->x_root;
      cio->showNameBlock.y = me->y_root;
      cio->showNameBlock.i = i;
      cio->showNameBlock.ptr = (void *) cio;
      cio->showNameTimerActive = 1;
      cio->showNameTimer = appAddTimeOut( cio->appCtx, 500, showColorName,
       &cio->showNameBlock );

      //cio->msgDialog.popup( cio->colorName(i), me->x_root, me->y_root+25 );
      cio->curPaletteRow = r;
      cio->curPaletteCol = c;
    }

  }
  else if ( e->type == ButtonPress ) {

    be = (XButtonEvent *) e;

    ncols = cio->num_color_cols;
    nrows = (cio->max_colors+cio->num_blinking_colors) / ncols;
    remainder = (cio->max_colors+cio->num_blinking_colors) % ncols;
    if ( remainder ) nrows++;

    r = be->y / 25;
    if ( r > nrows-1 ) r = nrows-1;
    c = be->x / 25;
    if ( c > ncols-1 ) c = ncols-1;

    pos = r * ncols + c;
    if ( pos > cio->numColors-1 ) pos = cio->numColors-1;

    stat = avl_get_match( cio->colorCacheByPosH, (void *) &pos,
     (void **) &cur );
    if ( !( stat & 1 ) ) {
      i = 0;
    }
    else if ( !cur ) {
      i = 0;
    }
    else {
      i = cur->index;
    }

    cio->setCurIndex( i );

    if ( cio->curCb ) cio->curCb->setIndex( i );

    bg = cio->colors[i];

    cio->change = 1;

    if ( cio->menuPosition(i) ) {
      XmListSelectPos( cio->colorList.listWidget(), cio->menuPosition(i),
       FALSE );
      XmListSetBottomPos( cio->colorList.listWidget(), cio->menuPosition(i) );
    }
    else {
      XmListDeselectAllItems( cio->colorList.listWidget() );
    }

    dest = cio->getCurDestination();
    if ( dest ) {
      *dest = i;
    }

    if ( showRGB ) {
      cio->getRGB( bg, &red, &green, &blue );
      fprintf( stderr, colorInfoClass_str8, i, red, green, blue );
    }

  }

}

static int compare_nodes_by_name (
  void *node1,
  void *node2
) {

colorCachePtr p1, p2;

  p1 = (colorCachePtr) node1;
  p2 = (colorCachePtr) node2;

  return strcmp( p1->name, p2->name );

}

static int compare_key_by_name (
  void *key,
  void *node
) {

colorCachePtr p;
char *oneIndex;

  p = (colorCachePtr) node;
  oneIndex = (char *) key;

  return strcmp( oneIndex, p->name );

}

static int compare_nodes_by_index (
  void *node1,
  void *node2
) {

colorCachePtr p1, p2;

  p1 = (colorCachePtr) node1;
  p2 = (colorCachePtr) node2;

  if ( p1->index > p2->index )
      return 1;
  else if ( p1->index < p2->index )
    return -1;

  return 0;

}

static int compare_key_by_index (
  void *key,
  void *node
) {

colorCachePtr p;
int *oneIndex;

  p = (colorCachePtr) node;
  oneIndex = (int *) key;

  if ( *oneIndex > p->index )
      return 1;
  else if ( *oneIndex < p->index )
    return -1;

  return 0;

}

static int compare_nodes_by_pixel (
  void *node1,
  void *node2
) {

colorCachePtr p1, p2;

  p1 = (colorCachePtr) node1;
  p2 = (colorCachePtr) node2;

  if ( p1->pixel > p2->pixel )
      return 1;
  else if ( p1->pixel < p2->pixel )
    return -1;

  return 0;

}

static int compare_key_by_pixel (
  void *key,
  void *node
) {

colorCachePtr p;
unsigned int *onePixel;

  p = (colorCachePtr) node;
  onePixel = (unsigned int *) key;

  if ( *onePixel > p->pixel )
      return 1;
  else if ( *onePixel < p->pixel )
    return -1;

  return 0;

}

static int compare_nodes_by_color (
  void *node1,
  void *node2
) {

int i;
colorCachePtr p1, p2;

  p1 = (colorCachePtr) node1;
  p2 = (colorCachePtr) node2;

  for ( i=0; i<3; i++ ) {
    if ( p1->rgb[i] > p2->rgb[i] )
      return 1;
    else if ( p1->rgb[i] < p2->rgb[i] )
      return -1;
  }

  return 0;

}

static int compare_key_by_color (
  void *key,
  void *node
) {

int i;
colorCachePtr p;
int *oneRgb;

  p = (colorCachePtr) node;
  oneRgb = (int *) key;

  for ( i=0; i<3; i++ ) {
    if ( oneRgb[i] > p->rgb[i] )
      return 1;
    else if ( oneRgb[i] < p->rgb[i] )
      return -1;
  }

  return 0;

}

static int compare_nodes_by_pos (
  void *node1,
  void *node2
) {

colorCachePtr p1, p2;

  p1 = (colorCachePtr) node1;
  p2 = (colorCachePtr) node2;

  if ( p1->position > p2->position )
      return 1;
  else if ( p1->position < p2->position )
    return -1;

  return 0;

}

static int compare_key_by_pos (
  void *key,
  void *node
) {

colorCachePtr p;
int *onePosition;

  p = (colorCachePtr) node;
  onePosition = (int *) key;

  if ( *onePosition > p->position )
      return 1;
  else if ( *onePosition < p->position )
    return -1;

  return 0;

}

static int copy_nodes (
  void *node1,
  void *node2
) {

colorCachePtr p1, p2;
ruleConditionPtr cur1, cur2;

  p1 = (colorCachePtr) node1;
  p2 = (colorCachePtr) node2;

  *p1 = *p2;

  // give p1 a copy of the name
  if ( p2->name ) {
    p1->name = new char[ strlen(p2->name) + 1 ];
    strcpy( p1->name, p2->name );
  }

  // give p1 a copy of the rule list
  if ( p2->rule ) {

    p1->rule->ruleHead = new ruleConditionType;
    p1->rule->ruleTail = p1->rule->ruleHead;
    p1->rule->ruleTail->flink = NULL;

    cur2 = p1->rule->ruleHead->flink;
    while ( cur2 ) {

      cur1 = new ruleConditionType;
      *cur1 = *cur2;
      cur1->resultName = new char[strlen(cur2->resultName) + 1];
      strcpy(cur1->resultName,  cur2->resultName );

      p1->rule->ruleTail->flink = cur1;
      p1->rule->ruleTail = cur1;
      p1->rule->ruleTail->flink = NULL;

      cur2 = cur2->flink;

    }

  }

  return 1;

}

static int compare_blink_nodes (
  void *node1,
  void *node2
) {

blinkNodePtr p1, p2;

  p1 = (blinkNodePtr) node1;
  p2 = (blinkNodePtr) node2;

  if ( (unsigned long) p1->obj < (unsigned long) p2->obj )
    return -1;
  else if ( (unsigned long) p1->obj > (unsigned long) p2->obj )
    return 1;
  else
    return 0;

}

static int compare_blink_key (
  void *key,
  void *node
) {

blinkNodePtr p;
void *oneIndex;

  p = (blinkNodePtr) node;
  oneIndex = (void *) key;

  if ( (unsigned long) oneIndex < (unsigned long) p->obj )
    return -1;
  else if ( (unsigned long) oneIndex > (unsigned long) p->obj )
    return 1;
  else
    return 0;

}

static int copy_blink_nodes (
  void *node1,
  void *node2
) {

blinkNodePtr p1, p2;

  p1 = (blinkNodePtr) node1;
  p2 = (blinkNodePtr) node2;

  *p1 = *p2;

  return 1;

}

static void copyRuleCondition (
  ruleConditionPtr dst,
  ruleConditionPtr src
) {

  dst->value1 = src->value1;
  dst->ruleFunc1 = src->ruleFunc1;
  dst->value2 = src->value2;
  dst->ruleFunc2 = src->ruleFunc2;
  dst->connectingFunc = src->connectingFunc;
  dst->joiningFunc = src->joiningFunc;
  dst->result = src->result;
  dst->resultName = new char[strlen(src->resultName)+1];
  strcpy( dst->resultName, src->resultName );

}

static void colorCacheInit (
  colorCachePtr cur
) {

  cur->rgb[0] = 0;
  cur->rgb[1] = 0;
  cur->rgb[2] = 0;
  cur->pixel = 0;
  cur->blinkRgb[0] = 0;
  cur->blinkRgb[1] = 0;
  cur->blinkRgb[2] = 0;
  cur->blinkPixel = 0;
  cur->index = 0;
  cur->position = 0;
  cur->name = NULL;
  cur->aliasValue = NULL;
  cur->rule = NULL;

}

colorInfoClass::colorInfoClass ( void ) {

int stat;

  change = 1;
  max_colors = 0;
  num_blinking_colors = 0;
  num_color_cols = 0;
  usingPrivateColorMap = 0;

  fg = 0;
  activeWidget = NULL;
  nameWidget = NULL;
  curDestination = NULL;
  curCb = NULL;
  colorWindowIsOpen = 0;

  stat = avl_init_tree( compare_nodes_by_color,
   compare_key_by_color, copy_nodes, &(this->colorCacheByColorH) );
  if ( !( stat & 1 ) ) this->colorCacheByColorH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_pixel,
   compare_key_by_pixel, copy_nodes, &(this->colorCacheByPixelH) );
  if ( !( stat & 1 ) ) this->colorCacheByPixelH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_index,
   compare_key_by_index, copy_nodes, &(this->colorCacheByIndexH) );
  if ( !( stat & 1 ) ) this->colorCacheByIndexH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_name,
   compare_key_by_name, copy_nodes, &(this->colorCacheByAliasH) );
  if ( !( stat & 1 ) ) this->colorCacheByAliasH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_name,
   compare_key_by_name, copy_nodes, &(this->colorCacheByNameH) );
  if ( !( stat & 1 ) ) this->colorCacheByNameH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_pos,
   compare_key_by_pos, copy_nodes, &(this->colorCacheByPosH) );
  if ( !( stat & 1 ) ) this->colorCacheByPosH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_blink_nodes,
   compare_blink_key, copy_blink_nodes, &(this->blinkH) );
  if ( !( stat & 1 ) ) this->blinkH = (AVL_HANDLE) NULL;

  blinkLookasideHead = new blinkNodeType; // sentinel node
  blinkLookasideTail = blinkLookasideHead;
  blinkLookasideTail->next = NULL;

  addBlinkHead = new blinkNodeType; // sentinel node
  addBlinkTail = addBlinkHead;
  addBlinkTail->next = NULL;

  remBlinkHead = new blinkNodeType; // sentinel node
  remBlinkTail = remBlinkHead;
  remBlinkTail->next = NULL;

  curPaletteRow = -1;
  curPaletteCol = -1;

  showNameTimer = 0;
  showNameTimerActive = 0;

  incrementTimerActive = 0;

  invisibleIndex = -1;

  useIndexFlag = 1;

  usePrivColorMapFlag = 0;

  menuIndexMap = NULL;

  showNoAlarmState = 1;

}

colorInfoClass::~colorInfoClass ( void ) {

colorCachePtr cur;
ruleConditionPtr curRule, nextRule;
blinkNodePtr curBlink, nextBlink;
int i, stat;

  if ( menuIndexMap ) {
    delete[] menuIndexMap;
    menuIndexMap = NULL;
  }

  for ( i=0; i<max_colors+num_blinking_colors; i++ ) {
    if ( simpleColorButtons[i].blink ) {
      removeFromBlinkList( (void *) &simpleColorButtons[i],
       (void *) doCiBlink );
      simpleColorButtons[i].blink = 0;
    }
  }

  XtDestroyWidget( shell );

  if ( showNameTimerActive ) {
    showNameTimerActive = 0;
    XtRemoveTimeOut( showNameTimer );
  }

  if ( incrementTimerActive ) {
    incrementTimerActive = 0;
    XtRemoveTimeOut( incrementTimer );
  }

  curBlink = blinkLookasideHead->next;
  while ( curBlink ) {
    nextBlink = curBlink->next;
    delete curBlink;
    curBlink = nextBlink;
  }
  delete blinkLookasideHead;

  curBlink = addBlinkHead->next;
  while ( curBlink ) {
    nextBlink = curBlink->next;
    delete curBlink;
    curBlink = nextBlink;
  }
  delete addBlinkHead;

  curBlink = remBlinkHead->next;
  while ( curBlink ) {
    nextBlink = curBlink->next;
    delete curBlink;
    curBlink = nextBlink;
  }
  delete remBlinkHead;

  //------------------------------------------------------------------------

  // Delete avl trees

  stat = avl_get_first( this->colorCacheByPosH, (void **) &cur );
  if ( !( stat & 1 ) ) {
    cur = NULL;
  }

  while ( cur ) {

    stat = avl_delete_node( this->colorCacheByPosH, (void **) &cur );
    if ( stat & 1 ) {

      if ( cur->name ) {
        delete[] cur->name;
        cur->name = NULL;
      }

      if ( cur->aliasValue ) {
        delete[] cur->aliasValue;
        cur->aliasValue = NULL;
      }

      if ( cur->rule ) {

        curRule = cur->rule->ruleHead->flink;
        while ( curRule ) {

          nextRule = curRule->flink;

          if ( curRule->resultName ) {
            delete[] curRule->resultName;
            curRule->resultName = NULL;
	  }

          delete curRule;

          curRule = nextRule;

	}
        delete cur->rule->ruleHead;
        cur->rule->ruleHead = NULL;

        delete cur->rule;
        cur->rule = NULL;

      }

      delete cur;
      cur = NULL;

    }

    stat = avl_get_first( this->colorCacheByPosH, (void **) &cur );
    if ( !( stat & 1 ) ) {
      cur = NULL;
    }

  }    

  //-------------------------------------------------------------------

  stat = avl_get_first( this->colorCacheByAliasH, (void **) &cur );
  if ( !( stat & 1 ) ) {
    cur = NULL;
  }

  while ( cur ) {

    stat = avl_delete_node( this->colorCacheByAliasH, (void **) &cur );
    if ( stat & 1 ) {

      if ( cur->name ) {
        delete[] cur->name;
        cur->name = NULL;
      }

      if ( cur->aliasValue ) {
        delete[] cur->aliasValue;
        cur->aliasValue = NULL;
      }

      if ( cur->rule ) {

        curRule = cur->rule->ruleHead->flink;
        while ( curRule ) {

          nextRule = curRule->flink;

          if ( curRule->resultName ) {
            delete[] curRule->resultName;
            curRule->resultName = NULL;
	  }

          delete curRule;

          curRule = nextRule;

	}
        delete cur->rule->ruleHead;
        cur->rule->ruleHead = NULL;

        delete cur->rule;
        cur->rule = NULL;

      }

      delete cur;
      cur = NULL;

    }

    stat = avl_get_first( this->colorCacheByAliasH, (void **) &cur );
    if ( !( stat & 1 ) ) {
      cur = NULL;
    }

  }    

  //-------------------------------------------------------------------

  stat = avl_get_first( this->colorCacheByNameH, (void **) &cur );
  if ( !( stat & 1 ) ) {
    cur = NULL;
  }

  while ( cur ) {

    stat = avl_delete_node( this->colorCacheByNameH, (void **) &cur );
    if ( stat & 1 ) {

      if ( cur->name ) {
        delete[] cur->name;
        cur->name = NULL;
      }

      if ( cur->aliasValue ) {
        delete[] cur->aliasValue;
        cur->aliasValue = NULL;
      }

      if ( cur->rule ) {

        curRule = cur->rule->ruleHead->flink;
        while ( curRule ) {

          nextRule = curRule->flink;

          if ( curRule->resultName ) {
            delete[] curRule->resultName;
            curRule->resultName = NULL;
	  }

          delete curRule;

          curRule = nextRule;

	}
        delete cur->rule->ruleHead;
        cur->rule->ruleHead = NULL;

        delete cur->rule;
        cur->rule = NULL;

      }

      delete cur;
      cur = NULL;

    }

    stat = avl_get_first( this->colorCacheByNameH, (void **) &cur );
    if ( !( stat & 1 ) ) {
      cur = NULL;
    }

  }    

  //-------------------------------------------------------------------

  stat = avl_get_first( this->colorCacheByIndexH, (void **) &cur );
  if ( !( stat & 1 ) ) {
    cur = NULL;
  }

  while ( cur ) {

    stat = avl_delete_node( this->colorCacheByIndexH, (void **) &cur );
    if ( stat & 1 ) {

      if ( cur->name ) {
        delete[] cur->name;
        cur->name = NULL;
      }

      if ( cur->aliasValue ) {
        delete[] cur->aliasValue;
        cur->aliasValue = NULL;
      }

      if ( cur->rule ) {

        curRule = cur->rule->ruleHead->flink;
        while ( curRule ) {

          nextRule = curRule->flink;

          if ( curRule->resultName ) {
            delete[] curRule->resultName;
            curRule->resultName = NULL;
	  }

          delete curRule;

          curRule = nextRule;

	}
        delete cur->rule->ruleHead;
        cur->rule->ruleHead = NULL;

        delete cur->rule;
        cur->rule = NULL;

      }

      delete cur;
      cur = NULL;

    }

    stat = avl_get_first( this->colorCacheByIndexH, (void **) &cur );
    if ( !( stat & 1 ) ) {
      cur = NULL;
    }

  }    

  //-------------------------------------------------------------------

  stat = avl_get_first( this->colorCacheByColorH, (void **) &cur );
  if ( !( stat & 1 ) ) {
    cur = NULL;
  }

  while ( cur ) {

    stat = avl_delete_node( this->colorCacheByColorH, (void **) &cur );
    if ( stat & 1 ) {

      if ( cur->name ) {
        delete[] cur->name;
        cur->name = NULL;
      }

      if ( cur->aliasValue ) {
        delete[] cur->aliasValue;
        cur->aliasValue = NULL;
      }

      if ( cur->rule ) {

        curRule = cur->rule->ruleHead->flink;
        while ( curRule ) {

          nextRule = curRule->flink;

          if ( curRule->resultName ) {
            delete[] curRule->resultName;
            curRule->resultName = NULL;
	  }

          delete curRule;

          curRule = nextRule;

	}
        delete cur->rule->ruleHead;
        cur->rule->ruleHead = NULL;

        delete cur->rule;
        cur->rule = NULL;

      }

      delete cur;
      cur = NULL;

    }

    stat = avl_get_first( this->colorCacheByColorH, (void **) &cur );
    if ( !( stat & 1 ) ) {
      cur = NULL;
    }

  }    

  //-------------------------------------------------------------------

  stat = avl_destroy( colorCacheByColorH );
  stat = avl_destroy( colorCacheByPixelH );
  stat = avl_destroy( colorCacheByIndexH );
  stat = avl_destroy( colorCacheByAliasH );
  stat = avl_destroy( colorCacheByNameH );
  stat = avl_destroy( colorCacheByPosH );
  stat = avl_destroy( blinkH );

  delete[] colors;
  colors = NULL;
  delete[] blinkingColors;
  blinkingColors = NULL;
  delete[] simpleColorButtons;
  simpleColorButtons = NULL;
  delete[] colorNames;
  colorNames = NULL;
  delete[] colorNodes;
  colorNodes = NULL;

}

static void file_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmPushButtonCallbackStruct *cb;
long num;
Widget p, prevP, curP;

  num = (long) client;
  cb = (XmPushButtonCallbackStruct *) call;

  if ( num == 0 ) {   // close window

    /* find topmost widget */
    prevP = curP = p = w;
    do {
      if ( XtParent(p) ) prevP = p;
      p = XtParent(p);
      if ( p ) curP = p;
    } while ( p );

    //XtUnmapWidget( curP );
    XtUnmapWidget( prevP );

  }
  else if ( num == 1 ) {

    if ( showRGB )
     showRGB = 0;
    else
      showRGB = 1;

  }

}

void colorInfoClass::initParseEngine (
  FILE *f )
{

  readFile = 1;
  tokenState = GET_1ST_NONWS_CHAR;
  parseIndex = -1;
  parseLine = 2;
  parseFile = f;
  colorIndex = 0;
  colorPosition = 0;

}

void colorInfoClass::parseError (
char *msg )
{

  fprintf( stderr, colorInfoClass_str6, parseLine, msg );

}

int colorInfoClass::getToken (
  char token[MAX_LINE_SIZE+1]
) {

int gotToken, l;
char *ptr;

  gotToken = 0;
  do {

    if ( readFile ) {
      tokenState = GET_1ST_NONWS_CHAR;
      ptr = fgets( parseBuf, MAX_LINE_SIZE, parseFile );
      parseBuf[MAX_LINE_SIZE] = 0;
      if ( !ptr ) {
        strcpy( token, "" );
        return SUCCESS;
      }
      parseLine++;
      readFile = 0;
      parseIndex = -1;
    }

    parseIndex++;

    switch ( tokenState ) {

    case GET_1ST_NONWS_CHAR:

      if ( parseBuf[parseIndex] == 0 ) {
	readFile = 1;
        continue;
      }

      if ( isspace(parseBuf[parseIndex]) || 
           ( parseBuf[parseIndex] == ',' ) ) continue;

      tokenFirst = parseIndex;

      if ( parseBuf[parseIndex] == '"' ) {
        tokenFirst = parseIndex + 1;
        tokenState = GET_TIL_END_OF_QUOTE;
      }
      else if ( ( parseBuf[parseIndex] == '<' ) ||
                ( parseBuf[parseIndex] == '>' ) ||
                ( parseBuf[parseIndex] == '=' ) ||
                ( parseBuf[parseIndex] == '|' ) ||
                ( parseBuf[parseIndex] == '&' ) ||
                ( parseBuf[parseIndex] == '!' ) ) {
        tokenState = GET_TIL_END_OF_SPECIAL;
      }
      else if ( parseBuf[parseIndex] == '#' ) {
        readFile = 1;
        tokenState = GET_1ST_NONWS_CHAR;
      }
      else {
        tokenState = GET_TIL_END_OF_TOKEN;
      }

      break;

    case GET_TIL_END_OF_TOKEN:

      if ( parseBuf[parseIndex] == 0 ) {
	readFile = 1;
      }
      else if ( parseBuf[parseIndex] == '"' ) {
        tokenLast = parseIndex - 1;
        tokenNext = parseIndex + 1;
        gotToken = 1;
        tokenState = GET_TIL_END_OF_QUOTE;
      }
      else if ( ( parseBuf[parseIndex] == '<' ) ||
                ( parseBuf[parseIndex] == '>' ) ||
                ( parseBuf[parseIndex] == '=' ) ||
                ( parseBuf[parseIndex] == '|' ) ||
                ( parseBuf[parseIndex] == '&' ) ||
                ( parseBuf[parseIndex] == '!' ) ) {
        tokenLast = parseIndex - 1;
        tokenNext = parseIndex;
        gotToken = 1;
        tokenState = GET_TIL_END_OF_SPECIAL;
      }
      else if ( parseBuf[parseIndex] == '#' ) {
        tokenLast = parseIndex - 1;
        tokenNext = parseIndex;
        gotToken = 1;
        readFile = 1;
        tokenState = GET_1ST_NONWS_CHAR;
      }
      else if ( isspace(parseBuf[parseIndex]) || 
	   ( parseBuf[parseIndex] == ',' ) ) {
        tokenLast = parseIndex - 1;
        tokenNext = parseIndex;
        gotToken = 1;
        tokenState = GET_1ST_NONWS_CHAR;
      }

      if ( gotToken ) {

        l = tokenLast - tokenFirst + 1;
        strncpy( parseToken, &parseBuf[tokenFirst], l );
        parseToken[l] = 0;
        tokenFirst = tokenNext;

      }

      break;

    case GET_TIL_END_OF_QUOTE:

      if ( parseBuf[parseIndex] == 0 ) {
	return FAIL;
      }
      else if ( parseBuf[parseIndex] == '"' ) {
        tokenLast = parseIndex - 1;
        tokenNext = parseIndex + 1;
        gotToken = 1;
        tokenState = GET_1ST_NONWS_CHAR;
      }

      if ( gotToken ) {

        l = tokenLast - tokenFirst + 1;
        strncpy( parseToken, &parseBuf[tokenFirst], l );
        parseToken[l] = 0;
        tokenFirst = tokenNext;

      }

      break;

    case GET_TIL_END_OF_SPECIAL:

      if ( parseBuf[parseIndex] == 0 ) {
	readFile = 1;
      }
      else if ( parseBuf[parseIndex] == '"' ) {
        tokenLast = parseIndex - 1;
        tokenNext = parseIndex + 1;
        gotToken = 1;
        tokenState = GET_TIL_END_OF_QUOTE;
      }
      else if ( parseBuf[parseIndex] == '#' ) {
        tokenLast = parseIndex - 1;
        tokenNext = parseIndex;
        gotToken = 1;
        readFile = 1;
        tokenState = GET_1ST_NONWS_CHAR;
      }
      else if ( isspace(parseBuf[parseIndex]) || 
	   ( parseBuf[parseIndex] == ',' ) ) {
        tokenLast = parseIndex - 1;
        tokenNext = parseIndex;
        gotToken = 1;
        tokenState = GET_1ST_NONWS_CHAR;
      }
      else if ( ( parseBuf[parseIndex] != '<' ) &&
                ( parseBuf[parseIndex] != '>' ) &&
                ( parseBuf[parseIndex] != '=' ) &&
                ( parseBuf[parseIndex] != '|' ) &&
                ( parseBuf[parseIndex] != '&' ) &&
                ( parseBuf[parseIndex] != '!' ) ) {
        tokenLast = parseIndex - 1;
        tokenNext = parseIndex;
        gotToken = 1;
        tokenState = GET_TIL_END_OF_TOKEN;
      }

      if ( gotToken ) {

        l = tokenLast - tokenFirst + 1;
        strncpy( parseToken, &parseBuf[tokenFirst], l );
        parseToken[l] = 0;
        tokenFirst = tokenNext;

      }

      break;

    }    

  } while ( !gotToken );

  strncpy( token, parseToken, MAX_LINE_SIZE );
  token[MAX_LINE_SIZE] = 0;
  return SUCCESS;

}

static int thisAnd (
  int variable,
  int conditionArg ) {

  return ( variable && conditionArg );

}

static int thisOr (
  int variable,
  int conditionArg ) {

  return ( variable || conditionArg );

}

static int equal (
  double variable,
  double conditionArg ) {

  return ( variable == conditionArg );

}

static int alwaysTrue (
  double variable,
  double conditionArg ) {

  return 1;

}

static int notEqual (
  double variable,
  double conditionArg ) {

  return ( variable != conditionArg );

}

static int lessThan (
  double variable,
  double conditionArg ) {

  return ( variable < conditionArg );

}

static int lessThanOrEqual (
  double variable,
  double conditionArg ) {

  return ( variable <= conditionArg );

}

static int greaterThan (
  double variable,
  double conditionArg ) {

  return ( variable > conditionArg );

}

static int greaterThanOrEqual (
  double variable,
  double conditionArg ) {

  return ( variable >= conditionArg );

}

int colorInfoClass::ver3InitFromFile (
  FILE *f,
  XtAppContext app,
  Display *d,
  Widget top,
  char *fileName )
{

char tk[MAX_LINE_SIZE+1], *endptr;
int i, ii, n, stat, nrows, ncols, remainder, dup,
 parseStatus, state, colorMult, val, index, maxSpecial=0, firstCond,
 x, y, r, c, pos;
XColor color;
Arg arg[20];
XmString str1, str2;
colorCachePtr cur1, cur2, cur[3], curSpecial;
ruleConditionPtr ruleCond=NULL;
unsigned long bgColor;
int tmpSize;
int *tmp;
char msg[127+1];

  for ( i=0; i<NUM_SPECIAL_COLORS; i++ ) {
    special[i] = 0;
    specialIndex[i] = 0;
  }

  appCtx = app;
  display = d;
  screen = DefaultScreen( d );
  depth = DefaultDepth( d, screen );
  visual = DefaultVisual( d, screen );

  if ( usePrivColorMapFlag ) {
    usingPrivateColorMap = 1;
    cmap = XCopyColormapAndFree( display, cmap );
    XSetWindowColormap( display, XtWindow(top), cmap );
  }
  else{
    cmap = DefaultColormap( d, screen );
  }

  num_color_cols = 10;
  maxColor = 0x10000;
  colorMult = 1;
  state = GET_FIRST_TOKEN;
  parseStatus = SUCCESS;
  initParseEngine( f );

  num_blinking_colors = 0;

  max_colors = 0;
  numColors = 0;
  // first, build a list of colors and rules

  while ( state != -1 ) {

    //fprintf( stderr, "[%-d]\n", state );

    switch ( state ) {

    case GET_FIRST_TOKEN:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }

      if ( strcmp( tk, "" ) == 0 ) {

        state = -1; // all done

      }
      else if ( strcmp( tk, "columns" ) == 0 ) {

        state = GET_NUM_COLUMNS;

      }
      else if ( strcmp( tk, "max" ) == 0 ) {

        state = GET_MAX;

      }
      else if ( strcmp( tk, "menumap" ) == 0 ) {

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, "{" ) != 0 ) {
          parseError( colorInfoClass_str11 );
          parseStatus = FAIL;
          goto term;
        }

        maxMenuItems = 0;
        menuMapSize = 128;
        menuIndexMap = new int[menuMapSize];
        for ( i=0; i<menuMapSize; i++ ) menuIndexMap[i] = 0;

        state = GET_MENU_MAP;

      }
      else if ( strcmp( tk, "alarm" ) == 0 ) {

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, "{" ) != 0 ) {
          parseError( colorInfoClass_str11 );
          parseStatus = FAIL;
          goto term;
        }

        maxSpecial = -1;

        state = GET_ALARM_PARAMS;

      }
      else if ( strcmp( tk, "static" ) == 0 ) {

        state = GET_COLOR;

      }
      else if ( strcmp( tk, "rule" ) == 0 ) {

        state = GET_RULE;

      }
      else if ( strcmp( tk, "alias" ) == 0 ) {

        state = GET_ALIAS;

      }
      else if ( strcmp( tk, "" ) != 0 ) {

        parseError( colorInfoClass_str12 );
        parseStatus = FAIL;
        goto term;

      }

      break;

    case GET_NUM_COLUMNS:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "=" ) != 0 ) {
        parseError( colorInfoClass_str13 );
        parseStatus = FAIL;
        goto term;
      }

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }
      num_color_cols = strtol( tk, &endptr, 0 );
      if ( strcmp( endptr, "" ) == 0 ) {
        state = GET_FIRST_TOKEN;
      }
      else {
        parseError( colorInfoClass_str14 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_MAX:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "=" ) != 0 ) {
        parseError( colorInfoClass_str13 );
        parseStatus = FAIL;
        goto term;
      }

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }
      maxColor = strtol( tk, &endptr, 0 );
      if ( strcmp( endptr, "" ) == 0 ) {
        colorMult = (int) rint( 0x10000 / maxColor );
        state = GET_FIRST_TOKEN;
      }
      else {
        parseError( colorInfoClass_str15 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_RULE:

      for( n=0; n<3; n++ ) {
        cur[n] = new colorCacheType;
        cur[n]->rule = new ruleType;
        cur[n]->pixel = 0;
        cur[n]->blinkPixel = 0;
        cur[n]->rule->ruleHead = new ruleConditionType; // sentinel
        cur[n]->rule->ruleTail = cur[n]->rule->ruleHead;
        cur[n]->rule->ruleTail->flink = NULL;
      }

      stat = getToken( tk ); // color name
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      for( n=0; n<3; n++ ) {
        cur[n]->name = new char[strlen(tk)+1];
        strcpy( cur[n]->name, tk );
        cur[n]->index = colorIndex; // this is simply an incrementing
	                           // sequence number
        cur[n]->position = colorIndex; // this is simply an incrementing
	                               // sequence number
      }

      stat = getToken( tk ); // {
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "{" ) != 0 ) {
        parseError( colorInfoClass_str11 );
        parseStatus = FAIL;
        goto term;
      }

      //fprintf( stderr, "rule is [%s]\n", cur[0]->name );

      state = GET_RULE_CONDITION;

      break;

    case GET_RULE_CONDITION:

      //fprintf( stderr, "new condition\n" );
      ruleCond = new ruleConditionType;
      state = GET_FIRST_OP_OR_ARG;
      break;

    case GET_FIRST_OP_OR_ARG:

      stat = getToken( tk ); // operator or number or "default"
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, "}" ) == 0 ) {

        //fprintf( stderr, "rule complete\n" );

        if ( ruleCond->resultName ) {
          delete[] ruleCond->resultName;
          ruleCond->resultName = NULL;
        }

        delete ruleCond;
        ruleCond = NULL;

        state = INSERT_COLOR;

        break;

      }

      if ( strcmp( tk, "default" ) == 0 ) {

        ruleCond->ruleFunc1 = alwaysTrue;
        ruleCond->value1 = 0;
        ruleCond->connectingFunc = NULL;
        ruleCond->joiningFunc = NULL;
        state = GET_COLON;

      }
      else if ( isLegalFloat( tk ) ) { // implied operator is =

        ruleCond->ruleFunc1 = equal;
        ruleCond->value1 = atof( tk );
        state = GET_CONNECTOR_OR_COLON;

      }
      else { // got an explicit operator

        if ( strcmp( tk, "=" ) == 0 ) {
          ruleCond->ruleFunc1 = equal;
	}
	else if ( strcmp( tk, "!=" ) == 0 ) {
          ruleCond->ruleFunc1 = notEqual;
	}
	else if ( strcmp( tk, ">" ) == 0 ) {
          ruleCond->ruleFunc1 = greaterThan;
	}
	else if ( strcmp( tk, ">=" ) == 0 ) {
          ruleCond->ruleFunc1 = greaterThanOrEqual;
	}
	else if ( strcmp( tk, "<" ) == 0 ) {
          ruleCond->ruleFunc1 = lessThan;
	}
	else if ( strcmp( tk, "<=" ) == 0 ) {
          ruleCond->ruleFunc1 = lessThanOrEqual;
	}
	else {
          parseError( colorInfoClass_str23 );
          parseStatus = FAIL;
          goto term;
        }

        state = GET_FIRST_ARG;

      }

      break;

    case GET_FIRST_ARG:

      stat = getToken( tk ); // number
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( isLegalFloat( tk ) ) {
        ruleCond->value1 = atof( tk );
        state = GET_CONNECTOR_OR_COLON;
      }
      else {
        parseError( colorInfoClass_str25 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_CONNECTOR_OR_COLON:

      stat = getToken( tk ); // &&, ||, or :
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, "&&" ) == 0 ) {
        ruleCond->connectingFunc = thisAnd;
        state = GET_NEXT_OP_OR_ARG;
      }
      else if ( strcmp( tk, "||" ) == 0 ) {
        ruleCond->connectingFunc = thisOr;
        state = GET_NEXT_OP_OR_ARG;
      }
      else if ( strcmp( tk, ":" ) == 0 ) {
        ruleCond->connectingFunc = NULL;
        state = GET_RESULT_NAME_OR_JOININGFUNC;
      }
      else {
        parseError( colorInfoClass_str24 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_NEXT_OP_OR_ARG:

      stat = getToken( tk ); // operator or number
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( isLegalFloat( tk ) ) { // implied operator is =

        ruleCond->ruleFunc2 = equal;
        ruleCond->value2 = atof( tk );
        state = GET_COLON;

      }
      else { // got an explicit operator

        if ( strcmp( tk, "=" ) == 0 ) {
          ruleCond->ruleFunc2 = equal;
	}
	else if ( strcmp( tk, "!=" ) == 0 ) {
          ruleCond->ruleFunc2 = notEqual;
	}
	else if ( strcmp( tk, ">" ) == 0 ) {
          ruleCond->ruleFunc2 = greaterThan;
	}
	else if ( strcmp( tk, ">=" ) == 0 ) {
          ruleCond->ruleFunc2 = greaterThanOrEqual;
	}
	else if ( strcmp( tk, "<" ) == 0 ) {
          ruleCond->ruleFunc2 = lessThan;
	}
	else if ( strcmp( tk, "<=" ) == 0 ) {
          ruleCond->ruleFunc2 = lessThanOrEqual;
	}
	else {
          parseError( colorInfoClass_str23 );
          parseStatus = FAIL;
          goto term;
        }

        state = GET_NEXT_ARG;

      }

      break;

    case GET_NEXT_ARG:

      stat = getToken( tk ); // number
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( isLegalFloat( tk ) ) {
        ruleCond->value2 = atof( tk );
        state = GET_COLON;
      }
      else {
        parseError( colorInfoClass_str25 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_COLON:

      stat = getToken( tk ); // :
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, ":" ) == 0 ) {
        state = GET_RESULT_NAME_OR_JOININGFUNC;
      }
      else {
        parseError( colorInfoClass_str19 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_RESULT_NAME_OR_JOININGFUNC:

      stat = getToken( tk ); // color name to use when this condition is true
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, "}" ) == 0 ) {
        parseError( colorInfoClass_str26 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "&&" ) == 0 ) {
        ruleCond->resultName = NULL;
        ruleCond->joiningFunc = thisAnd;
        ruleCond->result = 0; // we will map above name to an index
                              // later and store the index here
      }
      else if ( strcmp( tk, "||" ) == 0 ) {
        ruleCond->resultName = NULL;
        ruleCond->joiningFunc = thisOr;
        ruleCond->result = 0; // we will map above name to an index
                              // later and store the index here
      }
      else {
        ruleCond->joiningFunc = NULL;
        ruleCond->resultName = new char[strlen(tk)+1];
        strcpy( ruleCond->resultName, tk );
        ruleCond->result = 0; // we will map above name to an index
                              // later and store the index here
      }

      // link into condition list
      for( n=0; n<3; n++ ) {
        cur[n]->rule->ruleTail->flink = ruleCond;
        cur[n]->rule->ruleTail = ruleCond;
        cur[n]->rule->ruleTail->flink = NULL;
      }
      state = GET_RULE_CONDITION;

      break;

    case GET_COLOR:

      for( n=0; n<3; n++ ) {
        cur[n] = new colorCacheType;
        cur[n]->rule = NULL;
        cur[n]->pixel = 0;
        cur[n]->blinkPixel = 0;
      }

      stat = getToken( tk ); // color name
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      for( n=0; n<3; n++ ) {
        cur[n]->name = new char[strlen(tk)+1];
        strcpy( cur[n]->name, tk );
        cur[n]->index = colorIndex; // this is simply an incrementing
	                            // sequence number
        cur[n]->position = colorIndex; // this is simply an incrementing
	                               // sequence number
      }

      //fprintf( stderr, "[%s]\n", tk );
      if ( strcmp( tk, "invisible" ) == 0 ) {
        invisibleIndex = colorIndex;
      }

      stat = getToken( tk ); // {
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "{" ) != 0 ) {
        parseError( colorInfoClass_str11 );
        parseStatus = FAIL;
        goto term;
      }

      // get r, g, b
      for ( i=0; i<3; i++ ) {

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        for( n=0; n<3; n++ ) {
          val = strtol( tk, &endptr, 0 );
          if ( strcmp( endptr, "" ) != 0 ) {
            parseError( colorInfoClass_str16 );
            parseStatus = FAIL;
            goto term;
          }
          cur[n]->rgb[i] = val * colorMult; 
          cur[n]->blinkRgb[i] = cur[n]->rgb[i]; // init blink to the same;
	                                        // may be changed below
	}

      }

      // now we can have } or 3 more r, g, b values

      stat = getToken( tk ); // try }
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "}" ) != 0 ) {

        // cur token must be an rgb value
        for( n=0; n<3; n++ ) {
          val = strtol( tk, &endptr, 0 );
          if ( strcmp( endptr, "" ) != 0 ) {
            parseError( colorInfoClass_str16 );
            parseStatus = FAIL;
            goto term;
          }
          cur[n]->blinkRgb[0] = val * colorMult;
        }

	// now get g, b, }
        for ( i=1; i<3; i++ ) {

          stat = getToken( tk ); // R
          if ( stat == FAIL ) {
            parseError( colorInfoClass_str9 );
            parseStatus = stat;
            goto term;
          }
          if ( strcmp( tk, "" ) == 0 ) {
            parseError( colorInfoClass_str10 );
            parseStatus = FAIL;
            goto term;
          }
          for( n=0; n<3; n++ ) {
            val = strtol( tk, &endptr, 0 );
            if ( strcmp( endptr, "" ) != 0 ) {
              parseError( colorInfoClass_str16 );
              parseStatus = FAIL;
              goto term;
            }
            cur[n]->blinkRgb[i] = val * colorMult;
	  }

        }

        stat = getToken( tk ); // get }
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        else if ( strcmp( tk, "}" ) != 0 ) {
          parseError( colorInfoClass_str17 );
          parseStatus = FAIL;
          goto term;
        }

      }

      state = INSERT_COLOR;

      break;

    case GET_ALIAS:

      cur1 = new colorCacheType;

      stat = getToken( tk ); // alias name
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      cur1->name = new char[strlen(tk)+1];
      strcpy( cur1->name, tk );

      stat = getToken( tk ); // alias value
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      cur1->aliasValue = new char[strlen(tk)+1];
      strcpy( cur1->aliasValue, tk );

      stat = avl_insert_node( this->colorCacheByAliasH, (void *) cur1,
       &dup );
      if ( !( stat & 1 ) ) {
        delete cur1;
        fclose( f );
        return stat;
      }

      if ( dup ) {
        sprintf( msg, colorInfoClass_str34, cur[0]->name );
        parseError( msg );
        delete cur1;
      }

      state = GET_FIRST_TOKEN;

      break;

    case INSERT_COLOR:

      stat = avl_insert_node( this->colorCacheByNameH, (void *) cur[0],
       &dup );
      if ( !( stat & 1 ) ) {
        delete cur[0];
        fclose( f );
        return stat;
      }

      if ( dup ) {
        sprintf( msg, colorInfoClass_str7, cur[0]->name );
        parseError( msg );
        delete cur[0];
      }

      stat = avl_insert_node( this->colorCacheByIndexH, (void *) cur[1],
       &dup );
      if ( !( stat & 1 ) ) {
        delete cur[1];
        fclose( f );
        return stat;
      }

      if ( dup ) {
        parseError( colorInfoClass_str27 );
        delete cur[1];
        parseStatus = FAIL;
        goto term;
      }

      stat = avl_insert_node( this->colorCacheByPosH, (void *) cur[2],
       &dup );
      if ( !( stat & 1 ) ) {
        delete cur[2];
        fclose( f );
        return stat;
      }

      if ( dup ) {
        parseError( colorInfoClass_str27 );
        delete cur[2];
        parseStatus = FAIL;
        goto term;
      }

      colorIndex++;

      max_colors++;

      state = GET_FIRST_TOKEN;

      break;

    case GET_MENU_MAP:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, "}" ) == 0 ) {

        state = GET_FIRST_TOKEN;
 
      }
      else {

        stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
         (void **) &cur1 );
        if ( !( stat & 1 ) ) {
          parseError( colorInfoClass_str18 );
          parseStatus = FAIL;
          goto term;
        }
        if ( !cur1 ) {
          parseError( colorInfoClass_str18 );
          parseStatus = FAIL;
          goto term;
        }

        if ( ( maxMenuItems + 1 ) >= menuMapSize ) {
          tmpSize = menuMapSize + 128;
          tmp = new int[tmpSize];
          for ( i=0; i<menuMapSize; i++ ) {
            tmp[i] = menuIndexMap[i];
          }
          for ( i=menuMapSize; i<tmpSize; i++ ) {
            tmp[i] = 0;
	  }
          delete[] menuIndexMap;
          menuIndexMap = tmp;
          menuMapSize = tmpSize;
        }

        menuIndexMap[maxMenuItems++] = cur1->index;

      }

      break;

    case GET_ALARM_PARAMS:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, "disconnected" ) == 0 ) {

        index = COLORINFO_K_DISCONNECTED;

        if ( index > maxSpecial ) maxSpecial = index;

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, ":" ) != 0 ) {
          parseError( colorInfoClass_str19 );
          parseStatus = FAIL;
          goto term;
        }

        // get color name
        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }

        stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
         (void **) &curSpecial );
        if ( !( stat & 1 ) ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        if ( !curSpecial ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        specialIndex[index] = curSpecial->index;

      }
      else if ( strcmp( tk, "invalid" ) == 0 ) {

        index = COLORINFO_K_INVALID;

        if ( index > maxSpecial ) maxSpecial = index;

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, ":" ) != 0 ) {
          parseError( colorInfoClass_str19 );
          parseStatus = FAIL;
          goto term;
        }

        // get color name
        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }

        stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
         (void **) &curSpecial );
        if ( !( stat & 1 ) ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        if ( !curSpecial ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        specialIndex[index] = curSpecial->index;

      }
      else if ( strcmp( tk, "minor" ) == 0 ) {

        index = COLORINFO_K_MINOR;

        if ( index > maxSpecial ) maxSpecial = index;

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, ":" ) != 0 ) {
          parseError( colorInfoClass_str19 );
          parseStatus = FAIL;
          goto term;
        }

        // get color name
        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }

        stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
         (void **) &curSpecial );
        if ( !( stat & 1 ) ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        if ( !curSpecial ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        specialIndex[index] = curSpecial->index;

      }
      else if ( strcmp( tk, "major" ) == 0 ) {

        index = COLORINFO_K_MAJOR;

        if ( index > maxSpecial ) maxSpecial = index;

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, ":" ) != 0 ) {
          parseError( colorInfoClass_str19 );
          parseStatus = FAIL;
          goto term;
        }

        // get color name
        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }

        stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
         (void **) &curSpecial );
        if ( !( stat & 1 ) ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        if ( !curSpecial ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        specialIndex[index] = curSpecial->index;

      }
      else if ( strcmp( tk, "noalarm" ) == 0 ) {

        index = COLORINFO_K_NOALARM;

        if ( index > maxSpecial ) maxSpecial = index;

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, ":" ) != 0 ) {
          parseError( colorInfoClass_str19 );
          parseStatus = FAIL;
          goto term;
        }

        // get color name
        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }

        if ( strcmp( tk, "*" ) == 0 ) {

          specialIndex[index] = -1;
	  showNoAlarmState = 0;
          
	}
	else {

          stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
           (void **) &curSpecial );
          if ( !( stat & 1 ) ) {
            parseError( colorInfoClass_str20 );
            parseStatus = FAIL;
            goto term;
          }
          if ( !curSpecial ) {
            parseError( colorInfoClass_str20 );
            parseStatus = FAIL;
            goto term;
          }
          specialIndex[index] = curSpecial->index;

	}

      }
      else if ( strcmp( tk, "}" ) == 0 ) {

        if ( maxSpecial < 4 ) {
          parseError( colorInfoClass_str21 );
          parseStatus = FAIL;
          goto term;
        }
 
        state = GET_FIRST_TOKEN;
 
      }
      else {

        parseError( colorInfoClass_str22 );
        parseStatus = FAIL;
        goto term;

      }

      break;

    }

  }

term:

  fclose( f );

  if ( parseStatus != SUCCESS ) {
    return 0;
  }

  change = 1;
  blink = 0;
  curIndex = 0;
  curX = 5;
  curY = 5;

  colors = new unsigned int[max_colors];
  blinkingColors = new unsigned int[max_colors];
  simpleColorButtons = new simpleButtonType[max_colors];
  colorNames = new char *[max_colors];
  colorNodes = new colorCachePtr[max_colors];

  stat = avl_get_first( this->colorCacheByIndexH, (void **) &cur1 );
  if ( !( stat & 1 ) ) {
    return 0;
  }

  i = 0;

  while ( cur1 ) {

    colorNodes[i] = cur1;

    // Allocate X color for static rules only
    if ( !cur1->rule ) { // not a dynamic color rule

      colorNames[i] = cur1->name; // populate color name array

      color.red = cur1->rgb[0];
      color.green = cur1->rgb[1];
      color.blue = cur1->rgb[2];

      stat = XAllocColor( display, cmap, &color );
      if ( stat ) {
        colors[i] = color.pixel;
      }
      else {
        colors[i] = BlackPixel( display, screen );
      }
      cur1->pixel = colors[i];

      if ( ( cur1->rgb[0] != cur1->blinkRgb[0] ) ||
           ( cur1->rgb[1] != cur1->blinkRgb[1] ) ||
           ( cur1->rgb[2] != cur1->blinkRgb[2] ) ) {

        color.red = cur1->blinkRgb[0];
        color.green = cur1->blinkRgb[1];
        color.blue = cur1->blinkRgb[2];

        stat = XAllocColor( display, cmap, &color );
        if ( stat ) {
          blinkingColors[i] = color.pixel;
        }
        else {
          blinkingColors[i] = BlackPixel( display, screen );
        }
        cur1->blinkPixel = blinkingColors[i];

      }
      else {
        blinkingColors[i] = colors[i];
        cur1->blinkPixel = blinkingColors[i];
      }

      // --------------------------------------------------------------
      // update tree sorted by name
      stat = avl_get_match( this->colorCacheByNameH, (void *) cur1->name,
       (void **) &cur2 );
      if ( cur2 ) {
        cur2->pixel = cur1->pixel;
        cur2->blinkPixel = cur1->blinkPixel;
        for ( ii=0; ii<3; ii++ ) {
          cur2->rgb[ii] = cur1->rgb[ii];
          cur2->blinkRgb[ii] = cur1->blinkRgb[ii];
        }
      }
      // --------------------------------------------------------------

    }

    stat = avl_get_next( this->colorCacheByIndexH, (void **) &cur1 );
    if ( !( stat & 1 ) ) {
      return 0;
    }

    if ( i < max_colors-1 ) i++;

  }

  //fprintf( stderr, "fixup dynamic rules\n" );

  // map dynamic color rule result name into result index
  for ( i=0; i<max_colors; i++ ) {

    //fprintf( stderr, "i = %-d\n", i );

    if ( colorNodes[i]->rule ) { // dynamic color rules only

      //fprintf( stderr, "rule is %s, index=%-d\n", colorNodes[i]->name,
      // colorNodes[i]->index );

      colorNames[i] = colorNodes[i]->name; // populate color name array

      firstCond = 1;
      ruleCond = colorNodes[i]->rule->ruleHead->flink;

      if ( !ruleCond ) {
        fprintf( stderr, colorInfoClass_str30, colorNodes[i]->name );
        return 0;
      }

      while ( ruleCond ) {

        if ( ruleCond->resultName ) { // this condition might be combined
                                      // with others via ||, &&

          stat = avl_get_match( this->colorCacheByNameH,
           (void *) ruleCond->resultName, (void **) &cur1 );
          if ( cur1 ) {
            if ( isRule( cur1->index ) ) { // rules may not reference
	                                   // other rules
              fprintf( stderr, colorInfoClass_str29, colorNodes[i]->name );
              return 0;
            }
            ruleCond->result = cur1->index;
            //fprintf( stderr, "result name %s --> %-d\n", ruleCond->resultName,
            // ruleCond->result );
          }
          else {
            fprintf( stderr, colorInfoClass_str28, colorNodes[i]->name );
            return 0;
          }

          if ( firstCond ) { // use first rule condition as static
	                     // color for rule

            colors[i] = cur1->pixel;
            blinkingColors[i] = cur1->blinkPixel;

            firstCond = 0;
            colorNodes[i]->pixel = cur1->pixel;
            colorNodes[i]->blinkPixel = cur1->blinkPixel;
            for ( ii=0; ii<3; ii++ ) {
              colorNodes[i]->rgb[ii] = cur1->rgb[ii];
              colorNodes[i]->blinkRgb[ii] = cur1->blinkRgb[ii];
            }

            // --------------------------------------------------------------
            // update tree sorted by name
            stat = avl_get_match( this->colorCacheByNameH,
             (void *) colorNodes[i]->name,
             (void **) &cur2 );
            if ( cur2 ) {
              cur2->pixel = colorNodes[i]->pixel;
              cur2->blinkPixel = colorNodes[i]->blinkPixel;
              for ( ii=0; ii<3; ii++ ) {
                cur2->rgb[ii] = colorNodes[i]->rgb[ii];
                cur2->blinkRgb[ii] = colorNodes[i]->blinkRgb[ii];
              }
            }
            // --------------------------------------------------------------

	  }

	}

        ruleCond = ruleCond->flink;

      }

    }

  }

#if 0
  {

    double v;

    fprintf( stderr, "eval rules\n\n\n" );

    i = 0;

    v = 0;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = 1;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = .5;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = 1000;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = 5.5;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = 99.1;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = -1;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

  }
#endif

#if 0

  stat = avl_get_first( this->colorCacheByIndexH, (void **) &cur1 );
  if ( !( stat & 1 ) ) {
    return 0;
  }

  while ( cur1 ) {

    fprintf( stderr, "name: %s, index: %-d, r: %-d, g: %-d, b: %-d, p: %-d, bp: %-d\n",
     cur1->name, cur1->index, cur1->rgb[0], cur1->rgb[1], cur1->rgb[2],
     cur1->pixel, cur1->blinkPixel );

    stat = avl_get_next( this->colorCacheByIndexH, (void **) &cur1 );
    if ( !( stat & 1 ) ) {
      fprintf( stderr, "error 1\n" );
      return 0;
    }

  }

  fprintf( stderr, "\n" );

  stat = avl_get_first( this->colorCacheByNameH, (void **) &cur1 );
  if ( !( stat & 1 ) ) {
    fprintf( stderr, "error 1\n" );
    return 0;
  }

  while ( cur1 ) {

    fprintf( stderr, "name: %s, index: %-d, r: %-d, g: %-d, b: %-d, p: %-d, bp: %-d\n",
     cur1->name, cur1->index, cur1->rgb[0], cur1->rgb[1], cur1->rgb[2],
     cur1->pixel, cur1->blinkPixel );

    stat = avl_get_next( this->colorCacheByNameH, (void **) &cur1 );
    if ( !( stat & 1 ) ) {
      fprintf( stderr, "error 1\n" );
      return 0;
    }

  }

#endif

  // create window

  //shell = XtVaAppCreateShell( colorInfoClass_str2, colorInfoClass_str2,
  //shell = XtVaAppCreateShell( "edm", "edm",
  // topLevelShellWidgetClass,
  // XtDisplay(top),
  // XtNmappedWhenManaged, False,
  // NULL );

  shell = XtVaCreatePopupShell( "edm", topLevelShellWidgetClass,
   top,
   XtNmappedWhenManaged, False,
   NULL );

  rc = XtVaCreateWidget( "colorpallete", xmRowColumnWidgetClass, shell,
   XmNorientation, XmVERTICAL,
   XmNnumColumns, 1,
   NULL );

  str1 = XmStringCreateLocalized( colorInfoClass_str3 );
  mbar = XmVaCreateSimpleMenuBar( rc, "menubar",
   XmVaCASCADEBUTTON, str1, 'f',
   NULL );
  XmStringFree( str1 );

  str1 = XmStringCreateLocalized( colorInfoClass_str4 );
  str2 = XmStringCreateLocalized( colorInfoClass_str5 );
  mb1 = XmVaCreateSimplePulldownMenu( mbar, "pb", 0, file_cb,
   XmVaPUSHBUTTON, str1, 'x', NULL, NULL,
   XmVaPUSHBUTTON, str2, 's', NULL, NULL,
   NULL );
  XmStringFree( str1 );
  XmStringFree( str2 );

  ncols = num_color_cols;
  nrows = (max_colors) / ncols;
  remainder = (max_colors) % ncols;
  if ( remainder ) nrows++;

  form = XtVaCreateWidget( "form", xmDrawingAreaWidgetClass, rc,
   XmNwidth, ncols*20 + ncols*5 + 5,
   XmNheight, nrows*20 + nrows*5 + 5,
   XmNresizePolicy, XmRESIZE_NONE,
   NULL );

  XtAddEventHandler( form,
   PointerMotionMask|ButtonPressMask|ExposureMask|
   LeaveWindowMask|StructureNotifyMask, False,
   colorFormEventHandler, (XtPointer) this );

  XtAddEventHandler( shell,
   StructureNotifyMask, False,
   colorShellEventHandler, (XtPointer) this );

  Atom wm_delete_window = XmInternAtom( XtDisplay(shell), "WM_DELETE_WINDOW",
   False );

  XmAddWMProtocolCallback( shell, wm_delete_window, file_cb,
    (XtPointer) 0 );

  XtVaSetValues( shell, XmNdeleteResponse, XmDO_NOTHING, NULL );

  XtManageChild( mbar );
  XtManageChild( rc );
  XtManageChild( form );
  XtRealizeWidget( shell );
  XSetWindowColormap( display, XtWindow(shell), cmap );

  gc.create( shell );
  gc.setCI( this );

  n = 0;
  XtSetArg( arg[n], XmNbackground, &bgColor ); n++;
  XtGetValues( form, arg, n );

  gc.setBG( bgColor );
  gc.setBaseBG( bgColor );

  numColors = max_colors;

  // populate special array
  for ( i=0; i<NUM_SPECIAL_COLORS-2; i++ ) {
    if ( specialIndex[i] != -1 ) {
      special[i] = colors[ specialIndex[i] ];
    }
    else {
      special[i] = -1;
    }
  }

  msgDialog.create( shell );

  incrementTimerValue = 1000;
  incrementTimerActive = 1;
  incrementTimer = appAddTimeOut( appCtx, incrementTimerValue,
   toggleColorBlink, this );

  ncols = num_color_cols;
  nrows = (max_colors+num_blinking_colors) / ncols;
  remainder = (max_colors+num_blinking_colors) % ncols;

  pos = 0;
  for ( r=0; r<nrows; r++ ) {

    for ( c=0; c<ncols; c++ ) {

      x = c*5 + c*20 + 5;
      y = r*5 + r*20 + 5;

      stat = avl_get_match( this->colorCacheByPosH,
       (void *) &pos, (void **) &cur1 );
      if ( stat & 1 ) {
        if ( cur1 ) {
          simpleColorButtons[pos].colorIndex = cur1->index;
        }
        else {
          simpleColorButtons[pos].colorIndex = 0;
        }
      }
      else {
        simpleColorButtons[pos].colorIndex = 0;
      }

      simpleColorButtons[pos].wgt = form;
      simpleColorButtons[pos].cio = this;
      simpleColorButtons[pos].x = x;
      simpleColorButtons[pos].y = y;
      simpleColorButtons[pos].w = 23;
      simpleColorButtons[pos].h = 23;
      simpleColorButtons[pos].blink = 0;

      pos++;

    }

  }

  if ( remainder ) {

    r = nrows;

    for ( c=0; c<remainder; c++ ) {

      x = c*5 + c*20 + 5;
      y = r*5 + r*20 + 5;

      stat = avl_get_match( this->colorCacheByPosH,
       (void *) &pos, (void **) &cur1 );
      if ( stat & 1 ) {
        if ( cur1 ) {
          simpleColorButtons[pos].colorIndex = cur1->index;
        }
        else {
          simpleColorButtons[pos].colorIndex = 0;
        }
      }
      else {
        simpleColorButtons[pos].colorIndex = 0;
      }

      simpleColorButtons[pos].wgt = form;
      simpleColorButtons[pos].cio = this;
      simpleColorButtons[pos].x = x;
      simpleColorButtons[pos].y = y;
      simpleColorButtons[pos].w = 23;
      simpleColorButtons[pos].h = 23;
      simpleColorButtons[pos].blink = 0;

      pos++;

    }

  }

  return 1;

}

int colorInfoClass::ver4InitFromFile (
  FILE *f,
  XtAppContext app,
  Display *d,
  Widget top,
  char *fileName )
{

#define curMax 4

char tk[MAX_LINE_SIZE+1], *endptr;
int i, ii, n, stat, nrows, ncols, remainder, dup,
 parseStatus, state, colorMult, val, index, maxSpecial=0, firstCond,
 x, y, r, c, pos;

XColor color;
Arg arg[20];
XmString str1, str2;
colorCachePtr cur1, cur2, cur3, cur4, cur[curMax], curSpecial;
ruleConditionPtr ruleCond=NULL, ruleCondTmp, curRule, nextRule;
unsigned long bgColor;
int tmpSize;
int *tmp;
char msg[127+1];
int blinkMs = 500;

  for ( i=0; i<NUM_SPECIAL_COLORS; i++ ) {
    special[i] = 0;
    specialIndex[i] = 0;
  }

  appCtx = app;
  display = d;
  screen = DefaultScreen( d );
  depth = DefaultDepth( d, screen );
  visual = DefaultVisual( d, screen );

  if ( usePrivColorMapFlag ) {
    usingPrivateColorMap = 1;
    cmap = XCopyColormapAndFree( display, cmap );
    XSetWindowColormap( display, XtWindow(top), cmap );
  }
  else{
    cmap = DefaultColormap( d, screen );
  }

  num_color_cols = 10;
  maxColor = 0x10000;
  colorMult = 1;
  state = GET_FIRST_TOKEN;
  parseStatus = SUCCESS;
  initParseEngine( f );

  num_blinking_colors = 0;

  max_colors = 0;
  numColors = 0;
  // first, build a list of colors and rules

  while ( state != -1 ) {

    //fprintf( stderr, "[%-d]\n", state );

    switch ( state ) {

    case GET_FIRST_TOKEN:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }

      if ( strcmp( tk, "" ) == 0 ) {

        state = -1; // all done

      }
      else if ( strcmp( tk, "columns" ) == 0 ) {

        state = GET_NUM_COLUMNS;

      }
      else if ( strcmp( tk, "max" ) == 0 ) {

        state = GET_MAX;

      }
      else if ( strcmp( tk, "blinkms" ) == 0 ) {

        state = GET_BLINK_PERIOD;

      }
      else if ( strcmp( tk, "menumap" ) == 0 ) {

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, "{" ) != 0 ) {
          parseError( colorInfoClass_str11 );
          parseStatus = FAIL;
          goto term;
        }

        maxMenuItems = 0;
        menuMapSize = 128;
        menuIndexMap = new int[menuMapSize];
        for ( i=0; i<menuMapSize; i++ ) menuIndexMap[i] = 0;

        state = GET_MENU_MAP;

      }
      else if ( strcmp( tk, "alarm" ) == 0 ) {

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, "{" ) != 0 ) {
          parseError( colorInfoClass_str11 );
          parseStatus = FAIL;
          goto term;
        }

        maxSpecial = -1;

        state = GET_ALARM_PARAMS;

      }
      else if ( strcmp( tk, "static" ) == 0 ) {

        state = GET_COLOR;

      }
      else if ( strcmp( tk, "rule" ) == 0 ) {

        state = GET_RULE;

      }
      else if ( strcmp( tk, "alias" ) == 0 ) {

        state = GET_ALIAS;

      }
      else if ( strcmp( tk, "" ) != 0 ) {

        parseError( colorInfoClass_str12 );
        parseStatus = FAIL;
        goto term;

      }

      break;

    case GET_NUM_COLUMNS:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "=" ) != 0 ) {
        parseError( colorInfoClass_str13 );
        parseStatus = FAIL;
        goto term;
      }

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }
      num_color_cols = strtol( tk, &endptr, 0 );
      if ( strcmp( endptr, "" ) == 0 ) {
        state = GET_FIRST_TOKEN;
      }
      else {
        parseError( colorInfoClass_str14 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_MAX:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "=" ) != 0 ) {
        parseError( colorInfoClass_str13 );
        parseStatus = FAIL;
        goto term;
      }

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }
      maxColor = strtol( tk, &endptr, 0 );
      if ( strcmp( endptr, "" ) == 0 ) {
        colorMult = (int) rint( 0x10000 / maxColor );
        state = GET_FIRST_TOKEN;
      }
      else {
        parseError( colorInfoClass_str15 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_BLINK_PERIOD:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "=" ) != 0 ) {
        parseError( colorInfoClass_str13 );
        parseStatus = FAIL;
        goto term;
      }

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
	goto term;
      }
      blinkMs = strtol( tk, &endptr, 0 );
      if ( strcmp( endptr, "" ) == 0 ) {
        state = GET_FIRST_TOKEN;
      }
      else {
        parseError( colorInfoClass_str14 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_RULE:

      for( n=0; n<curMax; n++ ) {
        cur[n] = new colorCacheType;
        colorCacheInit( cur[n] );
        cur[n]->rule = new ruleType;
        cur[n]->pixel = 0;
        cur[n]->blinkPixel = 0;
        cur[n]->rule->ruleHead = new ruleConditionType; // sentinel
        cur[n]->rule->ruleHead->resultName = NULL;
        cur[n]->rule->ruleTail = cur[n]->rule->ruleHead;
        cur[n]->rule->ruleTail->flink = NULL;
        cur[n]->rgb[0] = 0;
        cur[n]->rgb[1] = 0;
        cur[n]->rgb[2] = 0;
      }

      stat = getToken( tk ); // color index
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      colorIndex = strtol( tk, &endptr, 0 );
      if ( strcmp( endptr, "" ) != 0 ) {
        parseError( colorInfoClass_str31 );
        parseStatus = FAIL;
        goto term;
      }

      stat = getToken( tk ); // color name
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      for( n=0; n<curMax; n++ ) {
        cur[n]->name = new char[strlen(tk)+1];
        strcpy( cur[n]->name, tk );
        cur[n]->index = colorIndex;
        cur[n]->position = colorPosition; // this is simply an incrementing
                                          // sequence number
      }

      stat = getToken( tk ); // {
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "{" ) != 0 ) {
        parseError( colorInfoClass_str11 );
        parseStatus = FAIL;
        goto term;
      }

      //fprintf( stderr, "rule is [%s]\n", cur[0]->name );

      state = GET_RULE_CONDITION;

      break;

    case GET_RULE_CONDITION:

      //fprintf( stderr, "new condition\n" );
      ruleCond = new ruleConditionType;
      ruleCond->resultName = NULL;
      state = GET_FIRST_OP_OR_ARG;
      break;

    case GET_FIRST_OP_OR_ARG:

      stat = getToken( tk ); // operator or number or "default"
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, "}" ) == 0 ) {

        //fprintf( stderr, "rule complete\n" );

        if ( ruleCond->resultName ) {
          delete[] ruleCond->resultName;
          ruleCond->resultName = NULL;
        }

        delete ruleCond;
        ruleCond = NULL;

        state = INSERT_COLOR;

        break;

      }

      if ( strcmp( tk, "default" ) == 0 ) {

        ruleCond->ruleFunc1 = alwaysTrue;
        ruleCond->value1 = 0;
        ruleCond->connectingFunc = NULL;
        ruleCond->joiningFunc = NULL;
        state = GET_COLON;

      }
      else if ( isLegalFloat( tk ) ) { // implied operator is =

        ruleCond->ruleFunc1 = equal;
        ruleCond->value1 = atof( tk );
        state = GET_CONNECTOR_OR_COLON;

      }
      else { // got an explicit operator

        if ( strcmp( tk, "=" ) == 0 ) {
          ruleCond->ruleFunc1 = equal;
	}
	else if ( strcmp( tk, "!=" ) == 0 ) {
          ruleCond->ruleFunc1 = notEqual;
	}
	else if ( strcmp( tk, ">" ) == 0 ) {
          ruleCond->ruleFunc1 = greaterThan;
	}
	else if ( strcmp( tk, ">=" ) == 0 ) {
          ruleCond->ruleFunc1 = greaterThanOrEqual;
	}
	else if ( strcmp( tk, "<" ) == 0 ) {
          ruleCond->ruleFunc1 = lessThan;
	}
	else if ( strcmp( tk, "<=" ) == 0 ) {
          ruleCond->ruleFunc1 = lessThanOrEqual;
	}
	else {
          parseError( colorInfoClass_str23 );
          parseStatus = FAIL;
          goto term;
        }

        state = GET_FIRST_ARG;

      }

      break;

    case GET_FIRST_ARG:

      stat = getToken( tk ); // number
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( isLegalFloat( tk ) ) {
        ruleCond->value1 = atof( tk );
        state = GET_CONNECTOR_OR_COLON;
      }
      else {
        parseError( colorInfoClass_str25 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_CONNECTOR_OR_COLON:

      stat = getToken( tk ); // &&, ||, or :
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, "&&" ) == 0 ) {
        ruleCond->connectingFunc = thisAnd;
        state = GET_NEXT_OP_OR_ARG;
      }
      else if ( strcmp( tk, "||" ) == 0 ) {
        ruleCond->connectingFunc = thisOr;
        state = GET_NEXT_OP_OR_ARG;
      }
      else if ( strcmp( tk, ":" ) == 0 ) {
        ruleCond->connectingFunc = NULL;
        state = GET_RESULT_NAME_OR_JOININGFUNC;
      }
      else {
        parseError( colorInfoClass_str24 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_NEXT_OP_OR_ARG:

      stat = getToken( tk ); // operator or number
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( isLegalFloat( tk ) ) { // implied operator is =

        ruleCond->ruleFunc2 = equal;
        ruleCond->value2 = atof( tk );
        state = GET_COLON;

      }
      else { // got an explicit operator

        if ( strcmp( tk, "=" ) == 0 ) {
          ruleCond->ruleFunc2 = equal;
	}
	else if ( strcmp( tk, "!=" ) == 0 ) {
          ruleCond->ruleFunc2 = notEqual;
	}
	else if ( strcmp( tk, ">" ) == 0 ) {
          ruleCond->ruleFunc2 = greaterThan;
	}
	else if ( strcmp( tk, ">=" ) == 0 ) {
          ruleCond->ruleFunc2 = greaterThanOrEqual;
	}
	else if ( strcmp( tk, "<" ) == 0 ) {
          ruleCond->ruleFunc2 = lessThan;
	}
	else if ( strcmp( tk, "<=" ) == 0 ) {
          ruleCond->ruleFunc2 = lessThanOrEqual;
	}
	else {
          parseError( colorInfoClass_str23 );
          parseStatus = FAIL;
          goto term;
        }

        state = GET_NEXT_ARG;

      }

      break;

    case GET_NEXT_ARG:

      stat = getToken( tk ); // number
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( isLegalFloat( tk ) ) {
        ruleCond->value2 = atof( tk );
        state = GET_COLON;
      }
      else {
        parseError( colorInfoClass_str25 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_COLON:

      stat = getToken( tk ); // :
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, ":" ) == 0 ) {
        state = GET_RESULT_NAME_OR_JOININGFUNC;
      }
      else {
        parseError( colorInfoClass_str19 );
        parseStatus = FAIL;
        goto term;
      }

      break;

    case GET_RESULT_NAME_OR_JOININGFUNC:

      stat = getToken( tk ); // color name to use when this condition is true
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, "}" ) == 0 ) {
        parseError( colorInfoClass_str26 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "&&" ) == 0 ) {
        ruleCond->resultName = NULL;
        ruleCond->joiningFunc = thisAnd;
        ruleCond->result = 0; // we will map above name to an index
                              // later and store the index here
      }
      else if ( strcmp( tk, "||" ) == 0 ) {
        ruleCond->resultName = NULL;
        ruleCond->joiningFunc = thisOr;
        ruleCond->result = 0; // we will map above name to an index
                              // later and store the index here
      }
      else {
        ruleCond->joiningFunc = NULL;
        ruleCond->resultName = new char[strlen(tk)+1];
        strcpy( ruleCond->resultName, tk );
        ruleCond->result = 0; // we will map above name to an index
                              // later and store the index here
      }

      // link into condition list
      cur[0]->rule->ruleTail->flink = ruleCond;
      cur[0]->rule->ruleTail = ruleCond;
      cur[0]->rule->ruleTail->flink = NULL;
      for( n=1; n<curMax; n++ ) {
        ruleCondTmp = new ruleConditionType;
        copyRuleCondition( ruleCondTmp, ruleCond );
        cur[n]->rule->ruleTail->flink = ruleCondTmp;
        cur[n]->rule->ruleTail = ruleCondTmp;
        cur[n]->rule->ruleTail->flink = NULL;
      }
      state = GET_RULE_CONDITION;

      break;

    case GET_COLOR:

      for( n=0; n<curMax; n++ ) {
        cur[n] = new colorCacheType;
        colorCacheInit( cur[n] );
        cur[n]->rule = NULL;
        cur[n]->pixel = 0;
        cur[n]->blinkPixel = 0;
      }

      stat = getToken( tk ); // color index
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      colorIndex = strtol( tk, &endptr, 0 );
      if ( strcmp( endptr, "" ) != 0 ) {
        parseError( colorInfoClass_str31 );
        parseStatus = FAIL;
        goto term;
      }

      stat = getToken( tk ); // color name
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      for( n=0; n<curMax; n++ ) {
        cur[n]->name = new char[strlen(tk)+1];
        strcpy( cur[n]->name, tk );
        cur[n]->index = colorIndex;
        cur[n]->position = colorPosition; // this is simply an incrementing
                                          // sequence number
      }

      //fprintf( stderr, "[%s]\n", tk );
      if ( strcmp( tk, "invisible" ) == 0 ) {
        invisibleIndex = colorIndex;
      }

      stat = getToken( tk ); // {
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "{" ) != 0 ) {
        parseError( colorInfoClass_str11 );
        parseStatus = FAIL;
        goto term;
      }

      // get r, g, b
      for ( i=0; i<3; i++ ) {

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        for( n=0; n<curMax; n++ ) {
          val = strtol( tk, &endptr, 0 );
          if ( strcmp( endptr, "" ) != 0 ) {
            parseError( colorInfoClass_str16 );
            parseStatus = FAIL;
            goto term;
          }
          cur[n]->rgb[i] = val * colorMult; 
          cur[n]->blinkRgb[i] = cur[n]->rgb[i]; // init blink to the same;
	                                        // may be changed below
	}

      }

      // now we can have } or 3 more r, g, b values

      stat = getToken( tk ); // try }
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }
      else if ( strcmp( tk, "}" ) != 0 ) {

        // cur token must be an rgb value
        for( n=0; n<curMax; n++ ) {
          val = strtol( tk, &endptr, 0 );
          if ( strcmp( endptr, "" ) != 0 ) {
            parseError( colorInfoClass_str16 );
            parseStatus = FAIL;
            goto term;
          }
          cur[n]->blinkRgb[0] = val * colorMult;
        }

	// now get g, b, }
        for ( i=1; i<3; i++ ) {

          stat = getToken( tk ); // R
          if ( stat == FAIL ) {
            parseError( colorInfoClass_str9 );
            parseStatus = stat;
            goto term;
          }
          if ( strcmp( tk, "" ) == 0 ) {
            parseError( colorInfoClass_str10 );
            parseStatus = FAIL;
            goto term;
          }
          for( n=0; n<curMax; n++ ) {
            val = strtol( tk, &endptr, 0 );
            if ( strcmp( endptr, "" ) != 0 ) {
              parseError( colorInfoClass_str16 );
              parseStatus = FAIL;
              goto term;
            }
            cur[n]->blinkRgb[i] = val * colorMult;
	  }

        }

        stat = getToken( tk ); // get }
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        else if ( strcmp( tk, "}" ) != 0 ) {
          parseError( colorInfoClass_str17 );
          parseStatus = FAIL;
          goto term;
        }

      }

      state = INSERT_COLOR;

      break;

    case GET_ALIAS:

      cur1 = new colorCacheType;
      colorCacheInit( cur1 );

      stat = getToken( tk ); // alias name
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      cur1->name = new char[strlen(tk)+1];
      strcpy( cur1->name, tk );

      stat = getToken( tk ); // alias value
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      cur1->aliasValue = new char[strlen(tk)+1];
      strcpy( cur1->aliasValue, tk );

      stat = avl_insert_node( this->colorCacheByAliasH, (void *) cur1,
       &dup );
      if ( !( stat & 1 ) ) {
        delete cur1;
        fclose( f );
        return stat;
      }

      if ( dup ) {
        sprintf( msg, colorInfoClass_str34, cur[0]->name );
        parseError( msg );
        delete cur1;
      }

      state = GET_FIRST_TOKEN;

      break;

    case INSERT_COLOR:

      stat = avl_insert_node( this->colorCacheByNameH, (void *) cur[0],
       &dup );
      if ( !( stat & 1 ) ) {
        delete cur[0];
        fclose( f );
        return stat;
      }

      if ( dup ) {
        sprintf( msg, colorInfoClass_str7, cur[0]->name );
        parseError( msg );
        delete cur[0];
      }

      stat = avl_insert_node( this->colorCacheByIndexH, (void *) cur[1],
       &dup );
      if ( !( stat & 1 ) ) {
        delete cur[1];
        fclose( f );
        return stat;
      }

      if ( dup ) {
        parseError( colorInfoClass_str27 );
        delete cur[1];
        parseStatus = FAIL;
        goto term;
      }

      stat = avl_insert_node( this->colorCacheByPosH, (void *) cur[2],
       &dup );
      if ( !( stat & 1 ) ) {
        delete cur[2];
        fclose( f );
        return stat;
      }

      if ( dup ) {
        parseError( colorInfoClass_str27 );
        delete cur[2];
        parseStatus = FAIL;
        goto term;
      }

      //===============================

      stat = avl_insert_node( this->colorCacheByColorH, (void *) cur[3],
       &dup );
      if ( !( stat & 1 ) ) {
        delete cur[3];
        fclose( f );
        return stat;
      }

      if ( dup ) { // in this case, some duplicates are expected

        if ( cur[3]->name ) {
          delete[] cur[3]->name;
          cur[3]->name = NULL;
        }

        if ( cur[3]->aliasValue ) {
          delete[] cur[3]->aliasValue;
          cur[3]->aliasValue = NULL;
        }

        if ( cur[3]->rule ) {

          curRule = cur[3]->rule->ruleHead->flink;
          while ( curRule ) {

            nextRule = curRule->flink;

            if ( curRule->resultName ) {
              delete[] curRule->resultName;
              curRule->resultName = NULL;
            }

            delete curRule;

            curRule = nextRule;

          }
          delete cur[3]->rule->ruleHead;
          cur[3]->rule->ruleHead = NULL;

          delete cur[3]->rule;
          cur[3]->rule = NULL;

        }

        delete cur[3];

      }

      //===============================

      if ( colorIndex+1 > max_colors ) max_colors = colorIndex+1;

      colorPosition += 1;
      if ( colorPosition > max_colors ) max_colors = colorPosition;

      state = GET_FIRST_TOKEN;

      break;

    case GET_MENU_MAP:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, "}" ) == 0 ) {

        state = GET_FIRST_TOKEN;
 
      }
      else {

        stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
         (void **) &cur1 );
        if ( !( stat & 1 ) ) {
          parseError( colorInfoClass_str18 );
          parseStatus = FAIL;
          goto term;
        }
        if ( !cur1 ) {
          parseError( colorInfoClass_str18 );
          parseStatus = FAIL;
          goto term;
        }

        if ( ( maxMenuItems + 1 ) >= menuMapSize ) {
          tmpSize = menuMapSize + 128;
          tmp = new int[tmpSize];
          for ( i=0; i<menuMapSize; i++ ) {
            tmp[i] = menuIndexMap[i];
          }
          for ( i=menuMapSize; i<tmpSize; i++ ) {
            tmp[i] = 0;
	  }
          delete[] menuIndexMap;
          menuIndexMap = tmp;
          menuMapSize = tmpSize;
        }

        menuIndexMap[maxMenuItems++] = cur1->index;

      }

      break;

    case GET_ALARM_PARAMS:

      stat = getToken( tk );
      if ( stat == FAIL ) {
        parseError( colorInfoClass_str9 );
        parseStatus = stat;
        goto term;
      }
      if ( strcmp( tk, "" ) == 0 ) {
        parseError( colorInfoClass_str10 );
        parseStatus = FAIL;
        goto term;
      }

      if ( strcmp( tk, "disconnected" ) == 0 ) {

        index = COLORINFO_K_DISCONNECTED;

        if ( index > maxSpecial ) maxSpecial = index;

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, ":" ) != 0 ) {
          parseError( colorInfoClass_str19 );
          parseStatus = FAIL;
          goto term;
        }

        // get color name
        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }

        stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
         (void **) &curSpecial );
        if ( !( stat & 1 ) ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        if ( !curSpecial ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        specialIndex[index] = curSpecial->index;

      }
      else if ( strcmp( tk, "invalid" ) == 0 ) {

        index = COLORINFO_K_INVALID;

        if ( index > maxSpecial ) maxSpecial = index;

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, ":" ) != 0 ) {
          parseError( colorInfoClass_str19 );
          parseStatus = FAIL;
          goto term;
        }

        // get color name
        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }

        stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
         (void **) &curSpecial );
        if ( !( stat & 1 ) ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        if ( !curSpecial ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        specialIndex[index] = curSpecial->index;

      }
      else if ( strcmp( tk, "minor" ) == 0 ) {

        index = COLORINFO_K_MINOR;

        if ( index > maxSpecial ) maxSpecial = index;

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, ":" ) != 0 ) {
          parseError( colorInfoClass_str19 );
          parseStatus = FAIL;
          goto term;
        }

        // get color name
        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }

        stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
         (void **) &curSpecial );
        if ( !( stat & 1 ) ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        if ( !curSpecial ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        specialIndex[index] = curSpecial->index;

      }
      else if ( strcmp( tk, "major" ) == 0 ) {

        index = COLORINFO_K_MAJOR;

        if ( index > maxSpecial ) maxSpecial = index;

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, ":" ) != 0 ) {
          parseError( colorInfoClass_str19 );
          parseStatus = FAIL;
          goto term;
        }

        // get color name
        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }

        stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
         (void **) &curSpecial );
        if ( !( stat & 1 ) ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        if ( !curSpecial ) {
          parseError( colorInfoClass_str20 );
          parseStatus = FAIL;
          goto term;
        }
        specialIndex[index] = curSpecial->index;

      }
      else if ( strcmp( tk, "noalarm" ) == 0 ) {

        index = COLORINFO_K_NOALARM;

        if ( index > maxSpecial ) maxSpecial = index;

        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }
        if ( strcmp( tk, ":" ) != 0 ) {
          parseError( colorInfoClass_str19 );
          parseStatus = FAIL;
          goto term;
        }

        // get color name
        stat = getToken( tk );
        if ( stat == FAIL ) {
          parseError( colorInfoClass_str9 );
          parseStatus = stat;
          goto term;
        }
        if ( strcmp( tk, "" ) == 0 ) {
          parseError( colorInfoClass_str10 );
          parseStatus = FAIL;
          goto term;
        }

        if ( strcmp( tk, "*" ) == 0 ) {

          specialIndex[index] = -1;
	  showNoAlarmState = 0;
          
	}
	else {

          stat = avl_get_match( this->colorCacheByNameH, (void *) tk,
           (void **) &curSpecial );
          if ( !( stat & 1 ) ) {
            parseError( colorInfoClass_str20 );
            parseStatus = FAIL;
            goto term;
          }
          if ( !curSpecial ) {
            parseError( colorInfoClass_str20 );
            parseStatus = FAIL;
            goto term;
          }
          specialIndex[index] = curSpecial->index;

	}

      }
      else if ( strcmp( tk, "}" ) == 0 ) {

        if ( maxSpecial < 4 ) {
          parseError( colorInfoClass_str21 );
          parseStatus = FAIL;
          goto term;
        }
 
        state = GET_FIRST_TOKEN;
 
      }
      else {

        parseError( colorInfoClass_str22 );
        parseStatus = FAIL;
        goto term;

      }

      break;

    }

  }

term:

  fclose( f );

  if ( parseStatus != SUCCESS ) {
    return 0;
  }

  change = 1;
  blink = 0;
  curIndex = 0;
  curX = 5;
  curY = 5;

  colors = new unsigned int[max_colors];
  blinkingColors = new unsigned int[max_colors];
  simpleColorButtons = new simpleButtonType[max_colors];
  colorNames = new char *[max_colors];
  colorNodes = new colorCachePtr[max_colors];

  for ( i=0; i<max_colors; i++ ) {
    colors[i] = 0;
    blinkingColors[i] = 0;
    colorNames[i] = NULL;
    colorNodes[i] = NULL;
  }

  stat = avl_get_first( this->colorCacheByPosH, (void **) &cur1 );
  if ( !( stat & 1 ) ) {
    return 0;
  }

  while ( cur1 ) {

    i = cur1->index;
    if ( i > max_colors-1 ) i = max_colors-1; // sanity check

    colorNodes[i] = cur1;

    // Allocate X color for static rules only
    if ( !cur1->rule ) { // not a dynamic color rule

      colorNames[i] = cur1->name; // populate color name array

      color.red = cur1->rgb[0];
      color.green = cur1->rgb[1];
      color.blue = cur1->rgb[2];

      stat = XAllocColor( display, cmap, &color );
      if ( stat ) {
        colors[i] = color.pixel;
      }
      else {
        colors[i] = BlackPixel( display, screen );
      }
      cur1->pixel = colors[i];

      if ( ( cur1->rgb[0] != cur1->blinkRgb[0] ) ||
           ( cur1->rgb[1] != cur1->blinkRgb[1] ) ||
           ( cur1->rgb[2] != cur1->blinkRgb[2] ) ) {

        color.red = cur1->blinkRgb[0];
        color.green = cur1->blinkRgb[1];
        color.blue = cur1->blinkRgb[2];

        stat = XAllocColor( display, cmap, &color );
        if ( stat ) {
          blinkingColors[i] = color.pixel;
        }
        else {
          blinkingColors[i] = BlackPixel( display, screen );
        }
        cur1->blinkPixel = blinkingColors[i];

      }
      else {
        blinkingColors[i] = colors[i];
        cur1->blinkPixel = blinkingColors[i];
      }

      // --------------------------------------------------------------
      // update tree sorted by name
      stat = avl_get_match( this->colorCacheByNameH, (void *) cur1->name,
       (void **) &cur2 );
      if ( cur2 ) {
        cur2->pixel = cur1->pixel;
        cur2->blinkPixel = cur1->blinkPixel;
        for ( ii=0; ii<3; ii++ ) {
          cur2->rgb[ii] = cur1->rgb[ii];
          cur2->blinkRgb[ii] = cur1->blinkRgb[ii];
        }
      }
      // --------------------------------------------------------------

      // --------------------------------------------------------------
      // update tree sorted by index
      stat = avl_get_match( this->colorCacheByIndexH,
       (void *) &cur1->index,
       (void **) &cur3 );
      if ( cur3 ) {
        cur3->pixel = cur1->pixel;
        cur3->blinkPixel = cur1->blinkPixel;
      }
      // --------------------------------------------------------------

      // --------------------------------------------------------------
      // update tree sorted by color
      stat = avl_get_match( this->colorCacheByColorH,
       (void *) &cur1->rgb,
       (void **) &cur4 );
      if ( cur4 ) {
        cur4->pixel = cur1->pixel;
        cur4->blinkPixel = cur1->blinkPixel;
      }
      // --------------------------------------------------------------

    }

    stat = avl_get_next( this->colorCacheByPosH, (void **) &cur1 );
    if ( !( stat & 1 ) ) {
      return 0;
    }

  }

  //fprintf( stderr, "fixup dynamic rules\n" );

  // map dynamic color rule result name into result index
  for ( i=0; i<max_colors; i++ ) {

    //fprintf( stderr, "i = %-d\n", i );

    if ( !colorNodes[i] ) {
      fprintf( stderr, colorInfoClass_str33, i );
      return 0;
    }

    if ( colorNodes[i]->rule ) { // dynamic color rules only

      //fprintf( stderr, "i=%-d, rule is %s, index=%-d\n", i, colorNodes[i]->name,
      // colorNodes[i]->index );

      colorNames[i] = colorNodes[i]->name; // populate color name array

      firstCond = 1;
      ruleCond = colorNodes[i]->rule->ruleHead->flink;

      if ( !ruleCond ) {
        fprintf( stderr, colorInfoClass_str30, colorNodes[i]->name );
        return 0;
      }

      while ( ruleCond ) {

        if ( ruleCond->resultName ) { // this condition might be combined
                                      // with others via ||, &&

          stat = avl_get_match( this->colorCacheByNameH,
           (void *) ruleCond->resultName, (void **) &cur1 );
          if ( cur1 ) {
            if ( isRule( cur1->index ) ) { // rules may not reference
	                                   // other rules
              fprintf( stderr, colorInfoClass_str29, colorNodes[i]->name );
              return 0;
            }
            ruleCond->result = cur1->index;
            //fprintf( stderr, "result name %s --> %-d\n", ruleCond->resultName,
            // ruleCond->result );
          }
          else {
            fprintf( stderr, colorInfoClass_str28, colorNodes[i]->name );
            return 0;
          }

          if ( firstCond ) { // use first rule condition as static
	                     // color for rule

            colors[i] = cur1->pixel;
            blinkingColors[i] = cur1->blinkPixel;

            firstCond = 0;
            colorNodes[i]->pixel = cur1->pixel;
            colorNodes[i]->blinkPixel = cur1->blinkPixel;
            for ( ii=0; ii<3; ii++ ) {
              colorNodes[i]->rgb[ii] = cur1->rgb[ii];
              colorNodes[i]->blinkRgb[ii] = cur1->blinkRgb[ii];
            }

            // --------------------------------------------------------------
            // update tree keyed by name
            stat = avl_get_match( this->colorCacheByNameH,
             (void *) colorNodes[i]->name,
             (void **) &cur2 );
            if ( cur2 ) {
              cur2->pixel = colorNodes[i]->pixel;
              cur2->blinkPixel = colorNodes[i]->blinkPixel;
              for ( ii=0; ii<3; ii++ ) {
                cur2->rgb[ii] = colorNodes[i]->rgb[ii];
                cur2->blinkRgb[ii] = colorNodes[i]->blinkRgb[ii];
              }
            }
            // --------------------------------------------------------------

            // --------------------------------------------------------------
            // update tree keyed by index
            stat = avl_get_match( this->colorCacheByIndexH,
             (void *) &cur2->index,
             (void **) &cur3 );
            if ( cur3 ) {
              cur3->pixel = colorNodes[i]->pixel;
              cur3->blinkPixel = colorNodes[i]->blinkPixel;
              for ( ii=0; ii<3; ii++ ) {
                cur3->rgb[ii] = colorNodes[i]->rgb[ii];
                cur3->blinkRgb[ii] = colorNodes[i]->blinkRgb[ii];
              }
            }
            // --------------------------------------------------------------

            // --------------------------------------------------------------
            // update tree keyed by position
            stat = avl_get_match( this->colorCacheByPosH,
             (void *) &cur2->position,
             (void **) &cur3 );
            if ( cur3 ) {
              cur3->pixel = colorNodes[i]->pixel;
              cur3->blinkPixel = colorNodes[i]->blinkPixel;
              for ( ii=0; ii<3; ii++ ) {
                cur3->rgb[ii] = colorNodes[i]->rgb[ii];
                cur3->blinkRgb[ii] = colorNodes[i]->blinkRgb[ii];
              }
            }
            // --------------------------------------------------------------

	  }

	}

        ruleCond = ruleCond->flink;

      }

    }

  }

#if 0
  {

    double v;

    fprintf( stderr, "eval rules\n\n\n" );

    i = 0;

    v = 0;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = 1;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = .5;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = 1000;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = 5.5;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = 99.1;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

    v = -1;
    fprintf( stderr, "rule %-d, value=%-g, result=%-d\n\n", i, v, evalRule( i, v ) );

  }
#endif

#if 0

  stat = avl_get_first( this->colorCacheByIndexH, (void **) &cur1 );
  if ( !( stat & 1 ) ) {
    return 0;
  }

  while ( cur1 ) {

    fprintf( stderr, "name: %s, index: %-d, r: %-d, g: %-d, b: %-d, p: %-d, bp: %-d\n",
     cur1->name, cur1->index, cur1->rgb[0], cur1->rgb[1], cur1->rgb[2],
     cur1->pixel, cur1->blinkPixel );

    stat = avl_get_next( this->colorCacheByIndexH, (void **) &cur1 );
    if ( !( stat & 1 ) ) {
      fprintf( stderr, "error 1\n" );
      return 0;
    }

  }

  fprintf( stderr, "\n" );

  stat = avl_get_first( this->colorCacheByNameH, (void **) &cur1 );
  if ( !( stat & 1 ) ) {
    fprintf( stderr, "error 1\n" );
    return 0;
  }

  while ( cur1 ) {

    fprintf( stderr, "name: %s, index: %-d, r: %-d, g: %-d, b: %-d, p: %-d, bp: %-d\n",
     cur1->name, cur1->index, cur1->rgb[0], cur1->rgb[1], cur1->rgb[2],
     cur1->pixel, cur1->blinkPixel );

    stat = avl_get_next( this->colorCacheByNameH, (void **) &cur1 );
    if ( !( stat & 1 ) ) {
      fprintf( stderr, "error 1\n" );
      return 0;
    }

  }

#endif

  // create window

  //shell = XtVaAppCreateShell( colorInfoClass_str2, colorInfoClass_str2,
  //shell = XtVaAppCreateShell( "edm", "edm",
  // topLevelShellWidgetClass,
  // XtDisplay(top),
  // XtNmappedWhenManaged, False,
  // NULL );

  shell = XtVaCreatePopupShell( "edm", topLevelShellWidgetClass,
   top,
   XtNmappedWhenManaged, False,
   NULL );

  rc = XtVaCreateWidget( "colorpallete", xmRowColumnWidgetClass, shell,
   XmNorientation, XmVERTICAL,
   XmNnumColumns, 1,
   NULL );

  str1 = XmStringCreateLocalized( colorInfoClass_str3 );
  mbar = XmVaCreateSimpleMenuBar( rc, "menubar",
   XmVaCASCADEBUTTON, str1, 'f',
   NULL );
  XmStringFree( str1 );

  str1 = XmStringCreateLocalized( colorInfoClass_str4 );
  str2 = XmStringCreateLocalized( colorInfoClass_str5 );
  mb1 = XmVaCreateSimplePulldownMenu( mbar, "pb", 0, file_cb,
   XmVaPUSHBUTTON, str1, 'x', NULL, NULL,
   XmVaPUSHBUTTON, str2, 's', NULL, NULL,
   NULL );
  XmStringFree( str1 );
  XmStringFree( str2 );

  ncols = num_color_cols;
  nrows = (max_colors) / ncols;
  remainder = (max_colors) % ncols;
  if ( remainder ) nrows++;

  form = XtVaCreateWidget( "form", xmDrawingAreaWidgetClass, rc,
   XmNwidth, ncols*20 + ncols*5 + 5,
   XmNheight, nrows*20 + nrows*5 + 5,
   XmNresizePolicy, XmRESIZE_NONE,
   NULL );

  XtAddEventHandler( form,
   PointerMotionMask|ButtonPressMask|ExposureMask|
   LeaveWindowMask|StructureNotifyMask, False,
   colorFormEventHandler, (XtPointer) this );

  XtAddEventHandler( shell,
   StructureNotifyMask, False,
   colorShellEventHandler, (XtPointer) this );

  Atom wm_delete_window = XmInternAtom( XtDisplay(shell), "WM_DELETE_WINDOW",
   False );

  XmAddWMProtocolCallback( shell, wm_delete_window, file_cb,
    (XtPointer) 0 );

  XtVaSetValues( shell, XmNdeleteResponse, XmDO_NOTHING, NULL );

  XtManageChild( mbar );
  XtManageChild( rc );
  XtManageChild( form );
  XtRealizeWidget( shell );
  XSetWindowColormap( display, XtWindow(shell), cmap );

  gc.create( shell );
  gc.setCI( this );

  n = 0;
  XtSetArg( arg[n], XmNbackground, &bgColor ); n++;
  XtGetValues( form, arg, n );

  gc.setBG( bgColor );
  gc.setBaseBG( bgColor );

  numColors = max_colors;

  // populate special array
  for ( i=0; i<NUM_SPECIAL_COLORS-2; i++ ) {
    if ( specialIndex[i] != -1 ) {
      special[i] = colors[ specialIndex[i] ];
    }
    else {
      special[i] = -1;
    }
  }

  msgDialog.create( shell );

  if ( blinkMs < 250 ) {
    incrementTimerValue = 250;
  }
  else if ( blinkMs > 2000 ) {
    incrementTimerValue = 2000;
  }
  else {
    incrementTimerValue = blinkMs;
  }
  incrementTimerActive = 1;
  incrementTimer = appAddTimeOut( appCtx, incrementTimerValue,
   toggleColorBlink, this );

  ncols = num_color_cols;
  nrows = (max_colors+num_blinking_colors) / ncols;
  remainder = (max_colors+num_blinking_colors) % ncols;

  pos = 0;
  for ( r=0; r<nrows; r++ ) {

    for ( c=0; c<ncols; c++ ) {

      x = c*5 + c*20 + 5;
      y = r*5 + r*20 + 5;

      stat = avl_get_match( this->colorCacheByPosH,
       (void *) &pos, (void **) &cur1 );
      if ( stat & 1 ) {
        if ( cur1 ) {
          simpleColorButtons[pos].colorIndex = cur1->index;
        }
        else {
          simpleColorButtons[pos].colorIndex = 0;
        }
      }
      else {
        simpleColorButtons[pos].colorIndex = 0;
      }

      simpleColorButtons[pos].wgt = form;
      simpleColorButtons[pos].cio = this;
      simpleColorButtons[pos].x = x;
      simpleColorButtons[pos].y = y;
      simpleColorButtons[pos].w = 23;
      simpleColorButtons[pos].h = 23;
      simpleColorButtons[pos].blink = 0;

      pos++;

    }

  }

  if ( remainder ) {

    r = nrows;

    for ( c=0; c<remainder; c++ ) {

      x = c*5 + c*20 + 5;
      y = r*5 + r*20 + 5;

      stat = avl_get_match( this->colorCacheByPosH,
       (void *) &pos, (void **) &cur1 );
      if ( stat & 1 ) {
        if ( cur1 ) {
          simpleColorButtons[pos].colorIndex = cur1->index;
        }
        else {
          simpleColorButtons[pos].colorIndex = 0;
        }
      }
      else {
        simpleColorButtons[pos].colorIndex = 0;
      }

      simpleColorButtons[pos].wgt = form;
      simpleColorButtons[pos].cio = this;
      simpleColorButtons[pos].x = x;
      simpleColorButtons[pos].y = y;
      simpleColorButtons[pos].w = 23;
      simpleColorButtons[pos].h = 23;
      simpleColorButtons[pos].blink = 0;

      pos++;

    }

  }

  return 1;

}

int colorInfoClass::initFromFile (
  XtAppContext app,
  Display *d,
  Widget top,
  char *fileName )
{

char line[127+1], *ptr, *tk, *junk, *envPtr;
int i, index, iOn, iOff, n, stat, nrows, ncols, remainder, dup, nSpecial,
 x, y, r, c, pos;
FILE *f;
XColor color;
Arg arg[20];
XmString str1, str2;
colorCachePtr cur, curSpecial;
int rgb[3], red=0, green=0, blue=0;
unsigned long plane_masks[1], bgColor;

  if ( !this->colorCacheByIndexH ) return 0;

  envPtr = getenv( "EDMCOLORMODE" );
  if ( envPtr ) {
    if ( strcmp( envPtr, "rgb" ) == 0 ) {
      useRGB();
    }
    else if ( strcmp( envPtr, "RGB" ) == 0 ) {
      useRGB();
    }
  }

  appCtx = app;
  display = d;
  screen = DefaultScreen( d );
  depth = DefaultDepth( d, screen );
  visual = DefaultVisual( d, screen );
  cmap = DefaultColormap( d, screen );

  change = 1;
  blink = 0;
  curIndex = 0;
  curX = 5;
  curY = 5;

  maxMenuItems = 0;
  menuIndexMap = NULL;

  //f = fopen( fileName, "r" );
  f = fileOpen( fileName, "r" );
  if ( !f ) {
    return COLORINFO_NO_FILE;
  }

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

  if ( major == 3 ) {
    stat = ver3InitFromFile( f, app, d, top, fileName );
    if ( stat & 1 ) colorList.create( max_colors, top, 20, this );
    return stat;
  }

  if ( major == 4 ) {
    stat = ver4InitFromFile( f, app, d, top, fileName );
    if ( stat & 1 ) colorList.create( max_colors, top, 20, this );
    return stat;
  }

  goto firstTry;

restart:

  // at this point, a private color map is being used

  delete colors;
  delete blinkingColors;
  delete blinkingColorCells;
  delete blinkingXColor;
  delete offBlinkingXColor;

  stat = avl_destroy( colorCacheByColorH );
  stat = avl_destroy( colorCacheByPixelH );
  stat = avl_destroy( colorCacheByIndexH );
  stat = avl_destroy( colorCacheByAliasH );
  stat = avl_destroy( colorCacheByNameH );
  stat = avl_destroy( colorCacheByPosH );
  stat = avl_destroy( blinkH );

  stat = avl_init_tree( compare_nodes_by_color,
   compare_key_by_color, copy_nodes, &(this->colorCacheByColorH) );
  if ( !( stat & 1 ) ) this->colorCacheByColorH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_pixel,
   compare_key_by_pixel, copy_nodes, &(this->colorCacheByPixelH) );
  if ( !( stat & 1 ) ) this->colorCacheByPixelH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_index,
   compare_key_by_index, copy_nodes, &(this->colorCacheByIndexH) );
  if ( !( stat & 1 ) ) this->colorCacheByIndexH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_name,
   compare_key_by_name, copy_nodes, &(this->colorCacheByAliasH) );
  if ( !( stat & 1 ) ) this->colorCacheByAliasH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_name,
   compare_key_by_name, copy_nodes, &(this->colorCacheByNameH) );
  if ( !( stat & 1 ) ) this->colorCacheByNameH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_nodes_by_pos,
   compare_key_by_pos, copy_nodes, &(this->colorCacheByPosH) );
  if ( !( stat & 1 ) ) this->colorCacheByPosH = (AVL_HANDLE) NULL;

  stat = avl_init_tree( compare_blink_nodes,
   compare_blink_key, copy_blink_nodes, &(this->blinkH) );
  if ( !( stat & 1 ) ) this->blinkH = (AVL_HANDLE) NULL;

  fclose( f );

  //f = fopen( fileName, "r" );
  f = fileOpen( fileName, "r" );
  if ( !f ) {
    return COLORINFO_NO_FILE;
  }

  fscanf( f, "%d %d %d\n", &major, &minor, &release );

firstTry:

  if ( major < 2 ) {
    max_colors = 88;
    num_blinking_colors = 8;
    num_color_cols = 11;
  }
  else {
    fscanf( f, "%d %d %d\n", &max_colors, &num_blinking_colors,
     &num_color_cols );
  }

  colors = new unsigned int[max_colors+num_blinking_colors];
  blinkingColors = new unsigned int[max_colors+num_blinking_colors];
  blinkingColorCells = new unsigned long[num_blinking_colors];
  blinkingXColor = new XColor[num_blinking_colors];
  offBlinkingXColor = new XColor[num_blinking_colors];
  colorNames = new char *[max_colors+num_blinking_colors+1];
  simpleColorButtons = new simpleButtonType[max_colors+num_blinking_colors];

  junk =  new char[strlen("n/a")+1]; // tiny memory leak here
  strcpy( junk, "n/a" );             // for color files < version 3

  numColors = 0;

  index = 0;
  for ( i=0; i<(max_colors); i++ ) {

    colorNames[i] = junk;

    ptr = fgets ( line, 127, f );
    if ( ptr ) {

      numColors++;

      tk = strtok( line, " \t\n" );
      if ( tk )
        red = atol( tk );
      else
        red = 0;

      if ( major < 2 ) red *= 256;
      color.red = red;

      tk = strtok( NULL, " \t\n" );
      if ( tk )
        green = atol( tk );
      else
        green = 0;

      if ( major < 2 ) green *= 256;
      color.green = green;

      tk = strtok( NULL, " \t\n" );
      if ( tk )
        blue = atol( tk );
      else
        blue = 0;

      if ( major < 2 ) blue *= 256;
      color.blue = blue;

      stat = XAllocColor( display, cmap, &color );

      if ( stat ) {
        colors[i] = color.pixel;
        blinkingColors[i] = color.pixel;
      }
      else {

        if ( !usingPrivateColorMap && usePrivColorMapFlag ) {
          usingPrivateColorMap = 1;
	  cmap = XCopyColormapAndFree( display, cmap );
	  XSetWindowColormap( display, XtWindow(top), cmap );
          goto restart;
	}

        colors[i] = BlackPixel( display, screen );
        blinkingColors[i] = BlackPixel( display, screen );

      }

    }
    else {
      if ( i ) {
        colors[i] = BlackPixel( display, screen );
        blinkingColors[i] = BlackPixel( display, screen );
      }
      else {
        colors[i] = WhitePixel( display, screen );
        blinkingColors[i] = WhitePixel( display, screen );
      }
    }

    cur = new colorCacheType;
    if ( !cur ) return 0;

    cur->rgb[0] = (unsigned int) red;
    cur->rgb[1] = (unsigned int) green;
    cur->rgb[2] = (unsigned int) blue;
    cur->pixel = colors[i];
    cur->blinkPixel = blinkingColors[i]; // new
    cur->index = index;
    cur->position = index;

    cur->name = NULL;
    cur->rule = NULL;

    stat = avl_insert_node( this->colorCacheByColorH, (void *) cur,
     &dup );
    if ( !( stat & 1 ) ) {
      delete cur;
      fclose( f );
      return stat;
    }

    if ( dup ) delete cur;

    cur = new colorCacheType;
    if ( !cur ) return 0;

    cur->rgb[0] = (unsigned int) red;
    cur->rgb[1] = (unsigned int) green;
    cur->rgb[2] = (unsigned int) blue;
    cur->pixel = colors[i];
    cur->blinkPixel = blinkingColors[i]; // new
    cur->index = index;
    cur->position = index;

    cur->name = NULL;
    cur->rule = NULL;

    stat = avl_insert_node( this->colorCacheByPixelH, (void *) cur,
     &dup );
    if ( !( stat & 1 ) ) {
      delete cur;
      fclose( f );
      return stat;
    }

    if ( dup ) delete cur;

    cur = new colorCacheType;
    if ( !cur ) return 0;

    cur->rgb[0] = (unsigned int) red;
    cur->rgb[1] = (unsigned int) green;
    cur->rgb[2] = (unsigned int) blue;
    cur->pixel = colors[i];
    cur->blinkPixel = blinkingColors[i]; // new
    cur->index = index;
    cur->position = index;

    cur->name = NULL;
    cur->rule = NULL;

    stat = avl_insert_node( this->colorCacheByIndexH, (void *) cur,
     &dup );
    if ( !( stat & 1 ) ) {
      delete cur;
      fclose( f );
      return stat;
    }

    if ( dup ) delete cur;

    cur = new colorCacheType;
    if ( !cur ) return 0;

    cur->rgb[0] = (unsigned int) red;
    cur->rgb[1] = (unsigned int) green;
    cur->rgb[2] = (unsigned int) blue;
    cur->pixel = colors[i];
    cur->blinkPixel = blinkingColors[i]; // new
    cur->index = index;
    cur->position = index;

    cur->name = NULL;
    cur->rule = NULL;

    stat = avl_insert_node( this->colorCacheByPosH, (void *) cur,
     &dup );
    if ( !( stat & 1 ) ) {
      delete cur;
      fclose( f );
      return stat;
    }

    if ( dup ) delete cur;

    index++;

  }

  stat = XAllocColorCells( display, cmap, False, plane_masks, 0,
   blinkingColorCells, num_blinking_colors );

  if ( stat ) { // success

    for ( i=0; i<num_blinking_colors; i++ ) {
      colorNames[max_colors+i] = junk;
    }

    // blinking colors
    iOn = 0;
    iOff = 0;
    for ( i=0; i<num_blinking_colors*2; i++ ) {

      ptr = fgets ( line, 127, f );
      if ( ptr ) {

        tk = strtok( line, " \t\n" );
        if ( tk )
          red = atol( tk );
        else
          red = 0;

        if ( major < 2 ) red *= 256;
        color.red = red;

        tk = strtok( NULL, " \t\n" );
        if ( tk )
          green = atol( tk );
        else
          green = 0;

        if ( major < 2 ) green *= 256;
        color.green = green;

        tk = strtok( NULL, " \t\n" );
        if ( tk )
          blue = atol( tk );
        else
          blue = 0;

        if ( major < 2 ) blue *= 256;
        color.blue = blue;

        if ( !( i % 2 ) ) {
          color.pixel = blinkingColorCells[iOn];
          color.flags = DoRed | DoGreen | DoBlue;
          colors[numColors] = color.pixel;
          blinkingColors[numColors] = color.pixel;
          blinkingXColor[iOn] = color;
          iOn++;
          XStoreColor( display, cmap, &color );
        }
        else {
          offBlinkingXColor[iOff] = color;
          iOff++;
        }

      }
      else {
        if ( numColors ) {
          colors[numColors] = BlackPixel( display, screen );
          blinkingColors[numColors] = BlackPixel( display, screen );
        }
        else {
          colors[numColors] = WhitePixel( display, screen );
          blinkingColors[numColors] = WhitePixel( display, screen );
        }
      }

      if ( !( i % 2 ) ) {

        cur = new colorCacheType;
        if ( !cur ) return 0;

        cur->rgb[0] = (unsigned int) red;
        cur->rgb[1] = (unsigned int) green;
        cur->rgb[2] = (unsigned int) blue;
        cur->pixel = colors[numColors];
        cur->blinkPixel = blinkingColors[i]; // new
        cur->index = index;
        cur->position = index;

        cur->name = NULL;
        cur->rule = NULL;

        stat = avl_insert_node( this->colorCacheByColorH, (void *) cur,
         &dup );
        if ( !( stat & 1 ) ) {
          delete cur;
          fclose( f );
          return stat;
        }

        if ( dup ) delete cur;

        cur = new colorCacheType;
        if ( !cur ) return 0;

        cur->rgb[0] = (unsigned int) red;
        cur->rgb[1] = (unsigned int) green;
        cur->rgb[2] = (unsigned int) blue;
        cur->pixel = colors[numColors];
        cur->blinkPixel = blinkingColors[i]; // new
        cur->index = index;
        cur->position = index;

        cur->name = NULL;
        cur->rule = NULL;

        stat = avl_insert_node( this->colorCacheByPixelH, (void *) cur,
         &dup );
        if ( !( stat & 1 ) ) {
          delete cur;
          fclose( f );
          return stat;
        }

        if ( dup ) delete cur;

        cur = new colorCacheType;
        if ( !cur ) return 0;

        cur->rgb[0] = (unsigned int) red;
        cur->rgb[1] = (unsigned int) green;
        cur->rgb[2] = (unsigned int) blue;
        cur->pixel = colors[numColors];
        cur->blinkPixel = blinkingColors[i]; // new
        cur->index = index;
        cur->position = index;

        cur->name = NULL;
        cur->rule = NULL;

        stat = avl_insert_node( this->colorCacheByIndexH, (void *) cur,
         &dup );
        if ( !( stat & 1 ) ) {
          delete cur;
          fclose( f );
          return stat;
        }

        if ( dup ) delete cur;

        cur = new colorCacheType;
        if ( !cur ) return 0;

        cur->rgb[0] = (unsigned int) red;
        cur->rgb[1] = (unsigned int) green;
        cur->rgb[2] = (unsigned int) blue;
        cur->pixel = colors[numColors];
        cur->blinkPixel = blinkingColors[i]; // new
        cur->index = index;
        cur->position = index;

        cur->name = NULL;
        cur->rule = NULL;

        stat = avl_insert_node( this->colorCacheByPosH, (void *) cur,
         &dup );
        if ( !( stat & 1 ) ) {
          delete cur;
          fclose( f );
          return stat;
        }

        if ( dup ) delete cur;

        numColors++;
        index++;

      }

    }

  }
  else {

    if ( !usingPrivateColorMap && usePrivColorMapFlag ) {
      usingPrivateColorMap = 1;
      cmap = XCopyColormapAndFree( display, cmap );
      XSetWindowColormap( display, XtWindow(top), cmap );
      goto restart;
    }

    fprintf( stderr, colorInfoClass_str1 );
    // discard file contents
    for ( i=0; i<num_blinking_colors*2; i++ ) {
      ptr = fgets ( line, 127, f );
    }
    num_blinking_colors = 0;

  }

  // special colors are disconnected, severity=invalid,
  // severity=minor, severity=major, severity=noalarm
  if ( major > 2 ) {
    nSpecial = NUM_SPECIAL_COLORS;
  }
  else {
    nSpecial = NUM_SPECIAL_COLORS - 2; // don't include ack alarm colors
    special[NUM_SPECIAL_COLORS - 2] = (int) BlackPixel( display, screen );
    special[NUM_SPECIAL_COLORS - 1] = (int) BlackPixel( display, screen );
  }

  for ( i=0; i<nSpecial; i++ ) {

    ptr = fgets ( line, 127, f );
    if ( ptr ) {

      tk = strtok( line, " \t\n" );
      if ( tk )
        rgb[0] = atol( tk );
      else
        rgb[0] = 0;

      tk = strtok( NULL, " \t\n" );
      if ( tk )
        rgb[1] = atol( tk );
      else
        rgb[1] = 0;

      tk = strtok( NULL, " \t\n" );
      if ( tk )
        rgb[2] = atol( tk );
      else
        rgb[2] = 0;

      if ( rgb[0] != -1 ) {

        if ( major < 2 ) {
          rgb[0] *= 256;
          rgb[1] *= 256;
          rgb[2] *= 256;
        }

        stat = avl_get_match( this->colorCacheByColorH, (void *) rgb,
         (void **) &curSpecial );

        if ( ( stat & 1 ) && curSpecial ) {
          special[i] = (int) curSpecial->pixel;
        }
        else {
          special[i] = (int) BlackPixel( display, screen );
        }

      }
      else {

        special[i] = -1;

      }

    }
    else {
      special[i] = (int) BlackPixel( display, screen );
    }

  }

  fclose( f );

#if 0

  stat = avl_get_first( this->colorCacheByIndexH, (void **) &cur );
  if ( !( stat & 1 ) ) {
    return 0;
  }

  while ( cur ) {

    fprintf( stderr, "name: %s, index: %-d, r: %-d, g: %-d, b: %-d, p: %-d, bp: %-d\n",
     cur->name, cur->index, cur->rgb[0], cur->rgb[1], cur->rgb[2],
     cur->pixel, cur->blinkPixel );

    stat = avl_get_next( this->colorCacheByIndexH, (void **) &cur );
    if ( !( stat & 1 ) ) {
      fprintf( stderr, "error 1\n" );
      return 0;
    }

  }

  fprintf( stderr, "\n" );

#endif

  // create window

  //shell = XtVaAppCreateShell( colorInfoClass_str2, colorInfoClass_str2,
  //shell = XtVaAppCreateShell( "edm", "edm",
  // topLevelShellWidgetClass,
  // XtDisplay(top),
  // XtNmappedWhenManaged, False,
  // NULL );

  shell = XtVaCreatePopupShell( "edm", topLevelShellWidgetClass,
   top,
   XtNmappedWhenManaged, False,
   NULL );

  rc = XtVaCreateWidget( "colorpallete", xmRowColumnWidgetClass, shell,
   XmNorientation, XmVERTICAL,
   XmNnumColumns, 1,
   NULL );

  str1 = XmStringCreateLocalized( colorInfoClass_str3 );
  mbar = XmVaCreateSimpleMenuBar( rc, "menubar",
   XmVaCASCADEBUTTON, str1, 'f',
   NULL );
  XmStringFree( str1 );

  str1 = XmStringCreateLocalized( colorInfoClass_str4 );
  str2 = XmStringCreateLocalized( colorInfoClass_str5 );
  mb1 = XmVaCreateSimplePulldownMenu( mbar, "pb", 0, file_cb,
   XmVaPUSHBUTTON, str1, 'x', NULL, NULL,
   XmVaPUSHBUTTON, str2, 's', NULL, NULL,
   NULL );
  XmStringFree( str1 );
  XmStringFree( str2 );

//   form = XtVaCreateManagedWidget( "form", xmFormWidgetClass, rc,
//    NULL );

  ncols = num_color_cols;
  nrows = (max_colors+num_blinking_colors) / ncols;
  remainder = (max_colors+num_blinking_colors) % ncols;
  if ( remainder ) nrows++;

  form = XtVaCreateManagedWidget( "form", xmDrawingAreaWidgetClass, rc,
   XmNwidth, ncols*20 + ncols*5 + 5,
   XmNheight, nrows*20 + nrows*5 + 5,
   NULL );

  XtAddEventHandler( form,
   PointerMotionMask|ButtonPressMask|ExposureMask|
   LeaveWindowMask|StructureNotifyMask, False,
   colorFormEventHandler, (XtPointer) this );

  XtAddEventHandler( shell,
   StructureNotifyMask, False,
   colorShellEventHandler, (XtPointer) this );

  Atom wm_delete_window = XmInternAtom( XtDisplay(shell), "WM_DELETE_WINDOW",
   False );

  XmAddWMProtocolCallback( shell, wm_delete_window, file_cb,
    (XtPointer) 0 );

  XtVaSetValues( shell, XmNdeleteResponse, XmDO_NOTHING, NULL );

  XtManageChild( mbar );
  XtManageChild( rc );
  XtRealizeWidget( shell );
  XSetWindowColormap( display, XtWindow(shell), cmap );

  gc.create( shell );
  gc.setCI( this );

   n = 0;
   XtSetArg( arg[n], XmNbackground, &bgColor ); n++;
   XtGetValues( form, arg, n );

  gc.setBG( bgColor );
  gc.setBaseBG( bgColor );

  if ( num_blinking_colors ) {
    incrementTimerValue = 500;
    incrementTimerActive = 1;
    incrementTimer = appAddTimeOut( appCtx, incrementTimerValue,
     doColorBlink, this );
  }

  colorList.create( max_colors+num_blinking_colors, top, 20, this );

  msgDialog.create( shell );

  ncols = num_color_cols;
  nrows = (max_colors+num_blinking_colors) / ncols;
  remainder = (max_colors+num_blinking_colors) % ncols;

  pos = 0;
  for ( r=0; r<nrows; r++ ) {

    for ( c=0; c<ncols; c++ ) {

      x = c*5 + c*20 + 5;
      y = r*5 + r*20 + 5;

      stat = avl_get_match( this->colorCacheByPosH,
       (void *) &pos, (void **) &cur );
      if ( stat & 1 ) {
        if ( cur ) {
          simpleColorButtons[pos].colorIndex = cur->index;
        }
        else {
          simpleColorButtons[pos].colorIndex = 0;
        }
      }
      else {
        simpleColorButtons[pos].colorIndex = 0;
      }

      simpleColorButtons[pos].wgt = form;
      simpleColorButtons[pos].cio = this;
      simpleColorButtons[pos].x = x;
      simpleColorButtons[pos].y = y;
      simpleColorButtons[pos].w = 23;
      simpleColorButtons[pos].h = 23;
      simpleColorButtons[pos].blink = 0;

      pos++;

    }

  }

  if ( remainder ) {

    r = nrows;

    for ( c=0; c<remainder; c++ ) {

      x = c*5 + c*20 + 5;
      y = r*5 + r*20 + 5;

      stat = avl_get_match( this->colorCacheByPosH,
       (void *) &pos, (void **) &cur );
      if ( stat & 1 ) {
        if ( cur ) {
          simpleColorButtons[pos].colorIndex = cur->index;
        }
        else {
          simpleColorButtons[pos].colorIndex = 0;
        }
      }
      else {
        simpleColorButtons[pos].colorIndex = 0;
      }

      simpleColorButtons[pos].wgt = form;
      simpleColorButtons[pos].cio = this;
      simpleColorButtons[pos].x = x;
      simpleColorButtons[pos].y = y;
      simpleColorButtons[pos].w = 23;
      simpleColorButtons[pos].h = 23;
      simpleColorButtons[pos].blink = 0;

      pos++;

    }

  }

  return 1;

}

int colorInfoClass::openColorWindow( void ) {

  XtMapWidget( shell );
  XRaiseWindow( display, XtWindow(shell) );

  colorWindowIsOpen = 1;

  return 1;

}

int colorInfoClass::closeColorWindow( void ) {

  XtUnmapWidget( shell );

  colorWindowIsOpen = 0;

  return 1;

}

unsigned int colorInfoClass::getFg( void ) {

  return fg;

}

void colorInfoClass::setCurDestination( int *ptr ) {

  curDestination = ptr;

}

int *colorInfoClass::getCurDestination( void ) {

  return curDestination;

}

void colorInfoClass::setCurCb( colorButtonClass *cb ) {

  curCb = cb;

}

colorButtonClass *colorInfoClass::getCurCb( void ) {

  return curCb;

}

int colorInfoClass::setActiveWidget( Widget w ) {

  activeWidget = w;

  return 1;

}

Widget colorInfoClass::getActiveWidget( void ) {

  return activeWidget;

}

int colorInfoClass::setNameWidget( Widget w ) {

  nameWidget = w;

  return 1;

}

Widget colorInfoClass::getNameWidget( void ) {

  return nameWidget;

}

int colorInfoClass::getRGB(
  unsigned int pixel,
  int *r,
  int *g,
  int *b )
{

XColor color;
int stat, dup;
colorCachePtr cur;

  stat = avl_get_match( this->colorCacheByPixelH, (void *) &pixel,
   (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( cur ) {
    *r = cur->rgb[0];
    *g = cur->rgb[1];
    *b = cur->rgb[2];
    return COLORINFO_SUCCESS;
  }

  *r = 0;
  *g = 0;
  *b = 0;
  color.pixel = pixel;
  stat = XQueryColor( display, cmap, &color );

  if ( !stat ) return COLORINFO_FAIL;

  *r = (int) color.red;
  *g = (int) color.green;
  *b = (int) color.blue;

  cur = new colorCacheType;
  if ( !cur ) return 0;

  colorCacheInit( cur );
  cur->rgb[0] = (unsigned int) *r;
  cur->rgb[1] = (unsigned int) *g;
  cur->rgb[2] = (unsigned int) *b;
  cur->pixel = (unsigned int) pixel;
  cur->blinkPixel = (unsigned int) pixel; // new
  cur->index = -1;

  cur->name = NULL;
  cur->rule = NULL;

  stat = avl_insert_node( this->colorCacheByPixelH, (void *) cur,
   &dup );
  if ( !( stat & 1 ) ) {
    delete cur;
    return stat;
  }

  if ( dup ) delete cur;

  cur = new colorCacheType;
  if ( !cur ) return 0;

  colorCacheInit( cur );
  cur->rgb[0] = (unsigned int) *r;
  cur->rgb[1] = (unsigned int) *g;
  cur->rgb[2] = (unsigned int) *b;
  cur->pixel = (unsigned int) pixel;
  cur->blinkPixel = (unsigned int) pixel; // new
  cur->index = -1;

  cur->name = NULL;
  cur->rule = NULL;

  stat = avl_insert_node( this->colorCacheByColorH, (void *) cur,
   &dup );
  if ( !( stat & 1 ) ) {
    delete cur;
    return stat;
  }

  if ( dup ) delete cur;

  return COLORINFO_SUCCESS;

}

int colorInfoClass::setRGB (
  int r,
  int g,
  int b,
  unsigned int *pixel )
{

int stat;
colorCachePtr cur;
int diff, min=0, foundOne;
unsigned int bestPixel=0;

  //fprintf( stderr, "\n\nstart r=%-d, g=%-d, b=%-d,\n", r, g, b );

  foundOne = 0;

  stat = avl_get_first( this->colorCacheByColorH, (void **) &cur );
  if ( !( stat & 1 ) ) return COLORINFO_FAIL;

  if ( cur ) {
    foundOne = 1;
    min = abs( r - cur->rgb[0] ) + abs( g - cur->rgb[1] ) +
          abs( b - cur->rgb[2] );
    //fprintf( stderr, "min=%-d\n", min );
    if ( min == 0 ) {
      *pixel = cur->pixel;
      return COLORINFO_SUCCESS;
    }
    bestPixel = cur->pixel;
  }

  stat = avl_get_next( this->colorCacheByColorH, (void **) &cur );
  if ( !( stat & 1 ) ) return COLORINFO_FAIL;

  while ( cur ) {

    foundOne = 1;

    diff = abs( r - cur->rgb[0] ) + abs( g - cur->rgb[1] ) +
          abs( b - cur->rgb[2] );
    //fprintf( stderr, "min=%-d\n", diff );
    if ( diff < min ) {
      if ( diff == 0 ) {
        *pixel = cur->pixel;
        return COLORINFO_SUCCESS;
      }
      min = diff;
      bestPixel = cur->pixel;
    }

    stat = avl_get_next( this->colorCacheByColorH, (void **) &cur );
    if ( !( stat & 1 ) ) return COLORINFO_FAIL;

  }

  if ( !foundOne ) return COLORINFO_FAIL;

  *pixel = bestPixel;

  return COLORINFO_SUCCESS;

}

int colorInfoClass::getIndex(
  unsigned int pixel,
  int *index )
{

int stat;
colorCachePtr cur;

  stat = avl_get_match( this->colorCacheByPixelH, (void *) &pixel,
   (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( cur ) {
    *index = cur->index;
    return COLORINFO_SUCCESS;
  }

  return COLORINFO_FAIL;

}

int colorInfoClass::setIndex (
  int index,
  unsigned int *pixel )
{

int stat;
colorCachePtr cur;

//fprintf( stderr, "obsolete colorInfoClass::setIndex\n" );

  stat = avl_get_match( this->colorCacheByIndexH, (void *) &index,
   (void **) &cur );
  if ( !(stat & 1) ) return stat;

  if ( cur ) {
    *pixel = cur->pixel;
    return COLORINFO_SUCCESS;
  }

  return COLORINFO_FAIL;

}

int colorInfoClass::getSpecialColor (
  int index ) {

  if ( index < 0 ) return -1;
  if ( index >= NUM_SPECIAL_COLORS ) return -1;

  return special[index];

}


int colorInfoClass::getSpecialIndex (
  int index ) {

int ix, stat;
unsigned int pixel;

  if ( major < 3 ) {
    pixel = getSpecialColor( index );
    stat = getIndex( pixel, &ix );
    if ( !( stat & 1 ) ) ix = -1;
    return ix;
  }

  if ( index < 0 ) return -1;
  if ( index >= NUM_SPECIAL_COLORS ) return -1;

  return specialIndex[index];

  return 0;

}

Colormap colorInfoClass::getColorMap ( void ) {

  return cmap;

}

int colorInfoClass::setCurIndexByPixel (
  unsigned int pixel ) {

int i, curI=0, stat;

  for ( i=0; i<max_colors+num_blinking_colors; i++ ) {

    if ( colors[i] == pixel ) {

      curI = i;
      break;

    }

  }

  stat = setCurIndex( curI );

  return stat;

}

int colorInfoClass::setCurIndex (
  int index ) {

int stat, x, y, pos, i, r, c, ncols, nrows, remainder;
colorCachePtr cur;

  if ( index > numColors-1 ) {
    curIndex = numColors-1;
  }
  else if ( index < 0 ) {
    curIndex = 0;
  }
  else {
    curIndex = index;
  }

  XDrawRectangle( display, XtWindow(form), gc.eraseGC(), curX-2, curY-2,
   23, 23 );

  ncols = num_color_cols;
  nrows = (max_colors+num_blinking_colors) / ncols;
  remainder = (max_colors+num_blinking_colors) % ncols;

  pos = 0;
  for ( r=0; r<nrows; r++ ) {

    for ( c=0; c<ncols; c++ ) {

      stat = avl_get_match( this->colorCacheByPosH, (void *) &pos,
       (void **) &cur );
      if ( !( stat & 1 ) ) {
        i = 0;
      }
      else if ( !cur ) {
        i = 0;
      }
      else {
        i = cur->index;
      }

      if ( i == curIndex ) {
        x = c*5 + c*20 + 5;
        y = r*5 + r*20 + 5;
        gc.setFG( BlackPixel( display, DefaultScreen(display) ) );
        XDrawRectangle( display, XtWindow(form), gc.normGC(), x-2, y-2,
         23, 23 );
        curX = x;
        curY = y;
      }

      pos++;

    }

  }

  if ( remainder ) {

    r = nrows;

    for ( c=0; c<remainder; c++ ) {

      stat = avl_get_match( this->colorCacheByPosH, (void *) &pos,
       (void **) &cur );
      if ( !( stat & 1 ) ) {
        i = 0;
      }
      else if ( !cur ) {
        i = 0;
      }
      else {
        i = cur->index;
      }

      if ( i == curIndex ) {
        x = c*5 + c*20 + 5;
        y = r*5 + r*20 + 5;
        gc.setFG( BlackPixel( display, DefaultScreen(display) ) );
        XDrawRectangle( display, XtWindow(form), gc.normGC(), x-2, y-2,
         23, 23 );
        curX = x;
        curY = y;
      }

      pos++;

    }

  }

  return 1;

}

int colorInfoClass::canDiscardPixel (
  unsigned int pixel )
{

int stat;
colorCachePtr cur;

  stat = avl_get_match( this->colorCacheByPixelH, (void *) &pixel,
   (void **) &cur );
  if ( !(stat & 1) ) return 0;

  if ( cur )
    return 0;
  else
    return 1;

}

unsigned int colorInfoClass::getPixelByIndexWithBlink (
  int index )
{

  if ( index >= max_colors+num_blinking_colors )
    //return BlackPixel( display, screen );
    return getSpecialColor( COLORINFO_K_INVALID );

  if ( index < 0 )
    //return WhitePixel( display, screen );
    return getSpecialColor( COLORINFO_K_INVALID );

  if ( blink ) {
    return blinkingColors[index];
  }
  else {
    return colors[index];
  }

}

unsigned int colorInfoClass::getPixelByIndex (
  int index )
{

  if ( index >= max_colors+num_blinking_colors )
    //return BlackPixel( display, screen );
    return getSpecialColor( COLORINFO_K_INVALID );

  if ( index < 0 )
    //return WhitePixel( display, screen );
    return getSpecialColor( COLORINFO_K_INVALID );

  return colors[index];

}

unsigned int colorInfoClass::labelPix ( // return reasonable fg for given bg
  int index )
{

int stat;
colorCachePtr cur;
int sum;

  stat = avl_get_match( this->colorCacheByIndexH, (void *) &index,
   (void **) &cur );
  if ( !(stat & 1) ) return BlackPixel( display, screen );

  if ( cur ) {

    sum = cur->rgb[0] + cur->rgb[1] * 3 + cur->rgb[2];

    if ( sum < 180000 )
      return WhitePixel( display, screen );
    else
      return BlackPixel( display, screen );

  }

  return BlackPixel( display, screen );

}

char *colorInfoClass::colorName (
  int index )
{

  if ( index >= max_colors+num_blinking_colors )
    return colorNames[max_colors-1];

  if ( index < 0 )
    return colorNames[0];

  return colorNames[index];

}

int colorInfoClass::colorIndexByName (
  const char *name )
{

int stat;
colorCachePtr cur;

  stat = avl_get_match( this->colorCacheByNameH, (void *) name,
   (void **) &cur );
  if ( !( stat & 1 ) ) {
    return 0;
  }
  if ( !cur ) {
    return 0;
  }

  return cur->index;

}

int colorInfoClass::colorIndexByAlias (
  const char *alias )
{

int stat;
colorCachePtr cur1, cur2;

  stat = avl_get_match( this->colorCacheByAliasH, (void *) alias,
   (void **) &cur1 );
  if ( !( stat & 1 ) ) {
    return 0;
  }
  if ( !cur1 ) {
    return 0;
  }

  stat = avl_get_match( this->colorCacheByNameH, (void *) cur1->aliasValue,
   (void **) &cur2 );
  if ( !( stat & 1 ) ) {
    return 0;
  }
  if ( !cur2 ) {
    return 0;
  }

  return cur2->index;

}

int colorInfoClass::pixIndex(
  unsigned int pixel )
{

int stat;
colorCachePtr cur;

  stat = avl_get_first( this->colorCacheByIndexH, (void **) &cur );
  if ( !( stat & 1 ) ) {
    return 0;
  }

  while ( cur ) {

    if ( cur->pixel == pixel ) return cur->index;

      stat = avl_get_next( this->colorCacheByIndexH, (void **) &cur );
      if ( !( stat & 1 ) ) {
        return 0;
      }

  }

  return 0;

}

int colorInfoClass::isRule (
  int index )
{

  if ( major < 3 ) return 0;

  if ( index >= max_colors+num_blinking_colors )
    return 0;

  if ( index < 0 )
    return 0;

  return ( colorNodes[index]->rule != NULL );

}

char *colorInfoClass::firstColor (
  colorCachePtr node ) {

int stat;

  stat = avl_get_first( this->colorCacheByIndexH, (void **) &node );
  if ( !( stat & 1 ) ) {
    return NULL;
  }

  if ( node ) {
    return node->name;
  }
  else {
    return NULL;
  }

}

char *colorInfoClass::nextColor (
  colorCachePtr node ) {

int stat;

  stat = avl_get_next( this->colorCacheByIndexH, (void **) &node );
  if ( !( stat & 1 ) ) {
    return NULL;
  }

  if ( node ) {
    return node->name;
  }
  else {
    return NULL;
  }

}

int colorInfoClass::majorVersion ( void ) {

  return major;

}

int colorInfoClass::menuIndex (
  int index )
{

  if ( !menuIndexMap ) return index;

  if ( index < 0 ) return 0;
  if ( index >= maxMenuItems ) return maxMenuItems-1;

  return menuIndexMap[index];

}

int colorInfoClass::menuPosition (
  int index )
{

int i;

  if ( !menuIndexMap ) return index + 1;

  if ( index < 0 ) return 0;

  for ( i=0; i<maxMenuItems; i++ ) {
    if ( index == menuIndexMap[i] ) return i+1;
  }

  return 0;

}

int colorInfoClass::menuSize ( void ) {

  if ( menuIndexMap )
    return maxMenuItems;
  else
    return max_colors+num_blinking_colors;

}

int colorInfoClass::evalRule (
  int index,
  double value )
{

ruleConditionPtr ruleCond;
int opResult, opResult1, opResult2;

  if ( !isRule(index) ) {
    return index;
  }

  colorNodes[index]->rule->needJoin = 0;

  ruleCond = colorNodes[index]->rule->ruleHead->flink;
  while ( ruleCond ) {

    opResult1 = ruleCond->ruleFunc1( value, ruleCond->value1 );

    if ( ruleCond->connectingFunc ) {
      opResult2 = ruleCond->ruleFunc2( value, ruleCond->value2 );
      opResult = ruleCond->connectingFunc( opResult1, opResult2 );
    }
    else {
      opResult = opResult1;
    }

    if ( colorNodes[index]->rule->needJoin ) { // set by prev rule
      opResult = colorNodes[index]->rule->curJoinFunc(
       opResult, colorNodes[index]->rule->combinedOpResult );
    }

    if ( ruleCond->joiningFunc ) { // set by this rule
      colorNodes[index]->rule->curJoinFunc = ruleCond->joiningFunc;
      colorNodes[index]->rule->combinedOpResult = opResult;
      colorNodes[index]->rule->needJoin = 1;
    }
    else {
      colorNodes[index]->rule->needJoin = 0;
      if ( opResult ) {
        return ruleCond->result;
      }
    }

    ruleCond = ruleCond->flink;

  }

  return index;

}

int colorInfoClass::isInvisible(
  int index
) {

  return ( index == invisibleIndex );

}

void colorInfoClass::useIndex ( void ) {

  useIndexFlag = 1;

}

void colorInfoClass::useRGB ( void ) {

  useIndexFlag = 0;

}

int colorInfoClass::colorModeIsRGB ( void ) {

  if ( useIndexFlag ) {
    return 0;
  }
  else {
    return 1;
  }

}

int colorInfoClass::writeColorIndex (
  FILE *f,
  int index
) {

int r, g, b, idx;
unsigned int pixel;

  if ( ( index < 0 ) || ( index >= max_colors+num_blinking_colors ) ) {
    fprintf( stderr, "colorInfoClass::writeColorIndex - bad index encountered [%-d]\n",
     index );
    idx = getSpecialIndex( COLORINFO_K_INVALID );
  }
  else {
    idx = index;
  }

  if ( useIndexFlag ) {

    writeStringToFile( f, "index" );
    fprintf( f, "%-d\n", index );

  }
  else {

    writeStringToFile( f, "rgb" );
    pixel = pix( idx );
    getRGB( pixel, &r, &g, &b );
    fprintf( f, "%-d,%-d,%-d\n", r, g, b );

  }

  return 1;

}

int colorInfoClass::writeColorIndex (
  FILE *f,
  char *tag,
  int index
) {

int r, g, b, idx;
unsigned int pixel;

  if ( ( index < 0 ) || ( index >= max_colors+num_blinking_colors ) ) {
    fprintf( stderr, "colorInfoClass::writeColorIndex - bad index encountered [%-d]\n",
     index );
    idx = getSpecialIndex( COLORINFO_K_INVALID );
  }
  else {
    idx = index;
  }

  if ( useIndexFlag ) {

    fprintf( f, "%s index %-d\n", tag, index );

  }
  else {

    pixel = pix( idx );
    getRGB( pixel, &r, &g, &b );
    fprintf( f, "%s rgb %-d %-d %-d\n", tag, r, g, b );

  }

  return 1;

}

int colorInfoClass::writeColorArrayIndex (
  FILE *f,
  int arrayIndex,
  int index
) {

int r, g, b, idx;
unsigned int pixel;

  if ( ( index < 0 ) || ( index >= max_colors+num_blinking_colors ) ) {
    fprintf( stderr, "colorInfoClass::writeColorArrayIndex - bad index encountered [%-d]\n",
     index );
    idx = getSpecialIndex( COLORINFO_K_INVALID );
  }
  else {
    idx = index;
  }

  if ( useIndexFlag ) {

    fprintf( f, "%-d index %-d\n", arrayIndex, index );

  }
  else {

    pixel = pix( idx );
    getRGB( pixel, &r, &g, &b );
    fprintf( f, "%-d rgb %-d %-d %-d\n", arrayIndex, r, g, b );

  }

  return 1;

}

int colorInfoClass::writeColorArrayIndex (
  FILE *f,
  int index
) {

int r, g, b, idx;
unsigned int pixel;

  if ( ( index < 0 ) || ( index >= max_colors+num_blinking_colors ) ) {
    fprintf( stderr, "colorInfoClass::writeColorArrayIndex - bad index encountered [%-d]\n",
     index );
    idx = getSpecialIndex( COLORINFO_K_INVALID );
  }
  else {
    idx = index;
  }

  if ( useIndexFlag ) {

    fprintf( f, "index %-d\n", index );

  }
  else {

    pixel = pix( idx );
    getRGB( pixel, &r, &g, &b );
    fprintf( f, "rgb %-d %-d %-d\n", r, g, b );

  }

  return 1;

}

int colorInfoClass::readColorIndex (
  FILE *f,
  int *index
) {

int r, g, b;
unsigned int pixel;
char colorMode[10+1];

  readStringFromFile( colorMode, 10, f );
  if ( strcmp( colorMode, "rgb" ) != 0 ) {

    fscanf( f, "%d\n", index );

  }
  else {

    fscanf( f, "%d,%d,%d\n", &r, &g, &b );
    setRGB( r, g, b, &pixel );
    *index = -1;
    *index = pixIndex( pixel );

  }

  if ( ( *index < 0 ) || ( *index >= max_colors+num_blinking_colors ) ) {
    fprintf( stderr, "colorInfoClass::readColorIndex - bad index encountered [%-d]\n",
     *index );
    //*index = getSpecialIndex( COLORINFO_K_INVALID );
  }

  return 1;

}

void colorInfoClass::usePrivColorMap ( void ) {

  usePrivColorMapFlag = 1;

}

int colorInfoClass::blinking (
  int index
) {

  if ( ( index < 0 ) || ( index >= max_colors+num_blinking_colors ) ) {
    return 0;
  }

  return (int) ( colors[index] != blinkingColors[index] );

}

int colorInfoClass::addToBlinkList (
  void *obj,
  void *func
) {

blinkNodePtr cur;

  if ( major < 3 ) {
    return 1;
  }

  //fprintf( stderr, "colorInfoClass::addToBlinkList - obj = %-X\n", (int) obj );

  if ( blinkLookasideHead->next ) {
    cur = blinkLookasideHead->next;
    blinkLookasideHead->next = cur->next;
    if ( !blinkLookasideHead->next ) {
      blinkLookasideTail = blinkLookasideHead;
    }
  }
  else {
    cur = new blinkNodeType;
  }

  cur->func = func;
  cur->obj = obj;
  cur->op = 1;

  addBlinkTail->next = cur;
  addBlinkTail = cur;
  addBlinkTail->next = NULL;

  //stat = avl_insert_node( this->blinkH, (void *) cur, &dup );
  //if ( !( stat & 1 ) ) {
  //  fprintf( stderr, colorInfoClass_str36, stat );
  //}

  return 1;

}

int colorInfoClass::removeFromBlinkList (
  void *obj,
  void *func
) {

blinkNodePtr cur;

  if ( major < 3 ) {
    return 1;
  }

  //fprintf( stderr, "colorInfoClass::removeFromBlinkList - obj = %-X\n", (int) obj );

  if ( blinkLookasideHead->next ) {
    cur = blinkLookasideHead->next;
    blinkLookasideHead->next = cur->next;
    if ( !blinkLookasideHead->next ) {
      blinkLookasideTail = blinkLookasideHead;
    }
  }
  else {
    cur = new blinkNodeType;
  }

  cur->func = func;
  cur->obj = obj;
  cur->op = 2;

  addBlinkTail->next = cur;
  addBlinkTail = cur;
  addBlinkTail->next = NULL;

  //stat = avl_get_match( blinkH, obj, (void **) &cur );
  //if ( !( stat & 1 ) ) {
  //  fprintf( stderr, colorInfoClass_str37, stat );
  //}
  //if ( !cur ) {
  //  fprintf( stderr, colorInfoClass_str38 );
  //  return 0;
  //}

  //stat = avl_delete_node( blinkH, (void **) &cur );
  //if ( !( stat & 1 ) ) {
  //  fprintf( stderr, colorInfoClass_str39, stat );
  //}

  //blinkLookasideTail->next = cur;
  //blinkLookasideTail = cur;
  //blinkLookasideTail->next = NULL;

  return 1;

}

int colorInfoClass::addAllToBlinkList ( void ) {

int stat, dup;
blinkNodePtr cur, next, prev, curBlinkNode;

  if ( major < 3 ) {
    return 1;
  }

  prev = addBlinkHead;
  cur = addBlinkHead->next;
  while ( cur ) {

    next = cur->next;

    if ( cur->op == 1 ) {

      // remove cur from list
      prev->next = next;
      cur->next = NULL; // diagnostic

      //fprintf( stderr, "colorInfoClass::addAllToBlinkList - insert\n" );

      stat = avl_insert_node( this->blinkH, (void *) cur, &dup );
      if ( !( stat & 1 ) ) {
        fprintf( stderr, colorInfoClass_str36, stat );
        blinkLookasideTail->next = cur;
        blinkLookasideTail = cur;
        blinkLookasideTail->next = NULL;
      }
      else if ( dup ) {
        fprintf( stderr, "dup\n" );
        blinkLookasideTail->next = cur;
        blinkLookasideTail = cur;
        blinkLookasideTail->next = NULL;
      }

    }
    else if ( cur->op == 2 ) {

      //fprintf( stderr, "colorInfoClass::removeAllFromBlinkList - find\n" );

      stat = avl_get_match( blinkH, cur->obj, (void **) &curBlinkNode );
      if ( !( stat & 1 ) ) {
        fprintf( stderr, colorInfoClass_str37, stat );
      }
      else if ( !curBlinkNode ) {
        fprintf( stderr, colorInfoClass_str38 );
      }
      else {

        //fprintf( stderr, "colorInfoClass::removeAllFromBlinkList - delete\n" );

        stat = avl_delete_node( blinkH, (void **) &curBlinkNode );
        if ( !( stat & 1 ) ) {
          fprintf( stderr, colorInfoClass_str39, stat );
        }

        blinkLookasideTail->next = curBlinkNode;
        blinkLookasideTail = curBlinkNode;
        blinkLookasideTail->next = NULL;

      }

      prev = cur;

    }

    cur = next;

  }

  // remove all "remove from blink list request" nodes from list
  cur = addBlinkHead->next;
  while ( cur ) {

    next = cur->next;

    if ( cur->op == 2 ) { // sanity check

      blinkLookasideTail->next = cur;
      blinkLookasideTail = cur;
      blinkLookasideTail->next = NULL;

    }
    else {

      fprintf( stderr, "Bad request node\n" );

    }

    cur = next;

  }

  // make add list empty
  addBlinkHead->next = NULL;
  addBlinkTail = addBlinkHead;
  addBlinkTail->next = NULL;

  return 1;

}

int colorInfoClass::removeAllFromBlinkList ( void ) {

int stat;
blinkNodePtr cur, next, curBlinkNode;

  return 1;

  if ( major < 3 ) {
    return 1;
  }

  // remove all blink nodes from tree
  cur = remBlinkHead->next;
  while ( cur ) {

    next = cur->next;

    fprintf( stderr, "colorInfoClass::removeAllFromBlinkList - find\n" );

    stat = avl_get_match( blinkH, cur->obj, (void **) &curBlinkNode );
    if ( !( stat & 1 ) ) {
      fprintf( stderr, colorInfoClass_str37, stat );
    }
    else if ( !curBlinkNode ) {
      fprintf( stderr, colorInfoClass_str38 );
    }
    else {

      fprintf( stderr, "colorInfoClass::removeAllFromBlinkList - delete\n" );

      stat = avl_delete_node( blinkH, (void **) &curBlinkNode );
      if ( !( stat & 1 ) ) {
        fprintf( stderr, colorInfoClass_str39, stat );
      }

      blinkLookasideTail->next = curBlinkNode;
      blinkLookasideTail = curBlinkNode;
      blinkLookasideTail->next = NULL;

    }

    cur = next;

  }

  // remove all nodes from list
  cur = remBlinkHead->next;
  while ( cur ) {

    next = cur->next;

    blinkLookasideTail->next = cur;
    blinkLookasideTail = cur;
    blinkLookasideTail->next = NULL;

    cur = next;

  }

  // make rem list empty
  remBlinkHead->next = NULL;
  remBlinkTail = remBlinkHead;
  remBlinkTail->next = NULL;

  return 1;

}

void colorInfoClass::warnIfBadIndex (
  int index,
  int line
) {

  if ( ( index < 0 ) || ( index >= max_colors+num_blinking_colors ) ) {
    fprintf( stderr, "Bad color index encountered [%-d] near line %-d\n",
     index, line );
  }

}

int colorInfoClass::shouldShowNoAlarmState ( void ) {

  return showNoAlarmState;

}
