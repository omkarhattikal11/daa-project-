#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <utility> // for pair
using namespace std;

// BFS simulation for water spread
void bfsFloodSimulation(vector<vector<int>>& cityGrid, int startX, int startY) {
    int rows = cityGrid.size();
    int cols = cityGrid[0].size();
    vector<vector<bool>> visited(rows, vector<bool>(cols, false));
    queue<pair<int,int>> q;

    q.push({startX, startY});
    visited[startX][startY] = true;

    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    cout << "\nFlood Simulation BFS:\n";
    while(!q.empty()) {
        pair<int,int> pos = q.front(); q.pop();
        int x = pos.first;
        int y = pos.second;
        cout << "Flood reached: (" << x << "," << y << ")\n";

        for(int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if(nx >= 0 && nx < rows && ny >= 0 && ny < cols
               && !visited[nx][ny] && cityGrid[nx][ny] == 0) {
                visited[nx][ny] = true;
                q.push({nx, ny});
            }
        }
    }
}

int main() {
    int n;
    cout << "Enter number of water-level readings: ";
    cin >> n;

    vector<int> waterLevels(n);
    queue<int> liveDataQueue;
    unordered_map<int, string> sensorLocations;

    cout << "Enter water-level readings (meters):\n";
    for(int i = 0; i < n; i++) {
        cin >> waterLevels[i];
        liveDataQueue.push(waterLevels[i]);
    }

    // Example sensor locations
    sensorLocations[101] = "River A";
    sensorLocations[102] = "Dam B";

    // Max-Heap to find highest water level
    priority_queue<int> maxHeap(waterLevels.begin(), waterLevels.end());
    cout << "\nHighest Water Level: " << maxHeap.top() << " meters\n";

    // Trend Analysis using Selection Sort
    vector<int> sortedLevels = waterLevels;
    for(int i = 0; i < n-1; i++){
        int minIdx = i;
        for(int j = i+1; j < n; j++){
            if(sortedLevels[j] < sortedLevels[minIdx])
                minIdx = j;
        }
        swap(sortedLevels[i], sortedLevels[minIdx]);
    }

    cout << "Water Level Trends (sorted): ";
    for(int level : sortedLevels) cout << level << " ";
    cout << "\n";

    // Sample city grid BFS
    vector<vector<int>> cityGrid = {
        {0,0,0,1,0},
        {0,1,0,1,0},
        {0,1,0,0,0},
        {0,0,0,1,0}
    };
    bfsFloodSimulation(cityGrid, 0, 0);

    // Alerts
    const int threshold = 6;
    cout << "\nAlerts:\n";
    while(!liveDataQueue.empty()) {
        int reading = liveDataQueue.front(); liveDataQueue.pop();
        if(reading >= threshold) {
            cout << "ALERT! Water level high: " << reading << " meters\n";
        }
    }

    return 0;
}
