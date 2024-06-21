#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
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
};

class SearchServer {
public:
  void SetStopWords(const string &text) {
    for (const string &word : SplitIntoWords(text)) {
      stop_words_.insert(word);
    }
  }

  void AddDocument(int document_id, const string &document) {
    const vector<string> words = SplitIntoWordsNoStop(document);
    double word_tf = 1. / words.size();
    for (const string &word : words) {
      word_to_document_freqs_[word][document_id] += word_tf;
    }
    document_count_++;
  }

  vector<Document> FindTopDocuments(const string &raw_query) const {
    const Query query_words = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query_words);
    sort(matched_documents.begin(),
      matched_documents.end(),
      [](const Document &lhs, const Document &rhs) {
        return lhs.relevance > rhs.relevance;
      });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
      matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
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

  map<string, map<int, double>> word_to_document_freqs_;
  set<string> stop_words_;
  int document_count_ = 0;

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

  static inline double CalculateIdf(double documents_count,
    double count_documents_contain_word) {
    return log(documents_count / count_documents_contain_word);
  }

  vector<Document> FindAllDocuments(const Query &query_words) const {
    vector<Document> matched_documents;
    map<int, double> id_relevance;

    for (const auto &word : query_words.plus_words) {
      if (word_to_document_freqs_.find(word) != word_to_document_freqs_.end()) {
        double idf = CalculateIdf(document_count_,
          word_to_document_freqs_.at(word).size());
        for (const auto &[id, tf] : word_to_document_freqs_.at(word)) {
          id_relevance[id] += tf * idf;
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
      matched_documents.push_back({id, rel});
    }

    return matched_documents;
  }
};

SearchServer CreateSearchServer() {
  SearchServer search_server;
  search_server.SetStopWords(ReadLine());

  const int document_count = ReadLineWithNumber();
  for (int document_id = 0; document_id < document_count;
    ++document_id) { search_server.AddDocument(document_id, ReadLine()); }
  return search_server;
}

int main() {
  const SearchServer search_server = CreateSearchServer();

  const string query = ReadLine();
  for (const auto &[document_id, relevance]
    : search_server.FindTopDocuments(query)) {
    cout << "{ document_id = "s << document_id << ", " << "relevance = "s
      << relevance << " }"s << endl;
  }
}
