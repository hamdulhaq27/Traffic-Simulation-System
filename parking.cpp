#include "parking.h"
using namespace std;

void initParkingLot(ParkingLot &lot, int id, int totalSpots, int waitingSlots) {
    lot.id = id;
    sem_init(&lot.spots, 0, totalSpots);
    sem_init(&lot.waitingQueue, 0, waitingSlots);

    cout << "[Lot " << lot.id << "] Ready (" 
         << totalSpots << " spots, " 
         << waitingSlots << " waiting slots)" << endl;
}

bool enterParkingLot(ParkingLot &lot, int vehicleID) {
    cout << "• Vehicle " << vehicleID << ": Approaching Lot " << lot.id << endl;

    if (sem_trywait(&lot.waitingQueue) == -1) {
        cout << "  → WaitingQueue FULL, cannot wait. Vehicle continues." << endl;
        return false;
    }

    cout << "  → Waiting…" << endl;
    sem_wait(&lot.spots);   // wait for parking spot
    sem_post(&lot.waitingQueue);

    cout << "  → Parked successfully." << endl;
    return true;
}

void leaveParkingLot(ParkingLot &lot, int vehicleID) {
    sem_post(&lot.spots);
    cout << "  → Vehicle " << vehicleID << " left Lot " << lot.id 
         << ", spot freed." << endl;
}

void destroyParkingLot(ParkingLot &lot) {
    sem_destroy(&lot.spots);
    sem_destroy(&lot.waitingQueue);
    cout << "[Lot " << lot.id << "] Shutdown completed." << endl;
}
