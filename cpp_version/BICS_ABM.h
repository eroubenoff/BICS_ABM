#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <igraph.h>
#include <random>
#include <map>
using namespace std;


class Params {

    private:
        int N_HH;
        int WAVE;
        int GAMMA_MIN;
        int GAMMA_MAX;
        int SIGMA_MIN;
        int SIGMA_MAX;
        float BETA;
        vector<float> MU_VEC;
        int INDEX_CASES;
        int SEED;
        int POP_SEED;
        int N_VAX_DAILY;
        float VE1;
        float VE2;

    public: 
        /* Default constructor */
        Params() {
            N_HH = 1000;
            WAVE = 4 ;
            GAMMA_MIN = 2*24;
            GAMMA_MAX = 4*24;
            SIGMA_MIN = 3*24;
            SIGMA_MAX = 7*24;
            BETA = 0.1;
            MU_VEC = {0.00001, 0.0001, 0.0001, 0.001, 0.001, 0.001, 0.01, 0.1, 0.1};
            INDEX_CASES = 5;
            SEED = 4949;
            POP_SEED = 4949;
            N_VAX_DAILY = N_HH / 20;
            VE1 = 0.75;
            VE2 = 0.95;
        } ;

        int n_hh() const {
            return N_HH;
        }
        void n_hh(int N_HH_) {
            N_HH = N_HH_;
        }

        int wave() const {
            return WAVE;
        }
        void wave(int WAVE_) {
            WAVE = WAVE_;
        }

        int gamma_min() const {
            return GAMMA_MIN;
        }
        void gamma_min(int GAMMA_MIN_) {
            GAMMA_MIN= GAMMA_MIN_;
        }

        int gamma_max() const {
            return GAMMA_MAX;
        }
        void gamma_max(int GAMMA_MAX_) {
            GAMMA_MAX= GAMMA_MAX_;
        }

        int sigma_min() const {
            return SIGMA_MIN;
        }
        void sigma_min(int SIGMA_MIN_) {
            SIGMA_MIN= SIGMA_MIN_;
        }

        int sigma_max() const {
            return SIGMA_MAX;
        }
        void sigma_max(int SIGMA_MAX_) {
            SIGMA_MAX= SIGMA_MAX_;
        }

        float beta() const {
            return SIGMA_MAX;
        }
        void beta(float BETA_) {
            BETA = BETA_;
        }

        vector<float> mu_vec() const {
            return MU_VEC;
        }
        void mu_vec(vector<float> MU_VEC_) {
            MU_VEC = MU_VEC_;
        }
        
        int index_cases() const {
            return INDEX_CASES;
        }
        void index_cases(int INDEX_CASES_) {
            INDEX_CASES= INDEX_CASES_;
        }
        
        int seed() const {
            return SEED;
        }
        void seed(int SEED_) {
            SEED= SEED_;
        }
        int pop_seed() const {
            return POP_SEED;
        }
        void pop_seed(int POP_SEED_) {
            POP_SEED= POP_SEED_;
        }

        int n_vax_daily() const {
            return N_VAX_DAILY;
        }
        void n_vax_daily(int N_VAX_DAILY_) {
            N_VAX_DAILY = N_VAX_DAILY_;
        }

        int ve1() const {
            return VE1;
        }
        void ve1(int VE1_) {
            VE1 = VE1_;
        }

        int ve2() const {
            return VE2;
        }
        void ve2(int VE2_) {
            VE2 = VE2_;
        }


};

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
        History() {};
        void add_history(int _S, int _E, int _I, int _R, int _D, int _V1, int _V2);
        void save(string path = "history.csv");
        void plot_trajectory(string path = "plot.png") ;
};



typedef tuple <int,string,string> key;

class Data {

    public:

        int BICS_nrow;
        unordered_map<string, int> BICS_colnames;
        vector<vector<string>> BICS_data;
        vector<float> BICS_weights;

        int POLYMOD_nrow;
        unordered_map<string, int> POLYMOD_colnames;
        vector<vector<string>> POLYMOD_data;

        map<key, vector<int>> eligible_nodes;
        map<key, vector<int>> eligible_POLYMOD;
        map<key, RandomVector> distributions_POLYMOD;
        map<key, RandomVector> hh_distn;


        void load_BICS(int wave=4, string path="df_all_waves.csv");
        void load_POLYMOD(string path = "./");
        void create_sampling_distns(); 

        // Constructor
        Data();



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
void gen_pop_from_survey_csv(Data *data, igraph_t *g, int n, int pop_seed);


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




void distribute_vax(igraph_t *g, int n_daily, int time_until_v2);

vector<float> stovf(string s);





/* Python interface */
extern "C" void hello_world();
extern "C" void BICS_ABM_py();

string recode_age(string age_s) ;
string recode_gender(string gender) ;

void BICS_ABM(Data *data, Params *params);
