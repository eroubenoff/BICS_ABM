
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



/* Params default values */ 
extern "C" Params init_params() {
    Params params{
        1000, // N_HH
        4,    // WAVE
        2*24, 4*24, // GAMMA_MIN, MAX
        3*24, 7*24, // SIGMA_MIN, MAX
        0.1, // BETA
        {0.00001, 0.0001, 0.0001, 0.001, 0.001, 0.001, 0.01, 0.1, 0.1}, // MU
        5, // INDEX_CASES
        49, 4949, // SEED, POP_SEED
        100, // N_VAX_DAILY
        0.75, 0.95, // VE1, 2

        "age;age;age;age;hesitancy", // VAX_RULES_COLS
        ">85;[75,85);[65,75);[55,65);0.5", // VAX_RULES_VALS
        {1,1,1,2},  // VAX_CONDS_N
        4 // VAX_RULES_N

    }; 

    return params;
};
void delete_all_edges(igraph_t *g) {
    igraph_es_t es;
    igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
    igraph_delete_edges(g, es);
    igraph_es_destroy(&es);

}

void daytime_mixing(igraph_t *g, vector<poisson_distribution<int>> &pois,  mt19937 &generator) {

    igraph_rng_seed(igraph_rng_default(), generator());
    // Clear any edges present
    delete_all_edges(g);

    // Generate degree sequence from num_cc_nonhh
    igraph_t new_graph;
    igraph_vector_int_t num_cc_nonhh;
    igraph_vector_int_init(&num_cc_nonhh, igraph_vcount(g));

    // Take a poisson value for each
    /*
    for (int i = 0; i < igraph_vcount(g); i++) {
        poisson_distribution<int> distribution(VAN(g, "num_cc_nonhh", i) / 10);
        VECTOR(num_cc_nonhh)[i]=distribution(generator);

    }
    */

   //  vector<int> num_cc_nonhh(igraph_vcount, 0);
    for (int i = igraph_vcount(g); i--; ) {
        VECTOR(num_cc_nonhh)[i] = pois[i](generator);
    }


    // Need to make the degree sequence sum to even number
    // Do this by randomly sampling indices until we find a nonzero one
    // and decrement it
    int sum = igraph_vector_int_sum(&num_cc_nonhh);

    if (sum % 2 == 1) { 
        // Pick a random index
        int tries = 0;
        int randomIndex;
        while (tries < 1000) {
            randomIndex = rand() % igraph_vector_int_size(&num_cc_nonhh);

            if (VECTOR(num_cc_nonhh)[randomIndex] > 1) {
                VECTOR(num_cc_nonhh)[randomIndex] -= 1;
                break;
            }

            tries++;

        }
    }

    // Form degree sequence and get edgelist
    igraph_degree_sequence_game(&new_graph, &num_cc_nonhh, NULL, IGRAPH_DEGSEQ_CONFIGURATION); //IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE ); // IGRAPH_DEGSEQ_CONFIGURATION);
    igraph_vector_int_t edgelist;
    igraph_vector_int_init(&edgelist, igraph_vcount(&new_graph));
    igraph_get_edgelist(&new_graph, &edgelist, false);

    // igraph_vector_int_print(&edgelist);

    // Add edgelist to old graph
    igraph_add_edges(g, &edgelist, NULL);


    igraph_vector_int_destroy(&num_cc_nonhh);
    igraph_vector_int_destroy(&edgelist);
    DELALL(&new_graph);
    igraph_destroy(&new_graph);



}

/*
    int N_HH;
    int WAVE;
    int GAMMA_MIN;
    int GAMMA_MAX;
    int SIGMA_MIN;
    int SIGMA_MAX;
    float BETA;
    float MU_VEC[9];
    int INDEX_CASES;
    int SEED;
    int POP_SEED;
    int N_VAX_DAILY;
    float VE1;
    float VE2;

    char VAX_RULES_COLS[1000];
    char VAX_RULES_VALS[1000];
    int VAX_CONDS_N[100];
    int VAX_RULES_N;
    */

void print_params(const Params *params) {
    cout << "----------------------------------------"<< endl;
    cout << "Parameters passed to simulation:" << endl;

    cout << "N_HH:            " << params->N_HH << endl;
    cout << "GAMMA_MIN, _MAX: " << params->GAMMA_MIN << ", " << params->GAMMA_MAX << endl;
    cout << "SIGMA_MIN, _MAX: " << params->SIGMA_MIN << ", " << params->SIGMA_MAX << endl;
    cout << "BETA:            " << params->BETA<< endl;
    cout << "MU_VEC:          ";
    for (auto i: params->MU_VEC) {
        cout << i << "  ";
    }   
    cout << endl;
    cout << "INDEX_CASES:     " << params->INDEX_CASES << endl;
    cout << "SEED:            " << params->SEED<< endl;
    cout << "POP_SEED:        " << params->POP_SEED<< endl;
    cout << "N_VAX_DAILY:     " << params->N_VAX_DAILY<< endl;
    cout << "VE1, VE2:        " << params->VE1 << ", " << params->VE2 << endl;
    cout << "VAX_RULES_COLS:  " << params->VAX_RULES_COLS << endl;
    cout << "VAX_RULES_VALS:  " << params->VAX_RULES_VALS << endl;
    cout << "VAX_CONDS_N:     " << params->VAX_CONDS_N << endl;
    cout << "VAX_RULES_N:     " << params->VAX_RULES_N << endl;
    cout << "----------------------------------------"<< endl;

}


