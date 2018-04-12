INSTALL -- README

The mex files in this directory need to be compiled for the matmap system
to work properly. In order to compile the mex files use the Makefile in
this directory. The mex files should work under MATLAB R12 and R13.

COMPILING MEXFILES:
The makefile needs to know the OS and the path to the matlab directory:

under OSX:
make osx MATLAB=<matlab dir>

under LINUX:
make linux MATLAB=<matlab dir>

under IRIX:
make irix MATLAB=<matlab dir>

Since the mex files in this directory have a different extension
for each architecture, the files can be compiled for different systems
in the same directory, without the need of creating any new copies
of the matmap package

KNOWN PROBLEMS:

When using GCC 3.x and OSX, an update from the mathworks.com website, 
is needed to be able to compile mex files. The following webpage 
describes the problem:
http://www.mathworks.com/support/solutions/data/35865.shtml


