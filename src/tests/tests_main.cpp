#include <gtest/gtest.h>
#include "../BICS_ABM.h"
#include "tests.h"


/* 
 * Create some test graphs 
 * Arguments are all uninitialized graphs
 * */
void create_test_graphs(igraph_t *g) {

    int pop_size = 36;
    int n_cols = 7;

    // {"hhid", "age", "gender", "num_cc_nonhh", "lefthome_num", "vaccine_priority", "NPI"};
    double pop[] = {
        0,  4,  0,  0,  0,  0,  0,  
        1,  6,  1,  3,  1,  0,  0, 
        1,  7,  0,  2,  3,  0,  0,  
        1,  7,  0, 15,  1,  0,  1,  
        2,  4,  0,  2,  2,  0,  1,  
        2,  3,  1,  2,  2,  0,  1,  
        2,  2,  0,  0,  0,  0,  0,  
        2,  2,  0,  4,  6,  0,  0,  
        3,  2,  1,  4,  2,  0,  1,  
        3,  5,  1, 60,  2,  0,  1,  
        3,  2,  1,  0,  0,  0,  0,  
        4,  2,  1,  4,  1,  0,  1,  
        4,  6,  0,  1,  2,  0,  0,  
        4,  6,  1,  6,  2,  0,  0,  
        4,  3,  0,  1,  2,  0,  1,  
        4,  3,  0,  4,  2,  0,  1,  
        4,  4,  0,  5,  3,  0,  1,
        5,  4,  0,  0,  2,  0,  0,  
        5,  4,  1, 14,  1,  0,  1,  
        5,  1,  0,  0,  1,  0,  0,  
        6,  5,  0,  4,  3,  0,  1,  
        6,  5,  1,  8,  2,  0,  0,  
        6,  5,  0,  1,  1,  0,  1,  
        7,  4,  1, 30,  2,  0,  1,  
        7,  4,  0,  1,  2,  0,  1,  
        7,  4,  0,  1,  3,  0,  1,  
        8,  4,  1,  3,  1,  0,  0,  
        8,  2,  0,  4,  0,  0,  1,  
        8,  5,  1,  8,  2,  0,  0,  
        8,  4,  0, 10,  2,  0,  1,  
        8,  4,  1,  3,  2,  0,  1,  
        8,  5,  0, 10,  3,  0,  1,  
        9,  6,  1,  1,  1,  0,  1,  
        9,  7,  0,  1,  0,  0,  1,
        9,  4,  0,  0,  0,  0,  0,  
        9,  6,  1,  4,  6,  0,  1
    };

    create_graph_from_pop(g, pop, pop_size, n_cols);

}

int main(int argc, char **argv)
{
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
