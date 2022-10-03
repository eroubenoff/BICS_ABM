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
#include "defs.h"
#include <random>
using namespace std;




void transmit(igraph_t *g) {

    int n_nodes = igraph_vcount(g);
    igraph_vector_int_t neighbors;
    igraph_vector_int_init(&neighbors, 0);
    string ds;

    for (int i = 0; i < n_nodes; i++) {
        ds = VAS(g, "disease_status", i);
        // cout << ds << endl;

        if (ds == "I") {
           igraph_neighbors(g, &neighbors, i, IGRAPH_ALL); 
           int n_neighbors = igraph_vector_int_size(&neighbors);
           for (int j = 0; j < n_neighbors; j++) {
               int n2 = VECTOR(neighbors)[j];
                if (strcmp(VAS(g, "disease_status", n2), "S") == 0 ) {
                    set_sick(g, n2, 3*24, 5*24, false);
                }
           }
        }
        else continue;
    }

    igraph_vector_int_destroy(&neighbors);

    if (false) {
    int edges = igraph_ecount(g);

    for (int i = 0; i < edges; i++) {

      // Get node ids
      int n1 = IGRAPH_FROM(g, i);
      int n2 = IGRAPH_TO(g, i);

      // cout << n1 << " " << n2 << endl;

      // Get disease status of each 
      string n1_ds = VAS(g, "disease_status", n1);
      string n2_ds = VAS(g, "disease_status", n2);

      if (n1_ds == "I" & n2_ds == "S") {
        // Transmit to n2
        set_sick(g, n2, 3*24, 5*24, false);
      } 
      else if(n1_ds == "S" & n2_ds == "I") {
        // Transmit to n1
        set_sick(g, n1, 3*24, 5*24, false);
      }
      else {
        continue;
      }

  }
    }

}
