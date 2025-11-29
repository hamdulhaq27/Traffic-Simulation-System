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
#include <pthread>
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
#include "controller.h"

using namespace std;

// Parking semaphores (global for simplicity)
sem_t parking_spots_F10;
sem_t waiting_queue_F10;
sem_t parking_spots_F11;
sem_t waiting_queue_F11;

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

    // Simulate parking for non-emergency vehicles
    if(v->type != AMBULANCE && v->type != FIRETRUCK) {
        sem_t* parking_spots = (v->origin == INT_F10) ? &parking_spots_F10 : &parking_spots_F11;
        sem_t* waiting_queue = (v->origin == INT_F10) ? &waiting_queue_F10 : &waiting_queue_F11;

        // Reserve waiting slot
        if(sem_trywait(waiting_queue) == 0) {
            // Wait for parking spot
            sem_wait(parking_spots);
            // Got parking, leave waiting queue
            sem_post(waiting_queue);

            // Simulate parking time
            sleep(1);
            {
                lock_guard<mutex> lock(cout_mutex);
                cout << "[Vehicle " << v->id << "] Parked at intersection " << v->origin << endl;
            }

            sem_post(parking_spots); // leave parking spot
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

int main(){

    // Initialize semaphores
    sem_init(&parking_spots_F10, 0, 10); // 10 parking spots
    sem_init(&waiting_queue_F10, 0, 5);  // 5 waiting queue slots
    sem_init(&parking_spots_F11, 0, 10);
    sem_init(&waiting_queue_F11, 0, 5);

    cout << "------------ Traffic Simulation System ------------" << endl;
    cout << "Group Member 1: Hamd-Ul-Haq (23i-0081)" << endl;
    cout << "Group Member 2: Haider Abbas (23i-2558)"<<endl;
    cout << "---------------------------------------------------" << endl;

    // Fork controller processes
    pid_t pid_F10 = fork();
    if(pid_F10 == 0){
        cout << "F10 Controller started..." << endl;
        exit(0);
    }

    pid_t pid_F11 = fork();
    if(pid_F11 == 0){
        cout << "F11 Controller started..." << endl;
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
        vehicles[i].priority = (vehicles[i].type <=2) ? PRIORITY_HIGH :
                               (vehicles[i].type==3) ? PRIORITY_MEDIUM : PRIORITY_LOW;
        vehicles[i].arrival_time = rand()%5; // 0-4 sec

        pthread_create(&threads[i], NULL, vehicleThread, (void*)&vehicles[i]);
    }

    // Wait for threads
    for(int i=0; i<NUM_VEHICLES; i++){
        pthread_join(threads[i], NULL);
    }

    // Cleanup semaphores
    sem_destroy(&parking_spots_F10);
    sem_destroy(&waiting_queue_F10);
    sem_destroy(&parking_spots_F11);
    sem_destroy(&waiting_queue_F11);

    return 0;
}