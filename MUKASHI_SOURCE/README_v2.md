
MUKASHI Space v2 - REAPER + FL Studio

ФИЧИ:
- X = панорама (constant power)
- Y = глубина (air filter + volume + width + reverb)
- Стерео-комната, без бинаурала
- SHIFT+DRAG: зажми SHIFT при перетаскивании - лочит ось. Если ведешь больше по горизонтали - лочит Y, только X. Если по вертикали - наоборот.
- Снапшоты A,B,C,D:
  * ALT+CLICK на кнопку A/B/C/D = SAVE текущего положения
  * Обычный клик = RECALL
  * Снапшоты сохраняются вместе с проектом REAPER (.rpp)

Для хаба всех инструментов (как Visual Mixer):
- Следующий шаг - сделать MUKASHI Hub, который собирает все инстансы.
- Сейчас каждая копия независима, но уже готова к хабу через shared file.

Сборка как обычно: cmake -B build && cmake --build build --config Release
