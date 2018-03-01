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

int inlen;
unsigned char *inbuf;
int iscompr;
int fshlen;
char *fshname;
int linecount;

void usage()
{
    printf("Usage: FSHTOOL filename.FSH [directory]\n");
    printf("       FSHTOOL filename.QFS [directory]\n");
    printf("       FSHTOOL directory\\INDEX.FSH [filename]\n\n");
    printf("Windows users: you can run FSHTOOL simply by double-clicking\n");
    printf("on a .FSH or .QFS file and telling Windows that you wish to\n");
    printf("use FSHTOOL to open the file (this only needs to be done the\n");
    printf("first time you open a file with given extension)\n\n");
    printf("SEE THE FILE README.TXT FOR MORE DETAILED INSTRUCTIONS\n\n");
}

void abort()
{
    printf("\nUnsuccessful termination. See README.TXT for more details.\n");
    printf("Press Enter to exit.\n"); getchar();
    exit(0);
}

void sanity_check() /* make sure we haven't used a braindead compiler */
{
  int x;
  
   if (sizeof(int)!=4) { printf("Problem: int is not 32-bit\n"); abort(); }
   if (sizeof(short)!=2) { printf("Problem: short is not 16-bit\n"); abort(); }
   if ((sizeof(struct BMPHEAD)!=52)||(sizeof(struct FSH_HDR)!=16)||
       (sizeof(struct BMPDIR)!=8)||(sizeof(struct ENTRYHDR)!=16))
     {  printf("Problem: structs are not correctly packed\n"); abort(); }
   x=0;
   *((char *)(&x))=1;
   if (x!=1) { printf("Problem: incorrect endianness on this architecture\n"); abort(); }
}

void mmemcpy(char *dest,char *src,int len) /* LZ-compatible memcopy */
{
  while (len--) *(dest++)=*(src++);
}

void quotify(unsigned char *src,int len,char *dest)
{
  while (len--) {
    if ((*src>32)&&(*src<=126)&&(*src!='%'))
      *(dest++)=*(src++);
    else {
      *(dest++)='%';
      *(dest++)="0123456789ABCDEF"[(*src)>>4];
      *(dest++)="0123456789ABCDEF"[(*src)&15];
      src++;
    }
  }
  *dest=0;
}

void hexify(unsigned char *src,int len,char *dest)
{
  while (len--) {
      *(dest++)="0123456789ABCDEF"[(*src)>>4];
      *(dest++)="0123456789ABCDEF"[(*src)&15];
      src++;
  }
  *dest=0;
}

void unquotify(char *src,int len,unsigned char *dest)
{
  while (len--) {
    if (*src!='%')
      *(dest++)=*(src++);
    else {
      src++;
      if (*src<='9') *dest=(*src-'0'); else *dest=10+((*src-'A')&15);
      *dest=(*dest<<4); src++;
      if (*src<='9') *dest+=(*src-'0'); else *dest+=10+((*src-'A')&15);
      src++; dest++;
    }
  }
}

void unhexify(char *src,int len,unsigned char *dest)
{
  while (len--) {
    if (*src<='9') *dest=(*src-'0'); else *dest=10+((*src-'A')&15);
    *dest=(*dest<<4); src++;
    if (*src<='9') *dest+=(*src-'0'); else *dest+=10+((*src-'A')&15);
    src++; dest++;
  }
}

/* uncompressing a QFS file */
unsigned char *uncompress_data(unsigned char *inbuf,int *buflen)
{
  unsigned char *outbuf;
  unsigned char packcode;
  int a,b,c,len,offset;
  int inlen,outlen,inpos,outpos;
  
  /* length of data */
  inlen=*buflen;
  outlen=(inbuf[2]<<16)+(inbuf[3]<<8)+inbuf[4];
  outbuf=malloc(outlen);
  if (outbuf==NULL) { printf("Insufficient memory.\n"); abort(); }
  
  /* position in file */
  if (inbuf[0]&0x01) inpos=8; else inpos=5;
  outpos=0;
  
  /* main decoding loop */
  while ((inpos<inlen)&&(inbuf[inpos]<0xFC))
  {
    packcode=inbuf[inpos];
    a=inbuf[inpos+1];
    b=inbuf[inpos+2];
    
    if (!(packcode&0x80)) {
      len=packcode&3;
      mmemcpy(outbuf+outpos,inbuf+inpos+2,len);
      inpos+=len+2;
      outpos+=len;
      len=((packcode&0x1c)>>2)+3;
      offset=((packcode>>5)<<8)+a+1;
      mmemcpy(outbuf+outpos,outbuf+outpos-offset,len);
      outpos+=len;
    }
    else if (!(packcode&0x40)) {
      len=(a>>6)&3; 
      mmemcpy(outbuf+outpos,inbuf+inpos+3,len);
      inpos+=len+3;
      outpos+=len;
      len=(packcode&0x3f)+4;
      offset=(a&0x3f)*256+b+1;
      mmemcpy(outbuf+outpos,outbuf+outpos-offset,len);
      outpos+=len;
    }  
    else if (!(packcode&0x20)) {
      c=inbuf[inpos+3];
      len=packcode&3; 
      mmemcpy(outbuf+outpos,inbuf+inpos+4,len);
      inpos+=len+4;
      outpos+=len;
      len=((packcode>>2)&3)*256+c+5;
      offset=((packcode&0x10)<<12)+256*a+b+1;
      mmemcpy(outbuf+outpos,outbuf+outpos-offset,len);
      outpos+=len;
    }  
    else {
      len=(packcode&0x1f)*4+4;
      mmemcpy(outbuf+outpos,inbuf+inpos+1,len);
      inpos+=len+1;
      outpos+=len;
    }
  }
  
  /* trailing bytes */
  if ((inpos<inlen)&&(outpos<outlen)) {
    mmemcpy(outbuf+outpos,inbuf+inpos+1,inbuf[inpos]&3);
    outpos+=inbuf[inpos]&3;
  }
  
  if (outpos!=outlen) printf("Warning: bad length ? %d instead of %d\n",outpos,outlen);
  *buflen=outlen;
  return outbuf;
}

/* compressing a QFS file */
/* note: inbuf should have at least 1028 bytes beyond buflen */

