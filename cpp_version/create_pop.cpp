#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <igraph.h>
#include<fstream>
#include "defs.h"
#include <random>
#include <map>
/*
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
*/
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
 * Load POLYMOD 
 *
 * POLYMOD is split into a number of files, and we only
 * need a little bit of data from each. The main operation
 * is to merge the polymod_participants file (containing
 * the participant information) with the polymod_contacts
 * file (from which we derive num_cc_nonhh). Need to also
 * pull in household size from file hh_common, which contains
 * household contacts
 *
 * @param path: location of the polymod file
 *
 * @return is a tuple containing:
 *  - unordered_map<string, int> containing the colnames and locations
 *  - vector<vector<string> >, the actual data in row-major format
 *
 *
 * */

auto load_POLYMOD(string path = "POLYMOD/") {

    /*----------------------------------------------
     *
     * Load participants file
     *
     *---------------------------------------------*/

    int n_polymod = 0;
    ifstream  polymod_f;
    polymod_f.open(path + "2008_Mossong_POLYMOD_participant_common.csv");

    /* 
     * Get header from polymod_participants and loop through to 
     * create a hash 
     * */

    vector<string> header = get_csv_row(polymod_f);

    unordered_map<string, int> polymod_participants_colnames;
    for (int i = 0; i < header.size(); i++) {
        // cout << header[i] << endl;
        polymod_participants_colnames[header[i]] = i;
    }

    /* 
     * Read in the participants file 
     * */ 

    vector<vector<string> > polymod_participants;
    int ncol = polymod_participants_colnames.size();
    int age_idx = polymod_participants_colnames["part_age"];
    int gen_idx = polymod_participants_colnames["part_gender"];
    while (polymod_f.peek() != EOF) {

        /* 
         * Push back the row 
         * */ 

        polymod_participants.push_back(
                get_csv_row(polymod_f, ncol)
                );

        /* 
         * Recode age to match BICS distributions 
         * */

        polymod_participants[n_polymod][age_idx] = recode_age(
                polymod_participants[n_polymod][age_idx]
                );

        /* 
         * Recode gender to match BICS distributions 
         * */

        polymod_participants[n_polymod][gen_idx] = recode_gender(
                polymod_participants[n_polymod][gen_idx]
                );


        n_polymod++;

    }

    polymod_f.close();





    /*----------------------------------------------
     *
     * Read in the contact_common file. All that we need here is the 
     * number of entries for each participant id; so, we'll create
     * a hash immediately on this count.
     *
     *---------------------------------------------*/


    /* 
     * Read the header and create a hash on colnames 
     * */ 
    polymod_f.open(path + "2008_Mossong_POLYMOD_contact_common.csv");
    header = get_csv_row(polymod_f);
    unordered_map<string, int> polymod_contact_colnames;
    for (int i = 0; i < header.size(); i++) {
        polymod_contact_colnames[header[i]] = i;
    }


    /* 
     * Create a hash of ids of number of household contacts
     * and read file to fill, incrementing the corresponding number
     *
     * */

    unordered_map<string, int> num_cc_nonhh;
    vector<string> row;

    while(polymod_f.peek() != EOF) {
        row = get_csv_row(polymod_f, polymod_contact_colnames.size());

        /* 
         * Only take nonhousehold contacts 
         * */

        if (!(row[polymod_contact_colnames["cnt_home"]] == "TRUE") ) {

            /* 
             * Increment the corresponding hash entry 
             * */ 

            num_cc_nonhh[row[polymod_contact_colnames["part_id"]]] += 1;
        }
    }

    polymod_f.close(); 




    /* 
     * Add this as a field to the main df 
     * */

    int part_id_col = polymod_participants_colnames["part_id"];
    polymod_participants_colnames["num_cc_nonhh"] = polymod_participants_colnames.size();

    for (int i = 0; i < n_polymod; i++) {

        /* 
         * Pull the participant id of row i for indexing num_cc_nonhh 
         * */

        string part_id = polymod_participants[i][part_id_col];

        /* 
         * Push this to the last field 
         * */

        int part_num_cc_nonhh = num_cc_nonhh[part_id];
        polymod_participants[i].push_back(to_string(part_num_cc_nonhh));

    }





    /*---------------------------------------------
     *
     * Read in the households file. Similar to the contact_common
     * file, all we need here is the hhsize; so, just 
     * extract this field, hash it, and add to the main df 
     * 
     *---------------------------------------------*/

    polymod_f.open(path + "2008_Mossong_POLYMOD_hh_common.csv") ;
    header = get_csv_row(polymod_f);
    unordered_map<string, int> polymod_hh_colnames;
    for (int i = 0; i < header.size(); i++) {
        polymod_hh_colnames[header[i]] = i;
    }

    /* 
     * All we need from this file are the hh_id
     * and the corresponding hh_size; note that we are 
     * merging on household id not participant id.
     * Create a hash of this information 
     *
     * */

    unordered_map<string, string> hh_size;
    while (polymod_f.peek() != EOF) {

        /* 
         * Read row 
         * */
        row = get_csv_row(polymod_f, polymod_hh_colnames.size());

        /* 
         * Get the corresponding data from the row
         * */ 

        hh_size[row[polymod_hh_colnames["hh_id"]]] = row[polymod_hh_colnames["hh_size"]];


    }

    polymod_f.close();




    /* 
     * Cycle through polymod participants and append household size 
     * */ 

    polymod_participants_colnames["hh_size"] = polymod_participants_colnames.size();
    int hh_idx = polymod_participants_colnames["hh_id"]; 
    for (int i = 0; i < n_polymod; i++) {

        /* 
         * Consistency check to make sure we haven't already appended 
         * */

        if (!(polymod_participants[i].size() == (polymod_participants_colnames.size() -1))) {
            throw runtime_error("Trying to append hh_id twice");
        }

        polymod_participants[i].push_back(hh_size[polymod_participants[i][hh_idx]]);

    }


    return make_tuple(polymod_participants_colnames, polymod_participants);


}


