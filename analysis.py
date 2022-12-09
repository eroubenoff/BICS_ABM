import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from BICS_ABM import BICS_ABM, VaccineRule
import time
from joblib import Parallel, delayed
from scipy.stats.qmc import LatinHypercube
from scipy.stats import norm, uniform, randint
import gc



nsims = 1000
sampler = LatinHypercube(22)
sample = sampler.random(nsims)

gamma_min_v = [int(randint.ppf(x, 0*24, 2*24)) for x in sample[:,0]] 
gamma_max_v = [int(randint.ppf(x, 2*24, 4*24)) for x in sample[:,1]] 
sigma_min_v = [int(randint.ppf(x, 3*24, 5*24)) for x in sample[:,2]] 
sigma_max_v = [int(randint.ppf(x, 5*24, 7*24)) for x in sample[:,3]] 
beta_v = [uniform.ppf(x, 0, .10) for x in sample[:,4]] 
alpha_v = [uniform.ppf(x, 0, .25) for x in sample[:,5]] 
rho_v = [uniform.ppf(x, .7, 1) for x in sample[:,6]] 
isomult_v = [uniform.ppf(x, 0, 1) for x in sample[:,7]] 
npi_v = [uniform.ppf(x, 0.25, 0.75) for x in sample[:,8]]

mu0_v = [uniform.ppf(x, 0.003, 0.005)/100 for x in sample[:,9]] 
mu1_v = [uniform.ppf(x, 0.003, 0.005)/100 for x in sample[:,10]] 
mu2_v = [uniform.ppf(x, 0.058, 0.078)/100 for x in sample[:,11]] 
mu3_v = [uniform.ppf(x, 0.20, 0.26)/100 for x in sample[:,12]] 
mu4_v = [uniform.ppf(x, 0.66, 0.87)/100 for x in sample[:,13]] 
mu5_v = [uniform.ppf(x, 0.66, 0.87)/100 for x in sample[:,14]] 
mu6_v = [uniform.ppf(x, 2.1, 3.0)/100 for x in sample[:,15]] 
mu7_v = [uniform.ppf(x, 6.9, 10.4)/100 for x in sample[:,16]] 
mu8_v = [uniform.ppf(x, 21.8, 36.6)/100 for x in sample[:,17]] 

ve1_v = [uniform.ppf(x, 0.65, 0.75) for x in sample[:,18]]
ve2_v = [uniform.ppf(x, 0.85, 0.95) for x in sample[:,19]]
vew_v = [uniform.ppf(x, 0.25, 0.55) for x in sample[:,20]]
veb_v = [uniform.ppf(x, 0.65, 0.75) for x in sample[:,21]]


t = time.time()


def multi_fn(i):
    """ Wrapper function for multiprocessing

    """

    gc.collect()
    params = {
        "N_HH" : 1000,
        "GAMMA_MIN" : gamma_min_v[i],
        "GAMMA_MAX" : gamma_max_v[i],
        "SIGMA_MIN" : sigma_min_v[i],
        "SIGMA_MAX" : sigma_max_v[i],
        "BETA" : beta_v[i],
        "ALPHA" : alpha_v[i],
        "RHO" : rho_v[i],
        "ISOLATION_MULTIPLIER" : isomult_v[i],
        "MU_VEC" : [
            mu0_v[i],
            mu1_v[i],
            mu2_v[i],
            mu3_v[i],
            mu4_v[i],
            mu5_v[i],
            mu6_v[i],
            mu7_v[i],
            mu8_v[i]],
        "silent" : True,
        "T0" : 0,
        "INDEX_CASES": 5,
        "SEED" : i,
        "VE1" : ve1_v[i],
        "VE2" : ve2_v[i],
        "VEW" : vew_v[i],
        "VEBOOST" : veb_v[i],
        "NPI" : npi_v[i]
    }

    result_none = BICS_ABM(
        **params,
        vax_rules = [VaccineRule(general = False)]
    )

    result_general = BICS_ABM(
        pop = result_none._pop,
        **params,
        vax_rules = [VaccineRule(general = True)]
    )

    result_age = BICS_ABM(
        pop = result_none._pop,
        **params,
        vax_rules = [
            VaccineRule("age > 80"),
            VaccineRule("age > 70"),
            VaccineRule("age > 60"),
            VaccineRule("age > 50"),
            VaccineRule("age > 30"),
            VaccineRule("age > 20"),
            VaccineRule(general = True)
        ]
    )

    result_cc = BICS_ABM(
        pop = result_none._pop,
        **params,
        vax_rules = [
            VaccineRule("num_cc_nonhh > 30"),
            VaccineRule("num_cc_nonhh > 25"),
            VaccineRule("num_cc_nonhh > 20"),
            VaccineRule("num_cc_nonhh > 15"),
            VaccineRule("num_cc_nonhh > 10"),
            VaccineRule("num_cc_nonhh > 5"),
            VaccineRule("num_cc_nonhh > 4"),
            VaccineRule("num_cc_nonhh > 3"),
            VaccineRule("num_cc_nonhh > 2"),
            VaccineRule("num_cc_nonhh > 1"),
            VaccineRule(general = True)
        ]
    )

    ret = {
        'none_d': result_none.D[-1] / (result_none.S[0] + result_none.E[0]),
        'none_r': result_none.R[-1] / (result_none.S[0] + result_none.E[0]),
        'general_d': result_general.D[-1] / (result_general.S[0] + result_general.E[0]),
        'general_r': result_general.R[-1] / (result_general.S[0] + result_general.E[0]),
        'age_d': result_age.D[-1] / (result_age.S[0] + result_age.E[0]),
        'age_r': result_age.R[-1] / (result_age.S[0] + result_age.E[0]),
        'cc_d': result_cc.D[-1] / (result_cc.S[0] + result_cc.E[0]),
        'cc_r': result_cc.R[-1] / (result_cc.S[0] + result_cc.E[0]),
    }


    del result_none
    del result_general
    del result_age
    del result_cc
    gc.collect()

    return ret


