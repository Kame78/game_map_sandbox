#pragma once

#include <vector>
#include <cstdint>

using TileRuntimeId = uint16_t;

class Chunk {
private:
    int m_chunkX;
    int m_chunkY;
    int m_chunkSize;
    int m_tileSize;

    std::vector<TileRuntimeId> m_localGrid;

public:
    Chunk(int chunkX, int chunkY, int chunkSize, int tileSize);

    TileRuntimeId GetTile(int localX, int localY) const;
    void SetTile(int localX, int localY, TileRuntimeId type);

    int GetChunkX() const {return m_chunkX;}
    int GetChunkY() const {return m_chunkY;}
};