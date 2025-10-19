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
    Texture roadTexture;
    Sprite* playerSprite = nullptr;
    
    // Анимация персонажа
    std::vector<Texture> runTextures;
    int currentFrame = 0;
    float animationTimer = 0.0f;
    float frameTime = 0.1f;
    
    // Дорога
    float roadOffset = 0.0f;
    float roadSpeed = 300.0f;
    
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
        int type;
        Vector2f position;
        Vector2f size;
    };
    std::vector<Obstacle> obstacles;
    float obstacleSpeed = 300.0f;
    Clock spawnClock;
    bool firstFrame = true;
    
    // Счет
    int score = 0;
    Clock scoreClock;
    Text* scoreText = nullptr;
    
    // Меню
    enum GameState { MENU, PLAYING, CONTROLS, GAME_OVER };
    GameState currentState = MENU;
    Font font;
    Text* titleText = nullptr;
    Text* playText = nullptr;
    Text* controlsText = nullptr;
    Text* exitText = nullptr;
    Text* backText = nullptr;
    
public:
    SimpleGame() : window(VideoMode({600, 600}), "Russia runner") {
        std::srand(std::time(nullptr));
        setup();
    }
    
    ~SimpleGame() {
        if (playerSprite) delete playerSprite;
        if (titleText) delete titleText;
        if (playText) delete playText;
        if (controlsText) delete controlsText;
        if (exitText) delete exitText;
        if (backText) delete backText;
        if (scoreText) delete scoreText;
    }
    
    void setup() {
        if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cout << "❌ Не удалось загрузить шрифт" << std::endl;
        }
        
        titleText = new Text(font, "RUSSIA RUNNER", 50);
        titleText->setFillColor(Color::Red);
        titleText->setPosition({150.0f, 150.0f});
        
        playText = new Text(font, "GAME", 40);
        playText->setFillColor(Color::White);
        playText->setPosition({250.0f, 300.0f});
        
        controlsText = new Text(font, "CONTROLS", 40);
        controlsText->setFillColor(Color::White);
        controlsText->setPosition({230.0f, 370.0f});
        
        exitText = new Text(font, "EXIT", 40);
        exitText->setFillColor(Color::White);
        exitText->setPosition({250.0f, 440.0f});
        
        scoreText = new Text(font, "Score: 0", 30);
        scoreText->setFillColor(Color::White);
        scoreText->setPosition({10.0f, 10.0f});
        
        // Загружаем текстуры анимации
        for (int i = 1; i <= 4; ++i) {
            Texture texture;
            std::string filename = "spryte/run" + std::to_string(i) + ".png";
            if (!texture.loadFromFile(filename)) {
                std::cout << "❌ Не удалось загрузить " << filename << std::endl;
            } else {
                runTextures.push_back(texture);
                std::cout << "✅ Загружен кадр анимации: " << filename << std::endl;
            }
        }
        
        // Загружаем текстуру дороги
        if (!roadTexture.loadFromFile("spryte/road.png")) {
            std::cout << "❌ Не удалось загрузить spryte/road.png" << std::endl;
        } else {
            std::cout << "✅ Загружена текстура дороги" << std::endl;
        }
        
        if (!benchTexture.loadFromFile("spryte/beanch.png")) {
            std::cout << "❌ Не удалось загрузить spryte/beanch.png" << std::endl;
        } else {
            std::cout << "✅ Загружен спрайт лавки" << std::endl;
        }
        
        if (!garageTexture.loadFromFile("spryte/garage.png")) {
            std::cout << "❌ Не удалось загрузить spryte/garage.png" << std::endl;
        } else {
            std::cout << "✅ Загружен спрайт гаража" << std::endl;
        }
        
        float totalWidth = window.getSize().x;
        laneWidth = totalWidth / 4.0f;
        float offset = (totalWidth - (laneWidth * 3)) / 2.0f;
        lanePositions = {offset, offset + laneWidth, offset + laneWidth * 2};
        
        // Создаем спрайт игрока
        if (!runTextures.empty()) {
            playerSprite = new Sprite(runTextures[0]);
            std::cout << "✅ Анимация персонажа загружена, кадров: " << runTextures.size() << std::endl;
        } else if (playerTexture.loadFromFile("spryte/player.png")) {
            playerSprite = new Sprite(playerTexture);
            std::cout << "✅ Загружен спрайт персонажа" << std::endl;
        } else if (benchTexture.getSize().x > 0) {
            playerSprite = new Sprite(benchTexture);
            std::cout << "⚠️ Использована текстура лавки для персонажа" << std::endl;
        } else if (garageTexture.getSize().x > 0) {
            playerSprite = new Sprite(garageTexture);
            std::cout << "⚠️ Использована текстура гаража для персонажа" << std::endl;
        } else {
            if (!runTextures.empty()) {
                playerSprite = new Sprite(runTextures[0]);
            } else {
                playerSprite = nullptr;
                std::cout << "❌ Не удалось создать спрайт персонажа" << std::endl;
            }
        }
        
        if (playerSprite) {
            playerSprite->setScale({0.8f, 0.8f});
        }
        
        updatePlayerPosition();
        std::cout << "✅ Игра инициализирована" << std::endl;
    }
    
    void handleMenuInput() {
        for (auto event = window.pollEvent(); event.has_value(); event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                return;
            }
            
            if (auto mousePressed = event->getIf<Event::MouseButtonPressed>()) {
                if (mousePressed->button == Mouse::Button::Left) {
                    Vector2f mousePos = window.mapPixelToCoords({mousePressed->position.x, mousePressed->position.y});
                    
                    if (playText->getGlobalBounds().contains(mousePos)) {
                        currentState = PLAYING;
                        resetGame();
                        std::cout << "🎮 Начало игры!" << std::endl;
                    }
                    
                    if (controlsText->getGlobalBounds().contains(mousePos)) {
                        currentState = CONTROLS;
                        std::cout << "🎮 Просмотр управления!" << std::endl;
                    }
                    
                    if (exitText->getGlobalBounds().contains(mousePos)) {
                        window.close();
                    }
                }
            }
            
            if (auto keyPressed = event->getIf<Event::KeyPressed>()) {
                if (keyPressed->scancode == Keyboard::Scan::Enter) {
                    currentState = PLAYING;
                    resetGame();
                    std::cout << "🎮 Начало игры!" << std::endl;
                }
                else if (keyPressed->scancode == Keyboard::Scan::Escape) {
                    window.close();
                }
            }
        }
    }
    
    void handleControlsInput() {
        for (auto event = window.pollEvent(); event.has_value(); event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                return;
            }
            
            if (auto mousePressed = event->getIf<Event::MouseButtonPressed>()) {
                if (mousePressed->button == Mouse::Button::Left) {
                    Vector2f mousePos = window.mapPixelToCoords({mousePressed->position.x, mousePressed->position.y});
                    if (backText && backText->getGlobalBounds().contains(mousePos)) {
                        currentState = MENU;
                    }
                }
            }
            
            if (auto keyPressed = event->getIf<Event::KeyPressed>()) {
                if (keyPressed->scancode == Keyboard::Scan::Escape) {
                    currentState = MENU;
                }
            }
        }
    }
    
    void renderMenu() {
        window.clear(Color(30, 30, 30));
        window.draw(*titleText);
        window.draw(*playText);
        window.draw(*controlsText);
        window.draw(*exitText);
        window.display();
    }
    
    void renderControls() {
        window.clear(Color(30, 30, 50));
        
        Text controlsTitle(font, "CONTROLS", 50);
        controlsTitle.setFillColor(Color::Yellow);
        controlsTitle.setPosition({200.0f, 100.0f});
        
        Text moveText(font, "A/D or LEFT/RIGHT - Move", 30);
        moveText.setFillColor(Color::White);
        moveText.setPosition({150.0f, 200.0f});
        
        Text jumpText(font, "W or SPACE - Jump", 30);
        jumpText.setFillColor(Color::White);
        jumpText.setPosition({150.0f, 250.0f});
        
        Text menuText(font, "ESC - Back to Menu", 30);
        menuText.setFillColor(Color::White);
        menuText.setPosition({150.0f, 300.0f});
        
        Text restartText(font, "R - Restart (in game)", 30);
        restartText.setFillColor(Color::White);
        restartText.setPosition({150.0f, 350.0f});
        
        backText = new Text(font, "BACK (ESC)", 35);
        backText->setFillColor(Color::Green);
        backText->setPosition({220.0f, 450.0f});
        
        window.draw(controlsTitle);
        window.draw(moveText);
        window.draw(jumpText);
        window.draw(menuText);
        window.draw(restartText);
        window.draw(*backText);
        
        window.display();
    }
    
    void updatePlayerPosition() {
        float x = lanePositions[currentLane] + laneWidth/2 - 25;
        float y = 500.0f - jumpHeight;
        if (playerSprite) {
            playerSprite->setPosition({x, y});
        }
    }
    
    void spawnObstacle() {
        if (spawnClock.getElapsedTime().asSeconds() > 0.8f) {
            Obstacle obstacle;
            obstacle.type = std::rand() % 2;
            
            if (obstacle.type == 0) {
                obstacle.size = Vector2f{60.0f, 30.0f};
            } else {
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
                    currentState = GAME_OVER;
                    std::cout << "💥 Игра окончена!" << std::endl;
                    return;
                }
            }
            return;
        }
        
        for (const auto& obstacle : obstacles) {
            FloatRect obstacleBounds(obstacle.position, obstacle.size);
            if (playerBounds.findIntersection(obstacleBounds).has_value()) {
                currentState = GAME_OVER;
                std::cout << "💥 Игра окончена!" << std::endl;
                return;
            }
        }
    }
    
    void handleGameInput() {
        for (auto event = window.pollEvent(); event.has_value(); event = window.pollEvent()) {
            if (event->is<Event::Closed>()) {
                window.close();
                return;
            }
            
            if (auto keyPressed = event->getIf<Event::KeyPressed>()) {
                if (keyPressed->scancode == Keyboard::Scan::A || keyPressed->scancode == Keyboard::Scan::Left) {
                    if (currentLane > 0) {
                        currentLane--;
                        std::cout << "← Движение влево, полоса: " << currentLane << std::endl;
                    }
                    updatePlayerPosition();
                }
                else if (keyPressed->scancode == Keyboard::Scan::D || keyPressed->scancode == Keyboard::Scan::Right) {
                    if (currentLane < 2) {
                        currentLane++;
                        std::cout << "→ Движение вправо, полоса: " << currentLane << std::endl;
                    }
                    updatePlayerPosition();
                }
                else if (keyPressed->scancode == Keyboard::Scan::W || keyPressed->scancode == Keyboard::Scan::Space) {
                    if (!isJumping && !isFalling) {
                        isJumping = true;
                        jumpHeight = 0.0f;
                        std::cout << "↑ Прыжок!" << std::endl;
                    }
                }
                else if (keyPressed->scancode == Keyboard::Scan::Escape) {
                    currentState = MENU;
                    resetGame();
                }
                else if (keyPressed->scancode == Keyboard::Scan::R && currentState == GAME_OVER) {
                    currentState = PLAYING;
                    resetGame();
                    std::cout << "🔄 Рестарт игры!" << std::endl;
                }
            }
        }
    }
    
    void resetGame() {
        obstacles.clear();
        currentLane = 1;
        isJumping = false;
        isFalling = false;
        jumpHeight = 0.0f;
        score = 0;
        scoreClock.restart();
        roadOffset = 0.0f;
        
        currentFrame = 0;
        animationTimer = 0.0f;
        
        if (!runTextures.empty() && playerSprite) {
            playerSprite->setTexture(runTextures[0]);
        }
        
        if (scoreText) {
            scoreText->setString("Score: 0");
        }
        
        updatePlayerPosition();
        spawnClock.restart();
        firstFrame = true;
        std::cout << "🔄 Игра сброшена!" << std::endl;
    }
    
    void update(float deltaTime) {
        // АНИМАЦИЯ ПЕРСОНАЖА
        if (!runTextures.empty() && playerSprite) {
            animationTimer += deltaTime;
            if (animationTimer >= frameTime) {
                currentFrame = (currentFrame + 1) % runTextures.size();
                playerSprite->setTexture(runTextures[currentFrame]);
                animationTimer = 0.0f;
            }
        }
        
        // ДВИЖЕНИЕ ДОРОГИ
        roadOffset += roadSpeed * deltaTime;
        if (roadOffset >= roadTexture.getSize().y * 0.25f) {
            roadOffset = 0.0f;
        }
        
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
        
        if (scoreClock.getElapsedTime().asSeconds() >= 1.0f) {
            score += 10;
            scoreClock.restart();
            
            if (scoreText) {
                scoreText->setString("Score: " + std::to_string(score));
            }
            
            std::cout << "⭐ Score: " << score << std::endl;
        }
        
        spawnObstacle();
        updateObstacles(deltaTime);
        checkCollisions();
    }
    
    void renderGame() {
        window.clear(Color(100, 100, 100));
        
        // ОТРИСОВКА ДВИЖУЩЕЙСЯ ДОРОГИ
        if (roadTexture.getSize().x > 0) {
            Vector2u originalRoadSize = roadTexture.getSize();
            float scaleFactor = 0.25f;
            Vector2f roadSpriteSize = {originalRoadSize.x * scaleFactor, originalRoadSize.y * scaleFactor};
            
            for (int i = 0; i < 3; ++i) {
                int tilesNeeded = static_cast<int>(600.0f / roadSpriteSize.y) + 2; // +2 для перекрытия
                for (int j = -1; j < tilesNeeded; ++j) {
                    Sprite roadSprite(roadTexture);
                    float posX = lanePositions[i] + 1.0f;
                    float posY = static_cast<float>(j) * roadSpriteSize.y + roadOffset;
                    roadSprite.setPosition({posX, posY});
                    float scaleX = (laneWidth - 2.0f) / originalRoadSize.x;
                    roadSprite.setScale({scaleX, scaleFactor});
                    window.draw(roadSprite);
                }
            }
        } else {
            // Запасной вариант без текстуры дороги
            for (int i = 0; i < 3; ++i) {
                RectangleShape lane({laneWidth - 2.0f, 600.0f});
                lane.setPosition({lanePositions[i] + 1.0f, 0.0f});
                lane.setFillColor(i == currentLane ? Color(150, 150, 150) : Color(120, 120, 120));
                window.draw(lane);
            }
        }
        
        for (const auto& obstacle : obstacles) {
            if (obstacle.type == 0 && benchTexture.getSize().x > 0) {
                Sprite benchSprite(benchTexture);
                Vector2u texSize = benchTexture.getSize();
                float scaleX = obstacle.size.x / texSize.x;
                float scaleY = obstacle.size.y / texSize.y;
                benchSprite.setScale({scaleX, scaleY});
                benchSprite.setPosition(obstacle.position);
                window.draw(benchSprite);
            } else if (obstacle.type == 1 && garageTexture.getSize().x > 0) {
                Sprite garageSprite(garageTexture);
                Vector2u texSize = garageTexture.getSize();
                float scaleX = obstacle.size.x / texSize.x;
                float scaleY = obstacle.size.y / texSize.y;
                garageSprite.setScale({scaleX, scaleY});
                garageSprite.setPosition(obstacle.position);
                window.draw(garageSprite);
            } else {
                RectangleShape obstacleShape(obstacle.size);
                obstacleShape.setFillColor(obstacle.type == 0 ? Color::Green : Color::Red);
                obstacleShape.setPosition(obstacle.position);
                window.draw(obstacleShape);
            }
        }
        
        if (scoreText) {
            window.draw(*scoreText);
        }
        
        if (playerSprite) {
            window.draw(*playerSprite);
        } else {
            RectangleShape playerShape({50.0f, 50.0f});
            playerShape.setFillColor(Color::Blue);
            playerShape.setPosition({lanePositions[currentLane] + laneWidth/2 - 25, 500.0f - jumpHeight});
            window.draw(playerShape);
        }
        
        window.display();
    }
    
    void renderGameOver() {
        window.clear(Color(30, 0, 0));
        
        Text gameOverText(font, "GAME OVER!", 40);
        gameOverText.setFillColor(Color::Red);
        gameOverText.setPosition({180.0f, 150.0f});
        
        Text scoreText(font, "Final Score: " + std::to_string(score), 35);
        scoreText.setFillColor(Color::Yellow);
        scoreText.setPosition({170.0f, 220.0f});
        
        Text restartText(font, "Press R for restart", 30);
        restartText.setFillColor(Color::White);
        restartText.setPosition({190.0f, 300.0f});
        
        Text menuText(font, "ESC for escape to menu", 30);
        menuText.setFillColor(Color::White);
        menuText.setPosition({170.0f, 350.0f});
        
        window.draw(gameOverText);
        window.draw(scoreText);
        window.draw(restartText);
        window.draw(menuText);
        
        window.display();
    }
    
    void run() {
        Clock clock;
        
        std::cout << "=== RUSSIA RUNNER ===" << std::endl;
        std::cout << "🎮 Игра запущена!" << std::endl;
        
        while (window.isOpen()) {
            float deltaTime = clock.restart().asSeconds();
            
            switch (currentState) {
                case MENU:
                    handleMenuInput();
                    renderMenu();
                    break;
                    
                case PLAYING:
                    handleGameInput();
                    update(deltaTime);
                    renderGame();
                    break;
                    
                case CONTROLS:
                    handleControlsInput();
                    renderControls();
                    break;
                    
                case GAME_OVER:
                    handleGameInput();
                    renderGameOver();
                    break;
            }
        }
    }
};

int main() {
    SimpleGame game;
    game.run();
    std::cout << "Игра завершена" << std::endl;
    return 0;
}