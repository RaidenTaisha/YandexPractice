#include <string>
#include <iostream>

using namespace std;

class Editor {
 public:
  Editor() = default;
  // сдвинуть курсор влево
  void Left() {
    if (line.empty() || pos == 0)
      return;
    pos--;
  }
  // сдвинуть курсор вправо
  void Right(){
    if (line.empty() || pos == line.size())
      return;
    pos++;
  }
  // вставить символ token
  void Insert(char token) {
    if (line.empty()) {
      line += token;
      pos++;
      return;
    }
    line = line.substr(0, pos) + token + line.substr(pos);
    pos++;
  }
  // вырезать не более tokens символов, начиная с текущей позиции курсора
  void Cut(size_t tokens = 1) {
    if (line.empty())
      return;
    buf = line.substr(pos, pos + tokens);
    line.erase(pos, pos + tokens);
  }
  // cкопировать не более tokens символов, начиная с текущей позиции курсора
  void Copy(size_t tokens = 1) {
    if (line.empty())
      return;
    buf = line.substr(pos, pos + tokens);
  }
  // вставить содержимое буфера в текущую позицию курсора
  void Paste() {
    if (buf.empty())
      return;
    if (line.empty()) {
      line += buf;
      return;
    }
    line = line.substr(0, pos) + buf + line.substr(pos);
    pos += buf.size();
  }
  // получить текущее содержимое текстового редактора
  string GetText() const {
    return line;
  }
 private:
  string line;
  string buf;
  size_t pos = 0;
};

int main() {
  Editor editor;
  const string text = "hello, world"s;
  for (char c : text) {
    editor.Insert(c);
  }
  // Текущее состояние редактора: `hello, world|`
  for (size_t i = 0; i < text.size(); ++i) {
    editor.Left();
  }
  // Текущее состояние редактора: `|hello, world`
  editor.Cut(7);
  // Текущее состояние редактора: `|world`
  // в буфере обмена находится текст `hello, `
  for (size_t i = 0; i < 5; ++i) {
    editor.Right();
  }
  // Текущее состояние редактора: `world|`
  editor.Insert(',');
  editor.Insert(' ');
  // Текущее состояние редактора: `world, |`
  editor.Paste();
  // Текущее состояние редактора: `world, hello, |`
  editor.Left();
  editor.Left();
  //Текущее состояние редактора: `world, hello|, `
  editor.Cut(3);  // Будут вырезаны 2 символа
  // Текущее состояние редактора: `world, hello|`
  cout << editor.GetText();
  return 0;
}
