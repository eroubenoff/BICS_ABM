#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <igraph.h>
#include <random>
using namespace std;


/*
 * Wrapper around Vector that will always return 
 */
template <class T>
class CyclingVector {

    private: 
        vector<T> values;
        int index; 
        int size;

    public:
        CyclingVector() = default;

        CyclingVector(vector<T> _values) {
            index = 0;
            size = _values.size();
            values = _values;
        }

        template <class G>
        CyclingVector(int _size, G gen){
            size = _size;
            index = 0;
            values.resize(size);
            generate(values.begin(), values.end(), gen);
            
        }

        T next() {
            if (index==size-1) index = 0;
            else index++;

            return values[index];
            
        }
};

/* 
 * Class to sample randomly from a distribution
 *
 * Basically a wrapper around discrete_distribtuion
 * */

class RandomVector{
    private:
        vector<int> ids;
        discrete_distribution<int> dd;

    public:

        RandomVector() {}

        RandomVector(vector<int> _ids, vector<float> probs) {

            ids = _ids;
            dd = discrete_distribution<int>(probs.begin(), probs.end());

        }

        RandomVector(vector<float> probs) {

            for (int i = 0; i < probs.size(); i++) {
                ids.push_back(i);
            };
            dd = discrete_distribution<int>(probs.begin(), probs.end());

        }

        // template<class G>
        int operator()(mt19937 &generator) {
            return ids[dd(generator)];

        }

};

class History {
    private:
        vector<int> S;
        vector<int> E;
        vector<int> I;
        vector<int> R;
        vector<int> D;
        vector<int> V1;
        vector<int> V2;
        int counter;
        int length;
    public: 
        History(int initial_length = 2000);
        History() {};
        void add_history(int _S, int _E, int _I, int _R, int _D, int _V1, int _V2);
        void save(string path = "history.csv");
};
/*
 * Reads a csv and single row 
 * 
 * @param fstream fin 
 *  - File pointer
 * @param int expected_length 
 *  - The expected number of fields in the csv. 
 *    If -1, does nothing, if not, will pad the row with "-1".
 *
 * @return vector<string> containing the values of the row 
 * If no data present (i.e., empty string) returns "-1" in place
 *
 */

vector<string> get_csv_row(istream &fin, int expected_length = -1, char sep = ',');

/* 
 * Read a population from a csv
 * 
 * @param string path 
 * 	path of the already-generated population, probably from python
 * @param igraph_t &graph
 *	pointer to graph to initialize population in 		
 * 					  

 * @return none; modifies &graph in place
 */
void read_pop_from_csv(string path, igraph_t *graph);
void gen_pop_from_survey_csv(string path, igraph_t *g, int n, bool fill_polymod = true);



/*
 * Prints attributes from graph. Borrowed from the igraph documentation
 * 
 * @param igraph_t *g pointer to graph 
 * @return void
 */

void print_attributes(const igraph_t *g, bool nodes_only = false);

/* 
   Loops through all edges and transmits if a susceptible node is in contact
   with an infectious node

   @param CyclingVector *gamma_vec, *sigma_vec: 
    - pointers to vectors that contain random draws of latent and infectious
    periods. CyclingVector has method .next() which returns the next value
    and loops to the beginning when exhausted.
   @return none
   */

void transmit(igraph_t *g, 
        unordered_map<int, CyclingVector<int>* > &beta_vec, 
        CyclingVector<int> &gamma_vec, 
        CyclingVector<int> &sigma_vec, 
        unordered_map<string, CyclingVector<int> > &mu) ;


/*
   Sets a node to be sick, parameterized with all conditions needed for the 
   rest of the node's lifecycle.

   @param int n: index of node to set sick
   @param int rde: time the node will spend exposed
   @param int rds: time the node will spend sick
   @param bool mu: if, at the end of rds, the node will die
   @return none 
   */
void set_sick(igraph_t *g, int n, int rde, int rds, bool mu);


/* 
   DEPRECATED; NOW DONE IN decrement 

   Tallies up the disease status of all nodes and sets it them to graph attributes
   */
void disease_status(igraph_t *g);


/* 
   Decrements each node's progression through disease time
   */
void decrement(igraph_t *g, History *h);

/*
 * Generate a random string (for hash)
 */
string randstring(int length);



vector<float> stovf(string s) ;

void distribute_vax(igraph_t *g, int n_daily, int time_until_v2);

