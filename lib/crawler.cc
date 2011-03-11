//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdio.h>
#include <stdlib.h>

#include "process.h"
#include "app_pkg.h"
#include "act_win.h"
#include "act_grf.h"
#include "tag_pkg.h"
#include "utility.h"
#include "crawler.h"
#include "crc.h"
#include "avl.h"

#include "act_win.str"


////////////////////////////////////////////////////////////////////////////
//
// This code implements the capability for edm to crawl through
// all input files and related files and generate many lines of output
// each relating a file name to a corresponding PV.
//
// The code creates a multitude of memory leaks and must therefore be used
// only for the case in which the files are processed, output is generated,
// and the program immediately exits.
//
////////////////////////////////////////////////////////////////////////////

static int numBaseMacros;
static char **baseSymbols;
static char **baseValues;

static int maxCrawlDepth = 20;
static int depth = 0;

static int totalRefs = 0;
static int totalDups = 0;

static int verbose = 0;

typedef struct nameListTag {
  AVL_FIELDS(nameListTag)
  char *pvAndFileName;
  int startOfFileName;
} nameListType, *nameListPtr;

static AVL_HANDLE pvAndFileNameTree = NULL;

typedef struct processedDisplayTag {
  AVL_FIELDS(processedDisplayTag)
  char *fname;
  unsigned int crc;
} processedDisplayType, *processedDisplayPtr;

static AVL_HANDLE processedDisplayTree = NULL;

static int compare_nodes (
  void *node1,
  void *node2
) {

nameListPtr p1, p2;

  p1 = (nameListPtr) node1;
  p2 = (nameListPtr) node2;

  return ( strcmp( p1->pvAndFileName, p2->pvAndFileName ) );

}

static int compare_key (
  void *key,
  void *node
) {

nameListPtr p;
char *one;

  p = (nameListPtr) node;
  one = (char *) key;

  return ( strcmp( one, p->pvAndFileName ) );

}

static int copy_node (
  void *node1,
  void *node2
) {

nameListPtr p1, p2;

  p1 = (nameListPtr) node1;
  p2 = (nameListPtr) node2;

  *p1 = *p2;

  return 1;

}

// ============================

static int pd_compare_nodes (
  void *node1,
  void *node2
) {

processedDisplayPtr p1, p2;
int namesNotEqual;

  p1 = (processedDisplayPtr) node1;
  p2 = (processedDisplayPtr) node2;

  namesNotEqual = strcmp( p1->fname, p2->fname );
  if ( namesNotEqual ) return namesNotEqual;

  if ( p1->crc > p2->crc )
    return 1;
  else if ( p1->crc < p2->crc )
    return -1;
  else
    return 0;

}

static int pd_compare_key (
  void *key,
  void *node
) {

processedDisplayPtr p = (processedDisplayPtr) node;
crawlListPtr oneItem = (crawlListPtr) key;
int namesNotEqual;

  namesNotEqual = strcmp( oneItem->fname, p->fname );
  if ( namesNotEqual ) return namesNotEqual;

  if ( oneItem->crc > p->crc )
    return 1;
  else if ( oneItem->crc < p->crc )
    return -1;
  else
    return 0;

}

static int pd_copy_node (
  void *node1,
  void *node2
) {

processedDisplayPtr p1, p2;

  p1 = (processedDisplayPtr) node1;
  p2 = (processedDisplayPtr) node2;

  *p1 = *p2;

  return 1;

}

// =============================

static void removeExt(
  char *name,
  char *fileName,
  int max ) {

char *sptr, *dptr;

  if ( !fileName ) {
    strncpy( name, "<null name>", max );
    name[max] = 0;
    return;
  }

  strncpy( name, fileName, max );
  name[max] = 0;

  // remove .edl extension

  sptr = strstr( name, ".edl" );

  if ( sptr ) {

    // make sure this is not part of a directory
    dptr = strstr( sptr, "/" );

    if ( !dptr ) {
      *sptr = 0;
    }

  }

}