void compress_data(unsigned char *inbuf,int *buflen,unsigned char *outbuf)
{

#define WINDOW_LEN (1<<17)
#define WINDOW_MASK (WINDOW_LEN-1)

  unsigned char *inrd,*inref,*incmp;
  int *rev_similar; /* where is the previous occurrence */
  int **rev_last;   /* idem */
  int offs,len,bestoffs,bestlen,lastwrot,i;
  int inpos,inlen,outpos;
  int *x;
  
  inlen=*buflen;
  inpos=0;
  inrd=inbuf;
  rev_similar=(int *)malloc(4*WINDOW_LEN);
  rev_last=(int **)malloc(256*sizeof(int *));
  if (rev_last) rev_last[0]=(int *)malloc(65536*4);
  
  if ((outbuf==NULL)||(rev_similar==NULL)||
      (rev_last==NULL)||(rev_last[0]==NULL))
    { printf("Insufficient memory.\n"); abort(); }
  for (i=1;i<256;i++) rev_last[i]=rev_last[i-1]+256;
  memset(rev_last[0],0xff,65536*4);
  memset(rev_similar,0xff,4*WINDOW_LEN);
  
  outbuf[0]=0x10; outbuf[1]=0xFB;
  outbuf[2]=inlen>>16; outbuf[3]=(inlen>>8)&255; outbuf[4]=inlen&255;
  outpos=5;
  lastwrot=0;
  
  /* main encoding loop */
  for (inpos=0,inrd=inbuf;inpos<inlen;inpos++,inrd++) {
    if ((inpos&0x3fff)==0) { putchar('.'); fflush(stdout); }
    /* adjust occurrence tables */
    x=rev_last[*inrd]+(inrd[1]);
    offs=rev_similar[inpos&WINDOW_MASK]=*x;
    *x=inpos;
    /* if this has already been compressed, skip ahead */
    if (inpos<lastwrot) continue;

    /* else look for a redundancy */
    bestlen=0; i=0;
    while ((offs>=0)&&(inpos-offs<WINDOW_LEN)&&(i++<QFS_MAXITER)) {
      len=2; incmp=inrd+2; inref=inbuf+offs+2;
      while ((*(incmp++)==*(inref++))&&(len<1028)) len++;
      if (len>bestlen) { bestlen=len; bestoffs=inpos-offs; }
      offs=rev_similar[offs&WINDOW_MASK];
    }
    
    /* check if redundancy is good enough */
    if (bestlen>inlen-inpos) bestlen=inpos-inlen;
    if (bestlen<=2) bestlen=0;
    if ((bestlen==3)&&(bestoffs>1024)) bestlen=0;
    if ((bestlen==4)&&(bestoffs>16384)) bestlen=0;

    /* update compressed data */
    if (bestlen) {
      while (inpos-lastwrot>=4) {
        len=(inpos-lastwrot)/4-1;
        if (len>0x1B) len=0x1B;
        outbuf[outpos++]=0xE0+len;
        len=4*len+4;
        memcpy(outbuf+outpos,inbuf+lastwrot,len);
        lastwrot+=len;
        outpos+=len;
      }
      len=inpos-lastwrot;
      if ((bestlen<=10)&&(bestoffs<=1024)) {
        outbuf[outpos++]=(((bestoffs-1)>>8)<<5)+((bestlen-3)<<2)+len;
        outbuf[outpos++]=(bestoffs-1)&0xff;
        while (len--) outbuf[outpos++]=inbuf[lastwrot++];
        lastwrot+=bestlen;
      }
      else if ((bestlen<=67)&&(bestoffs<=16384)) {
        outbuf[outpos++]=0x80+(bestlen-4);
        outbuf[outpos++]=(len<<6)+((bestoffs-1)>>8);
        outbuf[outpos++]=(bestoffs-1)&0xff;
        while (len--) outbuf[outpos++]=inbuf[lastwrot++];
        lastwrot+=bestlen;
      }
      else if ((bestlen<=1028)&&(bestoffs<WINDOW_LEN)) {
        bestoffs--;
        outbuf[outpos++]=0xC0+((bestoffs>>16)<<4)+(((bestlen-5)>>8)<<2)+len;
        outbuf[outpos++]=(bestoffs>>8)&0xff;
        outbuf[outpos++]=bestoffs&0xff;
        outbuf[outpos++]=(bestlen-5)&0xff;
        while (len--) outbuf[outpos++]=inbuf[lastwrot++];
        lastwrot+=bestlen;
      }
    }
  }
  
  /* end stuff */
  inpos=inlen;
  while (inpos-lastwrot>=4) {
    len=(inpos-lastwrot)/4-1;
    if (len>0x1B) len=0x1B;
    outbuf[outpos++]=0xE0+len;
    len=4*len+4;
    memcpy(outbuf+outpos,inbuf+lastwrot,len);
    lastwrot+=len;
    outpos+=len;
  }
  len=inpos-lastwrot;
  outbuf[outpos++]=0xFC+len;
  while (len--) outbuf[outpos++]=inbuf[lastwrot++];

  if (lastwrot!=inlen) {
    printf("Something strange happened at the end of compression!\n");
    abort();
  }
  *buflen=outpos;
}

/* convert a color palette from NFS format to BMP format */
void makepal(unsigned char *pos,int *len,int *pal)
{
  int i,code;
  struct ENTRYHDR *hdr;
  unsigned char *ptr;
  unsigned short *p16;
  
  memset(pal,0,1024);
  ptr=pos+16;
  hdr=(struct ENTRYHDR *)pos;
  code=hdr->code&0xff;
  *len=hdr->width;

  if (code==0x24) {
    for (i=0;i<*len;i++,ptr+=3)
      pal[i]=65536*ptr[0]+256*ptr[1]+ptr[2];
  } else if (code==0x22) {
    for (i=0;i<*len;i++,ptr+=3)
      pal[i]=(65536*ptr[0]+256*ptr[1]+ptr[2])<<2;
  } else if (code==0x2D) {
    p16=(unsigned short *)ptr;
    for (i=0;i<*len;i++,p16++) {
      pal[i]=((*p16&0x1F)+256*((*p16>>5)&0x1F)+65536*((*p16>>10)&0x1F))<<3;
      if (*p16&0x8000) pal[i]+=0xFF000000;
    }
  } else if (code==0x29) {
    p16=(unsigned short *)ptr;
    for (i=0;i<*len;i++,p16++) {
      pal[i]=((*p16&0x1f)+128*((*p16>>5)&0x3F)+65536*((*p16>>11)&0x1F))<<3;
    }
  } else if (code==0x2A) {
    memcpy(pal,ptr,4*(*len));
  }
  else { printf("Unknown palette format.\n"); abort(); }
}    
      
/* FSH expansion to BMPs */

