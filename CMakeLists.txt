cmake_minimum_required (VERSION 2.6)
project (dsb)

# Need to include staged files and libs
include_directories(${PROJECT_SOURCE_DIR}/common/include)
include_directories(${PROJECT_SOURCE_DIR}/daemon/include)
include_directories(${PROJECT_BINARY_DIR})

set(dsb_VERSION_MAJOR "0")
set(dsb_VERSION_MINOR "1")
set(dsb_VERSION_PATCH "1")

option(WITH_NO_THREADS "Disable multi-threading" OFF)

set(CMAKE_C_FLAGS "-Wall -Werror")
set(CMAKE_C_FLAGS_DEBUG "-D_DEBUG -g -Wall -Werror")
set(CMAKE_C_FLAGS_RELEASE "-O3")

IF(WIN32)
set(DSBCOMMONLIBS wsock32)
ENDIF(WIN32)

IF(UNIX)
set(DSBCOMMONLIBS dl pthread)
ENDIF(UNIX)

set(COMMONSOURCE common/src/xfunc.c common/src/agent.c common/src/nid.c common/src/test.c common/src/event.c common/src/errors.c common/src/common.c common/src/harc.c common/src/wrap.c common/src/module.c common/src/net.c common/src/net_protocol.c common/src/string.c common/src/array.c common/src/vm.c common/src/assembler.c common/src/names.c common/src/iterator.c common/src/pattern.c common/src/clone.c)
set(DSBDSOURCE daemon/src/dsbd.c daemon/src/processor.c daemon/src/router.c daemon/modules/net/netmod.c daemon/modules/net/handler.c daemon/modules/volatile/volatile.c daemon/src/harc_d.c daemon/modules/persistent/persistent.c)
set(DSBSOURCE lite/src/lite.c lite/src/cli.c)

configure_file( ${CMAKE_SOURCE_DIR}/common/include/dsb/config.h.in ${CMAKE_BINARY_DIR}/dsb/config.h )

add_library(dsbcommon SHARED ${COMMONSOURCE})
target_link_libraries(dsbcommon ${DSBCOMMONLIBS})

add_executable(dsbd ${DSBDSOURCE})
target_link_libraries(dsbd dsbcommon dl)

add_executable(dsbl ${DSBSOURCE})
target_link_libraries(dsbl dsbcommon dl)

add_executable(nid-test tests/nid-test.c)
target_link_libraries(nid-test dsbcommon)

add_executable(harc-test tests/harc-test.c daemon/src/harc_d.c)
target_link_libraries(harc-test dsbcommon dl)

add_executable(event-test tests/event-test.c)
target_link_libraries(event-test dsbcommon)

add_executable(router-test tests/router-test.c daemon/src/router.c)
target_link_libraries(router-test dsbcommon)

add_executable(system-perf tests/system-perf.c daemon/src/router.c daemon/src/processor.c daemon/src/harc_d.c daemon/modules/persistent/persistent.c daemon/modules/volatile/volatile.c)
target_link_libraries(system-perf dsbcommon)

add_executable(module-test tests/module-test.c)
target_link_libraries(module-test dsbcommon dl)

#add_executable(math-test tests/math-test.c daemon/src/router.c daemon/modules/math/math.c)
#target_link_libraries(math-test dsbcommon dl)

add_executable(volatile-test tests/volatile-test.c daemon/src/router.c daemon/src/harc_d.c daemon/modules/volatile/volatile.c)
target_link_libraries(volatile-test dsbcommon dl)

add_executable(agent-test tests/agent-test.c daemon/src/router.c daemon/src/harc_d.c daemon/modules/volatile/volatile.c)
target_link_libraries(agent-test dsbcommon dl)

add_executable(proc-test tests/proc-test.c daemon/src/router.c)
target_link_libraries(proc-test dsbcommon)

add_executable(vm-test tests/vm-test.c)
target_link_libraries(vm-test dsbcommon)

add_executable(vm-perf tests/vm-perf.c)
target_link_libraries(vm-perf dsbcommon)

add_executable(asm-test tests/asm-test.c)
target_link_libraries(asm-test dsbcommon)

add_executable(name-test tests/name-test.c)
target_link_libraries(name-test dsbcommon)

add_executable(iter-test tests/iter-test.c)
target_link_libraries(iter-test dsbcommon)

install(PROGRAMS "${PROJECT_BINARY_DIR}/libdsbcommon.so" DESTINATION lib)
install(PROGRAMS "${PROJECT_BINARY_DIR}/dsbd" DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/dsb/config.h" DESTINATION include/dsb)
install(DIRECTORY common/include/dsb DESTINATION include)

