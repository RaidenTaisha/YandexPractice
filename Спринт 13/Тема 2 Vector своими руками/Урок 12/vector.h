#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <new>
#include <utility>

template<typename T>
class RawMemory {
 public:
  RawMemory() = default;

  RawMemory(const RawMemory &other) = delete;
  RawMemory &operator=(const RawMemory &) = delete;

  explicit RawMemory(size_t capacity) : buffer_(Allocate(capacity)), capacity_(capacity) {}

  RawMemory(RawMemory &&other) noexcept {
    buffer_ = std::exchange(other.buffer_, nullptr);
    capacity_ = std::exchange(other.capacity_, 0);
  }

  ~RawMemory() {
    Deallocate(buffer_);
  }

  T *operator+(size_t offset) noexcept {
    // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
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

  RawMemory &operator=(RawMemory &&rhs) noexcept {
    if (this != &rhs) {
      Swap(rhs);
    }

    return *this;
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
  using reference = T &;
  using const_reference = const T &;
  using iterator = T *;
  using const_iterator = const T *;

  Vector() = default;
  Vector(Vector &&other) noexcept: data_(std::move(other.data_)), size_(std::exchange(other.size_, 0)) {}

  Vector(const Vector &other) : data_(other.size_), size_(other.size_) {
    std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
  }

  Vector(size_t size) : data_(size), size_(size) {
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
  }

  ~Vector() {
    DestroyN(data_.GetAddress(), size_);
  }

  Vector &operator=(const Vector &rhs) {
    if (this != &rhs) {
      if (rhs.size_ > data_.Capacity()) {
        Vector rhs_copy(rhs);
        Swap(rhs_copy);
      } else {
        const auto size_copy = rhs.size_ < size_ ? rhs.size_ : size_;
        std::copy_n(rhs.data_.GetAddress(), size_copy, data_.GetAddress());

        if (rhs.size_ < size_) {
          std::destroy_n(data_.GetAddress() + rhs.size_, size_ - rhs.size_);
        } else {
          std::uninitialized_copy_n(rhs.data_.GetAddress() + size_, rhs.size_ - size_, data_.GetAddress() + size_);
        }
        size_ = rhs.size_;
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

  iterator begin() noexcept {
    return data_.GetAddress();
  }

  iterator end() noexcept {
    return data_.GetAddress() + size_;
  }
  const_iterator begin() const noexcept {
    return const_cast<Vector &>(*this).begin();
  }

  const_iterator end() const noexcept {
    return const_cast<Vector &>(*this).end();
  }

  const_iterator cbegin() const noexcept {
    return begin();
  }

  const_iterator cend() const noexcept {
    return end();
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

  void Swap(Vector &other) noexcept {
    if (this != &other) {
      data_.Swap(other.data_);
      std::swap(size_, other.size_);
    }
  }

  size_t Size() const noexcept {
    return size_;
  }

  size_t Capacity() const noexcept {
    return data_.Capacity();
  }

  void Resize(size_t new_size) {
    if (new_size < size_) {
      DestroyN(data_.GetAddress() + new_size, size_ - new_size);
    } else {
      Reserve(new_size);
      std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
    }
    size_ = new_size;
  }

  template<typename... Args>
  reference EmplaceBack(Args &&... args) {
    return *Emplace(end(), std::forward<Args>(args)...);
  }

  template<typename... Args>
  iterator Emplace(const_iterator pos, Args &&... args) {
    auto dst = std::distance(cbegin(), pos);
    if (size_ >= data_.Capacity()) {
      RawMemory<T> new_data(size_ == 0 ? 1 : size_ * 2);
      new(new_data.GetAddress() + dst) T(std::forward<Args>(args)...);
      try {
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
          std::uninitialized_move_n(data_.GetAddress(), dst, new_data.GetAddress());
        } else {
          std::uninitialized_copy_n(data_.GetAddress(), dst, new_data.GetAddress());
        }
      } catch (...) {
        std::destroy_at(new_data.GetAddress() + dst);
        throw;
      }
      try {
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
          std::uninitialized_move_n(data_.GetAddress() + dst, size_ - dst, std::next(new_data.GetAddress() + dst));
        } else {
          std::uninitialized_copy_n(data_.GetAddress() + dst, size_ - dst, std::next(new_data.GetAddress() + dst));
        }
      } catch (...) {
        std::destroy_n(new_data.GetAddress(), dst);
        throw;
      }
      std::destroy_n(data_.GetAddress(), size_);
      data_.Swap(new_data);
    } else {
      if (pos != cend()) {
        T copy(std::forward<Args>(args)...);
        new(data_.GetAddress() + size_) T(std::move(data_[size_ - 1]));
        std::move_backward(data_.GetAddress() + dst, std::prev(data_.GetAddress() + size_), data_.GetAddress() + size_);
        data_[dst] = std::forward<T>(copy);
      } else {
        new(data_.GetAddress() + size_) T(std::forward<Args>(args)...);
      }
    }
    ++size_;
    return data_.GetAddress() + dst;
  }

  iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>) {
    auto dst = std::distance(cbegin(), pos);
    std::destroy_at(data_.GetAddress() + dst);
    std::move(std::next(data_.GetAddress() + dst), data_.GetAddress() + size_, data_.GetAddress() + dst);
    --size_;
    return data_.GetAddress() + dst;
  }

  iterator Insert(const_iterator pos, const_reference value) {
    return Emplace(pos, value);
  }

  iterator Insert(const_iterator pos, T &&value) {
    return Emplace(pos, std::forward<T>(value));
  }

  void PushBack(const_reference value) {
    EmplaceBack(value);
  }

  void PushBack(T &&value) {
    EmplaceBack(std::move(value));
  }

  void PopBack() {
      std::destroy_at(std::prev(end()));
      --size_;
  }

  const_reference operator[](size_t index) const noexcept {
    return const_cast<Vector &>(*this)[index];
  }

  reference operator[](size_t index) noexcept {
    assert(index < size_);
    return data_[index];
  }

 private:
  // Вызывает деструкторы n объектов массива по адресу buf
  static void DestroyN(T *buf, size_t n) noexcept {
    for (size_t i = 0; i != n; ++i) {
      Destroy(buf + i);
    }
  }

  // Вызывает деструктор объекта по адресу buf
  static void Destroy(T *buf) noexcept {
    buf->~T();
  }

  RawMemory<T> data_;
  size_t size_ = 0;
};
