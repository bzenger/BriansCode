/* FILENAME:  mexRewriteTSDF.c 
   AUTHOR: Jeroen Stinstra
   CONTENTS: File for rewriting a TSDF-file 
              Rewriting uses the original file and copies all the non-specified data from the old-file in the new one 
   LAST UPDATE: 4 JUN 2002
*/


/***************************
  HERE THE CODE STARTS
 ***************************/

/* Do the standard includes */

#include <stdio.h>
#include <math.h>
#include <mex.h>

#include "graphicsio.h"
#include "mexio.h"
#include "mexio.c"


/* Main entry function description starts here */

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
/* Entry point for MATLAB. 
    nlhs - number of parameters on leftside hand of expression
    plhs - a pointer to an array of pointers representing the various arrays on this side
    nrhs - idem right hand side
    prhs - idem array pointer right hand side
*/
{
    char	*filename;
    char	*sourcefilename;
    long	numindices;
    long	*indices;
    long	numtsdfindices;
    long	*tsdfindices;
    long	p;
  
    mxArray	*filenamearray, *tsarray, *optionsarray, *sourcefilenamearray, *tsdfindexarray;
    mxArray	*array;
    int		arrayindex;
    
    char	*expid,*text;
    float	*potvals;
    long	numframes,numleads;
    char	*geomfile, *label, *pakfile;
    long	unit,format;
    long	numleadinfo, *leadinfo;
    long	numts;
    long 	assoc,surfacenum;
    long	numtsarray;
    
    rewriteQueue	*rewritequeue; /* queue with all rewrites */
    rewriteRequest	rewriterequest; /* storage room for each request */
        
  /*****************************
    Start by scanning the input
   *****************************/
    /* Set usage */
    mexioError(NULL,NULL,"mexReriteTSDF(filename,TSindices or TS-structure,[sourcefilename],[tsdf-indices],[options])");
   
    if (nrhs > 5)
        mexioError("mexFunction","No more than three inputs allowed",NULL);
    if (nrhs < 2)
        mexioError("mexFunction","You have to specify a filename and a source TS",NULL);
   
   
    filenamearray = (mxArray *)prhs[0];
    if (!(mxIsChar(filenamearray)))
        mexioError("mexFunction","First parameter should be a filename",NULL);
            
    tsarray = (mxArray *)prhs[1];        
    if ((!mxIsCell(tsarray))&&(!mxIsStruct(tsarray))&&(!mxIsNumeric(tsarray)))
        mexioError("mexFunction","Second input should be a cell array, a struct or an indexed array",NULL);

    optionsarray = NULL;
    sourcefilenamearray = NULL;
    tsdfindexarray = NULL;
        
    if (nrhs > 2)
    {
        for (p=2;p<nrhs;p++)
        {
            if(mxIsStruct(prhs[p]))
            {
                optionsarray = (mxArray *)prhs[p];
            }
            if(mxIsChar(prhs[p]))
            {
                sourcefilenamearray = (mxArray *)prhs[p];
            }
            if(mxIsDouble(prhs[p]))
            {
                tsdfindexarray = (mxArray *)prhs[p];
            }
        }
    }    

    filename = NULL;
    sourcefilename = NULL;

    filename = mexioConvertString(filenamearray);
    if (sourcefilenamearray) sourcefilename = mexioConvertString(sourcefilenamearray);
    
    numtsdfindices = 0; /* intialise with nothing there */
    tsdfindices = NULL; 
    
    if (tsdfindexarray)
    {
        mexioConvertIndices(tsdfindexarray,&tsdfindices,&numtsdfindices);
    }
    
    if (numtsdfindices == 0) mexioCreateIndices(tsarray,&tsdfindices,&numtsdfindices); /* generates an index vector the size of tsarray */
    
    
    if (mxIsNumeric(tsarray)) 
    {   
        /* Go and get the TS-global */
        mexioConvertIndices(tsarray,&indices,&numindices);
        tsarray = mexioFetchTS();
    }
    else
    {
        mexioCreateIndices(tsarray,&indices,&numindices); /* generate a set of indices for this array */
    }
    
    /* all input has been read */
    
           
    if (numtsdfindices != numindices)
        mexioError("mexFunction","TSindices should equal tsdfindices",NULL);
        /* if not we cannot do a one to one mapping */
                    
    /* initiate rewriterequest */
    
    rewriterequest.dataType = 0; /* fill out for each individual rewriterequest field */
    rewriterequest.surfaceNumber = 0;
    rewriterequest.index = 0;
    rewriterequest.valueType = 0;
    rewriterequest.quantity = 0;
    rewriterequest.theDimension = 0;
    rewriterequest.dataPointer = NULL;
    rewriterequest.callBackRoutine = NULL; 
    
    /* now write the file */
    
    for (p=0;p<numindices;p++)
    {
        if (mxIsStruct(tsarray)) 
        {
            array = tsarray;
            arrayindex = p;
        }
        else
        {   
            numtsarray = mxGetN(tsarray)*mxGetM(tsarray);
            if (indices[p] > numtsarray)
                mexioError("mexFunction","You tried to save non-existing timeseries",NULL);
            array = mxGetCell(tsarray,(indices[p]-1));
            arrayindex = 0;
        }
        
        if (p==0)
        {
            /* Create a rewrite list */
            
            initrewrite(&rewritequeue); /* list started */
                                                                      
            /* if no name is given retrieve the filename from the TS-structure */                                                          
            if (sourcefilename == NULL) 
            {
                sourcefilename = mexioGetTSstring(array,0,"filename");
                if (sourcefilename == NULL) mexioError("mexFuncion","You should specify a sourcefilename",NULL);
            }
            /* get number of series in the original file */
            
            numts = mexioGetNumTS(sourcefilename);
            
            if ( tsdfindices[p] > numts) mexioWarning("mexFunction","Cannot store additional timeseries, Grapicsio does not support this option");
                                                                            
            if (expid = mexioGetTSstring(array,arrayindex,"expid"))
            {   
                rewriterequest.dataType = EXPID;
                rewriterequest.dataPointer = expid;
                if ( addrewriterequest(rewritequeue,&rewriterequest))
                    mexioError("mexFunction","could not add rewriterequest (EXPID)",NULL);
            }
                        
            if (text = mexioGetTSstring(array,arrayindex,"text"))
            {
                rewriterequest.dataType = TEXT;
                rewriterequest.dataPointer = text;
                if ( addrewriterequest(rewritequeue,&rewriterequest))
                    mexioError("mexFunction","could not add rewriterequest (TEXT)",NULL);
            }
            
           /* GRAPHICSIO DOES NOT SUPPORT REWRITING THE AUDIT STRING, YET
               IT'S ODD BUT THERE IS NO WAY OF DOING AN AUDIT REWRITE */
            
            mexioWarning("mexFunction","Could not write auditstring (option not supported by graphicsio !!)");    
                        
            /* header is written now do the individual timeseries */          
        }    
        
        /* set the pointer of the timeseries, we are gonna write */
            
        if (geomfile = mexioGetTSstring(array,arrayindex,"geomfile"))
        {
            rewriterequest.dataType = TIMESERIESGEOMFILE;
            rewriterequest.dataPointer = geomfile;
            rewriterequest.index = tsdfindices[p];
            
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (GEOMFILE)",NULL);
        }    
            
        if (label = mexioGetTSstring(array,arrayindex,"label"))
        {
            rewriterequest.dataType = TIMESERIESLABEL;
            rewriterequest.dataPointer = label;
            rewriterequest.index = tsdfindices[p];
            
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (LABEL)",NULL);
        }    
        
        if (pakfile = mexioGetTSstring(array,arrayindex,"pakfile"))
        {
            rewriterequest.dataType = TIMESERIESFILE;
            rewriterequest.dataPointer = pakfile;
            rewriterequest.index = tsdfindices[p];
            
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (PAKFILE)",NULL);
        }    
        
        
        if ( mexioGetTSLong(array,arrayindex,"format",&format))
        {
            rewriterequest.dataType = TIMESERIESFORMAT;
            rewriterequest.dataPointer = mexioStoreLong(format); /* don't bother where it is stored, matlab will do the cleanup anyway */
            rewriterequest.index = tsdfindices[p];
            
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (FORMAT)",NULL);
        }    
                       
        if ( mexioGetTSunit(array,arrayindex,&unit))
        {
            rewriterequest.dataType = TIMESERIESUNITS;
            rewriterequest.dataPointer = mexioStoreLong(unit); /* don't bother where, matlab will do the cleanup anyway */
            rewriterequest.index = tsdfindices[p];
            
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (UNIT)",NULL);
        }         
          
        if ( mexioGetTSpotvals(array,arrayindex,&potvals,&numleads,&numframes))            
        {
            rewriterequest.dataType = TIMESERIESSPECS;
            rewriterequest.dataPointer = mexioStoreLongs(numleads,numframes); /* two longs are stored in mem */ 
            rewriterequest.index = tsdfindices[p];
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (SPECS)",NULL);
            
            rewriterequest.dataType = TIMESERIESDATA;
            rewriterequest.dataPointer = potvals;
            rewriterequest.index = tsdfindices[p];
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (DATA)",NULL);
        }     
        
        if ( mexioGetTSleadinfo(array,arrayindex,&leadinfo,&numleadinfo))
        {
            rewriterequest.dataType = NUMCORRECTEDLEADS;
            rewriterequest.dataPointer = mexioStoreLong(numleadinfo); 
            rewriterequest.index = tsdfindices[p];
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (NUMCORRECTEDLEADS)",NULL);
            
            rewriterequest.dataType = CORRECTEDLEADS;
            rewriterequest.dataPointer = leadinfo;
            rewriterequest.index = tsdfindices[p];
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (CORRECTEDLEADS)",NULL);         
        }    
        
        if ( mexioGetTSLong(array,arrayindex,"surfacenum",&surfacenum))
        {
            rewriterequest.dataType = TIMESERIESSURFACE;
            rewriterequest.dataPointer = mexioStoreLong(surfacenum); /* don't bother where it is stored, matlab will do the cleanup anyway */
            rewriterequest.index = tsdfindices[p];
            
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (SURFACENUM)",NULL);
        }
        
        if ( mexioGetTSLong(array,arrayindex,"assoc",&assoc))
        {
            rewriterequest.dataType = TIMESERIESASSOC;
            rewriterequest.dataPointer = mexioStoreLong(assoc); /* don't bother where it is stored, matlab will do the cleanup anyway */
            rewriterequest.index = tsdfindices[p];
            
            if ( addrewriterequest(rewritequeue,&rewriterequest))
                mexioError("mexFunction","could not add rewriterequest (ASSOC)",NULL);
        }
        
    } /* end central loop */
    
   /* rewrite the file */   
   rewritefile(rewritequeue,sourcefilename,filename);
   /* fin */
}




