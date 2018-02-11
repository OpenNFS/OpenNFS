//
// Created by Amrik on 16/01/2018.
//


#include <iostream>
#include <fstream>
#include <cstring>
#include "trk_loader.h"
#include "NFS3_Mesh.h"

using namespace std;

bool trk_loader::LoadFRD(std::string frd_path)
{
    ifstream ar (frd_path, ios::in | ios::binary);

    int i,j,k,l;
    struct TRKBLOCK *b;
    struct POLYGONBLOCK *p;
    struct XOBJDATA *x;
    struct OBJPOLYBLOCK *o;

    if (ar.read(header,28).gcount() != 28) return false; // header & numblocks
    if (ar.read((char*) &nBlocks,4).gcount() <4) return false;
    nBlocks++;
    if ((nBlocks<1)||(nBlocks>500)) return false; // 1st sanity check
    
    trk=(struct TRKBLOCK *)malloc(nBlocks*sizeof(struct TRKBLOCK));
    if (trk==nullptr) return false;
    memset(trk,0,nBlocks*sizeof(struct TRKBLOCK));
    poly=(struct POLYGONBLOCK *)malloc(nBlocks*sizeof(struct POLYGONBLOCK));
    if (poly==nullptr) return false;
    memset(poly,0,nBlocks*sizeof(struct POLYGONBLOCK));
    xobj=(struct XOBJBLOCK *)malloc((4*nBlocks+1)*sizeof(struct XOBJBLOCK));
    if (xobj==nullptr) return false;
    memset(xobj,0,(4*nBlocks+1)*sizeof(struct XOBJBLOCK));
    
    if (ar.read((char *) &l,4).gcount() <4) return false; // choose between NFS3 & NFSHS
    if ((l<0)||(l>5000)) bHSMode=false;
    else if (((l+7)/8)==nBlocks) bHSMode=true;
    else return false; // unknown file type
    
    memcpy(trk,&l,4);
    if (ar.read(((char *)trk)+4,80).gcount()!=80) return false;

    // TRKBLOCKs
    for (i=0;i<nBlocks;i++) {
        b=&(trk[i]);
        // ptCentre, ptBounding, 6 nVertices == 84 bytes
        if (i!=0) { if (ar.read((char *)b,84).gcount() !=84) return false; }
        if ((b->nVertices<0)) return false;
        b->vert=(struct FLOATPT *)malloc(b->nVertices*sizeof(struct FLOATPT));
        if (b->vert==nullptr) return false;
        if ((long)ar.read((char *)b->vert,12*b->nVertices).gcount() !=12*b->nVertices) return false;
        b->unknVertices=(long *)malloc(b->nVertices*sizeof(long));
        if (b->unknVertices==nullptr) return false;
        if ((long)ar.read((char *)b->unknVertices,4*b->nVertices).gcount() !=4*b->nVertices) return false;
        if (ar.read((char *)b->nbdData,4*0x12C).gcount()!=4*0x12C) return false;

        // nStartPos & various blk sizes == 32 bytes
        if (ar.read((char *)&(b->nStartPos),32).gcount() !=32) return false;
        if (i>0) if (b->nStartPos!=trk[i-1].nStartPos+trk[i-1].nPositions) return false;
        b->posData=(struct POSITIONDATA *)malloc(b->nPositions*sizeof(struct POSITIONDATA));
        if (b->posData==nullptr) return false;
        if ((long)ar.read((char *)b->posData, 8*b->nPositions).gcount() !=8*b->nPositions) return false;
        b->polyData=(struct POLYVROADDATA *)malloc(b->nPolygons*sizeof(struct POLYVROADDATA));
        if (b->polyData==nullptr) return false;
        memset(b->polyData,0,b->nPolygons*sizeof(struct POLYVROADDATA));
        for (j=0;j<b->nPolygons;j++)
            if (ar.read((char *)b->polyData+j,8).gcount() !=8) return false;
        b->vroadData=(struct VROADDATA *)malloc(b->nVRoad*sizeof(struct VROADDATA));
        if (b->vroadData==nullptr) return false;
        if ((long)ar.read((char *)b->vroadData,12*b->nVRoad).gcount()!=12*b->nVRoad) return false;
        if (b->nXobj>0) {
            b->xobj=(struct REFXOBJ *)malloc(b->nXobj*sizeof(struct REFXOBJ));
            if (b->xobj==nullptr) return false;
            if ((long)ar.read((char *)b->xobj,20*b->nXobj).gcount() !=20*b->nXobj) return false;
        }
        if (b->nPolyobj>0) {
            char *buffer=(char *)malloc(b->nPolyobj*20);
            if ((long)ar.read(buffer,20*b->nPolyobj).gcount() !=20*b->nPolyobj) return false;
            free(buffer);
        }
        b->nPolyobj=0;
        if (b->nSoundsrc>0) {
            b->soundsrc=(struct SOUNDSRC *)malloc(b->nSoundsrc*sizeof(struct SOUNDSRC));
            if (b->soundsrc==nullptr) return false;
            if ((long)ar.read((char *)b->soundsrc,16*b->nSoundsrc).gcount() !=16*b->nSoundsrc) return false;
        }
        if (b->nLightsrc>0) {
            b->lightsrc=(struct LIGHTSRC *)malloc(b->nLightsrc*sizeof(struct LIGHTSRC));
            if (b->lightsrc==nullptr) return false;
            if ((long)ar.read((char *) b->lightsrc,16*b->nLightsrc).gcount() !=16*b->nLightsrc) return false;
        }
    }

    std::cout << "Writing Meshes to " << "Track.obj" << std::endl;
    std::ofstream obj_dump;
    obj_dump.open("Track.obj");

    /* Print Part name*/
    obj_dump << "o " << "Track" << std::endl;
    //Dump Vertices
    for(int dump_idx = 0; dump_idx < 279; dump_idx++){
        obj_dump << "v " <<  b->vert[dump_idx].x << " " << b->vert[dump_idx].y << " " << b->vert[dump_idx].z << std::endl;
    }
    obj_dump.close();


    // POLYGONBLOCKs
    for (i=0;i<nBlocks;i++) {
        p=&(poly[i]);
        for (j=0;j<7;j++) {
            if (ar.read((char *)&(p->sz[j]),4).gcount() !=4) return false;
            if (p->sz[j]!=0) {
                if (ar.read((char *)&(p->szdup[j]),4).gcount()!=4) return false;
                if (p->szdup[j]!=p->sz[j]) return false;
                p->poly[j]=(LPPOLYGONDATA)malloc(p->sz[j]*sizeof(struct POLYGONDATA));
                if (p->poly[j]==nullptr) return false;
                if ((long)ar.read((char *)p->poly[j],14*p->sz[j]).gcount() !=14*p->sz[j]) return false;
            }
        }
        if (p->sz[4]!=trk[i].nPolygons) return false; // sanity check
        for (j=0;j<4;j++) {
            o=&(p->obj[j]);
            if (ar.read((char *)&(o->n1),4).gcount()!=4) return false;
            if (o->n1>0) {
                if (ar.read((char *)&(o->n2),4).gcount()!=4) return false;
                o->types=(long *)calloc(o->n2, sizeof(long));
                if (o->types==nullptr) return false;
                o->numpoly=(long *)malloc(o->n2*sizeof(long));
                if (o->numpoly==nullptr) return false;
                o->poly=(LPPOLYGONDATA *)calloc(o->n2, sizeof(LPPOLYGONDATA));
                if (o->poly==nullptr) return false;
                o->nobj=0; l=0;
                for(k=0;k<o->n2;k++) {
                    if (ar.read((char *)o->types+k,4).gcount()!=4) return false;
                    //if (o->types[k]==1) {
                        if (ar.read((char *)o->numpoly+o->nobj,4).gcount() !=4) return false;
                        o->poly[o->nobj]=(LPPOLYGONDATA)malloc(o->numpoly[o->nobj]*sizeof(struct POLYGONDATA));
                        if (o->poly[o->nobj]==nullptr) return false;
                        if ((long)ar.read((char *)o->poly[o->nobj],14*o->numpoly[o->nobj]).gcount()!=14*o->numpoly[o->nobj]) return false;
                        l+=o->numpoly[o->nobj];
                        o->nobj++;
                    //}
                }
                if (l != o->n1) return false; // n1 == total nb polygons
            }
        }
    }

    // XOBJBLOCKs
    for (i=0;i<=4*nBlocks;i++) {
        if (ar.read((char *)&(xobj[i].nobj),4).gcount() !=4) return false;
        if (xobj[i].nobj>0) {
            xobj[i].obj=(struct XOBJDATA *)malloc(xobj[i].nobj*sizeof(struct XOBJDATA));
            if (xobj[i].obj==nullptr) return false;
            memset(xobj[i].obj,0,xobj[i].nobj*sizeof(struct XOBJDATA));
        }
        for (j=0;j<xobj[i].nobj;j++) {
            x=&(xobj[i].obj[j]);
            // 3 headers == 12 bytes
            if (ar.read((char *) x,12).gcount() !=12) return false;
            if (x->crosstype==4) { // basic objects
                if (ar.read((char *)&(x->ptRef),12).gcount()!=12) return false;
                if (ar.read((char *)&(x->unknown2),4).gcount()!=4) return false;
            }
            else if (x->crosstype==3) { // animated objects
                // unkn3, type3, objno, nAnimLength, unkn4 == 24 bytes
                if (ar.read((char *)x->unknown3,24).gcount()!=24) return false;
                if (x->type3!=3) return false;
                x->animData=(struct ANIMDATA *)malloc(20*x->nAnimLength);
                if (x->animData==nullptr) return false;
                if ((long)ar.read((char *)x->animData,20*x->nAnimLength).gcount()!=20*x->nAnimLength) return false;
                // make a ref point from first anim position
                x->ptRef.x=(float)(x->animData->pt.x/65536.0);
                x->ptRef.z=(float)(x->animData->pt.z/65536.0);
                x->ptRef.y=(float)(x->animData->pt.y/65536.0);
            }
            else return false; // unknown object type
            // common part : vertices & polygons
            if (ar.read((char *)&(x->nVertices),4).gcount()!=4) return false;
            x->vert=(struct FLOATPT *)malloc(12*x->nVertices);
            if (x->vert==nullptr) return false;
            if ((long)ar.read((char *)x->vert,12*x->nVertices).gcount()!=12*x->nVertices) return false;
            x->unknVertices=(long *)malloc(4*x->nVertices);
            if (x->unknVertices==nullptr) return false;
            if ((long)ar.read((char *)x->unknVertices,4*x->nVertices).gcount()!=4*x->nVertices) return false;
            if (ar.read((char *)&(x->nPolygons),4).gcount()!=4) return false;
            x->polyData=(struct POLYGONDATA *)malloc(x->nPolygons*14);
            if (x->polyData==nullptr) return false;
            if ((long)ar.read((char *)x->polyData,14*x->nPolygons).gcount()!=14*x->nPolygons) return false;
        }
    }

    // TEXTUREBLOCKs
    if (ar.read((char *)&nTextures,4).gcount() !=4) return false;
    texture=(struct TEXTUREBLOCK *)malloc(nTextures*sizeof(struct TEXTUREBLOCK));
    for (i=0;i<nTextures;i++)
        if (ar.read((char *)&(texture[i]),47).gcount() !=47) return false;

    return ar.read((char *)&i, 4).gcount() == 0; // we ought to be at EOF now
}

trk_loader::~trk_loader() {

}

trk_loader::trk_loader(std::string frd_path){
    bool result = LoadFRD(frd_path);
    cout << result << std::endl;
}
