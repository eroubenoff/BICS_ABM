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

using namespace std;

#define MAXCHAR 1000




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
  igraph_empty(&graph, 0, IGRAPH_DIRECTED);

  // Read a line from the input file
  // Note: will have to figure out what to do with end of file
  // Possibly try/throw/catch block of some sort
  vector<string> row;




  // Loop through the attributes of the first 5 nodes
  // for (int i = 0; i < 5; i++) {
  int i = 0;
  while( fin.peek() != EOF ) {

    // Read the row
    row = get_csv_row(fin);

    // Add vertex 
    igraph_add_vertices(&graph, 1, NULL);

    // Add attributes one by one by looping through the header 
    SETVAN(&graph, "age", i, stoi(row[colnames["age"]]));
    if (row[colnames["gender"]] == "Male") {
      SETVAN(&graph, "gender", i, 1);
    } else {
      SETVAN(&graph, "gender", i, 0);
    }
    SETVAS(&graph, "ethnicity", i, row[colnames["ethnicity"]].c_str());
    SETVAN(&graph, "num_cc_nonhh", i, stoi(row[colnames["num_cc_nonhh"]]));
    SETVAS(&graph, "hhid", i, row[colnames["hhid"]].c_str());


    i++;

  }


  /* Print the final result. */
  print_attributes(&graph);


  // Create an unordered_map of hhids
  unordered_map<string, vector<int> > hhids;
  // for

  /* Delete all remaining attributes. */
  DELALL(&graph);

  /* Destroy the graph. */
  igraph_destroy(&graph);

  /* Close the fstream */
  fin.close();


  return 0;
}