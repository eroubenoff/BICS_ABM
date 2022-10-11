#include<igraph.h>
#include<vector>
#include<string>
#include<iostream>
#include "defs.h"
#include <random>
using namespace std;

void transmit(igraph_t *g, 
        unordered_map<int, CyclingVector<int> > *beta_vec,
        CyclingVector<int> *gamma_vec, 
        CyclingVector<int> *sigma_vec, 
        unordered_map<string, CyclingVector<int> > *mu ){

    int n_nodes = igraph_vcount(g);
    igraph_vector_int_t neighbors;
    igraph_vector_int_init(&neighbors, 0);
    const char* ds;
    int n_neighbors, n2;

    for (int i = n_nodes; i--; ) {
        ds = VAS(g, "disease_status", i);

        if (ds[0] == 'I') {
            igraph_neighbors(g, &neighbors, i, IGRAPH_ALL); 
            n_neighbors = igraph_vector_int_size(&neighbors);
            for (n_neighbors; n_neighbors--; ) {
                n2 = VECTOR(neighbors)[n_neighbors];
                if ((VAS(g, "disease_status", n2)[0] == 'S') & (*beta_vec)[VAN(g, "vaccine_status", n2)].next()) {
                    set_sick(g, n2, (*gamma_vec).next(), (*sigma_vec).next(), (*mu)[VAS(g, "age", n2)].next());
                }
            }
        }
        else continue;
    }

    igraph_vector_int_destroy(&neighbors);


}
