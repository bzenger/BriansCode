/* FILENAME:  misctools.c
   AUTHOR:    JG STINSTRA
   CONTENTS:  Collection of different general purpose tools
   LAST UPDATE: 4 JULY 2003 
*/

#ifndef MISCTOOLS_C
#define MISCTOOLS_C 1

#include "misctools.h"

char *miscConvertString(mxArray *stringarray)
{
    char	*string;
    int		stringlen;
    
    stringlen = (int)(mxGetN(stringarray)*mxGetM(stringarray))+1; /* for the zero terminator */
    if (!( string = (char *)mxCalloc(stringlen,sizeof(char)))) return(NULL);
        
    if ( mxGetString(stringarray,string,stringlen))
    {
        errError("miscConvertString","Could not convert string");
	mxFree(string);
	return(NULL);
    }

    return(string);
}

mxArray *miscStructToCell(mxArray *structarray)
{
    mxArray	*cellarray;
    if (!( cellarray = mxCreateCellMatrix(1,1))) return(NULL);   
    mxSetCell(cellarray,0,structarray);
    return(cellarray);
}

#endif
