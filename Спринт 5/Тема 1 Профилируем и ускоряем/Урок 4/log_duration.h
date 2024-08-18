#pragma once

#include <chrono>
#include <iostream>
#include <utility>

class LogDuration {
 public:
  // заменим имя типа std::chrono::steady_clock
  // с помощью using для удобства
  using Clock = std::chrono::steady_clock;

  LogDuration(std::string title) : title(std::move(title)) {}

  ~LogDuration() {
    using namespace std::chrono;
    using namespace std::literals;

    const auto end_time = Clock::now();
    const auto dur = end_time - start_time_;
    std::cerr << title << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
  }

 private:
  const Clock::time_point start_time_ = Clock::now();
  const std::string title;
};
