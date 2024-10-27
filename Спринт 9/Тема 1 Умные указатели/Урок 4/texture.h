#pragma once
#include "common.h"

class Texture {
 public:
  explicit Texture(Image image)
      : image_(std::move(image)) {
  }

  Size GetSize() const {
    return GetImageSize(image_);
  }

  char GetPixelColor(Point p) const {
    char color = ' ';

    if (!image_.empty() && IsPointInRectangle(p, GetSize())) {
      color = image_.at(p.y).at(p.x);
    }

    return color;
  }

 private:
  Image image_;
};