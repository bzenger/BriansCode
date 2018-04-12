/* 
 *  mexScanTSDF.c based   
 *
 *  Created by Jeroen Stinstra on Thu May 31 2002.
 *
 */
 
 /* 
   This function scans a TSDF file and retrieves the labels from the various timeseries. 
 */
 
 
/* Do the standard includes */

#include <stdio.h>
#include <math.h>
#include <mex.h>

#include "graphicsio.h"


/* IN THE FUTURE DECLARATIONS AND STRUCTS SHOULD GO TO HEADER FILE */

/* Declare my structures */

typedef struct ScanTsdfOption {
    long	numTimeSeries; /* the number of Timeseries to read, 0 means to read all */
    long 	*TimeSeries;   /* an array containing the indices of the timeseries to read */
    } Options;
    
    
/* Declare my functions  */
    
long		rdfGetNumTS(char *Filename); /* Get the number of timeseries in a file */
void 		rdfError(char *Where, char *Error, char *Usage); /* output error to matlab and exit */
void            rdfWarning(char *Where, char *Warning); /* output a warning */

mxArray 	*rdfGetLabel(char *filename,long index); /* Read the label of one series of time data */

/* Main entry function description starts here */

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
/* Entry point for MATLAB. 
    nlhs - number of parameters on leftside hand of expression
    plhs - a pointer to an array of pointers representing the various arrays on this side
    nrhs - idem right hand side
    prhs - idem array pointer right hand side
*/
{
    char		*filename;
    int	  		filenamelen;
    
    Options		Options;	/* put options in this structure */
    mxArray		*OptionField;   /* store a field in here */
    double		*TimeSeries;	/* An array to store the timeseries to read */
    
    long		numTimeSeries;  /* To store the number of timeseries in file */
    long		p;		/* my loop counter */

    mxArray		*FilenameArray, *OptionsArray, *IndicesArray; /* to store the input arguments */
    mxArray		*CellArray, *Label;	/* Data structure containing all data that is used for one datasete */


    /* Set my usage string, so I do not have to put it every time my function fails */
        rdfError(NULL,NULL,"timeSeries = mexScanTSDF(filename,[indices],[options])"); /* in this setting the function does NOT return to matlab :) */

    /* Check number and type of arguments */

    /* On the input multiple settings are allowed as long as there are fewer inputs than 4. */

  /*****************
    CHECK  INPUT 
   *****************/
   
    if (nrhs == 0)
	rdfError("mexFunction","You need to specify at least a filename",NULL);
	
    if (nrhs > 3)
	rdfError("mexFunction","No more than 3 inputs are allowed",NULL);
	
    /* For looop reading arguments */
    
    FilenameArray = NULL;
    IndicesArray = NULL;
    OptionsArray = NULL;
    
    for (p=0;p<nrhs;p++)
    { /* loops through all input and selects which is which argument */
	if ((mxIsChar(prhs[p]))&&(FilenameArray == NULL))
	{
	    FilenameArray = (mxArray *)prhs[p];
	}    
	if ((mxIsNumeric(prhs[p]))&&(IndicesArray == NULL))
	{
	    IndicesArray = (mxArray *)prhs[p];
	}
	if ((mxIsStruct(prhs[p]))&&(OptionsArray == NULL))
	{
	    OptionsArray = (mxArray *)prhs[p];
	}
    }		
	    
  /*****************
    CHECK OUTPUT 
   *****************/

    if (nlhs == 0)
	return;  /* nothing to do for me now */

    if (nlhs != 1)
        rdfError("mexFunction","One output argument is needed",NULL); /* too many output arguments are assigned */ 
        
  /*****************
    GET FILENAME
   *****************/
    
    if (FilenameArray == NULL)
    {    
	rdfError("mexFunction","You should supply a filename",NULL);
    }
    else
    {	
	filenamelen = (mxGetM(FilenameArray)*mxGetN(FilenameArray)) + 1; /* +1 for the zero to store in */
	if (!(filename = (char *)mxCalloc(filenamelen,sizeof(char))))  /* reserve enough space to put filename */
	    rdfError("mexFunction","Not enough memory to convert filename string",NULL);
    
	if (mxGetString(FilenameArray,filename,filenamelen))  /* copy filename in C-style string */
	rdfError("mexFunction","Could not retrieve filename",NULL);
    }
    
 /************************
   GET NUMBER TIMESERIES  
  ************************/
    numTimeSeries = rdfGetNumTS(filename); /* is needed to process options and indices */
          
 /**************
   GET OPTIONS
  **************/  

    /* first set defaults */
    Options.numTimeSeries = 0; /* Read all */
    Options.TimeSeries = NULL; /* Not yet a number of channels is specified */


    if(OptionsArray != NULL) /* an option structure is specified */
    {
    /*************************
      Do timeseries remapping
     *************************/  

        if(OptionField = mxGetField(prhs[1],0,"timeseries"))
        { /* A set of timeseries is specified */
             Options.numTimeSeries = (long)(mxGetN(OptionField)*mxGetM(OptionField)); /* Determine the length of the array */
             if(!(Options.TimeSeries = (long *) mxCalloc(Options.numTimeSeries,sizeof(long)))) /* Allocate space to put timeseries numbers in */
                 rdfError("mexFunction","Could not allocate memory",NULL); 
             if(!(TimeSeries = mxGetPr(OptionField)))  /* Get array containing the timeserie numbers in doubles */
                 rdfError("mexFunction","Could not read timeseries field",NULL);
             for (p=0;p<Options.numTimeSeries;p++) Options.TimeSeries[p] = (long) TimeSeries[p]; /* Convert array to longs */
         }
     
     /* No more options to check, the options struct should be up to date right now */
    }

    if (IndicesArray != NULL)
    {
	/* A set of timeseries is specified */
        Options.numTimeSeries = (long)(mxGetN(IndicesArray)*mxGetM(IndicesArray)); /* Determine the length of the array */
        if (Options.numTimeSeries > 0)
	{
	     if(!(Options.TimeSeries = (long *)mxCalloc(Options.numTimeSeries,sizeof(long)))) /* Allocate space to put timeseries numbers in */
                 rdfError("mexFunction","Could not allocate memory",NULL); 
             if(!(TimeSeries = mxGetPr(IndicesArray)))  /* Get array containing the timeserie numbers in doubles */
                 rdfError("mexFunction","Could not read timeseries indices",NULL);
             for (p=0;p<Options.numTimeSeries;p++) Options.TimeSeries[p] = (long) TimeSeries[p]; /* Convert array to longs */
	}
     }
  
 /***********************************************
   Do some consistency checking for the options
  ***********************************************/ 
 
    /* Update the TimeSeries array to be read into memory using header information */

    if (Options.numTimeSeries == 0) /* initiate a proper array */
    {
        if(!(Options.TimeSeries = (long *)mxCalloc(numTimeSeries,sizeof(long))))
            rdfError("mexFunction","Could not allocate enough memory",NULL);

        /* Fill out a proper time series array */
        Options.numTimeSeries = numTimeSeries;
        for(p=0;p<numTimeSeries;p++) Options.TimeSeries[p] = p+1;
    }
    else	/* check whether current array is valid */
    {
        for (p=0;p<Options.numTimeSeries;p++)
        {
             if ((Options.TimeSeries[p] < 1) || (Options.TimeSeries[p] > numTimeSeries))
                rdfError("mexFunction","You are trying to read not existing timeseries !",NULL);
        }
    }
    
    
 /***********************************************
   Do the main program loop reading the datasets
  ***********************************************/  
    /* OK now everything is set to read the individual data sets */

    if ( !(CellArray = mxCreateCellMatrix(Options.numTimeSeries,1)))
        rdfError("mexFunction","Could not create a cell array",NULL);
    
    for (p = 0; p < Options.numTimeSeries; p++)
    {

        Label = rdfGetLabel(filename,p+1);

	if(Label) mxSetCell(CellArray,p,Label);    

    }

    /* now link TSData to output */

    plhs[0] = CellArray;

    /* C'est tout */
}


