#ifndef AVL_H
#define AVL_H

#include "IBST.h"
#include <sstream>
#include <list>
#include <string>

template <typename K, typename V>
class AVL : public IBST<K, V> {
public:
    struct Node {
        K key;
        V value;

        int bfactor;
        int height;

        Node* pLeft;
        Node* pRight;

        Node(const K& k, const V& v)
            : key(k), value(v), bfactor(0), height(1),
            pLeft(nullptr), pRight(nullptr) {}

        virtual ~Node() = default;

        int balance() const {
            int leftH = pLeft ? pLeft->height : 0;
            int rightH = pRight ? pRight->height : 0;
            return leftH - rightH;
        }
        
        static std::string defaultEntry2Str(const K& k, const V& v) {
    std::ostringstream oss;
    oss << "(Entry)"; // Thay vì in 'k' và 'v' trực tiếp, ta in chữ này để tránh lỗi compile cho pair
    return oss.str();
}
        
        std::string toString(std::string (*entry2str)(const K&, const V&) = nullptr) const {
            auto entryStr = [entry2str](const K& kk, const V& vv) -> std::string {
                return entry2str ? entry2str(kk, vv) : defaultEntry2Str(kk, vv);
            };
            std::ostringstream out;
            if (!pLeft && !pRight) {
                out << "[" << entryStr(key, value) << ":" << balance() << "]";
            }
            else if (!pLeft && pRight) {
                out << " (" << entryStr(key, value) << ":" << balance()
                    << "[.]" << pRight->toString(entry2str) << ")";
            }
            else if (pLeft && !pRight) {
                out << " (" << entryStr(key, value) << ":" << balance()
                    << pLeft->toString(entry2str) << "[.]" << ")";
            }
            else {
                out << " (" << entryStr(key, value) << ":" << balance()
                    << pLeft->toString(entry2str)
                    << pRight->toString(entry2str)
                    << ") ";
            }
            return out.str();
        }
    };

protected:
    Node* pRoot;
    int count;

    // --- HÀM TỰ CHẾ ---
    int maxVal(int a, int b) const {
        return (a > b) ? a : b;
    }

    int getHeight(Node* node) const {
        if (node == nullptr) return 0;
        return node->height;
    }

    int getBalance(Node* node) const {
        if (node == nullptr) return 0;
        return getHeight(node->pLeft) - getHeight(node->pRight);
    }

    Node* rotateRight(Node* y) {
        Node* x = y->pLeft;
        Node* T2 = x->pRight;
        
        x->pRight = y;
        y->pLeft = T2;
        
        y->height = 1 + maxVal(getHeight(y->pLeft), getHeight(y->pRight));
        x->height = 1 + maxVal(getHeight(x->pLeft), getHeight(x->pRight));
        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->pRight;
        Node* T2 = y->pLeft;
        
        y->pLeft = x;
        x->pRight = T2;
        
        x->height = 1 + maxVal(getHeight(x->pLeft), getHeight(x->pRight));
        y->height = 1 + maxVal(getHeight(y->pLeft), getHeight(y->pRight));
        return y;
    }

    Node* insertNode(Node* current, const K& key, const V& value,
                Node*& pred, Node*& succ, bool& isInserted, Node*& newNode) {
        if (current == nullptr) {
            newNode = createNode(key, value);
            isInserted = true;
            return newNode;
        }
        if (key < current->key) {
            succ = current;
            current->pLeft = insertNode(current->pLeft, key, value, pred, succ, isInserted, newNode);
        }
        else if (key > current->key) {
            pred = current;
            current->pRight = insertNode(current->pRight, key, value, pred, succ, isInserted, newNode);
        }
        else {
            isInserted = false;
            return current;
        }
        
        if (!isInserted) return current;
        
        current->height = 1 + maxVal(getHeight(current->pLeft), getHeight(current->pRight));
        
        int val = getBalance(current);
        if (val > 1 && key < current->pLeft->key) {
            return rotateRight(current);
        }
        if (val < -1 && key > current->pRight->key) {
            return rotateLeft(current);
        }
        if (val > 1 && key > current->pLeft->key) {
            current->pLeft = rotateLeft(current->pLeft);
            return rotateRight(current);
        }
        if (val < -1 && key < current->pRight->key) {
            current->pRight = rotateRight(current->pRight);
            return rotateLeft(current);
        }
        return current;
    }

