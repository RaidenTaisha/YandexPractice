#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
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
  int result;
  cin >> result;
  ReadLine();
  return result;
}

template <typename StringContainer>
set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
  set<string> non_empty_strings;
  for (const string& str : strings) {
    if (!str.empty()) {
      non_empty_strings.insert(str);
    }
  }
  return non_empty_strings;
}

vector<string> SplitIntoWords(const string& text) {
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

  Document(int id, double relevance, int rating)
      : id(id), relevance(relevance), rating(rating) {}

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
  template <typename StringContainer>
  explicit SearchServer(const StringContainer& stop_words)
      : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
    for (const auto& word : stop_words_) {
      if (!IsValidWord(word)) {
        throw invalid_argument("Invalid stop word");
      }
    }
  }

  explicit SearchServer(const string& stop_words_text)
      : SearchServer(SplitIntoWords(stop_words_text)) {}

  void AddDocument(int document_id,
                   const string& document,
                   DocumentStatus status,
                   const vector<int>& ratings) {
    if (document_id < 0 or documents_.count(document_id)) {
      throw invalid_argument("Invalid document id in AddDocument: "
                                 + to_string(document_id));
    }
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double word_tf = 1.0 / static_cast<double>(words.size());
    for (const auto& word : words) {
      if (!IsValidWord(word)) {
        throw invalid_argument("Invalid word in AddDocument");
      }
    }
    for (const string& word : words) {
      word_to_document_freqs_[word][document_id] += word_tf;
    }
    documents_.emplace(document_id,
                       DocumentData{ComputeAverageRating(ratings), status});
    if (documents_order_and_id_.empty()) {
      documents_order_and_id_.emplace_back(0, document_id);
    } else {
      int next = documents_order_and_id_.back().first + 1;
      documents_order_and_id_.emplace_back(next, document_id);
    }
  }

  template <typename Predicate>
  [[nodiscard]] vector<Document> FindTopDocuments(const string& raw_query,
                                                  Predicate predicate) const {
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, predicate);

    sort(matched_documents.begin(), matched_documents.end(),
         [](const Document& lhs, const Document& rhs) {
           if (abs(lhs.relevance - rhs.relevance)
               < numeric_limits<double>::epsilon()) {
             return lhs.rating > rhs.rating;
           }
           return lhs.relevance > rhs.relevance;
         });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
      matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
  }

  [[nodiscard]] vector<Document> FindTopDocuments(const string& raw_query,
                                                  DocumentStatus status = DocumentStatus::ACTUAL) const {
    return FindTopDocuments(
        raw_query,
        [status](int document_id, DocumentStatus document_status, int rating) {
          return document_status == status;
        });
  }

  [[nodiscard]] unsigned long GetDocumentCount() const {
    return documents_.size();
  }

  [[nodiscard]] tuple<vector<string>,
                      DocumentStatus> MatchDocument(const string& raw_query,
                                                    int document_id) const {
    const Query query = ParseQuery(raw_query);
    vector<string> matched_words;
    for (const string& word : query.plus_words) {
      if (word_to_document_freqs_.find(word) != word_to_document_freqs_.end()) {
        if (word_to_document_freqs_.at(word).count(document_id)) {
          matched_words.push_back(word);
        }
      }
    }
    for (const string& word : query.minus_words) {
      if (word_to_document_freqs_.find(word) != word_to_document_freqs_.end()) {
        if (word_to_document_freqs_.at(word).count(document_id)) {
          matched_words.clear();
          break;
        }
      }
    }
    return tuple{matched_words, documents_.at(document_id).status};
  }

  [[nodiscard]] int GetDocumentId(int index) const {
    return documents_order_and_id_.at(index).second;
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
  const set<string> stop_words_;
  map<int, DocumentData> documents_;
  vector<pair<int, int>> documents_order_and_id_;

  [[nodiscard]] bool IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
  }

  [[nodiscard]] static bool IsValidWord(const string& word) {
    return none_of(word.begin(), word.end(), [](char c) {
      return c >= 0 && c <= 31;
    });
  }

  [[nodiscard]] QueryWord ParseQueryWord(string text) const {
    bool is_minus = false;

    if (text.empty()) {
      throw invalid_argument("Word is empty");
    }

    if (text.front() == '-') {
      text = text.substr(1);
      is_minus = true;
    }

    if (text.empty() || text[0] == '-' || !IsValidWord(text)
        || (text.find("- ") != string::npos)) {
      throw invalid_argument("Incorrect raw query: ");
    }

    return {text, is_minus, IsStopWord(text)};
  }

  [[nodiscard]] Query ParseQuery(const string& text) const {
    Query query;
    for (const string& word : SplitIntoWords(text)) {
      const QueryWord query_word = ParseQueryWord(word);
      if (!query_word.is_stop) {
        if (query_word.is_minus) {
          query.minus_words.insert(query_word.data);
        } else {
          query.plus_words.insert(query_word.data);
        }
      }
    }
    return query;
  }

  [[nodiscard]] vector<string> SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
      if (!IsStopWord(word)) {
        words.push_back(word);
      }
    }
    return words;
  }

  static int ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
      return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
      rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
  }

  [[nodiscard]] inline double CalculateIdf(const string& word) const {
    return log(static_cast<double>(GetDocumentCount())
                   / static_cast<double>(word_to_document_freqs_.at(word)
                       .size()));
  }

  template <typename DocumentPredicate>
  [[nodiscard]] vector<Document> FindAllDocuments(const Query& query,
                                                  DocumentPredicate predicate) const {
    map<int, double> id_to_relevance;
    for (const auto& word : query.plus_words) {
      if (word_to_document_freqs_.find(word) != word_to_document_freqs_.end()) {
        const double idf = CalculateIdf(word);
        for (const auto& [id, tf] : word_to_document_freqs_.at(word)) {
          DocumentData document = documents_.at(id);
          if (predicate(id, document.status, document.rating)) {
            id_to_relevance[id] += tf * idf;
          }
        }
      }
    }

    for (const string& word : query.minus_words) {
      if (word_to_document_freqs_.find(word) != word_to_document_freqs_.end()) {
        for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
          id_to_relevance.erase(document_id);
        }
      }
    }

    vector<Document> matched_documents;
    for (const auto& [id, rel] : id_to_relevance) {
      matched_documents.emplace_back(id, rel, documents_.at(id).rating);
    }
    return matched_documents;
  }
};
