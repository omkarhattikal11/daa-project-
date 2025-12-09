#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// Customer structure
struct Customer {
    int id;
    string name;
    int priority;     // 0 = normal, 1 = senior/pregnant, 2 = VIP
    int serviceType;  // 0 = Bank, 1 = Govt
    int arrivalTime;  // simulated arrival time
};

// Comparison function for priority queue
struct ComparePriority {
    bool operator()(Customer const& c1, Customer const& c2) {
        return c1.priority < c2.priority; // higher priority first
    }
};

// Service counter
struct Counter {
    int id;
    queue<Customer> q;
    int totalServiceTime = 0;
};

// Function to assign customer to least busy counter
void assignCustomer(Customer c, vector<Counter>& counters) {
    int minSize = counters[0].q.size();
    int counterIndex = 0;
    for (int i = 1; i < counters.size(); i++) {
        if (counters[i].q.size() < minSize) {
            minSize = counters[i].q.size();
            counterIndex = i;
        }
    }
    counters[counterIndex].q.push(c);
    cout << "Customer " << c.name << " assigned to counter " << counters[counterIndex].id << endl;
}

// Function to serve customers from counters
void serveCustomers(vector<Counter>& counters) {
    cout << "\nServing customers at counters:\n";
    for (auto& counter : counters) {
        if (!counter.q.empty()) {
            Customer c = counter.q.front();
            counter.q.pop();
            counter.totalServiceTime += 5; // assume 5 mins per customer
            cout << "Counter " << counter.id << " served " << c.name;
            if (c.priority > 0) cout << " (Priority)";
            cout << endl;
        } else {
            cout << "Counter " << counter.id << " is empty.\n";
        }
    }
}

// Function to remove a customer if they leave
void removeCustomer(vector<Counter>& counters, int customerId) {
    for (auto& counter : counters) {
        queue<Customer> temp;
        bool found = false;
        while (!counter.q.empty()) {
            Customer c = counter.q.front();
            counter.q.pop();
            if (c.id == customerId) {
                found = true;
                cout << "Customer " << c.name << " removed from counter " << counter.id << endl;
                continue;
            }
            temp.push(c);
        }
        counter.q = temp;
        if (found) break;
    }
}

// Function to calculate average waiting time
void calculateAverageWaitingTime(vector<Counter>& counters) {
    int totalCustomers = 0;
    int totalTime = 0;
    for (auto& counter : counters) {
        totalCustomers += counter.q.size();
        totalTime += counter.totalServiceTime;
    }
    if (totalCustomers > 0) {
        cout << "Average waiting time per customer: " 
             << (totalTime / totalCustomers) << " mins\n";
    } else {
        cout << "No customers in queue.\n";
    }
}

int main() {
    int customerIdCounter = 1;
    int timeCounter = 0;

    priority_queue<Customer, vector<Customer>, ComparePriority> priorityQueueBank;
    priority_queue<Customer, vector<Customer>, ComparePriority> priorityQueueGovt;

    vector<Counter> bankCounters = {{1}, {2}, {3}};
    vector<Counter> govtCounters = {{1}, {2}};

    int choice;
    do {
        cout << "\n1. Add Customer\n2. Serve Customers\n3. Customer Leaves\n4. Average Waiting Time\n5. Exit\nChoice: ";
        cin >> choice;

        if (choice == 1) {
            Customer c;
            c.id = customerIdCounter++;
            c.arrivalTime = timeCounter++;

            cout << "Enter name: ";
            cin >> c.name;
            cout << "Enter priority (0-normal, 1-senior/pregnant, 2-VIP): ";
            cin >> c.priority;
            cout << "Enter service type (0-Bank, 1-Govt): ";
            cin >> c.serviceType;

            if (c.serviceType == 0) {
                if (c.priority > 0) priorityQueueBank.push(c);
                else assignCustomer(c, bankCounters);
            } else {
                if (c.priority > 0) priorityQueueGovt.push(c);
                else assignCustomer(c, govtCounters);
            }

        } else if (choice == 2) {
            // Serve priority customers first
            cout << "\nServing Priority Customers:\n";
            while (!priorityQueueBank.empty()) {
                Customer c = priorityQueueBank.top();
                priorityQueueBank.pop();
                assignCustomer(c, bankCounters);
            }
            while (!priorityQueueGovt.empty()) {
                Customer c = priorityQueueGovt.top();
                priorityQueueGovt.pop();
                assignCustomer(c, govtCounters);
            }

            // Serve normal customers at counters
            serveCustomers(bankCounters);
            serveCustomers(govtCounters);

        } else if (choice == 3) {
            int leaveId;
            cout << "Enter Customer ID to remove: ";
            cin >> leaveId;
            removeCustomer(bankCounters, leaveId);
            removeCustomer(govtCounters, leaveId);

        } else if (choice == 4) {
            calculateAverageWaitingTime(bankCounters);
            calculateAverageWaitingTime(govtCounters);

        }

    } while (choice != 5);

    return 0;
}
