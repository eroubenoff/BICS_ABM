cmake_minimum_required(VERSION 3.16)
# add_compile_options(-Wall -Wextra -Werror)
# add_link_options(-shared -undefined dynamic_lookup -fPIC)
# add_compile_options(-pg -g)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
project(BICS_ABM)
add_subdirectory(igraph)
add_subdirectory(googletest)

# Add testing 
enable_testing()

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
    )

add_executable(BICS_ABM 
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

    src/main.cpp 
    src/sample_pop.h
    )

# add_executable(BICS_ABM_tests
    # src/tests/pop_tests.cpp
    # src/tests/tests_main.cpp
    # src/tests/data_tests.cpp
    # src/tests/decrement_tests.cpp
    # )

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(BICS_ABM_lib PUBLIC igraph)
target_link_libraries(BICS_ABM PUBLIC igraph)
# target_link_libraries(BICS_ABM_tests PUBLIC BICS_ABM_lib PRIVATE gtest gtest_main )

# include(GoogleTest)
# gtest_discover_tests(BICS_ABM DISCOVERY_TIMEOUT 6000)
# add_test(src/tests/test.cpp BICS_ABM)
# add_test(Vax_tests BICS_ABM)
# add_test(DataTests BICS_ABM)
# add_test(DecrementTests BICS_ABM)

# Install headers

