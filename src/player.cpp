#include "player.hpp"
#include "map.hpp"
#include "tile_registry.hpp"


Player::Player() {
    m_shape.setFillColor(sf::Color::Red);
}

void Player::Spawn(int x, int y, float tileSize) {
    m_gridX = x;
    m_gridY = y;
    m_tileSize = tileSize;

    m_shape.setSize({m_tileSize - 6.f, m_tileSize - 6.f});
    m_shape.setOrigin(m_shape.getSize() / 2.f);
}

void Player::HandleInput(const sf::Event& event, const Map& map) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        int nextX = m_gridX;
        int nextY = m_gridY;

        if(keyEvent->code == sf::Keyboard::Key::W || keyEvent->code == sf::Keyboard::Key::Up) nextY--;
        if(keyEvent->code == sf::Keyboard::Key::S || keyEvent->code == sf::Keyboard::Key::Down) nextY++;
        if(keyEvent->code == sf::Keyboard::Key::A || keyEvent->code == sf::Keyboard::Key::Left) nextX--;
        if(keyEvent->code == sf::Keyboard::Key::D || keyEvent->code == sf::Keyboard::Key::Right) nextX++;

        TileRuntimeId targetTileId = map.GetTile(nextX, nextY);
        if(!TileRegistry::Instance().GetProperties(targetTileId).isSolid) {
            m_gridX = nextX;
            m_gridY = nextY;
        }
    }
}

sf::Vector2f Player::GetScreenPosition() const {
    float screenX = static_cast<float>(m_gridX) * m_tileSize + (m_tileSize / 2.f);
    float screenY = static_cast<float>(m_gridY) * m_tileSize + (m_tileSize / 2.f);
    return {screenX, screenY};
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    sf::RectangleShape renderShape = m_shape;
    renderShape.setPosition(GetScreenPosition());
    target.draw(renderShape, states);

}