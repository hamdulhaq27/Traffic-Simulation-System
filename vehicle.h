#ifndef VEHICLE_H
#define VEHICLE_H

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

/*******************
Vehicle Type Mapping
*******************/
const int AMBULANCE=1;
const int FIRETRUCK=2;
const int BUS=3;
const int CAR=4;
const int BIKE=5;
const int TRACTOR=6;

/***************
Movement Mapping
***************/
const int TURN_LEFT=1;
const int TURN_RIGHT=2;
const int GO_STRAIGHT=3;

/*********
Priorities
*********/
const int PRIORITY_HIGH=2;
const int PRIORITY_MEDIUM=1;
const int PRIORITY_LOW=0;

/****************
Vehicle Structure
****************/
struct Vehicle{
    int id;
    int type;         
    int origin;      
    int destination;  
    int priority;     
    int arrival_time; 
};

void* vehicleThread(void* arg);

#endif
