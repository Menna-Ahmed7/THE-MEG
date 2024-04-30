#include "sharkComponent.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    void SharkComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
    }
}