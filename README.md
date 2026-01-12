# Advanced Tic-Tac-Toe (C++20 + Qt 6)

Проект демонстрирует “крестики-нолики” с расширяемыми правилами и **строгим разделением**:
- `engine/` — движок без Qt (чистый C++20)
- `qt_gui/` — GUI на Qt 6 (QGraphicsView/QGraphicsScene)
- `tests/` — минимальные юнит‑тесты движка

---

## 1. Быстрый старт (Linux/macOS)

Требования:
- CMake >= 3.21
- Компилятор с C++20 (gcc/clang)
- Qt 6 (Widgets)

Сборка:
```bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
