#include <iostream>
#include <list>
#include <set>
#include <string_view>
#include <vector>

using namespace std;

// Измените сигнатуру этой функции, чтобы она могла
// сливать не только векторы, но и любые другие контейнеры
template<typename It1, typename It2>
void Merge(It1 first_begin,
           It1 first_end,
           It2 second_begin,
           It2 second_end,
           std::ostream &out) {
  auto it1 = first_begin;
  auto it2 = second_begin;
  while (it1 != first_end && it2 != second_end) {
    if (std::less{}(*it1, *it2)) {
      out << *(it1++) << std::endl;
    } else {
      out << *(it2++) << std::endl;
    }
  }

  for (; it1 != first_end; ++it1) {
    out << *it1 << std::endl;
  }

  for (; it2 != second_end; ++it2) {
    out << *it2 << std::endl;
  }
}

template<typename T, typename S>
void MergeSomething(const T &src1, const S &src2, ostream &out) {
  Merge(src1.begin(), src1.end(), src2.begin(), src2.end(), out);
}

template<typename T>
void MergeHalves(const vector<T> &src, ostream &out) {
  size_t mid = (src.size() + 1) / 2;
  Merge(src.begin(), src.begin() + mid, src.begin() + mid, src.end(), out);
}

int main() {
  vector<int> v1{60, 70, 80, 90};
  vector<int> v2{65, 75, 85, 95};
  vector<int> combined{60, 70, 80, 90, 65, 75, 85, 95};
  list<double> my_list{0.1, 72.5, 82.11, 1e+30};
  string_view my_string = "ACNZ"sv;
  set<unsigned> my_set{20u, 77u, 81u};

  // пока функция MergeSomething реализована пока только для векторов
  cout << "Merging vectors:"sv << endl;
  MergeSomething(v1, v2, cout);

  cout << "Merging vector and list:"sv << endl;
  MergeSomething(v1, my_list, cout);

  cout << "Merging string and list:"sv << endl;
  MergeSomething(my_string, my_list, cout);

  cout << "Merging set and vector:"sv << endl;
  MergeSomething(my_set, v2, cout);

  cout << "Merging vector halves:"sv << endl;
  MergeHalves(combined, cout);
}
