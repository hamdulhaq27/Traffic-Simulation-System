#ifndef VEHICLE_H
#define VEHICLE_H

/**********************
Vehicle Types
**********************/
const int CAR = 1;
const int AMBULANCE = 2;
const int BIKE = 3;
const int BUS = 4;
const int TRUCK = 5;
const int FIRETRUCK = 6;
const int TRACTOR = 7;

/**********************
Vehicle Priority Levels
**********************/
const int PRIORITY_HIGH = 3;
const int PRIORITY_MEDIUM = 2;
const int PRIORITY_LOW = 1;

/**********************
Vehicle Structure
**********************/
struct Vehicle {
    int id;
    int type;
    int origin;
    int destination;
    int priority;
    int arrival_time;
};

#endif