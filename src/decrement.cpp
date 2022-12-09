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
using namespace std;


void decrement(igraph_t *g, History *h, bool print) {
    igraph_real_t rds, rde, tv2, tvw, tvboost;

    // short int ds;
    int ds;
    igraph_real_t vs;

    int S_count = 0;
    int E_count = 0;
    int Ic_count = 0;
    int Isc_count = 0;
    int R_count = 0;
    int D_count = 0;
    int V1_count = 0;
    int V2_count = 0;
    int VW_count = 0;
    int VBoost_count = 0;

    int vcount = igraph_vcount(g);
    int mu;


    /* Get attributes as vectors */
    igraph_vector_t ds_vec;
    igraph_vector_init(&ds_vec, vcount);
    VANV(g, "disease_status", &ds_vec);
    igraph_vector_t vs_vec;
    igraph_vector_init(&vs_vec, vcount);
    VANV(g, "vaccine_status", &vs_vec);
    igraph_vector_t rde_vec;
    igraph_vector_init(&rde_vec, vcount);
    VANV(g, "remaining_days_exposed", &rde_vec);
    igraph_vector_t rds_vec;
    igraph_vector_init(&rds_vec, vcount);
    VANV(g, "remaining_days_sick", &rds_vec);
    igraph_vector_t mu_vec;
    igraph_vector_init(&mu_vec, vcount);
    VANV(g, "mu", &mu_vec);
    igraph_vector_t tv2_vec;
    igraph_vector_init(&tv2_vec, vcount);
    VANV(g, "time_until_v2", &tv2_vec);

    igraph_vector_t tvw_vec;
    igraph_vector_init(&tvw_vec, vcount);
    VANV(g, "time_until_vw", &tvw_vec);
    igraph_vector_t tvboost_vec;
    igraph_vector_init(&tvboost_vec, vcount);
    VANV(g, "time_until_vboost", &tvboost_vec);


    igraph_vector_t tsus_vec;
    igraph_vector_init(&tsus_vec, vcount);
    VANV(g, "time_until_susceptible", &tsus_vec);

    /* 
     * Loop through each vertex
     */ 
    for (int i = vcount; i--; ) {
        ds = VECTOR(ds_vec)[i]; 
        vs = VECTOR(vs_vec)[i];

        /* 
         * If susceptible do nothing
         */
        if (ds == ::S){
            ++S_count;
        }

        /*
         * If exposed and RDE = 0, set to infectious,
         * otherwise decrement RDE
         */
        else if (ds == ::E) {
            ++E_count;
            rde = VECTOR(rde_vec)[i]; 
            if (rde == 0.0) {
                if (VAN(g, "symptomatic", i) == ::Ic) {
                    VECTOR(ds_vec)[i] = ::Ic;
                } 
                else if (VAN(g, "symptomatic", i) == ::Isc) {
                    VECTOR(ds_vec)[i] = ::Isc;
                }
            } else {
                VECTOR(rde_vec)[i] -= 1;
            }
        }

        /*
         * If infectious and RDS=0, either set to R or D
         * depending on mu
         * Otherwise decrement RDS
         */
        else if (ds == ::Ic) {  
            ++Ic_count;
            rds = VECTOR(rds_vec)[i]; 
            mu = VECTOR(mu_vec)[i]; 
            if ((rds == 0.0) & (mu == 0.0)) {
                VECTOR(ds_vec)[i] = ::R;

            } else if ((rds == 0.0 ) & (mu == 1.0) ) {
                VECTOR(ds_vec)[i] = ::D;

            } else {
                VECTOR(rds_vec)[i] -= 1;
            }
        }

        else if (ds == ::Isc) {  
            ++Isc_count;
            rds = VECTOR(rds_vec)[i]; 
            mu = VECTOR(mu_vec)[i]; 
            if ((rds == 0.0) & (mu == 0.0)) {
                VECTOR(ds_vec)[i] = ::R;

            } else if ((rds == 0.0 ) & (mu == 1.0) ) {
                VECTOR(ds_vec)[i] = ::D;

            } else {
                VECTOR(rds_vec)[i] -= 1;
            }
        }

        /*
         * If recovered, do nothing
         */
        else if (ds == ::R) {
            ++R_count;
            if (VECTOR(tsus_vec)[i] == 0 ) {
                VECTOR(ds_vec)[i] = ::S;
            }
            else {
                VECTOR(tsus_vec)[i] -=1; 
            }

        }

        /* If dead, do nothing */
        else if (ds == ::D) {
            ++D_count;
        }


        /*
         * If vaccine status is V1, then decrement tv2
         * and wait until the next vaccine distribution
         */
        if (vs == ::V1) {
            ++V1_count;
            tv2 = VECTOR(tv2_vec)[i];
            if (tv2 > 0.0) {
                VECTOR(tv2_vec)[i] -= 1;
            }
        } 
        /*
         * If status is V2 decrement tvw (time until waned immunity)
         * until 0
         * If tvw == 0, set vaccine status to VW
         */
        else if (vs == ::V2) {
            ++V2_count;
            tvw = VECTOR(tvw_vec)[i];            
            if (tvw > 0.0) {
                VECTOR(tvw_vec)[i] -= 1;
            }
            else {
                VECTOR(vs_vec)[i] = ::VW;
            }
        } 

        /*
         * If vaccine status is VW, decrement tvboost.
         * If tvboost == 0, wait
         * until next vaccine distribution.
         */
        else if (vs == ::VW){
            ++VW_count;
            tvboost = VECTOR(tvboost_vec)[i];
            if (tvboost > 0.0){
                VECTOR(tvboost_vec)[i] -=1;
            }
        }

        else if (vs == ::VBoost) {
            ++VBoost_count;
        }
        
    }


    SETVANV(g, "disease_status", &ds_vec);
    SETVANV(g, "vaccine_status", &vs_vec);
    SETVANV(g, "remaining_days_exposed", &rde_vec);
    SETVANV(g, "remaining_days_sick", &rds_vec);
    SETVANV(g, "time_until_v2", &tv2_vec);
    SETVANV(g, "time_until_vw", &tvw_vec);
    SETVANV(g, "time_until_vboost", &tvboost_vec);
    SETVANV(g, "time_until_susceptible", &tsus_vec);
    igraph_vector_destroy(&ds_vec);
    igraph_vector_destroy(&vs_vec);
    igraph_vector_destroy(&rde_vec);
    igraph_vector_destroy(&rds_vec);
    igraph_vector_destroy(&mu_vec);
    igraph_vector_destroy(&tv2_vec);
    igraph_vector_destroy(&tvw_vec);
    igraph_vector_destroy(&tvboost_vec);
    igraph_vector_destroy(&tsus_vec);

    /* Tally edge counts */
    int hh_count = 0; 
    int random_count = 0;
    igraph_strvector_t etypes;
    igraph_strvector_init(&etypes, 0);
    EASV(g, "type", &etypes);
    for (int i = igraph_ecount(g); i--; ) {
        if (!strcmp(VECTOR(etypes)[i], "household") ) hh_count++;
        if (!strcmp(VECTOR(etypes)[i], "random") ) random_count++;
    }
    igraph_strvector_destroy(&etypes);



    SETGAN(g, "S_count", S_count);
    SETGAN(g, "E_count", E_count);
    SETGAN(g, "Ic_count", Ic_count);
    SETGAN(g, "Isc_count", Isc_count);
    SETGAN(g, "R_count", R_count);
    SETGAN(g, "D_count", D_count);
    SETGAN(g, "V1_count", V1_count);
    SETGAN(g, "V2_count", V2_count);
    SETGAN(g, "VW_count", VW_count);
    SETGAN(g, "VBoost_count", VBoost_count);
    SETGAN(g, "n_edges", hh_count + random_count);

    h->add_history(S_count, E_count, Ic_count, Isc_count, R_count, D_count, V1_count, V2_count, VW_count, VBoost_count, hh_count + random_count);

    if (print) {
        cout << "S: " << std::setw(5) << S_count << " | ";
        cout << "E: " << std::setw(5) << E_count << " | ";
        cout << "Ic: " << std::setw(5) << Ic_count << " | ";
        cout << "Isc: " << std::setw(5) << Isc_count << " | ";
        cout << "R: " << std::setw(5) << R_count << " | ";
        cout << "D: " << std::setw(5) << D_count << " | ";
        cout << "V1: " << std::setw(5) << V1_count << " | ";
        cout << "V2: " << std::setw(5) << V2_count << " | ";
        cout << "VW: " << std::setw(5) << VW_count << " | ";
        cout << "VBoost: " << std::setw(5) << VBoost_count << " | ";
        cout << "Edge counts: " << "Household: " <<  setw(5) << hh_count << " Work " << setw(5) << " Random  " << setw(5) <<  random_count;
        cout << flush;
    }
//     cout << endl;


}
