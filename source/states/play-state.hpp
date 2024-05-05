#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>
#include <systems/collision-system.hpp>

// This state shows how to use the ECS framework and deserialization.
class Playstate: public our::State {

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::CollisionControllerSystem collisionSystem;
    int health;

    void onInitialize() override {
        
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
        auto& config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if(config.contains("assets")){
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our world
        if(config.contains("world")){
            world.deserialize(config["world"]);
        }
        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        collisionSystem.enter(getApp());
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
    }

    void onDraw(double deltaTime) override {
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        health = collisionSystem.update(&world, (float)deltaTime);
        // cout << "My health is: " << health << endl ;

        //Todo Later : When health = -1 end game
        // if (health == 10)
        //     getApp()->changeState("win");
        // else if (health == -1)
        //     getApp()->changeState("end");


        // And finally we use the renderer system to draw the scene
        renderer.render(&world);


        // Get a reference to the keyboard object
        auto& keyboard = getApp()->getKeyboard();

        if(keyboard.justPressed(GLFW_KEY_ESCAPE)){
            // If the escape  key is pressed in this frame, go to the play state
            getApp()->changeState("menu");
        }
    }

    void onImmediateGui() override
    {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        // Box 1
        ImGui::SetNextWindowPos(ImVec2(20, 20)); // Set position of Box 1
        ImGui::Begin("HealthName", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowSize(ImVec2(250, 80));
        ImGui::SetWindowFontScale(3.0f);
        ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 1.0), "Your Health: ");
        ImGui::End();

        // // Box 2
        ImGui::SetNextWindowPos(ImVec2(270, 20)); // Set position of Box 2
        ImGui::Begin("HealthValue", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowSize(ImVec2(100, 50));
        ImGui::SetWindowFontScale(3.0f);
        ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 1.0), std::to_string(health).c_str());
        ImGui::End();
        ImGui::PopStyleColor();
    }

    void onDestroy() override {
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