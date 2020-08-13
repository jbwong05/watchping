#include "ping.h"
#include "watch.h"

int main(int argc, char *argv[]) {
    int currentIndex = 1;
    char command[1024] = "ping";
    char *currentArg;
    while(currentIndex < argc) {
        currentArg = argv[currentIndex];
        strncat(command, " ", 1);
        strncat(command, currentArg, strlen(currentArg));
        currentIndex++;
    }

    struct ping_setup_data pingSetupData;
    ping_initialize(argc, argv, &pingSetupData);

    return start_watch(&pingSetupData, command);
}