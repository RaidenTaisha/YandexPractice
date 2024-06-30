#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

using namespace std;

/* Подставьте вашу реализацию класса SearchServer сюда */
namespace {
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
}

/*
   Подставьте сюда вашу реализацию макросов
   ASSERT, ASSERT_EQUAL, ASSERT_EQUAL_HINT, ASSERT_HINT и RUN_TEST
*/
namespace {
void AssertImpl(bool value, const string &expr_str, const string &file, const string &func, unsigned line,
                const string &hint) {
  if (!value) {
    cerr << file << "("s << line << "): "s << func << ": "s;
    cerr << "Assert("s << expr_str << ") failed."s;
    if (!hint.empty()) {
      cerr << " Hint: "s << hint;
    }
    cerr << endl;
    abort();
  }
}

template<typename T, typename U>
void AssertEqualImpl(const T &t, const U &u, const string &t_str, const string &u_str, const string &file,
                     const string &func, unsigned line, const string &hint) {
  if (t != u) {
    cerr << boolalpha;
    cerr << file << "("s << line << "): "s << func << ": "s;
    cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
    cerr << t << " != "s << u << "."s;
    if (!hint.empty()) {
      cerr << " Hint: "s << hint;
    }
    cerr << endl;
    abort();
  }
}

template <typename TestFunc>
void RunTestImpl(const TestFunc& func, const string& test_name) {
  func();
  cerr << test_name << " OK"s << endl;
}
}

#define RUN_TEST(func) RunTestImpl(func, #func)
#define ASSERT(a) AssertImpl((a), #a, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_HINT(a, hint) AssertImpl((a), #a, __FILE__, __FUNCTION__, __LINE__, (hint))
#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
  const int doc_id = 42;
  const string content = "cat in the city"s;
  const vector<int> ratings = {1, 2, 3};
  {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    const vector<Document> found_docs = server.FindTopDocuments("in"s);
    ASSERT_EQUAL(found_docs.size(), 1u);
    const Document& doc0 = found_docs[0];
    ASSERT_EQUAL(doc0.id, doc_id);
  }

  {
    SearchServer server;
    server.SetStopWords("in the"s);
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
                "Stop words must be excluded from documents"s);
  }
}

// Проверка поддержки минус-слов. Документы, содержащие минус-слова поискового
// запроса, не должны включаться в результаты поиска
void TestExcludeMinusWordsFromAddedDocumentContent() {
  const int doc_id = 42;
  const string content = "cat in the city"s;
  const vector<int> ratings = {1, 2, 3};
  {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    const vector<Document> found_docs = server.FindTopDocuments("in"s);
    ASSERT_EQUAL(found_docs.size(), 1u);
    const Document& doc0 = found_docs[0];
    ASSERT_EQUAL(doc0.id, doc_id);
  }

  {
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    ASSERT_HINT(server.FindTopDocuments("city -cat"s).empty(),
                "Documents containing minus words should not be included in the search"s);
  }
}

// Сортировка найденных документов по релевантности. Возвращаемые при поиске
// документов результаты должны быть отсортированы в порядке убывания
// релевантности
void TestSortByRelevance() {
  const vector<int> ratings = {1, 2, 3};
  {
    SearchServer server;
    server.AddDocument(1, "cat in the city", DocumentStatus::ACTUAL, ratings);
    server.AddDocument(2, "dog in the city", DocumentStatus::ACTUAL, ratings);
    server.AddDocument(3, "the parrot", DocumentStatus::ACTUAL, ratings);
    const vector<Document> found_docs = server.FindTopDocuments("dog in the city"s);

    ASSERT_EQUAL_HINT(found_docs[0].id, 2, "Documents are not sorted by relevance"s);
    ASSERT_EQUAL_HINT(found_docs[1].id, 1, "Documents are not sorted by relevance"s);
    ASSERT_EQUAL_HINT(found_docs[2].id, 3, "Documents are not sorted by relevance"s);
  }
}

