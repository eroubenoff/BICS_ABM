cmake_minimum_required(VERSION 3.16)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
project(BICS_ABM)
add_subdirectory(googletest)
find_package(igraph REQUIRED)
# Add testing 
enable_testing()

#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_library(BICS_ABM_lib SHARED 
    src/BICS_ABM.h 
    src/print_attributes.cpp
    src/decrement.cpp
    src/set_sick.cpp
    src/transmit.cpp
    src/distribute_vax.cpp
    src/history.cpp
    src/BICS_ABM.cpp 
    src/py_defs.cpp
    src/params.cpp
    src/mixing.cpp
    src/update.cpp
    )

add_executable(BICS_ABM 
    src/main.cpp 
    src/sample_pop.h

    src/BICS_ABM.h 
    src/print_attributes.cpp
    src/decrement.cpp
    src/set_sick.cpp
    src/transmit.cpp
    src/distribute_vax.cpp
    src/history.cpp
    src/BICS_ABM.cpp 
    src/py_defs.cpp
    src/params.cpp
    src/mixing.cpp
    src/update.cpp
    )

add_executable(BICS_ABM_tests
    src/tests/tests.h
    src/tests/graph_diff.cpp
    # src/tests/pop_tests.cpp
    src/tests/tests_main.cpp
    # src/tests/data_tests.cpp
    src/tests/decrement_tests.cpp
    src/tests/update_tests.cpp
    )

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(BICS_ABM_lib PUBLIC igraph::igraph)
# target_link_libraries(BICS_ABM PUBLIC BICS_ABM_lib igraph::igraph)
target_link_libraries(BICS_ABM PUBLIC igraph::igraph)
target_link_libraries(BICS_ABM_tests PUBLIC BICS_ABM_lib igraph::igraph PRIVATE gtest gtest_main )

# include(GoogleTest)
# gtest_discover_tests(BICS_ABM DISCOVERY_TIMEOUT 6000)
# add_test(src/tests/test.cpp BICS_ABM)
# add_test(Vax_tests BICS_ABM)
# add_test(DataTests BICS_ABM)
add_test(DecrementTests BICS_ABM)
add_test(Decrement2Tests BICS_ABM)
add_test(UpdateTests BICS_ABM)

# Install headers

