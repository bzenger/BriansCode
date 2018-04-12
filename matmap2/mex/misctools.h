/* FILENAME:  misctools.h
   AUTHOR:    JG STINSTRA
   CONTENTS:  Collection of different general purpose tools
   LAST UPDATE: 4 JULY 2003 
*/

#ifndef MISCTOOLS_H
#define MISCTOOLS_H 1

#include "myerror.h"
#include <stdio.h>
#include <stdlib.h>
#include <mex.h>

char *miscConvertString(mxArray *stringarray);
mxArray *miscStructToCell(mxArray *structarray); 

#endif
