#ifndef LAB2_TPATRICIA_HPP
#define LAB2_TPATRICIA_HPP



#include <iostream>
#include <fstream>
#include <cstring>

#include "TVector.hpp"


#define SIZE 128
#define CHAR_SIZE 8


template <typename T>
class TErr {

public:
    TErr() = default;

    explicit TErr(const T& newValue) : value(newValue), hasValue(true) {}

    const T& operator* () const {
        if (hasValue) {
            return value;
        }
        throw std::runtime_error("err");
    }

    T& operator* () {
        if (hasValue) {
            return value;
        }
        throw std::runtime_error("err");
    }

    operator bool() const {
        return hasValue;
    }

private:
    T value;
    bool hasValue = false;
};



void printChar(unsigned char a) {
    for (int i = CHAR_SIZE - 1; i >= 0; --i) {
        std::cout << ((a & (1 << i)) != 0);
    }
}

bool getNthBit(const TVector<unsigned char>& lhs, size_t index) {
    if (index / CHAR_SIZE >= lhs.Size()) {
        return false;
    }
    unsigned char cur_char = lhs[index / CHAR_SIZE];
    return (cur_char & (SIZE >> (index % CHAR_SIZE))) != 0;
}

template <typename T>
std::ostream& operator << (std::ostream& os, const TVector<T>& vec) {
    for (size_t i = 0; i < vec.Size(); ++i) {
        os << vec[i];
    }
    return os;
}

