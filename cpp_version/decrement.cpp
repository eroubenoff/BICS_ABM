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

  int S_count = 0;
  int E_count = 0;
  int I_count = 0;
  int R_count = 0;
  int D_count = 0;
  
  int vcount = igraph_vcount(g);

    for (int i = 0; i < vcount; i++) {
        ds = VAS(g, "disease_status", i);
        
        if (ds == "S") {
          S_count++;
        }
        else if (ds == "E") {
            rde = VAN(g, "remaining_days_exposed", i);
            if (rde == 0) {
                 SETVAS(g, "disease_status", i, "I");
                 I_count++;
             } else {
                 SETVAN(g, "remaining_days_exposed", i, rde - 1);
                 E_count++;
             }

        }
        else if (ds == "I") {
            rds = VAN(g, "remaining_days_sick", i);
            if (rds == 0) {
                SETVAS(g, "disease_status", i, "R");
                R_count++;

            } else {
                SETVAN(g, "remaining_days_sick", i, rds - 1);
                I_count++;
            }
        } 
        else if (ds == "R") {
               R_count++; 
        }
        else if (ds == "D") {
               D_count++;
        }
  }


  SETGAN(g, "S_count", S_count);
  SETGAN(g, "E_count", E_count);
  SETGAN(g, "I_count", I_count);
  SETGAN(g, "R_count", R_count);
  SETGAN(g, "D_count", D_count);

  cout << "S: " << S_count << " | ";
  cout << "E: " << E_count << " | ";
  cout << "I: " << I_count << " | ";
  cout << "R: " << R_count << " | ";
  cout << "D: " << D_count << " | ";
  cout << endl;

}
