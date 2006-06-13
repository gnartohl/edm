#ifndef __crawler_h
#define __crawler_h 1

typedef struct parentNodeTag {
  char *parentName;
  struct parentNodeTag *flink;
} parentNodeType, *parentNodePtr;

typedef struct parentListTag {
  parentNodePtr head;
  parentNodePtr tail;
} parentListType, *parentListPtr;

typedef struct crawlListTag {
  struct crawlListTag *flink;
  struct crawlListTag *blink;
  parentListType pList;
  char *fname;
  unsigned int crc;
  int numMacros;
  char **symbols;
  char **values;
} crawlListType, *crawlListPtr;

int setCrawlListBaseMacros (
  int num,
  char **syms,
  char **vals
);

void setMaxCrawlDepth (
  int val
);

void setCrawlVerbose (
  int val
);

int initCrawlList (
  crawlListPtr *listHead
);

int addCrawlNode (
  crawlListPtr listHead,
  char *fname,
  int numMacros,
  char **symbols,
  char **values
);

int addChildCrawlNode (
  crawlListPtr listHead,
  parentListPtr curParentList,
  char *parentFname,
  char *fname,
  int numMacros,
  char **symbols,
  char **values
);

int crawlEdlFiles (
  appContextClass *appCtx,
  crawlListPtr listHead
);

int displayCrawlerResults ( void );

#endif
