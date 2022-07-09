#include "Raytracer.h"

Raytracer::Raytracer(float rho, float theta, int nBlocks, TRKBLOCK *trk, POLYGONBLOCK *poly, XOBJBLOCK *xobj) {
    int startbl = 0;
    int endbl   = nBlocks;

    for (int32_t curBlock = startbl; curBlock < endbl; curBlock++) // Shadow raytracing for selected Blocks
    {
        std::cout << "RayTracing Shadows (Block  " << curBlock << ")"
                  << "... (" << curBlock - startbl << " of " << endbl - startbl << " Blocks)" << std::endl;
        BlockShadingFixer(curBlock, theta, rho, trk, poly, xobj);
        ObjectShadingFixer(curBlock, theta, rho, trk, poly, xobj);
    }
}

FLOATPT VectorNormalize(FLOATPT nc) {
    float length = sqrt((nc.x * nc.x) + (nc.y * nc.y) + (nc.z * nc.z));
    nc.x         = nc.x / length;
    nc.y         = nc.y / length;
    nc.z         = nc.z / length;
    return nc;
}

FLOATPT NormalVectorCalc(FLOATPT a, FLOATPT b, FLOATPT c) {
    FLOATPT v1, v2, out;

    v1.x = c.x - a.x;
    v1.y = c.y - a.y;
    v1.z = c.z - a.z;

    v2.x = b.x - a.x;
    v2.y = b.y - a.y;
    v2.z = b.z - a.z;

    out.x = (v1.y * v2.z) - (v1.z * v2.y);
    out.y = (v1.z * v2.x) - (v1.x * v2.z);
    out.z = (v1.x * v2.y) - (v1.y * v2.x);

    return VectorNormalize(out);
}

FLOATPT QuadNormalVectorCalc(FLOATPT a, FLOATPT b, FLOATPT c, FLOATPT d) {
    struct FLOATPT n1, n2, nc;
    n1 = NormalVectorCalc(a, b, c);
    n2 = NormalVectorCalc(a, c, d);
    nc = SumVector(n1, n2);

    return nc;
}

FLOATPT SumVector(FLOATPT Vect1, FLOATPT Vect2) {
    struct FLOATPT SumVect;
    SumVect.x = Vect1.x + Vect2.x;
    SumVect.y = Vect1.y + Vect2.y;
    SumVect.z = Vect1.z + Vect2.z;
    SumVect   = VectorNormalize(SumVect);

    return SumVect;
}

FLOATPT VertexNormal(int blk, int VertexIndex, TRKBLOCK *trk, POLYGONBLOCK *poly) {
    FLOATPT a, b, c, d;
    FLOATPT normal;
    LPPOLYGONDATA p;
    FLOATPT *v;

    normal.x = 0;
    normal.y = 0;
    normal.z = 0;

    v       = trk[blk].vert;
    p       = poly[blk].poly[4];
    int num = poly[blk].sz[4];
    for (int32_t j = 0; j < num; j++, p++) {
        for (uint32_t k = 0; k < 4; k++) {
            if (p->vertex[k] == VertexIndex) {
                a      = v[p->vertex[0]];
                b      = v[p->vertex[1]];
                c      = v[p->vertex[2]];
                d      = v[p->vertex[3]];
                normal = SumVector(normal, QuadNormalVectorCalc(a, b, c, d));
            }
        }
    }
    return normal;
}

int intersect_triangle(double orig[3], double dir[3], double vert0[3], double vert1[3], double vert2[3], double *t, double *u, double *v) {
    double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
    double det, inv_det;

#define CROSS(dest, v1, v2)                  \
    dest[0] = v1[1] * v2[2] - v1[2] * v2[1]; \
    dest[1] = v1[2] * v2[0] - v1[0] * v2[2]; \
    dest[2] = v1[0] * v2[1] - v1[1] * v2[0];
#define DOT(v1, v2) (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2])

#define SUB(dest, v1, v2)    \
    dest[0] = v1[0] - v2[0]; \
    dest[1] = v1[1] - v2[1]; \
    dest[2] = v1[2] - v2[2];

    /* find vectors for two edges sharing vert0 */
    SUB(edge1, vert1, vert0);
    SUB(edge2, vert2, vert0);

    /* begin calculating determinant - also used to calculate U parameter */
    CROSS(pvec, dir, edge2);

    /* if determinant is near zero, ray lies in plane of triangle */
    det = DOT(edge1, pvec);

    /* calculate distance from vert0 to ray origin */
    SUB(tvec, orig, vert0);
    inv_det = 1.0 / det;

    CROSS(qvec, tvec, edge1);

    if (det > EPSILON) {
        *u = DOT(tvec, pvec);
        if (*u < 0.0 || *u > det)
            return 0;

        /* calculate V parameter and test bounds */
        *v = DOT(dir, qvec);
        if (*v < 0.0 || *u + *v > det)
            return 0;
    }
    /*else if(det < -EPSILON)
    {
       // calculate U parameter and test bounds
       *u = DOT(tvec, pvec);
       if (*u > 0.0 || *u < det)
      return 0;

       // calculate V parameter and test bounds
       *v = DOT(dir, qvec) ;
       if (*v > 0.0 || *u + *v < det)
      return 0;
    }*/
    else
        return 0; /* ray is parallell to the plane of the triangle */

    *t = DOT(edge2, qvec) * inv_det;
    (*u) *= inv_det;
    (*v) *= inv_det;

    return 1;
}

