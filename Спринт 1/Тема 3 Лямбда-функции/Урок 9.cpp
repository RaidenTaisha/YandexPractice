#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <map>
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
  int id;
  int relevance;
};

class SearchServer {
public:
  void
  SetStopWords(const string &text)
  {
    for (const string &word : SplitIntoWords(text)) {
      stop_words_.insert(word);
    }
  }

  void
  AddDocument(int document_id, const string &document)
  {
    const vector<string> words = SplitIntoWordsNoStop(document);
    for (const auto &word : words) {
      documents_[word].insert(document_id);
    }
  }

  vector<Document>
  FindTopDocuments(const string &raw_query) const
  {
    const set<string> query_words = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query_words);

    sort(matched_documents.begin(), matched_documents.end(),
      [](const Document &lhs, const Document &rhs) {
        return lhs.relevance > rhs.relevance;
      });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
      matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
  }

private:
  map<string, set<int>> documents_;

  set<string> stop_words_;

  bool
  IsStopWord(const string &word) const
  {
    return stop_words_.count(word) > 0;
  }

  vector<string>
  SplitIntoWordsNoStop(const string &text) const
  {
    vector<string> words;
    for (const string &word : SplitIntoWords(text)) {
      if (!IsStopWord(word)) {
        words.push_back(word);
      }
    }
    return words;
  }

  set<string>
  ParseQuery(const string &text) const
  {
    set<string> query_words;
    for (const string &word : SplitIntoWordsNoStop(text)) {
      query_words.insert(word);
    }
    return query_words;
  }

  vector<Document>
  FindAllDocuments(const set<string> &query_words) const
  {
    vector<Document> matched_documents;
    map<int, int> id_relevance;
    for (const auto &document : documents_) {
      if (query_words.count(document.first) == 1) {
        for (const auto &doc : document.second) {
          id_relevance[doc]++;
        }
      }
    }
    for (const auto &matched_document : id_relevance) {
      matched_documents.push_back({ matched_document.first,
        matched_document.second });
    }
    return matched_documents;
  }
};

SearchServer CreateSearchServer() {
  SearchServer search_server;
  search_server.SetStopWords(ReadLine());

  const int document_count = ReadLineWithNumber();
  for (int document_id = 0; document_id < document_count; ++document_id) {
    search_server.AddDocument(document_id, ReadLine());
  }

  return search_server;
}

int main() {
  const SearchServer search_server = CreateSearchServer();

  const string query = ReadLine();
  for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
    cout << "{ document_id = "s << document_id << ", "
      << "relevance = "s << relevance << " }"s << endl;
  }
}
