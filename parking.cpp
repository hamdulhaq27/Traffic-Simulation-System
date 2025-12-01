#include "parking.h"

// Initialize ParkingLot
void initParkingLot(ParkingLot &lot, int id, int capacity, int waiting_capacity) {
    lot.id = id;
    lot.capacity = capacity;
    lot.waiting_capacity = waiting_capacity;
    sem_init(&lot.spots, 0, capacity);
    sem_init(&lot.waiting_queue, 0, waiting_capacity);
    lot.waiting_count = 0; 
}


// Destroy semaphores
void destroyParkingLot(ParkingLot &lot) {
    sem_destroy(&lot.spots);
    sem_destroy(&lot.waiting_queue);
}

// Vehicle tries to enter parking lot immediately
bool enterParkingLot(ParkingLot &lot, int vehicleId) {
    // Spot available
    if(sem_trywait(&lot.spots) == 0) {
        return true;
    }

    // spot not available
    return false;
}

// Vehicle leaves parking lot
void leaveParkingLot(ParkingLot &lot, int vehicleId) {
    // Free up a spot
    sem_post(&lot.spots); 
}

