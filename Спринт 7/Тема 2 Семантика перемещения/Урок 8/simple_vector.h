#pragma once

#include "array_ptr.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>

class ReserveProxyObj {
 private:
  size_t capacity_ = 0;

 public:
  ReserveProxyObj() = default;
  explicit ReserveProxyObj(size_t capacity) : capacity_(capacity) {};

  [[nodiscard]] auto GetCapacity() const noexcept {
    return capacity_;
  }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
  return ReserveProxyObj(capacity_to_reserve);
}

template<typename Type>
class SimpleVector {
 public:
  using Iterator = Type *;
  using ConstIterator = const Type *;

  SimpleVector() noexcept = default;

  // Создаёт вектор из size элементов, инициализированных значением по умолчанию
  explicit SimpleVector(size_t size) : SimpleVector(size, Type{}) {}

  // Создаёт вектор из size элементов, инициализированных значением value
  SimpleVector(size_t size, const Type &value) : vector_(size), size_(size), capacity_(size) {
    std::fill(vector_.Get(), vector_.Get() + size, value);
  }

  // Создаёт вектор из std::initializer_list
  SimpleVector(std::initializer_list<Type> init) : vector_(init.size()), size_(init.size()), capacity_(init.size()) {
    std::copy(init.begin(), init.end(), vector_.Get());
  }

  SimpleVector(const SimpleVector &other) : vector_(other.GetCapacity()), size_(other.GetSize()),
                                            capacity_(other.GetCapacity()) {
    std::copy(other.begin(), other.end(), begin());
  }

  SimpleVector(SimpleVector &&other) noexcept: vector_(other.GetCapacity()), size_(other.GetSize()),
                                               capacity_(other.GetCapacity()) {
    std::copy(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), begin());

