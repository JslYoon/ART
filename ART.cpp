#include <iostream>
#include <cstring>

namespace ART {

    enum emptyset {
        EMPTY = 49
    };

    struct Node {
        uint16_t childCount;
        uint8_t prefix[8];
        uint8_t prefixLen;

        virtual Node* findChild(uint8_t key) = 0;
        virtual void addChild(uint8_t key, Node* child) = 0;
        virtual bool isFull() = 0;
    };


    struct Node4 : public Node {
        uint8_t keys[4] = {};
        Node* children[4] = {};

        Node4() = default;

        Node* findChild(uint8_t key) override {
            for (int i = 0; i < childCount; ++i) {
                if (keys[i] == key) { return children[i]; }
            }
            return nullptr;
        }


        void addChild(uint8_t key, Node* child) override {
            if (childCount < 4) {
                keys[childCount] = key;
                children[childCount] = child;
                childCount++;
            }
        }

        bool isFull() override { return childCount == 4; }
    };


    struct Node16 : public Node {
        uint8_t keys[16] = {};
        Node* children[16] = {};

        Node16() = default;

        Node* findChild(uint8_t key) override {
            for (int i = 0; i < childCount; ++i) {
                if (keys[i] == key) { return children[i]; }
            }
            return nullptr;
        }

        void addChild(uint8_t key, Node* child) override {
            if (childCount < 4) {
                keys[childCount] = key;
                children[childCount] = child;
                childCount++;
            }
        }

        bool isFull() override { return childCount == 16; }

    };


    struct Node48 : public Node {
        uint8_t childIndex[256] = {};
        Node* children[48] = {};

        Node48() = default;

        Node* findChild(uint8_t key) override {
            uint8_t idx = childIndex[key];
            if (idx != EMPTY) { return children[idx]; }
            return nullptr;
        }

        void addChild(uint8_t key, Node* child) override {
            if (childCount < 48) {
                childIndex[key] = childCount;
                children[childCount] = child;
                childCount++;
            }
        }

        bool isFull() override { return childCount == 48; }

    };


    struct Node256 : public Node {
        Node* children[256] = {};

        Node256() = default;

        Node* findChild(uint8_t key) override { return children[key]; }

        void addChild(uint8_t key, Node* child) override {
            if (!children[key]) {
                children[key] = child;
                childCount++;
            }
        }

        bool isFull() override { return childCount == 256; }

    };


    struct LeafNode : public Node {
        const uint8_t* key;
        long value;

        LeafNode(const uint8_t* k, long v) : key(k), value(v) {};

        Node* findChild(uint8_t key) override { return nullptr; }
        void addChild(uint8_t key, Node* child) override {}
        bool isFull() override { return true; }
    };



    class ART {

        public:
            ART() : root(nullptr) {}


            long search(const uint8_t* key) {
                Node* resultNode = search(root, key, 0);
                if (LeafNode* foundLeaf = dynamic_cast<LeafNode*>(resultNode)) {
                    return foundLeaf->value;
                } else {
                    return NULL;
                }
            }


            void insert(const uint8_t* key, long value) {

                if(root == nullptr) {
                    root = new LeafNode(key, value);
                } else {
                    insert(root, key, new LeafNode(key, value), 0);
                }

            }

        private:
            Node* root;


            Node* search(Node* node, const uint8_t* key, uint8_t depth) {
                if (node == nullptr) {
                    return nullptr;
                }

                if (LeafNode* leaf = dynamic_cast<LeafNode*>(node)) {
                    if (std::memcmp(leaf->key, key, std::strlen(reinterpret_cast<const char*>(key))) == 0) {
                        return node;
                    }
                    // if (leaf->key == key) {
                    //     return node;
                    // }
                    return nullptr;
                }

                if (checkPrefix(node, key, depth) != node->prefixLen) {
                    return nullptr;
                }
                depth += node->prefixLen;
                Node* next = node->findChild(key[depth]);
                return search(next, key, depth+1);

            }


            void insert(Node*& node, const uint8_t* key, LeafNode* leaf, uint8_t depth) {

                if (node == nullptr) {
                    node = leaf;
                    return;
                }
                if(LeafNode* resultLeaf = dynamic_cast<LeafNode*>(node)) { 
                    Node* newNode = new Node4();
                    const uint8_t* key2 = resultLeaf->key;
                    size_t i = depth;
                    while(key[i] == key2[i] && i < depth + 8) {
                        newNode->prefix[i-depth] = key[i];
                        ++i;
                    }

                    newNode->prefixLen = i - depth;
                    depth += newNode->prefixLen;

                    newNode->addChild(key[depth], leaf);
                    newNode->addChild(key2[depth], node);
                    node = newNode;
                    return;
                }

                uint8_t p = checkPrefix(node, key, depth);
                if(p != node->prefixLen) {
                    Node4* newNode = new Node4();
                    newNode->addChild(node->prefix[p], node);
                    newNode->addChild(key[depth + p], leaf);

                    newNode->prefixLen = p;
                    std::memcpy(newNode->prefix, node->prefix, p);
                    node->prefixLen -= (p+1);
                    std::memmove(node->prefix, node->prefix+p+1, node->prefixLen);
                    node = newNode;
                    return;
                }

                depth += node->prefixLen;
                Node* next = node->findChild(key[depth]);
                if(next) {
                    insert(next, key, leaf, depth+1);
                } else {
                    if(node->isFull()) {
                        grow(node);
                    }
                    node->addChild(key[depth], leaf);
                }
            }
            

