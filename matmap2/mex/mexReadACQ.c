/* FILENAME:  mexReadACQ.c
   AUTHOR:    JG STINSTRA
   CONTENTS:  This functions reads the contents of a ACQ file into matlab's memory
   LAST UPDATE: 4 JULY 2003 
*/

/*  The function reads the ACQ file into memory, the function can also be used to read a TSDF file in pieces,
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
    
/*
    Added support to read gainsettings per channel. So a new field will be read each time containing the gains per mapped channel
*/ 

/***************************
  HERE THE CODE STARTS
***************************/

/* Do the standard includes */

#include <stdio.h>
#include <math.h>
#include <mex.h>

#include "myindex.h"
#include "myfile.c"
#include "myindex.c"
#include "tstools.c"
#include "myerror.c"
#include "misctools.c"
#include "graphicsio.h"
 

typedef struct options {
    long	numtimeseries; /* the number of Timeseries to read, 0 means to read all */
    long 	*timeseries;   /* an array containing the indices of the timeseries to read */
    CBoolean 	scanfile;    /* TRUE if the file only needs scanning */
    long	numleadmaps; /* how many maps are there */
    mapdata	*leadmap;    /* the actual lead maps */
    long	numframemaps; /* how many frame maps are there */
    mapdata	*framemap;   /* the actual frame maps */
    long        numscalemaps;
    dmapdata    *scalemap;
    CBoolean	readfids;    /* do I need to get fiducials stored in tsdf file */
    } options;


/* To illustrate the format of an ACQ-file */
typedef struct MacMuxHeaderStruct {
    short numMuxedChans;
    short numHeaderBytes;
    long numRefBytes;
    long numMuxBytes;
    char patientName[80];
    char patientID[30];
    char diagnosis[80];
    char recordingCenter[80];
    char techInitials[12];
    char notes[256];
    char date[30];
    char time[12];
    struct {
        short year, month, day, hour, minute, second, dayOfWeek;
    } timeAndDate;
    short numberOfLeads;
    long numberOfFrames;
    long baseline0;
    long baseline1;
    long qTime;
    long sTime;
    long tTime;
    short sex;
    short sampleRate;
    short gain;
    char dataSourceBank1[14];
    char dataSourceBank2[14];
    char dataSourceBank3[14];
    char dataSourceBank4[14];
    char dataSourceBank5[14];
    char dataSourceBank6[14];
    char dataSourceBank7[14];
    char dataSourceBank8[14];
    char reserved[274];	/* We want sizeof(FileHeader) = 1024 */
} MacMuxHeaderStruct;


/* Declare my functions  */
    
mxArray 	*GetTimeSeries(char *filename,options *options); /* Read one series of time data */
mxArray 	**ReadPotVals(MFILE *FID,long numleads, long numframes, mapdata *leadmap,mapdata *framemap, dmapdata *scalemap); /* read potential values */