void rdfWarning(char *Where,char *Warning)
/* 
    This functions deals with putting an error msg to the user
    The function has to functionalities 
        1) Put an error to the user
        2) Set the usage string 
    The latter can be set by rdfError(NULL,NULL,"My Usage string")

*/
{
    char	ErrorStr[256]; /* Just that I do not need to worry about allocating memory again */

    /* a genuine error */
    sprintf(ErrorStr,"\nLocation : %0.20s\nWarning : %0.80s\n",Where,Warning);
    
    /* Go back to matlab with an error msg */
    mexWarnMsgTxt(ErrorStr);
   
}

void rdfError(char *Where, char *Error, char *Usage)
/* 
    This functions deals with putting an error msg to the user
    The function has to functionalities 
        1) Put an error to the user
        2) Set the usage string 
    The latter can be set by rdfError(NULL,NULL,"My Usage string")

*/
{
    char	ErrorStr[256]; /* Just that I do not need to worry about allocating memory again */
    static char	UsageStr[100];

    if ((Where == NULL)&&(Error == NULL)) 
    {
        sprintf(UsageStr,"%0.100s",Usage); /* usage sprintf just to copy the string quickly */
    }
    else
    {
        /* a genuine error */
        sprintf(ErrorStr,"\nLocation : %0.20s\nError : %0.80s\nUsage : %0.100s\n",Where,Error,UsageStr);
    
        /* Go back to matlab with an error msg */
        mexErrMsgTxt(ErrorStr);
    }
}

