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


void decrement(igraph_t *g, History *h) {
    igraph_real_t rds, rde, tv2;

    // short int ds;
    int ds;
    igraph_real_t vs;

    int S_count = 0;
    int E_count = 0;
    int I_count = 0;
    int R_count = 0;
    int D_count = 0;
    int V1_count = 0;
    int V2_count = 0;

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

    for (int i = vcount; i--; ) {
        ds = VECTOR(ds_vec)[i]; 
        vs = VECTOR(vs_vec)[i];

        if (ds == ::S){
                ++S_count;
        }

        else if (ds == ::E) {
                rde = VECTOR(rde_vec)[i]; 
                if (rde == 0.0) {
                    VECTOR(ds_vec)[i] = ::I;
                    ++I_count;
                } else {
                    VECTOR(rde_vec)[i] -= 1;
                    ++E_count;
                }
        }

        else if (ds == ::I) {  
                rds = VECTOR(rds_vec)[i]; 
                mu = VECTOR(mu_vec)[i]; 
                if ((rds == 0.0) & (mu == 0.0)) {
                    VECTOR(ds_vec)[i] = ::R;
                    ++R_count;

                } else if ((rds == 0.0 ) & (mu == 1.0) ) {
                    VECTOR(ds_vec)[i] = ::D;
                    ++D_count;

                } else {
                    VECTOR(rds_vec)[i] -= 1;
                    ++I_count;
                }
        }

        else if (ds == ::R) {

                ++R_count;

        }

        else if (ds == ::D) {

                ++D_count;
        }

        if (vs == ::V1) {
            ++V1_count;
            tv2 = VECTOR(tv2_vec)[i];
            if (tv2 > 0.0) VECTOR(tv2_vec)[i] -= 1;
        } else if (vs == ::V2) {
            ++V2_count;
        } else {
            continue;
        }

    }

    SETVANV(g, "disease_status", &ds_vec);
    SETVANV(g, "vaccine_status", &vs_vec);
    SETVANV(g, "remaining_days_exposed", &rde_vec);
    SETVANV(g, "remaining_days_sick", &rds_vec);
    SETVANV(g, "time_until_v2", &tv2_vec);
    igraph_vector_destroy(&ds_vec);
    igraph_vector_destroy(&vs_vec);
    igraph_vector_destroy(&rde_vec);
    igraph_vector_destroy(&rds_vec);
    igraph_vector_destroy(&mu_vec);
    igraph_vector_destroy(&tv2_vec);

    /* Tally edge counts */
    unordered_map<string, int> etypes;
    etypes["household"] = 0;
    etypes["work"] = 0;
    etypes["random"] = 0;
    for (int i = igraph_ecount(g); i--; ) {
        etypes[EAS(g, "type", i)] += 1;
    }



    SETGAN(g, "S_count", S_count);
    SETGAN(g, "E_count", E_count);
    SETGAN(g, "I_count", I_count);
    SETGAN(g, "R_count", R_count);
    SETGAN(g, "D_count", D_count);
    SETGAN(g, "V1_count", V1_count);
    SETGAN(g, "V2_count", V2_count);

    h->add_history(S_count, E_count, I_count, R_count, D_count, V1_count, V2_count);

    cout << "S: " << std::setw(5) << S_count << " | ";
    cout << "E: " << std::setw(5) << E_count << " | ";
    cout << "I: " << std::setw(5) << I_count << " | ";
    cout << "R: " << std::setw(5) << R_count << " | ";
    cout << "D: " << std::setw(5) << D_count << " | ";
    cout << "V1: " << std::setw(5) << V1_count << " | ";
    cout << "V2: " << std::setw(5) << V2_count << " | ";
    cout << "Edge counts: " << "Household: " <<  setw(5) << etypes["household"] << " Work " << setw(5) << etypes["work"] << " Random  " << setw(5) <<  etypes["random"];
    cout << flush;


}
