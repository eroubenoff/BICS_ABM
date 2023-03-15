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

/*
 * Functions to disconnect a node from their household
 *
 * Takes the graph, dictionary of household nodes,
 * node id, and modifies graph in place
 * Also modifies node home_status to be 'out' instead of 'in'
 *
 */

void disconnect_hh(igraph_t* g,
        unordered_map<int, vector<int>>& hhid_lookup,
        int node_id) {

    int hhid = VAN(g, "hhid", node_id);
    vector<int> hh_members = hhid_lookup[hhid];

    SETVAN(g, "home_status", node_id, _Out);

    /* 
     * Traverse household members; if there are any connections,
     * sever them.
     */

    igraph_bool_t are_connected;
    igraph_es_t edge;

    for (auto i: hh_members) {

        igraph_are_connected(g, node_id, i, &are_connected);

        if (are_connected) {
            igraph_es_pairs_small(&edge, true, node_id, i, -1);
            igraph_delete_edges(g, edge);
            
        }
    }

    igraph_es_destroy(&edge);
}

void reconnect_hh(igraph_t* g, 
        unordered_map<int, vector<int>>& hhid_lookup,
        int node_id) {
    

    int hhid = VAN(g, "hhid", node_id);
    vector<int> hh_members = hhid_lookup[hhid];

    SETVAN(g, "home_status", node_id, _In);


    for (auto i: hh_members) {

        int status = VAN(g, "home_status", node_id);

        if (status == _In) {
            igraph_add_edge(g, node_id, i);
        }
    }
}


/*
 * Includes random contacts as outlined in section 3.4
 *
 * Does not modify graph g, but does modify random_edgelist in place. 
 */

void random_contacts(igraph_t *g, 
        igraph_vector_t *regular_contacts_el,
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

    igraph_vector_t stubs_count;
    igraph_vector_init(&stubs_count, vcount);

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

    int sum = igraph_vector_sum(&stubs_count);
    if (sum % 2 == 1) { 
        // Pick a random index
        int tries = 0;
        int randomIndex;
        while (tries < 1000) {
            randomIndex = rand() % igraph_vector_size(&stubs_count);

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
    igraph_vector_t elist_to_remove;
    igraph_vector_init(&elist_to_remove, 0);
    igraph_vector_t temp_elist;
    igraph_vector_init(&temp_elist, 0);
    
    for (int i = 0; i < igraph_vcount(g); i++) {
        if (VECTOR(stubs_count)[i] > 0) {
            igraph_incident(g, &temp_elist, i, IGRAPH_ALL);
            igraph_vector_append(&elist_to_remove, &temp_elist);
        }
    }

    igraph_delete_edges(g, igraph_ess_vector(&elist_to_remove));

    igraph_vector_destroy(&temp_elist);
    igraph_vector_destroy(&elist_to_remove);

    /* 
     * Create a new graph with this random draw and
     *
     * save the edge list as random_edgelist
     *
     * */
    igraph_vector_t random_edgelist;
    igraph_degree_sequence_game(&new_graph, &stubs_count, NULL, IGRAPH_DEGSEQ_SIMPLE); // IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE); //IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE ); // IGRAPH_DEGSEQ_CONFIGURATION);
    //  igraph_realize_degree_sequence(&new_graph, &stubs_count, NULL, IGRAPH_MULTI_SW, IGRAPH_REALIZE_DEGSEQ_SMALLEST);
    /* Simplify graph */
    igraph_simplify(&new_graph, true, true, NULL);
    igraph_vector_init(&random_edgelist, igraph_vcount(&new_graph));
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
    igraph_vector_destroy(&stubs_count);
    igraph_vector_destroy(&random_edgelist);
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
void gen_hh_edges(igraph_t *graph, igraph_vector_t *hhedges){
    unordered_map<int, vector<int> > hhids;
    for (int i = 0; i < igraph_vcount(graph); i++) {
        hhids[VAN(graph, "hhid", i)].push_back(i);
    }

    // Create a household edge list
    igraph_vector_init(hhedges, 0);
    for (auto h : hhids) {

        // Create all combinations within the hh by nested loop
        for (int k=0; k < h.second.size(); k++) {
            for (int l=k+1; l < h.second.size(); l++) {
                if (k == l) {
                    continue;
                }
                else{
                    igraph_vector_push_back(hhedges, h.second[k]);
                    igraph_vector_push_back(hhedges, h.second[l]);
                }
            }
        }
    }

    cout << "N households: " << hhids.size() << endl;
    cout << "N household edges " << igraph_vector_size(hhedges) / 2 << endl;
}



