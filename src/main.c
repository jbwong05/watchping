#include "ping.h"
#include "watch.h"

int main(int argc, char *argv[]) {
    struct ping_setup_data pingSetupData;
    ping_initialize(argc, argv, &pingSetupData);
    return start_watch(&pingSetupData);
}