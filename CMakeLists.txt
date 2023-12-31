cmake_minimum_required(VERSION 3.10)
project(cplot)

set(CMAKE_C_STANDARD 11)
SET(COMPILE_FLAG -o3)
# add_definitions(-DLOG_LEVEL=7)
# add_definitions(-DUSE_CONSOLE)
add_definitions(-DSVPNG_LINKAGE=static)

set(SRC_DIR src)
set(PRI_INCLUDE ${SRC_DIR}/logger/logger.h ${SRC_DIR}/config.h ${SRC_DIR}/../svpng/svpng.inc)
set(PUB_INCLUDE ${SRC_DIR}/cplot/include/cplot.h ${SRC_DIR}/cplot/include/log_levels.h ${SRC_DIR}/cplot/include/setters.h)
set(SRC_FILES ${SRC_DIR}/logger/logger.c)

add_library(cplot SHARED ${SRC_DIR}/cplot/cplot.c ${SRC_FILES})
target_link_libraries(cplot m)
set_target_properties(cplot PROPERTIES PUBLIC_HEADER "${PUB_INCLUDE}")
set_target_properties(cplot PROPERTIES PRIVATE_HEADER "${PRI_INCLUDE}")

add_executable(cplotter ${SRC_DIR}/cplotter/cplotter.c ${SRC_DIR}/cplot/cplot.c ${SRC_FILES})
target_link_libraries(cplotter m)
set_target_properties(cplotter PROPERTIES PUBLIC_HEADER "${PUB_INCLUDE}")
set_target_properties(cplotter PROPERTIES PRIVATE_HEADER "${PRI_INCLUDE}")

install(TARGETS cplotter cplot
        LIBRARY DESTINATION /usr/lib
        # ARCHIVE DESTINATION lib
        RUNTIME DESTINATION /bin
        PUBLIC_HEADER DESTINATION /usr/include/${PROJECT_NAME}
)