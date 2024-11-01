#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace svg {

using Color = std::string;

inline const Color NoneColor{"none"};

enum class StrokeLineCap {
  BUTT,
  ROUND,
  SQUARE,
};

enum class StrokeLineJoin {
  ARCS,
  BEVEL,
  MITER,
  MITER_CLIP,
  ROUND,
};

std::ostream& operator<<(std::ostream& out, StrokeLineJoin param);
std::ostream& operator<<(std::ostream& out, StrokeLineCap param);

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

  virtual void Draw(ObjectContainer& container) const = 0;
};

template <typename Owner>
class PathProps {
 public:
  Owner& SetFillColor(Color color);
  Owner& SetStrokeColor(Color color);
  Owner& SetStrokeWidth(double width);
  Owner& SetStrokeLineCap(StrokeLineCap line_cap);
  Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

 protected:
  ~PathProps() = default;

  void RenderPathProps(std::ostream& out) const;

 private:
  Owner& AsOwner() {
    return static_cast<Owner&>(*this);
  }

  std::optional<Color> fill_;
  std::optional<Color> stroke_;
  std::optional<double> stroke_width_;
  std::optional<StrokeLineCap> line_cap_;
  std::optional<StrokeLineJoin> line_join_;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
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
class Polyline final : public Object, public PathProps<Polyline> {
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
class Text final : public Object, public PathProps<Text> {
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

template <typename Owner>
Owner& PathProps<Owner>::SetFillColor(Color color) {
  fill_ = std::move(color);
  return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeColor(Color color) {
  stroke_ = std::move(color);
  return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeWidth(double width) {
  stroke_width_ = width;
  return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
  line_cap_ = line_cap;
  return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
  line_join_ = line_join;
  return AsOwner();
}

template <typename Owner>
void PathProps<Owner>::RenderPathProps(std::ostream& out) const {
  if (fill_.has_value())
    out << R"(" fill=")" << fill_.value();

  if (stroke_.has_value())
    out << R"(" stroke=")" << stroke_.value();

  if (stroke_width_.has_value())
    out << R"(" stroke-width=")" << stroke_width_.value();

  if (line_cap_.has_value())
    out << R"(" stroke-linecap=")" << line_cap_.value();

  if (line_join_.has_value())
    out << R"(" stroke-linejoin=")" << line_join_.value();
}

}  // namespace svg
