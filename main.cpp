#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    std::cout << "=== ТЕСТ SFML 3.0.2 ===" << std::endl;
    std::cout << "Создаю окно..." << std::endl;

    // Создаем окно
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Its working!");
    
    // Красный круг
    sf::CircleShape circle(100.f);
    circle.setFillColor(sf::Color::Red);
    circle.setPosition({300.f, 200.f});
    
    std::cout << "Окно создано! Должен быть красный круг." << std::endl;
    std::cout << "Закройте окно крестиком или нажмите ESC" << std::endl;

    // Главный игровой цикл
    while (window.isOpen()) {
        // Обработка событий
        for (auto event = window.pollEvent(); event.has_value(); event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            if (event->is<sf::Event::KeyPressed>()) {
                auto keyEvent = event->getIf<sf::Event::KeyPressed>();
                if (keyEvent && keyEvent->scancode == sf::Keyboard::Scan::Escape) {
                    window.close();
                }
            }
        }

        // Отрисовка
        window.clear(sf::Color::White);
        window.draw(circle);
        window.display();
    }
    
    std::cout << "Окно закрыто. SFML 3.0.2 работает правильно!" << std::endl;
    std::cout << "Нажмите Enter для выхода..." << std::endl;
    std::cin.get();
    
    return 0;
}
