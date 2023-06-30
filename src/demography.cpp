#include<igraph.h>
#include<vector>
#include<string>
#include<iostream>
#include <unordered_map>
#include "BICS_ABM.h"
#include <random>

bool random_draw(double p, mt19937 &seed) {

    std::bernoulli_distribution d(p);
    return d(seed);
}
/*
 * Function to handle births and deaths
 *
 */
void demography(igraph_t *g, Params *params, unordered_map<int, vector<int>> &hh_lookup, 
        UpdateList &hh_ul, mt19937 &seed) {

    igraph_vector_t age;
    igraph_vector_init(&age, 0);
    VANV(g, "age", &age);

    igraph_vector_t gender;
    igraph_vector_init(&gender, 0);
    VANV(g, "gender", &gender);

    igraph_vector_t ds;
    igraph_vector_init(&ds, 0);
    VANV(g, "disease_status", &ds);

    // First handle deaths
    int v = igraph_vcount(g);
    int a;
    for (int i = 0; i < v; i++) {
        a = VECTOR(age)[i];
        if (VECTOR(ds)[i] != _D) {
            if (random_draw(params->FERTILITY_V[a]/12, seed) ) {
                // Set to be dead
                VECTOR(ds)[i] = _D;
            }
        }
    }

    SETVANV(g, "disease_status", &ds);
    // Then handle births
    for (int i = 0; i < v; i++) {
        a = VECTOR(age)[i];
        if (VECTOR(ds)[i] != _D & VECTOR(gender)[i] != 0) {
            if (random_draw(params->MORTALITY_V[a]/12, seed) ) {

                // Create a new node in parent's household
                igraph_add_vertices(g, 1, NULL);

                // Get idx of last vertex
                int new_v = igraph_vcount(g) -1;
                SETVAN(g, "hhid", new_v, VAN(g, "hhid", i)); 
                SETVAN(g, "age", new_v, 0);
                SETVAN(g, "gender", new_v, random_draw(0.6, seed));
                SETVAN(g, "num_cc_nonhh", new_v, 1);
                SETVAN(g, "num_cc_school", new_v, 1);
                SETVAN(g, "lefthome_num", new_v, 1);
                SETVAN(g, "vaccine_priority", new_v, 0);
                SETVAN(g, "NPI", new_v, 0);
                SETVAN(g, "mortality", new_v, 0);
                SETVAN(g, "fertility", new_v, 0);
                SETVAN(g, "disease_status", new_v, _S);
                SETVAN(g, "remaining_days_exposed", new_v, -1);
                SETVAN(g, "remaining_days_sick", new_v, -1);
                SETVAN(g, "time_until_v2", new_v, -1);
                SETVAN(g, "time_until_vw", new_v, -1);
                SETVAN(g, "time_until_boost", new_v, -1);
                SETVAN(g, "time_until_susceptible", new_v, -1);
                SETVAN(g, "symptomatic", new_v, -1);
                SETVAN(g, "vaccine_status", new_v, 0);
                SETVAN(g, "mu", new_v, 0);

                // Add to household lookup
                // hh_lookup[VAN(g, "hhid", i)].push_back(new_v);


                cout << "There was a birth! The number of nodes is now:" << igraph_vcount(g)<< endl;
                

            }
        }
    }



    igraph_vector_destroy(&age);
    igraph_vector_destroy(&gender);
    igraph_vector_destroy(&ds);


}
