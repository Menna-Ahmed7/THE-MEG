#pragma once

#include "../ecs/world.hpp"
#include"../components/sharkComponent.hpp"
#include"../components/collision.hpp"
#include "../components/camera.hpp"
#include "../components/free-camera-controller.hpp"


#include "../application.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <iostream>
using namespace std;

namespace our
{

    // This system is added as a slightly complex example for how use the ECS framework to implement logic.
    // For more information, see "common/components/free-camera-controller.hpp"
    class RegulatorControllerSystem
    {
        Application *app;          // The application in which the state runs
        bool mouse_locked = false; // Is the mouse locked

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application *app)
        {
            this->app = app;
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent
        void update(World *world, float deltaTime)
        {
            SharkObject *sharkComponent = nullptr;
            CollisionComponent *collisionComponent = nullptr;
            for (auto entity : world->getEntities())
            {
                sharkComponent = entity->getComponent<SharkObject>();
                if(sharkComponent) break;
            }
            Entity *SharkEntity = sharkComponent->getOwner();
            glm::vec3 Sharkposition = glm::vec3(SharkEntity->getLocalToWorldMatrix() * glm::vec4(SharkEntity->localTransform.position, 1.0f));
            for(auto entity : world->getEntities()){
                collisionComponent = entity->getComponent<CollisionComponent>();
                if(!collisionComponent) continue;
                Entity *myentity = collisionComponent->getOwner();
                glm::vec3& CollisionEntityposition = entity->localTransform.position;
                if(abs(CollisionEntityposition.z) > 1000 && abs(CollisionEntityposition.z - Sharkposition.z) > 200){
                    cout << "out of range" << myentity->localTransform.position[2] << endl ;
                    myentity->localTransform.position[2] = -myentity->localTransform.position[2] ;
                    cout << "in range" << myentity->localTransform.position[2] << endl ;
                }
                if(abs(CollisionEntityposition.y) > 500 && abs(CollisionEntityposition.y - Sharkposition.y) > 200){
                    myentity->localTransform.position[1] = -myentity->localTransform.position[1] ;
                }
                if(abs(CollisionEntityposition.x) > 500 && abs(CollisionEntityposition.x - Sharkposition.x) > 200 ){
                    myentity->localTransform.position[0] = -myentity->localTransform.position[0] ;
                }
            }   
            return;
        }

        // When the state exits, it should call this function to ensure the mouse is unlocked
        void exit()
        {
            if (mouse_locked)
            {
                mouse_locked = false;
                app->getMouse().unlockMouse(app->getWindow());
            }
        }
    };

}
