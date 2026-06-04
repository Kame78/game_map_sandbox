#include "tile_registry.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

TileRegistry::TileRegistry() {
    
    TileProperties defaultProps{
    .id = 0,
    .internalName = "fallback_empty",
    .isSolid = false,
    .speedModifier = 1.0f,
    .footstepType = "dirt"
    };
    m_registry.push_back(defaultProps);
    m_nameToIdMap["fallback_empty"] = 0;
}

bool TileRegistry::Initialize(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Registry Error: Unable to locate config file: " << configPath << "\n";
        return false;
    }

    nlohmann::json data;
    file >> data;

    for (const auto& sheet : data["sheets"]) {
        if (!sheet.contains("blocks")) continue;

        for (const auto& block : sheet["blocks"]) {
            std::string currentBlockId = block.value("id", "UNKNOWN_ID");

            try{
                TileProperties props;
                props.id = static_cast<TileRuntimeId>(m_registry.size());
                props.internalName = currentBlockId;
                props.startX = block.value("start_x", 0);
                props.startY = block.value("start_y", 0);


             if(!block.contains("properties")) {
                throw std::runtime_error("Missing 'properties' object entirely");
                }
            
                auto propsJson = block["properties"];

            if (!propsJson.contains("is_solid")) {
                throw std::runtime_error("Missing 'is_solid' key inside 'properties");
            }
            props.isSolid = propsJson["is_solid"].get<bool>();
            props.speedModifier = propsJson.value("speed_modifier", 1.0f);
            props.footstepType = propsJson.value("footstep_type", "dirt");

            m_registry.push_back(props);
            m_nameToIdMap[props.internalName] = props.id;
            }
            catch (const std::exception& e) {
                std::cerr << "Config Parsing Error\n" << "Tile Block ID: " << currentBlockId << "\n" << "Reason: " << e.what() << "'\n";
                return false;
            }
        }
    }
    std::cout << "TileRegistry successfully initialized. Loaded " << m_registry.size() - 1 << "custom assets.\n";
    return true;
}

TileRuntimeId TileRegistry::GetId(const std::string& name) const {
    auto it = m_nameToIdMap.find(name);
    return (it != m_nameToIdMap.end()) ? it->second : 0;
}

const TileProperties& TileRegistry::GetProperties(TileRuntimeId id) const {
    if (id >= m_registry.size()) { return m_registry[0]; }
    
    return m_registry[id];
   
}


