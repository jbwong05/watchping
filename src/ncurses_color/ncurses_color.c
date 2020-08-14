#include "ncurses_color.h"

#define BRIGHT_WHITE 15
#define BRIGHT_GREEN 10
#define BRIGHT_YELLOW 11
#define BRIGHT_RED 9

void initialize_colors() {
    if(has_colors()){
		start_color();
        init_pair(NORMAL_COLOR_INDEX, BRIGHT_WHITE, COLOR_BLACK);
        init_pair(LOW_COLOR_INDEX, BRIGHT_GREEN, COLOR_BLACK);
        init_pair(MEDIUM_COLOR_INDEX, BRIGHT_YELLOW, COLOR_BLACK);
        init_pair(HIGH_COLOR_INDEX, BRIGHT_RED, COLOR_BLACK);
	}
}

void set_color(int index) {
    if(index == NORMAL_COLOR_INDEX || index == LOW_COLOR_INDEX || index == MEDIUM_COLOR_INDEX || index == HIGH_COLOR_INDEX) {
        if(current_color) {
            attroff(COLOR_PAIR(current_color));
        }

        attron(COLOR_PAIR(index));
        current_color = index;
    }
}

void set_ping_color(long timeWhole) {
    if(timeWhole < MEDIUM_PING_MIN) {
        set_color(LOW_COLOR_INDEX);
    } else if(timeWhole < HIGH_PING_MIN) {
        set_color(MEDIUM_COLOR_INDEX);
    } else {
        set_color(HIGH_COLOR_INDEX);
    }
}

void set_packet_loss_color(float packet_loss) {
    if(packet_loss < MEDIUM_PACKET_LOSS_MIN) {
        set_color(LOW_COLOR_INDEX);
    } else if(packet_loss < HIGH_PACKET_LOSS_MIN) {
        set_color(MEDIUM_COLOR_INDEX);
    } else {
        set_color(HIGH_COLOR_INDEX);
    }
}

void set_deviation_color(long deviation) {
    if(deviation < MEDIUM_DEVIATION_MIN) {
        set_color(LOW_COLOR_INDEX);
    } else if(deviation < HIGH_DEVIATION_MIN) {
        set_color(MEDIUM_COLOR_INDEX);
    } else {
        set_color(HIGH_COLOR_INDEX);
    }
}