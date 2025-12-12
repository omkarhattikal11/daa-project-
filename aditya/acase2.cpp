#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <climits>
#include <string>
#include <algorithm>   // REQUIRED FOR max()
using namespace std;

// ---------------- GRAPH (Ambulance Routing) -------------------
class Graph {
public:
    int V;
    vector<vector<pair<int,int>>> adj;

    Graph(int V){
        this->V = V;
        adj.resize(V);
    }

    void addEdge(int u, int v, int w){
        adj[u].push_back({v,w});
        adj[v].push_back({u,w});
    }

    vector<int> dijkstra(int src){
        vector<int> dist(V, INT_MAX);
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;

        pq.push({0, src});
        dist[src] = 0;

        while(!pq.empty()){
            int u = pq.top().second;
            pq.pop();

            for(auto &p: adj[u]){
                int v = p.first;
                int w = p.second;

                if(dist[u] + w < dist[v]){
                    dist[v] = dist[u] + w;
                    pq.push({dist[v], v});
                }
            }
        }
        return dist;
    }
};

// -------- PRIORITY QUEUE (Patient Severity) --------
struct Patient {
    string name;
    int severity;
};

struct Compare {
    bool operator()(const Patient &a, const Patient &b){
        return a.severity < b.severity;
    }
};

// ---------------- AVL TREE (Bed Management) -------------------
class AVL {
public:
    int bedID, height;
    AVL *left, *right;

    AVL(int id){
        bedID = id;
        height = 1;
        left = right = nullptr;
    }
};

int height(AVL* N){
    return (N == nullptr) ? 0 : N->height;
}

int getBalance(AVL* N){
    return (N == nullptr) ? 0 : height(N->left) - height(N->right);
}

AVL* rightRotate(AVL* y){
    AVL* x = y->left;
    AVL* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

AVL* leftRotate(AVL* x){
    AVL* y = x->right;
    AVL* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

AVL* insertBed(AVL* node, int key){
    if (node == nullptr)
        return new AVL(key);

    if (key < node->bedID)
        node->left = insertBed(node->left, key);
    else if (key > node->bedID)
        node->right = insertBed(node->right, key);
    else
        return node;

    node->height = 1 + max(height(node->left), height(node->right));

    int balance = getBalance(node);

    if (balance > 1 && key < node->left->bedID)
        return rightRotate(node);

    if (balance < -1 && key > node->right->bedID)
        return leftRotate(node);

    if (balance > 1 && key > node->left->bedID){
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }

    if (balance < -1 && key < node->right->bedID){
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

void inorder(AVL* root){
    if (root != nullptr){
        inorder(root->left);
        cout << root->bedID << " ";
        inorder(root->right);
    }
}

// ---------- Medicine (Hash Table + Min-Heap) ----------- 
struct Medicine {
    string name;
    int expiryDays;
};

struct MinCompare {
    bool operator()(const Medicine &a, const Medicine &b){
        return a.expiryDays > b.expiryDays;
    }
};

unordered_map<string, int> stock;
priority_queue<Medicine, vector<Medicine>, MinCompare> expiryHeap;

// ---------------------- MAIN PROGRAM -------------------------------
int main(){

    cout << "\n===== HOSPITAL EMERGENCY RESOURCE MANAGEMENT SYSTEM =====\n\n";

    int V, E;
    cout << "Enter number of areas (nodes): ";
    cin >> V;

    Graph g(V);

    cout << "Enter number of roads: ";
    cin >> E;

    cout << "Enter roads in format: u v weight\n";
    for(int i=0;i<E;i++){
        int u,v,w;
        cin >> u >> v >> w;
        g.addEdge(u,v,w);
    }

    int src;
    cout << "\nEnter hospital node (source): ";
    cin >> src;

    auto dist = g.dijkstra(src);

    cout << "\n--- Ambulance Shortest Distance from Hospital ---\n";
    for(int i=0;i<V;i++)
        cout << "Area " << i << ": " << dist[i] << "\n";

    // -------- Patients PQ --------
    int P;
    cout << "\nEnter number of patients: ";
    cin >> P;

    priority_queue<Patient, vector<Patient>, Compare> pq;
    for(int i=0;i<P;i++){
        Patient p;
        cout << "Enter patient name & severity: ";
        cin >> p.name >> p.severity;
        pq.push(p);
    }

    cout << "\n--- Patient Treatment Order (High → Low severity) ---\n";
    while(!pq.empty()){
        auto p = pq.top(); pq.pop();
        cout << p.name << " (sev " << p.severity << ")\n";
    }

    // --------- Beds (AVL Tree) --------
    AVL* root = nullptr;
    int B;
    cout << "\nEnter number of beds: ";
    cin >> B;

    cout << "Enter bed IDs:\n";
    for(int i=0;i<B;i++){
        int id;
        cin >> id;
        root = insertBed(root, id);
    }

    cout << "\nAvailable Beds (Inorder Sorted): ";
    inorder(root);
    cout << "\n";

    // ---------- Medicines ----------
    int M;
    cout << "\nEnter number of medicines: ";
    cin >> M;

    for(int i=0;i<M;i++){
        Medicine m;
        cout << "Enter medicine name & expiry days: ";
        cin >> m.name >> m.expiryDays;

        stock[m.name]++;
        expiryHeap.push(m);
    }

    cout << "\nMost urgent medicine (min expiry): "
         << expiryHeap.top().name
         << " (" << expiryHeap.top().expiryDays << " days)\n";

    return 0;
}
