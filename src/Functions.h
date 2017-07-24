//
//  Functions.h
//  FCE-To-OBJ
//
//  Created by Amrik Sadhra on 29/01/2015.
//  Copyright (c) 2015 Amrik Sadhra. All rights reserved.
//

#ifndef __FCE_To_OBJ__Functions__
#define __FCE_To_OBJ__Functions__

#include <stdio.h>

#endif /* defined(__FCE_To_OBJ__Functions__) */
int endian_swap(unsigned int x);
int readInt32( FILE *file );
unsigned int readInt32LE( FILE *file );
long HexToDec(char *InputHex);
void getVerts(int offset, int length);
void getTextCoords(int offset, int length);
void getNorms(int offset, int length);
void getTris(int offset, int length, int prevNumFaces);