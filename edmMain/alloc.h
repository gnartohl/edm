#ifndef __alloc_h
#define __alloc_h 1

#define DIAGNOSTIC_ALLOC 1

char* zXtMalloc ( size_t size );
char* zXtCalloc ( size_t num, size_t size );
void zXtFree ( char *obj );
char* zXtRealloc ( char *oldPtr, size_t size );
void* znew ( size_t size );
void showMem ( void );
void zdelete ( void *obj );
void memTrackOn ( void );
void memTrackOff ( void );
void memTrackReset ( void );
void showMem ( void );
void zFunc ( void );

char* XtMalloc ( size_t size ){
  return zXtMalloc( size );
}
char* XtCalloc ( size_t num, size_t size ) {
  return zXtCalloc( num, size );
}
void XtFree ( char *obj ) {
  zXtFree( obj );
}
char* XtRealloc ( char *oldPtr, size_t size ) {
  return zXtRealloc( oldPtr, size );
}
void* operator new ( size_t size ) {
  return znew( size );
}
void operator delete ( void *obj ) {
  zdelete( obj );
}

#endif

