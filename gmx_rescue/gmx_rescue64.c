/***************************************************************************
                          gmx_rescue.c  -  description
                             -------------------
    begin                : Fri Apr  5 20:17:10 BST 2002
    origin               : from postings on Gromacs Mailing list by
                           Bert de Groot and Peter Tieleman
    modifications        : (C) 2003 by Marc Baaden, 
                               2004 Oliver Beckstein (large file support), see
                                http://www.ece.utexas.edu/~luo/linux_lfs.html
                                Linux only, LFS support
    email                : baaden@smplinux.de

    compile (Linux): gcc -DLARGEFILE -o gmx_rescue gmx_rescue.c
    
 ***************************************************************************/

/*
 * $Id: gmx_rescue64.c,v 1.3 2004/09/18 11:24:08 oliver Exp $
 * based on gmx_rescue (Id: gmx_rescue.c,v 1.5 2004/09/06 12:26:30)
 *
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef LARGEFILE
/*  see http://www.ece.utexas.edu/~luo/linux_lfs.html  */
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#else
#warning "For Large File Support please set -DLARGEFILE explicitly."
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


/* this only works for linux (I presume) */
#ifndef LINUX
#warning "This source will most likely only work for Linux."
#define LINUX
#endif

/* Linux magic number */
#ifdef LINUX
#define MAGIC_NUMBER -888733696
#endif


int main(int argc,  char *argv[]) {

    FILE* f;
    int   out;
    int value;
    int res, frames;
    off_t offset, n, ln, pos;

    char buf[1024*32];

    if (argc!=3 && argc !=4) {
      fprintf(stderr, 
              "usage: %s <filename> scan\n"
              "       %s <filename> <rescue file> <offset>\n", argv[0], argv[0]);
      fprintf(stderr, 
"First find the offset in the vicinity of the last frame, using 'scan' mode\n"
"(hint: redirect output to a file). Then extract the first half of the\n"
"trajectory with trjconv. Extract the second half into <rescue file> by giving\n"
"the appropriate offset.\n");
#ifdef LARGEFILE
        fprintf(stderr,
"Compiled with Linux Large File Support, allowing files > 2Gb.\n");
#endif
	return 10;
    }

    /* open xtc file readonly (as stream so that we can use LFS 
       enabled functions AND ftello() */
    f = fopen(argv[1], "r");

    if (f == NULL) {
        perror("couldn't open file for reading");
	return 20;
    }

    /* Scan for magic number positions */
    if (strcmp(argv[2],"scan")==0) {
      printf("Frame  Delta  Offset\n");
      ln=0; n=0; frames=0; res=1;
      while (res > 0) {
        while ((res=fread(&value, 1, 4, f)) == 4 && value != MAGIC_NUMBER) {
          n++;
        }
        printf("%5d  %5lld  %lld\n",frames,n-ln,n);
        frames++;
        ln=n;
      }
      return 0;
    }

    offset = (off_t)(4 * atoll(argv[3]));

    /* reposition file offset by setting it to offset bytes */
    if (-1 == fseeko(f, offset, SEEK_SET)) {
      fprintf(stderr,"Failed seeking to offset %lld. Exit.\n", offset/4);
      return(1);
    } else
      printf("Seeked to offset %lld. Now writing file %s.\n",offset/4,argv[2]);

    /* read up to 4 bytes from file into value until magic number is found */
    n=0;
    while (fread(&value, 1, 4, f) == 4 && value != MAGIC_NUMBER) {
      printf("Skipping unreadable stuff.....%lld\r",n++);
    }
    n=0;
    printf("\n");
    /* print real starting position as possible new input 'offset' */
    pos=ftello(f)-4;
    printf("Starting position at offset %lld \n", pos/4);
    
    out=open(argv[2], O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (out == -1) {
        perror("Couldn't open file for writing");
        perror("Please delete the file first, if it exists already !\n");
	return 30;
    }
    write(out, &value, 4);
    n=0;
    while ((res = fread(buf, 1, 1024*32, f)) > 0) {
       write(out, buf, res);
       printf("Writing block %lld\r",n++);
    }
    printf("Done. Written %s\n",argv[2]);
    fclose(f); close(out);
    return 0;
}
