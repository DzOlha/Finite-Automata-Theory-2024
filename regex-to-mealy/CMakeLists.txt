cmake_minimum_required(VERSION 3.10)

# Declare the project for the task prime numbers
project(RegexToMealy VERSION 1.0.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static")

# Add the source files
file(GLOB_RECURSE SOURCES ./src/*.cpp)

# Create the executable
add_executable(RegexToMealyExecutable ${SOURCES}
        main.cpp)

# Include directories
target_include_directories(RegexToMealyExecutable PRIVATE
        ./include
)