/* Main entry function description starts here */

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
    mxArray		*filenamearray, *optionsarray; /* to store the input arguments */

    mxArray		*TSdata, *cellarray;	/* Data structure containing all data that is used for one datasete */
    int                 p;  /* my loop counter */	

    /* On the input side multiple settings are allowed as long as there are fewer inputs than 4. */

    errUsage("data = mexReadACQ(filename,options)");

  /*****************
    CHECK  INPUT 
   *****************/
    if (nrhs == 0)
    {
        errError("mexFunction","You need to specify at least a filename");
        mexErrMsgTxt("ERROR\n");
	return;
    }    
	
    if (nrhs > 2)
    {
        errError("mexfunction","No more than two inputs are allowed");
	mexErrMsgTxt("ERROR\n");
        return;
    }    
	
    /* For looop reading arguments */
    
    filenamearray = NULL;
    optionsarray = NULL;
    
    /* Since we can be pretty flexible with input arguments, use their type to sort out which is which.
       So more combinations than the usage syntax states are allowed. Not that it matters, but the code
       is no big thing. It just requires one loop */
    
    for (p=0;p<nrhs;p++)
    { 
        /* loops through all input and selects which is which argument */
        if ((mxGetN(prhs[p])*mxGetM(prhs[p]))==0) continue;  /* no sense to check this one */
	if ((mxIsChar(prhs[p]))&&(filenamearray == NULL))  filenamearray = (mxArray *)prhs[p];    
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
	mexErrMsgTxt("ERROR\n");
        return;
    }    
    
  /*****************
    GET FILENAME
   *****************/
    
    if (filenamearray == NULL)
    {   /* Cannot do anything without a filename */ 
	errError("mexFunction","You should supply a filename");
	mexErrMsgTxt("ERROR\n");
        return;
    }
    else
    {	/* Get me a C-string of the filename */
	filename =  miscConvertString(filenamearray);
        if (filename == NULL)
        {
            errError("mexFunction","Could not convert filename");
	    mexErrMsgTxt("ERROR\n");
            return;
        }
    }
    
          
 /**************
   GET OPTIONS
  **************/  

    /* first set defaults */

    /* The first two options will be ignored, they don't matter for ACQ files */
    options.numtimeseries = 0; /* Read all */
    options.timeseries = NULL; /* Not yet a number of channels is specified */
    options.scanfile = FALSE;  /* Read all data */
    options.numleadmaps = 0; /* no Remapping */
    options.leadmap = NULL;
    options.numframemaps = 0; /* no remapping of frames */
    options.framemap = NULL;
    options.numscalemaps = 0; /* no remapping of scales */
    options.scalemap = NULL;
    options.readfids = FALSE; /* Ignore fiducials in file */

    if(optionsarray != NULL) /* an option structure is specified */
    {

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
      Check for lead scaling options 
     ************************************/    
    
     if(optionfield = mxGetField(prhs[1],0,"scalemap")) 
	{ 
            idxReadDMap(optionfield,&(options.scalemap),&(options.numscalemaps)); 	/* load the scalemap(s) */
        }    
    

    /************************************
      Check for lead remapping options 
     ************************************/
        
	if(optionfield = mxGetField(prhs[1],0,"leadmap")) 
	{ 
            idxReadIMap(optionfield,&(options.leadmap),&(options.numleadmaps)); 	/* load the leadmap(s) */
        } 
	   
    /************************************
      Check for frame remapping options 
     ************************************/
        
	if(optionfield = mxGetField(prhs[1],0,"framemap")) 
	{ 
            idxReadIMap(optionfield,&(options.framemap),&(options.numframemaps));	/* load the framemap(s) */
        }
        /* no more options to check for the moment */    
    }

 /***********************************************
   Do some consistency checking for the options
  ***********************************************/ 

   
    /* check the number of leadmaps and framemaps, there should be one for all or as many as timeseries specified */
    
    if ((options.numleadmaps != 0)&&(options.numleadmaps != 1))  
    {
    	errError("mexFunction","The number of Leadmaps specified should be one");
        mexErrMsgTxt("ERROR\n");
	mxFree(filename);
        return;
    }

    if ((options.numframemaps != 0)&&(options.numframemaps != 1))  
    {
    	errError("mexFunction","The number of FrameMaps specified should be one");
        mxFree(filename);
	mexErrMsgTxt("ERROR\n");
        return;
    }

	
 /***********************************************
   Do the main program loop reading the datasets
  ***********************************************/  
    /* OK now everything is set to read the individual data sets */

    /* Create a cell array to store each dataset in */
       
    if ( !(cellarray = mxCreateCellMatrix(1,1)))
    {
        errError("mexFunction","Could not create a cell array");
        mxFree(filename);
	idxFreeIMap(options.leadmap,options.numleadmaps);
    	idxFreeIMap(options.framemap,options.numframemaps);
    	idxFreeDMap(options.scalemap,options.numscalemaps);
	mexErrMsgTxt("ERROR\n");
        return;
    }    
    
    TSdata = NULL;
    TSdata = GetTimeSeries(filename,&options);
    mxSetCell(cellarray,0,TSdata);    

    /* now link TSdata to output */

    /* we already checked the number of output matrices and there is only one */
    plhs[0] = cellarray;

    idxFreeIMap(options.leadmap,options.numleadmaps);
    idxFreeIMap(options.framemap,options.numframemaps);
    idxFreeDMap(options.scalemap,options.numscalemaps);
    mxFree(filename);

    /* C'est tout */
}


