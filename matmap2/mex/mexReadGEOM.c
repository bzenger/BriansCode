/* FILENAME:  mexReadGEOM.c
   AUTHOR:    JG STINSTRA
   CONTENTS:  This functions reads the contents of a GEOM file into matlab's memory
   LAST UPDATE: 4 JULY 2003 
*/

/* Do the include things */

#include <stdio.h>
#include <math.h>
#include <mex.h>

#include "myerror.c"
#include "misctools.c"
#include "graphicsio.h"

#define VALID_GEOMETRY_FILE	TRUE
#define INVALID_GEOMETRY_FILE    FALSE

/* Declare my functions */

long rgfGetFileID(FileInfoPtr FIP);		   /* To check we are dealing with a genuine geom file */
long rgfGetNumSurfaces(FileInfoPtr FIP); 	   /* How many surfaces are described in the file */
mxArray *GetSurface(FileInfoPtr FIP,long index,char *filename); /* Loads the surface data into a matlab strucutured matrix */

mxArray *ReadCNode(FileInfoPtr fip);
mxArray *ReadCTri(FileInfoPtr fip);

/* Do my MEX Function */

void mexFunction(int nlhs,mxArray **plhs, int nrhs, const mxArray **prhs)
/* Entry point for MATLAB. 
    nlhs - number of parameters on leftside hand of expression
    plhs - a pointer to an array of pointers representing the various arrays on this side
    nrhs - idem right hand side
    prhs - idem array pointer right hand side
*/
{
    char		*filename;
    int			numsurfaces;
    long		p;		/* My loop parameter */
    int                 dim[2];

    FileInfoPtr		fip;
    mxArray		*surface;	/* Cell Array containing struct to surfaces */
    mxArray		*surf;		/* Pointer to individual surface data */

    /********* Input parameter processing ***********************/
    /* Now check parameter by parameter in and output */
    /* First do the filename parameter */

    errUsage("GEOM = mexReadGEOM(filename)"); /* Set usage string */

    if (nrhs < 1) 
    {
	errError("mexFunction","No filename supplied");
	mexErrMsgTxt("ERROR\n");
	return;
    }

    if (!mxIsChar(prhs[0])) 
    {
	errError("mexFunction","No valid filename is supplied");
	mexErrMsgTxt("ERROR\n");
	return;
    }

    /* convert name into zero terminated string */
        
    if(!( filename = miscConvertString((mxArray *)prhs[0]))) 
    {
	errError("mexFunction","Could not convert filename");
	mexErrMsgTxt("ERROR\n");
	return;
    }

    /* Now check the second parameter */
   
    if (nrhs > 2)
    {
	errError("mexFunction","Only one input parameters is required");
        mexErrMsgTxt("ERRO\nR");
	return;
    }

    /****** Output ***************/

    /* Generate a matrix to put the data in */
   
    if (nlhs != 1)
    {
	    errError("mexFunction","One output parameter is required");
            mexErrMsgTxt("ERROR\n");
	    return;
    }
    

    /******* Actual main program ************/

    /* First try to open the file */

    if (openfile_(filename,1,&fip)) 
    {
	errError("mexFunction","Could not open file");
	mxFree(filename);
	mexErrMsgTxt("ERROR\n");
	return;
    }

    /* Check whether we are dealing with the good file type */

    if( rgfGetFileID(fip) == INVALID_GEOMETRY_FILE)
    { 
	closefile_(fip); 
	mxFree(filename);
	errError("mexFunction","File is not a valid geometry file"); 
	mexErrMsgTxt("ERROR\n");
        return;
    }

    /* Go and retrieve the surface information this is done in a loop filling a cell array */

    numsurfaces = rgfGetNumSurfaces(fip); /* This should read the number of entries we have */

    /* First build the Surface Cell Array */

    dim[0] = 1;
    dim[1] = numsurfaces;

    if (!(surface = mxCreateCellArray(2,dim)))
    {  
	closefile_(fip);  
	mxFree(filename);
	errError("mexFunction","Could not obtain enough memory");
	mexErrMsgTxt("ERROR\n");
 	return;
    }
    
    /* Main program loop */

    for(p=1;p<=numsurfaces;p++)
    {
        if(!(surf = GetSurface(fip,p,filename)))
	{
		closefile_(fip);
		mxFree(filename);
		mxDestroyArray(surface);
		errError("mexFunction","Could read all surfaces");
		mexErrMsgTxt("ERROR\n");
	}
        mxSetCell(surface,(p-1),surf); /* Link the individual surfaces into one bundle of data */
    }
   
    /* close file before getting back to matlab */
    closefile_(fip);
    mxFree(filename);

    if (nlhs > 0) plhs[0] = surface; /* Link Surface to output */
    return;
}


