/*
 * mexWriteGEOM.c
 * 
 * Created by dr. J. Stinstra 10 Sept 2002
 * 
 */
 
#include <stdio.h>
#include <math.h>
#include <mex.h>

#include "graphicsio.h"
#include "myerror.c"
#include "misctools.c"

int WriteFileInfo(mxArray *surfmatrix,FileInfoPtr fip);
int WriteSurface(mxArray *surfmatrix,long surfnum,FileInfoPtr fip);
int WriteCPts(mxArray *cmatrix,int q,FileInfoPtr fip);
int WriteCFac(mxArray *cmatrix,int q,FileInfoPtr fip);

void mexFunction(int nlhs,mxArray **plhs, int nrhs, const mxArray **prhs)
/* Entry point for MATLAB. 
    nlhs - number of parameters on leftside hand of expression
    plhs - a pointer to an array of pointers representing the various arrays on this side
    nrhs - idem right hand side
    prhs - idem array pointer right hand side
*/
{
    char 		*geomfile;
    int			p;
    int			numsurf;
    mxArray 		*surfmatrix;		
    int			success,loopsuccess;
    FileInfoPtr		fip;

    
    /* SET UP THE SUCCESS PARAMETER AND THE USAGE */
    
    errUsage("mexWriteGEOM(GEOM-filename,GEOM)");  
    success = 0;
    
    /* INITIALISE SOME PARAMETERS */
    
    geomfile 	= NULL;
    numsurf 	= 0;
    fip		= NULL;
    
    while(1)    
    {
        if (nrhs != 2) { errError("mexFunction","Two input arguments are required"); break; }
        if (!mxIsChar(prhs[0])) { errError("mexFunction","GEOM-filename needs to be a string"); break; }

        if (!((mxIsStruct(prhs[1]))||(mxIsCell(prhs[1]))))  { errError("mexFunction","GEOM needs to be a cellarray or a struct"); break; }
        if (!( geomfile = miscConvertString((mxArray *)prhs[0]))) { errError("mexFunction","Could not convert the geom-string"); break; }
    
        /* Create a new file */
    
        if (createfile_(geomfile,2L,1,&fip) < 0) { errError("mexFunction","Could not open file"); break; }
        
        if (mxIsCell(prhs[1]))
        {
            numsurf = mxGetN(prhs[1])*mxGetM(prhs[1]);
        
            loopsuccess = 0;
            for (p=0;p<numsurf;p++)
            {
            
                surfmatrix = mxGetCell(prhs[1],p);
                if (!(mxIsStruct(surfmatrix))) { errError("mexFunction","The cells in the surface matrix need to be structured"); break; }

                if (p==0) 
                   if(!( WriteFileInfo(surfmatrix,fip))) { errError("mexFunction","Could not write file info"); break; }
            
                   if (!( WriteSurface(surfmatrix,p,fip))) { errError("mexFunction","Error writing surface to file"); break; }
                
                if (p==numsurf-1) loopsuccess = 1;     
            }
            
            if(!loopsuccess) break;
        }
        else
        {    
            if(!( WriteFileInfo((mxArray *)prhs[1],fip))) { errError("mexFunction","Could not write file info"); break; }
            if(!( WriteSurface((mxArray *)prhs[1],0,fip))) { errError("mexFunction","Error writing surface to file"); break; }
        }
   
        success = 1;
        break;
    }
    
    if (fip) closefile_(fip);
    if (geomfile) mxFree(geomfile);
    
    if (!success) 
    {
        mexErrMsgTxt("ERROR: mexWriteGEOM was not able to write the geom file\n");
    }
    
    return;
}    

