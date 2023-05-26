#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <igraph.h>
#include <random>
#include <map>
#include <tuple>
using namespace std;

/* 
 * Disease and vaccine status as macros. These are shorthands for numerical
 * representations. I.e, S = 1, E = 2. It is easier to read this way.
 *
 * This is done so that they can be internally
 * represented as ints, which is much faster than
 * strings with the igraph api. They are globals for consistency. 
 * */

#define _S 1
#define _E 2
#define _Ic 3
#define _Isc 4
#define _R 5
#define _D 6
#define _V0 0
#define _V1 1
#define _V2 2
#define _VW 3
#define _VBoost 4
#define _Random 2
#define _Household 1
#define _In 0
#define _Out 1
#define _dur_lt1m 1/60
#define _dur_lt15m 15/60
#define _dur_lt1hr 1.0
#define _dur_mt1hr 2.0


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
    float BETA_VEC[365];
    float MU_VEC[9];
    int INDEX_CASES;
    int IMPORT_CASES_VEC[365];
    int SEED;
    int N_VAX_DAILY;
    float VE1;
    float VE2;
    float VEW;
    float VEBOOST;
    float ISOLATION_MULTIPLIER;
    int T_REINFECTION;
    int T0;
    float ALPHA; 
    float RHO;
    float NPI;
    int MAX_DAYS;
    int BOOSTER_DAY;

} Params;


/*
 * Function to initialize parameter values. 
 * See documentation for values. 
 *
 * @param none
 * @return Params object with baseline values. 
 *
 */
// extern "C" Params init_params(); 
void print_params(const Params *params); 
extern "C" Params destroy_params(); 


class History {
    public: 
        vector<int> S;
        vector<int> E;
        vector<int> Ic;
        vector<int> Cc;
        vector<int> Isc;
        vector<int> Csc;
        vector<int> R;
        vector<int> D;
        vector<int> V1;
        vector<int> V2;
        vector<int> VW;
        vector<int> VBoost;
        vector<int> n_edges;
        int counter;
        int length;
        History(int initial_length = 2000);
        void add_history(int S_count, int E_count, int Ic_count, int Cc_count, int Isc_count, int Csc_count, int R_count, int D_count, int V1_count, int V2_count, int VW_count, int VBoost_count, int n_edges_count); 
        void save(string path = "history.csv");
        void plot_trajectory(string path = "plot.png") ;
};





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

tuple<int, int> transmit(igraph_t *g,
        float BETA,
        const Params *params,
        mt19937 &generator);


/*
   Sets a node to be sick, parameterized with all conditions needed for the 
   rest of the node's lifecycle.

   @param int n: index of node to set sick
   @param int rde: time the node will spend exposed
   @param int rds: time the node will spend sick
   @param bool mu: if, at the end of rds, the node will die
   @return none 
   */
void set_sick(igraph_t *g, int n, int rde, int rds, bool mu, int t_reinfection, int is_symptomatic);


/* 
   Decrements each node's progression through disease time
   */
void decrement(igraph_t *g, History *h, int Cc=0, int Csc=0, bool print = true);



void distribute_vax(igraph_t *g, int n_daily, int time_until_v2, int time_until_vw, /*int time_until_vboost, */ bool vboost) ;





void BICS_ABM(igraph_t *graph, Params *params, History *history);


void random_contacts(igraph_t *g, 
        igraph_vector_int_t *regular_contacts_el,
        igraph_vector_t *regular_contacts_type,
        float isolation_multiplier,
        mt19937 &generator) ;
class edgeinfo{
    public:
        int node1; 
        int node2; 
        edgeinfo(int n1, int n2){
            node1=n1;
            node2=n2;
        };
        edgeinfo(){};
};
/* Generates a single random graph of contacts,
 * accounting for isolation
 *
 * unordered_map<int start_time, tuple<int node1, int node2, int duration>>
 * Start time
 * duration
 * start node, end node
 * 
 */

void random_contacts_duration(const igraph_t *g,
        unordered_map<int, vector<edgeinfo>> &ret,
        float isolation_multiplier,
        mt19937 &generator) ;

void gen_hh_edges(igraph_t *graph, igraph_vector_int_t *hhedges);
/*
void gen_daytime_edges(const igraph_t *graph, 
        const igraph_vector_int_t *hh_edges, 
        igraph_vector_int_t *daytime_edges,
        igraph_strvector_t *daytime_edges_type
    );
*/

void create_graph_from_pop(igraph_t *g, double *pop, size_t pop_size, size_t n_cols)  ;


/* Functions to re/disconnect a node from 
 * their household members */
void reconnect_hh(igraph_t* g, 
        unordered_map<int, vector<int>> &hhid_lookup,
        igraph_vector_int_t* edges_to_add,
        int node_id) ;
void disconnect_hh(igraph_t* g,
        unordered_map<int, vector<int>> &hhid_lookup,
        igraph_vector_int_t* edges_to_delete,
        int node_id);

void set_duration(igraph_t* g,
        discrete_distribution<float> &dist,
        mt19937 &generator
        ) ;


void create_graph_from_pop(igraph_t *g, double *pop, size_t pop_size, size_t n_cols);

void set_edge_attribute(igraph_t *g, 
        igraph_vector_int_t *end_points,
        string attribute_name,
        igraph_real_t attribute_value,
        bool force = false) ;
