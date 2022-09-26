#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <igraph.h>
using namespace std;



/*
 * Reads a csv and single row 
 * 
 * @param fstream fin File pointer
 * @return vector<string> containing the values of the row 
 */

vector<string> get_csv_row(istream &fin);



/*
 * Prints attributes from graph. Borrowed from the igraph documentation
 * 
 * @param igraph_t *g pointer to graph 
 * @return void
 */

void print_attributes(const igraph_t *g);


/*
 * Main class used for simulation
 * 
 */

// class Population {
// private:
// 	// Base object
// 	igraph_t graph;

// 	// Vector of hh edges 
// 	igraph_vector_int_t hhedges;	

// public:
// 	// Constructor to create a population from a cached dataset
// 	Population(string path);
// 	~Population();

// 	// Hashmap of households
// 	unordered_map<string, vector<int> > hhids;

// 	// Connect household edges
// 	// void connect_hh_edges();

// 	// Wrapper around print_attributes()
// 	void print();




// };