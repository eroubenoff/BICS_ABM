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
#include "BICS_ABM.h"
#include <random>
#include <set>
#include <math.h>

/*
 * Functions to disconnect a node from their household
 *
 * Takes the graph, dictionary of household nodes,
 * node id, and modifies graph in place
 * Also modifies node home_status to be 'out' instead of 'in'
 *
 * Parameters
 * ----------
 * igraph_t* g 
 *  - pointer to igraph object
 * unordered_map<int, vector<ing>> &hhid_lookup
 *  - lookup object of households. Contains a vector of all 
 *    nodes in each household, hashed on the household id
 * igraph_vector_int_t* edges_to_delete
 *  - pointer to vector that contains the edges to delete,
 *    stored as adjacent edge end points
 * int node_id
 *  - node id that we are disconnecting
 *
 *
 * Returns
 * -------
 *  - None
 *
 * Modifies 
 * --------
 *  - adds pairs of vectors to edges_to_delete
 */

void disconnect_hh(igraph_t *g, UpdateList &ul,
        unordered_map<int, vector<int>> &hhid_lookup,
        int node_id) {

    int hhid = VAN(g, "hhid", node_id);

    /* 
     * Traverse household members; if there are any connections,
     * sever them.
     */

    vector<int> *hh = &hhid_lookup[hhid];
    int hhsize = hh->size();
    int node_id2;
    igraph_bool_t are_connected;
    for (int i = 0; i < hhsize; i++) {

        node_id2 = hh->at(i);
        igraph_are_connected(g, node_id, node_id2, &are_connected);

        if (are_connected) {
            // igraph_vector_int_push_back(edges_to_delete, eid);
            ul.add_update(DeleteEdge(node_id, node_id2));
        }
    }
}






/* 
 * Reconnects a node with their household -- in effect,
 * does the opposite of disconnect_hh
 *
 * Parameters
 * ---------
 *
 * igraph_t* g 
 *  - pointer to igraph object
 * unordered_map<int, vector<ing>> &hhid_lookup
 *  - lookup object of households. Contains a vector of all 
 *    nodes in each household, hashed on the household id
 * igraph_vector_t* edges_to_add 
 *  - pointer to vector that contains the household edges to add,
 *    stored as adjacent edge end points
 * int node_id
 *  - node id that we are re-connecting 
 *
 * Returns
 * -------
 *  - None
 *
 * Modifies 
 * --------
 *  - adds pairs of vectors to edges_to_delete
 *
 *
 */
void reconnect_hh(igraph_t* g, UpdateList &ul,
        unordered_map<int, vector<int>> &hhid_lookup,
        int node_id) {
    
    int hhid = VAN(g, "hhid", node_id);
    vector<int> *hh = &hhid_lookup[hhid];
    int hhsize = hh->size();

    /* 
     * Check if node has any other random edges;
     * if it does, then return without doing anything
     * */
    igraph_es_t es;
    igraph_es_incident(&es, node_id, IGRAPH_ALL);
    igraph_vector_t type;
    igraph_vector_init(&type, 0);
    igraph_cattribute_EANV(g, "type", es, &type);
    igraph_bool_t are_connected;


    for (int i = 0; i < igraph_vector_size(&type); i++) {
        if (VECTOR(type)[i] == _Random) {
            igraph_vector_destroy(&type);
            igraph_es_destroy(&es);
            return;
        }
    }


    int node_id2;
    for (int i = 0; i < hhsize; i++){

        node_id2 = hh->at(i);

        igraph_are_connected(g, node_id, node_id2, &are_connected);

        if ((!are_connected) & (VAN(g, "home_status", node_id2) == _In)){
            ul.add_update(CreateEdge(node_id, node_id2));
            ul.add_update(UpdateEdgeAttribute(node_id, node_id2, "type", _Household));
            ul.add_update(UpdateEdgeAttribute(node_id, node_id2, "duration", -1));
        }
    }

    igraph_vector_destroy(&type);
    igraph_es_destroy(&es);
}


/* 
 * Sets the duration of contacts as a random draw
 * from a set of options. This is to be called after 
 * random_contacts_duration. All edges are initialized
 * with a duration of 0. 
 *
 * This function works by scanning the non-household edges,
 * and assigning a duration to any edges with a duration of 
 * 0. Durations can be fractional hours, which is used to 
 * scale the probability used in transmit(), or multiple
 * hours. The duration of each edge is decremented by 1 (hour)
 * in the decrement() function. 
 * 
 *
 * Parameters
 * ----------
 * igraph_t* g
 *  - graph to modify
 * discrete_distribution<float> &dist
 *  - distribution to sample from of durations
 * mt19937 &generator
 *  - random state
 *
 * Returns 
 * ------
 *  None
 *
 * Modifies
 * --------
 *  graph object in place by added durations to edges.
 *
 */

