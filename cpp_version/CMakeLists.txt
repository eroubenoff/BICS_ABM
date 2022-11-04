cmake_minimum_required(VERSION 3.18)
# add_compile_options(-Wall -Wextra -Werror)
# add_link_options(-shared -undefined dynamic_lookup -fPIC)
# add_compile_options(-pg -g)
set (CMAKE_CXX_STANDARD 17)
project(BICS_ABM)


find_package(igraph REQUIRED)

add_library(BICS_ABM_lib MODULE 
    get_csv_row.cpp
    print_attributes.cpp
    decrement.cpp
    set_sick.cpp
    transmit.cpp
    create_pop.cpp
    randstring.cpp
    distribute_vax.cpp
    history.cpp
    main.cpp 
    BICS_ABM.h 
    BICS_ABM.cpp 
    stovf.cpp
    py_defs.cpp
    data.cpp
    )
add_executable(BICS_ABM 
    get_csv_row.cpp
    print_attributes.cpp
    decrement.cpp
    set_sick.cpp
    transmit.cpp
    create_pop.cpp
    randstring.cpp
    distribute_vax.cpp
    history.cpp
    main.cpp 
    BICS_ABM.h 
    BICS_ABM.cpp 
    stovf.cpp
    py_defs.cpp
    data.cpp
    )

target_link_libraries(BICS_ABM PUBLIC igraph::igraph)
target_link_libraries(BICS_ABM_lib PUBLIC igraph::igraph)

# Install headers