void fsh_to_bmp(char *fshname)
{
  FILE *log,*bmp,*alpha;
  unsigned short *sbuf;
  int nbmp,i,j,k,l,hasglobpal,globpallen,offs,nxoffs,paloffs,auxoffs,nattach;
  int isbmp,locpallen,bmpw,curoffs,compressed;
  int numscales;
  int globpal[256],locpal[256];
  struct FSH_HDR *fshhdr;
  struct BMPDIR *dir;
  struct ENTRYHDR *hdr,*auxhdr,*palhdr;
  struct BMPHEAD bmphdr;
  unsigned char pad[32768],*ptr,*bmpptr;
  
  fshhdr=(struct FSH_HDR *)inbuf;
  dir=(struct BMPDIR *)(inbuf+16);
  log=fopen("index.fsh","wt");
  if (log==NULL) { printf("Unable to create index.fsh\n"); abort(); }
  fprintf(log,"FSHTool generated file -- be very careful when editing\n");
  fprintf(log,"%s\n",fshname);
  if (iscompr) fprintf(log,"QFS\n"); else fprintf(log,"FSH\n");
  nbmp=fshhdr->nbmp;
  sprintf(pad,"SHPI %d objects, tag ",nbmp);
  quotify(fshhdr->dirId,4,pad+strlen(pad));
  puts(pad);
  fprintf(log,"%s\n",pad);
  
  if (inlen<500000) i=2*inlen;
  else i=inlen+500000;
  fprintf(log,"BUFSZ %d\n",i); /* buffer alloc size for file recomposition */
  
  /* look for a global palette */
  j=-1; hasglobpal=0;
  for (i=0;i<nbmp;i++) if (!hasglobpal) {
    hdr=(struct ENTRYHDR *)(inbuf+dir[i].ofs);
    k=hdr->code&0xff;
    if ((k==0x22)||(k==0x24)||(k==0x2D)||(k==0x2A)||(k==0x29)) j=i;
    if (!strncmp(dir[i].name,"!pal",4)) hasglobpal=1;
  }
  hasglobpal=j;
  if (hasglobpal>=0) {
   makepal(inbuf+dir[hasglobpal].ofs,&globpallen,globpal);
   fprintf(log,"GLOBPAL %04d.BIN\n",hasglobpal);
   printf("Found a global palette (tag '%c%c%c%c', will be saved as '%04d.BIN') with %d colors.\n",
     dir[hasglobpal].name[0],dir[hasglobpal].name[1],
     dir[hasglobpal].name[2],dir[hasglobpal].name[3],hasglobpal,globpallen);
  }
  else {
    fprintf(log,"NOGLOBPAL\n");
    printf("No global palette.\n");
  }

  j=fshhdr->filesize;
  for (i=0;i<nbmp;i++) if (dir[i].ofs<j) j=dir[i].ofs;
  if (j>16+8*nbmp) {
    fprintf(log,"!PAD %d ",j-16-8*nbmp);
    hexify(inbuf+16+8*nbmp,j-16-8*nbmp,pad);
    fprintf(log,"%s\n",pad);
  }
  
  /* main loop */
  for (i=0;i<nbmp;i++) {
    offs=dir[i].ofs;
    /* offset of the following entry ? */
    nxoffs=fshhdr->filesize;
    for (j=0;j<nbmp;j++) 
      if ((dir[j].ofs<nxoffs)&&(dir[j].ofs>offs)) nxoffs=dir[j].ofs;
    if (((i==nbmp-1)&&(nxoffs!=fshhdr->filesize)) ||
        ((i<nbmp-1)&&(nxoffs!=dir[i+1].ofs))) {
      printf("WARNING: FSH bitmaps are not correctly ordered.\n"
 "The reverse conversion from .BMP to .FSH/.QFS may give a corrupted file.\n");
      printf("Press Enter to continue.\n");
      getchar();
    }
    
    /* understand the attachment structure */
    hdr=(struct ENTRYHDR *)(inbuf+offs);
    j=hdr->code&0x7f;
    isbmp = (j==0x78)||(j==0x7B)||(j==0x7D)||(j==0x7E)||(j==0x7F)||(j==0x6D);
    compressed = (hdr->code&0x80);

    if (isbmp) {
      auxhdr=hdr; auxoffs=offs; nattach=0; palhdr=NULL; paloffs=0;
      while (auxhdr->code>>8) {
       nattach++;
       auxoffs+=(auxhdr->code>>8);
       if (auxoffs>nxoffs) {
         printf("ERROR: incorrect attachment structure !\n"); abort();
       }
       if (auxoffs==nxoffs) break;
       auxhdr=(struct ENTRYHDR *)(inbuf+auxoffs);
       j=auxhdr->code&255;
       if ((hdr->code&0x7f)==0x7B)
         if ((j==0x22)||(j==0x24)||(j==0x2D)||(j==0x2A)||(j==0x29))
           { palhdr=auxhdr; paloffs=auxoffs; }
      }
      
      numscales=0;
      if ((nattach==0)&&!compressed) { /* look for multiscale NFS5 bitmaps ? */
        if ((hdr->misc[3]&0x0fff)==0) numscales=(hdr->misc[3]>>12)&0x0f;
        if ((hdr->width!=(1<<numscales))||(hdr->height!=(1<<numscales)))
          numscales=0;
        if (numscales) {
          j=hdr->code&0x7f;
          if (j==0x7B) k=1;
          else if (j==0x7D) k=4;
          else if (j==0x7F) k=3;
          else k=2;
          j=0;
          for (l=0;l<=numscales;l++) { 
            j+=k<<(2*l);
            j+=((16-j)&15);
          }
          if (j+offs+16!=nxoffs) numscales=0;
        }
      }
    }
    
    /* name things */
    quotify(dir[i].name,4,pad);
    printf("'%s' [%02X] -> ",pad,hdr->code&0xff);
    fprintf(log,"%s ",pad);
    
    if (isbmp) {
      l=numscales;
      curoffs=offs+16;
      bmpptr=inbuf;
      j=nxoffs-curoffs;
      if (compressed) {
        bmpptr=uncompress_data(inbuf+curoffs,&j);
        curoffs=0;
      }

      if (!numscales) {
        sprintf(pad,"%04d.BMP",i);
        printf("'%s' (%dx%d) ",pad,hdr->width,hdr->height);
        if (compressed) printf("(compressed) ");
        fprintf(log,"%s\n",pad);
        fprintf(log,"BMP %02X +%d %d %d {%d %d %d %d}\n",
          hdr->code&0xff,nattach,hdr->width,hdr->height,
          hdr->misc[0],hdr->misc[1],hdr->misc[2],hdr->misc[3]);
      } else {
        printf("multiscale '%04d-[0 to %d].BMP' (up to %dx%d) ",
          i,numscales,hdr->width,hdr->height);
        fprintf(log,"%04d-%%d.BMP\n",i);
        fprintf(log,"MBP %02X x%d %d %d {%d %d %d}\n",
          hdr->code&0xff,numscales,hdr->width,hdr->height,
          hdr->misc[0],hdr->misc[1],hdr->misc[2]);
      }
     
     while (l>=0) {
      if (numscales) sprintf(pad,"%04d-%d.BMP",i,l);
      else sprintf(pad,"%04d.BMP",i);
      bmp=fopen(pad,"wb");
      if (bmp==NULL) { printf("Unable to create file.\n"); abort(); }
      
      /* prepare a BMP header */
      fputc('B',bmp);
      fputc('M',bmp);
      memset(&bmphdr,0,sizeof(bmphdr));
      bmphdr.hsz=40;
      bmphdr.planes=1;
      bmphdr.wid=hdr->width;
      bmphdr.hei=hdr->height;

      /* and also for the alpha channel if needed */
      if (((hdr->code&0x7f)==0x7D)||((hdr->code&0x7f)==0x7E)||
          ((hdr->code&0x7f)==0x6D)) {
        if (numscales) sprintf(pad,"%04d-%da.BMP",i,l);
        else sprintf(pad,"%04d-a.BMP",i);
        if (l==numscales) { /* only the first time */ 
          if (numscales) fprintf(log,"alpha %04d-%%da.BMP\n",i);
          else fprintf(log,"alpha %s\n",pad);
          printf(" (alpha: %s) ",pad);
        }
        
        alpha=fopen(pad,"wb");
        if (alpha==NULL) { printf("Unable to create file.\n"); abort(); }
        fputc('B',alpha);
        fputc('M',alpha);
        /* alpha channel is always 8-bit */
        bmpw=hdr->width;
        while (bmpw&3) bmpw++;
        bmphdr.size=54+4*256+bmpw*hdr->height;
        bmphdr.ofsbmp=54+4*256;
        bmphdr.bpp=8;
        bmphdr.imsz=bmpw*hdr->height;
        fwrite(&bmphdr,sizeof(bmphdr),1,alpha);
      }
      
      if ((hdr->code&0x7f)==0x7B) { /* 8-bit bitmap */
        if (l==numscales) printf("8 bit ");
        if (palhdr!=NULL) {
          makepal(inbuf+paloffs,&locpallen,locpal);
          if (l==numscales) printf("(local palette)\n");
        } 
        else if (hasglobpal>=0) {
          memcpy(locpal,globpal,1024);
          locpallen=globpallen;
          if (l==numscales) printf("(global palette)\n");
        }
        else {
          memset(locpal,0,1024);
          if (l==numscales) printf("(NO PALETTE !)\n");
          locpallen=0;
        }
        
        bmpw=hdr->width;
        while (bmpw&3) bmpw++;
        bmphdr.size=54+4*256+bmpw*hdr->height;
        bmphdr.ofsbmp=54+4*256;
        bmphdr.bpp=8;
        bmphdr.imsz=bmpw*hdr->height;
        fwrite(&bmphdr,sizeof(bmphdr),1,bmp);
        fwrite(locpal,1,1024,bmp);
        for (k=hdr->height-1;k>=0;k--)
          fwrite(bmpptr+curoffs+k*hdr->width,1,bmpw,bmp);
        curoffs+=hdr->width*hdr->height;
      }
      else
      if ((hdr->code&0x7f)==0x7D) { /* 32-bit bitmap */
        if (l==numscales) printf("32 bit (8:8:8:8)\n");
        
        for (k=0;k<256;k++) locpal[k]=k*0x01010101;
        fwrite(locpal,1,1024,alpha);
        for (k=hdr->height-1;k>=0;k--) {
          ptr=bmpptr+curoffs+4*k*hdr->width;
          for (j=0;j<hdr->width;j++) pad[j]=ptr[4*j+3];
          fwrite(pad,1,bmpw,alpha);
        }
        fclose(alpha);
        
        bmpw=3*hdr->width;
        while (bmpw&3) bmpw++;
        bmphdr.size=54+bmpw*hdr->height;
        bmphdr.ofsbmp=54;
        bmphdr.bpp=24;
        bmphdr.imsz=bmpw*hdr->height;
        fwrite(&bmphdr,sizeof(bmphdr),1,bmp);
        for (k=hdr->height-1;k>=0;k--) {
          ptr=bmpptr+curoffs+4*k*hdr->width;
          for (j=0;j<hdr->width;j++) 
           { pad[3*j]=ptr[4*j]; pad[3*j+1]=ptr[4*j+1]; pad[3*j+2]=ptr[4*j+2]; }
          fwrite(pad,1,bmpw,bmp);
        }
        curoffs+=4*hdr->width*hdr->height;
      }
      else
      if ((hdr->code&0x7f)==0x7F) { /* 24-bit bitmap */
        if (l==numscales) printf("24 bit (0:8:8:8)\n");
        bmpw=3*hdr->width;
        while (bmpw&3) bmpw++;
        bmphdr.size=54+bmpw*hdr->height;
        bmphdr.ofsbmp=54;
        bmphdr.bpp=24;
        bmphdr.imsz=bmpw*hdr->height;
        fwrite(&bmphdr,sizeof(bmphdr),1,bmp);
        for (k=hdr->height-1;k>=0;k--) {
          fwrite(bmpptr+curoffs+3*k*hdr->width,1,bmpw,bmp);
        }
        curoffs+=3*hdr->width*hdr->height;
      }
      else
      if ((hdr->code&0x7f)==0x7E) { /* 15-bit bitmap */
        if (l==numscales) printf("16 bit (1:5:5:5)\n");

        for (k=0;k<256;k++) locpal[k]=0;
        locpal[0]=0xffffffff;
        fwrite(locpal,1,1024,alpha);
        for (k=hdr->height-1;k>=0;k--) {
          sbuf=(unsigned short *)(bmpptr+curoffs+2*k*hdr->width);
          for (j=0;j<hdr->width;j++) pad[j]=((sbuf[j]&0x8000)?0:1);
          fwrite(pad,1,bmpw,alpha);
        }
        fclose(alpha);
        
        bmpw=3*hdr->width;
        while (bmpw&3) bmpw++;
        bmphdr.size=54+bmpw*hdr->height;
        bmphdr.ofsbmp=54;
        bmphdr.bpp=24;
        bmphdr.imsz=bmpw*hdr->height;
        fwrite(&bmphdr,sizeof(bmphdr),1,bmp);
        for (k=hdr->height-1;k>=0;k--) {
          sbuf=(unsigned short *)(bmpptr+curoffs+2*k*hdr->width);
          for (j=0;j<hdr->width;j++) {
           pad[3*j]=(sbuf[j]&0x1f)<<3;
           pad[3*j+1]=((sbuf[j]>>5)&0x1f)<<3;
           pad[3*j+2]=((sbuf[j]>>10)&0x1f)<<3;
          }
          fwrite(pad,1,bmpw,bmp);
        }
        curoffs+=2*hdr->width*hdr->height;
      }
      else
      if ((hdr->code&0x7f)==0x78) { /* 15-bit bitmap */
        if (l==numscales) printf("16 bit (0:5:6:5)\n");
        bmpw=3*hdr->width;
        while (bmpw&3) bmpw++;
        bmphdr.size=54+bmpw*hdr->height;
        bmphdr.ofsbmp=54;
        bmphdr.bpp=24;
        bmphdr.imsz=bmpw*hdr->height;
        fwrite(&bmphdr,sizeof(bmphdr),1,bmp);
        for (k=hdr->height-1;k>=0;k--) {
          sbuf=(unsigned short *)(bmpptr+curoffs+2*k*hdr->width);
          for (j=0;j<hdr->width;j++) {
           pad[3*j]=(sbuf[j]&0x1f)<<3;
           pad[3*j+1]=((sbuf[j]>>5)&0x3f)<<2;
           pad[3*j+2]=((sbuf[j]>>11)&0x1f)<<3;
          }
          fwrite(pad,1,bmpw,bmp);
        }
        curoffs+=2*hdr->width*hdr->height;
      }
      else
      if ((hdr->code&0x7f)==0x6D) { /* 4x4=16-bit bitmap */
        if (l==numscales) printf("16 bit (4:4:4:4)\n");

        for (k=0;k<16;k++) locpal[k]=k*0x11111111;
        for (k=16;k<256;k++) locpal[k]=0;
        fwrite(locpal,1,1024,alpha);
        for (k=hdr->height-1;k>=0;k--) {
          ptr=bmpptr+curoffs+2*k*hdr->width;
          for (j=0;j<hdr->width;j++) pad[j]=ptr[2*j+1]>>4;
          fwrite(pad,1,bmpw,alpha);
        }
        fclose(alpha);
        
        bmpw=3*hdr->width;
        while (bmpw&3) bmpw++;
        bmphdr.size=54+bmpw*hdr->height;
        bmphdr.ofsbmp=54;
        bmphdr.bpp=24;
        bmphdr.imsz=bmpw*hdr->height;
        fwrite(&bmphdr,sizeof(bmphdr),1,bmp);
        for (k=hdr->height-1;k>=0;k--) {
          ptr=bmpptr+curoffs+2*k*hdr->width;
          for (j=0;j<hdr->width;j++) {
            pad[3*j]=0x11*(ptr[2*j]&15); 
            pad[3*j+1]=0x11*(ptr[2*j]>>4); 
            pad[3*j+2]=0x11*(ptr[2*j+1]&15);
          }
          fwrite(pad,1,bmpw,bmp);
        }
        curoffs+=2*hdr->width*hdr->height;
      }
      fclose(bmp);
      
      if (numscales) { /* multiscale loop */
        hdr->width/=2;
        hdr->height/=2;
        j=(curoffs-offs)&15;
        if (j) curoffs+=16-j; /* padding */
      }
      l--;
    }
    
      if (compressed) {
        free(bmpptr);
        if (hdr->code>>8) curoffs=offs+(hdr->code>>8);
        else curoffs=nxoffs;
      }
      
      /* now, look at the attachments if any */
      auxhdr=hdr; auxoffs=offs;
      if (nattach>0) printf("  (attached:");
      k=nattach;
      while (k>0) {
        k--;
        auxoffs+=(auxhdr->code>>8);
        if (curoffs<auxoffs) {
          fprintf(log,"!PAD %d ",auxoffs-curoffs);
          hexify(inbuf+curoffs,auxoffs-curoffs,pad);
          fprintf(log,"%s\n",pad);
        }
        auxhdr=(struct ENTRYHDR *)(inbuf+auxoffs);
        j=auxhdr->code&0xff;
        if (((hdr->code&0x7f)==0x7b)&&(auxhdr==palhdr)) {
          printf(" palette");
          fprintf(log,"PAL %02X %d %d {%d %d %d %d}\n",
             auxhdr->code&0xff,auxhdr->width,auxhdr->height,
             auxhdr->misc[0],auxhdr->misc[1],auxhdr->misc[2],auxhdr->misc[3]);
          if ((j==0x2D)||(j==0x29)) j=2;
          else if (j==0x2A) j=4;
          else j=3;
          curoffs=auxoffs+16+auxhdr->width*j;
        }
        else if ((auxhdr->code&0xff)==0x6F) {
          printf(" text");
          fprintf(log,"TXT %02X %d %d\n",
             auxhdr->code&0xff,auxhdr->width,auxhdr->height);
          quotify(inbuf+auxoffs+8,auxhdr->width,pad);
          fprintf(log,"%s\n",pad);
          curoffs=auxoffs+8+auxhdr->width;
        }
        else {
          printf(" data");
          j=auxhdr->code>>8;
          if (j==0) j=nxoffs-auxoffs;
          if (j>16384) { printf("Attached data too large !\n"); abort(); }
          fprintf(log,"BIN %02X %d\n",auxhdr->code&0xff,j);
          hexify(inbuf+auxoffs,j,pad);
          fprintf(log,"%s\n",pad);
          curoffs=auxoffs+j;
        }
      }
      if (nattach>0) printf(")\n");
      
      if (curoffs<nxoffs) {
        fprintf(log,"!PAD %d ",nxoffs-curoffs);
        hexify(inbuf+curoffs,nxoffs-curoffs,pad);
        fprintf(log,"%s\n",pad);
      }
      if (curoffs>nxoffs) {
        printf("WARNING: passed the next block ?\n");
        printf("Press Enter to continue.\n");
        getchar();
      }
    }
    else {
      /* not a bitmap */
      j=hdr->code&0xff;
      sprintf(pad,"%04d.BIN",i);
      printf("'%s'",pad);
      if (j==0x24) printf(" (24-bit palette)");
      if (j==0x22) printf(" (24-bit DOS palette)");
      if (j==0x2D) printf(" (16-bit palette)");
      if (j==0x29) printf(" (16-bit NFS5 palette)");
      if (j==0x2A) printf(" (32-bit palette)");
      printf("\n");
      fprintf(log,"%s\nBIN\n",pad);
      bmp=fopen(pad,"wb");
      if (bmp==NULL) { printf("Unable to create file.\n"); abort(); }
      fwrite(inbuf+curoffs,1,nxoffs-curoffs,bmp);
      fclose(bmp);
    }
  }
  
  fprintf(log,"#END\n");
  fclose(log);
}

