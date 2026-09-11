//11220105 楊耀甯 11227203 謝采凌
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

//-----------------------------------------------------
// (A) 讀檔用結構: Data
//-----------------------------------------------------
struct Data {
    int inputOrder;   // 輸入順序 (1-based)
    string schoolCode;
    string schoolName;
    string department;
    string dayNight;
    string level;
    int studentNum;
    int teacherNum;
    int graduateNum;

    static string removeComma(const string &s) {
        string result;
        for (char c : s) {
            if (c >= '0' && c <= '9')
                result.push_back(c);
        }
        return result;
    }

    // 依照欄位順序，將資訊填入 Data 結構
    Data(const vector<string> &fields, int order) {
        inputOrder = order;
        schoolCode = fields[0];
        schoolName = fields[1];
        department = fields[3];
        dayNight   = fields[4];
        level      = fields[5];
        studentNum  = stoi(removeComma(fields[6]));
        teacherNum  = stoi(removeComma(fields[7]));
        graduateNum = stoi(removeComma(fields[8]));
    }
};

//-----------------------------------------------------
// 讀檔函式：讀取指定檔案，回傳 Data* 向量
//-----------------------------------------------------
vector<Data*> readDataFromFile(string &num) {


    ifstream fin("input" + num + ".txt");
    while (!fin.is_open()) {
        cout << endl << "### input" << num << ".txt does not exist! ###" << endl;
        cout << endl << "Input a file number ([0] Quit): ";
        cin >> num;
        if (num == "0") return {};
        fin.open("input" + num + ".txt");
    }
     // 假設前 3 行是表頭
    string line;
    for (int i = 0; i < 3; i++) getline(fin, line);

    vector<Data*> records;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        istringstream iss(line);
        vector<string> fields;
        string token;
        while (getline(iss, token, '\t')) {
            fields.push_back(token);
        }
        if (fields.size() >= 9) {
            Data* d = new Data(fields, records.size() + 1);
            records.push_back(d);
        }
    }
    fin.close();
    return records;
}

//-----------------------------------------------------
//  (B) 用於 2-3 Tree 的「Key = schoolName」結構
//-----------------------------------------------------
struct SchoolKey {
    string Name;  // 關鍵字：學校or科系名稱
    vector<Data*> records;     // 該學校多筆 Data*

    SchoolKey(const string &s) : Name(s) {}
};

//-----------------------------------------------------
//  (C) 2-3 Node: 節點可能有 1 or 2 個 SchoolKey
//-----------------------------------------------------
class TwoThreeNode {
public:
    SchoolKey* key1;    // 第 1 組 key
    SchoolKey* key2;    // 第 2 組 key (若為 3-node 才用)
    int nKeys;  // 目前存了多少個 key (1 or 2)

    TwoThreeNode* child0;   //子節點
    TwoThreeNode* child1;
    TwoThreeNode* child2;
    TwoThreeNode* parent;

    TwoThreeNode(SchoolKey* &k) {
        key1 = k;
        key2 = nullptr;
        nKeys  = 1;
        child0 = child1 = child2 = nullptr;
        parent = nullptr;
    }
};

//-----------------------------------------------------
// (D) 2-3 Tree：以「學校名稱」為排序依據
//-----------------------------------------------------
class TwoThreeTree {
private:
    TwoThreeNode *root;
    int calcHeight(TwoThreeNode* node) const {
        // tree is empty
        if (!node) return 0;

        // tree has only root
        if (!node->child0 && !node->child1 && !node->child2) return 1;


        // tree has children
        int maxChildHeight = 0;
        if (node->child0)
            maxChildHeight = calcHeight(node->child0);
        if (node->child1)
            maxChildHeight = max(maxChildHeight, calcHeight(node->child1));
        if (node->child2)
            maxChildHeight = max(maxChildHeight, calcHeight(node->child2)); 
        return maxChildHeight + 1;
    }
public:
    TwoThreeTree() : root(nullptr) {}
    ~TwoThreeTree() {
        // delete all nodes
        
    }
    void clearTree() {
        delete root;
        root = nullptr;
    }
    TwoThreeNode* getRoot() const { return root; }

    // 尋找是否已存在某 schoolName
    TwoThreeNode* findNode(const string &sch) {
        TwoThreeNode *cur = root;
        while (cur) {
            if (cur->nKeys == 1) { //2node
                if (cur->key1->Name == sch) {
                    return cur;
                }

                if (sch < cur->key1->Name)  cur = cur->child0; //L
                else  cur = cur->child1; //R
            }
            else {
                // 3node
                if (cur->key1->Name == sch) return cur;
                if (cur->key2->Name == sch) return cur;

                if (sch < cur->key1->Name) {
                    cur = cur->child0;  //L
                } else if (sch < cur->key2->Name) {
                    cur = cur->child1;  //R
                } else {
                    cur = cur->child2;  //Mid
                }
            }
        }
        return nullptr;
    }

