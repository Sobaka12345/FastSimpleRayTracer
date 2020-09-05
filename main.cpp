#include <iostream>

#include "Renderer.hpp"
#include <SFML/Graphics.hpp>

int main()
{
    constexpr size_t width = 1920;
    constexpr size_t height = 1080;
    glm::vec3 initial_pos = {7, 10, -20};
    glm::vec3 initial_dir = glm::rotateX(glm::vec3(0, 0, 1), -glm::pi<float>() / 12);
    glm::vec3 initial_up = glm::rotateX(glm::vec3(0, 1, 0), -glm::pi<float>() / 12);

    Renderer renderer(width, height);

    renderer.SetView(initial_pos, initial_dir, initial_up)
            .SetFieldOfViewHor(glm::pi<float>() / 3);

    renderer.SetTraceDepth(5)
            .SetDefaultColor(glm::vec3(113, 165, 239));

    renderer.SetAmbientLight(0.3f)
            .AddPointLightSource({0, 100, -100}, 0.3f)
            .AddDirectionalLightSource({0, -0.5, 1}, 0.2);

    renderer.AddSphere({0, -1, 15}, 2, {0,255,0}, 0.3f, 1)
            .AddSphere({0, -1001, 0}, 999, {255, 255, 0}, 0.0f, 1000)
            .AddSphere({15, 0, 5}, 2, {0,255,0}, 0.1f, 500)
            .AddSphere({20, 1, 40}, 5, {255,0,0}, 0.3f, 200)
            //.AddPlane({25, 0, 1}, {25, 0, 0}, {25, 1, 0}, {0, 0, 255}, 1.0f, 1)
            .AddPlane({0, 0, 100}, {0, 1, 100}, {1, 0, 100}, {113, 165, 239}, 1.0f, 1);

    sf::RenderWindow window(sf::VideoMode(width, height), "Fast and FURIOUS");
    sf::Texture texture;
    texture.create(width, height);

    texture.update(renderer.RenderToPixelArray().data());
    sf::Sprite sprite(texture);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear();
        window.draw(sprite);
        window.display();
    }

    return 0;
}
