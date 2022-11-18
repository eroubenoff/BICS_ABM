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


void decrement(igraph_t *g, History *h) {
    igraph_real_t rds, rde, tv2;

    short int ds;
    igraph_real_t vs;

    int S_count = 0;
    int E_count = 0;
    int I_count = 0;
    int R_count = 0;
    int D_count = 0;
    int V1_count = 0;
    int V2_count = 0;

    const char S = 'S';
    const char E = 'E';
    const char I = 'I';
    const char R = 'R';
    const char D = 'D';

    int vcount = igraph_vcount(g);
    int mu;

    for (int i = vcount; i--; ) {
        ds = VAS(g, "disease_status", i)[0];
        vs = VAN(g, "vaccine_status", i);

        switch(ds) {
            case S: 
                ++S_count;
                break;

            case E:
                rde = VAN(g, "remaining_days_exposed", i);
                if (rde == 0.0) {
                    SETVAS(g, "disease_status", i, "I");
                    ++I_count;
                } else {
                    SETVAN(g, "remaining_days_exposed", i, rde - 1.0);
                    ++E_count;
                }
                break;

            case I:
                rds = VAN(g, "remaining_days_sick", i);
                mu = VAN(g, "mu", i);
                if ((rds == 0.0) & (mu == 0.0)) {
                    SETVAS(g, "disease_status", i, "R");
                    ++R_count;

                } else if ((rds == 0.0 ) & (mu == 1.0) ) {
                    SETVAS(g, "disease_status", i, "D");
                    ++D_count;

                } else {
                    SETVAN(g, "remaining_days_sick", i, rds - 1.0);
                    ++I_count;
                }
                break;

            case R:
                ++R_count;
                break;

            case D:
                ++D_count;
                break;

        }

        /*
        switch(vs) {
            case -1: 
                break;
            case 0: 
                break;
            case 1: 
                V1_count++;
                tv2 = VAN(g, "time_until_v2", i);
                if (tv2 > 0) SETVAN(g, "time_until_v2", i,  tv2 - 1);
                break;
            case 2:
                V2_count++;
                break;

        }
        */

        if (vs == 1.0) {
            ++V1_count;
            tv2 = VAN(g, "time_until_v2", i);
            if (tv2 > 0.0) SETVAN(g, "time_until_v2", i,  tv2 - 1.0);
        } else if (vs == 2.0) {
            ++V2_count;
        } else {
            continue;
        }

    }


    /* Tally edge counts */
    unordered_map<string, int> etypes;
    etypes["household"] = 0;
    etypes["work"] = 0;
    etypes["random"] = 0;
    for (int i = igraph_ecount(g); i--; ) {
        etypes[EAS(g, "type", i)] += 1;
    }



    SETGAN(g, "S_count", S_count);
    SETGAN(g, "E_count", E_count);
    SETGAN(g, "I_count", I_count);
    SETGAN(g, "R_count", R_count);
    SETGAN(g, "D_count", D_count);
    SETGAN(g, "V1_count", V1_count);
    SETGAN(g, "V2_count", V2_count);

    h->add_history(S_count, E_count, I_count, R_count, D_count, V1_count, V2_count);

    cout << "S: " << std::setw(5) << S_count << " | ";
    cout << "E: " << std::setw(5) << E_count << " | ";
    cout << "I: " << std::setw(5) << I_count << " | ";
    cout << "R: " << std::setw(5) << R_count << " | ";
    cout << "D: " << std::setw(5) << D_count << " | ";
    cout << "V1: " << std::setw(5) << V1_count << " | ";
    cout << "V2: " << std::setw(5) << V2_count << " | ";
    cout << "Edge counts: " << "Household: " <<  setw(5) << etypes["household"] << " Work " << setw(5) << etypes["work"] << " Random  " << setw(5) <<  etypes["random"];
    cout << flush;

}
