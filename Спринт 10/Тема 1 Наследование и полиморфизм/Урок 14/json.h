#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using NodeData = std::variant<std::nullptr_t, bool, int, double, std::string, Array, Dict>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
 public:
  using runtime_error::runtime_error;
};

class Node {
 public:
  Node() = default;
  Node(std::nullptr_t);
  Node(Array array);
  Node(Dict map);
  Node(bool boolean);
  Node(int integer);
  Node(double dbl);
  Node(std::string str);

  bool IsNull() const;
  bool IsArray() const;
  bool IsMap() const;
  bool IsBool() const;
  bool IsInt() const;
  bool IsDouble() const;
  bool IsPureDouble() const;
  bool IsString() const;

  const Array& AsArray() const;
  const Dict& AsMap() const;
  bool AsBool() const;
  int AsInt() const;
  double AsDouble() const;
  const std::string& AsString() const;

  const NodeData& GetNodeType() const;

  bool operator==(const Node& rhs) const;
  bool operator!=(const Node& rhs) const;

 private:
  NodeData data_;
};

class Document {
 public:
  explicit Document(Node root);

  const Node& GetRoot() const;

  bool operator==(const Document& rhs) const;
  bool operator!=(const Document& rhs) const;

 private:
  Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
