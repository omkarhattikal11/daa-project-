#include <iostream>
#include <queue>
#include <vector>
#include <string>
using namespace std;

struct Customer {
    int id, priority, arrivalTime;
    string name;
};

struct ComparePriority {
    bool operator()(Customer const& a, Customer const& b) {
        return a.priority < b.priority; 
    }
};

int main() {
    int n, counters;

    // Input: number of customers and counters
    cout << "Enter number of customers: "; 
    cin >> n;
    cout << "Enter number of service counters: "; 
    cin >> counters;

    queue<Customer> regularQueue;
    priority_queue<Customer, vector<Customer>, ComparePriority> priorityQueue;

    // Input customer details
    for(int i = 0; i < n; i++) {
        Customer c;
        cout << "\nEnter Customer ID: "; cin >> c.id;
        cout << "Enter Name: "; cin >> c.name;
        cout << "Enter Priority (0 = regular, >0 = priority): "; cin >> c.priority;
        cout << "Enter Arrival Time: "; cin >> c.arrivalTime;

        if(c.priority > 0) 
            priorityQueue.push(c);
        else 
            regularQueue.push(c);
    }

    vector<int> counterTime(counters, 0);
    cout << "\nProcessing Customers:\n";

    // Process customers at counters
    while(!priorityQueue.empty() || !regularQueue.empty()) {
        for(int i = 0; i < counters; i++) {
            if(!priorityQueue.empty()) {
                Customer c = priorityQueue.top(); priorityQueue.pop();
                counterTime[i] = max(counterTime[i], c.arrivalTime) + 1;
                cout << "Counter " << i+1 << " PRIORITY " << c.name 
                     << " (ID:" << c.id << ") Time: " << counterTime[i] << "\n";
            } else if(!regularQueue.empty()) {
                Customer c = regularQueue.front(); regularQueue.pop();
                counterTime[i] = max(counterTime[i], c.arrivalTime) + 1;
                cout << "Counter " << i+1 << " REGULAR " << c.name 
                     << " (ID:" << c.id << ") Time: " << counterTime[i] << "\n";
            }
        }
    }

    // Maximum service time
    int maxWait = 0;
    for(int t : counterTime) 
        if(t > maxWait) maxWait = t;
    cout << "\nMaximum Service Time: " << maxWait << "\n";

    return 0;
}
