#include <gtest/gtest.h>
#include "../BICS_ABM.h"
#include "tests.h"
#include <vector>
#include <string>

using namespace std;

TEST(UpdateTests, GeneralTest) {
    History h;
    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);

    UpdateList update_list;
    igraph_t g0;
    // Create a deep copy of the graph
    // Confirmed that this does deep copy attributes
    igraph_copy(&g0, &graph);
    DiffList diffs;

    // Create graph  
    create_test_graphs(&graph);

    // Create hh edges
    igraph_vector_int_t hhedges;
    gen_hh_edges(&graph, &hhedges);

    for (int i = 0; i < igraph_vector_int_size(&hhedges); i+=2) {
        update_list.add_update(
                CreateEdge(
                VECTOR(hhedges)[i], 
                VECTOR(hhedges)[i+1]
                )
                );
    }

    // Add the updates to the graph
    update_list.add_updates_to_graph(&graph);

    // Check that the correct number of edges added
    EXPECT_EQ(igraph_ecount(&graph), igraph_vector_int_size(&hhedges) / 2);

    // Check another way
    //  diffs = graph_diff(&graph, &g0);
    // EXPECT_EQ(diffs.get("g", "ecount").v1(), igraph_vector_int_size(&hhedges)/2);


    update_list.clear_updates();

    

    // Add empty attributes to the graphs to create
    // them
    SETEAN(&graph, "type", 0, 6789);
    SETEAN(&graph, "duration", 0, 67890);
    igraph_copy(&g0, &graph);


    // Adding edge attributes
    // Adding by end points
    for (int i = 0; i < igraph_vector_int_size(&hhedges); i += 2) {
        update_list.add_update(
                UpdateEdgeAttribute(
                    VECTOR(hhedges)[i],
                    VECTOR(hhedges)[i+1],
                    "type",
                    _Household)
        );
    }
    update_list.add_updates_to_graph(&graph);
    update_list.clear_updates();

    // Adding by eid
    for (int i = 0; i < igraph_ecount(&graph); i++) {
        update_list.add_update(
                UpdateEdgeAttribute(
                    i,
                    "duration",
                    0)
        );
    }
    update_list.add_updates_to_graph(&graph);
    update_list.clear_updates();

    // Get diffs
    diffs = graph_diff(&g0, &graph);

    /*
    for (auto i: diffs.get()) {
        cout << i.print() << endl;
    }
    */

    
    // Begin tests
    EXPECT_EQ(diffs.get("e", "type").size(), igraph_ecount(&graph));
    EXPECT_EQ(diffs.get("e", "duration").size(), igraph_ecount(&graph));

    for (int i = 0; i < igraph_ecount(&graph); i++) {
        EXPECT_EQ(diffs.get("e", "type", i).v1(), _Household);
        EXPECT_EQ(diffs.get("e", "duration", i).v1(), 0);

    }


    // Test deleting a few edges
    igraph_copy(&g0, &graph);
    update_list.add_update(DeleteEdge(2,3));
    update_list.add_update(DeleteEdge(10));
    update_list.add_update(DeleteEdge(30));

    update_list.add_updates_to_graph(&graph);
    diffs = graph_diff(&g0, &graph);

    /*
    for (auto i: diffs.get()) {
        cout << i.print() << endl;
    }
    */

    EXPECT_EQ(diffs.get("g", "is_same")[0].v1(), 0);
    EXPECT_EQ(diffs.get("g", "ecount")[0].v0(), 146);
    EXPECT_EQ(diffs.get("g", "ecount")[0].v1(), 143);

    igraph_integer_t eid;
    igraph_get_eid(&graph, &eid, 2, 3, 0, 0);
    EXPECT_EQ(eid, -1);


    DELALL(&graph);
    igraph_vector_int_destroy(&hhedges);
    igraph_destroy(&graph);
    igraph_destroy(&g0);

}
