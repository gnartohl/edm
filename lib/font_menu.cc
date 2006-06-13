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

#include "font_menu.h"

#include "thread.h"

void Bold_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmToggleButtonCallbackStruct *cb;
fontMenuClass *fmp;

  cb = (XmToggleButtonCallbackStruct *) call;
  fmp = (fontMenuClass *) client;

  fmp->change = 1;

  if ( fmp->bold ) {
    strcpy( fmp->boldStr, "medium" );
    fmp->bold = 0;
  }
  else {
    strcpy( fmp->boldStr, "bold" );
    fmp->bold = 1;
  }

}

void Italics_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmToggleButtonCallbackStruct *cb;
fontMenuClass *fmp;

  cb = (XmToggleButtonCallbackStruct *) call;
  fmp = (fontMenuClass *) client;

  fmp->change = 1;

  if ( fmp->italics ) {
    strcpy( fmp->italicsStr, "r" );
    fmp->italics = 0;
  }
  else {
    strcpy( fmp->italicsStr, "i" );
    fmp->italics = 1;
  }

}

void setAlign_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmPushButtonCallbackStruct *cb;
alignOptionListPtr aop;

  cb = (XmPushButtonCallbackStruct *) call;
  aop = (alignOptionListPtr) client;

  aop->fmp->change = 1;

  aop->fmp->alignStr = aop->alignString;
  aop->fmp->align = aop->align;

}

void setFamily_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmPushButtonCallbackStruct *cb;
familyOptionListPtr fop;

Arg args[10];
int n;

  cb = (XmPushButtonCallbackStruct *) call;
  fop = (familyOptionListPtr) client;

  fop->fmp->change = 1;

//   fprintf( stderr, "name = %s\n", fop->name );

  n = 0;
  XtSetArg( args[n], XmNsubMenuId, (XtArgVal) fop->sizePullDown ); n++;
  XtSetArg( args[n], XmNmenuHistory, (XtArgVal) fop->sizeHistory ); n++;
  XtSetValues( fop->fmp->sizeOption, args, n );

  fop->fmp->familyStr = fop->name;
  fop->fmp->sizeStr = fop->curSizeString;

}

void setSize_cb (
  Widget w,
  XtPointer client,
  XtPointer call )
{

XmPushButtonCallbackStruct *cb;
sizeMenuListPtr smlp;
familyOptionListPtr fop;

  cb = (XmPushButtonCallbackStruct *) call;
  smlp = (sizeMenuListPtr) client;
  fop = (familyOptionListPtr) smlp->vfop;

  smlp->fmp->change = 1;

  fop->sizeHistory = w;
  fop->curSizeString = smlp->sizeString;

  smlp->fmp->sizeStr = smlp->sizeString;

}

fontMenuClass::fontMenuClass ( void ) {

  change = 1;

  familyHead = new familyOptionListType;
  familyTail = familyHead;
  familyTail->flink = NULL;

  alignHead = new alignOptionListType;
  alignTail = alignHead;
  alignTail->flink = NULL;

  alignStr = NULL;
  align = 0;
  sizeStr = NULL;
  familyStr = NULL;
  bold = 0;
  strcpy( boldStr, "medium" );
  italics = 0;
  strcpy( italicsStr, "r" );

}

fontMenuClass::~fontMenuClass ( void ) {

//    fprintf( stderr, "In fontMenuClass::~fontMenuClass\n" );

  if ( familyHead && alignHead ) {
    if ( familyHead->flink && alignHead->flink ) destroyFontMenu();
  }

  if ( familyHead ) {
    delete familyHead;
    familyHead = NULL;
  }

  if ( alignHead ) {
    delete alignHead;
    alignHead = NULL;
  }

}

void fontMenuClass::copy ( const fontMenuClass &source ) {

  change = 1;

  familyHead = new familyOptionListType;
  familyTail = familyHead;
  familyTail->flink = NULL;

  alignHead = new alignOptionListType;
  alignTail = alignHead;
  alignTail->flink = NULL;

  alignStr = NULL;
  align = 0;
  sizeStr = NULL;
  familyStr = NULL;
  bold = 0;
  strcpy( boldStr, "medium" );
  italics = 0;
  strcpy( italicsStr, "r" );

}

