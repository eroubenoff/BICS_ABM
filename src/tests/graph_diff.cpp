#include "../BICS_ABM.h"
#include "tests.h"
#include <vector>
#include <string>
#include <tuple>

using namespace std;

/*
 * Function to test the difference between two graphs. 
 * Returns a vector of the differences between two graphs
 */
DiffList graph_diff(igraph_t *g1, igraph_t *g2) {
    vector<Diff> diffs;

    bool same;
    // First check equality of the graphs
    igraph_is_same_graph(g1, g2, &same);
    if (!same) {
        diffs.emplace_back("g", "is_same", 0, 0);
    }

    // Number of nodes
    if (igraph_vcount(g1) != igraph_vcount(g2)) {
        diffs.emplace_back("g", "vcount", igraph_vcount(g1), igraph_vcount(g2));
    }
    // Number of edges
    if (igraph_ecount(g1) != igraph_ecount(g2)) {
        diffs.emplace_back("g", "ecount", igraph_ecount(g1), igraph_ecount(g2));
    }
    // Get into attributes

    igraph_vector_int_t gtypes, vtypes, etypes;
    igraph_strvector_t gnames, vnames, enames;
    igraph_integer_t i, j;

    igraph_vector_int_init(&gtypes, 0);
    igraph_vector_int_init(&vtypes, 0);
    igraph_vector_int_init(&etypes, 0);
    igraph_strvector_init(&gnames, 0);
    igraph_strvector_init(&vnames, 0);
    igraph_strvector_init(&enames, 0);

    igraph_cattribute_list(g1,
            &gnames, &gtypes,
            &vnames, &vtypes,
            &enames, &etypes);


    // Graph attributes
    int val1, val2;
    for (int i = 0; i < igraph_strvector_size(&gnames); i++) {
        if (VECTOR(gtypes)[i] == IGRAPH_ATTRIBUTE_NUMERIC) {
            val1 = GAN(g1, STR(gnames, i));
            val2 = GAN(g2, STR(gnames, i));
            if (val1 != val2) {
                diffs.emplace_back("g", STR(gnames, i), val1, val2);
            };
        } else {
            throw invalid_argument("All graph attributes should be numeric");
        }
    }

    // Vertex attributes
    for (int i = 0; i < igraph_vcount(g1); i++) {
        for (int j = 0; j < igraph_strvector_size(&vnames); j++) {
            if (VECTOR(vtypes)[j] == IGRAPH_ATTRIBUTE_NUMERIC) {
                val1 = VAN(g1, STR(vnames, j), i);
                val2 = VAN(g2, STR(vnames, j), i);
                if (val1 != val2) {
                    diffs.emplace_back("v", STR(vnames, j), val1, val2, i);
                }
            } else {
                throw invalid_argument("All graph attributes should be numeric");
            }
        }
    }

    // Edge attributes
    for (int i = 0; i < igraph_ecount(g1); i++) {
        for (int j = 0; j < igraph_strvector_size(&enames); j++) {
            if (VECTOR(etypes)[j] == IGRAPH_ATTRIBUTE_NUMERIC) {
                val1 = EAN(g1, STR(enames, j), i);
                val2 = EAN(g2, STR(enames, j), i);
                if (val1 != val2) {
                    diffs.emplace_back("e", STR(enames, j), val1, val2, i);
                }
            } else {
                throw invalid_argument("All graph attributes should be numeric");
            }
        }
    }

    igraph_strvector_destroy(&enames);
    igraph_strvector_destroy(&vnames);
    igraph_strvector_destroy(&gnames);
    igraph_vector_int_destroy(&etypes);
    igraph_vector_int_destroy(&vtypes);
    igraph_vector_int_destroy(&gtypes);


    return DiffList(diffs); 
}


