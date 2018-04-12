/* FILENAME:  myindex.c
   AUTHOR:    JG STINSTRA
   CONTENTS:  Tools for dealing with index vectors
   LAST UPDATE: 4 JULY 2003 
*/

#ifndef MYINDEX_C
#define MYINDEX_C 1

#include "myindex.h"
#include "myerror.h"
 
int idxReadIMap(mxArray *array,mapdata **maparray,long *nummap) 
/* Complicated, hmmm maparray is a pointer to an array containing sets of data specifying the length and elements of a mapping vector array can be either an indexed vector directly or a cell array specifying multiple */
{
    mapdata	*datamap; 	/* for temperal storage so I do not need to put the star every time */
    long	numdata;  	/* idem */
    long	p,q; 		/* my favorite counters */  
    mxArray	*subarray; 	/* to disassemble the matrix structure of matlab */
    double 	*data; 		/* The datafield in the structure where the actual values are */

    *maparray = NULL;		/* defaults */
    *nummap = 0;
          
    if( mxIsCell(array) ) 	/* are there more than one map defined */
    {
	numdata = mxGetN(array)*mxGetM(array); /* how big is the array */
	
	if (!(datamap = (mapdata *)mxCalloc(numdata,sizeof(mapdata)))) /* Allocate memory to store data */
	{
            errError("idxReadIMap","Could not allocate enough memory to store map");
            return(0);
        }    
	    
	for (p=0;p<numdata;p++)
	{  /* go and take the cell array apart */
	    if( !(subarray = mxGetCell(array,p)))  					/* get the data array */
            {
                errError("idxReadIMap","Could not get element from cell-array");
                idxFreeIMap(datamap,numdata);
                return(0);
            }    

	    if ((mxGetN(subarray) != 1)&&(mxGetM(subarray) != 1))			/* Check for dimensions */
            {
                errError("idxReadIMap","Index maps should be one dimensional");
                idxFreeIMap(datamap,numdata);
                return(0);
            }

	    datamap[p].num = mxGetN(subarray)*mxGetM(subarray); 				/* What is one of its dimensions */
	    if ( !(datamap[p].map = (long *)mxCalloc(datamap[p].num,sizeof(long))))	
            {
                errError("idxReadIMap","Could not allocate enough memory");
                idxFreeIMap(datamap,numdata);
                return(0);
            }    
    
            data = mxGetPr(subarray);							/* copy and convert the data to be stored in a C-matrix */
	    for (q=0;q<datamap[p].num;q++)
            { 
                /* loop to store an convert data into longs and check as well for chronological ordering */
                datamap[p].map[q] = (long) data[q]; /* store the data */
            }   	    
        }
	/* data is loaded so nothing to do more */ 
    }
    else if ( mxIsNumeric(array) ) /* just one array is defined */
    { 
	numdata = 1;			/* just one field to read */
		
	if (!(datamap = (mapdata *)mxCalloc(1,sizeof(mapdata)))) 				/* Allocate memory to store data */
        {
	    errError("idxReadIMap","Could not allocate enough memory to store map");
            return(0);
        }    
				
	if ((mxGetN(array) != 1)&&(mxGetM(array) != 1))				/* Check its dimensions */
	{
            errError("idxReadIMap","The map should be one dimensional");    
            idxFreeIMap(datamap,numdata);
	    return(0);
        }

	datamap[0].num = mxGetN(array)*mxGetM(array); 				/* what is one its dimensions */
	if ( !(datamap[0].map = (long *)mxCalloc(datamap[0].num,sizeof(long))))	
        {
	    errError("idxReadIMap","Could not allocate enough memory to store map");
            idxFreeIMap(datamap,numdata);
            return(0);
        }    
            
	data = mxGetPr(array);
		
	for (q=0;q<datamap[0].num;q++)
	{ /* loop to store an convert data into longs and check as well for chronological ordering */
	    datamap[0].map[q] = (long) data[q]; /* store the data */
	}
    }
    else
    {
	errError("idxReadIMap","Could you specify a cell array or a normal array for the mapping data");
        return(0);
    }
    /* copy output to output parameters */
    *maparray = datamap;
    *nummap = numdata;
    return(1);
}

