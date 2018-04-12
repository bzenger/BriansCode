/* FILENAME:  mexReadTSDF.c
   AUTHOR:    JG STINSTRA
   CONTENTS:  This functions reads the contents of a TSDF file into matlab's memory
   LAST UPDATE: 4 JULY 2003 
*/


/*  The function reads the TSDF file into memory, the function can also be used to read a TSDF file in pieces,
    using the options array one can specify the number of leads/channels to read and using the indices to specify which
    timeseries should be read */

/*
    There are two options, using a structured array or a cell array with structured cells. Both have
    advantages and of course disadvantages. In a structured array you are sure that the same fields are contained in each
    set of data, adding a field will cause the field to be added everywhere and the field names are only stored once. In a
    cell array containing structs this is more flexible as the number fields is stored locally. Hence this allows for local 
    extensions as well, without altering other data contained in other cells. The cost is quite clear the amount of memory
    to store the additional fieldnames. 
*/
    

/***************************
  HERE THE CODE STARTS
***************************/

/* Do the standard includes */


#include <stdio.h>
#include <math.h>
#include <mex.h>

#include "graphicsio.h"
#include "myerror.c"
#include "myindex.c"
#include "misctools.c"
#include "tstools.c"

typedef struct  {
    long	numtimeseries; /* the number of Timeseries to read, 0 means to read all */
    long 	*timeseries;   /* an array containing the indices of the timeseries to read */
    CBoolean 	scanfile;    /* TRUE if the file only needs scanning */
    long	numleadmaps; /* how many maps are there */
    mapdata	*leadmap;    /* the actual lead maps */
    long	numframemaps; /* how many frame maps are there */
    mapdata	*framemap;   /* the actual frame maps */
    CBoolean	readfids;    /* do I need to get fiducials stored in tsdf file */
	char    *datapath;   
    } options;


/* Declare my functions  */
    
int 		GetNumTS(char *filename,long *numtseries);
int 		SetFids(mxArray *fidsarray,long arrayindex,long value, long type, long fidset);
mxArray 	*GetTimeSeries(char *filename,long index,options *options); 
mxArray 	**ReadPotVals(FileInfoPtr fip,long numleads, long Numframes, mapdata *leadmap,mapdata *framemap); 


