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

// Loops through each edge and transmits
void transmit(igraph_t *g) {
    for (int i = 0; i < igraph_ecount(g); i++) {

      // Get node ids
      int n1 = IGRAPH_FROM(g, i);
      int n2 = IGRAPH_TO(g, i);

      // Get disease status of each 
      string n1_ds = VAS(g, "disease_status", n1);
      string n2_ds = VAS(g, "disease_status", n2);

      if (n1_ds == "I" & n2_ds == "S") {
        // Transmit to n2
        SETVAS(g, "disease_status", n2, "E");
        SETVAN(g, "remaining", n2, 3*24);
      } 
      else if(n1_ds == "S" & n2_ds == "I") {
        // Transmit to n1
        SETVAS(g, "disease_status", n1, "E");
        SETVAN(g, "remaining_days_exposed", n1, 3*24);
      }
      else {
        continue;
      }

  }

}


void disease_status(igraph_t *g) {

  int S_count = 0;
  int E_count = 0;
  int I_count = 0;
  int R_count = 0;
  int D_count = 0;

  for (int i = 0; i < igraph_vcount(g); i++) {
    string ds = VAS(g, "disease_status", i);

    if (ds == "S") {
      S_count++;
    }
    else if (ds == "E") {
      E_count++;
    }
    else if (ds == "I") {
      I_count++;
    } 
    else if (ds == "R") {
      R_count++; 
    }
    else if (ds == "D") {
      D_count++;
    }
  }

  cout << "S: " << S_count << " | ";
  cout << "E: " << E_count << " | ";
  cout << "I: " << I_count << " | ";
  cout << "R: " << R_count << " | ";
  cout << "D: " << D_count << " | ";
  cout << endl;
}

void decrement(igraph_t *g) {
    for (int i = 0; i < igraph_vcount(g); i++) {

      string ds = VAS(g, "disease_status", i);

      if (ds == "E") {
         int rde = VAN(g, "remaining_days_exposed", i);
         if (rde == 0) {
          SETVAS(g, "disease_status", i, "I");
          SETVAN(g, "remaining_days_sick", i, 5*24);
         } else {
           SETVAN(g, "remaining_days_exposed", i, rde - 1);
         }
      }

      if (ds == "I") {
         int rds = VAN(g, "remaining_days_sick", i);
         if (rds == 0) {
          SETVAS(g, "disease_status", i, "R");
         } else {
           SETVAN(g, "remaining_days_sick", i, rds - 1);
         }
      }


  }
}

void daytime_mixing(igraph_t *g) {
  // Clear any edges present
  igraph_delete_edges(g, igraph)

}

int main() {

  // Create pointer to pop file and open
  ifstream fin;
  fin.open("pop.csv");

  // Read the csv header (used to create attributes)
  vector<string> header = get_csv_row(fin);

  // Invert header into unordered map for column lookup
  unordered_map<string, int> colnames;
  for (int i = 0; i < header.size(); i++) {
    colnames[header[i]] = i;
  }


  // Create the empty graph
  igraph_t graph;

  /* Turn on attribute handling. */
  igraph_set_attribute_table(&igraph_cattribute_table);


  /* Create a directed graph with no vertices or edges. */
  igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);

  vector<string> row;

  // Read a line from the input file

  // Loop through the attributes of the first 5 nodes
  int i = 0;
  while( fin.peek() != EOF ) {

    // Read the row
    row = get_csv_row(fin);

    // Add vertex 
    igraph_add_vertices(&graph, 1, NULL);

    // Age
    SETVAN(&graph, "age", i, stoi(row[colnames["age"]]));

    // Disease Status
    SETVAS(&graph, "disease_status", i, "S");

    // Gender
    SETVAN(&graph, "gender", i, (row[colnames["gender"]] == "Male"? 1 : 0) );

    // Ethnicity
    SETVAS(&graph, "ethnicity", i, row[colnames["ethnicity"]].c_str());

    // Num cc nonhh
    SETVAN(&graph, "num_cc_nonhh", i, stoi(row[colnames["num_cc_nonhh"]]));

    // Hhid
    SETVAS(&graph, "hhid", i, row[colnames["hhid"]].c_str());

    // Remaining days exposed/sick
    SETVAN(&graph, "remaining_days_exposed", i, -1);
    SETVAN(&graph, "remaining_days_sick", i, -1);


    i++;

  }

  // Create an unordered_map of hhids
  unordered_map<string, vector<int> > hhids;
  
  for (i = 0; i < igraph_vcount(&graph); i++) {
    hhids[VAS(&graph, "hhid", i)].push_back(i);
  }

  // Create a household edge list
  igraph_vector_int_t hhedges;
  igraph_vector_int_init(&hhedges, 0);
  for (auto h : hhids) {

    // Create all combinations within the hh by nested loop
    for (int k=0; k < h.second.size(); k++) {
      for (int l=0; l < h.second.size(); l++) {
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
  print_attributes(&graph);


  // Pick nodes at random to be infected
  default_random_engine generator;
  uniform_int_distribution<int> distribution(0,igraph_vcount(&graph));
  for (i = 0; i < 5; i++) {
    SETVAS(&graph, "disease_status", distribution(generator), "I");
  }

  disease_status(&graph);
  for (int hr = 0; hr < 10*24; hr++ ) {
    transmit(&graph);
    decrement(&graph);
    disease_status(&graph);
  }

  /* Delete all remaining attributes. */
  DELALL(&graph);

  /* Destroy the graph. */
  igraph_destroy(&graph);

  // Destroy vectors
  igraph_vector_int_destroy(&hhedges);

  /* Close the fstream */
  fin.close();


  return 0;
}