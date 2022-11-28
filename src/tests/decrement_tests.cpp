#include <gtest/gtest.h>
#include "../BICS_ABM.h"
#include <vector>
#include <string>

using namespace std;

TEST(DecrementTests, GeneralTest) {
    History h;
    igraph_t new_graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_empty(&new_graph, 0, IGRAPH_UNDIRECTED);

    igraph_add_vertices(&new_graph, 7, 0);
    igraph_vector_t tempvec;
    vector<igraph_real_t> temparr;
    temparr = {(double) ::S, (double) ::E, (double) ::E, (double) ::I, (double) ::I , (double) ::R, (double) ::D};
    igraph_vector_init_array(&tempvec, temparr.data(), 7);
    SETVANV(&new_graph, "disease_status", &tempvec);

    temparr = {(double) ::V2, (double) ::V2, (double) ::V1, (double) ::V1, (double) ::V1, (double) ::V1, (double) ::V0};
    igraph_vector_init_array(&tempvec, temparr.data(), 7);
    SETVANV(&new_graph, "vaccine_status", &tempvec);


    temparr = {0, 3, 0, 0, 0, 0, 0};
    igraph_vector_init_array(&tempvec, temparr.data(), 7);
    SETVANV(&new_graph, "remaining_days_exposed", &tempvec);

    temparr = {0, 0, 0, 3, 0, 0, 0};
    igraph_vector_init_array(&tempvec, temparr.data(), 7);
    SETVANV(&new_graph, "remaining_days_sick", &tempvec);

    temparr = {0, 0, 0, 0, 1, 0, 1};
    igraph_vector_init_array(&tempvec, temparr.data(), 7);
    SETVANV(&new_graph, "mu", &tempvec);

    temparr = {0, 0, 1, 10, 10, 2};
    igraph_vector_init_array(&tempvec, temparr.data(), 7);
    SETVANV(&new_graph, "time_until_v2", &tempvec);


    igraph_add_edge(&new_graph, 0, 1);
    SETEAS(&new_graph, "type", 0, "household");
    igraph_add_edge(&new_graph, 2, 3);
    SETEAS(&new_graph, "type", 1, "random");

    decrement(&new_graph, &h);


    EXPECT_EQ(VAN(&new_graph, "disease_status", 0), ::S);
    EXPECT_EQ(VAN(&new_graph, "disease_status", 1), ::E);
    EXPECT_EQ(VAN(&new_graph, "disease_status", 2), ::I);
    EXPECT_EQ(VAN(&new_graph, "disease_status", 3), ::I);
    EXPECT_EQ(VAN(&new_graph, "disease_status", 4), ::D);
    EXPECT_EQ(VAN(&new_graph, "disease_status", 5), ::R);
    EXPECT_EQ(VAN(&new_graph, "disease_status", 6), ::D);

    DELALL(&new_graph);
    igraph_destroy(&new_graph);
    igraph_vector_destroy(&tempvec);

}
