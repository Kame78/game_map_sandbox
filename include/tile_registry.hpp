#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <cstdint>


using TileRuntimeId = uint16_t;

struct GridLocation {
    int col{};
    int row{};
};

struct TileProperties {
    TileRuntimeId id{};
    std::string internalName;
    std::string texturePath;
    int tileSize = 32;
    std::string layoutProfile;

    bool isSolid = false;
    int blendPriority{};
    std::string footstepType = "dirt";
    float speedModifier = 1.0f;
};

struct BiomeRule {
    std::string name;
    double minElevation = 0.0;
    double maxElevation = 1.0;
    double minMoisture = 0.0;
    double maxMoisture = 1.0;
    TileRuntimeId materialId = 0;
};

class TileRegistry {
private:
    std::vector<TileProperties> m_registry;
    std::unordered_map<std::string, TileRuntimeId> m_nameToIdMap;
    std::unordered_map<std::string, std::unordered_map<int, GridLocation>> m_layoutProfiles;

    std::vector<BiomeRule> m_biomeRules;

    TileRegistry() = default;  //Private contructor for Singleton

public:

static TileRegistry& Instance() { static TileRegistry instance; return instance; }

bool Initialize(const std::string& configPath);

TileRuntimeId GetId(const std::string& name) const;
const TileProperties& GetProperties(TileRuntimeId id) const;
GridLocation GetAutotileLocation(const std::string& profile, int bitmask) const;

const std::vector<BiomeRule>& GetBiomeRules() const { return m_biomeRules; }
};

