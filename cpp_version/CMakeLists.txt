cmake_minimum_required(VERSION 3.18)
set (CMAKE_CXX_STANDARD 17)
project(igraph_test)


find_package(igraph REQUIRED)

add_executable(igraph_test 
	igraph_test.cpp 
	defs.h 
	get_csv_row.cpp
	print_attributes.cpp
	disease_status.cpp
	decrement.cpp
	set_sick.cpp
	transmit.cpp
	create_pop.cpp
        randstring.cpp
	)


    # set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
# set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall")
target_link_libraries(igraph_test PUBLIC igraph::igraph)


# TO RUN WITH PROFILING:
# cmake . -B build/ -DCMAKE_CXX_FLAGS=-pg -DCMAKE_EXE_LINKER_FLAGS=-pg -DCMAKE_SHARED_LINKER_FLAGS=-pg -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++
# cmake --build build/ -DCMAKE_CXX_FLAGS=-pg -DCMAKE_EXE_LINKER_FLAGS=-pg -DCMAKE_SHARED_LINKER_FLAGS=-pg -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++
