// DynamicPricingSystem.cpp
// High-complexity example for Shop & Restaurant Dynamic Pricing
// Includes many data structures & algorithms for educational/demo use.
//
// Compile with: g++ -std=c++17 DynamicPricingSystem.cpp -O2 -o dynamic_pricing

#include <bits/stdc++.h>
using namespace std;

/* =========================
   Basic Types and Structs
   ========================= */

struct Item {
    int id;
    string name;
    double basePrice;    // base price
    int stock;           // current stock
    int soldToday;       // units sold today (demand measure)
    int dailyViews;      // how many times viewed/asked (demand cue)
    int expiryDays;      // days until expiry (for perishables)
    bool perishable;
    int lastUpdateHour;  // for time-based pricing
    double currentPrice; // last computed dynamic price

    Item() = default;
    Item(int _id, string _name, double _base, int _stock, bool _perishable=false, int _expiry=30)
        : id(_id), name(_name), basePrice(_base), stock(_stock), soldToday(0), dailyViews(0),
          expiryDays(_expiry), perishable(_perishable), lastUpdateHour(12), currentPrice(_base) {}
};

struct Order {
    int orderId;
    int itemId;
    int qty;
    int timestampHour; // simplified timestamp (hour)
    string customer;
    Order() = default;
    Order(int oid, int iid, int q, int t, string c) : orderId(oid), itemId(iid), qty(q), timestampHour(t), customer(c) {}
};

struct SupplierEdge {
    int to;
    int timeCost; // e.g., minutes to deliver
    int risk;     // additional metric (not used heavily here)
};

using Graph = vector<vector<SupplierEdge>>;

/* =========================
   Utility Components
   ========================= */

// Simple Stack wrapper
template<typename T>
class SimpleStack {
    vector<T> data;
public:
    void push(const T &v) { data.push_back(v); }
    void pop() { if(!data.empty()) data.pop_back(); }
    T top() const { return data.back(); }
    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }
};

// Simple Queue wrapper
template<typename T>
class SimpleQueue {
    deque<T> data;
public:
    void push(const T &v) { data.push_back(v); }
    void pop() { if(!data.empty()) data.pop_front(); }
    T front() const { return data.front(); }
    bool empty() const { return data.empty(); }
    size_t size() const { return data.size(); }
};

/* =========================
   Sorting Algorithms
   ========================= */

void mergeSort(vector<Item> &arr, int l, int r, function<bool(const Item&, const Item&)> comp) {
    if (l >= r) return;
    int m = (l + r) / 2;
    mergeSort(arr, l, m, comp);
    mergeSort(arr, m+1, r, comp);
    vector<Item> temp;
    int i = l, j = m+1;
    while (i <= m && j <= r) {
        if (comp(arr[i], arr[j])) temp.push_back(arr[i++]);
        else temp.push_back(arr[j++]);
    }
    while (i <= m) temp.push_back(arr[i++]);
    while (j <= r) temp.push_back(arr[j++]);
    for (int k = l; k <= r; ++k) arr[k] = temp[k - l];
}

int partition_quick(vector<Item> &arr, int low, int high, function<bool(const Item&, const Item&)> comp) {
    Item pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (comp(arr[j], pivot)) {
            ++i;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i+1], arr[high]);
    return i+1;
}

void quickSort(vector<Item> &arr, int low, int high, function<bool(const Item&, const Item&)> comp) {
    if (low < high) {
        int pi = partition_quick(arr, low, high, comp);
        quickSort(arr, low, pi-1, comp);
        quickSort(arr, pi+1, high, comp);
    }
}

// heap sort using comparator (ascending if comp(a,b) is a < b)
void heapSort(vector<Item> &arr, function<bool(const Item&, const Item&)> comp) {
    // We'll use std::make_heap with custom comparator that matches comp
    // std::make_heap uses the comparator as "less" for max-heap; to sort by comp ascending, we invert comp accordingly.
    auto heapComp = [&](const Item &a, const Item &b){
        // want top of heap to be "largest" according to comp false; so invert:
        return comp(a, b) == false;
    };
    make_heap(arr.begin(), arr.end(), heapComp);
    for (int end = arr.size()-1; end > 0; --end) {
        pop_heap(arr.begin(), arr.begin()+end+1, heapComp);
        // after pop_heap, the largest is at arr[end], continue
    }
}

