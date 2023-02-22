"""
This file runs the main simulations for analysis. Paths on this script are
specific to keyfitz.demog.berkeley.edu.

To replicate this analysis, you must first install the software as:

> git clone https://github.com/eroubenoff/BICS_ABM.git
> git submodule update --init --recursive
> make

Then, update the paths, and run the program as:

> tmux
> python run_sims.py
"""

import os
from datetime import date
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from BICS_ABM import BICS_ABM, VaccineRule
import time
from joblib import Parallel, delayed
import pickle
import gzip
from scipy.stats import norm, uniform, randint, qmc
from copy import deepcopy
import random



def write_sim(sim, fname):
    fname = os.path.join(sims_path, fname, '.pgz')
    with gzip.GzipFile(fname, 'wb') as f:
        pickle.dump(sim, f)


def read_sim(fname):
    with gzip.GzipFile(fname, 'bb') as f:
        sim = pickle.load(f)

    return sim

def baseline_params():

    return {
        "N_HH" : 1000,
        "WAVE" : 6,
        "GAMMA_MIN" : 2 * 24,
        "GAMMA_MAX" : 4 * 24,
        "SIGMA_MIN" : 4 * 24,
        "SIGMA_MAX" : 6 * 24,
        "MU_VEC" : np.array([0.004, 0.004, 0.004, 0.068, 0.23, 0.75, 2.5, 8.5, 28.3]) / 100,
        "INDEX_CASES" : 1,
        "SEED" : None,
        "N_VAX_DAILY" : int(1000*3/30),
        "IMPORT_CASES_VEC" : [1 if x%7 == 0 else 0 for x in range(365)],
        "VE1" : 0.8,
        "VE2" : 0.9,
        "VEW" : 0.38,
        "VEBOOST" : 0.82,
        "ISOLATION_MULTIPLIER" : 0.5,
        "T0" : 60,
        "ALPHA" : 0.5,
        "RHO" : 0.5,
        "NPI" : 0.5,
        "MAX_DAYS" : 365*5
    }

def create_params(nreps=100):
    random.seed(4949)

    sampler = qmc.LatinHypercube(d=6)
    sample = sampler.random(n=nreps)

    RHO = uniform.ppf(sample[:,0], 0.5, 0.3)
    VEBoost = uniform.ppf(sample[:,1], .437, .261)
    VEW = uniform.ppf(sample[:,2], 0.02, 44)
    ISOLATION_MULTIPLIER = uniform.ppf(sample[:,3], 0, 0.5)
    BETA0 = uniform.ppf(sample[:,4], 0.01, 0.1)
    BETA1 = uniform.ppf(sample[:,5], 0, 1)


    # Treatment Params
    BOOSTER_DAY = [0, 59, 151, 243]
    T_REINFECTION = [180*24, 270*24, 365*24]
    vu_vec = [0.25, 0.5, 0.75]

    params_list = []

    # Create all combinations of treatment x transmission params
    for bd in BOOSTER_DAY:
        for tr in T_REINFECTION:
            for vu in vu_vec:
                for n in range(nreps):
                    params_list.append(baseline_params())
                    params_list[-1]["BOOSTER_DAY"] = bd
                    params_list[-1]["T_REINFECTION"] = tr
                    params_list[-1]["vax_rules"] = [VaccineRule(general=True,
                                                                hesitancy=(1-vu))]
                    params_list[-1]["RHO"] = RHO[n]
                    params_list[-1]["VEBOOST"] = VEBoost[n]
                    params_list[-1]["VEW"] = VEW[n]
                    params_list[-1]["ISOLATION_MULTIPLIER"] = ISOLATION_MULTIPLIER[n]
                    params_list[-1]["BETA0"] = BETA0[n]
                    params_list[-1]["BETA1"] = BETA1[n]
                    params_list[-1]["SEED"] = random.randint(0, 10000000)

    return params_list


def dict2str(d):
    s = ""
    for k, v in d.items():
        s += str(k) + "_" + str(v) + "__"

    return s


def run_sim(params, path):
    fname = {k: params[k] for k in ["BETA0", "BETA1", "BOOSTER_DAY", "T_REINFECTION", "SEED"]}
    fname = os.path.join(path, dict2str(fname) + ".pgz")

    if os.path.isfile(fname):
        return 0
    else:

        sim = BICS_ABM(**params, silent=True)

        with gzip.GzipFile(fname, 'wb') as f:
            pickle.dump(sim, f)

    return 1


if __name__ == "__main__":
    # Create a new subfolder for each day of simulations
    sims_path = os.path.relpath("../90days/eroubenoff/BICS_ABM/sims")
    sims_path = os.path.join(sims_path, date.today().strftime("%Y-%m-%d"))

    if not os.path.exists(sims_path):
        os.makedirs(sims_path)

    params = create_params()


    """
    for v in params[0:1]:
        run_sim(v, sims_path)
    """

    Parallel(n_jobs=20, verbose = 10)(delayed(run_sim)(v, sims_path) for v in params)

    # TODO: Sensitivty tests
    ISOLATION_MULTIPLIER = [0.25, 0.9]
    NPI = [0.25, 0.9]