    // 找葉節點
    TwoThreeNode* findLeaf(const string &sch) {
        TwoThreeNode* cur = root;
        while (cur) {
            if (!cur->child0 && !cur->child1 && !cur->child2) {
                return cur;
            }
            if (cur->nKeys == 1) {  //2 child node
                if (sch < cur->key1->Name)  cur = cur->child0; //L
                else  cur = cur->child1; //R
            }
            else {  //3 child node
                if (sch < cur->key1->Name) { //比左邊還小
                    cur = cur->child0; //L
                } else if (sch > cur->key2->Name) { //比右邊還大
                    cur = cur->child2; //R
                } else {
                    cur = cur->child1; //Mid
                }
            }
        }
        cout << "error";
        return nullptr;
    }
//-----------------------------------------------------
// (E) insertKey: 在葉節點插入
//     沒有相同的資料
//-----------------------------------------------------
    // 插入一個新的 SchoolKey 到葉節點
    void insertKey(TwoThreeNode* node, SchoolKey* newKey);

//-----------------------------------------------------
// (F) splitNode: 將 midKey 合併到 parent，若 parent 超載則分裂
//-----------------------------------------------------
    // 分裂：若節點已裝 3 個 key，將中間 key 往上推
    void splitNode(TwoThreeNode* node, SchoolKey* midKey,
                   TwoThreeNode* leftChild, TwoThreeNode* rightChild);

    //插入一筆 Data*
    void insert(Data* d) {
         // 如果樹是空的，直接建立 root
        if (!root) { //first key
            SchoolKey *sk = new SchoolKey(d->schoolName);
            sk->records.push_back(d);
            root = new TwoThreeNode(sk);
            return;
        }

        // 檢查該學校是否已存在
        TwoThreeNode* found = findNode(d->schoolName);
        if (found) {
            // 已存在 => 放入該 key 的 records
            if (found->nKeys == 1) {
                found->key1->records.push_back(d);
            } else {
                // 節點有 2 keys
                if (found->key1->Name == d->schoolName) {
                    found->key1->records.push_back(d);
                } else {
                    found->key2->records.push_back(d);
                }
            }
            return;
        }

        // 在葉節點插入
        TwoThreeNode* leaf = findLeaf(d->schoolName);
        SchoolKey *newKey = new SchoolKey(d->schoolName);
        newKey->records.push_back(d);
        insertKey(leaf, newKey);

    }

    // 計算樹高
    int getHeight() const {
        return calcHeight(root);
    }

};

