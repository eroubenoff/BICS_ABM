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
        49, 4949, // SEED, POP_SEED
        100, // N_VAX_DAILY
        0.75, 0.95, // VE1, 2
        0.25, 0.95, // VEW, Boost 
        1,  // isolation_multiplier
        90*24, // t_reinfectoin
        0, // t0

        "age;age;age;age;hesitancy", // VAX_RULES_COLS
        ">85;[75,85);[65,75);[55,65);0.5", // VAX_RULES_VALS
        {1,1,1,2},  // VAX_CONDS_N
        4 // VAX_RULES_N
    }; 

    /*
    CyclingVector<int>* gamma = new CyclingVector<int>(1000, 
            [&generator, &params](){return (uniform_int_distribution<int>(params.GAMMA_MIN, params.GAMMA_MAX))(generator);});
    params.gamma_ptr =  gamma;

    CyclingVector<int>* sigma = new CyclingVector<int>(1000, 
            [&generator, &params](){return (uniform_int_distribution<int>(params.SIGMA_MIN, params.SIGMA_MAX))(generator);});
    params.sigma_ptr = sigma;


    // Transmission probability 
    unordered_map<int, CyclingVector<int>> *beta = new unordered_map<int, CyclingVector<int>>;
    beta->[::V0] = CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params.BETA)(generator));});
    beta->[::V1] = CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params.BETA*(1-params.VE1))(generator));});
    beta->[::V2] = CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params.BETA*(1-params.VE2))(generator));});
    beta->[::VW] = CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params.BETA*(1-params.VE2))(generator));});
    beta->[::VBoost] = CyclingVector<int>(1000, [&generator, &params](){return(bernoulli_distribution(params.BETA*(1-params.VE2))(generator));});
    params.beta_ptr = beta; 


    // Create mortality
    unordered_map<string, CyclingVector<int>> *mu = new unordered_map<string, CyclingVector<int>>;
    mu->["[0,18)"]  = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params.MU_VEC[0]))(generator);});
    mu->["[18,25)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params.MU_VEC[1]))(generator);});
    mu->["[25,35)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params.MU_VEC[2]))(generator);});
    mu->["[35,45)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params.MU_VEC[3]))(generator);});
    mu->["[45,55)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params.MU_VEC[4]))(generator);});
    mu->["[55,65)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params.MU_VEC[5]))(generator);});
    mu->["[65,75)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params.MU_VEC[6]))(generator);});
    mu->["[75,85)"] = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params.MU_VEC[7]))(generator);});
    mu->[">85"]     = CyclingVector<int>(1000, [&generator, &params](){return (bernoulli_distribution(params.MU_VEC[8]))(generator);});
    params.mu = mu;
    */

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
    cout << "POP_SEED:        " << params->POP_SEED<< endl;
    cout << "N_VAX_DAILY:     " << params->N_VAX_DAILY<< endl;
    cout << "VE1, VE2:        " << params->VE1 << ", " << params->VE2 << endl;
    cout << "VEW, VEBoost:    " << params->VEW << ", " << params->VEBOOST << endl;
    cout << "ISOLATION_MULT.: " << params->ISOLATION_MULTIPLIER<< endl;
    cout << "VAX_RULES_COLS:  " << params->VAX_RULES_COLS << endl;
    cout << "VAX_RULES_VALS:  " << params->VAX_RULES_VALS << endl;
    cout << "VAX_CONDS_N:     " << params->VAX_CONDS_N << endl;
    cout << "VAX_RULES_N:     " << params->VAX_RULES_N << endl;
    cout << "----------------------------------------"<< endl;

}