int fontMenuClass::destroyFontMenu ( void )
{

familyOptionListPtr curFamilyOption, nextFamilyOption;
sizeMenuListPtr curSizePB, nextSizePB;
alignOptionListPtr curAlign, nextAlign;

//    fprintf( stderr, "In fontMenuClass::destroyFontMenu\n" );

  // delete all family option button data

  if ( familyHead ) {

  curFamilyOption = familyHead->flink;
  while ( curFamilyOption ) {

    nextFamilyOption = curFamilyOption->flink;

// fprintf( stderr, "[%s]\n", curFamilyOption->name );
    delete[] curFamilyOption->name;

    // ==============================================
    // delete all size pushbutton data

    curSizePB = curFamilyOption->head->flink;
    while ( curSizePB ) {

      nextSizePB = curSizePB->flink;

// fprintf( stderr, "[%s]\n", curSizePB->sizeString );
      delete[] curSizePB->sizeString;

      delete curSizePB;

      curSizePB = nextSizePB;

    }

    // delete size sentinel node
// fprintf( stderr, "delete size sentinel node\n" );
    delete curFamilyOption->head;

    // ==============================================

// fprintf( stderr, "delete curFamilyOption\n" );
    delete curFamilyOption;

    curFamilyOption = nextFamilyOption;

  }

// fprintf( stderr, "set familyTail, familyHead empty\n" );
  familyTail = familyHead;
  familyTail->flink = NULL;

  }

  // delete all alignment data

  if ( alignHead ) {

  curAlign = alignHead->flink;
  while ( curAlign ) {

    nextAlign = curAlign->flink;

// fprintf( stderr, "[%s]\n", curAlign->alignString );
    delete[] curAlign->alignString;

// fprintf( stderr, "delete curAlign\n" );
    delete curAlign;

    curAlign = nextAlign;

  }

// fprintf( stderr, "set alignTail, alignHead empty\n" );
  alignTail = alignHead;
  alignTail->flink = NULL;

  }

  return 1;

}

