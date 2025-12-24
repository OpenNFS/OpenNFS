#ifdef VULKAN_BUILD
#define GLFW_INCLUDE_VULKAN
#include "Renderer/vkRenderer.h"
#endif

#include <LibOpenNFS.h>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "Config.h"
#include "GameState/GameStateManager.h"
#include "GameState/MainMenuState.h"
#include "GameState/RaceState.h"
#include "GameState/VehicleSelectionState.h"
#include "Renderer/Renderer.h"
#include "Util/Logger.h"

using namespace std::filesystem;
using namespace OpenNFS;

class OpenNFSEngine {
  public:
    explicit OpenNFSEngine(std::shared_ptr<Logger> const &onfs_logger) : logger(onfs_logger) {
        if (Config::get().renameAssets) {
            OpenNFS::Utils::RenameAssetsToLowercase();
        }
        InitDirectories();
        installedNFS = OpenNFS::Utils::PopulateAssets();

        if (Config::get().vulkanRender) {
#ifdef VULKAN_BUILD
            vkRenderer renderer;
            renderer.run();
#else
            CHECK_F(false, "This build of OpenNFS was not compiled with Vulkan support!");
#endif
        } else {
            run();
        }
    }

    void run() const {
        LOG(INFO) << "OpenNFS Version " << ONFS_VERSION;

        // Must initialise OpenGL here as the Loaders instantiate meshes which create VAO's
        std::shared_ptr<GLFWwindow> const window{Renderer::InitOpenGL(Config::get().resX, Config::get().resY, "OpenNFS v" + ONFS_VERSION)};

        // Create game context shared between all states
        GameContext context{
            .window = window,
            .logger = logger,
            .installedNFS = installedNFS,
            .loadedAssets = {get_enum(Config::get().carTag), Config::get().car, get_enum(Config::get().trackTag), Config::get().track}};

        CHECK_F(context.loadedAssets.trackTag != NFSVersion::UNKNOWN, "Unknown track type!");

        // Create and register game states
        GameStateManager stateManager;
        stateManager.RegisterState(GameState::MainMenu, std::make_unique<MainMenuState>(context));
        stateManager.RegisterState(GameState::Race, std::make_unique<RaceState>(context));
        stateManager.RegisterState(GameState::VehicleSelection, std::make_unique<VehicleSelectionState>(context));

        // Start with main menu
        stateManager.TransitionTo(GameState::MainMenu);

        // Main game loop
        double lastTime = glfwGetTime();

        while (!stateManager.ShouldExit() && !glfwWindowShouldClose(window.get())) {
            // Calculate delta time
            double const currentTime = glfwGetTime();
            auto const deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;

            Renderer::NewFrame();

            // Update current state
            stateManager.Update(deltaTime);

            Renderer::EndFrame();

            // Swap buffers
            glfwSwapBuffers(window.get());
            glfwPollEvents();
        }
        LOG(INFO) << "Exiting OpenNFS";

        // Close OpenGL window and terminate GLFW
        glfwTerminate();

        Renderer::Shutdown();
    }

  private:
    std::shared_ptr<Logger> logger;
    std::vector<NfsAssetList> installedNFS{};

    static void InitDirectories() {
        if (!(exists(OpenNFS::CAR_PATH))) {
            create_directories(OpenNFS::CAR_PATH);
        }
        if (!(exists(OpenNFS::TRACK_PATH))) {
            create_directories(OpenNFS::TRACK_PATH);
        }
    }
};

int main(int const argc, char **argv) {
    // Init the logger first, as used everywhere in ONFS
    auto const logger{std::make_shared<Logger>()};
    Config::get().InitFromCommandLine(argc, argv);

    // Pass through our g3log streams as callbacks to LibOpenNFS. This looks strange, but we define a lambda that
    // returns a std::function (the callback itself) to avoid duplicating broadly identical declarations for each log
    // level
    auto makeLogCallback{[=](auto &g3LogLevel) -> std::function<void(char const *, int, char const *, std::string)> {
        return [g3LogLevel](char const *file, int const line, char const *func, std::string const &logMessage) {
            LogCapture(file, line, func, g3LogLevel).stream() << logMessage;
        };
    }};

    // Map G3log levels to LibOpenNFS levels
    LibOpenNFS::RegisterLogCallback(LibOpenNFS::LogLevel::INFO, makeLogCallback(INFO));
    LibOpenNFS::RegisterLogCallback(LibOpenNFS::LogLevel::WARNING, makeLogCallback(WARNING));
    LibOpenNFS::RegisterLogCallback(LibOpenNFS::LogLevel::DEBUG, makeLogCallback(DEBUG));

    try {
        OpenNFSEngine game(logger);
    } catch (std::runtime_error const &e) {
        LOG(WARNING) << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
