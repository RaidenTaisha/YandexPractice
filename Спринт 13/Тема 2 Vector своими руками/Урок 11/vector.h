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

  explicit RawMemory(size_t capacity)
      : buffer_(Allocate(capacity)), capacity_(capacity) {
  }

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
  // Выделяет сырую память под n элементов и возвращает указатель на неё
  static T *Allocate(size_t n) {
    return n != 0 ? static_cast<T *>(operator new(n * sizeof(T))) : nullptr;
  }

  // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
  static void Deallocate(T *buf) noexcept {
    operator delete(buf);
  }

  T *buffer_ = nullptr;
  size_t capacity_ = 0;
};

template<typename T>
class Vector {
 public:
  Vector() = default;

  explicit Vector(size_t size) : data_(size), size_(size) {
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
  }

  Vector(const Vector &other) : data_(other.Size()), size_(other.Size()) {
    std::uninitialized_copy_n(other.data_.GetAddress(), other.Size(),
                              data_.GetAddress());
  }

  Vector(Vector &&other) {
    data_ = std::exchange(other.data_, {});
    size_ = std::exchange(other.size_, 0);
  }

  ~Vector() {
    std::destroy_n(data_.GetAddress(), Size());
  }

  size_t Size() const noexcept {
    return size_;
  }

  size_t Capacity() const noexcept {
    return data_.Capacity();
  }

  void Reserve(size_t new_capacity) {
    if (new_capacity <= data_.Capacity()) {
      return;
    }

    RawMemory<T> new_data(new_capacity);
    if constexpr (!std::is_copy_constructible_v<T> || std::is_nothrow_move_constructible_v<T>) {
      std::uninitialized_move_n(data_.GetAddress(), Size(), new_data.GetAddress());
    } else {
      std::uninitialized_copy_n(data_.GetAddress(), Size(), new_data.GetAddress());
    }

    std::destroy_n(data_.GetAddress(), Size());
    data_.Swap(new_data);
  }

  void Swap(Vector &other) noexcept {
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
  }

  void Resize(size_t new_size) {
    if (new_size < Size()) {
      std::destroy_n(data_.GetAddress() + new_size, Size() - new_size);
    } else if (new_size > Size()) {
      Reserve(new_size);
      std::uninitialized_value_construct_n(data_.GetAddress() + Size(), new_size - Size());
    }

    size_ = new_size;
  }

  void PushBack(const T &value) {
    PushBackInternal(value);
  }

  void PushBack(T &&value) {
    PushBackInternal(std::move(value));
  }

  template<typename... Args>
  T &EmplaceBack(Args &&... args) {
    if (size_ < Capacity()) {
      new(data_ + Size()) T(std::forward<Args>(args)...);
    } else {
      RawMemory<T> new_data(Size() == 0 ? 1 : Size() * 2);
      new(new_data + Size()) T(std::forward<Args>(args)...);

      try {
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
          std::uninitialized_move_n(data_.GetAddress(), Size(), new_data.GetAddress());
        } else {
          std::uninitialized_copy_n(data_.GetAddress(), Size(), new_data.GetAddress());
        }
      } catch (...) {
        std::destroy_at(new_data + Size());
        throw;
      }

      std::destroy_n(data_.GetAddress(), Size());
      data_.Swap(new_data);
    }

    return data_[size_++];
  }

  void PopBack() noexcept {
    --size_;
    std::destroy_at(data_ + Size());
  }

  const T &operator[](size_t index) const noexcept {
    return const_cast<Vector &>(*this)[index];
  }

  T &operator[](size_t index) noexcept {
    assert(index < size_);
    return data_[index];
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

 private:
  RawMemory<T> data_;
  size_t size_ = 0;

  template<typename V>
  void PushBackInternal(V &&value) {
    if (size_ < Capacity()) {
      // Реаллокация памяти не требуется. Конструируем новый элемент
      new(data_ + Size()) T(std::forward<V>(value));
    } else {
      // Требуется реаллокация памяти.
      // Элемент value потенциально может располагаться в текущем векторе
      // (внутри data_), поэтому нельзя уничтожать data_ до завершения вставки
      RawMemory<T> new_data(Size() == 0 ? 1 : Size() * 2);
      // Конструируем вставляемый элемент
      new(new_data + Size()) T(std::forward<V>(value));

      try {
        // Переносим либо копируем оставшиеся элементы
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
          std::uninitialized_move_n(data_.GetAddress(), Size(), new_data.GetAddress());
        } else {
          std::uninitialized_copy_n(data_.GetAddress(), Size(), new_data.GetAddress());
        }
      } catch (...) {
        // В случае выброса исключений, вставленный элемент необходимо разрушить
        std::destroy_at(new_data + Size());
        throw;
      }

      std::destroy_n(data_.GetAddress(), Size());
      data_.Swap(new_data);
    }

    ++size_;
  }
};