import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from BICS_ABM import BICS_ABM, VaccineRule
import time
from datetime import date
from joblib import Parallel, delayed
from scipy.stats.qmc import LatinHypercube
from scipy.stats import norm, uniform, randint
import gc
import json
import string
import random
import os


nsims = 10
sampler = LatinHypercube(16)
sample = sampler.random(nsims)

gamma_min_v = [int(randint.ppf(x, 0*24, 2*24)) for x in sample[:,0]] 
gamma_max_v = [int(randint.ppf(x, 2*24, 4*24)) for x in sample[:,1]] 
sigma_min_v = [int(randint.ppf(x, 3*24, 5*24)) for x in sample[:,2]] 
sigma_max_v = [int(randint.ppf(x, 5*24, 7*24)) for x in sample[:,3]] 
beta0_v = [uniform.ppf(x, 0, .10) for x in sample[:,4]] 
alpha_v = [uniform.ppf(x, 0, .25) for x in sample[:,5]] 
rho_v = [uniform.ppf(x, .7, 1) for x in sample[:,6]] 
isomult_v = [uniform.ppf(x, 0, 1) for x in sample[:,7]] 
npi_v = [uniform.ppf(x, 0.25, 0.75) for x in sample[:,8]]
t_reinf_v = [int(uniform.ppf(x, 180, 720) ) for x in sample[:, 9]]

"""
mu0_v = [uniform.ppf(x, 0.003, 0.005)/100 for x in sample[:,9]] 
mu1_v = [uniform.ppf(x, 0.003, 0.005)/100 for x in sample[:,10]] 
mu2_v = [uniform.ppf(x, 0.058, 0.078)/100 for x in sample[:,11]] 
mu3_v = [uniform.ppf(x, 0.20, 0.26)/100 for x in sample[:,12]] 
mu4_v = [uniform.ppf(x, 0.66, 0.87)/100 for x in sample[:,13]] 
mu5_v = [uniform.ppf(x, 0.66, 0.87)/100 for x in sample[:,14]] 
mu6_v = [uniform.ppf(x, 2.1, 3.0)/100 for x in sample[:,15]] 
mu7_v = [uniform.ppf(x, 6.9, 10.4)/100 for x in sample[:,16]] 
mu8_v = [uniform.ppf(x, 21.8, 36.6)/100 for x in sample[:,17]] 
"""

ve1_v = [uniform.ppf(x, 0.65, 0.75) for x in sample[:,10]]
ve2_v = [uniform.ppf(x, 0.85, 0.95) for x in sample[:,11]]
vew_v = [uniform.ppf(x, 0.25, 0.55) for x in sample[:,12]]
veb_v = [uniform.ppf(x, 0.65, 0.75) for x in sample[:,13]]

vu_v = [uniform.ppf(x, 0, 1) for x in sample[:, 14]]

t0_v = [int(uniform.ppf(x, 0, 364) ) for x in sample[:,15]]


params_v = [{
    "GAMMA_MIN": gamma_min_v[i],
    "GAMMA_MAX": gamma_max_v[i],
    "SIGMA_MIN": sigma_min_v[i],
    "SIGMA_MAX": sigma_max_v[i],
    "BETA0": beta0_v[i],
    "ALPHA": alpha_v[i],
    "RHO": rho_v[i],
    "ISOLATION_MULTIPLIER": isomult_v[i],
    "NPI": npi_v[i],
    "VE1": ve1_v[i],
    "VE2": ve2_v[i],
    "VEW": vew_v[i],
    "VEBoost": veb_v[i],
    "T0": t0_v[i],
    "T_REINFECTION": t_reinf_v[i],
    "vu": vu_v[i],
    "MAX_DAYS": 5*365
    } for i in range(nsims) ]

t = time.time()

# Generate all 'treatments'
b1_range = [0,.5]
cm_range = [0,.5]
sc_range = [True, False]


tm = [{"BETA1": b1, 
        "CONTACT_MULT_VEC": [max(0, 1+cm*np.cos(t*2*np.pi/365)) for t in range(365)], 
        "cm": cm,
        "SCHOOL_CONTACTS": False if cm == 0 else True} 
        for b1 in b1_range for cm in cm_range]

def id_generator(size=6, chars=string.ascii_uppercase + string.digits):
    return ''.join(random.choice(chars) for _ in range(size))

path = "sims/" + date.today().strftime("%y_%m_%d") + "/"

if not os.path.exists(path):
    os.mkdir(path)


def sim_fn(i: int):
    
    for j, t in enumerate(tm):
        print("Sim", i, "of", nsims, "treatment", j, "of", 4)
        p = params_v[i].copy()
        sim_id = id_generator(12)
        p.update(t)

        ## Save the sim parameters
        with open(path + sim_id + "_params.json", 'w') as f:
            json.dump(p, f)

        vu = p["vu"]
        del p["vu"]
        del p["cm"]

        result = BICS_ABM(**p, vax_rules = [VaccineRule(general=True, hesitancy = vu)], silent = True)

        ## Save the outbreak curve
        with open(path + sim_id + "_Ic.json", 'w') as f:
            json.dump(result.Ic, f)

    return


if __name__ == '__main__':
    print("Starting sims at", time.time())
    result_vec = Parallel(n_jobs=4, verbose = 10, prefer="threads"
            )(delayed(sim_fn)(v) for v in range(nsims))

