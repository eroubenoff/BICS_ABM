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
#include<iomanip>
using namespace std;

/* 
 * Version of the Decrement function that works with the new
 * update event handlers 
 * */

/* 
 * If susceptible do nothing
 */
inline void decrement_S(int i) {
    return;
}

/*
 * If exposed and RDE = 0, set to infectious,
 * otherwise decrement RDE
 */
inline void decrement_E(UpdateList &ul, int i, double rde, int symptomatic) {
    if (rde == 0) {
        if (symptomatic == _Ic) {
            ul.add_update(UpdateVertexAttribute(i, "disease_status", _Ic));
            ul.add_update(UpdateVertexAttribute(i, "remaining_days_exposed", -1));
        } 
        else if (symptomatic == _Isc) {
            ul.add_update(UpdateVertexAttribute(i, "disease_status", _Isc));
            ul.add_update(UpdateVertexAttribute(i, "remaining_days_exposed", -1));
        }
    } else {
        ul.add_update(UpdateVertexAttribute(i, "remaining_days_exposed", rde - 1));
    }
}

/*
 * If infectious and RDS=0, either set to R or D
 * depending on mu
 * Otherwise decrement RDS
 */
inline void decrement_Ic(UpdateList &ul, int i, double rds, double mu) {
    if ((rds == 0.0) & (mu == 0.0)) {
        ul.add_update(UpdateVertexAttribute(i, "disease_status", _R));
        ul.add_update(UpdateVertexAttribute(i, "remaining_days_sick", -1));

    } else if ((rds == 0.0 ) & (mu == 1.0) ) {
        ul.add_update(UpdateVertexAttribute(i, "disease_status", _D));
        ul.add_update(UpdateVertexAttribute(i, "remaining_days_sick", -1));

    } else {
        ul.add_update(UpdateVertexAttribute(i, "remaining_days_sick", rds - 1));
    }
}

inline void decrement_Isc(UpdateList &ul, int i, double rds) {
    if (rds == 0.0) {
        ul.add_update(UpdateVertexAttribute(i, "disease_status", _R));
    } else {
        ul.add_update(UpdateVertexAttribute(i, "remaining_days_sick", rds - 1));
    }

}

/*
 * If recovered, do nothing
 */
inline void decrement_R(UpdateList &ul, int i, double tsus) {
    if (tsus == 0 ) {
        ul.add_update(UpdateVertexAttribute(i, "disease_status", _S));
        ul.add_update(UpdateVertexAttribute(i, "time_until_susceptible", -1));
    }
    else {
        ul.add_update(UpdateVertexAttribute(i, "time_until_susceptible", tsus - 1));
    }

}

/* If dead, do nothing */
inline void decrement_D(int i) {
    return;
}

/*
 * If vaccine status is V1, then decrement tv2
 * and wait until the next vaccine distribution
 */
inline void decrement_V1(UpdateList &ul, int i, double tv2){
    if (tv2 > 0.0) {
        ul.add_update(UpdateVertexAttribute(i, "time_until_v2", tv2 -1)); 
    }
}

/*
 * If status is V2 decrement tvw (time until waned immunity)
 * until 0
 * If tvw == 0, set vaccine status to VW
 */
inline void decrement_V2(UpdateList &ul, int i, double tvw) {
    if (tvw > 0.0) {
        ul.add_update(UpdateVertexAttribute(i, "time_until_vw", tvw - 1));
    }
    else {
        ul.add_update(UpdateVertexAttribute(i, "vaccine_status", _VW));
        ul.add_update(UpdateVertexAttribute(i, "time_until_vw", -1));
    }

}

/*
 * If vaccine status is VW, decrement tvboost.
 * If tvboost == 0, wait
 * until next vaccine distribution.
 */
inline void decrement_VW(int i) {
    return;
}

inline void decrement_VBoost(UpdateList &ul, int i, double tvw){
    if (tvw > 0.0) {
        ul.add_update(UpdateVertexAttribute(i, "time_until_vw", tvw -1));
    }
    else {
        ul.add_update(UpdateVertexAttribute(i, "vaccine_status", _VW));
        ul.add_update(UpdateVertexAttribute(i, "time_until_vw", -1));
    }
}

