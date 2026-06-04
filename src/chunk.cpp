#include "chunk.hpp"
#include "map.hpp"

Chunk::Chunk(int chunkX, int chunkY, int chunkSize, int tileSize)
    : m_chunkX(chunkX), m_chunkY(chunkY), m_chunkSize(chunkSize), m_tileSize(tileSize),
      m_localGrid(chunkSize * chunkSize, 0)
      {
        
      }

      TileRuntimeId Chunk::GetTile(int localX, int localY) const {
        return m_localGrid[localY * m_chunkSize + localX];
      }

      void Chunk::SetTile(int localX, int localY, TileRuntimeId type) {
        m_localGrid[localY * m_chunkSize + localX] = type;
      }
