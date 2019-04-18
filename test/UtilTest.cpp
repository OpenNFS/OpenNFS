#define TINYOBJLOADER_IMPLEMENTATION

#include "gtest/gtest.h"

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
        InitOpenGL();
    }

    virtual void TearDown() {
        glfwTerminate();
    }
private:
    GLFWwindow *window;
    static void glfwError(int id, const char *description) {
        LOG(WARNING) << description;
    }
    void InitOpenGL() {
        // Initialise GLFW
        ASSERT(glfwInit(), "GLFW Init failed.\n");
        glfwSetErrorCallback(&glfwError);
        // TODO: Disable MSAA for now until texture array adds padding
        glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Appease the OSX Gods
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
        // TODO: If we fail to create a GL context on Windows, fall back to not requesting any (Keiiko Bug #1)
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
        window = glfwCreateWindow(10, 10, "OpenNFS Test", nullptr, nullptr);
        if (window == nullptr) {
            LOG(WARNING) << "Failed to create a GLFW window.";
            getchar();
            glfwTerminate();
            GTEST_FAIL();
        }
        glfwMakeContextCurrent(window);
        // Initialize GLEW
        glewExperimental = GL_TRUE; // Needed for core profile
        if (glewInit() != GLEW_OK) {
            LOG(WARNING) << "Failed to initialize GLEW";
            getchar();
            glfwTerminate();
            GTEST_FAIL();
        }
        // Ensure we can capture the escape key being pressed below
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        // Set the mouse at the center of the screen
        glfwPollEvents();
        // Dark blue background
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        // Accept fragment if it closer to the camera than the former one
        glDepthFunc(GL_LESS);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
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
