cmake_minimum_required(VERSION 3.10)
project(QuillDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Core source files (everything EXCEPT main.cpp and benchmark.cpp)
set(CORE_SOURCES
    src/lexer/Lexer.cpp
    src/parser/Parser.cpp
    src/planner/Planner.cpp
    src/executor/Executor.cpp
    src/optimizer/Optimizer.cpp
    src/catalog/Catalog.cpp
)

include_directories(src)

# Executable 1: The standard CLI
add_executable(quilldb src/cli/main.cpp ${CORE_SOURCES})

# Executable 2: The Benchmark runner
add_executable(quilldb_benchmark src/cli/benchmark.cpp ${CORE_SOURCES})