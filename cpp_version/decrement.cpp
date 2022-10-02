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


void decrement(igraph_t *g) {
	int rds, rde;
	string ds;
    for (int i = 0; i < igraph_vcount(g); i++) {

      ds = VAS(g, "disease_status", i);

      if (ds == "E") {
         rde = VAN(g, "remaining_days_exposed", i);
         if (rde == 0) {
          SETVAS(g, "disease_status", i, "I");
         } else {
           SETVAN(g, "remaining_days_exposed", i, rde - 1);
         }
         continue;
      }

      else if (ds == "I") {
         rds = VAN(g, "remaining_days_sick", i);
         if (rds == 0) {
          SETVAS(g, "disease_status", i, "R");
         } else {
           SETVAN(g, "remaining_days_sick", i, rds - 1);
         }
         continue;
      }

      else {
      	continue;
      }

  }
}