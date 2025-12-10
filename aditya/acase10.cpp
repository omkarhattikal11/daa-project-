// transport_opt.cpp
// Industrial Goods Transport Optimization Demo
// Uses: Graph, Dijkstra, greedy vehicle routing (nearest neighbor), 2-opt local improvement,
// priority queue for urgent shipments, capacity-aware assignment.
// Compile: g++ -std=c++17 transport_opt.cpp -O2 -o transport_opt

#include <bits/stdc++.h>
using namespace std;

/* -------------------------
   Basic types & structures
   ------------------------- */

struct Edge {
    int to;
    double dist; // kilometers
    double time; // minutes (optional)
    Edge(int _t=0, double _d=0.0, double _tm=0.0): to(_t), dist(_d), time(_tm) {}
};

using Graph = vector<vector<Edge>>;

struct Shipment {
    int id;
    int pickup;      // node id
    int delivery;    // node id
    double weight;   // in tonnes
    int priority;    // higher = more urgent
    double readyTime; // hours (optional)
};

struct Vehicle {
    int id;
    int depot;       // starting node
    double capacity; // tonnes
    double costPerKm; // currency per km
    double fuelPerKm; // liters per km
    vector<int> route; // sequence of node ids (depot,...deliveries,...depot)
    double loadAssigned; // current assigned load
    vector<int> assignedShipments; // shipment ids
};

/* -------------------------
   Utilities & constants
   ------------------------- */

const double CO2_PER_LITER_DIESEL = 2.68; // kg CO2 per liter (approx)
const double DEFAULT_SPEED_KMPH = 40.0; // for time estimation

/* -------------------------
   Shortest paths: Dijkstra
   ------------------------- */

vector<double> dijkstra(const Graph &g, int src) {
    int n = g.size();
    const double INF = 1e18;
    vector<double> dist(n, INF);
    using PDI = pair<double,int>;
    priority_queue<PDI, vector<PDI>, greater<PDI>> pq;
    dist[src] = 0;
    pq.push({0, src});
    while (!pq.empty()) {
        auto [d,u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;
        for (auto &e : g[u]) {
            if (dist[e.to] > d + e.dist) {
                dist[e.to] = d + e.dist;
                pq.push({dist[e.to], e.to});
            }
        }
    }
    return dist;
}

// All-pairs shortest distances for small graphs (we compute on demand per depot)
vector<vector<double>> allPairsShortest(const Graph &g) {
    int n = g.size();
    vector<vector<double>> ap(n, vector<double>(n, 1e18));
    for (int i = 0; i < n; ++i) {
        ap[i] = dijkstra(g, i);
    }
    return ap;
}

/* -------------------------
   Route utilities
   ------------------------- */

// Compute route distance given a sequence of nodes and precomputed matrix dist
double routeDistance(const vector<int> &route, const vector<vector<double>> &distMat) {
    double total = 0.0;
    for (size_t i = 1; i < route.size(); ++i) {
        total += distMat[route[i-1]][route[i]];
    }
    return total;
}

// 2-opt improvement for route (exclude first and last if depot duplicated)
bool twoOptImprove(vector<int> &route, const vector<vector<double>> &distMat) {
    bool improved = false;
    int n = route.size();
    if (n <= 3) return false;
    double bestGain = 1e-9;
    int bi=-1, bj=-1;
    for (int i = 1; i < n-2; ++i) {
        for (int j = i+1; j < n-1; ++j) {
            // current edges: (i-1,i) + (j,j+1)
            double before = distMat[route[i-1]][route[i]] + distMat[route[j]][route[j+1]];
            double after  = distMat[route[i-1]][route[j]] + distMat[route[i]][route[j+1]];
            double gain = before - after;
            if (gain > bestGain) {
                bestGain = gain;
                bi = i; bj = j;
                improved = true;
            }
        }
    }
    if (improved) {
        reverse(route.begin()+bi, route.begin()+bj+1);
    }
    return improved;
}

// Improve until no improvement or limit
void twoOptLoop(vector<int> &route, const vector<vector<double>> &distMat, int iterLimit=100) {
    int it = 0;
    while (it++ < iterLimit && twoOptImprove(route, distMat)) {}
}

/* -------------------------
   Assignment & Routing Core
   ------------------------- */

// Build routes for vehicles using greedy nearest neighbor for assigned delivery nodes
// Route: depot -> pickups/deliveries -> depot (we treat pickup and delivery nodes as single visit nodes)
// shipmentsMap: map id -> Shipment
// assigned shipments for a vehicle are provided as vector<int> shipmentIds
vector<int> buildRouteForVehicle(const Vehicle &veh,
    const vector<int> &shipmentIds,
    const unordered_map<int, Shipment> &shipMap,
    const vector<vector<double>> &distMat)
{
    // For simplicity, we include pickup and delivery nodes: visit pickup then delivery for each shipment.
    // A more advanced model would combine pickups and deliveries in same route ordering, respecting capacity timeline.
    // Here: we will visit pickups first (group), then deliveries, using nearest neighbor heuristic.
    vector<int> nodes; // remaining nodes to visit
    // Create sequence of nodes: pickups then deliveries (unique nodes may repeat if same node for multiple shipments)
    for (int sid : shipmentIds) {
        nodes.push_back(shipMap.at(sid).pickup);
        nodes.push_back(shipMap.at(sid).delivery);
    }
    // remove duplicates while preserving occurrences: for routing we can keep duplicates as separate visits
    // Greedy nearest neighbor starting from depot
    vector<int> route;
    route.push_back(veh.depot);
    vector<char> used(nodes.size(), 0);

    int current = veh.depot;
    for (size_t step = 0; step < nodes.size(); ++step) {
        double bestD = 1e18;
        int bestIdx = -1;
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (used[i]) continue;
            double d = distMat[current][nodes[i]];
            if (d < bestD) { bestD = d; bestIdx = (int)i; }
        }
        if (bestIdx == -1) break;
        used[bestIdx] = 1;
        route.push_back(nodes[bestIdx]);
        current = nodes[bestIdx];
    }
    // return to depot
    route.push_back(veh.depot);
    return route;
}

