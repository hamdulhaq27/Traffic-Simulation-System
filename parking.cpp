#include "parking.h"
using namespace std;

// Initialize a ParkingLot with given total spots and waiting slots
void initParkingLot(ParkingLot &lot, int id, int totalSpots, int waitingSlots) {
    lot.id = id;
    sem_init(&lot.spots, 0, totalSpots);       // parking spots
    sem_init(&lot.waitingQueue, 0, waitingSlots); // waiting queue slots

    cout << "[ParkingLot " << lot.id << "] Ready with "
         << totalSpots << " spots and " << waitingSlots << " waiting slots." << endl;
}

// Destroy semaphores in a ParkingLot
void destroyParkingLot(ParkingLot &lot) {
    sem_destroy(&lot.spots);
    sem_destroy(&lot.waitingQueue);

    // Optional: print message
    cout << "[ParkingLot " << lot.id << "] Destroyed." << endl;
}