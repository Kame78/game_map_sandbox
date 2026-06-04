#include <SFML/Graphics.hpp>
#include "map.hpp"
#include "player.hpp"
#include "world_editor.hpp"
#include "tile_registry.hpp"
#include "map_renderer.hpp"
#include <iostream>
#include <fstream>

int main() {

    sf::RenderWindow window(sf::VideoMode({1280,720}), "High-Performance Renderer");
    window.setFramerateLimit(60);

    if (!TileRegistry::Instance().Initialize("assets/config/tileset.json")) {
        return -1;
    }

    Map map(2000,2000);
    Player player;
    MapRenderer renderer;

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
    for (int i=0; i < 5; ++i) map.StepSimulation();
    
    for (int y = 1; y < map.GetHeight(); ++y) {
        for (int x = 1; x < map.GetWidth(); ++x) {
            if (!TileRegistry::Instance().GetProperties(map.GetTile(x, y)).isSolid) {
                player.Spawn(x, y, 32.f);
                goto spawnFound;
            }
        }
    }
    spawnFound:
        std::cout << "No save file found. Generated fresh world and found a default spawn point.\n";
    }

    if (!renderer.LoadTextures("assets/tilesets/earth.png")) {
        std::cerr << "Pipeline Error: Could not locate asset textures!\n";
        return -1;
    }

    renderer.RegenarateAllGeometry(map);

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
    renderer.Draw(window, sf::RenderStates::Default);
    window.draw(player);
    window.display();
    }
    return 0;
}
