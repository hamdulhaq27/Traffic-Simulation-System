/********************
Libraries and Headers
********************/
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <mutex>
#include <string>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "controller.h"
#include "vehicle.h"
#include "parking.h"

using namespace std;

// Intersection IDs
#define INT_F10 10
#define INT_F11 11

// Global parking lots
ParkingLot F10Lot, F11Lot;

// Mutexes for intersection safety, one vehicle at a time
mutex F10_mutex;
mutex F11_mutex;

// Single mutex for all console output
mutex event_mutex;

/***************************************************************
TWO PIPES PER CONTROLLER
cmd_pipe:  parent to controller  (send 'E' or 'Q')
ack_pipe:  controller to parent  (send 'A' when green is active)
***************************************************************/
int F10_cmd_pipe[2];   
int F10_ack_pipe[2]; 
int F11_cmd_pipe[2];
int F11_ack_pipe[2];

// Graceful shutdown bool
bool simulation_running=true;

/************************
Signal handler for Ctrl+C
************************/
void signalHandler(int signum) {
    destroyParkingLot(F10Lot);
    destroyParkingLot(F11Lot);
    event_mutex.lock();
    cout << "\nSimulation terminated by user (Ctrl+C)\n";
    event_mutex.unlock();
    exit(signum);
}

/***********************
String mapping functions
***********************/
string getVehicleTypeName(int type) {

    // Vehicle Types mapped to names
    switch(type){
        case CAR: 
            return "Car";
        case AMBULANCE: 
            return "Ambulance";
        case BIKE: 
            return "Bike";
        case BUS: 
            return "Bus";
        case TRUCK: 
            return "Truck";
        case FIRETRUCK: 
            return "Firetruck";
        case TRACTOR: 
            return "Tractor";
        default: 
            return "Unknown";
    }
}

string getMovementName(int move) {

    // Movement Types mapped to names
    switch(move) {
        case TURN_LEFT: 
            return "TURN_LEFT";
        case TURN_RIGHT: 
            return "TURN_RIGHT";
        case GO_STRAIGHT: 
            return "GO_STRAIGHT";
        default: 
            return "UNKNOWN";
    }
}

// Vehicle thread function
void* vehicleThread(void* arg){
    Vehicle* v=(Vehicle*)arg;
    sleep(v->arrival_time);

    // Vehicle arrival event
    event_mutex.lock();
    cout << "[Vehicle " << v->id << "] Arrived | Type: " << getVehicleTypeName(v->type)
         << ", Origin: F" << v->origin << ", Dest: " << getMovementName(v->destination)
         << ", Priority: " << v->priority << endl;
    event_mutex.unlock();

    ParkingLot* lot;
    mutex* inter_mutex;
    int cmd_pipe_fd;
    int ack_pipe_fd;

    if (v->origin == INT_F10)
    {
        lot = &F10Lot;
        inter_mutex = &F10_mutex;
        cmd_pipe_fd = F10_cmd_pipe[1];   
        ack_pipe_fd = F10_ack_pipe[0];  
    }

    else  
    {
        lot = &F11Lot;
        inter_mutex = &F11_mutex;
        cmd_pipe_fd = F11_cmd_pipe[1];
        ack_pipe_fd = F11_ack_pipe[0];
    }

    // HIGH PRIORITY: Emergency vehicles force green
    if (v->priority == PRIORITY_HIGH) {

        // Send 'E' to controller to request immediate green
        write(cmd_pipe_fd, "E", 1);
        event_mutex.lock();
        cout << "[Vehicle " << v->id << "] EMERGENCY! Requesting immediate GREEN from Controller " << v->origin << endl;
        event_mutex.unlock();

        // Wait for acknowledgment 'A', meaning green is granted
        char ack;
        while (read(ack_pipe_fd, &ack, 1) <= 0) {
            usleep(50000);
        }

        event_mutex.lock();
        cout << "[Vehicle " << v->id << "] GREEN confirmed by controller! Proceeding..." << endl;
        event_mutex.unlock();
    } 
    // NORMAL VEHICLES: Wait for natural green phase
    else {
        // Non-emergency vehicles must park first if not emergency
        sem_wait(&lot->spots);

        int spotsLeft;
        sem_getvalue(&lot->spots, &spotsLeft);

        // Vehicle parked event
        event_mutex.lock();
        cout << "[Vehicle " << v->id << "] PARKED at ParkingLot F" << lot->id
             << " | Spots left: " << spotsLeft << endl;
        event_mutex.unlock();

        // Parking duration
        sleep(2); 

        leaveParkingLot(*lot, v->id);
        sem_getvalue(&lot->spots, &spotsLeft);

        // Vehicle leaving parking event
        event_mutex.lock();
        cout << "[Vehicle " << v->id << "] LEFT ParkingLot F" << lot->id
             << " | Spots left: " << spotsLeft << endl;
        event_mutex.unlock();

        // Now wait for green light (controller sends 'A' every second it's green)
        event_mutex.lock();
        cout << "[Vehicle " << v->id << "] Waiting for GREEN light at F" << v->origin << "..." << endl;
        event_mutex.unlock();

        char ack;
        while (read(ack_pipe_fd, &ack, 1) <= 0) {
            usleep(100000);
        }

        // Green light received
        event_mutex.lock();
        cout << "[Vehicle " << v->id << "] Light is GREEN! Proceeding to cross..." << endl;
        event_mutex.unlock();
    }

    // CROSSING INTERSECTION
    inter_mutex->lock();
    event_mutex.lock();
    cout << "[Vehicle " << v->id << "] CROSSING intersection F" << v->origin
         << " -> " << getMovementName(v->destination) << endl;
    event_mutex.unlock();

    // crossing time
    usleep(700000); 

    inter_mutex->unlock();
    event_mutex.lock();
    cout << "[Vehicle " << v->id << "] COMPLETED crossing F" << v->origin << endl;
    event_mutex.unlock();

    pthread_exit(NULL);
}

