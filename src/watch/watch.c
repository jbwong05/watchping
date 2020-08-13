/*
 * watch -- execute a program repeatedly, displaying output fullscreen
 *
 * Based on the original 1991 'watch' by Tony Rems <rembo@unisoft.com>
 * (with mods and corrections by Francois Pinard).
 *
 * Substantially reworked, new features (differences option, SIGWINCH
 * handling, unlimited command length, long line handling) added Apr
 * 1999 by Mike Coleman <mkc@acm.org>.
 *
 * Changes by Albert Cahalan, 2002-2003.
 * stderr handling, exec, and beep option added by Morty Abzug, 2008
 * Unicode Support added by Jarrod Lowe <procps@rrod.net> in 2009.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "c.h"
#include "config.h"
#include "fileutils.h"
#include "nls.h"
#include "ping.h"
#include "strutils.h"
#include "xalloc.h"
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <locale.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "watch.h"
#ifdef WITH_WATCH8BIT
# define _XOPEN_SOURCE_EXTENDED 1
# include <wchar.h>
# include <wctype.h>
#endif	/* WITH_WATCH8BIT */
#include <ncurses.h>

#ifdef FORCE_8BIT
# undef isprint
# define isprint(x) ( (x>=' '&&x<='~') || (x>=0xa0) )
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/* Boolean command line options */
static int flags;
#define WATCH_DIFF	(1 << 1)
#define WATCH_CUMUL	(1 << 2)
#define WATCH_EXEC	(1 << 3)
#define WATCH_BEEP	(1 << 4)
#define WATCH_COLOR	(1 << 5)
#define WATCH_ERREXIT	(1 << 6)
#define WATCH_CHGEXIT	(1 << 7)

static int curses_started = 0;
static long height = 24, width = 80;
static int screen_size_changed = 0;
static int first_screen = 1;
static int show_title = 2;	/* number of lines used, 2 or 0 */
static int precise_timekeeping = 0;

#define min(x,y) ((x) > (y) ? (y) : (x))
#define MAX_ANSIBUF 100

static void __attribute__ ((__noreturn__))
    usage(FILE * out)
{
	fputs(USAGE_HEADER, out);
	fprintf(out,
              _(" %s [options] command\n"), program_invocation_short_name);
	fputs(USAGE_OPTIONS, out);
	fputs(_("  -b, --beep             beep if command has a non-zero exit\n"), out);
	fputs(_("  -c, --color            interpret ANSI color and style sequences\n"), out);
	fputs(_("  -d, --differences[=<permanent>]\n"
                "                         highlight changes between updates\n"), out);
	fputs(_("  -e, --errexit          exit if command has a non-zero exit\n"), out);
	fputs(_("  -g, --chgexit          exit when output from command changes\n"), out);
	fputs(_("  -n, --interval <secs>  seconds to wait between updates\n"), out);
	fputs(_("  -p, --precise          attempt run command in precise intervals\n"), out);
	fputs(_("  -t, --no-title         turn off header\n"), out);
	fputs(_("  -x, --exec             pass command to exec instead of \"sh -c\"\n"), out);
	fputs(USAGE_SEPARATOR, out);
	fputs(USAGE_HELP, out);
	fputs(_(" -v, --version  output version information and exit\n"), out);
	fprintf(out, USAGE_MAN_TAIL("watch(1)"));

	exit(out == stderr ? EXIT_FAILURE : EXIT_SUCCESS);
}

static int nr_of_colors;
static int attributes;
static int fg_col;
static int bg_col;

static ping_setup_data *pingSetupData = 0;

static void reset_ansi(void)
{
	attributes = A_NORMAL;
	fg_col = 0;
	bg_col = 0;
}

static void init_ansi_colors(void)
{
	short ncurses_colors[] = {
		-1, COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
		COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
	};

	nr_of_colors = sizeof(ncurses_colors) / sizeof(short);

	for (bg_col = 0; bg_col < nr_of_colors; bg_col++)
		for (fg_col = 0; fg_col < nr_of_colors; fg_col++)
			init_pair(bg_col * nr_of_colors + fg_col + 1, ncurses_colors[fg_col], ncurses_colors[bg_col]);
	reset_ansi();
}


