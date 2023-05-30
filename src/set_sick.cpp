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
using namespace std;

void set_sick(UpdateList &ul, int n, int rde, int rds, bool mu, int t_reinfection, int is_symptomatic) {
    ul.add_update(UpdateVertexAttribute(n, "disease_status", _E));
    ul.add_update(UpdateVertexAttribute(n, "remaining_days_exposed", rde));
    ul.add_update(UpdateVertexAttribute(n, "remaining_days_sick", rds));
    ul.add_update(UpdateVertexAttribute(n, "mu", mu));
    ul.add_update(UpdateVertexAttribute(n, "time_until_susceptible", t_reinfection));
    ul.add_update(UpdateVertexAttribute(n, "symptomatic", is_symptomatic));
}
