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

    print_params(params);
    cout << "N vertices: " << igraph_vcount(graph) << endl;

    const bool cached = false;
    mt19937 generator(params->SEED);



    /* 
     * Generate household edges and add to g 
     * */
    igraph_vector_t hhedges;
    gen_hh_edges(graph, &hhedges);
    igraph_add_edges(graph, &hhedges, NULL);

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

    /* Generate daytime edges that include work contacts*/ 
    /*
    igraph_vector_int_t daytime_edges;
    igraph_strvector_t daytime_edges_type;
    gen_daytime_edges(graph, &hhedges, &daytime_edges, &daytime_edges_type);
    */



    int day; 
    int hr;
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

    // cout << "IMPORT CASES BOOL " << imported_cases_daily_bool << endl;


    /* 
     * Run main sim 
     * */
    float BETA;
    int Cc, Csc;
    bool run = true;
    while (run) {
        Cc = 0;
        Csc = 0;



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
            // cout  << "Imported case"; 

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
                    //cout << import_case << "  " ;
                    set_sick(graph, import_case, 3*24, 5*24, false, params->T_REINFECTION, _Ic);
                }

                // cout << endl;
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

        // Hours 8-16
        for (hr = 8; hr < 16; hr++){
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";

            /* 
             * Generate random contacts on graph, and save them in random_contacts_es.
             * random_contacts_es is passed back to random_contacts so that
             * it can be used to efficiently remove the previous time step's 
             * random contacts, without having to re-wire everything using delete_all edges or 
             * similar.
             */

            random_contacts(graph, &hhedges, &hhedges_type, params -> ISOLATION_MULTIPLIER, generator);
            tie(Cc, Csc) = transmit(graph, BETA, params, generator);
            decrement(graph, history, Cc, Csc);

        }


        // Hours 16-24
        delete_all_edges(graph);
        igraph_add_edges(graph, &hhedges, NULL);
        SETEANV(graph, "type", &hhedges_type);
        for (hr = 16; hr < 24; hr++ ) {
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

    }

    cout << endl;


    // Destroy vector/
    igraph_vector_destroy(&hhedges);
    igraph_vector_destroy(&hhedges_type);

    /*
    igraph_vector_int_destroy(&daytime_edges);
    igraph_strvector_destroy(&daytime_edges_type);
    */


}
