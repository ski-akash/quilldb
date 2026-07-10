cmake_minimum_required(VERSION 3.14)
project(QuillDB VERSION 0.1.0 LANGUAGES CXX)

# Require C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings to keep our code clean
add_compile_options(-Wall -Wextra -Wpedantic)

# Define the source files (we will add to this as we build)
set(SOURCES
    src/cli/main.cpp
    src/lexer/Lexer.cpp
    src/parser/Parser.cpp
    src/planner/Planner.cpp
    src/executor/Executor.cpp
    src/optimizer/Optimizer.cpp
    src/catalog/Catalog.cpp
)

# Create the executable
add_executable(quilldb ${SOURCES})

# Include the src directory so we can use clean #include paths
target_include_directories(quilldb PRIVATE src)