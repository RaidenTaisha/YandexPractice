#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <set>
#include <vector>

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
      : id(id)
      , relevance(relevance)
      , rating(rating) {
  }

  int id = 0;
  double relevance = 0.0;
  int rating = 0;
};

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
    if (any_of(stop_words.begin(), stop_words.end(), CheckIncorrectInput)) {
      throw invalid_argument("incorrect stop word: ");
    }
  }

  explicit SearchServer(const string& stop_words_text)
      : SearchServer(SplitIntoWords(stop_words_text)) {}

  void AddDocument(int document_id, const string& document, DocumentStatus status,
                   const vector<int>& ratings) {
    if (document_id < 0 or documents_.count(document_id)) {
      throw invalid_argument("incorrect document_id: " + to_string(document_id));
    }
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    if (any_of(words.begin(), words.end(), CheckIncorrectInput)) {
      throw invalid_argument("incorrect word in AddDocument: ");
    }
    for (const string& word : words) {
      word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    if (document_index_.empty()) {
      document_index_.insert({0, document_id});
    } else {
      const int last_value = prev(document_index_.end())->first;
      document_index_.insert({last_value+1, document_id});
    }
  }

  template <typename DocumentPredicate>
  [[nodiscard]] vector<Document> FindTopDocuments(const string& raw_query,
                                                  DocumentPredicate document_predicate) const {
    const Query query = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query, document_predicate);

    sort(matched_documents.begin(), matched_documents.end(),
         [](const Document& lhs, const Document& rhs) {
           if (abs(lhs.relevance - rhs.relevance) < numeric_limits<double>::epsilon()) {
             return lhs.rating > rhs.rating;
           } else {
             return lhs.relevance > rhs.relevance;
           }
         });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
      matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
  }

  [[nodiscard]] vector<Document> FindTopDocuments(const string& raw_query,
                                                  DocumentStatus status = DocumentStatus::ACTUAL) const {
    return FindTopDocuments(
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
          return document_status == status; });
  }

  unsigned long GetDocumentCount() const {
    return documents_.size();
  }

  [[nodiscard]] tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
                                                                    int document_id) const {
    const Query query = ParseQuery(raw_query);
    vector<string> matched_words;
    for (const string& word : query.plus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      if (word_to_document_freqs_.at(word).count(document_id)) {
        matched_words.push_back(word);
      }
    }
    for (const string& word : query.minus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      if (word_to_document_freqs_.at(word).count(document_id)) {
        matched_words.clear();
        break;
      }
    }
    return tuple{matched_words, documents_.at(document_id).status};
  }

  [[nodiscard]] int GetDocumentId(int index) const {
    return document_index_.at(index);
  }

 private:
  struct DocumentData {
    int rating;
    DocumentStatus status;
  };
  const set<string> stop_words_;
  map<string, map<int, double>> word_to_document_freqs_;
  map<int, DocumentData> documents_;
  map<int, int> document_index_;

  [[nodiscard]] bool IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
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

  [[nodiscard]] static bool CheckIncorrectInput(const string& word) {
    if ((word.find("--"s) != -1) || (word == "-"s)) {
      return true;
    }
    const auto index_tilda = word.find('-');
    if (index_tilda != 0) {
      if (index_tilda+1 == word.size() or (index_tilda+1 < word.size() && (word[index_tilda+1] == ' '))) {
        return true;
      }
    }

    if (any_of(word.cbegin(), word.cend(), [](char c){
      return c >= 0 && c <= 31;
    })) {
      return true;
    }
    return false;
  }

  static int ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
      return 0;
    }
    int rating_sum = 0;
    accumulate(ratings.begin(), ratings.end(), rating_sum);
    return rating_sum / static_cast<int>(ratings.size());
  }

  struct QueryWord {
    string data;
    bool is_minus;
    bool is_stop;
  };

  [[nodiscard]] QueryWord ParseQueryWord(string text) const {
    if (CheckIncorrectInput(text)) {
      throw invalid_argument("incorrect raw query: " + text);
    }

    bool is_minus = false;
    if (text[0] == '-') {
      is_minus = true;
      text = text.substr(1);
    }
    return {text, is_minus, IsStopWord(text)};
  }

  struct Query {
    set<string> plus_words;
    set<string> minus_words;
  };

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

  [[nodiscard]] double ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
  }

  template <typename DocumentPredicate>
  [[nodiscard]] vector<Document> FindAllDocuments(const Query& query,
                                                  DocumentPredicate document_predicate) const {
    map<int, double> document_to_relevance;
    for (const string& word : query.plus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
      for (const auto &[document_id, term_freq] : word_to_document_freqs_.at(word)) {
        const auto& document_data = documents_.at(document_id);
        if (document_predicate(document_id, document_data.status, document_data.rating)) {
          document_to_relevance[document_id] += term_freq * inverse_document_freq;
        }
      }
    }

    for (const string& word : query.minus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      for (const auto &[document_id, _] : word_to_document_freqs_.at(word)) {
        document_to_relevance.erase(document_id);
      }
    }

    vector<Document> matched_documents;
    for (const auto &[document_id, relevance] : document_to_relevance) {
      matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
  }
};
