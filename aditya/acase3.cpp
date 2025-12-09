#include <bits/stdc++.h>
using namespace std;

// ----------- Structure for Sensor -------------
struct Sensor {
    float level;   // water level
    float rain;    // rainfall
    long time;     // timestamp
};

queue<Sensor> liveData;          // real-time queue
priority_queue<float> maxHeap;   // maximum water level
unordered_map<int,string> sensorLocation; // hashing lookup table

// ---------- BFS Flood Spread Simulation ----------
void floodBFS(vector<vector<int>>& grid, int x, int y) {
    int n = grid.size();
    int m = grid[0].size();
    queue<pair<int,int>> q;
    q.push({x,y});

    int dx[4] = {1,-1,0,0};
    int dy[4] = {0,0,1,-1};

    while(!q.empty()) {
        auto [a,b] = q.front(); q.pop();

        for(int i=0;i<4;i++){
            int nx = a + dx[i];
            int ny = b + dy[i];

            if(nx>=0 && ny>=0 && nx<n && ny<m && grid[nx][ny]==0){
                grid[nx][ny] = 1; // mark flooded
                q.push({nx,ny});
            }
        }
    }
}

// ----------- Selection Sort for Trend Analysis -----------
void selectionSort(vector<float>& arr) {
    int n = arr.size();
    for(int i=0;i<n;i++){
        int minIndex = i;
        for(int j=i+1;j<n;j++)
            if(arr[j] < arr[minIndex])
                minIndex = j;
        swap(arr[i], arr[minIndex]);
    }
}

// ------------------ Main Program ------------------
int main() {

    // Hash table (sensor ID -> location)
    sensorLocation[101] = "River Bank";
    sensorLocation[102] = "Dam Gate";

    // Sample sensors
    Sensor s1 = {3.5, 10, 100};
    Sensor s2 = {4.8, 20, 110};
    Sensor s3 = {5.5, 30, 120}; // high value

    liveData.push(s1);
    liveData.push(s2);
    liveData.push(s3);

    maxHeap.push(s1.level);
    maxHeap.push(s2.level);
    maxHeap.push(s3.level);

    cout << "Highest water level: " << maxHeap.top() << " m\n";

    // Lookup using hashing
    cout << "Sensor 101 located at: " << sensorLocation[101] << "\n";

    // Sorting trend
    vector<float> levels = {s1.level, s2.level, s3.level};
    selectionSort(levels);

    cout << "Sorted water levels: ";
    for(float x : levels) cout << x << " ";
    cout << endl;

    // Flood simulation grid
    vector<vector<int>> area = {
        {0,0,0},
        {0,1,0}, // flood starts here
        {0,0,0}
    };

    floodBFS(area, 1, 1);

    cout << "\nFlood Spread Simulation:\n";
    for(auto &row : area){
        for(int cell : row) cout << cell << " ";
        cout << endl;
    }

    return 0;
}
