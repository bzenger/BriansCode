/* 
 *  mexReadTSDFC.c based   
 *
 *  Created by Jeroen Stinstra on Thu May 31 2002.
 *
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
	

/* 
    Some changes to the architechture:
    - fidnum is renamed fidset which is more appropriate
    - fidsinfo is renamed fidset and fidset has beome a cell array
       - an advantage of a cell array is that it may contain empty spots, if one wants to delete a set
         this can and the pointers from the fids do not need to be altered aas fidset just contains an
	 empty spot
*/	 
 
/* Do the standard includes */

#include <stdio.h>
#include <math.h>
#include <mex.h>
#include "gdbm.h"

/* include a bunch of functions */

#include "myfile.c"
#include "myerror.c" 
#include "misctools.c"


typedef struct myfids     { mxArray 		*fids;
			    mxArray 		*type;
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

mxArray		**GetFids(char *tsdfcfilename, char *tsdffilename);


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
    mxArray		**fidsarray;
    int			success;
    
    /* SETUP USAGE AND SUCCES VARIABLE */
    
    errUsage("[fids,fidset] = mexReadTSDFC(TSDFC-filename,TSDF-filename/key)"); 
    success = 0;
    
    /* SETUP POINTERS */
    
    tsdffilename = NULL;
    tsdfcfilename = NULL;
    fidsarray = NULL;
    
    while(1)
    {
    
        /* CHECK INPUT AND OUTPUT OF THE FUNCTION */
        if (nrhs != 2)		 { errError("mexFunction","The name of the TSDFC-file and the TSDF-file are needed"); break; }
        if (!mxIsChar(prhs[0]))  { errError("mexFunction","TSDFC-filename needs to be a string"); break; }
        if (!mxIsChar(prhs[1]))  { errError("mexFunction","TSDF-filename needs to be a string"); break; }
        if (nlhs < 1) 		 { errError("mexFunction","No output variables specified"); break; }
        if (nlhs > 2)		 { errError("mexFunction","Only two output arguments are generated"); break; }
        
        /* CONVERT THE INPUT AND OUTPUT STRINGS */
        
        if(!( tsdfcfilename = miscConvertString((mxArray *)prhs[0])))  { errError("mexFunction","Could not retrieve tsdfc-filename"); break; }
        if(!( tsdffilename = miscConvertString((mxArray *)prhs[1])))   { errError("mexFunction","Could not retrieve tsdf-filename"); break; }
        
        if(!( fidsarray = GetFids(tsdfcfilename,tsdffilename))) 
            { errError("mexFunction","Could not read fiducials"); break; }
  
        /* WE MADE IT UNTIL THE END OF THE LOOP */
        
        success = 1;
        break;
    }
    

    /* START THE CLEAN-UP */
    
    if (tsdfcfilename) mxFree(tsdfcfilename);
    if (tsdffilename)  mxFree(tsdffilename);
    
    /* RETURN DATA OR CLEANUP THE MESS */
    
    if (success)
    {
        if (fidsarray)
        {
            plhs[0] = fidsarray[0];
            plhs[1] = fidsarray[1];
            mxFree(fidsarray);
        }
        return;
    }
    else
    {
        if (fidsarray) mxFree(fidsarray);   /* For future use */
        mexErrMsgTxt("ERROR: an error has occured in mexReadTSDFC"); /* fork back to matlab with an error */
        return;
    }
}
    
