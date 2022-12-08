import os
import ctypes
import numpy as np
import pandas as pd
from numpy.ctypeslib import ndpointer
from copy import deepcopy
import matplotlib.pyplot as plt


path = os.getcwd()
path = os.path.join(path, "build", "libBICS_ABM_lib.dylib")
_BICS_ABM = ctypes.CDLL(path)


def recode_age(age):
    
    if np.isnan(age):
        return -1

    if (age < 18):
        return 0
    elif (age < 25):
        return 1
    elif (age < 35):
        return 2
    elif (age < 45):
        return 3
    elif (age < 55):
        return 4
    elif (age < 65):
        return 5
    elif (age < 75):
        return 6
    elif (age < 85):
        return 7
    elif (age >= 85):
        return 8

    else:
        raise ValueError("Invalid age: "  + str(age))

def recode_gender(gender):
    try:
        if np.isnan(gender):
            return -1
    except:
        pass


    if (gender == "F"):
        return 1
    elif gender == "Female":
        return 1
    elif (gender == "M"):
        return 0
    elif (gender == "Male"):
        return 0

    else:
        raise ValueError("Invalid Gender: " + str(gender))

def recode_lefthome(lefthome_num):
    if lefthome_num == "More than 5":
        return 6
    else:
        try: 
            return int(lefthome_num)
        except:
            raise ValueError("Invalid lefthome num: " +  lefthome_num)

def sum_to_1(v):
    return v/sum(v)



def load_data():
    BICS = pd.read_csv("data/df_all_waves.csv")

    # Need to recode all age and gender columns
    BICS["agecat"] = BICS["age"].apply(recode_age)
    # BICS["resp_hh_roster#1_0_1"] = BICS["resp_hh_roster#1_0_1"].apply(recode_age)
    BICS["resp_hh_roster#1_1_1"] = BICS["resp_hh_roster#1_1_1"].apply(recode_age)
    BICS["resp_hh_roster#1_2_1"] = BICS["resp_hh_roster#1_2_1"].apply(recode_age)
    BICS["resp_hh_roster#1_3_1"] = BICS["resp_hh_roster#1_3_1"].apply(recode_age)
    BICS["resp_hh_roster#1_4_1"] = BICS["resp_hh_roster#1_4_1"].apply(recode_age)
    BICS["resp_hh_roster#1_5_1"] = BICS["resp_hh_roster#1_5_1"].apply(recode_age)

    BICS["gender"] = BICS["gender"].apply(recode_gender)
    # BICS["resp_hh_roster#2_0"] = BICS["resp_hh_roster#2_0"].apply(recode_gender)
    BICS["resp_hh_roster#2_1"] = BICS["resp_hh_roster#2_1"].apply(recode_gender)
    BICS["resp_hh_roster#2_2"] = BICS["resp_hh_roster#2_2"].apply(recode_gender)
    BICS["resp_hh_roster#2_3"] = BICS["resp_hh_roster#2_3"].apply(recode_gender)
    BICS["resp_hh_roster#2_4"] = BICS["resp_hh_roster#2_4"].apply(recode_gender)
    BICS["resp_hh_roster#2_5"] = BICS["resp_hh_roster#2_5"].apply(recode_gender)

    BICS["lefthome_num"] = BICS["lefthome_num"].apply(recode_lefthome)


    # Set index for efficient querying
    BICS = BICS.reset_index(drop=True).sort_index()

    return BICS

# Create these as globals
BICS_global =  load_data()




# @profile
def create_pop(colnames: list, pop: np.ndarray, n_hh: int = 1000, wave: int = 6, seed = 49) -> np.ndarray:

    np.random.seed(seed)

    colnames = {k:v for v, k in enumerate(colnames)}

    pop_list = list()

    # Pre-generate households 
    hhs = np.random.choice(
            pop.shape[0],
            size = n_hh, 
            replace = True, 
            p = sum_to_1(pop[:,colnames["weight_pooled"]])
            )

    for hhid, hh in enumerate(hhs):

        hhead = pop[hh,:]
        hhead[colnames["hhid"]] = hhid 
        pop_list.append(hhead.copy())

        hhsize = int(hhead[colnames['hhsize']])


        if hhsize > 1: 

            for hhmember in range(min(hhsize, 5)):
                hhmember = hhmember + 1
                hhmember_age = hhead[colnames["resp_hh_roster#1_" + str(hhmember) + "_1"]]
                hhmember_gender = hhead[colnames["resp_hh_roster#2_" + str(hhmember)]]

                if hhmember_age == 0:
                    # TODO: go to POLYMOD
                    hhmember = pop 
                elif hhmember_age == -1 or hhmember_gender == -1:
                    hhmember = pop

                else:
                    hhmember = pop[(pop[:,colnames['agecat']] == hhmember_age) & (pop[:,colnames['gender']] == hhmember_gender), :]
                    if hhmember.shape[0] == 0:
                        hhmember = pop 


                # Sample a corresponding person
                hhmember_id = np.random.choice( hhmember.shape[0], size = 1)
                hhmember =  hhmember[hhmember_id][0]
                hhmember[colnames['hhid']] = hhid
                pop_list.append(hhmember.copy())

    # Pull out the columns relevant and put everybody in a COLUMN-MAJOR (fortran-style) matrix! 
    ret = np.stack(pop_list, axis = 0)

    ret = ret[:, [colnames[x] for x in ["hhid", "agecat", "gender", "num_cc_nonhh", "lefthome_num", "vaccine_priority", "NPI"]]]
    ret = np.asfortranarray(ret)

    return ret 