/******************************
  MEXFUNCTION
 ******************************/


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
/* Entry point for MATLAB. 
    nlhs - number of parameters on leftside hand of expression
    plhs - a pointer to an array of pointers representing the various arrays on this side
    nrhs - idem right hand side
    prhs - idem array pointer right hand side
*/
{
    char		*filename;	/* to store the filename */
    options		options;	/* put options in this structure */
    mxArray		*optionfield;   /* store a field in here */
    long		numtimeseries;  /* To store the number of timeseries in file */
    long		p;		/* my loop counter */
    mxArray		*filenamearray, *optionsarray, *indicesarray; /* to store the input arguments */
    mxArray		*TSdata, *cellarray;	/* Data structure containing all data that is used for one datasete */
    int			success;

    /* Set my usage string, so I do not have to put it every time my function fails */
    errUsage("timeSeries = mexReadTSDF(filename,[indices],[options])"); 

    /* On the input multiple settings are allowed as long as there are fewer inputs than 4. */

  /*****************
    CHECK  INPUT 
   *****************/
    if (nrhs == 0)
    {
	errError("mexFunction","You need to specify at least a filename");
        mexErrMsgTxt("ERROR");
	return;
    }

    if (nrhs > 3)
    {
	errError("mexFunction","No more than 3 inputs are allowed");
        mexErrMsgTxt("ERROR");
	return;
    }
	
    /* For looop reading arguments */
    
    filenamearray = NULL;
    indicesarray = NULL;
    optionsarray = NULL;
    
    /* Since we can be pretty flexible with input arguments, use their type to sort out which is which.
       So more combinations than the usage syntax states are allowed. Not that it matters, but the code
       is no big thing. It just requires one loop */
    
    for (p=0;p<nrhs;p++)
    { 
        /* loops through all input and selects which is which argument */
        if ((mxGetN(prhs[p])*mxGetM(prhs[p]))==0) continue;  /* no sense to check this one */
	if ((mxIsChar(prhs[p]))&&(filenamearray == NULL))  filenamearray = (mxArray *)prhs[p];    
	if ((mxIsNumeric(prhs[p]))&&(indicesarray == NULL))  indicesarray = (mxArray *)prhs[p];
	if ((mxIsStruct(prhs[p]))&&(optionsarray == NULL))  optionsarray = (mxArray *)prhs[p];
    }		
	    
  /*****************
    CHECK OUTPUT 
   *****************/

    if (nlhs == 0)
    {
        /* Just inform the user about his/her mistake if it is one ? */
        errWarning("mexFunction","You did not specify any output arguments, there is nothing to do for me");
	return;  /* nothing to do for me now. Thee user does not want to have any output */
    }
    
    if (nlhs != 1)
    {
        errError("mexFunction","One output argument is needed"); /* too many output arguments are assigned */ 
        mexErrMsgTxt("ERROR");
	return;
    }

        
  /*****************
    GET FILENAME
   *****************/
    
    if (filenamearray == NULL)
    {   /* Cannot do anything without a filename */ 
	errError("mexFunction","You should supply a filename");
        mexErrMsgTxt("ERROR");
	return;
    }
    else
    {	/* Get me a C-string of the filename */
	filename =  miscConvertString(filenamearray);
	if (filename == NULL)
	{
		errError("mexFunction","Could not allocate enough memory");
        	mexErrMsgTxt("ERROR");
		return;
    	}
    }
    
 /************************
   GET NUMBER TIMESERIES  
  ************************/
    if(!(GetNumTS(filename,&numtimeseries)))
    {
	mxFree(filename);
	errError("mexFunction","Could not read the number of timeseries");
        mexErrMsgTxt("ERROR");
	return;
    }          
 /**************
   GET OPTIONS
  **************/  

    /* first set defaults */
    options.numtimeseries = 0; /* Read all */
    options.timeseries = NULL; /* Not yet a number of channels is specified */
    options.scanfile = FALSE;  /* Read all data */
    options.numleadmaps = 0; /* no Remapping */
    options.leadmap = NULL;
    options.numframemaps = 0; /* no remapping of frames */
    options.framemap = NULL;
    options.readfids = FALSE; /* Ignore fiducials in file */
	options.datapath = 0;

    success  = 1;
	
    if(optionsarray != NULL) /* an option structure is specified */
    {
    /*************************
      Do timeseries remapping
     *************************/  

        if(optionfield = mxGetField(prhs[1],0,"datapath"))
        {
			options.datapath = miscConvertString(optionfield);
		}


        if(optionfield = mxGetField(prhs[1],0,"timeseries"))
        {
            /* convert the matrix with the indices into an array with indices */
            if(!(idxConvertIndices(optionfield,&(options.timeseries),&(options.numtimeseries)))) success = 0;
        }

    /*******************************
      Set the file for scanning only
     ******************************/

        if(optionfield = mxGetField(prhs[1],0,"scantsdffile"))
        { /* Do I need to scan the file only and not load any large matrices */
            
            options.scanfile = TRUE;
            if (mxGetScalar(optionfield) == 0.0) options.scanfile = FALSE; /* Oh the user intended to keep me from scanning the file :) */
        }
        
    /****************************************
      Set whether the fiducials need to read
     ****************************************/

        if(optionfield = mxGetField(prhs[1],0,"readtsdffids"))
        { /* Do I need to scan the file only and not load any large matrices */
            
            options.readfids = TRUE;
            if (mxGetScalar(optionfield) == 0.0) options.readfids = FALSE; /* Oh the user intended to keep me from scanning the file :) */
        }
        
        

    /************************************
      Check for lead remapping options 
     ************************************/
        
	if(optionfield = mxGetField(prhs[1],0,"leadmap")) 
	{ 
            if(!(idxReadIMap(optionfield,&(options.leadmap),&(options.numleadmaps)))) success = 0;
        } 
	   
    /************************************
      Check for frame remapping options 
     ************************************/
        
	if(optionfield = mxGetField(prhs[1],0,"framemap")) 
	{ 
            if(!(idxReadIMap(optionfield,&(options.framemap),&(options.numframemaps)))) success = 0;	/* load the framemap(s) */
        }
        /* no more options to check for the moment */    
    }

    if (indicesarray != NULL)
    {
	/* A set of timeseries is specified */
        if(!(idxConvertIndices(indicesarray,&(options.timeseries),&(options.numtimeseries)))) success = 0; /* Convert the indicesarray into C-style indices */
    }
  
 /***********************************************
   Do some consistency checking for the options
  ***********************************************/ 
 
    /* Update the TimeSeries array to be read into memory using header information */

    if (options.numtimeseries == 0) /* initiate a proper array */
    {
        if(!(options.timeseries = (long *) mxCalloc(numtimeseries,sizeof(long)))) 
	{
		success = 0;
	}
        else
	{
		options.numtimeseries = numtimeseries;
	        for(p=0;p<numtimeseries;p++) options.timeseries[p] = p+1;
	}
    }
    else	/* check whether current array is valid */
    {
        for (p=0;p<options.numtimeseries;p++)
        {
             	if ((options.timeseries[p] < 1) || (options.timeseries[p] > numtimeseries))
             	{
			errError("mexFunction","You are trying to read not existing timeseries !");
			success = 0;
	     	}
        }
    }
    
    /* check the number of leadmaps and framemaps, there should be one for all or as many as timeseries specified */
    
    if ((options.numleadmaps != 0)&&(options.numleadmaps != 1)&&(options.numleadmaps != options.numtimeseries))  
	{ errError("mexFunction","The number of Leadmaps specified should be one or equal to the number of timeseries loaded"); success = 0;}

    if ((options.numframemaps != 0)&&(options.numframemaps != 1)&&(options.numframemaps != options.numtimeseries))  
	{ errError("mexFunction","The number of FrameMaps specified should be one or equal to the number of timeseries loaded"); success = 0;}
	
    if (!success)
    {
	mxFree(options.timeseries);
	idxFreeIMap(options.leadmap,options.numleadmaps);
	idxFreeIMap(options.framemap,options.numframemaps);
	mxFree(filename);
	errError("mexFunction","Could not process every option");
        mexErrMsgTxt("ERROR");
	return;
    }

 /***********************************************
   Do the main program loop reading the datasets
  ***********************************************/  
    /* OK now everything is set to read the individual data sets */

    /* Create a cell array to store each dataset in */
       
    if (!(cellarray = mxCreateCellMatrix(options.numtimeseries,1)))
    {
	mxFree(options.timeseries);
	idxFreeIMap(options.leadmap,options.numleadmaps);
	idxFreeIMap(options.framemap,options.numframemaps);
	mxFree(filename);
	errError("mexFunction","Could not create cell-array");
        mexErrMsgTxt("ERROR");
	return;
    }    
    
    for (p=0;p<options.numtimeseries;p++)
    {
        if(!(TSdata = GetTimeSeries(filename,p+1,&options)))
	{
      	    mxDestroyArray(cellarray);
      	    cellarray = NULL;
      	    break;
      	}
        mxSetCell(cellarray,p,TSdata);    
    }

    /* now link TSdata to output */

    /* we already checked the number of output matrices and there is only one */
    plhs[0] = cellarray;

    mxFree(options.timeseries);
    idxFreeIMap(options.leadmap,options.numleadmaps);
    idxFreeIMap(options.framemap,options.numframemaps);
    mxFree(filename);
	
	if (options.datapath) mxFree(options.datapath);

    /* C'est tout */
}


