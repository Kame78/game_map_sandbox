#pragma once

#include <SFML/Graphics.hpp>
#include "player.hpp"
#include "map.hpp"

enum class EditMode {
    PaintFloor,
    PaintWall
};

class WorldEditor {
private:
    EditMode m_currentMode{EditMode::PaintWall};
    bool m_isPainting{false};

public:
    WorldEditor() = default;

    void HandleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& camera, Map& map, Player& player);

    void Update(const sf::RenderWindow& window, const sf::View& camera, Map& map, Player& player);

    void SetMode(EditMode mode) {m_currentMode = mode;}
    EditMode GetMode() const {return m_currentMode;}

};