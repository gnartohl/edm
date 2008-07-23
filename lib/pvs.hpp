#ifndef __pvsClass_hpp
#define __pvsClass_hpp 1

#ifdef __linux__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/time.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#endif

class pvsClass {

public:

// odd status - ok, even status - error
static const int PVS_SUCCESS = 1;
static const int PVS_FAILURE = 2;
static const int PVS_NOMORE = 3;
static const int PVS_SERVER_FAIL = 100;
static const int PVS_INVALID_HANDLE = 102;
#define PVS_INCOMPATIBLE_VERION 104

pvsClass( void );

pvsClass (
  char *hostWithPort
);

~pvsClass( void );

int destroy ( void );

int getNumPvs (
  int *n
);

int getFirstPvsName (
  char **name
);

int getNextPvsName (
  char **name
);

private:

int sendCmd (
  int socketFd,
  char *msg
);

int getReply (
  int socketFd,
  char *msg,
  int maxLen
);

int cmd (
  char *ipAddrArg,
  char *portArg,
  char *cmd,
  char *reply,
  int maxReplySize
);

int init ( void );

int readGroup ( void );

int needInit;
int numPvs;
int bufSize;
int curGroup;
int curNumNames;
int curNameIndex;
char *host;
char *port;
char *buf;
char *buf2;
char *tk;
char *tk2;
char *ctx;
char *ctx2;

};

#endif