/**********************************************************************/
/* Sub functions start here */


mxArray *GetTimeSeries(char *filename,options *options)
/* 
    This function retrieves one timeseries from the file
    and puts it directly into a matlab structure
    Options contains a field describing the options that are specified */

/* This function is an adapted version of GetTSInfo */ 
{
    MFILE* FID;                  /* To store file pointer */
    
    /* fields to read */
    long numleads;        	/* Number of leads in this Time Series (TS) */
    long numframes;       	/* Number of frames in this TS */

    
    /* If we encounter other formatted files in matlab, this routine should be able to handle them */
    
    mapdata			*leadmap,*framemap; /* to store remapping pointers */
    dmapdata                    *scalemap; /* scaling map */

    mxArray		*TSarray; /* The structure that is being build */
    mxArray		**arraylist; /* pointer to a list of pointers */


    /* define structure of array */

    const char 		*TSfieldnames[] = {
			    "filename",
                            "label",
			    "potvals",
                            "gain",
                            "numleads",
                            "numframes",
			    "leadinfo",
			    "unit",
 			    "geom",
			    "geomfile",
			    "expid",
			    "text",
			    "audit",
                            "time",
                            "fids",
                            "fidset" };
    /* this enum should mirror the field names array */	
      
    enum  TSfields	{ FILENAME = 0, LABEL, POTVALS, GAIN, NUMLEADS, NUMFRAMES, LEADINFO, UNIT, GEOM, GEOMFILE, EXPID, TEXT, AUDIT, TIME, FIDS, TSFIDSET };
    	 
    
    const int		numTSfields = 14;
    mapdata		*maparray;
    dmapdata            *dmaparray;
 
   /* For reading the fiducials if requested from the tsdf files */
    
    int                 count; 
    char                buffer[258];
    int			success;
    short 		nleads;
   
 /**************
   Open file
  **************/    

    leadmap = NULL;
    framemap = NULL;
    scalemap = NULL;
    success = 1;

    if ( !(FID = mfopen(filename,"r") )) 
    {
        errError("GetTimeSeries","Could not open file");
        return(NULL);
    }    
    
 /******************
   Set mapping data
  ******************/
    
    /* Read lead mapping indices  */
    if (options->numleadmaps == 0) { leadmap = NULL; }
    else if (options->numleadmaps == 1) { maparray = options->leadmap; leadmap = &(maparray[0]); }
   
    /* Read frame mapping indices */
    if (options->numframemaps == 0) { framemap = NULL; }
    else if (options->numframemaps == 1) { maparray = options->framemap; framemap = &(maparray[0]); }

    /* Read scale mapping indices */
    if (options->numscalemaps == 0) { scalemap = NULL; }
    else if (options->numscalemaps == 1) { dmaparray = options->scalemap; scalemap = &(dmaparray[0]); }
    
 /*****************************
   read the header of the file 
  ******************************/

    if(!(TSarray = mxCreateStructMatrix(1,1,numTSfields,TSfieldnames)))
    { 
        mfclose(FID); 
        return(NULL);
    }

    /* FILL OUT THE TS-STRUCTURE WITH DATA FROM THE HEADER */ 


    /* NUMLEADS */
    
    mfseek(FID,606,SEEK_SET);
    count = mfread((void *)&nleads,sizeof(short),1,FID,mfSHORT);
    if (count != 1) { errError("GetTimeSeries","Could not read number of leads"); success = 0; }
    numleads = (long) nleads;

    /* NUMFRAMES */

    mfseek(FID,608,SEEK_SET);
    count = mfread((void *)&numframes,sizeof(long),1,FID,mfLONG);
    if (count != 1) { errError("GetTimeSeries","Could not read number of frames"); success = 0; }

     /* TIME STAMP */

    mfseek(FID,580,SEEK_SET);
    count = mfread((void *)buffer,sizeof(char),12,FID,mfCHAR);
    if (count != 12)  { errError("GetTimeSeries","Could not read time label"); success = 0; }
    buffer[buffer[0]+1] = 0;
    if (!(tsPutTSstring(TSarray,0,TIME,&(buffer[1])))) success = 0;

    /* LABEL */
    
    mfseek(FID,122,SEEK_SET);
    count = mfread((void *)buffer,sizeof(char),80,FID,mfCHAR);
    if (count != 80)  { errError("GetTimeSeries","Could not read experiment label"); success = 0; }
    buffer[buffer[0]+1] = 0;
    if (!(tsPutTSstring(TSarray,0,LABEL,&(buffer[1])))) success = 0;
    
    /* GEOMETRY FILE */

    if (!(tsPutTSstring(TSarray,0,GEOMFILE,""))) success = 0; 

    /* TEXT */

    mfseek(FID,214,SEEK_SET);
    count = mfread((void *)buffer,sizeof(char),256,FID,mfCHAR);
    if (count != 256) { errError("GetTimeSeries","Could not read experiment notes"); success = 0; }
    buffer[buffer[0]+1] =0;
    if (!(tsPutTSstring(TSarray,0,TEXT,&(buffer[1])))) success = 0;

    
    /* FILENAME */

    if (!(tsPutTSstring(TSarray,0,FILENAME,filename))) success = 0;


    /* AUDIT STRING */

    if (!(tsPutTSstring(TSarray,0,AUDIT,"ACQ-file converted into TSDF-file by mexReadACQ"))) success = 0;


    /* UNITS */

    if (scalemap == NULL)
    {
        if (!(tsPutTSstring(TSarray,0,UNIT,"bits"))) success = 0;
    }
    else
    {
        if (!(tsPutTSstring(TSarray,0,UNIT,"mV"))) success = 0;
    }
        

    /* EXPID */
    if (!(tsPutTSstring(TSarray,0,EXPID,""))) success = 0;

    /****************************************************
      Elements which will be read as well in normal mode 
     ****************************************************/

    mfseek(FID,1024,SEEK_SET);
    if (options->scanfile == FALSE)
    {	
       
        /* POTENTIAL DATA */

	arraylist = ReadPotVals(FID,numleads,numframes,leadmap,framemap,scalemap);
        if (arraylist == NULL)
        {
	    errError("GetTimeSeries","Could not read potvals/leadinfo/gain");
            success = 0;
        }
        else
        {
            mxSetFieldByNumber(TSarray,0,POTVALS,arraylist[0]);     
            numleads = mxGetM(arraylist[0]);
            numframes = mxGetN(arraylist[0]);
    
            mxSetFieldByNumber(TSarray,0,LEADINFO,arraylist[1]);
            mxSetFieldByNumber(TSarray,0,GAIN,arraylist[2]);
            
            mxFree(arraylist); arraylist = NULL;
        }    
    }
         
    /* NUMLEADS AND NUMFRAMES */

    if (leadmap) numleads = leadmap->num;
    if (framemap) numframes = framemap->num;
    
    if (!( tsPutTSlong(TSarray,0,NUMLEADS,(long) numleads))) success = 0;
    if (!( tsPutTSlong(TSarray,0,NUMFRAMES,(long) numframes))) success = 0;
    
    
    if (success == 0)
    {
        errError("GetTimeSeries","Could not read all fields");
        mxDestroyArray(TSarray);
        TSarray = NULL;
    }

    mfclose(FID); /* close file and return to main program */
    return(TSarray);
}



