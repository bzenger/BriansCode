/* 
 *  mexIsKeyTSDFC.c based   
 *
 *  Created by Jeroen Stinstra on Thu May 31 2002.
 *
 */
 
 /* 
   This function reads a TSDFC file and sees whether the key is there */
 
 
/* Do the standard includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mex.h>

#include "myerror.c"
#include "misctools.c"
#include "gdbm.h"




void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
/* Entry point for MATLAB. 
    lefthandside                        righthandside
    [outmat1,outmat2,...] = function(inmat1,inmat2,...)
    nlhs - number of parameters on leftside hand of expression
    plhs - a pointer to an array of pointers representing the various arrays on this side
    nrhs - idem right hand side
    prhs - idem array pointer right hand side
*/
{
    char		*tsdffilename, *tsdfcfilename;
    mxArray		*resultarray;
    GDBM_FILE 		gdbmfile;			/* the tsdfc file */
    datum		key;				/* the key in the tsdfc file */
    int			result;
    double		*dataarray;

    errUsage("result = mexIsKeyTSDFC(TSDFC-filename,TSDF-filename/key)"); /* no error, just setting usage once and for all */
    /* INPUT */
    
    if (nrhs != 2)
    {
	errError("mexFunction","The name of the TSDFC-file and the TSDF-file are needed");
	mexErrMsgTxt("ERROR\n");
	return;
    }

    if (!mxIsChar(prhs[0]))
    {
         errError("mexFunction","TSDFC-filename needs to be a string");
	 mexErrMsgTxt("ERROR\n");
	 return;
    }

    if (!mxIsChar(prhs[1]))
    {
         errError("mexFunction","TSDF-filename needs to be a string");
	 mexErrMsgTxt("ERROR\n");
    }

    /* OUTPUT */

    if (nlhs < 0) return;
	/* If no output is requested, don't do any work */
    
    if (nlhs > 1)
    {
	errError("mexFunction","Only two output arguments are generated");
	mexErrMsgTxt("ERROR\n");
	return;
    }
	
    /* Retrieve the filenames */

    tsdfcfilename = miscConvertString((mxArray *)prhs[0]);
    tsdffilename = miscConvertString((mxArray *)prhs[1]);
    
    /* So the filenames are read */	
 
    if (!( gdbmfile = gdbm_open(tsdfcfilename,1024,GDBM_READER|GDBM_NOLOCK,00644,0)))
    { 
          errError("mexFunction","Could not open file");
	  mxFree(tsdfcfilename);
	  mxFree(tsdffilename);
	  mexErrMsgTxt("ERROR\n");
	  return;
    }
    
    key.dptr = tsdffilename;
    key.dsize = strlen(tsdffilename);
 
    result = gdbm_exists(gdbmfile,key); /* search for the requested key */
    gdbm_close(gdbmfile);
 

    mxFree(tsdfcfilename);
    mxFree(tsdffilename);
 
    /* create a matrix and dump the result */
    if (!( resultarray = mxCreateDoubleMatrix(1,1,mxREAL)))
    {
         errError("mexFunction","Could not generate output array");
	 mexErrMsgTxt("ERROR\n");
         return;
    }

    dataarray = mxGetPr(resultarray);
    dataarray[0] = (double) result;
    
    plhs[0] = resultarray;
}


