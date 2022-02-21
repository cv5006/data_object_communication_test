cmake_minimum_required(VERSION 3.0.0)
project(data_obejct_communication_test VERSION 0.1.0)

include(CTest)
enable_testing()

set(CORE
    core/src/main.c
)

set(COMMON
)

set(UTILS
    # utils/src/csv_parser.c
)

include_directories(core/inc)
include_directories(common/inc)
include_directories(utils/inc)

add_executable(main
    ${CORE}
    ${UTILS}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
