cmake_minimum_required (VERSION 2.8)
project (dsb)

# Need to include staged files and libs
include_directories(${PROJECT_SOURCE_DIR}/common/include)

set(dsb_VERSION_MAJOR "0")
set(dsb_VERSION_MINOR "1")
set(dsb_VERSION_PATCH "1")

set(CMAKE_C_FLAGS "-DLINUX -DTHREADED -g -Wall")

set(COMMONSOURCE common/src/nid.c common/src/test.c common/src/event.c common/src/router.c common/src/errors.c common/src/processor.c common/src/dsb.c)

add_library(dsbcommon ${COMMONSOURCE})

add_executable(nid-test tests/nid-test.c)
target_link_libraries(nid-test dsbcommon)

add_executable(event-test tests/event-test.c)
target_link_libraries(event-test dsbcommon)

add_executable(router-test tests/router-test.c)
target_link_libraries(router-test dsbcommon)

install(FILES "${PROJECT_BINARY_DIR}/libdsbcommon.a" DESTINATION lib)
install(DIRECTORY common/include DESTINATION include USE_SOURCE_PERMISSIONS)

