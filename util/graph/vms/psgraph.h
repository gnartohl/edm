#ifndef __psgraph_h
#define __psgraph_h 1

/*
* limits
*/
#define TITLE_MAX_CHARS		80
#define FORMAT_MAX_CHARS	20
#define LABEL_MAX_CHARS		40

/*
* constants
*/
#define PSGRAPH_POINT		0X1001
#define PSGRAPH_LINE		0X1002
#define PSGRAPH_NEEDLE		0X1003
#define PSGRAPH_BAR		0X1004
#define PSGRAPH_FILL_BAR	0X1005
#define PSGRAPH_NOSYMBOL	0X2000
#define PSGRAPH_CIRCLE		0X2002
#define PSGRAPH_SQUARE		0X2003
#define PSGRAPH_DIAMOND		0X2004
#define PSGRAPH_TRIANGLE	0X2005
#define PSGRAPH_SOLID		0X3001
#define PSGRAPH_DASH		0X3002
#define PSGRAPH_DOT		0X3003
#define PSGRAPH_DASH_DOT_DOT	0X3004
#define PSGRAPH_SCALE_LINEAR	0X4001
#define PSGRAPH_SCALE_LOG	0X4002

typedef void *GENERIC_PTR;

typedef void *GWIN_ID;

typedef void *TEXT_ID;

typedef void *AREA_ID;

typedef void *PLOT_ID;

/*
* global values
*/
globalvalue PSGRAPH_SUCCESS;
globalvalue PSGRAPH_NO_MEM;
globalvalue PSGRAPH_NO_COLOR;
globalvalue PSGRAPH_NO_FONT;
globalvalue PSGRAPH_BAD_OBJ;
globalvalue PSGRAPH_SCALE_MISMATCH;
globalvalue PSGRAPH_BAD_LOG_VALUE;
globalvalue PSGRAPH_NO_DISPLAY;

#endif
