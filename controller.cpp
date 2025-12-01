#include "controller.h"
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <mutex>

using namespace std;

// External mutex for synchronized output
extern mutex event_mutex;

/**************************
Controller process function
**************************/
void runController(int intersectionId, int readPipe, int ackPipe)
{
    // Initial state: GREEN
    bool green = true;
    const int greenDuration = 3;
    const int redDuration = 5;
    bool lastAnnouncedState = true;

    while (true)
    {
        if (green != lastAnnouncedState) {
            // Announce state change
            lock_guard<mutex> lock(event_mutex);
            
            if (green){
                cout << "[Controller " << intersectionId << "] >>> SIGNAL NOW GREEN <<<" << endl;
            
            }
            
            else{
                cout << "[Controller " << intersectionId << "] >>> SIGNAL NOW RED <<<" << endl;
            }
            
            lastAnnouncedState = green;
        }

        // Announce the current phase with duration
        {
            lock_guard<mutex> lock(event_mutex);
            cout << "[Controller " << intersectionId << "] Signal -> ";
            
            // print color
            if(green){
                cout << "GREEN";
            }

            else{
                cout << "RED";
            }

            // print duration
            cout << " for ";
            if (green){
                cout << greenDuration;
            }

            else{
                cout << redDuration;
            }

            cout << " seconds" << endl;
        }
        
        // assign duration based on current color
        int duration;
        if (green){
            duration = greenDuration;
        }

        else{
            duration = redDuration;
        }

        bool emergencyThisPhase = false;

        // Loop for the duration of the current phase
        for (int sec = 0; sec < duration; ++sec)
        {   
            // wait for 1 second
            usleep(1000000); 

            // Send acknowledgment during GREEN so vehicles know they can go
            if (green){
                write(ackPipe, "A", 1);
            }

            // Check for incoming commands (E or Q)
            char buf[32];
            int n;
            bool gotQuit = false;

            while ((n = read(readPipe, buf, sizeof(buf))) > 0)
            {
                for (int i = 0; i < n; ++i)
                { 
                    // Emergency or Quit command
                    if (buf[i] == 'E'){
                        emergencyThisPhase = true;
                    }

                    if (buf[i] == 'Q'){
                        gotQuit = true;
                    }
                }
            }

            // Shutdown if Quit received
            if (gotQuit)
            {
                lock_guard<mutex> lock(event_mutex);
                cout << "[Controller " << intersectionId
                     << "] Received QUIT command. Shutting down.\n";
                return;
            }

            // If emergency arrives during RED, then force GREEN immediately
            if (emergencyThisPhase && !green)
            {
                green = true;

                lock_guard<mutex> lock(event_mutex);
                cout << "[Controller " << intersectionId
                     << "] EMERGENCY DETECTED! Forcing GREEN now!\n";
                cout << "[Controller " << intersectionId
                     << "] >>> SIGNAL NOW GREEN <<<" << endl;

                write(ackPipe, "A", 1);
                // exit loop and restart with GREEN
                break;
            }
        }

        // Only switch to opposite color if no emergency happened
        if (!emergencyThisPhase)
        {
            if (green){
                green = false;
            }

            else{
                green = true;
            }
        }
        // If emergency occurred, stay GREEN and restart loop
    }
}
