cmake_minimum_required(VERSION 2.6)
project(webserver)

add_executable(webserver main.c cmdparam.c settings.c sockutils.c proto.c cJSON.c)
target_link_libraries(webserver event pthread config pq m)
include_directories(/usr/include/postgresql)

install(TARGETS webserver RUNTIME DESTINATION bin)