#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <map>
#include <cmath>
#include <vector>
#include <tuple>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
  string s;
  getline(cin, s);
  return s;
}

int ReadLineWithNumber() {
  int result = 0;
  cin >> result;
  ReadLine();
  return result;
}

vector<string> SplitIntoWords(const string &text) {
  vector<string> words;
  string word;
  for (const char c : text) {
    if (c == ' ') {
      if (!word.empty()) {
        words.push_back(word);
        word.clear();
      }
    } else {
      word += c;
    }
  }
  if (!word.empty()) {
    words.push_back(word);
  }

  return words;
}

struct Document {
  Document() = default;
  Document(int id, double relevance, int rating) : id(id), relevance(relevance), rating(rating) {}

  int id = 0;
  double relevance = 0.0;
  int rating = 0;
};

enum class DocumentStatus {
  ACTUAL,
  IRRELEVANT,
  BANNED,
  REMOVED,
};

class SearchServer {
 public:
  inline static constexpr int INVALID_DOCUMENT_ID = -1;

  template <typename StringContainer>
  explicit SearchServer(const StringContainer& stop_words)
      : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {}

  explicit SearchServer(const string& stop_words_text)
      : SearchServer(SplitIntoWords(stop_words_text)) {}

  int GetDocumentId(int index) const {
    if (index >= 0 && index < GetDocumentCount()) {
      return documents_order_[index];
    }
    return INVALID_DOCUMENT_ID;
  }

  [[nodiscard]] bool AddDocument(int document_id, const string &document, DocumentStatus status,
                                 const vector<int> &ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
      return false;
    }
    vector<string> words;
    if (!SplitIntoWordsNoStop(document, words)) {
      return false;
    }

    const double word_tf = 1.0 / words.size();
    for (const string &word : words) {
      word_to_document_freqs_[word][document_id] += word_tf;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    documents_order_.push_back(document_id);
    return true;
  }

  template<class Predicate>
  [[nodiscard]] bool FindTopDocuments(const string &raw_query,
                                      Predicate predicate, vector<Document> &result) const {
    result.clear();
    Query query_words;
    if (!ParseQuery(raw_query, query_words)) {
      return false;
    }
    auto matched_documents = FindAllDocuments(query_words, predicate);
    sort(matched_documents.begin(),
         matched_documents.end(),
         [](const Document &lhs, const Document &rhs) {
           if (abs(lhs.relevance - rhs.relevance) < numeric_limits<double>::epsilon()) {
             return lhs.rating > rhs.rating;
           } else {
             return lhs.relevance > rhs.relevance;
           }
         });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
      matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    result.swap(matched_documents);
    return true;
  }

  [[nodiscard]] bool FindTopDocuments(const string &raw_query,
                                      DocumentStatus requested_status, vector<Document> &result) const {
    return FindTopDocuments(raw_query,
                            [requested_status](int, DocumentStatus status, int) {
                              return requested_status == status;
                            },
                            result);
  }

