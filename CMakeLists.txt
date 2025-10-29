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


set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


# target_include_directories(sbox-ga PRIVATE ${CMAKE_SOURCE_DIR}/include)
