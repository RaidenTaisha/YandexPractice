#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace svg {

struct Point {
  Point() = default;
  Point(double x, double y);
  double x = 0;
  double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
  RenderContext(std::ostream& out);
  RenderContext(std::ostream& out, int indent_step, int indent = 0);

  RenderContext Indented() const;
  void RenderIndent() const;

  std::ostream& out;
  int indent_step = 0;
  int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
 public:
  void Render(const RenderContext& context) const;

  virtual ~Object() = default;

 private:
  virtual void RenderObject(const RenderContext& context) const = 0;
};

class ObjectContainer {
 public:
  virtual ~ObjectContainer() = default;

  template <typename Obj>
  void Add(Obj obj);

  virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
};

class Drawable {
 public:
  virtual ~Drawable() = default;

  virtual void Draw(ObjectContainer &container) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object {
 public:
  Circle& SetCenter(Point center);
  Circle& SetRadius(double radius);

 private:
  void RenderObject(const RenderContext& context) const override;

  Point center;
  double radius = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object {
 public:
  // Добавляет очередную вершину к ломаной линии
  Polyline& AddPoint(Point point);

 private:
  void RenderObject(const RenderContext& context) const override;

  std::vector<std::string> points;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object {
 public:
  // Задаёт координаты опорной точки (атрибуты x и y)
  Text& SetPosition(Point pos);

  // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
  Text& SetOffset(Point offset);

  // Задаёт размеры шрифта (атрибут font-size)
  Text& SetFontSize(uint32_t size);

  // Задаёт название шрифта (атрибут font-family)
  Text& SetFontFamily(std::string font_family);

  // Задаёт толщину шрифта (атрибут font-weight)
  Text& SetFontWeight(std::string font_weight);

  // Задаёт текстовое содержимое объекта (отображается внутри тега text)
  Text& SetData(std::string data);

 private:
  void RenderObject(const RenderContext& context) const override;

  Point base_point;
  Point offset;
  uint32_t font_size = 1;
  std::string font_weight;
  std::string font_family;
  std::string data;
};

class Document final : public ObjectContainer {
 public:
  // Добавляет в svg-документ объект-наследник svg::Object
  void AddPtr(std::unique_ptr<Object>&& obj);

  // Выводит в ostream svg-представление документа
  void Render(std::ostream& out) const;

  // Прочие методы и данные, необходимые для реализации класса Document
 private:
  std::vector<std::unique_ptr<Object>> objects_;
};

template <typename Obj>
void ObjectContainer::Add(Obj obj) {
  AddPtr((std::make_unique<Obj>(std::move(obj))));
}

}  // namespace svg
