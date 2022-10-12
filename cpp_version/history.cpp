/*
 * Functions to save and plot history. Part of a 'history' object
 */

#include<vector>
#include<string>
#include<iostream>
#include<fstream>
#include "matplotlib.h"
#include "defs.h"
using namespace std;
namespace plt = matplotlibcpp;

History::History(int initial_length) {
    length = initial_length;
    counter = 0;
    S.resize(length, -1);
    E.resize(length, -1);
    I.resize(length, -1);
    R.resize(length, -1);
    D.resize(length, -1);
    V1.resize(length, -1);
    V2.resize(length, -1);
}


void History::add_history(int _S, int _E, int _I, int _R, int _D, int _V1, int _V2) {
    if (counter == length-1) {
        length += 500;
        S.resize(length, -1);
        E.resize(length, -1);
        I.resize(length, -1);
        R.resize(length, -1);
        D.resize(length, -1);
        V1.resize(length, -1);
        V2.resize(length, -1);
    }

    counter++; 

    S[counter] = _S;
    E[counter] = _E;
    I[counter] = _I;
    R[counter] = _R;
    D[counter] = _D;
    V1[counter] = _V1;
    V2[counter] = _V2;

}

void History::save(string path) {

    ofstream fout; 
    fout.open(path);

    fout << "S,E,I,R,D,V1,V2" << endl;

    for (int i = 0; i < counter + 1; i++) {
        
        fout << S[i] << ",";
        fout << E[i] << ",";
        fout << I[i] << ",";
        fout << R[i] << ",";
        fout << D[i] << ",";
        fout << V1[i] << ",";
        fout << V2[i] << ",";

        fout << endl;

    } 
        
}
