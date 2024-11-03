#pragma once

#include <utility>

namespace raii {

template<typename Provider>
class Booking {
 private:
  using BookingId = typename Provider::BookingId;

  Provider *provider_;
  BookingId id_;

 public:
  Booking(Provider *p, BookingId id) : provider_(p), id_(id) {}

  Booking(const Booking &) = delete;

  Booking(Booking &&other) {
    provider_ = std::exchange(other.provider_, nullptr);
    id_ = std::exchange(other.id_, 0);
  }

  ~Booking() {
    if (provider_ != nullptr) {
      provider_->CancelOrComplete(*this);
    }
  }

  Booking &operator=(const Booking &) = delete;

  Booking &operator=(Booking &&other) {
    provider_ = std::exchange(other.provider_, nullptr);
    id_ = std::exchange(other.id_, 0);

    return *this;
  }

  // Эта функция не требуется в тестах, но в реальной программе она может быть нужна
  BookingId GetId() const {
    return id_;
  }
};

} // namespace raii