char *next_lf(char *p)
{
  linecount++;
  while ((*p!='\n')&&(*p!=0)) { if (*p=='\r') *p=0; p++; }
  if (*p==0) { printf("Truncated INDEX.FSH\n"); abort(); }
  *p=0;
  p++; 
  if (*p=='\r') p++;
  return p;
}

void idxerr()
{
  printf("Format error in INDEX.FSH near line %d\n",linecount);
  abort();
}

void outmem()
{
  printf("FSH data buffer full. Increase BUFSZ parameter in INDEX.FSH\n");
  abort();
}

void map_file(char *name,unsigned char **ptr,int *size)
{
  FILE *f;
  f=fopen(name,"rb");
  if (f==NULL) { printf("Cannot open file '%s'\n",name); abort(); }
  fseek(f,0,SEEK_END);
  *size=ftell(f);
  rewind(f);
  *ptr=malloc(*size);
  if (*ptr==NULL) { printf("Out of memory.\n"); abort(); }
  if (fread(*ptr,1,*size,f)!=(size_t)*size)
    { printf("File read error.\n"); abort(); }
  fclose(f);
}

/* BMP to FSH conversion */

void bmp_get_file(char *bmpname,unsigned char code,int width,int height,
                  int *locpal,unsigned char **data,int *datalen)
{
  struct BMPHEAD *bmphdr;
  unsigned short *t16;
  unsigned char *pix,*bmp,*src,*tgt;
  int i,j,bmpsz,bmpw;
  
  /* sanity checks */
  map_file(bmpname,&bmp,&bmpsz);
  if ((bmp[0]!='B')||(bmp[1]!='M'))
    { printf("Not a BMP file !\n"); abort(); }
  bmphdr=(struct BMPHEAD *)(bmp+2);
  if ((bmphdr->planes!=1)||(bmphdr->compr))
    { printf("Unsupported BMP format !\n"); abort(); }
  if ((bmphdr->wid!=width)||(bmphdr->hei!=height))
    { printf("Incorrect geometry (%dx%d instead of %dx%d)\n",
        bmphdr->wid,bmphdr->hei,width,height);
      abort(); }
  pix=bmp+bmphdr->ofsbmp;
  
  /* 8-bit */
  if (code==0x7B) {
    *data=malloc(width*height+2048);
    *datalen=width*height;
    if (bmphdr->bpp!=8) { printf("Not a 8-bit BMP !\n"); abort(); }
    bmpw=width;
    while (bmpw&3) bmpw++;
    memcpy(locpal,bmp+54,1024);  
    for (i=0;i<height;i++)
      memcpy(*data+i*width,pix+(height-1-i)*bmpw,width);
  }
  else {
    if (bmphdr->bpp!=24) { printf("Not a 24-bit BMP !\n"); abort(); }
    bmpw=3*width;
    while (bmpw&3) bmpw++;
    if (code==0x7D) { /* 32-bit */
      *data=malloc(4*width*height+2048);
      *datalen=4*width*height;
      for (i=0;i<height;i++) {
        src=pix+(height-1-i)*bmpw;
        tgt=*data+i*4*width;
        for (j=0;j<width;j++) {
          tgt[4*j]=src[3*j]; tgt[4*j+1]=src[3*j+1]; tgt[4*j+2]=src[3*j+2];
        }
      }
    } else
    if (code==0x7F) { /* 24-bit */
      *data=malloc(3*width*height+2048);
      *datalen=3*width*height;
      for (i=0;i<height;i++) {
        src=pix+(height-1-i)*bmpw;
        tgt=*data+i*3*width;
        memcpy(tgt,src,3*width);
      }
    } else
    if (code==0x7E) { /* 16-bit 1:5:5:5 */
      *data=malloc(2*width*height+2048);
      *datalen=2*width*height;
      for (i=0;i<height;i++) {
        src=pix+(height-1-i)*bmpw;
        t16=(unsigned short *)(*data+i*2*width);
        for (j=0;j<width;j++)
          t16[j]=(src[3*j]>>3)+((src[3*j+1]>>3)<<5)+((src[3*j+2]>>3)<<10);
      }
    } else
    if (code==0x78) { /* 16-bit 0:5:6:5 */
      *data=malloc(2*width*height+2048);
      *datalen=2*width*height;
      for (i=0;i<height;i++) {
        src=pix+(height-1-i)*bmpw;
        t16=(unsigned short *)(*data+i*2*width);
        for (j=0;j<width;j++)
          t16[j]=(src[3*j]>>3)+((src[3*j+1]>>2)<<5)+((src[3*j+2]>>3)<<11);
      }
    } else
    if (code==0x6D) { /* 16-bit 4:4:4:4 */
      *data=malloc(2*width*height+2048);
      *datalen=2*width*height;
      for (i=0;i<height;i++) {
        src=pix+(height-1-i)*bmpw;
        t16=(unsigned short *)(*data+i*2*width);
        for (j=0;j<width;j++)
          t16[j]=(src[3*j]>>4)+((src[3*j+1]>>4)<<4)+((src[3*j+2]>>4)<<8);
      }
    }
    else { printf("Unknown FSH bitmap code !\n"); abort(); }
  }
  free(bmp);
}

