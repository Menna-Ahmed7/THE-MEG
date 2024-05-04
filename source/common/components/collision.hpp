
#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our {
    class CollisionComponent : public Component {
    public:
        std::string collisionType = "";
        static std::string getID() { return "Collision"; }
        std::string getCollisionType() { return this->collisionType; }

        void deserialize(const nlohmann::json& data) override;
    };

}