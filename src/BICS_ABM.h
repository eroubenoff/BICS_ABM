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
#define _Household 1
#define _Random 2
#define _School 3
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
    float CONTACT_MULT_VEC[365];
    bool SCHOOL_CONTACTS;
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
// extern "C" Params destroy_params(); 


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
        void add_history(int S_count, int E_count, int Ic_count, int Cc_count, int Isc_count, int Csc_count, 
                int R_count, int D_count, int V1_count, int V2_count, int VW_count, int VBoost_count, int n_edges_count); 
        void save(string path = "history.csv");
        void plot_trajectory(string path = "plot.png") ;
};



/* Update function declarations */
// Base classes
class Update {};

class VertexUpdate: virtual public Update {
    protected:
        int _vid;
    public: 
        VertexUpdate(){};
        VertexUpdate(int vid);
        int get_vid(); 
};

class EdgeUpdate: virtual public Update {
    protected:
        int _v1;
        int _v2;
        int _eid;
        bool _eid_or_vpair;
        bool _break_on_failure;
    public:
        EdgeUpdate(){};
        EdgeUpdate(int v1, int v2, bool break_on_failure = false);
        EdgeUpdate(int eid, bool break_on_failure = true);
        int get_v1(); 
        int get_v2(); 
        int get_eid(); 
        bool eid_or_vpair();
        bool break_on_failure();
        void retrieve_eid(igraph_t* g);
        void retrieve_endpoints(igraph_t* g);
};

class AttributeUpdate: public Update {
    protected:
        string _attr;
        double _value;
    public: 
        AttributeUpdate(){};
        AttributeUpdate(string attr, double );
        string get_attr();
        double get_value();
};


// Derived classes

/* 
 * Class for graph update 
 * 
 * */ 

class UpdateGraphAttribute: public AttributeUpdate {
    public:
        UpdateGraphAttribute() {};
        UpdateGraphAttribute(string attr, double value): AttributeUpdate(attr, value){};
};

/*
 * Creating an edge: needs both end points, cannot include attributes.
 * To add attributes, call CreateEdge followed by UpdateEdgeAttributes.
 * Similar process for DelteEdge.
 *
 */ 

class CreateEdge: public EdgeUpdate {
    public: 
        CreateEdge() {};
        CreateEdge(int v1, int v2): EdgeUpdate(v1, v2){};
};

/* 
 * Deleting an edge: can take either the end points
 * or the eid. Default behavior is to fail silently
 * deleteing an edge by end points that don't connect, but to raise an
 * exception if passed a bad eid.
 * 
 */

class DeleteEdge: public EdgeUpdate {
    public: 
        DeleteEdge() {};
        DeleteEdge(int v1, int v2, bool break_on_failure = false): EdgeUpdate(v1, v2, break_on_failure){};
        DeleteEdge(int eid, bool break_on_failure = true): EdgeUpdate(eid, break_on_failure){};
};

/* 
 * Update an edge attribute: be defined for either a pair of
 * end points, or an edge id.
 * 
 * Single attributes only
 */
class UpdateEdgeAttribute: public EdgeUpdate, public AttributeUpdate {
    public: 
        UpdateEdgeAttribute() {};
        UpdateEdgeAttribute(int eid, string attr, double value): EdgeUpdate(eid), AttributeUpdate(attr, value){};
        UpdateEdgeAttribute(int v1, int v2, string attr, double value): EdgeUpdate(v1, v2), AttributeUpdate(attr, value){};
};


/* 
 * Update a vertex attribute:
 * 
 * Single attribute update only
 */
class UpdateVertexAttribute: public VertexUpdate, public AttributeUpdate {
    public: 
        UpdateVertexAttribute() {};
        UpdateVertexAttribute(int vid, string attr, double value): VertexUpdate(vid), AttributeUpdate(attr, value){};
};

/*
 * List of updates
 *
 * Needs to be grown one at a time using the
 * overloaded add_update() function
 *
 */
class UpdateList {
    protected: 
        vector<UpdateGraphAttribute> _update_graph_attribute_v;
        vector<CreateEdge> _create_edge_v;
        vector<DeleteEdge> _delete_edge_v;
        vector<UpdateEdgeAttribute> _update_edge_attribute_v;
        vector<UpdateVertexAttribute> _update_vertex_attribute_v;

    public:
        UpdateList();
        void add_update(UpdateGraphAttribute update) ;
        void add_update(CreateEdge update) ;
        void add_update(DeleteEdge update) ;
        void add_update(UpdateEdgeAttribute update) ;
        void add_update(UpdateVertexAttribute update) ;
        void clear_updates() ;
        void add_updates_to_graph(igraph_t *g) ;
        string print_updates();
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
void set_sick(UpdateList &ul, int n, int rde, int rds, bool mu, int t_reinfection, int is_symptomatic);


/* 
   Decrements each node's progression through disease time
   */
void decrement(igraph_t *g, History *h, int Cc=0, int Csc=0, bool print = true);
void decrement2(igraph_t *g, History *h, int Cc=0, int Csc=0, bool print = true);



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
        string print(){return "Edge between: " + to_string(node1) + " and " + to_string(node2); }
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
        float contact_multiplier,
        mt19937 &generator) ;

void gen_hh_edges(igraph_t *graph, UpdateList &hh_ul, unordered_map<int, vector<int>> &hh_lookup);
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
void reconnect_hh(igraph_t* g, UpdateList &ul,
        unordered_map<int, vector<int>> &hhid_lookup,
        int node_id) ;
void disconnect_hh(igraph_t* g, UpdateList &ul,
        unordered_map<int, vector<int>> &hhid_lookup,
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



void gen_school_contacts(
        igraph_t* graph, 
        igraph_t* school_contacts, 
        UpdateList &school_ul, 
        unordered_map<int, vector<int>>&hh_lookup);

void make_stubcount_sum_even(igraph_vector_int_t& stubs_count); 

void demography(igraph_t *g, unordered_map<int, vector<int>> &hh_lookup, 
        UpdateList &hh_ul, mt19937 &seed); 