long rdfGetNumTS(char *filename)
/* Just get the number of timeseries stored in this file cause I need it assessing the input parameters */
{
   FileInfoPtr FIP;
   long fileType;
   long numSurface;
   long numBSurface;
   long numTSeries;
   CBoolean pSetting;

   /* open the file being read */
   if ( openfile_(filename,1,&FIP))
      rdfError("rdfGetFileInfo","Could not open file",NULL);
  
   /* get all the info out, well most of it I don't need at this point */
   if (getfileinfo_(FIP, &fileType, &numSurface, &numBSurface,&numTSeries, &pSetting))
      { closefile_(FIP); rdfError("rdfGetFileInfo","Could not read fileinfo header",NULL); }

   /* make sure the file is closed properly */
   closefile_(FIP);  

   return numTSeries;
}



mxArray *rdfGetLabel(char *filename,long index)
/* 
    This function retrieves the label of one timeseries from the file
    and puts it directly into a matlab structure  */ 
{
    FileInfoPtr 	FIP; /* To store file pointer */
    

    char label[80];       /* Text field for this file */
    
    mxArray		*Array;	/* to store individual field temparoraly in */
 
  
 /**************
   Open file
  **************/    

    if ( openfile_(filename, 1, &FIP) ) 
        rdfError("rdfGetTimeSeries","Could not open file",NULL);

    /* go to the part in the file which contains the data to be read */
    if( settimeseriesindex_(FIP,index) < 0)
        { closefile_(FIP); rdfWarning("rdfGetTimeSeries","Could not set timeseries index");
	  return(NULL); }
   
    /* LABEL */

    if ( gettimeserieslabel_(FIP,label) )
         { closefile_(FIP); rdfError("rdfGetTimeSeries","Could not get timeseries label",NULL);}
    if (!(Array = mxCreateString(label)))
         { closefile_(FIP); rdfError("rdfGetTimeSeries","Could not create a matlab array",NULL);}
 
    closefile_(FIP);
    
    return(Array);
 }   