    std::exchange(other.size_, 0);
    std::exchange(other.capacity_, 0);
  }

  explicit SimpleVector(const ReserveProxyObj obj) : vector_(obj.GetCapacity()), size_(0),
                                                     capacity_(obj.GetCapacity()) {}

  SimpleVector &operator=(const SimpleVector &other) {
    if (this != &other) {
      if (other.IsEmpty()) {
        Clear();
      } else {
        SimpleVector copy(other);
        swap(copy);
      }
    }
    return *this;
  }

  void PushBack(const Type &item) {
    IncrementSize()
    *(end() - 1) = item;
  }

  // Версия для объектов, которые нельзя копировать, но можно перемещать
  void PushBack(Type &&item) {
    IncrementSize();
    std::exchange(*(end() - 1), std::move(item));
  }

  void PopBack() noexcept {
    size_--;
  }

  Iterator Insert(ConstIterator pos, const Type& value) {
    auto pos_to_insert = SafeInsertionPos(pos);
    *pos_to_insert = value;
    return pos_to_insert;
  }

  // Версия для объектов, которые нельзя копировать, но можно перемещать
  Iterator Insert(ConstIterator pos, Type&& value) {
    auto pos_to_insert = SafeInsertionPos(pos);
    std::exchange(*pos_to_insert, std::move(value));
    return pos_to_insert;
  }

  Iterator Erase(ConstIterator pos) {
    auto distance_to_pos = std::distance(cbegin(), pos);
    auto pos_to_erase = begin() + distance_to_pos;

    std::copy(std::make_move_iterator(pos_to_erase + 1), std::make_move_iterator(end()), pos_to_erase);
    size_--;

    return pos_to_erase;
  }

  void swap(SimpleVector &other) noexcept {
    vector_.swap(other.vector_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
  }

  // Возвращает количество элементов в массиве
  size_t GetSize() const noexcept {
    return size_;
  }

  // Возвращает вместимость массива
  size_t GetCapacity() const noexcept {
    return capacity_;
  }

  // Сообщает, пустой ли массив
  bool IsEmpty() const noexcept {
    return size_ == 0;
  }

  // Возвращает ссылку на элемент с индексом index
  Type &operator[](size_t index) noexcept {
    return vector_[index];
  }

  // Возвращает константную ссылку на элемент с индексом index
  const Type &operator[](size_t index) const noexcept {
    return vector_[index];
  }

  // Возвращает константную ссылку на элемент с индексом index
  // Выбрасывает исключение std::out_of_range, если index >= size
  Type &At(size_t index) {
    if (index >= size_)
      throw std::out_of_range("");
    return vector_[index];
  }

  // Возвращает константную ссылку на элемент с индексом index
  // Выбрасывает исключение std::out_of_range, если index >= size
  const Type &At(size_t index) const {
    if (index >= size_)
      throw std::out_of_range("");
    return vector_[index];
  }

  // Обнуляет размер массива, не изменяя его вместимость
  void Clear() noexcept {
    size_ = 0;
  }

  // Изменяет размер массива.
  // При увеличении размера новые элементы получают значение по умолчанию для типа Type
  void Resize(size_t new_size) {
    if (new_size <= capacity_) {
      if (new_size > size_) {
        for (auto it = begin() + size_; it != begin() + new_size; ++it)
          std::exchange(*it, std::move(Type{}));
      }
    } else {
      auto new_capacity = std::max(capacity_ * 2, new_size);
      ArrayPtr<Type> new_vector(new_capacity);
      std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_vector.Get());
      for (auto it = new_vector.Get() + size_; it != new_vector.Get() + new_size; ++it)
        std::exchange(*it, std::move(Type{}));

      vector_.swap(new_vector);
      capacity_ = new_capacity;
    }
    size_ = new_size;
  }

  // Возвращает итератор на начало массива
  // Для пустого массива может быть равен (или не равен) nullptr
  Iterator begin() noexcept {
    return vector_.Get();
  }

  // Возвращает итератор на элемент, следующий за последним
  // Для пустого массива может быть равен (или не равен) nullptr
  Iterator end() noexcept {
    return vector_.Get() + size_;
  }

  // Возвращает константный итератор на начало массива
  // Для пустого массива может быть равен (или не равен) nullptr
  ConstIterator begin() const noexcept {
    return cbegin();
  }

  // Возвращает итератор на элемент, следующий за последним
  // Для пустого массива может быть равен (или не равен) nullptr
  ConstIterator end() const noexcept {
    return cend();
  }

  // Возвращает константный итератор на начало массива
  // Для пустого массива может быть равен (или не равен) nullptr
  ConstIterator cbegin() const noexcept {
    return vector_.Get();
  }

  // Возвращает итератор на элемент, следующий за последним
  // Для пустого массива может быть равен (или не равен) nullptr
  ConstIterator cend() const noexcept {
    return vector_.Get() + size_;
  }

  void Reserve(size_t new_capacity) {
    if (new_capacity > capacity_) {
      ArrayPtr<Type> new_vector(new_capacity);

      std::copy(begin(), end(), new_vector.Get());
      vector_.swap(new_vector);
      capacity_ = new_capacity;
    }
  };

 private:
  ArrayPtr<Type> vector_;
  size_t size_ = 0;
  size_t capacity_ = 0;

  void IncrementSize() {
    if (size_ == capacity_) {
      Resize(size_ + 1);
    } else {
      ++size_;
    }
  }

  Type *SafeInsertionPos(const Type* pos) {
    auto distance_to_pos = std::distance(cbegin(), pos);
    IncrementSize();
    auto pos_to_insert = begin() + distance_to_pos;

    if (size_ > 1) {
      std::copy_backward(std::make_move_iterator(pos_to_insert),
                         std::make_move_iterator(end() - 1), end());
    }

    return pos_to_insert;
  }
};

template<typename Type>
bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
  return &lhs == &rhs || (lhs.GetSize() == rhs.GetSize()
      && std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template<typename Type>
bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
  return !(lhs == rhs);
}

template<typename Type>
bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
  return !(rhs < lhs);
}

template<typename Type>
bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
  return rhs < lhs;
}

template<typename Type>
bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs) {
  return rhs <= lhs;
}
