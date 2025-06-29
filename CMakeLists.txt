cmake_minimum_required(VERSION 3.28.2)

include(../sdk/config.cmake)
project(sieelf_sie_alarm C)

include_directories("../libsie/src/include")
link_libraries(crt crt_helper c)
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(SieAlarm_ELKA ${SOURCES})
target_sdk_setup(SieAlarm_ELKA ELKA)
target_link_directories(SieAlarm_ELKA PUBLIC "../libsie/build/lib/ELKA")
target_compile_options(SieAlarm_ELKA BEFORE PUBLIC -std=c11 -Wall -Werror -DSWILIB_MODERN)
