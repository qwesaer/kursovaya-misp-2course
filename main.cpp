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
    
    // Игрок
    RectangleShape player;
    int currentLane = 1;
    bool isJumping = false;
    float jumpHeight = 0.0f;
    float jumpSpeed = 500.0f;
    
    // Полосы
    float laneWidth;
    std::vector<float> lanePositions;
    
    // Препятствия
    std::vector<RectangleShape> obstacles;
    float obstacleSpeed = 150.0f;
    Clock spawnClock;
    bool firstFrame = true;
    
public:
    SimpleGame() : window(VideoMode({800, 600}), "Бегун - 3 полосы") {
        std::srand(std::time(nullptr));
        setup();
    }
    
    void setup() {
        // Настройка полос
        laneWidth = window.getSize().x / 3.0f;
        lanePositions = {0, laneWidth, laneWidth * 2};
        
        // Настройка игрока
        player.setSize({50.0f, 50.0f});
        player.setFillColor(Color::Blue);
        updatePlayerPosition();
        
        std::cout << "✅ Игра инициализирована" << std::endl;
    }
    
    void updatePlayerPosition() {
        float x = lanePositions[currentLane] + laneWidth/2 - 25;
        float y = isJumping ? 500.0f - jumpHeight : 500.0f;
        player.setPosition({x, y});
    }
    
    void spawnObstacle() {
        // Спавним каждые 1.5 секунды
        if (spawnClock.getElapsedTime().asSeconds() > 1.5f) {
            RectangleShape obstacle;
            obstacle.setSize({60.0f, 60.0f});
            
            // Случайный цвет для разных типов препятствий
            Color obstacleColors[] = {
                Color::Red,     // Мент
                Color(255, 100, 0), // Гопник  
                Color::Green,   // Лавка
                Color::Magenta, // Гараж
                Color::Yellow,  // Мухтар
                Color::Cyan     // Бобик
            };
            obstacle.setFillColor(obstacleColors[std::rand() % 6]);
            
            // СЛУЧАЙНАЯ ПОЛОСА (0, 1 или 2)
            int lane = std::rand() % 3;
            float x = lanePositions[lane] + laneWidth/2 - 30;
            obstacle.setPosition({x, -80.0f}); // Начинаем выше экрана
            
            obstacles.push_back(obstacle);
            spawnClock.restart();
            
            std::cout << "🆕 Создано препятствие на полосе " << lane << std::endl;
        }
    }
    
    void updateObstacles(float deltaTime) {
        // Пропускаем первый кадр
        if (firstFrame) {
            firstFrame = false;
            return;
        }
        
        // Ограничиваем deltaTime
        if (deltaTime > 0.1f) deltaTime = 0.1f;
        
        // Движение препятствий вниз
        for (auto& obstacle : obstacles) {
            obstacle.move({0, obstacleSpeed * deltaTime});
        }
        
        // Удаляем только ушедшие далеко вниз
        obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(),
            [](const RectangleShape& o) { 
                bool shouldRemove = o.getPosition().y > 650.0f;
                if (shouldRemove) {
                    std::cout << "🗑️ Удалено препятствие" << std::endl;
                }
                return shouldRemove;
            }),
            obstacles.end());
    }
    
    void checkCollisions() {
        FloatRect playerBounds = player.getGlobalBounds();
        
        for (const auto& obstacle : obstacles) {
            if (playerBounds.findIntersection(obstacle.getGlobalBounds()).has_value()) {
                std::cout << "💥 СТОЛКНОВЕНИЕ! Игра окончена." << std::endl;
                window.close();
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
                    if (!isJumping) {
                        isJumping = true;
                        jumpHeight = 0.0f;
                    }
                }
                else if (keyPressed->scancode == Keyboard::Scan::Escape) {
                    window.close();
                }
                else if (keyPressed->scancode == Keyboard::Scan::R) {
                    // Рестарт игры
                    obstacles.clear();
                    currentLane = 1;
                    isJumping = false;
                    updatePlayerPosition();
                    spawnClock.restart();
                    std::cout << "🔄 РЕСТАРТ ИГРЫ!" << std::endl;
                }
            }
        }
    }
    
    void update(float deltaTime) {
        if (isJumping) {
            jumpHeight += jumpSpeed * deltaTime;
            if (jumpHeight >= 100.0f) isJumping = false;
            updatePlayerPosition();
        }
        
        spawnObstacle();
        updateObstacles(deltaTime);
        checkCollisions();
    }
    
    void render() {
        window.clear(Color(100, 100, 100));
        
        // Отрисовка полос
        for (int i = 0; i < 3; ++i) {
            RectangleShape lane({laneWidth - 2.0f, 600.0f});
            lane.setPosition({lanePositions[i] + 1.0f, 0.0f});
            lane.setFillColor(i == currentLane ? Color(150, 150, 150) : Color(120, 120, 120));
            window.draw(lane);
        }
        
        // Отрисовка препятствий
        for (const auto& obstacle : obstacles) {
            window.draw(obstacle);
        }
        
        // Отрисовка игрока
        window.draw(player);
        
        window.display();
    }
    
    void run() {
        Clock clock;
        
        std::cout << "🎮 Игра началась! Управление:" << std::endl;
        std::cout << "A/← - влево, D/→ - вправо, W/ПРОБЕЛ - прыжок" << std::endl;
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
    std::cout << "=== ИГРА - 3 ПОЛОСЫ ===" << std::endl;
    
    SimpleGame game;
    game.run();
    
    std::cout << "Игра завершена" << std::endl;
    return 0;
}