// Тест  проверяет, что при вызове TestMatchingDocuments возвращаются
// все слова из поискового запроса, присутствующие в документе.
// Если есть соответствие хотя бы по одному минус-слову, должен возвращаться
// пустой список слов.
void TestMatchingDocuments() {
  SearchServer server;

  server.AddDocument(0, "white cat and pink collar"s,
                     DocumentStatus::ACTUAL, {8, -3});
  server.AddDocument(1, "furry cat furry tail"s,
                     DocumentStatus::ACTUAL, {7, 2, 7});
  server.AddDocument(2, "groomed dog expressive eyes"s,
                     DocumentStatus::ACTUAL, {5, -12, 2, 1});

  // Должны быть возвращены все слова из поискового запроса, присутствующие в
  // документе
  {
    const auto [words, status] = server.MatchDocument("furry cat"s, 1);
    vector<string> query_w = {"furry"s, "cat"s};
    ASSERT_EQUAL(words.size(), query_w.size());

    // Вектор слов результата должен содержать только те же слова, и в том
    // же количестве, что и вектор слов запроса
    for (const auto &word : words)
      ASSERT(count(query_w.begin(), query_w.end(), word) == 1);
  }

  // Если есть соответствие хотя бы по одному минус-слову, должен возвращаться
  // пустой список слов
  {
    const auto [words, status] = server.MatchDocument("furry -cat"s, 1);
    ASSERT_HINT(words.empty(),
                "Minus-word was detected, but result doesn't empty"s);
  }
}

// Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему
// арифметическому оценок документа.
void TestCalcRating() {
  const vector<vector<int>> ratings = {{1, 2, 3},
                                       {1, 2, 3, 6},
                                       {1, 5, 5, 4, 5}};

  const vector<double> average_ratings = {2, 3, 4};
  {
    SearchServer server;
    server.AddDocument(1, "cat in the city"s, DocumentStatus::ACTUAL, ratings[0]);
    server.AddDocument(2, "dog in the city", DocumentStatus::ACTUAL, ratings[1]);
    server.AddDocument(3, "the parrot", DocumentStatus::ACTUAL, ratings[2]);

    const vector<Document> found_docs = server.FindTopDocuments("cat in the"s);

    ASSERT_EQUAL_HINT(found_docs[0].rating, average_ratings[0], "Documents are not sorted by rating");
    ASSERT_EQUAL_HINT(found_docs[1].rating, average_ratings[1], "Documents are not sorted by rating");
    ASSERT_EQUAL_HINT(found_docs[2].rating, average_ratings[2], "Documents are not sorted by rating");
  }
}

// Фильтрация результатов поиска с использованием предиката, задаваемого
// пользователем
void TestFilterPredicate() {
  const int id0 = 0, id1 = 1, id2 = 2;
  const vector<vector<int>> ratings = {{1, 2, 3},
                                       {1, 2, 3, 6},
                                       {1, 5, 5, 4, 5}};

  const vector<double> average_ratings = {2, 3, 4};
  {
    SearchServer server;
    server.AddDocument(id0, "cat in the city"s, DocumentStatus::ACTUAL, ratings[0]);
    server.AddDocument(id1, "dog in the city", DocumentStatus::ACTUAL, ratings[1]);
    server.AddDocument(id2, "the parrot", DocumentStatus::ACTUAL, ratings[2]);

    const vector<Document> found_docs = server.FindTopDocuments("cat in the"s,
                                                    [](int document_id, DocumentStatus status, int rating) {
                                                      return document_id % 2 == 0;
                                                    });

    ASSERT_EQUAL_HINT(found_docs[0].rating, average_ratings[0], "Document Predicate Is Ignored ");
    ASSERT_EQUAL_HINT(found_docs[1].rating, average_ratings[2], "Document Predicate Is Ignored ");
  }
}

