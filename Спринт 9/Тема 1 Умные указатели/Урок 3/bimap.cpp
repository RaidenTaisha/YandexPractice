#include "bimap.h"

#include <deque>
#include <unordered_map>
#include <string>

struct BiMap::Impl {
  bool Add(std::string_view key, std::string_view value) {
    if (key_to_value.find(key) == key_to_value.end() || value_to_key.find(value) == value_to_key.end()) {
      return false;
    }

    auto& [k, v] = storage.emplace_back(std::string(key), std::string(value));
    key_to_value.emplace(k, v);
    value_to_key.emplace(v, k);

    return true;
  }

  std::optional<std::string_view> FindValue(std::string_view key) const noexcept {
    if(auto it = key_to_value.find(key); it != key_to_value.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  std::optional<std::string_view> FindKey(std::string_view value) const noexcept {
    if(auto it = value_to_key.find(value); it != value_to_key.end()) {
      return it->second;
    }
    return std::nullopt;
  }

 private:
  std::unordered_map<std::string_view, std::string_view> key_to_value;
  std::unordered_map<std::string_view, std::string_view> value_to_key;
  std::deque<std::pair<std::string, std::string>> storage;
};

BiMap::BiMap() : impl_(std::make_unique<Impl>()) {}

BiMap::~BiMap() = default;
BiMap::BiMap(BiMap&& other) noexcept = default;
BiMap& BiMap::operator=(BiMap&&) noexcept = default;

BiMap::BiMap(const BiMap& other) : impl_(std::make_unique<Impl>(*other.impl_)){}

BiMap& BiMap::operator=(const BiMap& other) {
  if (this != std::addressof(other)) {
    impl_ = other.impl_ ? std::make_unique<Impl>(*other.impl_) : nullptr;
  }
  return *this;
}

bool BiMap::Add(std::string_view key, std::string_view value) {
  return impl_->Add(key, value);
}

std::optional<std::string_view> BiMap::FindValue(std::string_view key) const noexcept {
  return impl_->FindValue(key);
}

std::optional<std::string_view> BiMap::FindKey(std::string_view value) const noexcept {
  return impl_->FindKey(value);
}
