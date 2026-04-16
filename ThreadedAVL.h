#ifndef THREADED_AVL_H
#define THREADED_AVL_H

#include "AVL.h"
#include <list>

// =======================
// ThreadedAVL (SKELETON - BONUS)
// =======================

template <typename K, typename V>
class ThreadedAVL : public AVL<K, V> {
private:
    using Base = AVL<K, V>;
    using AVLNode = typename Base::Node;

    struct TNode : public AVLNode {
        TNode* prev;
        TNode* next;

        TNode(const K& k, const V& v)
            : AVLNode(k, v), prev(nullptr), next(nullptr) {}
    };

    TNode* head_;
    TNode* tail_;

public:

    class Iterator {
        friend class ThreadedAVL<K, V>;
        TNode* p_;
        explicit Iterator(TNode* p) : p_(p) {}
    public:
        Iterator() : p_(nullptr) {}

        bool operator==(const Iterator& other) const { return p_ == other.p_; }
        bool operator!=(const Iterator& other) const { return p_ != other.p_; }

        Iterator& operator++() { if (p_) p_ = p_->next; return *this; }
        Iterator& operator--() { if (p_) p_ = p_->prev; return *this; }

        const K& key() const { return p_->key; }
        V& value() { return p_->value; }
        const V& value() const { return p_->value; }
        bool isNull() const { return p_ == nullptr; }

        bool hasTwoChildren() const {
            return p_ && p_->pLeft != nullptr && p_->pRight != nullptr;
        }
    };

public:
    ThreadedAVL() : Base(), head_(nullptr), tail_(nullptr) {}
    ~ThreadedAVL() { this->clear(); }
    
    Iterator rbeginIt() const { return Iterator(tail_); }

    TNode* getHead() const { return head_; }
    TNode* getTail() const { return tail_; }
    
    std::list<K> ascendingList() const override {
        std::list<K> keys;
        TNode* current = head_;
        while (current != nullptr) {
            keys.push_back(current->key);
            current = current->next;
        }
        return keys;
    }

    std::list<K> descendingList() const override {
        std::list<K> keys;
        TNode* current = tail_;
        while (current != nullptr) {
            keys.push_back(current->key);
            current = current->prev;
        }
        return keys;
    }

    void clear() override {
        AVL<K, V>::clear();
        head_ = nullptr;
        tail_ = nullptr;
    }

    Iterator beginIt() { return Iterator(head_); }
    Iterator beginIt() const { return Iterator(head_); }

    Iterator endIt() { return Iterator(nullptr); }
    Iterator endIt() const { return Iterator(nullptr); }

    Iterator findIt(const K& key) {
        AVLNode* current = this->pRoot;
        while (current != nullptr) {
            if (key == current->key) return Iterator(static_cast<TNode*>(current));
            else if (key < current->key) current = current->pLeft;
            else current = current->pRight;
        }
        return Iterator(nullptr);
    }

protected:
    AVLNode* createNode(const K& key, const V& value) override {
        return new TNode(key, value);
    }

    void onInserted(AVLNode* newNode,
                    AVLNode* pred,
                    AVLNode* succ) override {
        TNode* tNew = static_cast<TNode*>(newNode);
        TNode* tPred = static_cast<TNode*>(pred);
        TNode* tSucc = static_cast<TNode*>(succ);
        
        tNew->prev = tPred;
        tNew->next = tSucc;
        
        if (tPred != nullptr) tPred->next = tNew;
        else head_ = tNew;
        
        if (tSucc != nullptr) tSucc->prev = tNew;
        else tail_ = tNew;
    }

    void onErasing(AVLNode* delNode,
                    AVLNode* pred,
                    AVLNode* succ) override {
        TNode* tDel = static_cast<TNode*>(delNode);
        TNode* tPred = tDel->prev;
        TNode* tSucc = tDel->next;

        if (tPred != nullptr) tPred->next = tSucc;
        else head_ = tSucc;
        
        if (tSucc != nullptr) tSucc->prev = tPred;
        else tail_ = tPred;
        
        tDel->prev = nullptr;
        tDel->next = nullptr;
    }

    void onReplaceBySuccessor(AVLNode* /*target*/,
                            AVLNode* /*successor*/,
                            AVLNode* /*successorNext*/) override {
    }
};

#endif /* THREADED_AVL_H */