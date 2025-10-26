cmake_minimum_required(VERSION 3.10)
project(sbox-ga)
set(CMAKE_CXX_STANDARD 20)
add_executable(sbox-ga
				src/main.cpp
				src/utils.cpp
				src/sbox.cpp
				src/genetic.cpp
				src/analysis.cpp
)

target_include_directories(sbox-ga PRIVATE ${CMAKE_SOURCE_DIR}/include)
