set(IP_UTILS_COMMON_SRCS iputils/common/iputils_common.c)
set(IP_UTILS_MD5_SRCS iputils/md5/md5.c)
set(PING_SRCS ping/ping.c ping/ping_common.c ping/ping6_common.c ping/node_info.c)

add_library(iputilscommon ${IP_UTILS_COMMON_SRCS})

add_library(iputilsmd5 ${IP_UTILS_MD5_SRCS})

add_executable(ping ${PING_SRCS})
target_include_directories(ping PUBLIC iputils/include iputils/common iputils/md5)
target_link_libraries(ping iputilscommon iputilsmd5 ${RESOLV_LIBRARY})