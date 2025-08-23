cmake_minimum_required(VERSION 3.28)
project(IrisLog)

set(CMAKE_CXX_STANDARD 17)

add_library(IrisLog STATIC include/IrisLog.h
            src/IrisLog.cpp)
target_include_directories(IrisLog PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_subdirectory(deps/fmt)
target_link_libraries(IrisLog PUBLIC fmt::fmt-header-only)


option(IRISLOG_BUILD_TESTS "Build tests for IrisLog" ON)

if(IRISLOG_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()