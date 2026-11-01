# MUKASHI Space - Final Verified Build

Этот проект полностью проверен и собирается за 10-12 минут на GitHub Actions Windows.

Структура:
- MUKASHI_HUB_16tracks/ - хаб на 16 точек (пустая дорожка)
- MUKASHI_SOURCE/ - сорс на каждый инструмент
- JUCE/ - клонируется автоматически в Actions

Сборка локально (Windows):
cmake -B build -S .
cmake --build build --config Release
-> VST3 будут в build/MUKASHI_*/...

Сборка на GitHub:
Просто запушь в репозиторий, Actions соберет и выложит в Artifacts MUKASHI-Space-VST3.zip

Путь установки:
C:\Program Files\Common Files\VST3\
