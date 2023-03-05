#include "RaceNetRenderer.h"

RaceNetRenderer::RaceNetRenderer(const std::shared_ptr<GLFWwindow> &window, const std::shared_ptr<Logger> &onfs_logger) : m_window(window), logger(onfs_logger) {
    projectionMatrix = glm::ortho(minX, maxX, minY, maxY, -1.0f, 1.0f);

    /*------- ImGui -------*/
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window.get(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
}

void RaceNetRenderer::Render(uint32_t tick, std::vector<TrainingAgent> &carList, std::shared_ptr<Track> &trackToRender) {
    raceNetShader.HotReload(); // Racenet shader hot reload
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glfwPollEvents();
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // ImGui restore state
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    RescaleUI();
    std::vector<int> visibleTrackBlocks = GetVisibleTrackBlocks(trackToRender);

    // Only bother to render cars if the track is visible
    if (!visibleTrackBlocks.empty()) {
        raceNetShader.use();
        raceNetShader.loadProjectionMatrix(projectionMatrix);
        // Draw TrackModel
        raceNetShader.loadColor(glm::vec3(0.f, 0.5f, 0.5f));
        for (auto &visibleTrackBlockID : visibleTrackBlocks) {
            for (auto &track_block_entity : trackToRender->trackBlocks[visibleTrackBlockID].track) {
                raceNetShader.loadTransformationMatrix(std::get<TrackModel>(track_block_entity.raw).ModelMatrix);
                std::get<TrackModel>(track_block_entity.raw).render();
            }
        }

        // Draw Cars
        for (auto &car_agent : carList) {
            std::swap(car_agent.vehicle->carBodyModel.position.y, car_agent.vehicle->carBodyModel.position.z);
            std::swap(car_agent.vehicle->carBodyModel.orientation.y, car_agent.vehicle->carBodyModel.orientation.z);
            car_agent.vehicle->carBodyModel.update();

            raceNetShader.loadColor(car_agent.vehicle->vehicleProperties.colour);
            raceNetShader.loadTransformationMatrix(car_agent.vehicle->carBodyModel.ModelMatrix);
            car_agent.vehicle->carBodyModel.render();
        }

        raceNetShader.unbind();
    }

    // Draw some useful info
    ImGui::Text("Tick %d", tick);
    ImGui::Text("Name Fitness Vroad AvgSpeed");
    for (auto &carAgent : carList) {
        ImGui::Text("%s %d %d %f", carAgent.name.c_str(), carAgent.fitness, carAgent.ticksInsideVroad, carAgent.averageSpeed);
    }

    // Draw Logger UI
    logger->onScreenLog.Draw("ONFS Log");

    // Render UI and frame
    int display_w, display_h;
    glfwGetFramebufferSize(m_window.get(), &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(m_window.get());
}

std::vector<int> RaceNetRenderer::GetVisibleTrackBlocks(shared_ptr<Track> &track_to_render) {
    std::vector<int> activeTrackBlockIds;

    for (auto &track_block : track_to_render->trackBlocks) {
        if ((track_block.position.x > minX) && (track_block.position.x < maxX) && (track_block.position.z < minY) && (track_block.position.z > maxY)) {
            activeTrackBlockIds.emplace_back(track_block.id);
        }
    }

    return activeTrackBlockIds;
}

void RaceNetRenderer::RescaleUI() {
    if (ImGui::IsAnyItemActive())
        return;

    // Get mouse movement and compute new projection matrix with it
    static float prevZoomLevel = ImGui::GetIO().MouseWheel * 4.0f;
    float currentZoomLevel     = ImGui::GetIO().MouseWheel * 4.0f;

    // If panning, update projection matrix
    if (ImGui::GetIO().MouseDown[0]) {
        float xChange = ImGui::GetIO().MouseDelta.x;
        float yChange = ImGui::GetIO().MouseDelta.y;

        minX -= xChange * 0.5f;
        maxX -= xChange * 0.5f;
        minY -= yChange * 0.5f;
        maxY -= yChange * 0.5f;

        projectionMatrix = glm::ortho(minX, maxX, minY, maxY, -1.0f, 1.0f);
    }

    // If scrolling, update projection matrix
    if (prevZoomLevel != currentZoomLevel) {
        prevZoomLevel = currentZoomLevel;

        minX += currentZoomLevel;
        maxX -= currentZoomLevel;
        minY -= (currentZoomLevel * 0.5625f);
        maxY += (currentZoomLevel * 0.5625f);

        projectionMatrix = glm::ortho(minX, maxX, minY, maxY, -1.0f, 1.0f);
    }
}

RaceNetRenderer::~RaceNetRenderer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
