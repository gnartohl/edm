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

char* zlocXtMalloc ( size_t size, char* _fname, int _line );
char* zlocXtCalloc ( size_t num, size_t size, char* _fname, int _line );
void zlocXtFree ( char *obj, char* _fname, int _line );
char* zlocXtRealloc ( char *oldPtr, size_t size, char* _fname, int _line );
void* zlocnew ( size_t size, char* _fname, int _line );
void zlocdelete ( void *obj, char* _fname, int _line );

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