static int set_ansi_attribute(const int attrib)
{
	switch (attrib) {
	case -1:	/* restore last settings */
		break;
	case 0:		/* restore default settings */
		reset_ansi();
		break;
	case 1:		/* set bold / increased intensity */
		attributes |= A_BOLD;
		break;
	case 2:		/* set decreased intensity (if supported) */
		attributes |= A_DIM;
		break;
#ifdef A_ITALIC
	case 3:		/* set italic (if supported) */
		attributes |= A_ITALIC;
		break;
#endif
	case 4:		/* set underline */
		attributes |= A_UNDERLINE;
		break;
	case 5:		/* set blinking */
		attributes |= A_BLINK;
		break;
	case 7:		/* set inversed */
		attributes |= A_REVERSE;
		break;
	case 21:	/* unset bold / increased intensity */
		attributes &= ~A_BOLD;
		break;
	case 22:	/* unset bold / any intensity modifier */
		attributes &= ~(A_BOLD | A_DIM);
		break;
#ifdef A_ITALIC
	case 23:	/* unset italic */
		attributes &= ~A_ITALIC;
		break;
#endif
	case 24:	/* unset underline */
		attributes &= ~A_UNDERLINE;
		break;
	case 25:	/* unset blinking */
		attributes &= ~A_BLINK;
		break;
	case 27:	/* unset inversed */
		attributes &= ~A_REVERSE;
		break;
    case 39:
        fg_col = 0;
        break;
    case 49:
        bg_col = 0;
        break;
	default:
		if (attrib >= 30 && attrib <= 37) {	/* set foreground color */
			fg_col = attrib - 30 + 1;
		} else if (attrib >= 40 && attrib <= 47) { /* set background color */
			bg_col = attrib - 40 + 1;
		} else {
			return 0; /* Not understood */
		}
	}
	attrset(attributes | COLOR_PAIR(bg_col * nr_of_colors + fg_col + 1));
    return 1;
}

static void process_ansi(FILE * fp)
{
	int i, c;
	char buf[MAX_ANSIBUF];
	char *numstart, *endptr;

	c = getc(fp);
	if (c != '[') {
		ungetc(c, fp);
		return;
	}
	for (i = 0; i < MAX_ANSIBUF; i++) {
		c = getc(fp);
		/* COLOUR SEQUENCE ENDS in 'm' */
		if (c == 'm') {
			buf[i] = '\0';
			break;
		}
		if ((c < '0' || c > '9') && c != ';') {
			return;
		}
		buf[i] = (char)c;
	}
	/*
	 * buf now contains a semicolon-separated list of decimal integers,
	 * each indicating an attribute to apply.
	 * For example, buf might contain "0;1;31", derived from the color
	 * escape sequence "<ESC>[0;1;31m". There can be 1 or more
	 * attributes to apply, but typically there are between 1 and 3.
	 */

    /* Special case of <ESC>[m */
    if (buf[0] == '\0')
        set_ansi_attribute(0);

    for (endptr = numstart = buf; *endptr != '\0'; numstart = endptr + 1) {
        if (!set_ansi_attribute(strtol(numstart, &endptr, 10)))
            break;
    }
}

static void __attribute__ ((__noreturn__)) do_exit(int status)
{
	if (curses_started)
		endwin();
	exit(status);
}

/* signal handler */
static void die(int notused __attribute__ ((__unused__)))
{
	if(pingSetupData) {
		cleanup(pingSetupData);
		free(pingSetupData);
	}
	do_exit(EXIT_SUCCESS);
}

static void winch_handler(int notused __attribute__ ((__unused__)))
{
	screen_size_changed = 1;
}

static char env_col_buf[24];
static char env_row_buf[24];
static int incoming_cols;
static int incoming_rows;

static void get_terminal_size(void)
{
	struct winsize w;
	if (!incoming_cols) {
		/* have we checked COLUMNS? */
		const char *s = getenv("COLUMNS");
		incoming_cols = -1;
		if (s && *s) {
			long t;
			char *endptr;
			t = strtol(s, &endptr, 0);
			if (!*endptr && 0 < t)
				incoming_cols = t;
			width = incoming_cols;
			snprintf(env_col_buf, sizeof env_col_buf, "COLUMNS=%ld",
				 width);
			putenv(env_col_buf);
		}
	}
	if (!incoming_rows) {
		/* have we checked LINES? */
		const char *s = getenv("LINES");
		incoming_rows = -1;
		if (s && *s) {
			long t;
			char *endptr;
			t = strtol(s, &endptr, 0);
			if (!*endptr && 0 < t)
				incoming_rows = t;
			height = incoming_rows;
			snprintf(env_row_buf, sizeof env_row_buf, "LINES=%ld",
				 height);
			putenv(env_row_buf);
		}
	}
	if (ioctl(STDERR_FILENO, TIOCGWINSZ, &w) == 0) {
		if (incoming_cols < 0 || incoming_rows < 0) {
			if (incoming_rows < 0 && w.ws_row > 0) {
				height = w.ws_row;
				snprintf(env_row_buf, sizeof env_row_buf,
					 "LINES=%ld", height);
				putenv(env_row_buf);
			}
			if (incoming_cols < 0 && w.ws_col > 0) {
				width = w.ws_col;
				snprintf(env_col_buf, sizeof env_col_buf,
					 "COLUMNS=%ld", width);
				putenv(env_col_buf);
			}
		}
	}
}

