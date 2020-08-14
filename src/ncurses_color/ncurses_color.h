#include <ncurses.h>

#define MEDIUM_MIN 50
#define HIGH_MIN 100

#define BRIGHT_WHITE 15
#define BRIGHT_GREEN 10
#define BRIGHT_YELLOW 14
#define BRIGHT_RED 12

#define NORMAL_COLOR_INDEX 1
#define LOW_COLOR_INDEX 2
#define MEDIUM_COLOR_INDEX 3
#define HIGH_COLOR_INDEX 4

static int current_color;

void initialize_colors();
void set_color();
void set_ping_color(long timeWhole);