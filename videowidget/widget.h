#include <X11/X.h>

typedef void * widgetData;

// widget class initialization (once for entire application)
widgetData widgetCreate(void);

// actually create a widget instance (many times per application)
Widget widgetCreateWidget(widgetData wd, XtAppContext app, Display *d, Colormap cm, Widget parent, int x, int y, int h, int w);

// feed the widget new information to use in rendering the display
void widgetNewDisplayData(widgetData wd, time_t time, unsigned long nano, unsigned long widgetw, unsigned long widgeth, unsigned long dataw, unsigned long datah, const double *data);
void widgetNewDisplayInfo(widgetData wd, bool valid, short status, short severity);

// free up resources associated with widget instance
void widgetDestroyWidget(widgetData wd);

// free up class resources (this may not actually ever be called)
void widgetDestroy(widgetData wd);

// communicate properties with EDM so that they can be saved and restored
// some properties may be controllable only at "run time" via the widget
int widgetGetProperties(widgetData wd);
void widgetSetProperties(widgetData wd);