/* -------------------------
   Greedy assignment of shipments to vehicles
   ------------------------- */
// Approach:
// - Put shipments into max-heap by priority (higher first), break ties by heavier weight.
// - For each shipment, find the vehicle with enough spare capacity that yields smallest marginal distance increase.
// - If none fits, open new vehicle (if available) or mark as unassigned.

struct ShipmentHeapItem {
    int sid;
    int priority;
    double weight;
};
struct SHComp {
    bool operator()(const ShipmentHeapItem &a, const ShipmentHeapItem &b) const {
        if (a.priority != b.priority) return a.priority < b.priority;
        return a.weight < b.weight;
    }
};

// compute marginal insertion cost if we simply append pickup+delivery into existing route at best positions.
// For simplicity, we'll compute estimated marginal cost as dist(depot->pickup->delivery->depot) if vehicle has no route,
// otherwise incremental estimate: dist(lastNode, pickup) + dist(pickup, delivery) + dist(delivery, depot) - dist(lastNode, depot)
double estimatedMarginalCostAppend(const Vehicle &veh, const Shipment &s, const vector<vector<double>> &distMat) {
    int depot = veh.depot;
    double lastToDepot = 0;
    if (veh.route.empty()) lastToDepot = 0;
    else {
        int lastNode = veh.route.back();
        lastToDepot = distMat[lastNode][depot];
    }
    double lastToPickup = veh.route.empty() ? distMat[depot][s.pickup] : distMat[veh.route.back()][s.pickup];
    double costNew = lastToPickup + distMat[s.pickup][s.delivery] + distMat[s.delivery][depot];
    double costOld = lastToDepot;
    return costNew - costOld;
}

