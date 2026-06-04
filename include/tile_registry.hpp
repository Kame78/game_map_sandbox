#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdint>


using TileRuntimeId = uint16_t;

struct TileProperties {
    TileRuntimeId id;
    std::string internalName;
    bool isSolid = false;
    float speedModifier = 1.0f;
    std::string footstepType = "dirt";
    int startX = 0;
    int startY = 0;
    
};

class TileRegistry {
private:
    std::vector<TileProperties> m_registry;
    std::unordered_map<std::string, TileRuntimeId> m_nameToIdMap;

    TileRegistry();  //Private contructor for Singleton

public:

static TileRegistry& Instance() {
    static TileRegistry instance;
    return instance;
}

bool Initialize(const std::string& configPath);

TileRuntimeId GetId(const std::string& name) const;
const TileProperties& GetProperties(TileRuntimeId id) const;
};

