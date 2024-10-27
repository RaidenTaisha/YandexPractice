#pragma once
#include <array>
#include <cassert>
#include <memory>

#include "geom.h"

class Tile {
 public:
  // Размер тайла 8*8 пикселей.
  constexpr static int SIZE = 8;

  // Конструктор по умолчанию. Заполняет тайл указанным цветом.
  Tile(char color = ' ') noexcept {
    map_.fill(color);
    // -------------- не удалять ------------
    assert(instance_count_ >= 0);
    ++instance_count_;  // Увеличиваем счётчик тайлов (для целей тестирования).
    // -------------- не удалять ------------
  }

  Tile(const Tile& other) {
    map_ = other.map_;
    // -------------- не удалять ------------
    assert(instance_count_ >= 0);
    ++instance_count_;  // Увеличиваем счётчик тайлов (для целей тестирования).
    // -------------- не удалять ------------
  }

  ~Tile() {
    // -------------- Не удалять ------------
    --instance_count_;  // Уменьшаем счётчик тайлов.
    assert(instance_count_ >= 0);
    // -------------- не удалять ------------
  }

  /**
   * Изменяет цвет пикселя тайла.
   * Если координаты выходят за пределы тайла, метод ничего не делает.
   */
  void SetPixel(Point p, char color) noexcept {
    if (p.x >= SIZE || p.y >= SIZE)
      return;
    map_.at(p.x + p.y * 8) = color;
  }

  /**
   * Возвращает цвет пикселя. Если координаты выходят за пределы тайла, возвращается пробел.
   */
  char GetPixel(Point p) const noexcept {
    if (p.x >= SIZE || p.y >= SIZE || p.x < 0 || p.y < 0) return ' ';
    return (map_.at(p.x + p.y * 8));
  }

  // Возвращает количество экземпляра класса Tile в программе.
  static int GetInstanceCount() noexcept {
    // -------------- не удалять ------------
    return instance_count_;
    // -------------- не удалять ------------
  }

 private:
  // -------------- не удалять ------------
  inline static int instance_count_ = 0;
  // -------------- не удалять ------------
  std::array<char, 64> map_;
};