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

    // The free camera controller system is responsible for moving every entity which contains a FreeCameraControllerComponent.
    // This system is added as a slightly complex example for how use the ECS framework to implement logic.
    // For more information, see "common/components/free-camera-controller.hpp"
    class CollisionControllerSystem
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
            cout<<"I enetered";
            SharkObject *sharkComponent = nullptr;

            CameraComponent *cameraComponent = nullptr;
            CollisionComponent *collisionComponent = nullptr;
            // FreeCameraControllerComponent *controller = nullptr;
            for (auto entity : world->getEntities())
            {
                sharkComponent = entity->getComponent<SharkObject>();
                if(sharkComponent) break;
            }
            Entity *SharkEntity = sharkComponent->getOwner();
            glm::vec3& Sharkposition = SharkEntity->localTransform.position;
            cout << "posX " << Sharkposition.x << "posY "<<  Sharkposition.y << "posZ " << Sharkposition.z << endl;
            //Done : Check for collision components
            for(auto entity : world->getEntities()){
                 collisionComponent = entity->getComponent<CollisionComponent>();
                 if(!collisionComponent) continue;
                glm::vec3& CollisionEntityposition = entity->localTransform.position;
                if(abs(CollisionEntityposition.z - Sharkposition.z) < 0.5 && abs(CollisionEntityposition.y - Sharkposition.y) < 2 && abs(CollisionEntityposition.x- Sharkposition.x) < 1 )
                {
                     cout<< "Collision" << endl ;
                     world->markForRemoval(collisionComponent->getOwner());
                     cout<<"marked"<<endl;
                }
               
                     cout << "FishposX " << CollisionEntityposition.x << "FishposY "<<  CollisionEntityposition.y << "FishposZ" << CollisionEntityposition.z << endl;
            }

      
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
