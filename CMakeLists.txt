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

set(COMMONSOURCE common/src/nid.c common/src/test.c common/src/event.c common/src/errors.c common/src/common.c common/src/harc.c common/src/wrap.c common/src/module.c common/src/net.c common/src/net_protocol.c common/src/string.c common/src/array.c common/src/vm.c common/src/assembler.c common/src/names.c)
set(DSBDSOURCE daemon/src/dsbd.c daemon/src/processor.c daemon/src/router.c daemon/modules/net/netmod.c daemon/modules/net/handler.c daemon/modules/volatile/volatile.c daemon/modules/evaluators/evaluators.c daemon/modules/evaluators/basic.c daemon/modules/evaluators/vm.c daemon/src/evaluator.c daemon/src/harc_d.c daemon/modules/persistent/persistent.c)

configure_file( ${CMAKE_SOURCE_DIR}/common/include/dsb/config.h.in ${CMAKE_BINARY_DIR}/dsb/config.h )

add_library(dsbcommon SHARED ${COMMONSOURCE})
target_link_libraries(dsbcommon dl)

add_executable(dsbd ${DSBDSOURCE})
target_link_libraries(dsbd dsbcommon dl)

add_executable(nid-test tests/nid-test.c)
target_link_libraries(nid-test dsbcommon)

add_executable(harc-test tests/harc-test.c daemon/modules/evaluators/evaluators.c daemon/modules/evaluators/basic.c daemon/modules/evaluators/vm.c daemon/src/evaluator.c daemon/src/harc_d.c)
target_link_libraries(harc-test dsbcommon dl)

add_executable(event-test tests/event-test.c)
target_link_libraries(event-test dsbcommon)

add_executable(router-test tests/router-test.c daemon/src/router.c)
target_link_libraries(router-test dsbcommon)

add_executable(module-test tests/module-test.c)
target_link_libraries(module-test dsbcommon dl)

#add_executable(math-test tests/math-test.c daemon/src/router.c daemon/modules/math/math.c)
#target_link_libraries(math-test dsbcommon dl)

add_executable(volatile-test tests/volatile-test.c daemon/src/router.c daemon/src/harc_d.c daemon/modules/volatile/volatile.c daemon/src/evaluator.c)
target_link_libraries(volatile-test dsbcommon dl)

add_executable(eval-test tests/eval-test.c daemon/modules/evaluators/evaluators.c daemon/modules/evaluators/basic.c daemon/modules/evaluators/vm.c daemon/src/evaluator.c)
target_link_libraries(eval-test dsbcommon dl)

add_executable(proc-test tests/proc-test.c daemon/src/processor.c daemon/src/router.c)
target_link_libraries(proc-test dsbcommon)

add_executable(vm-test tests/vm-test.c)
target_link_libraries(vm-test dsbcommon)

add_executable(name-test tests/name-test.c)
target_link_libraries(name-test dsbcommon)

install(PROGRAMS "${PROJECT_BINARY_DIR}/libdsbcommon.so" DESTINATION lib)
install(FILES "${PROJECT_BINARY_DIR}/dsb/config.h" DESTINATION include/dsb)
install(DIRECTORY common/include/dsb DESTINATION include)