class VaccineRule:
    def __init__(self, query = "index == index or index != index", hesitancy = None, general = False):
        if hesitancy is not None: 
            if hesitancy < 0 or hesitancy > 1:
                raise ValueError("Hesitancy must be between 0 and 1, not " + str(hesitancy))
            else:
                self.hesitancy = hesitancy

        else:
            self.hesitancy = None

        if general == True:
            self.general = True
        elif general == False:
            self.general = False
        else: 
            raise ValueError("General must be true, false, or none " + str(general))
        
        self.query = query



def create_vax_priority(pop, vax_rules = None, seed = 49): 
    """
    Assume that vax_rules is in the form of:
    [
        VaccineRule("age > 80"), 
        VaccineRule("age > 60 & num_cc_nonhh > 10"),
        VaccineRule(general = True, hesitancy = 0.5)
    ]

    """
    # TODO: Need to add hesitancy and general distribution

    pop["vaccine_priority"] = -1

    if vax_rules is None: 
        return pop

    vax_rules.reverse()

    vaccine_priority = pop.columns.get_loc("vaccine_priority")

    # Parse rules into a more ordered format 
    for i, v in enumerate(vax_rules):
        mask = pop.eval(v.query)
        idx = mask * range(pop.shape[0])
        idx = idx[idx != 0]

        if v.general is True and v.hesitancy is None:
            pop.iloc[:,vaccine_priority] = i 

        elif v.general is True and v.hesitancy is not None:
            idx = idx.sample(frac = v.hesitancy, random_state = seed).tolist()
            pop.iloc[idx, vaccine_priority] = i

        elif v.hesitancy is not None:
            idx = idx.sample(frac = v.hesitancy, random_state = seed).tolist()
            pop.iloc[idx, vaccine_priority] = i

        else: 
            pop.iloc[idx, vaccine_priority] = i

    # Tally total counts in each
    # print(pop["vaccine_priority"].value_counts())

    return pop 


    
def pop_to_np(pop: pd.DataFrame):

    # All data types are numeric

    pop.loc[:,"hhid"] = 0
    colnames = ["hhid", "agecat", "gender", "num_cc_nonhh", 
            "lefthome_num", "vaccine_priority", "weight_pooled", "hhsize", "NPI",
            "resp_hh_roster#1_1_1",
            "resp_hh_roster#1_2_1",
            "resp_hh_roster#1_3_1",
            "resp_hh_roster#1_4_1",
            "resp_hh_roster#1_5_1",
            "resp_hh_roster#2_1",
            "resp_hh_roster#2_2",
            "resp_hh_roster#2_3",
            "resp_hh_roster#2_4",
            "resp_hh_roster#2_5",
            ]

    return colnames, pop.loc[:, colnames].to_numpy().astype(float)



"""
Parameters object. Note that all of the arguments
here ABSOLUTELY must match the order in 
BICS_ABM.h! 
"""

class Params(ctypes.Structure):
    _fields_ = [
            ('N_HH', ctypes.c_int),
            ('WAVE', ctypes.c_int),
            ('GAMMA_MIN', ctypes.c_int),
            ('GAMMA_MAX', ctypes.c_int),
            ('SIGMA_MIN', ctypes.c_int),
            ('SIGMA_MAX', ctypes.c_int),
            ('BETA', ctypes.c_float),
            ('MU_VEC', ctypes.c_float*9),
            ('INDEX_CASES', ctypes.c_int),
            ('SEED', ctypes.c_int),
            ('N_VAX_DAILY', ctypes.c_int),
            ('VE1', ctypes.c_float),
            ('VE2', ctypes.c_float),
            ('VEW', ctypes.c_float),
            ('VEBOOST', ctypes.c_float),
            ('ISOLATION_MULTIPLIER', ctypes.c_float),
            ('T_REINFECTION', ctypes.c_int),
            ('T0', ctypes.c_int),
            ('ALPHA', ctypes.c_float),
            ('RHO', ctypes.c_float),
            ('NPI', ctypes.c_float)
            
    ]

""" 
Passing population array to ABM
C-type corresponding to numpy 2-dimensional array (matrix)
https://asiffer.github.io/posts/numpy/

IMPORTANT: note that this is COLUMN-MAJOR order, Fortran style,
which saves us a loop on the c++ side when adding to igraph

""" 
ND_POINTER_2 = np.ctypeslib.ndpointer(dtype=np.float64,
                                      ndim=2,
                                      flags="F")


