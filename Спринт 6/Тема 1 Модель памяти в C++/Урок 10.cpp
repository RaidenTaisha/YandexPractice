#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <vector>

using namespace std;

// Используйте эту заготовку PtrVector или замените её на свою реализацию
template <typename T>
class PtrVector {
 public:
  PtrVector() = default;

  // Создаёт вектор указателей на копии объектов из other
  PtrVector(const PtrVector& other) {
    // Резервируем место в vector-е для хранения нужного количества элементов
    // Благодаря этому при push_back не будет выбрасываться исключение
    items_.reserve(other.items_.size());

    try {
      for (auto p : other.items_) {
        // Копируем объект, если указатель на него ненулевой
        auto p_copy = p ? new T(*p) : nullptr;  // new может выбросить исключение

        // Не выбросит исключение, т. к. в vector память уже зарезервирована
        items_.push_back(p_copy);
      }
    } catch (...) {
      // удаляем элементы в векторе и перевыбрасываем пойманное исключение
      DeleteItems();
      throw;
    }
  }

  // Деструктор удаляет объекты в куче, на которые ссылаются указатели,
  // в векторе items_
  ~PtrVector() {
    DeleteItems();
  }

  // Возвращает ссылку на вектор указателей
  vector<T*>& GetItems() noexcept {
    return items_;
  }

  // Возвращает константную ссылку на вектор указателей
  vector<T*> const& GetItems() const noexcept {
    return items_;
  }

  PtrVector& operator=(const PtrVector& rhs) {
    // При присваивании PtrVector самому себе произойдёт удаление не только
    // своих элементов, но и элементов rhs.
    if (this != &rhs) {
      auto rhs_copy(rhs);
      items_.swap(rhs_copy.items_);
    }

    return *this;
  }

 private:
  void DeleteItems() noexcept {
    for (auto p : items_) {
      delete p;
    }
  }

  vector<T*> items_;
};

template <typename T>
class ScopedPtr {
 public:
  ScopedPtr() = default;

  explicit ScopedPtr(T* raw_ptr) noexcept
      : ptr_(raw_ptr) {
  }

  // Запрещаем копирование указателя
  ScopedPtr(const ScopedPtr&) = delete;

  ~ScopedPtr() {
    delete ptr_;
  }

  T* GetRawPtr() const noexcept {
    return ptr_;
  }

  T* Release() noexcept {
    T* p = ptr_;
    ptr_ = nullptr;
    return p;
  }

  explicit operator bool() const {
    return ptr_ != nullptr;
  }

  T* operator->() const {
    using namespace std::literals;
    if (!ptr_) {
      throw std::logic_error("Scoped ptr is null"s);
    }
    return ptr_;
  }

  T& operator*() const {
    using namespace std::literals;
    if (!ptr_) {
      throw std::logic_error("Scoped ptr is null"s);
    }
    return *ptr_;
  }

 private:
  T* ptr_ = nullptr;
};

// Щупальце
class Tentacle {
 public:
  explicit Tentacle(int id) noexcept
      : id_(id) {
  }

  int GetId() const noexcept {
    return id_;
  }

  Tentacle* GetLinkedTentacle() const noexcept {
    return linked_tentacle_;
  }
  void LinkTo(Tentacle& tentacle) noexcept {
    linked_tentacle_ = &tentacle;
  }
  void Unlink() noexcept {
    linked_tentacle_ = nullptr;
  }

 private:
  int id_ = 0;
  Tentacle* linked_tentacle_ = nullptr;
};

// Осьминог
class Octopus {
 public:
  Octopus()
      : Octopus(8) {
  }

  explicit Octopus(int num_tentacles) {
    for (int i = 1; i <= num_tentacles; ++i) {
      AddTentacle();  // Может выбросить исключение
    }
  }

  // Конструктор копирования класса PtrVector корректно скопирует щупальца,
  // поэтому сгенерированный компилятором конструктор копирования нас вполне устроит

  Tentacle& AddTentacle() {
    ScopedPtr<Tentacle> tentacle(
        new Tentacle(GetTentacleCount() + 1));  // Может выбросить исключение
    tentacles_.GetItems().push_back(tentacle.GetRawPtr());  // Может выбросить исключение

    tentacle.Release();
    return *tentacles_.GetItems().back();
  }