int WriteFileInfo(mxArray *surfmatrix,FileInfoPtr fip)
{
    mxArray 	*datamatrix;
    char	*expid;
    char 	*text;
    char	*audit;
    int		success;

    success 	= 0;
    expid	= NULL;
    text	= NULL;
    audit	= NULL; 
    
    while(1)
    {
        datamatrix = mxGetField(surfmatrix,1,"expid");
        if (datamatrix)
        {
            if (!(expid = miscConvertString(datamatrix))) { errError("WriteFileInfo","Could not convert string"); break; }
            if (setexpid_(fip,expid) < 0)	{ errError("WriteFileInfo","Could not set expid"); break; }
        }

        datamatrix = mxGetField(surfmatrix,1,"text");
        if (datamatrix)
        {
            if (!(text = miscConvertString(datamatrix))) { errError("WriteFileInfo","Could not convert string"); break; }
            if (settext_(fip,text) < 0)    { errError("WriteFileInfo","Could not set text"); break; }
        }

        datamatrix = mxGetField(surfmatrix,1,"audit");
        if (datamatrix)
        {
            if(!( audit = miscConvertString(datamatrix)))  { errError("WriteFileInfo","Could not convert string"); break; }
            if(setauditstring_(fip,audit) < 0)    {errError("WriteFileInfo","Could not set audit"); break; }
        }
        success = 1;
        break;
    }
    
    if (audit) 	mxFree(audit);
    if (text) 	mxFree(text);
    if (expid)	mxFree(expid);
    
    return(success);
}


int WriteSurface(mxArray *surfacematrix,long surfnum,FileInfoPtr fip)
{

    mxArray 	*datamatrix, *cmatrix;
    char	*surfacename;
    long	*databuffer; 
    float	*databufferfloat;
    double	*dataptr;
    long	datalen,p; 
    long	numel,elsize;
    int		success;

    success = 0;
    surfacename 	= NULL;
    databufferfloat	= NULL;
    databuffer		= NULL;
    
    while(1)
    {
        if (surfacematrix == NULL) return(1);
        
        setsurfaceindex_(fip,(long)surfnum+1);
  
      
        datamatrix  = mxGetField(surfacematrix,0,"name");
        if (datamatrix)
        {
            if (!(surfacename = miscConvertString(datamatrix))) { errError("WriteSurface","Could not convert string"); break; }
            if (setsurfacename_(fip,surfacename) < 0) { errError("WriteSurface","Could not set surface name"); break; }
        }
  
        datamatrix = mxGetField(surfacematrix,0,"pts");
        if (datamatrix)
        {	
            if (!mxIsEmpty(datamatrix))
            {
                elsize = mxGetM(datamatrix);
                numel = mxGetN(datamatrix);
        
                if (elsize != 3) 
                    { errError("WriteSurface","The pts matrix should be a 3xn matrix"); break; }
            }
        
            datalen = elsize*numel;
            if(!( databufferfloat = (float *)mxCalloc(datalen,sizeof(float)))) { errError("WriteSurface","Could not allocate enough memory"); break; }
            dataptr = (double *)mxGetPr(datamatrix);
            for(p=0;p<datalen;p++) databufferfloat[p] = (float)dataptr[p];
        
            if(setnodes_(fip,numel,(NodePtr)databufferfloat) < 0) { errError("WriteSurface","Could set the nodes"); break; }
            
        }     
    
        datamatrix = mxGetField(surfacematrix,0,"fac");
        if (datamatrix)
        {
            if (!mxIsEmpty(datamatrix))
            {
                elsize = mxGetM(datamatrix);
                numel = mxGetN(datamatrix);
        
                datalen = elsize*numel;
                if(!(databuffer = (long *)mxCalloc(datalen,sizeof(long)))) { errError("WriteSurface","Could not allocate enough memory"); break; }
                dataptr = (double *)mxGetPr(datamatrix);
                for(p=0;p<datalen;p++) databuffer[p] = (long)dataptr[p];
        
                if(setelements_(fip,elsize,numel,databuffer) < 0) { errError("WriteSurface","Could not set the elements matrix"); break; }
            }    
        }
    
        datamatrix = mxGetField(surfacematrix,0,"cpts");
        if (datamatrix)
        {
            if (!mxIsEmpty(datamatrix))
            {

                if (!mxIsStruct(datamatrix)) 
                    { errError("WriteSurface","cpts should be a structured array"); break; }
            
                numel = mxGetM(datamatrix)*mxGetN(datamatrix);
                for (p=0;p<numel;p++)
                {
                   if(!( WriteCPts(datamatrix,p,fip))) { errError("WriteSurface","could not write cpts"); break; }
                }
            }
        }
    
        datamatrix = mxGetField(surfacematrix,0,"cfac");
        if (datamatrix)
        {
            if (!mxIsEmpty(datamatrix))
            {
                if (!mxIsCell(datamatrix))
                { errError("WriteSurface","cfac should be a structured array"); break; }
                        
                numel = mxGetM(datamatrix)*mxGetN(datamatrix);
                for (p=0;p<numel;p++)
                {
                   if(!(WriteCFac(cmatrix,p,fip))) { errError("WriteSurface","could not write cfac"); break; }
                }
            }
        }
     
        success = 1;
        break;
    }
    
    if (surfacename) mxFree(surfacename);
    if (databufferfloat) mxFree(databufferfloat);
    if (databuffer) mxFree(databuffer);
    
    return(success);
}    


