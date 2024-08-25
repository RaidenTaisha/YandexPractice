#pragma once

#include <cassert>
#include <stdexcept>
#include <cstddef>
#include <initializer_list>

template<typename Type>
class SimpleVector {
 public:
  using Iterator = Type *;
  using ConstIterator = const Type *;

  SimpleVector() noexcept = default;
  ~SimpleVector() {
    delete[] vector_;
  };

  // Создаёт вектор из size элементов, инициализированных значением по умолчанию
  explicit SimpleVector(size_t size) {
    if (size == 0)
      return;
    vector_ = new Type[size]();
    capacity_ = size;
    size_ = size;
  }

  // Создаёт вектор из size элементов, инициализированных значением value
  SimpleVector(size_t size, const Type &value) {
    if (size == 0)
      return;
    vector_ = new Type[size];
    capacity_ = size;
    size_ = size;
    for (size_t i = 0; i < size; i++)
      vector_[i] = value;
  }

  // Создаёт вектор из std::initializer_list
  SimpleVector(std::initializer_list<Type> init) {
    vector_ = new Type[init.size()];
    capacity_ = init.size();
    size_ = init.size();
    auto it = init.begin();
    for (size_t i = 0; i < size_; i++)
      vector_[i] = *(it + i);
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
    if (new_size < size_)
      size_ = new_size;
    if (new_size >= capacity_) {
      Type *new_vector = new Type[new_size]();
      for (size_t i = 0; i < size_; i++)
        new_vector[i] = vector_[i];
      delete[] vector_;
      vector_ = new_vector;
      size_ = new_size;
      capacity_ = new_size;
    }
    for (size_t i = 0; i < new_size - size_; i++) {
      vector_[size_ + i] = Type();
    }
  }

  // Возвращает итератор на начало массива
  // Для пустого массива может быть равен (или не равен) nullptr
  Iterator begin() noexcept {
    if (size_ == 0)
      return nullptr;
    return vector_;
  }

  // Возвращает итератор на элемент, следующий за последним
  // Для пустого массива может быть равен (или не равен) nullptr
  Iterator end() noexcept {
    if (size_ == 0)
      return nullptr;
    return &vector_[size_];
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
    if (size_ == 0)
      return nullptr;
    return const_cast<ConstIterator>(vector_);
  }

  // Возвращает итератор на элемент, следующий за последним
  // Для пустого массива может быть равен (или не равен) nullptr
  ConstIterator cend() const noexcept {
    if (size_ == 0)
      return nullptr;
    return const_cast<ConstIterator>(&vector_[size_]);
  }
 private:
  Type *vector_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = 0;
};
