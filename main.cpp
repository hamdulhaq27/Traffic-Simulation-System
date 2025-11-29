/***********************************************************************************
------------------------------------------------------------------------------------
                                OS Project Fall 2025
                             Traffic Simulation System
------------------------------------------------------------------------------------                                 
        Group Member#          Group Member Name         Group Member Roll#
            1                     Hamd-Ul-Haq                 23i-0081
            2                     Haider Abbas                23i-2558
------------------------------------------------------------------------------------
***********************************************************************************/

/***************************
Importing Required Libraries
***************************/
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <mutex>
#include <string>
#include <ctime>
#include <cstring>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#include <sys/stat.h>
#include "controller.h"
#include "vehicle.h"
#include "parking.h"

using namespace std;

// Parking lots
ParkingLot F10Lot, F11Lot;

// Mutex for console output (to prevent jumbled prints)
mutex cout_mutex;

// Vehicle thread function
void* vehicleThread(void* arg) {
    Vehicle* v = (Vehicle*)arg;
    sleep(v->arrival_time); // simulate arrival delay

    // Print arrival
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "[Vehicle " << v->id << "] Type: " << v->type
             << ", Origin: " << v->origin
             << ", Destination: " << v->destination
             << ", Priority: " << v->priority
             << " has arrived." << endl;
    }

    // Determine which parking lot to use
    ParkingLot* lot = (v->origin == INT_F10) ? &F10Lot : &F11Lot;

    // Simulate parking for non-emergency vehicles
    if(v->type != AMBULANCE && v->type != FIRETRUCK) {
        if(sem_trywait(&lot->waitingQueue) == 0) {
            // Wait for parking spot
            sem_wait(&lot->spots);
            // Got parking, leave waiting queue
            sem_post(&lot->waitingQueue);

            // Simulate parking time
            sleep(1);
            {
                lock_guard<mutex> lock(cout_mutex);
                cout << "[Vehicle " << v->id << "] Parked at intersection " << lot->id << endl;
            }

            sem_post(&lot->spots); // leave parking spot
        } else {
            // Waiting queue full
            {
                lock_guard<mutex> lock(cout_mutex);
                cout << "[Vehicle " << v->id << "] Waiting queue full, skipping parking." << endl;
            }
        }
    }

    // Simulate intersection crossing
    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "[Vehicle " << v->id << "] Crossing intersection " << v->origin
             << " towards destination " << v->destination << endl;
    }

    pthread_exit(NULL);
}

int main() {

    // Initialize parking lots
    initParkingLot(F10Lot, INT_F10, 10, 5);
    initParkingLot(F11Lot, INT_F11, 10, 5);

    cout << "------------ Traffic Simulation System ------------" << endl;
    cout << "Group Member 1: Hamd-Ul-Haq (23i-0081)" << endl;
    cout << "Group Member 2: Haider Abbas (23i-2558)" << endl;
    cout << "---------------------------------------------------" << endl;

    // Fork controller processes
    pid_t pid_F10 = fork();
    if(pid_F10 == 0){
        cout << "F10 Controller started..." << endl;
        //runController(INT_F10, 0, 0);
        exit(0);
    }

    pid_t pid_F11 = fork();
    if(pid_F11 == 0){
        cout << "F11 Controller started..." << endl;
        //runController(INT_F11, 0, 0);
        exit(0);
    }

    // Seed random generator
    srand(time(0));

    // Create vehicle threads
    const int NUM_VEHICLES = 15;
    Vehicle vehicles[NUM_VEHICLES];
    pthread_t threads[NUM_VEHICLES];

    for(int i=0; i<NUM_VEHICLES; i++){
        vehicles[i].id = i+1;
        vehicles[i].type = rand()%6 + 1; // 1-6
        vehicles[i].origin = (rand()%2==0) ? INT_F10 : INT_F11;
        vehicles[i].destination = rand()%3 + 1; // 1-3
        vehicles[i].priority = (vehicles[i].type <= 2) ? PRIORITY_HIGH :
                               (vehicles[i].type == 3) ? PRIORITY_MEDIUM : PRIORITY_LOW;
        vehicles[i].arrival_time = rand()%5; // 0-4 sec

        pthread_create(&threads[i], NULL, vehicleThread, (void*)&vehicles[i]);
    }

    // Wait for threads
    for(int i=0; i<NUM_VEHICLES; i++){
        pthread_join(threads[i], NULL);
    }

    // Wait for child processes to finish
    waitpid(pid_F10, NULL, 0);
    waitpid(pid_F11, NULL, 0);

    // Cleanup parking lot semaphores
    destroyParkingLot(F10Lot);
    destroyParkingLot(F11Lot);

    cout << "------------ Simulation Complete ------------" << endl;

    return 0;
}