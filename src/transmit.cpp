#include<igraph.h>
#include<vector>
#include<string>
#include<iostream>
#include "BICS_ABM.h"
#include <random>
using namespace std;

void transmit(igraph_t *g,
        const Params *params,
        mt19937 &generator){

    bernoulli_distribution dist;
    uniform_int_distribution uint_dist;

    int vcount= igraph_vcount(g);
    igraph_vector_int_t neighbors;
    igraph_vector_int_init(&neighbors, 0);

    // Faster to compare int than char, avoids call to strcmp 
    int ds; // Disease status of infectious node
    int ds2; // Disease status of neighbor node
    int n2; // Number of neighbors and index of neighbor node
    int vs2;         // 

    bool vs_next;
    int gamma;
    int sigma;
    int mu; 
    int rho; 

    unordered_map<string, int> mu_lookup;
    mu_lookup["[0,18)"]  = 0;
    mu_lookup["[18,25)"] = 1;
    mu_lookup["[25,35)"] = 2;
    mu_lookup["[35,45)"] = 3;
    mu_lookup["[45,55)"] = 4;
    mu_lookup["[55,65)"] = 5;
    mu_lookup["[65,75)"] = 6;
    mu_lookup["[75,85)"] = 7;
    mu_lookup[">85"]     = 8;  

    /* Pull attributes from g */
    igraph_vector_t ds_vec;
    igraph_vector_init(&ds_vec, vcount);
    VANV(g, "disease_status", &ds_vec);
    igraph_vector_t vs_vec;
    igraph_vector_init(&vs_vec, vcount);
    VANV(g, "vaccine_status", &vs_vec);

    for (int i = vcount; i--; ) {
        ds = VECTOR(ds_vec)[i]; 

        if ((ds == ::Ic) || (ds == ::Isc) || (ds == ::E)) {
            igraph_neighbors(g, &neighbors, i, IGRAPH_ALL); 
            for (int n_neighbors = igraph_vector_int_size(&neighbors) ; n_neighbors--; ) {
                n2 = VECTOR(neighbors)[n_neighbors];
                ds2 = VECTOR(ds_vec)[n2];  
                vs2 = VECTOR(vs_vec)[n2]; 
                if (ds2 != ::S) continue; 

                if (vs2 == ::V0){
                    dist = bernoulli_distribution(params->BETA);
                    vs_next = dist(generator);
                } else if (vs2 == ::V1){
                    dist = bernoulli_distribution(params->BETA * params->VE1);
                    vs_next = dist(generator);
                } else if (vs2 == ::V2) {
                    dist = bernoulli_distribution(params->BETA * params->VE2);
                    vs_next = dist(generator);
                } else if (vs2 == ::VW) {
                    dist = bernoulli_distribution(params->BETA * params->VEW);
                    vs_next = dist(generator);
                } else if (vs2 == ::VBoost) {
                    dist = bernoulli_distribution(params->BETA * params->VEBOOST);
                    vs_next = dist(generator);
                } else {
                    cout << "Error in switch " << endl;
                    dist = bernoulli_distribution(params->BETA);
                    vs_next = dist(generator);
                }
                if (vs_next) {
                    uint_dist = uniform_int_distribution(params->GAMMA_MIN, params->GAMMA_MAX);
                    gamma = uint_dist(generator);
                    uint_dist = uniform_int_distribution(params->SIGMA_MIN, params->SIGMA_MAX);
                    sigma = uint_dist(generator);
                    dist = bernoulli_distribution(params->MU_VEC[mu_lookup[VAS(g, "age", n2)]]); 
                    mu = dist(generator);
                    dist = bernoulli_distribution(params->RHO); 
                    rho = dist(generator);


                    set_sick(g, n2, gamma, sigma, mu, params->T_REINFECTION, rho ? ::Ic : ::Isc);
                }
            }
        }
        else continue;
    }

    igraph_vector_destroy(&ds_vec);
    igraph_vector_destroy(&vs_vec);
    igraph_vector_int_destroy(&neighbors);


}
