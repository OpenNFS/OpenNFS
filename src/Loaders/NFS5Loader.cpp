#include "NFS5Loader.h"

using namespace Utils;

// CAR
std::shared_ptr<Car> NFS5::LoadCar(const std::string &carBasePath) {
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

    ASSERT(DecompressCRP(compressedCrpPath.str(), decompressedCrpPath.str()),
           "Unable to extract " << carBasePath << " to " << decompressedCrpPath.str());

    LoadCRP(decompressedCrpPath.str());
    //return std::make_shared<Car>();//LoadFCE(fce_path.str()), NFS_3, car_name);
}

CarData NFS5::LoadCRP(const std::string &crpPath) {
    LOG(INFO) << "Parsing CRP File located at " << crpPath;

    CarData carData;
    std::ifstream crp(crpPath, std::ios::in | std::ios::binary);

    auto *crpFileHeader = new CRP::HEADER();
    ASSERT(crp.read((char *) crpFileHeader, sizeof(CRP::HEADER)).gcount() == sizeof(CRP::HEADER),
           "Couldn't open file/truncated.");

    // Each entry here points to a part table
    auto *articleTable = new CRP::ARTICLE[crpFileHeader->headerInfo.getNumParts()];
    crp.read((char *) articleTable, sizeof(CRP::ARTICLE) * crpFileHeader->headerInfo.getNumParts());

    std::streamoff articleTableEnd = crp.tellg();

    // Work out whether we're parsing a MISC_PART, MATERIAL_PART or FSH_PART, then parse
    CRP::BASE_PART *miscPartTable = new CRP::BASE_PART[crpFileHeader->nMiscData];
    crp.read((char *) miscPartTable, sizeof(CRP::BASE_PART) * crpFileHeader->nMiscData);

    std::vector<CRP::MISC_PART> miscParts;
    std::vector<CRP::MATERIAL_PART> materialParts;
    std::vector<CRP::FSH_PART> fshParts;

    for (uint32_t miscPartIdx = 0; miscPartIdx < crpFileHeader->nMiscData; ++miscPartIdx) {
        uint32_t currentCrpOffset = articleTableEnd + (miscPartIdx * 16);

        switch (miscPartTable[miscPartIdx].getPartType()) {
            case CRP::MiscPart:
                // Add current offset to misc part offset for easy lookup later
                miscPartTable[miscPartIdx].miscPart.offset += currentCrpOffset;
                miscParts.emplace_back(miscPartTable[miscPartIdx].miscPart);
                break;
            case CRP::PartType::MaterialPart:
                miscPartTable[miscPartIdx].materialPart.offset += currentCrpOffset;
                materialParts.emplace_back(miscPartTable[miscPartIdx].materialPart);
                break;
            case CRP::FshPart:
                miscPartTable[miscPartIdx].fshPart.offset += currentCrpOffset;
                fshParts.emplace_back(miscPartTable[miscPartIdx].fshPart);
                break;
            default:
                ASSERT(false, "Unknown miscellaneous part type in CRP misc part table!");
        }
    }

    std::streamoff endRead = crp.tellg();

    // Lets dump the FSH's quickly so I can be sure this parser is coming along well
    for (auto &fshPart : fshParts) {
        // Build the output FSH file path
        std::stringstream fshPath, fshOutputPath;
        boost::filesystem::path outputPath(crpPath);
        fshPath << outputPath.parent_path().string() << "/" << outputPath.filename().replace_extension("").string() << fshPart.index << ".fsh";
        fshOutputPath << outputPath.parent_path().string() << "/textures/" << outputPath.filename().replace_extension("").string() << fshPart.index << "/";

        // Lets go dump that fsh..
        char *fileBuffer = new char[fshPart.lengthInfo.getLength()];

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

    LOG(INFO) << "OHNO";

    // Clean up
    delete[] articleTable;
    delete[] miscPartTable;
    delete crpFileHeader;
}

// Modified Arushan CRP decompresser. Removes LZ77 style decompression from CRPs
bool NFS5::DecompressCRP(const std::string &compressedCrpPath, const std::string &decompressedCrpPath) {
    LOG(INFO) << "Decompressing CRP File located at " << compressedCrpPath;

    // Bail early if decompressed CRP present already
    if (boost::filesystem::exists(decompressedCrpPath)) {
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
    if ((id & 0x0000FFFF) != 0xFB10) {
        file.close();
        LOG(INFO) << "CRP is already decompressed. Skipping.";
        boost::filesystem::copy_file(compressedCrpPath, decompressedCrpPath,
                                     boost::filesystem::copy_option::overwrite_if_exists);
        return true;
    }
        //Compressed CRP
    else {
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
        while ((!file.eof()) && (inbyte < 0xFC)) {
            if (!(inbyte & 0x80)) {
                file.read((char *) &tmp1, 1);
                len = inbyte & 0x03;
                if (len != 0) {
                    file.read((char *) (data + datapos), len);
                    datapos += len;
                }
                len = ((inbyte & 0x1C) >> 2) + 3;
                if (len != 0) {
                    offset = ((inbyte >> 5) << 8) + tmp1 + 1;
                    dstpos = data + datapos;
                    srcpos = data + datapos - offset;
                    datapos += len;
                    while (len--) *dstpos++ = *srcpos++;
                }
            } else if (!(inbyte & 0x40)) {
                file.read((char *) &tmp1, 1);
                file.read((char *) &tmp2, 1);
                len = (tmp1 >> 6) & 0x03;
                if (len != 0) {
                    file.read((char *) (data + datapos), len);
                    datapos += len;
                }
                len = (inbyte & 0x3F) + 4;
                if (len != 0) {
                    offset = ((tmp1 & 0x3F) * 256) + tmp2 + 1;
                    srcpos = data + datapos - offset;
                    dstpos = data + datapos;
                    datapos += len;
                    while (len--) *dstpos++ = *srcpos++;
                }
            } else if (!(inbyte & 0x20)) {
                file.read((char *) &tmp1, 1);
                file.read((char *) &tmp2, 1);
                file.read((char *) &tmp3, 1);
                len = inbyte & 0x03;
                if (len != 0) {
                    file.read((char *) (data + datapos), len);
                    datapos += len;
                }
                len = (((inbyte >> 2) & 0x03) * 256) + tmp3 + 5;
                if (len != 0) {
                    offset = ((inbyte & 0x10) << 0x0C) + (tmp1 * 256) + tmp2 + 1;
                    srcpos = data + datapos - offset;
                    dstpos = data + datapos;
                    datapos += len;
                    while (len--) *dstpos++ = *srcpos++;
                }
            } else {
                len = ((inbyte & 0x1F) * 4) + 4;
                if (len != 0) {
                    file.read((char *) (data + datapos), len);
                    datapos += len;
                }
            }
            inbyte = tmp1 = tmp2 = tmp3 = 0;
            file.read((char *) &inbyte, 1);
        }
        if ((!file.eof()) && (datapos < length)) {
            len = inbyte & 0x03;
            if (len != 0) {
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
