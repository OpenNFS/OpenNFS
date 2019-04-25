/************************************************************************

 New generation FSH/QFS decompressor/compressor
 Version 1.22 - copyright (c) Denis Auroux 1998-2002
 auroux@math.polytechnique.fr
 http://www.math.polytechnique.fr/cmat/auroux/nfs/
 http://auroux.free.fr/nfs/

 This is free software. It is distributed under the terms of the
 GNU General Public License.
 Distributing this software without its source code is illegal.

 CHANGES in v1.22:
 - added compatibility with multiscale bitmaps from FIFAglxinfo | grep OpenGL"glxinfo | grep OpenGL"

 CHANGES in v1.21:
 - added bitmap type 0x60 from NFS6 demo

 CHANGES in v1.2:
 - bug fix for insufficient memory allocation
 - added bitmap type 0x61 from NFS6 demo
 - added text attachment types 0x69 & 0x70 from NFS6 demo

 CHANGES in v1.11:
 - bug fix for global palettes

 CHANGES in v1.10 since v1.00:

 - extract 256-color bitmap alpha channels to ALPHA8.DAT text file
 - lost alpha-channel data in 256-color BMPs is read from ALPHA8.DAT
   or defaults to white.

 CHANGES since QFS Suite:

 - integrated everything into one single program
 - eliminated intermediary FSH step
 - Linux portability
 - QFS compression code is 20 times faster :-)
 - can be Windows-click-click-driven
 - added support for new NFS5 files
   (palette type 0x29, bitmap type 0x6D, compressed/multiscale bitmaps)
 - extracts alpha layer when present
 - SHPI.DAT has been replaced by human-readable INDEX.FSH
   (modify only with great care !)

 ************************************************************************/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>


/* Windows compatibility */
#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#elif _WIN32
#include <direct.h>
#define rmdir _rmdir
#define mkdir(a,b) _mkdir(a)
#define chdir _chdir
#endif

/* QFS compression quality factor */
#define QFS_MAXITER 50  /* quick and not so bad */

typedef struct FSH_HDR {  /* header of a FSH file */
    char SHPI[4]; /* 'SHPI' */
    int filesize;
    int nbmp;
    char dirId[4];
} FSH_HDR;

typedef struct BMPDIR {
    char name[4];
    int ofs;
} BMPDIR;

typedef struct BMPHEAD {
    int size,null,ofsbmp;
    int hsz,wid,hei;
    short planes,bpp;
    int compr,imsz,xpel,ypel,clrs,clri;
} BMPHEAD;

typedef struct ENTRYHDR {
    int code;
    short width,height;
    short misc[4];
} ENTRYHDR;

void usage();

void abandon_ship();

void sanity_check(); /* make sure we haven't used a braindead compiler */

void mmemcpy(char *dest,char *src,int len); /* LZ-compatible memcopy */

void quotify(unsigned char *src,int len,char *dest);

void hexify(unsigned char *src,int len,char *dest);

void unquotify(char *src,int len,unsigned char *dest);

void unhexify(char *src,int len,unsigned char *dest);

/* uncompressing a QFS file */
unsigned char *uncompress_data(unsigned char *inbuf,int *buflen);

/* compressing a QFS file */
/* note: inbuf should have at least 1028 bytes beyond buflen */
void compress_data(unsigned char *inbuf,int *buflen,unsigned char *outbuf);

/* convert a color palette from NFS format to BMP format */
void makepal(unsigned char *pos,int *len,int *pal);

/* write a line of alpha channel data text */
int makealpha8(FILE **alpha8,char *fname,int *pal);

/* unpack a DXT packed color */
void unpack_dxt(unsigned char mask, unsigned short col1, unsigned short col2, unsigned char *target);

/* pack a DXT 4x4 cell; px = 16 RGB colors; nstep = 2 or 3; dest = 8 bytes */
int score_dxt(unsigned long *px, int nstep, unsigned long col1, unsigned long col2, unsigned long *pack);

void pack_dxt(unsigned long *px, unsigned char *dest);

/* FSH expansion to BMPs */
void fsh_to_bmp(char *fshname);

char *next_lf(char *p);

void idxerr();

void outmem();

void map_file(char *name,unsigned char **ptr,int *size);

void alpha8_fix_locpal(char *fname,char *alpha8,int *locpal);

/* BMP to FSH conversion */
void bmp_get_file(char *bmpname,unsigned char code,int width,int height, int *locpal,unsigned char **data,int *datalen,char *alpha8);

void bmp_get_alpha(char *bmpname,unsigned char code,int width,int height, unsigned char *data);

int make_nfs_pal(int code,int len,unsigned char *dest,unsigned int *src);

char *read_alpha8();

unsigned char *bmp_to_fsh();

int fsh_main(int argc,char **argv);


