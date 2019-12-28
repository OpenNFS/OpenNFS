#include "Utils.h"

namespace Utils
{
    float RandomFloat(float min, float max)
    {
        static std::mt19937 mt(std::random_device{}());
        std::uniform_real_distribution<double> fdis(min, max);

        return static_cast<float>(fdis(mt));
    }

    glm::vec3 bulletToGlm(const btVector3 &v)
    { return glm::vec3(v.getX(), v.getY(), v.getZ()); }

    btVector3 glmToBullet(const glm::vec3 &v)
    { return btVector3(v.x, v.y, v.z); }

    glm::quat bulletToGlm(const btQuaternion &q)
    {
        return glm::quat(q.getW(), q.getX(), q.getY(), q.getZ());
    }

    btQuaternion glmToBullet(const glm::quat &q)
    { return btQuaternion(q.x, q.y, q.z, q.w); }

    btMatrix3x3 glmToBullet(const glm::mat3 &m)
    {
        return btMatrix3x3(m[0][0], m[1][0], m[2][0], m[0][1], m[1][1], m[2][1], m[0][2], m[1][2], m[2][2]);
    }

    // btTransform does not contain a full 4x4 matrix, so this transform is lossy.
    // Affine transformations are OK but perspective transformations are not.
    btTransform glmToBullet(const glm::mat4 &m)
    {
        glm::mat3 m3(m);
        return btTransform(glmToBullet(m3), glmToBullet(glm::vec3(m[3][0], m[3][1], m[3][2])));
    }

    glm::mat4 bulletToGlm(const btTransform &t)
    {
        glm::mat4 m = glm::mat4();
        const btMatrix3x3 &basis = t.getBasis();
        // rotation
        for (int r = 0; r < 3; r++)
        {
            for (int c = 0; c < 3; c++)
            {
                m[c][r] = basis[r][c];
            }
        }
        // traslation
        btVector3 origin = t.getOrigin();
        m[3][0] = origin.getX();
        m[3][1] = origin.getY();
        m[3][2] = origin.getZ();
        // unit scale
        m[0][3] = 0.0f;
        m[1][3] = 0.0f;
        m[2][3] = 0.0f;
        m[3][3] = 1.0f;
        return m;
    }

    DimensionData GenDimensions(std::vector<glm::vec3> vertices)
    {
        DimensionData modelDimensions = {};
        modelDimensions.maxVertex = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
        modelDimensions.minVertex = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);

        for (auto &vertex : vertices)
        {
            modelDimensions.minVertex.x = glm::min(modelDimensions.minVertex.x, vertex.x);
            modelDimensions.minVertex.y = glm::min(modelDimensions.minVertex.y, vertex.y);
            modelDimensions.minVertex.z = glm::min(modelDimensions.minVertex.z, vertex.z);

            modelDimensions.maxVertex.x = glm::max(modelDimensions.maxVertex.x, vertex.x);
            modelDimensions.maxVertex.y = glm::max(modelDimensions.maxVertex.y, vertex.y);
            modelDimensions.maxVertex.z = glm::max(modelDimensions.maxVertex.z, vertex.z);
        }

