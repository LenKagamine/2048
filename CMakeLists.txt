cmake_minimum_required (VERSION 3.15)

project (game)
option(BUILD_STATIC_LIBS "Build the static library" ON)
add_executable(2048 cpp/main.cpp cpp/2048.cpp)
set(CMAKE_CXX_COMPILER g++)
target_include_directories(2048 PUBLIC cpp)
set_target_properties(2048 PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS NO
)
target_compile_options(2048 PUBLIC -O3 -g)

find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable"
)
set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
