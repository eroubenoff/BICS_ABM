#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <igraph.h>
#include <random>
#include "defs.h"
using namespace std;


/*
 * Vector to float. Assumes that vector is defined with 
 * hard brackets [] and commas, otherwise will fail.
 *
 */

vector<float> stovf(string s) {
    string word;
    vector<float> vec;

    for (int i = 0; i < s.size(); i++) {
        if (s[i] == '[') {
            continue;
        }
        else if (s[i] == ']') {
            vec.push_back(stof(word));
            return vec;
        }
        else if (s[i] == ',') {
            vec.push_back(stof(word));
        }
        else {
            word += s[i];
        }

    }

    if (word != "" ) vec.push_back(stof(word));

    return vec;

}
