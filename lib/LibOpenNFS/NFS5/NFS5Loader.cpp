#include "NFS5Loader.h"

// CAR
std::shared_ptr<Car> NFS5::LoadCar(const std::string& carBasePath) {
    boost::filesystem::path p(carBasePath);
    std::string carName = p.filename().string();

    std::stringstream compressedCrpPath, decompressedCrpPath;
    compressedCrpPath << carBasePath << ".crp";
    decompressedCrpPath << CAR_PATH << ToString(NFS_5) << "/" << carName << "/" << carName << ".crp";

    // Create output directory for ONFS Car assets if doesn't exist
    boost::filesystem::path outputPath(decompressedCrpPath.str());
    if (!boost::filesystem::exists(outputPath.parent_path())) {
        boost::filesystem::create_directories(outputPath.parent_path());
    }

    ASSERT(DecompressCRP(compressedCrpPath.str(), decompressedCrpPath.str()), "Unable to extract " << carBasePath << " to " << decompressedCrpPath.str());

    DumpCrpTextures(decompressedCrpPath.str());

    // Let's make an NFS5 car texture array
    // For every file in here that's a BMP, load the data into a Texture object. This lets us easily access textures by an ID.
    std::map<unsigned int, Texture> car_textures;
    std::map<std::string, uint32_t> remapped_texture_ids;
    uint32_t remappedTextureID = 0;

    /*for (boost::filesystem::directory_iterator itr(car_out_path.str()); itr != boost::filesystem::directory_iterator(); ++itr) {
        if (itr->path().filename().string().find("BMP") != std::string::npos && itr->path().filename().string().find("-a") ==
    std::string::npos) {
            // Map texture names, strings, into numbers so I can use them for indexes into the eventual Texture Array
            remapped_texture_ids[itr->path().filename().replace_extension("").string()] = remappedTextureID++;
            if (std::is_same<Platform, PS1>::value) {
                GLubyte *data;
                GLsizei width;
                GLsizei height;
                ASSERT(ImageLoader::LoadBmpCustomAlpha(itr->path().string().c_str(), &data, &width, &height, 0u), "Texture " <<
    itr->path().string() << " did not load succesfully!");
    car_textures[remapped_texture_ids[itr->path().filename().replace_extension("").string()]] =
    Texture(remapped_texture_ids[itr->path().filename().replace_extension("").string()], data, static_cast<unsigned int>(width),
    static_cast<unsigned int>(height)); } else { bmpread_t bmpAttr; // This will leak. ASSERT(bmpread(itr->path().string().c_str(),
    BMPREAD_ANY_SIZE | BMPREAD_ALPHA, &bmpAttr), "Texture " << itr->path().string() << " did not load succesfully!");
                car_textures[remapped_texture_ids[itr->path().filename().replace_extension("").string()]] =
    Texture(remapped_texture_ids[itr->path().filename().replace_extension("").string()], bmpAttr.data, static_cast<unsigned
    int>(bmpAttr.width), static_cast<unsigned int>(bmpAttr.height));
            }
        }
    }

    GLuint textureArray_id = MakeTextureArray(car_textures, false);*/

    return std::make_shared<Car>(LoadCRP(decompressedCrpPath.str()), NFS_5, carName);
}