/****************************************************************************************************/



/********** functions *****************/


long rgfGetFileID(FileInfoPtr FIP)
/* Get the type of the file to see whether it is a geometry file or a data file */
{
    long	Surfaces;
    long 	BoundedSurfaces;
    long 	TimeSeries;
    long 	FileType;
    CBoolean	booldummy;

    getfileinfo_(FIP,&FileType,&Surfaces,&BoundedSurfaces,&TimeSeries,&booldummy);

    /* Apparently, specifying the file as type 2 should indicate that the file is geometry,
       however tests show it is not the case in all circumstances. Hence the file will be 
       accepted when eihter number of Surfaces or number of bounded Surfaces is larger than
       zero. Hence a file can contain timeseries as well as surfaces. This function only 
       loads the latter into memory */
    
    if((FileType == 2L)||(Surfaces > 0L)||(BoundedSurfaces > 0L))
        return(VALID_GEOMETRY_FILE);

    return(INVALID_GEOMETRY_FILE);
}

long rgfGetNumSurfaces(FileInfoPtr FIP) 	   
/* How many surfaces are described in the file */
{
    long	Surfaces;
    long 	BoundedSurfaces;
    long 	dummy[2];
    CBoolean	booldummy;

    /** Have to adjust this function in future to cope with Bounded surfaces as well */
    getfileinfo_(FIP,&dummy[0],&Surfaces,&BoundedSurfaces,&dummy[1],&booldummy);
    return(Surfaces);
}


