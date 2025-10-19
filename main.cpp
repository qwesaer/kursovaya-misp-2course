#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace sf;

class SimpleGame {
private:
    RenderWindow window;
    
    // Текстуры и спрайты
    Texture playerTexture;
    Texture benchTexture;
    Texture garageTexture;
    Sprite* playerSprite = nullptr;
    
    // Игрок
    int currentLane = 1;
    bool isJumping = false;
    bool isFalling = false;
    float jumpHeight = 0.0f;
    float jumpSpeed = 400.0f;
    float maxJumpHeight = 150.0f;
    
    // Полосы
    float laneWidth;
    std::vector<float> lanePositions;
    
    // Препятствия
    struct Obstacle {
        int type; // 0 - лавка, 1 - гараж
        Vector2f position;
        Vector2f size;
    };
    std::vector<Obstacle> obstacles;
    float obstacleSpeed = 300.0f;
    Clock spawnClock;
    bool firstFrame = true;
    
public:
    SimpleGame() : window(VideoMode({600, 600}), "Russia runner") {
        std::srand(std::time(nullptr));
        setup();
    }
    
    ~SimpleGame() {
        if (playerSprite) delete playerSprite;
    }
    
void setup() {
    // Загружаем текстуру лавки из папки spryte
    if (!benchTexture.loadFromFile("spryte/beanch.png")) {
        std::cout << "❌ Не удалось загрузить spryte/beanch.png" << std::endl;
    } else {
        std::cout << "✅ Загружен спрайт лавки" << std::endl;
    }
    
    // Загружаем текстуру гаража из папки spryte
    if (!garageTexture.loadFromFile("spryte/garage.png")) {
        std::cout << "❌ Не удалось загрузить spryte/garage.png" << std::endl;
    } else {
        std::cout << "✅ Загружен спрайт гаража" << std::endl;
        std::cout << "📏 Размер текстуры гаража: " << garageTexture.getSize().x << "x" << garageTexture.getSize().y << std::endl;
    }
    
    // Настройка полос - узкие, по центру
    float totalWidth = window.getSize().x;
    laneWidth = totalWidth / 4.0f;
    float offset = (totalWidth - (laneWidth * 3)) / 2.0f;
    lanePositions = {offset, offset + laneWidth, offset + laneWidth * 2};
    
    // Пытаемся загрузить текстуру игрока из папки spryte
    if (playerTexture.loadFromFile("spryte/player.png")) {
        std::cout << "✅ Загружен спрайт персонажа" << std::endl;
        playerSprite = new Sprite(playerTexture);
        playerSprite->setScale({0.8f, 0.8f});
    } else {
        std::cout << "❌ Не удалось загрузить spryte/player.png" << std::endl;
        std::cout << "💡 Использую синий квадрат" << std::endl;
    }
    
    updatePlayerPosition();
    
    std::cout << "✅ Игра инициализирована" << std::endl;
    std::cout << "🛣️ 3 узкие полосы (ширина: " << laneWidth << "px)" << std::endl;
    std::cout << "📁 Спрайты загружаются из папки: spryte/" << std::endl;
}
    
    void updatePlayerPosition() {
        float x, y;
        
        if (playerSprite) {
            FloatRect bounds = playerSprite->getGlobalBounds();
            x = lanePositions[currentLane] + laneWidth/2 - 25;
            y = 500.0f - jumpHeight;
            playerSprite->setPosition({x, y});
        } else {
            x = lanePositions[currentLane] + laneWidth/2 - 25;
            y = 500.0f - jumpHeight;
        }
    }
    
    void spawnObstacle() {
        if (spawnClock.getElapsedTime().asSeconds() > 0.8f) {
            Obstacle obstacle;
            obstacle.type = std::rand() % 2;
            
            if (obstacle.type == 0) {
                // ЛАВКА
                obstacle.size = Vector2f{60.0f, 30.0f};
            } else {
                // ГАРАЖ
                obstacle.size = Vector2f{80.0f, 80.0f};
            }
            
            int lane = std::rand() % 3;
            float x = lanePositions[lane] + laneWidth/2 - obstacle.size.x/2;
            obstacle.position = {x, -obstacle.size.y};
            
            obstacles.push_back(obstacle);
            spawnClock.restart();
        }
    }
    
