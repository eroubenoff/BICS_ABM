#include "BICS_ABM.h"
#include <stdio.h>


struct Trajectory {
    int S_array[5000];
    int E_array[5000];
    int Ic_array[5000];
    int Isc_array[5000];
    int R_array[5000];
    int D_array[5000];
    int V1_array[5000];
    int V2_array[5000];
    int VW_array[5000];
    int VBoost_array[5000];
    int n_edges_array[5000];
    int counter;

};


/* 
 * https://asiffer.github.io/posts/numpy/
 */
void create_graph_from_pop(igraph_t *g, double *pop, size_t pop_size, size_t n_cols)  {

    igraph_add_vertices(g, pop_size, 0);

    igraph_vector_t col;
    vector<string> colnames = {"hhid", "age", "gender", "num_cc_nonhh", "lefthome_num", "vaccine_priority", "NPI"};

    for (int c = 0; c < n_cols; c++){
        igraph_vector_init_array(&col, &pop[c * pop_size], pop_size); // Might have to check the pointer arithmetic

        SETVANV(g, colnames[c].c_str(), &col);

    }

    // Fixed characteristics
    igraph_vector_fill(&col, ::S);
    SETVANV(g, "disease_status", &col);

    igraph_vector_fill(&col, -1);
    SETVANV(g, "remaining_days_exposed", &col);
    SETVANV(g, "remaining_days_sick", &col);
    // SETVANV(g, "vaccine_priority", &col);
    SETVANV(g, "time_until_v2", &col);
    SETVANV(g, "time_until_vw", &col);
    SETVANV(g, "time_until_vboost", &col);
    SETVANV(g, "time_until_susceptible", &col);

    igraph_vector_fill(&col, 0);
    SETVANV(g, "vaccine_status", &col);
    SETVANV(g, "mu", &col);

    igraph_vector_destroy(&col);
}



/* Treat data as a gloabl */
// https://stackoverflow.com/questions/30184998/how-to-disable-cout-output-in-the-runtime
extern "C" /*struct Trajectory*/ void BICS_ABM(double *pop, size_t pop_size, size_t n_cols, Trajectory *trajectory, Params params, bool silent = false) {

    // get underlying buffer
    streambuf* orig_buf = cout.rdbuf();

    if (silent) {
        // set null
        cout.rdbuf(NULL);
    }

    // Trajectory trajectory; 
    History history;

    /* 
     * Create the empty graph 
     * Turn on attribute handling. 
     * Create a directed graph with no vertices or edges. 
     * */

    igraph_t graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);


    /* 
     * Generate population
     * */
    // gen_pop_from_survey_csv(::database[params.WAVE], &graph, &params);
    create_graph_from_pop(&graph, pop, pop_size, n_cols);

    /*
     * Run sim
     */

    BICS_ABM(&graph, &params, &history);

    /* 
     * Delete all remaining attributes. 
     * */
    DELALL(&graph);

    /* 
     * Destroy the graph. 
     * */
    igraph_destroy(&graph);


    trajectory->counter = history.counter;
    copy(history.S.begin(), history.S.end(), trajectory->S_array);
    copy(history.E.begin(), history.E.end(), trajectory->E_array);
    copy(history.Ic.begin(), history.Ic.end(), trajectory->Ic_array);
    copy(history.Isc.begin(), history.Isc.end(), trajectory->Isc_array);
    copy(history.R.begin(), history.R.end(), trajectory->R_array);
    copy(history.D.begin(), history.D.end(), trajectory->D_array);
    copy(history.V1.begin(), history.V1.end(), trajectory->V1_array);
    copy(history.V2.begin(), history.V2.end(), trajectory->V2_array);
    copy(history.VW.begin(), history.VW.end(), trajectory->VW_array);
    copy(history.VBoost.begin(), history.VBoost.end(), trajectory->VBoost_array);
    copy(history.n_edges.begin(), history.n_edges.end(), trajectory->n_edges_array);
 
    if (silent) {
        // restore buffer
        cout.rdbuf(orig_buf); 
    }

    // return trajectory;

}
