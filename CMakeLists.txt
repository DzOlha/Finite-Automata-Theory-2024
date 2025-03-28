cmake_minimum_required(VERSION 3.29)
project(Finite_Automaton_Theory_2024)

# Set C++ standard globally for all projects (optional)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")

# Add subdirectories
add_subdirectory(regex-to-nfa)
add_subdirectory(regex-to-mealy)
add_subdirectory(regex-to-buchi)