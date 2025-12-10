#include <iostream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
using namespace std;

/* -------------------------------
   SECTION 1: HASH TABLE
   Track failed login attempts
--------------------------------- */
unordered_map<string, int> failedAttempts;

void trackLogin(string ip, bool success) {
    if (!success)
        failedAttempts[ip]++;
}

/* -------------------------------
   SECTION 2: KMP PATTERN MATCHING
   Detect malware signatures
--------------------------------- */
vector<int> buildLPS(string pat) {
    int n = pat.length();
    vector<int> lps(n, 0);
    int len = 0, i = 1;

    while (i < n) {
        if (pat[i] == pat[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0)
                len = lps[len - 1];
            else
                lps[i++] = 0;
        }
    }
    return lps;
}

bool KMPSearch(string text, string pat) {
    vector<int> lps = buildLPS(pat);
    int i = 0, j = 0;

    while (i < text.length()) {
        if (text[i] == pat[j]) {
            i++; j++;
        }
        if (j == pat.length()) return true;

        else if (i < text.length() && text[i] != pat[j]) {
            if (j != 0) j = lps[j - 1];
            else i++;
        }
    }
    return false;
}

/* -------------------------------
   SECTION 3: PRIORITY QUEUE
   Top suspicious IPs
--------------------------------- */
priority_queue<pair<int,string>> pq;

void loadPriorityQueue() {
    for (auto &it : failedAttempts)
        pq.push({it.second, it.first});
}

/* -------------------------------
   SECTION 4: BFS for Lateral Movement
--------------------------------- */
vector<vector<int>> graph;

bool detectLateralMovement(int start, int target) {
    vector<bool> visited(graph.size(), false);
    queue<int> q;

    visited[start] = true;
    q.push(start);

    while (!q.empty()) {
        int node = q.front(); q.pop();

        if (node == target)
            return true;

        for (int neigh : graph[node]) {
            if (!visited[neigh]) {
                visited[neigh] = true;
                q.push(neigh);
            }
        }
    }
    return false;
}

/* -------------------------------
   MAIN PROGRAM
--------------------------------- */
int main() {

    cout << "\n--- Cybersecurity Threat Detection System ---\n";

    // 1) Track login attempts
    trackLogin("192.168.1.5", false);
    trackLogin("192.168.1.5", false);
    trackLogin("10.0.0.2", false);
    trackLogin("10.0.0.2", true);
    trackLogin("172.16.0.9", false);

    // 2) Malware detection using KMP
    string logData = "User downloaded malware.exe from suspicious site";
    string signature = "malware.exe";

    if (KMPSearch(logData, signature))
        cout << "Malware Signature Detected!\n";

    // 3) Top suspicious IP using heap
    loadPriorityQueue();
    cout << "\nTop Suspicious IP: " 
         << pq.top().second << " (Attempts: " << pq.top().first << ")\n";

    // 4) Graph (network) for lateral movement
    graph = {
        {1},        // 0 connected to 1
        {0, 2},     // 1 connected to 0 and 2
        {1, 3},     // 2 connected to 1 and 3
        {}          // 3 isolated or sensitive server
    };

    if (detectLateralMovement(0, 3))
        cout << "Lateral Movement Detected (0 → 3)\n";
    else
        cout << "No Lateral Movement Detected\n";

    return 0;
}

