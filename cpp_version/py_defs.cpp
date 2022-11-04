#include "BICS_ABM.h"
#include <stdio.h>

extern "C" void hello_world() {
    cout << "Hello world!" << endl;
}

extern "C" void BICS_ABM() {
    Data data;
    Params params;

    BICS_ABM(&data, &params);

}