mxArray **ReadPotVals(MFILE* FID, long numleads, long numframes, mapdata *leadmap, mapdata *framemap, dmapdata *scalemap)
/*  Read the potential values based on all data avaible
    Function returns an array of matlab arrays as multiple arrays are build in this function 
    The lead info is now processed in this fucntion as well, as it depends on the mapping data */
{
    CBoolean		remapping,scaling,warning,createdleadmap,createdframemap; /* Yes or no */
    short		*databuff;
    double		*arraydata, *gainarraydata;
    mxArray		*array, *gainarray,*leadinfoarray; 
    mxArray		**arraylist; /* first one being the potvals matrix, second the leadinfo and third the gainsetting per channel */
    long		p,q; /* loop counters */
    long		pp,qq; /*additional counters */
    double              ss;  /* temp field for scaling */
    long		lnum,fnum; /* new lead and frame stats */
    int                 count;
    long		r1,r2,gain;
    long		n,m;
	long		muxshift;


    scaling = FALSE;
    remapping = FALSE;
    warning = FALSE;
    createdleadmap = FALSE;
    createdframemap = FALSE;
 
    /*************************************
     Build an array of matlabarray pointers
     *************************************/

    if( !(arraylist = (mxArray **)mxCalloc(3,sizeof(mxArray *))))
    { 
        errError("ReadPotVals","Could not obtain enough memory"); 
        return(NULL); 
    }

    /*************************************
      Load the data into memory 
     *************************************/

    if( !(databuff = (short *)mxCalloc(numleads*numframes,sizeof(short))))
    { 
        errError("ReadPotVals","ould not obtain enough memory for databuffer");
        mxFree(arraylist);
        return(NULL); 
    }

    count = mfread(databuff,sizeof(short),numleads*numframes,FID,mfSHORT);
   
    if (count < numleads*numframes)
        numframes = (long) count/numleads;

    /*************************************
      First do some more integrity checks 
     *************************************/
  
    if ((leadmap != NULL)||(framemap != NULL))
    {   /* Check whether indices in remapping data go beyond boundaries */
        if (leadmap != NULL)
      	    for(p = 0; p < leadmap->num; p++)
	        if ( (leadmap->map[p] < 1)||(leadmap->map[p] > numleads))
                { 
                    errError("ReadPotVals","leadmap values are out of range");
                    mxFree(databuff);
                    mxFree(arraylist);
                    return(NULL); 
                }
 
        if (framemap != NULL)
    	    for(p = 0; p < framemap->num; p++)
	        if ( (framemap->map[p] < 1)||(framemap->map[p] > numframes))
	        { 
                    errError("ReadPotVals","framemap values are out of range");
                    mxFree(databuff);
                    mxFree(arraylist);
                    return(NULL); 
                }
 
	remapping = TRUE;	     
    }

    if ((scalemap != NULL)&&(leadmap != NULL))
    { /* check whether they have same dimension */
        if(scalemap->num != leadmap->num)
        { 
             errError("ReadPotVals","scalemap and leadmap should have same dimensions");
             mxFree(databuff);
             mxFree(arraylist);
             return(NULL); 
        }
     }

    if ((scalemap != NULL)&&(leadmap == NULL))
    { /* check whether they have same dimension */
        if(scalemap->num != numleads)
	{ 
             errError("ReadPotVals","improper size of the scalemap");
             mxFree(databuff);
             mxFree(arraylist);
             return(NULL); 
        }
    }


    if (scalemap !=NULL) scaling = TRUE;

 /*******************************************************
   Complete the mapping structure, I'll need them anyway 
  *******************************************************/
  
    /* Complete the mapping scheme if necessary, meaning you do both variables in mapping mode
       or none, so if one is specified just make the other as well.
       Besides I need the structures completed to fill out the mapping scheme, so
       if none is provided it is just an array going from 1 to number of leads/frames */
   
    if (remapping)       
    {
        if (leadmap == NULL)
        {
            leadmap = (mapdata *)mxCalloc(1,sizeof(mapdata));
            if (leadmap != NULL) 
            {
                leadmap->map = (long *)mxCalloc(numleads,sizeof(long));
                leadmap->num = numleads;
                if (leadmap->map == NULL) {mxFree(leadmap); leadmap = NULL;}
            }
        
            if (leadmap == NULL)
            {
                errError("(ReadPotVals","Could not allocate enough memory");
                mxFree(databuff);
                mxFree(arraylist);
                return(NULL); 
            }
            createdleadmap = TRUE;
            for ( p = 0; p < numleads; p++ ) leadmap->map[p] = p+1; /* fill out the normal mapping you would expect starting 1 endind at numLeads */
        }
        if (framemap == NULL)
        {
            framemap =(mapdata *)mxCalloc(1,sizeof(mapdata));
            if (framemap  != NULL)
            {
                framemap->map = (long *)mxCalloc(numframes,sizeof(long));
                framemap->num = numframes;
                if (framemap->map == NULL) {mxFree(framemap); framemap = NULL;}
            }
       
            if (framemap == NULL)
            {
                errError("ReadPotVals","Could not allocate enough memory");
                mxFree(databuff); mxFree(arraylist);
                if (createdleadmap) { mxFree(leadmap->map); mxFree(leadmap); }
                return(NULL); 
            }
            createdframemap = TRUE;
            for ( p = 0; p < numframes; p++ ) framemap->map[p] = p+1; /* fill out the normal mapping you would expect starting 1 endind at numLeads */
        }
    }
 
    /**********************************************************
      Four ways of importing data, with or without mapping, with or without scaling
     **********************************************************/
   
    if (remapping)
    {
        m = leadmap->num;
        n = framemap->num;
    }
    else
    {
        m = numleads;
        n = numframes;
    }
   
   muxshift = numleads >> 6;
   if (muxshift == 0) muxshift = 1;
   muxshift = muxshift*4;
   
   array = mxCreateDoubleMatrix(m,n,mxREAL);
   gainarray = mxCreateDoubleMatrix(m,1,mxREAL);
   leadinfoarray = mxCreateDoubleMatrix(m,1,mxREAL);
   
   if ((array == NULL)||(gainarray == NULL)||(leadinfoarray == NULL))
   {
        errError("ReadPotVals","Could not allocate enough memory matlab arrays");
        mxFree(databuff); mxFree(arraylist);
        if (createdleadmap) { mxFree(leadmap->map); mxFree(leadmap); }
        if (createdframemap) { mxFree(framemap->map); mxFree(framemap); }
        if (array) mxDestroyArray(array);
        if (gainarray) mxDestroyArray(gainarray);
        if (leadinfoarray) mxDestroyArray(leadinfoarray);
        return(NULL); 
    }
   
   if (remapping&&(!scaling))
   { /* load data so they can be adjusted to the mapping scheme supplied */

        arraydata = mxGetPr(array);
        gainarraydata = mxGetPr(gainarray);
		lnum = leadmap->num;  /* new dimensions of the resulting matrix */
		fnum = framemap->num;
		for(p = 0; p < lnum; p++)
        {
            for(q = 0; q < fnum; q++)
			{
				pp = (leadmap->map[p]-1);
				qq = (framemap->map[q]-1);
				if((pp>=0)&&(qq>=0)) /* Else data is not specified */
				arraydata[p+(q*lnum)] = (double) ((0x0FFF&databuff[pp+(qq*numleads)])-2048); /* do casting and mapping all in one step */
			}	
            qq = (framemap->map[0]-1);
            pp = (leadmap->map[p]-1);
            gainarraydata[p] = (double) ((0x7000&databuff[pp]) >> (short) 12);
			if (0x8000&databuff[pp]) gainarraydata[p] = (double) ((0x7000&databuff[pp+muxshift]) >> (short) 12);
        }    	
   }
   
   if ((!remapping)&&(!scaling))
   { /* load it the tradional way, just do a casting operation */
   
        arraydata = mxGetPr(array);
        gainarraydata = mxGetPr(gainarray);
                
		for(p = 0;p < (numleads*numframes); p++)
		{
			arraydata[p] = (double) ((0x0FFF&databuff[p])-2048);
		}
        for (p=0; p < numleads;p++)
        {
            gainarraydata[p] = (double) ((0x7000&databuff[p]) >> (short) 12);
			if (0x8000&databuff[p]) gainarraydata[p] = (double) ((0x7000&databuff[p+muxshift]) >> (short) 12);
        }
   
   }

   if (remapping&&scaling)
   { /* load data so they can be adjusted to the mapping scheme supplied */

        arraydata = mxGetPr(array);
        gainarraydata = mxGetPr(gainarray);
        
		lnum = leadmap->num;  /* new dimensions of the resulting matrix */
		fnum = framemap->num;
		for(p = 0; p < lnum; p++)
        {
            qq = (framemap->map[0]-1);
            pp = (leadmap->map[p]-1);
            gainarraydata[p] = (double) ((0x7000&databuff[pp+(qq*numleads)]) >> (short) 12); 
            if (0x8000&databuff[pp+(qq*numleads)]) gainarraydata[p] = (double) ((0x7000&databuff[pp+(qq*numleads)+muxshift]) >> (short) 12); 
            
            if (scalemap->numcol > 1)
            {
                r1 = scalemap->num; 
                r2 = scalemap->numcol;
                for(q = 0; q < fnum; q++)
                {
                    pp = (leadmap->map[p]-1);
                    qq = (framemap->map[q]-1);
					
                    gain = ((0x7000&databuff[pp]) >> (short) 12);
					if (0x8000&databuff[pp]) gain = ((0x7000&databuff[pp+muxshift]) >> (short) 12);
                    if (gain < r2)
                    {
                        ss = (scalemap->map[p+r1*gain]);
                        if (ss == 0.0) { ss = 1.0; warning = 1; }
                    }
                    else
                    {
                        ss = 1;
                        warning = 1;
                    }    
                        
                    if((pp>=0)&&(qq>=0)) /* Else data is not specified */
                        arraydata[p+(q*lnum)] = (double) ((0x0FFF&databuff[pp+(qq*numleads)])-2048)*ss; /* do casting and mapping all in one step */
                }	
            }
            else
            {
                for(q = 0; q < fnum; q++)
                {
                    pp = (leadmap->map[p]-1);
                    qq = (framemap->map[q]-1);
                    ss = (scalemap->map[p]);
                    if((pp>=0)&&(qq>=0)) /* Else data is not specified */
                        arraydata[p+(q*lnum)] = (double) ((0x0FFF&databuff[pp+(qq*numleads)])-2048)*ss; /* do casting and mapping all in one step */
                }		
            }
        }
   }
   
   if ((!remapping)&&(scaling))
   { /* load it the tradional way, just do a casting operation */
   
        arraydata = mxGetPr(array);
        gainarraydata = mxGetPr(gainarray);
                
		for(p = 0;p < numleads; p++)
        {
           gainarraydata[p] = (double) ((0x7000&databuff[p]) >> (short) 12);      
		   if (0x8000&databuff[p]) gainarraydata[p] = (double) ((0x7000&databuff[p+muxshift]) >> (short) 12);  
           
           if (scalemap->numcol > 1)
           {
                r1 = scalemap->num;
                r2 = scalemap->numcol;
                
				gain = ((0x7000&databuff[p]) >> (short) 12);
				if (0x8000&databuff[p]) gain = ((0x7000&databuff[p+muxshift]) >> (short) 12);
                for(q=0;q<numframes;q++)
                {
                    if (gain < r2)
                    {
                        ss = (scalemap->map[p+gain*r1]);
                        if (ss == 0.0) { ss = 1.0; warning = 1;}		/* In case a zero is entered in the matrix this means no calibration factor is known */
                    }
                    else
                    {
                        ss = 1;							/* Matrix is just too small */
                        warning = 1;
                    }    
                    arraydata[p+q*numleads] = (double) ((0x0FFF&databuff[p+(q*numleads)])-2048) * ss;
                }
           
           }
           else
           {
				ss = (scalemap->map[p]);
                for(q=0;q<numframes;q++)
                {
                    arraydata[p+q*numleads] = (double) ((0x0FFF&databuff[p+(q*numleads)])-2048) * ss;
                }
           }     
        }              
   
    }   

    if (warning)
    {
        errWarning("ReadPotVals","The scaling matrix is incomplete, not every lead can be calibrated");
        errWarning("ReadPotVals","Some channels were not calibrated");
    }


    arraylist[0] = array;
    arraylist[1] = leadinfoarray;
    arraylist[2] = gainarray;
    
    mxFree(databuff); 
    if (createdleadmap) { mxFree(leadmap->map); mxFree(leadmap); }
    if (createdframemap) {mxFree(framemap->map); mxFree(framemap); }

   return(arraylist);			       
}
  
/* fin */
