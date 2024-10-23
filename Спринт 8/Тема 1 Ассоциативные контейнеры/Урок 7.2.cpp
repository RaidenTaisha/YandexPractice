#include <iostream>
#include <string>

using namespace std;

struct Circle {
  double x;
  double y;
  double r;
};

struct Dumbbell {
  Circle circle1;
  Circle circle2;
  string text;
};

struct CircleHasher {
  size_t operator() (const Circle& circle) const {
    size_t h_x = d_hasher_(circle.x);
    size_t h_y = d_hasher_(circle.y);
    size_t h_r = d_hasher_(circle.r);

    return h_x + h_y * 37 + h_r * (37 * 37);
  }

 private:
  std::hash<double> d_hasher_;
};

struct DumbbellHash {
  size_t operator()(const Dumbbell &dumbbell) const {
    CircleHasher circle_hasher;
    return circle_hasher(dumbbell.circle1) * 37 * 37 * 37 * 37 + circle_hasher(dumbbell.circle2) * 37
        + hasher_(dumbbell.text);
  }
 private:
  hash<string> hasher_;
};

int main() {
  DumbbellHash hash;
  Dumbbell dumbbell{{10, 11.5, 2.3}, {3.14, 15, -8}, "abc"s};
  cout << "Dumbbell hash "s << hash(dumbbell);
}