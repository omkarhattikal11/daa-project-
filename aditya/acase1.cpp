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
    Graph g(6);

    g.addEdge(0,1,4);
    g.addEdge(1,2,3);
    g.addEdge(2,3,6);
    g.addEdge(3,4,2);
    g.addEdge(4,5,5);
    g.addEdge(0,5,10);

    cout << "\n--- Scenario 1: Peak Hour Optimization (Dijkstra) ---\n";
    vector<int> d = dijkstra(g, 0);
    for(size_t i=0;i<d.size();i++)
        cout << "Distance to " << i << " = " << d[i] << endl;

    cout << "\n--- Scenario 3: Emergency Route (A*) ---\n";
    cout << "Fastest path cost from 0 to 5 = " << Astar(g,0,5) << endl;

    cout << "\n--- Scenario 2: Bus Route Optimization (Greedy) ---\n";

    int arr[5][5] = {
        {0,4,6,7,8},
        {4,0,2,5,7},
        {6,2,0,3,4},
        {7,5,3,0,2},
        {8,7,4,2,0}
    };

    vector<vector<int> > dist(5, vector<int>(5));
    for(int i=0;i<5;i++)
        for(int j=0;j<5;j++)
            dist[i][j] = arr[i][j];

    vector<int> route = greedyRoute(dist);

    cout << "Optimized Bus Route: ";
    for(size_t i = 0; i < route.size(); i++)
        cout << route[i] << " ";

    cout << endl;

    return 0;
}
