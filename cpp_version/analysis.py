from BICS_ABM import BICS_ABM

result = BICS_ABM()


result = BICS_ABM( 
        SEED = 1231, 
        POP_SEED = 100,
        vax_rules = [
            [("age", ">85")],
            [("age", "[75,85)")],
            [("age", "[65,75)")],
            [("age", "[55,65)"), ("hesitancy", "0.5")]
        ])


# Just as a quick test: vary Beta from 0 to 1 in increments of 0.05 and plot
import matplotlib.pyplot as plt 
import numpy as np
import random 

beta_range = np.arange(0, .5, 0.01)
deaths = list() 

for beta in beta_range:
    tmp_list = list()
    for j in range(10):
        print(j + 49, j + 4949)
        ABM = BICS_ABM(
            INDEX_CASES = 1,
            N_HH = 100, 
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