        return modelDimensions;
    }

    uint32_t SwapEndian(uint32_t x)
    {
        int swapped;

        swapped = (x >> 24) |
                  ((x << 8) & 0x00FF0000) |
                  ((x >> 8) & 0x0000FF00) |
                  (x << 24);

        return static_cast<uint32_t>(swapped);
    }

    glm::vec3 FixedToFloat(glm::vec3 fixedPoint)
    {
        return fixedPoint / 65536.0f;
    }

    // Move these to a native resource handler class
    std::vector<CarModel> LoadOBJ(std::string obj_path)
    {
        std::vector<CarModel> meshes;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;
        std::string warn;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_path.c_str(), nullptr, true, true))
        {
            LOG(WARNING) << err;
            return meshes;
        }
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++)
        {
            std::vector<glm::vec3> verts = std::vector<glm::vec3>();
            std::vector<glm::vec3> norms = std::vector<glm::vec3>();
            std::vector<glm::vec2> uvs = std::vector<glm::vec2>();
            std::vector<unsigned int> indices = std::vector<unsigned int>();
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
            {
                int fv = shapes[s].mesh.num_face_vertices[f];
                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++)
                {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    indices.emplace_back((const unsigned int &) idx.vertex_index);

                    verts.emplace_back(glm::vec3(attrib.vertices[3 * idx.vertex_index + 0] * 0.1,
                                                 attrib.vertices[3 * idx.vertex_index + 1] * 0.1,
                                                 attrib.vertices[3 * idx.vertex_index + 2] * 0.1));
                    norms.emplace_back(
                            glm::vec3(attrib.normals[3 * idx.normal_index + 0], attrib.normals[3 * idx.normal_index + 1],
                                      attrib.normals[3 * idx.normal_index + 2]));
                    uvs.emplace_back(glm::vec2(attrib.texcoords[2 * idx.texcoord_index + 0],
                                               1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]));
                }
                index_offset += fv;
                // per-face material
                shapes[s].mesh.material_ids[f];
            }
            CarModel obj_mesh = CarModel(shapes[s].name + "_obj", verts, uvs, norms, indices, glm::vec3(0, 0, 0), 0.01f, 0.0f, 0.5f);
            meshes.emplace_back(obj_mesh);
        }
        return meshes;
    }

    bool ExtractVIV(const std::string &viv_path, const std::string &output_dir)
    {
        LOG(INFO) << "Extracting VIV file: " << viv_path << " to " << output_dir;
        std::ifstream viv(viv_path, std::ios::in | std::ios::binary);
        ASSERT(viv.is_open(), "Couldn't open viv file at " << viv_path);

        if (boost::filesystem::exists(output_dir))
        {
            LOG(INFO) << "VIV has already been extracted. Skipping.";
            return true;
        }
        else
        {
            boost::filesystem::create_directories(output_dir);
        }

        char vivHeader[4];
        viv.read((char *) vivHeader, sizeof(vivHeader));
        if (memcmp(vivHeader, "BIGF", sizeof(vivHeader)))
        {
            LOG(WARNING) << "Not a valid VIV file (BIGF header missing)";
            viv.close();
            return false;
        }

        uint32_t vivSize;
        viv.read((char *) &vivSize, sizeof(uint32_t));
        vivSize = SwapEndian(vivSize);

        uint32_t nFiles;
        viv.read((char *) &nFiles, sizeof(uint32_t));
        nFiles = SwapEndian(nFiles);
        LOG(INFO) << "VIV contains " << nFiles << " files";

        uint32_t startPos;
        viv.read((char *) &startPos, sizeof(uint32_t));
        startPos = SwapEndian(startPos);

        std::streampos currentPos = viv.tellg();

        for (uint8_t fileIdx = 0; fileIdx < nFiles; ++fileIdx)
        {
            viv.seekg(currentPos, std::ios_base::beg);
            uint32_t filePos = 0, fileSize = 0;
            viv.read((char *) &filePos, sizeof(uint32_t));
            viv.read((char *) &fileSize, sizeof(uint32_t));
            filePos = SwapEndian(filePos);
            fileSize = SwapEndian(fileSize);

            char fileName[100];
            int pos = 0;
            char c = ' ';
            viv.read(&c, sizeof(char));
            while (c != '\0')
            {
                fileName[pos] = c;
                pos++;
                viv.read(&c, sizeof(char));
            }
            fileName[pos] = '\0';

            currentPos = viv.tellg();

            std::stringstream out_file_path;
            out_file_path << output_dir << fileName;
            std::ofstream out(out_file_path.str(), std::ios::out | std::ios::binary);
            if (!out.is_open())
            {
                LOG(WARNING) << "Error while creating output file " << fileName;
                viv.close();
                return false;
            }

            viv.seekg(filePos, std::ios_base::beg);
            char *fileBuffer = new char[fileSize];
            viv.read(fileBuffer, fileSize);
            out.write(fileBuffer, fileSize);
            delete[] fileBuffer;
            out.close();
            LOG(INFO) << "File " << fileName << " was written successfully";
        }
        viv.close();

        return true;
    }

    // Modified Arushan CRP decompresser. Removes LZ77 style decompression from CRPs
    bool DecompressCRP(const std::string &compressedCrpPath, const std::string &decompressedCrpPath)
    {
        LOG(INFO) << "Decompressing CRP File located at " << compressedCrpPath;

        // Bail early if decompressed CRP present already
        if (boost::filesystem::exists(decompressedCrpPath))
        {
            LOG(INFO) << "Already decompressed. Skipping.";
            return true;
        }

        const char *filename = compressedCrpPath.c_str();

        // Open file
        std::ifstream file;
        file.open(filename, std::ios::binary);
        ASSERT(file.is_open(), "Unable to open CRP at " << compressedCrpPath << " for decompression!");

        // Check valid file length
        file.seekg(0, std::ios::end);
        unsigned int filesize = file.tellg();
        file.seekg(0);
        ASSERT(filesize > 0x10, "CRP at " << compressedCrpPath << " has invalid file size");

        //Initialization
        unsigned int length = 0;
        unsigned char *data = NULL;
        //CRP compression type
        unsigned int id = 0;
        file.read((char *) &id, 4);
        //Uncompressed CRP
        if ((id & 0x0000FFFF) != 0xFB10)
        {
            file.close();
            LOG(INFO) << "CRP is already decompressed. Skipping.";
            boost::filesystem::copy_file(compressedCrpPath, decompressedCrpPath,
                                         boost::filesystem::copy_option::overwrite_if_exists);
            return true;
        }
            //Compressed CRP
        else
        {
            //Create uncompressed data array
            file.seekg(2);
            unsigned int elhi, elmd, ello;
            elhi = elmd = ello = 0;
            file.read((char *) &elhi, 1);
            file.read((char *) &elmd, 1);
            file.read((char *) &ello, 1);
            length = (((elhi * 256) + elmd) * 256) + ello;
            data = new unsigned char[length];
            //Memory allocation
            ASSERT(data != NULL, "Unable to allocate buffer for decompressed CRP data! Possible invalid length read");
            //Initialization
            file.seekg(5);
            unsigned int datapos, len, offset, inbyte, tmp1, tmp2, tmp3;
            unsigned char *srcpos, *dstpos;
            datapos = len = offset = inbyte = tmp1 = tmp2 = tmp3 = 0;
            file.read((char *) &inbyte, 1);
            //Decompress
            while ((!file.eof()) && (inbyte < 0xFC))
            {
                if (!(inbyte & 0x80))
                {
                    file.read((char *) &tmp1, 1);
                    len = inbyte & 0x03;
                    if (len != 0)
                    {
                        file.read((char *) (data + datapos), len);
                        datapos += len;
                    }
                    len = ((inbyte & 0x1C) >> 2) + 3;
                    if (len != 0)
                    {
                        offset = ((inbyte >> 5) << 8) + tmp1 + 1;
                        dstpos = data + datapos;
                        srcpos = data + datapos - offset;
                        datapos += len;
                        while (len--) *dstpos++ = *srcpos++;
                    }
                }
                else if (!(inbyte & 0x40))
                {
                    file.read((char *) &tmp1, 1);
                    file.read((char *) &tmp2, 1);
                    len = (tmp1 >> 6) & 0x03;
                    if (len != 0)
                    {
                        file.read((char *) (data + datapos), len);
                        datapos += len;
                    }
                    len = (inbyte & 0x3F) + 4;
                    if (len != 0)
                    {
                        offset = ((tmp1 & 0x3F) * 256) + tmp2 + 1;
                        srcpos = data + datapos - offset;
                        dstpos = data + datapos;
                        datapos += len;
                        while (len--) *dstpos++ = *srcpos++;
                    }
                }
                else if (!(inbyte & 0x20))
                {
                    file.read((char *) &tmp1, 1);
                    file.read((char *) &tmp2, 1);
                    file.read((char *) &tmp3, 1);
                    len = inbyte & 0x03;
                    if (len != 0)
                    {
                        file.read((char *) (data + datapos), len);
                        datapos += len;
                    }
                    len = (((inbyte >> 2) & 0x03) * 256) + tmp3 + 5;
                    if (len != 0)
                    {
                        offset = ((inbyte & 0x10) << 0x0C) + (tmp1 * 256) + tmp2 + 1;
                        srcpos = data + datapos - offset;
                        dstpos = data + datapos;
                        datapos += len;
                        while (len--) *dstpos++ = *srcpos++;
                    }
                }
                else
                {
                    len = ((inbyte & 0x1F) * 4) + 4;
                    if (len != 0)
                    {
                        file.read((char *) (data + datapos), len);
                        datapos += len;
                    }
                }
                inbyte = tmp1 = tmp2 = tmp3 = 0;
                file.read((char *) &inbyte, 1);
            }
            if ((!file.eof()) && (datapos < length))
            {
                len = inbyte & 0x03;
                if (len != 0)
                {
                    file.read((char *) (data + datapos), len);
                }
            }
        }
        //Clean up
        file.close();

        // Write out uncompressed data
        std::ofstream ofile;
        ofile.open(decompressedCrpPath.c_str(), std::ios::binary);
        ASSERT(ofile.is_open(),
               "Unable to open output CRP at " << decompressedCrpPath << " for write of decompressed data.");
        ofile.write((const char *) data, length);
        ofile.close();


        delete[] data;

        return true;
    }

    glm::vec3 HSLToRGB(glm::vec4 hsl)
    {
        float H = hsl.x / 255.f;
        float S = hsl.y / 255.f;
        float L = hsl.z / 255.f;

        glm::vec3 rgb;

        ImGui::ColorConvertHSVtoRGB(H, S, L, rgb.x, rgb.y, rgb.z);

        return rgb;
    }

    glm::vec3 ParseRGBString(const std::string &rgb_string)
    {
        std::stringstream tempComponent;
        uint8_t commaCount = 0;
        glm::vec3 rgbValue;

        for (int char_Idx = 0; char_Idx < rgb_string.length(); ++char_Idx)
        {
            if (rgb_string[char_Idx] == ',')
            {
                switch (commaCount)
                {
                    case 0:
                        rgbValue.x = (float) stoi(tempComponent.str());
                        break;
                    case 1:
                        rgbValue.y = (float) stoi(tempComponent.str());
                        break;
                    case 2:
                        rgbValue.z = (float) stoi(tempComponent.str());
                        break;
                    default:
                        ASSERT(false, "Attempted to parse 4 component RGB");
                        break;
                }
                tempComponent.str("");
                if (++commaCount >= 3)
                { break; }
            }
            else
            {
                tempComponent << rgb_string[char_Idx];
            }
        }

        return rgbValue;
    }

    glm::vec3 CalculateQuadNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4)
    {
        glm::vec3 triANormal = CalculateNormal(p1, p2, p3);
        glm::vec3 triBNormal = CalculateNormal(p1, p3, p4);
        return glm::normalize(triANormal + triBNormal);
    }

    glm::vec3 CalculateNormal(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
    {
        glm::vec3 vertexNormal(0, 0, 0);

        glm::vec3 U = p2 - p1;
        glm::vec3 V = p3 - p1;

        vertexNormal.x = (U.y * V.z) - (U.z * V.y);
        vertexNormal.y = (U.z * V.x) - (U.x * V.z);
        vertexNormal.z = (U.x * V.y) - (U.y * V.x);

        return vertexNormal;
    }

    btTransform MakeTransform(glm::vec3 position, glm::quat orientation)
    {
        btTransform transform;
        transform.setOrigin(glmToBullet(position));
        transform.setRotation(glmToBullet(orientation));

        return transform;
    }
}


