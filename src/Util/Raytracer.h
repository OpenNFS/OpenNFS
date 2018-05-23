//
// Created by Amrik on 22/05/2018.
//

#ifndef OPENNFS3_RAYTRACER_H
#define OPENNFS3_RAYTRACER_H

#include "../nfs_data.h"
#define EPSILON 0.000001

FLOATPT VectorNormalize(FLOATPT nc);

FLOATPT NormalVectorCalc(FLOATPT a, FLOATPT b, FLOATPT c);

FLOATPT QuadNormalVectorCalc(FLOATPT a, FLOATPT b, FLOATPT c, FLOATPT d);

FLOATPT SumVector(FLOATPT Vect1, FLOATPT Vect2);

FLOATPT VertexNormal(int blk, int VertexIndex, TRKBLOCK *trk, POLYGONBLOCK *poly);

int intersect_triangle(double orig[3], double dir[3], double vert0[3], double vert1[3], double vert2[3], double *t, double *u, double *v) ;

void BlockShadingFixer(int blk, float theta, float rho, TRKBLOCK *trk, POLYGONBLOCK *poly, XOBJBLOCK *xobj);

void ObjectShadingFixer(int blk, float theta, float rho, TRKBLOCK *trk, POLYGONBLOCK *poly, XOBJBLOCK *xobj);

class Raytracer {
public:
    explicit Raytracer(float rho, float theta, int nBlocks, TRKBLOCK *trk, POLYGONBLOCK *poly, XOBJBLOCK *xobj);
};


#endif //OPENNFS3_RAYTRACER_H
