cmake_minimum_required(VERSION 3.14)
project(Portalg VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(portalg_core 
    src/ErrorHandler.cpp
    src/Scanner.cpp
    src/Parser.cpp
    src/Environment.cpp
    src/Resolver.cpp
    src/Callables.cpp
    src/Interpreter.cpp
)

add_executable(portalg src/main.cpp)
target_link_libraries(portalg PRIVATE portalg_core)

set(TARGET_NAME portalg)

if(WIN32)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_link_options(${TARGET_NAME} PRIVATE -static -s)
    elseif(MSVC)
        set_property(TARGET ${TARGET_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif()

elseif(UNIX AND NOT APPLE)
    target_link_options(${TARGET_NAME} PRIVATE -static-libgcc -static-libstdc++ -s)
endif()