/* 
 *
 * Generates a population of n households within graph g
 *
 */

void gen_pop_from_survey_csv(string path, igraph_t *g, int n, bool fill_polymod) {


    /* Create pointer to input files */
    ifstream fin; 
    fin.open(path);

    /* Return object as vector of vectors */
    vector<vector<string> >  input_data;

    /* Get the header */
    vector<string> header = get_csv_row(fin);

    /* Invert header into unordered map for column lookup */
    unordered_map<string, int> colnames;
    for (int i = header.size(); i--;) {
        colnames[header[i]] = i;
    }

    /* Get each row and append it */
    int nrow = 0;
    vector<string> row;

    while (fin.peek() != EOF) {
        row = get_csv_row(fin);

        input_data.push_back(row);

        nrow++;
    }

    cout << nrow << " rows read" << endl;

    /* Create vector of weights */
    vector<float> weights(nrow);
    for (int i = nrow; i--; ) {
        weights[i] = stof(input_data[i][colnames["weight_pooled"]]);
    }


    /* Turn weight vector into sampling distribution for the whole population*/
    RandomVector dd(weights);
    // discrete_distribution<int> dd{weights.begin(), weights.end()};
    // default_random_engine generator;
    mt19937 generator(4949);


    /* Need to create sampling distributions for all 
     * combinations of age, gender, and hhsize

     * The process here is to create tuple keys for all combinations of
     * hhsize, age, gender, in that order
     *
     * Then create a hash of weights, by tuple key. For each 
     * combination of <hhsize, age, gender> initialize a vector of 
     * zeros. If a node is of the correct combination, put it's weight
     * in the corresponding point in the vector.
     *
     * Then, create sampling distributions from each key combination.
     */

    /* Tuple for keys*/ 
    typedef tuple <int,string,string> key;
    map<key, vector<int>> eligible_nodes;

    /* Loop through each node; append node id to the corresponding
     * key in the hash
     */
    for (int i = nrow; i--; ) {
        /* Create key */
        key k = key(
                stoi(input_data[i][colnames["hhsize"]]),
                input_data[i][colnames["age"]],
                input_data[i][colnames["gender"]]);


        eligible_nodes[k].push_back(i);
    }



    /* Create a hash of the distributions */
    map<key, RandomVector> hh_distn;

    /* Loop through )ashes and turn into a sampling distn */
    for (const auto& [k, v]: eligible_nodes) {
        weights.clear();

        for (int i: v) {
            weights.push_back(stof(input_data[i][colnames["weight_pooled"]]));
        }

        hh_distn[k] = RandomVector(eligible_nodes[k], weights);

    }


    /* Need to do the same for POLYMOD */
    auto [POLYMOD_colnames, POLYMOD_data] = load_POLYMOD();
    int nrow_POLYMOD = POLYMOD_data.size(); 

    /* Create lookups for distributions */ 
    map<key, vector<int>> eligible_POLYMOD;
    map<key, RandomVector> distributions_POLYMOD;

    for (int i = nrow_POLYMOD; i--;  ) {
        /* Create key */
        key k = key(
                stoi(POLYMOD_data[i][POLYMOD_colnames["hh_size"]]),
                POLYMOD_data[i][POLYMOD_colnames["part_age"]],
                POLYMOD_data[i][POLYMOD_colnames["part_gender"]]);

        eligible_POLYMOD[k].push_back(i);
    }


    /* Convert to distributions */ 


    for (const auto& [k, v]: eligible_POLYMOD) {
        weights.clear();

        for (int i: v) {
            // Unknown weights for POLYMOD; sample equally
            weights.push_back(1);
        }

        distributions_POLYMOD[k] = RandomVector(eligible_POLYMOD[k], weights);

        }




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
        int hhsize = stoi(input_data[hhead][colnames["hhsize"]]); 
        // cout << hhsize << endl;
        string hhid = randstring(16);

        // cout << "Adding respondent " << hhead << " as head of hhid "<< hhid << endl;

        add_vertex(g,
                input_data[hhead][colnames["age"]],
                input_data[hhead][colnames["gender"]],
                input_data[hhead][colnames["ethnicity"]],
                stoi(input_data[hhead][colnames["num_cc_nonhh"]]), 
                hhid) ;

        // Sample who matches that 
        for (int j = 1; j < min(5,hhsize); j++) {

            string hhmember_age = input_data[hhead][colnames["resp_hh_roster#1_" + to_string(j) + "_1"]];
            string hhmember_gender = input_data[hhead][colnames["resp_hh_roster#2_" + to_string(j)]];

            if (hhmember_age == "") {/*cout<<"Not enough info on household member" << endl; */continue;}


            // Create  vector of weights; set weight to 0 if doesn't match
            key k = key(hhsize, hhmember_age, hhmember_gender);
            //cout << get<0>(k) << "  " << get<1>(k) << "  " <<  get<2>(k) << endl;


            // Check if key exists in map
            if (!hh_distn.count(k) & fill_polymod) {

                // Check if the key is in the POLYMOD distributions m.find("f") == m.end()

                if (distributions_POLYMOD.find(k) == distributions_POLYMOD.end()) {
                    //cout << "Corresponding person not found in POLYMOD" << endl;

                    continue;

                }

                int pmod_member = distributions_POLYMOD[k](generator); 

                // cout << "Adding POLYMOD respondent " << pmod_member << " to hhid "<< hhid << endl;
                add_vertex(g,
                        POLYMOD_data[pmod_member][POLYMOD_colnames["part_age"]],
                        POLYMOD_data[pmod_member][POLYMOD_colnames["part_gender"]],
                        "NA", // POLYMOD_data[i][POLYMOD_colnames["ethnicity"]],
                        stoi(POLYMOD_data[pmod_member][POLYMOD_colnames["num_cc_nonhh"]]), 
                        hhid) ;


                continue;
            } 


            // Sample a corresponding person
            int hh_member = hh_distn[k](generator) ;
            //cout << "Adding respondent " << hh_member << " to hhid "<< hhid << endl;

            add_vertex(g,
                    input_data[hh_member][colnames["age"]],
                    input_data[hh_member][colnames["gender"]],
                    input_data[hh_member][colnames["ethnicity"]],
                    stoi(input_data[hh_member][colnames["num_cc_nonhh"]]), 
                    hhid) ;

        }

    }


}