void bmp_get_alpha(char *bmpname,unsigned char code,int width,int height,
                   unsigned char *data)
{
  struct BMPHEAD *bmphdr;
  unsigned int *pal;
  unsigned char *pix,*bmp,*src,*tgt;
  int i,j,bmpsz,bmpw,warn;
  
  /* sanity checks */
  map_file(bmpname,&bmp,&bmpsz);
  if ((bmp[0]!='B')||(bmp[1]!='M'))
    { printf("Not a BMP file !\n"); abort(); }
  bmphdr=(struct BMPHEAD *)(bmp+2);
  if ((bmphdr->planes!=1)||(bmphdr->compr))
    { printf("Unsupported BMP format !\n"); abort(); }
  if ((bmphdr->wid!=width)||(bmphdr->hei!=height))
    { printf("Incorrect geometry (%dx%d instead of %dx%d)\n",
        bmphdr->wid,bmphdr->hei,width,height);
      abort(); }
  if (bmphdr->bpp!=8) { printf("Alpha channel is not a 8-bit BMP !\n"); abort(); }
  
  pix=bmp+bmphdr->ofsbmp;
  pal=(unsigned int *)(bmp+54);
  warn=0;
  for (i=0;i<256;i++) 
    if ((pal[i]&0xffffff)%0x010101!=0) warn=1;
  if (warn) {
    printf("WARNING: color palette is not grayscale in alpha channel bitmap.\n");
    printf("         Keeping blue component.\n");
    printf("Press Enter to continue.\n");
    getchar();
  }
  bmpw=width;
  while (bmpw&3) bmpw++;
  
  if (code==0x7D) { /* 32 bit */
    for (i=0;i<height;i++) {
      src=pix+(height-1-i)*bmpw;
      tgt=data+i*4*width;
      for (j=0;j<width;j++)
        tgt[4*j+3]=pal[src[j]]&0xff;
    }
  }
  else
  if (code==0x7E) { /* 16 bit 1:5:5:5 */
    for (i=0;i<height;i++) {
      src=pix+(height-1-i)*bmpw;
      tgt=data+i*2*width;
      for (j=0;j<width;j++)
        tgt[2*j+1]+=pal[src[j]]&0x80;
    }
  }
  else
  if (code==0x6D) { /* 16 bit 4:4:4:4 */
    for (i=0;i<height;i++) {
      src=pix+(height-1-i)*bmpw;
      tgt=data+i*2*width;
      for (j=0;j<width;j++)
        tgt[2*j+1]+=pal[src[j]]&0xf0;
    }
  }
  else { printf("Unknown FSH data type.\n"); abort(); }
  
  free(bmp);
}

