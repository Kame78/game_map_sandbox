#include <SFML/Graphics.hpp>
#include "map.hpp"
#include "player.hpp"
#include "world_editor.hpp"
#include <iostream>
#include <fstream>

int main() {

    sf::RenderWindow window(sf::VideoMode({1280,720}), "High-Performance Renderer");
    window.setFramerateLimit(60);

    Map map(2000,2000);
    Player player;
    std::ifstream testFile("saved_map.dat");
    if (testFile.is_open()) {
        testFile.close();
        
        int startX{};
        int startY{};

        map.LoadFromFile("saved_map.dat", startX, startY);
        player.Spawn(startX, startY, 32.f);
        std::cout << "Loaded existing world state. Spawning player at (" << startX << ", " << startY << ")\n";
    } else {
        map.Randomize(35);
    for (int i=0; i < 5; ++i) {
        map.StepSimulation();
    }

    for (int y = 1; y < map.GetHeight(); ++y) {
        for (int x = 1; x < map.GetWidth(); ++x) {
            if (map.getTile(x, y) == TileType::Floor) {
                player.Spawn(x, y, 32.f);
                goto spawnFound;
            }
        }
    }
    spawnFound:
        std::cout << "No save file found. Generated fresh world and found a default spawn point.\n";
    }

    if (!map.LoadTextures("assets/textures/tilesets/grass.png", 
                           "assets/textures/tilesets/wall.png")) {
        std::cerr << "Pipeline Error: Could not locate asset textures!\n";
        return -1;
    }

    map.UpdateAllGeometry();

    WorldEditor editor;
    sf::View camera(sf::FloatRect({0.f, 0.f}, {1280.f, 720.f}));

    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            player.HandleInput(*event, map);
            editor.HandleEvent(*event, window, camera, map, player);
        }
    editor.Update(window, camera, map, player);
    camera.setCenter(player.GetScreenPosition());
    window.clear(sf::Color(25, 25, 25));
    window.setView(camera);
    window.draw(map);
    window.draw(player);
    window.display();
    }
    return 0;
}
