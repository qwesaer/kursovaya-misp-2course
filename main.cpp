#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace sf;

class RussiaRunner {
private:
    RenderWindow window;
    
    // Текстуры
    Texture playerTexture;
    Texture benchTexture;
    Texture garageTexture;
    Texture roadTexture;
    Texture beerTexture;
    Texture rubleTexture;
    Texture energyTexture;
    Texture seedsTexture;
    Texture macasinTexture;
    Texture mopedItemTexture;
    std::vector<Texture> mopedRideTextures;
    Sprite* playerSprite = nullptr;
    
    // Анимация бега
    std::vector<Texture> runTextures;
    int currentFrame = 0;
    float animationTimer = 0.0f;
    float frameTime = 0.1f;
    
    // Анимация мопеда
    int mopedRideFrame = 0;
    float mopedRideAnimationTimer = 0.0f;
    float mopedRideFrameTime = 0.1f;
    
    // Дорога
    float roadOffset = 0.0f;
    float roadSpeed = 300.0f;
    float baseRoadSpeed = 300.0f;
    
    // Игрок
    int currentLane = 1;
    bool isJumping = false;
    bool isFalling = false;
    float jumpHeight = 0.0f;
    float jumpSpeed = 400.0f;
    float maxJumpHeight = 150.0f;
    
    // Дорожные полосы
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
    
    // Бусты
    enum BoostType { BEER, RUBLE, ENERGY, SEEDS, MACASIN, MOPED };
    struct Boost {
        int type;
        Vector2f position;
        Vector2f size;
        bool active;
    };
    std::vector<Boost> boosts;
    Clock boostSpawnClock;
    
    // Активные бусты
    bool hasEnergyBoost = false;
    float energyTimer = 0.0f;
    const float ENERGY_DURATION = 15.0f;
    
    bool hasSeedsBoost = false;
    float seedsTimer = 0.0f;
    const float SEEDS_DURATION = 15.0f;
    
    bool hasMacasinBoost = false;
    float macasinTimer = 0.0f;
    const float MACASIN_DURATION = 15.0f;
    
    // Мопед
    int mopedCount = 1;
    bool isMopedActive = false;
    float mopedTimer = 0.0f;
    const float MOPED_DURATION = 20.0f;
    const int MAX_MOPEDS = 3;
    float mopedCooldown = 0.0f;
    
    // Счет
    int score = 0;
    int scoreMultiplier = 1;
    Clock scoreClock;
    Text* scoreText = nullptr;
    Text* boostTimerText = nullptr;
    
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
    RussiaRunner() : window(VideoMode({600, 600}), "Russia runner") {
        std::srand(std::time(nullptr));
        setup();
    }
    
    ~RussiaRunner() {
        if (playerSprite) delete playerSprite;
        if (titleText) delete titleText;
        if (playText) delete playText;
        if (controlsText) delete controlsText;
        if (exitText) delete exitText;
        if (backText) delete backText;
        if (scoreText) delete scoreText;
        if (boostTimerText) delete boostTimerText;
    }
    
    void setup() {
        if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
            return;
        }
        
        // Тексты меню
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
        
        // Тексты игры
        scoreText = new Text(font, "Score: 0", 30);
        scoreText->setFillColor(Color::White);
        scoreText->setPosition({10.0f, 10.0f});
        
        boostTimerText = new Text(font, "", 25);
        boostTimerText->setFillColor(Color::Yellow);
        boostTimerText->setPosition({10.0f, 50.0f});
        
        // Загрузка текстур анимации бега
        for (int i = 1; i <= 4; ++i) {
            Texture texture;
            std::string filename = "spryte/run" + std::to_string(i) + ".png";
            if (!texture.loadFromFile(filename)) {
            } else {
                runTextures.push_back(texture);
            }
        }
        
