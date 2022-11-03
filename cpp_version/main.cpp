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


void delete_all_edges(igraph_t *g) {
    igraph_es_t es;
    igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
    igraph_delete_edges(g, es);
    igraph_es_destroy(&es);

}

void daytime_mixing(igraph_t *g, vector<poisson_distribution<int>> &pois,  mt19937 &generator) {

    igraph_rng_seed(igraph_rng_default(), generator());
    // Clear any edges present
    delete_all_edges(g);

    // Generate degree sequence from num_cc_nonhh
    igraph_t new_graph;
    igraph_vector_int_t num_cc_nonhh;
    igraph_vector_int_init(&num_cc_nonhh, igraph_vcount(g));

    // Take a poisson value for each
    /*
    for (int i = 0; i < igraph_vcount(g); i++) {
        poisson_distribution<int> distribution(VAN(g, "num_cc_nonhh", i) / 10);
        VECTOR(num_cc_nonhh)[i]=distribution(generator);

    }
    */

   //  vector<int> num_cc_nonhh(igraph_vcount, 0);
    for (int i = igraph_vcount(g); i--; ) {
        VECTOR(num_cc_nonhh)[i] = pois[i](generator);
    }


    // Need to make the degree sequence sum to even number
    // Do this by randomly sampling indices until we find a nonzero one
    // and decrement it
    int sum = igraph_vector_int_sum(&num_cc_nonhh);

    if (sum % 2 == 1) { 
        // Pick a random index
        int tries = 0;
        int randomIndex;
        while (tries < 1000) {
            randomIndex = rand() % igraph_vector_int_size(&num_cc_nonhh);

            if (VECTOR(num_cc_nonhh)[randomIndex] > 1) {
                VECTOR(num_cc_nonhh)[randomIndex] -= 1;
                break;
            }

            tries++;

        }
    }

    // Form degree sequence and get edgelist
    igraph_degree_sequence_game(&new_graph, &num_cc_nonhh, NULL, IGRAPH_DEGSEQ_CONFIGURATION); //IGRAPH_DEGSEQ_FAST_HEUR_SIMPLE ); // IGRAPH_DEGSEQ_CONFIGURATION);
    igraph_vector_int_t edgelist;
    igraph_vector_int_init(&edgelist, igraph_vcount(&new_graph));
    igraph_get_edgelist(&new_graph, &edgelist, false);

    // igraph_vector_int_print(&edgelist);

    // Add edgelist to old graph
    igraph_add_edges(g, &edgelist, NULL);


    igraph_vector_int_destroy(&num_cc_nonhh);
    igraph_vector_int_destroy(&edgelist);


}


