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

// vector<Data> data = {Data(0), Data(1), Data(2), Data(3), Data(4), Data(5), Data(5)};

Database database;


Data::Data(int wave) {
    load_BICS(wave);
    cout << "Loaded BICS" << endl;
    load_POLYMOD();
    cout << "Loaded POLYMOD" << endl;
    create_sampling_distns();
    cout << "Created sampling distn" << endl;
}

void Data::create_sampling_distns() {
    bool cached = false;

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

    vector<float> weights;
    // discrete_distribution<int> dd{weights.begin(), weights.end()};
    // default_random_engine generator;


    /* Loop through each node; append node id to the corresponding
     * key in the hash
     */

    for (int i = BICS_nrow; i--; ) {
        /* Create key */


        key k = key(
                stoi(BICS_data[i][BICS_colnames["hhsize"]]),
                BICS_data[i][BICS_colnames["age"]],
                BICS_data[i][BICS_colnames["gender"]]);

        eligible_nodes[k].push_back(i);
    }



    /* Create a hash of the distributions */

    /* Loop through hashes and turn into a sampling distn */
    for (const auto& [k, v]: eligible_nodes) {
        weights.clear();

        for (int i: v) {
            weights.push_back(stof(BICS_data[i][BICS_colnames["weight_pooled"]]));
        }

        hh_distn[k] = RandomVector(eligible_nodes[k], weights);

    }


    /* Need to do the same for POLYMOD */

    /* Create lookups for distributions */ 

    for (int i = POLYMOD_nrow; i--;  ) {
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



    // return make_tuple(dd, vax_vec, hh_distn, eligible_POLYMOD, distributions_POLYMOD);

}


void Data::load_BICS(int wave, string path) {


    /* Create pointer to input files */
    ifstream fin; 
    fin.open(path);

    /* Return object as vector of vectors */
    // vector<vector<string> >  BICS_data;

    /* Get the header */
    vector<string> header = get_csv_row(fin);

    /* Invert header into unordered map for column lookup */
    // unordered_map<string, int> BICS_colnames;
    for (int i = header.size(); i--;) {
        BICS_colnames[header[i]] = i;
    }

    /* Get each row and append it */
    BICS_nrow = 0;
    vector<string> row;

    while (fin.peek() != EOF) {
        row = get_csv_row(fin);

        /* Filter out any NAs */
        if (  (row[BICS_colnames["hhsize"]] == "NA") | 
              (row[BICS_colnames["age"]] == "NA") | 
              (row[BICS_colnames["gender"]] == "NA")  
           ) continue; 

        /* Recode age to string */ 
        row[BICS_colnames["age"]] = recode_age(row[BICS_colnames["age"]]);

        /* Filter by wave */ 
        if (stoi(row[BICS_colnames["wave"]] ) != wave)  continue;

        BICS_data.push_back(row);

        BICS_nrow++;
    }

    cout << BICS_nrow << " BICS respondents read" << endl;

    /* Create vector of weights */
    // vector<float> BICS_weights(BICS_nrow);
    BICS_weights.resize(BICS_nrow);
    for (int i = BICS_nrow; i--; ) {
        BICS_weights[i] = stof(BICS_data[i][BICS_colnames["weight_pooled"]]);
    }


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

void Data::load_POLYMOD(string path) {

    unordered_map<string, int> polymod_participants_colnames;
    vector<vector<string> > polymod_participants;
    ifstream  polymod_f;
    ofstream fout; 


    /*----------------------------------------------
     *
     * Load participants file
     *
     *---------------------------------------------*/

    int n_polymod = 0;
    polymod_f.open(path + "2008_Mossong_POLYMOD_participant_common.csv");
    // cout << "participant_common opened successfully? " << polymod_f.is_open() << endl;

    /* 
     * Get header from polymod_participants and loop through to 
     * create a hash 
     * */

    vector<string> header = get_csv_row(polymod_f);

    for (int i = 0; i < header.size(); i++) {
        // cout << header[i] << endl;
        polymod_participants_colnames[header[i]] = i;
    }

    /* 
     * Read in the participants file 
     * */ 

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

    cout << "size of polymod participants" << polymod_participants.size() << endl;




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

    
    POLYMOD_colnames = polymod_participants_colnames;
    POLYMOD_data = polymod_participants;
    POLYMOD_nrow = polymod_participants.size();

    cout << POLYMOD_nrow << " POLYMOD respondents read" << endl;

}
