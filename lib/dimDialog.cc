#define __dimDialog_cc 1

#include "dimDialog.h"
#include "dimDialog.str"

static void kill_cb (
  Widget w,
  XtPointer client,
  XtPointer call ) {

dimDialogClass *ptr = (dimDialogClass *) client;
activeWindowClass *awoptr = ptr->awo;

  awoptr->viewDims = 0;
  ptr->popdown();

}

dimDialogClass::dimDialogClass () {

  isPoppedUp = widgetsCreated = 0;
  awo = NULL;

}

dimDialogClass::~dimDialogClass( void ) {

}

int dimDialogClass::destroy ( void ) {

  if ( isPoppedUp ) {
    popdown();
  }

  XtUnmanageChild( shell );

  XtDestroyWidget( sep1 );
  XtDestroyWidget( sep2 );
  XtDestroyWidget( xLabel );
  XtDestroyWidget( xValue );
  XtDestroyWidget( yLabel );
  XtDestroyWidget( yValue );
  XtDestroyWidget( lenLabel );
  XtDestroyWidget( lenValue );
  XtDestroyWidget( angleLabel );
  XtDestroyWidget( angleValue );
  XtDestroyWidget( relAngleLabel );
  XtDestroyWidget( relAngleValue );
  XtDestroyWidget( objXLabel );
  XtDestroyWidget( objXValue );
  XtDestroyWidget( objYLabel );
  XtDestroyWidget( objYValue );
  XtDestroyWidget( objWLabel );
  XtDestroyWidget( objWValue );
  XtDestroyWidget( objHLabel );
  XtDestroyWidget( objHValue );
  XtDestroyWidget( objTopDistLabel );
  XtDestroyWidget( objTopDistValue );
  XtDestroyWidget( objBotDistLabel );
  XtDestroyWidget( objBotDistValue );
  XtDestroyWidget( objLeftDistLabel );
  XtDestroyWidget( objLeftDistValue );
  XtDestroyWidget( objRightDistLabel );
  XtDestroyWidget( objRightDistValue );
  XtDestroyWidget( topForm );
  XtDestroyWidget( shell );
  widgetsCreated = 0;

  return 1;

}

