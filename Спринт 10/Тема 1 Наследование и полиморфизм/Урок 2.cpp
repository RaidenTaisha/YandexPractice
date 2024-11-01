#include <cassert>
#include <string>
#include <iostream>
#include <stdexcept>

using namespace std;

constexpr int BricksPerMeterWidth = 4;
constexpr int BricksPerMeterHeigh = 8;

class House {
 public:
  House(int length, int width, int height) : length_(length), width_(width), height_(height) {}

  int GetLength() const { return length_; }
  int GetWidth() const { return width_; }
  int GetHeight() const { return height_; }

 private:
  int length_{}, width_{}, height_{};
};

class Resources {
 public:
  Resources(int brick_count) : bricks_count_(brick_count) {}

  void TakeBricks(int bricks_count) {
    if (bricks_count < 0 || bricks_count > bricks_count_) throw std::out_of_range("");
    bricks_count_ -= bricks_count;
  }

  int GetBrickCount() const { return bricks_count_; }

 private:
  int bricks_count_{};
};

struct HouseSpecification {
  int length = 0;
  int width = 0;
  int height = 0;
};

class Builder {
 public:
  Builder(Resources &resources) : resources_(&resources) {}

  House
  BuildHouse(HouseSpecification spec) {
    int vertical_wall = spec.length * BricksPerMeterWidth * spec.height * BricksPerMeterHeigh * 2;
    int horizontal_wall = spec.width * BricksPerMeterWidth * spec.height * BricksPerMeterHeigh * 2;
    int needed_bricks = vertical_wall + horizontal_wall;
    if (needed_bricks > resources_->GetBrickCount()) throw std::runtime_error("");
    resources_->TakeBricks(vertical_wall + horizontal_wall);
    return {spec.length, spec.width, spec.height};
  }

 private:
  Resources *resources_ = nullptr;
};

int main() {
  Resources resources{10000};
  Builder builder1{resources};
  Builder builder2{resources};

  House house1 = builder1.BuildHouse(HouseSpecification{12, 9, 3});
  assert(house1.GetLength() == 12);
  assert(house1.GetWidth() == 9);
  assert(house1.GetHeight() == 3);
  cout << resources.GetBrickCount() << " bricks left"s << endl;

  House house2 = builder2.BuildHouse(HouseSpecification{8, 6, 3});
  assert(house2.GetLength() == 8);
  cout << resources.GetBrickCount() << " bricks left"s << endl;

  House banya = builder1.BuildHouse(HouseSpecification{4, 3, 2});
  assert(banya.GetHeight() == 2);
  cout << resources.GetBrickCount() << " bricks left"s << endl;
}