void BICS_ABM(const Data *data, const Params *params, History *history) {

    print_params(params);

    const bool cached = false;
    mt19937 generator(params->SEED);

    // History object
    //  history(2000);
    /* 
     * Pre-generate all distributions as cycling vectors
     *
     * */ 

    CyclingVector<int> gamma_vec(1000, [&generator, &params](){return (uniform_int_distribution<int>(params->GAMMA_MIN, params->GAMMA_MAX))(generator);});
    CyclingVector<int> sigma_vec(1000, [&generator, &params](){return (uniform_int_distribution<int>(params->SIGMA_MIN, params->SIGMA_MAX))(generator);});
    // CyclingVector<int> beta_vec(1000, [&generator, BETA](){return (bernoulli_distribution(BETA))(generator);});

    // Transmission probability 
    unordered_map<int, CyclingVector<int>*> beta;
    beta[0] = new CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA)(generator));});
    beta[1] = new CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA*(1-params->VE1))(generator));});
    beta[2] = new CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA*(1-params->VE2))(generator));});


    // Create mortality
    unordered_map<string, CyclingVector<int> >mu;
    mu["[0,18)"]  = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[0]))(generator);});
    mu["[18,25)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[1]))(generator);});
    mu["[25,35)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[2]))(generator);});
    mu["[35,45)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[3]))(generator);});
    mu["[45,55)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[4]))(generator);});
    mu["[55,65)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[5]))(generator);});
    mu["[65,75)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[6]))(generator);});
    mu["[75,85)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[7]))(generator);});
    mu[">85"]     = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[8]))(generator);});
    

    // Create the empty graph
    igraph_t graph;

    /* Turn on attribute handling. */
    igraph_set_attribute_table(&igraph_cattribute_table);

    /* Create a directed graph with no vertices or edges. */
    igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);

    /* Generate population*/
    gen_pop_from_survey_csv(data, &graph, params);

    cout << "N vertices: " << igraph_vcount(&graph) << endl;

    // Create an unordered_map of hhids
    unordered_map<string, vector<int> > hhids;
    for (int i = 0; i < igraph_vcount(&graph); i++) {
        hhids[VAS(&graph, "hhid", i)].push_back(i);
    }

    // Create a household edge list
    igraph_vector_int_t hhedges;
    igraph_vector_int_init(&hhedges, 0);
    for (auto h : hhids) {

        // Create all combinations within the hh by nested loop
        for (int k=0; k < h.second.size(); k++) {
            for (int l=k+1; l < h.second.size(); l++) {
                if (k == l) {
                    continue;
                }
                else{
                    igraph_vector_int_push_back(&hhedges, h.second[k]);
                    igraph_vector_int_push_back(&hhedges, h.second[l]);
                }
            }
        }
    }

    // Add hh edges
    igraph_add_edges(&graph, &hhedges, NULL);

    /* Print the final result. */
    // print_attributes(&graph, true);


    // Pick nodes at random to be infected
    uniform_int_distribution<int> distribution(0,igraph_vcount(&graph) - 1);
    cout << "Index cases: ";
    for (int i = 0; i < params->INDEX_CASES; i++) {
        int index_case = distribution(generator);
        cout << index_case << "  " ;
        set_sick(&graph, index_case, 3*24, 5*24, false);
    }

    cout << endl;


    int day = 0;
    int hr = 0;
    decrement(&graph, history);


    /* For daytime, need to create poisson generators for contact */
    vector<poisson_distribution<int>> num_cc_nonhh;
    for (int i = 0; i < igraph_vcount(&graph); i++) {
        num_cc_nonhh.push_back(poisson_distribution<int>(VAN(&graph, "num_cc_nonhh", i)/float(10)));
    }

    // igraph_add_edges(&graph, &hhedges, NULL);

    while (GAN(&graph, "I_count") + GAN(&graph, "E_count") > 0){

        // Hours 0-8
        for (hr = 0; hr < 8; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            transmit(&graph, beta, gamma_vec, sigma_vec, mu);
            decrement(&graph, history);

        }

        delete_all_edges(&graph);

        distribute_vax(&graph, params->N_VAX_DAILY, 25*24);

        // Hours 8-16
        for (hr = 8; hr < 16; hr++){
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            daytime_mixing(&graph, num_cc_nonhh, generator);
            transmit(&graph, beta, gamma_vec, sigma_vec, mu);
            decrement(&graph, history);
        }


        // Hours 16-24
        delete_all_edges(&graph);
        igraph_add_edges(&graph, &hhedges, NULL);
        for (hr = 16; hr < 24; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            transmit(&graph, beta, gamma_vec, sigma_vec, mu);
            decrement(&graph, history);
        }

        day++;


    }

    // history.save();
    // history.plot_trajectory();

    /* Delete all remaining attributes. */
    DELALL(&graph);

    /* Destroy the graph. */
    igraph_destroy(&graph);

    // Destroy vectors
    igraph_vector_int_destroy(&hhedges);

    delete beta[0];
    delete beta[1];
    delete beta[2];

}
