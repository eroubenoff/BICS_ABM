#include "BICS_ABM.h"
#include <stdio.h>
extern "C" void hello_world() {
    cout << "Hello world!" << endl;
}



struct ABM_result {
    /* Test case: just return the end points */ 
    int S_array[5000];
    int E_array[5000];
    int I_array[5000];
    int R_array[5000];
    int D_array[5000];
    int V1_array[5000];
    int V2_array[5000];
    int counter;

} ;

/* NEED TO WRITE DESTRUCTOR ! */
extern "C" void ABM_result_destroy (ABM_result *r) {
    delete r;
    cout << "Destructor successful at " << r << endl;

}


/* Params */
extern "C" Params* ABM_params_new() {
    return new Params;
}

extern "C" void ABM_params_destroy(Params* params) {
    delete params;
    cout << "Destructor successful on ABM params at" << params << endl;
}


extern "C" struct ABM_result* BICS_ABM() {
    ABM_result *result = new ABM_result; 
    Data data;
    Params params;
    History history;

    BICS_ABM(&data, &params, &history);

    result->counter = history.counter;
    copy(history.S.begin(), history.S.end(), result->S_array);
    copy(history.E.begin(), history.E.end(), result->E_array);
    copy(history.I.begin(), history.I.end(), result->I_array);
    copy(history.R.begin(), history.R.end(), result->R_array);
    copy(history.D.begin(), history.D.end(), result->D_array);
    copy(history.V1.begin(), history.V1.end(), result->V1_array);
    copy(history.V2.begin(), history.V2.end(), result->V2_array);


    return result;

}