    void updateObstacles(float deltaTime) {
        if (firstFrame) {
            firstFrame = false;
            return;
        }
        
        if (deltaTime > 0.1f) deltaTime = 0.1f;
        
        for (auto& obstacle : obstacles) {
            obstacle.position.y += obstacleSpeed * deltaTime;
        }
        
        obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
            [](const Obstacle& o) { 
                return o.position.y > 650.0f;
            }),
            obstacles.end());
    }
    
    void checkCollisions() {
        FloatRect playerBounds;
        
        if (playerSprite) {
            playerBounds = playerSprite->getGlobalBounds();
        } else {
            playerBounds = FloatRect({lanePositions[currentLane] + laneWidth/2 - 25, 500.0f - jumpHeight}, {50.0f, 50.0f});
        }
        
        if (isJumping || isFalling) {
            for (const auto& obstacle : obstacles) {
                FloatRect obstacleBounds(obstacle.position, obstacle.size);
                if (obstacle.type == 1 && playerBounds.findIntersection(obstacleBounds).has_value()) {
                    std::cout << "💥 СТОЛКНОВЕНИЕ С ГАРАЖОМ!" << std::endl;
                    window.close();
                    return;
                }
            }
            return;
        }
        
        for (const auto& obstacle : obstacles) {
            FloatRect obstacleBounds(obstacle.position, obstacle.size);
            if (playerBounds.findIntersection(obstacleBounds).has_value()) {
                std::cout << "💥 СТОЛКНОВЕНИЕ!" << std::endl;
                window.close();
                return;
            }
        }
    }
    
    void handleInput() {
        for (auto event = window.pollEvent(); event.has_value(); event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                return;
            }
            
            if (auto keyPressed = event->getIf<Event::KeyPressed>()) {
                if (keyPressed->scancode == Keyboard::Scan::A || keyPressed->scancode == Keyboard::Scan::Left) {
                    if (currentLane > 0) currentLane--;
                    updatePlayerPosition();
                }
                else if (keyPressed->scancode == Keyboard::Scan::D || keyPressed->scancode == Keyboard::Scan::Right) {
                    if (currentLane < 2) currentLane++;
                    updatePlayerPosition();
                }
                else if (keyPressed->scancode == Keyboard::Scan::W || keyPressed->scancode == Keyboard::Scan::Space) {
                    if (!isJumping && !isFalling) {
                        isJumping = true;
                        jumpHeight = 0.0f;
                    }
                }
                else if (keyPressed->scancode == Keyboard::Scan::Escape) {
                    window.close();
                }
                else if (keyPressed->scancode == Keyboard::Scan::R) {
                    obstacles.clear();
                    currentLane = 1;
                    isJumping = false;
                    isFalling = false;
                    jumpHeight = 0.0f;
                    updatePlayerPosition();
                    spawnClock.restart();
                    std::cout << "🔄 РЕСТАРТ!" << std::endl;
                }
            }
        }
    }
    
    void update(float deltaTime) {
        if (isJumping) {
            jumpHeight += jumpSpeed * deltaTime;
            if (jumpHeight >= maxJumpHeight) {
                isJumping = false;
                isFalling = true;
            }
            updatePlayerPosition();
        }
        else if (isFalling) {
            jumpHeight -= jumpSpeed * deltaTime;
            if (jumpHeight <= 0.0f) {
                isFalling = false;
                jumpHeight = 0.0f;
            }
            updatePlayerPosition();
        }
        
        spawnObstacle();
        updateObstacles(deltaTime);
        checkCollisions();
    }
    
    void render() {
    window.clear(Color(100, 100, 100));
    
    // Статическая текстура дороги из папки spryte
    static Texture roadTexture;
    static bool textureLoaded = roadTexture.loadFromFile("spryte/road.png");
    static Vector2u originalRoadSize = textureLoaded ? roadTexture.getSize() : Vector2u{338, 333};
    
    // Уменьшаем размер спрайта дороги
    static float scaleFactor = 0.25f;
    static Vector2f roadSpriteSize = {
        originalRoadSize.x * scaleFactor,
        originalRoadSize.y * scaleFactor
    };
    
    // Отрисовка 3 узких полос с повторяющимися спрайтами дороги
    for (int i = 0; i < 3; ++i) {
        if (textureLoaded) {
            // Рассчитываем сколько спрайтов нужно для заполнения полосы
            int tilesNeeded = static_cast<int>(600.0f / roadSpriteSize.y) + 1;
            
            for (int j = 0; j < tilesNeeded; ++j) {
                Sprite roadSprite(roadTexture);
                
                // Позиция спрайта
                float posX = lanePositions[i] + 1.0f;
                float posY = static_cast<float>(j) * roadSpriteSize.y;
                roadSprite.setPosition({posX, posY});
                
                // Масштабируем по ширине узкой полосы
                float scaleX = (laneWidth - 2.0f) / originalRoadSize.x;
                roadSprite.setScale({scaleX, scaleFactor});
                
                window.draw(roadSprite);
            }
        } else {
            // Запасной вариант - цветные полосы
            RectangleShape lane({laneWidth - 2.0f, 600.0f});
            lane.setPosition({lanePositions[i] + 1.0f, 0.0f});
            lane.setFillColor(i == currentLane ? Color(150, 150, 150) : Color(120, 120, 120));
            window.draw(lane);
        }
    }
    
    // Отрисовка препятствий
    for (const auto& obstacle : obstacles) {
        if (obstacle.type == 0 && benchTexture.getSize().x > 0) {
            // ЛАВКА - спрайт
            Sprite benchSprite(benchTexture);
            Vector2u texSize = benchTexture.getSize();
            float scaleX = obstacle.size.x / texSize.x;
            float scaleY = obstacle.size.y / texSize.y;
            benchSprite.setScale({scaleX, scaleY});
            benchSprite.setPosition(obstacle.position);
            window.draw(benchSprite);
        } else if (obstacle.type == 1 && garageTexture.getSize().x > 0) {
            // ГАРАЖ - спрайт
            Sprite garageSprite(garageTexture);
            Vector2u texSize = garageTexture.getSize();
            float scaleX = obstacle.size.x / texSize.x;
            float scaleY = obstacle.size.y / texSize.y;
            garageSprite.setScale({scaleX, scaleY});
            garageSprite.setPosition(obstacle.position);
            window.draw(garageSprite);
        } else {
            // Запасной вариант - цветные квадраты
            RectangleShape obstacleShape(obstacle.size);
            obstacleShape.setFillColor(obstacle.type == 0 ? Color::Green : Color::Red);
            obstacleShape.setPosition(obstacle.position);
            window.draw(obstacleShape);
        }
    }
    
    // Отрисовка игрока
    if (playerSprite) {
        window.draw(*playerSprite);
    } else {
        RectangleShape playerShape({50.0f, 50.0f});
        playerShape.setFillColor(Color::Blue);
        Vector2f playerPos = {lanePositions[currentLane] + laneWidth/2 - 25, 500.0f - jumpHeight};
        playerShape.setPosition(playerPos);
        window.draw(playerShape);
    }
    
    window.display();
}
    
    void run() {
        Clock clock;
        
        std::cout << "=== RUSSIA RUNNER ===" << std::endl;
        std::cout << "🎮 Игра началась!" << std::endl;
        std::cout << "Управление: A/D ←→ - движение, W/ПРОБЕЛ - прыжок" << std::endl;
        std::cout << "R - рестарт, ESC - выход" << std::endl;
        
        while (window.isOpen()) {
            float deltaTime = clock.restart().asSeconds();
            handleInput();
            update(deltaTime);
            render();
        }
    }
};

int main() {
    SimpleGame game;
    game.run();
    std::cout << "Игра завершена" << std::endl;
    return 0;
}