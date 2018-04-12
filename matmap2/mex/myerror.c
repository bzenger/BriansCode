/* FILENAME:  myerror.c
   AUTHOR:    JG STINSTRA
   CONTENTS:  Tools for reporting errors
   LAST UPDATE: 4 JULY 2003 
*/

#ifndef MYERROR_C
#define MYERROR_C 1

#include "myerror.h"

void errUsage(char *usage)
/*    Set the usage string. Set this string upon running the mex function */
{
    static char 	usagebuffer[100];

    if (usage)  /* Load usage string */
    {
        sprintf(usagebuffer,"%0.80s",usage);
    }
    else	/* Print usage string */
    {
        if (usagebuffer[0] != 0)
        {
            printf("USAGE: %0.80s\n",usagebuffer);
            usagebuffer[0] = '\0';
        }
            
    }
        
}

   
void errWarning(char *where,char *warning)
/*    Put a warning to the user */
{
    printf("WARNING (%0.20s): %0.100s\n",where,warning);
}

void errError(char *where, char *error)
/*   This functions deals with putting an error msg to the user */
{
    errUsage(NULL);   /* Dump usage string to screen */
    printf("ERROR (%0.20s): %0.100s\n",where,error);
}

#endif
