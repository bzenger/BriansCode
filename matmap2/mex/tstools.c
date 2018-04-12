/* FILENAME:  tstools.c
   AUTHOR:    JG STINSTRA
   CONTENTS:  Tools for editing the TS array (matlab structure)
   LAST UPDATE: 4 JULY 2003 
*/

#ifndef TSTOOLS_C
#define TSTOOLS_C 1

#include "tstools.h"

#include "myerror.c"
#include "misctools.c"

/* Put data in a timeseries (TS) (matlab-structure) */

int 	 tsPutTSlong(mxArray *TS,long arrayindex,long idnumber, long value)
{
    mxArray *array;
    double  *arraydata;
    
    if (!(array = mxCreateDoubleMatrix(1,1,mxREAL))) return (0); 		
    if (!(arraydata = mxGetPr(array))) { mxDestroyArray(array); return (0); } 	
    arraydata[0] = (double)value;  
    mxSetFieldByNumber(TS,arrayindex,idnumber,array);
    return (1); 
}


int 	 tsPutTSstring(mxArray *TS,long arrayindex,long idnumber, char *string)
{
    mxArray	*array;
      
    if (!(array = mxCreateString(string))) return (0); 
    mxSetFieldByNumber(TS,arrayindex,idnumber,array);
    return (1);  
}	

int 	 tsPutTSunit(mxArray *TS,long arrayindex, long idnumber, long unit)
{
    mxArray	*array;
    char	*unitname[] = { "mV","uV","ms","V","mVms" };
    long	numunitname = 5;
    double	*data;		
    
    if ((unit == 0)||(unit > numunitname))
	{    
	    if (!(array = mxCreateDoubleMatrix(1,1,mxREAL))) return (0);
	    if (!(data = mxGetPr(array))) {mxDestroyArray(array); return(0); }
	    data[0] = (double)unit;   
	}
	else 
	{
	    if (!(array= mxCreateString(unitname[unit-1]))) return (0);
	}				

    mxSetFieldByNumber(TS,arrayindex,idnumber,array);
    return (1);  
}


mxArray *tsFetchTS()
{
    /* Fetch the data structure from the global work space */

    const char		*varname = "TS"; /* TS = timeseries */
    const char		*workspace = "global";
    mxArray		*array;

#if ! defined (mexGetArrayPtr)
    array = (mxArray *) mexGetArrayPtr(varname,workspace); /* I know it should be const, but then I have to alter all my pointer types */
#else
    array = (mxArray *) mexGetVariablePtr(varname,workspace); /* I know it should be const, but then I have to alter all my pointer types */   
#endif
    return(array); /* get the pointer from matlab */
}
 
								 			
/* Get data from TS (matlab array) structure */

int	 tsGetTSstring(mxArray *TS,long arrayindex, char *idstring,char **string)
{
    mxArray 	*array;
    
    *string = NULL;

    if (!( array = mxGetField(TS,(int)arrayindex,idstring))) return (0);
    if (!(mxIsChar(array))) return(0);

    *string = miscConvertString(array);
    if (*string == NULL) return(0);
    return(1);
}    

int 	 tsGetTSlong(mxArray *TS,long arrayindex,char *idstring, long *value)
{
    mxArray 	*array;
    long	*ldata;
    short     	*sdata;
    char	*cdata;
    
    if (!( array = mxGetField(TS,(int)arrayindex,idstring))) return (0);
    
    if ((mxIsDouble(array)))
    {
    	*value = (long) mxGetScalar(array);
    	return(1); 
    }

    if ((mxIsInt32(array))||(mxIsUint32(array)))
    {
	ldata = (long *)mxGetData(array);
        *value = (long) ldata[0];
        return(1);
    }

    if ((mxIsInt16(array))||(mxIsUint16(array)))
    {
	sdata = (short *)mxGetData(array);
        *value = (long) sdata[0];
        return(1);
    }

    if ((mxIsInt8(array))||(mxIsUint8(array)))
    {
	cdata = (char *)mxGetData(array);
        *value = (long) cdata[0];
        return(1);
    }
    errError("tsGetTSlong","Unknown matlab class");
    return (0);
} 