template <typename T>
bool operator == (const TVector<T>& lhs, const TVector<T>& rhs) {
    if (lhs.Size() != rhs.Size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.Size(); ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool operator != (const TVector<T>& lhs, const TVector<T>& rhs) {
    return !(lhs == rhs);
}


template <typename T>
class TPatricia {
public:
    TPatricia() = default;

    ~TPatricia() {
        DeleteTree(root);
    }



    TErr<T> operator [] (const TVector<unsigned char>& string) const {
        Node* endNode = SearchKey(string);
        if (endNode && endNode->string == string) {
            return TErr<T>(endNode->data);
        }
        return TErr<T>();
    }

    bool Insert(TVector<unsigned char> string, T data) {
        Node* reachedNode = SearchKey(string);
        if (!reachedNode) {
            root = new Node;
            root->left = root;
            root->right = nullptr;
            root->string = std::move(string);
            root->data = data;
            root->index_to_check = -1;
            return true;
        }

        const TVector<unsigned char>& reachedKey = reachedNode->string;
        if (reachedKey == string) {
            return false;
        }

        long long differPos = -1;
        for (size_t i = 0; i < std::max(string.Size(), reachedKey.Size()) * CHAR_SIZE; ++i) {
            if (getNthBit(reachedKey, i) != getNthBit(string, i)) {
                differPos = i;
                break;
            }
        }
        Node* new_node = new Node;
        new_node->index_to_check = differPos;
        new_node->string = std::move(string);
        new_node->data = data;

        Node* tmp_prev = root;
        Node* tmp_node = root->left;
        while (tmp_node->index_to_check > tmp_prev->index_to_check
               && tmp_node->index_to_check < differPos) {
            tmp_prev = tmp_node;
            tmp_node = (getNthBit(new_node->string, tmp_node->index_to_check) ? tmp_node->right : tmp_node->left);
        }
        Node* link_to_break = tmp_node;

        if (tmp_node == tmp_prev->right) {
            tmp_prev->right = new_node;
        } else {
            tmp_prev->left = new_node;
        }

        if (getNthBit(new_node->string, differPos)) {
            new_node->right = new_node;
            new_node->left = link_to_break;
        } else {
            new_node->left = new_node;
            new_node->right = link_to_break;
        }
        return true;
    }

    bool Erase(const TVector<unsigned char>& string) {
        Node* backwardPtrFrom;
        Node* parentNode;
        Node* reachedNode = SearchKey(string, &backwardPtrFrom);
        if (!reachedNode || reachedNode->string != string) {
            return false;
        }
        parentNode = SearchParentNode(reachedNode);
        if (reachedNode == root && root->left == root) {
            delete root;
            root = nullptr;
            return true;
        }
        if (reachedNode->left == reachedNode || reachedNode->right == reachedNode) {
            Node* nonSelfLink;
            if (reachedNode->left == reachedNode) {
                nonSelfLink = reachedNode->right;
            } else {
                nonSelfLink = reachedNode->left;
            }
            if (reachedNode == parentNode->right) {
                parentNode->right = nonSelfLink;
            } else if (reachedNode == parentNode->left) {
                parentNode->left = nonSelfLink;
            } else {
                throw std::logic_error("err");
            }
            delete reachedNode;
            return true;
        }
        Node* r = nullptr;
        Node* backParent = SearchParentNode(backwardPtrFrom);
        SearchKey(backwardPtrFrom->string, &r);
        bool isRight = getNthBit(r->string, backwardPtrFrom->index_to_check);
        std::swap(reachedNode->string, backwardPtrFrom->string);
        std::swap(reachedNode->data, backwardPtrFrom->data);
        if (r->right == backwardPtrFrom) {
            r->right = reachedNode;
        } else if (r->left == backwardPtrFrom) {
            r->left = reachedNode;
        } else {
            throw std::logic_error("err");
        }

        if (backwardPtrFrom == backParent->right) {
            if (isRight) {
                backParent->right = backwardPtrFrom->right;
            } else {
                backParent->right = backwardPtrFrom->left;
            }
        } else if (backwardPtrFrom == backParent->left) {
            if (isRight) {
                backParent->left = backwardPtrFrom->right;
            } else {
                backParent->left = backwardPtrFrom->left;
            }
        } else {
            throw std::logic_error("err");
        }
        delete backwardPtrFrom;
        return true;
    }


    void ScanFromFile(const char* filename) {
        std::fstream file(filename, std::ios::in|std::ios::binary);
        if (!file.is_open()) {
            return;
        }
        if (file.peek() == EOF) {
            TPatricia<T> patr;
            std::swap(patr.root, root);
            return;
        }
        Node* new_header = nullptr;
        size_t nodesCount;
        file.read((char*)&nodesCount, sizeof(size_t));
        TVector<Node*> nodes(nodesCount);
        for (size_t i = 0; i < nodesCount; ++i) {
            size_t id, stringSize;
            T data;
            long long index;
            file.read((char*)&id, sizeof(size_t));
            file.read((char*)&data, sizeof(T));
            file.read((char*)&stringSize, sizeof(size_t));
            TVector<unsigned char> string(stringSize);
            for (size_t j = 0; j < stringSize; ++j) {
                file.read((char*)&string[j],sizeof(unsigned char));
            }
            file.read((char*)&index, sizeof(long long));
            Node* new_node = new Node;
            new_node->index_to_check = index;
            new_node->string = string;
            new_node->data = data;
            nodes[id] = new_node;
            if (index == -1) {
                new_header = new_node;
            }
        }
        new_header->right = nullptr;
        for (size_t i = 0; i < nodes.Size() * 2 - 1; ++i) {
            size_t id, link;
            char direction;
            file.read((char*)&id, sizeof(size_t));
            file.read((char*)&link, sizeof(size_t));
            file.read((char*)&direction, sizeof(char));
            if (direction == '0') {
                nodes[id]->left = nodes[link];
            } else if (direction == '1') {
                nodes[id]->right = nodes[link];
            }
        }

        TPatricia<T> patr;
        patr.root = new_header;
        std::swap(patr.root, root);
    }

    void PrintToFile(const char* filename) const {
        std::ofstream file;
        file.open(filename, std::ios::binary | std::ios::out);
        file.sync_with_stdio(false);
        if (!file.is_open()) {
            return;
        }
        if (root == nullptr) {
            return;
        }
        TVector<Node*> nodes;
        int counter = 0;
        CountIds(root, counter, nodes);
        size_t size = nodes.Size();
        file.write((char*)&size, sizeof(size_t));
        for (size_t i = 0; i < nodes.Size(); ++i) {
            Node* node = nodes[i];
            file.write((char*)&node->id, sizeof(size_t));
            file.write((char*)&node->data, sizeof(T));
            size_t string_size = node->string.Size();

            file.write((char*)&string_size, sizeof(size_t));
            file.write((char*)node->string.begin(), sizeof(unsigned char) * string_size);
            file.write((char*)&node->index_to_check, sizeof(long long));
        }
        for (size_t i = 0; i < nodes.Size(); ++i) {
            Node* node = nodes[i];
            Node* nodeRight = node->right;
            Node* nodeLeft = node->left;
            if (nodeRight != nullptr) {
                file.write((char*)&node->id, sizeof(size_t));
                size_t id;
                id = nodeRight->id;
                file.write((char*)&id, sizeof(size_t));
                char right = '1';
                file.write((char*)&right, sizeof(char));
            }
            file.write((char*)&node->id, sizeof(size_t));
            size_t id;
            id = nodeLeft->id;
            file.write((char*)&id, sizeof(size_t));
            char right = '0';
            file.write((char*)&right, sizeof(char));
        }
        file.close();
    }



    void Print(std::ostream& os) const {
        Print(root, os, 0);
    }



private:
    struct Node {
        long long index_to_check;
        TVector<unsigned char> string;
        T data;
        Node* left;
        Node* right;
        size_t id;
    };

    void CountIds(Node* node, int& id, TVector<Node*>& nodes) const {
        node->id = id;
        nodes.PushBack(node);
        id++;
        if (node->right != nullptr && node->index_to_check < node->right->index_to_check) {
            CountIds(node->right, id, nodes);
        }
        if (node->index_to_check < node->left->index_to_check) {
            CountIds(node->left, id, nodes);
        }
    }

    void DeleteTree(Node* node) {
        if (node == nullptr) {
            return;
        }
        if(node->right != nullptr && node->index_to_check < node->right->index_to_check) {
            DeleteTree(node->right);
        }
        if (node->index_to_check < node->left->index_to_check) {
            DeleteTree(node->left);
        }
        delete node;
    }

    Node* SearchParentNode(Node* node) const {
        if (node == root->left) {
            return root;
        }
        Node* tmp = root->left;
        while(tmp->right != node && tmp->left != node) {
            if (getNthBit(node->string, tmp->index_to_check)) {
                tmp = tmp->right;
            } else {
                tmp = tmp->left;
            }
        }
        if (tmp->right == node || tmp->left == node) {
            return tmp;
        } else {
            throw std::logic_error("err");
        }
    }

    Node* SearchKey(const TVector<unsigned char>& string, Node** back = nullptr) const {
        if (root == nullptr) {
            return nullptr;
        }
        Node* prev_node = root;
        Node* cur_node = root->left;
        while (cur_node->index_to_check > prev_node->index_to_check) {
            bool cur_bit = getNthBit(string, cur_node->index_to_check);
            prev_node = cur_node;
            if (cur_bit){
                cur_node = cur_node->right;
            } else {
                cur_node = cur_node->left;
            }
        }
        if (back != nullptr) {
            *back = prev_node;
        }
        return cur_node;
    }



    void Print(Node* node, std::ostream& os, int depth) const {
        if(node == nullptr){
            return;
        }
        for (int i = 0; i < depth * 4; ++i) {
            os << " ";
        }
        os << "key: " << node->string << ", value: " << node->data << ", index: " << node->index_to_check;
        os << "\n";
        if (node->left != nullptr && node->index_to_check < node->left->index_to_check) {
            Print(node->left, os, depth + 1);
        }
        if (node->right != nullptr && node->index_to_check < node->right->index_to_check) {
            Print(node->right, os, depth + 1);
        }
    }

    Node* root = nullptr;

};

#endif //LAB2_TPATRICIA_HPP
