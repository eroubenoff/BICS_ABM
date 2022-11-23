#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <igraph.h>
#include <random>
#include <map>
using namespace std;

/* 
 * Disease and vaccine status as globals 
 *
 * This is done so that they can be internally
 * represented as ints, which is much faster than
 * strings with the igraph api. They are globals for consistency. 
 * */

extern int S;
extern int E;
extern int I;
extern int R;
extern int D;
extern int V0;
extern int V1;
extern int V2;
extern int VW;
extern int VBoost;

/* 
 * key is a tuple used to index nodes. Values correspond to:
 *
 * @param int hhsize
 * @param string age
 * @param gender
 *
 * */
typedef tuple <int,string,string> key;


/*
 * Rule is a vector of tuples.  Each tuple is a (column, value) 
 * condition. In order to be 
 * eligible for vaccination, a node must meet all of
 * the conditions.  EX:
 *
 * {("age", ">85"),("is_employed", "true")}
 *
 * Means that only employed  85+ year olds are eligible
 * for vaccination.
 *
 * Note that all values must be strings.
 * */
typedef vector<tuple<string,string>> rule;


/*
 * Struct for passing parameters to the function.
 * Is designated extern "C" so that can interface with python.
 *
 * NB: in python ctypes structs MUST be in the 
 * same order as the C-implemented struct.
 *
 */
extern "C" typedef struct Params {

    int N_HH;
    int WAVE;
    int GAMMA_MIN;
    int GAMMA_MAX;
    int SIGMA_MIN;
    int SIGMA_MAX;
    float BETA;
    float MU_VEC[9];
    int INDEX_CASES;
    int SEED;
    int POP_SEED;
    int N_VAX_DAILY;
    float VE1;
    float VE2;
    float ISOLATION_MULTIPLIER;

    char VAX_RULES_COLS[1000];
    char VAX_RULES_VALS[1000];
    int VAX_CONDS_N[100];
    int VAX_RULES_N;


} Params;


/*
 * Function to initialize parameter values. 
 * See documentation for values. 
 *
 * @param none
 * @return Params object with baseline values. 
 *
 */
extern "C" Params init_params(); 




/*
 * CyclingVector is a template class that is essentially
 * a wrapper around std::vector. The point of this
 * class is to pre-generate a series of (pseudo-random) values and 
 * to return the next one. Mainly includes a method, .next(),
 * that does so. When all values are exausted, the vector
 * starts over. 
 *
 *
 *
 * Sample usage: 
 *
 * vector<int> v{3,2,1};
 * CyclingVector<int> cv = CyclingVector(v);
 * 
 * cv.next(); // Returns 3
 * cv.next(); // Returns 2 
 * cv.next(); // Returns 1
 * cv.next(); // Returns 3
 *
 * 
 * Constructor will also take a generator
 *
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
 * RandoClass to sample randomly from a distribution
 *
 * Basically a wrapper around discrete_distribtuion
 * */

class RandomVector{
    private:
        vector<int> ids;
        mutable discrete_distribution<int> dd;

    public:

        RandomVector() {}

        RandomVector(vector<int> _ids, vector<float> probs) {

            /* Filter out the zero probs */
            for (int i = probs.size(); i--; ) {
                if (probs[i] == 0) {
                    probs.erase(probs.begin() + i - 1);
                    _ids.erase(_ids.begin() + i - 1);
                }
            }

            ids = _ids;
            dd = discrete_distribution<int>(probs.begin(), probs.end());

        }

        RandomVector(vector<float> probs) {

            for (int i = 0; i < probs.size(); i++) {
                ids.push_back(i);
            };

            /* Filter out the zero probs */
            for (int i = probs.size(); i--; ) {
                if (probs[i] == 0) {
                    probs.erase(probs.begin() + i - 1);
                    ids.erase(ids.begin() + i - 1);
                }
            }

            dd = discrete_distribution<int>(probs.begin(), probs.end());

        }

        // template<class G>
        int operator()(mt19937 &generator) {
            return ids[dd(generator)];

        }

        int operator()(mt19937 &generator) const {
            return ids.at(dd(generator));

        }

};

class History {
    public: 
        vector<int> S;
        vector<int> E;
        vector<int> I;
        vector<int> R;
        vector<int> D;
        vector<int> V1;
        vector<int> V2;
        int counter;
        int length;
        History(int initial_length = 2000);
        void add_history(int _S, int _E, int _I, int _R, int _D, int _V1, int _V2);
        void save(string path = "history.csv");
        void plot_trajectory(string path = "plot.png") ;
};




class Data {

    private: 
        void load_BICS(int wave, string path="data/df_all_waves.csv");
        void load_POLYMOD(string path = "data/POLYMOD/");
        void create_sampling_distns(); 
        vector<vector<string>> BICS_data;
        vector<vector<string>> POLYMOD_data;

    public:

