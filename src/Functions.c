//
//  Functions.c
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 29/01/2015.
//  Copyright (c) 2015 Amrik Sadhra. All rights reserved.
//

#include "Functions.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

extern FILE *fcefile, *outfile, *mtlfile;  // file stream
extern char *fcefilename, *outfilename, *mtlfilename;

int endian_swap(unsigned int x)
{
    int swapped;
    
    swapped = (x>>24) |
    ((x<<8) & 0x00FF0000) |
    ((x>>8) & 0x0000FF00) |
    (x<<24);
    
    return swapped;
}

unsigned int readInt32LE( FILE *file )
{
    int data;
    
    data = fgetc( file );
    data = data << 8 | fgetc( file );
    data = data << 8 | fgetc( file );
    data = data << 8 | fgetc( file );
    
    return endian_swap(data);
}

void getVerts(int offset, int length){
    int counter = 0;
    /*Temporary holder for X, Y, Z Coordinates.*/
    char XCoord[10];
    char YCoord[10];
    char ZCoord[10];
    
    /*String to be printed to obj file*/
    char outString[100];
    
    /*X,Y,Z raw coords stored into float buffer by fread*/
    float buffer[3];
    
    /*Buffer to store global coordinates. All X,Y,Z coords currently local. To Global, + global coordinates to local*/
    float globalBuffer[3];
    
    /*HACK: Needed so that can Identify new part. If new part, get new Global coordinates*/
    static int globalIndex;
    static int previousOffset;

    if (previousOffset != offset){globalIndex+=12;}
    fseek(fcefile, 252+globalIndex, SEEK_SET);//retrieve global coords
    fread(globalBuffer, 4,3,fcefile);
    previousOffset = offset;
    
    fseek(fcefile, offset, SEEK_SET);
    /*Read Verts in*/
    for(counter = 0; counter <= length; counter++){
        fread(buffer, 4, 3,fcefile);
        
        sprintf(XCoord, "%f", buffer[0]+globalBuffer[0]);
        sprintf(YCoord, "%f", buffer[1]+globalBuffer[1]);
        sprintf(ZCoord, "%f", buffer[2]+globalBuffer[2]);
        
        strcpy(outString, "v");
        strcat(outString, " ");
        strcat(outString, XCoord);
        strcat(outString, " ");
        strcat(outString, YCoord);
        strcat(outString, " ");
        strcat(outString, ZCoord);
        strcat(outString, "\n");
        
        fprintf(outfile, "%s", outString);
    }
}

void getTextCoords(int offset, int length){
    float texBuffer[6];
    int counter = 0;
    char V1U[20];
    char V2U[20];
    char V3U[20];
    char V1V[20];
    char V2V[20];
    char V3V[20];
    char outString[200];
    
    fseek(fcefile, offset, SEEK_SET);
    /*Read Triangles in*/
    for(counter = 0; counter <= length; counter++){
        fread(texBuffer, 4, 6, fcefile);
        
        sprintf(V1U, "%f", texBuffer[0]);
        sprintf(V1V, "%f", texBuffer[3]);
        sprintf(V2U, "%f", texBuffer[1]);
        sprintf(V2V, "%f", texBuffer[4]);
        sprintf(V3U, "%f", texBuffer[2]);
        sprintf(V3V, "%f", texBuffer[5]);
        
        strcpy(outString, "vt");strcat(outString, " ");strcat(outString, V1U);strcat(outString, " ");
        strcat(outString, V1V);strcat(outString, "\n");
        fprintf(outfile, "%s", outString);
        strcpy(outString, "vt");strcat(outString, " ");strcat(outString, V2U);strcat(outString, " ");
        strcat(outString, V2V);strcat(outString, "\n");
        fprintf(outfile, "%s", outString);
        strcpy(outString, "vt");strcat(outString, " ");strcat(outString, V3U);strcat(outString, " ");
        strcat(outString, V3V);strcat(outString, "\n");
        fprintf(outfile, "%s", outString);
        fseek(fcefile, offset + 56*counter, SEEK_SET);
    }
}

void getNorms(int offset, int length){
    int counter = 0;
    char XVect[10];
    char YVect[10];
    char ZVect[10];
    char outString[100];
    float normalLength = 0;
    float buffer[3];
    
    fseek(fcefile, offset, SEEK_SET);
    /*Read Normals in, and normalize dem normals!*/
    for(counter = 0; counter <= length; counter++){
        fread(buffer, 4,3,fcefile);
        normalLength = sqrt(pow(buffer[0],2)+pow(buffer[1],2)+pow(buffer[2],2));
        sprintf(XVect, "%f", buffer[0]/normalLength);
        sprintf(YVect, "%f", buffer[1]/normalLength);
        sprintf(ZVect, "%f", buffer[2]/normalLength);
        
        strcpy(outString, "vn");strcat(outString, " ");strcat(outString, XVect);strcat(outString, " ");
        strcat(outString, YVect);strcat(outString, " ");strcat(outString, ZVect);strcat(outString, "\n");
        
        fprintf(outfile, "%s", outString);
    }
}

void getTris(int offset, int length, int prevNumFaces){
    int indexBuffer[3];
    int counter = 0;
    char XIndex[20];
    char YIndex[20];
    char ZIndex[20];
    char outString[100];
    
    fseek(fcefile, offset, SEEK_SET);
    /*Read Triangles in*/
    for(counter = 0; counter <= length; counter++){
        indexBuffer[0] = readInt32LE(fcefile)+1;
        indexBuffer[1] = readInt32LE(fcefile)+1;
        indexBuffer[2] = readInt32LE(fcefile)+1;
        sprintf(XIndex, "%d", indexBuffer[0]+prevNumFaces);
        sprintf(YIndex, "%d", indexBuffer[1]+prevNumFaces);
        sprintf(ZIndex, "%d", indexBuffer[2]+prevNumFaces);
        
        strcpy(outString, "f");strcat(outString, " ");strcat(outString, XIndex);strcat(outString, " ");
        strcat(outString, YIndex);strcat(outString, " ");strcat(outString, ZIndex);strcat(outString, "\n");
        if (counter != 0) fprintf(outfile, "%s", outString);
        fseek(fcefile, offset + 56*counter, SEEK_SET);
    }
}