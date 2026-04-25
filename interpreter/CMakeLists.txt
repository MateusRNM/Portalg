cmake_minimum_required(VERSION 3.14)
project(Portalg VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(portalg_core 
    src/Scanner.cpp
)

add_executable(portalg src/main.cpp)
target_link_libraries(portalg PRIVATE portalg_core)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_disable_pthreads ON CACHE BOOL "" FORCE) 
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)          
FetchContent_MakeAvailable(googletest)

add_executable(portalg_tests 
    tests/test_scanner.cpp
)

target_link_libraries(portalg_tests PRIVATE portalg_core gtest_main)

include(GoogleTest)
gtest_discover_tests(portalg_tests)