void BICS_ABM(
    History *history, 
    const int N_HH,
    const int WAVE,
    const int GAMMA_MIN,
    const int GAMMA_MAX,
    const int SIGMA_MIN,
    const int SIGMA_MAX,
    const float BETA,
    const vector<float> MU_VEC,
    const int INDEX_CASES,
    const int SEED,
    const int N_VAX_DAILY,
    const float VE1,
    const float VE2) {

    const bool cached = false;
    mt19937 generator(SEED);

    /* 
     * Pre-generate all distributions as cycling vectors
     *
     * */ 

    CyclingVector<int> gamma_vec(1000, [&generator, GAMMA_MIN, GAMMA_MAX](){return (uniform_int_distribution<int>(GAMMA_MIN, GAMMA_MAX))(generator);});
    CyclingVector<int> sigma_vec(1000, [&generator, SIGMA_MIN, SIGMA_MAX](){return (uniform_int_distribution<int>(SIGMA_MIN, SIGMA_MAX))(generator);});
    // CyclingVector<int> beta_vec(1000, [&generator, BETA](){return (bernoulli_distribution(BETA))(generator);});

    // Transmission probability 
    unordered_map<int, CyclingVector<int>*> beta;
    beta[0] = new CyclingVector<int>(1000, [&generator, BETA](){return(bernoulli_distribution(BETA)(generator));});
    beta[1] = new CyclingVector<int>(1000, [&generator, BETA, VE1](){return(bernoulli_distribution(BETA*(1-VE1))(generator));});
    beta[2] = new CyclingVector<int>(1000, [&generator, BETA, VE2](){return(bernoulli_distribution(BETA*(1-VE2))(generator));});


    // Create mortality
    unordered_map<string, CyclingVector<int> >mu;
    mu["[0,18)"]  = CyclingVector<int>(1000, [&generator, MU_VEC](){return (bernoulli_distribution(MU_VEC[0]))(generator);});
    mu["[18,25)"] = CyclingVector<int>(1000, [&generator, MU_VEC](){return (bernoulli_distribution(MU_VEC[1]))(generator);});
    mu["[25,35)"] = CyclingVector<int>(1000, [&generator, MU_VEC](){return (bernoulli_distribution(MU_VEC[2]))(generator);});
    mu["[35,45)"] = CyclingVector<int>(1000, [&generator, MU_VEC](){return (bernoulli_distribution(MU_VEC[3]))(generator);});
    mu["[45,55)"] = CyclingVector<int>(1000, [&generator, MU_VEC](){return (bernoulli_distribution(MU_VEC[4]))(generator);});
    mu["[55,65)"] = CyclingVector<int>(1000, [&generator, MU_VEC](){return (bernoulli_distribution(MU_VEC[5]))(generator);});
    mu["[65,75)"] = CyclingVector<int>(1000, [&generator, MU_VEC](){return (bernoulli_distribution(MU_VEC[6]))(generator);});
    mu["[75,85)"] = CyclingVector<int>(1000, [&generator, MU_VEC](){return (bernoulli_distribution(MU_VEC[7]))(generator);});
    mu[">85"]     = CyclingVector<int>(1000, [&generator, MU_VEC](){return (bernoulli_distribution(MU_VEC[8]))(generator);});
    

    // Create the empty graph
    igraph_t graph;

    /* Turn on attribute handling. */
    igraph_set_attribute_table(&igraph_cattribute_table);

    /* Create a directed graph with no vertices or edges. */
    igraph_empty(&graph, 0, IGRAPH_UNDIRECTED);

    /* Generate population*/
    gen_pop_from_survey_csv(WAVE, &graph, N_HH, cached);

    cout << "N vertices: " << igraph_vcount(&graph) << endl;

    // Create an unordered_map of hhids
    unordered_map<string, vector<int> > hhids;
    for (int i = 0; i < igraph_vcount(&graph); i++) {
        hhids[VAS(&graph, "hhid", i)].push_back(i);
    }

    // Create a household edge list
    igraph_vector_int_t hhedges;
    igraph_vector_int_init(&hhedges, 0);
    for (auto h : hhids) {

        // Create all combinations within the hh by nested loop
        for (int k=0; k < h.second.size(); k++) {
            for (int l=k+1; l < h.second.size(); l++) {
                if (k == l) {
                    continue;
                }
                else{
                    igraph_vector_int_push_back(&hhedges, h.second[k]);
                    igraph_vector_int_push_back(&hhedges, h.second[l]);
                }
            }
        }
    }

    // Add hh edges
    igraph_add_edges(&graph, &hhedges, NULL);

    /* Print the final result. */
    // print_attributes(&graph, true);


    // Pick nodes at random to be infected
    uniform_int_distribution<int> distribution(0,igraph_vcount(&graph));
    for (int i = 0; i < 5; i++) {
        int index_case = distribution(generator);
        cout << "index case " << index_case << endl;
        set_sick(&graph, index_case, 3*24, 5*24, false);
    }

    int day = 0;
    int hr = 0;
    decrement(&graph, history);


    /* For daytime, need to create poisson generators for contact */
    vector<poisson_distribution<int>> num_cc_nonhh;
    for (int i = 0; i < igraph_vcount(&graph); i++) {
        num_cc_nonhh.push_back(poisson_distribution<int>(VAN(&graph, "num_cc_nonhh", i)/float(10)));
    }

    // igraph_add_edges(&graph, &hhedges, NULL);

    while (GAN(&graph, "I_count") + GAN(&graph, "E_count") > 0){

        // Hours 0-8
        for (hr = 0; hr < 8; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            transmit(&graph, beta, gamma_vec, sigma_vec, mu);
            decrement(&graph, history);

        }

        delete_all_edges(&graph);

        distribute_vax(&graph, N_VAX_DAILY, 25*24);

        // Hours 8-16
        for (hr = 8; hr < 16; hr++){
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            daytime_mixing(&graph, num_cc_nonhh, generator);
            transmit(&graph, beta, gamma_vec, sigma_vec, mu);
            decrement(&graph, history);
        }


        // Hours 16-24
        delete_all_edges(&graph);
        igraph_add_edges(&graph, &hhedges, NULL);
        for (hr = 16; hr < 24; hr++ ) {
            cout << "\r" << "Day " << std::setw(4) << day <<  " Hour " << std::setw(2) << hr << " | ";
            transmit(&graph, beta, gamma_vec, sigma_vec, mu);
            decrement(&graph, history);
        }

        day++;


    }


    /* Delete all remaining attributes. */
    DELALL(&graph);

    /* Destroy the graph. */
    igraph_destroy(&graph);

    // Destroy vectors
    igraph_vector_int_destroy(&hhedges);

    delete beta[0];
    delete beta[1];
    delete beta[2];

}