/* =========================
   Searching
   ========================= */

// Binary search on sorted vector by a key comparator; returns index or -1
int binarySearchIndex(const vector<Item> &arr, function<int(const Item&)> keyFunc, int target) {
    int l = 0, r = (int)arr.size() - 1;
    while (l <= r) {
        int m = (l + r) / 2;
        int key = keyFunc(arr[m]);
        if (key == target) return m;
        else if (key < target) l = m + 1;
        else r = m - 1;
    }
    return -1;
}

/* =========================
   BST and AVL Tree
   ========================= */

// Simple BST Node (used for demonstration)
struct BSTNode {
    Item val;
    BSTNode *left, *right;
    BSTNode(const Item &v) : val(v), left(nullptr), right(nullptr) {}
};

class BST {
    BSTNode *root;
    void insertNode(BSTNode *&node, const Item &val) {
        if (!node) { node = new BSTNode(val); return; }
        if (val.id < node->val.id) insertNode(node->left, val);
        else if (val.id > node->val.id) insertNode(node->right, val);
        else { node->val = val; } // update on equal id
    }
    void inorder(BSTNode *node, vector<Item> &out) {
        if (!node) return;
        inorder(node->left, out);
        out.push_back(node->val);
        inorder(node->right, out);
    }
public:
    BST(): root(nullptr) {}
    void insert(const Item &v) { insertNode(root, v); }
    vector<Item> inOrder() { vector<Item> out; inorder(root, out); return out; }
};

// AVL Tree node
struct AVLNode {
    Item val;
    AVLNode *left, *right;
    int height;
    AVLNode(const Item &v): val(v), left(nullptr), right(nullptr), height(1) {}
};

int getHeight(AVLNode* n) { return n ? n->height : 0; }
int getBalance(AVLNode* n) { return n ? getHeight(n->left) - getHeight(n->right) : 0; }
void updateHeight(AVLNode* n) { if(n) n->height = 1 + max(getHeight(n->left), getHeight(n->right)); }

AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateHeight(y); updateHeight(x);
    return x;
}

AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x); updateHeight(y);
    return y;
}

AVLNode* avlInsertNode(AVLNode* node, const Item &val) {
    if (!node) return new AVLNode(val);
    if (val.id < node->val.id) node->left = avlInsertNode(node->left, val);
    else if (val.id > node->val.id) node->right = avlInsertNode(node->right, val);
    else { node->val = val; return node; } // update

    updateHeight(node);
    int balance = getBalance(node);

    // LL
    if (balance > 1 && val.id < node->left->val.id) return rotateRight(node);
    // RR
    if (balance < -1 && val.id > node->right->val.id) return rotateLeft(node);
    // LR
    if (balance > 1 && val.id > node->left->val.id) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    // RL
    if (balance < -1 && val.id < node->right->val.id) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

void avlInorder(AVLNode* node, vector<Item> &out) {
    if (!node) return;
    avlInorder(node->left, out);
    out.push_back(node->val);
    avlInorder(node->right, out);
}

class AVLTree {
    AVLNode* root;
public:
    AVLTree(): root(nullptr) {}
    void insert(const Item &v) { root = avlInsertNode(root, v); }
    vector<Item> inOrder() { vector<Item> out; avlInorder(root, out); return out; }
};

/* =========================
   Heaps (Min & Max)
   ========================= */

// Max-heap of items by demand
struct DemandComparator {
    bool operator()(const Item &a, const Item &b) const {
        return a.soldToday < b.soldToday;
    }
};

// Min-heap by currentPrice
struct PriceMinComparator {
    bool operator()(const Item &a, const Item &b) const {
        return a.currentPrice > b.currentPrice; // invert for min-heap using priority_queue
    }
};

/* =========================
   Hash Table (fast lookup)
   ========================= */

class ItemCatalog {
    unordered_map<int, Item> idMap; // id -> item
    unordered_map<string, int> nameToId; // name -> id
public:
    void addOrUpdate(const Item &it) {
        idMap[it.id] = it;
        nameToId[it.name] = it.id;
    }
    bool existsId(int id) const { return idMap.find(id) != idMap.end(); }
    bool existsName(const string &name) const { return nameToId.find(name) != nameToId.end(); }
    Item* getById(int id) {
        if (idMap.find(id) == idMap.end()) return nullptr;
        return &idMap[id];
    }
    Item* getByName(const string &name) {
        if (nameToId.find(name) == nameToId.end()) return nullptr;
        int id = nameToId.at(name);
        return &idMap[id];
    }
    vector<Item> allItems() const {
        vector<Item> out; out.reserve(idMap.size());
        for (auto &p : idMap) out.push_back(p.second);
        return out;
    }
};

/* =========================
   Graph: Supplier Routing & Dijkstra
   ========================= */

vector<int> dijkstraShortest(const Graph &g, int src) {
    int n = g.size();
    const int INF = INT_MAX/4;
    vector<int> dist(n, INF);
    dist[src] = 0;
    using PI = pair<int,int>;
    priority_queue<PI, vector<PI>, greater<PI>> pq;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d != dist[u]) continue;
        for (const auto &e : g[u]) {
            int v = e.to;
            int nd = d + e.timeCost;
            if (nd < dist[v]) {
                dist[v] = nd;
                pq.push({nd, v});
            }
        }
    }
    return dist;
}

