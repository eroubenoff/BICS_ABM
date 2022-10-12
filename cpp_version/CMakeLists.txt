cmake_minimum_required(VERSION 3.18)
# add_compile_options(-Wall -Wextra -Werror)
# add_compile_options(-pg -g)
set (CMAKE_CXX_STANDARD 17)
project(igraph_test)


# add_compile_options(-I /usr/local/Cellar/python@2/2.7.15_3/Frameworks/Python.framework/Versions/2.7/include/python2.7/Python.h)
add_library(matplotlib_cpp INTERFACE)
target_include_directories(matplotlib_cpp
  INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/examples>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(matplotlib_cpp INTERFACE
  cxx_std_11
)
# TODO: Use `Development.Embed` component when requiring cmake >= 3.18
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
target_link_libraries(matplotlib_cpp INTERFACE
  Python3::Python
  Python3::Module
)
find_package(Python3 COMPONENTS NumPy)
if(Python3_NumPy_FOUND)
  target_link_libraries(matplotlib_cpp INTERFACE
    Python3::NumPy
  )
else()
  target_compile_definitions(matplotlib_cpp INTERFACE WITHOUT_NUMPY)
endif()
install(
  TARGETS matplotlib_cpp
  EXPORT install_targets
)

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
    utils.cpp
    distribute_vax.cpp
    history.cpp
    matplotlib.h
    )

target_link_libraries(igraph_test PUBLIC igraph::igraph)
target_link_libraries(igraph_test PRIVATE matplotlib_cpp)

