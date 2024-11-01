#define _USE_MATH_DEFINES
#include "svg.h"

#include <cmath>

using namespace std::literals;
using namespace svg;

/*
Пример использования библиотеки. Он будет компилироваться и работать, когда вы реализуете
все классы библиотеки.
*/

namespace {

Polyline CreateStar(Point center, double outer_rad, double inner_rad, int num_rays) {
  Polyline polyline;
  for (int i = 0; i <= num_rays; ++i) {
    double angle = 2 * M_PI * (i % num_rays) / num_rays;
    polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
    if (i == num_rays) {
      break;
    }
    angle += M_PI / num_rays;
    polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
  }
  return polyline;
}

template <typename DrawableIterator>
void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) {
  for (auto it = begin; it != end; ++it) {
    (*it)->Draw(target);
  }
}

template <typename Container>
void DrawPicture(const Container& container, svg::ObjectContainer& target) {
  using namespace std;
  DrawPicture(begin(container), end(container), target);
}

}  // namespace

namespace shapes {

class Triangle : public Drawable {
 public:
  Triangle(Point p1, Point p2, Point p3) : p1_(p1), p2_(p2), p3_(p3) {}

  void Draw(ObjectContainer &container) const override {
    container.Add(Polyline()
                      .AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
  }

 private:
  Point p1_, p2_, p3_;
};

class Star : public Drawable {
 public:
  Star(Point center, double outer_rad, double inner_rad, int num_rays)
      : center_(center), outer_rad_(outer_rad), inner_rad_(inner_rad),
        num_rays_(num_rays) {}

  void Draw(ObjectContainer &container) const override {
    container.Add(CreateStar(center_, outer_rad_, inner_rad_, num_rays_)
                        .SetFillColor("red"s).SetStrokeColor("black"s));
  }

 private:
  Point center_;
  double outer_rad_;
  double inner_rad_;
  int num_rays_;
};

class Snowman : public Drawable {
 public:
  Snowman(Point head_center, double head_rad)
      : head_center_(head_center), head_rad_(head_rad) {}

  void Draw(ObjectContainer &container) const override {
    Point base_center = {head_center_.x, head_center_.y + 5 * head_rad_};
    Point mid_center = {head_center_.x, head_center_.y + 2 * head_rad_};

    auto color_circle = Circle()
        .SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s);

    auto base = Circle(color_circle)
        .SetCenter(base_center).SetRadius(head_rad_ * 2);
    auto mid = Circle(color_circle)
        .SetCenter(mid_center).SetRadius(head_rad_ * 1.5);
    auto head = Circle(color_circle)
        .SetCenter(head_center_).SetRadius(head_rad_);

    container.Add(base);
    container.Add(mid);
    container.Add(head);
  }

 private:
  Point head_center_;
  double head_rad_;
};

} // namespace Shapes

int main() {
  using namespace svg;
  using namespace shapes;
  using namespace std;

  vector<unique_ptr<svg::Drawable>> picture;
  picture.emplace_back(make_unique<Triangle>(Point{100, 20}, Point{120, 50}, Point{80, 40}));
  picture.emplace_back(make_unique<Star>(Point{50.0, 20.0}, 10.0, 4.0, 5));
  picture.emplace_back(make_unique<Snowman>(Point{30, 20}, 10.0));

  svg::Document doc;
  DrawPicture(picture, doc);

  const Text base_text = Text().SetFontFamily("Verdana"s)
                               .SetFontSize(12)
                               .SetPosition({10, 100})
                               .SetData("Happy New Year!"s);
  doc.Add(Text{base_text}
              .SetStrokeColor("yellow"s)
              .SetFillColor("yellow"s)
              .SetStrokeLineJoin(StrokeLineJoin::ROUND)
              .SetStrokeLineCap(StrokeLineCap::ROUND)
              .SetStrokeWidth(3));
  doc.Add(Text{base_text}.SetFillColor("red"s));

  doc.Render(cout);
}