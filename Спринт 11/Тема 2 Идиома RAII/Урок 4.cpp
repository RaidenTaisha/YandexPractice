#include <iostream>
#include <deque>
#include <unordered_map>
#include <unordered_set>

using namespace std;

const int64_t one_day_sec = 86400;

class HotelManager {
 public:
  void Book(int64_t time, std::string &hotel_name, int client_id,
            int room_count) {
    current_time_ = time;
    ++hotel_to_books_[hotel_name];
    hotel_to_rooms_[hotel_name] += room_count;

    books_.push_back({time, hotel_name, client_id, room_count});
    while (!is_booked_in_current_day(books_.front().time)) {
      auto &book = books_.front();

      hotel_to_rooms_[book.hotel_name] -= book.room_count;

      if (hotel_to_books_.count(book.hotel_name) != 0) {
        --hotel_to_books_[book.hotel_name];
      }

      books_.pop_front();
    }

  }

  int ComputeClientCount(std::string &hotel_name) {
    if (hotel_to_books_.count(hotel_name) == 0) {
      return 0;
    }

    std::unordered_set<int> ids;
    for (const auto &b : books_) {
      if (b.hotel_name == hotel_name) {
        ids.insert(b.client_id);
      }
    }

    return static_cast<int>(ids.size());
  }

  int ComputeRoomCount(std::string &hotel_name) {
    if (hotel_to_books_.count(hotel_name) == 0) {
      return 0;
    }

    return hotel_to_rooms_.at(hotel_name);
  }

 private:
  struct book {
    int64_t time;
    std::string hotel_name;
    int client_id;
    int room_count;
  };
  std::deque<book> books_;
  int64_t current_time_ = 0;

  std::unordered_map<std::string, int> hotel_to_books_;
  std::unordered_map<std::string, int> hotel_to_rooms_;

  bool is_booked_in_current_day(int64_t time) const {
    return (time > (current_time_ - 86400)) && (time <= current_time_);
  }
};

int main() {
  HotelManager manager;

  int query_count;
  cin >> query_count;

  for (int query_id = 0; query_id < query_count; ++query_id) {
    string query_type;
    cin >> query_type;

    if (query_type == "BOOK") {
      int64_t time;
      cin >> time;
      string hotel_name;
      cin >> hotel_name;
      int client_id, room_count;
      cin >> client_id >> room_count;
      manager.Book(time, hotel_name, client_id, room_count);
    } else {
      string hotel_name;
      cin >> hotel_name;
      if (query_type == "CLIENTS") {
        cout << manager.ComputeClientCount(hotel_name) << "\n";
      } else if (query_type == "ROOMS") {
        cout << manager.ComputeRoomCount(hotel_name) << "\n";
      }
    }
  }

  return 0;
}