  int GetTentacleCount() const noexcept {
    return static_cast<int>(tentacles_.GetItems().size());
  }

  const Tentacle& GetTentacle(size_t index) const {
    return *tentacles_.GetItems().at(index);
  }
  Tentacle& GetTentacle(size_t index) {
    return *tentacles_.GetItems().at(index);
  }

 private:
  PtrVector<Tentacle> tentacles_;
};

// Эта функция main тестирует шаблон класса PtrVector
int main() {
  struct CopyingSpy {
    CopyingSpy(int& copy_count, int& deletion_count)
        : copy_count_(copy_count)
        , deletion_count_(deletion_count) {
    }
    CopyingSpy(const CopyingSpy& rhs)
        : copy_count_(rhs.copy_count_)          // счётчик копирований
        , deletion_count_(rhs.deletion_count_)  // счётчик удалений
    {
      if (rhs.throw_on_copy_) {
        throw runtime_error("copy construction failed"s);
      }
      ++copy_count_;
    }
    ~CopyingSpy() {
      ++deletion_count_;
    }
    void ThrowOnCopy() {
      throw_on_copy_ = true;
    }

   private:
    int& copy_count_;
    int& deletion_count_;
    bool throw_on_copy_ = false;
  };

  // Проверка присваивания
  {
    int item0_copy_count = 0;
    int item0_deletion_count = 0;
    {
      PtrVector<CopyingSpy> v;

      v.GetItems().push_back(new CopyingSpy(item0_copy_count, item0_deletion_count));
      v.GetItems().push_back(nullptr);
      {
        PtrVector<CopyingSpy> v_copy;
        v_copy = v;
        assert(v_copy.GetItems().size() == v.GetItems().size());
        assert(v_copy.GetItems().at(0) != v.GetItems().at(0));
        assert(v_copy.GetItems().at(1) == nullptr);
        assert(item0_copy_count == 1);
        assert(item0_deletion_count == 0);
      }
      assert(item0_deletion_count == 1);
    }
    assert(item0_deletion_count == 2);
  }

  // Проверка корректности самоприсваивания
  {
    int item0_copy_count = 0;
    int item0_deletion_count = 0;

    PtrVector<CopyingSpy> v;
    v.GetItems().push_back(new CopyingSpy(item0_copy_count, item0_deletion_count));
    CopyingSpy* first_item = v.GetItems().front();

    v = v;
    assert(v.GetItems().size() == 1);
    // При самоприсваивании объекты должны быть расположены по тем же адресам
    assert(v.GetItems().front() == first_item);
    assert(item0_copy_count == 0);
    assert(item0_deletion_count == 0);
  }

  // Проверка обеспечения строгой гарантии безопасности исключений при присваивании
  {
    int item0_copy_count = 0;
    int item0_deletion_count = 0;

    int item1_copy_count = 0;
    int item1_deletion_count = 0;

    // v хранит 2 элемента
    PtrVector<CopyingSpy> v;
    v.GetItems().push_back(new CopyingSpy(item0_copy_count, item0_deletion_count));
    v.GetItems().push_back(new CopyingSpy(item1_copy_count, item1_deletion_count));

    int other_item0_copy_count = 0;
    int other_item0_deletion_count = 0;
    // other_vector хранит 1 элемент, при копировании которого будет выброшено исключение
    PtrVector<CopyingSpy> other_vector;
    other_vector.GetItems().push_back(new CopyingSpy(other_item0_copy_count, other_item0_deletion_count));
    other_vector.GetItems().front()->ThrowOnCopy();

    // Сохраняем массив указателей
    auto v_items(v.GetItems());

    try {
      v = other_vector;
      // Операция должна выбросить исключение
      assert(false);
    } catch (const runtime_error&) {
    }

    // Элементы массива должны остаться прежними
    assert(v.GetItems() == v_items);
    assert(item0_copy_count == 0);
    assert(item1_copy_count == 0);
    assert(other_item0_copy_count == 0);
  }
}
