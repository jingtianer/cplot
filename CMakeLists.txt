cmake_minimum_required(VERSION 3.10)
project(cplot)

set(CMAKE_C_STANDARD 11)
SET(COMPILE_FLAG -o3)
# add_definitions(-DLOG_LEVEL=7)
# add_definitions(-DUSE_CONSOLE)
add_definitions(-DSVPNG_LINKAGE=static)

set(SRC_DIR src)
set(PRI_INCLUDE ${SRC_DIR}/logger/logger.h ${SRC_DIR}/config.h ${SRC_DIR}/../svpng/svpng.inc)
set(PUB_INCLUDE ${SRC_DIR}/cplot/include/cplot.h)
set(SRC_FILES ${SRC_DIR}/logger/logger.c)

add_library(cplot SHARED ${SRC_DIR}/cplot/cplot.c ${SRC_FILES})
target_link_libraries(cplot m)
set_target_properties(cplot PROPERTIES PUBLIC_HEADER ${SRC_DIR}/cplot/include/cplot.h)
set_target_properties(cplot PROPERTIES PRIVATE_HEADER ${SRC_DIR}/logger/logger.h PRIVATE_HEADER ${SRC_DIR}/config.h PRIVATE_HEADER ${SRC_DIR}/../svpng/svpng.inc)

add_executable(cplotter ${SRC_DIR}/cplotter/cplotter.c ${SRC_FILES})
target_link_libraries(cplotter cplot)
set_target_properties(cplotter PROPERTIES PUBLIC_HEADER ${SRC_DIR}/cplot/include/cplot.h)
set_target_properties(cplotter PROPERTIES PRIVATE_HEADER ${SRC_DIR}/logger/logger.h PRIVATE_HEADER ${SRC_DIR}/config.h PRIVATE_HEADER ${SRC_DIR}/../svpng/svpng.inc)

install(TARGETS cplotter cplot
        LIBRARY DESTINATION /usr/lib
        # ARCHIVE DESTINATION lib
        RUNTIME DESTINATION /bin
        PUBLIC_HEADER DESTINATION /usr/include/${PROJECT_NAME}
)