static void destroyMacros (
  int totalMacros,
  char **symbols,
  char **values
) {

int i;

  for ( i=0; i<totalMacros; i++ ) {

    delete[] symbols[i];
    delete[] values[i];

  }

  delete[] symbols;
  delete[] values;

}

static int buildNewMacroList (
  char *macros,
  char *replaceOldMacros,
  char *propagate,
  int numOldMacros,
  char **oldSymbols,
  char **oldValues,
  int *numNewMacros,
  char ***newSymbols,
  char ***newValues,
  int *totalMacros
) {

int i, ii, numDeleted, dup, n;
char *tk, *context, buf[1023+1];
char *tk1, *context1, buf1[1023+1];
char *s, *v, *saveSym, *saveVal;

  *totalMacros = 0;
  n = 0;

  if ( strcmp( replaceOldMacros, "append" ) == 0 ) {

    if ( strcmp( propagate, "propagate" ) == 0 ) {

      n += numOldMacros;

    }
    else {

      n += numBaseMacros;

    }

  }

  if ( macros ) {

    // count tokens in macro string
    context = NULL;
    strncpy( buf, macros, 1023 );
    buf[1023] = 0;
    tk = strtok_r( buf, ",", &context );
    while ( tk ) {

      n++;

      tk = strtok_r( NULL, ",", &context );

    }

  }

  *totalMacros = n;
  *numNewMacros = n;
  *newSymbols = new char *[n];
  *newValues = new char *[n];

  n = 0;

  if ( strcmp( replaceOldMacros, "append" ) == 0 ) {

    if ( strcmp( propagate, "propagate" ) == 0 ) {

      for ( n=0; n<numOldMacros; n++ ) {
        (*newSymbols)[n] = new char[strlen(oldSymbols[n])+1];
        strcpy( (*newSymbols)[n], oldSymbols[n] );
        (*newValues)[n] = new char[strlen(oldValues[n])+1];
	if ( strcmp( oldValues[n], "''" ) == 0 ) {
          strcpy( (*newValues)[n], "" );
	}
	else {
          strcpy( (*newValues)[n], oldValues[n] );
	}
      }

    }
    else {

      for ( n=0; n<numBaseMacros; n++ ) {
        (*newSymbols)[n] = new char[strlen(baseSymbols[n])+1];
        strcpy( (*newSymbols)[n], baseSymbols[n] );
        (*newValues)[n] = new char[strlen(oldValues[n])+1];
	if ( strcmp( baseValues[n], "''" ) == 0 ) {
          strcpy( (*newValues)[n], "" );
	}
	else {
          strcpy( (*newValues)[n], baseValues[n] );
	}
      }

    }

  }

  if ( macros ) {

    // add tokens in macro string
    context = NULL;
    strncpy( buf, macros, 1023 );
    buf[1023] = 0;
    tk = strtok_r( buf, ",", &context );
    while ( tk ) {

      strcpy( buf1, tk );
      context1 = NULL;
      tk1 = strtok_r( buf1, " \t=", &context1 );
      if ( tk1 ) {

        s = new char[strlen(tk1)+1];
        strcpy( s, tk1 );
        (*newSymbols)[n] = s;

        tk1 = strtok_r( NULL, " \t=", &context1 );
        if ( tk1 ) {

          v = new char[strlen(tk1)+1];
          if ( strcmp( tk1, "''" ) == 0 ) {
            strcpy( v, "" );
          }
          else {
            strcpy( v, tk1 );
          }
          (*newValues)[n] = v;

        }
        else {

          v = new char[1];
          strcpy( v, "" );
          (*newValues)[n] = v;

        }

      }
      else {

        s = new char[1];
        strcpy( s, "" );
        (*newSymbols)[n] = s;

        v = new char[1];
        strcpy( v, "" );
        (*newValues)[n] = v;

      }

      n++;

      tk = strtok_r( NULL, ",", &context );

    }

  }

  *totalMacros = n;

  // Eliminate duplicate symbols

  numDeleted = 0;

  for ( i=(*numNewMacros)-1; i>0; i-- ) {

    dup = 0;
    ii = 0;
    while ( ii < i ) {

      if ( strcmp( (*newSymbols)[ii], (*newSymbols)[i] ) == 0 ) {
        dup = 1;
	break;
      }

      ii++;

    }

    if ( dup ) {

      saveSym = (*newSymbols)[i];
      saveVal = (*newValues)[i];

      // delete entry i
      for ( ii=i; ii<(*numNewMacros)-1; ii++ ) {
        (*newSymbols)[ii] = (*newSymbols)[ii+1];
        (*newValues)[ii] = (*newValues)[ii+1];
      }

      (*newSymbols)[(*numNewMacros)-1] = saveSym;
      (*newValues)[(*numNewMacros)-1] = saveVal;

      numDeleted++;

    }

  }

  (*numNewMacros) -= numDeleted;

  return 1;

}

