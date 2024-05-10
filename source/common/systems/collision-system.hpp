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
        int health = 0;

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application *app)
        {
            this->app = app;
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent
        int update(World *world, float deltaTime)
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
            // cout << "posX " << Sharkposition.x << " posY "<<  Sharkposition.y << " posZ " << Sharkposition.z << endl;
            //Done : Check for collision components
            for(auto entity : world->getEntities()){
                collisionComponent = entity->getComponent<CollisionComponent>();
                if(!collisionComponent) continue;
                glm::vec3& CollisionEntityposition = entity->localTransform.position;
                // cout << "CollisionposX " << CollisionEntityposition.x << " ColliionposY "<<  CollisionEntityposition.y << " CollisionposZ " << CollisionEntityposition.z << endl;
                if(abs(CollisionEntityposition.z - Sharkposition.z) < 4 
                && abs(CollisionEntityposition.y - Sharkposition.y) < 4 
                && abs(CollisionEntityposition.x- Sharkposition.x) < 2 )
                {
                     cout<< "Collision" << endl ;
                     cout <<"entity name : " << entity->name << endl;
                     world->markForRemoval(entity);
                     world->deleteMarkedEntities();
                    if (collisionComponent->collisionType=="reward")
                    {
                        health++;
                        //Make Shark Smaller
                        SharkEntity->localTransform.scale[0] = SharkEntity->localTransform.scale[0]*1.1 ;
                        SharkEntity->localTransform.scale[1] = SharkEntity->localTransform.scale[1]*1.1 ;
                        SharkEntity->localTransform.scale[2] = SharkEntity->localTransform.scale[2]*1.1 ;
                    }
                    else if (collisionComponent->collisionType=="penalty")
                    {
                        health--;
                        //Make Shark Smaller
                        SharkEntity->localTransform.scale[0] = SharkEntity->localTransform.scale[0]*0.9 ;
                        SharkEntity->localTransform.scale[1] = SharkEntity->localTransform.scale[1]*0.9 ;
                        SharkEntity->localTransform.scale[2] = SharkEntity->localTransform.scale[2]*0.9 ;
                    }
                    break;
                }
            }   
            return health;
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
