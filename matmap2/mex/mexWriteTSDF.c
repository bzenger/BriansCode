/* FILENAME:  mexWriteTSDF.c 
   AUTHOR: Jeroen Stinstra
   CONTENTS: File for writing a TSDF-file (creating a new one fromscratch)
  
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
#include "tstools.c"
#include "myerror.c"
#include "myindex.c"
#include "misctools.c"

/* IN THE FUTURE DECLARATIONS AND STRUCTS SHOULD GO TO HEADER FILE */

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
    long	numindices;
    long	*indices;
    long	p;
  
    mxArray	*filenamearray, *tsarray, *optionsarray;
    mxArray	*array;
    int		arrayindex;
    FileInfoPtr	fip;
    
    char	*expid,*text,*audit;
    float	*potvals;
    long	numframes,numleads;
    long	assoc, surfacenum;
    char	*geomfile, *label, *pakfile;
    long	unit,format;
    long	numleadinfo, *leadinfo;
    long	numtsarray;
    int		success, loopsuccess;
    
    
  /*****************************
    Start by scanning the input
   *****************************/
    /* Set usage */
    errUsage("mexWriteTSDF(filename,TSindices or TS-structure,[options])");
   
    success 	= 0;
    filename 	= NULL;
    indices	= NULL;
    fip		= NULL;
    
    while(1)
    {
        if (nrhs > 3) { errError("mexFunction","No more than three inputs allowed"); break; }
        if (nrhs < 2) { errError("mexFunction","You have to specify a filename and a source TS"); break; }
   
        filenamearray = (mxArray *)prhs[0];
        if (!(mxIsChar(filenamearray))) { errError("mexFunction","First parameter should be a filename"); break; }
            
        tsarray = (mxArray *)prhs[1];        
        if ((!mxIsCell(tsarray))&&(!mxIsStruct(tsarray))&&(!mxIsNumeric(tsarray)))
                        { errError("mexFunction","Second input should be a cell array, a struct or an indexed array"); break; }
       
        if (nrhs > 2) 
        {
            optionsarray = (mxArray *)prhs[2];
            if (!(mxIsStruct(optionsarray))) { errError("mexFunction","Options should be a struct"); break; }
        }    

        if(!( filename = miscConvertString(filenamearray))) { errError("mexFunction","Could not convert string"); break; }
    
        if (mxIsNumeric(tsarray)) 
        {   
            /* Go and get the TS-global */
            if(!( idxConvertIndices(tsarray,&indices,&numindices))) { errError("mexFunction","Could not read TS indices"); break; }
            tsarray = tsFetchTS();
            if (tsarray == NULL) {errError("mexFunction","No access to global TS array"); break; }
        }
        else
        {
            if(!( idxCreateIndices(tsarray,&indices,&numindices)))  { errError("mexFunction","Could not create indices for array"); break; } 
            /* generate a set of indices for this array */
        }
    
        /* all input has been read */
        /* now write the file */
    
        loopsuccess = 0;
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
                if (indices[p] > numtsarray) { errError("mexFunction","You tried to save non-existing timeseries"); break; }
                array = mxGetCell(tsarray,(indices[p]-1));
                arrayindex = 0;
            }
        
            if (p==0)
            {
                /* Create a new file and write the file header */
                if (createfile_(filename,0,1,&fip))
                    { errError("mexFunction","Could not open file"); break; }
            
                if (tsGetTSstring(array,arrayindex,"expid",&expid))
                {  
                    if (expid)
                    {
                        if ( settext_(fip,expid) < 0) { errError("mexFunction","Could not write experiment id"); mxFree(expid); break; }
                        mxFree(expid);
                    }
                }
                
                if (tsGetTSstring(array,arrayindex,"text",&text))
                {
                    if (text)
                    {
                        if ( settext_(fip,text) < 0) { errError("mexFunction","Could not write text"); mxFree(text); break; }
                        mxFree(text);
                    }
                }
            
                if (tsGetTSstring(array,arrayindex,"audit",&audit))
                {
                    if (audit)
                    {
                        if ( setauditstring_(fip,audit) < 0) { errError("mexFunction","Could not write audit string"); mxFree(audit); break; }
                        mxFree(audit);
                    }
                }    

                /* header is written now do the individual timeseries */          

            }    
        
            /* set the pointer of the timeseries, we are gonna write */
            if ( settimeseriesindex_(fip,p+1) < 0) { errError("mexFunction","Could not set timeseries index"); break; }
            
            if (tsGetTSstring(array,arrayindex,"geomfile",&geomfile))
            {
                if (geomfile)
                {
                    if ( settimeseriesgeomfile_(fip,geomfile) ) { errError("mexFunction","Could not set geomfile"); mxFree(geomfile); break; }
                    mxFree(geomfile);
                }
            }    
            
            if (tsGetTSstring(array,arrayindex,"label",&label))
            {
                if (label)
                {
                    if ( settimeserieslabel_(fip,label) ) { errError("mexFunction","Could not set label"); mxFree(label); break; }
                    mxFree(label);
                }
            }    
        
            if (tsGetTSstring(array,arrayindex,"pakfile",&pakfile))
            {
                if (pakfile)
                {
                    if ( settimeseriesfile_(fip,pakfile) ) { errError("mexFunction","Could not set geomfile"); mxFree(pakfile); break; }
                    mxFree(pakfile);
                }
            }   
        
            if ( tsGetTSlong(array,arrayindex,"assoc",&assoc))
            {
                if ( settimeseriesassoc_(fip,assoc)) { errError("mexFunction","Could not set timeseriesassociation"); break; }
            }    
        
            if ( tsGetTSlong(array,arrayindex,"surfacenum",&surfacenum))
            {
                if (settimeseriessurface_(fip,surfacenum)) { errError("mexFunction","Could not set surface number"); break; }
            }
        
            if ( tsGetTSlong(array,arrayindex,"format",&format))
            {
                if ( settimeseriesformat_(fip,format)) { errError("mexFunction","Could not set format"); break; }
            } 
               
            if ( tsGetTSunit(array,arrayindex,&unit))
            {
                if ( settimeseriesunits_(fip,unit)) { errError("mexFunction","Could not set unit"); break; }
            }       
          
            if ( tsGetTSpotvals(array,arrayindex,&potvals,&numleads,&numframes))            
            {
                if ( settimeseriesspecs_(fip,numleads,numframes)) { errError("mexFunction","Count not set timeseriesspecs"); mxFree(potvals); break; }
                if ( settimeseriesdata_(fip,potvals))             { errError("mexFunction","Could not save timeseriesdata"); mxFree(potvals); break; }
                mxFree(potvals); /* free the memory as it can be quite large */
            }     
        
            if ( tsGetTSleadinfo(array,arrayindex,&leadinfo,&numleadinfo))
            {
                if ( setnumcorrectedleads_(fip,numleadinfo)) { errError("mexFunction","Could not set the number of bad leads"); mxFree(leadinfo); break; }
                if ( setcorrectedleads_(fip,leadinfo)) { errError("mexFunction","Could not set bad leads data"); mxFree(leadinfo); break; }    
                mxFree(leadinfo); 
            }    
        
            if (p==numindices-1) { loopsuccess = 1; break;} 
        }
        
        if (!loopsuccess) break;
        
        success = 1;
        break;
    } 
    
    if (fip) closefile_(fip);  /* fin */
    if (filename) mxFree(filename);
    if (indices) mxFree(indices);
    
    if (!success)
    {
        mexErrMsgTxt("ERROR : an error occured writing a TSDF-file using mexWriteTSDF");
        return;
    }
    return;
}




































