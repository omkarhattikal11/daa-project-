#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <algorithm>
using namespace std;

// Structure for a Parking Slot
struct ParkingSlot {
    int slotNumber;
    int floor;
    int zone;
    int distance;      // distance from entrance
    bool isOccupied;
    bool isPriority;   // VIP or handicapped
};

// Comparison for min-heap (closest available slot first)
struct CompareDistance {
    bool operator()(ParkingSlot const& s1, ParkingSlot const& s2) {
        return s1.distance > s2.distance;
    }
};

// Parking lot represented as floor -> zone -> slots
vector<vector<vector<ParkingSlot>>> parkingLot;

// Map vehicle number to slot info
map<string, ParkingSlot> vehicleMap;

// Initialize parking lot
void initializeParking(int floors, int zones, int slotsPerZone) {
    parkingLot.resize(floors);
    int slotId = 1;
    for (int f = 0; f < floors; f++) {
        parkingLot[f].resize(zones);
        for (int z = 0; z < zones; z++) {
            for (int s = 0; s < slotsPerZone; s++) {
                parkingLot[f][z].push_back({slotId++, f, z, (f+1)*10 + (z+1)*5 + s, false, false});
            }
        }
    }
}

// Find nearest available slot
bool findNearestSlot(ParkingSlot &slot, bool priority=false) {
    priority_queue<ParkingSlot, vector<ParkingSlot>, CompareDistance> pq;
    for (auto &floor : parkingLot) {
        for (auto &zone : floor) {
            for (auto &s : zone) {
                if (!s.isOccupied && (priority ? s.isPriority : true))
                    pq.push(s);
            }
        }
    }
    if (!pq.empty()) {
        slot = pq.top();
        return true;
    }
    return false;
}

// Park a vehicle
void parkVehicle() {
    string vehicleNumber;
    int prio;
    cout << "Enter Vehicle Number: ";
    cin >> vehicleNumber;
    cout << "Is it VIP/Handicapped? (1-Yes, 0-No): ";
    cin >> prio;

    ParkingSlot slot;
    if (findNearestSlot(slot, prio==1)) {
        slot.isOccupied = true;
        slot.isPriority = prio==1;
        // Mark slot occupied in parking lot
        parkingLot[slot.floor][slot.zone][slot.slotNumber -1].isOccupied = true;
        parkingLot[slot.floor][slot.zone][slot.slotNumber -1].isPriority = prio==1;

        vehicleMap[vehicleNumber] = slot;
        cout << "Vehicle parked at Slot " << slot.slotNumber
             << " (Floor " << slot.floor+1 << ", Zone " << slot.zone+1 << ")\n";
    } else {
        cout << "No available slot!\n";
    }
}

// Remove vehicle
void removeVehicle() {
    string vehicleNumber;
    cout << "Enter Vehicle Number to remove: ";
    cin >> vehicleNumber;

    if (vehicleMap.find(vehicleNumber) != vehicleMap.end()) {
        ParkingSlot slot = vehicleMap[vehicleNumber];
        parkingLot[slot.floor][slot.zone][slot.slotNumber-1].isOccupied = false;
        vehicleMap.erase(vehicleNumber);
        cout << "Vehicle removed from Slot " << slot.slotNumber << endl;
    } else {
        cout << "Vehicle not found!\n";
    }
}

// Display all available slots sorted by distance
void displayAvailableSlots() {
    vector<ParkingSlot> available;
    for (auto &floor : parkingLot) {
        for (auto &zone : floor) {
            for (auto &s : zone) {
                if (!s.isOccupied)
                    available.push_back(s);
            }
        }
    }
    sort(available.begin(), available.end(), [](ParkingSlot a, ParkingSlot b){
        return a.distance < b.distance;
    });

    cout << "Available Slots (sorted by distance):\n";
    for (auto &s : available) {
        cout << "Slot " << s.slotNumber << " (Floor " << s.floor+1 << ", Zone " << s.zone+1
             << ", Distance " << s.distance << ")\n";
    }
}

// Search vehicle
void searchVehicle() {
    string vehicleNumber;
    cout << "Enter Vehicle Number to search: ";
    cin >> vehicleNumber;

    if (vehicleMap.find(vehicleNumber) != vehicleMap.end()) {
        ParkingSlot slot = vehicleMap[vehicleNumber];
        cout << "Vehicle found at Slot " << slot.slotNumber
             << " (Floor " << slot.floor+1 << ", Zone " << slot.zone+1 << ")\n";
    } else {
        cout << "Vehicle not found!\n";
    }
}

int main() {
    int floors = 2, zones = 3, slotsPerZone = 5;
    initializeParking(floors, zones, slotsPerZone);

    int choice;
    do {
        cout << "\nSmart Parking System Menu:\n";
        cout << "1. Park Vehicle\n2. Remove Vehicle\n3. Display Available Slots\n4. Search Vehicle\n5. Exit\nChoice: ";
        cin >> choice;

        switch(choice) {
            case 1: parkVehicle(); break;
            case 2: removeVehicle(); break;
            case 3: displayAvailableSlots(); break;
            case 4: searchVehicle(); break;
            case 5: cout << "Exiting...\n"; break;
            default: cout << "Invalid Choice!\n"; break;
        }

    } while(choice != 5);

    return 0;
}