// perform greedy packing & routing assignment
void assignShipmentsToVehicles(
    const vector<Shipment> &shipments,
    vector<Vehicle> &vehicles,
    const vector<vector<double>> &distMat,
    unordered_map<int, Shipment> &shipMap,
    vector<int> &unassigned)
{
    // build priority heap
    priority_queue<ShipmentHeapItem, vector<ShipmentHeapItem>, SHComp> heap;
    for (auto &s : shipments) {
        heap.push({s.id, s.priority, s.weight});
        shipMap[s.id] = s;
    }

    // Reset vehicle assigned info
    for (auto &v : vehicles) {
        v.assignedShipments.clear();
        v.loadAssigned = 0;
        v.route.clear();
    }

    while (!heap.empty()) {
        auto top = heap.top(); heap.pop();
        int sid = top.sid;
        Shipment s = shipMap[sid];

        // scan vehicles for feasible insertion (capacity)
        double bestCost = 1e18;
        int bestVid = -1;

        for (int vid = 0; vid < (int)vehicles.size(); ++vid) {
            auto &veh = vehicles[vid];
            if (veh.loadAssigned + s.weight <= veh.capacity) {
                double marginal = estimatedMarginalCostAppend(veh, s, distMat);
                if (marginal < bestCost) {
                    bestCost = marginal;
                    bestVid = vid;
                }
            }
        }

        if (bestVid == -1) {
            // no vehicle fits; try to find any vehicle with remaining capacity (allow splitting? here we don't split)
            unassigned.push_back(sid);
        } else {
            // assign to best vehicle
            vehicles[bestVid].assignedShipments.push_back(sid);
            vehicles[bestVid].loadAssigned += s.weight;
            // build route incrementally (append) -- we'll rebuild full route later
            // For now we append nodes to route to approximate
            vehicles[bestVid].route.push_back(s.pickup);
            vehicles[bestVid].route.push_back(s.delivery);
        }
    }

    // After assignment, rebuild and optimize routes for each vehicle
    for (auto &veh : vehicles) {
        // deduplicate simple route and convert to proper sequence: we'll rebuild using assigned shipments
        veh.route = buildRouteForVehicle(veh, veh.assignedShipments, shipMap, distMat);
        // Improve route using 2-opt
        twoOptLoop(veh.route, distMat, 200);
    }
}

/* -------------------------
   Evaluation & metrics
   ------------------------- */

struct Metrics {
    double totalDistance = 0.0;
    double totalCost = 0.0;
    double totalFuel = 0.0;
    double totalCO2kg = 0.0;
    int vehiclesUsed = 0;
    int shipmentsDelivered = 0;
    int shipmentsUnassigned = 0;
};

Metrics evaluateSolution(const vector<Vehicle> &vehicles,
                         const unordered_map<int, Shipment> &shipMap,
                         const vector<vector<double>> &distMat)
{
    Metrics m;
    for (const auto &veh : vehicles) {
        if (veh.assignedShipments.empty()) continue;
        m.vehiclesUsed++;
        double dist = routeDistance(veh.route, distMat);
        m.totalDistance += dist;
        m.totalCost += dist * veh.costPerKm;
        double fuel = dist * veh.fuelPerKm;
        m.totalFuel += fuel;
        m.totalCO2kg += fuel * CO2_PER_LITER_DIESEL;
        m.shipmentsDelivered += (int)veh.assignedShipments.size();
    }
    m.shipmentsUnassigned = 0;
    return m;
}

/* -------------------------
   Demo & interactive menu
   ------------------------- */

void printGraph(const Graph &g) {
    cout << "Graph (adjacency list):\n";
    for (int i = 0; i < (int)g.size(); ++i) {
        cout << "Node " << i << ": ";
        for (auto &e : g[i]) cout << "(" << e.to << ", " << e.dist << "km) ";
        cout << "\n";
    }
}

void printShipments(const vector<Shipment> &s) {
    cout << "Shipments:\n";
    for (auto &sh : s) {
        cout << "ID " << sh.id << " [" << sh.priority << "] w=" << sh.weight
             << " from " << sh.pickup << " -> " << sh.delivery << "\n";
    }
}