class AVLNode {
public:
    SchoolKey *key;  
    AVLNode *left;  // left subtree
    AVLNode *right;  // right subtree
    // AVLNode *parent;
    AVLNode(SchoolKey *k) {
        key = k;
        left = right = nullptr;
        // parent = nullptr;
    }
};
class AVLTree {
private:
    AVLNode *root;
    AVLNode* insert(Data *d, AVLNode *&cur) {

        // if current node is empty, create a new root
        if (!cur) {
            SchoolKey *sk = new SchoolKey(d->department);
            sk->records.push_back(d);
            cur = new AVLNode(sk);

            return cur;
        }
        // check if the school already at this node
        if (cur->key->Name == d->department) {
            cur->key->records.push_back(d);
            return cur;
        }

        if (cur->key->Name > d->department) {
            cur->left = insert(d, cur->left);
            //cur->left->parent = cur;
        } else if (cur->key->Name < d->department) {
            cur->right = insert(d, cur->right);
            // cur->right->parent = cur;
        }
        bool needRotate = (abs(calHeight(cur->left) - calHeight(cur->right)) > 1);
        if (needRotate) {
            // case1: LL (left-left)
            // case2: LR (left-right)
            if (calHeight(cur->left) > calHeight(cur->right)) {
                
                // if case2(LR)
                if (calHeight(cur->left->right) > calHeight(cur->left->left))
                    rotateLeft(cur->left);
                rotateRight(cur);
            }

            // case3: RL (right-left)
            // case4: RR (right-right)
            else {

                // if case3(RL)
                if (calHeight(cur->right->left) > calHeight(cur->right->right))
                    rotateRight(cur->right);
                rotateLeft(cur);
            }


        }

        return cur;
    }
    int calHeight(AVLNode *cur) const {
        if (!cur) return 0;
        int leftHeight = calHeight(cur->left);
        int rightHeight = calHeight(cur->right);
        return max(leftHeight, rightHeight) + 1;
    }
    void rotateLeft(AVLNode *&cur) {    
        // make sure cur and cur->right are not nullptr
        if (!cur || !cur->right) return;

        AVLNode *newRoot = cur->right;
        cur->right = newRoot->left;
        //if (cur->parent) newRoot->parent = cur->parent;
        //cur->parent = newRoot;
        //if (newRoot->left) newRoot->left->parent = cur; 
        newRoot->left = cur;
        cur = newRoot;
        return;
    }
    void rotateRight(AVLNode *&cur) {

        // make sure cur and cur->left are not nullptr
        if (!cur || !cur->left) return;

        AVLNode *newRoot = cur->left;
        cur->left = newRoot->right;
        //if (cur->parent) newRoot->parent = cur->parent;
        //cur->parent = newRoot;
        //if (newRoot->right) newRoot->right->parent = cur; 
        newRoot->right = cur;
        cur = newRoot;
        return;

    }
public:
    AVLTree() {
        root = nullptr;
    }
    ~AVLTree() {
        // delete all nodes
    }
    AVLNode* findNode(AVLNode* cur, string schoolName) {

        if (!cur) return nullptr;
        if (cur->key->Name == schoolName) return cur;
        else if (schoolName < cur->key->Name) {
            return findNode(cur->left, schoolName);
        } else if (schoolName > cur->key->Name) {
            return findNode(cur->right, schoolName);
        }

    }
    void clearTree() {
        delete root;
        root = nullptr;

    }
    void insertAll(vector<Data*> &dataset) {
        for (Data *&d : dataset) {
            root = insert(d, root);
        }
    }
    int getHeight() const {
        return calHeight(root);
    }
    AVLNode* getRoot() const { return root; }
};

