#include <bits/stdc++.h>
using namespace std;

/*==========================================
            KMP PATTERN SEARCH
===========================================*/
vector<int> buildLPS(string pat) {
    int n = pat.size();
    vector<int> lps(n, 0);
    int len = 0, i = 1;

    while (i < n) {
        if (pat[i] == pat[len]) {
            lps[i++] = ++len;
        } else {
            if (len != 0) len = lps[len - 1];
            else lps[i++] = 0;
        }
    }
    return lps;
}

bool KMP_Search(string text, string pat) {
    vector<int> lps = buildLPS(pat);
    int i = 0, j = 0;

    while (i < text.size()) {
        if (text[i] == pat[j]) { i++; j++; }
        if (j == pat.size()) return true;
        else if (i < text.size() && text[i] != pat[j]) {
            if (j != 0) j = lps[j - 1];
            else i++;
        }
    }
    return false;
}

/*==========================================
            HASH TABLE FOR EMERGENCY TYPE
===========================================*/
unordered_map<string, int> emergencyLevel = {
    {"FIRE", 10},
    {"EARTHQUAKE", 9},
    {"FLOOD", 8},
    {"MEDICAL", 7},
    {"ACCIDENT", 6}
};

/*==========================================
          PRIORITY QUEUE (MAX-HEAP)
===========================================*/
struct Alert {
    int severity;
    string message;
};

struct Compare {
    bool operator()(Alert const &a, Alert const &b) {
        return a.severity < b.severity; // Max-Heap
    }
};

priority_queue<Alert, vector<Alert>, Compare> alertHeap;

/*==========================================
         BFS SAFE ZONE CHECK
===========================================*/
bool bfsSafeZone(vector<vector<int>> &graph, int start, int safeZone) {
    queue<int> q;
    vector<bool> visited(graph.size(), false);

    q.push(start);
    visited[start] = true;

    while (!q.empty()) {
        int node = q.front(); q.pop();
        if (node == safeZone) return true;

        for (int nxt : graph[node]) {
            if (!visited[nxt]) {
                visited[nxt] = true;
                q.push(nxt);
            }
        }
    }
    return false;
}

/*==========================================
        DIJKSTRA SHORTEST EVACUATION PATH
===========================================*/
vector<int> dijkstra(vector<vector<pair<int,int>>> &graph, int src) {
    int n = graph.size();
    vector<int> dist(n, INT_MAX);
    dist[src] = 0;

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, node] = pq.top(); pq.pop();

        if (d > dist[node]) continue;

        for (auto &edge : graph[node]) {
            int nxt = edge.first;
            int w = edge.second;

            if (dist[nxt] > dist[node] + w) {
                dist[nxt] = dist[node] + w;
                pq.push({dist[nxt], nxt});
            }
        }
    }
    return dist;
}

/*==========================================
                 MAIN SYSTEM
===========================================*/
int main() {
    cout << "\n=== EMERGENCY ALERT SYSTEM ===\n";

    /* -------- Step 1: Detect Emergency Keyword -------- */
    string logText = "Smoke detected! Possible FIRE in Block A.";
    vector<string> patterns = {"FIRE", "EARTHQUAKE", "FLOOD", "MEDICAL"};

    for (string p : patterns) {
        if (KMP_Search(logText, p)) {
            alertHeap.push({ emergencyLevel[p], "Emergency Detected: " + p });
        }
    }

    /* -------- Step 2: BFS — Check Safe Zone Reachability -------- */
    vector<vector<int>> safeGraph = {
        {1},      // 0 → 1
        {2},      // 1 → 2
        {3},      // 2 → 3 (safe zone)
        {}        // 3
    };

    if (bfsSafeZone(safeGraph, 0, 3)) {
        alertHeap.push({6, "Safe zone reachable from current position."});
    }

    /* -------- Step 3: Dijkstra — Fastest Evacuation Path -------- */
    vector<vector<pair<int,int>>> evacGraph = {
        {{1, 4}},         // 0 → 1 (time 4)
        {{2, 3}},         // 1 → 2 (time 3)
        {{3, 2}},         // 2 → 3 (time 2)
        {}                // 3 (exit)
    };

    vector<int> dist = dijkstra(evacGraph, 0);
    alertHeap.push({5, "Fastest evacuation time to exit: " + to_string(dist[3])});

    /* -------- Step 4: Display Alerts (Highest Priority First) -------- */
    cout << "\n--- ALERTS (High → Low severity) ---\n";
    while (!alertHeap.empty()) {
        Alert a = alertHeap.top(); alertHeap.pop();
        cout << "Severity: " << a.severity << " | " << a.message << "\n";
    }

    return 0;
}

