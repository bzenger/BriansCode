#ifndef TSTOOLS_H
#define TSTOOLS_H 1

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <mex.h>
#include "myerror.h"
#include "misctools.h"

/* Put data in a TS (matlab)structure */

int 	 tsPutTSlong(mxArray *TS,long arrayindex,long idnumber, long value);				/* put a long value in the TS-structure */
int 	 tsPutTSstring(mxArray *TS,long arrayindex,long idnumber, char *string);		 	/* put a string in the TS-structure */
int 	 tsPutTSunit(mxArray *TS,long arrayindex, long idnumber, long unit);				/* Do the unit to string conversion */

mxArray *tsFetchTS(); 								 			/* Get TS pointer */

/* Get data from TS (matlab array) structure */

int	 tsGetTSstring(mxArray *TS,long arrayindex, char *idstring,char **string); 			/* Get a string out of TS-structure */
int 	 tsGetTSlong(mxArray *TS,long arrayindex,char *idstring, long *value); 				/* Get a int from the TS */
int 	 tsGetTSunit(mxArray *TS,long arrayindex,long *value); 						/* Do unit conversion as well */
int 	 tsGetTSpotvals(mxArray *TS,long arrayindex,float **potvals,long *leadnum,long *framenum); 	/* getting data */
int 	 tsGetTSleadinfo(mxArray *TS,long arrayindex,long **leadinfo,long *numleadinfo); 		/* Getting bad leadinfo */

#endif
