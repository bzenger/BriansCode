// This file contains a set of functions to write structured files
// The syntax should be equal to most stdio lib functions 

#include "fileformat.h"

// Open a file for reading or writing
#ifdef EXTERNC
extern "C" { 
#endif

int	dByteSwap()
{
	// Detect byte swapping
	// This function loads a 2 byte value into test
	// and then tests the values of both bytes to 
	// determine the order in which they are written

	short test;
	unsigned char *ptr;
	test = 0x00FF;
	ptr = (unsigned char *) &(test);
	if (ptr[0]) return(1); // Bytes are swapped, we must be running on some INTEL machine
	return(0);
}



void	dSwapBytes(void *buffer,size_t count)
{
	unsigned char 	*ptr;
	int 		m,m8,p;
	unsigned char	temp;

	m = count;
	ptr = (unsigned char *)buffer;

	m8 = m-8;
	
	// unroll loop for swapping bytes
	// to increase efficiency

	for (p=0;p<m8;)
	{
		temp = ptr[p]; ptr[p] = ptr[p+1]; ptr[p+1] = temp; temp = ptr[p+2]; ptr[p+2] = ptr[p+3]; ptr[p+3] = temp; 
		temp = ptr[p+4]; ptr[p+4] = ptr[p+5]; ptr[p+5] = temp; temp = ptr[p+6]; ptr[p+6] = ptr[p+7]; ptr[p+7] = temp; 
		p += 8;	
	}
	for (;p<m;p+=2)	{temp = ptr[p]; ptr[p] = ptr[p+1]; ptr[p+1] = temp;}

	// end of byte swapping
}

void	dSwapBytes4(void *buffer,size_t count)
{
	unsigned char 	*ptr;
	int 		m,m8,p;
	unsigned char	temp;

	m = count;
	ptr = (unsigned char *)buffer;

	m8 = m-8;
	
	// unroll loop for swapping bytes
	// to increase efficiency

	for (p=0;p<m8;)
	{
		temp = ptr[p]; ptr[p] = ptr[p+3]; ptr[p+3] = temp; temp = ptr[p+1]; ptr[p+1] = ptr[p+2]; ptr[p+2] = temp;  
		temp = ptr[p+4]; ptr[p+4] = ptr[p+7]; ptr[p+7] = temp; temp = ptr[p+5]; ptr[p+5] = ptr[p+6]; ptr[p+6] = temp; 
		p += 8;	
	}
	for (;p<m;p+=4)	
	{
		temp = ptr[p]; ptr[p] = ptr[p+3]; ptr[p+3] = temp; temp = ptr[p+1]; ptr[p+1] = ptr[p+2]; ptr[p+2] = temp; 
	}

	// end of byte swapping
}

void	dSwapBytes8(void *buffer,size_t count)
{
	unsigned char 	*ptr;
	int 		m,p;
	unsigned char	temp;

	m = count;
	ptr = (unsigned char *)buffer;

	for (p=0;p<m;p+=8)	
	{
		temp = ptr[p]; ptr[p] = ptr[p+7]; ptr[p+7] = temp; temp = ptr[p+1]; ptr[p+1] = ptr[p+6]; ptr[p+6] = temp;
		temp = ptr[p+2]; ptr[p+2] = ptr[p+5]; ptr[p+5] = temp; temp = ptr[p+3]; ptr[p+3] = ptr[p+4]; ptr[p+4] = temp;
	}

	// end of byte swapping
}

size_t 	dbsreadf(void *buffer,int size,int count,FILE *fptr, int type,int byteswap)
{
	// Lengths of the different objects. 
	// If the lengths in bytes do not correspond with the machines sizeof() operator
	// a warning is issued
	// FUTURE: implementations may correct this by padding extra zeros in between

	size_t lengths[] = {dsNONE,dsCHAR,dsSHORT,dsINT,dsLONGLONG,dsFLOAT,dsDOUBLE};
	size_t rcount;

	// Check whether format is OK

	if (size == lengths[type])
	{
		rcount = fread(buffer,size,count,fptr);
		if (byteswap)
		{
			if (type == dSHORT)    dSwapBytes(buffer,size*count);
			if (type == dINT)      dSwapBytes4(buffer,size*count);
			if (type == dFLOAT)    dSwapBytes4(buffer,size*count);
			if (type == dLONGLONG) dSwapBytes8(buffer,size*count);
			if (type == dDOUBLE)   dSwapBytes8(buffer,size*count);
		}
	}
	else
	{
		printf("Incompatible formats: variable size does not correspond with size of variable in file\n");
	}
	return(rcount);
}

size_t	dbswritef(void *buffer,int size,int count,FILE *fptr, int type,int byteswap)
{
	size_t lengths[] = {dsNONE,dsCHAR,dsSHORT,dsINT,dsLONGLONG,dsFLOAT,dsDOUBLE};
	size_t rcount;

	// Check whether format is OK

	if (size == lengths[type])
	{
		
		if (byteswap)
		{
			if (type == dSHORT)    dSwapBytes(buffer,size*count);
			if (type == dINT)      dSwapBytes4(buffer,size*count);
			if (type == dFLOAT)    dSwapBytes4(buffer,size*count);
			if (type == dLONGLONG) dSwapBytes8(buffer,size*count);
			if (type == dDOUBLE)   dSwapBytes8(buffer,size*count);
		}

		rcount = fwrite(buffer,size,count,fptr);
		
		if (byteswap)
		{
			if (type == dSHORT)    dSwapBytes(buffer,size*count);
			if (type == dINT)      dSwapBytes4(buffer,size*count);
			if (type == dFLOAT)    dSwapBytes4(buffer,size*count);
			if (type == dLONGLONG) dSwapBytes8(buffer,size*count);
			if (type == dDOUBLE)   dSwapBytes8(buffer,size*count);
		}
	}
	else
	{
		printf("Incompatible formats: variable size does not correspond with size of variable in file\n");
	}
	return(rcount);
}



DFILE *dopen(char *name, char *mode)
{
    DFILE   *dptr;
    FILE    *fptr;
    int	    version,count; 


    dptr = (DFILE *)calloc(sizeof(DFILE),1);

#ifdef DEBUGM
printf("dopen: %d\n",sizeof(DFILE));
#endif

    if (!dptr) return(NULL);

    dptr->machinebswap = dByteSwap();
    dptr->byteswap = 0;


    // Determine whether file exists and is byteswapped
    if (fptr = fopen(name,"r"))
    {
        fseek(fptr,8,0);
        count = fread(&version,sizeof(int),1,fptr);
        if (count == 1)
        {
	    if (dptr->machinebswap)
	    {
	      dSwapBytes4((void *)&version,sizeof(int));
	    }
            if (version > 0x00FF)
            {
                dptr->byteswap = 1; // file is byteswapped
            }
        }
        fclose(fptr);
    }
    
    if (dptr->machinebswap) printf("machine is byte swapped\n");

    dptr->byteswap ^= dptr->machinebswap;

    dptr->fptr = fopen(name,mode);
    if (dptr->fptr == NULL)
    {
    	free(dptr);
    	return(NULL);
    }		
 
    dptr->level = 0;
    dptr->cpos[dptr->level] = 0;
       
    return(dptr);
}


// Close the file and free the extra memory we used
// as well. Syntax equal to fclose()

int dclose(DFILE *dptr)
{
    if(dptr)
    {
        if(dptr->fptr) fclose(dptr->fptr);
        free(dptr);
    }
    return(1);    // THIS FUNCTION IS ALWAYS A SUCCESS     
}

int dseek(DFILE *dptr,int offset)
{
    fseek(dptr->fptr,offset,1);
    dptr->cpos[dptr->level] += offset;
    return(1);
}

// Extra function to write a header and go a level deeper
// In the file structure

int dwriteheader(DFILE *dptr,int name,int version)
{
    int size,reserved;
    
    if (dptr->level == MAXLEVEL) return(0);

    size = 0;
    if (version == 0) version = 1;
    version = version & VERSIONMASK;
    reserved = 0;
    
    if (dbswritef((void *)&name,sizeof(int),1,dptr->fptr,dINT,dptr->byteswap) != 1) return(0);
    if (dbswritef((void *)&size,sizeof(int),1,dptr->fptr,dINT,dptr->byteswap) != 1) return(0);
    if (dbswritef((void *)&version,sizeof(int),1,dptr->fptr,dINT,dptr->byteswap) != 1) return(0);
    if (dbswritef((void *)&reserved,sizeof(int),1,dptr->fptr,dINT,dptr->byteswap) != 1) return(0);
    
    dptr->cpos[dptr->level] += 4*sizeof(int);
    dptr->cpos[dptr->level+1] = dptr->cpos[dptr->level];
    dptr->level++;
    
    return(1);
}

// Go a level up again and end the part that is controlled by
// the previous header 

int dwriteend(DFILE *dptr)
{
    int size, len;
    
    if (dptr->level == 0) return(0);
    size = dptr->cpos[dptr->level] - dptr->cpos[dptr->level-1];
    
    // update size in file
    
    fseek(dptr->fptr,-size-3*sizeof(int),1);
    len = dbswritef(&size,sizeof(int),1,dptr->fptr,dINT,dptr->byteswap);
    fseek(dptr->fptr,size+2*sizeof(int),1);
    if (len != 1) return(0);
    
    dptr->level--;
    dptr->cpos[dptr->level] += size;
    return(1);
    
}

int dwrite(void *buffer,int size, int count,DFILE *dptr)
{
    int len;
    
    len = fwrite(buffer,size,count,dptr->fptr);
    dptr->cpos[dptr->level] += len*size;
    return(len);
}

int dwritef(void *buffer,int size, int count,DFILE *dptr,int format)
{
    int len;
    
    len = dbswritef(buffer,size,count,dptr->fptr,format,dptr->byteswap);
    dptr->cpos[dptr->level] += len*size;
    return(len);
}

// Write a string length and string in one chunck
// that has a total length that can be divided by four

int dwritestring(DFILE *dptr,char *str)
{
    int len,size,rem,wlen;
    char buffer[3];
    
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;
    
    len  = strlen(str);
    size = (len+3) & 0xFFFFFFFC;
    rem  = size-len;
    
    wlen = dwritef((void *)&len,sizeof(int),1,dptr,dINT);
    if (wlen != 1) return(0);
    
    wlen = dwritef((void *)str,sizeof(char),len,dptr,dCHAR);
    if (wlen != len) return(0);

    // align to a int-sized address
    if(rem) dwritef((void *)buffer,sizeof(char),rem,dptr,dCHAR);    

    return(1);
}


int dreadheader(DFILE *dptr, int *name, int *size, int *version)
{
    int		reserved, asize;
    
    if (dptr->level == MAXLEVEL) return(0);
    if (dptr->level > 0)
    {
	asize = dptr->cpos[dptr->level]-dptr->cpos[dptr->level-1];
        if (asize >= dptr->size[dptr->level]) return(0);
    }     

    if (dbsreadf((void *)name,sizeof(int),1,dptr->fptr,dINT,dptr->byteswap) != 1) return(0);
    if (dbsreadf((void *)size,sizeof(int),1,dptr->fptr,dINT,dptr->byteswap) != 1) return(0);
    if (dbsreadf((void *)version,sizeof(int),1,dptr->fptr,dINT,dptr->byteswap) != 1) return(0);
    if (dbsreadf((void *)&reserved,sizeof(int),1,dptr->fptr,dINT,dptr->byteswap) != 1) return(0);
    
    dptr->cpos[dptr->level] += 4*sizeof(int);
    dptr->cpos[dptr->level+1] = dptr->cpos[dptr->level];
    dptr->size[dptr->level+1] = *size;
    dptr->level++;   

#ifdef DEBUG
printf("header: level=%d, cpos[level]=%d, size=%d\n",dptr->level,dptr->cpos[dptr->level],*size);
#endif
 
    *version = *version & VERSIONMASK;
    return(1);
}

int dgotoheader(DFILE *dptr)
{
    int asize;

    if (dptr->level == 0) return(0);
    asize = dptr->cpos[dptr->level] - dptr->cpos[dptr->level-1];

#ifdef DEBUG
printf("asize=%d\n",asize);
printf("cpos[level]=%d, cpos[level-1]=%d, level=%d\n",dptr->cpos[dptr->level], dptr->cpos[dptr->level-1],dptr->level);
#endif


    fseek(dptr->fptr,-asize,1);
    dptr->cpos[dptr->level] -= asize;
    return(1);
}

int dmark(DFILE *dptr, int *position)
{
    *position = dptr->cpos[dptr->level];
    return(1);
}

int dgoto(DFILE *dptr,int position)
{
    int asize;
    if (dptr->level == 0) return(0);
    asize = dptr->cpos[dptr->level] - position;

    fseek(dptr->fptr,-asize,1);
    dptr->cpos[dptr->level] -= asize;
    return(1);
}

int dreadend(DFILE *dptr)
{
  
    int size, asize;

#ifdef DEBUG
printf("end (1): level=%d cpos[level]=%d\n",dptr->level-1,dptr->cpos[dptr->level-1]);
#endif
   
    if (dptr->level == 0) return(0);
    asize = dptr->cpos[dptr->level] - dptr->cpos[dptr->level-1];
    size = dptr->size[dptr->level];
    
    // update size in file
    
    fseek(dptr->fptr,size-asize,1);
    
    dptr->level--;
    dptr->cpos[dptr->level] += size;

#ifdef DEBUG
printf("end: level=%d cpos[level]=%d\n",dptr->level,dptr->cpos[dptr->level]);
#endif
 
    return(1);
}


int dread(void *buffer,int size,int count,DFILE *dptr)
{
    int len;
    
    len = fread(buffer,size,count,dptr->fptr);
    dptr->cpos[dptr->level] += len*size;
    return(len);
}
    
int dreadf(void *buffer,int size,int count,DFILE *dptr,int format)
{
    int len;
    
    len = dbsreadf(buffer,size,count,dptr->fptr,format,dptr->byteswap);
    dptr->cpos[dptr->level] += len*size;

#ifdef DEBUG
printf("len=%d\n",len);
#endif

    return(len);
}    
    
int dreadstring(DFILE *dptr,char **str)
{
    int len,size,rem,wlen;
    char buffer[3];
    
    wlen = dreadf((void *)&len,sizeof(int),1,dptr,dINT);
    if (wlen != 1) return(0);
    
    size = (len+3) & 0xFFFFFFFC;
    rem  = size-len;
    
    *str = (char *)calloc(len+1,sizeof(char));
    if (*str == NULL) return(0);
    
    wlen = dreadf((void *)*str,sizeof(char),len,dptr,dCHAR);
    if (wlen != len) { free(*str); return(0); }

    // align to a int-sized address
    if(rem) dreadf((void *)buffer,sizeof(char),rem,dptr,dCHAR);    

    return(1);
}

#ifdef EXTERNC
}
#endif

