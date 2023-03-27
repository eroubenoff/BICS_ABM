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
#include <iomanip>

void delete_all_edges(igraph_t *g) {
    igraph_es_t es;
    igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
    igraph_delete_edges(g, es);
    igraph_es_destroy(&es);

}

/*
 * Helper fn to set edge attributes given
 * a vector of end points
 */
void set_edge_attribute(igraph_t *g, 
        igraph_vector_t *end_points,
        string attribute_name,
        igraph_real_t attribute_value,
        bool force = false) {

    // Pull edge attribute vector


    if (force){
        for (int i = 0; i < igraph_ecount(g); i++) {
            SETEAN(g, attribute_name.c_str(), i, attribute_value);
        }
        /*
        igraph_vector_t attr;
        igraph_vector_init(&attr, igraph_ecount(g));
        igraph_vector_fill(&attr, attribute_value); 
        SETEANV(g, attribute_name.c_str(), &attr);
        igraph_vector_destroy(&attr);
        */
    } 
    else {
        igraph_vector_t eids;
        igraph_vector_init(&eids, 0);
        igraph_get_eids(g, &eids, end_points, NULL, false, false);

        for (int i = 0; i < igraph_vector_size(&eids); i++) {
            if (VECTOR(eids)[i] == -1) {
                cout << "Edge not present " << endl; continue;
            } else {
                SETEAN(g, attribute_name.c_str(), VECTOR(eids)[i], attribute_value);
            }
        }

        igraph_vector_destroy(&eids);

    }

}

void figure_out_bug(igraph_t *g) {
        igraph_vector_t temp_v;
        igraph_vector_t edges_to_delete;
        igraph_vector_init_seq(&temp_v, 0, igraph_ecount(g)-1);
        SETEANV(g, "test", &temp_v);


        /* Delete some random edges */
        igraph_vector_init(&edges_to_delete, 10);
        for (int i = 0; i < 10; i++) {
            VECTOR(edges_to_delete)[i] = i * 10;
        }

        // igraph_delete_edges(g, igraph_ess_vector(&edges_to_delete));
        igraph_delete_edges(g, igraph_ess_1(6));
        EANV(g, "test", &temp_v);

        cout << "Number of edges in graph: " << igraph_ecount(g) << "; Number of items in attribute vector: " << igraph_vector_size(&temp_v) << endl;

        for (int i = 0; i < igraph_ecount(g); i++) {
            cout << "Edge no. " << i << " with graph value " << EAN(g, "test", i) <<  " and vector value " << VECTOR(temp_v)[i] << endl;
        }

        igraph_vector_destroy(&temp_v);
        igraph_vector_destroy(&edges_to_delete);
}