int WriteCPts(mxArray *cmatrix,int q, FileInfoPtr fip)
{
    mxArray	*datamatrix,*typematrix,*dimmatrix;
    int 	success;
    int		dim,type;
    int		numdims,*dims;
    int		numn,numvec;
    float	*floatbuffer;
    double	*doublebuffer;
    int		p;
    
    success  = 1;
    
    while(1)
    {
        datamatrix = mxGetField(cmatrix,q,"cdata");
        if(!datamatrix) { errError("WriteCPts","Conductivity data needs to have a field cdata"); break; }
        typematrix = mxGetField(cmatrix,q,"type");
        if(!typematrix) { errError("WriteCPts","Conductivity data needs to have a field type"); break; }
        dimmatrix = mxGetField(cmatrix,q,"dim");
        if(!dimmatrix) { errError("WriteCPts","Conductivity data needs to have a field dim"); break; }
        
        if (!(mxIsNumeric(dimmatrix))) { errError("WriteCPts","Dim matrix needs to be numeric"); break; } 
        if (!(mxIsNumeric(typematrix))) { errError("WriteCPts","Type matrix needs to be numeric"); break; } 
        
        dim = (int) mxGetScalar(dimmatrix);
        if ((dim < 0)||(dim > 3)) { errError("WriteCPts","dim field needs to be 1,2 or 3"); break;}
        type = (int) mxGetScalar(typematrix);
        
        numdims = mxGetNumberOfDimensions(datamatrix);
        dims = (int *) mxGetDimensions(datamatrix);
        
        if (dim == 1)
        {
            if (numdims > 2)  { errError("WriteCPts","scalar can not be three dimensional"); break; } 
            if ((dims[0] != 1)&&(dims[1] != 1)) { errError("WriteCpts","scalar cannot be a 2D matrix"); break;}
            numn = dims[0]; if (numn == 1) numn = dims[1];
            if (!( floatbuffer = (float *) mxCalloc(numn,sizeof(float)))) { errError("WriteCpts","Cannot allocate enough memory"); break;}
            doublebuffer = mxGetPr(datamatrix);
            for(p=0;p<numn;p++) floatbuffer[p] = (float) doublebuffer[p];
            if( setnodescalars_(fip, (long)type, (long)numn, (ScalarPtr) floatbuffer) < 0) { errError("WriteCpts","Cannot set scalar conductivities"); break;}
        }
        if (dim == 2)
        {
            if (numdims > 2)  { errError("WriteCPts","vector can not be three dimensional"); break; } 
            numn = dims[1]; numvec = dims[0];
            if (numvec != 3) { errError("WriteCpts","GraphicsIO can only store vectors of size 3"); break; }
            if (!( floatbuffer = (float *) mxCalloc(numn*numvec,sizeof(float)))) { errError("WriteCpts","Cannot allocate enough memory"); break;}
            doublebuffer = mxGetPr(datamatrix);
            for(p=0;p<numn*numvec;p++) floatbuffer[p] = (float) doublebuffer[p];
            if( setnodevectors_(fip, (long)type,(long) numn, (VectorPtr) floatbuffer) < 0) { errError("WriteCpts","Cannot set vector conductivities"); break;}
        }

        if (dim == 3)
        {
            if (numdims != 3)  { errError("WriteCPts","tensors need to be three dimensional"); break; } 
            if (dims[0] != dims[1]) { errError("WriteCpts","tensor conductivities are stored in a mxmxn"); break;}
            numn = dims[2]; numvec = dims[0];
            if (!( floatbuffer = (float *) mxCalloc(numn*numvec*numvec,sizeof(float)))) { errError("WriteCpts","Cannot allocate enough memory"); break;}
            doublebuffer = mxGetPr(datamatrix);
            for(p=0;p<numn*numvec*numvec;p++) floatbuffer[p] = (float) doublebuffer[p];
            if( setnodetensors_(fip, (long)type,(long) numn, (long) numvec, (TensorPtr) floatbuffer) < 0) { errError("WriteCpts","Cannot set tensor conductivities"); break;}
        }

        success = 1;
        break;
    }
    
    if (floatbuffer) mxFree(floatbuffer);
    
    return(success);
}

