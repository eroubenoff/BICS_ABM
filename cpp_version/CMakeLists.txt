cmake_minimum_required(VERSION 3.18)
# add_compile_options(-Wall -Wextra -Werror)
# add_compile_options(-pg -g)
set (CMAKE_CXX_STANDARD 17)
project(igraph_test)


find_package(igraph REQUIRED)

add_executable(igraph_test 
    igraph_test.cpp 
    defs.h 
    get_csv_row.cpp
    print_attributes.cpp
    decrement.cpp
    set_sick.cpp
    transmit.cpp
    create_pop.cpp
    randstring.cpp
    )

target_link_libraries(igraph_test PUBLIC igraph::igraph)