int make_nfs_pal(int code,int len,unsigned char *dest,unsigned int *src)
{
  int i;
  unsigned short *d;
  
  d=(unsigned short *)dest;
  if (code==0x24) {
    for (i=0;i<len;i++) {
      *(dest++)=((*src)>>16)&0xff;
      *(dest++)=((*src)>>8)&0xff;
      *(dest++)=(*src)&0xff;
      src++;
    }
    return 3*len;
  }
  if (code==0x22) {
    for (i=0;i<len;i++) {
      *(dest++)=((*src)>>18)&0x3f;
      *(dest++)=((*src)>>10)&0x3f;
      *(dest++)=((*src)>>2)&0x3f;
      src++;
    }
    return 3*len;
  }
  if (code==0x2D) {
    for (i=0;i<len;i++) {
      *(d++)=(((*src>>31)&0x01)<<15)+
             (((*src>>19)&0x1f)<<10)+
             (((*src>>11)&0x1f)<<5)+
             ((*src>>3)&0x1f);
      src++;
    }
    return 2*len;
  }
  if (code==0x29) {
    for (i=0;i<len;i++) {
      *(d++)=(((*src>>19)&0x1f)<<11)+
             (((*src>>10)&0x3f)<<5)+
             ((*src>>3)&0x1f);
      src++;
    }
    return 2*len;
  }
  if (code==0x2A) {
    memcpy(dest,src,4*len);
    return 4*len;
  }
  printf("Unknown palette type.\n");
  abort();
}


