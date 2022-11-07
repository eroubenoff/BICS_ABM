#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <igraph.h>
#include<fstream>
#include "BICS_ABM.h"
#include <random>
#include <map>
#include <fstream>
#include <iostream>


using namespace std;


/*
 * Adds a vertex to the graph. 
 *
 * @param g graph pointer
 * @param i index of node to add 
 */
void add_vertex(igraph_t *g,
        string age,
        string gender,
        string ethnicity,
        int num_cc_nonhh, 
        int vaccine_priority,
        string hhid) {

    if (false) {
        cout << "Age: " << age << " Gender: " << gender << " Ethnicity: " << \
            ethnicity << " num_Cc_nonhh: " << num_cc_nonhh << " hhid: " << hhid << endl;
    }
    // Add vertex 
    igraph_add_vertices(g, 1, NULL);

    int i = igraph_vcount(g) - 1;

    // Age
    SETVAS(g, "age", i, age.c_str());

    // Gender
    SETVAS(g, "gender", i, gender.c_str());

    // Ethnicity
    SETVAS(g, "ethnicity", i, ethnicity.c_str());

    // Num cc nonhh
    SETVAN(g, "num_cc_nonhh", i, num_cc_nonhh);

    // Hhid
    SETVAS(g, "hhid", i, hhid.c_str());

    // Fixed characteristics
    SETVAS(g, "disease_status", i, "S");
    SETVAN(g, "remaining_days_exposed", i, -1);
    SETVAN(g, "remaining_days_sick", i, -1);
    SETVAN(g, "vaccine_status", i, 0);
    SETVAN(g, "vaccine_priority", i, vaccine_priority);
    SETVAN(g, "time_until_v2", i, -1);




}






/* 
 * Read a population from a csv
 * 
 * @param string path 
 * 	path of the already-generated population, probably from python
 * @param igraph_t &graph
 *	pointer to graph to initialize population in 		
 * 					  

 * @return none; modifies &graph in place
 */
void read_pop_from_csv(string path, igraph_t *g) {

    // Create pointer to pop file and open
    ifstream fin;
    fin.open("pop.csv");

    // Read the csv header (used to create attributes)
    vector<string> header = get_csv_row(fin);

    // Invert header into unordered map for column lookup
    unordered_map<string, int> colnames;
    for (int i = 0; i < header.size(); i++) {
        colnames[header[i]] = i;
    }

    // Loop through the attributes of the first nodes
    vector<string> row;
    int i = 0;
    while( fin.peek() != EOF ) {

        // Read the row
        row = get_csv_row(fin);

        add_vertex(g, 
                row[colnames["age"]],
                row[colnames["gender"]],
                row[colnames["ethnicity"]],
                stoi(row[colnames["num_cc_nonhh"]]),
                0,
                row[colnames["hhid"]]
                );

        i++;

    }

    cout << "Loaded " << i << " nodes from " << path << endl;


    /* Close the fstream */
    fin.close();


}



/*
 * Function to recode POLYMOD age categories to match BICS
 *
 * @param string age_s: input string
 * @return string, age in bins
 */
string recode_age(string age_s) {

    if (age_s == "NA") return 0; 

    int age = stoi(age_s);

    if (age < 18)
        return "[0,18)"; 
    else if (age < 25)
        return "[18,25)";
    else if (age < 35)
        return "[25,35)";
    else if (age < 45)
        return "[35,45)";
    else if (age < 55)
        return "[45,55)";
    else if (age < 65)
        return "[55,65)" ;
    else if (age < 75)
        return "[65,75)"; 
    else if (age < 85)
        return "[75,85)";
    else if (age >= 85)
        return ">85";
    else 
        throw runtime_error("Invalid age");


}







/*
 * Function to recode POLYMOD gender categories to match BICS
 *
 * @param string age_s: input string
 * @return string, gender
 */
string recode_gender(string gender) {


    if (gender == "M")
        return "Male"; 
    else if (gender == "F")
        return "Female";
    else 
        return "-1";

}


/*
 * Helper function to wrap any strings containing
 * commas in quotes
 *
 */
string escape_commas(string s) {

    bool has_comma = false;

    for (int i = 0; i < s.size(); i++) {
        if (s[i] == ',') has_comma = true;
        else continue;
    }

    if (has_comma) {
        s = "\""  + s + "\"";
    }

    return s;
    
}