/* =========================
   Dynamic Pricing Engine
   ========================= */

// Pricing formula: base * (1 + alpha * demandIndex - beta * stockIndex + gamma * timeFactor - delta * expiryFactor)
double computeDynamicPrice(const Item &it, int currentHour) {
    double alpha = 0.02; // impact of soldToday
    double beta = 0.001; // impact of stock
    double gamma = 0.05; // time-of-day factor (peak times)
    double delta = it.perishable ? 0.05 : 0.0; // expiry sensitivity

    double demandIndex = min(200.0, (double)it.soldToday); // cap
    double stockIndex = max(1.0, (double)it.stock);

    // timeFactor: peak lunch/dinner increases price slightly
    double timeFactor = 0.0;
    if (currentHour >= 11 && currentHour <= 14) timeFactor = 0.10; // lunch peak
    else if (currentHour >= 19 && currentHour <= 22) timeFactor = 0.15; // dinner peak
    else if (currentHour >= 22 || currentHour <= 5) timeFactor = -0.12; // late-night discount

    // expiryFactor: if expiry is close, discount
    double expiryFactor = 0.0;
    if (it.perishable) {
        if (it.expiryDays <= 1) expiryFactor = 0.40; // heavy discount needed
        else expiryFactor = (double)max(0, 7 - it.expiryDays) * 0.05; // closer expiry -> bigger discount
    }

    double price = it.basePrice * (1.0 + alpha * demandIndex - beta * stockIndex + gamma * timeFactor - delta * expiryFactor);
    // ensure not negative
    if (price < 0.1) price = 0.1;
    return round(price * 100.0) / 100.0; // round to 2 decimals
}

/* =========================
   Order Processing Pipeline
   ========================= */

class OrderManager {
    SimpleQueue<Order> incoming;
    SimpleQueue<Order> processing;
    SimpleStack<Order> failed;
    int nextOrderId = 1;
public:
    int placeOrder(int itemId, int qty, int hour, string customer) {
        int oid = nextOrderId++;
        incoming.push(Order(oid, itemId, qty, hour, customer));
        return oid;
    }
    void moveToProcessing() {
        while (!incoming.empty()) {
            processing.push(incoming.front());
            incoming.pop();
        }
    }
    void processOrders(ItemCatalog &catalog, priority_queue<Item, vector<Item>, DemandComparator> &demandHeap) {
        while (!processing.empty()) {
            Order o = processing.front(); processing.pop();
            Item *it = catalog.getById(o.itemId);
            if (!it) {
                // failed: item not found
                failed.push(o);
                continue;
            }
            if (it->stock >= o.qty) {
                it->stock -= o.qty;
                it->soldToday += o.qty;
                // update demand heap: push updated copy (heap contains copies)
                demandHeap.push(*it);
                cout << "Order " << o.orderId << " fulfilled for item " << it->name << " qty " << o.qty << "\n";
            } else {
                cout << "Order " << o.orderId << " partial/failed for item " << it->name << " (stock " << it->stock << ")\n";
                // push failed (simulate backorder)
                failed.push(o);
            }
        }
    }
    void printFailed() {
        cout << "*** Failed / backorder list (LIFO) ***\n";
        SimpleStack<Order> temp;
        while (!failed.empty()) {
            Order o = failed.top();
            cout << "Order " << o.orderId << " item " << o.itemId << " qty " << o.qty << "\n";
            temp.push(o);
            failed.pop();
        }
        // restore
        while (!temp.empty()) { failed.push(temp.top()); temp.pop(); }
    }
};

