#include "world_editor.hpp"
#include <iostream>

void WorldEditor::HandleEvent(const sf::Event& event, const sf::RenderWindow& window, const sf::View& camera, Map& map, Player& player) {
    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->code == sf::Keyboard::Key::Num1) {
            m_currentMode = EditMode::PaintFloor;
            std::cout << "Switched to PaintFloor mode\n";
        }
        if (keyEvent->code == sf::Keyboard::Key::Num2) {
            m_currentMode = EditMode::PaintWall;
            std::cout << "Switched to PaintWall mode\n";
        }
        if (keyEvent->code == sf::Keyboard::Key::F5) {
            map.SaveToFile("saved_map.dat", player.GetGridX(), player.GetGridY());
        }
        if (keyEvent->code == sf::Keyboard::Key::F9) {
            int loadedX, loadedY;
            map.LoadFromFile("saved_map.dat", loadedX, loadedY);
            player.Spawn(loadedX, loadedY);
        }
    }

    if (const auto* mouseButtonEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseButtonEvent->button == sf::Mouse::Button::Left) {
        m_isPainting = true;
        }
    }
    if (const auto* mouseButtonEvent = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseButtonEvent->button == sf::Mouse::Button::Left) {
            m_isPainting = false;
        }
    }
}

void WorldEditor::Update(const sf::RenderWindow& window, const sf::View& camera, Map& map, Player& player) {
    if (!m_isPainting) return;

    sf::Vector2i windowMousePos = sf::Mouse::getPosition(window);
    sf::Vector2f worldMousePos = window.mapPixelToCoords(windowMousePos, camera);

    int gridX = static_cast<int>(worldMousePos.x / 32.f);
    int gridY = static_cast<int>(worldMousePos.y / 32.f);

    if (gridX < 0 || gridX >= map.GetWidth() || gridY < 0 || gridY >= map.GetHeight()) {
        return;
    }
    TileType targetType = (m_currentMode == EditMode::PaintWall) ? TileType::Wall : TileType::Floor;

    if (map.getTile(gridX, gridY) != targetType) {
        if (targetType == TileType:: Wall && gridX == player.GetGridX() && gridY == player.GetGridY())  {
            return;
        }
    }
    if (targetType == TileType::Wall && gridX == player.GetGridX() && gridY == player.GetGridY()) {
        return;
    }

    map.setTile(gridX, gridY, targetType);
    map.UpdateGeometry();
}