    Node* eraseNode(Node* current, const K& key, bool& isDeleted) {
        if (current == nullptr) {
            isDeleted = false;
            return nullptr;
        }
        if (key < current->key) {
            current->pLeft = eraseNode(current->pLeft, key, isDeleted);
        }
        else if (key > current->key) {
            current->pRight = eraseNode(current->pRight, key, isDeleted);
        }
        else {
            if (current->pLeft == nullptr || current->pRight == nullptr) {
                Node* temp = current->pLeft ? current->pLeft : current->pRight;
                onErasing(current, nullptr, nullptr); 
                delete current;
                isDeleted = true;
                return temp;
            }
            else {
                Node* successor = current->pRight;
                while (successor->pLeft != nullptr) successor = successor->pLeft;
                onReplaceBySuccessor(current, successor, successor->pRight); 
                current->key = successor->key;
                current->value = successor->value;
                current->pRight = eraseNode(current->pRight, successor->key, isDeleted);
            }
        }
        if (current == nullptr) return nullptr;
        
        current->height = 1 + maxVal(getHeight(current->pLeft), getHeight(current->pRight));
        int val = getBalance(current);
        
        if (val > 1) {
            if (getBalance(current->pLeft) >= 0) return rotateRight(current);
            current->pLeft = rotateLeft(current->pLeft);
            return rotateRight(current);
        }
        if (val < -1) {
            if (getBalance(current->pRight) <= 0) return rotateLeft(current);
            current->pRight = rotateRight(current->pRight);
            return rotateLeft(current);
        }
        return current;
    }

    void Traverse(Node* cur, std::list<K>& result) const {
        if (cur == nullptr) return;
        Traverse(cur->pLeft, result);
        result.push_back(cur->key);
        Traverse(cur->pRight, result);
    }
    
    void TraverseReverse(Node* cur, std::list<K>& result) const {
        if (cur == nullptr) return;
        TraverseReverse(cur->pRight, result);
        result.push_back(cur->key);
        TraverseReverse(cur->pLeft, result);
    }
    
    int countNodes(Node* node) const {
        if(node == nullptr) return 0;
        return 1 + countNodes(node->pLeft) + countNodes(node->pRight);
    }
    
    void clearNodes(Node* cur){
        if (cur == nullptr) return;
        clearNodes(cur->pLeft);
        clearNodes(cur->pRight);
        delete cur;
    }

protected:
    virtual void onInserted(Node* /*newNode*/, Node* /*pred*/, Node* /*succ*/) {}
    virtual void onErasing(Node* /*delNode*/, Node* /*pred*/, Node* /*succ*/) {}
    virtual void onReplaceBySuccessor(Node* /*target*/, Node* /*successor*/, Node* /*successorNext*/) {}
    virtual Node* createNode(const K& key, const V& value) { return new Node(key, value); }

public:
    AVL() : pRoot(nullptr), count(0) {}
    virtual ~AVL() { clear(); }

    bool insert(const K& key, const V& value) override {
        Node *pred = nullptr, *succ = nullptr, *newNode = nullptr;
        bool isInserted = false;
        pRoot = insertNode(pRoot, key, value, pred, succ, isInserted, newNode);
        if (isInserted) {
            onInserted(newNode, pred, succ);
            count++;
            return true;
        }
        return false;
    }

    bool erase(const K& key) override {
        bool isDeleted = false;
        pRoot = eraseNode(pRoot, key, isDeleted);
        if(isDeleted) {
            count--;
            return true;
        }
        return isDeleted;
    }

    V* find(const K& key) override {
        Node* cur = pRoot;
        while(cur) {
            if (key == cur->key) return &(cur->value); 
            else if (key < cur->key) cur = cur->pLeft;
            else cur = cur->pRight;
        }
        return nullptr;
    }

    bool contains(const K& key) const override {
        Node* cur = pRoot;
        while(cur) {
            if (key == cur->key) return true;
            else if (key < cur->key) cur = cur->pLeft;
            else cur = cur->pRight;
        }
        return false;
    }

    int size() const override {
        return countNodes(pRoot);
    }

    bool empty() const override {
        return pRoot == nullptr;
    }

    void clear() override {
        clearNodes(pRoot);
        pRoot = nullptr;
        count = 0;
    }

    int height() const override {
        return getHeight(pRoot);
    }

    std::list<K> ascendingList() const override {
        std::list<K> keys;
        Traverse(pRoot, keys);
        return keys;
    }

    std::list<K> descendingList() const override {
        std::list<K> keys;
        TraverseReverse(pRoot, keys);
        return keys;
    }

    std::string toString(std::string (*entry2str)(const K&, const V&) = nullptr) const override {
        if (!pRoot) return "(NULL)";
        return pRoot->toString(entry2str);
    }
};

#endif /* AVL_H */