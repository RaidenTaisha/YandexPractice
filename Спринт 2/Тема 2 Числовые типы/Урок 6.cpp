#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

struct Document {
  int id;
  int rating;
};

void PrintDocuments(vector<Document> documents,
                    size_t skip_start,
                    size_t skip_finish) {
  if (!documents.empty()) {
    sort(documents.begin(),
         documents.end(),
         [](const Document &lhs, const Document &rhs) {
           return lhs.rating > rhs.rating;
         });
    if (documents.size() > skip_start + skip_finish) {
      for (size_t i = skip_start; i < documents.size() - skip_finish; i++) {
        cout << "{ id = " << documents[i].id << ", rating = "
             << documents[i].rating << " }" << endl;
      }
    }
  }
}

int main() {
  PrintDocuments({{100, 5}, {101, 7}, {102, -4}, {103, 9}, {104, 1}}, 1, 2);
}
