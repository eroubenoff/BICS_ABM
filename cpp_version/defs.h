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

// class Population() {
// private:
// 	igraph_t graph;

// public:
// 	unordered_map households;

// }