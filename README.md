# watchping

## Description
A simple combination of the [watch](https://linux.die.net/man/1/watch) and [ping](https://linux.die.net/man/8/ping) utilities that allows me to monitor my unstable internet connection. [watchping](https://github.com/jbwong05/watchping/tree/master) is similar to running `watch ping -c 1 <destination>` except that the ping statistics actually update after every interval.

## Usage
```
Usage
  watchping [options] <destination>

Options:
  <destination>      dns name or ip address
  -a                 use audible ping
  -A                 use adaptive ping
  -B                 sticky source address
  -D                 print timestamps
  -d                 use SO_DEBUG socket option
  -f                 flood ping
  -h                 print help and exit
  -H                 turn off header
  -I <interface>     either interface name or address
  -i <interval>      seconds between sending each packet
  -L                 suppress loopback of multicast packets
  -l <preload>       send <preload> number of packages while waiting replies
  -m <mark>          tag the packets going out
  -M <pmtud opt>     define mtu discovery, can be one of <do|dont|want>
  -n                 no dns name resolution
  -O                 report outstanding replies
  -p <pattern>       contents of padding byte
  -P                 attempt run command in precise intervals
  -q                 quiet output
  -Q <tclass>        use quality of service <tclass> bits
  -s <size>          use <size> as number of data bytes to be sent
  -S <size>          use <size> as SO_SNDBUF socket option value
  -t <ttl>           define time to live
  -U                 print user-to-user latency
  -v                 verbose output
  -V                 print version and exit
  -w <deadline>      reply wait <deadline> in seconds
  -W <timeout>       time to wait for response

IPv4 options:
  -4                 use IPv4
  -b                 allow pinging broadcast
  -R                 record route
  -T <timestamp>     define timestamp, can be one of <tsonly|tsandaddr|tsprespec>

IPv6 options:
  -6                 use IPv6
  -F <flowlabel>     define flow label, default is random
  -N <nodeinfo opt>  use icmp6 node info query, try <help> as argument
```

## Dependencies
* libresolv
* libncursesw

## Building from sources
```
git clone https://github.com/jbwong05/watchping.git
cd watchping
mkdir build
cd build
cmake ..
make
```

The `CMAKE_BUILD_TYPE` variable can also be set to Debug using `-DCMAKE_BUILD_TYPE=Debug` if a debug version is required.

## Installation
```
git clone https://github.com/jbwong05/watchping.git
cd watchping
mkdir build
cd build
cmake ..
make
sudo make install
```

By default the install path is set to `/usr/local/bin`. This can be changed by setting the `CMAKE_INSTALL_PREFIX` using `-DCMAKE_INSTALL_PREFIX=<dir>` if another directory is preferred.

## Uninstallation
```
sudo make uninstall
```

## Credits
Most of the [watch](https://linux.die.net/man/1/watch) code was adapted from the [procps](https://gitlab.com/procps-ng/procps/-/tree/master) repository.
Most of the [ping](https://linux.die.net/man/8/ping) code was adapted from the [iputils](https://github.com/iputils/iputils) repository.