int dimDialogClass::create (
  activeWindowClass *ptr
) {

XmString str;
char buf[7+1];
Atom wm_delete_window;
Dimension formw, formh;

  if ( widgetsCreated ) return 1;
  widgetsCreated = 1;

  awo = ptr;

  shell = XtVaCreatePopupShell( "Dimensions", xmDialogShellWidgetClass,
   awo->top,
   XmNmappedWhenManaged, False,
   NULL );

  topForm = XtVaCreateWidget( "topform", xmFormWidgetClass, shell,
   XmNallowResize, True,
   XmNmarginHeight, 20,
   XmNmarginWidth, 20,
   XmNhorizontalSpacing, 5,
   XmNverticalSpacing, 15,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str1 );
  xLabel = XtVaCreateManagedWidget( "xlabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   NULL );

  strcpy( buf, "" );

  xValue = XtVaCreateManagedWidget( "xvalue", xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, xLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, xLabel,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str2 );
  yLabel = XtVaCreateManagedWidget( "ylabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, xLabel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  yValue = XtVaCreateManagedWidget( "yvalue", xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, yLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, yLabel,
   NULL );

  sep1 = XtVaCreateManagedWidget( "sep1", xmSeparatorWidgetClass,
   topForm,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, yLabel,
   XmNrightAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str3 );
  lenLabel = XtVaCreateManagedWidget( "lenlabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, sep1,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  lenValue = XtVaCreateManagedWidget( "lenvalue", xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, lenLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, lenLabel,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str4 );
  angleLabel = XtVaCreateManagedWidget( "anglelabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, lenLabel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  angleValue = XtVaCreateManagedWidget( "anglevalue", xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, angleLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, angleLabel,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str5 );
  relAngleLabel = XtVaCreateManagedWidget( "relanglelabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, angleLabel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  relAngleValue = XtVaCreateManagedWidget( "relanglevalue",
   xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, relAngleLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, relAngleLabel,
   NULL );

  sep2 = XtVaCreateManagedWidget( "sep2", xmSeparatorWidgetClass,
   topForm,
   XmNmarginTop, 7,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, relAngleLabel,
   XmNrightAttachment, XmATTACH_FORM,
   XmNleftAttachment, XmATTACH_FORM,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str6 );
  objXLabel = XtVaCreateManagedWidget( "objxlabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, sep2,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  objXValue = XtVaCreateManagedWidget( "objxvalue",
   xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, objXLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, objXLabel,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str7 );
  objYLabel = XtVaCreateManagedWidget( "objylabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, objXLabel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  objYValue = XtVaCreateManagedWidget( "objyvalue",
   xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, objYLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, objYLabel,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str8 );
  objWLabel = XtVaCreateManagedWidget( "objwlabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, objYLabel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  objWValue = XtVaCreateManagedWidget( "objwvalue",
   xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, objWLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, objWLabel,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str9 );
  objHLabel = XtVaCreateManagedWidget( "objhlabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, objWLabel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  objHValue = XtVaCreateManagedWidget( "objhvalue",
   xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, objHLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, objHLabel,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str10 );
  objTopDistLabel = XtVaCreateManagedWidget( "topdistlabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, objHLabel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  objTopDistValue = XtVaCreateManagedWidget( "topdistvalue",
   xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, objTopDistLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, objTopDistLabel,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str11 );
  objBotDistLabel = XtVaCreateManagedWidget( "botdistlabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, objTopDistLabel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  objBotDistValue = XtVaCreateManagedWidget( "botdistvalue",
   xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, objBotDistLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, objBotDistLabel,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str12 );
  objLeftDistLabel = XtVaCreateManagedWidget( "leftdistlabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, objBotDistLabel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  objLeftDistValue = XtVaCreateManagedWidget( "leftdistvalue",
   xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, objLeftDistLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, objLeftDistLabel,
   NULL );

  str = XmStringCreateLocalized( dimDialog_str13 );
  objRightDistLabel = XtVaCreateManagedWidget( "rightdistlabel", xmLabelWidgetClass,
   topForm,
   XmNlabelString, str,
   XmNtopAttachment, XmATTACH_WIDGET,
   XmNtopWidget, objLeftDistLabel,
   XmNrightAttachment, XmATTACH_WIDGET,
   XmNrightWidget, xValue,
   NULL );

  strcpy( buf, "" );

  objRightDistValue = XtVaCreateManagedWidget( "rightdistvalue",
   xmTextFieldWidgetClass,
   topForm,
   XmNcolumns, (short) 8,
   XmNvalue, buf,
   XmNmaxLength, (short) 10,
   XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
   XmNtopWidget, objRightDistLabel,
   XmNtopOffset, -8,
   XmNleftAttachment, XmATTACH_WIDGET,
   XmNleftWidget, objRightDistLabel,
   NULL );

  wm_delete_window = XmInternAtom( XtDisplay(awo->top),
   "WM_DELETE_WINDOW", False );

  XmAddWMProtocolCallback( shell, wm_delete_window,
   kill_cb, this );

  XtVaSetValues( shell, XmNdeleteResponse,
   XmDO_NOTHING, NULL );

  XtRealizeWidget( topForm );
  XtManageChild( topForm );

  XtVaGetValues( topForm,
   XmNwidth, &formw,
   XmNheight, &formh,
   NULL );

  XtUnmanageChild( topForm );

  formw += 15;
  formh += 15;
  XtVaSetValues( topForm,
   XmNwidth, formw,
   XmNheight, formh,
   NULL );

  XtVaGetValues( topForm,
   XmNwidth, &formw,
   NULL );

  XtManageChild( topForm );

  return 1;

}

int dimDialogClass::popup ( void ) {

  if ( !widgetsCreated ) return 1;
  if ( isPoppedUp ) return 1;

  XtPopup( shell, XtGrabNone );
  isPoppedUp = 1;

  return 1;

}

int dimDialogClass::popdown ( void ) {

  if ( !widgetsCreated ) return 1;
  if ( !isPoppedUp ) return 1;

  XtPopdown( shell );
  isPoppedUp = 0;

  return 1;

}

int dimDialogClass::dialogIsPoppedUp ( void ) {

  return isPoppedUp;

}

int dimDialogClass::setX (
  int x
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-d", x );

  XtVaSetValues( xValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setY (
  int y
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-d", y );

  XtVaSetValues( yValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setLen (
  double len
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-.2f", len );

  XtVaSetValues( lenValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setAngle (
  double angle
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-.2f", angle );

  XtVaSetValues( angleValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setRelAngle (
  double relAngle
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-.2f", relAngle );

  XtVaSetValues( relAngleValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setObjX (
  int x
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-d", x );

  XtVaSetValues( objXValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setObjY (
  int y
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-d", y );

  XtVaSetValues( objYValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setObjW (
  int w
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-d", w );

  XtVaSetValues( objWValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setObjH (
  int h
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-d", h );

  XtVaSetValues( objHValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setObjTopDist (
  int d
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-d", d );

  XtVaSetValues( objTopDistValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setObjBotDist (
  int d
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-d", d );

  XtVaSetValues( objBotDistValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setObjLeftDist (
  int d
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-d", d );

  XtVaSetValues( objLeftDistValue,
   XmNvalue, buf,
   NULL );

  return 1;

}

int dimDialogClass::setObjRightDist (
  int d
) {

char buf[15+1];

  if ( !widgetsCreated ) return 1;

  snprintf( buf, 15, "%-d", d );

  XtVaSetValues( objRightDistValue,
   XmNvalue, buf,
   NULL );

  return 1;

}
