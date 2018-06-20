//
// Created by Amrik on 22/05/2018.
//

#pragma once

#include <iostream>
#include <cmath>
#include <assert.h>
#include "../Loaders/nfs3_loader.h"

#define EPSILON 0.000001

NFS3::FLOATPT VectorNormalize(NFS3::FLOATPT nc);

NFS3::FLOATPT NormalVectorCalc(NFS3::FLOATPT a, NFS3::FLOATPT b, NFS3::FLOATPT c);

NFS3::FLOATPT QuadNormalVectorCalc(NFS3::FLOATPT a, NFS3::FLOATPT b, NFS3::FLOATPT c, NFS3::FLOATPT d);

NFS3::FLOATPT SumVector(NFS3::FLOATPT Vect1, NFS3::FLOATPT Vect2);

NFS3::FLOATPT VertexNormal(int blk, int VertexIndex, NFS3::TRKBLOCK *trk, NFS3::POLYGONBLOCK *poly);

int intersect_triangle(double orig[3], double dir[3], double vert0[3], double vert1[3], double vert2[3], double *t, double *u, double *v) ;

void BlockShadingFixer(int blk, float theta, float rho, NFS3::TRKBLOCK *trk, NFS3::POLYGONBLOCK *poly, NFS3::XOBJBLOCK *xobj);

void ObjectShadingFixer(int blk, float theta, float rho, NFS3::TRKBLOCK *trk, NFS3::POLYGONBLOCK *poly, NFS3::XOBJBLOCK *xobj);

class Raytracer {
public:
    explicit Raytracer(float rho, float theta, int nBlocks, NFS3::TRKBLOCK *trk, NFS3::POLYGONBLOCK *poly, NFS3::XOBJBLOCK *xobj);
};
