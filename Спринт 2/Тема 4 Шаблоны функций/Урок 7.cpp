#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <map>
#include <cmath>
#include <vector>

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
  int id;
  double relevance;
  int rating;
};

enum class DocumentStatus {
  ACTUAL,
  IRRELEVANT,
  BANNED,
  REMOVED,
};

class SearchServer {
 public:
  void SetStopWords(const string &text) {
    for (const string &word : SplitIntoWords(text)) {
      stop_words_.insert(word);
    }
  }

  void AddDocument(int document_id, const string &document, DocumentStatus status,
                   const vector<int> &ratings) {
    const vector<string> words = SplitIntoWordsNoStop(document);
    double word_tf = 1. / words.size();
    for (const string &word : words) {
      word_to_document_freqs_[word][document_id] += word_tf;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
  }

  template<class Predicate>
  vector<Document> FindTopDocuments(const string &raw_query,
                                    Predicate predicate) const {
    const Query query_words = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query_words, predicate);
    sort(matched_documents.begin(),
         matched_documents.end(),
         [](const Document &lhs, const Document &rhs) {
           bool ret;
           if (abs(lhs.relevance - rhs.relevance) < numeric_limits<double>::epsilon()) {
             ret = lhs.rating > rhs.rating;
           } else {
             ret = lhs.relevance > rhs.relevance;
           }
           return ret;
         });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
      matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
  }

  vector<Document> FindTopDocuments(const string &raw_query,
                                    DocumentStatus requested_status = DocumentStatus::ACTUAL) const {
    return FindTopDocuments(raw_query,
                            [requested_status](int, DocumentStatus status, int) {
                              return requested_status == status;
                            });
  }

  int GetDocumentCount() const {
    return documents_.size();
  }

  tuple<vector<string>, DocumentStatus> MatchDocument(const string &raw_query,
                                                      int document_id) const {
    const Query query_words = ParseQuery(raw_query);
    vector<string> matched_words;
    for (const string &word : query_words.plus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      if (word_to_document_freqs_.at(word).count(document_id)) {
        matched_words.push_back(word);
      }
    }
    for (const string &word : query_words.minus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      if (word_to_document_freqs_.at(word).count(document_id)) {
        matched_words.clear();
        break;
      }
    }
    return {matched_words, documents_.at(document_id).status};
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

  QueryWord ParseQueryWord(string text) const {
    bool is_minus = false;
    if (text.front() == '-') {
      is_minus = true;
      text = text.substr(1);
    }
    return {text, is_minus, IsStopWord(text)};
  }

  Query ParseQuery(const string &text) const {
    Query query;
    for (const string &word : SplitIntoWords(text)) {
      const QueryWord query_word = ParseQueryWord(word);
      if (!query_word.is_stop) {
        if (query_word.is_minus) {
          query.minus_words.insert(query_word.data);
        } else { query.plus_words.insert(query_word.data); }
      }
    }
    return query;
  }

  bool IsStopWord(const string &word) const {
    return stop_words_.count(word) > 0;
  }

  vector<string> SplitIntoWordsNoStop(const string &text) const {
    vector<string> words;
    for (const string &word : SplitIntoWords(text)) {
      if (!IsStopWord(word)) {
        words.push_back(word);
      }
    }
    return words;
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

void PrintDocument(const Document &document) {
  cout << "{ "s
       << "document_id = "s << document.id << ", "s
       << "relevance = "s << document.relevance << ", "s
       << "rating = "s << document.rating
       << " }"s << endl;
}

int main() {
  SearchServer search_server;
  search_server.SetStopWords("и в на"s);
  search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
  search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
  search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
  search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9});
  cout << "ACTUAL by default:"s << endl;
  for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
    PrintDocument(document);
  }
  cout << "ACTUAL:"s << endl;
  for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s,
                                                                 [](int,
                                                                    DocumentStatus status,
                                                                    int) {
                                                                   return status == DocumentStatus::ACTUAL;
                                                                 })) {
    PrintDocument(document);
  }
  cout << "Even ids:"s << endl;
  for (const Document &document : search_server.FindTopDocuments("пушистый ухоженный кот"s,
                                                                 [](int document_id,
                                                                    DocumentStatus,
                                                                    int) { return document_id % 2 == 0; })) {
    PrintDocument(document);
  }
  return 0;
}
