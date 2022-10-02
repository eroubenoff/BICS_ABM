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
#include "defs.h"
#include <random>
using namespace std;

void set_sick(igraph_t *g, int n, int rde, int rds, bool mu) {
  SETVAS(g, "disease_status", n, "E");
  SETVAN(g, "remaining_days_exposed", n, rde);
  SETVAN(g, "remaining_days_sick", n, rds);
  SETVAN(g, "mu", n, mu);
}