"""
Return object
"""
class Trajectory (ctypes.Structure):
    _fields_ = [
        ('S_array', ctypes.c_int*5000),
        ('E_array', ctypes.c_int*5000),
        ('Ic_array', ctypes.c_int*5000),
        ('Isc_array', ctypes.c_int*5000),
        ('R_array', ctypes.c_int*5000),
        ('D_array', ctypes.c_int*5000),
        ('V1_array', ctypes.c_int*5000),
        ('V2_array', ctypes.c_int*5000),
        ('VW_array', ctypes.c_int*5000),
        ('VBoost_array', ctypes.c_int*5000),
        ('n_edges_array', ctypes.c_int*5000),
        ('counter', ctypes.c_int)
    ]

_BICS_ABM.BICS_ABM.argtypes = [ND_POINTER_2, ctypes.c_size_t, ctypes.c_size_t, Params, ctypes.c_bool]
_BICS_ABM.BICS_ABM.restype = Trajectory

_BICS_ABM.init_params.argtypes = () 
_BICS_ABM.init_params.restype = Params 



class BICS_ABM:

    def __init__(self, n_hh = 1000, wave = 6, vax_rules = [VaccineRule(general=True, hesitancy = 0.5)], silent = False, **kwargs):
        self._params = _BICS_ABM.init_params()
        for k, v in kwargs.items():
            if k not in self._params.__dir__():

                raise ValueError("Invalid parameter " + k + " passed to BICS_ABM")

            else:
                if k == "MU_VEC":
                    if len(v) != 9:
                        raise ValueError("Case fatality must be 9 element list")
                    v = (ctypes.c_float * 9)(* v)
                setattr(self._params, k, v)

        self.seed = self._params.SEED

        self._pop = BICS_global.loc[BICS_global.wave == self._params.WAVE,:].copy(deep=True)
        self._pop = create_vax_priority(self._pop, vax_rules, seed=self.seed)
        self._colnames, self._pop = pop_to_np(self._pop)
        self._pop = create_pop(self._colnames, self._pop, n_hh = self._params.N_HH, seed=self.seed)

        self._instance = _BICS_ABM.BICS_ABM(self._pop, *self._pop.shape, self._params, silent)

        self.counter = self._instance.counter
        self.S = self._instance.S_array[:self.counter]
        self.E = self._instance.E_array[:self.counter]
        self.Ic = self._instance.Ic_array[:self.counter]
        self.Isc = self._instance.Isc_array[:self.counter]
        self.R = self._instance.R_array[:self.counter]
        self.D = self._instance.D_array[:self.counter]
        self.V1 = self._instance.V1_array[:self.counter]
        self.V2 = self._instance.V2_array[:self.counter]
        self.VW = self._instance.VW_array[:self.counter]
        self.VBoost = self._instance.VBoost_array[:self.counter]
        self.n_edges= self._instance.n_edges_array[:self.counter]

    def plot_trajectory(self):

        xaxis =  [x / 24 for x in range(self.counter)]

        fig, ax = plt.subplots(2)

        ax[0].plot(xaxis, self.S, label = "S")
        ax[0].plot(xaxis, self.E, label = "E")
        ax[0].plot(xaxis, self.Ic, label = "Ic")
        ax[0].plot(xaxis, self.Isc, label = "Isc")
        ax[0].plot(xaxis, self.R, label = "R")
        ax[0].plot(xaxis, self.D, label = "D")
        ax[0].legend()
        ax[0].set_title("Disease States")

        ax[1].plot(xaxis, self.V1, label = "V1")
        ax[1].plot(xaxis, self.V2, label = "V2")
        ax[1].plot(xaxis, self.VW, label = "VW")
        ax[1].plot(xaxis, self.VBoost, label = "VBoost")
        ax[1].legend()
        ax[1].set_title("Vaccination Rates")

        plt.show()




if __name__ == "__main__" :
    b0 = BICS_ABM()
    BICS_ABM(**{'N_HH': 1000,
     'GAMMA_MIN': 24,
     'GAMMA_MAX': 72,
     'SIGMA_MIN': 72,
     'SIGMA_MAX': 120,
     'BETA': 0.05507544898896605,
     'ALPHA': 0.1830251830862492,
     'RHO': 0.6474413453425609,
     'ISOLATION_MULTIPLIER': 0.824480802946687,
     'MU_VEC': [3.5228070072044665e-05,
      4.238936395192768e-05,
      0.0010822282956317395,
      0.002854986079377454,
      0.010868770209864088,
      0.013643996316117462,
      0.04180962401407782,
      0.16165378408335598,
      0.2899616251020893],
     'silent': False,
     'INDEX_CASES' : 1,
     'T0': 30},
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

        ])
              
