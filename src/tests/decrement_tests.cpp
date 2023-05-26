#include <gtest/gtest.h>
#include "../BICS_ABM.h"
#include "tests.h"
#include <vector>
#include <string>

using namespace std;

TEST(DecrementTests, GeneralTest) {
    History h;
    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);

    // Create graph  
    create_test_graphs(&graph);
    History history;

    // Create hh edges
    igraph_vector_int_t hhedges;
    gen_hh_edges(&graph, &hhedges);
    igraph_add_edges(&graph, &hhedges, NULL);

    // Get all hhs into a dict 
    unordered_map<int, vector<int>> hh_lookup;
    int hhid;
    for (int i = igraph_vcount(&graph); i--; ) {
        hhid = VAN(&graph, "hhid", i);
        if (hh_lookup.count(i) == 0) {
            hh_lookup[hhid] = vector<int>{i};
        } else {
            hh_lookup[hhid].push_back(i);
        }
    }

    // Set edges types 
    set_edge_attribute(&graph, &hhedges, "type", _Household, true);
    set_edge_attribute(&graph, &hhedges, "duration", 0, true);


    // Create a deep copy of the graph
    // Confirmed that this does deep copy attributes
    igraph_t g0;
    igraph_copy(&g0, &graph);

    // Set a few of these people to be sick
    // void set_sick(igraph_t *g, int n, int rde, int rds, bool mu, int t_reinfection, int is_symptomatic);
    //
    // After 10 periods, this person should be dead
    set_sick(&graph, 4, 1, 3, 1, 100, 1);
    EXPECT_EQ(VAN(&graph, "mu", 4), 1);
    // After 10 periods, this person should be susceptible again
    set_sick(&graph, 10, 2, 3, 0, 1, 0);
    // After 10 periods, this person should be recovered
    set_sick(&graph, 20, 10, 2, 0, 100, 0);

    // Distribute vaccines (Just doing this by hand for now-- will have to test the 
    // vax function separately 
    // After 10 periods, this person should be waned
    SETVAN(&graph, "vaccine_status", 5, _V1);
    SETVAN(&graph, "time_until_v2", 5, 3);
    SETVAN(&graph, "time_until_vw", 5, 3);

    // After 10 periods, this person should be on their second dose
    SETVAN(&graph, "vaccine_status", 2, _V2);
    SETVAN(&graph, "time_until_v2", 2, 30);
    SETVAN(&graph, "time_until_vw", 2, 30);

    for (int i = 0; i < 10; i++) {
        decrement(&graph, &history, 0, 0, 0);
    }

    DiffList diffs = graph_diff(&g0, &graph);

    for (auto i: diffs.get()) {
        cout << i.print() << endl;
    }

    
    // Begin tests

    Diff tmp_diff;
    EXPECT_EQ(diffs.get().size(), 19);
    EXPECT_EQ(diffs.get("v", 2).size(), 3);
    tmp_diff = diffs.get("v", "time_until_v2", 2);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 30);
    tmp_diff = diffs.get("v", "time_until_vw", 2);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 20);
    tmp_diff = diffs.get("v", "vaccine_status", 2);
    EXPECT_EQ(tmp_diff.v0(), _V0);
    EXPECT_EQ(tmp_diff.v1(), _V2);

    tmp_diff = diffs.get("v", "disease_status", 4);
    EXPECT_EQ(tmp_diff.v0(), _S);
    EXPECT_EQ(tmp_diff.v1(), _D); // NOT WORKING
    tmp_diff = diffs.get("v", "remaining_days_exposed", 4);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 0);
    tmp_diff = diffs.get("v", "remaining_days_sick", 4);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 3);
    tmp_diff = diffs.get("v", "time_until_susceptible", 4);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 100);
    tmp_diff = diffs.get("v", "mu", 4);
    EXPECT_EQ(tmp_diff.v0(), 0);
    EXPECT_EQ(tmp_diff.v1(), 1);

    tmp_diff = diffs.get("v", "time_until_v2", 5);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 0);
    tmp_diff = diffs.get("v", "time_until_vw", 5);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 3);
    tmp_diff = diffs.get("v", "vaccine_status", 5);
    EXPECT_EQ(tmp_diff.v0(), _V0);
    EXPECT_EQ(tmp_diff.v1(), _V1);

    tmp_diff = diffs.get("v", "remaining_days_exposed", 10);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 0);
    tmp_diff = diffs.get("v", "remaining_days_sick", 10);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 3);
    tmp_diff = diffs.get("v", "time_until_susceptible", 10);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 1);

    tmp_diff = diffs.get("v", "disease_status", 4);
    EXPECT_EQ(tmp_diff.v0(), _S);
    EXPECT_EQ(tmp_diff.v1(), _E);
    tmp_diff = diffs.get("v", "remaining_days_exposed", 4);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 0);
    tmp_diff = diffs.get("v", "remaining_days_sick", 4);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 3);
    tmp_diff = diffs.get("v", "time_until_susceptible", 4);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 100);



    DELALL(&graph);
    igraph_vector_int_destroy(&hhedges);
    igraph_destroy(&graph);
    igraph_destroy(&g0);

}
