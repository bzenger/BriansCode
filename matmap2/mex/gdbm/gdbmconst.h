/* gdbmconst.h - The constants defined for use in gdbm. */

/*  This file is part of GDBM, the GNU data base manager, by Philip A. Nelson.
    Copyright (C) 1990, 1991, 1993  Free Software Foundation, Inc.

    GDBM is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2, or (at your option)
    any later version.

    GDBM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with GDBM; see the file COPYING.  If not, write to
    the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

    You may contact the author by:
       e-mail:  phil@cs.wwu.edu
      us-mail:  Philip A. Nelson
                Computer Science Department
                Western Washington University
                Bellingham, WA 98226
       
*************************************************************************/

/* PATCHED VERSION OF GDBM LIBRARY *******************************************
 * This patch is intended to copy the functionality of the 
 * GDBM library as it is found on the SGI platform.
 * The original version of the library created different 
 * fileformats at different platforms. This patched version
 * intends to create the same file at each different platform.
 * The current patched version has been tested for OS X, IRIX,
 * and LINUX. As the original files were all created under
 * IRIX, this patch conforms to the files created at that
 * platform.
 *
 * patch includes:
 * 1) definition of off_t as my_int64_t to be sure that this
 *    field is 8 bytes across platforms. off_t is 4 bytes
 *    on linux and 8 bytes on sgi and osx.
 * 2) includes byteswapping to store the files in the SGI
 *    native format to ensure cross platform compatibility.
 * 3) saving data structures field by field to ensure that
 *    no zero padding is added in between members of these
 *    structures.
 * 4) to add empty dummy fields at those places where the original
 *    SGI version of this library did put additional zeros to have
 *    64 bit integers align properly in memory.
 *
 * notes:
 * the definition of my_int64_t in autoconf.h may need some adjustments
 * in the future to be compatible with more compilers.
 *
 * All lines that have been patched are marked with a PATCH marker
 * the only exception is the conversion of off_t to my_int64_t
 *
 * patch generated by : JG Stinstra
 *
 ****************************************************************************/



/* Start with the constant definitions.  */
#define  TRUE    1
#define  FALSE   0

/* Parameters to gdbm_open. */
#define  GDBM_READER  0		/* READERS only. */
#define  GDBM_WRITER  1		/* READERS and WRITERS.  Can not create. */
#define  GDBM_WRCREAT 2		/* If not found, create the db. */
#define  GDBM_NEWDB   3		/* ALWAYS create a new db.  (WRITER) */
#define  GDBM_OPENMASK 7	/* Mask for the above. */
#define  GDBM_FAST    0x10	/* Write fast! => No fsyncs.  OBSOLETE. */
#define  GDBM_SYNC    0x20	/* Sync operations to the disk. */
#define  GDBM_NOLOCK  0x40	/* Don't do file locking operations. */

/* Parameters to gdbm_store for simple insertion or replacement in the
   case a key to store is already in the database. */
#define  GDBM_INSERT  0		/* Do not overwrite data in the database. */
#define  GDBM_REPLACE 1		/* Replace the old value with the new value. */

/* Parameters to gdbm_setopt, specifing the type of operation to perform. */
#define	 GDBM_CACHESIZE	1	/* Set the cache size. */
#define  GDBM_FASTMODE	2	/* Turn on or off fast mode.  OBSOLETE. */
#define  GDBM_SYNCMODE	3	/* Turn on or off sync operations. */
#define  GDBM_CENTFREE	4	/* Keep all free blocks in the header. */
#define  GDBM_COALESCEBLKS 5	/* Attempt to coalesce free blocks. */

/* In freeing blocks, we will ignore any blocks smaller (and equal) to
   IGNORE_SIZE number of bytes. */
#define IGNORE_SIZE 4

/* The number of key bytes kept in a hash bucket. */
#define SMALL    4

/* The number of bucket_avail entries in a hash bucket. */
#define BUCKET_AVAIL 6

/* The size of the bucket cache. */
#define DEFAULT_CACHESIZE  100
