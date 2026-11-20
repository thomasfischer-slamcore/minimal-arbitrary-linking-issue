cmake_minimum_required(VERSION 3.10)
project(inline_copies_link_order_toy CXX)

# One set of flags for every file.
# Override with -DTOY_FLAGS="-O2 -march=native", for example.
set(TOY_FLAGS "-O3 -march=haswell -mtune=generic" CACHE STRING "Compile flags of every file")
separate_arguments(TOY_FLAGS_LIST UNIX_COMMAND "${TOY_FLAGS}")
add_compile_options(${TOY_FLAGS_LIST})

# Library A is header-only: it holds the inline function a().
add_library(A INTERFACE)
target_include_directories(A INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Libraries B and C both use a().
add_library(B STATIC B.cpp)
target_link_libraries(B PRIVATE A)

add_library(C STATIC C.cpp)
target_link_libraries(C PRIVATE A)

# Binaries D and F are the same source linked with the same two libraries. Only the order differs.
add_executable(D main.cpp)
target_link_libraries(D PRIVATE B C)

add_executable(F main.cpp)
target_link_libraries(F PRIVATE C B)
