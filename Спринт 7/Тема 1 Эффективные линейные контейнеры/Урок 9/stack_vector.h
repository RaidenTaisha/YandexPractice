#pragma once

#include <array>
#include <stdexcept>

template<typename T, size_t N>
class StackVector {
 public:
  explicit StackVector(size_t a_size = 0) {
    if (a_size > N) throw std::invalid_argument("");
    size_ = a_size;
  }

  T &operator[](size_t index) {
    return array_[index];
  };
  const T &operator[](size_t index) const {
    return array_[index];
  };

  T *begin() {
    return &array_.front();
  };
  T *end() {
    return &array_[size_];
  };
  T *begin() const {
    return const_cast<T*>(&array_.front());
  };
  T *end() const {
    return const_cast<T*>(&array_[size_]);
  };
  size_t Size() const {
    return size_;
  };
  size_t Capacity() const {
    return array_.size();
  };

  void PushBack(const T &value) {
    if (size_ == array_.size()) throw std::overflow_error("");
    array_[size_] = value;
    size_++;
  };
  T PopBack() {
    if (size_ == 0) throw std::underflow_error("");
    return array_[--size_];
  };
 private:
  std::array<T, N> array_;
  size_t size_ = 0;
};