/* =========================
   Reporting Utilities
   ========================= */

void printItem(const Item &it) {
    cout << "ID:" << it.id << " | " << it.name << " | Base: " << it.basePrice << " | Current: " << it.currentPrice
         << " | Stock: " << it.stock << " | SoldToday: " << it.soldToday << " | ExpiryDays: " << it.expiryDays << "\n";
}

/* =========================
   Demo / Main Program
   ========================= */

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "=== Shop & Restaurant Dynamic Pricing System (Option B Demonstration) ===\n\n";

    // Initialize catalog
    ItemCatalog catalog;
    vector<Item> seed = {
        Item(101, "Veg Burger", 100.0, 50, true, 3),
        Item(102, "Chicken Burger", 150.0, 40, true, 5),
        Item(103, "French Fries", 60.0, 30, true, 2),
        Item(104, "Margherita Pizza", 250.0, 20, true, 4),
        Item(105, "Coke", 40.0, 100, false, 365),
        Item(106, "Pasta", 180.0, 25, true, 6),
        Item(107, "Salad", 120.0, 15, true, 1)
    };
    for (auto &it : seed) {
        catalog.addOrUpdate(it);
    }

    // Build BST and AVL (for demo)
    BST bst; AVLTree avl;
    for (auto &it : seed) {
        bst.insert(it);
        avl.insert(it);
    }

    // Build initial heaps
    priority_queue<Item, vector<Item>, DemandComparator> demandHeap;
    priority_queue<Item, vector<Item>, PriceMinComparator> cheapHeap;

    // Add initial items to heaps
    auto all = catalog.allItems();
    for (auto &it : all) {
        demandHeap.push(it);
        cheapHeap.push(it);
    }

    // Graph of suppliers (0 = our store, 1..n suppliers)
    Graph suppliers;
    suppliers.resize(6);
    // add edges: undirected-like
    auto addEdge = [&](int u, int v, int timeCost){
        suppliers[u].push_back({v, timeCost, 0});
        suppliers[v].push_back({u, timeCost, 0});
    };
    addEdge(0,1,15); addEdge(1,2,20); addEdge(0,3,12); addEdge(3,4,25); addEdge(2,5,30);

    // Order manager
    OrderManager om;

    // Simulated clock hour
    int hour = 12;

    // Main simulation loop with menu
    bool running = true;
    int menuChoice = 0;
    while (running) {
        cout << "\n--- Menu ---\n"
             << "1. Show Catalog\n"
             << "2. Place Order (simulate)\n"
             << "3. Process Orders\n"
             << "4. Recompute Dynamic Prices\n"
             << "5. Show Top Demand Items\n"
             << "6. Replenish (find nearest supplier with Dijkstra)\n"
             << "7. Run Sorting Demo (Quick/Merge/Heap)\n"
             << "8. Print BST / AVL in-order\n"
             << "9. Show cheap items (min-heap peek)\n"
             << "10. Show failed orders\n"
             << "11. Advance hour\n"
             << "0. Exit\nChoice: ";
        cin >> menuChoice;

        switch (menuChoice) {
            case 1: {
                cout << "\n*** Catalog ***\n";
                auto items = catalog.allItems();
                for (auto &it : items) printItem(it);
                break;
            }
            case 2: {
                int iid, qty;
                string cust;
                cout << "Enter Item ID and Qty: ";
                cin >> iid >> qty;
                cout << "Customer name: ";
                cin >> cust;
                int oid = om.placeOrder(iid, qty, hour, cust);
                cout << "Order placed. OrderID: " << oid << "\n";
                break;
            }
            case 3: {
                // move incoming to processing and process
                om.moveToProcessing();
                om.processOrders(catalog, demandHeap);
                break;
            }
            case 4: {
                cout << "Recomputing dynamic prices for hour " << hour << "...\n";
                auto items = catalog.allItems();
                for (auto &it : items) {
                    Item *p = catalog.getById(it.id);
                    if (!p) continue;
                    p->currentPrice = computeDynamicPrice(*p, hour);
                    p->lastUpdateHour = hour;
                }
                cout << "Prices updated.\n";
                break;
            }
            case 5: {
                cout << "Top demand items (by soldToday) — peek 5:\n";
                // copy heap to display top 5
                auto tmp = demandHeap;
                int cnt = 0;
                while (!tmp.empty() && cnt < 5) {
                    Item it = tmp.top(); tmp.pop();
                    cout << it.name << " soldToday=" << it.soldToday << " currentPrice=" << it.currentPrice << "\n";
                    cnt++;
                }
                break;
            }
            case 6: {
                // simulate needing to replenish item with lowest stock
                auto items = catalog.allItems();
                sort(items.begin(), items.end(), [](const Item &a, const Item &b){ return a.stock < b.stock; });
                if (items.empty()) { cout << "No items.\n"; break; }
                Item low = items[0];
                cout << "Lowest stock item: " << low.name << " stock=" << low.stock << "\n";
                // find shortest supplier route from node 0 to any supplier node (we choose 1..n)
                auto dist = dijkstraShortest(suppliers, 0);
                int bestNode = -1, bestTime = INT_MAX;
                for (int i = 1; i < (int)dist.size(); ++i) {
                    if (dist[i] < bestTime) { bestTime = dist[i]; bestNode = i; }
                }
                if (bestNode == -1) cout << "No supplier reachable.\n";
                else cout << "Nearest supplier: Node " << bestNode << " time " << bestTime << " min.\n";
                // simulate replenishment
                Item *p = catalog.getById(low.id);
                if (p) {
                    p->stock += 50; // restock
                    cout << "Restocked " << p->name << " by 50 units. New stock=" << p->stock << "\n";
                }
                break;
            }
            case 7: {
                cout << "Sorting demo (by dynamic price descending):\n";
                auto items = catalog.allItems();
                // ensure prices aktual
                for (auto &it : items) {
                    Item *pp = catalog.getById(it.id);
                    if (pp) it.currentPrice = pp->currentPrice;
                }
                // quick sort
                auto arrQ = items;
                quickSort(arrQ, 0, (int)arrQ.size()-1, [](const Item &a, const Item &b){
                    return a.currentPrice > b.currentPrice; // descending
                });
                cout << "QuickSort top 3:\n";
                for (int i=0;i<min((int)arrQ.size(),3);++i) printItem(arrQ[i]);

                // merge sort
                auto arrM = items;
                if (!arrM.empty()) mergeSort(arrM, 0, (int)arrM.size()-1, [](const Item &a, const Item &b){
                    return a.currentPrice > b.currentPrice;
                });
                cout << "MergeSort top 3:\n";
                for (int i=0;i<min((int)arrM.size(),3);++i) printItem(arrM[i]);

                // heap sort
                auto arrH = items;
                heapSort(arrH, [](const Item &a, const Item &b){ return a.currentPrice > b.currentPrice; });
                cout << "HeapSort top 3:\n";
                for (int i=0;i<min((int)arrH.size(),3);++i) printItem(arrH[i]);
                break;
            }
            case 8: {
                cout << "BST in-order (by id):\n";
                auto bitems = bst.inOrder();
                for (auto &it : bitems) printItem(it);
                cout << "\nAVL in-order (by id):\n";
                auto aitems = avl.inOrder();
                for (auto &it : aitems) printItem(it);
                break;
            }
            case 9: {
                cout << "6 cheapest items (min-heap peek):\n";
                auto tmp = cheapHeap;
                for (int i=0;i<6 && !tmp.empty(); ++i) {
                    Item it = tmp.top(); tmp.pop();
                    cout << it.name << " price=" << it.currentPrice << " stock=" << it.stock << "\n";
                }
                break;
            }
            case 10: {
                om.printFailed();
                break;
            }
            case 11: {
                hour = (hour + 1) % 24;
                cout << "Advanced hour. Current hour = " << hour << "\n";
                break;
            }
            case 0: {
                running = false;
                break;
            }
            default:
                cout << "Invalid choice\n";
        }
    }

    cout << "\nExiting Dynamic Pricing System Demo.\n";
    return 0;
}

