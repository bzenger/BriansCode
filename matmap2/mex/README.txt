   NOTES ON MEX FILES
   The mex files in this directory are used for importing and exporting files. Most of the
   file formats used at CVRTI are supported through these interface functions. The functions
   do not need to be called directly. Use the ioRead... and ioWrite... functions to read 
   and write CVRTI files. The different fileformats supported are:

   SUPPORTED FILES

   acq	 - Raw files from the acquisition system
   tsdf  - Time series data file
   tsdfc - Container for the tsdf-files
   dfc   - Container for tsdfc files
   geom  - Geometry files
   data  - File format similar to tsdf

   NOT SUPPORTED YET
   pak   - Raw files of the old acquisition system 

   CROSS PLATFORM COMPATABILITY   
   SGI    - functions compile and run 
   MAC    - functions compile and run
   LINUX  - functions compile and run

   LIBRARIES NEEDED
   No explicit libraries are needed. Everything is included in the package and all
   support libraries are compiled when making the mex files. Problems with the GDBM
   library have been fixed as well and everything should work fine across platforms.

 