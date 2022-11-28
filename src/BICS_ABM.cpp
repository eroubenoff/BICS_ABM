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
#include <set>


/* 
 * Disease and vaccine status as globals 
 *
 * This is done so that they can be internally
 * represented as ints, which is much faster than
 * strings with the igraph api. They are globals for consistency. 
 * */

int S = 1;
int E = 2;
int Ic = 3;
int Isc = 4;
int R = 5;
int D = 6;
int V0 = 0;
int V1 = 1;
int V2 = 2;
int VW = 3;
int VBoost = 4;



void delete_all_edges(igraph_t *g) {
    igraph_es_t es;
    igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
    igraph_delete_edges(g, es);
    igraph_es_destroy(&es);

}




void BICS_ABM(const Data *data, const Params *params, History *history) {

    print_params(params);

    const bool cached = false;
    mt19937 generator(params->SEED);

    /* 
     * Pre-generate all distributions as cycling vectors
     *
     * */ 

    CyclingVector<int> gamma_vec(1000, [&generator, &params](){return (uniform_int_distribution<int>(params->GAMMA_MIN, params->GAMMA_MAX))(generator);});
    CyclingVector<int> sigma_vec(1000, [&generator, &params](){return (uniform_int_distribution<int>(params->SIGMA_MIN, params->SIGMA_MAX))(generator);});
    // CyclingVector<int> beta_vec(1000, [&generator, BETA](){return (bernoulli_distribution(BETA))(generator);});

    // Transmission probability 
    unordered_map<int, CyclingVector<int>> beta;
    beta[::V0] = CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA)(generator));});
    beta[::V1] = CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA*(1-params->VE1))(generator));});
    beta[::V2] = CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA*(1-params->VE2))(generator));});
    beta[::VW] = CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA*(1-params->VE2))(generator));});
    beta[::VBoost] = CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA*(1-params->VE2))(generator));});


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
    

    /* Create the empty graph */
    igraph_t graph;

    /* Turn on attribute handling. */
    igraph_set_attribute_table(&igraph_cattribute_table);

    /* Create a directed graph with no vertices or edges. */
    igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);

    /* Generate population*/
    gen_pop_from_survey_csv(data, &graph, params);

    cout << "N vertices: " << igraph_vcount(&graph) << endl;


    /* Generate household edges and add to g */
    igraph_vector_int_t hhedges;
    gen_hh_edges(&graph, &hhedges);
    igraph_add_edges(&graph, &hhedges, NULL);

    /* 
     * Vector containing the type of edges (all are household) 
     * Set type attribute 
     * */
    igraph_strvector_t hhedges_type;
    igraph_strvector_init(&hhedges_type, igraph_ecount(&graph));
    for (int i = 0; i < igraph_strvector_size(&hhedges_type);i++){igraph_strvector_set(&hhedges_type, i, "household");}
    igraph_cattribute_EAS_setv(&graph, "type", &hhedges_type);

    /* Generate daytime edges that include work contacts*/ 
    /*
    igraph_vector_int_t daytime_edges;
    igraph_strvector_t daytime_edges_type;
    gen_daytime_edges(&graph, &hhedges, &daytime_edges, &daytime_edges_type);
    */





    int day; 
    int hr;
    decrement(&graph, history);

    // Pick nodes at random to be infected
    for (day = 0; day < params->T0; day++){
        for (hr = 0; hr < 8; hr++) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            decrement(&graph, history);
        }
        distribute_vax(&graph, params->N_VAX_DAILY, 25*24, 30*24, 30*24);
        for (hr = 8; hr < 24; hr++) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            decrement(&graph, history);
        }

    }
    uniform_int_distribution<int> distribution(0,igraph_vcount(&graph) - 1);
    cout << "\r";
    cout << "Index cases: " ;
    for (int i = 0; i < params->INDEX_CASES; i++) {
        int index_case = distribution(generator);
        cout << index_case << "  " ;
        set_sick(&graph, index_case, 3*24, 5*24, false, params->T_REINFECTION, ::Ic);
    }

    decrement(&graph, history);

    
    igraph_integer_t n_edges;
    while (GAN(&graph, "Ic_count") + GAN(&graph, "E_count") + GAN(&graph, "Isc_count") > 0){

        // Hours 0-8
        for (hr = 0; hr < 8; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            transmit(&graph, beta, gamma_vec, sigma_vec, mu, params->T_REINFECTION);
            decrement(&graph, history);

        }


        distribute_vax(&graph, params->N_VAX_DAILY, 25*24, 30*24, 30*24);

        // Hours 8-16
        for (hr = 8; hr < 16; hr++){
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";

            /* 
             * Generate random contacts on &graph, and save them in random_contacts_es.
             * random_contacts_es is passed back to random_contacts so that
             * it can be used to efficiently remove the previous time step's 
             * random contacts, without having to re-wire everything using delete_all edges or 
             * similar.
             */

            // random_contacts(&graph, &daytime_edges, &daytime_edges_type, generator);
            random_contacts(&graph, &hhedges, &hhedges_type, params -> ISOLATION_MULTIPLIER, generator);
            transmit(&graph, beta, gamma_vec, sigma_vec, mu, params->T_REINFECTION);
            decrement(&graph, history);

        }


        // Hours 16-24
        delete_all_edges(&graph);
        igraph_add_edges(&graph, &hhedges, NULL);
        igraph_cattribute_EAS_setv(&graph, "type", &hhedges_type);
        for (hr = 16; hr < 24; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            transmit(&graph, beta, gamma_vec, sigma_vec, mu, params->T_REINFECTION);
            decrement(&graph, history);

        }

        day++;


    }


    /* Delete all remaining attributes. */
    DELALL(&graph);

    /* Destroy the graph. */
    igraph_destroy(&graph);

    // Destroy vector/
    igraph_vector_int_destroy(&hhedges);
    igraph_strvector_destroy(&hhedges_type);
    /*
    igraph_vector_int_destroy(&daytime_edges);
    igraph_strvector_destroy(&daytime_edges_type);
    */


}