inline void decrement_hh(int i){
    return;
}

inline void decrement_random(int i, igraph_vector_t &durations) {

    if (VECTOR(durations)[i] > 1) {
        VECTOR(durations)[i] = VECTOR(durations)[i] - 1;

    } else {
        VECTOR(durations)[i] = -1;

    }

}

void decrement(igraph_t *g, History *h, int Cc, int Csc, bool print) {
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

    UpdateList ul;

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


    igraph_vector_t tsus_vec;
    igraph_vector_init(&tsus_vec, vcount);
    VANV(g, "time_until_susceptible", &tsus_vec);

    /* 
     * Loop through each vertex
     */ 
    for (int i = vcount; i--; ) {
        ds = VECTOR(ds_vec)[i]; 
        vs = VECTOR(vs_vec)[i];

        if (ds == _S){
            S_count++;
            decrement_S(i);
        }
        else if (ds == _E) {
            E_count++;
            decrement_E(ul, i, VECTOR(rde_vec)[i], VAN(g, "symptomatic", i));
        }
        else if (ds == _Ic) {  
            Ic_count++;
            decrement_Ic(ul, i, VECTOR(rds_vec)[i], VECTOR(mu_vec)[i]); 
        }
        else if (ds == _Isc) {  
            Isc_count++;
            decrement_Isc(ul, i, VECTOR(rds_vec)[i]); 
        }
        else if (ds == _R) {
            R_count++;
            decrement_R(ul, i, VECTOR(tsus_vec)[i]);
        }
        else if (ds == _D) {
            D_count++;
            decrement_D(i);
        }
        

        if (vs == _V1) {
            V1_count++;
            decrement_V1(ul, i, VECTOR(tv2_vec)[i]);
        } 
        else if (vs == _V2) {
            V2_count++;
            decrement_V2(ul, i, VECTOR(tvw_vec)[i]);
        } 
        else if (vs == _VW){
            VW_count++;
            decrement_VW(i);
        }
        else if (vs == _VBoost) {
            VBoost_count++;
            decrement_VBoost(ul, i, VECTOR(tvw_vec)[i]);
        }
        
    }


    //SETVANV(g, "disease_status", &ds_vec);
    //SETVANV(g, "vaccine_status", &vs_vec);
    //SETVANV(g, "remaining_days_exposed", &rde_vec);
    //SETVANV(g, "remaining_days_sick", &rds_vec);
    //SETVANV(g, "time_until_v2", &tv2_vec);
    //SETVANV(g, "time_until_vw", &tvw_vec);
    //SETVANV(g, "time_until_susceptible", &tsus_vec);
    igraph_vector_destroy(&ds_vec);
    igraph_vector_destroy(&vs_vec);
    igraph_vector_destroy(&rde_vec);
    igraph_vector_destroy(&rds_vec);
    igraph_vector_destroy(&mu_vec);
    igraph_vector_destroy(&tv2_vec);
    igraph_vector_destroy(&tvw_vec);
    igraph_vector_destroy(&tsus_vec); 

    ul.add_updates_to_graph(g);


    /* Tally edge counts
     *
     * For random edges, decrement the 'duration' if it is greater
     * than 1 hr, otherwise terminate
     *
     * */
    int hh_count = 0; 
    int random_count = 0;
    
    igraph_vector_t etypes;
    igraph_vector_init(&etypes, 0);
    EANV(g, "type", &etypes);
    igraph_vector_t durations;
    igraph_vector_init(&durations, 0);
    EANV(g, "duration", &durations);

    for (int i = igraph_ecount(g); i--; ) {
        if (VECTOR(etypes)[i] == _Household) {
            hh_count++;
            decrement_hh(i);
        }
        if (VECTOR(etypes)[i] == _Random) {
            random_count++;
            decrement_random(i, durations);
        }
    }

    SETEANV(g, "duration", &durations);
    igraph_vector_destroy(&etypes);
    igraph_vector_destroy(&durations);


    h->add_history(S_count, E_count, Ic_count, Cc, Isc_count, Csc, R_count, D_count, V1_count, V2_count, VW_count, VBoost_count, hh_count + random_count);

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


}
