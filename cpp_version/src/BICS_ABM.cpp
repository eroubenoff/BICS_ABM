
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
 * Disease and vaccine status as globals 
 *
 * This is done so that they can be internally
 * represented as ints, which is much faster than
 * strings with the igraph api. They are globals for consistency. 
 * */

int S = 1;
int E = 2;
int I = 3;
int R = 4;
int D = 5;
int V0 = 0;
int V1 = 1;
int V2 = 2;


/* Params default values */ 
extern "C" Params init_params() {
    Params params{
        1000, // N_HH
        4,    // WAVE
        2*24, 4*24, // GAMMA_MIN, MAX
        3*24, 7*24, // SIGMA_MIN, MAX
        0.1, // BETA
        {0.00001, 0.0001, 0.0001, 0.001, 0.001, 0.001, 0.01, 0.1, 0.1}, // MU
        5, // INDEX_CASES
        49, 4949, // SEED, POP_SEED
        100, // N_VAX_DAILY
        0.75, 0.95, // VE1, 2
        0.90,  // isolation_multiplier

        "age;age;age;age;hesitancy", // VAX_RULES_COLS
        ">85;[75,85);[65,75);[55,65);0.5", // VAX_RULES_VALS
        {1,1,1,2},  // VAX_CONDS_N
        4 // VAX_RULES_N

    }; 

    return params;
};

void delete_all_edges(igraph_t *g) {
    igraph_es_t es;
    igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
    igraph_delete_edges(g, es);
    igraph_es_destroy(&es);

}


/*
 * Includes random contacts as outlined in section 3.4
 *
 * Does not modify graph g, but does modify random_edgelist in place. 
 */

