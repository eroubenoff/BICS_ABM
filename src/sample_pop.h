Params sample_params;
int sample_pop_size = 362;
int sample_pop_cols = 8;
double sample_pop[362 * 8] = {
         0.,  0.,  0.,  0.,  1.,  1.,  1.,  2.,  2.,  2.,  3.,  3.,  3.,
        4.,  4.,  4.,  4.,  4.,  5.,  6.,  6.,  6.,  6.,  7.,  7.,  7.,
        7.,  8.,  8.,  8.,  8.,  8.,  8.,  9., 10., 10., 10., 11., 11.,
       11., 11., 11., 12., 12., 12., 12., 12., 13., 13., 13., 14., 14.,
       14., 14., 14., 15., 16., 16., 16., 16., 16., 17., 17., 17., 17.,
       18., 18., 18., 18., 18., 18., 19., 19., 19., 20., 20., 20., 21.,
       21., 21., 21., 22., 22., 22., 22., 22., 23., 23., 23., 23., 24.,
       24., 24., 25., 25., 25., 26., 26., 26., 26., 27., 27., 27., 28.,
       28., 28., 29., 30., 30., 30., 30., 30., 31., 31., 31., 31., 32.,
       33., 33., 33., 33., 33., 33., 34., 34., 34., 35., 35., 35., 35.,
       35., 36., 37., 37., 37., 37., 37., 37., 38., 38., 38., 39., 39.,
       39., 39., 40., 40., 40., 40., 40., 41., 41., 41., 41., 41., 42.,
       42., 42., 42., 42., 43., 44., 45., 46., 47., 48., 48., 48., 49.,
       49., 49., 49., 49., 50., 50., 50., 50., 51., 51., 51., 51., 51.,
       52., 52., 52., 52., 52., 53., 53., 53., 53., 53., 53., 54., 54.,
       54., 55., 55., 55., 56., 56., 56., 57., 57., 57., 57., 58., 58.,
       58., 59., 59., 59., 60., 60., 60., 60., 60., 60., 61., 62., 62.,
       62., 62., 63., 63., 63., 64., 64., 64., 64., 65., 66., 67., 67.,
       67., 68., 68., 68., 68., 69., 70., 70., 70., 70., 71., 71., 71.,
       72., 72., 72., 73., 73., 73., 73., 73., 73., 74., 74., 74., 74.,
       74., 75., 76., 76., 76., 77., 77., 77., 78., 78., 78., 78., 78.,
       78., 79., 79., 79., 80., 80., 80., 81., 81., 81., 81., 81., 81.,
       82., 82., 82., 83., 83., 83., 83., 84., 84., 84., 84., 84., 85.,
       85., 85., 85., 85., 85., 86., 86., 86., 87., 88., 88., 88., 89.,
       89., 89., 89., 89., 89., 90., 90., 90., 90., 90., 91., 91., 91.,
       91., 91., 92., 92., 92., 93., 93., 93., 93., 94., 94., 94., 94.,
       94., 95., 95., 95., 95., 96., 96., 96., 96., 96., 96., 97., 97.,
       97., 98., 98., 98., 98., 98., 99., 99., 99., 99., 99.,
       5., 5., 2., 5., 5., 6., 2., 1., 2., 5., 5., 6., 3., 3., 5., 0., 0.,
       3., 3., 3., 0., 0., 2., 3., 6., 6., 3., 2., 0., 0., 0., 0., 5., 5.,
       5., 5., 3., 4., 3., 0., 0., 3., 3., 3., 0., 0., 1., 6., 7., 6., 1.,
       0., 4., 3., 1., 3., 3., 6., 3., 0., 3., 5., 2., 5., 3., 2., 0., 0.,
       2., 0., 2., 7., 7., 1., 2., 2., 1., 2., 2., 3., 2., 4., 4., 1., 2.,
       3., 5., 0., 0., 4., 7., 7., 5., 4., 4., 3., 5., 6., 2., 3., 5., 5.,
       3., 6., 0., 4., 7., 5., 4., 7., 7., 4., 6., 6., 2., 2., 5., 3., 3.,
       3., 5., 7., 2., 6., 5., 2., 2., 6., 6., 3., 4., 3., 2., 0., 0., 0.,
       0., 3., 6., 3., 4., 2., 2., 2., 1., 3., 2., 0., 0., 1., 3., 3., 0.,
       0., 7., 1., 2., 0., 0., 2., 2., 6., 1., 6., 5., 3., 3., 2., 2., 0.,
       7., 6., 5., 2., 3., 0., 4., 1., 2., 0., 0., 5., 1., 2., 4., 4., 3.,
       3., 0., 0., 0., 0., 6., 5., 0., 3., 1., 1., 1., 3., 2., 5., 4., 4.,
       1., 3., 6., 5., 1., 6., 6., 3., 4., 3., 2., 0., 0., 3., 2., 2., 0.,
       2., 6., 6., 5., 1., 6., 3., 3., 6., 3., 6., 5., 4., 5., 2., 3., 0.,
       1., 7., 3., 2., 0., 6., 6., 5., 4., 6., 3., 1., 3., 3., 1., 0., 0.,
       0., 2., 3., 5., 7., 1., 6., 3., 3., 6., 6., 7., 4., 1., 5., 0., 1.,
       0., 0., 4., 6., 3., 7., 7., 7., 1., 0., 1., 2., 0., 1., 5., 7., 2.,
       7., 5., 5., 4., 5., 4., 7., 7., 2., 1., 1., 2., 2., 0., 2., 1., 0.,
       1., 6., 6., 7., 3., 3., 3., 0., 0., 0., 0., 3., 3., 0., 0., 4., 4.,
       4., 1., 2., 5., 5., 5., 3., 1., 1., 0., 5., 3., 3., 0., 0., 2., 4.,
       1., 4., 5., 3., 3., 2., 0., 0., 0., 5., 5., 2., 2., 2., 0., 0., 3.,
       5., 5., 1., 2., 5.,
       1., 0., 1., 1., 1., 0., 1., 1., 0., 0., 0., 1., 0., 1., 0., 1., 0.,
       1., 1., 0., 0., 1., 1., 1., 1., 0., 0., 0., 0., 0., 1., 0., 1., 1.,
       0., 1., 0., 0., 1., 0., 0., 1., 1., 0., 1., 0., 1., 1., 0., 0., 0.,
       0., 0., 1., 0., 1., 0., 1., 0., 1., 0., 1., 0., 0., 1., 0., 0., 0.,
       1., 1., 0., 0., 1., 1., 1., 1., 1., 0., 1., 0., 1., 0., 1., 1., 0.,
       0., 1., 1., 1., 0., 0., 1., 0., 1., 1., 1., 1., 0., 1., 1., 1., 0.,
       0., 0., 1., 0., 1., 1., 1., 1., 0., 1., 0., 1., 0., 0., 0., 0., 0.,
       1., 1., 1., 1., 1., 0., 1., 1., 0., 1., 1., 1., 0., 0., 0., 1., 1.,
       0., 1., 0., 0., 0., 1., 0., 1., 1., 0., 0., 1., 1., 1., 0., 1., 0.,
       1., 0., 1., 0., 0., 1., 1., 1., 0., 0., 1., 1., 0., 1., 1., 1., 1.,
       0., 1., 1., 1., 0., 1., 1., 1., 0., 0., 0., 1., 0., 0., 0., 1., 0.,
       1., 1., 1., 0., 1., 0., 1., 1., 0., 0., 0., 0., 1., 0., 0., 1., 0.,
       0., 1., 0., 1., 1., 1., 0., 0., 0., 1., 0., 1., 0., 0., 1., 1., 1.,
       0., 1., 1., 0., 1., 1., 1., 0., 0., 1., 1., 0., 1., 1., 1., 0., 1.,
       1., 1., 0., 1., 0., 0., 0., 1., 0., 0., 0., 0., 0., 1., 0., 1., 1.,
       0., 1., 0., 1., 0., 1., 1., 0., 1., 0., 1., 0., 1., 0., 1., 1., 1.,
       1., 1., 1., 1., 1., 0., 1., 1., 0., 0., 0., 1., 1., 1., 1., 0., 0.,
       1., 0., 0., 1., 1., 1., 1., 0., 1., 1., 1., 0., 1., 1., 1., 0., 0.,
       0., 0., 1., 0., 0., 1., 0., 0., 1., 0., 1., 0., 1., 1., 1., 1., 0.,
       1., 1., 0., 1., 1., 0., 1., 1., 0., 0., 1., 0., 0., 1., 0., 0., 1.,
       1., 0., 1., 0., 0., 1., 0., 1., 1., 1., 0., 0., 0., 0., 1., 0., 1.,
       1., 0., 0., 0., 1.,
        0.,  3.,  2.,  3., 15.,  0.,  2.,  1.,  2.,  9.,  5.,  1.,  1.,
        0.,  2.,  9.,  6.,  4.,  0.,  4., 11., 11.,  0.,  0.,  3.,  1.,
        4.,  5.,  2., 11., 14.,  9.,  2.,  2.,  0.,  1.,  1., 12.,  1.,
       20.,  5.,  5.,  5., 25.,  6.,  5.,  2.,  3., 12.,  2.,  0., 12.,
       10.,  0.,  0., 15.,  2.,  4.,  2.,  3.,  1.,  0.,  2.,  7.,  3.,
        2., 13., 17.,  2., 12.,  3.,  0.,  5.,  0.,  0.,  2.,  3., 25.,
        0.,  0.,  1.,  2.,  0.,  3.,  0.,  2.,  5., 20., 10.,  0.,  0.,
        2., 10.,  1., 12.,  0.,  3.,  0.,  0., 99.,  0.,  1., 13.,  5.,
        8.,  2.,  0.,  0.,  2., 15.,  1.,  0.,  4.,  0.,  0.,  2.,  1.,
        3., 30., 65.,  1.,  5.,  2., 10.,  0.,  0.,  0.,  1.,  0., 13.,
        1.,  1.,  0.,  1.,  2., 25.,  7.,  1.,  0.,  0.,  2.,  6.,  0.,
       10.,  1., 40., 10., 17.,  2.,  1.,  2., 60., 13., 14.,  4.,  0.,
        0., 12.,  8.,  0.,  3.,  1.,  1.,  2.,  3.,  5.,  1.,  1., 40.,
        3.,  1.,  4.,  3.,  0.,  2., 39.,  5.,  0.,  2., 32., 11.,  2.,
        1., 10.,  1.,  1.,  2., 17.,  8.,  9.,  9.,  4.,  3.,  2., 10.,
        5.,  1.,  1.,  4.,  5., 22.,  1.,  3.,  4.,  1.,  1.,  3.,  0.,
        0.,  4.,  0.,  1.,  3.,  0.,  3.,  6.,  9.,  2., 16.,  3., 10.,
        3.,  1., 10.,  5.,  0.,  0.,  0.,  1.,  2.,  1.,  2.,  1.,  5.,
        0.,  0.,  2., 17.,  6.,  3.,  2.,  1., 11., 20.,  2.,  0.,  1.,
        0., 10., 36.,  2., 11., 34., 14., 10.,  4.,  0.,  8., 20.,  4.,
        1.,  4., 20., 10.,  0.,  0.,  0.,  2.,  9.,  0., 12., 20.,  9.,
        3.,  5.,  0.,  1.,  5.,  7.,  4.,  0., 28.,  0., 10.,  5., 60.,
        1.,  1.,  1.,  0.,  3., 12.,  0.,  0.,  1.,  0.,  4.,  3.,  0.,
        2., 60.,  0., 15.,  0.,  0.,  9.,  3.,  1.,  0.,  0.,  3.,  2.,
        3.,  7.,  4.,  2.,  6., 10.,  6., 11.,  5.,  4.,  2., 20.,  2.,
        2.,  2.,  1.,  0.,  4.,  3., 34.,  6.,  0.,  1.,  1., 22.,  6.,
        1., 30.,  0.,  3., 10.,  5.,  6.,  0.,  0., 17.,  5., 10.,  4.,
       10.,  0.,  2.,  6.,  8.,  4.,  6.,  1., 49.,  2., 68.,
           0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  5.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  1.,  0., 18.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
       10.,  0.,  0.,  0.,  0.,  8.,  0.,  0.,  0.,  0.,  0.,  0., 12.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0., 16., 19.,  0.,  1.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0., 10.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
       14.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0., 14., 13.,  5.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  2.,  0.,  0.,  0.,  1.,  9.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        1.,  0.,  0.,  0.,  0.,  0., 11.,  0.,  0.,  0., 15., 11.,  0.,
        0.,  0.,  0.,  0.,  0.,  0., 11.,  2.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0., 20.,  7.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0., 12.,  0.,  0.,  0.,  0., 14.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0., 26.,  4.,  3.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  6.,  0.,  0., 27.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  4.,  0.,  9.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  7.,  0.,  0.,  0.,  0., 15.,
        0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  6.,  0.,  0.,  0.,  0.,
        0.,  0.,  0.,  0.,  4.,  0.,  0.,  0.,  0.,  0.,  0.,
        2., 1., 2., 0., 1., 1., 1., 2., 1., 2., 3., 0., 5., 2., 4., 1., 1.,
       2., 0., 3., 1., 1., 2., 0., 1., 1., 2., 6., 1., 1., 1., 1., 1., 2.,
       0., 1., 3., 2., 1., 1., 1., 0., 1., 3., 1., 1., 1., 1., 2., 1., 1.,
       1., 3., 0., 1., 1., 2., 0., 0., 1., 2., 0., 1., 3., 3., 2., 1., 1.,
       2., 1., 5., 0., 2., 5., 0., 2., 2., 2., 0., 1., 2., 1., 1., 3., 1.,
       0., 3., 1., 1., 1., 1., 2., 1., 2., 3., 0., 1., 0., 0., 2., 2., 3.,
       3., 2., 1., 2., 1., 0., 1., 1., 1., 1., 2., 1., 2., 2., 1., 2., 4.,
       1., 4., 2., 2., 1., 0., 0., 1., 3., 0., 4., 1., 1., 1., 1., 1., 1.,
       1., 2., 1., 2., 1., 3., 1., 4., 2., 4., 2., 1., 1., 1., 3., 1., 1.,
       1., 2., 2., 3., 1., 1., 0., 1., 1., 3., 2., 6., 2., 1., 1., 1., 1.,
       2., 1., 1., 1., 4., 1., 3., 3., 1., 1., 1., 3., 0., 2., 0., 4., 3.,
       4., 1., 1., 1., 1., 1., 2., 1., 2., 2., 3., 3., 2., 2., 3., 1., 2.,
       1., 6., 2., 0., 1., 0., 2., 1., 3., 1., 2., 1., 1., 6., 5., 2., 1.,
       2., 4., 1., 3., 0., 0., 3., 5., 3., 1., 1., 0., 3., 3., 1., 4., 1.,
       2., 1., 2., 1., 1., 2., 0., 0., 2., 1., 2., 2., 4., 2., 2., 1., 1.,
       1., 0., 6., 5., 1., 0., 4., 6., 2., 2., 0., 1., 2., 4., 3., 1., 2.,
       1., 1., 1., 1., 3., 1., 4., 0., 1., 1., 0., 2., 1., 2., 1., 2., 2.,
       1., 4., 3., 2., 0., 1., 0., 2., 2., 1., 1., 2., 2., 1., 1., 1., 1.,
       5., 1., 0., 2., 2., 2., 2., 1., 1., 1., 1., 3., 1., 1., 1., 2., 1.,
       1., 1., 3., 0., 1., 0., 2., 1., 2., 1., 0., 1., 4., 1., 1., 1., 1.,
       0., 3., 0., 2., 3., 0., 1., 1., 1., 3., 2., 2., 0., 3., 1., 1., 2.,
       4., 2., 1., 3., 2.,
       -1.,  0., -1., -1.,  0., -1., -1., -1., -1.,  0., -1.,  0., -1.,
       -1., -1., -1.,  0.,  0., -1., -1.,  0.,  0., -1., -1.,  0.,  0.,
        0., -1., -1.,  0., -1., -1., -1.,  0.,  0.,  0., -1., -1., -1.,
        0., -1., -1.,  0., -1.,  0., -1., -1., -1.,  0., -1.,  0., -1.,
        0.,  0., -1.,  0., -1.,  0., -1., -1.,  0.,  0., -1.,  0., -1.,
        0., -1., -1., -1.,  0., -1., -1.,  0.,  0.,  0., -1.,  0., -1.,
       -1.,  0.,  0.,  0., -1., -1., -1., -1., -1., -1., -1., -1., -1.,
       -1., -1., -1.,  0., -1.,  0.,  0., -1.,  0.,  0.,  0., -1.,  0.,
        0., -1., -1.,  0.,  0., -1., -1., -1.,  0.,  0.,  0., -1.,  0.,
        0., -1.,  0., -1.,  0.,  0.,  0., -1., -1.,  0.,  0., -1., -1.,
        0.,  0., -1.,  0., -1.,  0., -1., -1., -1.,  0.,  0., -1.,  0.,
        0., -1.,  0.,  0., -1.,  0.,  0., -1., -1.,  0., -1.,  0., -1.,
        0., -1., -1., -1., -1.,  0., -1.,  0., -1., -1., -1.,  0.,  0.,
       -1., -1.,  0.,  0., -1.,  0., -1.,  0.,  0., -1., -1.,  0.,  0.,
       -1.,  0.,  0., -1.,  0.,  0., -1.,  0., -1.,  0.,  0.,  0.,  0.,
       -1., -1., -1.,  0.,  0., -1.,  0.,  0., -1., -1.,  0., -1., -1.,
        0., -1., -1., -1., -1., -1., -1., -1.,  0.,  0.,  0., -1., -1.,
       -1.,  0.,  0., -1., -1.,  0., -1., -1.,  0., -1., -1., -1., -1.,
       -1., -1., -1.,  0.,  0.,  0.,  0.,  0., -1., -1., -1.,  0.,  0.,
       -1., -1.,  0.,  0., -1., -1.,  0., -1.,  0.,  0.,  0.,  0.,  0.,
        0.,  0., -1., -1.,  0., -1.,  0., -1., -1., -1., -1., -1.,  0.,
        0., -1., -1., -1., -1.,  0.,  0.,  0.,  0., -1., -1., -1., -1.,
        0.,  0.,  0.,  0., -1., -1., -1.,  0.,  0.,  0.,  0.,  0.,  0.,
        0.,  0., -1.,  0.,  0., -1.,  0.,  0.,  0., -1., -1.,  0., -1.,
        0., -1.,  0.,  0., -1., -1., -1., -1., -1.,  0.,  0.,  0.,  0.,
        0., -1., -1.,  0.,  0., -1., -1., -1.,  0., -1.,  0.,  0.,  0.,
        0., -1.,  0., -1.,  0., -1.,  0.,  0., -1.,  0.,  0.,  0.,  0.,
        0., -1., -1.,  0., -1.,  0.,  0.,  0.,  0., -1.,  0.,
        0., 1., 1., 0., 0., 0., 0., 0., 1., 1., 0., 0., 1., 0., 0., 0., 0.,
       1., 0., 1., 0., 0., 0., 0., 0., 1., 1., 1., 0., 0., 0., 0., 1., 0.,
       0., 1., 1., 1., 1., 0., 0., 0., 1., 0., 0., 0., 1., 1., 0., 1., 0.,
       0., 1., 0., 0., 1., 1., 1., 1., 0., 1., 0., 1., 1., 1., 1., 0., 0.,
       1., 0., 1., 0., 1., 0., 0., 1., 1., 1., 0., 0., 0., 1., 0., 1., 0.,
       0., 1., 0., 0., 0., 0., 1., 1., 1., 1., 0., 1., 0., 0., 1., 0., 1.,
       1., 1., 0., 1., 0., 0., 1., 1., 1., 0., 1., 0., 0., 1., 0., 1., 1.,
       1., 1., 1., 1., 1., 0., 0., 0., 0., 0., 1., 1., 1., 0., 0., 0., 0.,
       0., 0., 0., 0., 1., 1., 0., 1., 1., 1., 1., 0., 0., 1., 1., 1., 0.,
       0., 1., 0., 0., 0., 0., 0., 1., 1., 1., 1., 0., 1., 1., 1., 1., 0.,
       1., 1., 1., 0., 0., 0., 1., 0., 1., 0., 0., 1., 1., 1., 1., 0., 1.,
       1., 0., 0., 0., 0., 1., 0., 0., 1., 1., 1., 1., 1., 1., 1., 1., 1.,
       0., 1., 1., 0., 0., 1., 0., 1., 1., 0., 1., 0., 0., 1., 1., 0., 0.,
       0., 1., 1., 0., 0., 0., 0., 1., 1., 1., 1., 1., 0., 0., 0., 1., 0.,
       1., 1., 1., 1., 0., 0., 1., 0., 0., 0., 0., 1., 1., 1., 1., 0., 0.,
       0., 0., 0., 1., 1., 1., 1., 1., 1., 0., 0., 0., 1., 0., 0., 0., 0.,
       0., 0., 1., 0., 0., 0., 1., 1., 0., 0., 0., 1., 0., 1., 0., 0., 1.,
       0., 1., 1., 0., 0., 1., 0., 1., 1., 0., 1., 0., 0., 0., 0., 0., 0.,
       1., 1., 0., 0., 1., 1., 1., 0., 0., 0., 0., 0., 1., 0., 0., 1., 1.,
       1., 1., 1., 1., 1., 0., 1., 1., 1., 0., 0., 1., 1., 0., 0., 0., 1.,
       0., 1., 1., 1., 0., 0., 0., 0., 0., 1., 1., 1., 0., 1., 0., 0., 1.,
       1., 1., 1., 0., 1.

};