static int displayHasBeenProcessed (
  crawlListPtr crawlListNode
) {

  // return true if display has already been processed

processedDisplayPtr node;
int status;

  if ( !processedDisplayTree ) {
    return 0;
  }

  status = avl_get_match( processedDisplayTree, (void *) crawlListNode,
   (void **) &node );
  if ( node ) {
    return 1;
  }

  return 0;

}

static int recordDisplay (
  crawlListPtr crawlListNode
) {

processedDisplayPtr cur;
int duplicate;
int status;

  // record display so it is only processed once

  if ( !processedDisplayTree ) {
    // init on first call
    status = avl_init_tree( pd_compare_nodes, pd_compare_key, pd_copy_node,
     &processedDisplayTree );
  }

  cur = new processedDisplayType;
  cur->fname = new char[strlen(crawlListNode->fname)+1];
  strcpy( cur->fname, crawlListNode->fname );
  cur->crc = crawlListNode->crc;

  // insert
  status = avl_insert_node( processedDisplayTree, (void *) cur, &duplicate );
  if ( duplicate ) {
    delete[] cur->fname;
    delete cur;
  }

  return 1;

}

int setCrawlListBaseMacros (
  int num,
  char **syms,
  char **vals
) {

  numBaseMacros = num;
  baseSymbols = syms;
  baseValues = vals;

  return 1;

}

int empty (
  crawlListPtr listHead
) {

  if ( !listHead ) return 1;

  if ( listHead->flink == listHead ) return 1;

  return 0;

}

void setMaxCrawlDepth (
  int val
) {

  if ( val > 0 ) {
    maxCrawlDepth = val;
  }

}

void setCrawlVerbose (
  int val
) {

  verbose = val;

}

int initCrawlList (
  crawlListPtr *listHead
) {

  *listHead = new crawlListType;
  (*listHead)->flink = *listHead;
  (*listHead)->blink = *listHead;

  return 1;

}

int destroyCrawlList (
  crawlListPtr *listHead
) {

crawlListPtr cur, next;
parentNodePtr pcur, pnext;

  cur = (*listHead)->flink;
  while ( cur != (*listHead) ) {

    next = cur->flink;
    cur->blink->flink = next;
    next->blink = cur->blink;

    pcur = cur->pList.head->flink;
    while ( pcur ) {
      pnext = pcur->flink;
      delete[] pcur->parentName;
      delete pcur;
      pcur = pnext;
    }
    delete  cur->pList.head;

    //printf( "fname = [%s]\n", cur->fname );
    delete[] cur->fname;
    delete cur;

    cur = next;

  }

  delete *listHead;
  *listHead = NULL;

  return 1;

}

