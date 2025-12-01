#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <string>

using namespace std;

/**********************
Intersection ID Mapping
**********************/
const int INT_F10 = 10;
const int INT_F11 = 11;

/**********************
Traffic Signal States (Boolean)
**********************/
const bool RED   = false;   // stop
const bool GREEN = true;    // go

/**********************
Controller Function
**********************/
void runController(int intersectionID, int readPipe, int writePipe);

#endif

