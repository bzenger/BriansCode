/* FILENAME:  myindex.h
   AUTHOR:    JG STINSTRA
   CONTENTS:  Tools for dealing with index vectors
   LAST UPDATE: 4 JULY 2003 
*/

#ifndef MYINDEX_H 
#define MYINDEX_H 1

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <mex.h>
#include "myerror.h"

typedef struct {
    long num;	/* how many channels/leads/frames need to be mapped */
    long *map; 	/* Containing an entry for each frame/channel/lead to where they are mapped */
} mapdata;

typedef struct {
    long num;	/* how many channels/leads/frames need to be scaled */
    double *map; 	/* Containing an entry for each scale to where they are mapped */
    long numcol;
} dmapdata;
 
int  idxReadIMap(mxArray *array,mapdata **maparray,long *nummap); 
void idxFreeIMap(mapdata *maparray,long nummap);
void idxFreeDMap(dmapdata *maparray,long nummap);
int  idxReadDMap(mxArray *array,dmapdata **maparray,long *nummap);

int  idxConvertIndices(mxArray *indexarray,long **indices, long *numindices); 	/* Convert index arrays */
int  idxCreateIndices(mxArray *indexarray,long **indices, long *numindices);	/* Create a set of indices */

#endif