int 	 tsGetTSunit(mxArray *TS,long arrayindex,long *value) 						
{
    mxArray	*array;
    char	*unitstring;
    char	*unitname[] = { "mV","uV","ms","V","mVms" };
    long	numunitname = 5;
    long	p; /* counter */		
    long	*ldata;
    short     	*sdata;
    char	*cdata;
    
    if (!( array = mxGetField(TS,(int)arrayindex,"unit"))) return (0);
    if(mxIsChar(array))
    {
        unitstring = miscConvertString(array);
        *value = 0; /* Unknown unit */
        for (p=0;p<numunitname;p++)
            if (strcmp(unitname[p],unitstring)==0)
                *value = p+1;
	if (*value == 0) return(0);
	return(1);
    }
    else
    {
 	if ((mxIsDouble(array)))
    	{
    		*value = (long) mxGetScalar(array);
    		return(1); 
    	}	

	    if ((mxIsInt32(array))||(mxIsUint32(array)))
    	{
		ldata = (long *)mxGetData(array);
        	*value = (long) ldata[0];
        	return(1);
    	}

    	if ((mxIsInt16(array))||(mxIsUint16(array)))
    	{
		sdata = (short *)mxGetData(array);
        	*value = (long) sdata[0];
        	return(1);
    	}

    	if ((mxIsInt8(array))||(mxIsUint8(array)))
    	{
		cdata = (char *)mxGetData(array);
        	*value = (long) cdata[0];
        	return(1);
    	}
	return(0);
    }
    errError("tsGetTSunit","Unknown unit class");
    return (0);
}

int 	 tsGetTSpotvals(mxArray *TS,long arrayindex,float **potvals,long *leadnum,long *framenum)
{
    mxArray	*array; 
    double	*data;  /* the data from matlab */
    float	*databuff; /* data buffer */
    long	datasize;  /* the length of the data */
    long	p;
    
    if (!( array = mxGetField(TS,(int)arrayindex,"potvals"))) return(0);
    
    *leadnum = mxGetM(array); /* get the dimensions of the matrix */
    *framenum = mxGetN(array);
   
    if ((leadnum == 0)||(framenum == 0))
    {
	errError("tsGetTSpotvals","potvals array is empty");
	return(0);
    }

    if (!( mxIsDouble(array))) /* Test whether the array type is OK */
    {
        errError("tsGetTSpotvals","potvals are not of the double-class");
        return(0); /* Data is of wrong class so I cannot deal with it */
    }
        
    datasize = (long)mxGetM(array)*mxGetN(array);

    if (!( databuff = (float *)mxCalloc(datasize,sizeof(float)))) return(0);
    
    data = mxGetPr(array);        
    for (p=0;p<datasize;p++) databuff[p] = (float) data[p]; /* copy and convert data */
    *potvals = databuff;
    
    return(1);  
}    
 	
int 	 tsGetTSleadinfo(mxArray *TS,long arrayindex,long **leadinfo,long *numleadinfo) 		/* Getting bad leadinfo */
{
    mxArray	 	*array; 
    unsigned int	*datauint32;  /* the data from matlab */
    double              *datadouble;
    unsigned char       *datauint8;
    long		*databuff; 
    long         	p;
    
    if (!( array = mxGetField(TS,(int)arrayindex,"leadinfo"))) return (0);
    
    *numleadinfo = (long) (mxGetM(array)*mxGetN(array)); /* get the dimensions of the matrix */
    
    if (*numleadinfo==0) return(0);	

    if ((!mxIsUint32(array))&&(!mxIsDouble(array))&&(!mxIsInt32(array))&&(!mxIsInt8(array))&&(!mxIsUint8(array))) /* Test whether the array type is OK */
    {
        errError("tsGetTSleadinfo","leadinfo is not of uint32 or double class");
        return (0); /* data is of wrong class */
    }
     
    if (!( databuff = (long *) mxCalloc(*numleadinfo,sizeof(long)))) return(0);
        
    if (mxIsUint32(array)|mxIsInt32(array))
    {
        datauint32 = (unsigned int *)mxGetData(array); 
        for (p=0;p<(*numleadinfo);p++) databuff[p] = (long) datauint32[p]; /* copy and convert data */
    }
 
    if (mxIsUint8(array)|mxIsInt8(array))
    {
        datauint8 = (unsigned char *)mxGetData(array); 
        for (p=0;p<(*numleadinfo);p++) databuff[p] = (long) datauint8[p]; /* copy and convert data */
    }

    if (mxIsDouble(array))
    {
        datadouble = (double *)mxGetData(array); 
        for (p=0;p<(*numleadinfo);p++) databuff[p] = (long) datadouble[p]; /* copy and convert data */
    }

    *leadinfo = databuff;
    
    return(1); /* success */
}

#endif
