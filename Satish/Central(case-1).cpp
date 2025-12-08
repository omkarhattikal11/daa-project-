#include <bits/stdc++.h>
using namespace std;

// -------------------------------
// STRUCTURE FOR EVENTS (ARRAY)
// -------------------------------
struct EventSlot {
    string eventName;
    int duration;
};

// -------------------------------
// STRUCTURE FOR ADS (PRIORITY QUEUE)
// -------------------------------
struct Ad {
    string advertiser;
    int priority;
};

struct CompareAd {
    bool operator()(Ad const &a, Ad const &b) {
        return a.priority < b.priority;  
    }
};

// -------------------------------
// BFS FOR CROWD EVACUATION ROUTE
// -------------------------------
vector<int> graphPath[20];
bool visitedNode[20];
int parentNode[20];

// BFS FUNCTION
void bfs(int start) {
    queue<int> q;
    visitedNode[start] = true;
    q.push(start);

    while(!q.empty()) {
        int node = q.front();
        q.pop();

        for(int next : graphPath[node]) {
            if(!visitedNode[next]) {
                visitedNode[next] = true;
                parentNode[next] = node;
                q.push(next);
            }
        }
    }
}

// PRINT SHORTEST PATH
void printPath(int end) {
    if(end == -1) return;
    printPath(parentNode[end]);
    cout << end << " ";
}

// -------------------------------
// MAIN PROGRAM
// -------------------------------
int main() {

    cout << "\n==============================\n";
    cout << "VELORA – Times Square Control System\n";
    cout << "==============================\n\n";

    // -----------------------------------------------------
    // 1) EVENT SCHEDULE (ARRAY + STRUCTURES)
    // -----------------------------------------------------
    EventSlot slots[5] = {
        {"Music Concert", 120},
        {"Food Festival", 180},
        {"Street Show", 60},
        {"Tech Expo", 240},
        {"Art Exhibition", 90}
    };

    cout << "📌 Daily Event Schedule:\n";
    for(int i=0; i<5; i++) {
        cout << "  ▸ " << slots[i].eventName 
             << " — " << slots[i].duration << " mins\n";
    }
    cout << "\n====================================\n\n";

    // -----------------------------------------------------
    // 2) ADVERTISER DATABASE USING HASHING
    // -----------------------------------------------------
    unordered_map<string, int> advertiserBudget;
    advertiserBudget["Coca-Cola"] = 500000;
    advertiserBudget["Nike"] = 350000;
    advertiserBudget["Dominos"] = 200000;
    advertiserBudget["RelianceDigital"] = 425000;

    cout << "💼 Registered Advertisers (Budget in ₹):\n";
    for(auto &x : advertiserBudget) {
        cout << "  ▸ " << x.first << " — ₹" << x.second << endl;
    }
    cout << "\n====================================\n\n";

    // -----------------------------------------------------
    // 3) PRIORITY QUEUE FOR AD SCHEDULING
    // -----------------------------------------------------
    priority_queue<Ad, vector<Ad>, CompareAd> adQueue;

    adQueue.push({"Coca-Cola", 90});
    adQueue.push({"Nike", 70});
    adQueue.push({"Dominos", 85});
    adQueue.push({"RelianceDigital", 95});

    cout << "📺 AD Display Order (Highest Priority First):\n";
    while(!adQueue.empty()) {
        Ad a = adQueue.top();
        cout << "  ▸ " << a.advertiser << " (Priority: " << a.priority << ")\n";
        adQueue.pop();
    }
    cout << "\n====================================\n\n";

    // -----------------------------------------------------
    // 4) BFS FOR CROWD EVACUATION ROUTE PLANNING
    // -----------------------------------------------------
    // Creating sample city connections
    graphPath[1] = {2, 3};
    graphPath[2] = {4};
    graphPath[3] = {4, 5};
    graphPath[4] = {6};
    graphPath[5] = {6};

    for(int i=0;i<20;i++) parentNode[i] = -1;

    int crowdLocation = 1;   // Sample position
    int exitGate = 6;        // Safe exit

    bfs(crowdLocation);

    cout << "🚨 Shortest Crowd Evacuation Route: ";
    printPath(exitGate);
    cout << "\n";

    cout << "\n====================================\n";

    return 0;
}