// Поиск документов, имеющих заданный статус
void TestStatus() {
  {
    SearchServer server;

    server.AddDocument(0, "white cat and pink collar"s,
                       DocumentStatus::ACTUAL, {8, -3});
    server.AddDocument(1, "furry cat furry tail"s,
                       DocumentStatus::BANNED, {7, 2, 7});
    server.AddDocument(2, "groomed dog expressive eyes"s,
                       DocumentStatus::ACTUAL, {5, -12, 2, 1});

    auto found_docs = server.FindTopDocuments("cat"s,
                                              DocumentStatus::BANNED);
    ASSERT(found_docs.size() == 1u);
    ASSERT(found_docs[0].id == 1);
  }

  {
    const int id0 = 0, id1 = 1, id2 = 2, id3 = 3;
    const vector<vector<int>> ratings = {{1, 2, 3}, {1, 2, 3, 6}, {1, 5, 5, 4, 5}}; // 2 3 4
    {
      SearchServer server;
      server.AddDocument(id0, "cat in the city"s, DocumentStatus::ACTUAL, ratings[0]);
      server.AddDocument(id1, "dog in the city", DocumentStatus::BANNED, ratings[1]);
      server.AddDocument(id2, "the parrot", DocumentStatus::IRRELEVANT, ratings[2]);
      server.AddDocument(id3, "the parrot", DocumentStatus::REMOVED, ratings[2]);

      const auto found_docs = server.FindTopDocuments("cat in the"s,
                                                      [](int document_id, DocumentStatus status, int rating) {
                                                        return status == DocumentStatus::ACTUAL;
                                                      });
      const auto found_docs1 = server.FindTopDocuments("cat in the"s,
                                                       [](int document_id, DocumentStatus status, int rating) {
                                                         return status == DocumentStatus::BANNED;
                                                       });
      const auto found_docs2 = server.FindTopDocuments("cat in the"s,
                                                       [](int document_id, DocumentStatus status, int rating) {
                                                         return status == DocumentStatus::IRRELEVANT;
                                                       });
      const auto found_docs3 = server.FindTopDocuments("cat in the"s,
                                                       [](int document_id, DocumentStatus status, int rating) {
                                                         return status == DocumentStatus::REMOVED;
                                                       });

      bool test_bool;
      DocumentStatus temp = get<DocumentStatus>(server.MatchDocument("in the city"s, 1));
      if (temp == DocumentStatus::BANNED) { test_bool = true; }
      ASSERT_EQUAL_HINT(test_bool, true, "Document Status Is Ignore"s);
    }
  }
}

// Вспомогательная функция для теста TestCalcRelevance()
vector<string> SplitIntoWordsNoStop(const string& text,
                                    const vector<string> &stop_words) {
  vector<string> words;
  for (const string& word : SplitIntoWords(text)) {
    if (count(stop_words.begin(), stop_words.end(), word) == 0) {
      words.push_back(word);
    }
  }
  return words;
}

// Корректное вычисление релевантности найденных документов
void TestCalcRelevance() {
  string stop_words = "and in a"s;
  SearchServer server;
  server.SetStopWords(stop_words);

  vector<string> stop_words_v = SplitIntoWords(stop_words);
  string doc1 = "white cat and pink collar"s;
  string doc2 = "furry cat furry tail"s;
  string doc3 = "groomed dog expressive eyes"s;

  server.AddDocument(0, doc1, DocumentStatus::ACTUAL, {8, -3});
  server.AddDocument(1, doc2, DocumentStatus::ACTUAL, {7, 2, 7});
  server.AddDocument(2, doc3, DocumentStatus::ACTUAL, {5, -12, 2, 1});

  string query_word = "cat"s;
  vector<string> doc1_v = SplitIntoWordsNoStop(doc1, stop_words_v);
  vector<string> doc2_v = SplitIntoWordsNoStop(doc2, stop_words_v);

  vector<Document> found_docs = server.FindTopDocuments(query_word);

  double num_of_all_documents = server.GetDocumentCount();
  double num_of_found_documents = static_cast<double>(found_docs.size());

  const auto CalculateIdf = [num_of_all_documents, num_of_found_documents]() {
    return log(num_of_all_documents / num_of_found_documents);
  };

  double idf = CalculateIdf();
  double tf1 = static_cast<double>(count(doc1_v.begin(), doc1_v.end(), query_word))
      / static_cast<double>(doc1_v.size());
  double tf2 = static_cast<double>(count(doc2_v.begin(), doc2_v.end(), query_word))
      / static_cast<double>(doc2_v.size());

  double tfidf1 = idf * tf1;
  double tfidf2 = idf * tf2;

  for (const auto &doc : found_docs) {
    if (doc.id == 0)
      ASSERT_HINT(abs(doc.relevance - tfidf1) < numeric_limits<double>::epsilon(),
                  "Incorrect document relevance"s);
    else if (doc.id == 1)
      ASSERT_HINT(abs(doc.relevance - tfidf2) < numeric_limits<double>::epsilon(),
                  "Incorrect document relevance"s);
  }
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
  RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
  RUN_TEST(TestExcludeMinusWordsFromAddedDocumentContent);
  RUN_TEST(TestMatchingDocuments);
  RUN_TEST(TestSortByRelevance);
  RUN_TEST(TestCalcRating);
  RUN_TEST(TestFilterPredicate);
  RUN_TEST(TestStatus);
  RUN_TEST(TestCalcRelevance);
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
  TestSearchServer();
  // Если вы видите эту строку, значит все тесты прошли успешно
  cout << "Search server testing finished"s << endl;
}