# 10 is about the efficiency limit-- 2400 cycles will take about 2 mins
result_vec = Parallel(n_jobs=-1, verbose = 10, prefer="threads"
        )(delayed(multi_fn)(v) for v in range(nsims))


t = time.time() - t

print(len(result_vec), "iterations took", t, "seconds")

q1_df = pd.DataFrame(result_vec)

q1_df.to_csv("q1_df.csv")


t = time.time()


def multi_fn(i):
    """ Wrapper function for multiprocessing

    """

    gc.collect()
    params = {
        "N_HH" : 1000,
        "GAMMA_MIN" : gamma_min_v[i],
        "GAMMA_MAX" : gamma_max_v[i],
        "SIGMA_MIN" : sigma_min_v[i],
        "SIGMA_MAX" : sigma_max_v[i],
        "BETA" : beta_v[i],
        "ALPHA" : alpha_v[i],
        "RHO" : rho_v[i],
        "ISOLATION_MULTIPLIER" : isomult_v[i],
        "MU_VEC" : [
            mu0_v[i],
            mu1_v[i],
            mu2_v[i],
            mu3_v[i],
            mu4_v[i],
            mu5_v[i],
            mu6_v[i],
            mu7_v[i],
            mu8_v[i]],
        "silent" : True,
        "T0" : 30,
        "INDEX_CASES": 5,
        "SEED" : i,
        "VE1" : ve1_v[i],
        "VE2" : ve2_v[i],
        "VEW" : vew_v[i],
        "VEBOOST" : veb_v[i],
        "NPI" : npi_v[i]
    }

    result_none = BICS_ABM(
        **params,
        vax_rules = [VaccineRule(general = False)]
    )

    result_general = BICS_ABM(
        pop = result_none._pop,
        **params,
        vax_rules = [VaccineRule(general = True)]
    )

    result_age = BICS_ABM(
        pop = result_none._pop,
        **params,
        vax_rules = [
            VaccineRule("age > 80"),
            VaccineRule("age > 70"),
            VaccineRule("age > 60"),
            VaccineRule("age > 50"),
            VaccineRule("age > 30"),
            VaccineRule("age > 20"),
            VaccineRule(general = True)
        ]
    )

    result_cc = BICS_ABM(
        pop = result_none._pop,
        **params,
        vax_rules = [
            VaccineRule("num_cc_nonhh > 30"),
            VaccineRule("num_cc_nonhh > 25"),
            VaccineRule("num_cc_nonhh > 20"),
            VaccineRule("num_cc_nonhh > 15"),
            VaccineRule("num_cc_nonhh > 10"),
            VaccineRule("num_cc_nonhh > 5"),
            VaccineRule("num_cc_nonhh > 4"),
            VaccineRule("num_cc_nonhh > 3"),
            VaccineRule("num_cc_nonhh > 2"),
            VaccineRule("num_cc_nonhh > 1"),
            VaccineRule(general = True)
        ]
    )

    return {
        'none_d': result_none.D[-1] / (result_none.S[0] + result_none.E[0]),
        'none_r': result_none.R[-1] / (result_none.S[0] + result_none.E[0]),
        'general_d': result_general.D[-1] / (result_general.S[0] + result_general.E[0]),
        'general_r': result_general.R[-1] / (result_general.S[0] + result_general.E[0]),
        'age_d': result_age.D[-1] / (result_age.S[0] + result_age.E[0]),
        'age_r': result_age.R[-1] / (result_age.S[0] + result_age.E[0]),
        'cc_d': result_cc.D[-1] / (result_cc.S[0] + result_cc.E[0]),
        'cc_r': result_cc.R[-1] / (result_cc.S[0] + result_cc.E[0]),
    }


# 10 is about the efficiency limit-- 2400 cycles will take about 2 mins
result_vec = Parallel(n_jobs=-1, verbose = 10, prefer="threads"
        )(delayed(multi_fn)(v) for v in range(nsims))


