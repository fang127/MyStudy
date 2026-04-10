#include <queue>
#include <vector>
#include <iostream>

class Node
{
public:
    int val;
    Node *left;
    Node *right;

    Node(int x = 0) : val(x), left(nullptr), right(nullptr) {}
};

// 构建二叉树
Node *makeBinTree(const std::vector<int> &vec)
{
    if (vec.empty()) return nullptr;
    Node *root = new Node(vec[0]);
    std::vector<Node *> tree(vec.size());
    tree[0] = root;
    for (int i = 1; i < vec.size(); ++i)
    {
        Node *node = new Node(vec[i]);
        tree[i] = node;
        Node *parent = tree[(i - 1) / 2];
        if (i % 2 == 1)
            parent->left = node;
        else
            parent->right = node;
    }

    return root;
}

void traversal(Node *root)
{
    // 验证二叉树结构
    if (root == nullptr) return;
    std::cout << root->val << " ";
    traversal(root->left);
    traversal(root->right);
}

int main()
{
    std::vector<int> vec;
    for (int i = 0; i < 10; ++i) vec.push_back(i);

    for (int e : vec) std::cout << e << " ";
    std::cout << std::endl;

    Node *root = makeBinTree(vec);
    // 层序遍历二叉树
    std::queue<Node *> que;
    que.push(root);
    while (!que.empty())
    {
        int size = que.size();
        for (int i = 0; i < size; ++i)
        {
            Node *node = que.front();
            std::cout << node->val << " ";
            que.pop();
            if (node->left) que.push(node->left);
            if (node->right) que.push(node->right);
        }
    }
}