#include <bits/stdc++.h>
using namespace std;

#define INF 1e9

// ---------------- STRUCTURES ----------------
struct Patient {
    string name;
    string condition;
    int severity; // 3 = Critical, 2 = Urgent, 1 = Normal
};

struct Ambulance {
    string id;
    int location;  // node index
    bool available;
};

struct Resources {
    int beds;
    int icuBeds;
    int ventilators;
    map<string,int> blood; // A+,A-,B+,B-,O+,O-,AB+,AB-
};

// ---------------- DIJKSTRA: SHORTEST PATH ----------------
vector<int> dijkstra(int src, vector<vector<pair<int,int>>> &graph) {
    int n = graph.size();
    vector<int> dist(n, INF);
    dist[src] = 0;

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;
    pq.push({0, src});

    while(!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if(d > dist[u]) continue;

        for(auto &edge : graph[u]) {
            int v = edge.first, w = edge.second;
            if(dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

// ---------------- TRIAGE PRIORITY QUEUE ----------------
struct Compare {
    bool operator()(Patient &a, Patient &b) {
        return a.severity < b.severity;
    }
};

// ---------------- MAIN PROGRAM ----------------
int main() {

    // ---------------- HOSPITAL RESOURCES ----------------
    Resources r = {50, 10, 7, 
        {{"A+",5},{"A-",4},{"B+",3},{"O+",8},{"O-",2}}
    };

    // ---------------- AMBULANCES ----------------
    vector<Ambulance> amb = {
        {"AMB1",0,true},
        {"AMB2",3,true},
        {"AMB3",5,false}
    };

    // ---------------- HOSPITAL GRAPH ----------------
    vector<vector<pair<int,int>>> graph(6);
    graph[0].push_back({1,7});
    graph[1].push_back({2,5});
    graph[1].push_back({3,3});
    graph[2].push_back({4,4});
    graph[3].push_back({4,6});
    graph[4].push_back({5,2});

    // ---------------- TRIAGE QUEUE ----------------
    priority_queue<Patient, vector<Patient>, Compare> triage;

    // Add some patients
    triage.push({"Rahul","Heart Attack",3});
    triage.push({"Asha","Accident Injury",2});
    triage.push({"John","Fever",1});

    cout << "\n=== PATIENT TRIAGE LIST ===\n";
    while(!triage.empty()) {
        auto p = triage.top(); triage.pop();
        cout << p.name << " | Condition: " << p.condition 
             << " | Severity Level: " << p.severity << endl;
    }

    // ---------------- FIND NEAREST AMBULANCE ----------------
    int emergencyLocation = 4;
    cout << "\n=== NEAREST AMBULANCE DISPATCH ===\n";

    int bestAmbulance = -1;
    int bestDist = INF;

    for(int i=0;i<amb.size();i++){
        if(!amb[i].available) continue;

        auto dist = dijkstra(amb[i].location, graph);
        if(dist[emergencyLocation] < bestDist){
            bestDist = dist[emergencyLocation];
            bestAmbulance = i;
        }
    }

    if(bestAmbulance != -1){
        cout << "Dispatch Ambulance: " << amb[bestAmbulance].id 
             << " | Distance: " << bestDist << endl;
    }
    else cout << "No ambulance available!\n";

    // ---------------- RESOURCE STATUS ----------------
    cout << "\n=== RESOURCE STATUS ===\n";
    cout << "Beds Available: " << r.beds << endl;
    cout << "ICU Beds: " << r.icuBeds << endl;
    cout << "Ventilators: " << r.ventilators << endl;

    cout << "\nBlood Stock:\n";
    for(auto &b : r.blood)
        cout << b.first << ": " << b.second << endl;

    // ---------------- BLOOD MATCH SYSTEM ----------------
    string needed = "O+";
    cout << "\n=== BLOOD MATCH ===\nNeed Blood: " << needed << endl;
    if(r.blood[needed] > 0) cout << "Blood Available!" << endl;
    else cout << "Not Available!" << endl;

    return 0;
}

