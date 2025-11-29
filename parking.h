#ifndef PARKING_H
#define PARKING_H

#include <semaphore.h>
#include <iostream>
#include <unistd.h>

using namespace std;


struct ParkingLot {
    int id;                 
    sem_t spots;            
    sem_t waitingQueue;     
};

void initParkingLot(ParkingLot &lot, int id, int totalSpots, int waitingSlots);

bool enterParkingLot(ParkingLot &lot, int vehicleID);

void leaveParkingLot(ParkingLot &lot, int vehicleID);

void destroyParkingLot(ParkingLot &lot);

#endif