t = time.time() - t

print(len(result_vec), "iterations took", t, "seconds")
q2_df = pd.DataFrame(result_vec)
q2_df.to_csv("q2_df.csv")


t = time.time()


def multi_fn(i):
    """ Wrapper function for multiprocessing

    """

    gc.collect()
    params = {
        "N_HH" : 1000,
        "GAMMA_MIN" : gamma_min_v[i],
        "GAMMA_MAX" : gamma_max_v[i],
        "SIGMA_MIN" : sigma_min_v[i],
        "SIGMA_MAX" : sigma_max_v[i],
        "BETA" : beta_v[i],
        "ALPHA" : alpha_v[i],
        "RHO" : rho_v[i],
        "ISOLATION_MULTIPLIER" : isomult_v[i],
        "MU_VEC" : [
            mu0_v[i],
            mu1_v[i],
            mu2_v[i],
            mu3_v[i],
            mu4_v[i],
            mu5_v[i],
            mu6_v[i],
            mu7_v[i],
            mu8_v[i]],
        "silent" : True,
        "T0" : 30,
        "INDEX_CASES": 5,
        "SEED" : i,
        "VE1" : ve1_v[i],
        "VE2" : ve2_v[i],
        "VEW" : vew_v[i],
        "VEBOOST" : veb_v[i],
        "NPI" : npi_v[i]
    }


    result_general = BICS_ABM(
        **params,
        vax_rules = [
            VaccineRule(general=True)
        ]
    )
    result_30 = BICS_ABM(
        pop = result_general._pop,
        **params,
        vax_rules = [
            VaccineRule("num_cc_nonhh > 30")
        ]
    )
    result_25 = BICS_ABM(
        pop = result_general._pop,
        **params,
        vax_rules = [
            VaccineRule("num_cc_nonhh > 25")
        ]
    )
    result_20 = BICS_ABM(
        pop = result_general._pop,
        **params,
        vax_rules = [
            VaccineRule("num_cc_nonhh > 20")
        ]
    )

    result_15 = BICS_ABM(
        pop = result_general._pop,
        **params,
        vax_rules = [
            VaccineRule("num_cc_nonhh > 15")
        ]
    )
    result_10 = BICS_ABM(
        pop = result_general._pop,
        **params,
        vax_rules = [
            VaccineRule("num_cc_nonhh > 10")
        ]
    )

    result_5 = BICS_ABM(
        pop = result_general._pop,
        **params,
        vax_rules = [
            VaccineRule("num_cc_nonhh > 5")
        ]
    )
    result_2 = BICS_ABM(
        pop = result_general._pop,
        **params,
        vax_rules = [
            VaccineRule("num_cc_nonhh > 2")
        ]
    )
    result_1 = BICS_ABM(
        pop = result_general._pop,
        **params,
        vax_rules = [
            VaccineRule("num_cc_nonhh > 1")
        ]
    )

    return {
        'general_d': result_general.D[-1] / (result_general.S[0] + result_general.E[0]),
        'general_r': result_general.R[-1] / (result_general.S[0] + result_general.E[0]),
        '30_d': result_30.D[-1] / (result_30.S[0] + result_30.E[0]),
        '30_r': result_30.R[-1] / (result_30.S[0] + result_30.E[0]),
        '25_d': result_25.D[-1] / (result_25.S[0] + result_25.E[0]),
        '25_r': result_25.R[-1] / (result_25.S[0] + result_25.E[0]),
        '20_d': result_20.D[-1] / (result_20.S[0] + result_20.E[0]),
        '20_r': result_20.R[-1] / (result_20.S[0] + result_20.E[0]),
        '15_d': result_15.D[-1] / (result_15.S[0] + result_15.E[0]),
        '15_r': result_15.R[-1] / (result_15.S[0] + result_15.E[0]),
        '10_d': result_10.D[-1] / (result_10.S[0] + result_10.E[0]),
        '10_r': result_10.R[-1] / (result_10.S[0] + result_10.E[0]),
        '5_d': result_5.D[-1] / (result_5.S[0] + result_5.E[0]),
        '5_r': result_5.R[-1] / (result_5.S[0] + result_5.E[0]),
        '2_d': result_2.D[-1] / (result_2.S[0] + result_2.E[0]),
        '2_r': result_2.R[-1] / (result_2.S[0] + result_2.E[0]),
        '1_d': result_1.D[-1] / (result_1.S[0] + result_1.E[0]),
        '1_r': result_1.R[-1] / (result_1.S[0] + result_1.E[0]),
    }


# 10 is about the efficiency limit-- 2400 cycles will take about 2 mins
result_vec = Parallel(n_jobs=-1, verbose = 10, prefer="threads"
        )(delayed(multi_fn)(v) for v in range(nsims))


t = time.time() - t

print(len(result_vec), "iterations took", t, "seconds")
q3_df = pd.DataFrame(result_vec)
q3_df.to_csv("q3_df.csv")
