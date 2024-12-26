#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <unordered_map>

#include "log_duration.h"

using namespace std;

class RandomContainer {
 public:
  void Insert(int val) {
    values_pool_.push_back(val);
    val_to_index[val] = values_pool_.size() - 1;
  }
  void Remove(int val) {
    values_pool_[val_to_index[val]] = values_pool_.back();
    val_to_index[values_pool_.back()] = val_to_index[val];
    values_pool_.pop_back();
    val_to_index.erase(val);
  }
  bool Has(int val) const {
    return val_to_index.find(val) != val_to_index.end();
  }
  int GetRand() const {
    uniform_int_distribution<int> distr(0, values_pool_.size() - 1);
    int rand_index = distr(engine_);
    return values_pool_[rand_index];
  }

 private:
  vector<int> values_pool_;
  unordered_map<int, int> val_to_index;
  mutable mt19937 engine_;
};

int main() {
  RandomContainer container;
  int query_num = 0;
  cin >> query_num;
  {
    LOG_DURATION("Requests handling"s);
    for (int query_id = 0; query_id < query_num; query_id++) {
      string query_type;
      cin >> query_type;
      if (query_type == "INSERT"s) {
        int value = 0;
        cin >> value;
        container.Insert(value);
      } else if (query_type == "REMOVE"s) {
        int value = 0;
        cin >> value;
        container.Remove(value);
      } else if (query_type == "HAS"s) {
        int value = 0;
        cin >> value;
        if (container.Has(value)) {
          cout << "true"s << endl;
        } else {
          cout << "false"s << endl;
        }
      } else if (query_type == "RAND"s) {
        cout << container.GetRand() << endl;
      }
    }
  }
}
