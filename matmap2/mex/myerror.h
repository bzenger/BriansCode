/* FILENAME:  myerror.h
   AUTHOR:    JG STINSTRA
   CONTENTS:  Tools for reporting errors
   LAST UPDATE: 4 JULY 2003 
*/

#ifndef MYERROR_H
#define MYERROR_H 1

void 	errUsage(char *usage);
void	errError(char *where,char *error);
void	errWarning(char *where,char *warning);

#endif