int addCrawlNode (
  crawlListPtr listHead,
  char *fname,
  int numMacros,
  char **symbols,
  char **values
) {

crawlListPtr cur;
int i;
char name[1023+1];

  removeExt( name, fname, 1023 );

  cur = new crawlListType;
  //cur->fname = fname;
  cur->fname = new char[strlen(name)+1];
  strcpy( cur->fname, name );
  cur->numMacros = numMacros;
  cur->symbols = symbols;
  cur->values = values;

  cur->crc = 0;
  for ( i=0; i<numMacros; i++ ) {
    if ( symbols[i] && values[i] ) {
      cur->crc = updateCRC( cur->crc, symbols[i], strlen(symbols[i]) );
      cur->crc = updateCRC( cur->crc, values[i], strlen(values[i]) );
    }
  }

  // =====================================================
  // init list of parents

  cur->pList.head = new parentNodeType;
  cur->pList.tail = cur->pList.head;
  cur->pList.tail->flink = NULL;

  // =====================================================

  cur->blink = listHead->blink;
  listHead->blink->flink = cur;
  cur->flink = listHead;
  listHead->blink  = cur;

  return 1;

}

int addChildCrawlNode (
  crawlListPtr listHead,
  parentListPtr curParentList,
  char *parentName,
  char *fname,
  int numMacros,
  char **symbols,
  char **values
) {

crawlListPtr cur;
parentNodePtr pNode, pNodeNew;
int i;
char name[1023+1];

  //printf( "addChildCrawlNode, fname = [%s], parent = [%s]\n",
  // fname, parentName );

  removeExt( name, fname, 1023 );

  // =====================================================
  // name may not be in parent list but it may be the same as parent

  pNode = curParentList->head->flink;
  while ( pNode ) {
    if ( strcmp( name, pNode->parentName ) == 0 ) {
      if ( verbose ) {
        fprintf( stderr, "displayCrawlerInfo: Discarding reverse self reference to [%s]\n",
         name );
      }
      return 0;
    }
    pNode = pNode->flink;
  }

  // =====================================================

  cur = new crawlListType;
  //cur->fname = fname;
  cur->fname = new char[strlen(name)+1];
  strcpy( cur->fname, name );
  cur->numMacros = numMacros;
  cur->symbols = symbols;
  cur->values = values;

  cur->crc = 0;
  for ( i=0; i<numMacros; i++ ) {
    if ( symbols[i] && values[i] ) {
      cur->crc = updateCRC( cur->crc, symbols[i], strlen(symbols[i]) );
      cur->crc = updateCRC( cur->crc, values[i], strlen(values[i]) );
    }
  }

  // =====================================================
  // update list of parents

  cur->pList.head = new parentNodeType;
  cur->pList.tail = cur->pList.head;
  cur->pList.tail->flink = NULL;

  // copy prev
  pNode = curParentList->head->flink;
  while ( pNode ) {
    pNodeNew = new parentNodeType;
    pNodeNew->parentName = new char[strlen(pNode->parentName)+1];
    strcpy( pNodeNew->parentName, pNode->parentName );
    cur->pList.tail->flink = pNodeNew;
    cur->pList.tail = pNodeNew;
    cur->pList.tail->flink = NULL;
    pNode = pNode->flink;
  }

  // add new parent name
  pNodeNew = new parentNodeType;
  pNodeNew->parentName = new char[strlen(parentName)+1];
  strcpy( pNodeNew->parentName, parentName );
  cur->pList.tail->flink = pNodeNew;
  cur->pList.tail = pNodeNew;
  cur->pList.tail->flink = NULL;

  // =====================================================

  cur->blink = listHead->blink;
  listHead->blink->flink = cur;
  cur->flink = listHead;
  listHead->blink  = cur;

  return 1;

}

static void showParents (
  crawlListPtr curNode
) {

parentNodePtr pNode;

  pNode = curNode->pList.head->flink;
  while ( pNode ) {
    fprintf( stderr, "[%s]-->", pNode->parentName );
    pNode = pNode->flink;
  }

}

static void displayNode (
  nameListPtr node
) {

char pvName[1023+1];

  if ( !node ) return;

  strncpy( pvName, node->pvAndFileName, node->startOfFileName );
  pvName[node->startOfFileName] = 0;

  printf( "displayCrawler: file=\"%s\",pv=\"%s\"\n",
   &node->pvAndFileName[node->startOfFileName], pvName );

  fflush( NULL );

}