int WriteCFac(mxArray *cmatrix,int q, FileInfoPtr fip)
{
    mxArray	*datamatrix,*typematrix,*dimmatrix;
    int 	success;
    int		dim,type;
    int		numdims,*dims;
    int		numn,numvec;
    float	*floatbuffer;
    double	*doublebuffer;
    int		p;
    
    success  = 1;
    
    while(1)
    {
        datamatrix = mxGetField(cmatrix,q,"cdata");
        if(!datamatrix) { errError("WriteCfac","Conductivity data needs to have a field cdata"); break; }
        typematrix = mxGetField(cmatrix,q,"type");
        if(!typematrix) { errError("WriteCfac","Conductivity data needs to have a field type"); break; }
        dimmatrix = mxGetField(cmatrix,q,"dim");
        if(!dimmatrix) { errError("WriteCfac","Conductivity data needs to have a field dim"); break; }
        
        if (!(mxIsNumeric(dimmatrix))) { errError("WriteCfac","Dim matrix needs to be numeric"); break; } 
        if (!(mxIsNumeric(typematrix))) { errError("WriteCfac","Type matrix needs to be numeric"); break; } 
        
        dim = (int) mxGetScalar(dimmatrix);
        if ((dim < 0)||(dim > 3)) { errError("WriteCfac","dim field needs to be 1,2 or 3"); break;}
        type = (int) mxGetScalar(typematrix);
        
        numdims = mxGetNumberOfDimensions(datamatrix);
        dims = (int *)mxGetDimensions(datamatrix);
        
        if (dim == 1)
        {
            if (numdims > 2)  { errError("WriteCfac","scalar can not be three dimensional"); break; } 
            if ((dims[0] != 1)&&(dims[1] != 1)) { errError("WriteCfac","scalar cannot be a 2D matrix"); break;}
            numn = dims[0]; if (numn == 1) numn = dims[1];
            if (!( floatbuffer = (float *) mxCalloc(numn,sizeof(float)))) { errError("WriteCfac","Cannot allocate enough memory"); break;}
            doublebuffer = mxGetPr(datamatrix);
            for(p=0;p<numn;p++) floatbuffer[p] = (float) doublebuffer[p];
            if( setelementscalars_(fip, (long)type, (long)numn, (ScalarPtr) floatbuffer) < 0) { errError("WriteCfac","Cannot set scalar conductivities"); break;}
        }
        if (dim == 2)
        {
            if (numdims > 2)  { errError("WriteCfac","vector can not be three dimensional"); break; } 
            numn = dims[1]; numvec = dims[0];
            if (numvec != 3) { errError("WriteCfac","GraphicsIO can only store vectors of size 3"); break; }
            if (!( floatbuffer = (float *) mxCalloc(numn*numvec,sizeof(float)))) { errError("WriteCfac","Cannot allocate enough memory"); break;}
            doublebuffer = mxGetPr(datamatrix);
            for(p=0;p<numn*numvec;p++) floatbuffer[p] = (float) doublebuffer[p];
            if( setelementvectors_(fip, (long)type,(long) numn, (VectorPtr) floatbuffer) < 0) { errError("WriteCfac","Cannot set vector conductivities"); break;}
        }

        if (dim == 3)
        {
            if (numdims != 3)  { errError("WriteCfac","tensors need to be three dimensional"); break; } 
            if (dims[0] != dims[1]) { errError("WriteCfac","tensor conductivities are stored in a mxmxn"); break;}
            numn = dims[2]; numvec = dims[0];
            if (!( floatbuffer = (float *) mxCalloc(numn*numvec*numvec,sizeof(float)))) { errError("WriteCfac","Cannot allocate enough memory"); break;}
            doublebuffer = mxGetPr(datamatrix);
            for(p=0;p<numn*numvec*numvec;p++) floatbuffer[p] = (float) doublebuffer[p];
            if( setelementtensors_(fip, (long)type,(long) numn, (long) numvec, (TensorPtr) floatbuffer) < 0) { errError("WriteCfac","Cannot set tensor conductivities"); break;}
        }

        success = 1;
        break;
    }
    
    if (floatbuffer) mxFree(floatbuffer);
    
    return(success);

}

