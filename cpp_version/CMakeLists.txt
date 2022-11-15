cmake_minimum_required(VERSION 3.18)
# add_compile_options(-Wall -Wextra -Werror)
# add_link_options(-shared -undefined dynamic_lookup -fPIC)
# add_compile_options(-pg -g)
set (CMAKE_CXX_STANDARD 17)
project(BICS_ABM)

enable_testing()
include(CTest)

find_package(igraph REQUIRED)

# Add testing 
enable_testing()
#find_package(GTest REQUIRED)
# include_directories(${GTEST_INCLUDE_DIRS})
include(FetchContent)
FetchContent_Declare(
  googletest
  # URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip # From instructions
    URL https://github.com/google/googletest/archive/58d77fa8070e8cec2dc1ed015d66b454c8d78850.zip # Current
  )
FetchContent_MakeAvailable(googletest)

add_library(BICS_ABM_lib SHARED # MODULE 
    src/BICS_ABM.h 
    src/get_csv_row.cpp
    src/print_attributes.cpp
    src/decrement.cpp
    src/set_sick.cpp
    src/transmit.cpp
    src/create_pop.cpp
    src/randstring.cpp
    src/distribute_vax.cpp
    src/history.cpp
    src/BICS_ABM.cpp 
    src/stovf.cpp
    src/py_defs.cpp
    src/data.cpp
    )
add_executable(BICS_ABM 
    # src/BICS_ABM.h 
    src/main.cpp 
    src/tests/test.cpp
    src/tests/pop_tests.cpp
    )

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(BICS_ABM_lib PUBLIC igraph::igraph)
target_link_libraries(BICS_ABM PUBLIC BICS_ABM_lib PRIVATE GTest::gtest_main)

include(GoogleTest)
# gtest_discover_tests(BICS_ABM DISCOVERY_TIMEOUT 6000)
add_test(src/tests/test.cpp BICS_ABM)
add_test(src/tests/pop_test.cpp BICS_ABM)

# Install headers

