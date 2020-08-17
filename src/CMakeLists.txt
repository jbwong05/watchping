set(NCURSES_COLOR_SRCS ncurses_color/ncurses_color.c)
set(IP_UTILS_SRCS ping/iputils/common/iputils_common.c ping/iputils/md5/md5.c)
set(PING_SRCS ping/ping.c ping/ping_common.c ping/ping6_common.c ping/node_info.c)
set(WATCH_SRCS watch/watch.c watch/fileutils/fileutils.c watch/strutils/strutils.c)
set(WATCHPING_SRCS ./main.c)

add_library(ncursescolor ${NCURSES_COLOR_SRCS})
target_link_libraries(ncursescolor ${NCURSES_LIBRARY})

add_library(iputils ${IP_UTILS_SRCS})

add_library(ping ${PING_SRCS})
target_include_directories(ping PUBLIC ncurses_color ping/iputils/include ping/iputils/common ping/iputils/md5)
target_link_libraries(ping ncursescolor iputils ${RESOLV_LIBRARY} ${NCURSES_LIBRARY})

add_library(watch ${WATCH_SRCS})
target_include_directories(watch PUBLIC ncurses_color watch/include watch/fileutils watch/strutils ping)
target_link_libraries(watch ${NCURSES_LIBRARY} ncursescolor ping)

add_executable(watchping ${WATCHPING_SRCS})
target_include_directories(watchping PUBLIC ping watch)
target_link_libraries(watchping ping watch)

install(TARGETS watchping DESTINATION ${CMAKE_INSTALL_PREFIX} PERMISSIONS SETUID OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
add_custom_target(uninstall COMMAND rm -f ${CMAKE_INSTALL_PREFIX}/watchping)