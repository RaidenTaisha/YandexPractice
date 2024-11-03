#include <cassert>
#include <cstddef>  // нужно для nullptr_t
#include <utility>

using namespace std;

template <typename T>
class UniquePtr {
 private:
  T* ptr_;

 public:
  UniquePtr()
      : ptr_(nullptr)
  {
  }
  explicit UniquePtr(T* ptr)
      : ptr_(ptr)
  {
  }
  UniquePtr(const UniquePtr&) = delete;
  UniquePtr(UniquePtr&& other)
      : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }
  UniquePtr& operator=(const UniquePtr&) = delete;
  UniquePtr& operator=(std::nullptr_t) {
    Reset(nullptr);
    return *this;
  }
  UniquePtr& operator=(UniquePtr&& other) {
    if (this != &other) {
      Reset(other.ptr_);
      other.ptr_ = nullptr;
    }
    return *this;
  }
  ~UniquePtr() {
    delete ptr_;
  }
  T& operator*() const {
    return *ptr_;
  }
  T* operator->() const {
    return ptr_;
  }
  T* Release() {
    auto result = ptr_;
    ptr_ = nullptr;
    return result;
  }
  void Reset(T* ptr) {
    delete ptr_;
    ptr_ = ptr;
  }
  void Swap(UniquePtr& other) {
    std::swap(ptr_, other.ptr_);
  }
  T* Get() const {
    return ptr_;
  }
};

struct Item {
  static int counter;
  int value;
  Item(int v = 0)
      : value(v)
  {
    ++counter;
  }
  Item(const Item& other)
      : value(other.value)
  {
    ++counter;
  }
  ~Item() {
    --counter;
  }
};

int Item::counter = 0;

void TestLifetime() {
  Item::counter = 0;
  {
    UniquePtr<Item> ptr(new Item);
    assert(Item::counter == 1);

    ptr.Reset(new Item);
    assert(Item::counter == 1);
  }
  assert(Item::counter == 0);

  {
    UniquePtr<Item> ptr(new Item);
    assert(Item::counter == 1);

    auto rawPtr = ptr.Release();
    assert(Item::counter == 1);

    delete rawPtr;
    assert(Item::counter == 0);
  }
  assert(Item::counter == 0);
}

void TestGetters() {
  UniquePtr<Item> ptr(new Item(42));
  assert(ptr.Get()->value == 42);
  assert((*ptr).value == 42);
  assert(ptr->value == 42);
}

int main() {
  TestLifetime();
  TestGetters();
}
