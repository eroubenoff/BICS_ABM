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

#define MAXCHAR 1000


void delete_all_edges(igraph_t *g) {
  igraph_es_t es;
  igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
  igraph_delete_edges(g, es);
  igraph_es_destroy(&es);

}

void daytime_mixing(igraph_t *g) {

    igraph_rng_seed(igraph_rng_default(), 122);
  // Clear any edges present
  delete_all_edges(g);

  // Generate degree sequence from num_cc_nonhh
  igraph_t new_graph;
  igraph_vector_int_t num_cc_nonhh;
  igraph_vector_int_init(&num_cc_nonhh, igraph_vcount(g));

  default_random_engine generator;

  // Take a poisson value for each
  for (int i = 0; i < igraph_vcount(g); i++) {
    poisson_distribution<int> distribution(VAN(g, "num_cc_nonhh", i) / 10);
    VECTOR(num_cc_nonhh)[i]=distribution(generator);

  }


  // Need to make the degree sequence sum to even number
  // Do this by randomly sampling indices until we find a nonzero one
  // and decrement it
  int sum = igraph_vector_int_sum(&num_cc_nonhh);

  if (sum % 2 == 1) { 
    // Pick a random index
    int tries = 0;
    int randomIndex;
    while (tries < 1000) {
      randomIndex = rand() % igraph_vector_int_size(&num_cc_nonhh);

      if (VECTOR(num_cc_nonhh)[randomIndex] > 1) {
        VECTOR(num_cc_nonhh)[randomIndex] -= 1;
        break;
      }

      tries++;

    }
  }

  // Form degree sequence and get edgelist
  igraph_degree_sequence_game(&new_graph, &num_cc_nonhh, NULL, IGRAPH_DEGSEQ_CONFIGURATION);
  igraph_vector_int_t edgelist;
  igraph_vector_int_init(&edgelist, igraph_vcount(&new_graph));
  igraph_get_edgelist(&new_graph, &edgelist, false);

  // igraph_vector_int_print(&edgelist);

  // Add edgelist to old graph
  igraph_add_edges(g, &edgelist, NULL);


  igraph_vector_int_destroy(&num_cc_nonhh);
  igraph_vector_int_destroy(&edgelist);


}


int main() {




  // Create the empty graph
  igraph_t graph;

  /* Turn on attribute handling. */
  igraph_set_attribute_table(&igraph_cattribute_table);


  /* Create a directed graph with no vertices or edges. */
  igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);

 
  /* Generate graph by reading csv */
  // read_pop_from_csv("pop.csv", &graph);
  gen_pop_from_survey_csv("lucid/wave4.csv", &graph, 1000, true);

  // Create an unordered_map of hhids
  unordered_map<string, vector<int> > hhids;
  for (int i = 0; i < igraph_vcount(&graph); i++) {
    hhids[VAS(&graph, "hhid", i)].push_back(i);
  }

  // Create a household edge list
  igraph_vector_int_t hhedges;
  igraph_vector_int_init(&hhedges, 0);
  for (auto h : hhids) {

    // Create all combinations within the hh by nested loop
    for (int k=0; k < h.second.size(); k++) {
      for (int l=k+1; l < h.second.size(); l++) {
        if (k == l) {
          continue;
        }
        else{
          igraph_vector_int_push_back(&hhedges, h.second[k]);
          igraph_vector_int_push_back(&hhedges, h.second[l]);
        }
      }
     }
  }

  // Add hh edges
  igraph_add_edges(&graph, &hhedges, NULL);

  /* Print the final result. */
  print_attributes(&graph, true);


  // Pick nodes at random to be infected
  mt19937 generator(49);
  uniform_int_distribution<int> distribution(0,igraph_vcount(&graph));
  for (int i = 0; i < 5; i++) {
      int index_case = distribution(generator);
      cout << "index case " << index_case << endl;
        set_sick(&graph, index_case, 3*24, 5*24, false);
      cout << "Here! "<< endl;
  }

  disease_status(&graph);
  int day = 0;
  int hr = 0;
  while (GAN(&graph, "I_count") + GAN(&graph, "E_count") > 0){

    // Hours 0-8
    igraph_add_edges(&graph, &hhedges, NULL);
    for (hr = 0; hr < 8; hr++ ) {
      cout << "Day " << day <<  "Hour " << hr << "| ";
      transmit(&graph);
      decrement(&graph);
       // disease_status(&graph);
      
    }

    delete_all_edges(&graph);

    // Hours 8-16
    for (hr = 8; hr < 16; hr++){
      cout << "Day " << day <<  "Hour " << hr << "| ";
      daytime_mixing(&graph);
      transmit(&graph);
      decrement(&graph);
      // cout << igraph_ecount(&graph) << endl;
      // disease_status(&graph);
    }


    // Hours 16-24
    delete_all_edges(&graph);
    igraph_add_edges(&graph, &hhedges, NULL);
    for (hr = 16; hr < 24; hr++ ) {
      cout << "Day " << day <<  "Hour " << hr << "| ";
      transmit(&graph);
      decrement(&graph);
      // disease_status(&graph);
      }

    day++;

    // if (day > 10) {
    //   break;
    // }

  }

  /* Delete all remaining attributes. */
  DELALL(&graph);

  /* Destroy the graph. */
  igraph_destroy(&graph);

  // Destroy vectors
  igraph_vector_int_destroy(&hhedges);



  return 0;
}
