#include <ncurses.h>

#define MEDIUM_PING_MIN 60
#define HIGH_PING_MIN 100

#define MEDIUM_PACKET_LOSS_MIN 3
#define HIGH_PACKET_LOSS_MIN 5

#define MEDIUM_DEVIATION_MIN 6
#define HIGH_DEVIATION_MIN 11

#define NORMAL_COLOR_INDEX 1
#define LOW_COLOR_INDEX 2
#define MEDIUM_COLOR_INDEX 3
#define HIGH_COLOR_INDEX 4

static int current_color;

void initialize_colors();
void set_color();
void set_ping_color(long timeWhole);
void set_packet_loss_color(float packet_loss);
void set_deviation_color(long deviation);