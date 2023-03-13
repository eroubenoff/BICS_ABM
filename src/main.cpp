#include<igraph.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<vector>
#include<string>
#include<iostream>
#include<fstream>
#include <sstream>  
#include <unordered_map>
#include "BICS_ABM.h"
#include <random>
#include "sample_pop.h"
// #include <gtest/gtest.h>

using namespace std;

#define MAXCHAR 1000





int main(int argc, char **argv) {

    float mu[9] = {0.00001, 0.0001, 0.0001, 0.001, 0.001, 0.001, 0.01, 0.1, 0.1};
     sample_params.N_HH = 100;
     sample_params.WAVE = 6;
     sample_params.GAMMA_MIN = 2*24;
     sample_params.GAMMA_MAX = 4*24;
     sample_params.SIGMA_MIN = 3*24;
     sample_params.SIGMA_MAX = 7*24;
     std::fill(sample_params.BETA_VEC, sample_params.BETA_VEC + 365, 0.05 ); 
     std::copy(std::begin(mu), std::end(mu), std::begin(sample_params.MU_VEC));
     // sample_params.MU_VEC = mu;
     sample_params.INDEX_CASES = 5;
     std::fill(sample_params.IMPORT_CASES_VEC, sample_params.IMPORT_CASES_VEC + 365, 1); 
     sample_params.SEED =  49;
     sample_params.N_VAX_DAILY =  100;
     sample_params.VE1 =  0.75;
     sample_params.VE2 =  0.95;
     sample_params.VEW =  0.25;
     sample_params.VEBOOST = 0.95;
     sample_params.ISOLATION_MULTIPLIER = 1;
     sample_params.T_REINFECTION = 90*24;
     sample_params.T0 =  0;
     sample_params.ALPHA =  0.5;
     sample_params.RHO =  0.5;
     sample_params.NPI =  0.75;
     sample_params.MAX_DAYS = -1;
     sample_params.BOOSTER_DAY =  400;

    if (argc > 1) {
        throw runtime_error("Command line API is DEPRECATED! Use Python API."); 
    }

    // Trajectory trajectory; 
    History history;

    /* 
     * Create the empty graph 
     * Turn on attribute handling. 
     * Create a directed graph with no vertices or edges. 
     * */

    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);


    /* 
     * Generate population
     * */
    create_graph_from_pop(&graph, sample_pop, sample_pop_size, sample_pop_cols);

    /*
     * Run sim
     */
    BICS_ABM(&graph, &sample_params, &history);

    /* 
     * Delete all remaining attributes. 
     * */
    DELALL(&graph);

    /* 
     * Destroy the graph. 
     * */
    igraph_destroy(&graph);


    return 0;
}
