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

    int n_nodes = igraph_vcount(g);
    igraph_vector_int_t neighbors;
    igraph_vector_int_init(&neighbors, 0);

    // Faster to compare int than char, avoids call to strcmp 
    short int ds; // Disease status of infectious node
    short int dsn; // Disease status of neighbor node
    short int S = 'S'; // Constant int for S
    short int I = 'I'; // Constant int for i 
    int n_neighbors, n2; // Number of neighbors and index of neighbor node

    short int vs;
    bool vs_next;

    /* Slight optimization to unwrap the unordered_maps */
    CyclingVector<int>* beta0 = beta_vec[0];
    CyclingVector<int>* beta1 = beta_vec[1];
    CyclingVector<int>* beta2 = beta_vec[2];

    for (int i = n_nodes; i--; ) {
        ds = VAS(g, "disease_status", i)[0];

        if (ds == I) {
            igraph_neighbors(g, &neighbors, i, IGRAPH_ALL); 
            n_neighbors = igraph_vector_int_size(&neighbors);
            for (n_neighbors; n_neighbors--; ) {
                n2 = VECTOR(neighbors)[n_neighbors];
                dsn = VAS(g, "disease_status", n2)[0];
                if (dsn != S) continue; 

                vs = VAN(g, "vaccine_status", n2);
                switch (vs){
                    case 0:
                        vs_next = beta0 -> next();
                        break;
                    case 1:
                        vs_next = beta1 -> next();
                        break;
                    case 2:
                        vs_next = beta2 -> next();
                        break;
                    default:
                        cout << "Error in switch " << endl;
                        vs_next = beta0 -> next();
                        break;
                }
                // vs_next = beta_vec[vs] -> next();
                if (vs_next) {
                    set_sick(g, n2, gamma_vec.next(), sigma_vec.next(), mu[VAS(g, "age", n2)].next());
                }
            }
        }
        else continue;
    }

    igraph_vector_int_destroy(&neighbors);


}
