#include "svg.h"

#include <sstream>

namespace svg {

using namespace std::literals;

std::string NormalizeStr(const std::string& line) {
  std::ostringstream out;

  for (const auto& c : line) {
    switch (c) {
      case '&': out << "&amp;"sv;
        break;
      case '<': out << "&lt;"sv;
        break;
      case '>': out << "&gt;"sv;
        break;
      case '\'':  // Для одинарной кавычки используется случай апострофа
      case '`': out << "&apos;"sv;
        break;
      case '"': out << "&quot;"sv;
        break;
      default: out << c;
        break;
    }
  }

  return out.str();
}

Point::Point(double x, double y) : x(x), y(y) {}

RenderContext::RenderContext(std::ostream& out) : out(out) {}

RenderContext::RenderContext(std::ostream& out, int indent_step, int indent)
    : out(out), indent_step(indent_step), indent(indent) {}

RenderContext RenderContext::Indented() const {
  return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const {
  for (int i = 0; i < indent; ++i) {
    out.put(' ');
  }
}

void Object::Render(const RenderContext& context) const {
  context.RenderIndent();
  RenderObject(context);
  context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point c) {
  center = c;
  return *this;
}

Circle& Circle::SetRadius(double r) {
  radius = r;
  return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
  context.out << R"(<circle cx=")" << center.x << R"(" cy=")" << center.y << R"(" r=")" << radius;
  RenderPathProps(context.out);
  context.out << R"(" />)"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
  std::ostringstream out;

  out << point.x << "," << point.y;
  points.emplace_back(out.str());

  return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
  context.out << R"(<polyline points=")";
  auto is_first = true;
  for (const auto& point : points) {
    if (is_first) {
      context.out << point;
      is_first = false;
    } else {
      context.out << " "sv << point;
    }
  }
  RenderPathProps(context.out);
  context.out << R"(" />)";
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
  base_point = pos;
  return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point of) {
  offset = of;
  return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
  font_size = size;
  return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string f) {
  font_family = std::move(f);
  return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string w) {
  font_weight = std::move(w);
  return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string d) {
  data = std::move(d);
  return *this;
}

void Text::RenderObject(const RenderContext& context) const {
  context.out << R"(<text x=")" << base_point.x
              << R"(" y=")" << base_point.y
              << R"(" dx=")" << offset.x
              << R"(" dy=")" << offset.y
              << R"(" font-size=")" << font_size;

  if (!font_family.empty())
    context.out << R"(" font-family=")" << font_family;
  if (!font_weight.empty())
    context.out << R"(" font-weight=")" << font_weight;

  RenderPathProps(context.out);

  context.out << R"(">)" << NormalizeStr(data) << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
  objects_.push_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
  std::cout << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << std::endl;
  std::cout << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << std::endl;
  for (const auto& obj : objects_)
    obj->Render({out, 0, 0});
  std::cout << "</svg>"sv;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin param) {
  switch (param) {
    case StrokeLineJoin::ARCS:return out << "arcs";
    case StrokeLineJoin::BEVEL:return out << "bevel";
    case StrokeLineJoin::MITER:return out << "miter";
    case StrokeLineJoin::MITER_CLIP:return out << "miter-clip";
    case StrokeLineJoin::ROUND:return out << "round";
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap param) {
  switch (param) {
    case StrokeLineCap::BUTT:return out << "butt";
    case StrokeLineCap::ROUND:return out << "round";
    case StrokeLineCap::SQUARE:return out << "square";
  }
  return out;
}

}  // namespace svg