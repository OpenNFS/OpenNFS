//
// Created by Amrik on 16/01/2018.
//

#pragma GCC push_options
#pragma GCC optimize ("O0")

#include <iostream>
#include <fstream>
#include <cstring>
#include "trk_loader.h"
#include "Model.h"

using namespace std;

bool trk_loader::LoadFRD(std::string frd_path)
{
    // TODO: Wrap each fread with if(STATEMENT) != numElementsToRead) return false; MACRO?
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
            auto *buffer=(char *)malloc(static_cast<size_t>(b->nPolyobj * 20));
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

    // TODO: Identify in what cases stream reads garbage
    // This workaround is emblematic of a larger problem, why does the file stream 'ar' not read into these structs
    auto pos = static_cast<int>(ar.tellg());
    FILE *trk_file = fopen(frd_path.c_str(), "rb");
    fseek(trk_file, pos, SEEK_SET);

    // POLYGONBLOCKs
    for (i=0;i<nBlocks;i++) {
        p=&(poly[i]);
        for (j=0;j<7;j++) {
            fread(&(p->sz[j]), 0x4, 1, trk_file);
            if (p->sz[j]!=0) {
                fread(&(p->szdup[j]), 0x4, 1, trk_file);
                if (p->szdup[j]!=p->sz[j]) return false;
                p->poly[j]=(LPPOLYGONDATA)malloc(p->sz[j]*sizeof(struct POLYGONDATA));
                if (p->poly[j]==nullptr) return false;
                fread(p->poly[j], static_cast<size_t>(14 * p->sz[j]), 1, trk_file);
            }
        }
        if (p->sz[4]!=trk[i].nPolygons) return false; // sanity check
        for (j=0;j<4;j++) {
            o=&(p->obj[j]);
            fread(&(o->n1), 0x4, 1, trk_file);
            if (o->n1>0) {
                fread(&(o->n2), 0x4, 1, trk_file);
                o->types=(long *)calloc(static_cast<size_t>(o->n2), sizeof(long));
                if (o->types==nullptr) return false;
                o->numpoly=(long *)malloc(o->n2*sizeof(long));
                if (o->numpoly==nullptr) return false;
                o->poly=(LPPOLYGONDATA *)calloc(static_cast<size_t>(o->n2), sizeof(LPPOLYGONDATA));
                if (o->poly==nullptr) return false;
                o->nobj=0; l=0;
                for(k=0;k<o->n2;k++) {
                    fread(o->types+k, 0x4, 1, trk_file);
                    if (o->types[k]==1) {
                        fread(o->numpoly+o->nobj, 0x4, 1, trk_file);
                        o->poly[o->nobj]=(LPPOLYGONDATA)malloc(o->numpoly[o->nobj]*sizeof(struct POLYGONDATA));
                        if (o->poly[o->nobj]==nullptr) return false;
                        fread(o->poly[o->nobj], static_cast<size_t>(14 * o->numpoly[o->nobj]), 1, trk_file);
                        l+=o->numpoly[o->nobj];
                        o->nobj++;
                    }
                }
                if (l != o->n1) return false; // n1 == total nb polygons
            }
        }
    }

    ar.seekg(ftell(trk_file), ar.beg);
    fclose(trk_file);

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
                x->animData=(struct ANIMDATA *)malloc(static_cast<size_t>(20 * x->nAnimLength));
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
            x->vert=(struct FLOATPT *)malloc(static_cast<size_t>(12 * x->nVertices));
            if (x->vert==nullptr) return false;
            if ((long)ar.read((char *)x->vert,12*x->nVertices).gcount()!=12*x->nVertices) return false;
            x->unknVertices=(long *)malloc(static_cast<size_t>(4 * x->nVertices));
            if (x->unknVertices==nullptr) return false;
            if ((long)ar.read((char *)x->unknVertices,4*x->nVertices).gcount()!=4*x->nVertices) return false;
            if (ar.read((char *)&(x->nPolygons),4).gcount()!=4) return false;
            x->polyData=(struct POLYGONDATA *)malloc(static_cast<size_t>(x->nPolygons * 14));
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

bool trk_loader::LoadCOL(std::string col_path)
{
    // TODO: Wrap each fread with if(STATEMENT) != numElementsToRead) return false; MACRO?
    ifstream coll (col_path, ios::in | ios::binary);

    struct COLSTRUCT3D *s;
    struct COLOBJECT *o;
    int i,delta,dummy;

    col.hs_extra=NULL;
    if (coll.read((char *)&col,16).gcount()!=16) return false;
    if ((col.collID[0]!='C')||(col.collID[1]!='O')||
        (col.collID[2]!='L')||(col.collID[3]!='L')) return false;
    if (col.version!=11) return false;
    if ((col.nBlocks!=2)&&(col.nBlocks!=4)&&(col.nBlocks!=5)) return false;
    if ((long)coll.read((char *)col.xbTable,4*col.nBlocks).gcount()!=4*col.nBlocks) return false;

    // texture XB
    if (coll.read((char *)&col.textureHead,8).gcount()!=8) return false;
    if (col.textureHead.xbid!=XBID_TEXTUREINFO) return false;
    if (col.textureHead.size!=8+8*col.textureHead.nrec) return false;
    col.texture=(struct COLTEXTUREINFO *)
            malloc(col.textureHead.nrec*sizeof(struct COLTEXTUREINFO));
    if (col.texture==NULL) return false;
    if ((long)coll.read((char *)col.texture,8*col.textureHead.nrec).gcount()!=8*col.textureHead.nrec) return false;

    // struct3D XB
    if (col.nBlocks>=4) {
        if (coll.read((char *)&col.struct3DHead,8).gcount()!=8) return false;
        if (col.struct3DHead.xbid!=XBID_STRUCT3D) return false;
        s=col.struct3D=(struct COLSTRUCT3D *)
                malloc(col.struct3DHead.nrec*sizeof(struct COLSTRUCT3D));
        if (s==NULL) return false;
        memset(s,0,col.struct3DHead.nrec*sizeof(struct COLSTRUCT3D));
        for (i=0;i<col.struct3DHead.nrec;i++,s++) {
            if (coll.read((char *)s,8).gcount()!=8) return false;
            delta=(8+16*s->nVert+6*s->nPoly)%4;
            delta=(4-delta)%4;
            if (s->size!=8+16*s->nVert+6*s->nPoly+delta) return false;
            s->vertex=(struct COLVERTEX *)malloc(16*s->nVert);
            if (s->vertex==NULL) return false;
            if ((long)coll.read((char *)s->vertex,16*s->nVert).gcount()!=16*s->nVert) return false;
            s->polygon=(struct COLPOLYGON *)malloc(6*s->nPoly);
            if (s->polygon==NULL) return false;
            if ((long)coll.read((char *)s->polygon,6*s->nPoly).gcount()!=6*s->nPoly) return false;
            if (delta>0) if ((int)coll.read((char *)&dummy,delta).gcount()!=delta) return false;
        }

        // object XB
        if (coll.read((char *)&col.objectHead,8).gcount()!=8) return false;
        if ((col.objectHead.xbid!=XBID_OBJECT)&&(col.objectHead.xbid!=XBID_OBJECT2)) return false;
        o=col.object=(struct COLOBJECT *)
                malloc(col.objectHead.nrec*sizeof(struct COLOBJECT));
        if (o==NULL) return false;
        memset(o,0,col.objectHead.nrec*sizeof(struct COLOBJECT));
        for (i=0;i<col.objectHead.nrec;i++,o++) {
            if (coll.read((char *)o,4).gcount()!=4) return false;
            if (o->type==1) {
                if (o->size!=16) return false;
                if (coll.read((char *)&(o->ptRef),12).gcount()!=12) return false;
            } else if (o->type==3) {
                if (coll.read((char *)&(o->animLength),4).gcount()!=4) return false;
                if (o->size!=8+20*o->animLength) return false;
                o->animData=(struct ANIMDATA *)malloc(20*o->animLength);
                if (o->animData==nullptr) return false;
                if ((long)coll.read((char *)o->animData,20*o->animLength).gcount()!=20*o->animLength) return false;
                o->ptRef.x=o->animData->pt.x;
                o->ptRef.z=o->animData->pt.z;
                o->ptRef.y=o->animData->pt.y;
            } else return false; // unknown object type
        }
    }

    // object2 XB
    if (col.nBlocks==5) {
        if (coll.read((char *)&col.object2Head,8).gcount()!=8) return false;
        if ((col.object2Head.xbid!=XBID_OBJECT)&&(col.object2Head.xbid!=XBID_OBJECT2)) return false;
        o=col.object2=(struct COLOBJECT *)
                malloc(col.object2Head.nrec*sizeof(struct COLOBJECT));
        if (o==NULL) return false;
        memset(o,0,col.object2Head.nrec*sizeof(struct COLOBJECT));
        for (i=0;i<col.object2Head.nrec;i++,o++) {
            if (coll.read((char *)o,4).gcount()!=4) return false;
            if (o->type==1) {
                if (o->size!=16) return false;
                if (coll.read((char *)&(o->ptRef),12).gcount()!=12) return false;
            } else if (o->type==3) {
                if (coll.read((char *)&(o->animLength),4).gcount()!=4) return false;
                if (o->size!=8+20*o->animLength) return false;
                o->animData=(struct ANIMDATA *)malloc(20*o->animLength);
                if (o->animData==NULL) return false;
                if ((long)coll.read((char *)o->animData,20*o->animLength).gcount()!=20*o->animLength) return false;
                o->ptRef.x=o->animData->pt.x;
                o->ptRef.z=o->animData->pt.z;
                o->ptRef.y=o->animData->pt.y;
            } else return false; // unknown object type
        }
    }

    // vroad XB
    if (coll.read((char *)&col.vroadHead,8).gcount()!=8) return false;
    if (col.vroadHead.xbid!=XBID_VROAD) return false;
    if (col.vroadHead.size!=8+36*col.vroadHead.nrec) return false;
    //ASSERT(col.vroadHead.nrec==trk[nBlocks-1].nStartPos+trk[nBlocks-1].nPositions);
    col.vroad=(struct COLVROAD *)malloc(col.vroadHead.nrec*sizeof(struct COLVROAD));
    if (col.vroad==NULL) return false;
    if ((long)coll.read((char *)col.vroad,36*col.vroadHead.nrec).gcount()!=36*col.vroadHead.nrec) return false;

    return coll.read((char *)&i, 4).gcount() == 0; // we ought to be at EOF now
}

trk_loader::~trk_loader() = default;

trk_loader::trk_loader(const std::string &frd_path){
    if(LoadFRD(frd_path)){
        //if(LoadCOL("../resources/TR00.COL"))
        //    std::cout << "Successful track load!" << std::endl;
        //else
        //    return;
    } else
        return;

    for(int i = 0; i < nBlocks; i++) {
        // Get Verts from Trk block, indices from associated polygon block
        TRKBLOCK trk_block = trk[i];
        POLYGONBLOCK polygon_block = poly[i];
        Model current_trk_block_model = Model("TrkBlock");

        // Fillers
        std::vector<glm::vec2> uvs;
        std::vector<glm::vec3> normals;
        // Real shit
        std::vector<unsigned int> indices;
        // Get indices from Chunk 4 for High Res polys
        LPPOLYGONDATA poly_chunk = polygon_block.poly[4];
        for(int chnk = 0; chnk < 6; chnk++){
            for(int k = 0; k < polygon_block.sz[chnk]; k++)
            {
                indices.push_back((unsigned int) poly_chunk[k].vertex[0]);
                indices.push_back((unsigned int) poly_chunk[k].vertex[1]);
                indices.push_back((unsigned int) poly_chunk[k].vertex[2]);
                //indices.push_back((unsigned int) poly_chunk[k].vertex[3]);
                uvs.push_back(glm::vec2(0,0));
                uvs.push_back(glm::vec2(0,0));
                uvs.push_back(glm::vec2(0,0));
                uvs.push_back(glm::vec2(0,0));
                //normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
                //std::cout << poly_chunk[k].vertex[0] << " " << poly_chunk[k].vertex[1] << " " << poly_chunk[k].vertex[2] << " " << poly_chunk[k].vertex[3] << std::endl;
            }
        }
        current_trk_block_model.setIndices(indices);
        current_trk_block_model.setUVs(uvs);
        current_trk_block_model.setNormals(normals);
        // Get all vertices
        std::vector<glm::vec3> verts;
        for (int j = 0; j < trk_block.nVertices; j++) {
            verts.push_back(glm::vec3( trk_block.vert[j].x/1000,
                                       trk_block.vert[j].y/1000,
                                       trk_block.vert[j].z/1000));
        }
        current_trk_block_model.setVertices(verts, true);
        current_trk_block_model.enable();
        //current_trk_block_model.indexed = true;
        trk_blocks.push_back(current_trk_block_model);
    }
}

#pragma GCC pop_options