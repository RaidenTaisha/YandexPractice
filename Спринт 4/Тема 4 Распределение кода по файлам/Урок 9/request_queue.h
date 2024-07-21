#pragma once

#include "document.h"
#include "search_server.h"

#include <deque>
#include <string>
#include <vector>

class RequestQueue {
 public:
  explicit RequestQueue(const SearchServer &search_server);
  template<typename DocumentPredicate>
  std::vector<Document> AddFindRequest(const std::string &raw_query,
                                                     DocumentPredicate document_predicate);
  std::vector<Document> AddFindRequest(const std::string &raw_query, DocumentStatus status);
  std::vector<Document> AddFindRequest(const std::string &raw_query);
  [[nodiscard]] int GetNoResultRequests() const;

 private:
  struct QueryResult {
    std::vector<Document> documents;
    bool is_empty;
  };

  const SearchServer *search_server_;
  std::deque<QueryResult> requests_;
  const static int min_in_day_ = 1440;
  int no_results_requests_;
};

template<typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string &raw_query, DocumentPredicate document_predicate) {
  // Добавление нового запроса в дек
  QueryResult results;
  results.documents = search_server_->FindTopDocuments(raw_query, document_predicate);
  results.is_empty = results.documents.empty();
  requests_.push_back(results);

  // Обновление счётчика пустых запросов в деке
  if (results.is_empty) {
    no_results_requests_++;
  }

  // Удаление из дека устаревших запросов (старше min_in_day_)
  while (requests_.size() > min_in_day_) {
    // Обновление счётчика пустых запросов в деке
    if (requests_.front().is_empty) {
      no_results_requests_--;
    }
    requests_.pop_front();
  }

  return results.documents;
}
