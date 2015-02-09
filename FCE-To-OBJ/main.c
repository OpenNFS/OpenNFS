//
//  main.c
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 27/01/2015.
//  Copyright (c) 2015 Amrik Sadhra. All rights reserved.
//

#include "Functions.h"
#include <stdlib.h>
#define DATA_START 7940

FILE *fcefile, *outfile, *mtlfile;// file stream
char *fcefilename = "car.fce", *outfilename = "Model.obj", *mtlfilename = "textures.mtl";

int main(int argc, const char * argv[]) {
    //TODO: Get colours, store to .mtl file, map U/V coords.
    fcefile = fopen(fcefilename, "r");
    outfile = fopen(outfilename, "w");
    mtlfile = fopen(mtlfilename, "w");
    
    if(!fcefile)
    {
        printf( "Error while opening %s\n", fcefilename );
        exit(2);
    }
    /*Retrieve Vertices, normal, triangle offsets from DATA_START*/
    fseek(fcefile, 16, SEEK_SET);
    int vertOffset = DATA_START + readInt32LE(fcefile);
    
    fseek(fcefile, 20, SEEK_SET);
    int normOffset = DATA_START + readInt32LE(fcefile);
    
    fseek(fcefile, 24, SEEK_SET);
    int triOffset = DATA_START + readInt32LE(fcefile) + 4;
    
    /*Retrieve number of colours*/
    fseek(fcefile, 2044, SEEK_SET);
    int numPriColours = readInt32LE(fcefile);
    
    fseek(fcefile, 2304, SEEK_SET);
    int numSecColours = readInt32LE(fcefile);
    
    /*Retrieve number of parts*/
    fseek(fcefile, 248, SEEK_SET);
    int numParts = readInt32LE(fcefile);
    
    /*Get part names, store to numParts+1 x 64 array*/
    int counter = 0;
    int counterExt = 0;
    fseek(fcefile, 3588, SEEK_SET);
    char partNames[numParts + 1][64];
    for(counterExt = 0; counterExt <= numParts; counterExt++){
        for (counter = 0; counter <= 63; counter++){
            if(partNames[counterExt][counter] == '.'){break;}
            partNames[counterExt][counter] = fgetc(fcefile);
        }
    }
    
    /*Retrieve part by part data, Vert/Tri*/
    fseek(fcefile, 1276, SEEK_SET);//Get part vertex amount
    int partVertNumbers[numParts + 1];
    for(counter = 0; counter <= numParts; counter++){
        partVertNumbers[counter] = readInt32LE(fcefile);
    }
    fseek(fcefile, 1020, SEEK_SET);//Get part vert offsets
    int partVertOffsets[numParts + 1];
    for(counter = 0; counter <= numParts; counter++){
        partVertOffsets[counter] = readInt32LE(fcefile)*12;
    }
    fseek(fcefile, 1788, SEEK_SET);//Get part Tri amount
    int partTriNumbers[numParts + 1];
    for(counter = 0; counter <= numParts; counter++){
        partTriNumbers[counter] = readInt32LE(fcefile);
    }
    fseek(fcefile, 1532, SEEK_SET);//Get part tri offsets
    int partTriOffsets[numParts + 1];
    for(counter = 0; counter <= numParts; counter++){
        partTriOffsets[counter] = readInt32LE(fcefile)*56;
    }

    /*Write all part data to output obj file*/
    //fprintf(outfile, "%s", "mtllib textures.mtl\n");
    static int cumulativeVerts;
    for (counter = 0; counter <= numParts-1; counter++){
        cumulativeVerts += partVertNumbers[counter-1]+1;
        if(counter == 0){cumulativeVerts = 0;}
        fprintf(outfile, "%c", 'o');fprintf(outfile, "%c", ' ');
        fprintf(outfile, "%s", partNames[counter]); fprintf(outfile, "%c", '\n');
        //fprintf(outfile, "%s", "usemtl bodymaterial\n");
        getVerts(vertOffset + partVertOffsets[counter], partVertNumbers[counter]);
        getTextCoords(triOffset + partTriOffsets[counter] + 28, partTriNumbers[counter]);
        getNorms(normOffset + partVertOffsets[counter], partVertNumbers[counter]);
        getTris(triOffset + partTriOffsets[counter], partTriNumbers[counter], cumulativeVerts);
    }
    
    fclose(mtlfile);
    fclose(fcefile);
    fclose(outfile);
}
