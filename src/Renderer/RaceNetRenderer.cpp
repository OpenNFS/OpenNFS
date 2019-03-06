//
// Created by Amrik on 28/10/2018.
//

#include "RaceNetRenderer.h"

RaceNetRenderer::RaceNetRenderer(GLFWwindow *gl_window, std::shared_ptr<Logger> &onfs_logger) : window(gl_window), logger(onfs_logger) {
    projectionMatrix = glm::ortho(minX, maxX, minY, maxY, -1.0f, 1.0f);

    /*------- ImGui -------*/
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
}

void RaceNetRenderer::Render(uint32_t tick, std::vector<CarAgent> &carList, std::shared_ptr<ONFSTrack> &trackToRender) {
    raceNetShader.shaders.UpdatePrograms(); // Racenet shader hot reload
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
    if(!visibleTrackBlocks.empty()){
        raceNetShader.use();
        raceNetShader.loadProjectionMatrix(projectionMatrix);
        // Draw Track
        raceNetShader.loadColor(glm::vec3(0.f, 0.5f, 0.5f));
        for (auto &visibleTrackBlockID : visibleTrackBlocks) {
            for (auto &track_block_entity : trackToRender->trackBlocks[visibleTrackBlockID].track) {
                raceNetShader.loadTransformationMatrix(boost::get<Track>(track_block_entity.glMesh).ModelMatrix);
                boost::get<Track>(track_block_entity.glMesh).render();
            }
        }

        // Draw Cars
        for (auto &car_agent : carList) {
            std::swap(car_agent.car->carBodyModel.position.y, car_agent.car->carBodyModel.position.z);
            std::swap(car_agent.car->carBodyModel.orientation.y, car_agent.car->carBodyModel.orientation.z);
            car_agent.car->carBodyModel.update();

            raceNetShader.loadColor(car_agent.car->colour);
            raceNetShader.loadTransformationMatrix(car_agent.car->carBodyModel.ModelMatrix);
            car_agent.car->carBodyModel.render();
        }

        raceNetShader.unbind();
    }

    // Draw some useful info
    ImGui::Text("Tick %d", tick);
    for (auto &carAgent : carList) {
        ImGui::Text("%s %f %f %f %d", carAgent.name.c_str(), carAgent.car->carBodyModel.position.x, carAgent.car->carBodyModel.position.y, carAgent.car->carBodyModel.position.z, carAgent.fitness);
    }

    // Draw Logger UI
    logger->onScreenLog.Draw("ONFS Log");

    // Render UI and frame
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

std::vector<int> RaceNetRenderer::GetVisibleTrackBlocks(shared_ptr <ONFSTrack> &track_to_render){
    std::vector<int> activeTrackBlockIds;

    for(auto &track_block : track_to_render->trackBlocks){
        if((track_block.center.x > minX)&&(track_block.center.x < maxX)&&(track_block.center.z < minY)&&(track_block.center.z > maxY)){
            activeTrackBlockIds.emplace_back(track_block.blockId);
        }
    }

    return activeTrackBlockIds;
}

void RaceNetRenderer::RescaleUI(){
    if(ImGui::IsAnyItemActive()) return;

    // Get mouse movement and compute new projection matrix with it
    static float prevZoomLevel = ImGui::GetIO().MouseWheel * 4.0f;
    float currentZoomLevel     = ImGui::GetIO().MouseWheel * 4.0f;

    // If panning, update projection matrix
    if (ImGui::GetIO().MouseDown[0]) {
        float xChange = ImGui::GetIO().MouseDelta.x;
        float yChange = ImGui::GetIO().MouseDelta.y;

        minX -= xChange*0.5f;
        maxX -= xChange*0.5f;
        minY -= yChange*0.5f;
        maxY -= yChange*0.5f;

        projectionMatrix = glm::ortho(minX, maxX, minY, maxY, -1.0f, 1.0f);
    }

    // If scrolling, update projection matrix
    if(prevZoomLevel != currentZoomLevel){
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