/* get current time in usec */
typedef unsigned long long watch_usec_t;
#define USECS_PER_SEC (1000000ull)
static watch_usec_t get_time_usec()
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return USECS_PER_SEC * now.tv_sec + now.tv_usec;
}

#ifdef WITH_WATCH8BIT
/* read a wide character from a popen'd stream */
#define MAX_ENC_BYTES 16
wint_t my_getwc(FILE * s);
wint_t my_getwc(FILE * s)
{
	/* assuming no encoding ever consumes more than 16 bytes */
	char i[MAX_ENC_BYTES];
	int byte = 0;
	int convert;
	int x;
	wchar_t rval;
	while (1) {
		i[byte] = getc(s);
		if (i[byte] == EOF) {
			return WEOF;
		}
		byte++;
		errno = 0;
		mbtowc(NULL, NULL, 0);
		convert = mbtowc(&rval, i, byte);
		x = errno;
		if (convert > 0) {
			/* legal conversion */
			return rval;
		}
		if (byte == MAX_ENC_BYTES) {
			while (byte > 1) {
				/* at least *try* to fix up */
				ungetc(i[--byte], s);
			}
			errno = -EILSEQ;
			return WEOF;
		}
	}
}
#endif	/* WITH_WATCH8BIT */

#ifdef WITH_WATCH8BIT
static void output_header(wchar_t *restrict wcommand, int wcommand_characters, double interval)
#else
static void output_header(char *restrict command, double interval)
#endif	/* WITH_WATCH8BIT */
{
	time_t t = time(NULL);
	char *ts = ctime(&t);
	char *header;
	char *right_header;
    int max_host_name_len = (int) sysconf(_SC_HOST_NAME_MAX);
	char hostname[max_host_name_len + 1];
	int command_columns = 0;	/* not including final \0 */

	gethostname(hostname, sizeof(hostname));

	/*
	 * left justify interval and command, right justify hostname and time,
	 * clipping all to fit window width
	 */
	int hlen = asprintf(&header, _("Every %.1fs: "), interval);
	int rhlen = asprintf(&right_header, _("%s: %s"), hostname, ts);

	/*
	 * the rules:
	 *   width < rhlen : print nothing
	 *   width < rhlen + hlen + 1: print hostname, ts
	 *   width = rhlen + hlen + 1: print header, hostname, ts
	 *   width < rhlen + hlen + 4: print header, ..., hostname, ts
	 *   width < rhlen + hlen + wcommand_columns: print header,
	 *                           truncated wcommand, ..., hostname, ts
	 *   width > "": print header, wcomand, hostname, ts
	 * this is slightly different from how it used to be
	 */
	if (width < rhlen) {
		free(header);
		free(right_header);
		return;
	}
	if (rhlen + hlen + 1 <= width) {
		mvaddstr(0, 0, header);
		if (rhlen + hlen + 2 <= width) {
			if (width < rhlen + hlen + 4) {
				mvaddstr(0, width - rhlen - 4, "... ");
			} else {
#ifdef WITH_WATCH8BIT
	            command_columns = wcswidth(wcommand, -1);
				if (width < rhlen + hlen + command_columns) {
					/* print truncated */
					int available = width - rhlen - hlen;
					int in_use = command_columns;
					int wcomm_len = wcommand_characters;
					while (available - 4 < in_use) {
						wcomm_len--;
						in_use = wcswidth(wcommand, wcomm_len);
					}
					mvaddnwstr(0, hlen, wcommand, wcomm_len);
					mvaddstr(0, width - rhlen - 4, "... ");
				} else {
					mvaddwstr(0, hlen, wcommand);
				}
#else
                command_columns = strlen(command);
                if (width < rhlen + hlen + command_columns) {
                    /* print truncated */
                    mvaddnstr(0, hlen, command, width - rhlen - hlen - 4);
                    mvaddstr(0, width - rhlen - 4, "... ");
                } else {
                    mvaddnstr(0, hlen, command, width - rhlen - hlen);
                }
#endif	/* WITH_WATCH8BIT */
			}
		}
	}
	mvaddstr(0, width - rhlen + 1, right_header);
	free(header);
	free(right_header);
	return;
}

