#define __undo_cc 1

#include "undo.h"

undoNodeClass::undoNodeClass () {

  actGrfAddr = NULL;
  actGrfCopyAddr = NULL;
  opPtr = NULL;

}


undoNodeClass::~undoNodeClass () {

  if ( actGrfCopyAddr ) {
    delete actGrfCopyAddr;
    actGrfCopyAddr = NULL;
  }
  if ( opPtr ) {
    delete opPtr;
    opPtr = NULL;
  }

}

int undoNodeClass::undo ( void ) {

  fprintf( stderr, "undoNodeClass::undo( void )\n" );

  return undoClass::success;

}

// --------------------- derived classes ---------------------------

undoCreateNodeClass::undoCreateNodeClass () {

}

undoCreateNodeClass::~undoCreateNodeClass () {

}

int undoCreateNodeClass::undo ( void ) {

  return undoClass::success;

}

undoMoveNodeClass::undoMoveNodeClass () {

}

undoMoveNodeClass::~undoMoveNodeClass () {

}

int undoMoveNodeClass::undo ( void ) {

int stat;

  stat = actGrfAddr->undoMove( opPtr, x, y );

  return stat;

}

undoResizeNodeClass::undoResizeNodeClass () {

}

undoResizeNodeClass::~undoResizeNodeClass () {

}

int undoResizeNodeClass::undo ( void ) {

int stat;

  stat = actGrfAddr->undoResize( opPtr, x, y, w, h );

  return stat;

}

undoCopyNodeClass::undoCopyNodeClass () {

}

undoCopyNodeClass::~undoCopyNodeClass () {

}

int undoCopyNodeClass::undo ( void ) {

  return undoClass::success;

}

undoCutNodeClass::undoCutNodeClass () {

}

undoCutNodeClass::~undoCutNodeClass () {

}

int undoCutNodeClass::undo ( void ) {

  return undoClass::success;

}

undoPasteNodeClass::undoPasteNodeClass () {

}

undoPasteNodeClass::~undoPasteNodeClass () {

}

int undoPasteNodeClass::undo ( void ) {

  return undoClass::success;

}

undoReorderNodeClass::undoReorderNodeClass () {

}

undoReorderNodeClass::~undoReorderNodeClass () {

}

int undoReorderNodeClass::undo ( void ) {

  return undoClass::success;

}

undoEditNodeClass::undoEditNodeClass () {

}

undoEditNodeClass::~undoEditNodeClass () {

}

int undoEditNodeClass::undo ( void ) {

int stat;

  stat = actGrfAddr->undoEdit( opPtr );

  return stat;

}

undoGroupNodeClass::undoGroupNodeClass () {

}

undoGroupNodeClass::~undoGroupNodeClass () {

}

int undoGroupNodeClass::undo ( void ) {

  return undoClass::success;

}

undoRotateNodeClass::undoRotateNodeClass () {

}

undoRotateNodeClass::~undoRotateNodeClass () {

}

int undoRotateNodeClass::undo ( void ) {

int stat;

  stat = actGrfAddr->undoRotate( opPtr, x, y, w, h );

  return stat;

}

undoFlipNodeClass::undoFlipNodeClass () {

}

undoFlipNodeClass::~undoFlipNodeClass () {

}

int undoFlipNodeClass::undo ( void ) {

int stat;

  stat = actGrfAddr->undoFlip( opPtr, x, y, w, h );

  return stat;

}

// -------------------------------------------------------------------

undoClass::undoClass () {

int i;

  head = tail = 0;

  wantFlush = 0;

  // init
  for ( i=0; i<undoClass::max; i++ ) {

    // create sentinel nodes
    undoList[i].head = new undoListType;
    undoList[i].tail = undoList[i].head;
    undoList[i].tail->flink = NULL;

  }

}

undoClass::~undoClass () {

int i;

  i = head;
  while ( i != tail ) {

    // delete all nodes in list
    deleteNodes( i );
    //delete undoList[i].head;

    i++;
    if ( i >= max ) i = 0;

  }

  i = tail;
  deleteNodes( i );
  //delete undoList[i].head;

  for ( i=0; i<undoClass::max; i++ ) {
    delete undoList[i].head;
  }

}

void undoClass::deleteNodes (
  int i
) {

undoListPtr cur, next;

  // delete all nodes in list
  cur = undoList[i].head->flink;
  while ( cur ) {
    next = cur->flink;
    delete cur->node;
    cur->node = NULL;
    delete cur;
    cur = next;
  }

  undoList[i].tail = undoList[i].head;
  undoList[i].tail->flink = NULL;

}

void undoClass::startNewUndoList (
  char *undoText )
{

  tail++;
  if ( tail >= max ) tail = 0;

  if ( tail == head ) {
    deleteNodes( head );
    head++;
    if ( head >= max ) head = 0;
  }

  strncpy( undoButtonText[tail], undoText, 15 );
  undoButtonText[tail][15] = 0;

}

