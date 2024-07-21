#include <algorithm>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

using namespace std;

template<typename It>
void PrintRange(It range_begin, It range_end) {
  for (auto it = range_begin; it != range_end; ++it) {
    cout << *it << " "s;
  }
  cout << endl;
}

template<typename Type>
class Stack {
 public:
  void Push(const Type &element) {
    elements_.push_back(element);
  }

  void Pop() {
    elements_.pop_back();
  }

  [[nodiscard]] const Type &Peek() const {
    return elements_.back();
  }

  [[nodiscard]] Type &Peek() {
    return elements_.back();
  }

  void Print() const {
    PrintRange(elements_.begin(), elements_.end());
  }

  [[nodiscard]] uint64_t Size() const {
    return elements_.size();
  }

  [[nodiscard]] bool IsEmpty() const {
    return elements_.empty();
  }

 private:
  vector<Type> elements_;
};

// используем струкруту из элемента
// и минимума в стеке до дна стека до текущего элемента включительно
template <typename Type>
struct PairWithMin {
  Type element;
  Type minimum;
};

template <typename Type>
ostream& operator<<(ostream& out, const PairWithMin<Type>& pair_to_print) {
  // будем выводить только сам элемент, выводить минимум не будем
  out << pair_to_print.element;
  return out;
}

template<typename Type>
class StackMin {
 public:
  void Push(const Type &new_element) {
    // создадим структуру, которую будем класть в стек
    PairWithMin<Type> new_pair = {new_element, new_element};
    // проверим, что должно стать минимумом для текущего элемента -
    // он сам, или предыдущий минимум
    if (!elements_.IsEmpty() && new_pair.minimum > elements_.Peek().minimum) {
      new_pair.minimum = elements_.Peek().minimum;
    }
    // положим структуру в стек
    elements_.Push(new_pair);
  }

  void Pop() {
    elements_.Pop();
  }

  const Type &Peek() const {
    return elements_.Peek().element;
  }

  Type &Peek() {
    return elements_.Peek().element;
  }

  void Print() const {
    elements_.Print();
  }

  uint64_t Size() const {
    return elements_.size();
  }

  bool IsEmpty() const {
    return elements_.IsEmpty();
  }

  const Type &PeekMin() const {
    return elements_.Peek().minimum;
  }

  Type &PeekMin() {
    return elements_.Peek().minimum;
  }

 private:
  // используем в качестве элемента стека нашу структуру
  Stack<PairWithMin<Type>> elements_;
};
template <typename Type>

class SortedSack {
 public:
  void Push(const Type& element) {
    // Если стек пуст, то просто вставляем в него новый элемент.
    if (elements_.IsEmpty()) {
      elements_.Push(element);
      return;
    }
    // Если в стеке уже есть элементы,
    // возьмем копию элемента сверху
    Type last_elem = elements_.Peek();
    // и сравним ее с элементом, который мы хотим вставить.
    if (last_elem < element) {
      // Если при добавлении элемента сортировка будет нарушаться,
      // вытащим из стека верхний элемент и попытаемся вставить новый элемент
      // в стек на одну позицию ниже рекурсивно.
      elements_.Pop();
      Push(element);
      // когда вставка удалась (рекурсивный стек начал раскручиваться назад),
      // нужно вернуть на место тот элемент, который мы из стека доставали.
      elements_.Push(last_elem);
    } else {
      // В этом случае сортировка при вставке не нарушается,
      // и мы можем смело вставлять элемент сверху стека
      elements_.Push(element);
    }
  }

  void Pop() {
    elements_.Pop();
  }

  const Type& Peek() const {
    return elements_.Peek();
  }

  Type& Peek() {
    return elements_.Peek();
  }

  void Print() const {
    elements_.Print();
  }

  uint64_t Size() const {
    return elements_.Size();
  }

  bool IsEmpty() const {
    return elements_.IsEmpty();
  }

 private:
  Stack<Type> elements_;
};

int main() {
  SortedSack<int> sack;
  vector<int> values(5);
  // заполняем вектор для тестирования нашего класса
  iota(values.begin(), values.end(), 1);
  // перемешиваем значения
  random_device rd;
  mt19937 g(rd());
  shuffle(values.begin(), values.end(), g);
  // заполняем класс и проверяем, что сортировка сохраняется после каждой вставки
  for (int i = 0; i < 5; ++i) {
    cout << "Вставляемый элемент = "s << values[i] << endl;
    sack.Push(values[i]);
    sack.Print();
  }
}
