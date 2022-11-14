#include "BICS_ABM.h"
#include <stdio.h>



struct Trajectory {
    int S_array[5000];
    int E_array[5000];
    int I_array[5000];
    int R_array[5000];
    int D_array[5000];
    int V1_array[5000];
    int V2_array[5000];
    int counter;

} ;



/* Treat data as a gloabl */
extern "C" struct Trajectory BICS_ABM(Params params) {
    Trajectory trajectory; 
    History history;

    cout << params.VAX_RULES_COLS << endl;
    cout << params.VAX_RULES_VALS << endl;
    cout << params.VAX_RULES_N << endl;


    BICS_ABM(&::data[params.WAVE], &params, &history);

    trajectory.counter = history.counter;
    copy(history.S.begin(), history.S.end(), trajectory.S_array);
    copy(history.E.begin(), history.E.end(), trajectory.E_array);
    copy(history.I.begin(), history.I.end(), trajectory.I_array);
    copy(history.R.begin(), history.R.end(), trajectory.R_array);
    copy(history.D.begin(), history.D.end(), trajectory.D_array);
    copy(history.V1.begin(), history.V1.end(), trajectory.V1_array);
    copy(history.V2.begin(), history.V2.end(), trajectory.V2_array);


    return trajectory;

}
