#include <bits/stdc++.h>
using namespace std;

#define INF 1e9

// --------------------- Structures ---------------------
struct Edge {
    int to, weight;  // Graph edge: destination node, weight (time/distance)
};

struct Vehicle {
    string id;
    string route;
    int currentLocation; // node index
    int nextStop;        // node index
    int ETA;             // in minutes
    int occupancy;       // %
};

// --------------------- Graph Algorithms ---------------------

// Dijkstra: Shortest Path (Time Complexity: O(E log V))
vector<int> dijkstra(int src, vector<vector<Edge>> &graph) {
    int n = graph.size();
    vector<int> dist(n, INF);
    dist[src] = 0;

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    pq.push({0, src});

    while(!pq.empty()) {
        int d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if(d > dist[u]) continue;

        for(auto &edge : graph[u]) {
            int v = edge.to, w = edge.weight;
            if(dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

// Bellman-Ford: Shortest Path with negative edges (O(VE))
vector<int> bellmanFord(int src, vector<vector<Edge>> &graph) {
    int n = graph.size();
    vector<int> dist(n, INF);
    dist[src] = 0;

    for(int i=0;i<n-1;i++){
        for(int u=0;u<n;u++){
            for(auto &edge : graph[u]){
                int v=edge.to,w=edge.weight;
                if(dist[u]!=INF && dist[u]+w < dist[v])
                    dist[v] = dist[u]+w;
            }
        }
    }

    // Negative cycle detection
    for(int u=0;u<n;u++){
        for(auto &edge: graph[u]){
            int v=edge.to,w=edge.weight;
            if(dist[u]!=INF && dist[u]+w<dist[v])
                cout<<"Warning: Negative cycle detected!\n";
        }
    }

    return dist;
}

// Floyd-Warshall: All-pairs shortest path (O(V^3))
vector<vector<int>> floydWarshall(vector<vector<int>> &graphMatrix) {
    int n = graphMatrix.size();
    vector<vector<int>> dist = graphMatrix;
    for(int k=0;k<n;k++){
        for(int i=0;i<n;i++){
            for(int j=0;j<n;j++){
                if(dist[i][k]!=INF && dist[k][j]!=INF)
                    dist[i][j] = min(dist[i][j], dist[i][k]+dist[k][j]);
            }
        }
    }
    return dist;
}

// Kruskal MST: Optimize route network (O(E log E))
struct UF {
    vector<int> parent, rank;
    UF(int n){ parent.resize(n); rank.resize(n,0); iota(parent.begin(),parent.end(),0);}
    int find(int x){ return parent[x]==x ? x : parent[x]=find(parent[x]); }
    void unite(int x,int y){
        x=find(x); y=find(y);
        if(x==y) return;
        if(rank[x]<rank[y]) parent[x]=y;
        else { parent[y]=x; if(rank[x]==rank[y]) rank[x]++; }
    }
};

int kruskalMST(vector<tuple<int,int,int>> &edges, int n){
    sort(edges.begin(),edges.end());
    UF uf(n);
    int cost=0;
    for(auto &[w,u,v]: edges){
        if(uf.find(u)!=uf.find(v)){
            uf.unite(u,v);
            cost+=w;
        }
    }
    return cost;
}

// --------------------- Trie for Route Lookup ---------------------
struct TrieNode{
    unordered_map<char,TrieNode*> children;
    bool isEnd=false;
};

class Trie{
    TrieNode *root;
public:
    Trie(){ root=new TrieNode(); }
    void insert(string s){
        TrieNode *node=root;
        for(char c:s){
            if(!node->children[c]) node->children[c]=new TrieNode();
            node=node->children[c];
        }
        node->isEnd=true;
    }

    bool search(string s){
        TrieNode *node=root;
        for(char c:s){
            if(!node->children[c]) return false;
            node=node->children[c];
        }
        return node->isEnd;
    }

    void autocomplete(string prefix){
        TrieNode *node=root;
        for(char c:prefix){
            if(!node->children[c]) return;
            node=node->children[c];
        }
        vector<string> results;
        function<void(TrieNode*, string)> dfs=[&](TrieNode* n, string s){
            if(n->isEnd) results.push_back(prefix+s);
            for(auto &[ch,child]: n->children) dfs(child, s+ch);
        };
        dfs(node,"");
        for(auto &r: results) cout<<r<<" ";
        cout<<endl;
    }
};

// --------------------- Main Program ---------------------
int main() {
    int n=6; // Number of junctions/nodes
    vector<vector<Edge>> graph(n);

    // Sample road network
    graph[0].push_back({1,7});
    graph[0].push_back({2,9});
    graph[1].push_back({3,15});
    graph[2].push_back({3,11});
    graph[2].push_back({5,2});
    graph[3].push_back({4,6});
    graph[5].push_back({4,9});

    // ----------------- Shortest Path Algorithms -----------------
    cout<<"--- Dijkstra ---\n";
    vector<int> dist = dijkstra(0, graph);
    for(int i=0;i<n;i++) cout<<"Node "<<i<<": "<<dist[i]<<"\n";

    cout<<"\n--- Bellman-Ford ---\n";
    dist = bellmanFord(0, graph);
    for(int i=0;i<n;i++) cout<<"Node "<<i<<": "<<dist[i]<<"\n";

    cout<<"\n--- Floyd-Warshall ---\n";
    vector<vector<int>> matrix(n, vector<int>(n,INF));
    for(int i=0;i<n;i++) matrix[i][i]=0;
    for(int u=0;u<n;u++){
        for(auto &e:graph[u]) matrix[u][e.to]=e.weight;
    }
    auto fw = floydWarshall(matrix);
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++)
            cout<<fw[i][j]<<" ";
        cout<<"\n";
    }

    // ----------------- Route Optimization (Kruskal MST) -----------------
    cout<<"\n--- Kruskal MST Cost ---\n";
    vector<tuple<int,int,int>> edges={{7,0,1},{9,0,2},{15,1,3},{11,2,3},{2,2,5},{6,3,4},{9,5,4}};
    int mstCost = kruskalMST(edges,n);
    cout<<"MST Total Cost: "<<mstCost<<"\n";

    // ----------------- Route Lookup & Autocomplete -----------------
    cout<<"\n--- Trie Autocomplete ---\n";
    Trie t;
    t.insert("A->B->C");
    t.insert("A->B->D");
    t.insert("A->C->D");
    cout<<"Autocomplete for A->B->: ";
    t.autocomplete("A->B->");

    // ----------------- Sample Vehicle Info -----------------
    vector<Vehicle> vehicles = {
        {"Bus101","A->B->C",0,1,3,75},
        {"Bus102","B->C->D",1,2,5,50},
        {"Train1","X->Y->Z",2,3,7,90}
    };

    cout<<"\n--- Vehicle Info ---\n";
    for(auto &v: vehicles){
        cout<<"ID: "<<v.id<<", Route: "<<v.route
            <<", Current Node: "<<v.currentLocation
            <<", Next Node: "<<v.nextStop
            <<", ETA: "<<v.ETA<<" min"
            <<", Occupancy: "<<v.occupancy<<"%\n";
    }

    return 0;
}

