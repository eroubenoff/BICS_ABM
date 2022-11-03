/*
 * Functions to save and plot history. Part of a 'history' object
 */

#include<vector>
#include<string>
#include<iostream>
#include<fstream>
#include "matplotlibcpp.h"
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


    S[counter] = _S;
    E[counter] = _E;
    I[counter] = _I;
    R[counter] = _R;
    D[counter] = _D;
    V1[counter] = _V1;
    V2[counter] = _V2;

    counter++; 
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

void History::plot_trajectory(string path) {

    S.resize(counter);
    E.resize(counter);
    I.resize(counter);
    R.resize(counter);
    D.resize(counter);
    V1.resize(counter);
    V2.resize(counter);

    vector<float> time;
    for (int i = 0; i < counter; i++) {
        time.push_back(i/24.0);
    }

    // Set the size of output image to 1200x780 pixels
    plt::figure_size(1200, 780);
    plt::named_plot("S", time, S);
    plt::named_plot("E", time, E);
    plt::named_plot("I", time, I);
    plt::named_plot("R", time, R);
    plt::named_plot("D", time, D);
    plt::named_plot("V1", time, V1);
    plt::named_plot("V2", time, V2);
    plt::title("Trajectories");
    // Enable legend.
    plt::legend();
    // Save the image (file format is determined by the extension)
    plt::save(path);


}