int start_watch(struct ping_setup_data *pingSetupData, char *command) {
	int optc;
	double interval = 2;
	char *interval_string;
	char **command_argv;
	int command_length = 0;	/* not including final \0 */
	watch_usec_t next_loop;	/* next loop time in us, used for precise time
				 * keeping only */
#ifdef WITH_WATCH8BIT
	wchar_t *wcommand = NULL;
	int wcommand_characters = 0;	/* not including final \0 */
#endif	/* WITH_WATCH8BIT */

	/*static struct option longopts[] = {
		{"color", no_argument, 0, 'c'},
		{"differences", optional_argument, 0, 'd'},
		{"help", no_argument, 0, 'h'},
		{"interval", required_argument, 0, 'n'},
		{"beep", no_argument, 0, 'b'},
		{"errexit", no_argument, 0, 'e'},
		{"chgexit", no_argument, 0, 'g'},
		{"exec", no_argument, 0, 'x'},
		{"precise", no_argument, 0, 'p'},
		{"no-title", no_argument, 0, 't'},
		{"version", no_argument, 0, 'v'},
		{0, 0, 0, 0}
	};*/

#ifdef HAVE_PROGRAM_INVOCATION_NAME
	program_invocation_name = program_invocation_short_name;
#endif
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	atexit(fileutils_close_stdout);

	interval_string = getenv("WATCH_INTERVAL");
	if(interval_string != NULL)
		interval = strutils_strtod_nol_or_err(interval_string, _("Could not parse interval from WATCH_INTERVAL"));

	/*while ((optc =
		getopt_long(argc, argv, "+bced::ghn:pvtx", longopts, (int *)0))
	       != EOF) {
		switch (optc) {
		case 'b':
			flags |= WATCH_BEEP;
			break;
		case 'c':
			flags |= WATCH_COLOR;
			break;
		case 'd':
			flags |= WATCH_DIFF;
			if (optarg)
				flags |= WATCH_CUMUL;
			break;
		case 'e':
			flags |= WATCH_ERREXIT;
			break;
		case 'g':
			flags |= WATCH_CHGEXIT;
			break;
		case 't':
			show_title = 0;
			break;
		case 'x':
			flags |= WATCH_EXEC;
			break;
		case 'n':
			interval = strutils_strtod_nol_or_err(optarg, _("failed to parse argument"));
			break;
		case 'p':
			precise_timekeeping = 1;
			break;
		case 'h':
			usage(stdout);
			break;
		case 'v':
			printf(PROCPS_NG_VERSION);
			return EXIT_SUCCESS;
		default:
			usage(stderr);
			break;
		}
	}

	if (interval < 0.1)
		interval = 0.1;
	if (interval > UINT_MAX)
		interval = UINT_MAX;

	if (optind >= argc)
		usage(stderr);

	command_argv = &(argv[optind]);

	command = xstrdup(argv[optind++]);
	command_length = strlen(command);
	for (; optind < argc; optind++) {
		char *endp;
		int s = strlen(argv[optind]);
		// space and \0 
		command = xrealloc(command, command_length + s + 2);
		endp = command + command_length;
		*endp = ' ';
		memcpy(endp + 1, argv[optind], s);
		// space then string length 
		command_length += 1 + s;
		command[command_length] = '\0';
	}*/

	get_terminal_size();

	/* Catch keyboard interrupts so we can put tty back in a sane
	 * state.  */
	signal(SIGINT, die);
	signal(SIGTERM, die);
	signal(SIGHUP, die);
	signal(SIGWINCH, winch_handler);

	/* Set up tty for curses use.  */
	curses_started = 1;
	initscr();
	if (flags & WATCH_COLOR) {
		if (has_colors()) {
			start_color();
			use_default_colors();
			init_ansi_colors();
		} else {
			flags &= ~WATCH_COLOR;
		}
	}
	nonl();
	noecho();
	cbreak();

	if (precise_timekeeping)
		next_loop = get_time_usec();

	int count = 0;
	
	while (1) {
		if (screen_size_changed) {
			printf("Screen changed size\n");
			get_terminal_size();
			resizeterm(height, width);
			clear();
			/* redrawwin(stdscr); */
			screen_size_changed = 0;
			first_screen = 1;
		}

		if (show_title)
#ifdef WITH_WATCH8BIT
			output_header(wcommand, wcommand_characters, interval);
#else
			output_header(command, interval);
#endif	/* WITH_WATCH8BIT */

		print_ping_header(pingSetupData->ipv4, pingSetupData->rts);

		if(pingSetupData->ipv4) {
			main_ping(pingSetupData->rts, pingSetupData->fset, pingSetupData->sock4, pingSetupData->packet, pingSetupData->packlen);
		} else {
			main_ping(pingSetupData->rts, pingSetupData->fset, pingSetupData->sock6, pingSetupData->packet, pingSetupData->packlen);
		}

		refresh();

		if (precise_timekeeping) {
			watch_usec_t cur_time = get_time_usec();
			next_loop += USECS_PER_SEC * interval;
			if (cur_time < next_loop)
				usleep(next_loop - cur_time);
		} else
			if (interval < UINT_MAX / USECS_PER_SEC)
				usleep(interval * USECS_PER_SEC);
			else
				sleep(interval);
	}

	endwin();
	return EXIT_SUCCESS;
}
