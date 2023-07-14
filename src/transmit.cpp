#include<igraph.h>
#include<vector>
#include<string>
#include<iostream>
#include "BICS_ABM.h"
#include <random>
#include<tuple>
using namespace std;

tuple<int, int> transmit(igraph_t *g,
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
    float symptomatic;
    double prob;
    float NPI;
    float vaccine;
    float reinf;

    /* Pull attributes from g */

    igraph_vector_t vs_vec;
    igraph_vector_init(&vs_vec, vcount);
    VANV(g, "vaccine_status", &vs_vec);

    igraph_vector_t ds_vec;
    igraph_vector_init(&ds_vec, vcount);
    VANV(g, "disease_status", &ds_vec);

    igraph_vector_t NPI_vec;
    igraph_vector_init(&NPI_vec, vcount);
    VANV(g, "NPI", &NPI_vec);

    igraph_vector_t etypes_vec;
    igraph_vector_init(&etypes_vec, 0);
    EANV(g, "type", &etypes_vec);

    igraph_vector_t eduration_vec;
    igraph_vector_init(&eduration_vec, 0);
    EANV(g, "duration", &eduration_vec);


    // Counts of new infections (to be returned)
    int Cc = 0;
    int Csc = 0;

    // Edge id
    igraph_integer_t eid;
    double duration;

    UpdateList ul;


    for (int i = vcount; i--; ) {
        ds = VECTOR(ds_vec)[i]; 

        if ((ds == _Ic) || (ds == _Isc) /*|| (ds == _E)*/) {
            igraph_neighbors(g, &neighbors, i, IGRAPH_ALL); 

            for (int n_neighbors = igraph_vector_int_size(&neighbors) ; n_neighbors--; ) {
                n2 = VECTOR(neighbors)[n_neighbors];
                igraph_get_eid(g, &eid, i, n2, false, true);
                duration = VECTOR(eduration_vec)[eid];
                // duration = EAN(g, "duration", eid);
                duration = max(duration, 1.0);

                ds2 = VECTOR(ds_vec)[n2];  
                vs2 = VECTOR(vs_vec)[n2]; 
                if (!((ds2 == _S) || (ds2 == _RW))) continue; 


                /* Assemble the components of the transmission probability */

                NPI = 1;
                if (VECTOR(NPI_vec)[i] && VECTOR(NPI_vec)[n2]) {
                    NPI = 1-params->NPI;
                } 
                if (VECTOR(etypes_vec)[i] == _Household) {
                    NPI = 1;
                }

                symptomatic = 0;
                if (ds == _Ic) {
                    symptomatic = 1;
                } else if (ds == _Ic) {
                    symptomatic = params->ALPHA;
                } 
                
                reinf = 1;
                if (ds2 == _RW) {
                    reinf = 1-params -> VEW;
                } 

                vaccine = 1;
                if (vs2 == _V0) {
                    vaccine = 1;
                } else if (vs2 == _V1) {
                    vaccine = 1-params->VE1;
                } else if (vs2 == _V2) {
                    vaccine = 1-params->VE2;
                } else if (vs2 == _VW) {
                    vaccine = 1-params->VEW;
                } else if (vs2 == _VBoost) {
                    vaccine = 1-params->VEBOOST;
                }

                prob = duration * BETA * NPI * symptomatic * reinf * vaccine;
                dist = bernoulli_distribution(prob);
                vs_next = dist(generator);





                /*

                if (vs2 == _V0){
                    prob = duration * BETA * (NPI ? (1-params->NPI) : 1) * (symptomatic ? 1 : params->ALPHA) * ((ds2 == _RW) ? params -> VEW : 1);
                    dist = bernoulli_distribution(prob);
                    vs_next = dist(generator);
                } else if (vs2 == _V1){
                    prob = duration * BETA * (NPI ? (1-params->NPI) : 1) * (1-params->VE1) * (symptomatic ? 1 : params->ALPHA) * ((ds2 == _RW) ? params -> VEW : 1);
                    dist = bernoulli_distribution(prob);
                    vs_next = dist(generator);
                } else if (vs2 == _V2) {
                    prob = duration * BETA * (NPI ? (1-params->NPI) : 1) * (1-params->VE2) * (symptomatic ? 1 : params->ALPHA) * ((ds2 == _RW) ? params -> VEW : 1);
                    dist = bernoulli_distribution(prob);
                    vs_next = dist(generator);
                } else if (vs2 == _VW) {
                    prob = duration * BETA * (NPI ? (1-params->NPI) : 1) * (1-params->VEW) * (symptomatic ? 1 : params->ALPHA) * ((ds2 == _RW) ? params -> VEW : 1);
                    dist = bernoulli_distribution(prob);
                    vs_next = dist(generator);
                } else if (vs2 == _VBoost) {
                    prob = duration * BETA * (NPI ? (1-params->NPI) : 1) * (1-params->VEBOOST) * (symptomatic ? 1 : params->ALPHA) * ((ds2 == _RW) ? params -> VEW : 1);
                    dist = bernoulli_distribution(prob);
                    vs_next = dist(generator);
                } else {
                    throw runtime_error("Error in Transmit switch");
                }
                */ 
                if (vs_next) {
                    uint_dist = uniform_int_distribution(params->GAMMA_MIN, params->GAMMA_MAX);
                    gamma = uint_dist(generator);
                    uint_dist = uniform_int_distribution(params->SIGMA_MIN, params->SIGMA_MAX);
                    sigma = uint_dist(generator);
                    dist = bernoulli_distribution(params->MU_VEC[(int)VAN(g, "age", n2)]); 
                    mu = dist(generator);
                    dist = bernoulli_distribution(params->RHO); 
                    rho = dist(generator);

                    if (rho) Cc++; else Csc++;

                    set_sick(ul, n2, gamma, sigma, mu, params->T_REINFECTION, rho ? _Ic : _Isc);
                }
            }
        }
        else continue;
    }

    ul.add_updates_to_graph(g);

    igraph_vector_destroy(&ds_vec);
    igraph_vector_destroy(&vs_vec);
    igraph_vector_destroy(&NPI_vec);
    igraph_vector_destroy(&etypes_vec);
    igraph_vector_destroy(&eduration_vec);
    igraph_vector_int_destroy(&neighbors);

    return make_tuple(Cc, Csc);


}