        // Загрузка текстур объектов
        if (!roadTexture.loadFromFile("spryte/road.png")) {}
        if (!benchTexture.loadFromFile("spryte/beanch.png")) {}
        if (!garageTexture.loadFromFile("spryte/garage.png")) {}
        if (!beerTexture.loadFromFile("spryte/beer.png")) {}
        if (!rubleTexture.loadFromFile("spryte/ruble.png")) {}
        if (!energyTexture.loadFromFile("spryte/energy.png")) {}
        if (!seedsTexture.loadFromFile("spryte/seeds.png")) {}
        if (!macasinTexture.loadFromFile("spryte/macasin.png")) {}
        
        // Загрузка текстур мопеда
        if (!mopedItemTexture.loadFromFile("spryte/moped_item.png")) {}
        
        for (int i = 1; i <= 4; ++i) {
            Texture texture;
            std::string filename = "spryte/moped_ride" + std::to_string(i) + ".png";
            if (!texture.loadFromFile(filename)) {
            } else {
                mopedRideTextures.push_back(texture);
            }
        }
        
        // Инициализация дорожных полос
        float totalWidth = window.getSize().x;
        laneWidth = totalWidth / 4.0f;
        float offset = (totalWidth - (laneWidth * 3)) / 2.0f;
        lanePositions = {offset, offset + laneWidth, offset + laneWidth * 2};
        
        // Создание спрайта игрока
        if (!runTextures.empty()) {
            playerSprite = new Sprite(runTextures[0]);
        } else if (playerTexture.loadFromFile("spryte/player.png")) {
            playerSprite = new Sprite(playerTexture);
        } else {
            playerSprite = nullptr;
        }
        
        if (playerSprite) {
            playerSprite->setScale({0.8f, 0.8f});
        }
        
