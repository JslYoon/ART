#include <iostream>

namespace ART {

    struct Node {
        uint16_t childCount;
        uint8_t prefix[8];
        uint8_t prefixLen;

        virtual Node* findChild(uint8_t key) = 0;
        virtual void addChild(uint8_t key, Node* child) = 0;
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


        void addChild(uint8_t key, Node* child) {
            if (childCount < 4) {
                keys[childCount] = key;
                children[childCount] = child;
                childCount++;
            }
        }

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

        void addChild(uint8_t key, Node* child) {
            if (childCount < 4) {
                keys[childCount] = key;
                children[childCount] = child;
                childCount++;
            }
        }

    };


    struct Node48 : public Node {
        uint8_t childIndex[256] = {};
        Node* children[48] = {};

        Node48() = default;

        Node* findChild(uint8_t key) override {
            uint8_t idx = childIndex[key];
            if (idx != 255) { return children[idx]; }
            return nullptr;
        }

        void addChild(uint8_t key, Node* child) {
            if (childCount < 48) {
                childIndex[key] = childCount;
                children[childCount] = child;
                childCount++;
            }
        }

    };


    struct Node256 : public Node {
        Node* children[256] = {};

        Node256() = default;

        Node* findChild(uint8_t key) override { return children[key]; }

        void addChild(uint8_t key, Node* child) {
            if (!children[key]) {
                children[key] = child;
                childCount++;
            }
        }

    };


    struct LeafNode : public Node {
        std::string key;
        std::string value;

        LeafNode(const std::string& k, const std::string& v) : key(k), value(v) {};
        Node* findChild(uint8_t key) override { return nullptr; }

        void addChild(uint8_t key, Node* child) override {}
    };



    class ART {
        public:
            
            ART() : root(nullptr) {}

            Node* root;

            std::string* search(const std::string& key) {
                Node* resultNode = search(root, key, 0);
                if (LeafNode* foundLeaf = dynamic_cast<LeafNode*>(resultNode)) {
                    return &foundLeaf->value;
                } else {
                    return nullptr;
                }
            }

            void insert(const std::string& key, const std::string& value) {

                if(root == nullptr) {
                    root = new LeafNode(key, value);
                } else {
                    insert(root, key, new LeafNode(key, value), 0);
                }

            }

            


        
        private:

            // Node* root;

            Node* search(Node* node, const std::string& key, uint8_t depth) {
                if (node == nullptr) {
                    return nullptr;
                }

                if (LeafNode* leaf = dynamic_cast<LeafNode*>(node)) {
                    if (leaf->key == key) {
                        return node;
                    }
                    return nullptr;
                }

                if (checkPrefix(node, key, depth) != node->prefixLen) {
                    return nullptr;
                }
                depth += node->prefixLen;
                Node* next = node->findChild(key[depth]);
                return search(next, key, depth);

            }


            void insert(Node* node, const std::string& key, LeafNode* leaf, uint8_t depth) {

                if (node == nullptr) {
                    return nullptr;
                }

                if(LeafNode* resultLeaf = dynamic_cast<LeafNode*>(node)) {
                    Node* newNode = new Node4();
                    std::string& key2 = resultLeaf->key;

                    uint8_t i;
                    for (i=depth; i < key.size() && i < key2.size() && key[i]==key2[i]; i++) {
                        newNode->prefix[i-depth] = key[i];
                    }

                    newNode->prefixLen = i - depth;
                    newNode->addChild(key[i], leaf);
                    newNode->addChild(key2[i], node);

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

                    
                    node = newNode;
                    return;
                }

                depth += node->prefixLen;
                Node* child = node->findChild(key[depth]);
                insert(child, key, leaf, depth + 1);
                node->addChild(key[depth], child);

            }
            

            int checkPrefix(Node* node, const std::string& key, int depth) {
                int len = std::min(node->prefixLen, (uint8_t)(key.size() - depth));
                for (int i = 0; i < len; ++i) {
                    if (node->prefix[i] != key[depth + i]) {
                        return i;
                    }
                }
                return len;
            }

    };
} // namespace ART

int main() {
    ART::ART tree;

    // Example of inserting leaf node
    ART::LeafNode* leaf1 = new ART::LeafNode("applee", "fruasdfit");
    // ART::Node* leaf1 = new ART::Node4();
    tree.root = leaf1;  // For simplicity, we use a leaf node as root

    // Perform search
    std::string* result = tree.search("applee");
    if (result != nullptr) {
        std::cout << "Found value: " << *result << std::endl;
    } else {
        std::cout << "Key not found" << std::endl;
    }

    return 0;
}