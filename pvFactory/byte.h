// -*- C++ -*-
// EDM Byte Widget
//
// Carl Lionberger
//

#ifndef __BYTE_H__
#define __BYTE_H__

#include "act_grf.h"
#include "entry_form.h"
#include "pv_factory.h"

#define BYTE_CLASSNAME "ByteClass"
#define BYTE_MAJOR 4
#define BYTE_MINOR 0
#define BYTE_RELEASE 0
#define BYTE_COLORMODE_STATIC 0
#define BYTE_COLORMODE_ALARM 1

class edmByteClass : public activeGraphicClass
{
public:
    edmByteClass();
    edmByteClass(edmByteClass *rhs);
    virtual ~edmByteClass();
    void bufInvalidate(void) { bufInvalid++; };
    char *objName();
    const char *getRawPVName();
    const char *getExpandedPVName();

    
    // Load/save
    int save(FILE *f);
    int old_save(FILE *f);
    int createFromFile(FILE *fptr, char *name, activeWindowClass *actWin);
    int old_createFromFile(FILE *fptr, char *name, activeWindowClass *actWin);

    // Edit Mode
    int createInteractive(activeWindowClass *aw_obj,
                          int x, int y, int w, int h);
    int edit();
    int draw();
    int erase();
    
        // Group Edit
    void changeDisplayParams(unsigned int flag,
                             char *fontTag,
                             int alignment,
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
                             int botShadowColor);

    void changePvNames(int flag,
                       int numCtlPvs,
                       char *ctlPvs[],
                       int numReadbackPvs,
                       char *readbackPvs[],
                       int numNullPvs,
                       char *nullPvs[],
                       int numVisPvs,
                       char *visPvs[],
                       int numAlarmPvs,
                       char *alarmPvs[]);
    
    void getPvs(int max,
			  ProcessVariable *pvs[],
			  int *n);

    char *getSearchString (
      int i
    );

    void replaceString (
      int i,
      int max,
      char *string
    );

    // Macro support
    int containsMacros();
    int expandTemplate (int numMacros, char *macros[], char *expansions[]);
    int expand1st(int numMacros, char *macros[], char *expansions[]);
    int expand2nd(int numMacros, char *macros[], char *expansions[]);
    
    // Execute
    int activate(int pass, void *ptr);
    int deactivate(int pass);
    int drawActive();
    int eraseActive();
    int eraseUnconditional();
    void executeDeferred();

    // accessed from callbacks
    int init;
    int lineColor;
    int onColor, offColor;
    int onPixel, offPixel, fgPixel;
    int minorPixel, majorPixel, invalidPixel;
    void updateDimensions();

    char *firstDragName();
    char *nextDragName();
    char *dragValue(int i);

    char *crawlerGetFirstPv ( void );
    char *crawlerGetNextPv ( void );
    
protected:

    void clone(const edmByteClass *rhs, const char *classname);
    int drawActiveFull();
    int drawActiveBits();
    inline void innerDrawFull(int value, int i, int mask, 
                                     int &previous, int &lastseg);
    inline void innerDrawBits(int value, int i, int mask);

    void bufValidate() { if (bufInvalid > 0) bufInvalid--; };
    bool is_executing;          // edit or execute mode? (was activeMode?)
    bool is_pvname_valid;       
    ProcessVariable *valuePvId;        // ChannelAccess, PV
    int bufInvalid;
    bool validFlag;
    unsigned int value, lastval, dmask, lastsev;
    
    // Properties
    enum bdir { BIGENDIAN, LITTLEENDIAN };
    bdir theDir;
    int nobt;		// number of bits
    int shft;		// shift
    bdir bufTheDir;
    int bufNobt;		// number of bits
    int bufShft;		// shift

    expStringClass pv_exp_str;  // PV name as macro-expandable string

    // Helpers for createInteractive & edit,
    // buffers for property dialog
    int genericEdit();
    int editCreate();
    int bufX, bufY, bufW, bufH;

    int bufLineColor;
    colorButtonClass lineCb;
    
    colorButtonClass onColorCb;
    colorButtonClass offColorCb;
    
    int lineWidth, bufLineWidth;
    int lineStyle, bufLineStyle;

    XSegment *theOutline;

    char bufPvName[PV_Factory::MAX_PV_NAME+1];
    int bufOnColor;
    int bufOffColor;

    // Callbacks for property edit
    static void edit_update(Widget w, XtPointer client, XtPointer call);
    static void edit_ok(Widget w, XtPointer client, XtPointer call);
    static void edit_apply(Widget w, XtPointer client, XtPointer call);
    static void edit_cancel(Widget w, XtPointer client, XtPointer call);
    static void edit_cancel_delete(Widget w, XtPointer client,
                                   XtPointer call);
    // CA callbacks
    static void pv_callback(ProcessVariable *pv, void *userarg);

};

#endif
