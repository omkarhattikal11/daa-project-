#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <cmath>
using namespace std;

// ------------------- GRAPH DEFINITION -------------------
class Graph {
public:
    int V;
    vector<vector<pair<int,int> > > adj;

    Graph(int V) {
        this->V = V;
        adj.resize(V);
    }

    void addEdge(int u, int v, int w) {
        adj[u].push_back(make_pair(v,w));
        adj[v].push_back(make_pair(u,w));
    }
};

// ------------------- DIJKSTRA (Scenario 1) -------------------
vector<int> dijkstra(Graph &g, int src) {
    vector<int> dist(g.V, INT_MAX);
    dist[src] = 0;

    priority_queue< pair<int,int>, vector<pair<int,int> >, greater<pair<int,int> > > pq;
    pq.push(make_pair(0, src));

    while(!pq.empty()) {
        int u = pq.top().second;
        pq.pop();

        for(size_t i=0;i<g.adj[u].size();i++) {
            int v = g.adj[u][i].first;
            int w = g.adj[u][i].second;

            if(dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push(make_pair(dist[v], v));
            }
        }
    }
    return dist;
}

// ------------------- A* (Scenario 3) -------------------
int heuristic(int a, int b) {
    return abs(a - b);
}

int Astar(Graph &g, int start, int goal) {
    vector<int> g_cost(g.V, INT_MAX);
    g_cost[start] = 0;

    priority_queue< pair<int,int>, vector<pair<int,int> >, greater<pair<int,int> > > open;
    open.push(make_pair(0, start));

    while(!open.empty()) {
        int u = open.top().second;
        open.pop();

        if(u == goal) return g_cost[u];

        for(size_t i=0;i<g.adj[u].size();i++) {
            int v = g.adj[u][i].first;
            int w = g.adj[u][i].second;

            int tentative = g_cost[u] + w;
            if(tentative < g_cost[v]) {
                g_cost[v] = tentative;
                int f = tentative + heuristic(v, goal);
                open.push(make_pair(f, v));
            }
        }
    }
    return -1;
}

// ------------------- GREEDY + TSP (Scenario 2) -------------------
int nearestStop(vector<vector<int> >& dist, int current, vector<bool>& visited) {
    int best = -1;

    for(size_t i = 0; i < dist.size(); i++) {
        if(!visited[i] &&
           (best == -1 || dist[current][i] < dist[current][best])) {
            best = i;
        }
    }
    return best;
}

vector<int> greedyRoute(vector<vector<int> >& dist) {
    int n = dist.size();
    vector<bool> visited(n, false);
    vector<int> path;

    int current = 0;
    visited[0] = true;
    path.push_back(0);

    for(int i = 1; i < n; i++) {
        int next = nearestStop(dist, current, visited);
        visited[next] = true;
        path.push_back(next);
        current = next;
    }
    return path;
}

// ------------------- MAIN -------------------
int main() {
    int V, E;
    cout << "Enter number of nodes in the graph: ";
    cin >> V;
    Graph g(V);

    cout << "Enter number of edges: ";
    cin >> E;
    cout << "Enter edges (u v w) with 0-based indexing:\n";
    for(int i=0;i<E;i++) {
        int u,v,w;
        cin >> u >> v >> w;
        g.addEdge(u,v,w);
    }

    cout << "\n--- Scenario 1: Peak Hour Optimization (Dijkstra) ---\n";
    int src;
    cout << "Enter source node: ";
    cin >> src;
    vector<int> d = dijkstra(g, src);
    for(size_t i=0;i<d.size();i++)
        cout << "Distance to " << i << " = " << d[i] << endl;

    cout << "\n--- Scenario 3: Emergency Route (A*) ---\n";
    int start, goal;
    cout << "Enter start and goal nodes for emergency route: ";
    cin >> start >> goal;
    cout << "Fastest path cost from " << start << " to " << goal << " = " << Astar(g,start,goal) << endl;

    cout << "\n--- Scenario 2: Bus Route Optimization (Greedy) ---\n";
    int n;
    cout << "Enter number of bus stops: ";
    cin >> n;
    vector<vector<int> > dist(n, vector<int>(n));
    cout << "Enter distance matrix:\n";
    for(int i=0;i<n;i++)
        for(int j=0;j<n;j++)
            cin >> dist[i][j];

    vector<int> route = greedyRoute(dist);
    cout << "Optimized Bus Route: ";
    for(size_t i = 0; i < route.size(); i++)
        cout << route[i] << " ";
    cout << endl;

    return 0;
}
