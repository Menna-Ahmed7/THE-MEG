#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>
#include <systems/collision-system.hpp>

#include <iostream>
#include <unistd.h>
using namespace std;

// This state shows how to use the ECS framework and deserialization.
class Playstate : public our::State
{

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::CollisionControllerSystem collisionSystem;
    int health;
    bool grey_effect;

    void onInitialize() override {
        

        cout << "Test the state" << endl;
        std::string gameMode = "config/game.jsonc";
        std::ifstream file_in(gameMode);
        if (!file_in)
        {
            std::cerr << "Couldn't open file: " << gameMode << std::endl;
            return;
        }
        // Read the file into a json object then close the file
        nlohmann::json gameConfig = nlohmann::json::parse(file_in, nullptr, true, true);
        file_in.close();
        // First of all, we get the scene configuration from the app config
        auto &config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if (config.contains("assets"))
        {
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our world
        if (config.contains("world"))
        {
            world.deserialize(config["world"]);
        }
        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        collisionSystem.enter(getApp());
        // Then we initialize the renderer
        grey_effect = false;
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
    }

    void onDraw(double deltaTime) override
    {
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        health = collisionSystem.update(&world, (float)deltaTime);

        // Todo Later : When health = -1 end game
        if (health == 10){
            getApp()->changeState("win");
        } 
        else if (health == -1){
           getApp()->changeState("lose");

        }
            


        // And finally we use the renderer system to draw the scene
        renderer.render(&world, grey_effect);

        // Get a reference to the keyboard object
        auto &keyboard = getApp()->getKeyboard();

        if (keyboard.justPressed(GLFW_KEY_ESCAPE))
        {
            // If the escape  key is pressed in this frame, go to the play state
            getApp()->changeState("menu");
        }
    }

    void onImmediateGui() override
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        // Health Bar
        ImGui::SetNextWindowPos(ImVec2(20, 20)); // Set position of Health Bar
        ImGui::Begin("HealthBar", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowSize(ImVec2(250, 30));

        // Calculate health percentage
        float healthPercentage = static_cast<float>(health) / static_cast<float>(10.0f);

        // Draw health bar
        ImVec2 barSize = ImVec2(200, 20);                                              // Adjust the width of the health bar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red color for the health bar
        ImGui::ProgressBar(healthPercentage, barSize);
        ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleColor();
    }

    void onDestroy() override
    {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        // Clear the world
        world.clear();
        // and we delete all the loaded assets to free memory on the RAM and the VRAM
        our::clearAllAssets();
    }
};