mxArray *GetSurface(FileInfoPtr fip,long index,char *filename)
/* Loads the surface data into a matlab strucutured matrix
   It is case index is a number starting with 1 and not the in C that common 0.
   Although not described in the code of the graphicsio.lib, the code clearly
   assumes index to be greater than 0.
 */
{
    mxArray		*namestr,*filenamearray;
    char		name[80];

    mxArray		*surface;	/* Array to return */

    const int		dim = 1;
    const int		numfields = 6;
    const char		*fieldnames[] = {
                            "filename",
                            "name",
                            "pts",
                            "cpts",
                            "fac",
                            "cfac" };
    enum 		fields {FILENAME=0, NAME, PTS, CPTS, TRI, CTRI};	

    long		numnodes;
    long		numelements;
    long		elementsize;
    long		numscalar;
    long		numvector;
    long		numtensor;

    int			p;		/* loop parameter */

    mxArray		*node;
    mxArray		*cnode, *ctri;
    float		*nodesrc;
    double		*nodedst;

    mxArray		*element;
    long		*elementsrc;
    double		*elementdst;

    char		*string;

    /* Go to the surface of interest */
    setsurfaceindex_(fip,index);

    /* First setup an struct array to put all the data in */
    /* This array contains fields like the Name, Node for a matrix containing all node points as well as
       a Tri matrix for defining the elements, A CNode and a CTri define the conductivities assigned to
       both the elements as well as the nodes */

    if (!( surface = mxCreateStructArray(1,&dim,numfields,fieldnames)))
    { 
	errError("GetSurface","Could not allocate enough memory"); 
        return(NULL); 
    }

    if(!(string = (char *)mxCalloc(strlen(filename)+20,sizeof(char)))) 
    {	
	errError("GetSurface","Could not allocate enogh memory (string)");
	return(NULL);
    }
    else    
    {	/* create a filename with @<number> */    
        
    	sprintf(string,"%s@%d\0",filename,(int)index);    
    	if (!( filenamearray = mxCreateString(string)))
    	{ 
		errError("GetSurface","Could not allocate enough memory (filenamearray)");
		mxFree(string);
		mxDestroyArray(surface);
		return(NULL); 
	}      
	mxSetFieldByNumber(surface,0,FILENAME,filenamearray);
	mxFree(string);	
    }


    /* First deal with the name of the surface */
    getsurfacename_(fip,name);

    if(!( namestr = mxCreateString(name)))
    { 
	errError("GetSurface","Could not allocate enough memory (name)");
	mxDestroyArray(surface);
	return(NULL);
    }
    mxSetFieldByNumber(surface,0,NAME,namestr);  /* Set SurfaceName into structure */

    /* Next Target are the Node fields */
    /* We need to find out how many there are, allocate enough space to store them and link the matrix */

    if(getnodeinfo_(fip,&numnodes,&numscalar,&numvector,&numtensor))
    {
        errError("GetSurface","Could not get Node info");
	mxDestroyArray(surface);
	return(NULL);
    }

    if (numnodes > 0) /* try to avoid loading empty data sets, as this will allocate no memory and stall the program */
    {
        /* Get memory stuff done */
        node = mxCreateDoubleMatrix(3,(int)numnodes,mxREAL);  /* create matrix structure */
        nodesrc = (float *)mxCalloc((int)numnodes*3,sizeof(float));    /* create buffer for graphicsio to write in */	
        if ((node == NULL)||(nodesrc==NULL))
        {   
		if (node) mxDestroyArray(node);
		if (nodesrc) mxFree(nodesrc);
		errError("GetSurface","Could not allocate enough memory");
		mxDestroyArray(surface);
		return(NULL);
 	}

        getnodes_(fip,(NodePtr)nodesrc);		/* Data from graphicsio in buffer */			
        nodedst = mxGetPr(node);		/* Get pointer where matlab stores its data */

        for(p=0;p<(numnodes*3);p++) 
            nodedst[p] = (double)nodesrc[p]; /* Copy array and do the cast operator stuff */

        /* finally I got the data now link the matrix with the Surface matrix */
        mxSetFieldByNumber(surface,0,PTS,node);
	mxFree(nodesrc);

        if(!(cnode = ReadCNode(fip)))
	{
		errError("GetSurface","Could not obtain conductivity node matrix");
		mxDestroyArray(surface);
		return(NULL);
	}
        mxSetFieldByNumber(surface,0,CPTS,cnode);
    }

    /* Do the triangulation matrices as well */
    /* We need to find out how many there are, allocate enough space to store them and link the matrix */

    if(getelementinfo_(fip,&numelements,&elementsize,&numscalar,&numvector,&numtensor))
    {
        errError("GetSurface","Could not get Element info");
	mxDestroyArray(surface);
	return(NULL);
    }

    if ((numelements > 0)&&(elementsize > 0)) /* try to avoid loading empty data sets program */
    {
        /* Get memory stuff done */
        element = mxCreateDoubleMatrix(elementsize,(int)numelements,mxREAL);  /* create matrix structure */
        elementsrc = (long *)mxCalloc((int)(numelements*elementsize),sizeof(long));    /* create buffer for graphicsio to write in */	

        if ((element == NULL)||(elementsrc==NULL))
        { 
		errError("GetSurface","Could not allocate enough memory");
		if (element) mxDestroyArray(element);
		if (elementsrc) mxFree(elementsrc);
		mxDestroyArray(surface);
		return(NULL);
	}

        getelements_(fip,elementsrc);		/* Data from graphicsio in buffer */			
        elementdst = mxGetPr(element);		/* Get pointer where matlab stores its data */

        for(p=0;p<(numelements*elementsize);p++) 
            elementdst[p] = (double)elementsrc[p]; /* Copy array and do the cast operator stuff */

        /* finally I got the data now link the matrix with the Surface matrix */
        mxSetFieldByNumber(surface,0,TRI,element);

        if(!(ctri = ReadCTri(fip)))
	{
		errError("GetSurface","Could not obtain conductivity element matrix");
		mxDestroyArray(surface);
		return(NULL);
	}
        mxSetFieldByNumber(surface,0,CTRI,ctri);
    }

    return(surface);
}


