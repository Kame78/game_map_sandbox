#include "tile_registry.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>


bool TileRegistry::Initialize(const std::string& configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Registry Error: Unable to locate config file: " << configPath << "\n";
        return false;
    }

    nlohmann::json data;
    file >> data;

    m_registry.clear();
    m_nameToIdMap.clear();
    m_layoutProfiles.clear();
    m_biomeRules.clear();


    if (data.contains("autotile_layouts")) {
        for (auto& [profileName, profileData] : data["autotile_layouts"].items()) {
            auto& bitmaskMap = m_layoutProfiles[profileName];
            for (auto& [bitmaskStr, locJson] : profileData["bitmask_mappings"].items()) {
                int bitmask = std::stoi(bitmaskStr);
                bitmaskMap[bitmask] = {locJson["col"], locJson["row"]};
            }
        }
    }
    if (data.contains("materials")) {
        for (const auto& matJson : data["materials"]) {
            TileProperties props;
            props.id = static_cast<TileRuntimeId>(m_registry.size());
            props.internalName = matJson.value("id", "unknown");
            props.texturePath = matJson.value("texture_path", "");
            props.tileSize = matJson.value("tile_size", 32);
            props.layoutProfile = matJson.value("layout_profile", "none");

            if(matJson.contains("properties")) {
                auto pJson = matJson["properties"];
                props.isSolid = pJson.value("is_solid", false);
                props.blendPriority = pJson.value("blend_priority", 0);
                props.footstepType = pJson.value("footstep_type", "dirt");
                props.speedModifier = pJson.value("speed_modifier", 1.0f);
            }

            m_registry.push_back(props);
            m_nameToIdMap[props.internalName] = props.id;

        }
    }

    if (data.contains("biome_rules")) {
        for(const auto& ruleJson : data["biome_rules"]) {
            BiomeRule rule;
            rule.name = ruleJson.value("name", "unknown_name");
            rule.minElevation = ruleJson.value("min_elev", 0.0);
            rule.maxElevation = ruleJson.value("max_elev", 1.0);
            rule.minMoisture = ruleJson.value("min_moist", 0.0);
            rule.maxMoisture = ruleJson.value("max_moist", 1.0);

            std::string targetMaterial = ruleJson.value("material", "clear_water");
            rule.materialId = GetId(targetMaterial);

            if (rule.materialId == 0 && targetMaterial != "clear_water") {
                std::cerr << "\n[Registry] FATAL CONFIG ERROR: Biome Rule '" << rule.name
                          <<"' targets an unrecognized material token string: '" << targetMaterial << "'\n"
                          << "Verify that your spelling matches your materials block configuration fields exactly!\n\n";
                return false;
            }

            m_biomeRules.push_back(rule);
        }
    }

    if (data.contains("vegetation_rules")) {
        for (const auto& vegJson : data["vegetation_rules"]) {
            VegatationRule rule;
            rule.biomeName = vegJson.value("biome_name", "unknown_flora");
            rule.spawnChance = vegJson.value("spawn_chance", 0.5);

            std::string spawnTarget = vegJson.value("spawn_on", "");
            rule.spawnOnMaterialId = GetId(spawnTarget);

            if (vegJson.contains("decorations")) {
                for (const auto& itemStr : vegJson["decorations"]) {
                        rule.decorationIds.push_back(itemStr);
                }
            }
            if (rule.spawnOnMaterialId != 0 && !rule.decorationIds.empty()) {
                m_vegetationRules.push_back(rule);
            }
         }
    }

    std::cout << "[Registry] Successfully initialized " << m_registry.size() << " materials"
    << m_biomeRules.size() << " biomes, and " << m_vegetationRules.size() << " foliage matrices.\n";
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

GridLocation TileRegistry::GetAutotileLocation(const std::string& profile, int bitmask) const {
    auto pIt = m_layoutProfiles.find(profile);
    if (pIt != m_layoutProfiles.end()) {
        auto bIt = pIt->second.find(bitmask);
        if (bIt != pIt->second.end()) {
            return bIt->second;
        }
    }
    return GridLocation{0, 0};
}


