cmake_minimum_required(VERSION 3.10)
project(cplot)

set(CMAKE_C_STANDARD 11)
include_directories(./svpng .)
link_libraries(m)
SET(COMPILE_FLAG -o3)
# add_definitions(-DLOG_LEVEL=7)
# add_definitions(-DUSE_CONSOLE)
add_executable(cplot main.c)

install(TARGETS cplot cplot
        # LIBRARY DESTINATION lib
        # ARCHIVE DESTINATION lib
        RUNTIME DESTINATION /bin
        # PUBLIC_HEADER DESTINATION include
)