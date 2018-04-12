/* 
 *  mexReadTSDFClib.c based   
 *
 *  Created by Jeroen Stinstra on Thu May 31 2002.
 *
 *  This is an updated version of mexReadTSDFC.c using the TSDFClib
 */
 
 /* 
   This function reads a TSDFC file and retrieves the fiducial sets recorded for one TSDF-file. 
 */
 
 
/* 
    The data is reordered in the following way
    In matlab each fiducial entry consists of a type and number for the fiducial for each channel or just one number for all channels
    In everett more fiducials are grouped into a fiducial entry, with a label and audit as they originate from the same sub-program.
    Well in matlab, we do not use this grouping of fiducials. Hence this function ungroups them all. 
    The function reads one set of fiducials and determines how many subgroups there are. mexReadTSDFC considers something a subgroup if they have
    the same type. If more than one fiducial of the same type is grouped, matlab selects the first one, groups it into a matlab-fiducial then graps the 
    second, groups it and so on. In order to be able to store everything back in a container file, the number of fidset where the fiducial was derived from 
    is stored as well. 
    
    To be able to gather more info on the fiducial groups, a fidsinfo is derived which stores the fiducial set labels and audit strings. So if you want
    the original label get the fidnum in the fids array and use this to index into thefidsinfo array. Thus the fidsinfo and fids array are not of the 
    same dimension.
    
*/    
	
/* Do the standard includes */

#include <stdio.h>
#include <math.h>
#include <mex.h>
#include "graphicsio.h"

/* All these includes are for the TSDFlib (which does TSDFC support :)) */
#include <string>
#include "fidset.h"
#include "container.h"
#include "fslist.h"
#include "pslist.h"
#include "paramset.h"
#include "gdbmkeys.h"  
#include "gdbm.h" 
#include "giofidset.h"

#include "mexio2.h"
#include "mexio2.c"     


/* My typedef declarations 
 * These structures are used to temporarily store the fiducials before compiling a final matrix 
 */

typedef struct myfids     { mxArray 		*fids;
			    mxArray 		*type;
                            mxArray		*name;
			    mxArray 		*fidset;
			    struct myfids	*next; 
			  } myfids;

typedef struct myfidset   { mxArray 		*filename;
			    mxArray 		*key;
			    mxArray 		*fidnum;
			    mxArray 		*label;
			    mxArray	 	*audit;
			    struct myfidset 	*next;
		          } myfidset;	


/* Function declarations  */

extern 		void _main();							/* Apparently you need this to do C++ things */


/* FUNCTION mexFunction  
 *
 * Entry point for MATLAB. 
 * lefthandside                        righthandside
 * [outmat1,outmat2,...] = function(inmat1,inmat2,...)
 *  nlhs - number of parameters on leftside hand of expression
 *  plhs - a pointer to an array of pointers representing the various arrays on this side
 *  nrhs - idem right hand side
 *  prhs - idem array pointer right hand side
 */

