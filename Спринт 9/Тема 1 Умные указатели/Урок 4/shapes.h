#pragma once
#include "texture.h"

#include <memory>

// Поддерживаемые виды фигур: прямоугольник и эллипс
enum class ShapeType { RECTANGLE, ELLIPSE };

class Shape {
 public:
  // Фигура после создания имеет нулевые координаты и размер,
  // а также не имеет текстуры
  explicit Shape(ShapeType type) : type_(type), position_({0, 0}), size_({0, 0}) {}

  void SetPosition(Point pos) {
    position_ = pos;
  }

  void SetSize(Size size) {
    size_ = size;
  }

  void SetTexture(std::shared_ptr<Texture> texture) {
    texture_ = std::move(texture);
  }

  // Рисует фигуру на указанном изображении
  // В зависимости от типа фигуры должен рисоваться либо эллипс, либо прямоугольник
  // Пиксели фигуры, выходящие за пределы текстуры, а также в случае, когда текстура не задана,
  // должны отображаться с помощью символа точка '.'
  // Части фигуры, выходящие за границы объекта image, должны отбрасываться.
  void Draw(Image& image) const {
    for (auto h = 0; h < size_.height; ++h) {
      for (auto w = 0; w < size_.width; ++w) {
        Point current_point = {w, h};

        auto status = false;
        if (type_ == ShapeType::RECTANGLE) {
          status = IsPointInRectangle(current_point, size_);
        } else if (type_ == ShapeType::ELLIPSE) {
          status = IsPointInEllipse(current_point, size_);
        }

        if (!status) {
          continue;
        }

        char color = '.';
        if (texture_ != nullptr && IsPointInRectangle(current_point, texture_->GetSize())) {
          color = texture_->GetPixelColor(current_point);
        }

        Point image_point = {w + position_.x, h + position_.y};
        if (IsPointInRectangle(image_point, GetImageSize(image))) {
          image.at(image_point.y).at(image_point.x) = color;
        }
      }
    }
  }

 private:
  ShapeType type_;
  Point position_;
  Size size_;
  std::shared_ptr<Texture> texture_;
};