void random_contacts(igraph_t *g, 
        igraph_vector_int_t *regular_contacts_el,
        igraph_strvector_t *regular_contacts_type,
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
    igraph_cattribute_EAS_setv(g, "type", regular_contacts_type);

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
    for (int i = vcount; i--; ) {
        /* See if node is in isolation */
        if (VECTOR(ds_vec)[i] == ::I){ 
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
            if (!isolation) {
                VECTOR(stubs_count)[i] = p_ber * p_pois;
            } else {
                VECTOR(stubs_count)[i] = round(isolation_multiplier * p_ber * p_pois);
            }
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
     * Create a new graph with this random draw and
     *
     * save the edge list as random_edgelist
     *
     * */
    igraph_vector_int_t random_edgelist;
    igraph_degree_sequence_game(&new_graph, &stubs_count, NULL, IGRAPH_DEGSEQ_CONFIGURATION ); // IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE); //IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE ); // IGRAPH_DEGSEQ_CONFIGURATION);
    igraph_vector_int_init(&random_edgelist, igraph_vcount(&new_graph));
    igraph_get_edgelist(&new_graph, &random_edgelist, false);



    /* 
     * Remove any regular edges in g from vertices found in random_edgelist
     * Do this by creating a regular int vector with the 
     * edge-ids to remove, then passing that to 
     * igraph_delete_edges
     */
    
    igraph_es_t temp_es; // Temporary edge selector
    igraph_eit_t temp_eit; // Temporary edge iterator
    int random_edgelist_size = igraph_vector_int_size(&random_edgelist); // Number of nodes having random contacts
    vector<igraph_integer_t> edges_to_remove(random_edgelist_size); // Regular vector of edges to remove

    for (int i = 0; i < random_edgelist_size; i++) {

        // Get edges for each vertex in random_edgelist
        igraph_es_incident(&temp_es, VECTOR(random_edgelist)[i], IGRAPH_ALL);
        igraph_eit_create(g, temp_es, &temp_eit);

        // Iterate over them, adding s
        int e = 0;
        while(!IGRAPH_EIT_END(temp_eit)) {
            // cout << "Edge" << IGRAPH_EIT_GET(temp_eit) << " connectes vertices  " << IGRAPH_FROM(g, IGRAPH_EIT_GET(temp_eit)) << " and " << IGRAPH_TO(g, IGRAPH_EIT_GET(temp_eit)) << endl;

            edges_to_remove[e] = IGRAPH_EIT_GET(temp_eit);

            IGRAPH_EIT_NEXT(temp_eit);
            e++;

        }
        
    }

    igraph_es_destroy(&temp_es);
    igraph_eit_destroy(&temp_eit);

    // There may be duplicate edges here if both endpoints
    // have a random contact
    sort(edges_to_remove.begin(), edges_to_remove.end());
    int current_val;
    for (int i = 0; i < edges_to_remove.size(); i++){
        current_val = edges_to_remove[i];
        if (current_val == edges_to_remove[i+1]){
            // cout << "Removing duplicate value at location " << i << endl;
            edges_to_remove.erase(edges_to_remove.begin() + i);
        }
    }

    igraph_vector_int_t edges_to_remove_2;
    igraph_vector_int_init_array(&edges_to_remove_2, &edges_to_remove.data()[0], edges_to_remove.size());

    igraph_es_t es_to_remove;
    igraph_es_vector(&es_to_remove, &edges_to_remove_2);

    igraph_delete_edges(g, es_to_remove);

    igraph_vector_int_destroy(&edges_to_remove_2);
    igraph_es_destroy(&es_to_remove);

    /* Get the type of the reduced edges */
    igraph_strvector_t edges_type;
    igraph_strvector_init(&edges_type, igraph_ecount(g));
    EASV(g, "type", &edges_type);

    /*
     * Add random contacts to the main graph
     */
    igraph_add_edges(g, &random_edgelist, NULL);

    /* Set any additional contacts to type "random" */
    int n_regular = igraph_strvector_size(&edges_type);
    igraph_strvector_resize(&edges_type, n_regular + igraph_vector_int_size(&random_edgelist)/2);
    for (int i = n_regular; i < igraph_strvector_size(&edges_type); i++) {
        igraph_strvector_set(&edges_type, i, "random");
    }

    /* Set edge types to graph */
    igraph_cattribute_EAS_setv(g, "type", &edges_type);

    /* 
     * Call destructors 
     * */
    DELALL(&new_graph);
    igraph_destroy(&new_graph);
    igraph_vector_int_destroy(&stubs_count);
    igraph_vector_int_destroy(&random_edgelist);
    igraph_strvector_destroy(&edges_type);

}


/* 
 * Generate household edges
 * First create an unordered_map of hhids 
 *
 * hhids is a hash on the household id and each
 * value is an igraph vector of the vertices in that 
 * household
 * */
void gen_hh_edges(igraph_t *graph, igraph_vector_int_t *hhedges){
    unordered_map<string, vector<int> > hhids;
    for (int i = 0; i < igraph_vcount(graph); i++) {
        hhids[VAS(graph, "hhid", i)].push_back(i);
    }

    // Create a household edge list
    igraph_vector_int_init(hhedges, 0);
    for (auto h : hhids) {

        // Create all combinations within the hh by nested loop
        for (int k=0; k < h.second.size(); k++) {
            for (int l=k+1; l < h.second.size(); l++) {
                if (k == l) {
                    continue;
                }
                else{
                    igraph_vector_int_push_back(hhedges, h.second[k]);
                    igraph_vector_int_push_back(hhedges, h.second[l]);
                }
            }
        }
    }
}


/*
 * Generate a random graph of work edges from deg dist 
 *
 */
void gen_daytime_edges(const igraph_t *graph, 
        const igraph_vector_int_t *hh_edges, 
        igraph_vector_int_t *daytime_edges,
        igraph_strvector_t *daytime_edges_type
    ){

    /* 
     * Get work contacts deg dist 
     *
     * */
    igraph_vector_int_t degdist;
    igraph_vector_int_t work_edges;
    igraph_vector_int_init(&degdist, igraph_vcount(graph));

    string tmp;
    for (int i = igraph_vcount(graph); i--; ){
        tmp = VAS(graph, "num_cc_work", i);

        if (tmp == "NA") VECTOR(degdist)[i] = 0;
        else {
            try {
                VECTOR(degdist)[i] = stoi(tmp);
            }
            catch(...) {
                cout << "Error! " << endl;
            }
        }
    }

    /* 
     * Generate random network and turn into edgelist
     */
    igraph_t new_graph;
    igraph_empty(&new_graph, 0, IGRAPH_UNDIRECTED);
    igraph_degree_sequence_game(&new_graph, &degdist, NULL, IGRAPH_DEGSEQ_CONFIGURATION ); // IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE); //IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE ); // IGRAPH_DEGSEQ_CONFIGURATION);
    igraph_vector_int_init(&work_edges, igraph_vcount(&new_graph));
    igraph_get_edgelist(&new_graph, &work_edges, false);


    /* 
     * work_edges now contains the random draw of work contacts
     *
     * Need to create daytime_edges, which consists of
     * work contacts + hh contacts(for any vertex without
     * work contacts)
     *
     */

    /* First duplicate hh_edges into daytime_edges */
    igraph_vector_int_init_copy(daytime_edges, hh_edges);

    /* Then get unique values in work_edges as a set*/ 
    set<int> work_vertices;
    for (int i = 0; i < igraph_vector_int_size(&work_edges); i++){
        work_vertices.insert(VECTOR(work_edges)[i]);
    }

    /* Traverse daytime_edges in pairs and remove */
    for (int i = 0; i < igraph_vector_int_size(daytime_edges); i+= 2) {
        if ((work_vertices.count(igraph_vector_int_get(daytime_edges, i)) > 0 ) || 
            (work_vertices.count(igraph_vector_int_get(daytime_edges, i+1)) > 0 ))
        {
            igraph_vector_int_remove_section(daytime_edges, i, i+2);
        }
    }

    /* Fill the daytime type vector, 1 per edge (every 2) */
    int n_daytime_edges = igraph_vector_int_size(daytime_edges);
    int n_work_edges = igraph_vector_int_size(&work_edges);
    igraph_strvector_init(daytime_edges_type, 0);
    for (int i = 0; i < n_daytime_edges/2; i++) igraph_strvector_push_back(daytime_edges_type, "household");
    for (int i = 0; i < n_work_edges/2; i++) igraph_strvector_push_back(daytime_edges_type,  "work");


    /* Concat work edges to daytime edges */
    igraph_vector_int_append(daytime_edges, &work_edges);


    /* Destructors */
    igraph_vector_int_destroy(&degdist);
    igraph_vector_int_destroy(&work_edges);
    igraph_destroy(&new_graph);

}

void print_params(const Params *params) {
    cout << "----------------------------------------"<< endl;
    cout << "Parameters passed to simulation:" << endl;

    cout << "N_HH:            " << params->N_HH << endl;
    cout << "GAMMA_MIN, _MAX: " << params->GAMMA_MIN << ", " << params->GAMMA_MAX << endl;
    cout << "SIGMA_MIN, _MAX: " << params->SIGMA_MIN << ", " << params->SIGMA_MAX << endl;
    cout << "BETA:            " << params->BETA<< endl;
    cout << "MU_VEC:          ";
    for (auto i: params->MU_VEC) {
        cout << i << "  ";
    }   
    cout << endl;
    cout << "INDEX_CASES:     " << params->INDEX_CASES << endl;
    cout << "SEED:            " << params->SEED<< endl;
    cout << "POP_SEED:        " << params->POP_SEED<< endl;
    cout << "N_VAX_DAILY:     " << params->N_VAX_DAILY<< endl;
    cout << "VE1, VE2:        " << params->VE1 << ", " << params->VE2 << endl;
    cout << "ISOLATION_MULT.: " << params->ISOLATION_MULTIPLIER<< endl;
    cout << "VAX_RULES_COLS:  " << params->VAX_RULES_COLS << endl;
    cout << "VAX_RULES_VALS:  " << params->VAX_RULES_VALS << endl;
    cout << "VAX_CONDS_N:     " << params->VAX_CONDS_N << endl;
    cout << "VAX_RULES_N:     " << params->VAX_RULES_N << endl;
    cout << "----------------------------------------"<< endl;

}


void BICS_ABM(const Data *data, const Params *params, History *history) {

    print_params(params);

    const bool cached = false;
    mt19937 generator(params->SEED);

    // History object
    //  history(2000);
    /* 
     * Pre-generate all distributions as cycling vectors
     *
     * */ 

    CyclingVector<int> gamma_vec(1000, [&generator, &params](){return (uniform_int_distribution<int>(params->GAMMA_MIN, params->GAMMA_MAX))(generator);});
    CyclingVector<int> sigma_vec(1000, [&generator, &params](){return (uniform_int_distribution<int>(params->SIGMA_MIN, params->SIGMA_MAX))(generator);});
    // CyclingVector<int> beta_vec(1000, [&generator, BETA](){return (bernoulli_distribution(BETA))(generator);});

    // Transmission probability 
    unordered_map<int, CyclingVector<int>*> beta;
    beta[0] = new CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA)(generator));});
    beta[1] = new CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA*(1-params->VE1))(generator));});
    beta[2] = new CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params->BETA*(1-params->VE2))(generator));});


    // Create mortality
    unordered_map<string, CyclingVector<int> >mu;
    mu["[0,18)"]  = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[0]))(generator);});
    mu["[18,25)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[1]))(generator);});
    mu["[25,35)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[2]))(generator);});
    mu["[35,45)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[3]))(generator);});
    mu["[45,55)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[4]))(generator);});
    mu["[55,65)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[5]))(generator);});
    mu["[65,75)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[6]))(generator);});
    mu["[75,85)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[7]))(generator);});
    mu[">85"]     = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params->MU_VEC[8]))(generator);});
    

    /* Create the empty graph */
    igraph_t graph;

    /* Turn on attribute handling. */
    igraph_set_attribute_table(&igraph_cattribute_table);

    /* Create a directed graph with no vertices or edges. */
    igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);

    /* Generate population*/
    gen_pop_from_survey_csv(data, &graph, params);

    cout << "N vertices: " << igraph_vcount(&graph) << endl;


    /* Generate household edges and add to g */
    igraph_vector_int_t hhedges;
    gen_hh_edges(&graph, &hhedges);
    igraph_add_edges(&graph, &hhedges, NULL);

    /* 
     * Vector containing the type of edges (all are household) 
     * Set type attribute 
     * */
    igraph_strvector_t hhedges_type;
    igraph_strvector_init(&hhedges_type, igraph_vector_int_size(&hhedges)/2);
    for (int i = 0; i < igraph_vector_int_size(&hhedges)/2; i+=2 ){igraph_strvector_set(&hhedges_type, i, "household");}
    igraph_cattribute_EAS_setv(&graph, "type", &hhedges_type);

    /* Generate daytime edges that include work contacts*/ 
    /*
    igraph_vector_int_t daytime_edges;
    igraph_strvector_t daytime_edges_type;
    gen_daytime_edges(&graph, &hhedges, &daytime_edges, &daytime_edges_type);
    */


    // Pick nodes at random to be infected
    uniform_int_distribution<int> distribution(0,igraph_vcount(&graph) - 1);
    cout << "Index cases: ";
    for (int i = 0; i < params->INDEX_CASES; i++) {
        int index_case = distribution(generator);
        cout << index_case << "  " ;
        set_sick(&graph, index_case, 3*24, 5*24, false);
    }

    cout << endl;


    int isolation_multiplier = params->ISOLATION_MULTIPLIER;


    int day = 0;
    int hr = 0;
    decrement(&graph, history);


    
    igraph_integer_t n_edges;
    while (GAN(&graph, "I_count") + GAN(&graph, "E_count") > 0){

        // Hours 0-8
        for (hr = 0; hr < 8; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            transmit(&graph, beta, gamma_vec, sigma_vec, mu);
            decrement(&graph, history);

        }


        distribute_vax(&graph, params->N_VAX_DAILY, 25*24);

        // Hours 8-16
        for (hr = 8; hr < 16; hr++){
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";

            /* 
             * Generate random contacts on &graph, and save them in random_contacts_es.
             * random_contacts_es is passed back to random_contacts so that
             * it can be used to efficiently remove the previous time step's 
             * random contacts, without having to re-wire everything using delete_all edges or 
             * similar.
             */

            // random_contacts(&graph, &daytime_edges, &daytime_edges_type, generator);
            random_contacts(&graph, &hhedges, &hhedges_type, isolation_multiplier, generator);
            transmit(&graph, beta, gamma_vec, sigma_vec, mu);
            decrement(&graph, history);

        }


        // Hours 16-24
        delete_all_edges(&graph);
        igraph_add_edges(&graph, &hhedges, NULL);
        for (hr = 16; hr < 24; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            transmit(&graph, beta, gamma_vec, sigma_vec, mu);
            decrement(&graph, history);

        }

        day++;


    }

    // history.save();
    // history.plot_trajectory();

    /* Delete all remaining attributes. */
    DELALL(&graph);

    /* Destroy the graph. */
    igraph_destroy(&graph);

    // Destroy vector/
    igraph_vector_int_destroy(&hhedges);
    igraph_strvector_destroy(&hhedges_type);
    /*
    igraph_vector_int_destroy(&daytime_edges);
    igraph_strvector_destroy(&daytime_edges_type);
    */

    delete beta[0];
    delete beta[1];
    delete beta[2];

}
