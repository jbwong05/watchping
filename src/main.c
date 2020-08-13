#include "ping.h"
#include "watch.h"

#define VERSION 1.0
#define COMMAND_BUFFER_SIZE 1024

void parse_args(int argc, char *argv[], char (*command)[COMMAND_BUFFER_SIZE], 
        struct addrinfo *hints, struct ping_rts *rts, char **outpack_fill, char **target) {
    int ch;
    *outpack_fill = NULL;

	hints->ai_family = AF_UNSPEC;
	hints->ai_protocol = IPPROTO_UDP;
	hints->ai_socktype = SOCK_DGRAM;
	hints->ai_flags = getaddrinfo_flags;

    rts->interval = 1000;
	rts->preload = 1;
	rts->lingertime = MAXWAIT * 1000;
	rts->confirm_flag = MSG_CONFIRM;
	rts->tmin = LONG_MAX;
	rts->pipesize = -1;
	rts->datalen = DEFDATALEN;
	rts->screen_width = INT_MAX;
#ifdef HAVE_LIBCAP
	rts->cap_raw = CAP_NET_RAW;
	rts->cap_admin = CAP_NET_ADMIN;
#endif
	rts->pmtudisc = -1;
	rts->source.sin_family = AF_INET;
	rts->source6.sin6_family = AF_INET6;
	rts->ni.query = -1;
	rts->ni.subject_type = -1;

    int currentIndex = 1;
    strcpy(*command, "ping\0");
    char *currentArg;
    while(currentIndex < argc) {
        currentArg = argv[currentIndex];
        strncat(*command, " ", 1);
        strncat(*command, currentArg, strlen(currentArg));
        currentIndex++;
    }

    /* Support being called using `ping4` or `ping6` symlinks */
	if (argv[0][strlen(argv[0]) - 1] == '4')
		hints->ai_family = AF_INET;
	else if (argv[0][strlen(argv[0]) - 1] == '6')
		hints->ai_family = AF_INET6;

	/* Parse command line options */
	while ((ch = getopt(argc, argv, "h?" "4bRT:" "6F:N:" "aABc:dDfi:I:l:Lm:M:nOp:qQ:rs:S:t:UvVw:W:")) != EOF) {
		switch(ch) {
		/* IPv4 specific options */
		case '4':
			if (hints->ai_family == AF_INET6)
				error(2, 0, _("only one -4 or -6 option may be specified"));
			hints->ai_family = AF_INET;
			break;
		case 'b':
			rts->broadcast_pings = 1;
			break;
		case 'R':
			if (rts->opt_timestamp)
				error(2, 0, _("only one of -T or -R may be used"));
			rts->opt_rroute = 1;
			break;
		case 'T':
			if (rts->opt_rroute)
				error(2, 0, _("only one of -T or -R may be used"));
			rts->opt_timestamp = 1;
			if (strcmp(optarg, "tsonly") == 0)
				rts->ts_type = IPOPT_TS_TSONLY;
			else if (strcmp(optarg, "tsandaddr") == 0)
				rts->ts_type = IPOPT_TS_TSANDADDR;
			else if (strcmp(optarg, "tsprespec") == 0)
				rts->ts_type = IPOPT_TS_PRESPEC;
			else
				error(2, 0, _("invalid timestamp type: %s"), optarg);
			break;
		/* IPv6 specific options */
		case '6':
			if (hints->ai_family == AF_INET)
				error(2, 0, _("only one -4 or -6 option may be specified"));
			hints->ai_family = AF_INET6;
			break;
		case 'F':
			rts->flowlabel = parseflow(optarg);
			rts->opt_flowinfo = 1;
			break;
		case 'N':
			if (niquery_option_handler(&(rts->ni), optarg) < 0)
				ping_usage();
			hints->ai_socktype = SOCK_RAW;
			break;
		/* Common options */
		case 'a':
			rts->opt_audible = 1;
			break;
		case 'A':
			rts->opt_adaptive = 1;
			break;
		case 'B':
			rts->opt_strictsource = 1;
			break;
		case 'c':
			rts->npackets = strtol_or_err(optarg, _("invalid argument"), 1, LONG_MAX);
			break;
		case 'd':
			rts->opt_so_debug = 1;
			break;
		case 'D':
			rts->opt_ptimeofday = 1;
			break;
		case 'i':
		{
			double optval;

			optval = ping_strtod(optarg, _("bad timing interval"));
			if (isgreater(optval, (double)INT_MAX / 1000))
				error(2, 0, _("bad timing interval: %s"), optarg);
			rts->interval = (int)(optval * 1000);
			rts->opt_interval = 1;
		}
			break;
		case 'I':
			/* IPv6 */
			if (strchr(optarg, ':')) {
				char *p, *addr = strdup(optarg);

				if (!addr)
					error(2, errno, _("cannot copy: %s"), optarg);

				p = strchr(addr, SCOPE_DELIMITER);
				if (p) {
					*p = '\0';
					rts->device = optarg + (p - addr) + 1;
				}

				if (inet_pton(AF_INET6, addr, (char *)&rts->source6.sin6_addr) <= 0)
					error(2, 0, _("invalid source address: %s"), optarg);

				rts->opt_strictsource = 1;

				free(addr);
			} else if (inet_pton(AF_INET, optarg, &rts->source.sin_addr) > 0) {
				rts->opt_strictsource = 1;
			} else {
				rts->device = optarg;
			}
			break;
		case 'l':
			rts->preload = strtol_or_err(optarg, _("invalid argument"), 1, MAX_DUP_CHK);
			if (rts->uid && rts->preload > 3)
				error(2, 0, _("cannot set preload to value greater than 3: %d"), rts->preload);
			break;
		case 'L':
			rts->opt_noloop = 1;
			break;
		case 'm':
			rts->mark = strtol_or_err(optarg, _("invalid argument"), 0, INT_MAX);
			rts->opt_mark = 1;
			break;
		case 'M':
			if (strcmp(optarg, "do") == 0)
				rts->pmtudisc = IP_PMTUDISC_DO;
			else if (strcmp(optarg, "dont") == 0)
				rts->pmtudisc = IP_PMTUDISC_DONT;
			else if (strcmp(optarg, "want") == 0)
				rts->pmtudisc = IP_PMTUDISC_WANT;
			else
				error(2, 0, _("invalid -M argument: %s"), optarg);
			break;
		case 'n':
			rts->opt_numeric = 1;
			break;
		case 'O':
			rts->opt_outstanding = 1;
			break;
		case 'f':
			rts->opt_flood = 1;
			/* avoid `getaddrinfo()` during flood */
			rts->opt_numeric = 1;
			setbuf(stdout, (char *)NULL);
			break;
		case 'p':
			rts->opt_pingfilled = 1;
			*outpack_fill = strdup(optarg);
			if (!*outpack_fill)
				error(2, errno, _("memory allocation failed"));
			break;
		case 'q':
			rts->opt_quiet = 1;
			break;
		case 'Q':
			rts->settos = parsetos(optarg); /* IPv4 */
			rts->tclass = rts->settos; /* IPv6 */
			break;
		case 'r':
			rts->opt_so_dontroute = 1;
			break;
		case 's':
			rts->datalen = strtol_or_err(optarg, _("invalid argument"), 0, INT_MAX);
			break;
		case 'S':
			rts->sndbuf = strtol_or_err(optarg, _("invalid argument"), 1, INT_MAX);
			break;
		case 't':
			rts->ttl = strtol_or_err(optarg, _("invalid argument"), 0, 255);
			rts->opt_ttl = 1;
			break;
		case 'U':
			rts->opt_latency = 1;
			break;
		case 'v':
			rts->opt_verbose = 1;
			break;
		case 'V':
			printf("watchping %.1f\n", VERSION);
			exit(0);
		case 'w':
			rts->deadline = strtol_or_err(optarg, _("invalid argument"), 0, INT_MAX);
			break;
		case 'W':
		{
			double optval;

			optval = ping_strtod(optarg, _("bad linger time"));
			if (isless(optval, 0.001) || isgreater(optval, (double)INT_MAX / 1000))
				error(2, 0, _("bad linger time: %s"), optarg);
			/* lingertime will be converted to usec later */
			rts->lingertime = (int)(optval * 1000);
		}
			break;
		default:
			ping_usage();
			break;
		}
	}

	argc -= optind;
	argv += optind;

	if (!argc)
		error(1, EDESTADDRREQ, "usage error");

	iputils_srand();

	*target = argv[argc - 1];

	rts->outpack = malloc(rts->datalen + 28);
	if (!rts->outpack)
		error(2, errno, _("memory allocation failed"));
	if (*outpack_fill) {
		fill(rts, *outpack_fill, rts->outpack, rts->datalen);
		free(*outpack_fill);
	}
}

int main(int argc, char *argv[]) {
    struct ping_rts *rts = (struct ping_rts *)calloc(1, sizeof(struct ping_rts));
    struct addrinfo * hints = (struct addrinfo *)calloc(1, sizeof(struct addrinfo));
    char command[COMMAND_BUFFER_SIZE];
    char *outpack_fill;
    char *target;
    parse_args(argc, argv, &command, hints, rts, &outpack_fill, &target);

    struct ping_setup_data pingSetupData;
    ping_initialize(&pingSetupData, hints, rts, target);

    free(hints);

    return start_watch(&pingSetupData, command);
}