@echo off
chcp 65001
echo Компиляция простой игры...

g++ main.cpp -o game.exe ^
-ISFML-3.0.2/include ^
-LSFML-3.0.2/lib ^
-lsfml-graphics ^
-lsfml-window ^
-lsfml-system ^
-lopengl32 ^
-lgdi32 ^
-static-libgcc ^
-static-libstdc++

if exist game.exe (
    echo ✅ КОМПИЛЯЦИЯ УСПЕШНА!
    echo Копирование DLL...
    copy SFML-3.0.2\bin\sfml-graphics-3.dll .
    copy SFML-3.0.2\bin\sfml-window-3.dll .
    copy SFML-3.0.2\bin\sfml-system-3.dll .
    echo.
    echo 🎮 ЗАПУСК ИГРЫ...
    echo.
    game.exe
) else (
    echo ❌ Ошибка компиляции!
    pause
)