unsigned char *bmp_to_fsh()
{
  struct FSH_HDR *fshhdr,tmphdr;
  struct BMPDIR *dir;
  struct ENTRYHDR *hdr,*auxhdr;
  int tmp[6];

  unsigned char *outbuf,*outptr,*data,*p;
  char *log,*nxline,*ptr,*bmpname,*alphaname;
  int nbmp,i,j,datalen,allocsz,nattach,warn,scl;
  int hasglobpal,globpallen,locpallen;
  int globpal[256],locpal[256];
  char tmpfilename[1024];
  
  inbuf[inlen]=0; /* to be safe */
  linecount=0;
  /* header line */
  log=next_lf(inbuf);
  
  /* fsh file name */
  nxline=next_lf(log);
  fshname=strdup(log);
  log=nxline;
  
  /* compressed ? */
  if (!strncmp(log,"FSH",3)) iscompr=0;
  else if (!strncmp(log,"QFS",3)) iscompr=1;
  else idxerr();
  log=next_lf(log);
  
  /* shpi header */
  if (strncmp(log,"SHPI ",5)) idxerr();
  memcpy(tmphdr.SHPI,"SHPI",4);
  nbmp=strtol(log+5,&ptr,10);
  if (nbmp==0) idxerr();
  if (strncmp(ptr," objects, tag ",14)) idxerr();
  tmphdr.nbmp=nbmp;
  unquotify(ptr+14,4,tmphdr.dirId);
  
  /* buffer size */
  log=next_lf(log);
  if (strncmp(log,"BUFSZ ",6)) idxerr();
  allocsz=strtol(log+6,NULL,10);
  if (allocsz<40) idxerr();
  printf("Expecting %d objects, allocating %d bytes of memory\n",nbmp,allocsz);
  outbuf=(unsigned char *)malloc(allocsz);
  if (outbuf==NULL) { printf("Out of memory.\n"); abort(); }
  fshhdr=(struct FSH_HDR *)outbuf;
  memcpy(fshhdr,&tmphdr,16);
  dir=(struct BMPDIR *)(outbuf+16);
  fshlen=16+8*nbmp;
  outptr=outbuf+fshlen;
  
  /* global palette */
  log=next_lf(log);
  nxline=next_lf(log);
  if (!strncmp(log,"NOGLOBPAL",9)) hasglobpal=0;
  else {
    if (strncmp(log,"GLOBPAL ",8)) idxerr();
    hasglobpal=1;
    log+=8;
    map_file(log,&data,&datalen);
    makepal(data,&globpallen,globpal);
    free(data);
  }
  log=nxline;
  
  /* main loop */
  for (i=0;i<=nbmp;i++) {
    /* possible padding */
    if (!strncmp(log,"!PAD ",5)) {
      j=strtol(log+5,&ptr,10);
      if (j>0) {
        unhexify(ptr+1,j,outptr);
        outptr+=j;
        fshlen+=j;
      }
      log=next_lf(log);
    }
    
    /* end of list */
    if (!strncmp(log,"#END",4)) {
      if (i!=nbmp) { 
        printf("Insufficiently many objects listed (%d vs. %d)\n",i,nbmp);
        idxerr();
      }
      break;
    }
    if (i==nbmp) {
      printf("End of file not found after all %d objects were added\n",nbmp);
      idxerr();
    }
    
    /* object name & file name */
    unquotify(log,4,dir[i].name);
    dir[i].ofs=fshlen;
    while (*log!=' ') { if (*log=='\n') idxerr(); log++; }
    bmpname=log+1;
    log=next_lf(log);
    printf("Adding %s\n",bmpname);
    
    /* BIN case */
    if (!strncmp(log,"BIN",3)) {
      map_file(bmpname,&data,&datalen);
      if (fshlen+datalen>allocsz-2000) outmem();
      memcpy(outptr,data,datalen);
      fshlen+=datalen;
      outptr+=datalen;
      free(data);
      log=next_lf(log);
      continue; /* next object */
    }
    
    /* BMP case */
    if (!strncmp(log,"BMP ",4)) {
      hdr=(struct ENTRYHDR *)outptr;
      hdr->code=0;
      unhexify(log+4,1,outptr); /* code */
      log+=7;
      if (sscanf(log,"+%d %d %d {%d %d %d %d}",&nattach,tmp,tmp+1,
            tmp+2,tmp+3,tmp+4,tmp+5)!=7) idxerr();
      hdr->width=tmp[0];
      hdr->height=tmp[1];
      hdr->misc[0]=tmp[2];
      hdr->misc[1]=tmp[3];
      hdr->misc[2]=tmp[4];
      hdr->misc[3]=tmp[5];
      log=next_lf(log);

      /* do things */
      bmp_get_file(bmpname,*outptr&0x7F,
                    hdr->width,hdr->height,locpal,&data,&datalen);
      if (((*outptr&0x7f)==0x7D)||((*outptr&0x7f)==0x7E)||
          ((*outptr&0x7f)==0x6D)) {
        if (strncmp(log,"alpha ",6)) idxerr();
        alphaname=log+6;
        log=next_lf(log);
        bmp_get_alpha(alphaname,*outptr&0x7f,hdr->width,hdr->height,data);
      }
      outptr+=16;
      fshlen+=16;
      
      /* compress the bitmap if necessary */
      if (fshlen+datalen>allocsz-2000) outmem();
      if (hdr->code&0x80) { 
        compress_data(data,&datalen,outptr); 
        printf("\n");
      }
      else memcpy(outptr,data,datalen);
      outptr+=datalen;
      fshlen+=datalen;
      datalen+=16;
      auxhdr=hdr;
      locpallen=-1;

      /* attachments if any */
      while (nattach>0) {
        /* skip possible padding */
        if (!strncmp(log,"!PAD ",5)) {
          j=strtol(log+5,&ptr,10);
          if (j>0) {
            unhexify(ptr+1,j,outptr);
            outptr+=j;
            fshlen+=j;
            datalen+=j;
          }
          log=next_lf(log);
        }
        /* attach link */
        auxhdr->code=(auxhdr->code&0xff)+(datalen<<8);
        auxhdr=(struct ENTRYHDR *)outptr;
        
        if (!strncmp(log,"PAL ",4)) {
          /* palette */
          auxhdr->code=0;
          unhexify(log+4,1,outptr); /* code */
          log+=7;
          if (sscanf(log,"%d %d {%d %d %d %d}",tmp,tmp+1,
            tmp+2,tmp+3,tmp+4,tmp+5)!=6) idxerr();
          locpallen=auxhdr->width=tmp[0];
          auxhdr->height=tmp[1];
          auxhdr->misc[0]=tmp[2];
          auxhdr->misc[1]=tmp[3];
          auxhdr->misc[2]=tmp[4];
          auxhdr->misc[3]=tmp[5];
          log=next_lf(log);
          datalen=16+make_nfs_pal(auxhdr->code,auxhdr->width,outptr+16,locpal);
        } else if (!strncmp(log,"TXT ",4)) {
          /* text */
          auxhdr->code=0;
          unhexify(log+4,1,outptr); /* code */
          log+=7;
          if (sscanf(log,"%d %d",tmp,tmp+1)!=2) idxerr();
          auxhdr->width=tmp[0];
          auxhdr->height=tmp[1];
          log=next_lf(log);
          unquotify(log,auxhdr->width,outptr+8);
          log=next_lf(log);
          datalen=8+auxhdr->width;
        } else if (!strncmp(log,"BIN ",4)) {
          /* data */
          datalen=strtol(log+7,NULL,10);
          log=next_lf(log);
          unhexify(log,datalen,outptr); /* code */
          log=next_lf(log);
        } else idxerr();

        outptr+=datalen;
        fshlen+=datalen;
        nattach--;
      }

      /* palette check */
      if ((hdr->code&0x7f)==0x7B) {
        warn=0;
        if (locpallen>=0) {
          p=data;
          for (j=hdr->height*hdr->width;j>0;j--,p++)
            if (*p>=locpallen) warn=1;
        }
        else if (hasglobpal) {
          p=data;
          for (j=hdr->height*hdr->width;j>0;j--,p++)
            if (*p>=globpallen) warn=1;
        }
        if (warn) {
          printf("WARNING: bitmap contains more colors than its palette !!!\n");
          printf("Press Enter to continue.\n");
          getchar();
        }
        warn=0;
        if (hasglobpal&&(locpallen<0)) {
          for (j=0;j<globpallen;j++) 
            if ((locpal[j]&0xffffff)!=(globpal[j]&0xffffff)) warn=1;
        }
        if (warn) {
          printf("WARNING: bitmap palette differs from the global palette !!!\n");
          printf("Press Enter to continue.\n");
          getchar();
        }
      }
      free(data);
      
      continue; /* next object */
    }

    /* multiscale bitmaps */    
    if (!strncmp(log,"MBP ",4)) {
      hdr=(struct ENTRYHDR *)outptr;
      hdr->code=0;
      unhexify(log+4,1,outptr); /* code */
      log+=7;
      if (sscanf(log,"x%d %d %d {%d %d %d}",&scl,tmp,tmp+1,
            tmp+2,tmp+3,tmp+4)!=6) idxerr();
      hdr->width=tmp[0];
      hdr->height=tmp[1];
      hdr->misc[0]=tmp[2];
      hdr->misc[1]=tmp[3];
      hdr->misc[2]=tmp[4];
      hdr->misc[3]=scl<<12;
      if ((hdr->width!=(1<<scl))||(hdr->height!=(1<<scl)))
        { printf("Multiscale BMP has wrong size.\n"); idxerr(); }
      log=next_lf(log);
      if (((*outptr&0x7f)==0x7D)||((*outptr&0x7f)==0x7E)||
          ((*outptr&0x7f)==0x6D)) {
        if (strncmp(log,"alpha ",6)) idxerr();
        alphaname=log+6;
        log=next_lf(log);
      }

      outptr+=16;
      fshlen+=16;
      while (scl>=0) {
        sprintf(tmpfilename,bmpname,scl);
        bmp_get_file(tmpfilename,hdr->code&0xff,1<<scl,1<<scl,locpal,&data,&datalen);
        if (((hdr->code&0x7f)==0x7D)||((hdr->code&0x7f)==0x7E)||
            ((hdr->code&0x7f)==0x6D)) {
          sprintf(tmpfilename,alphaname,scl);
          bmp_get_alpha(tmpfilename,hdr->code&0xff,1<<scl,1<<scl,data);
        }
      
        if (hdr->code&0x80) { 
          printf("Compressed multiscale bitmaps not allowed\n");
          idxerr();
        }
        while (datalen&15) data[datalen++]=0;
        if (fshlen+datalen>allocsz-2000) outmem();
        memcpy(outptr,data,datalen);
        free(data);
        outptr+=datalen;
        fshlen+=datalen;
        scl--;
      }
      
      continue; /* next object */
    }
    
    idxerr(); /* unknown object */
  }
  
  fshhdr->filesize=fshlen;
  return outbuf;
}

