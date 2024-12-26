#include <stdexcept>
#include <utility>

// Исключение этого типа должно генерироватся при обращении к пустому optional
class BadOptionalAccess : public std::exception {
 public:
  using exception::exception;

  virtual const char* what() const noexcept override {
    return "Bad optional access";
  }
};

template <typename T>
class Optional {
 public:
  Optional() = default;

  Optional(const T& value) {
    obj_ = new (&data_[0]) T{ value };
  }

  Optional(T&& value) {
    obj_ = new (&data_[0]) T{ std::move(value) };
  }

  Optional(const Optional& other) {
    if (other.HasValue()) {
      obj_ = new (&data_[0]) T{ other.Value() };
    }
  }

  Optional(Optional&& other) {
    if (other.HasValue()) {
      obj_ = new (&data_[0]) T{ std::move(other.Value()) };
    }
  }

  Optional& operator=(const T& value) {
    if (HasValue()) {
      *obj_ = value;
    } else {
      obj_ = new (&data_[0]) T{ value };
    }

    return *this;
  }
  Optional& operator=(T&& rhs) {
    if (HasValue()) {
      *obj_ = std::move(rhs);
    } else {
      obj_ = new (&data_[0]) T{ std::move(rhs) };
    }

    return *this;
  }
  Optional& operator=(const Optional& rhs) {
    if (this != &rhs) {
      if (!rhs.HasValue()) {
        Reset();
      } else if (HasValue()) {
        *obj_ = rhs.Value();
      } else {
        obj_ = new (&data_[0]) T{ rhs.Value() };
      }
    }

    return *this;
  }
  Optional& operator=(Optional&& rhs) {
    if (this != &rhs) {
      if (!rhs.HasValue()) {
        Reset();
      } else if (HasValue()) {
        *obj_ = std::move(rhs.Value());
      } else {
        obj_ = new (&data_[0]) T{ std::move(rhs.Value()) };
      }
    }

    return *this;
  }

  ~Optional() {
    Reset();
  }

  bool HasValue() const {
    return obj_ != nullptr;
  }

  // Операторы * и -> не должны делать никаких проверок на пустоту Optional.
  // Эти проверки остаются на совести программиста
  T& operator*() & {
    return *obj_;
  }

  const T& operator*() const & {
    return *obj_;
  }

  T&& operator*() && {
    return std::move(*obj_);
  }

  T* operator->() {
    return obj_;
  }

  const T* operator->() const {
    return obj_;
  }

  // Метод Value() генерирует исключение BadOptionalAccess, если Optional пуст
  T& Value() & {
    if (!HasValue()) {
      throw BadOptionalAccess();
    }

    return *obj_;
  }

  const T& Value() const & {
    if (!HasValue()) {
      throw BadOptionalAccess();
    }

    return *obj_;
  }

  T&& Value() && {
    if (!HasValue()) {
      throw BadOptionalAccess();
    }

    return std::move(*obj_);
  }

  void Reset() {
    if (HasValue()) {
      obj_->~T();
      obj_ = nullptr;
    }
  }

  template <typename... Args>
  void Emplace(Args&&... args) {
    Reset();
    obj_ = new (&data_[0]) T{std::forward<Args>(args)...};
  }

 private:
  // alignas нужен для правильного выравнивания блока памяти
  alignas(T) char data_[sizeof(T)];
  T *obj_ = nullptr;
};