/**********************************************************************/
/* Sub functions start here */


/********************
  info on tsdffiles
 ********************/
 
int GetNumTS(char *filename,long *numtseries)
/* Just get the number of timeseries stored in this file cause I need it assessing the input parameters */
{
   FileInfoPtr fip;
   long filetype;
   long numsurface;
   long numbsurface;
   CBoolean psetting;

   /* open the file being read */
   if ( openfile_(filename,1,&fip))
   {
      errError("GetNumTS","Could not open file");
      return(0);
   }
  
   /* get all the info out, well most of it I don't need at this point */
   if (getfileinfo_(fip,&filetype, &numsurface,&numbsurface,numtseries, &psetting))
   { 
	closefile_(fip); 
	errError("GetNumTS","Could not read fileinfo header");
	return(0);
   }

   /* make sure the file is closed properly */
   closefile_(fip);  

   return(1);
}


mxArray *GetTimeSeries(char *filename,long index,options *options)
/* 
    This function retrieves one timeseries from the file
    and puts it directly into a matlab structure
    Options contains a field describing the options that are specified */

/* This function is an adapted version of GetTSInfo */ 
{
    FileInfoPtr 	fip; /* To store file pointer */
    
    /* fields to read */
    long numleads;        	/* Number of leads in this Time Series (TS) */
    long numframes;       	/* Number of frames in this TS */
    long units;           	/* Units of the data (see graphiciso.h for values) */ 
    char geomfile[80];    	/* Associated geometry file */
    char label[80];       	/* Text field for this file */
    char text[256];		/* Text field for comment on experiment */
    char expid[80];		/* experiment id string */
    
    /* If we encounter other formatted files in matlab, this routine should be able to handle them */
    
    mapdata			*leadmap,*framemap; /* to store remapping pointers */

    mxArray		*TSarray; /* The structure that is being build */
    mxArray		**arraylist; /* pointer to a list of pointers */

    long		auditlen;
    char		*audit;    /* Ptr to buffer with audit strings */   

    /* define structure of array */
    const char 		*TSfieldnames[] = {
			    "filename",
                            "label",
			    "potvals",
                            "numleads",
                            "numframes",
			    "leadinfo",
			    "unit",
 			    "geom",
			    "geomfile",
			    "expid",
			    "text",
			    "audit",
                            "fids",
                            "fidset" };
    enum  TSfields	{ FILENAME = 0, LABEL, POTVALS, NUMLEADS, NUMFRAMES, LEADINFO, UNIT, GEOM, GEOMFILE, EXPID, TEXT, AUDIT, FIDS, TSFIDSET };
    	 /* this enum should mirror the field names array */	
    
    const int		numTSfields = 12;
    mapdata		*maparray;
   
   /* For reading the fiducials if requested from the tsdf files */
   
    long		pon,poff,qrson,qrspeak,qrsoff,toff,tpeak; /* tsdf fiducials */
    CBoolean		extendedfiducials; /* Are there just three or are there four more */
    int			numfids; /* number of fiducials we are dealing with */
    mxArray		*fidsarray, *fidsetarray, *fidsetcellarray; /* the output arrays */
    mxArray		*filenamestring, *labelstring; /* For storing the filename and a label as a matlab string*/
    const int		fidsnumfields = 3;  /* define the structures */
    const char		*fidsfieldnames[] = {
			    "value",	/* fiducial time values in ms */
			    "type",     /* the type of the fiducial */
			    "fidset" }; /* from which fiducial set is it originating ? */
    enum  fidsfields	{VALUE = 0, TYPE, FIDSET };			    
    const int		fidsetnumfields = 2;
    const char		*fidsetfieldnames[] = { /* an overview of the fiducial sets read into memory */
			    "filename", /* name of the tsdfc-file the fiducial came from */
			    "label" };    /* label of the fiducial set, normally the subprogram of Everett that created the fiducial */
    enum  fidsetfields	{FIDSETFILENAME =0, FIDSETLABEL};
    int			success;   

 /**************
   Open file
  **************/    

    if ( openfile_(filename, 1, &fip) ) 
    {
	errError("GetTimeSeries","Could not open file");
	return(NULL);
    }

	if (options->datapath)
	{
		settimeseriesdatapath_(fip,options->datapath);
	}

    /* go to the part in the file which contains the data to be read */
    if( settimeseriesindex_(fip,index) < 0)
    { 
	closefile_(fip); 
	errError("GetTimeSeries","Could not set timeseries index");
   	return(NULL);
    }

 /******************
   Set mapping data
  ******************/
    
    /* Read lead mapping indices  */
    if (options->numleadmaps == 0) { leadmap = NULL; }
    else if (options->numleadmaps == 1) { maparray = options->leadmap; leadmap = &(maparray[0]); }
    else {maparray = options->leadmap; leadmap = &(maparray[index-1]); } 

    /* Read frame mapping indices */
    if (options->numframemaps == 0) { framemap = NULL; }
    else if (options->numframemaps == 1) { maparray = options->framemap; framemap = &(maparray[0]); }
    else {maparray = options->framemap; framemap = &(maparray[index-1]); } 
    
 /*****************************
   begin reading structure here 
  ******************************/

    /* Start with initialising a matrix struct from matlab */
    if (options->readfids == TRUE)
    {
        /* in this case I need to allocate two more fields to store the fiducials */
        if(!(TSarray = mxCreateStructMatrix(1,1,numTSfields+2,TSfieldnames)))
        { 
		closefile_(fip); 
		errError("GetTimeSeries","Could not create a matlab array");
		return(NULL);
	}
    }
    else
    {
            if(!(TSarray = mxCreateStructMatrix(1,1,numTSfields,TSfieldnames)))
            { 
		closefile_(fip); 
		errError("GetTimeSeries","Could not create a matlab array");
		return(NULL);
	    }
    
    }

    /* So now collect info and start filling out the matrix */

  /***********************************************
    Elements to read when just scanning the file 
   ***********************************************/

    /* NUMLEADS AND NUMFRAMES */

     success = 1;	

     if ( gettimeseriesspecs_(fip, &numleads, &numframes) )
     { 	success = 0; errError("GetTimeSeries","Could not get number of lead and number of frames"); }

     if (!leadmap) 
     {
         if (!( tsPutTSlong(TSarray,0,NUMLEADS,numleads)))
            { success = 0; errError("GetTimeSeries","Could not create a matlab array");}
     }
     else
     {
        if (!( tsPutTSlong(TSarray,0,NUMLEADS,leadmap->num)))
            { success = 0; errError("GetTimeSeries","Could not create a matlab array");}
     }
     
     if (!framemap) 
     {
         if (!( tsPutTSlong(TSarray,0,NUMFRAMES,numframes)))
            { success = 0; errError("GetTimeSeries","Could not create a matlab array");}
     }
     else
     {
        if (!( tsPutTSlong(TSarray,0,NUMFRAMES,framemap->num)))
            { success= 0; errError("GetTimeSeries","Could not create a matlab array");}
     }
           
    /* LABEL */
    if ( gettimeserieslabel_(fip,label) )
         { success = 0; errError("GetTimeSeries","Could not get timeseries label");}
         
    if (!(tsPutTSstring(TSarray,0,LABEL,label)))
         { success = 0; errError("GetTimeSeries","Could not create a matlab array");}
    
    /* GEOMETRY FILE */
    if ( gettimeseriesgeomfile_(fip,geomfile) )
       { success = 0; errError("GetTimeSeries","Could not get geometryfile");}

    if (!(tsPutTSstring(TSarray,0,GEOMFILE,geomfile)))
       { success = 0; errError("GetTimeSeries","Could not create a matlab array");}

    /* PAKFILE 
    if ( gettimeseriesfile_(fip,pakfile) )
       { success = 0; errError("GetTimeSeries","Could not get pakfile");}
    if (!(tsPutTSstring(TSarray,0,PAKFILE,pakfile)))
       { success = 0; errError("GetTimeSeries","Could not create a matlab array");} */

    /* KEEP GEOM FREE SINCE WE ARE NOT LOADING THE GEOMETRY */

    /* EXPID */
    if ( getexpid_(fip,expid))
        { success = 0; errError("GetTimeSeries","Could not get experiment id");}
    if (!(tsPutTSstring(TSarray,0,EXPID,expid)))
         { success = 0; errError("GetTimeSeries","Could not create a matlab array");}

    /* TEXT */
    if ( gettext_(fip,text))
        { success = 0; errError("GetTimeSeries","Could not get text");}
    if (!(tsPutTSstring(TSarray,0,TEXT,text)))
         { success = 0 ; errError("GetTimeSeries","Could not create a matlab array");}
    
    /* FILENAME */
    if (!(tsPutTSstring(TSarray,0,FILENAME,filename)))
         { success = 0; errError("GetTimeSeries","Could not create a matlab array");}

    /* AUDIT STRING */
    audit = NULL;
    if ( !(getauditstringlength_(fip,&auditlen)))
        { audit = (char *)mxCalloc(auditlen+1,sizeof(char)); }

    if (!audit)
    { 
	success = 0;  errError("GetTimeSeries","Could not allocate enough memory");
    }
    else
    {
	if (getauditstring_(fip,auditlen,audit))
        { 
		/* errWarning("GetTimeSeries","Could not get audit string"); */
		/* this error seems to be confusing */
		} 
    	else 
    	{
        	if (!(tsPutTSstring(TSarray,0,AUDIT,audit)))
            		{ success = 0; errError("GetTimeSeries","Could not create a matlab array");}
    	}
	mxFree(audit);
    }

    if(!success)
    {
	closefile_(fip);
      	mxDestroyArray(TSarray);
        errError("GetTimeSeries","Could not read all fields from the TSDF-file");
	return(NULL);
    }

    /****************************************************
      Elements which will be read as well in normal mode 
     ****************************************************/

    if (options->scanfile == FALSE)
    {	
        /* Create arrays and link these to the global structure */
          /* UNIT */
       
        /* Changed name from units to unit as it carries only one unit */
        if ( gettimeseriesunits_(fip, (long *)&units) )
            { success = 0; errError("GetTimeSeries","Could not get unit information");}
	if ( !(tsPutTSunit(TSarray,0,UNIT,units)))
            { success = 0; errError("GetTimeSeries","Could not create matlab matrix");}        

        /* POTENTIAL DATA */
       
	if(!(arraylist = ReadPotVals(fip,numleads,numframes,leadmap,framemap)))
	{
	     success = 0; errError("GetTimeSeries","Could not load potvals");
	}
	else
	{
		mxSetFieldByNumber(TSarray,0,POTVALS,arraylist[0]);    
		mxSetFieldByNumber(TSarray,0,LEADINFO,arraylist[1]);
		mxFree(arraylist);	
		} 
    }
    
    if(!success)
    {
	closefile_(fip);
      	mxDestroyArray(TSarray);
        errError("GetTimeSeries","Could not read all fields from the TSDF-file");
	return(NULL);
    }

    /* Do the fiducial stuff */
    
    if (options->readfids == TRUE)
    {
        if ( getqsttimes_(fip,(long *)&qrson,(long *)&qrsoff,(long *)&toff) )
            { success = 0; errError("GetTimeSeries","Could not get fiducials");}
        extendedfiducials = 0;
        if ( checkextendedfiducials_(fip,&extendedfiducials))
            { success = 0; errError("GetTimeSeries","Could not determine whether there are extended fiducials");}
        if (extendedfiducials)
        {
            if ( getextendedfiducials_(fip,(long *)&pon,(long *)&poff,(long *)&qrspeak,(long *)&tpeak))
                { success = 0; errError("GetTimeSeries","Could not get are extended fiducials");}
        }
        
       	if(!success)
    	{
		closefile_(fip);
      		mxDestroyArray(TSarray);
        	errError("GetTimeSeries","Could not read fiducials from the TSDF-file");
		return(NULL);
    	}

	numfids = 3;
        if (extendedfiducials) numfids = 7;
        
        if (!( fidsarray = mxCreateStructMatrix(1,numfids,fidsnumfields,fidsfieldnames))) 
        { 
		closefile_(fip);
		mxDestroyArray(TSarray);
		errError("GetFids","Could not allocate struct (fidsarray)"); 
		return(NULL);
	}

      	mxSetFieldByNumber(TSarray,0,FIDS,fidsarray);  
    	
        if(!(SetFids(fidsarray,0,qrson,2,1))) success = 0;
        if(!(SetFids(fidsarray,1,qrsoff,4,1))) success = 0;
        if(!(SetFids(fidsarray,2,toff,7,1))) success = 0;
        if (extendedfiducials)
        {
            if(!(SetFids(fidsarray,3,pon,0,1))) success = 0;
            if(!(SetFids(fidsarray,4,poff,1,1))) success = 0;
            if(!(SetFids(fidsarray,5,qrspeak,3,1))) success = 0;
            if(!(SetFids(fidsarray,6,tpeak,6,1))) success = 0;
        }
        if (!success) 
        { 
		closefile_(fip);
		mxDestroyArray(TSarray);
		errError("GetFids","Could not set all fiducials"); 
		return(NULL);
	}
        /* build the information fidset array. This one consists of a cellarray in which a struct matrix is stored */

        if (!( fidsetcellarray = mxCreateCellMatrix(1,1)))
        { 		
		closefile_(fip);
		mxDestroyArray(TSarray); 
		errError("GetFids","Could not allocate cell array");
		return(NULL);
	}
 	mxSetFieldByNumber(TSarray,0,TSFIDSET,fidsetcellarray);	

        if (!( fidsetarray = mxCreateStructMatrix(1,1,fidsetnumfields,fidsetfieldnames)))
	{ 
		closefile_(fip);
		mxDestroyArray(TSarray);
		errError("GetFids","Could not allocate struct");
		return(NULL);
        }
        mxSetCell(fidsetcellarray,0,fidsetarray);   
        
        /* Create the contents for the fidset cellarray */
        
        if (!( filenamestring = mxCreateString(filename)))
        { 
		closefile_(fip); 
		mxDestroyArray(TSarray);
		errError("GetFids","Could not allocate filenamestring");
		return(NULL);
        }
        mxSetFieldByNumber(fidsetarray,0,FIDSETFILENAME,filenamestring);

        if (!(labelstring = mxCreateString("TSDF fiducials\0")))
        { 
		closefile_(fip); 
		mxDestroyArray(TSarray);
		errError("GetFids","Could not allocate labelstring");
		return(NULL);
	}
        mxSetFieldByNumber(fidsetarray,0,FIDSETLABEL,labelstring);
            
    }
 
    closefile_(fip); /* close file and return to main program */
    return(TSarray);
}


