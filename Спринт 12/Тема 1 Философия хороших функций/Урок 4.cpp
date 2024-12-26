#include <algorithm>
#include <iostream>
#include <vector>

using namespace std::string_view_literals;

class ReadingManager {
 public:
  // Т.к. нумерация страниц и пользователей начинается с единицы - необходимо
  // увеличить размеры векторов на единицу
  ReadingManager() : user_id_to_page_(max_users_ + 1, default_user_id_),
                     users_count_to_page_(max_pages_ + 1, 0) {}

  void Read(int user_id, int page) {
    // Индекс страницы, на которой остановился пользователь user_id
    // Если пользователь новый, то индекс будет равен нулю (-1 + 1 = 0)
    auto index = user_id_to_page_[user_id] + 1;
    for (auto i = index; i < page + 1; ++i) {
      ++users_count_to_page_[i];
    }

    user_id_to_page_[user_id] = page;
  }

  double Cheer(int user_id) const {
    const auto page = user_id_to_page_[user_id];
    // Если для данного пользователя не было ни одного события READ
    if (page == -1) {
      return 0;
    }

    const auto user_count = GetUserCount();
    // Если пользователь на данный момент единственный
    if (user_count == 1) {
      return 1;
    }

    return static_cast<double>(user_count - users_count_to_page_[page])
        / static_cast<double>(user_count - 1);
  }

 private:
  // Id пользователей не превосходит 10^5 (100000)
  const size_t max_users_ = 100'000;
  // Номера страниц не превосходят 1000
  const size_t max_pages_ = 1'000;
  // Значения по умолчанию для id пользователей равно минус единице - если для
  // пользователя не было ни одного события READ (новый пользователь)
  const int default_user_id_ = -1;

  // Индекс элемента - id пользователя,
  // значение элемента - число прочитанных страниц (номер страницы)
  std::vector<int> user_id_to_page_;

  // Индекс элемента - номер страницы,
  // значение элемента - количество пользователей дочитавших до текущей страницы
  std::vector<int> users_count_to_page_;

  size_t GetUserCount() const {
    // Каждый новый пользователь, когда читает, инкрементирует значение нулевого
    // индекса. Т.о. это значение равно количеству пользователей
    return users_count_to_page_[0];
  }
};

int main() {
  ReadingManager rm;

  size_t query_count;
  std::cin >> query_count;

  for (size_t i = 0; i < query_count; ++i) {
    int user_id, page;
    std::string query;

    std::cin >> query >> user_id;
    if (query == "READ"sv) {
      std::cin >> page;
      rm.Read(user_id, page);
    } else if (query == "CHEER"sv) {
      std::cout << rm.Cheer(user_id) << std::endl;
    }
  }

  return 0;
}