void idxFreeIMap(mapdata *maparray,long nummap)
{
    long q;
    
    if (maparray)
    {
        for (q=0;q<nummap;q++)
        {
            if (maparray[q].map) mxFree(maparray[q].map);
        }
        mxFree(maparray);
    }
}

void idxFreeDMap(dmapdata *maparray,long nummap)
{
    long q;
    
    if (maparray)
    {
        for (q=0;q<nummap;q++)
        {
            if (maparray[q].map) mxFree(maparray[q].map);
        }
        mxFree(maparray);
    }
}

 
int idxReadDMap(mxArray *array,dmapdata **maparray,long *nummap) 
/* Same as idxReadIMap but then for doubles and without the option of a cell array */
{
    dmapdata	*datamap; 	/* for temperal storage so I do not need to put the star every time */
    long	numdata,n;  	/* idem */
    long	q; 		/* my favorite counters */  
    double 	*data; 		/* The datafield in the structure where the actual values are */
    
    *maparray = NULL;
    *nummap = 0;      
                      
    if ( mxIsNumeric(array) ) /* just one array is defined */
    { 
	numdata = 1;			/* just one field to read */
		
	if (!(datamap = (dmapdata *)mxCalloc(1,sizeof(dmapdata)))) 				/* Allocate memory to store data */
        {
	    errError("idxReadDMap","could not allocate enough memory to store map");
            return(0);
        }
				
	if ((mxGetN(array) != 1)&&(mxGetM(array) != 1))				/* Check its dimensions */
	{
            datamap[0].num = mxGetM(array);
            datamap[0].numcol = mxGetN(array);
            n = datamap[0].num*datamap[0].numcol;
        }
	else
        {
            datamap[0].num = mxGetN(array)*mxGetM(array); 				/* what is one its dimensions */
            datamap[0].numcol = 1;
            n = datamap[0].num;
        }
            
	
        if ( !(datamap[0].map = (double *)mxCalloc(n,sizeof(double))))	
        {
	    errError("idxReadDMap","Could not allocate enough memory to store map");
            idxFreeDMap(datamap,numdata);
            return(0);
        }
            
	data = mxGetPr(array);
		
	for (q=0;q<n;q++)
	{ /* loop to store an convert data into longs and check as well for chronological ordering */
	    datamap[0].map[q] = (double) data[q]; /* store the data */
	}
    }
    else
    {
	errError("idxReadDMap","Could you specify a cell array or a normal array for the mapping data");
        return(0);
    }
    /* copy output to output parameters */
    *maparray = datamap;
    *nummap = numdata;
    return(1);
}


int idxConvertIndices(mxArray *indexarray,long **indices, long *numindices)
/* Convert an array containing indices into C, the data is written back into indices and numindices */
{
    double	*data;
    long	*tempindices;
    long	p; /* counter */
    
    *indices = NULL;
    *numindices = 0;

    *numindices = (long) (mxGetN(indexarray)*mxGetM(indexarray));
    if (!(tempindices = (long *) mxCalloc(*numindices,sizeof(long)))) return(0);
        
    if (!( data = mxGetPr(indexarray)))
    {
        mxFree(tempindices);
	return(0);    
    }
    for (p=0;p<*numindices;p++) tempindices[p] = (long) data[p];
    *indices = tempindices;
    return(1);
}
    
int idxCreateIndices(mxArray *indexarray,long **indices, long *numindices)
/* Create a number of indices for the current array */
{
    long	p; /* counter */
    long	*tempindices; 
    
    *numindices = (long) (mxGetN(indexarray)*mxGetM(indexarray));
    if(!(tempindices = (long *) mxCalloc(*numindices,sizeof(long)))) return(0);
    for (p=0;p<*numindices;p++) tempindices[p] = (long) p+1;
    *indices = tempindices;

    return(1); /* success */
}

#endif
