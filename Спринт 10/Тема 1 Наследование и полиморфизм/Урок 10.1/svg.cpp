#include "svg.h"

#include <sstream>
#include <utility>

namespace svg {

void NormalizeStr(std::ostream& out, const std::string_view line) {
  for (const auto& c : line) {
    switch (c) {
      case '&': out << "&amp;";
        break;
      case '<': out << "&lt;";
        break;
      case '>': out << "&gt;";
        break;
      case '\'':  // Для одинарной кавычки используется случай апострофа
      case '`': out << "&apos;";
        break;
      case '"': out << "&quot;";
        break;
      default: out << c;
        break;
    }
  }
}

std::ostream& operator<<(std::ostream& out, const std::monostate) {
  return out << "none";
}

std::ostream& operator<<(std::ostream& out, const Rgb& rgb) {
  return out << "rgb(" << static_cast<int>(rgb.red) << ','
             << static_cast<int>(rgb.green) << ','
             << static_cast<int>(rgb.blue) << ')';
}

std::ostream& operator<<(std::ostream& out, const Rgba rgba) {
  return out << "rgba(" << static_cast<int>(rgba.red) << ','
             << static_cast<int>(rgba.green) << ','
             << static_cast<int>(rgba.blue) << ',' << rgba.opacity << ')';
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
  switch (line_cap) {
    case StrokeLineCap::BUTT: return out << "butt";
    case StrokeLineCap::ROUND: return out << "round";
    case StrokeLineCap::SQUARE: return out << "square";
    default: break;
  }

  return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
  switch (line_join) {
    case StrokeLineJoin::ARCS: return out << "arcs";
    case StrokeLineJoin::BEVEL: return out << "bevel";
    case StrokeLineJoin::MITER: return out << "miter";
    case StrokeLineJoin::MITER_CLIP: return out << "miter-clip";
    case StrokeLineJoin::ROUND: return out << "round";
    default: break;
  }

  return out;
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
  std::visit([&out](const auto color) {
    out << color;
  }, color);

  return out;
}

RenderContext::RenderContext(std::ostream& out) : out(out) {}

RenderContext::RenderContext(std::ostream& out, int indent_step, int indent = 0)
    : out(out), indent_step(indent_step), indent(indent) {}

RenderContext RenderContext::Indented() const {
  return {out, indent_step, indent + indent_step};
}

void RenderContext::RenderIndent() const {
  for (int i = 0; i < indent; ++i)
    out.put(' ');
}

void Object::Render(const RenderContext& context) const {
  context.RenderIndent();
  RenderObject(context);
  context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
  center_ = center;
  return *this;
}

Circle& Circle::SetRadius(double radius) {
  radius_ = radius;
  return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
  context.out << "<circle";
  context.out << " cx=\"" << center_.x << "\"";
  context.out << " cy=\"" << center_.y << "\"";
  context.out << " r=\"" << radius_ << "\"";
  RenderPathProps(context.out);
  context.out << "/>";
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
  std::ostringstream out;

  out << point.x << "," << point.y;
  points_.emplace_back(out.str());

  return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
  context.out << R"(<polyline points=")";
  auto is_first = true;
  for (const auto& point : points_) {
    if (is_first) {
      context.out << point;
      is_first = false;
    } else {
      context.out << ' ' << point;
    }
  }

  context.out << R"(" )";
  RenderPathProps(context.out);
  context.out << "/>";
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
  base_point_ = pos;
  return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
  offset_ = offset;
  return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
  font_size_ = size;
  return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
  font_family_ = std::move(font_family);
  return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
  font_weight_ = std::move(font_weight);
  return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
  data_ = std::move(data);
  return *this;
}

void Text::RenderObject(const RenderContext& context) const {
  context.out << "<text";
  context.out << " x=\"" << base_point_.x << "\"";
  context.out << " y=\"" << base_point_.y << "\"";
  context.out << " dx=\"" << offset_.x << "\"";
  context.out << " dy=\"" << offset_.y << "\"";
  context.out << " font-size=\"" << font_size_ << "\"";

  if (font_family_.has_value())
    context.out << " font-family=\"" << *font_family_ << "\"";

  if (font_weight_.has_value())
    context.out << " font-weight=\"" << *font_weight_ << "\"";

  RenderPathProps(context.out);

  context.out << ">";
  NormalizeStr(context.out, data_);
  context.out << "</text>";
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
  objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
  out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << std::endl
      << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << std::endl;

  for (const auto& obj : objects_)
    obj->Render(out);

  out << "</svg>" << std::endl;
}

}  // namespace svg
