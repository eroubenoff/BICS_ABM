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
    const char* ds;

    int S_count = 0;
    int E_count = 0;
    int I_count = 0;
    int R_count = 0;
    int D_count = 0;

    int vcount = igraph_vcount(g);
    int mu;

    for (int i = vcount; i--; ) {
        ds = VAS(g, "disease_status", i);

        switch(ds[0]) {
            case 'S': 
                S_count++;
                break;

            case 'E':
                rde = VAN(g, "remaining_days_exposed", i);
                if (rde == 0) {
                    SETVAS(g, "disease_status", i, "I");
                    I_count++;
                } else {
                    SETVAN(g, "remaining_days_exposed", i, rde - 1);
                    E_count++;
                }
                break;

            case 'I':
                rds = VAN(g, "remaining_days_sick", i);
                mu = VAN(g, "mu", i);
                if ((rds == 0) & (mu == 0)) {
                    SETVAS(g, "disease_status", i, "R");
                    R_count++;

                } else if ((rds == 0 ) & (mu == 1) ) {
                    SETVAS(g, "disease_status", i, "D");
                    D_count++;

                } else {
                    SETVAN(g, "remaining_days_sick", i, rds - 1);
                    I_count++;
                }
                break;

            case 'R':
                R_count++;
                break;

            case 'D':
                D_count++;
                break;

        }

    }


    SETGAN(g, "S_count", S_count);
    SETGAN(g, "E_count", E_count);
    SETGAN(g, "I_count", I_count);
    SETGAN(g, "R_count", R_count);
    SETGAN(g, "D_count", D_count);

    cout << "S: " << std::setw(5) << S_count << " | ";
    cout << "E: " << std::setw(5) << E_count << " | ";
    cout << "I: " << std::setw(5) << I_count << " | ";
    cout << "R: " << std::setw(5) << R_count << " | ";
    cout << "D: " << std::setw(5) << D_count << " | ";
    cout << flush;

}
