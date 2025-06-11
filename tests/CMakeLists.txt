cmake_minimum_required(VERSION 3.10)
project(Tests_FIFO)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
  Doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG        v2.4.12
)
FetchContent_MakeAvailable(Doctest)

# Your test executable
add_executable(tests tests_fifo.cpp)
target_include_directories(tests PRIVATE ${doctest_SOURCE_DIR}/doctest)
target_compile_options(tests PRIVATE -Wall -Werror -Wconversion)