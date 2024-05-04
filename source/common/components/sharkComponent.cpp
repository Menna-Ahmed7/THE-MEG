#include "sharkComponent.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    void SharkObject::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;

    }
}