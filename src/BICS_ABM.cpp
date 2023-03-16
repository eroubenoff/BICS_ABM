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

    print_params(params);
    cout << "N vertices: " << igraph_vcount(graph) << endl;

    mt19937 generator(params->SEED);

    /* 
     * Generate household edges and add to g 
     * */
    igraph_vector_t hhedges;
    gen_hh_edges(graph, &hhedges);
    igraph_add_edges(graph, &hhedges, NULL);


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
    igraph_vector_t hhedges_type;
    igraph_vector_init(&hhedges_type, igraph_ecount(graph));
    for (int i = 0; i < igraph_vector_size(&hhedges_type);i++){
        VECTOR(hhedges_type)[i] = _Household;
    }
    SETEANV(graph, "type", &hhedges_type);
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
        igraph_delete_edges(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID));

        // Hours 8-16
        for (hr = 8; hr < 18; hr++){
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            

            /* 
             * Retrieve the corresponding vector of edges we should connect at each hour,
             * remove each end point from their home, then 
             * connect the edges.
             */

            vector<edgeinfo> hourly_contacts = daily_contacts[hr];

            /* Need to turn the vector of edgeinfo into vectors of endpoints */
            if (true) {
                igraph_vector_resize(&hourly_edges, 0);

                for (auto c: hourly_contacts) {
                    // Confirm that nodes 1 and 2 are valid
                    if ((c.node1 < 0) | (c.node1 > igraph_vcount(graph))){
                        throw runtime_error("Node 1 out of range");
                    }
                    if ((c.node2 < 0) | (c.node2 > igraph_vcount(graph))){
                        throw runtime_error("Node 1 out of range");
                    }
                    igraph_vector_push_back(&hourly_edges, c.node1);
                    igraph_vector_push_back(&hourly_edges, c.node2);
                }

                igraph_add_edges(graph, &hourly_edges, 0);
            }

            if (false) {
                for (auto c: hourly_contacts) {
                    cout << "Attempting connection between nodes " <<c.node1 << "  and " << c.node2 << endl;
                    cout << "Disease status of node " << c.node1 << ": " << VAN(graph, "disease_status", c.node1);
                    cout << " and  node " << c.node2 << " " << VAN(graph, "disease_status", c.node2) << endl;

                    // continue;

                    /* Check if edge already exists; if so, pass */
                    igraph_are_connected(graph, c.node1, c.node2, &connected);
                    cout << "Are nodes connected? " << endl;
                    
                    if (connected) {
                        cout << "Nodes are already connected" << endl;
                    } else {cout << "Nope" << endl;}

                    igraph_add_edge(graph, c.node1, c.node2);
                    cout << "Edge added" << endl;
                    igraph_get_eid(graph, &eid, c.node1, c.node2, false, false);
                    if (eid == -1) {
                        throw runtime_error("Incorrect edge number");
                    }
                    cout << "Edge id " << eid << "Added successfully" << endl;
                    
                    // Check disease status, since I think that's what's throwing us off here
                    SETEAN(graph, "duration", eid, c.duration);
                    SETEAN(graph, "type", eid, _Random);
                    cout << "with duration " << EAN(graph, "duration", eid) << endl;
                    //disconnect_hh(graph, hh_lookup, c.node1);           
                    //disconnect_hh(graph, hh_lookup, c.node2);           
                }
            }
            
            
            tie(Cc, Csc) = transmit(graph, BETA, params, generator);
            decrement(graph, history, Cc, Csc);
            
            // Deleting all
            igraph_delete_edges(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID));

        }


        // Hours 18-24
        //delete_all_edges(graph);
        //igraph_add_edges(graph, &hhedges, NULL);
        //SETEANV(graph, "type", &hhedges_type);
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


    // Destroy vector/
    igraph_vector_destroy(&hhedges);
    igraph_vector_destroy(&hhedges_type);
    igraph_vector_destroy(&hourly_edges);
    /*
    igraph_vector_int_destroy(&daytime_edges);
    igraph_strvector_destroy(&daytime_edges_type);
    */


}
