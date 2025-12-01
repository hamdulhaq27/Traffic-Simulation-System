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

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <mutex>
#include <string>
#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "controller.h"
#include "vehicle.h"
#include "parking.h"

using namespace std;

// Global parking lots
ParkingLot F10Lot, F11Lot;

// For neat, unjumbled console output
mutex cout_mutex;

/*****************************************
Vehicle Thread Function
******************************************/
void* vehicleThread(void* arg) {
    Vehicle* v = (Vehicle*)arg;
    sleep(v->arrival_time);

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "[Vehicle " << v->id << "] Arrived | Type: " << v->type
             << ", Origin: " << v->origin
             << ", Dest: " << v->destination
             << ", Priority: " << v->priority << endl;
    }

    ParkingLot* lot = (v->origin == INT_F10) ? &F10Lot : &F11Lot;

    // Emergency vehicles skip parking
    if(v->priority == PRIORITY_LOW) {
        if(enterParkingLot(*lot, v->id)) {
            sleep(1); // pretend parking
            leaveParkingLot(*lot, v->id);
        }
    }

    {
        lock_guard<mutex> lock(cout_mutex);
        cout << "[Vehicle " << v->id << "] Crossing intersection " << v->origin
             << " -> movement " << v->destination << endl;
    }

    pthread_exit(NULL);
}


/*****************************************
Main Function
******************************************/
int main() {

    cout << "\n============ Traffic Simulation System ============\n";
    cout << "Group Member 1: Hamd-Ul-Haq (23i-0081)\n";
    cout << "Group Member 2: Haider Abbas (23i-2558)\n";
    cout << "===================================================\n\n";

    /*****************************************
    Initialize Parking Lots
    ******************************************/
    initParkingLot(F10Lot, INT_F10, 10, 5);
    initParkingLot(F11Lot, INT_F11, 10, 5);


    /*****************************************
    Create Pipes for Controllers
    ******************************************/
    int F10_pipe[2];
    int F11_pipe[2];

    pipe(F10_pipe);
    pipe(F11_pipe);


    /*****************************************
    Fork Controller for F10
    ******************************************/
    pid_t pidF10 = fork();
    if(pidF10 == 0) {
        close(F10_pipe[1]); // child reads only
        runController(INT_F10, F10_pipe[0], F10_pipe[0]);
        exit(0);
    }

    /*****************************************
    Fork Controller for F11
    ******************************************/
    pid_t pidF11 = fork();
    if(pidF11 == 0) {
        close(F11_pipe[1]); // child reads only
        runController(INT_F11, F11_pipe[0], F11_pipe[0]);
        exit(0);
    }


    /*****************************************
    Create Vehicle Threads
    ******************************************/
    srand(time(NULL));
    const int NUM_VEHICLES = 15;

    Vehicle vehicles[NUM_VEHICLES];
    pthread_t threads[NUM_VEHICLES];

    for(int i = 0; i < NUM_VEHICLES; i++){
        vehicles[i].id = i + 1;
        vehicles[i].type = rand() % 6 + 1;
        vehicles[i].origin = (rand() % 2 == 0) ? INT_F10 : INT_F11;
        vehicles[i].destination = rand() % 3 + 1;

        vehicles[i].priority =
            (vehicles[i].type == AMBULANCE || vehicles[i].type == FIRETRUCK) ? PRIORITY_HIGH :
            (vehicles[i].type == BUS) ? PRIORITY_MEDIUM : PRIORITY_LOW;

        vehicles[i].arrival_time = rand() % 5;

        pthread_create(&threads[i], NULL, vehicleThread, &vehicles[i]);
    }

    // Wait for all vehicle threads
    for(int i = 0; i < NUM_VEHICLES; i++){
        pthread_join(threads[i], NULL);
    }


    /*****************************************
    Cleanup
    ******************************************/
    waitpid(pidF10, NULL, 0);
    waitpid(pidF11, NULL, 0);

    destroyParkingLot(F10Lot);
    destroyParkingLot(F11Lot);

    cout << "\n============ Simulation Complete ============\n";

    return 0;
}