void set_duration(igraph_t* g,
        discrete_distribution<float> &dist,
        mt19937 &generator
        ) {

    throw runtime_error("Function set_duration is obsolete!");
    /* 
     * Get edge type and duration
     * */

    igraph_vector_t type;
    igraph_vector_init(&type, 0);
    EANV(g, "type", &type);

    igraph_vector_t duration;
    igraph_vector_init(&duration, 0);
    EANV(g, "duration", &duration);

    /*
     * Loop over edges, doing random draws and modifying
     * duration vector as necessary
     */
    int n_edges = igraph_vector_size(&type);
    int draw; 

    for (int i = 0; i < n_edges; i++) {

        /* Random choices are 0-3; recode to
            0: _dur_lt1m 1/60
            1: _dur_lt15m 15/60
            2: _dur_lt1hr 1.0
            3: _dur_mt1hr 2.0
        */ 

        // cout << "Type: " << VECTOR(type)[i]  << "  duration: " << VECTOR(duration)[i] << endl;

        if ((VECTOR(type)[i] == _Random) & isnan(VECTOR(duration)[i]) ) {
            draw = dist(generator);
            
            switch(draw){
                case 0:
                    VECTOR(duration)[i] = _dur_lt1m;
                    break;

                case 1:
                    VECTOR(duration)[i] = _dur_lt15m;
                    break;

                case 2:
                    VECTOR(duration)[i] = _dur_lt1hr;
                    break;

                case 3:
                    VECTOR(duration)[i] = _dur_mt1hr;
                    break;

                default:
                    VECTOR(duration)[i] = 0;
                    break;
            }

        } 

    }

    /* Assign dist back to graph object */
    SETEANV(g, "duration", &duration);

    /* Garbage collect */
    igraph_vector_destroy(&duration);
    igraph_vector_destroy(&type);
}






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
        mt19937 &generator) {

    /* Create return object */
    ret.clear();

    /* Pull the information from the graph */
    igraph_vector_t num_cc_nonhh;
    igraph_vector_init(&num_cc_nonhh, igraph_vcount(g));
    VANV(g, "num_cc_nonhh", &num_cc_nonhh);
    igraph_vector_t ds_vec;
    igraph_vector_init(&ds_vec, igraph_vcount(g));
    VANV(g, "disease_status", &ds_vec);

    /* Create a vector of stubs count */
    igraph_vector_int_t stubs_count;
    igraph_vector_int_init(&stubs_count, igraph_vcount(g));
    /* See if node is in isolation */
    for (int i = igraph_vcount(g); i--; ) {
        if (VECTOR(ds_vec)[i] == _Ic){ 
            // Trick for rounding becuase float -> int cast truncates
            VECTOR(stubs_count)[i] = (int) round(VECTOR(num_cc_nonhh)[i] * isolation_multiplier * contact_multiplier);
        }  else {
            VECTOR(stubs_count)[i] = (int) round(VECTOR(num_cc_nonhh)[i] * contact_multiplier);
        }
    }

    /* 
     * Make sure sum is an even number; otherwise decrease a random stub until it is 
     *
     * */
    int sum = igraph_vector_int_sum(&stubs_count);
    if (sum % 2 == 1) { 
        // Pick a random index
        int tries = 0;
        int randomIndex;
        while (tries < 1000) {
            randomIndex = rand() % igraph_vector_int_size(&stubs_count);

            if (VECTOR(stubs_count)[randomIndex] > 1) {
                VECTOR(stubs_count)[randomIndex] -= 1;
                break;
            }
            tries++;
        }
    }

    /* Draw random graph */
    igraph_t new_graph;
    // igraph_empty(&new_graph, 0, IGRAPH_UNDIRECTED);
    igraph_degree_sequence_game(&new_graph, &stubs_count, NULL, IGRAPH_DEGSEQ_CONFIGURATION); 
    // IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE); //IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE ); // IGRAPH_DEGSEQ_CONFIGURATION);
    /* Simplify graph */
    // igraph_simplify(&new_graph, true, true, NULL);

    /* Time of day */
    uniform_int_distribution<int> ToD(8,18);
    for (int i = 8; i <= 18; i++) {
        ret[i] = vector<edgeinfo>();
    }

    /* Loop over each edge and extract the relevant information */
    edgeinfo ei;
    igraph_es_t es;
    igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
    igraph_eit_t eit;
    igraph_eit_create(g, es, &eit);
    int current_edge;
    while(!IGRAPH_EIT_END(eit)) {
        current_edge = IGRAPH_EIT_GET(eit);

        int time = ToD(generator);

        /* Extract FROM and TO */
        ei = edgeinfo(
            IGRAPH_FROM(&new_graph, current_edge),
            IGRAPH_TO(&new_graph, current_edge)
        );

        /* Append to return object */
        ret[time].push_back(ei);

        IGRAPH_EIT_NEXT(eit);
    }
    igraph_es_destroy(&es);
    igraph_eit_destroy(&eit);
    igraph_vector_int_destroy(&stubs_count);
    igraph_vector_destroy(&num_cc_nonhh);
    igraph_vector_destroy(&ds_vec);
    igraph_destroy(&new_graph);

}


