#include <iostream>
#include <SFML/Graphics.hpp>
#include "Box2DHelper.h"
#include "SFMLRenderer.h"
#include <vector>

class Pelota : public Box2DHelper {
public:
    sf::Sprite pelota;
    sf::Texture texturaPelota;
    b2Body* bb;

    Pelota(b2World& mundo, float radio);
    void update();
};

Pelota::Pelota(b2World& mundo, float radio) {
    if (!texturaPelota.loadFromFile("Pelota.png")) {
        std::cerr << "Error cargando la textura" << std::endl;
        return;
    }

    pelota.setTexture(texturaPelota);

    float scale = (2 * radio) / texturaPelota.getSize().x;
    pelota.setScale(scale, scale);
    pelota.setOrigin(texturaPelota.getSize().x / 2, texturaPelota.getSize().y / 2);

    bb = Box2DHelper::CreateCircularDynamicBody(&mundo, radio, 0.2, 0.8, 0.3);
    bb->SetTransform(b2Vec2(20, 70), 0);
}

void Pelota::update() {
    b2Vec2 position = bb->GetPosition();
    pelota.setPosition(position.x, position.y);
}

class Canon {
public:
    sf::Sprite sprite;
    sf::Texture texturaCanon;

    Canon(float scaleX, float scaleY);
};

Canon::Canon(float scaleX, float scaleY) {
    if (!texturaCanon.loadFromFile("canon.png")) {
        std::cerr << "Error cargando la textura del cañón" << std::endl;
        return;
    }

    sprite.setTexture(texturaCanon);
    sprite.setOrigin(texturaCanon.getSize().x / 2, texturaCanon.getSize().y / 2);
    sprite.setScale(scaleX, scaleY);
    sprite.setPosition(20, 72.5);
}

int main() {
    float fps = 60;
    float frameTime = 1.0f / fps;

    sf::RenderWindow window(sf::VideoMode(1024, 760), "El cañoncito malvado");

    SFMLRenderer renderer(&window);
    b2World phyWorld(b2Vec2(0.0f, 90.8f));

    window.setFramerateLimit(fps);
    phyWorld.SetDebugDraw(&renderer);

    std::vector<std::unique_ptr<Pelota>> proyectil;

    sf::View camera;
    camera.setSize(100, 100);
    camera.setCenter(50, 50);
    window.setView(camera);

    sf::RectangleShape groundShape(sf::Vector2f(300, 1));
    groundShape.setFillColor(sf::Color::Blue);
    groundShape.setOrigin(150, 2.5);
    groundShape.setPosition(20, 80);

    b2Body* groundBody = Box2DHelper::CreateRectangularStaticBody(&phyWorld, 500, 5);
    groundBody->SetTransform(b2Vec2(50.0f, 80.0f), 0);
    groundBody->GetFixtureList()->SetFriction(0.9);

    Canon canon(0.1f, 0.1f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f worldMousePos = window.mapPixelToCoords(mousePos);

                    auto b = std::make_unique<Pelota>(phyWorld, 3.0f);
                    b2Vec2 direction1 = b2Vec2(worldMousePos.x - b->bb->GetPosition().x,
                        worldMousePos.y - b->bb->GetPosition().y);
                    direction1.Normalize();
                    b->bb->ApplyLinearImpulseToCenter(b2Vec2(direction1.x * 3000.0f, direction1.y * 3000.0f), true);

                    proyectil.push_back(std::move(b));
                }
            }
        }

        for (auto& bala : proyectil) {
            bala->update();
        }

        phyWorld.Step(frameTime, 8, 8);
        phyWorld.ClearForces();
        phyWorld.DebugDraw();

        window.clear(sf::Color::Black);
        window.draw(groundShape);
        for (auto& bala : proyectil) {
            window.draw(bala->pelota);
        }
        window.draw(canon.sprite);
        window.display();
    }

    return 0;
}
