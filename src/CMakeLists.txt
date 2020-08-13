set(IP_UTILS_COMMON_SRCS ping/iputils/common/iputils_common.c)
set(IP_UTILS_MD5_SRCS ping/iputils/md5/md5.c)
set(FILEUTILS_SRCS watch/fileutils/fileutils.c)
set(STRUTILS_SRCS watch/strutils/strutils.c)
set(PING_SRCS ping/ping.c ping/ping_common.c ping/ping6_common.c ping/node_info.c)
set(WATCH_SRCS watch/watch.c)
set(WATCHPING_SRCS ./main.c)

add_library(iputilscommon ${IP_UTILS_COMMON_SRCS})

add_library(iputilsmd5 ${IP_UTILS_MD5_SRCS})

add_library(ping ${PING_SRCS})
target_include_directories(ping PUBLIC ping/iputils/include ping/iputils/common ping/iputils/md5)
target_link_libraries(ping iputilscommon iputilsmd5 ${RESOLV_LIBRARY} ${NCURSES_LIBRARY})

add_library(fileutils ${FILEUTILS_SRCS})
target_include_directories(fileutils PUBLIC watch/include)

add_library(strutils ${STRUTILS_SRCS})
target_include_directories(strutils PUBLIC watch/include)

add_library(watch ${WATCH_SRCS})
target_include_directories(watch PUBLIC watch/include watch/fileutils watch/strutils ping)
target_link_libraries(watch ${NCURSES_LIBRARY} fileutils strutils ping)

add_executable(watchping ${WATCHPING_SRCS})
target_include_directories(watchping PUBLIC ping watch)
target_link_libraries(watchping ping watch)

install(TARGETS watchping DESTINATION /usr/local/bin PERMISSIONS SETUID OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)