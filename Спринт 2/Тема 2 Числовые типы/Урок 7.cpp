#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

const size_t MAX_RESULT_DOCUMENT_COUNT = 5;

enum DocumentStatus {
  ACTUAL,
  IRRELEVANT,
  BANNED,
  REMOVED
};

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

vector<string> SplitIntoWords(const string &text) {
  vector<string> words;
  string word;
  for (const char c : text) {
    if (c == ' ') {
      words.push_back(word);
      word = "";
    } else {
      word += c;
    }
  }
  words.push_back(word);

  return words;
}

struct Document {
  int id;
  double relevance;
  int rating;
  DocumentStatus status;
};

class SearchServer {
 public:
  void SetStopWords(const string &text) {
    for (const string &word : SplitIntoWords(text)) {
      stop_words_.insert(word);
    }
  }

  void AddDocument(int document_id,
                   const string &document,
                   const DocumentStatus status,
                   const vector<int> &ratings) {
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / static_cast<double>(words.size());
    for (const string &word : words) {
      word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    document_id_to_rating_.emplace(document_id, ComputeAverageRating(ratings));
    document_id_to_status_.emplace(document_id, status);
  }

  vector<Document> FindTopDocuments(
      const string &raw_query,
      const DocumentStatus status = DocumentStatus::ACTUAL
  ) const {
    const Query query = ParseQuery(raw_query);
    vector<Document> matched_documents;
    auto all_the_documents = FindAllDocuments(query);

    sort(all_the_documents.begin(), all_the_documents.end(),
         [](const Document &lhs, const Document &rhs) {
           return lhs.relevance > rhs.relevance;
         });
    for (const auto &document : all_the_documents) {
      if (document.status == status) {
        matched_documents.push_back(document);
      }
    }
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
      matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
  }

 private:
  set<string> stop_words_;
  map<string, map<int, double>> word_to_document_freqs_;
  map<int, int> document_id_to_rating_;
  map<int, DocumentStatus> document_id_to_status_;

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
    int rating_sum = 0;
    for (const int rating : ratings) {
      rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
  }

  struct QueryWord {
    string data;
    bool is_minus;
    bool is_stop;
  };

  QueryWord ParseQueryWord(string text) const {
    bool is_minus = false;
    // Word shouldn't be empty
    if (text[0] == '-') {
      is_minus = true;
      text = text.substr(1);
    }
    return {
        text,
        is_minus,
        IsStopWord(text)
    };
  }

  struct Query {
    set<string> plus_words;
    set<string> minus_words;
  };

  Query ParseQuery(const string &text) const {
    Query query;
    for (const string &word : SplitIntoWords(text)) {
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

  // Existence required
  double ComputeWordInverseDocumentFreq(const string &word) const {
    return log(static_cast<double>(document_id_to_rating_.size()) * 1.0
                   / static_cast<double>(word_to_document_freqs_.at(word)
                       .size()));
  }

  vector<Document> FindAllDocuments(const Query &query) const {
    map<int, double> document_to_relevance;
    for (const string &word : query.plus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
      for (const auto &[document_id, term_freq] : word_to_document_freqs_
          .at(word)) {
        document_to_relevance[document_id] += term_freq * inverse_document_freq;
      }
    }

    for (const string &word : query.minus_words) {
      if (word_to_document_freqs_.count(word) == 0) {
        continue;
      }
      for (const auto &[document_id, _] : word_to_document_freqs_.at(word)) {
        document_to_relevance.erase(document_id);
      }
    }

    vector<Document> matched_documents;
    for (const auto &[document_id, relevance] : document_to_relevance) {
      matched_documents.push_back({
                                      document_id,
                                      relevance,
                                      document_id_to_rating_.at(document_id),
                                      document_id_to_status_.at(document_id)
                                  });
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
  search_server.AddDocument(0,
                            "белый кот и модный ошейник"s,
                            DocumentStatus::ACTUAL,
                            {8, -3});
  search_server.AddDocument(1,
                            "пушистый кот пушистый хвост"s,
                            DocumentStatus::ACTUAL,
                            {7, 2, 7});
  search_server.AddDocument(2,
                            "ухоженный пёс выразительные глаза"s,
                            DocumentStatus::ACTUAL,
                            {5, -12, 2, 1});
  search_server.AddDocument(3,
                            "ухоженный скворец евгений"s,
                            DocumentStatus::BANNED,
                            {9});
  cout << "ACTUAL:"s << endl;
  for (const Document &document
      : search_server.FindTopDocuments("пушистый ухоженный кот"s,
                                       DocumentStatus::ACTUAL)) {
    PrintDocument(document);
  }
  cout << "BANNED:"s << endl;
  for (const Document &document
      : search_server.FindTopDocuments("пушистый ухоженный кот"s,
                                       DocumentStatus::BANNED)) {
    PrintDocument(document);
  }
  return 0;
}