extern "C" {
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    char		*tsdffilename, *tsdfcfilename;
    int			result;
    
    int			numparamset;
    int			fidnum;				/* counter on the number of the block we are reading */ 
    int			p,q;				/* multi-purpose counter for copying memory blocks */
    
      
    int			fiddescarraysize; /* arrays to store the packed fiducial data */
    short		*fiddescarray;
    int			fidvaluesarraysize;
    float		*fidvaluesarray;
    short		*fidtypesarray,*temp;
    
    int			leadread; /*  boolean is the lead already assigned with a fiducial */
    int			fidnp,leadp,typenp,typep,numfidp,fidtype; /* a bunch of counters for resorting the fiducial information */
    int			fidn,lead,typen,numfid;
    int			matnumfidset,matnumfids; /* counter counting the numbver of elements in the linked lists */
    myfids		*firstfids,*currentfids; /* Fids storage, this is a linked list */
    double		*matrix; /* to store a pointer to the matrix with fiducial values to be filled out*/
    
    myfidset		*firstfidset,*currentfidset; /* FidsInfo storage this is a linked list   */
    
    mxArray		*fidsarray, *fidsetarray, *fidsetcellarray; /* the output arrays */
    const int		fidsnumfields = 4;  /* define the structures */
    const char		*fidsfieldnames[] = {
			    "value",	/* fiducial time values in ms */
			    "type",     /* the type of the fiducial */
                            "name",
			    "fidset" }; /* from which fiducial set is it originating ? */
    enum  fidsfields	{VALUE = 0, TYPE, NAME, FIDSET };			    
    const int		fidsetnumfields = 5;
    const char		*fidsetfieldnames[] = { /* an overview of the fiducial sets read into memory */
			    "filename", /* name of the tsdfc-file the fiducial came from */
			    "key",	/* name of the tsdf-file, which forms the key in unlocking the file */
			    "fidnum",	/* the number of the fiducial in the tsdfc-file */
			    "label",    /* label of the fiducial set, normally the subprogram of Everett that created the fiducial */
			    "audit" };  /* the log on how it was created */ 
    enum  fidsetfields	{FILENAME =0, KEY, FIDNUM, LABEL, AUDIT};
    double		NaN; /* Not a Number, matlab uses it to tell that the number is not tto be used */
    
    
    /*  Set usage string 
	The function input is: 
	    - TSDFC char matrix containing the filename
	    - TSDF char matrix containing the key 
	The output are two sets of arrays
	    - a fids array this one contains a fid entry and contains parts of the 
	      fidsets broken down in parts so in each entry there is one type of
	      fiducial and which can be a vector or scalar depending on the local or global 
	      status of the fiducial.
	    - fidset cell array, representing all fidsets in the timeseries, it contains all
	      the data like the auditstring, the label, but not the data as it is broken down
	      in smaller pieces in fids. fids does contain a pointer to fidset in the form of
	      an index.       
     */
    mexioSetSyntax("[fids,fidset] = mexReadTSDFC(TSDFC-filename,TSDF-filename/key)");

    /* Check the INPUT parameters */
    
    if (nrhs != 2) mexioReportError("mexFunction","The name of the TSDFC-file and the TSDF-file are needed");
    if (!mxIsChar(prhs[0])) mexioReportError("mexFunction","TSDFC-filename needs to be a string");
    if (!mxIsChar(prhs[1])) mexioReportError("mexFunction","TSDF-filename needs to be a string");

    /* Check the OUTPUT parameters */

    if (nlhs < 0) return; 								/* If no output is requested, don't do any work */
    if (nlhs > 2) mexioReportError("mexFunction","Only two output arguments are generated");
	
    /* Retrieve the filenames */

    tsdfcfilename = mexioConvertMString((mxArray *)prhs[0]);
    tsdffilename  = mexioConvertMString((mxArray *)prhs[1]);

    fidsarray = NULL;
    
    /* 
     * Going to C++ 
     */
    
     
    /* RETRIEVE THE FIDUCIALS */
    
    /* It turns out using GIOFidSet is best compatible with my C-code
     * So I decided not to use FSList and Container as they make the 
     * programming only more comnplicated and more inefficient
     *
     * Using this class seems the best option in getting a reasonable performance
     * using C++. Which allows some very nasty code to be produced
     *
     */
     
     NaN = mxGetNaN(); 									/* get the NaN number to initialise not used positions in the vector */ 
    

    {   
       GIOFidSet Fidset;
    
       /* Apperently the only way for this object to test whether there is a TSDFC/TSDF-entry is just to try
        * to read the number of fidset. So if no sensible number is retrieved it is assumed the file or the entry
        * does not exist
        * 
        */
          
        numparamset = Fidset.NumSets(tsdfcfilename,tsdffilename);   			/* Try to establish the number of fidsets */			
    }
    
             
    if(numparamset < 1)									/* Check whether the tsdf entry exists */
       mexioReportError("mexFunction","Could not find the specified TSDFC/TSDF-entry");	/* The entry does not exist in this one */
    
            
    /* PROCESS THE FIDUCIALS */
    
    /* PROCESS FIRST THE FIDSET INFORMATION HEADERS */
    
    /* It seems that TSDFlib does not check properly whether a parameterset is a fiducial set or not 
     * So make sure that we are only taking fidsets that are known at the moment
     *
     */
    
    
    /* Since it is not clear whether the number we retrieved is the actual number of fidsets.
     * i.e. other parameters set are assumed to be fidsets as well
     * 
     * To be sure we check the version and type numbers
     *
     * As dimensions are not clear it does not make sense to allocate a matrix for the total 
     * structure at the start. Hence this leaves us with a small problem being that their is
     * no space to store the read data.
     * Hence the following approach is taken:
     * The data is read is converted into submatrices of the fidset and fids matrices and is
     * temporarily stored in a linked list. Counters count the number of entries in the list
     * and when all data is read and the dimensions of the total matrices are known the linked
     * list is rewritten as a matlab matrix
     */ 
    
    /* Set-up the linked lists for both information matrices 
     * The head of the list is initiated and the pointer to the 
     * first entry is stored and the counters counting the number
     * of entries in the list is set to zero.
     */
    
    firstfids = (myfids *)mexioCalloc(1,sizeof(myfids)); 				/* linked list for fiducial sets of one type */
    currentfids = firstfids;								/* of course the current one is the first one */
    matnumfids = 0; 									/* number of fids in matlab */
    
     /* Create another linked list for the fidsinfo */
    
    firstfidset = (myfidset *)mexioCalloc(1,sizeof(myfidset));				/* linked list for fidsets */
    currentfidset = firstfidset;					
    matnumfidset = 0;        								/* number of fidsets for matlab code */
    
    
     for (p=0; p<numparamset; p++)							/* loop through all parameter sets */
     {
       /* Judging from the GIOFidset code
        *
        * It is not properly tested whether there is data in the fidset
        * So reusing an old object is probably not a good idea
        * 
        * Again C++ is great, but it is so fragile if you do not know
        * what the code underneath is doing
        *
        * So a solution to this problem is to define a new object in this
        * scope and pray that it is removed properly by the destructor again
        *
        */ 
        
        GIOFidSet 	myFidset;							/* This one is only here in this scope */
                 
        result = myFidset.ReadGIOFidSet(tsdfcfilename,tsdffilename,p);			/* Load the data into the fidset */
        
        if (result < 0) mexioReportError("mexFunction","Could not read fidset from TSDFC-file (file corrupt ?)");
     
        if ((myFidset.Type() != 1)&&(myFidset.Version() != 0))
        {
            mexPrintf("WARNING: Some unknown fidsets/parametersets were encountered, ignoring these sets as their types were not known when this function was written\n");
        }
        else
        {										/* Process the data in the fidset */
     
            if (!( currentfidset->filename = mxCreateString(tsdfcfilename))) mexioReportError("mexFunction","Could not allocate string (filename)");
            if (!( currentfidset->key = mxCreateString(tsdffilename))) mexioReportError("mexFunction","Could not allocate string (key)");
            if (!( currentfidset->fidnum = mxCreateScalarDouble((double)p+1))) mexioReportError("mexFunction","Could not allocate scalar (fidnum)");
            if (!( currentfidset->label = mxCreateString(myFidset.Name()))) mexioReportError("mexFunction","Could not allocate string (fidname)");
            if (!( currentfidset->audit = mxCreateString(myFidset.AuditString()))) mexioReportError("mexFunction","Could not allocate string (audit)");
	    
            currentfidset->next = (myfidset *)mexioCalloc(1,sizeof(myfidset));		/* It is already tested whether the memory is there in the function */
            currentfidset = currentfidset->next; 					/* prepare for next one */
            matnumfidset++; 								/* yes we read another fiducial set so count this one */    		

            typen = 0; 									/* Analyse fiducial contents and prepare a matrix */
	
            /* Get pointers and arrays from the object */
            fiddescarraysize = myFidset.DescArraySize();
            fiddescarray = (short *)(myFidset.DescArray());
            fidvaluesarraysize = myFidset.ArraySize();
            fidvaluesarray = (float *)(myFidset.ValueArray());
            
            /* Let's be nice and make my own copy
             * So I do not temper with the objects own 
             * data system
             */
            
            temp = (short *)(myFidset.TypeArray());
            fidtypesarray = (short *) mexioCalloc(fidvaluesarraysize,sizeof(short));
            memcpy(fidtypesarray, temp, fidvaluesarraysize*sizeof(short));
            
            /* Now start decoding the data 
             * The loop goes through all leads and examines the contents of the types
             * As soon as it discovers a new type, it will create a new fiducial entry
             * It will assign an empty matrix with NaNs every where and starts to fill 
             * this matrix. For this purpose it goes through the remaining leads to see
             * whether they got a lead of the same type as well. In case they do this
             * one is added to this fiducial vector. To know which fiducial values have
             * been processed the type is set to -1 after the corresponding value is read
             * hence it cannot be read twice.
             */
            for (lead=0;lead<fiddescarraysize;lead++)
            {
                numfid = (int) fiddescarray[lead];
                for(fidn=0;fidn<numfid;fidn++)
                {
                    fidtype = (int) fidtypesarray[typen++];  
                    if (fidtype > 0)
                    {
                        string 	name("unknown");							/* Name of the fiducial */
                        FidSet	FS;									/* For name conversion purposes only */	
                        if (fidtype < 17)								/* Otherwise the FI lib crashes :-( */
                            name = FS.GetFidName(fidtype);
                                    
                    
                        if (!( currentfids->fids = mxCreateDoubleMatrix(fiddescarraysize,1,mxREAL))) mexioReportError("mexFunction","Could not allocate matrix (fids)");
                        if (!( currentfids->type = mxCreateScalarDouble(fidtype))) mexioReportError("mexFunction","Could not create scalar (fidtype)");
                        if (!( currentfids->name = mxCreateString(name.c_str()))) mexioReportError("mexFucntion","Could not create a string (name)");
                        if (!( currentfids->fidset = mxCreateScalarDouble(fidnum+1 ))) mexioReportError("mexFunction","Could not create scalar (fidset)");
                        if (!( currentfids->next = (myfids *)mxCalloc(1,sizeof(myfids)))) mexioReportError("mexFunction","Could not allocate memory (next)");
                        matrix = mxGetPr(currentfids->fids);	
                        
                        for (q=0;q<fiddescarraysize;q++) matrix[q] = NaN;		/* initialise fiducials with NaNs */
		        matnumfids++;
                        currentfids = currentfids->next;
		
                        /* In the next for loops, go through the data and find all entries with the same type
                         * For each lead only one can be added. By setting the type to minus one after an entry is
		         * read the program knows which have been read and does not need to reappear in another 
		         * fiducials matrix 
                         */
		
                        typenp = 0; /* register where we are in the fidsvaluesarray, since for every lead a different number of fiducials can be 
                                    inserted this has to be done by a double for loop */

                        for (leadp=0;leadp<fiddescarraysize;leadp++)
                        {
                            numfidp = (int) fiddescarray[leadp];
                            leadread = FALSE;
                            for (fidnp=0;fidnp<numfidp;fidnp++)
                            {
                                typep = fidtypesarray[typenp];
                                if ((typep == fidtype)&&(!leadread))
                                {
                                    fidtypesarray[typenp] = -1; /* mark this one as read */
                                    matrix[leadp] = (double) fidvaluesarray[typenp];
                                    leadread = TRUE;
                                }
                                typenp++;
                            }	
                        }
                    }							
                }										    
            }
            
            mexioFree(fidtypesarray);    	
        }
    }

    /* Construct the final matrices */
    
    if (!( fidsarray = mxCreateStructMatrix(1,matnumfids,fidsnumfields,fidsfieldnames))) 
        mexioReportError("mexFunction","Could not allocate struct");
    
    /* Convert the linked list into a matlab struct array 
     * the loop reads each next element and fills it out 
     *
     * Each element is now put at its place
     */ 
 
 
    currentfids = firstfids;
    for (p=0;p<matnumfids;p++) /* read out the linked list item by item */
    {
        mxSetFieldByNumber(fidsarray,p,VALUE,currentfids->fids);
        mxSetFieldByNumber(fidsarray,p,TYPE,currentfids->type);
        mxSetFieldByNumber(fidsarray,p,NAME,currentfids->name);
        mxSetFieldByNumber(fidsarray,p,FIDSET,currentfids->fidset);
        if (currentfids->next != NULL) 								/* security check */
        {
            currentfids = currentfids->next;
        }    
        else
            break;										/* oops counted too much */
    }	
    
    /* Construct the final fidsinfo matrix 
     * Same system as the previous one only this one is a cell matrix   
     */
        
    if (!( fidsetcellarray = mxCreateCellMatrix(1,matnumfidset)))
        mexioReportError("mexFunction","Could not allocate cell array");
	
    currentfidset = firstfidset;
    for (p=0;p<matnumfidset;p++) /* read out the linked list item by item */
    {
        if (!( fidsetarray = mxCreateStructMatrix(1,1,fidsetnumfields,fidsetfieldnames)))
            mexioReportError("mexFunction","Could not allocate struct (fidsetarray)");

        mxSetFieldByNumber(fidsetarray,0,FILENAME,currentfidset->filename);
        mxSetFieldByNumber(fidsetarray,0,KEY,currentfidset->key);
        mxSetFieldByNumber(fidsetarray,0,FIDNUM,currentfidset->fidnum);
        mxSetFieldByNumber(fidsetarray,0,LABEL,currentfidset->label);
        mxSetFieldByNumber(fidsetarray,0,AUDIT,currentfidset->audit);
	
        mxSetCell(fidsetcellarray,p,fidsetarray); 						/* link to cell array */
        if (currentfidset->next != NULL)  							/* additional check whether the linkeed list is complete */
        {
            currentfidset = currentfidset->next;
        }
        else
            break; 										/* miscount ? */
    }
   	
    /*
     * Returning from C++
     */    
    
    mexioCleanup();									/* cleanup anything left */

    /* Fill out the return matrices */

    if(nlhs > 0) plhs[0] = fidsarray;
    if(nlhs > 1) plhs[1] = fidsetcellarray;
    
}}
    
  
