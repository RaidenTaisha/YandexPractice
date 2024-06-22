#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

template<class Documents, class Term>
std::vector<double> ComputeTfIdfs(const Documents &documents, const Term &term) {
  std::vector<double> ret;
  for (auto document : documents) {
    double tf, idf, count, size = 0;
    { // TF-algorithm
      count = std::count(document.begin(), document.end(), term);
      size = document.size();
      tf = count / size;
    }

    { //IDF-algorithm
      count = count_if(documents.begin(),
                       documents.end(),
                       [term](const auto &doc) {
                         bool does_doc_contain_word = std::count(doc.begin(), doc.end(), term) > 0;
                         return does_doc_contain_word;
                       });
      size = documents.size();
      idf = log(size / count);
    }

    ret.push_back(tf * idf);
  }
  return ret;
}

int main() {
  const std::vector<std::vector<std::string>> documents = {
      {"белый", "кот", "и", "модный", "ошейник"},
      {"пушистый", "кот", "пушистый", "хвост"},
      {"ухоженный", "пёс", "выразительные", "глаза"},
  };
  const auto tf_idfs = ComputeTfIdfs(documents, "кот");
  for (const double tf_idf : tf_idfs) {
    std::cout << tf_idf << " ";
  }
  std::cout << std::endl;
  return 0;
}