void BICS_ABM(igraph_t *graph, Params *params, History *history) {

    int day; 
    int hr;
    igraph_bool_t connected;
    igraph_integer_t eid; 
    const bool cached = false;
    int hhid;
    float BETA;
    int Cc, Csc;
    bool run = true;
    igraph_vector_t edges_to_delete;
    igraph_vector_init(&edges_to_delete, 0);

    print_params(params);
    cout << "N vertices: " << igraph_vcount(graph) << endl;

    mt19937 generator(params->SEED);

    /* 
     * Generate distribution of contact duration 
     *
     * Values here come from the weighted proportion
     * of contact duration derived from wave 6: 
     *   p_lt15 p_lt1hr p_lt1min p_mt1hr
           <dbl>   <dbl>    <dbl>   <dbl>
           0.452   0.187    0.171   0.189
     * */

    /* Arrange them in order from shortest to longest time
        _dur_lt1m 1/60
        _dur_lt15m 15/60
        _dur_lt1hr 1.0
        _dur_mt1hr 2.0
    */

    discrete_distribution<float> duration_dist{0.171, 0.452, 0.187, 0.189};


    /* 
     * Generate household edges and add to g 
     * */
    igraph_vector_t hhedges;
    gen_hh_edges(graph, &hhedges);

    /* Get all hhs into a dict */
    unordered_map<int, vector<int>> hh_lookup;
    for (int i = igraph_vcount(graph); i--; ) {
        hhid = VAN(graph, "hhid", i);
        if (hh_lookup.count(i) == 0) {
            hh_lookup[hhid] = vector<int>{i};
        } else {
            hh_lookup[hhid].push_back(i);
        }
    }



    /* 
     * Vector containing the type of edges (all are household) 
     * Set type attribute 
     * */

    igraph_add_edges(graph, &hhedges, 0);


    /* TRYING TO FIGURE OUT THIS STUPID BUG */
    cout << "Test 0" << endl;
    figure_out_bug(graph);

    set_edge_attribute(graph, &hhedges, "type", _Household, true);
    set_edge_attribute(graph, &hhedges, "duration", 0, true);

    decrement(graph, history);

    /* 
     * Distribute vaccines before T0 
     * */
    for (day = 0; day < params->T0; day++){
        for (hr = 0; hr < 8; hr++) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            decrement(graph, history);
        }
        distribute_vax(graph, params->N_VAX_DAILY, 25*24, params->T_REINFECTION, false);
        for (hr = 8; hr < 24; hr++) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            decrement(graph, history);
        }
    }
    cout << endl;

    /* 
     * Randomly choose index cases 
     * */
    uniform_int_distribution<int> distribution(0,igraph_vcount(graph) - 1);
    cout << "\r";
    cout << "Index cases: " ;
    for (int i = 0; i < params->INDEX_CASES; i++) {
        int index_case = distribution(generator);
        cout << index_case << "  " ;
        set_sick(graph, index_case, 3*24, 5*24, false, params->T_REINFECTION, _Ic);
    }

    cout << endl;

    decrement(graph, history);

    
    /* 
     * Set flags indicating if there is a time series of 
     * index cases or not 
     */

    bool imported_cases_daily_bool;

    for (int i = 0; i < 365; i++) {
        if (params->IMPORT_CASES_VEC [i] != 0) {
            imported_cases_daily_bool = true;
            break;
        }
    }

    unordered_map<int, vector<edgeinfo>> daily_contacts;

    igraph_vector_t hourly_edges;
    igraph_vector_init(&hourly_edges, 0);
    /* 
     * Run main sim 
     * */

    while (run) {
        Cc = 0;
        Csc = 0;

        /* Reset all edges */
        igraph_delete_edges(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID));
        igraph_add_edges(graph, &hhedges, NULL);

        /* 
         * If there is a daily seasonal forcing of Beta, 
         * update it now 
         */
        BETA = params->BETA_VEC[day % 365];

        /*
         * If there are daily imported cases,
         * pick a random susceptible to get sick
         */
        if (imported_cases_daily_bool) {

            // Tally up all Susceptibles 
            vector<int> susceptibles; 
            for (int i = 0; i < igraph_vcount(graph); i++) {
                if (VAN(graph, "disease_status", i) == _S) {
                    susceptibles.push_back(i);
                }
            }

            if (susceptibles.size() > 0) {
                for (int i = 0; i < params->IMPORT_CASES_VEC[day % 365]; i++) {
                    uniform_int_distribution<int> sus_distr(0, susceptibles.size() - 1);
                    int import_case = susceptibles[sus_distr(generator)];
                    set_sick(graph, import_case, 3*24, 5*24, false, params->T_REINFECTION, _Ic);
                }
            }
        }

        // Hours 0-8
        for (hr = 0; hr < 8; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            tie(Cc, Csc) = transmit(graph, BETA, params, generator);
            decrement(graph, history, Cc, Csc);

        }
        


        bool vboost = (day % 365) >= params->BOOSTER_DAY;
        distribute_vax(graph, params->N_VAX_DAILY, 25*24, params->T_REINFECTION, vboost);
        daily_contacts = random_contacts_duration(graph, params->ISOLATION_MULTIPLIER, generator);

        // Hours 8-16
        for (hr = 8; hr < 18; hr++){
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            
            igraph_delete_edges(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID));
            igraph_add_edges(graph, &hhedges, NULL);
            set_edge_attribute(graph, &hhedges, "type", _Household, false);

            /* 
             * Retrieve the corresponding vector of edges we should connect at each hour,
             * remove each end point from their home, then 
             * connect the edges.
             */

            vector<edgeinfo> hourly_contacts = daily_contacts[hr];

            /* Need to turn the vector of edgeinfo into vectors of endpoints */
            igraph_vector_resize(&edges_to_delete, 0);
            igraph_vector_resize(&hourly_edges, 0);

            for (auto c: hourly_contacts) {
                // Confirm that nodes 1 and 2 are valid
                if ((c.node1 < 0) | (c.node1 > igraph_vcount(graph))){
                    throw runtime_error("Node 1 out of range");
                }
                if ((c.node2 < 0) | (c.node2 > igraph_vcount(graph))){
                    throw runtime_error("Node 2 out of range");
                }

                igraph_vector_push_back(&hourly_edges, c.node1);
                igraph_vector_push_back(&hourly_edges, c.node2);

                // Disconnect node1 and 2 from hh edges /
                disconnect_hh(graph, hh_lookup, &edges_to_delete, c.node1);
                disconnect_hh(graph, hh_lookup, &edges_to_delete, c.node2);
            }


            /* Delete HH Edges */
            igraph_delete_edges(graph, igraph_ess_vector(&edges_to_delete));

            /* Add new hourly random edges */
            igraph_add_edges(graph, &hourly_edges, 0);

            /* Add edge type and duration */
            set_edge_attribute(graph, &hourly_edges, "type", _Random, false);

            // set_duration(graph, duration_dist, generator);

            /* Transmit and decrement */
            tie(Cc, Csc) = transmit(graph, BETA, params, generator);
            decrement(graph, history, Cc, Csc);
        }

        igraph_delete_edges(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID));
        igraph_add_edges(graph, &hhedges, 0);
        set_edge_attribute(graph, &hhedges, "type", _Household, false);

        // Hours 18-24
        for (hr = 19; hr < 24; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            tie(Cc, Csc) = transmit(graph, BETA, params, generator);
            decrement(graph, history, Cc, Csc);

        }

        day++;

        /* If max days is given, break on that day;
         * if no max days is given (-1), then break
         * when infection count drops to 0
         */
        if (params->MAX_DAYS != -1) {
            if (day > params->MAX_DAYS) {
                run = false;
            }
        }

        // If no max days is set, break when infection count drops 
        // to 0 to avoid an infinite loop 
        else if (params->MAX_DAYS == -1) {
            if ((GAN(graph, "Ic_count") + GAN(graph, "E_count") + GAN(graph, "Isc_count")) == 0 ) {
                run = false;
            }
        }

        if (day > 10000) {
            throw runtime_error("Day limit reached! Check code.");
        }

    }

    cout << endl;


    /* Garbage collect */
    igraph_vector_destroy(&hhedges);
    igraph_vector_destroy(&hourly_edges);
    igraph_vector_destroy(&edges_to_delete);

}
