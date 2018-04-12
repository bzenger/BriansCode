# MAKEFILE TO MAKE THE MEX COMPONENTS UNDER OSX/IRIX/LINUX
#
# USAGE:
# make <OS> MATLAB=<matlabdir>
# make osx MATLAB=/Applications/MATLAB6p5p1
# make linux MATLAB=/usr/local/matlab
# make linux MATLAB=/usr/local/matlab
#

MATLAB =

MEXFLAGS = 
# MEXFLAGS = -DOLDMATLAB

MEX = $(MATLAB)/bin/mex 

GDBMDIR = gdbm

GDBMC = $(GDBMDIR)/gdbmopen.c $(GDBMDIR)/gdbmdelete.c $(GDBMDIR)/gdbmfetch.c $(GDBMDIR)/gdbmstore.c $(GDBMDIR)/gdbmclose.c \
	$(GDBMDIR)/gdbmreorg.c $(GDBMDIR)/gdbmseq.c $(GDBMDIR)/gdbmsync.c $(GDBMDIR)/gdbmerrno.c $(GDBMDIR)/gdbmexists.c $(GDBMDIR)/gdbmfdesc.c \
	$(GDBMDIR)/gdbmsetopt.c $(GDBMDIR)/bucket.c $(GDBMDIR)/falloc.c $(GDBMDIR)/findkey.c $(GDBMDIR)/global.c $(GDBMDIR)/hash.c $(GDBMDIR)/update.c \
	$(GDBMDIR)/version.c $(GDBMDIR)/gdbmpatch.c


GDBMO =  gdbmopen.o  gdbmdelete.o  gdbmfetch.o  gdbmstore.o  gdbmclose.o \
	 gdbmreorg.o  gdbmseq.o  gdbmsync.o  gdbmerrno.o  gdbmexists.o   gdbmfdesc.o \
	 gdbmsetopt.o  bucket.o  falloc.o  findkey.o  global.o  hash.o  update.o \
	 version.o gdbmpatch.o

GRAPHICSIODIR = graphicsio

GRAPHICSIOC = $(GRAPHICSIODIR)/gi_elements.c $(GRAPHICSIODIR)/gi_graphicsio.c $(GRAPHICSIODIR)/gi_leadfiducials.c $(GRAPHICSIODIR)/gi_nodes.c \
	$(GRAPHICSIODIR)/gi_rewrites.c $(GRAPHICSIODIR)/gi_surfaces.c $(GRAPHICSIODIR)/gi_timeseries.c $(GRAPHICSIODIR)/gi_utilities.c

GRAPHICSIOO = gi_elements.o gi_graphicsio.o gi_leadfiducials.o gi_nodes.o \
	gi_rewrites.o gi_surfaces.o gi_timeseries.o gi_utilities.o

GIOCFLAGS_OSX = -c -g  -DOSX
GIOCFLAGS_LINUX = -c -g -DLINUX 
GIOCFLAGS_IRIX = -c -g  -fullwarn -woff 1506

osx:
	cc -c $(GDBMC)
	ar cr gdbm_osx.a $(GDBMO)
	ranlib gdbm_osx.a
	rm *.o
	
	cc -c $(GRAPHICSIOC) $(GIOCFLAGS_OSX)
	ar cr graphicsio_osx.a $(GRAPHICSIOO)
	ranlib graphicsio_osx.a
	rm *.o

	$(MEX)  -Igraphicsio -Igdbm -I. -DOSX mexReadTSDF.c graphicsio_osx.a 
	$(MEX)  -Igraphicsio -Igdbm -I. -DOSX mexReadACQ.c
	$(MEX)  -Igraphicsio -Igdbm -I. -DOSX mexReadGEOM.c graphicsio_osx.a
	$(MEX)  -Igraphicsio -Igdbm -I. -DOSX mexReadTSDFC.c gdbm_osx.a  graphicsio_osx.a
	$(MEX)  -Igraphicsio -Igdbm -I. -DOSX mexListDFC.c gdbm_osx.a
	$(MEX)  -Igraphicsio -Igdbm -I. -DOSX mexIsKeyTSDFC.c gdbm_osx.a
	$(MEX)  -Igraphicsio -Igdbm -I. -DOSX mexWriteGEOM.c  graphicsio_osx.a
	$(MEX)  -Igraphicsio -Igdbm -I. -DOSX mexWriteTSDF.c  graphicsio_osx.a
	$(MEX)  -Igraphicsio -Igdbm -I. -DOSX mexWriteTSDFC.c gdbm_osx.a  graphicsio_osx.a
	$(MEX)  -Igraphicsio -Igdbm -I. -DOSX mexScanTSDF.c  graphicsio_osx.a

linux:
	cc -c $(GDBMC) -fPIC
	ar cr gdbm_linux.a $(GDBMO)
	ranlib gdbm_linux.a
	rm *.o
	
	cc -c $(GRAPHICSIOC) $(GIOCFLAGS_LINUX) -fPIC
	ar cr graphicsio_linux.a $(GRAPHICSIOO)
	ranlib graphicsio_linux.a
	rm *.o

	$(MEX)  mexReadTSDF.c graphicsio_linux.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexReadACQ.c -Igraphicsio -Igdbm -I.  $(MEXFLAGS)
	$(MEX)  mexReadGEOM.c graphicsio_linux.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexReadTSDFC.c gdbm_linux.a  graphicsio_linux.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexListDFC.c gdbm_linux.a -Igraphicsio -Igdbm -I.  $(MEXFLAGS)
	$(MEX)  mexIsKeyTSDFC.c gdbm_linux.a -Igraphicsio -Igdbm -I.  $(MEXFLAGS)
	$(MEX)  mexWriteGEOM.c  graphicsio_linux.a -Igraphicsio -Igdbm -I.  $(MEXFLAGS)
	$(MEX)  mexWriteTSDF.c  graphicsio_linux.a -Igraphicsio -Igdbm -I.  $(MEXFLAGS)
	$(MEX)  mexWriteTSDFC.c gdbm_linux.a  graphicsio_linux.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexScanTSDF.c  graphicsio_linux.a -Igraphicsio -Igdbm -I.  $(MEXFLAGS)

irix:
	cc -c $(GDBMC)
	ar cr gdbm_irix.a $(GDBMO)
	ranlib gdbm_irix.a
	rm *.o
	
	cc -c $(GRAPHICSIOC) $(GIOCFLAGS_IRIX)
	ar cr graphicsio_irix.a $(GRAPHICSIOO)
	ranlib graphicsio_irix.a
	rm *.o

	$(MEX)  mexReadTSDF.c graphicsio_irix.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexReadACQ.c -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexReadGEOM.c graphicsio_irix.a -Igraphicsio -Igdbm -I. $(MEXFLAGS) 
	$(MEX)  mexReadTSDFC.c gdbm_irix.a  graphicsio_irix.a -Igraphicsio -Igdbm -I. $(MEXFLAGS) 
	$(MEX)  mexListDFC.c gdbm_irix.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexIsKeyTSDFC.c gdbm_irix.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexWriteGEOM.c  graphicsio_irix.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexWriteTSDF.c  graphicsio_irix.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexWriteTSDFC.c gdbm_irix.a  graphicsio_irix.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)
	$(MEX)  mexScanTSDF.c  graphicsio_irix.a -Igraphicsio -Igdbm -I. $(MEXFLAGS)