  [[nodiscard]] bool FindTopDocuments(const string &raw_query, vector<Document> &result) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL, result);
  }

  int GetDocumentCount() const {
    return documents_.size();
  }

  [[nodiscard]] bool MatchDocument(const string &raw_query,
                                   int document_id, tuple<vector<string>, DocumentStatus>& result) const {
    result = {};
    Query query_words;
    if (!ParseQuery(raw_query, query_words)) {
      return false;
    }
    vector<string> matched_words;
    for (const string &word : query_words.plus_words) {
      if (word_to_document_freqs_.find(word) != word_to_document_freqs_.end()) {
        if (word_to_document_freqs_.at(word).count(document_id)) {
          matched_words.push_back(word);
        }
      }
    }
    for (const string &word : query_words.minus_words) {
      if (word_to_document_freqs_.find(word) != word_to_document_freqs_.end()) {
        if (word_to_document_freqs_.at(word).count(document_id)) {
          matched_words.clear();
          break;
        }
      }
    }
    result = {matched_words, documents_.at(document_id).status};
    return true;
  }

 private:
  struct QueryWord {
    string data;
    bool is_minus;
    bool is_stop;
  };

  struct Query {
    set<string> plus_words;
    set<string> minus_words;
  };

  struct DocumentData {
    int rating;
    DocumentStatus status;
  };

  map<string, map<int, double>> word_to_document_freqs_;
  set<string> stop_words_;
  map<int, DocumentData> documents_;
  vector<int> documents_order_;

  [[nodiscard]] bool ParseQueryWord(string text, QueryWord& result) const {
    result = {};
    if (text.empty()) {
      return false;
    }

    bool is_minus = false;
    if (text[0] == '-') {
      is_minus = true;
      text = text.substr(1);
    }

    if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
      return false;
    }

    result = QueryWord{text, is_minus, IsStopWord(text)};
    return true;
  }

  [[nodiscard]] bool ParseQuery(const string &text, Query& result) const {
    result = {};
    for (const string &word : SplitIntoWords(text)) {
      QueryWord query_word;
      if (!ParseQueryWord(word, query_word)) {
        return false;
      }
      if (!query_word.is_stop) {
        if (query_word.is_minus) {
          result.minus_words.insert(query_word.data);
        } else {
          result.plus_words.insert(query_word.data);
        }
      }
    }
    return true;
  }

  bool IsStopWord(const string &word) const {
    return stop_words_.count(word) > 0;
  }

  // Проверка строки на запрещённые символы (с кодами от 0 до 31))
  static bool IsValidWord(const string& word) {
    return none_of(word.begin(), word.end(), [](char c) {
      return c >= '\0' && c < ' ';
    });
  }

  // Возвращает множество уникальных НЕ пустых строк
  template <typename StringContainer>
  set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    set<string> non_empty_strings;
    for (const string& str : strings)
      if (!str.empty()) {
        if (!IsValidWord(str))
          throw invalid_argument("Special character detected");

        non_empty_strings.insert(str);
      }
    return non_empty_strings;
  }

  [[nodiscard]] bool SplitIntoWordsNoStop(const string& text, vector<string>& result) const {
    result.clear();
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
      if (!IsValidWord(word)) {
        return false;
      }
      if (!IsStopWord(word)) {
        words.push_back(word);
      }
    }
    result.swap(words);
    return true;
  }

  static int ComputeAverageRating(const vector<int> &ratings) {
    if (ratings.empty()) {
      return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
      rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
  }

  inline double CalculateIdf(const string &word) const {
    return log(static_cast<double>(GetDocumentCount()) / word_to_document_freqs_.at(word).size());
  }

  template<typename Predicate>
  vector<Document> FindAllDocuments(const Query &query_words, Predicate predicate) const {
    vector<Document> matched_documents;
    map<int, double> id_relevance;

    for (const auto &word : query_words.plus_words) {
      if (word_to_document_freqs_.find(word) != word_to_document_freqs_.end()) {
        const double idf = CalculateIdf(word);
        for (const auto &[id, tf] : word_to_document_freqs_.at(word)) {
          DocumentData document = documents_.at(id);
          if (predicate(id, document.status, document.rating)) {
            id_relevance[id] += tf * idf;
          }
        }
      }
    }

    for (const string &word : query_words.minus_words) {
      if (word_to_document_freqs_.find(word) != word_to_document_freqs_.end()) {
        for (const auto &[document_id, _] : word_to_document_freqs_.at(word)) {
          id_relevance.erase(document_id);
        }
      }
    }

    for (const auto &[id, rel] : id_relevance) {
      matched_documents.push_back({id, rel, documents_.at(id).rating});
    }

    return matched_documents;
  }
};

void PrintDocument(const Document& document) {
  cout << "{ "s
       << "document_id = "s << document.id << ", "s
       << "relevance = "s << document.relevance << ", "s
       << "rating = "s << document.rating << " }"s << endl;
}

int main() {
  SearchServer search_server("и в на"s);
  // Явно игнорируем результат метода AddDocument, чтобы избежать предупреждения
  // о неиспользуемом результате его вызова
  (void) search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
  if (!search_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2})) {
    cout << "Документ не был добавлен, так как его id совпадает с уже имеющимся"s << endl;
  }
  if (!search_server.AddDocument(-1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2})) {
    cout << "Документ не был добавлен, так как его id отрицательный"s << endl;
  }
  if (!search_server.AddDocument(3, "большой пёс скво\x12рец"s, DocumentStatus::ACTUAL, {1, 3, 2})) {
    cout << "Документ не был добавлен, так как содержит спецсимволы"s << endl;
  }
  (void) search_server.AddDocument(3, "большой пёс скво\x12рец"s, DocumentStatus::ACTUAL, {1, 3, 2});
  cout << "ID документа, добавленного нулевым по счёту: " << search_server.GetDocumentId(0) << endl;
  cout << "ID документа, добавленного первым по счёту: " << search_server.GetDocumentId(1) << endl;
  cout << "ID документа, добавленного вторым по счёту: " << search_server.GetDocumentId(2) << endl;
  cout << "ID документа, добавленного минус первым по счёту: " << search_server.GetDocumentId(-1) << endl;
  vector<Document> documents;
  if (search_server.FindTopDocuments("--пушистый"s, documents)) {
    for (const Document& document : documents) {
      PrintDocument(document);
    }
  } else {
    cout << "Ошибка в поисковом запросе"s << endl;
  }
}