void BlockShadingFixer(int blk, float theta, float rho, TRKBLOCK *trk, POLYGONBLOCK *poly, XOBJBLOCK *xobj) {
    int i, j, j2, k, num, num2, blk2;
    int result;
    double uu, vv, tt;
    double corner1[3], corner2[3], corner3[3];
    double Pos[3], RayDir[3], StartPos[3];

    // long nGPolygons;
    LPPOLYGONDATA p, p2;
    struct TRKBLOCK *t;
    struct FLOATPT *v, *v2;
    unsigned long *sv;

    t  = &trk[blk];
    v  = trk[blk].vert;
    sv = reinterpret_cast<unsigned long *>(trk[blk].unknVertices);

    double PI         = 3.14159265359;
    double PitchAngle = (double) theta; // 0.00;//4.712; //0.44; //Theta
    double RhoAngle   = (double) rho;   // 0.165; //0.13; //RHO
    // to get real Yaw from Rho, it has to be multiplied with 4.(*WRONG*)
    // Rho is given as fixed point. (1.0 is 2*pi in radian scale.)
    double YawAngle = (2 * PI) * RhoAngle;

    // to get real Pitch from Theta, it needs +pi
    // PitchAngle=PitchAngle+pi;

    // double r;
    struct FLOATPT VNormal;
    VNormal.x = 0;
    VNormal.y = 0;
    VNormal.z = (float) 0.05;
    /*
    x=rsin(phi)cos(theta), y=rsin(phi)sin(theta), z=rcos(phi), r=sqrt(x*x+y*y+z*z)...
    */

    p2   = poly[blk].poly[4];
    num2 = poly[blk].sz[4];
    for (j2 = 0; j2 < num2; j2++, p2++) {
        for (k = 0; k < 4; k++) {
            // bool tested = new bool[t->nVertices];
            /*for (uint32_t temp=0; temp>trk[blk].nVertices; temp++)
                tested[temp]=false;*/

            assert(p2->vertex[k] >= 0 && p2->vertex[k] < t->nVertices);

            if ((p2->vertex[k] < 0) || (p2->vertex[k] > t->nVertices)) {
                std::cout << "FAILED! K: " << k << ", J2:" << j2 << ", BLK:" << blk << std::endl;
                return;
            }

            VNormal     = VertexNormal(blk, p2->vertex[k], trk, poly);
            StartPos[0] = (double) v[p2->vertex[k]].x + (VNormal.x / 5);
            StartPos[1] = (double) v[p2->vertex[k]].y + (VNormal.y / 5);
            StartPos[2] = (double) v[p2->vertex[k]].z + (VNormal.z / 5);

            RayDir[0] = sin(YawAngle) * cos(PitchAngle);
            RayDir[1] = sin(YawAngle) * sin(PitchAngle);
            RayDir[2] = cos(YawAngle);

            // used for known light point
            /*EndPos[0]=372;
            EndPos[1]=-790;
            EndPos[2]=10;

            RayDir[0]=(EndPos[0]-StartPos[0]);
            RayDir[1]=(EndPos[1]-StartPos[1]);
            RayDir[2]=(EndPos[2]-StartPos[2]);
            r=sqrt((RayDir[0]*RayDir[0])+(RayDir[1]*RayDir[1])+(RayDir[2]*RayDir[2]));

            RayDir[0]=RayDir[0]/r;
            RayDir[1]=RayDir[1]/r;
            RayDir[2]=RayDir[2]/r;*/

            result = 0;
            i      = 0;

            while ((t->nbdData[i].blk != -1) && (i < 300)) {
                assert(i >= 0 && i < 300);

                blk2 = t->nbdData[i].blk;

                assert(blk2 >= 0 && blk2 < 300);

                p   = poly[blk2].poly[4];
                num = poly[blk2].sz[4];
                v2  = trk[blk2].vert;
                // Track Polygons
                //--------------

                for (j = 0; j < num; j++, p++) {
                    if (blk == 0 && j2 == 0 && k == 1) {
                        std::cout << "keke" << std::endl;
                    }

                    // p->texture
                    if (result != 1) {
                        assert(p->vertex[0] >= 0 && p->vertex[0] < trk[blk2].nVertices);
                        corner1[0] = (double) v2[p->vertex[0]].x;
                        corner1[2] = (double) v2[p->vertex[0]].z;
                        corner1[1] = (double) v2[p->vertex[0]].y;

                        assert(p->vertex[1] >= 0 && p->vertex[1] < trk[blk2].nVertices);
                        corner2[0] = (double) v2[p->vertex[3]].x;
                        corner2[2] = (double) v2[p->vertex[3]].z;
                        corner2[1] = (double) v2[p->vertex[3]].y;

                        assert(p->vertex[2] >= 0 && p->vertex[2] < trk[blk2].nVertices);
                        corner3[0] = (double) v2[p->vertex[2]].x;
                        corner3[2] = (double) v2[p->vertex[2]].z;
                        corner3[1] = (double) v2[p->vertex[2]].y;
                        // run first Ray test, if false, then next quad.
                        Pos[0] = StartPos[0];
                        Pos[1] = StartPos[1];
                        Pos[2] = StartPos[2];
                        result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                    } else {
                        j = num;
                        i = 300;
                    }

                    if (result != 1) {
                        corner1[0] = (double) v2[p->vertex[0]].x;
                        corner1[2] = (double) v2[p->vertex[0]].z;
                        corner1[1] = (double) v2[p->vertex[0]].y;

                        corner2[0] = (double) v2[p->vertex[2]].x;
                        corner2[2] = (double) v2[p->vertex[2]].z;
                        corner2[1] = (double) v2[p->vertex[2]].y;

                        corner3[0] = (double) v2[p->vertex[1]].x;
                        corner3[2] = (double) v2[p->vertex[1]].z;
                        corner3[1] = (double) v2[p->vertex[1]].y;
                        // check the second half of the quad

                        Pos[0] = StartPos[0];
                        Pos[1] = StartPos[1];
                        Pos[2] = StartPos[2];
                        result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                    } else {
                        j = num;
                        i = 300;
                    }

                    if (result == 1) {
                        std::cout << "COLLISION! (BLK/POLY/Node) " << blk << "/" << j2 << "/" << k << " Collided with (BLK/Poly) " << blk2 << "/" << j << std::endl;
                    }
                }
                // TrackPolygons End
                // lane Polygons
                //-------------
                // if (result!=1) {
                num = poly[blk2].sz[5];
                p   = poly[blk2].poly[5];
                for (j = 0; j < num; j++, p++) {
                    if (blk == 0 && j2 == 0 && k == 1) {
                        std::cout << "keke" << std::endl;
                    }

                    // p->texture
                    if (result != 1) {
                        assert(p->vertex[0] >= 0 && p->vertex[0] < trk[blk2].nVertices);
                        corner1[0] = (double) v2[p->vertex[0]].x;
                        corner1[2] = (double) v2[p->vertex[0]].z;
                        corner1[1] = (double) v2[p->vertex[0]].y;

                        assert(p->vertex[1] >= 0 && p->vertex[1] < trk[blk2].nVertices);
                        corner2[0] = (double) v2[p->vertex[3]].x;
                        corner2[2] = (double) v2[p->vertex[3]].z;
                        corner2[1] = (double) v2[p->vertex[3]].y;

                        assert(p->vertex[2] >= 0 && p->vertex[2] < trk[blk2].nVertices);
                        corner3[0] = (double) v2[p->vertex[2]].x;
                        corner3[2] = (double) v2[p->vertex[2]].z;
                        corner3[1] = (double) v2[p->vertex[2]].y;
                        // run first Ray test, if false, then next quad.
                        Pos[0] = StartPos[0];
                        Pos[1] = StartPos[1];
                        Pos[2] = StartPos[2];
                        result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                    } else {
                        // j=num;
                        i = 300;
                    }

                    if (result != 1) {
                        corner1[0] = (double) v2[p->vertex[0]].x;
                        corner1[2] = (double) v2[p->vertex[0]].z;
                        corner1[1] = (double) v2[p->vertex[0]].y;

                        corner2[0] = (double) v2[p->vertex[2]].x;
                        corner2[2] = (double) v2[p->vertex[2]].z;
                        corner2[1] = (double) v2[p->vertex[2]].y;

                        corner3[0] = (double) v2[p->vertex[1]].x;
                        corner3[2] = (double) v2[p->vertex[1]].z;
                        corner3[1] = (double) v2[p->vertex[1]].y;
                        // check the second half of the quad

                        Pos[0] = StartPos[0];
                        Pos[1] = StartPos[1];
                        Pos[2] = StartPos[2];
                        result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                    } else {
                        // j=num;
                        i = 300;
                    }

                    if (result == 1) {
                        std::cout << "COLLISION! (BLK/POLY/Node) " << blk << "/" << j2 << "/" << k << " Collided with (BLK/Poly) " << blk2 << "/" << j << std::endl;
                    }
                }
                //}
                // Lane Polygons End
                // Blue Objects
                //------------
                for (long chunkcounter = 0; chunkcounter < 4; chunkcounter++) {
                    long numobj = poly[blk2].obj[chunkcounter].nobj;
                    for (long objcounter = 0; objcounter < numobj; objcounter++) {
                        num = poly[blk2].obj[chunkcounter].numpoly[objcounter];

                        p = poly[blk2].obj[chunkcounter].poly[objcounter];
                        for (j = 0; j < num; j++, p++) {
                            if (blk == 0 && j2 == 0 && k == 1) {
                                std::cout << "keke" << std::endl;
                            }

                            // p->texture
                            if (result != 1) {
                                assert(p->vertex[0] >= 0 && p->vertex[0] < trk[blk2].nVertices);
                                corner1[0] = (double) v2[p->vertex[2]].x;
                                corner1[2] = (double) v2[p->vertex[2]].z;
                                corner1[1] = (double) v2[p->vertex[2]].y;

                                assert(p->vertex[1] >= 0 && p->vertex[1] < trk[blk2].nVertices);
                                corner2[0] = (double) v2[p->vertex[3]].x;
                                corner2[2] = (double) v2[p->vertex[3]].z;
                                corner2[1] = (double) v2[p->vertex[3]].y;

                                assert(p->vertex[2] >= 0 && p->vertex[2] < trk[blk2].nVertices);
                                corner3[0] = (double) v2[p->vertex[0]].x;
                                corner3[2] = (double) v2[p->vertex[0]].z;
                                corner3[1] = (double) v2[p->vertex[0]].y;
                                // run first Ray test, if false, then next quad.
                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                // j=num;
                                i = 300;
                            }

                            if (result != 1) {
                                corner1[0] = (double) v2[p->vertex[1]].x;
                                corner1[2] = (double) v2[p->vertex[1]].z;
                                corner1[1] = (double) v2[p->vertex[1]].y;

                                corner2[0] = (double) v2[p->vertex[2]].x;
                                corner2[2] = (double) v2[p->vertex[2]].z;
                                corner2[1] = (double) v2[p->vertex[2]].y;

                                corner3[0] = (double) v2[p->vertex[0]].x;
                                corner3[2] = (double) v2[p->vertex[0]].z;
                                corner3[1] = (double) v2[p->vertex[0]].y;
                                // check the second half of the quad

                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                // j=num;
                                i = 300;
                            }
                            // Make same with another side:
                            if (result != 1) {
                                assert(p->vertex[0] >= 0 && p->vertex[0] < trk[blk2].nVertices);
                                corner1[0] = (double) v2[p->vertex[0]].x;
                                corner1[2] = (double) v2[p->vertex[0]].z;
                                corner1[1] = (double) v2[p->vertex[0]].y;

                                assert(p->vertex[1] >= 0 && p->vertex[1] < trk[blk2].nVertices);
                                corner2[0] = (double) v2[p->vertex[3]].x;
                                corner2[2] = (double) v2[p->vertex[3]].z;
                                corner2[1] = (double) v2[p->vertex[3]].y;

                                assert(p->vertex[2] >= 0 && p->vertex[2] < trk[blk2].nVertices);
                                corner3[0] = (double) v2[p->vertex[2]].x;
                                corner3[2] = (double) v2[p->vertex[2]].z;
                                corner3[1] = (double) v2[p->vertex[2]].y;
                                // run first Ray test, if false, then next quad.
                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                // j=num;
                                i = 300;
                            }

                            if (result != 1) {
                                corner1[0] = (double) v2[p->vertex[0]].x;
                                corner1[2] = (double) v2[p->vertex[0]].z;
                                corner1[1] = (double) v2[p->vertex[0]].y;

                                corner2[0] = (double) v2[p->vertex[2]].x;
                                corner2[2] = (double) v2[p->vertex[2]].z;
                                corner2[1] = (double) v2[p->vertex[2]].y;

                                corner3[0] = (double) v2[p->vertex[1]].x;
                                corner3[2] = (double) v2[p->vertex[1]].z;
                                corner3[1] = (double) v2[p->vertex[1]].y;
                                // check the second half of the quad

                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                // j=num;
                                i = 300;
                            }

                            if (result == 1) {
                                std::cout << "COLLISION! (BLK/POLY/Node) " << blk << "/" << j2 << "/" << k << " Collided with (BLK/Poly) " << blk2 << "/" << j << std::endl;
                            }
                        }
                    }
                }

                //}
                // Blue Polygons End.
                // Xtra Object Start

                for (uint32_t XObjCounter = 0; XObjCounter < xobj[blk2].nobj; XObjCounter++) {
                    FLOATPT *vert_array = xobj[blk2].obj[XObjCounter].vert;
                    if (xobj[blk2].obj[XObjCounter].crosstype != 6) {
                        for (uint32_t XObjPoly = 0; XObjPoly < xobj[blk2].obj[XObjCounter].nPolygons; XObjPoly++) {
                            POLYGONDATA &quad  = xobj[blk2].obj[XObjCounter].polyData[XObjPoly];
                            FLOATPT &ref_point = xobj[blk2].obj[XObjCounter].ptRef;

                            if (result != 1) {
                                if (xobj[blk2].obj[XObjCounter].crosstype != 1) {
                                    corner1[0] = (double) vert_array[quad.vertex[0]].x + ref_point.x;
                                    corner1[2] = (double) vert_array[quad.vertex[0]].z + ref_point.z;
                                    corner1[1] = (double) vert_array[quad.vertex[0]].y + ref_point.y;

                                    corner2[0] = (double) vert_array[quad.vertex[3]].x + ref_point.x;
                                    corner2[2] = (double) vert_array[quad.vertex[3]].z + ref_point.z;
                                    corner2[1] = (double) vert_array[quad.vertex[3]].y + ref_point.y;

                                    corner3[0] = (double) vert_array[quad.vertex[2]].x + ref_point.x;
                                    corner3[2] = (double) vert_array[quad.vertex[2]].z + ref_point.z;
                                    corner3[1] = (double) vert_array[quad.vertex[2]].y + ref_point.y;
                                } else {
                                    corner1[0] = (double) vert_array[quad.vertex[0]].x;
                                    corner1[2] = (double) vert_array[quad.vertex[0]].z;
                                    corner1[1] = (double) vert_array[quad.vertex[0]].y;

                                    corner2[0] = (double) vert_array[quad.vertex[3]].x;
                                    corner2[2] = (double) vert_array[quad.vertex[3]].z;
                                    corner2[1] = (double) vert_array[quad.vertex[3]].y;

                                    corner3[0] = (double) vert_array[quad.vertex[2]].x;
                                    corner3[2] = (double) vert_array[quad.vertex[2]].z;
                                    corner3[1] = (double) vert_array[quad.vertex[2]].y;
                                }
                                // run first Ray test, if false, then next quad.
                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                // j=num;
                                i = 300;
                            }

                            if (result != 1) {
                                if (xobj[blk2].obj[XObjCounter].crosstype != 1) {
                                    corner1[0] = (double) vert_array[quad.vertex[0]].x + ref_point.x;
                                    corner1[2] = (double) vert_array[quad.vertex[0]].z + ref_point.z;
                                    corner1[1] = (double) vert_array[quad.vertex[0]].y + ref_point.y;

                                    corner2[0] = (double) vert_array[quad.vertex[2]].x + ref_point.x;
                                    corner2[2] = (double) vert_array[quad.vertex[2]].z + ref_point.z;
                                    corner2[1] = (double) vert_array[quad.vertex[2]].y + ref_point.y;

                                    corner3[0] = (double) vert_array[quad.vertex[1]].x + ref_point.x;
                                    corner3[2] = (double) vert_array[quad.vertex[1]].z + ref_point.z;
                                    corner3[1] = (double) vert_array[quad.vertex[1]].y + ref_point.y;
                                } else {
                                    corner1[0] = (double) vert_array[quad.vertex[0]].x;
                                    corner1[2] = (double) vert_array[quad.vertex[0]].z;
                                    corner1[1] = (double) vert_array[quad.vertex[0]].y;

                                    corner2[0] = (double) vert_array[quad.vertex[2]].x;
                                    corner2[2] = (double) vert_array[quad.vertex[2]].z;
                                    corner2[1] = (double) vert_array[quad.vertex[2]].y;

                                    corner3[0] = (double) vert_array[quad.vertex[1]].x;
                                    corner3[2] = (double) vert_array[quad.vertex[1]].z;
                                    corner3[1] = (double) vert_array[quad.vertex[1]].y;
                                }
                                // run first Ray test, if false, then next quad.
                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                // j=num;
                                i = 300;
                            }
                            // usually Dualsided, so another look with reversed vertex order:
                            if (result != 1) {
                                if (xobj[blk2].obj[XObjCounter].crosstype != 1) {
                                    corner1[0] = (double) vert_array[quad.vertex[2]].x + ref_point.x;
                                    corner1[2] = (double) vert_array[quad.vertex[2]].z + ref_point.z;
                                    corner1[1] = (double) vert_array[quad.vertex[2]].y + ref_point.y;

                                    corner2[0] = (double) vert_array[quad.vertex[3]].x + ref_point.x;
                                    corner2[2] = (double) vert_array[quad.vertex[3]].z + ref_point.z;
                                    corner2[1] = (double) vert_array[quad.vertex[3]].y + ref_point.y;

                                    corner3[0] = (double) vert_array[quad.vertex[0]].x + ref_point.x;
                                    corner3[2] = (double) vert_array[quad.vertex[0]].z + ref_point.z;
                                    corner3[1] = (double) vert_array[quad.vertex[0]].y + ref_point.y;
                                } else {
                                    corner1[0] = (double) vert_array[quad.vertex[2]].x;
                                    corner1[2] = (double) vert_array[quad.vertex[2]].z;
                                    corner1[1] = (double) vert_array[quad.vertex[2]].y;

                                    corner2[0] = (double) vert_array[quad.vertex[3]].x;
                                    corner2[2] = (double) vert_array[quad.vertex[3]].z;
                                    corner2[1] = (double) vert_array[quad.vertex[3]].y;

                                    corner3[0] = (double) vert_array[quad.vertex[0]].x;
                                    corner3[2] = (double) vert_array[quad.vertex[0]].z;
                                    corner3[1] = (double) vert_array[quad.vertex[0]].y;
                                }
                                // run first Ray test, if false, then next quad.
                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                // j=num;
                                i = 300;
                            }

                            if (result != 1) {
                                if (xobj[blk2].obj[XObjCounter].crosstype != 1) {
                                    corner1[0] = (double) vert_array[quad.vertex[1]].x + ref_point.x;
                                    corner1[2] = (double) vert_array[quad.vertex[1]].z + ref_point.z;
                                    corner1[1] = (double) vert_array[quad.vertex[1]].y + ref_point.y;

                                    corner2[0] = (double) vert_array[quad.vertex[2]].x + ref_point.x;
                                    corner2[2] = (double) vert_array[quad.vertex[2]].z + ref_point.z;
                                    corner2[1] = (double) vert_array[quad.vertex[2]].y + ref_point.y;

                                    corner3[0] = (double) vert_array[quad.vertex[0]].x + ref_point.x;
                                    corner3[2] = (double) vert_array[quad.vertex[0]].z + ref_point.z;
                                    corner3[1] = (double) vert_array[quad.vertex[0]].y + ref_point.y;
                                } else {
                                    corner1[0] = (double) vert_array[quad.vertex[1]].x;
                                    corner1[2] = (double) vert_array[quad.vertex[1]].z;
                                    corner1[1] = (double) vert_array[quad.vertex[1]].y;

                                    corner2[0] = (double) vert_array[quad.vertex[2]].x;
                                    corner2[2] = (double) vert_array[quad.vertex[2]].z;
                                    corner2[1] = (double) vert_array[quad.vertex[2]].y;

                                    corner3[0] = (double) vert_array[quad.vertex[0]].x;
                                    corner3[2] = (double) vert_array[quad.vertex[0]].z;
                                    corner3[1] = (double) vert_array[quad.vertex[0]].y;
                                }
                                // run first Ray test, if false, then next quad.
                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                // j=num;
                                i = 300;
                            }
                        }
                    }
                }
                //}
                // Xtra Object End.
                i++;
            }
            if (result == 1)
                sv[p2->vertex[k]] = 0xFF666666; // 0xFF777777;
            else
                sv[p2->vertex[k]] = 0xFFFFFFFF;
        }
    }
}

