#ifndef __undo_h
#define __undo_h 1

#include <stdlib.h>
#include "act_grf.h"

#ifdef __undo_cc

#include "undo.str"

#endif

class undoOpClass {

public:

undoOpClass () {

}

virtual ~undoOpClass () {

}

};

class undoNodeClass {

public:

class activeGraphicClass *actGrfAddr;
class activeGraphicClass *actGrfCopyAddr;
undoOpClass *opPtr; // an activeGraphicClass object may create this
int x;
int y;
int w;
int h;

undoNodeClass ();

virtual ~undoNodeClass ();

virtual int undo ( void );

};

// --------------------- derived classes ---------------------------

class undoCreateNodeClass : public undoNodeClass {

public:

int undo ( void );
undoCreateNodeClass ();
~undoCreateNodeClass ();

};

class undoMoveNodeClass : public undoNodeClass {

public:

int undo ( void );
undoMoveNodeClass ();
~undoMoveNodeClass ();

};

class undoResizeNodeClass : public undoNodeClass {

public:

int undo ( void );
undoResizeNodeClass ();
~undoResizeNodeClass ();

};

class undoCopyNodeClass : public undoNodeClass {

public:

int undo ( void );
undoCopyNodeClass ();
~undoCopyNodeClass ();

};

class undoCutNodeClass : public undoNodeClass {

public:

int undo ( void );
undoCutNodeClass ();
~undoCutNodeClass ();

};

class undoPasteNodeClass : public undoNodeClass {

public:

int undo ( void );
undoPasteNodeClass ();
~undoPasteNodeClass ();

};

class undoReorderNodeClass : public undoNodeClass {

public:

int undo ( void );
undoReorderNodeClass ();
~undoReorderNodeClass ();

};

class undoEditNodeClass : public undoNodeClass {

public:

int undo ( void );
undoEditNodeClass ();
~undoEditNodeClass ();

};

class undoGroupNodeClass : public undoNodeClass {

public:

int undo ( void );
undoGroupNodeClass ();
~undoGroupNodeClass ();

};

class undoRotateNodeClass : public undoNodeClass {

public:

int undo ( void );
undoRotateNodeClass ();
~undoRotateNodeClass ();

};

class undoFlipNodeClass : public undoNodeClass {

public:

int undo ( void );
undoFlipNodeClass ();
~undoFlipNodeClass ();

};

//class undo?NodeClass : public undoNodeClass {

//public:

//int undo?undo ( void );
//undo?undo?NodeClass ();
//undo?~undo?NodeClass ();

//};

//-------------------------------------------------------------------------

typedef struct undoListTag {
  struct undoListTag *flink;
  undoNodeClass *node;
} undoListType, *undoListPtr;

typedef struct undoListHead {
  undoListPtr head;
  undoListPtr tail;
} undoListHeadType, *undoListHeadPtr;

//-------------------------------------------------------------------------

class undoClass {

public:

static const int max = 32;

int wantFlush;
int head, tail;
undoListHeadType undoList[max+1];
char undoButtonText[max+1][15+1];

static const int success = 1;
static const int noMem = 100;
//static const = 10;

undoClass ();

~undoClass ();

void deleteNodes (
  int i );

void startNewUndoList (
  char *undoText );

int addCreateNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int addMoveNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr,
  int x,
  int y );

int addResizeNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int addCopyNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int addCutNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int addPasteNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int addReorderNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int addEditNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int addGroupNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int addRotateNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int addFlipNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int performUndo ( void );

int performSubUndo ( void ); // called from group.cc

void flush ( void );

void show ( void );

int listEmpty ( void );

void discard ( void );

void requestFlush ( void );

int flushRequested ( void );

};

#endif