// Hook into CrpLib using ZModeler Import.cpp mechanism
CarData NFS5::LoadCRP(const std::string& crpPath) {
    LOG(INFO) << "Parsing CRP File located at " << crpPath << " with Arushan's CrpLib";

    CarData carData;
    CCrpFile* crp = new CCrpFile();
    ASSERT(crp->Open(crpPath), "Could not open CRP file located at " << crpPath);

    for (int i = 0; i < crp->GetArticleCount(); i++) {
        CEntry* arti = crp->GetArticle(i);

        CEntry* en         = arti->GetSubEntry(ID_NAME);
        char* partNameOrig = ((CRawData*) en->GetData())->GetData();

        en          = arti->GetSubEntry(ID_BASE);
        CBase* base = (CBase*) en->GetData();

        CEntry* enEf = arti->GetSubEntry(ID_EFFECT);

        if (enEf == NULL) {
            for (int v = 0; v < base->GetLevelCount(); v++) {
                int lev = base->GetLevel(v)->Level;

                char partName[100];
                sprintf(partName, "(%d,%d)%s", base->GetBaseInfo()->GeomIndex, base->GetBaseInfo()->TypeIndex, partNameOrig);
                switch (lev) {
                case 0:
                    strcat(partName, "_def");
                    break;
                case 1:
                    strcat(partName, "_fe1");
                    break;
                case 2:
                    strcat(partName, "_fe2");
                    break;
                case 3:
                    strcat(partName, "_ig1");
                    break;
                case 4:
                    strcat(partName, "_ig2");
                    break;
                case 5:
                    strcat(partName, "_ig3");
                    break;
                case 6:
                    strcat(partName, "_ig4");
                    break;
                case 7:
                    strcat(partName, "_ig5");
                    break;
                default:
                    strcat(partName, "_unk");
                    break;
                }

                en = arti->GetDataEntry(ID_VERTEX, lev);
                if (en == NULL)
                    continue;
                CVector4* deVt = (CVector4*) en->GetData();
                tVector4* vt   = deVt->GetItem(0);

                en             = arti->GetDataEntry(ID_NORMAL, lev);
                CVector4* deNm = NULL;
                tVector4* nm   = NULL;
                if (en != NULL) {
                    deNm = (CVector4*) en->GetData();
                    nm   = deNm->GetItem(0);
                }

                CVector2* deUv = NULL;
                tVector2* uv   = NULL;
                en             = arti->GetDataEntry(ID_UV, lev);
                if (en != NULL) {
                    deUv = (CVector2*) en->GetData();
                    uv   = deUv->GetItem(0);
                }

                int maxPr   = -1;
                int vtCount = deVt->GetCount();
                int inCount = 0;

                CEntry* enPr = NULL;
                do {
                    enPr = arti->GetPartEntry(lev, maxPr + 1);
                    if (enPr != NULL) {
                        maxPr++;
                        inCount += enPr->GetCount();
                    }
                } while (enPr != NULL);

                // Lets build an ONFS CarModel from CRP data
                std::vector<uint32_t> indices;
                std::vector<uint32_t> polygonFlags;
                std::vector<glm::vec3> vertices;
                std::vector<glm::vec3> normals;
                std::vector<glm::vec2> uvs;

                // TODO: Set center properly
                std::string part_name(partName);
                glm::vec3 center(0, 0, 0);
                float specularDamper       = 0.2f;
                float specularReflectivity = 0.02f;
                float envReflectivity      = 0.4f;

                for (int j = 0; j < vtCount; j++) {
                    vertices.emplace_back(glm::vec3(vt[j].x / 5, vt[j].y / 5, vt[j].z / 5));
                    if (nm != NULL) {
                        normals.emplace_back(glm::vec3(nm[j].x, nm[j].y, nm[j].z));
                    }
                }

                CEntry* tr = (CEntry*) arti->GetDataEntry(ID_TRANSFORM, lev);
                if (tr != NULL) {
                    CMatrix* deTr = (CMatrix*) tr->GetData();
                    glm::mat4 mat = glm::make_mat4(deTr->GetValues());
                    center        = mat[3];
                    center /= 5;
                }

                int k = 0, l = 0;
                bool doNotAdd = false;
                for (int j = 0; j <= maxPr; j++) {
                    enPr      = arti->GetPartEntry(lev, j);
                    CPart* pr = (CPart*) enPr->GetData();

                    if (pr->FindIndex(ID_INDEX_VERTEX) == -1) {
                        doNotAdd = true;
                        break;
                    }

                    CEntry* enMt  = crp->GetMisc(ID_MATERIAL, pr->GetMaterial());
                    CMaterial* mt = (CMaterial*) enMt->GetData();
                    if (mt->GetTpgIndex() < 4) {
                        LOG(INFO) << partName << " TP: " << mt->GetTpgIndex() << " TN: " << enMt->GetIndex();
                    }

                    int vtOff = pr->GetIndex(pr->FindIndex(ID_INDEX_VERTEX))->Offset;
                    int vtAdj = pr->GetInfo(pr->FindInfo(ID_INFO_VERTEX))->GetOffsetIndex();

                    int uvOff = 0;
                    int uvAdj = 0;
                    if (uv != NULL) {
                        uvOff = pr->GetIndex(pr->FindIndex(ID_INDEX_UV))->Offset;
                        uvAdj = pr->GetInfo(pr->FindInfo(ID_INFO_UV))->GetOffsetIndex();
                    }

                    unsigned char* indicesRaw = pr->GetIndices(0);

                    for (k = 0; k < (enPr->GetCount() / 3); k++, l++) {
                        polygonFlags.emplace_back(0);
                        polygonFlags.emplace_back(0);
                        polygonFlags.emplace_back(0);
                        indices.emplace_back(indicesRaw[vtOff + k * 3 + 0] + vtAdj);
                        indices.emplace_back(indicesRaw[vtOff + k * 3 + 1] + vtAdj);
                        indices.emplace_back(indicesRaw[vtOff + k * 3 + 2] + vtAdj);
                        if (uv != NULL) {
                            uvs.emplace_back(glm::vec2(uv[indicesRaw[uvOff + k * 3 + 0] + uvAdj].u, uv[indicesRaw[uvOff + k * 3 + 0] + uvAdj].v));
                            uvs.emplace_back(glm::vec2(uv[indicesRaw[uvOff + k * 3 + 1] + uvAdj].u, uv[indicesRaw[uvOff + k * 3 + 1] + uvAdj].v));
                            uvs.emplace_back(glm::vec2(uv[indicesRaw[uvOff + k * 3 + 2] + uvAdj].u, uv[indicesRaw[uvOff + k * 3 + 2] + uvAdj].v));
                        }
                    }
                }

                // hide all driver parts
                // if (((LEVELINDEX_LEVEL(base->GetBaseInfo()->LevelIndex))&BIL_DRIVER)!=0)
                //	pObj->Hide(true);
                // show only level(0) part and typeindex=0 parts
                // else

                if (!doNotAdd)
                    carData.meshes.emplace_back(CarModel(part_name, vertices, uvs, normals, indices, polygonFlags, center, specularDamper, specularReflectivity, envReflectivity));

                if (v != 0 || base->GetBaseInfo()->TypeIndex != 0)
                    carData.meshes.back().enabled = false;
            }
        } else {
            CEffect* ef   = (CEffect*) enEf->GetData();
            CMatrix* enTr = (CMatrix*) arti->GetSubEntry(ID_TRANSFORM)->GetData();

            /*tMATRIX mat = ConvertMatrix(ef->GetTransform());

            if (enTr!=NULL) {
                tMATRIX mat2 = ConvertMatrix(enTr);
                mat._41 += mat2._41;
                mat._42 += mat2._42;
                mat._43 += mat2._43;
            }

            tObject* pObj = CreatePyramid(mat._41, mat._42, mat._43, partNameOrig, false);
            Objects->AddObject(pObj);
            delete pObj;

            if (ef->GetMirrored()) {
                char partNameM[100];
                sprintf(partNameM, "%s_M", partNameOrig);
                pObj = CreatePyramid(-mat._41, mat._42, mat._43, partNameM, false);
                Objects->AddObject(pObj);
                delete pObj;
            }*/
        }
    }

    // blah!
    // create parts for bound planes
    {
        CEntry* en = NULL;
        int loc    = crp->FindMisc(ID_BPLANES);
        if (loc > -1) {
            en = crp->GetMisc(loc++);
            while (en && en->GetId() == ID_BPLANES) {
                CBPlanes* bplane = (CBPlanes*) en->GetData();
                /*tObject* pObj = new tObject("BoundPlane", bplane->GetCount()*4, bplane->GetCount()*2);
                tVector4* vt = bplane->GetVertices();
                for (int i=0; i < bplane->GetCount(); i++) {
                    pObj->VertTable->Table[i*4+0] = tNormalVertex(vt[i*4+0].x,vt[i*4+0].y,vt[i*4+0].z,0.0f,0.0f,0.0f, -1);
                    pObj->VertTable->Table[i*4+1] = tNormalVertex(vt[i*4+1].x,vt[i*4+1].y,vt[i*4+1].z,0.0f,0.0f,0.0f, -1);
                    pObj->VertTable->Table[i*4+2] = tNormalVertex(vt[i*4+2].x,vt[i*4+2].y,vt[i*4+2].z,0.0f,0.0f,0.0f, -1);
                    pObj->VertTable->Table[i*4+3] = tNormalVertex(vt[i*4+3].x,vt[i*4+3].y,vt[i*4+3].z,0.0f,0.0f,0.0f, -1);
                    pObj->FaceTable->Table[i*2+0] = tFace(i*4+0,i*4+1,i*4+3);
                    pObj->FaceTable->Table[i*2+1] = tFace(i*4+1,i*4+2,i*4+3);
                }
                pObj->Hide(true);
                Objects->AddObject(pObj);
                delete pObj;*/
                en = crp->GetMisc(loc++);
            }
        }
    }

    // and for Wndo
    {
        CEntry* en = NULL;
        int loc    = crp->FindMisc((ENTRY_ID) 0x576E646F);
        if (loc > -1) {
            en = crp->GetMisc(loc++);

            CEntry en2(ID_BPLANES, 0);
            en2.SetCount(4);
            en2.SetLength(8 + 16 * 4);
            CBPlanes* bplane = new CBPlanes();
            en2.SetData(bplane);

            CRawData* raw = (CRawData*) en->GetData();
            /*raw->ParseTo(&en2);

            tObject* pObj = new tObject("Wndo", bplane->GetCount()*4, bplane->GetCount()*2);
            tVector4* vt = bplane->GetVertices();
            for (int i=0; i < bplane->GetCount(); i++) {
                pObj->VertTable->Table[i*4+0] = tNormalVertex(vt[i*4+0].x,vt[i*4+0].y,vt[i*4+0].z,0.0f,0.0f,0.0f, -1);
                pObj->VertTable->Table[i*4+1] = tNormalVertex(vt[i*4+1].x,vt[i*4+1].y,vt[i*4+1].z,0.0f,0.0f,0.0f, -1);
                pObj->VertTable->Table[i*4+2] = tNormalVertex(vt[i*4+2].x,vt[i*4+2].y,vt[i*4+2].z,0.0f,0.0f,0.0f, -1);
                pObj->VertTable->Table[i*4+3] = tNormalVertex(vt[i*4+3].x,vt[i*4+3].y,vt[i*4+3].z,0.0f,0.0f,0.0f, -1);
                pObj->FaceTable->Table[i*2+0] = tFace(i*4+0,i*4+1,i*4+3);
                pObj->FaceTable->Table[i*2+1] = tFace(i*4+1,i*4+2,i*4+3);
            }
            pObj->Hide(false);
            Objects->AddObject(pObj);
            delete pObj;*/
        }
    }

    delete crp;

    return carData;
}

