/*  FILENAME: 	mexListDFC.c 
    AUTHOR:	JG STINSTRA
    
    CONTENTS:   This mexfile uses gdbm to read dfc-files it requires a filename as input and returns the keys that are stored
		in this file. As both DFC and TSDFC files have the same structure the function works for both

    LAST UPDATE: 4 JUN 2002
*/


/* Do some standard includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mex.h"
#include "gdbm.h"
#include "myerror.c"
#include "misctools.c"


/* Function declarations  */

/********* Functions start here **********************/

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
/* Entry point for MATLAB. 
    nlhs - number of parameters on leftside hand of expression
    plhs - a pointer to an array of pointers representing the various arrays on this side
    nrhs - idem right hand side
    prhs - idem array pointer right hand side
*/
{
    char		*filename;
    
    mxArray		*keylist;
    mxArray		*stringarray;
    char		**keyblock;
    char		**newkeyblock;
    char		**firstkeyblock;
			/* kind of complicated but at least it can read an unlimited amount of data */
    char		*keystring;
    int			blocksize = 30;	/* to test whether code works */
    int			numkeys;
    int 		p,q;
    int                 success;    
    
    datum		key,nextkey;
    GDBM_FILE		gdbmfile;

    /* Set usage string */
    errUsage("keys = mexListDFC(filename)");
    /* INPUT */
    
    if (nrhs != 1)
    {
	errError("mexFunction","None or more than one input argument is allowed");
	mexErrMsgTxt("ERROR\n");
	return;
    }
	
    if (!mxIsChar(prhs[0]))
    {
        errError("mexFunction","filename needs to be a string");
	mexErrMsgTxt("ERROR\n");
	return;
    }

    /* OUTPUT */

    if (nlhs < 0) return;
	/* If no output is requested, don't do any work */
    
    if (nlhs > 1)
    {
	errError("mexFunction","Only one output argument is needed");
	mexErrMsgTxt("ERROR\n");
	return;
    }
	
    /* Retrieve the filename */

    if(!(filename = miscConvertString((mxArray *)prhs[0])))
    {
        errError("mexFunction","Could not obtain filename");
	mexErrMsgTxt("ERROR\n");
	return;
    }
    /* So the filename is done */	
 
    /* the number of keys unknown. So the program reserves space to store a couple of keys
       in one block or to be more precise to store a pointer to the string */
       
    if ( !(firstkeyblock = (char **) mxCalloc((blocksize+1),sizeof(char *))))
    {
        mxFree(filename);
        errError("mexFunction","Could not allocate memory");
	mexErrMsgTxt("ERROR\n");
	return;
    }
	    /* +1 to store a pointer in to the next block of data */
	    
    /* Open the gdbm file for reading */
    if( !(gdbmfile = gdbm_open(filename,1024, GDBM_READER|GDBM_NOLOCK, 00644, 0)))
    {
        mxFree(firstkeyblock);
        mxFree(filename);
	errError("mexFunction","Could not open file");
	mexErrMsgTxt("ERROR\n");
	return;
    }
    
    key = gdbm_firstkey(gdbmfile);
    numkeys=0; p=0;
    success = 1;
   
    keyblock = firstkeyblock; /* Start filling her up */
    while (key.dptr) 
    {
    /* Read every key from the file and store them in the keyblocks
       the keyblocks can store multiple keys for efficiency reasons
       when a block is full a new one is create and the pointer of this
       new one is stored at the end of the old one.
       As no complicated lists are needed this one just links from the first to the last 
       and not vice versa, hence a pointer to the first block needs to be stored at all time */
                
	    if( !(keystring = (char *) mxCalloc((key.dsize+1),sizeof(char))))
	      {  
		errError("mexFunction","Could not obtain enough memory for string");
		success = 0;
		break;
	      }
	    memcpy(keystring, key.dptr, key.dsize);
	    
	    if ( p == blocksize)  /* a new block of keys has to be generated */
	    {
	    /* generate a new block, put the pointer of the new block at the end of
	       the old one and get the new pointer and start filling from 0 again */
	       
	         if ( !(newkeyblock = (char **) mxCalloc((blocksize+1),sizeof(char *))))
		 {
		      free(key.dptr); /* do not leave a memory leak */
		      errError("mexFunction","Could not allocate memory");
		      success = 0;
		      break;
		 }  
		 keyblock[p] = (char *) newkeyblock; /* link the newblock to the last one */
		 keyblock = newkeyblock; /* go and use the new one, it is all empty */
		 p = 0; /* reset counter on keyblock entry */
            }		     
	    
	    keyblock[p] = keystring; /* put key in the block */
	    
	    nextkey = gdbm_nextkey(gdbmfile, key); /* go to the next key */
            free(key.dptr); /* since key memory was NOT allocated by matlab, I have to free it */
            key = nextkey;
	    numkeys++; p++;
    }
 

     gdbm_close(gdbmfile);
  
    /* Create an output array to store all keys that have been read */
    
    if (success)
    { 
        if ( !(keylist = mxCreateCellMatrix(numkeys,1)))
	{
	    errError("mexFunction","Could not create cell array");
	    success = 0;
	}
		    
    }
    
    keyblock = firstkeyblock;	/* reset block counter and start all over again but now retrieving the key from the blocks */	    
    for (p=0, q=0;p<numkeys;p++, q++)
    {
	if (q == blocksize)
	{
	    newkeyblock = (char **) keyblock[q];
	    mxFree(keyblock);
	    keyblock = newkeyblock;
	    q = 0;
	}
	if (success)
	{  
	    if ( !(stringarray = mxCreateString(keyblock[q])))
	    {  
		    errError("mexFunction","Could not create string");
		    mxDestroyArray(keylist);
		    success = 0;
            }
	    else
	    {  
	            mxSetCell(keylist,p,stringarray);	/* Connect the pieces */
	    }
        } 
	if (keyblock[q]) mxFree(keyblock[q]);
    }
    mxFree(keyblock);
			
    if (success) plhs[0] = keylist;
}
   
