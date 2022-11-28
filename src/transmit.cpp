#include<igraph.h>
#include<vector>
#include<string>
#include<iostream>
#include "BICS_ABM.h"
#include <random>
using namespace std;

void transmit(igraph_t *g,
        unordered_map<int, CyclingVector<int>> &beta_vec,
        CyclingVector<int> &gamma_vec, 
        CyclingVector<int> &sigma_vec, 
        unordered_map<string, CyclingVector<int> > &mu,
        int t_reinfection){

    int vcount= igraph_vcount(g);
    igraph_vector_int_t neighbors;
    igraph_vector_int_init(&neighbors, 0);

    // Faster to compare int than char, avoids call to strcmp 
    int ds; // Disease status of infectious node
    int ds2; // Disease status of neighbor node
    int n2; // Number of neighbors and index of neighbor node
    int vs2;         // 

    bool vs_next;

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
                        vs_next = beta_vec[::V0].next();
                } else if (vs2 == ::V1){
                        vs_next = beta_vec[::V1].next();
                } else if (vs2 == ::V2) {
                        vs_next = beta_vec[::V2].next();
                } else if (vs2 == ::VW) {
                        vs_next = beta_vec[::VW].next();
                } else if (vs2 == ::VBoost) {
                        vs_next = beta_vec[::VBoost].next();
                } else {
                        cout << "Error in switch " << endl;
                        vs_next = beta_vec[::V0].next();
                }
                if (vs_next) {
                    set_sick(g, n2, gamma_vec.next(), sigma_vec.next(), mu[VAS(g, "age", n2)].next(), t_reinfection, ::Ic);
                }
            }
        }
        else continue;
    }

    igraph_vector_destroy(&ds_vec);
    igraph_vector_destroy(&vs_vec);
    igraph_vector_int_destroy(&neighbors);


}
