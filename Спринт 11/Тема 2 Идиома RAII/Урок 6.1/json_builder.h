#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {

class Builder {
 public:
  Builder();
  Node Build();
  Builder &Key(std::string key);
  Builder &Value(Node::Value value);
  Builder &StartDict();
  Builder &StartArray();
  Builder &EndDict();
  Builder &EndArray();

 private:
  Node root_;
  std::vector<Node *> nodes_stack_;

  Node::Value &GetCurrentValue();
  const Node::Value &GetCurrentValue() const;

  void AssertNewObjectContext() const;
  void AddObject(Node::Value value, bool one_shot);
};

}  // namespace json
