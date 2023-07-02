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
        igraph_vector_int_t *end_points,
        string attribute_name,
        igraph_real_t attribute_value,
        bool force) {

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
        igraph_vector_int_t eids;
        igraph_vector_int_init(&eids, 0);
        igraph_get_eids(g, &eids, end_points, false, false);

        for (int i = 0; i < igraph_vector_int_size(&eids); i++) {
            if (VECTOR(eids)[i] == -1) {
                cout << "Edge not present " << endl; continue;
            } else {
                SETEAN(g, attribute_name.c_str(), VECTOR(eids)[i], attribute_value);
            }
        }

        igraph_vector_int_destroy(&eids);

    }

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
    igraph_vector_int_t edges_to_delete;
    igraph_vector_int_init(&edges_to_delete, 0);

    // For connecting/disconnecting hhs
    igraph_integer_t degree; 
    vector<int> reconnect_hhs; 
    reconnect_hhs.reserve(igraph_vcount(graph));

    print_params(params);
    // print_attributes(graph);
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

    discrete_distribution<int> duration_dist{0.171, 0.452, 0.187, 0.189};


    /* 
     * Generate household edges and add to g,
     * including type and duration
     * */
    UpdateList hh_ul;
    unordered_map<int, vector<int>> hh_lookup;

    gen_hh_edges(graph, hh_ul, hh_lookup);

    hh_ul.add_updates_to_graph(graph);

    decrement(graph, history);


    /* 
     * Generate school contacts 
     * */ 
    igraph_t school_contacts;
    igraph_empty(&school_contacts, 0, IGRAPH_UNDIRECTED);
    igraph_vector_t num_cc_school_;
    igraph_vector_init(&num_cc_school_, igraph_vcount(graph));
    VANV(graph, "num_cc_school", &num_cc_school_);
    igraph_vector_int_t num_cc_school;
    igraph_vector_int_init(&num_cc_school, igraph_vcount(graph));
    for (int i = 0; i < igraph_vcount(graph); i++) {
        VECTOR(num_cc_school)[i] = (int)VECTOR(num_cc_school_)[i];
    }
    make_stubcount_sum_even(num_cc_school);
    igraph_degree_sequence_game(&school_contacts, &num_cc_school, NULL, IGRAPH_DEGSEQ_CONFIGURATION);
    igraph_simplify(&school_contacts, true, true, NULL);
    igraph_vector_destroy(&num_cc_school_);
    igraph_vector_int_destroy(&num_cc_school);

    UpdateList school_ul;



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
    UpdateList ul;
    for (int i = 0; i < params->INDEX_CASES; i++) {
        int index_case = distribution(generator);
        cout << index_case << "  " ;
        set_sick(ul, index_case, 3*24, 5*24, false, params->T_REINFECTION, _Ic);
    }
    ul.add_updates_to_graph(graph);
    ul.clear_updates();

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

    igraph_vector_int_t hourly_edges;
    igraph_vector_int_init(&hourly_edges, 0);
    /* 
     * Run main sim 
     * */

    int n_imported_cases;

    igraph_vector_t ds0_vec;
    igraph_vector_init(&ds0_vec, 0);
    igraph_vector_t ds1_vec;
    igraph_vector_init(&ds1_vec, 0);

    while (run) {
        Cc = 0;
        Csc = 0;
        n_imported_cases = 0;


        // Pull disease status vector
        VANV(graph, "disease_status", &ds0_vec);

        /* Reset all edges */
        // igraph_delete_edges(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID));
        // hh_ul.add_updates_to_graph(graph);

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

            n_imported_cases = params -> IMPORT_CASES_VEC[day%365];

            // Tally up all Susceptibles 
            vector<int> susceptibles; 
            for (int i = 0; i < igraph_vcount(graph); i++) {
                if (VAN(graph, "disease_status", i) == _S) {
                    susceptibles.push_back(i);
                }
            }

            if (susceptibles.size() > 0) {
                for (int i = 0; i < n_imported_cases; i++) {
                    uniform_int_distribution<int> sus_distr(0, susceptibles.size() - 1);
                    int import_case = susceptibles[sus_distr(generator)];
                    set_sick(ul, import_case, 3*24, 5*24, false, params->T_REINFECTION, _Ic);
                    ul.add_updates_to_graph(graph);
                    ul.clear_updates();
                }
            }
        }
        


        // Hours 0-8
        for (hr = 0; hr < 1; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            tie(Cc, Csc) = transmit(graph, BETA, params, generator);
            decrement(graph, history, Cc + n_imported_cases, Csc);

        }
        for (hr = 1; hr < 8; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            tie(Cc, Csc) = transmit(graph, BETA, params, generator);
            decrement(graph, history, Cc, Csc);

        }
        


        bool vboost = (day % 365) >= params->BOOSTER_DAY;
        distribute_vax(graph, params->N_VAX_DAILY, 25*24, params->T_REINFECTION, vboost);
        random_contacts_duration(graph, daily_contacts, params->ISOLATION_MULTIPLIER, params->CONTACT_MULT_VEC[day%365], generator);

        // Connect school contacts
        if (params->SCHOOL_CONTACTS && ((day % 365 < 152 ) || (day % 365 > 244)) && ((day % 7 < 2 ) || (day % 7 > 5) )) {
            gen_school_contacts(graph, &school_contacts, school_ul, hh_lookup);
            school_ul.add_updates_to_graph(graph);
        }

        // Hours 8-16
        for (hr = 8; hr < 18; hr++){
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            
            // igraph_delete_edges(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID));
            // igraph_add_edges(graph, &hhedges, NULL);
            // set_edge_attribute(graph, &hhedges, "type", _Household, false);

            /* 
             * Retrieve the corresponding vector of edges we should connect at each hour,
             * remove each end point from their home, then 
             * connect the edges.
             */

            ul.clear_updates();

            vector<edgeinfo> hourly_contacts = daily_contacts[hr];

            for (auto c: hourly_contacts) {
                // Confirm that nodes 1 and 2 are valid
                if ((c.node1 < 0) | (c.node1 > igraph_vcount(graph))){
                    throw runtime_error("Node 1: " + to_string(c.node1) + " out of range");
                }
                if ((c.node2 < 0) | (c.node2 > igraph_vcount(graph))){
                    throw runtime_error("Node 2: " + to_string(c.node2) + " out of range");
                }

                ul.add_update(CreateEdge(c.node1, c.node2));
                ul.add_update(UpdateEdgeAttribute(c.node1, c.node2, "type", _Random));
                ul.add_update(UpdateVertexAttribute(c.node1, "home_status", _Out));
                ul.add_update(UpdateVertexAttribute(c.node1, "home_status", _Out));

                // TODO: Fix duration
                switch(duration_dist(generator)){
                    case 0:
                        ul.add_update(UpdateEdgeAttribute(c.node1, c.node2, "duration", _dur_lt1m));
                        break;

                    case 1:
                        ul.add_update(UpdateEdgeAttribute(c.node1, c.node2, "duration", _dur_lt15m));
                        break;

                    case 2:
                        ul.add_update(UpdateEdgeAttribute(c.node1, c.node2, "duration", _dur_lt1hr));
                        break;

                    case 3:
                        ul.add_update(UpdateEdgeAttribute(c.node1, c.node2, "duration", _dur_mt1hr));
                        break;

                    default:
                        ul.add_update(UpdateEdgeAttribute(c.node1, c.node2, "duration", 0));
                        break;
                }

                // Disconnect node1 and 2 from hh edges /
                disconnect_hh(graph, ul, hh_lookup, c.node1);
                disconnect_hh(graph, ul, hh_lookup, c.node2);

            }


            ul.add_updates_to_graph(graph);

            /*
            for (int i = 0; i < igraph_vcount(graph); i++) cout << EAN(graph, "type",i) << "  ";
            cout << endl;
            */
            ul.clear_updates();

            /* Transmit and decrement */
            tie(Cc, Csc) = transmit(graph, BETA, params, generator);
            decrement(graph, history, Cc, Csc);

            /*
             * Reconnect nodes finished with their contacts
             * with their houseold members, provided those household
             * members do not have any additional nonhh-contacts.
             *
             * To do this, need to loop through each node and evalutate
             * if they still have any contacts; if they don't set home 
             * status to "In". Collect these such nodes in a vector to 
             * then reconnect to household. 
             */
            /* Reconnect everybody */
            reconnect_hhs.clear();
            for (int i = igraph_vcount(graph); i--; ) {
                if (VAN(graph, "home_status", i) == _Out) {
                    igraph_degree_1(graph, &degree, i, IGRAPH_ALL, false);
                    if (degree == 0) {
                        ul.add_update(UpdateVertexAttribute(i, "home_status", _In));
                        reconnect_hhs.push_back(i);
                    }
                }
            }

            ul.add_updates_to_graph(graph);
            ul.clear_updates();

            for (auto i: reconnect_hhs) {
                reconnect_hh(graph, ul, hh_lookup, i);
            }

            ul.add_updates_to_graph(graph);
            ul.clear_updates();




        }

        igraph_delete_edges(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID));
        // hh_ul.clear_updates();
        // gen_hh_edges(graph, hh_ul, hh_lookup);
        hh_ul.add_updates_to_graph(graph);

        // Hours 18-24
        for (hr = 19; hr < 24; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            tie(Cc, Csc) = transmit(graph, BETA, params, generator);
            decrement(graph, history, Cc, Csc);

        }

        day++;


        /* Compare ds vectors to disconnect anyone that died */
        VANV(graph, "disease_status", &ds1_vec);
        for (int i =0; i < igraph_vector_size(&ds0_vec); i++) {
            if ((VECTOR(ds1_vec)[i]  == _D )&& (VECTOR(ds0_vec)[i] != _D)) {
                remove_deceased_from_hhs(graph, i, hh_lookup, hh_ul);
            }
        }

        /* Handle Demography monthly*/
        if ((day % 30) == 0) {
            demography(graph, params, hh_lookup, hh_ul, generator);
        }

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
    igraph_vector_int_destroy(&hourly_edges);
    igraph_vector_int_destroy(&edges_to_delete);
    igraph_vector_destroy(&ds0_vec);
    igraph_vector_destroy(&ds1_vec);
    igraph_destroy(&school_contacts);

}
