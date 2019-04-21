#define TINYOBJLOADER_IMPLEMENTATION

#include "gtest/gtest.h"

#include "../src/Renderer/Renderer.h"
#include "../src/Util/Logger.h"
#include "../src/Util/Utils.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <map>
#include <boost/filesystem.hpp>


class UtilTest : public testing::Test {
public:
    virtual void SetUp()
    {
        // Initialise Logging
        std::shared_ptr<Logger> logger = std::make_shared<Logger>();
        // And OpenGL
        GLFWwindow *window = Renderer::InitOpenGL(10, 10, "OpenNFS v" + ONFS_VERSION + " Test");
    }

    virtual void TearDown() {
        glfwTerminate();
    }
};

TEST_F(UtilTest, ExtractVIV){
    using namespace boost::filesystem;

    const std::string vivPath = "../resources/NFS_3/gamedata/carmodel/diab/car.viv";
    const std::string targetPath = "./test/assets/car/NFS_3/diab/";
    std::map<std::string, int> expectedVivContents {{"car.bnk", 217136},
                                                    {"car.fce", 152948},
                                                    {"car00.tga", 262188},
                                                    {"carp.txt", 5023},
                                                    {"dash.qfs", 516356},
                                                    {"fedata.bri", 687},
                                                    {"fedata.eng", 689},
                                                    {"fedata.fre", 630},
                                                    {"fedata.fsh", 131112},
                                                    {"fedata.ger", 683},
                                                    {"fedata.ita", 619},
                                                    {"fedata.spa", 692},
                                                    {"fedata.swe", 685},
                                                    {"ocar.bnk", 69528},
                                                    {"ocard.bnk", 69544},
                                                    {"scar.bnk", 152288}};

    // Attempt to extract the VIV file
    Utils::ExtractVIV(vivPath, targetPath);

    // Check the number of files extracted matches that which is expected
    const path vivExtractPath(targetPath);

    for (directory_iterator itr(vivExtractPath); itr != directory_iterator(); ++itr) {
        std::string vivFileName = itr->path().filename().string();
        // Ensure the file is present and filesize is correct
        ASSERT_EQ(expectedVivContents.count(vivFileName), 1) << "Extracted viv file " << vivFileName << " not expected";
        ASSERT_EQ(expectedVivContents[vivFileName], file_size(itr->path().string())) << "Extracted viv file " << vivFileName << " did not have expected file size";
    }
}
