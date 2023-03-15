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

void set_sick(igraph_t *g, int n, int rde, int rds, bool mu, int t_reinfection, int is_symptomatic) {
  SETVAN(g, "disease_status", n, _E);
  SETVAN(g, "remaining_days_exposed", n, rde);
  SETVAN(g, "remaining_days_sick", n, rds);
  SETVAN(g, "mu", n, mu);
  SETVAN(g, "time_until_susceptible", n, t_reinfection);
  SETVAN(g, "symptomatic", n, is_symptomatic);
}
