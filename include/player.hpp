#pragma once

#include <SFML/Graphics.hpp>

class Map;

class Player : public sf::Drawable {
private:
    int m_gridX{0};
    int m_gridY{0};
    float m_tileSize{32.f};

    sf::RectangleShape m_shape;

public:
    Player();

    void Spawn(int x, int y, float tileSize = 32.f);
    void HandleInput(const sf::Event& event, const Map& map);

    sf::Vector2f GetScreenPosition() const;
    int GetGridX() const {return m_gridX;}
    int GetGridY() const {return m_gridY;}

protected:
    virtual void draw(sf::RenderTarget& target, const sf::RenderStates states) const override;
};
