#ifndef __xgraph_h
#define __xgraph_h 1

/*
* limits
*/
#define TITLE_MAX_CHARS		80
#define FORMAT_MAX_CHARS	20
#define LABEL_MAX_CHARS		40

/*
* constants
*/
#define XGRAPH_POINT		0X1001
#define XGRAPH_LINE		0X1002
#define XGRAPH_NEEDLE		0X1003
#define XGRAPH_BAR		0X1004
#define XGRAPH_FILL_BAR		0X1005
#define XGRAPH_NOSYMBOL		0X2000
#define XGRAPH_CIRCLE		0X2002
#define XGRAPH_SQUARE		0X2003
#define XGRAPH_DIAMOND		0X2004
#define XGRAPH_TRIANGLE		0X2005
#define XGRAPH_SOLID		0X3001
#define XGRAPH_DASH		0X3002
#define XGRAPH_DOT		0X3003
#define XGRAPH_DASH_DOT_DOT	0X3004
#define XGRAPH_SCALE_LINEAR	0X4001
#define XGRAPH_SCALE_LOG	0X4002

typedef void *GENERIC_PTR;

typedef void *GWIN_ID;

typedef void *TEXT_ID;

typedef void *AREA_ID;

typedef void *PLOT_ID;

/*
* global values
*/
#define XGRAPH_SUCCESS 1
#define XGRAPH_NO_DISPLAY 2
#define XGRAPH_NO_MEM 4
#define XGRAPH_NO_COLOR 6
#define XGRAPH_NO_FONT 8
#define XGRAPH_BAD_OBJ 10
#define XGRAPH_SCALE_MISMATCH 12
#define XGRAPH_BAD_LOG_VALUE 14

#endif
