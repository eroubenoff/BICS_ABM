#include<igraph.h>
#include<vector>
#include<string>
#include<iostream>
#include "BICS_ABM.h"
#include <random>
using namespace std;

void transmit(igraph_t *g, 
        unordered_map<int, CyclingVector<int>* > &beta_vec,
        CyclingVector<int> &gamma_vec, 
        CyclingVector<int> &sigma_vec, 
        unordered_map<string, CyclingVector<int> > &mu ){

    int vcount= igraph_vcount(g);
    igraph_vector_int_t neighbors;
    igraph_vector_int_init(&neighbors, 0);

    // Faster to compare int than char, avoids call to strcmp 
    int ds; // Disease status of infectious node
    int dsn; // Disease status of neighbor node
    int n2; // Number of neighbors and index of neighbor node

    short int vs;
    bool vs_next;

    /* Slight optimization to unwrap the unordered_maps */
    CyclingVector<int>* beta0 = beta_vec[0];
    CyclingVector<int>* beta1 = beta_vec[1];
    CyclingVector<int>* beta2 = beta_vec[2];

    /* Pull attributes from g */
    igraph_vector_t ds_vec;
    igraph_vector_init(&ds_vec, vcount);
    VANV(g, "disease_status", &ds_vec);
    igraph_vector_t vs_vec;
    igraph_vector_init(&vs_vec, vcount);
    VANV(g, "vaccine_status", &vs_vec);
    igraph_vector_t rde_vec;
    igraph_vector_init(&rde_vec, vcount);

    for (int i = vcount; i--; ) {
        ds = VECTOR(ds_vec)[i]; 

        if (ds == ::I) {
            igraph_neighbors(g, &neighbors, i, IGRAPH_ALL); 
            for (int n_neighbors = igraph_vector_int_size(&neighbors) ; n_neighbors--; ) {
                n2 = VECTOR(neighbors)[n_neighbors];
                dsn = VECTOR(ds_vec)[n2];  
                if (dsn != ::S) continue; 

                vs = VECTOR(vs_vec)[i]; 
                if (vs == ::V0){
                        vs_next = beta0 -> next();
                } else if (vs == ::V1){
                        vs_next = beta1 -> next();
                } else if (vs == ::V2) {
                        vs_next = beta2 -> next();
                } else {
                        cout << "Error in switch " << endl;
                        vs_next = beta0 -> next();
                }
                // vs_next = beta_vec[vs] -> next();
                if (vs_next) {
                    set_sick(g, n2, gamma_vec.next(), sigma_vec.next(), mu[VAS(g, "age", n2)].next());
                }
            }
        }
        else continue;
    }

    igraph_vector_destroy(&ds_vec);
    igraph_vector_destroy(&vs_vec);
    igraph_vector_int_destroy(&neighbors);


}