        int BICS_nrow;
        vector<float> BICS_weights;
        unordered_map<string, int> BICS_colnames;

        /* 
         * Accessor functions 
         *
         * @param int i: index of the row (repondent)
         * @param string colname to retrieve. 
         *
         * @return the value as a string
         * */
        string BICS(int i, string colname) const {
            try {
                BICS_data.at(i);
            } 
            catch (out_of_range) {
                throw invalid_argument("Vertex " + to_string(i) + " not present in BICS data. Highest value is " + to_string(BICS_nrow-1));
            }
            try {
                BICS_data.at(i).at(BICS_colnames.at(colname));
            } 
            catch (out_of_range) {
                throw invalid_argument("Columnn " + colname + " not present in BICS data");
            }

            return BICS_data.at(i).at(BICS_colnames.at(colname));
        };

        /* 
         * Accessor functions 
         *
         * @param int i: index of the row (repondent)
         * 
         * @return the whole row as an unorderd_map of strings.
         * */
        vector<string> BICS(int i) const {
            try {
                BICS_data.at(i);
            } 
            catch (out_of_range) {
                throw invalid_argument("Vertex " + to_string(i) + " not present in BICS data. Highest value is " + to_string(BICS_nrow-1));
            }

            return BICS_data.at(i);
        };

        int POLYMOD_nrow;
        unordered_map<string, int> POLYMOD_colnames;

        /* 
         * Accessor functions 
         *
         * @param int i: index of the row (repondent)
         * @param string colname to retrieve. 
         *
         * @return the value as a string
         * */
        string POLYMOD(int i, string colname) const {
            try {
                POLYMOD_data.at(i);
            } 
            catch (out_of_range) {
                throw invalid_argument("Vertex " + to_string(i) + " not present in POLYMOD data. Highest value is " + to_string(POLYMOD_nrow-1));
            }
            try {
                POLYMOD_data.at(i).at(POLYMOD_colnames.at(colname));
            } 
            catch (out_of_range) {
                throw invalid_argument("Columnn " + colname + " not present in POLYMOD data");
            }
            return POLYMOD_data.at(i).at(POLYMOD_colnames.at(colname));
        };
        /* 
         * Accessor functions 
         *
         * @param int i: index of the row (repondent)
         * 
         * @return the whole row as a vector of strings.
         * */
        vector<string> POLYMOD(int i) const {
            try {
                POLYMOD_data.at(i);
            } 
            catch (out_of_range) {
                throw invalid_argument("Vertex " + to_string(i) + " not present in POLYMOD data. Highest value is " + to_string(POLYMOD_nrow-1));
            }
            return POLYMOD_data.at(i);
        };

        map<key, vector<int>> eligible_nodes;
        map<key, vector<int>> eligible_POLYMOD;
        map<key, RandomVector> distributions_POLYMOD;
        map<key, RandomVector> hh_distn;




        // Constructor
        Data(int wave);
        Data(){};





};

/* Global data object */ 
// extern vector<Data> data;


/* Wrapper class for lazy loading */
class Database{
    private:
        unordered_map<int, Data*> data;
    public: 
        Data* operator[](int i) {
            //  Check bounds 
            if (i < 0 || i > 6) throw runtime_error("Only valid waves are between 0 and 6");
            //  If i is not in keys, load it 
            if (data.find(i) == data.end()) {
                data[i] = new Data(i);
            }
            // either way, return a pointer to it 
            return data[i];
        }
        ~Database() {
            for (auto i: data) {
                delete i.second;
            }
        }

};

extern Database database;

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
// void read_pop_from_csv(string path, igraph_t *graph);


void gen_pop_from_survey_csv(const Data *data, igraph_t *g,const Params *params); // Defualt is no vaccine rules


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




void distribute_vax(igraph_t *g, int n_daily, int time_until_v2, int time_until_vw, int time_until_vboost) ;

vector<float> stovf(string s);





/* Python interface */

string recode_age(string age_s) ;
string recode_gender(string gender) ;

void BICS_ABM(const Data *data, const Params *params, History *history);

/*
 * C interface to parse vaccine rules. Arguments are:
 *
 * @param char Colname[] string of colnames in the data, comma-separated
 * @param char Value[] string of values, comma-separated
 * @param int n_conditions the number of conditions per rule
 *
 * So, if we wanted to vaccinate elderly women first, and then 
 * workers, would pass:
 *
 *
 *
 */
vector<rule> parse_vax_rules(const char Colname[], const char Value[], const int n_conditions[], const int n_rules) ;

/* Global data object */ 
// Data data(4);
//
void add_vertex(igraph_t *g,
        const string age,
        string gender,
        string ethnicity,
        int num_cc_nonhh, 
        int vaccine_priority,
        string hhid);

void set_vaccine_priority(
        igraph_t *g, 
        const vector<vector<tuple<string, string>>> rules,
        mt19937 generator
        );

