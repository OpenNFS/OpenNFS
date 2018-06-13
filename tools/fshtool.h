/************************************************************************

 New generation FSH/QFS decompressor/compressor
 Version 1.00 - copyright (c) Denis Auroux 1998-2000
 auroux@math.polytechnique.fr
 http://www.math.polytechnique.fr/cmat/auroux/nfs/
 http://auroux.free.fr/nfs/

 This is free software. It is distributed under the terms of the
 GNU General Public License.
 Distributing this software without its source code is illegal.

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
#ifndef OPENNFS3_FSHTOOL_H
#define OPENNFS3_FSHTOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

/* Windows compatibility */
#ifdef _WIN32
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

void abort();
void sanity_check() ;

void mmemcpy(char *dest,char *src,int len);
void quotify(unsigned char *src,int len,char *dest);
void hexify(unsigned char *src,int len,char *dest);

void unquotify(char *src,int len,unsigned char *dest);
/* uncompressing a QFS file */
unsigned char *uncompress_data(unsigned char *inbuf,int *buflen);
void compress_data(unsigned char *inbuf,int *buflen,unsigned char *outbuf);
/* convert a color palette from NFS format to BMP format */
void makepal(unsigned char *pos,int *len,int *pal);

void fsh_to_bmp(char *fshname);
char *next_lf(char *p);
void idxerr();
void outmem();
void map_file(char *name,unsigned char **ptr,int *size);
void bmp_get_file(char *bmpname,unsigned char code,int width,int height,
                  int *locpal,unsigned char **data,int *datalen);
void bmp_get_alpha(char *bmpname,unsigned char code,int width,int height,
                   unsigned char *data);
int make_nfs_pal(int code,int len,unsigned char *dest,unsigned int *src);
unsigned char *bmp_to_fsh();
int fsh_main(int argc, char **argv);

#endif //OPENNFS3_FSHTOOL_H