void NFS5::DumpCrpTextures(const std::string& crpPath) {
    LOG(INFO) << "Dumping FSH texture packs located in " << crpPath;

    std::ifstream crp(crpPath, std::ios::in | std::ios::binary);

    auto* crpFileHeader = new CRP::HEADER();
    ASSERT(crp.read((char*) crpFileHeader, sizeof(CRP::HEADER)).gcount() == sizeof(CRP::HEADER), "Couldn't open file/truncated");

    // Each entry here points to a part table
    auto* articleTable = new CRP::ARTICLE[crpFileHeader->headerInfo.getNumParts()];
    crp.read((char*) articleTable, sizeof(CRP::ARTICLE) * crpFileHeader->headerInfo.getNumParts());

    std::streamoff articleTableEnd = crp.tellg();

    // Work out whether we're parsing a MISC_PART, MATERIAL_PART or FSH_PART. Read into generic table then sort.
    auto* miscPartTable = new CRP::GENERIC_PART[crpFileHeader->nMiscData];
    crp.read((char*) miscPartTable, sizeof(CRP::GENERIC_PART) * crpFileHeader->nMiscData);

    std::vector<CRP::FSH_PART> fshParts;

    for (uint32_t miscPartIdx = 0; miscPartIdx < crpFileHeader->nMiscData; ++miscPartIdx) {
        uint32_t currentCrpOffset = static_cast<uint32_t>(articleTableEnd + (miscPartIdx * 16));

        switch (miscPartTable[miscPartIdx].getPartType()) {
        case CRP::MiscPart:
        case CRP::MaterialPart:
            continue;
        case CRP::FshPart:
            miscPartTable[miscPartIdx].fshPart.offset += currentCrpOffset;
            fshParts.emplace_back(miscPartTable[miscPartIdx].fshPart);
            break;
        default:
            ASSERT(false, "Unknown miscellaneous part type in CRP misc part table!");
        }
    }
    // Clean up
    delete[] articleTable;
    delete[] miscPartTable;
    delete crpFileHeader;

    // Lets dump the FSH's quickly so I can be sure this parser is coming along well
    for (auto& fshPart : fshParts) {
        // Build the output FSH file path
        std::stringstream fshPath, fshOutputPath;
        boost::filesystem::path outputPath(crpPath);
        fshPath << outputPath.parent_path().string() << "/" << outputPath.filename().replace_extension("").string() << fshPart.index << ".fsh";
        fshOutputPath << outputPath.parent_path().string() << "/textures/" << outputPath.filename().replace_extension("").string() << fshPart.index << "/";

        // Lets go dump that fsh..
        char* fileBuffer = new char[fshPart.lengthInfo.getLength()];

        // Read it in
        crp.seekg(fshPart.offset, std::ios::beg);
        crp.read(fileBuffer, fshPart.lengthInfo.getLength());

        // Dump it out
        std::ofstream fsh(fshPath.str(), std::ios::out | std::ios::binary);
        fsh.write(fileBuffer, fshPart.lengthInfo.getLength());
        fsh.close();
        delete[] fileBuffer;

        // And lets extract that badboy
        ImageLoader::ExtractQFS(fshPath.str(), fshOutputPath.str());
    }
    crp.close();
    LOG(INFO) << "Done";
}

// Debug
void NFS5::DumpArticleVertsToObj(CRP::ARTICLE_DATA article) {
    for (uint32_t vertexTableIdx = 0; vertexTableIdx < article.vertPartTableData.size(); ++vertexTableIdx) {
        std::ofstream obj;
        std::stringstream objPath;
        objPath << "./assets/nfs5_test/gt1_crp_arti_" << article.index << "_vpart_" << vertexTableIdx << ".obj";
        obj.open(objPath.str());

        // Print Part name
        obj << "o "
            << "NFS5_Test_VertPart_" << vertexTableIdx << std::endl;
        // And then the verts
        for (uint32_t vertIdx = 0; vertIdx < article.vertPartTableData[vertexTableIdx].size(); ++vertIdx) {
            obj << "v " << article.vertPartTableData[vertexTableIdx][vertIdx].x << " " << article.vertPartTableData[vertexTableIdx][vertIdx].y << " "
                << article.vertPartTableData[vertexTableIdx][vertIdx].z << std::endl;
        }
        obj.close();
    }
}