#pragma once

#include <iostream>
#include <cmath>
#include <assert.h>
#include "../Loaders/NFS3/NFS3Loader.h"

#define EPSILON 0.000001

NFS3_4_DATA::FLOATPT VectorNormalize(NFS3_4_DATA::FLOATPT nc);

NFS3_4_DATA::FLOATPT NormalVectorCalc(NFS3_4_DATA::FLOATPT a, NFS3_4_DATA::FLOATPT b, NFS3_4_DATA::FLOATPT c);

NFS3_4_DATA::FLOATPT QuadNormalVectorCalc(NFS3_4_DATA::FLOATPT a, NFS3_4_DATA::FLOATPT b, NFS3_4_DATA::FLOATPT c, NFS3_4_DATA::FLOATPT d);

NFS3_4_DATA::FLOATPT SumVector(NFS3_4_DATA::FLOATPT Vect1, NFS3_4_DATA::FLOATPT Vect2);

NFS3_4_DATA::FLOATPT VertexNormal(int blk, int VertexIndex, NFS3_4_DATA::TRKBLOCK *trk, NFS3_4_DATA::POLYGONBLOCK *poly);

int intersect_triangle(double orig[3], double dir[3], double vert0[3], double vert1[3], double vert2[3], double *t, double *u, double *v);

void BlockShadingFixer(int blk, float theta, float rho, NFS3_4_DATA::TRKBLOCK *trk, NFS3_4_DATA::POLYGONBLOCK *poly, NFS3_4_DATA::XOBJBLOCK *xobj);

void ObjectShadingFixer(int blk, float theta, float rho, NFS3_4_DATA::TRKBLOCK *trk, NFS3_4_DATA::POLYGONBLOCK *poly, NFS3_4_DATA::XOBJBLOCK *xobj);

class Raytracer {
public:
    explicit Raytracer(float rho, float theta, int nBlocks, NFS3_4_DATA::TRKBLOCK *trk, NFS3_4_DATA::POLYGONBLOCK *poly, NFS3_4_DATA::XOBJBLOCK *xobj);
};