int main() {

    // Register signal handler for Ctrl+C
    signal(SIGINT, signalHandler);

    event_mutex.lock();
    cout << "\n============ Traffic Simulation System ============\n";
    cout << "Group Member 1: Hamd-Ul-Haq (23i-0081)\n";
    cout << "Group Member 2: Haider Abbas (23i-2558)\n";
    cout << "==================================================\n\n";
    event_mutex.unlock();

    // Initialize parking lots
    initParkingLot(F10Lot, INT_F10, 10, 5);
    initParkingLot(F11Lot, INT_F11, 10, 5);

    event_mutex.lock();
    cout << "[System] Parking Lots initialized (10 spots each, 5 waiting)\n\n";
    event_mutex.unlock();

    // CREATE PIPES
    pipe(F10_cmd_pipe); 
    pipe(F10_ack_pipe);
    pipe(F11_cmd_pipe); 
    pipe(F11_ack_pipe);

    // Make read ends non-blocking
    fcntl(F10_cmd_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(F11_cmd_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(F10_ack_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(F11_ack_pipe[0], F_SETFL, O_NONBLOCK);

    // FORK CONTROLLERS
    pid_t pidF10 = fork();
    if (pidF10 == 0) {
        close(F10_cmd_pipe[1]);
        close(F10_ack_pipe[0]);
        runController(INT_F10, F10_cmd_pipe[0], F10_ack_pipe[1]);
        exit(0);
    }

    pid_t pidF11 = fork();
    if (pidF11 == 0) {
        close(F11_cmd_pipe[1]);
        close(F11_ack_pipe[0]);
        runController(INT_F11, F11_cmd_pipe[0], F11_ack_pipe[1]);
        exit(0);
    }

    // Parent closes unused ends
    close(F10_cmd_pipe[0]); close(F10_ack_pipe[1]);
    close(F11_cmd_pipe[0]); close(F11_ack_pipe[1]);

    // Allow controllers to start
    usleep(200000);

    event_mutex.lock();
    cout << "[System] Traffic Controllers started for F10 and F11\n\n";
    event_mutex.unlock();

    // GENERATE VEHICLES
    const int NUM_VEHICLES = 15;
    Vehicle vehicles[NUM_VEHICLES];
    pthread_t threads[NUM_VEHICLES];

    srand(time(NULL));

    event_mutex.lock();
    cout << "============ Vehicle Generation ============\n";
    event_mutex.unlock();

    // Create vehicles with random attributes
    for (int i = 0; i < NUM_VEHICLES; i++)
    {
        vehicles[i].id = i + 1;
        vehicles[i].type = rand() % 7 + 1;

        // Origin: F10 or F11
        if (rand() % 2 == 0){
            vehicles[i].origin = INT_F10;
        }

        else{
            vehicles[i].origin = INT_F11;
        }

        // Destination: Left, Right, or Straight
        vehicles[i].destination = rand() % 3 + 1;

        // Priority mapping
        if (vehicles[i].type == AMBULANCE || vehicles[i].type == FIRETRUCK)
        {
            vehicles[i].priority = PRIORITY_HIGH;
        }
        else if (vehicles[i].type == BUS)
        {
            vehicles[i].priority = PRIORITY_MEDIUM;
        }
        else
        {
            vehicles[i].priority = PRIORITY_LOW;
        }

        // Arrival time
        vehicles[i].arrival_time = rand() % 6 + 1;

        // Print scheduled vehicle info (thread-safe)
        event_mutex.lock();
        cout << "[System] Scheduled Vehicle " << vehicles[i].id << ": "
             << getVehicleTypeName(vehicles[i].type)
             << " | Origin F" << vehicles[i].origin
             << " | Arrival in " << vehicles[i].arrival_time << "s\n";
        event_mutex.unlock();

        // Create vehicle thread
        pthread_create(&threads[i], NULL, vehicleThread, &vehicles[i]);
    }

    event_mutex.lock();
    cout << "\n============ Simulation Running ============\n\n";
    event_mutex.unlock();

    // Wait for all vehicles
    for (int i = 0; i < NUM_VEHICLES; i++) {
        pthread_join(threads[i], NULL);
    }

    // SHUTDOWN CONTROLLERS
    write(F10_cmd_pipe[1], "Q", 1);
    write(F11_cmd_pipe[1], "Q", 1);

    // Allow time for controllers to process quit
    usleep(200000);
    waitpid(pidF10, NULL, 0);
    waitpid(pidF11, NULL, 0);

    // Cleanup parking lots
    destroyParkingLot(F10Lot);
    destroyParkingLot(F11Lot);

    // Final simulation complete message
    event_mutex.lock();
    cout << "\n============ Simulation Complete ============\n";
    cout << "All vehicles processed. Controllers terminated gracefully.\n";
    event_mutex.unlock();

    return 0;
}