int main() {
    TwoThreeTree tree;
    AVLTree avl;
    string choice = "-1";
    vector<Data*> dataset;
    bool first = false;
    bool second = false;
    
    while (choice != "0") {

        cout << endl;
        cout << "*** Search Tree Utilities **" << endl;
        cout << "* 0. QUIT                  *" << endl;
        cout << "* 1. Build 2-3 tree        *" << endl;
        cout << "* 2. Build AVL tree        *" << endl;
        cout << "* 3. Intersection Query    *" << endl;
        cout << "*************************************" << endl;
        cout << "Input a choice(0, 1, 2, 3): ";
    
        cin >> choice;
        if(choice == "1") {

            
            cout << "\nInput a file number ([0] Quit): ";
            string num;
            cin >> num;
            if (num == "0") {
                cout << endl;
                continue;
            }

            

            
            dataset = readDataFromFile(num);
            if (dataset.empty()) {
                first = false;
                continue;
            }
            if (num == "0") {
                cout << endl;
                continue;
            }

            first = true;
            second = false;
            tree.clearTree();
            avl.clearTree();
            
            for (Data* &d : dataset) {
                tree.insert(d);
            }


            TwoThreeNode* rootNode = tree.getRoot();
            if (!rootNode) continue;

            // print tree height
            cout << "Tree height = " << tree.getHeight() << endl;

            vector<Data*> rootRecords;
            if (rootNode->key1) {
                for (Data* d : rootNode->key1->records) {
                    rootRecords.push_back(d);
                }
            }
            if (rootNode->nKeys == 2 && rootNode->key2) {
                for (Data* d : rootNode->key2->records) {
                    rootRecords.push_back(d);
                }
            }

            for (int i = 0; i < rootRecords.size() - 1; i++) {
                for (int j = i + 1; j < rootRecords.size(); j++) {
                    if (rootRecords[j]->inputOrder < rootRecords[i]->inputOrder) {
                        swap(rootRecords[i], rootRecords[j]);
                    }
                }
            }

            int order = 1;

            // print root node
            for (Data* &d : rootRecords) {
                cout << order << ": "
                    <<"["<< d->inputOrder << "] "
                    << d->schoolName  << ", "
                    << d->department  << ", "
                    << d->dayNight    << ", "
                    << d->level       << ", "
                    << d->studentNum  << "\n";
                    order++;
            }
            cout << endl;
        } else if (choice == "2") {
            second = true;
            if (!first) {
               cout << "### Choose 1 first. ###" << endl;
               continue;
            }

            if (avl.getRoot()) cout << "### AVL tree has been built. ###" << endl;

            avl.clearTree();

            avl.insertAll(dataset);

            // print tree height
            cout << "Tree height = " << avl.getHeight() << endl;
            int order = 1;
            for (int i = 0; i < avl.getRoot()->key->records.size(); i++) {
                cout << order << ": "
                    << "[" << avl.getRoot()->key->records[i]->inputOrder << "] "
                    << avl.getRoot()->key->records[i]->schoolName << ", "
                    << avl.getRoot()->key->records[i]->department << ", "
                    << avl.getRoot()->key->records[i]->dayNight << ", "
                    << avl.getRoot()->key->records[i]->level << ", "
                    << avl.getRoot()->key->records[i]->studentNum << endl;
                order++;
            }
            cout << endl;
        } else if (choice == "3") {
            if (!first) {
                cout << "### Choose 1 first. ###" << endl;
                continue;
            }
            if (!second) {
                cout << "### Choose 2 first. ###" << endl;
                continue;
            }
            string collegeName;
            string departmentName;
            cout << "Enter a college name to search [*]: ";
            cin >> collegeName;
            cout << "Enter a department name to search [*]: ";
            cin >> departmentName;

            int order = 1;
            if (collegeName == "*" && departmentName == "*") {
                for (Data* d : dataset) {
                    cout << order << ": "
                         << "[" << d->inputOrder << "] " 
                         << d->schoolName << ", " 
                         << d->department << ", "
                         << d->dayNight << ", "
                         << d->level << ", "
                         << d->studentNum << endl;
                    order++;
                }
                cout << endl;
                continue;
            } 

            vector<Data*> twoThreeNodeData;
            vector<Data*> avlNodeData;
            TwoThreeNode* node23;
            AVLNode* avlNode;

            // load 2-3 tree's node data to vector
            node23 = tree.findNode(collegeName);
            if (node23) {
                if (node23->key1->Name == collegeName) {
                    for (Data* d : node23->key1->records) {
                        twoThreeNodeData.push_back(d);
                    }
                } else if (node23->key2->Name == collegeName) {
                    for (Data* d : node23->key2->records) {
                        twoThreeNodeData.push_back(d);
                    }
                }
            }

            // load avl tree's node data to vector
            avlNode = avl.findNode(avl.getRoot(), departmentName);
            if (avlNode) {
                for (Data* d : avlNode->key->records) {
                    avlNodeData.push_back(d);
                }
            }

            vector<Data*> intersection;

            if (departmentName == "*") {
                for (Data* d1 : twoThreeNodeData) {
                    for (Data* d2 : dataset) {
                        if (d1 == d2) {  
                            intersection.push_back(d1);
                            break;
                        }
                    }
                }
            } else if (collegeName == "*") {
                for (Data* d1 : dataset) {
                    for (Data* d2 : avlNodeData) {
                        if (d1 == d2) {  
                            intersection.push_back(d1);
                            break;
                        }
                    }
                }
            } else {
                for (Data* d1 : twoThreeNodeData) {
                    for (Data* d2 : avlNodeData) {
                        if (d1 == d2) {  
                            intersection.push_back(d1);
                            break;
                        }
                    }
                }
            }
            //--------------------------------------------------------------------------
            // sort intersection by inputOrder
            for (int i = 0; i < intersection.size() - 1; i++) {
                for (int j = i + 1; j < intersection.size(); j++) {
                    if (intersection[j]->inputOrder < intersection[i]->inputOrder) {
                        swap(intersection[i], intersection[j]);
                    }
                }
            }
            //--------------------------------------------------------------------------           
            for (Data* d : intersection) {
                cout << order << ": "
                     << "[" << d->inputOrder << "] " 
                     << d->schoolName << ", "                          
                     << d->department << ", "
                     << d->dayNight << ", "
                     << d->level << ", "
                     << d->studentNum << endl;
                    order++;
            }
            cout << endl;
            

        } else if (choice != "0") {
            cout << "\nCommand does not exist!\n";
        }

    }
    return 0;
}

