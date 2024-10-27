#include <cassert>
#include <iostream>
#include <memory>
#include <string>

struct Cat {
  Cat(const std::string& name, int age) : name_(name), age_(age) {}
  const std::string& GetName() const noexcept {
    return name_;
  }
  int GetAge() const noexcept {
    return age_;
  }
  void Speak() const {
    std::cout << "Meow!" << std::endl;
  }

 private:
  std::string name_;
  int age_;
};

// Функция создаёт двухлетних котов
std::unique_ptr<Cat> CreateCat(const std::string& name) {
  return std::make_unique<Cat>(name, 2);
}

class Witch {
 public:
  explicit Witch(const std::string& name) : name_(name) {}

  Witch(const Witch &other) : name_(other.name_), cat_(std::make_unique<Cat>(*other.cat_)) {}

  Witch& operator=(const Witch& other) {
    if (this != &other) {
      auto witch_copy(other);
      std::swap(name_, witch_copy.name_);
      std::swap(cat_, witch_copy.cat_);
    }

    return *this;
  }

  // При явном объявлении конструктора копирования и копирующего оператора
  // присваивания компилятор не будет генерировать перемещающий конструктор и
  // перемещающий оператор присваивания.
  // Поэтому явно просим компилятор сгенерировать их
  Witch(Witch&&) = default;
  Witch &operator=(Witch&&) = default;

  const std::string& GetName() const noexcept {
    return name_;
  }
  void SetCat(std::unique_ptr<Cat>&& cat) noexcept {
    cat_ = std::move(cat);
  }
  std::unique_ptr<Cat> ReleaseCat() noexcept {
    return std::move(cat_);
  }

 private:
  std::string name_;
  std::unique_ptr<Cat> cat_;
};

void Test() {
// Объекты Witch можно перемещать
  {
    Witch witch("Hermione");
    auto cat = CreateCat("Crookshanks");
    Cat *raw_cat = cat.get();
    assert(raw_cat);
    witch.SetCat(std::move(cat));

    Witch moved_witch(std::move(witch));
    auto released_cat = moved_witch.ReleaseCat();
    assert(released_cat.get() == raw_cat);  // Кот переместился от witch к moved_witch
  }

// Можно использовать перемещающий оператор присваивания
  {
    Witch witch("Hermione");
    auto cat = CreateCat("Crookshanks");
    Cat *raw_cat = cat.get();
    witch.SetCat(std::move(cat));

    Witch witch2("Minerva McGonagall");
    witch2 = std::move(witch);
    auto released_cat = witch.ReleaseCat();
    assert(!released_cat);
    released_cat = witch2.ReleaseCat();
    assert(released_cat.get() == raw_cat);
  }

// Можно копировать волшебниц
  {
    Witch witch("Hermione");
    auto cat = CreateCat("Crookshanks");
    witch.SetCat(std::move(cat));

    Witch witch_copy(witch);
    assert(!cat);
    cat = witch.ReleaseCat();
    assert(cat);  // У первой волшебницы кот никуда не делся

    auto cat_copy = witch_copy.ReleaseCat();
    assert(cat_copy != nullptr && cat_copy != cat);
    assert(cat_copy->GetName() == cat->GetName());  // Копия волшебницы содержит копию кота
  }

// Работает копирующее присваивание волшебниц
  {
    Witch witch("Hermione");
    auto cat = CreateCat("Crookshanks");
    witch.SetCat(std::move(cat));

    Witch witch2("Minerva McGonagall");
    witch2 = witch;

    assert(!cat);
    cat = witch.ReleaseCat();
    assert(cat);  // У первой волшебницы кот никуда не делся

    auto cat_copy = witch2.ReleaseCat();
    assert(cat_copy != nullptr && cat_copy != cat);
    assert(cat_copy->GetName() == cat->GetName());  // При присваивании скопировался кот
  }
}

int main() {
  Test();
}