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

  void Draw(ObjectContainer& container) const override {
    container.Add(Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
  }

 private:
  Point p1_, p2_, p3_;
};

class Star : public Drawable {
 public:
  Star(Point center, double outer_rad, double inner_rad, int num_rays)
      : center_(center), outer_rad_(outer_rad), inner_rad_(inner_rad),
        num_rays_(num_rays) {}

  void Draw(ObjectContainer& container) const override {
    container.Add(CreateStar(center_, outer_rad_, inner_rad_, num_rays_));
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

  void Draw(ObjectContainer& container) const override {
    Point base_center = {head_center_.x, head_center_.y + 5 * head_rad_};
    Point mid_center = {head_center_.x, head_center_.y + 2 * head_rad_};
    auto base = Circle().SetCenter(base_center).SetRadius(head_rad_ * 2);
    auto mid = Circle().SetCenter(mid_center).SetRadius(head_rad_ * 1.5);
    auto head = Circle().SetCenter(head_center_).SetRadius(head_rad_);

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
  // 5-лучевая звезда с центром {50, 20}, длиной лучей 10 и внутренним радиусом 4
  picture.emplace_back(make_unique<Star>(Point{50.0, 20.0}, 10.0, 4.0, 5));
  // Снеговик с "головой" радиусом 10, имеющей центр в точке {30, 20}
  picture.emplace_back(make_unique<Snowman>(Point{30, 20}, 10.0));

  svg::Document doc;
  // Так как документ реализует интерфейс ObjectContainer,
  // его можно передать в DrawPicture в качестве цели для рисования
  DrawPicture(picture, doc);

  // Выводим полученный документ в stdout
  doc.Render(cout);
}