void TwoThreeTree::insertKey(TwoThreeNode* leafnode, SchoolKey* newKey) {

    vector<SchoolKey*> temporary;
    temporary.push_back(leafnode->key1);  //key1
    if (leafnode->nKeys == 2) {
        temporary.push_back(leafnode->key2);  //key2
    }
    temporary.push_back(newKey);  //become key2? key3?

    // bubble sort
    for (int i = 0; i < temporary.size(); i++) {
        for (int j = i + 1; j < temporary.size(); j++) {
            if (temporary[j]->Name < temporary[i]->Name) {
                swap(temporary[i], temporary[j]);
            }
        }
    }


     // 若插入後 key 數量為 2，不超載
    if (temporary.size() == 2) {      //case1
        leafnode->key1 = temporary[0];
        leafnode->key2 = temporary[1];
        leafnode->nKeys = 2;
    } else {
         // 否則有 3 個 key，需分裂
        SchoolKey* leftKey  = temporary[0]; //L
        SchoolKey* midKey   = temporary[1]; //Mid 往上
        SchoolKey* rightKey = temporary[2]; //R

        // leafnode 變成裝 leftKey
        // 變成新左節點
        leafnode->key1 = leftKey;
        leafnode->key2 = nullptr;
        leafnode->nKeys = 1;

        // 建立新節點裝 rightKey
        // 變成新右節點
        TwoThreeNode* newRight = new TwoThreeNode(rightKey);
        newRight->parent = leafnode->parent;
        

        // 將 midKey 往上推到 leafnode->parent
        splitNode(leafnode->parent, midKey, leafnode, newRight);
    }
}

void TwoThreeTree::splitNode(TwoThreeNode* parent, SchoolKey* midKey, TwoThreeNode* leftChild, TwoThreeNode* rightChild) {
    // case1: parent is NULL
    if (!parent) {
        // 說明 node 是 root，需新建一個更高層的 root
        TwoThreeNode* newRoot = new TwoThreeNode(midKey);
        newRoot->child0 = leftChild;
        newRoot->child1 = rightChild;
        // 更新parent
        leftChild->parent = newRoot;
        rightChild->parent = newRoot;
        root = newRoot;  // 將 root 指向新 root
        return;
    }
    // collect all keys that need to be sorted
    vector<SchoolKey*> parentkey;
    parentkey.push_back(parent->key1);
    if (parent->nKeys == 2) {
        parentkey.push_back(parent->key2);
    }
    parentkey.push_back(midKey);

    // bubble sort
    for (int i = 0; i < parentkey.size(); i++) {
        for (int j = i + 1; j < parentkey.size(); j++) {
            if (parentkey[j]->Name < parentkey[i]->Name) {
                swap(parentkey[i], parentkey[j]);
            }
        }
    }

    // case2: parent is 2-node
    if (parent->nKeys == 1) { 
        parent->key1 = parentkey[0];
        parent->key2 = parentkey[1]; //NEW key
        parent->nKeys = 2;

        vector<TwoThreeNode*> newchild;
        newchild.push_back(parent->child0);
        newchild.push_back(parent->child1);
        newchild.push_back(rightChild);


        
        for (int i = 0; i < newchild.size(); i++) {
            for (int j = i + 1; j < newchild.size(); j++) {
                if (newchild[j]->key1->Name < newchild[i]->key1->Name) {
                    swap(newchild[i], newchild[j]);
                }
            }
        }
        
        // reassign sorted children
        parent->child0 = newchild[0]; //L
        parent->child1 = newchild[1]; //Mid
        parent->child2 = newchild[2]; //R

        for (TwoThreeNode* child : newchild) {
            if (child) child->parent = parent;
        }

    }
    // case3: parent is 3-node 
    else {  
        // Distribute keys
        SchoolKey* leftK  = parentkey[0];
        SchoolKey* midK   = parentkey[1]; // spilt
        SchoolKey* rightK = parentkey[2];

        // Update current parent node
        parent->key1  = leftK;
        parent->key2  = nullptr;
        parent->nKeys = 1;

        // create new parent node for rightK
        TwoThreeNode* newParent = new TwoThreeNode(rightK);
        // newParent's parent: original parent's parent(grandparent)
        newParent->parent = parent->parent;




        // 子節點->parent重設
        vector<TwoThreeNode*> child;
        child.push_back(parent->child0);
        child.push_back(parent->child1);
        child.push_back(parent->child2);
        child.push_back(rightChild);
        parent->child0 =  nullptr;
        parent->child1 =  nullptr;
        parent->child2 =  nullptr;

        for (int i = 0; i < child.size(); i++){
            for (int j = i + 1; j < child.size(); j++){
                if (child[j]->key1->Name < child[i]->key1->Name) {
                    swap(child[i], child[j]);
                }
            }
        }


        // distribute children
        parent->child0 = child[0];
        parent->child1 = child[1];
        parent->child2 = nullptr;
        parent->nKeys  = 1;

        newParent->child0 = child[2];
        newParent->child1 = child[3];
        newParent->child2 = nullptr;
        newParent->nKeys  = 1;

        child[0]->parent = parent;
        child[1]->parent = parent;
        child[2]->parent = newParent;
        child[3]->parent = newParent;

        // 將 midK 往上推到 parent->parent
        splitNode(parent->parent, midK, parent, newParent);
    }
}