        updatePlayerPosition();
    }
    
    // Обработка ввода в меню
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
                    }
                    
                    if (controlsText->getGlobalBounds().contains(mousePos)) {
                        currentState = CONTROLS;
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
                }
                else if (keyPressed->scancode == Keyboard::Scan::Escape) {
                    window.close();
                }
            }
        }
    }
    
    // Обработка ввода в управлении
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
    
    // Отрисовка меню
    void renderMenu() {
        window.clear(Color(30, 30, 30));
        window.draw(*titleText);
        window.draw(*playText);
        window.draw(*controlsText);
        window.draw(*exitText);
        window.display();
    }
    
    // Отрисовка управления
    void renderControls() {
        window.clear(Color(30, 30, 50));
        
        Text controlsTitle(font, "BOOSTS", 50);
        controlsTitle.setFillColor(Color::Yellow);
        controlsTitle.setPosition({220.0f, 60.0f});
        
        Text beerText(font, "BEER +100 points", 25);
        beerText.setFillColor(Color(255, 200, 0));
        beerText.setPosition({150.0f, 140.0f});
        
        Text rubleText(font, "RUBLE +50 points", 25);
        rubleText.setFillColor(Color::Green);
        rubleText.setPosition({150.0f, 180.0f});
        
        Text energyText(font, "ENERGY +20% speed (15s)", 25);
        energyText.setFillColor(Color::Red);
        energyText.setPosition({150.0f, 220.0f});
        
        Text seedsText(font, "SEEDS 2x points (15s)", 25);
        seedsText.setFillColor(Color::Cyan);
        seedsText.setPosition({150.0f, 260.0f});
        
        Text macasinText(font, "MACASIN jump over GARAGES (15s)", 25);
        macasinText.setFillColor(Color::Magenta);
        macasinText.setPosition({150.0f, 300.0f});
        
        Text mopedText(font, "MOPED [Q] - invincibility (20s or 1 hit)", 25);
        mopedText.setFillColor(Color(150, 150, 150));
        mopedText.setPosition({150.0f, 340.0f});
        
        Text mopedStackText(font, "Can stack up to 3 mopeds", 20);
        mopedStackText.setFillColor(Color(150, 150, 150));
        mopedStackText.setPosition({150.0f, 370.0f});
        
        Text menuText(font, "ESC - Back to Menu", 30);
        menuText.setFillColor(Color::White);
        menuText.setPosition({150.0f, 410.0f});
        
        Text restartText(font, "R - Restart (in game)", 30);
        restartText.setFillColor(Color::White);
        restartText.setPosition({150.0f, 460.0f});
        
        backText = new Text(font, "BACK (ESC)", 35);
        backText->setFillColor(Color::Green);
        backText->setPosition({220.0f, 520.0f});
        
        window.draw(controlsTitle);
        window.draw(beerText);
        window.draw(rubleText);
        window.draw(energyText);
        window.draw(seedsText);
        window.draw(macasinText);
        window.draw(mopedText);
        window.draw(mopedStackText);
        window.draw(menuText);
        window.draw(restartText);
        window.draw(*backText);
        
        window.display();
    }
    
    // Обновление позиции игрока
    void updatePlayerPosition() {
        float x = lanePositions[currentLane] + laneWidth/2 - 25;
        float y = 500.0f - jumpHeight;
        if (playerSprite) {
            playerSprite->setPosition({x, y});
        }
    }
    
    // Создание препятствий
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
    
    // Создание бустов
    void spawnBoost() {
        if (boostSpawnClock.getElapsedTime().asSeconds() > 5.0f && boosts.size() < 3) {
            Boost boost;
            boost.type = std::rand() % 6;
            boost.size = Vector2f{40.0f, 40.0f};
            boost.active = true;
            
            int lane = std::rand() % 3;
            float x = lanePositions[lane] + laneWidth/2 - boost.size.x/2;
            boost.position = {x, -boost.size.y};
            
            boosts.push_back(boost);
            boostSpawnClock.restart();
        }
    }
    
    // Обновление препятствий
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
    
    // Обновление бустов
    void updateBoosts(float deltaTime) {
        for (auto& boost : boosts) {
            if (boost.active) {
                boost.position.y += obstacleSpeed * deltaTime;
            }
        }
        
        boosts.erase(std::remove_if(boosts.begin(), boosts.end(),
            [](const Boost& b) { 
                return b.position.y > 650.0f || !b.active;
            }),
            boosts.end());
    }
    
    // Применение эффектов бустов
    void applyBoostEffect(int boostType) {
        switch (boostType) {
            case BEER:
                score += 100;
                break;
                
            case RUBLE:
                score += 50;
                break;
                
            case ENERGY:
                hasEnergyBoost = true;
                energyTimer = ENERGY_DURATION;
                roadSpeed = baseRoadSpeed * 1.2f;
                obstacleSpeed = 300.0f * 1.2f;
                break;
                
            case SEEDS:
                hasSeedsBoost = true;
                seedsTimer = SEEDS_DURATION;
                scoreMultiplier = 2;
                break;
                
            case MACASIN:
                hasMacasinBoost = true;
                macasinTimer = MACASIN_DURATION;
                break;
                
            case MOPED:
                if (mopedCount < MAX_MOPEDS) {
                    mopedCount++;
                }
                break;
        }
    }
    
    // Проверка столкновений
    void checkCollisions() {
        FloatRect playerBounds;
        
        if (playerSprite) {
            playerBounds = playerSprite->getGlobalBounds();
        } else {
            playerBounds = FloatRect({lanePositions[currentLane] + laneWidth/2 - 25, 500.0f - jumpHeight}, {50.0f, 50.0f});
        }
        
        // Столкновения с бустами
        for (auto& boost : boosts) {
            if (boost.active) {
                FloatRect boostBounds(boost.position, boost.size);
                if (playerBounds.findIntersection(boostBounds).has_value()) {
                    applyBoostEffect(boost.type);
                    boost.active = false;
                }
            }
        }
        
        // Мопед активен - проверка на поломку
        if (isMopedActive) {
            bool collisionHappened = false;
            for (const auto& obstacle : obstacles) {
                FloatRect obstacleBounds(obstacle.position, obstacle.size);
                if (playerBounds.findIntersection(obstacleBounds).has_value()) {
                    collisionHappened = true;
                    break;
                }
            }
            
            if (collisionHappened) {
                isMopedActive = false;
                mopedTimer = 0.0f;
                mopedCooldown = 1.0f;
                if (!runTextures.empty() && playerSprite) {
                    playerSprite->setTexture(runTextures[currentFrame]);
                }
            }
            return;
        }
        
        // Задержка после поломки мопеда
        if (mopedCooldown > 0.0f) {
            return;
        }
        
        // Макасин активен - логика прыжков
        if (hasMacasinBoost) {
            if (isJumping || isFalling) {
                return;
            }
            for (const auto& obstacle : obstacles) {
                FloatRect obstacleBounds(obstacle.position, obstacle.size);
                if (playerBounds.findIntersection(obstacleBounds).has_value()) {
                    currentState = GAME_OVER;
                    return;
                }
            }
            return;
        }
        
        // Обычная логика столкновений
        if (isJumping || isFalling) {
            for (const auto& obstacle : obstacles) {
                FloatRect obstacleBounds(obstacle.position, obstacle.size);
                if (obstacle.type == 1 && playerBounds.findIntersection(obstacleBounds).has_value()) {
                    currentState = GAME_OVER;
                    return;
                }
            }
        } else {
            for (const auto& obstacle : obstacles) {
                FloatRect obstacleBounds(obstacle.position, obstacle.size);
                if (playerBounds.findIntersection(obstacleBounds).has_value()) {
                    currentState = GAME_OVER;
                    return;
                }
            }
        }
    }
    
    // Обновление таймеров бустов
    void updateBoostTimers(float deltaTime) {
        std::string boostText = "";
        
        // Задержка мопеда
        if (mopedCooldown > 0.0f) {
            mopedCooldown -= deltaTime;
            if (mopedCooldown < 0.0f) {
                mopedCooldown = 0.0f;
            }
        }
        
        // Энергетик
        if (hasEnergyBoost) {
            energyTimer -= deltaTime;
            int secondsLeft = static_cast<int>(energyTimer) + 1;
            boostText += "ENERGY: " + std::to_string(secondsLeft) + "s ";
            
            if (energyTimer <= 0.0f) {
                hasEnergyBoost = false;
                roadSpeed = baseRoadSpeed;
                obstacleSpeed = 300.0f;
            }
        }
        
        // Семечки
        if (hasSeedsBoost) {
            seedsTimer -= deltaTime;
            int secondsLeft = static_cast<int>(seedsTimer) + 1;
            boostText += "SEEDS: " + std::to_string(secondsLeft) + "s ";
            
            if (seedsTimer <= 0.0f) {
                hasSeedsBoost = false;
                scoreMultiplier = 1;
            }
        }
        
        // Макасин
        if (hasMacasinBoost) {
            macasinTimer -= deltaTime;
            int secondsLeft = static_cast<int>(macasinTimer) + 1;
            boostText += "MACASIN: " + std::to_string(secondsLeft) + "s ";
            
            if (macasinTimer <= 0.0f) {
                hasMacasinBoost = false;
            }
        }
        
        // Мопед
        if (isMopedActive) {
            mopedTimer -= deltaTime;
            int secondsLeft = static_cast<int>(mopedTimer) + 1;
            boostText += "MOPED: " + std::to_string(secondsLeft) + "s ";
            
            if (mopedTimer <= 0.0f) {
                isMopedActive = false;
            }
        }
        
        // Инвентарь мопедов
        if (mopedCount > 0) {
            boostText += "MOPEDx" + std::to_string(mopedCount) + " [Q] ";
        }
        
        if (boostTimerText) {
            boostTimerText->setString(boostText);
        }
    }
    
    // Обработка ввода в игре
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
                    }
                    updatePlayerPosition();
                }
                else if (keyPressed->scancode == Keyboard::Scan::D || keyPressed->scancode == Keyboard::Scan::Right) {
                    if (currentLane < 2) {
                        currentLane++;
                    }
                    updatePlayerPosition();
                }
                else if (keyPressed->scancode == Keyboard::Scan::W || keyPressed->scancode == Keyboard::Scan::Space) {
                    if (!isJumping && !isFalling) {
                        isJumping = true;
                        jumpHeight = 0.0f;
                    }
                }
                else if (keyPressed->scancode == Keyboard::Scan::Q && mopedCount > 0 && !isMopedActive) {
                    isMopedActive = true;
                    mopedTimer = MOPED_DURATION;
                    mopedCount--;
                }
                else if (keyPressed->scancode == Keyboard::Scan::Escape) {
                    currentState = MENU;
                    resetGame();
                }
                else if (keyPressed->scancode == Keyboard::Scan::R && currentState == GAME_OVER) {
                    currentState = PLAYING;
                    resetGame();
                }
            }
        }
    }
    
    // Сброс игры
    void resetGame() {
        obstacles.clear();
        boosts.clear();
        currentLane = 1;
        isJumping = false;
        isFalling = false;
        jumpHeight = 0.0f;
        score = 0;
        scoreMultiplier = 1;
        scoreClock.restart();
        roadOffset = 0.0f;
        roadSpeed = baseRoadSpeed;
        obstacleSpeed = 300.0f;
        
        hasEnergyBoost = false;
        energyTimer = 0.0f;
        hasSeedsBoost = false;
        seedsTimer = 0.0f;
        hasMacasinBoost = false;
        macasinTimer = 0.0f;
        
        mopedCount = 1;
        isMopedActive = false;
        mopedTimer = 0.0f;
        mopedCooldown = 0.0f;
        mopedRideFrame = 0;
        mopedRideAnimationTimer = 0.0f;
        
        currentFrame = 0;
        animationTimer = 0.0f;
        
        if (!runTextures.empty() && playerSprite) {
            playerSprite->setTexture(runTextures[0]);
        }
        
        if (scoreText) {
            scoreText->setString("Score: 0");
        }
        
        if (boostTimerText) {
            boostTimerText->setString("");
        }
        
        updatePlayerPosition();
        spawnClock.restart();
        boostSpawnClock.restart();
        firstFrame = true;
    }
    
    // Основное обновление игры
    void update(float deltaTime) {
        // Анимация бега
        if (!runTextures.empty() && playerSprite) {
            animationTimer += deltaTime;
            if (animationTimer >= frameTime) {
                currentFrame = (currentFrame + 1) % runTextures.size();
                if (!isMopedActive) {
                    playerSprite->setTexture(runTextures[currentFrame]);
                }
                animationTimer = 0.0f;
            }
        }
        
        // Анимация мопеда
        if (isMopedActive && !mopedRideTextures.empty()) {
            mopedRideAnimationTimer += deltaTime;
            if (mopedRideAnimationTimer >= mopedRideFrameTime) {
                mopedRideFrame = (mopedRideFrame + 1) % mopedRideTextures.size();
                playerSprite->setTexture(mopedRideTextures[mopedRideFrame]);
                mopedRideAnimationTimer = 0.0f;
            }
        }
        
        // Движение дороги
        roadOffset += roadSpeed * deltaTime;
        if (roadOffset >= roadTexture.getSize().y * 0.25f) {
            roadOffset = 0.0f;
        }
        
        // Прыжок
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
        
        // Обновление счета
        if (scoreClock.getElapsedTime().asSeconds() >= 1.0f) {
            score += 10 * scoreMultiplier;
            scoreClock.restart();
            
            if (scoreText) {
                scoreText->setString("Score: " + std::to_string(score));
            }
        }
        
        spawnObstacle();
        spawnBoost();
        updateObstacles(deltaTime);
        updateBoosts(deltaTime);
        updateBoostTimers(deltaTime);
        checkCollisions();
    }
    
    // Отрисовка игры
    void renderGame() {
        window.clear(Color(100, 100, 100));
        
        // Отрисовка дороги
        if (roadTexture.getSize().x > 0) {
            Vector2u originalRoadSize = roadTexture.getSize();
            float scaleFactor = 0.25f;
            Vector2f roadSpriteSize = {originalRoadSize.x * scaleFactor, originalRoadSize.y * scaleFactor};
            
            for (int i = 0; i < 3; ++i) {
                int tilesNeeded = static_cast<int>(600.0f / roadSpriteSize.y) + 2;
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
            for (int i = 0; i < 3; ++i) {
                RectangleShape lane({laneWidth - 2.0f, 600.0f});
                lane.setPosition({lanePositions[i] + 1.0f, 0.0f});
                lane.setFillColor(i == currentLane ? Color(150, 150, 150) : Color(120, 120, 120));
                window.draw(lane);
            }
        }
        
        // Отрисовка препятствий
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
        
        // Отрисовка бустов
        for (const auto& boost : boosts) {
            if (boost.active) {
                Texture* currentTexture = nullptr;
                Color fallbackColor;
                
                switch (boost.type) {
                    case BEER:
                        currentTexture = &beerTexture;
                        fallbackColor = Color(255, 200, 0);
                        break;
                    case RUBLE:
                        currentTexture = &rubleTexture;
                        fallbackColor = Color::Green;
                        break;
                    case ENERGY:
                        currentTexture = &energyTexture;
                        fallbackColor = Color::Red;
                        break;
                    case SEEDS:
                        currentTexture = &seedsTexture;
                        fallbackColor = Color::Cyan;
                        break;
                    case MACASIN:
                        currentTexture = &macasinTexture;
                        fallbackColor = Color::Magenta;
                        break;
                    case MOPED:
                        currentTexture = &mopedItemTexture;
                        fallbackColor = Color(150, 150, 150);
                        break;
                }
                
                if (currentTexture && currentTexture->getSize().x > 0) {
                    Sprite boostSprite(*currentTexture);
                    Vector2u texSize = currentTexture->getSize();
                    float scaleX = boost.size.x / texSize.x;
                    float scaleY = boost.size.y / texSize.y;
                    boostSprite.setScale({scaleX, scaleY});
                    boostSprite.setPosition(boost.position);
                    window.draw(boostSprite);
                } else {
                    RectangleShape boostShape(boost.size);
                    boostShape.setFillColor(fallbackColor);
                    boostShape.setPosition(boost.position);
                    window.draw(boostShape);
                }
            }
        }
        
        if (scoreText) {
            window.draw(*scoreText);
        }
        
        if (boostTimerText) {
            window.draw(*boostTimerText);
        }
        
        if (playerSprite) {
            // Визуальные эффекты бустов
            if (isMopedActive) {
                if (static_cast<int>(mopedTimer * 10) % 2 == 0) {
                    playerSprite->setColor(Color(100, 100, 255, 200));
                } else {
                    playerSprite->setColor(Color(200, 200, 255, 150));
                }
            }
            else if (hasEnergyBoost && static_cast<int>(energyTimer * 10) % 2 == 0) {
                playerSprite->setColor(Color(255, 100, 100));
            } else if (hasSeedsBoost && static_cast<int>(seedsTimer * 10) % 2 == 0) {
                playerSprite->setColor(Color(100, 255, 255));
            } else if (hasMacasinBoost && static_cast<int>(macasinTimer * 10) % 2 == 0) {
                playerSprite->setColor(Color(255, 100, 255));
            } else {
                playerSprite->setColor(Color::White);
            }
            window.draw(*playerSprite);
        } else {
            RectangleShape playerShape({50.0f, 50.0f});
            if (isMopedActive) {
                playerShape.setFillColor(Color::Blue);
            } else if (hasEnergyBoost) {
                playerShape.setFillColor(Color::Red);
            } else if (hasSeedsBoost) {
                playerShape.setFillColor(Color::Cyan);
            } else if (hasMacasinBoost) {
                playerShape.setFillColor(Color::Magenta);
            } else {
                playerShape.setFillColor(Color::Blue);
            }
            playerShape.setPosition({lanePositions[currentLane] + laneWidth/2 - 25, 500.0f - jumpHeight});
            window.draw(playerShape);
        }
        
        window.display();
    }
    
    // Отрисовка Game Over
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
    
    // Главный цикл игры
    void run() {
        Clock clock;
        
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
    RussiaRunner game;
    game.run();
    return 0;
}