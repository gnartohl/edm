#ifndef __undo_h
#define __undo_h 1

#include <stdlib.h>
#include "act_grf.h"

#ifdef __undo_cc

#include "undo.str"

#endif

class undoOpClass {

public:

undoOpClass::undoOpClass () {

}

virtual undoOpClass::~undoOpClass () {

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

undoNodeClass::undoNodeClass ();

virtual undoNodeClass::~undoNodeClass ();

virtual int undoNodeClass::undo ( void );

};

// --------------------- derived classes ---------------------------

class undoCreateNodeClass : public undoNodeClass {

public:

int undoCreateNodeClass::undo ( void );
undoCreateNodeClass::undoCreateNodeClass ();
undoCreateNodeClass::~undoCreateNodeClass ();

};

class undoMoveNodeClass : public undoNodeClass {

public:

int undoMoveNodeClass::undo ( void );
undoMoveNodeClass::undoMoveNodeClass ();
undoMoveNodeClass::~undoMoveNodeClass ();

};

class undoResizeNodeClass : public undoNodeClass {

public:

int undoResizeNodeClass::undo ( void );
undoResizeNodeClass::undoResizeNodeClass ();
undoResizeNodeClass::~undoResizeNodeClass ();

};

class undoCopyNodeClass : public undoNodeClass {

public:

int undoCopyNodeClass::undo ( void );
undoCopyNodeClass::undoCopyNodeClass ();
undoCopyNodeClass::~undoCopyNodeClass ();

};

class undoCutNodeClass : public undoNodeClass {

public:

int undoCutNodeClass::undo ( void );
undoCutNodeClass::undoCutNodeClass ();
undoCutNodeClass::~undoCutNodeClass ();

};

class undoPasteNodeClass : public undoNodeClass {

public:

int undoPasteNodeClass::undo ( void );
undoPasteNodeClass::undoPasteNodeClass ();
undoPasteNodeClass::~undoPasteNodeClass ();

};

class undoReorderNodeClass : public undoNodeClass {

public:

int undoReorderNodeClass::undo ( void );
undoReorderNodeClass::undoReorderNodeClass ();
undoReorderNodeClass::~undoReorderNodeClass ();

};

class undoEditNodeClass : public undoNodeClass {

public:

int undoEditNodeClass::undo ( void );
undoEditNodeClass::undoEditNodeClass ();
undoEditNodeClass::~undoEditNodeClass ();

};

class undoGroupNodeClass : public undoNodeClass {

public:

int undoGroupNodeClass::undo ( void );
undoGroupNodeClass::undoGroupNodeClass ();
undoGroupNodeClass::~undoGroupNodeClass ();

};

class undoRotateNodeClass : public undoNodeClass {

public:

int undoRotateNodeClass::undo ( void );
undoRotateNodeClass::undoRotateNodeClass ();
undoRotateNodeClass::~undoRotateNodeClass ();

};

class undoFlipNodeClass : public undoNodeClass {

public:

int undoFlipNodeClass::undo ( void );
undoFlipNodeClass::undoFlipNodeClass ();
undoFlipNodeClass::~undoFlipNodeClass ();

};

//class undo?NodeClass : public undoNodeClass {

//public:

//int undo?NodeClass::undo ( void );
//undo?NodeClass::undo?NodeClass ();
//undo?NodeClass::~undo?NodeClass ();

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

undoClass::undoClass ();

undoClass::~undoClass ();

void undoClass::deleteNodes (
  int i );

void undoClass::startNewUndoList (
  char *undoText );

int undoClass::addCreateNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int undoClass::addMoveNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr,
  int x,
  int y );

int undoClass::addResizeNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int undoClass::addCopyNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int undoClass::addCutNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int undoClass::addPasteNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int undoClass::addReorderNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int undoClass::addEditNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int undoClass::addGroupNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr );

int undoClass::addRotateNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int undoClass::addFlipNode (
  activeGraphicClass *actGrfAddr,
  undoOpClass *opPtr,
  int x,
  int y,
  int w,
  int h );

int undoClass::performUndo ( void );

int undoClass::performSubUndo ( void ); // called from group.cc

void undoClass::flush ( void );

void undoClass::show ( void );

int undoClass::listEmpty ( void );

void undoClass::discard ( void );

void undoClass::requestFlush ( void );

int undoClass::flushRequested ( void );

};

#endif
