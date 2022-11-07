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

#define MAXCHAR 1000


int main(int argc, char **argv) {

    Params params = init_params();
    Data data;
    History history;

    // Parse command line options into an unordered map
    unordered_map<string, string> args;

    for (int i = 1; i < argc; i++) {
        if ((argv[i][0] == '-') & (argv[i+1][0] != '-'))  {
            // Pull a pair like 
            args[argv[i]] = argv[i+1]; 
            i++;
        } else {
            args[argv[i]] = "true";
        }
    }

    for (auto i: args) {
        cout << i.first << ": " << i.second<< endl;
    }

    // Parse the args into params object

    // Number of households
    if (args.find("-n_hh") != args.end()) {
        params.N_HH = stoi(args["-n_hh"]);
    }

    // Wave to simulate from
    if (args.find("-wave") != args.end()){
        params.WAVE = stoi(args["-wave"]); 
    }

    // Lower and upper bounds on latent period, in hours
    if (args.find("-gamma_min") != args.end() ){
        params.GAMMA_MIN = stoi(args["-gamma_min"]);
    }
    if (args.find("-gamma_max") != args.end() ){
        params.GAMMA_MAX = stoi(args["-gamma_max"]);
    }

    // Lower and upper bounds on infectious period, in hours
    if (args.find("-sigma_min") != args.end() ){
        params.SIGMA_MIN = stoi(args["-sigma_min"]);
    }
    if (args.find("-sigma_max") != args.end() ){
        params.SIGMA_MAX = stoi(args["-sigma_max"]);
    }

    // Per-contact probability of transmission
    if (args.find("-beta") != args.end()){
        params.BETA = stof(args["-beta"]);
    }

    // Mortality rate vector. Must be as long age 
    if (args.find("-mu") != args.end()) {
        vector<float> muvec = stovf(args["-mu"]) ; 
        copy(muvec.begin(), muvec.end(), params.MU_VEC);
        // params.MU_VEC = stovf(args["-mu"]);
    }

    // Number of initial cases
    if (args.find("-index_cases") != args.end()) {
        params.INDEX_CASES = stoi(args["-index_cases"]);
    }

    // Passed to generator
    if (args.find("-seed") != args.end()) {
        params.POP_SEED = stoi(args["-seed"]);
    }
    if (args.find("-pop_seed") != args.end()) {
        params.POP_SEED = stoi(args["-pop_seed"]);
    }

    // Vaccine params
    if (args.find("-n_vax_daily") != args.end() ) {
        params.N_VAX_DAILY = stoi(args["-n_vax_daily"]);
    }
    if (args.find("-ve1") != args.end()) {
        params.VE1 = stof(args["-ve1"]);
    }
    if (args.find("-ve2") != args.end()) {
        params.VE2 = stof(args["-ve2"]);
    }



    for (int i = 0; i < 10; i++) {
        cout << "sim no " << i << endl;
        params.SEED = 49+i;
        params.POP_SEED = 49+i;
        BICS_ABM(&data, &params, &history);
    }


    return 0;
}
