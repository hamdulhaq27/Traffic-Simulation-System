#ifndef PARKING_H
#define PARKING_H

#include <semaphore.h>
#include <iostream>
#include <unistd.h>

using namespace std;

struct ParkingLot {
    int id;
    int capacity;          
    int waiting_capacity;  
    sem_t spots;           
    sem_t waiting_queue;   
    int waiting_count;     
};

void initParkingLot(ParkingLot &lot, int id, int capacity, int waiting_capacity);
void destroyParkingLot(ParkingLot &lot);
bool enterParkingLot(ParkingLot &lot, int vehicleId);
void leaveParkingLot(ParkingLot &lot, int vehicleId);


#endif