int main(int argc, char **argv) {

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

    // return 0;
    // Parse the args
    // Run in interactive mode?
    // const bool INTERACTIVE = args.find("-i") == args.end() ? false : true;
    // Save hourly node and edgelist
    // const bool FULL_HISTORY = args.find("-full_history") == args.end() ? false : true;
    // Number of households
    const int N_HH = args.find("-n_hh") == args.end() ? 1000: stoi(args["-n_hh"]);
    // Wave to simulate from
    const int WAVE = args.find("-wave") == args.end() ? 4: stoi(args["-wave"]);
    // Lower and upper bounds on latent period, in hours
    const int GAMMA_MIN = args.find("-gamma_min") == args.end() ? 2*24: stoi(args["-gamma_min"]);
    const int GAMMA_MAX = args.find("-gamma_max") == args.end() ? 4*24: stoi(args["-gamma_max"]);
    // Lower and upper bounds on infectious period, in hours
    const int SIGMA_MIN = args.find("-sigma_min") == args.end() ? 3*24: stoi(args["-sigma_min"]);
    const int SIGMA_MAX = args.find("-sigma_max") == args.end() ? 7*24: stoi(args["-sigma_max"]);
    // Per-contact probability of transmission
    const float BETA = args.find("-beta") == args.end() ? 0.2: stof(args["-beta"]);
    // Mortality rate vector. Must be as long age 
    const vector<float> MU_VEC = args.find("-mu") == args.end() ? vector<float>{0.00001, 0.0001, 0.0001, 0.001, 0.001, 0.001, 0.01, 0.1, 0.1}: stovf(args["-mu"]);
    // Number of initial cases
    const int INDEX_CASES = args.find("-index_cases") == args.end() ? 5 : stoi(args["-index_cases"]);                
    // Passed to generator
    const int SEED = args.find("-seed") == args.end() ? 494949 : stoi(args["-seed"]);                   
    // Used cached data
    // const bool cached = args.find("-cached") == args.end() ? 1 : (args["-cached"] == "false" ? 0  : 1);

    // Vaccine params
    const int N_VAX_DAILY= args.find("-n_vax_daily") == args.end() ? N_HH / 20: stoi(args["-n_vax_daily"]);                   
    const float VE1 = args.find("-ve1") == args.end() ? 0.75 : stof(args["-ve1"]);                   
    const float VE2 = args.find("-ve2") == args.end() ? 0.95 : stof(args["-ve2"]);                   


    // History object
    History history(2000);

    BICS_ABM(
        &history,
        N_HH,
        WAVE,
        GAMMA_MIN,
        GAMMA_MAX,
        SIGMA_MIN,
        SIGMA_MAX,
        BETA,
        MU_VEC,
        INDEX_CASES,
        SEED,
        N_VAX_DAILY,
        VE1,
        VE2);

    history.save();
    // history.plot_trajectory();

    return 0;
}