mxArray *ReadCNode(FileInfoPtr fip)
/*  This function reads the CNode data, which contains the conductivity data ordered by Node
*/
{
    const char*			fieldnames[] = {
                                "cdata",
                                "type",
                                "dim"  };
    enum			fields {CDATA = 0, TYPE, DIM};
    const int			numfields = 3;

    mxArray			*cnode;	/* to store final structure */
    mxArray			*scalartype, *scalardim, *vectortype, *vectordim, *tensortype, *tensordim;
    mxArray			*scalararray, *vectorarray, *tensorarray; /* To store the fields linked to the cells of scalar/vector/tensor etc. */

    long			numscalar, numvector, numtensor, numnodes; /* The number of layers contained in the file of each type */
    long			p, q; /* My loop parameter */
    
    int				tensordimensions[3];
    long			dimension;

    long			type;
    long			bufsize;
    long			maxdim;		/* for tensors handling */
    long			*types,*dimensions; /* for handling tensors */

    float			*buffer; /* buffer used to put the read values in, this one is needed because the matlab data needs to be in doubles not floats */
    double			*scalardata, *vectordata, *tensordata;


    /* The first thing to do is to obtain the length and size of data,
       Hence read how much layers each type has */
 
    if(getnodeinfo_(fip,&numnodes,&numscalar,&numvector,&numtensor))
    {  
	errError("ReadCNode","getnodeinfo failed");
	return(NULL);
    }

    
    /* Now create the bases of the subfields in the structure and check in one time whether one could not be allocated */

    cnode = mxCreateStructMatrix(1,(numscalar+numvector+numtensor),numfields,fieldnames);
    if(cnode == NULL) 
    { 
	errError("ReadCNode","Could not allocate enough memory");
	return(NULL);
    }

    /* Decide on how big the buffer should be, as it can be used over and over again it should accomadate the largest one */

    bufsize = 1;
    if (numvector > 0)
        bufsize = 3;

    if (numtensor > 0)
    {
        types = (long *)mxCalloc(numtensor,sizeof(long));
        dimensions = (long *)mxCalloc(numtensor,sizeof(long));
 
        if((types==NULL)||(dimensions==NULL))       
        { 
		errError("ReadCNode","Could not obtain enough memory");
		if (types) mxFree(types);
		if (dimensions) mxFree(dimensions);
		mxDestroyArray(cnode);
		return(NULL);
	}
        
        if(getnodetensortypes_(fip,types,dimensions)) /* obtain dimensions to determine maximum buffer size */
        { 
		errError("ReadCNode","getnodetensortypes failed");
		mxFree(types);
		mxFree(dimensions);
		mxDestroyArray(cnode);
		return(NULL);
	}

        maxdim = 0;
        for (p=0;p<numtensor;p++) { if (dimensions[p] > maxdim) maxdim = dimensions[p]; }
        if (bufsize < maxdim*maxdim) bufsize = maxdim*maxdim; /* make sure enough space is avaible in the buffer */

	mxFree(types);
	mxFree(dimensions);
    }

    /* no do loops and read the data */
    buffer = (float *)mxCalloc(numnodes*bufsize,sizeof(float));
    if(buffer==NULL) 
    { 
	errError("ReadCNode","Could not obtain enough memory");
	mxDestroyArray(cnode);
	return(NULL);
    }


    for (p=0;p<numscalar;p++)
    {
        /* Set the CData field */
        getnodescalars_(fip,p+1,&type,buffer);	/* Get the data from the file */
        if (!( scalararray = mxCreateDoubleMatrix(1,numnodes,mxREAL)))	/* To store the actual data in, this matrix will be linked in the cells of Scalar */
	{ 
		errError("ReadCNode","Could not obtain enough memory (cdata)");
		mxDestroyArray(cnode);
		return(NULL);	
 	}
        
	scalardata = mxGetPr(scalararray);
        for (q=0;q<numnodes;q++)	/* loop to translate all floats into doubles */
        { 
		scalardata[q] = (double) buffer[q];  /* do all the casting */
	}
        mxSetFieldByNumber(cnode,p,CDATA,scalararray);    
 
        /* Set the Type field */               
        if (!( scalartype = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCNode","Could not obtain enough memory (type)");
		mxDestroyArray(cnode);
		return(NULL); 
	}     
        scalardata = mxGetPr(scalartype);
        scalardata[0] = (double) type;
        mxSetFieldByNumber(cnode,p,TYPE,scalartype);
        
        /* Set the Dimension field */               
        if (!( scalardim = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCNode","Could not obtain enough memory (dim)");
		mxDestroyArray(cnode);
		return(NULL); 
	}     
        scalardata = mxGetPr(scalardim);
        scalardata[0] = 1.0;
        mxSetFieldByNumber(cnode,p,DIM,scalardim); 
    }

    for (p=0;p<numvector;p++)
    {
        getnodevectors_(fip,p+1,&type,(VectorPtr)buffer);	/* Get the data from the file */
        
        /* Set the CData field */
        if (!( vectorarray = mxCreateDoubleMatrix(3,numnodes,mxREAL)))	/* To store the actual data in, this matrix will be linked in the cells of Vector */
        { 
		errError("ReadCNode","Could not obtain enough memory (cdata)");
		mxDestroyArray(cnode);	
		return(NULL);
	}

        vectordata =  mxGetPr(vectorarray);
        for (q=0;q<numnodes*3;q++)	/* loop to translate all floats into doubles */
            { vectordata[q] = (double) buffer[q];} /* do all the casting */
        mxSetFieldByNumber(cnode,numscalar+p,CDATA,vectorarray);
        
        /* Set the Type field */               
        if (!( vectortype = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        {
		errError("ReadCNode","Could not obtain enough memory (type)");
		mxDestroyArray(cnode);
		return(NULL);
	}     
        vectordata = mxGetPr(vectortype);
        vectordata[0] = type;
        mxSetFieldByNumber(cnode,numscalar+p,TYPE,vectortype);
        
        /* Set the Dimension field */               
        if (!( vectordim = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCNode","Could not obtain enough memory (dim)");
		mxDestroyArray(cnode);
		return(NULL);	
	}     
        vectordata = mxGetPr(vectordim);
        vectordata[0] = 2.0;
        mxSetFieldByNumber(cnode,numscalar+p,DIM,vectordim); 
    }

    for (p=0;p<numtensor;p++)
    {
        getnodetensors_(fip,p+1,&dimension,&type,(TensorPtr)buffer);	/* Get the data from the file */
        
        tensordimensions[0] = (int) dimension;  /* setup a multidimension matrix */
        tensordimensions[1] = (int) dimension;
        tensordimensions[2] = (int) numnodes;
        if (!( tensorarray = mxCreateNumericArray(3,tensordimensions,mxDOUBLE_CLASS,mxREAL)))	/* Generate a multidimensional array */
        { 
		errError("ReadCNode","Could not obtain enough memory (cdata)");
		mxDestroyArray(cnode);
		return(NULL);
	}
        tensordata = mxGetPr(tensorarray);
        for (q=0;q<numnodes*dimension*dimension;q++)	/* loop to translate all floats into doubles */
            { tensordata[q] = (double) buffer[q]; } /* do all the casting */
        mxSetFieldByNumber(cnode,numscalar+numvector+p,CDATA,tensorarray);

        /* Set the Type field */               
        if (!( tensortype = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCNode","Could not obtain enough memory (type)");
		mxDestroyArray(cnode);
		return(NULL);
	}      
        tensordata = mxGetPr(tensortype);
        tensordata[0] = type;
        mxSetFieldByNumber(cnode,numscalar+numvector+p,TYPE,tensortype);
        
        /* Set the Dimension field */               
        if (!( tensordim = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCNode","Could not obtain enough memory (dim)");
		mxDestroyArray(cnode);
		return(NULL);
	}        
	tensordata = mxGetPr(tensordim);
        tensordata[0] = 3.0;
        mxSetFieldByNumber(cnode,numscalar+numvector+p,DIM,tensordim); 
    } 
    return(cnode);
}

mxArray *ReadCTri(FileInfoPtr fip)
/* This functions reads the conducitivities ordered by element
*/

{
    const char*			fieldnames[] = {
                                "cdata",
                                "type",
                                "dim"  };
    enum			fields {CDATA = 0, TYPE, DIM};
    const int			numfields = 3;

    mxArray			*ctri;	/* to store final structure */
    mxArray			*scalartype, *scalardim, *vectortype, *vectordim, *tensortype, *tensordim;
    mxArray			*scalararray, *vectorarray, *tensorarray; /* To store the fields linked to the cells of scalar/vector/tensor etc. */

    long			numscalar, numvector, numtensor, numtri; /* The number of layers contained in the file of each type */
    long			p, q; /* My loop parameter */
    
    int				tensordimensions[3];
    long			dimension;

    long			dummy;
    long			type;
    long			bufsize;
    long			maxdim;		/* for tensors handling */
    long			*types,*dimensions; /* for handling tensors */

    float			*buffer; /* buffer used to put the read values in, this one is needed because the matlab data needs to be in doubles not floats */
    double			*scalardata, *vectordata, *tensordata;


    /* The first thing to do is to obtain the length and size of data,
       Hence read how much layers each type has */
 
    if(getelementinfo_(fip,&dummy,&numtri,&numscalar,&numvector,&numtensor))
    { 
	errError("ReadCTri","getnodeinfo failed");
	return(NULL);
    }

    
    /* Now create the bases of the subfields in the structure and check in one time whether one could not be allocated */

    ctri = mxCreateStructMatrix(1,(numscalar+numvector+numtensor),numfields,fieldnames);
    if(ctri == NULL) 
    {
	errError("ReadCTri","Could not allocate enough memory (ctri)");
	return(NULL);
    }

    /* Decide on how big the buffer should be, as it van be used over and over again it should accomadate the largest one */

    bufsize = 1;
    if (numvector > 0) bufsize = 3;

    if (numtensor > 0)
    {
        types = (long *)mxCalloc(numtensor,sizeof(long));
        dimensions = (long *)mxCalloc(numtensor,sizeof(long));
 
        if((types==NULL)||(dimensions==NULL))       
        { 
		errError("ReadCTri","Could not obtain enough memory");
		if (types) mxFree(types);
		if (dimensions) mxFree(dimensions);
 		mxDestroyArray(ctri);
		return(NULL);
	}
        
        if(getnodetensortypes_(fip,types,dimensions)) /* obtain dimensions to determine maximum buffer size */
        { 
		errError("ReadCTri","getnodetensortypes failed");
		if (types) mxFree(types);
		if (dimensions) mxFree(dimensions);
		mxDestroyArray(ctri);
		return(NULL);
	}

        maxdim = 0;
        for (p=0;p<numtensor;p++) { if (dimensions[p] > maxdim) maxdim = dimensions[p]; }
        if (bufsize < maxdim*maxdim) bufsize = maxdim*maxdim; /* make sure enough space is avaible in the buffer */

	mxFree(types);
	mxFree(dimensions);
    }

    /* no do loops and read the data */
    buffer = (float *)mxCalloc(numtri*bufsize,sizeof(float));
    if(buffer==NULL) 
    { 
	errError("ReadCTri","Could not obtain enough memory");
	mxDestroyArray(ctri);
	return(NULL);
    }


    for (p=0;p<numscalar;p++)
    {
        /* Set the CData field */

        getelementscalars_(fip,p+1,&type,buffer);	/* Get the data from the file */
        if (!( scalararray = mxCreateDoubleMatrix(1,numtri,mxREAL)))	/* To store the actual data in, this matrix will be linked in the cells of Scalar */
        { 
		errError("ReadCTri","Could not obtain enough memory (cdata)");
		mxDestroyArray(ctri);
		return(NULL);
	}
        scalardata = mxGetPr(scalararray);
        for (q=0;q<numtri;q++)	/* loop to translate all floats into doubles */
            { scalardata[q] = (double) buffer[q]; } /* do all the casting */
        mxSetFieldByNumber(ctri,p,CDATA,scalararray);    
 
        /* Set the Type field */               
        if (!( scalartype = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCTri","Could not obtain enough memory (type)");
		mxDestroyArray(ctri);
		return(NULL);
	}        scalardata = mxGetPr(scalartype);
        scalardata[0] = (double) type;
        mxSetFieldByNumber(ctri,p,TYPE,scalartype);
        
        /* Set the Dimension field */               
        if (!( scalardim = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCTri","Could not obtain enough memory (dim)");
		mxDestroyArray(ctri);
		return(NULL);
	}    
        scalardata = mxGetPr(scalardim);
        scalardata[0] = 1.0;
        mxSetFieldByNumber(ctri,p,DIM,scalardim); 
    }

    for (p=0;p<numvector;p++)
    {
        getelementvectors_(fip,p+1,&type,(VectorPtr)buffer);	/* Get the data from the file */
        
        /* Set the CData field */
        if (!( vectorarray = mxCreateDoubleMatrix(3,numtri,mxREAL)))	/* To store the actual data in, this matrix will be linked in the cells of Vector */
        { 
		errError("ReadCTri","Could not obtain enough memory (cdata)");
		mxDestroyArray(ctri);
		return(NULL);
	}
        vectordata =  mxGetPr(vectorarray);
        for (q=0;q<numtri*3;q++)	/* loop to translate all floats into doubles */
            { vectordata[q] = (double) buffer[q];} /* do all the casting */
        mxSetFieldByNumber(ctri,numscalar+p,CDATA,vectorarray);
        
        /* Set the Type field */               
        if (!( vectortype = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCTri","Could not obtain enough memory (type)");
		mxDestroyArray(ctri);
		return(NULL);
	}   
        vectordata = mxGetPr(vectortype);
        vectordata[0] = type;
        mxSetFieldByNumber(ctri,numscalar+p,TYPE,vectortype);
        
        /* Set the Dimension field */               
        if (!( vectordim = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCTri","Could not obtain enough memory (dim)");
		mxDestroyArray(ctri);
		return(NULL);
	}
        vectordata = mxGetPr(vectordim);
        vectordata[0] = 2.0;
        mxSetFieldByNumber(ctri,numscalar+p,DIM,vectordim); 
    }

    for (p=0;p<numtensor;p++)
    {
        getelementtensors_(fip,p+1,&dimension,&type,(TensorPtr)buffer);	/* Get the data from the file */
        
        tensordimensions[0] = (int) dimension;  /* setup a multidimension matrix */
        tensordimensions[1] = (int) dimension;
        tensordimensions[2] = (int) numtri;
        if (!( tensorarray = mxCreateNumericArray(3,tensordimensions,mxDOUBLE_CLASS,mxREAL)))	/* Generate a multidimensional array */
        { 
		errError("ReadCTri","Could not obtain enough memory (cdata)");
		mxDestroyArray(ctri);
		return(NULL);
	}
        tensordata = mxGetPr(tensorarray);
        for (q=0;q<numtri*dimension*dimension;q++)	/* loop to translate all floats into doubles */
            { tensordata[q] = (double) buffer[q]; } /* do all the casting */
        mxSetFieldByNumber(ctri,numscalar+numvector+p,CDATA,tensorarray);

        /* Set the Type field */               
        if (!( tensortype = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCTri","Could not obtain enough memory (type)");
		mxDestroyArray(ctri);
		return(NULL);
	}    
        tensordata = mxGetPr(tensortype);
        tensordata[0] = type;
        mxSetFieldByNumber(ctri,numscalar+numvector+p,TYPE,tensortype);
        
        /* Set the Dimension field */               
        if (!( tensordim = mxCreateDoubleMatrix(1,1,mxREAL))) /* To store the type of the data set */
        { 
		errError("ReadCTri","Could not obtain enough memory (dim)");
		mxDestroyArray(ctri);
		return(NULL);
	}   
        tensordata = mxGetPr(tensordim);
        tensordata[0] = 3.0;
        mxSetFieldByNumber(ctri,numscalar+numvector+p,DIM,tensordim); 
    } 
    return(ctri);
}


/* End of File */
