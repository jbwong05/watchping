#include <stdio.h>

void print_usage(void) {
	fprintf(stderr,
		"\nUsage\n"
		"  watchping [options] <destination>\n"
		"\nOptions:\n"
		"  <destination>      dns name or ip address\n"
		"  -a                 use audible ping\n"
		"  -A                 use adaptive ping\n"
		"  -B                 sticky source address\n"
		"  -c <count>         stop after <count> replies\n"
		"  -C <count>         only print statistics for the last <count> pings\n"
		"  -D                 print timestamps\n"
		"  -d                 use SO_DEBUG socket option\n"
		"  -f                 flood ping\n"
		"  -h                 print help and exit\n"
		"  -H                 turn off header\n"
		"  -I <interface>     either interface name or address\n"
		"  -i <interval>      seconds between sending each packet\n"
		"  -L                 suppress loopback of multicast packets\n"
		"  -l <preload>       send <preload> number of packages while waiting replies\n"
		"  -m <mark>          tag the packets going out\n"
		"  -M <pmtud opt>     define mtu discovery, can be one of <do|dont|want>\n"
		"  -n                 no dns name resolution\n"
		"  -O                 report outstanding replies\n"
		"  -p <pattern>       contents of padding byte\n"
		"  -P                 attempt run command in precise intervals\n"
		"  -q                 quiet output\n"
		"  -Q <tclass>        use quality of service <tclass> bits\n"
		"  -s <size>          use <size> as number of data bytes to be sent\n"
		"  -S <size>          use <size> as SO_SNDBUF socket option value\n"
		"  -t <ttl>           define time to live\n"
		"  -U                 print user-to-user latency\n"
		"  -v                 verbose output\n"
		"  -V                 print version and exit\n"
		"  -w <deadline>      reply wait <deadline> in seconds\n"
		"  -W <timeout>       time to wait for response\n"
		"\nIPv4 options:\n"
		"  -4                 use IPv4\n"
		"  -b                 allow pinging broadcast\n"
		"  -R                 record route\n"
		"  -T <timestamp>     define timestamp, can be one of <tsonly|tsandaddr|tsprespec>\n"
		"\nIPv6 options:\n"
		"  -6                 use IPv6\n"
		"  -F <flowlabel>     define flow label, default is random\n"
		"  -N <nodeinfo opt>  use icmp6 node info query, try <help> as argument\n"
	);
	exit(2);
}