int SetFids(mxArray *fidsarray,long arrayindex,long value, long type, long fidset)
/* This function writes a fiducial in the fids array. It creates the submatrices and
   links everything together. The function results a success or 0 if no success */
{
    mxArray 	*array;
    double	*data;
    
    /* Create a new matrix, put the value in and link it to the struct matrix */
    if (!( array = mxCreateDoubleMatrix(1,1,mxREAL))) return(0);
    if (!( data = mxGetPr(array))) return (0);
    data[0] = (double) value;
    mxSetFieldByNumber(fidsarray,(int)arrayindex,0,array);

    /* same for type */
    if (!( array = mxCreateDoubleMatrix(1,1,mxREAL))) return(0);
    if (!( data = mxGetPr(array))) return (0);
    data[0] = (double) type;
    mxSetFieldByNumber(fidsarray,(int)arrayindex,1,array);

    /* and same for value */    
    if (!( array = mxCreateDoubleMatrix(1,1,mxREAL))) return(0);
    if (!( data = mxGetPr(array))) return (0);
    data[0] = (double) fidset;
    mxSetFieldByNumber(fidsarray,(int)arrayindex,2,array);

    return (1); /* success */
}   

/************************************
	READPOTVALS
 ************************************/

typedef struct {
	mxArray **arraylist;
	float	*fdatabuff;
	long	*ldatabuff;
	mxArray	*potvalsarray;
	mxArray	*leadinfoarray;
	long	*leadinfo;
	} POTVALSDATA;

