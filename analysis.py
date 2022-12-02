from BICS_ABM import BICS_ABM, VaccineRule

result = BICS_ABM(silent = True)


result = BICS_ABM( 
        N_HH = 10000,
        SEED = 4949, 
        RHO = 0.9,
        ALPHA = 0.25,
        BETA = 0.01,
        T0 = 60,
        ISOLATION_MULTIPLIER = 0.1, 
        N_VAX_DAILY = 1500,
        vax_rules = [
            VaccineRule("age > 80"),
            VaccineRule("age > 70"),
            VaccineRule("age > 60"),
            VaccineRule("age > 50", hesitancy = 0.5),
            VaccineRule("age > 40", hesitancy = 0.5),
            VaccineRule("age > 30", hesitancy = 0.5),
            VaccineRule(general = True, hesitancy = 0.5),
        ])

result.plot_trajectory()


# Just as a quick test: vary Beta from 0 to 1 in increments of 0.05 and plot
import matplotlib.pyplot as plt 
import numpy as np
import random 

beta_range = np.arange(0, .25, 0.01)
deaths = list() 

"""
for beta in beta_range:
    tmp_list = list()
    for j in range(5):
        print(j + 49, j + 4949)
        ABM = BICS_ABM(
            INDEX_CASES = 1,
            N_HH = 1000, 
            BETA=beta,
            SEED = 49+j,
            POP_SEED = 4949 + j
        )
        abm_d = ABM.D[-1]
        abm_totpop = ABM.S[0] + ABM.E[0]

        tmp_list.append(abm_d/abm_totpop)

    deaths.append(np.mean(tmp_list))


plt.scatter(beta_range, deaths)
plt.show()
"""
