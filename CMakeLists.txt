cmake_minimum_required (VERSION 2.8)
project (dsb)

# Need to include staged files and libs
include_directories(${PROJECT_SOURCE_DIR}/common/include)
include_directories(${PROJECT_SOURCE_DIR}/daemon/include)
include_directories(${PROJECT_BINARY_DIR})

set(dsb_VERSION_MAJOR "0")
set(dsb_VERSION_MINOR "1")
set(dsb_VERSION_PATCH "1")

option(WITH_NO_THREADS "Disable multi-threading" OFF)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG -g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(COMMONSOURCE common/src/nid.c common/src/test.c common/src/event.c common/src/errors.c common/src/common.c common/src/harc.c common/src/wrap.c)
set(DSBDSOURCE daemon/src/dsbd.c daemon/src/processor.c daemon/src/router.c daemon/src/module.c daemon/modules/math/math.c daemon/modules/volatile/volatile.c daemon/src/evaluator.c)

configure_file( ${CMAKE_SOURCE_DIR}/common/include/dsb/config.h.in ${CMAKE_BINARY_DIR}/config.h )

add_library(dsbcommon ${COMMONSOURCE})

add_executable(dsbd ${DSBDSOURCE})
target_link_libraries(dsbd dsbcommon)

add_executable(nid-test tests/nid-test.c)
target_link_libraries(nid-test dsbcommon)

add_executable(harc-test tests/harc-test.c)
target_link_libraries(harc-test dsbcommon)

add_executable(event-test tests/event-test.c)
target_link_libraries(event-test dsbcommon)

add_executable(router-test tests/router-test.c daemon/src/router.c)
target_link_libraries(router-test dsbcommon)

add_executable(module-test tests/module-test.c daemon/src/module.c)
target_link_libraries(module-test dsbcommon)

add_executable(math-test tests/math-test.c daemon/src/module.c daemon/src/router.c daemon/modules/math/math.c)
target_link_libraries(math-test dsbcommon)

add_executable(volatile-test tests/volatile-test.c daemon/src/module.c daemon/src/router.c daemon/modules/volatile/volatile.c daemon/src/evaluator.c)
target_link_libraries(volatile-test dsbcommon)

install(FILES "${PROJECT_BINARY_DIR}/libdsbcommon.a" DESTINATION lib)
install(DIRECTORY common/include DESTINATION include USE_SOURCE_PERMISSIONS)