int main(int argc,char **argv)
{
  FILE *f;
  char *outfn,*p;
  unsigned char *tmpbuf;
  int i,j,choice;
  
  printf("===========================================================================\n");
  printf("FSHTOOL version 1.00 - (c) Denis Auroux 2000 - auroux@math.polytechnique.fr\n");
  printf("===========================================================================\n");
  sanity_check();
  if (argc==1) {
    usage();
    abort();
  }
  
  /* try to open the given file and determine its type from the first few bytes */
  f=fopen(argv[1],"rb");
  if (f==NULL) {
    printf("Input file %s not found.\n",argv[1]);
    abort();
  }
  
  fseek(f,0,SEEK_END);
  inlen=ftell(f);
  rewind(f);
  if (inlen<4) { printf("Truncated file ?\n"); abort(); }
  inbuf=malloc(inlen+2048); /* safety margin */
  if (inbuf==NULL) { printf("Insufficient memory.\n"); abort(); }
  if (fread(inbuf,1,inlen,f)!=(size_t)inlen)
    { printf("File read error.\n"); abort(); }
  fclose(f);
  
  iscompr=0;
  if (((inbuf[0]&0xfe)==0x10)&&(inbuf[1]==0xfb)) {
    /* this is a compressed QFS file */
    iscompr=1;
    printf("Uncompressing QFS file (%d bytes) to memory...\n",inlen);
    tmpbuf=uncompress_data(inbuf,&inlen);
    free(inbuf);
    inbuf=tmpbuf;
  }
  
  if (!strncmp(inbuf,"SHPI",4)) {  
    /* this is a FSH file */
    printf("FSH data (%d bytes).\n",inlen);
    
    /* find a target directory name */
    if (argv[2]!=NULL)
      outfn=strdup(argv[2]);
    else {
      outfn=strdup(argv[1]);
      i=strlen(outfn)-4;
      if (outfn[i]!='.') {
        printf("Don't know how to derive a directory name from '%s'\n",outfn);
        printf("Please specify a second command-line argument.\n");
        abort();
      }
      outfn[i]=0;
    }
    rmdir(outfn);
    if (mkdir(outfn,0777)!=0) {
      printf("The directory '%s' is not empty or cannot be created.\n",outfn);
      printf("Choose one of the following and press Enter: \n");
      printf("  1. Overwrite files in that directory.\n");
      printf("  2. Derive an alternate directory name.\n");
      printf("  3. Abort execution <default>.\n");
      printf("\n(If the source file is located on a CD-ROM or read-only drive, please\n");
      printf("choose '3', move the file to your hard disk, and re-run FSHTOOL)\n");
      printf("\nYour choice [1-3]: ");
      fflush(stdout);
      choice=getchar();
      if ((choice!='1')&&(choice!='2')) {
        printf("Aborting.\n"); abort();
      }
      if (choice=='2') {
        printf("Deriving alternate directory names...\n");
        i=strlen(outfn);
        outfn[i]='.'; outfn[i+2]=0;
        for (j=1;j<=9;j++) { 
          outfn[i+1]='0'+j;
          rmdir(outfn);
          if (mkdir(outfn,0777)==0) break;
        }
        if (j==10) outfn[i]=0;
      }
    }
    if ((j==10)&&(choice=='2')) {
      printf("\nCould not create any of the derived directories.\n");
      printf("Target path might be located on a read-only disk.\n");
      if (argv[2]==NULL) printf("Please specify a second command-line argument.\n");
      abort();
    }
    printf("Unpacking to directory %s\n",outfn);
    if (chdir(outfn)!=0) { printf("Unable to access directory.\n"); abort(); }
    
    fsh_to_bmp(argv[1]);
  }

  else if (!strncmp(inbuf,"FSHT",4)) {
    /* this should be an INDEX.FSH */

    /* first obtain an output file if rel. to cwd */
    if (argv[2]!=NULL) {
      printf("Writing output to %s\n",argv[2]);
      f=fopen(argv[2],"wb");
      if (f==NULL) { printf("Unable to create file.\n"); abort(); }
    }

    outfn=strdup(argv[1]);
    p=outfn+strlen(outfn);
    while ((p!=outfn)&&(*p!='/')&&(*p!='\\')) p--;
    *p=0;
    if (*outfn)
      if (chdir(outfn)!=0)
        { printf("Unable to access directory.\n"); abort(); }
    free(outfn);
    
    tmpbuf=bmp_to_fsh();
    free(inbuf);
    inbuf=tmpbuf;
    printf("Successfully obtained %d bytes of FSH data\n",fshlen);

    if (iscompr) {
      printf("Compressing to QFS (each dot is 16K)\n");
      tmpbuf=malloc(fshlen+16);
      compress_data(inbuf,&fshlen,tmpbuf);
      free(inbuf);
      inbuf=tmpbuf;
      printf("\nGot %d bytes.\n",fshlen);
    }
    
    /* now obtain an output file if rel. to bmp dir */
    if (argv[2]==NULL) {
      printf("Writing to %s\n",fshname);
      chdir("..");
      f=fopen(fshname,"wb");
      if (f==NULL) { 
        printf("Cannot create output file.\n");
        p=fshname+strlen(fshname);
        while ((p!=fshname)&&(*p!='/')&&(*p!='\\')) p--;
        if (p!=fshname) p++;
        printf("Trying alternate name %s\n",p);
        f=fopen(p,"wb");
        if (f==NULL) { printf("Cannot create output file.\n"); abort(); }
      }
    }
    
    if (fwrite(inbuf,1,fshlen,f)!=(size_t)fshlen) { printf("File write error ?\n"); abort(); }
    fclose(f);
  }
  else { printf("Unknown file format.\n"); abort(); }

  printf("Conversion performed successfully.\n");
  printf("Press Enter to exit.\n"); getchar();
  return 1;
}