Widget fontMenuClass::createFontMenu (
  Widget parent,
  fontInfoClass *fi,
  Arg args[],
  int numArgs,
  int includeAlignInfo )
{

familyListPtr curFamily;
sizeListPtr curSize;
familyOptionListPtr curFamilyOption;
sizeMenuListPtr curSizePB;
alignOptionListPtr curAlign;
char string[31+1];
XmString str;
Arg locArgs[10];
int n;
Widget firstFamilyPb = NULL;
Widget firstSizePulldown = NULL;
Widget firstSizePb = NULL;


  form = XtCreateManagedWidget( "fontmenuform", xmFormWidgetClass, parent,
   args, numArgs );

  familyPullDown = XmCreatePulldownMenu( form, "familymenu", NULL, 0 );


  // for each family, create family pushbutton and size pulldown
  curFamily = fi->familyHead->flink;
  while ( curFamily ) {

//     fprintf( stderr, "Family: %s\n", curFamily->name );

    curFamilyOption = new familyOptionListType;

    curFamilyOption->fmp = this;

    // copy name
    curFamilyOption->name = new char[strlen(curFamily->name)+1];
    strcpy( curFamilyOption->name, curFamily->name );

    // creat pushbutton
    str = XmStringCreateLocalized( curFamily->name );
    curFamilyOption->familyPb = XtVaCreateManagedWidget( "pb",
     xmPushButtonWidgetClass, familyPullDown,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );

    if ( !firstFamilyPb ) {
      firstFamilyPb = curFamilyOption->familyPb;
      familyStr = curFamilyOption->name;
    }

    XtAddCallback( curFamilyOption->familyPb, XmNactivateCallback,
     setFamily_cb, (XtPointer) curFamilyOption );


    // create pulldown
    curFamilyOption->sizePullDown = XmCreatePulldownMenu( form, "familymenu", NULL, 0 );

    if ( !firstSizePulldown )
     firstSizePulldown = curFamilyOption->sizePullDown;

    // create sizeMenuListPtr sentinel node
    curFamilyOption->head = new sizeMenuListType;
    curFamilyOption->tail = curFamilyOption->head;
    curFamilyOption->tail->flink = NULL;

    curFamilyOption->sizeHistory = NULL;
    curFamilyOption->curSizeString = NULL;

    // for each size, create pushbutton
    curSize = curFamily->sizeHead->flink;
    while ( curSize ) {

//       fprintf( stderr, "size: %-.1f\n", curSize->fsize );
      sprintf( string, "%-.1f", curSize->fsize );

      curSizePB = new sizeMenuListType;

      curSizePB->fmp = this;
      curSizePB->vfop = (void *) curFamilyOption;

      curSizePB->sizeString = new char[strlen(string)+1];
      strcpy( curSizePB->sizeString, string );

      curSizePB->size = curSize->size;
      curSizePB->fsize = curSize->fsize;


      str = XmStringCreateLocalized( string );
      curSizePB->pb = XtVaCreateManagedWidget( "pb", xmPushButtonWidgetClass,
       curFamilyOption->sizePullDown,
       XmNlabelString, str,
       NULL );
      XmStringFree( str );


      if ( !curFamilyOption->sizeHistory ) {
        curFamilyOption->sizeHistory = curSizePB->pb;
        curFamilyOption->curSizeString = curSizePB->sizeString;
      }

      if ( !firstSizePb ) {
        firstSizePb = curSizePB->pb;
        sizeStr = curSizePB->sizeString;
      }


      XtAddCallback( curSizePB->pb, XmNactivateCallback,
       setSize_cb, (XtPointer) curSizePB );


      // link it in
      curFamilyOption->tail->flink = curSizePB;
      curFamilyOption->tail = curSizePB;
      curFamilyOption->tail->flink = NULL;

      curSize = curSize->flink;

    }

    // link it in
    familyTail->flink = curFamilyOption;
    familyTail = curFamilyOption;
    familyTail->flink = NULL;

//     fprintf( stderr, "\n" );
    curFamily = curFamily->flink;

  }

  if ( includeAlignInfo ) {

    // Alignment

    alignPullDown = XmCreatePulldownMenu( form, "alignmenu", NULL, 0 );

    curAlign = new alignOptionListType;

    curAlign->fmp = this;
    curAlign->alignString = new char[2];
    strcpy( curAlign->alignString, "L" );
    curAlign->align = (int) XmALIGNMENT_BEGINNING;

    str = XmStringCreateLocalized( curAlign->alignString );
    curAlign->pb = XtVaCreateManagedWidget( "pb",
     xmPushButtonWidgetClass, alignPullDown,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );

    XtAddCallback( curAlign->pb, XmNactivateCallback,
     setAlign_cb, (XtPointer) curAlign );


    alignTail->flink = curAlign;
    alignTail = curAlign;
    alignTail->flink = NULL;

    this->alignStr = curAlign->alignString;
    this->align = curAlign->align;

    curAlign = new alignOptionListType;

    curAlign->fmp = this;
    curAlign->alignString = new char[2];
    strcpy( curAlign->alignString, "C" );
    curAlign->align = (int) XmALIGNMENT_CENTER;


    str = XmStringCreateLocalized( curAlign->alignString );
    curAlign->pb = XtVaCreateManagedWidget( "pb",
     xmPushButtonWidgetClass, alignPullDown,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );

    XtAddCallback( curAlign->pb, XmNactivateCallback,
     setAlign_cb, (XtPointer) curAlign );


    alignTail->flink = curAlign;
    alignTail = curAlign;
    alignTail->flink = NULL;

    curAlign = new alignOptionListType;
    curAlign->fmp = this;
    curAlign->alignString = new char[2];
    strcpy( curAlign->alignString, "R" );
    curAlign->align = (int) XmALIGNMENT_END;


    str = XmStringCreateLocalized( curAlign->alignString );
    curAlign->pb = XtVaCreateManagedWidget( "pb",
     xmPushButtonWidgetClass, alignPullDown,
     XmNlabelString, str,
     NULL );
    XmStringFree( str );

    XtAddCallback( curAlign->pb, XmNactivateCallback,
     setAlign_cb, (XtPointer) curAlign );


    alignTail->flink = curAlign;
    alignTail = curAlign;
    alignTail->flink = NULL;

  }

  // create option menus & toggles


  n = 0;
  XtSetArg( locArgs[n], XmNsubMenuId, (XtArgVal) familyPullDown ); n++;
  XtSetArg( locArgs[n], XmNmenuHistory, (XtArgVal) firstFamilyPb ); n++;
  XtSetArg( locArgs[n], XmNtopAttachment, (XtArgVal) XmATTACH_FORM ); n++;
  XtSetArg( locArgs[n], XmNleftAttachment, (XtArgVal) XmATTACH_FORM ); n++;
  familyOption = XmCreateOptionMenu( form, "familyoptionmenu", locArgs, n );

  XtManageChild( familyOption );



  n = 0;
  XtSetArg( locArgs[n], XmNsubMenuId, (XtArgVal) firstSizePulldown ); n++;
  XtSetArg( locArgs[n], XmNmenuHistory, (XtArgVal) firstSizePb ); n++;
  XtSetArg( locArgs[n], XmNtopAttachment,
   (XtArgVal) XmATTACH_WIDGET ); n++;
  XtSetArg( locArgs[n], XmNtopWidget, (XtArgVal) familyOption ); n++;
  XtSetArg( locArgs[n], XmNleftAttachment,
   (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
  XtSetArg( locArgs[n], XmNleftWidget, (XtArgVal) familyOption ); n++;
  sizeOption = XmCreateOptionMenu( form, "sizeoptionmenu", locArgs, n );

  XtManageChild( sizeOption );



  // bold togglebutton

  strcpy( boldStr, "medium" );
  bold = 0;

  n = 0;
  str = XmStringCreateLocalized( "B" );
  XtSetArg( locArgs[n], XmNlabelString, (XtArgVal) str ); n++;
  XtSetArg( locArgs[n], XmNset, (XtArgVal) False ); n++;
  XtSetArg( locArgs[n], XmNtopAttachment,
   (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
  XtSetArg( locArgs[n], XmNtopWidget, (XtArgVal) sizeOption ); n++;
  XtSetArg( locArgs[n], XmNleftAttachment,
   (XtArgVal) XmATTACH_WIDGET ); n++;
  XtSetArg( locArgs[n], XmNleftWidget, (XtArgVal) sizeOption ); n++;
  boldToggle =   XtCreateManagedWidget( "boldtoggle", xmToggleButtonWidgetClass,
   form, locArgs, n );
  XmStringFree( str );

  XtAddCallback( boldToggle, XmNarmCallback, Bold_cb,
   (XtPointer) this );


  // italics togglebutton

  strcpy( italicsStr, "r" );
  italics = 0;


  n = 0;
  str = XmStringCreateLocalized( "I" );
  XtSetArg( locArgs[n], XmNlabelString, (XtArgVal) str ); n++;
  XtSetArg( locArgs[n], XmNset, (XtArgVal) False ); n++;
  XtSetArg( locArgs[n], XmNtopAttachment,
   (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
  XtSetArg( locArgs[n], XmNtopWidget, (XtArgVal) sizeOption ); n++;
  XtSetArg( locArgs[n], XmNleftAttachment,
   (XtArgVal) XmATTACH_WIDGET ); n++;
  XtSetArg( locArgs[n], XmNleftWidget, (XtArgVal) boldToggle ); n++;
  italicsToggle =   XtCreateManagedWidget( "italicstoggle", xmToggleButtonWidgetClass,
   form, locArgs, n );
  XmStringFree( str );

  XtAddCallback( italicsToggle, XmNarmCallback, Italics_cb,
   (XtPointer) this );

  XtManageChild( italicsToggle );


  if ( includeAlignInfo ) {

    n = 0;
    XtSetArg( locArgs[n], XmNsubMenuId, (XtArgVal) alignPullDown ); n++;
    XtSetArg( locArgs[n], XmNmenuHistory, (XtArgVal) alignHead->flink->pb );
     n++;
    XtSetArg( locArgs[n], XmNtopAttachment,
     (XtArgVal) XmATTACH_OPPOSITE_WIDGET ); n++;
    XtSetArg( locArgs[n], XmNtopWidget, (XtArgVal) sizeOption ); n++;
    XtSetArg( locArgs[n], XmNleftAttachment,
     (XtArgVal) XmATTACH_WIDGET ); n++;
    XtSetArg( locArgs[n], XmNleftWidget, (XtArgVal) italicsToggle ); n++;
    alignOption = XmCreateOptionMenu( form, "alignoptionmenu", locArgs, n );

    XtManageChild( alignOption );

  }

  return form;

}

Widget fontMenuClass::createFontMenu (
  Widget parent,
  fontInfoClass *fi,
  Arg args[],
  int numArgs ) {

  return createFontMenu( parent, fi, args, numArgs, 0 );

}

void fontMenuClass::show( void ) {

  fprintf( stderr, "%s-%s-%s-%s\n", this->familyStr, this->boldStr,
   this->italicsStr, this->sizeStr );

}

char *fontMenuClass::currentFontTag ( void ) {

  strncpy( fontTagStr, this->familyStr, 127 );
  Strncat( fontTagStr, "-", 127 );
  Strncat( fontTagStr, this->boldStr, 127 );
  Strncat( fontTagStr, "-", 127 );
  Strncat( fontTagStr, this->italicsStr, 127 );
  Strncat( fontTagStr, "-", 127 );
  Strncat( fontTagStr, this->sizeStr, 127 );

  return fontTagStr;

}

int fontMenuClass::setFontTag (
  char *string )
{

char buf[127+1], familyVal[127+1], boldVal[7+1], italicsVal[3+1],
 sizeVal[7+1], *tk;
familyOptionListPtr cur;
sizeMenuListPtr curSize;
Arg args[5];
int n;
int familyOK, sizeOK;

  strncpy( buf, string, 127 );

  tk = strtok( buf, "-" );
  if ( !tk ) return FONTMENU_FAIL;
  strncpy( familyVal, tk, 127 );

  tk = strtok( NULL, "-" );
  if ( !tk ) return FONTMENU_FAIL;
  strncpy( boldVal, tk, 7 );

  tk = strtok( NULL, "-" );
  if ( !tk ) return FONTMENU_FAIL;
  strncpy( italicsVal, tk, 3 );

  tk = strtok( NULL, "-" );
  if ( !tk ) return FONTMENU_FAIL;
  strncpy( sizeVal, tk, 7 );

  // family

  familyOK = 0;
  cur = familyHead->flink;
  while ( cur ) {

    if ( strcmp( familyVal, cur->name ) == 0 ) {


      n = 0;
      XtSetArg( args[n], XmNmenuHistory, (XtArgVal) cur->familyPb ); n++;
      XtSetValues( familyOption, args, n );

      n = 0;
      XtSetArg( args[n], XmNsubMenuId, (XtArgVal) cur->sizePullDown ); n++;
      XtSetArg( args[n], XmNmenuHistory, (XtArgVal) cur->sizeHistory ); n++;
      XtSetValues( sizeOption, args, n );


      familyStr = cur->name;
      sizeStr = cur->curSizeString;

      familyOK = 1;
      break;

    }

    cur = cur->flink;

  }

  if ( !familyOK ) return FONTMENU_FAIL;

  // bold

  if ( strcmp( boldVal, "medium" ) == 0 ) {

    bold = 0;
    strcpy( boldStr, "medium" );
    XmToggleButtonSetState( boldToggle, False, True );

  }
  else if ( strcmp( boldVal, "bold" ) == 0 ) {

    bold = 1;
    strcpy( boldStr, "bold" );
    XmToggleButtonSetState( boldToggle, True, True );

  }
  else {

    return FONTMENU_FAIL;

  }

  // italics

  if ( strcmp( italicsVal, "r" ) == 0 ) {

    italics = 0;
    strcpy( italicsStr, "r" );
    XmToggleButtonSetState( italicsToggle, False, True );

  }
  else if ( strcmp( italicsVal, "i" ) == 0 ) {

    italics = 1;
    strcpy( italicsStr, "i" );
    XmToggleButtonSetState( italicsToggle, True, True );

  }
  else {

    return FONTMENU_FAIL;

  }

  // size    ( cur points to the current familyOption node )

  sizeOK = 0;
  curSize = cur->head->flink;
  while ( curSize ) {

    if ( strcmp( curSize->sizeString, sizeVal ) == 0 ) {

      sizeStr = curSize->sizeString;
      cur->curSizeString = curSize->sizeString;
      cur->sizeHistory = curSize->pb;

      n = 0;
      XtSetArg( args[n], XmNmenuHistory, (XtArgVal) curSize->pb ); n++;
      XtSetValues( sizeOption, args, n );

      sizeOK = 1;
      break;

    }

    curSize = curSize->flink;

  }

  if ( !sizeOK ) return FONTMENU_FAIL;

  return FONTMENU_SUCCESS;

}

int fontMenuClass::setFontAlignment (
 int _align )
{

alignOptionListPtr curAlign;
int n;
Arg locArgs[3];

  curAlign = alignHead->flink;
  while ( curAlign ) {

    if ( curAlign->align == _align ) {

      align=curAlign->align;
      alignStr = curAlign->alignString;

      n = 0;
      XtSetArg( locArgs[n], XmNmenuHistory, (XtArgVal) curAlign->pb ); n++;
      XtSetValues( alignOption, locArgs, n );

      return 1;

    }

    curAlign = curAlign->flink;

  }

  return 0;

}
