#include<igraph.h>
#include<vector>
#include<string>
#include<iostream>
#include "BICS_ABM.h"
#include <random>
using namespace std;

void transmit(igraph_t *g,
        float BETA,
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
    bool symptomatic;
    double prob;
    bool NPI;

    /* Pull attributes from g */
    igraph_vector_t ds_vec;
    igraph_vector_init(&ds_vec, vcount);
    VANV(g, "disease_status", &ds_vec);
    igraph_vector_t vs_vec;
    igraph_vector_init(&vs_vec, vcount);
    VANV(g, "vaccine_status", &vs_vec);

    igraph_vector_t NPI_vec;
    igraph_vector_init(&NPI_vec, vcount);
    VANV(g, "NPI", &NPI_vec);

    for (int i = vcount; i--; ) {
        ds = VECTOR(ds_vec)[i]; 

        if ((ds == ::Ic) || (ds == ::Isc) /*|| (ds == ::E)*/) {
            igraph_neighbors(g, &neighbors, i, IGRAPH_ALL); 
            if (ds == ::Ic) symptomatic=true;

            for (int n_neighbors = igraph_vector_int_size(&neighbors) ; n_neighbors--; ) {
                n2 = VECTOR(neighbors)[n_neighbors];
                ds2 = VECTOR(ds_vec)[n2];  
                vs2 = VECTOR(vs_vec)[n2]; 
                if (ds2 != ::S) continue; 

                if (VECTOR(NPI_vec)[i] && VECTOR(NPI_vec)[n2]) NPI = true;

                if (vs2 == ::V0){
                    prob = BETA * (NPI ? 1 : (1-params->NPI)) * (symptomatic ? 1 : params->ALPHA);
                    dist = bernoulli_distribution(prob);
                    vs_next = dist(generator);
                } else if (vs2 == ::V1){
                    prob = BETA * (NPI ? 1 : (1-params->NPI)) * (1-params->VE1) * (symptomatic ? 1 : params->ALPHA);
                    dist = bernoulli_distribution(prob);
                    vs_next = dist(generator);
                } else if (vs2 == ::V2) {
                    prob = BETA * (NPI ? 1 : (1-params->NPI)) * (1-params->VE2) * (symptomatic ? 1 : params->ALPHA);
                    dist = bernoulli_distribution(prob);
                    vs_next = dist(generator);
                } else if (vs2 == ::VW) {
                    prob = BETA * (NPI ? 1 : (1-params->NPI)) * (1-params->VEW) * (symptomatic ? 1 : params->ALPHA);
                    dist = bernoulli_distribution(prob);
                    vs_next = dist(generator);
                } else if (vs2 == ::VBoost) {
                    prob = BETA * (NPI ? 1 : (1-params->NPI)) * (1-params->VEBOOST) * (symptomatic ? 1 : params->ALPHA);
                    dist = bernoulli_distribution(prob);
                    vs_next = dist(generator);
                } else {
                    throw runtime_error("Error in Transmit switch");
                }
                if (vs_next) {
                    uint_dist = uniform_int_distribution(params->GAMMA_MIN, params->GAMMA_MAX);
                    gamma = uint_dist(generator);
                    uint_dist = uniform_int_distribution(params->SIGMA_MIN, params->SIGMA_MAX);
                    sigma = uint_dist(generator);
                    dist = bernoulli_distribution(params->MU_VEC[(int)VAN(g, "age", n2)]); 
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
    igraph_vector_destroy(&NPI_vec);
    igraph_vector_int_destroy(&neighbors);


}