void 	ReadPotValsCleanup(POTVALSDATA *data,char *error);

void	ReadPotValsCleanup(POTVALSDATA *data,char *error)
{
	if (error)		errError("ReadPotvals",error);
	if (data->arraylist) 	mxFree(data->arraylist);
	if (data->fdatabuff)	mxFree(data->fdatabuff);
	if (data->ldatabuff)	mxFree(data->ldatabuff);
	if (data->potvalsarray) mxDestroyArray(data->potvalsarray);
	if (data->leadinfoarray) mxDestroyArray(data->leadinfoarray);
	if (data->leadinfo)	mxFree(data->leadinfo);
}

mxArray **ReadPotVals(FileInfoPtr fip,long numleads, long numframes, mapdata *leadmap,mapdata *framemap)
{
    	long		p,q; /* loop counters */
    	long		pp,qq; /*additional counters */
    	long		lnum,fnum; /* new lead and frame stats */
    	long		numleadinfo;
        long            *arraydatauint;
	mxArray		**arraylist;
	float		*databuff;
	double		*arraydata;

    	POTVALSDATA		d;	
    	d.arraylist 		= NULL;	
    	d.fdatabuff		= NULL;
	d.ldatabuff		= NULL;
	d.potvalsarray		= NULL;
	d.leadinfoarray	        = NULL;
	d.leadinfo		= NULL;

 	/*************************************
  	  Build an array of matlabarray pointers
 	 *************************************/
  
    	if( !(d.arraylist = (mxArray **)mxCalloc(2,sizeof(mxArray *))))
    		{ ReadPotValsCleanup(&d,"could not obtain enough memory"); return(NULL); }

 	/*************************************
   	  First do some more integrity checks 
  	 *************************************/
  
        if (leadmap != NULL)
      	    for(p = 0; p < leadmap->num; p++)
	        if ( (leadmap->map[p] < 1)||(leadmap->map[p] > numleads))
		    { ReadPotValsCleanup(&d,"leadmap values are out of range"); return(NULL); }
 
        if (framemap != NULL)
    	    for(p = 0; p < framemap->num; p++)
	        if ( (framemap->map[p] < 1)||(framemap->map[p] > numframes))
		    { ReadPotValsCleanup(&d,"framemap values are out of range"); return(NULL); }

 
 	/**********************************************************
   	 Four ways of importing data, with or without lead/framemapping
 	 **********************************************************/
   
   	if ( !(d.fdatabuff = (float *)mxCalloc(numframes*numleads,sizeof(float))))
   		{ ReadPotValsCleanup(&d,"Could not obtain enough memory to load data"); return(NULL); }

   	if ( gettimeseriesdata_(fip, d.fdatabuff) )
   		{ ReadPotValsCleanup(&d,"Could not load time dataseries"); return(NULL); }


   	if ((leadmap != NULL)&&(framemap != NULL))
   	{ 

        	if ( !(d.potvalsarray = mxCreateDoubleMatrix(leadmap->num,framemap->num,mxREAL)))
        		{ ReadPotValsCleanup(&d,"Could not allocate matlab matrix (potvals)"); return(NULL); }

        	arraydata = mxGetPr(d.potvalsarray);
		lnum = leadmap->num;  /* new dimensions of the resulting matrix */
		fnum = framemap->num;
		databuff = d.fdatabuff;

		for(p = 0; p < lnum; p++)
		    	for(q = 0; q < fnum; q++)
		    	{
				pp = (leadmap->map[p]-1);
				qq = (framemap->map[q]-1);
				if((pp>=0)&&(qq>=0)) 
				    	arraydata[p+(q*lnum)] = (double) databuff[pp+(qq*numleads)]; /* do casting and mapping all in one step */
	    		}		
   	}

	if ((leadmap == NULL)&&(framemap != NULL))
   	{ 

        	if ( !(d.potvalsarray = mxCreateDoubleMatrix(numleads,framemap->num,mxREAL)))
        		{ ReadPotValsCleanup(&d,"Could not allocate matlab matrix (potvals)"); return(NULL); }

        	arraydata = mxGetPr(d.potvalsarray);
		fnum = framemap->num;
		databuff = d.fdatabuff;

		for(p = 0; p < numleads; p++)
		    	for(q = 0; q < fnum; q++)
		    	{
				qq = (framemap->map[q]-1);
				if(qq>=0) arraydata[p+(q*numleads)] = (double) databuff[p+(qq*numleads)]; /* do casting and mapping all in one step */
	    		}		
   	}

	if ((leadmap != NULL)&&(framemap == NULL))
   	{ 

        	if ( !(d.potvalsarray = mxCreateDoubleMatrix(leadmap->num,numframes,mxREAL)))
        		{ ReadPotValsCleanup(&d,"Could not allocate matlab matrix (potvals)"); return(NULL); }

        	arraydata = mxGetPr(d.potvalsarray);
		lnum = leadmap->num;  
		databuff = d.fdatabuff;

		for(p = 0; p < lnum; p++)
		    	for(q = 0; q < numframes; q++)
		    	{
				pp = (leadmap->map[p]-1);
				if(pp>=0) 
				    	arraydata[p+(q*lnum)] = (double) databuff[pp+(q*numleads)]; /* do casting and mapping all in one step */
	    		}		
   	}

	if ((leadmap == NULL)&&(framemap == NULL))
	{ /* load it the tradional way, just do a casting operation */
        	if ( !(d.potvalsarray = mxCreateDoubleMatrix(numleads,numframes,mxREAL)))
        		{ ReadPotValsCleanup(&d,"Could not allocate matlab matrix (potvals)"); return(NULL); }
      
	        arraydata = mxGetPr(d.potvalsarray);
		databuff = d.fdatabuff;
		for(p = 0;p < (numleads*numframes); p++)
		{
	     		arraydata[p] = (double) databuff[p];
		}	
   	}
   

 	/*************************************************************   
	   Do the lead information stuff
	 *************************************************************/
  
   	/* Since the lead order may be altered, do the same ordering stuff for the lead info */ 
  

	if ( getnumcorrectedleads_(fip, &numleadinfo))
   		{ ReadPotValsCleanup(&d,"Could not get bad lead information"); return(NULL); }

   	if (leadmap != NULL)
	{
   		if (!(d.leadinfoarray = mxCreateNumericMatrix(leadmap->num,1,mxUINT32_CLASS,mxREAL))) /* Assuming long is uint32 on this platform */
	            { ReadPotValsCleanup(&d,"Could not create a matlab array"); return(NULL); }
	}
	else
	{
		if (!(d.leadinfoarray = mxCreateNumericMatrix(numleads,1,mxUINT32_CLASS,mxREAL))) /* Assuming long is uint32 on this platform */
	            { ReadPotValsCleanup(&d,"Could not create a matlab array"); return(NULL); }
	}
  
   	arraydatauint = (long *) mxGetData(d.leadinfoarray); /* Has to be of the uint type */
            
   	if (numleadinfo > 0) /* if none defined it does not make any sence to process the data */
   	{
        	if ( !(d.leadinfo = (long *)mxCalloc(numleadinfo,sizeof(long))))
            		{ ReadPotValsCleanup(&d,"Could not obtain enough memory to load data"); return(NULL); }

        	if ( getcorrectedleads_(fip,d.leadinfo)) 
	                { ReadPotValsCleanup(&d,"Could not load bad leads"); return(NULL); }
	
		if (leadmap != NULL)
		{    
			if ( numleadinfo != leadmap->num)	 /* Do some checking */
		    		errWarning("ReadPotVals","Number of leads and number of badleads info is not equal");
	    
			for (p=0;p < leadmap->num; p++)
			{
			    if ( (leadmap->map[p] <= numleadinfo)&&(leadmap->map[p] > 0)) /* properly check whether the lead number exists */
				arraydatauint[p] = d.leadinfo[(leadmap->map[p])-1];  /* Store the mapped record in the array */
			} 
		}
		else
		{
			if ( numleadinfo != numleads)	 /* Do some checking */
		    		errWarning("ReadPotVals","Number of leads and number of badleads info is not equal");
	    
			for (p=0;p < numleads; p++)
			{
			    if (p < numleadinfo) /* properly check whether the lead number exists */
				arraydatauint[p] = d.leadinfo[p];  /* Store the mapped record in the array */
			} 
		}	
	    
   	}
   
	arraylist = d.arraylist;
   	arraylist[0] = d.potvalsarray;
	arraylist[1] = d.leadinfoarray;

	d.arraylist = NULL;
	d.potvalsarray = NULL;
	d.leadinfoarray = NULL;
	ReadPotValsCleanup(&d,NULL);		   		
   	return(arraylist);			       
}
    
/* fin */
