#include "BICS_ABM.h"
#include <stdio.h>

extern "C" void hello_world() {
    cout << "Hello world!" << endl;
}

/*
struct ABM_data {

}
*/
extern "C" void BICS_ABM() {
    Data data;
    Params params;
    History history;

    BICS_ABM(&data, &params, &history);

}
