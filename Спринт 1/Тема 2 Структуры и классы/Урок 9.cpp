#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string
ReadLine()
{
  string s;
  getline(cin, s);
  return s;
}

int
ReadLineWithNumber()
{
  int result = 0;
  cin >> result;
  ReadLine();
  return result;
}

struct DocumentContent {
  int id = 0;
  vector<string> words;
};

struct Document {
  int id;
  int relevance;
};

bool
HasDocumentGreaterRelevance(const Document &lhs, const Document &rhs)
{
  return lhs.relevance > rhs.relevance;
}

class SearchServer {
public:
  void
  SetStopWords(const string &text)
  {
    vector<string> words = SplitIntoWords(text);
    for (const auto &word : words) {
      stop_words_.insert(word);
    }
  }

  void
  AddDocument(int document_id, const string &document)
  {
    documents_.push_back(
      { document_id, SplitIntoWordsNoStop(document, stop_words_) });
  }

  vector<Document>
  FindTopDocuments(const string &raw_query)
  {
    const set<string> query_words = ParseQuery(raw_query);
    auto matched_documents = FindAllDocuments(query_words);

    sort(matched_documents.begin(), matched_documents.end(),
      HasDocumentGreaterRelevance);
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
      matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
  }

private:
  static int
  MatchDocument(const DocumentContent &content, const set<string> &query_words)
  {
    if (query_words.empty()) {
      return 0;
    }
    set<string> matched_words;
    for (const string &word : content.words) {
      if (matched_words.count(word) != 0) {
        continue;
      }
      if (query_words.count(word) != 0) {
        matched_words.insert(word);
      }
    }
    return static_cast<int>(matched_words.size());
  }

  set<string>
  ParseQuery(const string &text)
  {
    set<string> query_words;
    for (const string &word : SplitIntoWordsNoStop(text, stop_words_)) {
      query_words.insert(word);
    }
    return query_words;
  }

  static vector<string>
  SplitIntoWords(const string &text)
  {
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

  static vector<string>
  SplitIntoWordsNoStop(const string &text, const set<string> &stop_words)
  {
    vector<string> words;
    for (const string &word : SplitIntoWords(text)) {
      if (stop_words.count(word) == 0) {
        words.push_back(word);
      }
    }
    return words;
  }

  vector<Document>
  FindAllDocuments(const set<string> &query_words)
  {
    vector<Document> matched_documents;
    for (const auto &document : documents_) {
      const int relevance = MatchDocument(document, query_words);
      if (relevance > 0) {
        matched_documents.push_back({ document.id, relevance });
      }
    }
    return matched_documents;
  }

  vector<DocumentContent> documents_;
  set<string> stop_words_;
};

SearchServer CreateSearchServer() {
  SearchServer server;
  server.SetStopWords(ReadLine());
  int iterations = ReadLineWithNumber();
  for (int i = 0; i < iterations; i++) {
    server.AddDocument(i, ReadLine());
  }
  return server;
}

int
main()
{
  SearchServer search_server = CreateSearchServer();
  for(const auto& i : search_server.FindTopDocuments(ReadLine())){
    cout << "{ document_id = " << i.id << ", relevance = " << i.relevance
      << " }" << endl;
  }
}