            int checkPrefix(Node* node, const uint8_t* key, int depth) {
                int keyLength = std::strlen(reinterpret_cast<const char*>(key));
                int len = std::min(node->prefixLen, static_cast<uint8_t>(keyLength - depth));
                for (int i = 0; i < len; ++i) {
                    if (node->prefix[i] != key[depth + i]) {
                        return i;
                    }
                }
                return len;
            }

            void grow(Node*& node) {
                if (Node4* n4 = dynamic_cast<Node4*>(node)) {
                    Node16* newNode = new Node16();
                    for (int i = 0; i < 4; ++i) {
                        newNode->keys[i] = n4->keys[i];
                        newNode->children[i] = n4->children[i];
                    }
                    newNode->childCount = n4->childCount;
                    newNode->prefixLen = n4->prefixLen;
                    std::memcpy(newNode->prefix, n4->prefix, n4->prefixLen);

                    delete n4;
                    node = newNode;
                } else if (Node16* n16 = dynamic_cast<Node16*>(node)) {
                    Node48* newNode = new Node48();

                    for (int i = 0; i < 16; ++i) {
                        newNode->childIndex[n16->keys[i]] = i;
                        newNode->children[i] = n16->children[i];
                    }
                    newNode->childCount = n16->childCount;
                    newNode->prefixLen = n16->prefixLen;
                    std::memcpy(newNode->prefix, n16->prefix, n16->prefixLen);

                    delete n16;
                    node = newNode;
                } else if (Node48* n48 = dynamic_cast<Node48*>(node)) {
                    Node256* newNode = new Node256();

                    for (int i = 0; i < 256; ++i) {
                        if (n48->childIndex[i] != EMPTY) {
                            newNode->children[i] = n48->children[n48->childIndex[i]];
                        }
                    }
                    newNode->childCount = n48->childCount;
                    newNode->prefixLen = n48->prefixLen;
                    std::memcpy(newNode->prefix, n48->prefix, n48->prefixLen);

                    delete n48;
                    node = newNode;
                }
            }


    };
} // namespace ART

void testInsertAndSearch() {
    ART::ART tree;

    // Sample keys and values
    const uint8_t key1[] = { 'a', 'p', 'p', 'l', 'e', '\0' };  // "apple"
    const uint8_t key2[] = { 'b', 'a', 'n', 'a', 'n', 'a', '\0' };  // "banana"
    const uint8_t key3[] = { 'g', 'r', 'a', 'p', 'e', '\0' };  // "grape"
    const uint8_t key4[] = { 'o', 'r', 'a', 'n', 'g', 'e', '\0' };  // "orange"
    const uint8_t key5[] = { 'w', 'a', 't', 'e', 'r', 'm', 'e', 'l', 'o', 'n', '\0' };  // "watermelon"
    const uint8_t key6[] = { 'x', 'a', 't', 'l', 'e', '\0' };  // "xatle"

    // Insert keys and values into the ART
    tree.insert(key1, 100);  // apple -> 100
    tree.insert(key2, 200);  // banana -> 200
    tree.insert(key3, 300);  // grape -> 300
    tree.insert(key4, 400);  // orange -> 400
    tree.insert(key5, 500);  // watermelon -> 500
    tree.insert(key6, 150);  // xatle -> 150

    // Test cases for searching the inserted keys
    std::cout << "Searching for 'apple': " << tree.search(key1) << " (expected: 100)" << std::endl;
    std::cout << "Searching for 'banana': " << tree.search(key2) << " (expected: 200)" << std::endl;
    std::cout << "Searching for 'grape': " << tree.search(key3) << " (expected: 300)" << std::endl;
    std::cout << "Searching for 'orange': " << tree.search(key4) << " (expected: 400)" << std::endl;
    std::cout << "Searching for 'watermelon': " << tree.search(key5) << " (expected: 500)" << std::endl;
    std::cout << "Searching for 'xatle': " << tree.search(key6) << " (expected: 150)" << std::endl;

    // Test for non-existent key
    const uint8_t nonExistentKey[] = { 'p', 'e', 'a', 'r', '\0' };  // "pear"
    std::cout << "Searching for 'pear' (non-existent): " 
              << (tree.search(nonExistentKey) == NULL ? "PASS" : "FAIL") << " (expected: PASS)" << std::endl;
}

int main() {
    testInsertAndSearch();

    return 0;
}