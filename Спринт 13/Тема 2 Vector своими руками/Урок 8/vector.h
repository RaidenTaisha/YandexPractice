#pragma once

#include <cassert>
#include <algorithm>
#include <cstddef>
#include <new>
#include <utility>
#include <memory>

template<typename T>
class RawMemory {
 public:
  RawMemory() = default;

  explicit RawMemory(size_t capacity) : buffer_(Allocate(capacity)), capacity_(capacity) {}

  RawMemory(const RawMemory &) = delete;

  RawMemory &operator=(const RawMemory &rhs) = delete;

  RawMemory(RawMemory &&other) noexcept {
    buffer_ = std::exchange(other.buffer_, nullptr);
    capacity_ = std::exchange(other.capacity_, 0);
  }

  RawMemory &operator=(RawMemory &&rhs) noexcept {
    if (this != &rhs) {
      Swap(rhs);
    }
    return *this;
  }

  ~RawMemory() {
    Deallocate(buffer_);
  }

  T *operator+(size_t offset) noexcept {
    assert(offset <= capacity_);
    return buffer_ + offset;
  }

  const T *operator+(size_t offset) const noexcept {
    return const_cast<RawMemory &>(*this) + offset;
  }

  const T &operator[](size_t index) const noexcept {
    return const_cast<RawMemory &>(*this)[index];
  }

  T &operator[](size_t index) noexcept {
    assert(index < capacity_);
    return buffer_[index];
  }

  void Swap(RawMemory &other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
  }

  const T *GetAddress() const noexcept {
    return buffer_;
  }

  T *GetAddress() noexcept {
    return buffer_;
  }

  size_t Capacity() const {
    return capacity_;
  }

 private:
  static T *Allocate(size_t n) {
    return n != 0 ? static_cast<T *>(operator new(n * sizeof(T))) : nullptr;
  }

  static void Deallocate(T *buf) noexcept {
    operator delete(buf);
  }

  T *buffer_ = nullptr;
  size_t capacity_ = 0;
};

template<typename T>
class Vector {
 public:
  Vector() noexcept = default;

  explicit Vector(size_t size) : data_(size), size_(size) {
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
  }

  Vector(const Vector &other) : data_(other.size_), size_(other.size_) {
    std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
  }

  Vector(Vector &&other) {
    data_ = std::exchange(other.data_, {});
    size_ = std::exchange(other.size_, 0);
  }

  Vector &operator=(const Vector &rhs) {
    if (this != &rhs) {
      if (rhs.Size() > data_.Capacity()) {
        Vector rhs_copy(rhs);
        Swap(rhs_copy);
      } else {
        const auto min_copy_size = std::min(Size(), rhs.Size());
        auto copy_pos = std::copy_n(rhs.data_.GetAddress(), min_copy_size, data_.GetAddress());
        if (Size() > rhs.Size()) {
          std::destroy_n(copy_pos, Size() - rhs.Size());
        } else {
          std::uninitialized_copy_n(rhs.data_.GetAddress() + Size(), rhs.Size() - Size(), copy_pos);
        }
        size_ = rhs.Size();
      }
    }

    return *this;
  }

  Vector &operator=(Vector &&rhs) noexcept {
    if (this != &rhs) {
      Swap(rhs);
    }
    return *this;
  }

  ~Vector() {
    DestroyN(data_.GetAddress(), size_);
  }

  void Swap(Vector &other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
  }

  void Reserve(size_t new_capacity) {
    if (new_capacity <= data_.Capacity()) {
      return;
    }
    RawMemory<T> new_data(new_capacity);
    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
      std::uninitialized_move_n(data_.GetAddress(), size_, new_data.GetAddress());
    } else {
      std::uninitialized_copy_n(data_.GetAddress(), size_, new_data.GetAddress());
    }
    std::destroy_n(data_.GetAddress(), size_);
    data_.Swap(new_data);
  }

  size_t Size() const noexcept {
    return size_;
  }

  size_t Capacity() const noexcept {
    return data_.Capacity();
  }

  const T &operator[](size_t index) const noexcept {
    return const_cast<Vector &>(*this)[index];
  }

  T &operator[](size_t index) noexcept {
    assert(index < size_);
    return data_[index];
  }

 private:
  // Выделяет сырую память под n элементов и возвращает указатель на неё
  static T *Allocate(size_t n) {
    return n != 0 ? static_cast<T *>(operator new(n * sizeof(T))) : nullptr;
  }

  // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
  static void Deallocate(T *buf) noexcept {
    operator delete(buf);
  }

  // Вызывает деструкторы n объектов массива по адресу buf
  static void DestroyN(T *buf, size_t n) noexcept {
    std::destroy_n(buf, n);
  }

  // Создаёт копию объекта elem в сырой памяти по адресу buf
  static void CopyConstruct(T *buf, const T &elem) {
    new(buf) T(elem);
  }

  // Вызывает деструктор объекта по адресу buf
  static void Destroy(T *buf) noexcept {
    buf->~T();
  }

  RawMemory<T> data_;
  size_t size_ = 0;
};