int undoClass::addCreateNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr
) {

  return 1;

}

int undoClass::addMoveNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr,
  int _x,
  int _y
) {

undoNodeClass *node;
undoListPtr cur;

  node = new undoMoveNodeClass;
  if ( !node ) return undoClass::noMem;

  node->actGrfAddr = _actGrfAddr;
  node->actGrfCopyAddr = NULL;
  node->opPtr = _opPtr;
  node->x = _x;
  node->y = _y;

  if ( strcmp( undoButtonText[tail], "" ) != 0 ) {
    node->actGrfAddr->setUndoText( undoButtonText[tail] );
  }

  cur = (undoListPtr) new undoListType;
  if ( !cur ) return undoClass::noMem;

  cur->node = node;
  undoList[tail].tail->flink = cur;
  undoList[tail].tail = cur;
  undoList[tail].tail->flink = NULL;
  
  return 1;

}

int undoClass::addResizeNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr,
  int _x,
  int _y,
  int _w,
  int _h
) {

undoNodeClass *node;
undoListPtr cur;

  node = new undoResizeNodeClass;
  if ( !node ) return undoClass::noMem;

  node->actGrfAddr = _actGrfAddr;
  node->actGrfCopyAddr = NULL;
  node->opPtr = _opPtr;
  node->x = _x;
  node->y = _y;
  node->w = _w;
  node->h = _h;

  if ( strcmp( undoButtonText[tail], "" ) != 0 ) {
    node->actGrfAddr->setUndoText( undoButtonText[tail] );
  }

  cur = (undoListPtr) new undoListType;
  if ( !cur ) return undoClass::noMem;

  cur->node = node;
  undoList[tail].tail->flink = cur;
  undoList[tail].tail = cur;
  undoList[tail].tail->flink = NULL;
  
  return 1;

}

int undoClass::addCopyNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr
) {

  return 1;

}

int undoClass::addCutNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr
) {

  return 1;

}

int undoClass::addPasteNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr
) {

  return 1;

}

int undoClass::addReorderNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr
) {

  return 1;

}

int undoClass::addEditNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr
) {

undoNodeClass *node;
undoListPtr cur;

  node = new undoEditNodeClass;
  if ( !node ) return undoClass::noMem;

  node->actGrfAddr = _actGrfAddr;
  node->actGrfCopyAddr = NULL;
  node->opPtr = _opPtr;

  if ( strcmp( undoButtonText[tail], "" ) != 0 ) {
    node->actGrfAddr->setUndoText( undoButtonText[tail] );
  }

  cur = (undoListPtr) new undoListType;
  if ( !cur ) return undoClass::noMem;

  cur->node = node;
  undoList[tail].tail->flink = cur;
  undoList[tail].tail = cur;
  undoList[tail].tail->flink = NULL;

  return 1;

}

int undoClass::addGroupNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr
) {

  return 1;

}

int undoClass::addRotateNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr,
  int _x,
  int _y,
  int _w,
  int _h
) {

undoNodeClass *node;
undoListPtr cur;

  node = new undoRotateNodeClass;
  if ( !node ) return undoClass::noMem;

  node->actGrfAddr = _actGrfAddr;
  node->actGrfCopyAddr = NULL;
  node->opPtr = _opPtr;
  node->x = _x;
  node->y = _y;
  node->w = _w;
  node->h = _h;

  if ( strcmp( undoButtonText[tail], "" ) != 0 ) {
    node->actGrfAddr->setUndoText( undoButtonText[tail] );
  }

  cur = (undoListPtr) new undoListType;
  if ( !cur ) return undoClass::noMem;

  cur->node = node;
  undoList[tail].tail->flink = cur;
  undoList[tail].tail = cur;
  undoList[tail].tail->flink = NULL;
  
  return 1;

}

int undoClass::addFlipNode (
  activeGraphicClass *_actGrfAddr,
  undoOpClass *_opPtr,
  int _x,
  int _y,
  int _w,
  int _h
) {

undoNodeClass *node;
undoListPtr cur;

  node = new undoFlipNodeClass;
  if ( !node ) return undoClass::noMem;

  node->actGrfAddr = _actGrfAddr;
  node->actGrfCopyAddr = NULL;
  node->opPtr = _opPtr;
  node->x = _x;
  node->y = _y;
  node->w = _w;
  node->h = _h;

  if ( strcmp( undoButtonText[tail], "" ) != 0 ) {
    node->actGrfAddr->setUndoText( undoButtonText[tail] );
  }

  cur = (undoListPtr) new undoListType;
  if ( !cur ) return undoClass::noMem;

  cur->node = node;
  undoList[tail].tail->flink = cur;
  undoList[tail].tail = cur;
  undoList[tail].tail->flink = NULL;
  
  return 1;

}

