#include "controller.h"

void runController(int intersectionID, int readPipe, int writePipe) {
    bool signalState = GREEN;   // start with green
    const int GREEN_TIME = 3;   // seconds green
    const int RED_TIME   = 3;   // seconds red

    while (true) {
        // --- Set current signal state ---
        if (signalState == GREEN) {
            string msg = "GREEN";
            write(writePipe, msg.c_str(), msg.size());
            cout << "[Controller " << intersectionID << "] Signal -> GREEN" << endl;

            sleep(GREEN_TIME);
            signalState = RED;     // flip
        }
        else {
            string msg = "RED";
            write(writePipe, msg.c_str(), msg.size());
            cout << "[Controller " << intersectionID << "] Signal -> RED" << endl;

            sleep(RED_TIME);
            signalState = GREEN;   // flip
        }

        // --- Check incoming messages (non-blocking) ---
        char buffer[64];
        int bytesRead = read(readPipe, buffer, sizeof(buffer)-1);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            cout << "[Controller " << intersectionID 
                 << "] Received: " << buffer << endl;
        }
    }
}