void printVehicles(const vector<Vehicle> &v) {
    cout << "Vehicles:\n";
    for (auto &veh: v) {
        cout << "V" << veh.id << " depot=" << veh.depot << " cap=" << veh.capacity
             << " assigned=" << veh.assignedShipments.size() << "\n";
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "=== Industrial Goods Transport Optimization Demo ===\n\n";

    // Build a sample graph (nodes 0..6)
    int N = 8;
    Graph g(N);
    auto addEdge = [&](int u, int v, double d){
        g[u].push_back(Edge(v,d));
        g[v].push_back(Edge(u,d));
    };
    // Sample highway network distances (km)
    addEdge(0,1,10); addEdge(0,2,15); addEdge(1,3,12); addEdge(2,3,10);
    addEdge(1,2,5); addEdge(3,4,7); addEdge(2,5,20); addEdge(4,6,11); addEdge(5,6,9); addEdge(6,7,8);

    printGraph(g);

    // Precompute all-pairs shortest distances
    auto distMat = allPairsShortest(g);

    // Example shipments
    vector<Shipment> shipments = {
        {1, 0, 4, 2.5, 10, 8.0},
        {2, 1, 5, 3.0, 8, 9.0},
        {3, 2, 6, 5.0, 9, 8.0},
        {4, 0, 7, 1.0, 5, 10.0},
        {5, 3, 5, 4.0, 7, 6.0},
        {6, 1, 6, 2.0, 6, 7.0},
        {7, 2, 4, 1.5, 4, 9.0}
    };
    printShipments(shipments);

    // Vehicles (fleet)
    vector<Vehicle> vehicles = {
        {1, 0, 8.0, 1.5, 0.25}, // id, depot, capacity (tonnes), cost/km, fuel/km
        {2, 1, 6.0, 1.2, 0.20},
        {3, 2, 10.0, 1.7, 0.30}
    };
    printVehicles(vehicles);

    // Run assignment
    unordered_map<int, Shipment> shipMap;
    vector<int> unassigned;
    assignShipmentsToVehicles(shipments, vehicles, distMat, shipMap, unassigned);

    // Evaluate
    Metrics metrics = evaluateSolution(vehicles, shipMap, distMat);

    // Print vehicle routes & assignment
    cout << "\n--- Assignment & Routes ---\n";
    for (auto &veh : vehicles) {
        cout << "Vehicle " << veh.id << " depot " << veh.depot << " capacity " << veh.capacity << " assigned load " << veh.loadAssigned << "\n";
        cout << "  Shipments: ";
        for (int sid : veh.assignedShipments) cout << sid << " ";
        cout << "\n  Route: ";
        for (int node : veh.route) cout << node << " ";
        double rdist = routeDistance(veh.route, distMat);
        cout << "\n  Route distance: " << rdist << " km\n";
    }

    if (!unassigned.empty()) {
        cout << "\nUnassigned shipments: ";
        for (int u : unassigned) cout << u << " ";
        cout << "\n";
        metrics.shipmentsUnassigned = (int)unassigned.size();
    }

    // Compute totals
    cout << "\n--- Metrics ---\n";
    cout << "Total vehicles used: " << metrics.vehiclesUsed << "\n";
    cout << "Total distance (km): " << metrics.totalDistance << "\n";
    cout << "Total cost: " << metrics.totalCost << "\n";
    cout << "Total fuel (L): " << metrics.totalFuel << "\n";
    cout << "Total CO2 (kg): " << metrics.totalCO2kg << "\n";
    cout << "Shipments delivered: " << metrics.shipmentsDelivered << "\n";
    cout << "Shipments unassigned: " << metrics.shipmentsUnassigned << "\n";

    // Simple what-if: add new vehicle and re-run assignment
    cout << "\n--- What-if: add a new larger vehicle to reduce unassigned ---\n";
    vehicles.push_back({4, 0, 12.0, 1.6, 0.28});
    unassigned.clear(); shipMap.clear();
    assignShipmentsToVehicles(shipments, vehicles, distMat, shipMap, unassigned);
    metrics = evaluateSolution(vehicles, shipMap, distMat);
    cout << "After adding vehicle 4:\n";
    for (auto &veh : vehicles) {
        cout << "V" << veh.id << " assigned " << veh.assignedShipments.size() << " shipments, load " << veh.loadAssigned << "\n";
    }
    cout << "Unassigned now: " << unassigned.size() << "\n";

    cout << "\nDemo complete. You can extend:\n- time windows, split deliveries, dynamic rebalancing,\n- vehicle return-to-depot scheduling optimization,\n- richer VRP metaheuristics (simulated annealing, Tabu, Clarke-Wright)\n- real road network import and real distances.\n";

    return 0;
}

