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
    UpdateList hh_ul;
    unordered_map<int, vector<int>> hh_lookup;
    gen_hh_edges(&graph, hh_ul, hh_lookup);
    hh_ul.add_updates_to_graph(&graph);

    // Create a deep copy of the graph
    // Confirmed that this does deep copy attributes
    igraph_t g0;
    igraph_copy(&g0, &graph);

    // Set a few of these people to be sick
    // void set_sick(igraph_t *g, int n, int rde, int rds, bool mu, int t_reinfection, int is_symptomatic);
    //
    // After 10 periods, this person should be dead
    UpdateList ul;
    set_sick(ul, 4, 1, 3, 1, 100, _Ic);
    // After 10 periods, this person should be susceptible again
    set_sick(ul, 10, 2, 3, 0, 1, _Isc);
    // After 10 periods, this person should be recovered
    set_sick(ul, 20, 10, 2, 0, 100, _Isc);
    ul.add_updates_to_graph(&graph);
    ul.clear_updates();
    EXPECT_EQ(VAN(&graph, "mu", 4), 1);

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
        // print_attributes(&graph);
        decrement(&graph, &history, 0, 0, 0);
    }

    DiffList diffs = graph_diff(&g0, &graph);

    /*
    for (auto i: diffs.get()) {
        cout << i.print() << endl;
    }
    */

    
    // Begin tests

    Diff tmp_diff;
    EXPECT_EQ(diffs.get().size(), 14);
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
    EXPECT_EQ(tmp_diff.v1(), _D); 
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


    /* Node 10 was sick and is now susceptible again */ 

    tmp_diff = diffs.get("v", "disease_status", 20);
    EXPECT_EQ(tmp_diff.v0(), _S);
    EXPECT_EQ(tmp_diff.v1(), _E);
    tmp_diff = diffs.get("v", "remaining_days_exposed", 20);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 0);
    tmp_diff = diffs.get("v", "remaining_days_sick", 20);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 2);
    tmp_diff = diffs.get("v", "time_until_susceptible", 20);
    EXPECT_EQ(tmp_diff.v0(), -1);
    EXPECT_EQ(tmp_diff.v1(), 100);



    DELALL(&graph);
    igraph_destroy(&graph);
    igraph_destroy(&g0);

}