/*
 * Includes random contacts as outlined in section 3.4
 *
 * Does not modify graph g, but does modify random_edgelist in place. 
 */

void random_contacts(igraph_t *g, 
        igraph_vector_int_t *regular_contacts_el,
        igraph_vector_t *regular_contacts_type,
        float isolation_multiplier,
        mt19937 &generator) {


    /* New graph to work on temporary edges */
    igraph_t new_graph;
    igraph_empty(&new_graph, 0, IGRAPH_UNDIRECTED);

    /* Pull attributes from the original graph */
    int vcount = igraph_vcount(g);

    /* 
     * First remove the previous step's random contacts
     * and re-connect them with their household contacts 
     */ 

    igraph_delete_edges(g, igraph_ess_all(IGRAPH_EDGEORDER_ID));
    igraph_add_edges(g, regular_contacts_el, NULL);

    /* 
     * Add type 
     * */
    SETEANV(g, "type", regular_contacts_type);

    /* 
     * ZIP parameters 
     * Random draw of stubs for each vertex 
     *
     * First get attributes
     * */ 
    bernoulli_distribution ber; int p_ber;
    poisson_distribution pois; int p_pois;

    igraph_vector_int_t stubs_count;
    igraph_vector_int_init(&stubs_count, vcount);

    igraph_vector_t ds_vec;
    igraph_vector_init(&ds_vec, vcount);
    VANV(g, "disease_status", &ds_vec);

    igraph_vector_t lh_vec;
    igraph_vector_init(&lh_vec, vcount);
    VANV(g, "lefthome_num", &lh_vec);

    igraph_vector_t cc_vec;
    igraph_vector_init(&cc_vec, vcount);
    VANV(g, "num_cc_nonhh", &cc_vec);


    bool isolation = false;
    int n_stubs;
    float temp;
    for (int i = vcount; i--; ) {
        /* See if node is in isolation */
        if (VECTOR(ds_vec)[i] == _Ic){ 
            isolation = true;
        } else {
            isolation = false;
        }

        /* Draw probability of an excursion */ 
        if (VECTOR(lh_vec)[i] > 0){ 
            ber = bernoulli_distribution(VECTOR(lh_vec)[i]); 
            p_ber = ber(generator);

            /* Draw lambda */
            pois = poisson_distribution(VECTOR(cc_vec)[i]/VECTOR(lh_vec)[i]); 
            p_pois = pois(generator);

            /* Draw stubs */
            n_stubs = p_ber * p_pois;

            if (isolation) {
                // cout << " Original stub count " << n_stubs;
                // cout << " Isolation mult  " << isolation_multiplier; 
                temp = (((float) n_stubs) * isolation_multiplier) ;
                // cout << " temp " << temp;
                n_stubs = temp + 0.5; // Trick for rounding becuase float -> int cast truncates
                // cout << " Isolation stub count " << n_stubs << endl;
            }

            VECTOR(stubs_count)[i] = n_stubs;
        }
        else {
            VECTOR(stubs_count)[i] = 0;
        }

    }

    igraph_vector_destroy(&ds_vec);
    igraph_vector_destroy(&lh_vec);
    igraph_vector_destroy(&cc_vec);

    /* 
     * Make sure sum is an even number; otherwise decrease a random stub until it is 
     *
     * */

    int sum = igraph_vector_int_sum(&stubs_count);
    if (sum % 2 == 1) { 
        // Pick a random index
        int tries = 0;
        int randomIndex;
        while (tries < 1000) {
            randomIndex = rand() % igraph_vector_int_size(&stubs_count);

            if (VECTOR(stubs_count)[randomIndex] > 1) {
                VECTOR(stubs_count)[randomIndex] -= 1;
                break;
            }
            tries++;
        }
    }
    
    /* 
     * For any vertex with stubs_count>0, remove any existing edges 
     * */

    // igraph_es_t temp_es; // Temporary edge selector
    igraph_vector_int_t elist_to_remove;
    igraph_vector_int_init(&elist_to_remove, 0);
    igraph_vector_int_t temp_elist;
    igraph_vector_int_init(&temp_elist, 0);
    
    for (int i = 0; i < igraph_vcount(g); i++) {
        if (VECTOR(stubs_count)[i] > 0) {
            igraph_incident(g, &temp_elist, i, IGRAPH_ALL);
            igraph_vector_int_append(&elist_to_remove, &temp_elist);
        }
    }

    igraph_delete_edges(g, igraph_ess_vector(&elist_to_remove));

    igraph_vector_int_destroy(&temp_elist);
    igraph_vector_int_destroy(&elist_to_remove);

    /* 
     * Create a new graph with this random draw and
     *
     * save the edge list as random_edgelist
     *
     * */
    igraph_vector_int_t random_edgelist;
    igraph_degree_sequence_game(&new_graph, &stubs_count, NULL, IGRAPH_DEGSEQ_CONFIGURATION); // IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE); //IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE ); // IGRAPH_DEGSEQ_CONFIGURATION);
    //  igraph_realize_degree_sequence(&new_graph, &stubs_count, NULL, IGRAPH_MULTI_SW, IGRAPH_REALIZE_DEGSEQ_SMALLEST);
    /* Simplify graph */
    igraph_simplify(&new_graph, true, true, NULL);
    igraph_vector_int_init(&random_edgelist, igraph_vcount(&new_graph));
    igraph_get_edgelist(&new_graph, &random_edgelist, false);

    /* 
     * Set edge types 
     * First get the type of the reduced edges 
     * */
    igraph_vector_t edges_type;
    igraph_vector_init(&edges_type, igraph_ecount(g));
    // EANV(g, "type", &edges_type);


    /* 
     * Set any additional contacts to type "random" 
     * */

    int n_regular = igraph_vector_size(&edges_type);
    igraph_vector_resize(&edges_type, n_regular + igraph_ecount(&new_graph));
    for (int i = 0; i < n_regular; i++) {
        VECTOR(edges_type)[i] = _Household;
    }
    for (int i = n_regular; i < igraph_vector_size(&edges_type); i++) {
        VECTOR(edges_type)[i] = _Random;
    }

    /*
     * Add random contacts to the main graph
     */
    igraph_add_edges(g, &random_edgelist, NULL);

    /* 
     * Set edge types to graph 
     * */
    
    SETEANV(g, "type", &edges_type);



    /* 
     * Call destructors 
     * */
    //DELALL(&new_graph);
    igraph_destroy(&new_graph);
    igraph_vector_int_destroy(&stubs_count);
    igraph_vector_int_destroy(&random_edgelist);
    igraph_vector_destroy(&edges_type);

}


/* 
 * Generate household edges
 * First create an unordered_map of hhids 
 *
 * hhids is a hash on the household id and each
 * value is an igraph vector of the vertices in that 
 * household
 * */
void gen_hh_edges(igraph_t *graph, UpdateList &hh_ul, unordered_map<int, vector<int>> &hh_lookup){

    for (int i = 0; i < igraph_vcount(graph); i++) {
        hh_lookup[VAN(graph, "hhid", i)].push_back(i);
    }

    // Create a household edge list
    for (auto h : hh_lookup) {

        // Create all combinations within the hh by nested loop
        for (int k=0; k < h.second.size(); k++) {
            for (int l=k+1; l < h.second.size(); l++) {
                if (k == l) {
                    continue;
                }
                else{
                    hh_ul.add_update(CreateEdge(h.second[k], h.second[l]));
                    hh_ul.add_update(UpdateEdgeAttribute(h.second[k], h.second[l], "type", _Household));
                    hh_ul.add_update(UpdateEdgeAttribute(h.second[k], h.second[l], "duration", 0));
                }
            }
        }
    }

}



