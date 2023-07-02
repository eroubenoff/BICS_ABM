#include<igraph.h>
#include<vector>
#include<string>
#include<iostream>
#include <unordered_map>
#include "BICS_ABM.h"
#include <random>

bool random_draw(double p, mt19937 &seed) {

    std::bernoulli_distribution d(p);
    return d(seed);
}
void remove_deceased_from_hhs(igraph_t *g, 
        int vid, 
        unordered_map<int, vector<int>> &hh_lookup,
        UpdateList &hh_ul){

    int hhid = VAN(g, "hhid", vid);

    // Iterate over the vector to remove the node form hh_lookup
    int i;
    bool found = false;
    for (i = 0; i < hh_lookup[hhid].size(); i++) {
        if (hh_lookup[hhid][i] == vid) {
            hh_lookup[hhid].erase(hh_lookup[hhid].begin() + i);
            found = true;
            break;
        }
    }

    if (!found) {
        // cout << "In demography::remove_deceased_from_hhs: could not find node "+ to_string(vid) << endl;
        throw runtime_error("In demography::remove_deceased_from_hhs: could not find node " +  to_string(vid));
    } 

    // Now need to remove from the updatelist 
    // Need to do both the create and the attributes

    vector<int> remove_edge_updates;
    for (i = 0; i < hh_ul._create_edge_v.size(); i++) {
        if ((hh_ul._create_edge_v[i].get_v1() == vid) || (hh_ul._create_edge_v[i].get_v2() == vid)) {
            remove_edge_updates.push_back(i);
        }
    }

    for (i = remove_edge_updates.size(); i--; ) {
        hh_ul._create_edge_v.erase(hh_ul._create_edge_v.begin() + i);
    }

    remove_edge_updates.clear();
    for (i = 0; i < hh_ul._update_edge_attribute_v.size(); i++) {
        if ((hh_ul._update_edge_attribute_v[i].get_v1() == vid) || (hh_ul._update_edge_attribute_v[i].get_v2() == vid)) {
            remove_edge_updates.push_back(i);
        }
    }

    for (i = remove_edge_updates.size(); i--; ) {
        hh_ul._update_edge_attribute_v.erase(hh_ul._update_edge_attribute_v.begin() + i);
    }

}
/*
 * Function to handle births and deaths
 *
 */
void demography(igraph_t *g, Params *params, unordered_map<int, vector<int>> &hh_lookup, 
        UpdateList &hh_ul, mt19937 &seed) {

    igraph_vector_t age;
    igraph_vector_init(&age, 0);
    VANV(g, "age", &age);

    igraph_vector_t gender;
    igraph_vector_init(&gender, 0);
    VANV(g, "gender", &gender);

    igraph_vector_t ds;
    igraph_vector_init(&ds, 0);
    VANV(g, "disease_status", &ds);

    igraph_vector_t num_cc;
    igraph_vector_init(&num_cc, 0);
    VANV(g, "num_cc_nonhh", &num_cc);

    igraph_vector_t num_cc_school;
    igraph_vector_init(&num_cc_school, 0);
    VANV(g, "num_cc_school", &num_cc_school);

    float mean_school=0;
    float mean_cc=0;
    int num_0=0;
    for (int i = 0; i < igraph_vcount(g); i++) {
        if (VECTOR(age)[i] == 0) {
            mean_cc += VECTOR(num_cc)[i];
            mean_school += VECTOR(num_cc_school)[i];
            num_0++;
        }
    }

    mean_cc = round(mean_cc / num_0);
    mean_school = round(mean_school / num_0);


    // First handle deaths
    int v = igraph_vcount(g);
    int a;
    for (int i = 0; i < v; i++) {
        a = VECTOR(age)[i];
        if (VECTOR(ds)[i] != _D) {
            if (random_draw(params->MORTALITY_V[a]/12, seed) ) {
                //cout << "There was a death!!" << endl;
                // Set to be dead
                VECTOR(ds)[i] = _D;

                remove_deceased_from_hhs(g, i, hh_lookup, hh_ul);
            }
        }
    }

    SETVANV(g, "disease_status", &ds);
    // Then handle births
    for (int i = 0; i < v; i++) {
        a = VECTOR(age)[i];
        if (VECTOR(ds)[i] != _D & VECTOR(gender)[i] != 0) {
            if (random_draw(params->FERTILITY_V[a]/12, seed) ) {

                // Create a new node in parent's household
                igraph_add_vertices(g, 1, NULL);

                // Get idx of last vertex
                int new_v = igraph_vcount(g) -1;
                SETVAN(g, "hhid", new_v, VAN(g, "hhid", i)); 
                SETVAN(g, "age", new_v, 0);
                SETVAN(g, "gender", new_v, random_draw(0.6, seed));
                SETVAN(g, "num_cc_nonhh", new_v, mean_cc);
                SETVAN(g, "num_cc_school", new_v, mean_school);
                SETVAN(g, "lefthome_num", new_v, 1);
                SETVAN(g, "vaccine_priority", new_v, 0);
                SETVAN(g, "NPI", new_v, 0);
                SETVAN(g, "mortality", new_v, 0);
                SETVAN(g, "fertility", new_v, 0);
                SETVAN(g, "disease_status", new_v, _S);
                SETVAN(g, "remaining_days_exposed", new_v, -1);
                SETVAN(g, "remaining_days_sick", new_v, -1);
                SETVAN(g, "time_until_v2", new_v, -1);
                SETVAN(g, "time_until_vw", new_v, -1);
                SETVAN(g, "time_until_boost", new_v, -1);
                SETVAN(g, "time_until_susceptible", new_v, -1);
                SETVAN(g, "symptomatic", new_v, -1);
                SETVAN(g, "vaccine_status", new_v, 0);
                SETVAN(g, "mu", new_v, 0);

                // Add to household lookup
                for (auto i: hh_lookup[VAN(g, "hhid", i)]) {
                    hh_ul.add_update(CreateEdge(i, new_v));
                }
                hh_lookup[VAN(g, "hhid", i)].push_back(new_v);




                //cout << "There was a birth! The number of nodes is now:" << igraph_vcount(g)<< endl;
                

            }
        }
    }



    igraph_vector_destroy(&age);
    igraph_vector_destroy(&gender);
    igraph_vector_destroy(&ds);
    igraph_vector_destroy(&num_cc);
    igraph_vector_destroy(&num_cc_school);


}
