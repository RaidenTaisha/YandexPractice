#include <cassert>
#include <iostream>
#include <memory>

template <typename T>
struct TreeNode;

template <typename T>
using TreeNodePtr = std::unique_ptr<TreeNode<T>>;

template <typename T>
struct TreeNode {
  TreeNode(T val, TreeNodePtr<T>&& left, TreeNodePtr<T>&& right)
      : value(std::move(val))
      , left(std::move(left))
      , right(std::move(right)) {
  }

  T value;
  TreeNodePtr<T> left;
  TreeNodePtr<T> right;
  TreeNode* parent = nullptr;
};

template <typename T>
bool CheckTreeProperty(const TreeNode<T>* node) noexcept {
  if (node->left == nullptr && node->right == nullptr) {
    return true;
  }

  bool l_res = true;
  bool r_res = true;

  // Проход по левому дереву
  if (node->left != nullptr) {
    if (node->value >= node->left->value) {
      l_res = CheckTreeProperty(node->left.get());
    } else {
      return false;
    }
  }

  // Проход по правому дереву
  if (node->right != nullptr) {
    if (node->value <= node->right->value) {
      r_res = CheckTreeProperty(node->right.get());
    } else {
      return false;
    }
  }

  return l_res && r_res;
}

template <typename T>
TreeNode<T>* begin(TreeNode<T>* node) noexcept {
  while (node->left != nullptr) {
    node = node->left.get();
  }

  return node;
}

template <typename T>
TreeNode<T>* next(TreeNode<T>* node) noexcept {
  if (node->right != nullptr) {
    return begin(node->right.get());
  }

  while (node->parent != nullptr) {
    if (node->parent->right.get() == node) {
      node = node->parent;
    } else {
      return node->parent;
    }
  }

  return nullptr;
}

// функция создаёт новый узел с заданным значением и потомками
TreeNodePtr<int> N(int val, TreeNodePtr<int>&& left = {}, TreeNodePtr<int>&& right = {}) {
  auto node = std::make_unique<TreeNode<int>>(val, std::move(left), std::move(right));

  if (node->left != nullptr) {
    node->left->parent = node.get();
  }

  if (node->right != nullptr) {
    node->right->parent = node.get();
  }

  return node;
}

int main() {
  using namespace std;
  using T = TreeNode<int>;
  auto root1 = N(6, N(4, N(3), N(5)), N(7));
  assert(CheckTreeProperty(root1.get()));

  T* iter = begin(root1.get());
  while (iter) {
    cout << iter->value << " "s;
    iter = next(iter);
  }
  cout << endl;

  auto root2 = N(6, N(4, N(3), N(5)), N(7, N(8)));
  assert(!CheckTreeProperty(root2.get()));

  // Функция DeleteTree не нужна. Узлы дерева будут рекурсивно удалены
  // благодаря деструкторам unique_ptr
}