mxArray **GetFids(char *tsdfcfilename, char *tsdffilename)
/* This function reads the entry from the TSDFC-file abd looks for the
   key tsdf-filename and puts the result in two structs. The latter is empty
   if the key is not found 
   
   fidsarray = an array of each fiducial, when a fiducial consists of multiple timeinstants it is broken down into individual ones
               the array stores the type and the fiducialnumber to trace back the origin and the make search a certain fiducial easy
   fidsetarray = a cell array containing the labels and the auditstrings of each set of fiducials	       
   
   INPUT : tsdfcfilename and tsdffilename, the latter is used as key into the tsdfc-file
   OUTPUT : returnarray, which consists of returnarray[0] = fidsarray and returnarray[1] = fidsetarray 
*/
{    
    GDBM_FILE 		gdbmfile = NULL;		/* the tsdfc file */
    datum		key,data;			/* the key in the tsdfc file */
    char		*cdata = NULL;
    char		*enddata = NULL;
    char 		*nextrec = NULL;		/* cdata/enddata/begindata/nextrec counts where we are in reading a memory block with fiducial data */
    short		type, version;			/* properties of the fiducials not used at the moment */
    int			size;				/* size of the memory block for one fiducial */
    int			fidnum;				/* counter on the number of the block we are reading */ 
    int			p;				/* multi-purpose counter for copying memory blocks */
    
    int			fidnamesize, auditsize; 	/* filename and audit storage and length counters */
    char		*fidname, *audit;
    
    int			fiddescarraysize; 		/* arrays to store the packed fiducial data */
    short		*fiddescarray;
    int			fidvaluesarraysize;
    float		*fidvaluesarray;
    short		*fidtypesarray;
    
    int			leadread;			 /*  boolean is the lead already assigned with a fiducial */
    int			fidnp,leadp,typenp,typep,numfidp,fidtype; /* a bunch of counters for resorting the fiducial information */
    int			fidn,lead,typen,numfid;
    int			matnumfidset,matnumfids; 	/* counter counting the numbver of elements in the linked lists */
    myfids		*firstfids,*currentfids; 	/* Fids storage, this is a linked list */
    double		*matrix; 			/* to store a pointer to the matrix with fiducial values to be filled out*/
    
    myfidset		*firstfidset,*currentfidset; 	/* FidsInfo storage this is a linked list   */
    
    mxArray		*fidsarray, *fidsetarray, *fidsetcellarray; /* the output arrays */
    const int		fidsnumfields = 3;  		/* define the structures */
    const char		*fidsfieldnames[] = {
			    "value",			/* fiducial time values in ms */
			    "type",    			/* the type of the fiducial */
			    "fidset" };			/* from which fiducial set is it originating ? */
    enum  fidsfields	{VALUE = 0, TYPE, FIDSET };			    
    const int		fidsetnumfields = 5;
    const char		*fidsetfieldnames[] = { 	/* an overview of the fiducial sets read into memory */
			    "filename", 		/* name of the tsdfc-file the fiducial came from */
			    "key",			/* name of the tsdf-file, which forms the key in unlocking the file */
			    "fidnum",			/* the number of the fiducial in the tsdfc-file */
			    "label",    		/* label of the fiducial set, normally the subprogram of Everett that created the fiducial */
			    "audit" };  		/* the log on how it was created */ 
    enum  fidsetfields	{FILENAME =0, KEY, FIDNUM, LABEL, AUDIT};
    double		NaN; 				/* Not a Number, matlab uses it to tell that the number is not tto be used */
    mxArray		**returnarray; 			/* return multiple arrays [0] = fidsarray, [1] = fidsinfoarray */    
    int			success, loopsuccess, loop2success;	
    
    /* SET UP MAIN LOOP */
    
    success = 0;
    
    /* INITIALSE SOME MORE POINTERS */
    
    data.dptr = NULL;
    data.dsize = 0;
    currentfids = NULL;
    firstfids = NULL;
    currentfidset = NULL;
    firstfidset = NULL;
    fidname = NULL;
    audit = NULL;
    fiddescarray = NULL; 
    fidvaluesarray = NULL; 
    fidtypesarray = NULL;
    fidsarray = NULL;
    fidsetarray = NULL;
    fidsetcellarray = NULL;
    gdbmfile = NULL;
    returnarray = NULL;
    
    /* MAIN EXECUTION LOOP */
    
    while(1)
    {
    
        if (!(returnarray = (mxArray **)mxCalloc(2,sizeof(mxArray *)))) 	{ errError("GetFids","Could not allocate memory (returnarray)"); break; }

        /* READ DATA FROM FILE */
        if (!(gdbmfile = gdbm_open(tsdfcfilename,1024,GDBM_READER|GDBM_NOLOCK,00644,0))) 	{ errError("GetFids","Could not open file"); break; }
        key.dptr = tsdffilename;
        key.dsize = strlen(tsdffilename);
        data = gdbm_fetch(gdbmfile,key); 					/* search for the requested key */
        gdbm_close(gdbmfile); gdbmfile = NULL;					/* Don't need that anymore */
        if (data.dptr == NULL)	{ errError("GetFids","Could not findkey"); break;}
	
        cdata = data.dptr; /* get pointer to datablock */
        enddata = data.dptr + data.dsize - 3;
    
        /* PROCESS THE DATABLOCK */
    
        fidnum = 0; 								/* counter for the fiducial number we are reading */
        
        /* Create a structure that will store the matrices for each individual field of the fids structured array
           As I do not know howmany fiducials there are, it does not make sense to create a matrix as a bundle to store them
            Hence the CurrentFids stores them all plus a pointer to the next field. FirstFids stores the pointer to the start of the list */
    
        if  (!(firstfids = (myfids *)mxCalloc(1,sizeof(myfids)))) { errError("GetFids","Could not allocate memory (myfids)"); break; }
        currentfids = firstfids;
        matnumfids = 0; 							/* to compute howmany fiducials there are in the list */
    
        /* Create another linked list for the fidsinfo */
    
        if (!(firstfidset = (myfidset *)mxCalloc(1,sizeof(myfidset)))) { errError("GetFids","Could not allocate memory (myfidset)"); break; }
        currentfidset = firstfidset;	
        matnumfidset = 0;    
    	
        NaN = mxGetNaN(); /* get the NaN number to initialise not used positions in the vector */

        loopsuccess = 0;
     
      
        while (cdata < enddata)
        { /* keep on reading we run out of loaded data */
    
            /* Read the record completely and store it into memory and them reproces the data to fit into matlab */
            
            /* READ FIDSET HEADER */

            cdata = (char *)mfmemread((void *)&type,sizeof(short),1,(void *)cdata,mfSHORT); 
            cdata = (char *)mfmemread((void *)&version,sizeof(short),1,(void *)cdata,mfSHORT); 
            cdata = (char *)mfmemread((void *)&size,sizeof(int),1,(void *)cdata,mfINT); 
        
            nextrec = cdata+size;
            if ((version == 0)&&(type == 1))	/* OTHERWISE BLOCK IS NO FIDSET */
            {
            
                /* Do the fiducials name */
                cdata = (char *)mfmemread((void *)&fidnamesize,sizeof(int),1,cdata,mfINT);
                if (!( fidname = (char *)mxCalloc(fidnamesize+1,sizeof(char))))
                    { errError("GetFids","Could not allocate memory (fidname)"); break; }
                cdata = (char *)mfmemread((void *)fidname,sizeof(char),fidnamesize,(void *)cdata,mfCHAR);  /* since calloc is used the last byte is automaticly 0 */
	
                /* Do the fiducials auditstring */
                cdata = (char *)mfmemread((void *)&auditsize,sizeof(int),1,(void *)cdata,mfINT); 
                if (!( audit = (char *)mxCalloc(auditsize+1,sizeof(char))))
                    { errError("GetFids","Could not allocate memory (audit)"); break; }
                cdata = (char *)mfmemread((void *)audit,sizeof(char),auditsize,(void *)cdata,mfCHAR);  /* since calloc is used the last byte is automaticly 0 */
	
                /* Do the description array */
                cdata = (char *)mfmemread((void *)&fiddescarraysize,sizeof(int),1,(void *)cdata,mfINT);
                if (fiddescarraysize > 0)
                {   
                    if (!( fiddescarray = (short *)mxCalloc(fiddescarraysize,sizeof(short))))
                            { errError("GetFids","Could not allocate memory (descarray)"); break; }
                    cdata = (char *)mfmemread(fiddescarray,sizeof(short),fiddescarraysize,(void *)cdata,mfSHORT); 
                }
        
                /* Do the fid values */
                cdata = (char *)mfmemread(&fidvaluesarraysize,sizeof(int),1,(void *)cdata,mfINT);
                if (fidvaluesarraysize > 0)
                {   
                    if (!( fidvaluesarray = (float *)mxCalloc(fidvaluesarraysize,sizeof(float))))
                            { errError("GetFids","Could not allocate memory (fidvalues)"); break; }        
                    cdata = (char *)mfmemread(fidvaluesarray,sizeof(float),fidvaluesarraysize,(void *)cdata,mfFLOAT); 
                    
                    /* The type array uses the same size as the fid values and thus is not repeated in the file structure */
		    if (!( fidtypesarray = (short *)mxCalloc(fidvaluesarraysize,sizeof(short))))
                            { errError("GetFids","Could not allocate memory (fidtypes)"); break; }
                    cdata = (char *)mfmemread((void *)fidtypesarray,sizeof(short),fidvaluesarraysize,(void *)cdata,mfSHORT);	        
                }
                
                /* the next block of data should start after this one */
                /* NEXT: fill out the fidsinfo structures */
                /* It is not the prettiest linked list, but it works */
	
                if (!( currentfidset->filename = mxCreateString(tsdfcfilename))) { errError("GetFids","Could not allocate string (filename)"); break; }
                if (!( currentfidset->key = mxCreateString(tsdffilename))) { errError("GetFids","Could not allocate string (key)"); break; }	
                if (!( currentfidset->fidnum = mxCreateScalarDouble((double)fidnum+1))) { errError("GetFids","Could not allocate string (fidnum)"); break; }
                if (!( currentfidset->label = mxCreateString(fidname))) { errError("GetFids","Could not allocate string (fidname)"); break; }
                if (!( currentfidset->audit = mxCreateString(audit))) { errError("GetFids","Could not allocate string (audit)"); break; }					
        
                if (!( currentfidset->next = (myfidset *)mxCalloc(1,sizeof(myfidset)))) { errError("GetFids","Could not allocate string (next)"); break; }

                currentfidset = currentfidset->next; /* prepare for next one */
                matnumfidset++; /* yes we read another one */    		

                typen = 0; 
                /* Analyse fiducial contents and prepare a matrix */
	
                loop2success = 1;
                
                for (lead=0;lead<fiddescarraysize;lead++)
                {
                    numfid = (int) fiddescarray[lead];
                    for(fidn=0;fidn<numfid;fidn++)
                    {
                        fidtype = (int) fidtypesarray[typen++];  
                        if (fidtype > -1)
                        {
                             if (!( currentfids->fids = mxCreateDoubleMatrix(fiddescarraysize,1,mxREAL)))
                                { errError("GetFids","Could not allocate matrix (fids)"); loop2success = 0; break; }
                            if (!( currentfids->type = mxCreateScalarDouble(fidtype)))
                                { errError("GetFids","Could not allocate matrix (fidtype)"); loop2success = 0; break; }	
                            if (!( currentfids->fidset = mxCreateScalarDouble(fidnum+1 ))) 
                                { errError("GetFids","Could not allocate matrix (fidnum)"); loop2success = 0; break; }
                            if (!( currentfids->next = (myfids *)mxCalloc(1,sizeof(myfids))))
                                { errError("GetFids","Could not allocate matrix (next)"); loop2success = 0; break; }

                            matrix = mxGetPr(currentfids->fids);	
                            /* initialise fiducials with NaNs */
                            for (p=0;p<fiddescarraysize;p++) matrix[p] = NaN;
		    
                            matnumfids++;
                            currentfids = currentfids->next;
		
                            /* In the next for loops, go through the data and find all entries with the same type
                               For each lead only one can be added. By setting the type to minus one after an entry is
                               read the program know which have been read and do not need to reappear in another 
                               fiducials matrix */
		
                            typenp = 0; /* register where we are in the fidsvaluesarray, since for every lead a different number of fiducials can be 
                                                        inserted this has to be done by a double for loop */
                            for (leadp=0;leadp<fiddescarraysize;leadp++)
                            {
                                numfidp = (int) fiddescarray[leadp];
                                leadread = 0;
                                for (fidnp=0;fidnp<numfidp;fidnp++)
                                {
                                    typep = fidtypesarray[typenp];
                                    if ((typep == fidtype)&&(!leadread))
                                    {
                                        fidtypesarray[typenp] = -1; /* mark this one as read */
                                        matrix[leadp] = (double) fidvaluesarray[typenp];
                                        leadread = 1;
                                    }
                                    typenp++;
                                }	
                            }
                        }							
                    }										    
                }

                
                if (fidname) mxFree(fidname); fidname = NULL;
                if (audit) mxFree(audit); audit = NULL;
                if (fiddescarray) mxFree(fiddescarray); fiddescarray = NULL;
                if (fidvaluesarray) mxFree(fidvaluesarray); fidvaluesarray = NULL;
                if (fidtypesarray) mxFree(fidtypesarray); fidtypesarray = NULL;
             
                if (loop2success == 0) break;        
                fidnum++; /* go to the next one */    
            }
            
            cdata = nextrec;
            if (cdata >= enddata) { loopsuccess = 1; break; }
          }

        if (loopsuccess == 0)
        {
            if (fidname) mxFree(fidname); fidname = NULL;
            if (audit) mxFree(audit); audit = NULL;
            if (fiddescarray) mxFree(fiddescarray); fiddescarray = NULL;
            if (fidvaluesarray) mxFree(fidvaluesarray); fidvaluesarray = NULL;
            if (fidtypesarray) mxFree(fidtypesarray); fidtypesarray = NULL;
            break;
        }    

        if (!( fidsarray = mxCreateStructMatrix(1,matnumfids,fidsnumfields,fidsfieldnames))) 
                { errError("GetFids","Could not allocate struct"); break; }
                
        /* Convert the linked list into a matlab struct array ; the loop reads each next element and fills it out */ 
    
        currentfids = firstfids;
        for (p=0;p<matnumfids;p++) /* read out the linked list item by item */
        {
            if (!currentfids) break;
            mxSetFieldByNumber(fidsarray,p,VALUE,currentfids->fids);
            mxSetFieldByNumber(fidsarray,p,TYPE,currentfids->type);
            mxSetFieldByNumber(fidsarray,p,FIDSET,currentfids->fidset);
            currentfids->fids = NULL;
            currentfids->type = NULL;
            currentfids->fidset = NULL;;
            if (currentfids->next != NULL)
                currentfids = currentfids->next;
            else
                break;
        }	
    
        /* Construct the final fidsinfo matrix */
        
        if (!( fidsetcellarray = mxCreateCellMatrix(1,matnumfidset)))
                { errError("GetFids","Could not allocate cell array"); break; }
	
        currentfidset = firstfidset;
        loop2success = 1;
        for (p=0;p<matnumfidset;p++) /* read out the linked list item by item */
        {
            if (!currentfidset) break;
            if (!( fidsetarray = mxCreateStructMatrix(1,1,fidsetnumfields,fidsetfieldnames)))
                    { errError("GetFids","Could not allocate struct"); loop2success = 0; break; }

            mxSetFieldByNumber(fidsetarray,0,FILENAME,currentfidset->filename);
            mxSetFieldByNumber(fidsetarray,0,KEY,currentfidset->key);
            mxSetFieldByNumber(fidsetarray,0,FIDNUM,currentfidset->fidnum);
            mxSetFieldByNumber(fidsetarray,0,LABEL,currentfidset->label);
            mxSetFieldByNumber(fidsetarray,0,AUDIT,currentfidset->audit);
            currentfidset->filename = NULL;
            currentfidset->key = NULL;
            currentfidset->fidnum = NULL;
            currentfidset->label = NULL;
            currentfidset->audit = NULL;
            mxSetCell(fidsetcellarray,p,fidsetarray); /* link to cell array */
            if (currentfidset->next != NULL)  /* additional check whether the linkeed list is complete */
                currentfidset = currentfidset->next;
            else
                break; /* no more to read */
        }
    
        if (loop2success == 0) break;

        success = 1;
        break;
    }
    
    /* FREE UP RESOURCES */
    
    if (data.dptr)	free(data.dptr);

    if (firstfidset)
    {
        currentfidset = firstfidset;
        while(currentfidset)
        {
            if (currentfidset->filename) mxDestroyArray(currentfidset->filename);
            if (currentfidset->key) mxDestroyArray(currentfidset->key);
            if (currentfidset->fidnum) mxDestroyArray(currentfidset->fidnum);
            if (currentfidset->label) mxDestroyArray(currentfidset->label);
            if (currentfidset->audit) mxDestroyArray(currentfidset->audit);
            if (currentfidset->next != NULL)  /* additional check whether the linked list is complete */
            {
                firstfidset = currentfidset;
                currentfidset = currentfidset->next;
                mxFree(firstfidset);
            }
            else
                break; /* no more to read */
        
        }
    }

    if (firstfids)
    {
        currentfids = firstfids;
        while(currentfids)
        {
            if (currentfids->fids) mxDestroyArray(currentfids->fids);
            if (currentfids->type) mxDestroyArray(currentfids->type);
            if (currentfids->fidset) mxDestroyArray(currentfids->fidset);
            if (currentfids->next != NULL)  /* additional check whether the linked list is complete */
            {
                firstfids = currentfids;
                currentfids = currentfids->next;
                mxFree(firstfids);
            }
            else
                break; /* no more to read */
        
        }
    }

 
    if (success)
    {
        returnarray[0] = fidsarray;   /* Set the output array of array pointers */
        returnarray[1] = fidsetcellarray;
        return(returnarray);    
    }
    else
    {
        if (fidsarray) mxDestroyArray(fidsarray);
        if (fidsetcellarray) mxDestroyArray(fidsetcellarray);
        if (returnarray) 
        {
                if (!( fidsarray = mxCreateStructMatrix(1,0,fidsnumfields,fidsfieldnames))) 
                    { errError("GetFids","Could not allocate struct"); 
                      mxFree(returnarray); return(NULL); }
                if (!( fidsetcellarray = mxCreateCellMatrix(1,0)))
                    { errError("GetFids","Could not allocate cell array");
                      mxFree(fidsarray);
                      mxFree(returnarray); 
                      return(NULL); 
                    }
                    
        
            returnarray[0] = fidsarray;
            returnarray[1] = fidsetcellarray;
            return(returnarray);
        
        }
        mxFree(returnarray);
        return(NULL);
    }
}
 
    
       
          
             
                
                   
   