/* 
 *
 * Generates a population of n households within graph g
 *
 */

void gen_pop_from_survey_csv(Data *data, igraph_t *g, int n, int pop_seed) {

    bool verbose = false;
    bool fill_polymod = true;

    RandomVector dd(data->BICS_weights);
 
    // discrete_distribution<int> dd{weights.begin(), weights.end()};
    // default_random_engine generator;
    mt19937 generator(pop_seed);
    CyclingVector<int> vax_vec(100, [&generator](){return (discrete_distribution{1,1,1,1})(generator) - 1;});


    /* Tuple for keys*/ 
    typedef tuple <int,string,string> key;



    /* 
     * Draw n random nids to be the head of hh. Then,
     * booststrap the population to match the reported
     * hh_roster by hhsize, age, and gender.
     * Create a node for each within the passed graph.
     * 
     */
    for (n; n--; ) {
        // cout <<"Initializing new hh"<< endl;

        int hhead = dd(generator);
        // cout << hhead << endl;
        int hhsize = stoi(data->BICS_data[hhead][data->BICS_colnames["hhsize"]]); 
        // cout << hhsize << endl;
        string hhid = randstring(16);

        if (verbose) cout << "Adding respondent " << hhead << " as head of hhid "<< hhid << " of size " << hhsize << endl;

        add_vertex(g,
                data->BICS_data[hhead][data->BICS_colnames["age"]],
                data->BICS_data[hhead][data->BICS_colnames["gender"]],
                data->BICS_data[hhead][data->BICS_colnames["ethnicity"]],
                stoi(data->BICS_data[hhead][data->BICS_colnames["num_cc_nonhh"]]), 
                vax_vec.next(),
                hhid);

        // Sample who matches that 
        for (int j = 1; j < min(5,hhsize); j++) {

            string hhmember_age = recode_age(data->BICS_data[hhead][data->BICS_colnames["resp_hh_roster#1_" + to_string(j) + "_1"]]);
            string hhmember_gender = recode_gender(data->BICS_data[hhead][data->BICS_colnames["resp_hh_roster#2_" + to_string(j)]]);

            if (hhmember_age == "") {cout<<"Not enough info on household member" << endl; continue;}


            // Create  vector of weights; set weight to 0 if doesn't match
            key k = key(hhsize, hhmember_age, hhmember_gender);
            //cout << get<0>(k) << "  " << get<1>(k) << "  " <<  get<2>(k) << endl;


            // Check if key exists in map
            if (!data->hh_distn.count(k) & fill_polymod) {

                // Check if the key is in the POLYMOD distributions m.find("f") == m.end()

                if (data->distributions_POLYMOD.find(k) == data->distributions_POLYMOD.end()) {
                    if (verbose) cout << "Corresponding person not found in POLYMOD" << endl;

                    continue;

                }

                int pmod_member = data->distributions_POLYMOD[k](generator); 

                if (verbose) cout << "Adding POLYMOD respondent " << pmod_member << " to hhid "<< hhid << endl;
                add_vertex(g,
                        data->POLYMOD_data[pmod_member][data->POLYMOD_colnames["part_age"]],
                        data->POLYMOD_data[pmod_member][data->POLYMOD_colnames["part_gender"]],
                        "NA", // POLYMOD_data[i][POLYMOD_colnames["ethnicity"]],
                        stoi(data->POLYMOD_data[pmod_member][data->POLYMOD_colnames["num_cc_nonhh"]]), 
                        vax_vec.next(),
                        hhid);


                continue;
            } 


            // Sample a corresponding person
            int hh_member = data->hh_distn[k](generator) ;
            if (verbose) cout << "Adding respondent " << hh_member << " to hhid "<< hhid << endl;

            add_vertex(g,
                    data->BICS_data[hh_member][data->BICS_colnames["age"]],
                    data->BICS_data[hh_member][data->BICS_colnames["gender"]],
                    data->BICS_data[hh_member][data->BICS_colnames["ethnicity"]],
                    stoi(data->BICS_data[hh_member][data->BICS_colnames["num_cc_nonhh"]]), 
                    vax_vec.next(),
                    hhid) ;

        }

    }


}



