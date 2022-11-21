#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <igraph.h>
#include "BICS_ABM.h"
#include <random>

void distribute_vax(igraph_t *g, int n_daily, int time_until_v2) {
    int n_remaining_v1 = n_daily / 2;
    int n_remaining_v2 = n_daily / 2;
    default_random_engine gen;
    /* 
     * Get the list of highest priority people
     */

    unordered_map<int, vector<int>> priorities_v1;
    vector<int> priorities_v2;

    for (int i = igraph_vcount(g); i--; ) {
        priorities_v1[VAN(g, "vaccine_priority", i)].push_back(i);
        if ((VAN(g, "vaccine_status", i) == ::V0) & (VAN(g, "time_until_v2", i) == 0))
            priorities_v2.push_back(i);
    }

    /*
     * Get highest priority, then shuffle all lists
     */
    int max_prior = 0;
    for (auto i: priorities_v1) {
        max_prior = max(max_prior, i.first);
        shuffle(i.second.begin(), i.second.end(), gen);
    }
    shuffle(priorities_v2.begin(), priorities_v2.end(), gen);

    /*
     * Walk down the priority list, moving people to V1 if possible
     */

    int node_to_vax;
    while ((n_remaining_v1 > 0) & (max_prior >= 0)) {

        if (priorities_v1[max_prior].size() == 0) {
            max_prior--;
            continue;
        }

        node_to_vax = priorities_v1[max_prior][priorities_v1[max_prior].size() - 1];
        priorities_v1[max_prior].pop_back();
        
        SETVAN(g, "vaccine_priority", node_to_vax, -1);
        SETVAN(g, "vaccine_status", node_to_vax, ::V1);
        SETVAN(g, "time_until_v2", node_to_vax, time_until_v2);

        n_remaining_v1--;

    }

    /*
     * Move to V2 as possible
     */

    while ((n_remaining_v2 > 0) & (priorities_v2.size() > 0)) {
        
        node_to_vax = priorities_v2.size() - 1;
        priorities_v2.pop_back();

        SETVAN(g, "vaccine_status", node_to_vax, ::V2);
        SETVAN(g, "time_until_v2", node_to_vax, -1);

        n_remaining_v2--;
    }

    


}