void ObjectShadingFixer(int blk, float theta, float rho, TRKBLOCK *trk, POLYGONBLOCK *poly, XOBJBLOCK *xobj) {
    int i, j, j2, k, num, num2, blk2;
    int result;
    double uu, vv, tt;
    double corner1[3], corner2[3], corner3[3];
    double Pos[3], RayDir[3], StartPos[3];

    LPPOLYGONDATA p, p2;

    struct TRKBLOCK *t;
    struct FLOATPT *v, *v2;
    unsigned long *sv;

    t  = &trk[blk];
    v  = trk[blk].vert;
    sv = reinterpret_cast<unsigned long *>(trk[blk].unknVertices);

    // malloc example:
    // a1 = (double *)malloc(m*m*sizeof(double));

    double PI         = 3.14159265359;
    double PitchAngle = (double) theta; // 0.00;//4.712; //0.44; //Theta
    double RhoAngle   = (double) rho;   // 0.165; //0.13; //RHO
    // to get real Yaw from Rho, it has to be multiplied with 4.(*WRONG*)
    // Rho is given as fixed point. (1.0 is 2*pi in radian scale.)
    double YawAngle = (2 * PI) * RhoAngle;

    // to get real Pitch from Theta, it needs +pi
    // PitchAngle=PitchAngle+pi;

    // double r;
    struct FLOATPT VNormal;
    VNormal.x = 0;
    VNormal.y = 0;
    VNormal.z = (float) 0.05;

    /*
    x=rsin(phi)cos(theta), y=rsin(phi)sin(theta), z=rcos(phi), r=sqrt(x*x+y*y+z*z)...
    */

    for (long chkchunkcounter = 0; chkchunkcounter < 4; chkchunkcounter++) {
        long chknumobj = poly[blk].obj[chkchunkcounter].nobj;
        for (long chkobjcounter = 0; chkobjcounter < chknumobj; chkobjcounter++) {
            num2 = poly[blk].obj[chkchunkcounter].numpoly[chkobjcounter];
            p2   = poly[blk].obj[chkchunkcounter].poly[chkobjcounter];

            for (j2 = 0; j2 < num2; j2++, p2++) {
                for (k = 0; k < 4; k++) {
                    // bool tested = new bool[t->nVertices];
                    /*for (uint32_t temp=0; temp>trk[blk].nVertices; temp++)
                        tested[temp]=false;*/

                    assert(p2->vertex[k] >= 0 && p2->vertex[k] < t->nVertices);

                    if ((p2->vertex[k] < 0) || (p2->vertex[k] > t->nVertices)) {
                        std::cout << "FAILED! K: " << k << ", J2:" << j2 << ", BLK:" << blk << std::endl;
                        return;
                    }

                    VNormal     = VertexNormal(blk, p2->vertex[k], trk, poly);
                    StartPos[0] = (double) v[p2->vertex[k]].x + (VNormal.x / 5);
                    StartPos[1] = (double) v[p2->vertex[k]].y + (VNormal.y / 5);
                    StartPos[2] = (double) v[p2->vertex[k]].z + (VNormal.z / 5);

                    RayDir[0] = sin(YawAngle) * cos(PitchAngle);
                    RayDir[1] = sin(YawAngle) * sin(PitchAngle);
                    RayDir[2] = cos(YawAngle);

                    // used for known light point
                    /*EndPos[0]=372;
                    EndPos[1]=-790;
                    EndPos[2]=10;

                    RayDir[0]=(EndPos[0]-StartPos[0]);
                    RayDir[1]=(EndPos[1]-StartPos[1]);
                    RayDir[2]=(EndPos[2]-StartPos[2]);
                    r=sqrt((RayDir[0]*RayDir[0])+(RayDir[1]*RayDir[1])+(RayDir[2]*RayDir[2]));

                    RayDir[0]=RayDir[0]/r;
                    RayDir[1]=RayDir[1]/r;
                    RayDir[2]=RayDir[2]/r;*/

                    result = 0;
                    i      = 0;

                    while ((t->nbdData[i].blk != -1) && (i < 300)) {
                        assert(i >= 0 && i < 300);

                        blk2 = t->nbdData[i].blk;

                        assert(blk2 >= 0 && blk2 < 300);

                        p   = poly[blk2].poly[4];
                        num = poly[blk2].sz[4];
                        v2  = trk[blk2].vert;
                        // Track Polygons
                        //--------------

                        for (j = 0; j < num; j++, p++) {
                            if (blk == 0 && j2 == 0 && k == 1) {
                                std::cout << "keke" << std::endl;
                            }

                            // p->texture
                            if (result != 1) {
                                assert(p->vertex[0] >= 0 && p->vertex[0] < trk[blk2].nVertices);
                                corner1[0] = (double) v2[p->vertex[0]].x;
                                corner1[2] = (double) v2[p->vertex[0]].z;
                                corner1[1] = (double) v2[p->vertex[0]].y;

                                assert(p->vertex[1] >= 0 && p->vertex[1] < trk[blk2].nVertices);
                                corner2[0] = (double) v2[p->vertex[3]].x;
                                corner2[2] = (double) v2[p->vertex[3]].z;
                                corner2[1] = (double) v2[p->vertex[3]].y;

                                assert(p->vertex[2] >= 0 && p->vertex[2] < trk[blk2].nVertices);
                                corner3[0] = (double) v2[p->vertex[2]].x;
                                corner3[2] = (double) v2[p->vertex[2]].z;
                                corner3[1] = (double) v2[p->vertex[2]].y;
                                // run first Ray test, if false, then next quad.
                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                j = num;
                                i = 300;
                            }

                            if (result != 1) {
                                corner1[0] = (double) v2[p->vertex[0]].x;
                                corner1[2] = (double) v2[p->vertex[0]].z;
                                corner1[1] = (double) v2[p->vertex[0]].y;

                                corner2[0] = (double) v2[p->vertex[2]].x;
                                corner2[2] = (double) v2[p->vertex[2]].z;
                                corner2[1] = (double) v2[p->vertex[2]].y;

                                corner3[0] = (double) v2[p->vertex[1]].x;
                                corner3[2] = (double) v2[p->vertex[1]].z;
                                corner3[1] = (double) v2[p->vertex[1]].y;
                                // check the second half of the quad

                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                j = num;
                                i = 300;
                            }

                            if (result == 1) {
                                std::cout << "COLLISION! (BLK/POLY/Node) " << blk << "/" << j2 << "/" << k << " Collided with (BLK/Poly) " << blk2 << "/" << j << std::endl;
                            }
                        }
                        // TrackPolygons End
                        // lane Polygons
                        //-------------
                        // if (result!=1) {
                        num = poly[blk2].sz[5];
                        p   = poly[blk2].poly[5];
                        for (j = 0; j < num; j++, p++) {
                            if (blk == 0 && j2 == 0 && k == 1) {
                                std::cout << "keke" << std::endl;
                            }

                            // p->texture
                            if (result != 1) {
                                assert(p->vertex[0] >= 0 && p->vertex[0] < trk[blk2].nVertices);
                                corner1[0] = (double) v2[p->vertex[0]].x;
                                corner1[2] = (double) v2[p->vertex[0]].z;
                                corner1[1] = (double) v2[p->vertex[0]].y;

                                assert(p->vertex[1] >= 0 && p->vertex[1] < trk[blk2].nVertices);
                                corner2[0] = (double) v2[p->vertex[3]].x;
                                corner2[2] = (double) v2[p->vertex[3]].z;
                                corner2[1] = (double) v2[p->vertex[3]].y;

                                assert(p->vertex[2] >= 0 && p->vertex[2] < trk[blk2].nVertices);
                                corner3[0] = (double) v2[p->vertex[2]].x;
                                corner3[2] = (double) v2[p->vertex[2]].z;
                                corner3[1] = (double) v2[p->vertex[2]].y;
                                // run first Ray test, if false, then next quad.
                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                // j=num;
                                i = 300;
                            }

                            if (result != 1) {
                                corner1[0] = (double) v2[p->vertex[0]].x;
                                corner1[2] = (double) v2[p->vertex[0]].z;
                                corner1[1] = (double) v2[p->vertex[0]].y;

                                corner2[0] = (double) v2[p->vertex[2]].x;
                                corner2[2] = (double) v2[p->vertex[2]].z;
                                corner2[1] = (double) v2[p->vertex[2]].y;

                                corner3[0] = (double) v2[p->vertex[1]].x;
                                corner3[2] = (double) v2[p->vertex[1]].z;
                                corner3[1] = (double) v2[p->vertex[1]].y;
                                // check the second half of the quad

                                Pos[0] = StartPos[0];
                                Pos[1] = StartPos[1];
                                Pos[2] = StartPos[2];
                                result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                            } else {
                                // j=num;
                                i = 300;
                            }

                            if (result == 1) {
                                std::cout << "COLLISION! (BLK/POLY/Node) " << blk << "/" << j2 << "/" << k << " Collided with (BLK/Poly) " << blk2 << "/" << j << std::endl;
                            }
                        }
                        //}
                        // Lane Polygons End
                        // Blue Objects
                        //------------
                        // if (result!=1) {
                        // for (k = 0; k < 4; k++)
                        //{
                        // for (j = 0; j < poly[nBlock].obj[k].nobj; j++)
                        //{
                        // for (i = 0; i < poly[nBlock].obj[k].numpoly[j]; i++)
                        //{
                        // poly[nBlock].obj[k].poly[j][i];
                        for (long chunkcounter = 0; chunkcounter < 4; chunkcounter++) {
                            long numobj = poly[blk2].obj[chunkcounter].nobj;
                            for (long objcounter = 0; objcounter < numobj; objcounter++) {
                                num = poly[blk2].obj[chunkcounter].numpoly[objcounter];

                                p = poly[blk2].obj[chunkcounter].poly[objcounter];
                                for (j = 0; j < num; j++, p++) {
                                    if (blk == 0 && j2 == 0 && k == 1) {
                                        std::cout << "keke" << std::endl;
                                    }

                                    // p->texture
                                    if (result != 1) {
                                        assert(p->vertex[0] >= 0 && p->vertex[0] < trk[blk2].nVertices);
                                        corner1[0] = (double) v2[p->vertex[0]].x;
                                        corner1[2] = (double) v2[p->vertex[0]].z;
                                        corner1[1] = (double) v2[p->vertex[0]].y;

                                        assert(p->vertex[1] >= 0 && p->vertex[1] < trk[blk2].nVertices);
                                        corner2[0] = (double) v2[p->vertex[3]].x;
                                        corner2[2] = (double) v2[p->vertex[3]].z;
                                        corner2[1] = (double) v2[p->vertex[3]].y;

                                        assert(p->vertex[2] >= 0 && p->vertex[2] < trk[blk2].nVertices);
                                        corner3[0] = (double) v2[p->vertex[2]].x;
                                        corner3[2] = (double) v2[p->vertex[2]].z;
                                        corner3[1] = (double) v2[p->vertex[2]].y;
                                        // run first Ray test, if false, then next quad.
                                        Pos[0] = StartPos[0];
                                        Pos[1] = StartPos[1];
                                        Pos[2] = StartPos[2];
                                        result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                                    } else {
                                        // j=num;
                                        i = 300;
                                    }

                                    if (result != 1) {
                                        corner1[0] = (double) v2[p->vertex[0]].x;
                                        corner1[2] = (double) v2[p->vertex[0]].z;
                                        corner1[1] = (double) v2[p->vertex[0]].y;

                                        corner2[0] = (double) v2[p->vertex[2]].x;
                                        corner2[2] = (double) v2[p->vertex[2]].z;
                                        corner2[1] = (double) v2[p->vertex[2]].y;

                                        corner3[0] = (double) v2[p->vertex[1]].x;
                                        corner3[2] = (double) v2[p->vertex[1]].z;
                                        corner3[1] = (double) v2[p->vertex[1]].y;
                                        // check the second half of the quad

                                        Pos[0] = StartPos[0];
                                        Pos[1] = StartPos[1];
                                        Pos[2] = StartPos[2];
                                        result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                                    } else {
                                        // j=num;
                                        i = 300;
                                    }
                                    // Make same with another side: (FIX: not needed...)
                                    /*if (result != 1) {
                                        assert(p->vertex[0] >= 0 && p->vertex[0] < trk[blk2].nVertices);
                                        corner1[0] = (double)v2[p->vertex[0]].x;
                                        corner1[2] = (double)v2[p->vertex[0]].z;
                                        corner1[1] = (double)v2[p->vertex[0]].y;

                                        assert(p->vertex[1] >= 0 && p->vertex[1] < trk[blk2].nVertices);
                                        corner2[0] = (double)v2[p->vertex[3]].x;
                                        corner2[2] = (double)v2[p->vertex[3]].z;
                                        corner2[1] = (double)v2[p->vertex[3]].y;

                                        assert(p->vertex[2] >= 0 && p->vertex[2] < trk[blk2].nVertices);
                                        corner3[0] = (double)v2[p->vertex[2]].x;
                                        corner3[2] = (double)v2[p->vertex[2]].z;
                                        corner3[1] = (double)v2[p->vertex[2]].y;
                                        //run first Ray test, if false, then next quad.
                                        Pos[0]=StartPos[0];
                                        Pos[1]=StartPos[1];
                                        Pos[2]=StartPos[2];
                                        result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                                    }
                                    else
                                    {
                                        //j=num;
                                        i=300;
                                    }

                                    if (result != 1) {
                                        corner1[0] = (double)v2[p->vertex[0]].x;
                                        corner1[2] = (double)v2[p->vertex[0]].z;
                                        corner1[1] = (double)v2[p->vertex[0]].y;

                                        corner2[0] = (double)v2[p->vertex[2]].x;
                                        corner2[2] = (double)v2[p->vertex[2]].z;
                                        corner2[1] = (double)v2[p->vertex[2]].y;

                                        corner3[0] = (double)v2[p->vertex[1]].x;
                                        corner3[2] = (double)v2[p->vertex[1]].z;
                                        corner3[1] = (double)v2[p->vertex[1]].y;
                                        //check the second half of the quad

                                        Pos[0]=StartPos[0];
                                        Pos[1]=StartPos[1];
                                        Pos[2]=StartPos[2];
                                        result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                                    }
                                    else
                                    {
                                        //j=num;
                                        i=300;
                                    }*/

                                    if (result == 1) {
                                        std::cout << "COLLISION! (BLK/POLY/Node) " << blk << "/" << j2 << "/" << k << " Collided with (BLK/Poly) " << blk2 << "/" << j << std::endl;
                                    }
                                }
                            }
                        }

                        //}
                        // Blue Polygons End.
                        // Xtra Object Start
                        // if (result=!1) {
                        for (uint32_t XObjCounter = 0; XObjCounter < xobj[blk2].nobj; XObjCounter++) {
                            FLOATPT *vert_array = xobj[blk2].obj[XObjCounter].vert;
                            if (xobj[blk2].obj[XObjCounter].crosstype != 6) {
                                for (uint32_t XObjPoly = 0; XObjPoly < xobj[blk2].obj[XObjCounter].nPolygons; XObjPoly++) {
                                    POLYGONDATA &quad  = xobj[blk2].obj[XObjCounter].polyData[XObjPoly];
                                    FLOATPT &ref_point = xobj[blk2].obj[XObjCounter].ptRef;

                                    if (result != 1) {
                                        if (xobj[blk2].obj[XObjCounter].crosstype != 1) {
                                            corner1[0] = (double) vert_array[quad.vertex[0]].x + ref_point.x;
                                            corner1[2] = (double) vert_array[quad.vertex[0]].z + ref_point.z;
                                            corner1[1] = (double) vert_array[quad.vertex[0]].y + ref_point.y;

                                            corner2[0] = (double) vert_array[quad.vertex[3]].x + ref_point.x;
                                            corner2[2] = (double) vert_array[quad.vertex[3]].z + ref_point.z;
                                            corner2[1] = (double) vert_array[quad.vertex[3]].y + ref_point.y;

                                            corner3[0] = (double) vert_array[quad.vertex[2]].x + ref_point.x;
                                            corner3[2] = (double) vert_array[quad.vertex[2]].z + ref_point.z;
                                            corner3[1] = (double) vert_array[quad.vertex[2]].y + ref_point.y;
                                        } else {
                                            corner1[0] = (double) vert_array[quad.vertex[0]].x;
                                            corner1[2] = (double) vert_array[quad.vertex[0]].z;
                                            corner1[1] = (double) vert_array[quad.vertex[0]].y;

                                            corner2[0] = (double) vert_array[quad.vertex[3]].x;
                                            corner2[2] = (double) vert_array[quad.vertex[3]].z;
                                            corner2[1] = (double) vert_array[quad.vertex[3]].y;

                                            corner3[0] = (double) vert_array[quad.vertex[2]].x;
                                            corner3[2] = (double) vert_array[quad.vertex[2]].z;
                                            corner3[1] = (double) vert_array[quad.vertex[2]].y;
                                        }
                                        // run first Ray test, if false, then next quad.
                                        Pos[0] = StartPos[0];
                                        Pos[1] = StartPos[1];
                                        Pos[2] = StartPos[2];
                                        result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                                    } else {
                                        // j=num;
                                        i = 300;
                                    }

                                    if (result != 1) {
                                        if (xobj[blk2].obj[XObjCounter].crosstype != 1) {
                                            corner1[0] = (double) vert_array[quad.vertex[0]].x + ref_point.x;
                                            corner1[2] = (double) vert_array[quad.vertex[0]].z + ref_point.z;
                                            corner1[1] = (double) vert_array[quad.vertex[0]].y + ref_point.y;

                                            corner2[0] = (double) vert_array[quad.vertex[2]].x + ref_point.x;
                                            corner2[2] = (double) vert_array[quad.vertex[2]].z + ref_point.z;
                                            corner2[1] = (double) vert_array[quad.vertex[2]].y + ref_point.y;

                                            corner3[0] = (double) vert_array[quad.vertex[1]].x + ref_point.x;
                                            corner3[2] = (double) vert_array[quad.vertex[1]].z + ref_point.z;
                                            corner3[1] = (double) vert_array[quad.vertex[1]].y + ref_point.y;
                                        } else {
                                            corner1[0] = (double) vert_array[quad.vertex[0]].x;
                                            corner1[2] = (double) vert_array[quad.vertex[0]].z;
                                            corner1[1] = (double) vert_array[quad.vertex[0]].y;

                                            corner2[0] = (double) vert_array[quad.vertex[2]].x;
                                            corner2[2] = (double) vert_array[quad.vertex[2]].z;
                                            corner2[1] = (double) vert_array[quad.vertex[2]].y;

                                            corner3[0] = (double) vert_array[quad.vertex[1]].x;
                                            corner3[2] = (double) vert_array[quad.vertex[1]].z;
                                            corner3[1] = (double) vert_array[quad.vertex[1]].y;
                                        }
                                        // run first Ray test, if false, then next quad.
                                        Pos[0] = StartPos[0];
                                        Pos[1] = StartPos[1];
                                        Pos[2] = StartPos[2];
                                        result = intersect_triangle(Pos, RayDir, corner1, corner2, corner3, &tt, &uu, &vv);
                                    } else {
                                        // j=num;
                                        i = 300;
                                    }
                                }
                            }
                        }
                        //}
                        // Xtra Object End.
                        i++;
                    }
                    if (result == 1)
                        sv[p2->vertex[k]] = 0xFF666666; // 0xFF777777;
                    else
                        sv[p2->vertex[k]] = 0xFFFFFFFF;
                    // result=0;
                    // delete tested[];
                }
            }
        }
    }
}