int crawlEdlFiles (
  appContextClass *appCtx,
  crawlListPtr listHead
) {

char *pv;
activeGraphicListPtr cur;
objBindingClass obj;
activeWindowClass *actWin = NULL;
expStringClass expStr, finalExpStr, nameExpStr, finalNameExpStr,
 macExpStr, finalMacExpStr;
activeGraphicClass *muxNode;
crawlListPtr curNode;
int i, ii, status, printSingleValue;
int numMuxes, numSets;
int numMuxMacros;
char **muxSymbols, **muxValues;
int numRelatedDisplays;
char *relatedDisplayName, *relatedDisplayMacros;
char propagateString[16], replaceString[16];

crawlListPtr childCrawlNodeList;
char *childFname;
int childNumMacros, totalMacros;
char **childSymbols, **childValues;

nameListPtr node;
int duplicate;

int c;

  if ( !pvAndFileNameTree ) {
    status = avl_init_tree( compare_nodes, compare_key, copy_node,
     &pvAndFileNameTree );
  }

  if ( empty( listHead ) ) return 1;

  disableBadWindowErrors( 1 );

  // init
  //initCrawlList( &childCrawlNodeList );

  // ---------------------------------------------------------------
  // process each file

  curNode = listHead->flink;
  while ( curNode != listHead ) {

    if ( !displayHasBeenProcessed( curNode ) ) {

      processAllEvents( appCtx->appContext(), appCtx->getDisplay() );

      if ( verbose ) {
        fprintf( stderr, "displayCrawlerProgress: Crawling " );
        showParents( curNode );
        fprintf( stderr, "[%s] (%-d)\n", curNode->fname, depth );
      }

      recordDisplay( curNode );

      actWin = new activeWindowClass;
      actWin->create( appCtx, NULL, 0, 0, 0, 0, 0, NULL, NULL );
      actWin->ci = &appCtx->ci;
      actWin->fi = &appCtx->fi;
      actWin->drawGc.create( actWin->drawWidget );
      actWin->drawGc.setCI( actWin->ci );
      status = actWin->createNodeForCrawler( appCtx, curNode->fname );

      if ( status & 1 ) {

        // first pass: count muxes and get mux macros

        numMuxes = 0;
        muxNode = NULL;

        cur = actWin->head->flink;
        while ( cur != actWin->head ) {

          if ( cur->node->isMux() ) {
            numMuxes++;
            muxNode = cur->node;
          }

          cur = cur->flink;

        }

        if ( numMuxes == 1 ) {
          numSets = muxNode->getNumMacroSets();
        }
        else {
          numSets = 0;
        }

	c = 0;

        // second pass: get all pv names

        cur = actWin->head->flink;
        while ( cur != actWin->head ) {

          pv = cur->node->crawlerGetFirstPv();
          while ( pv ) {

            expStr.setRaw( pv );

            printSingleValue = 1;

            for ( i=0; i<numSets; i++ ) {

              if ( expStr.containsPrimaryMacros() ) {

                expStr.expand1st( curNode->numMacros, curNode->symbols,
                 curNode->values );

                if ( expStr.containsSecondaryMacros() ) {

                  muxNode->getMacrosSet( &numMuxMacros, &muxSymbols,
                   &muxValues, i );

                  expStr.expand2nd( numMuxMacros, muxSymbols, muxValues );

                  finalExpStr.setRaw( expStr.getExpanded() );

                  if ( !blank(finalExpStr.getExpanded()) ) {
                    if ( !finalExpStr.containsPrimaryMacros() ) {
                      printSingleValue = 0;
		      node = new nameListType;
                      node->pvAndFileName =
                       new char[strlen(actWin->fileNameForSym)+
                                strlen(finalExpStr.getExpanded())+10];
                      strcpy( node->pvAndFileName, finalExpStr.getExpanded() );
                      node->startOfFileName = strlen(node->pvAndFileName);
                      strcat( node->pvAndFileName, actWin->fileNameForSym );
                      status = avl_insert_node( pvAndFileNameTree, (void *) node,
                       &duplicate );
                      if ( duplicate ) {
                        delete[] node->pvAndFileName;
                        delete node;
			totalDups++;
                      }
		      else {
			totalRefs++;
                        displayNode( node );
		      }
                    }
                  }

                }

              }

            }

            if ( printSingleValue ) {

              expStr.expand1st( curNode->numMacros, curNode->symbols,
                 curNode->values );

              if ( !blank(expStr.getExpanded()) ) {
                if ( !expStr.containsSecondaryMacros() ) {
                  node = new nameListType;
                  node->pvAndFileName =
                   new char[strlen(actWin->fileNameForSym)+
                            strlen(expStr.getExpanded())+10];
                  strcpy( node->pvAndFileName, expStr.getExpanded() );
                  node->startOfFileName = strlen(node->pvAndFileName);
                  strcat( node->pvAndFileName, actWin->fileNameForSym );
                  status = avl_insert_node( pvAndFileNameTree, (void *) node,
                   &duplicate );
                  if ( duplicate ) {
                    delete[] node->pvAndFileName;
                    delete node;
		    totalDups++;
                  }
		  else {
		    totalRefs++;
                    displayNode( node );
		  }
                }
              }

            }

            pv = cur->node->crawlerGetNextPv();

          }

          cur = cur->flink;

        }

        // third pass: get all related display info

        cur = actWin->head->flink;
        while ( cur != actWin->head ) {

          if ( cur->node->isRelatedDisplay() ) {

            numRelatedDisplays = cur->node->getNumRelatedDisplays();

            for ( ii=0; ii<numRelatedDisplays; ii++ ) {

              if ( cur->node->getRelatedDisplayProperty( ii, "propagate" ) ) {
                strcpy( propagateString, "propagate" );
              }
              else {
                strcpy( propagateString, "nopropagate" );
              }

              if ( cur->node->getRelatedDisplayProperty( ii, "replace" ) ) {
                strcpy( replaceString, "replace" );
              }
              else {
                strcpy( replaceString, "append" );
              }

              relatedDisplayName = cur->node->getRelatedDisplayName( ii );
              relatedDisplayMacros = cur->node->getRelatedDisplayMacros( ii );

              nameExpStr.setRaw( relatedDisplayName );
              macExpStr.setRaw( relatedDisplayMacros );

              printSingleValue = 1;

              for ( i=0; i<numSets; i++ ) {

                printSingleValue = 0;

                nameExpStr.expand1st( curNode->numMacros, curNode->symbols,
                 curNode->values );

                muxNode->getMacrosSet( &numMuxMacros, &muxSymbols,
                 &muxValues, i );

                nameExpStr.expand2nd( numMuxMacros, muxSymbols, muxValues );

                finalNameExpStr.setRaw( nameExpStr.getExpanded() );

                macExpStr.expand1st( curNode->numMacros, curNode->symbols,
                 curNode->values );

                muxNode->getMacrosSet( &numMuxMacros, &muxSymbols,
                 &muxValues, i );

                macExpStr.expand2nd( numMuxMacros, muxSymbols, muxValues );

                finalMacExpStr.setRaw( macExpStr.getExpanded() );

                if ( !finalNameExpStr.containsPrimaryMacros() &&
                     !finalMacExpStr.containsPrimaryMacros() ) {

                  buildNewMacroList( finalMacExpStr.getExpanded(), replaceString,
                   propagateString, curNode->numMacros, curNode->symbols,
                   curNode->values, &childNumMacros, &childSymbols, &childValues,
                   &totalMacros );

                  childFname = new char[strlen(finalNameExpStr.getExpanded())+1];
                  strcpy( childFname, finalNameExpStr.getExpanded() );

		  // ?????????????????????
                  // init
                  initCrawlList( &childCrawlNodeList );

                  addChildCrawlNode( childCrawlNodeList, &(curNode->pList),
                   curNode->fname, childFname, childNumMacros,
                   childSymbols, childValues );

		  // ?????????????????????
                  // call recursively
                  depth++;
                  if ( depth <= maxCrawlDepth ) {
                    status = crawlEdlFiles( appCtx, childCrawlNodeList );
                  }
                  else {
                    fprintf( stderr, "displayCrawlerStatus: Crawler max depth (%-d) exceeded\n",
                     maxCrawlDepth );
                  }
                  destroyCrawlList( &childCrawlNodeList );
                  if ( depth > 0 ) depth--;

		  destroyMacros( totalMacros, childSymbols, childValues );

		  delete[] childFname;

                }

              }

              if ( printSingleValue ) {

                nameExpStr.expand1st( curNode->numMacros, curNode->symbols,
                 curNode->values );

                macExpStr.expand1st( curNode->numMacros, curNode->symbols,
                 curNode->values );

                if ( !nameExpStr.containsSecondaryMacros() &&
                     !macExpStr.containsSecondaryMacros() ) {

                  buildNewMacroList( macExpStr.getExpanded(), replaceString,
                   propagateString, curNode->numMacros, curNode->symbols,
                   curNode->values, &childNumMacros, &childSymbols, &childValues,
                   &totalMacros );

                  childFname = new char[strlen(nameExpStr.getExpanded())+1];
                  strcpy( childFname, nameExpStr.getExpanded() );

		  // ?????????????????????
                  // init
                  initCrawlList( &childCrawlNodeList );

                  addChildCrawlNode( childCrawlNodeList, &(curNode->pList),
                   curNode->fname, childFname, childNumMacros,
                   childSymbols, childValues );

		  // ?????????????????????
                  // call recursively
                  depth++;
                  if ( depth <= maxCrawlDepth ) {
                    status = crawlEdlFiles( appCtx, childCrawlNodeList );
                  }
                  else {
                    fprintf( stderr, "displayCrawlerStatus: Crawler max depth (%-d) exceeded\n",
                     maxCrawlDepth );
                  }
                  destroyCrawlList( &childCrawlNodeList );
                  if ( depth > 0 ) depth--;

		  destroyMacros( totalMacros, childSymbols, childValues );

		  delete[] childFname;

                }

              }

            }

            pv = cur->node->crawlerGetNextPv();

          }

          cur = cur->flink;

        }

      }

      // ---------------------------------------------------------------

      if ( actWin ) {
        delete actWin;
        actWin = NULL;
      }

      if ( verbose ) {
        fprintf( stderr, "displayCrawlerInfo: %-d hit(s), %-d duplicate(s)\n", totalRefs, totalDups );
      }

    }

    curNode = curNode->flink;

  }

  // call recursively
  //depth++;
  //if ( depth <= maxCrawlDepth ) {
  //  status = crawlEdlFiles( appCtx, childCrawlNodeList );
  //}
  //else {
  //  fprintf( stderr, "displayCrawlerStatus: Crawler max depth (%-d) exceeded\n", maxCrawlDepth );
  //}
  //if ( depth > 0 ) depth--;

  disableBadWindowErrors( 0 );

  return 1;

}

int displayCrawlerResults ( void ) {

nameListPtr node;
int status;
char pvName[1023+1];

  return 1;

  status = avl_get_first( pvAndFileNameTree, (void **) &node );
  while ( node ) {

    strncpy( pvName, node->pvAndFileName, node->startOfFileName );
    pvName[node->startOfFileName] = 0;

    printf( "displayCrawler: file=\"%s\",pv=\"%s\"\n",
     &node->pvAndFileName[node->startOfFileName], pvName );

    //printf( "epicsDisplayCrawler: insert into table values (\"%s\",\"%s\");\n",
    // &node->pvAndFileName[node->startOfFileName], pvName );

    status = avl_get_next( pvAndFileNameTree, (void **) &node );

  }

  return 1;

}
