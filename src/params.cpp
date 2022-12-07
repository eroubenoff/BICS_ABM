#include "BICS_ABM.h"
#include<string>
#include<iostream>
using namespace std;

/* Params default values */ 
extern "C" Params init_params(mt19937 generator) {

    Params params{
        1000, // N_HH
        4,    // WAVE
        2*24, 4*24, // GAMMA_MIN, MAX
        3*24, 7*24, // SIGMA_MIN, MAX
        0.1, // BETA
        {0.00001, 0.0001, 0.0001, 0.001, 0.001, 0.001, 0.01, 0.1, 0.1}, // MU
        5, // INDEX_CASES
        49, // SEED, POP_SEED
        100, // N_VAX_DAILY
        0.75, 0.95, // VE1, 2
        0.25, 0.95, // VEW, Boost 
        1,  // isolation_multiplier
        90*24, // t_reinfectoin
        0, // t0
        0.5, // alpha
        0.5    // rho

    }; 

    return params;
};

void print_params(const Params *params) {
    cout << "----------------------------------------"<< endl;
    cout << "Parameters passed to simulation:" << endl;

    cout << "N_HH:            " << params->N_HH << endl;
    cout << "GAMMA_MIN, _MAX: " << params->GAMMA_MIN << ", " << params->GAMMA_MAX << endl;
    cout << "SIGMA_MIN, _MAX: " << params->SIGMA_MIN << ", " << params->SIGMA_MAX << endl;
    cout << "BETA:            " << params->BETA<< endl;
    cout << "MU_VEC:          ";
    for (auto i: params->MU_VEC) {
        cout << i << "  ";
    }   
    cout << endl;
    cout << "INDEX_CASES:     " << params->INDEX_CASES << endl;
    cout << "SEED:            " << params->SEED<< endl;
    cout << "N_VAX_DAILY:     " << params->N_VAX_DAILY<< endl;
    cout << "VE1, VE2:        " << params->VE1 << ", " << params->VE2 << endl;
    cout << "VEW, VEBoost:    " << params->VEW << ", " << params->VEBOOST << endl;
    cout << "ALPHA:           " << params->ALPHA << endl;
    cout << "RHO:             " << params->RHO << endl;
    cout << "ISOLATION_MULT.: " << params->ISOLATION_MULTIPLIER<< endl;
    cout << "----------------------------------------"<< endl;

}