int undoClass::performUndo( void ) {

undoListPtr cur;
int stat, checkTail, willBeEmpty;

  if ( tail == head ) {
    return 0; // error, empty list
  }

  // see if list will become empty
  checkTail = tail;
  checkTail--;
  if ( checkTail < 0 ) checkTail = undoClass::max-1;
  if ( checkTail == head ) {
    willBeEmpty = 1;
  }
  else {
    willBeEmpty = 0;
  }

  // process all nodes in list
  cur = undoList[tail].head->flink;

  // update undo button text for first node
  if ( cur && willBeEmpty ) {
    if ( strcmp( undoButtonText[tail], "" ) != 0 ) {
      cur->node->actGrfAddr->setUndoText( NULL );
    }
  }
  else if ( cur ) {
    if ( strcmp( undoButtonText[tail], "" ) != 0 ) {
      cur->node->actGrfAddr->setUndoText( undoButtonText[checkTail] );
    }
  }

  while ( cur ) {
    stat = cur->node->undo();
    if ( !( stat & 1 ) ) goto err_return;
    cur = cur->flink;
  }

  stat = 1;

err_return:

  deleteNodes( tail );

  tail--;
  if ( tail < 0 ) tail = undoClass::max-1;

  return stat;

}

int undoClass::performSubUndo( void ) {

undoListPtr cur;
int stat;

  if ( tail == head ) {
    return 1; // always return success for sub list
  }

  // process all nodes in list
  cur = undoList[tail].head->flink;

  while ( cur ) {
    stat = cur->node->undo();
    if ( !( stat & 1 ) ) goto err_return;
    cur = cur->flink;
  }

  stat = 1;

err_return:

  deleteNodes( tail );

  tail--;
  if ( tail < 0 ) tail = undoClass::max-1;

  return stat;

}

void undoClass::flush ( void )
{

int i;
undoListPtr cur;

  wantFlush = 0;

  if ( head == tail ) return; // empty list

  // update undo button text and sensitivity
  cur = undoList[tail].head->flink;
  if ( strcmp( undoButtonText[tail], "" ) != 0 ) {
    cur->node->actGrfAddr->setUndoText( NULL );
  }

  i = head;
  //  i = head + 1;
  //  if ( i >= max ) i = 0;

  while ( i != tail ) {

    cur = undoList[i].head->flink;
    while ( cur ) {
      cur->node->actGrfAddr->flushUndo();
      cur = cur->flink;
    }

    i++;
    if ( i >= max ) i = 0;

  }

  i = tail;
  cur = undoList[i].head->flink;
  while ( cur ) {
    cur->node->actGrfAddr->flushUndo();
    cur = cur->flink;
  }

  i = head;
  // i = head + 1;
  // if ( i >= max ) i = 0;

  while ( i != tail ) {

    // delete all nodes in list
    deleteNodes( i );

    i++;
    if ( i >= max ) i = 0;

  }

  deleteNodes( tail );

  head = tail = 0;

}

// ==================================================

void undoClass::show( void ) {

undoListPtr cur;
activeGraphicClass *gPtr;

  if ( tail == head ) {
    return;
  }

  // process all nodes in list
  cur = undoList[tail].head->flink;

  while ( cur ) {
    gPtr = cur->node->actGrfAddr;
    fprintf( stderr, "obj = %s, edit = %-d\n", gPtr->objName(),
     gPtr->checkEditStatus() );
    cur = cur->flink;
  }

}

int undoClass::listEmpty ( void ) {

undoListPtr cur;
activeGraphicClass *gPtr;

  if ( tail == head ) {
    return 1; // empty
  }

  // process all nodes in list
  cur = undoList[tail].head->flink;
  if ( !cur ) return 1; // empty

  while ( cur ) {
    gPtr = cur->node->actGrfAddr;
    if ( gPtr->checkEditStatus() ) return 0; // not empty
    cur = cur->flink;
  }

  return 1; // empty

}

void undoClass::discard ( void ) {

undoListPtr cur;

  if ( tail == head ) {
    return;
  }

  // do this in case undo list will become empty
  cur = undoList[tail].head->flink;
  if ( cur ) cur->node->actGrfAddr->setUndoText( NULL );

  deleteNodes( tail );

  tail--;
  if ( tail < 0 ) tail = undoClass::max-1;

  if ( tail != head ) { // undo list did not become empty
    cur = undoList[tail].head->flink;
    if ( cur ) cur->node->actGrfAddr->setUndoText( undoButtonText[tail] );
  }

}

void undoClass::requestFlush ( void ) {

  wantFlush = 1;

}

int undoClass::flushRequested ( void ) {

  return wantFlush;

}
