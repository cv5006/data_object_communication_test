cmake_minimum_required(VERSION 3.0.0)
project(data_obejct_communication_test VERSION 0.1.0)

include(CTest)
enable_testing()


set(COMMON
    common/src/data_object.c
    common/src/data_object_protocol.c
)

set(UTILS
    # utils/src/csv_parser.c
)

include_directories(core/inc)
include_directories(common/inc)
include_directories(utils/inc)

add_executable(test_dod
    core/src/test_dod.c
    ${COMMON}
    ${UTILS}
)

add_executable(test_cli
    core/src/test_cli.cc
    ${COMMON}
    ${UTILS}
)

add_executable(test_srv
    core/src/test_srv.cc
    ${COMMON}
    ${UTILS}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
