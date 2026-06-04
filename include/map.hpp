#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "chunk.hpp"

class Map {
private:
    int m_width{0};
    int m_height{0};
    
    static constexpr int CHUNK_SIZE = 16;
    static constexpr int TILE_SIZE = 32;

    int m_numChunksX{0};
    int m_numChunksY{0};

    std::vector<Chunk> m_chunks;

public:
    Map(int width, int height);

    void Randomize(int fillPercentage);
    void StepSimulation();
    int CountActiveNeighbors(int x, int y) const;

    int GetWidth() const{ return m_width; }
    int GetHeight() const{ return m_height; }
    int GetChunkSize() const {return CHUNK_SIZE;}
    int GetTileSize() const {return TILE_SIZE;}
    int GetNumChunksX() const {return m_numChunksX;}
    int GetNumChunksY() const {return m_numChunksY;}


    void PrintToConsole() const;
    void SaveToFile(const std::string& filepath, int playerX, int playerY) const;
    void LoadFromFile(const std::string& filepath, int& outPlayerX, int& outPlayerY);



    TileRuntimeId GetTile(int x, int y) const;
    void SetTile(int x, int y, TileRuntimeId typeId);

    const Chunk& GetChunk(int cx, int cy) const {return m_chunks[cy * m_numChunksX + cx];}

};