import os
import ctypes
import numpy as np
import pandas as pd
from numpy.ctypeslib import ndpointer
from copy import deepcopy
import matplotlib.pyplot as plt
from threading import Thread
import pdb
from scipy.stats import bernoulli, poisson


path = os.getcwd()

paths = os.path.join(path, "build", "libBICS_ABM_lib.so")
pathd = os.path.join(path, "build", "libBICS_ABM_lib.dylib")

if os.path.isfile(paths):
    path = paths
elif os.path.isfile(pathd):
    path = pathd
_BICS_ABM = ctypes.CDLL(path)


def recode_age(age):

    if pd.isna(age):
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
        if pd.isna(gender):
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


def load_POLYMOD(path="data/POLYMOD/"):
    # Read in the participants file
    POLYMOD_participant = pd.read_csv(path + "2008_Mossong_POLYMOD_participant_common.csv",
            dtype={'part_id': 'Int64', 'hh_id': str, 'part_age': 'Int64', 'part_gender': str},
            na_values=['none', -1, 'NA'])
     
    # Rename age variable
    POLYMOD_participant['age'] = POLYMOD_participant['part_age']
    # Recode age into same bins as BICS
    POLYMOD_participant['agecat'] = POLYMOD_participant['part_age'].apply(recode_age)
    POLYMOD_participant['gender'] = POLYMOD_participant['part_gender'].apply(recode_gender)


    # Read in the households files
    POLYMOD_hh = pd.read_csv(path + "2008_Mossong_POLYMOD_hh_common.csv", dtype = {'hh_id': str,
        'country': 'category', 'hh_size': 'Int64'}, dtype_backend="numpy_nullable")
    POLYMOD_hh['hhsize'] = POLYMOD_hh['hh_size']

    

    # Read in the contacts and also re-gorup age
    POLYMOD_contact = pd.read_csv(path + "2008_Mossong_POLYMOD_contact_common.csv",
            dtype={'part_id': 'Int64', 'cont_id': 'Int64', 'cnt_age_exact': 'Int64', 'cnt_age_est_min': 'Int64',
                'cnt_age_est_max': 'Int64', 'cnt_gender': 'category', 'cnt_home': 'boolean', 'cnt_work': 'boolean',
                'cnt_school': 'boolean', 'cnt_transport': 'boolean', 'cnt_leisure': 'boolean', 'cnt_otherplace': 'boolean',
                'frequency_multi': 'Int64', 'phys_contact': 'Int64', 'duration_multi': 'Int64'},
            na_values=['none', -1, 'NA'])
    POLYMOD_contact["cnt_agecat"] = POLYMOD_contact["cnt_age_exact"].apply(recode_age)

    # Tally up the number of contacts for each participant
    POLYMOD_contact = POLYMOD_contact.groupby('part_id').agg({
        'cnt_home': sum, 'cnt_work': sum, 
        'cnt_school': sum, 'cnt_transport': sum, 
        'cnt_leisure': sum, 'cnt_otherplace': sum
    })

    POLYMOD_contact['num_cc_school'] = POLYMOD_contact['cnt_school']
    del POLYMOD_contact['cnt_school']
    POLYMOD_contact['num_cc_nonhh'] = POLYMOD_contact[[col for col in POLYMOD_contact if col.startswith('cnt_')]].sum(axis=1)

    # Join the contacts to the participants
    POLYMOD = POLYMOD_participant.join(POLYMOD_contact, on = 'part_id')

    # Join the participants to the households
    POLYMOD = POLYMOD.set_index('hh_id').join(POLYMOD_hh.set_index('hh_id')).reset_index()

    # drop NA
    POLYMOD = POLYMOD.dropna()

    # Fill num_lefthome
    POLYMOD["lefthome_num"] = 1
    POLYMOD["weight_pooled"] = 1
    POLYMOD["NPI"] = False
    POLYMOD["resp_hh_roster#1_1_1"]  = -1
    POLYMOD["resp_hh_roster#1_2_1"] = -1
    POLYMOD["resp_hh_roster#1_3_1"] = -1
    POLYMOD["resp_hh_roster#1_4_1"] = -1
    POLYMOD["resp_hh_roster#1_5_1"] = -1
    POLYMOD["resp_hh_roster#2_1"] = -1
    POLYMOD["resp_hh_roster#2_2"] = -1
    POLYMOD["resp_hh_roster#2_3"] = -1
    POLYMOD["resp_hh_roster#2_4"] = -1
    POLYMOD["resp_hh_roster#2_5"] = -1

    # Reset index
    POLYMOD = POLYMOD.reset_index(drop=True).sort_index()


    return POLYMOD



def load_data(path="data/df_all_waves.csv"):
    BICS = pd.read_csv("data/df_all_waves.csv", dtype_backend="numpy_nullable")

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

    BICS["num_cc_school"] = 0


    # Set index for efficient querying
    BICS = BICS.reset_index(drop=True).sort_index()

    return BICS

# Create these as globals
BICS_global =  load_data()





# @profile
def create_pop(colnames: list, pop: np.ndarray, n_hh: int, wave: int, seed, polymod: np.ndarray = None) -> np.ndarray:

    np.random.seed(seed)

    colnames = {k:v for v, k in enumerate(colnames)}

    pop_list = list()

    ## DF with probabilities
    probs = pd.read_csv("age_and_sex.csv")
    probs["agecat"] = probs["agecat"] - 1
    probs = probs[probs["agecat"] > 0]

    hhs = probs.sample(n = n_hh, replace = True, weights = probs.value )
    hhs = hhs[["agecat", "gender"]]

    hhs_list =  list()
    for i,h in hhs.iterrows():
        w = deepcopy(pop[:, colnames["weight_pooled"]])
        w[
                (pop[:, colnames["agecat"]] != h.agecat) &
                (pop[:, colnames["gender"]] != h.gender)
                ] = 0
        if (sum(w) == 0):
            w = pop[:, colnames["weight_pooled"]]

        hhs_list.append(np.random.choice(pop.shape[0], 1, p = sum_to_1(w))[0])
        
    hhs = np.array(hhs_list)
    print(hhs)

    # Pre-generate households
    """ 
    hhs = np.random.choice(
            pop.shape[0],
            size = n_hh,
            replace = True,
            p = sum_to_1(pop[:,colnames["weight_pooled"]])
            )
    """

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
                    if polymod is not None:
                        hhmember = polymod[(polymod[:, colnames['agecat']] == hhmember_age) & (polymod[:, colnames['gender']] == hhmember_gender),:]

                    else:
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

    ret = ret[:, [colnames[x] for x in ["hhid", "agecat", "gender", "num_cc_nonhh", "num_cc_school", "lefthome_num", "vaccine_priority", "NPI"]]]



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
            if not isinstance(idx, list):
                idx = idx.tolist()
            pop.iloc[idx, vaccine_priority] = i

        elif v.hesitancy is not None:
            idx = idx.sample(frac = v.hesitancy, random_state = seed).tolist()
            if not isinstance(idx, list):
                idx = idx.tolist()
            pop.iloc[idx, vaccine_priority] = i

        else:
            if not isinstance(idx, list):
                idx = idx.tolist()
            pop.iloc[idx, vaccine_priority] = i

    # Tally total counts in each
    # print(pop["vaccine_priority"].value_counts())

    return pop



def pop_to_np(pop: pd.DataFrame):

    # All data types are numeric

    pop.loc[:,"hhid"] = 0
    colnames = ["hhid", "agecat", "gender", "num_cc_nonhh", "num_cc_school",
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
            ('BETA_VEC', ctypes.c_float*365),
            ('CONTACT_MULT_VEC', ctypes.c_float*365),
            ('SCHOOL_CONTACTS', ctypes.c_bool),
            ('MU_VEC', ctypes.c_float*9),
            ('INDEX_CASES', ctypes.c_int),
            ('IMPORT_CASES_VEC', ctypes.c_int*365),
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
            ('NPI', ctypes.c_float),
            ('MAX_DAYS', ctypes.c_int),
            ('BOOSTER_DAY', ctypes.c_int),
            ('FERTILITY_V', ctypes.c_float*9),
            ('MORTALITY_V', ctypes.c_float*9)

    ]

    def __init__(self):
        self.N_HH = 1000
        self.WAVE = 6
        self.GAMMA_MIN = 2*24
        self.GAMMA_MAX = 4*24
        self.SIGMA_MIN = 3*24
        self.SIGMA_MAX = 7*24
        beta_vec = [0] * 365
        self.BETA_VEC = (ctypes.c_float * 365)(*beta_vec)
        contact_mult_vec = [1] * 365
        self.CONTACT_MULT_VEC = (ctypes.c_float * 365)(*contact_mult_vec)
        self.SCHOOL_CONTACTS = 0
        mu = [0.00001, 0.0001, 0.0001, 0.001, 0.001, 0.001, 0.01, 0.1, 0.1]
        self.MU_VEC = (ctypes.c_float * 9)(*mu)
        self.INDEX_CASES = 5
        index_cases_vec = [0] * 365
        self.IMPORT_CASES_VEC= (ctypes.c_int* 365)(*index_cases_vec)
        self.SEED = 49
        self.N_VAX_DAILY = 100
        self.VE1 = 0.75
        self.VE2 = 0.95
        self.VEW = 0.25
        self.VEBOOST = 0.95
        self.ISOLATION_MULTIPLIER = 1
        self.T_REINFECTION = 90*24
        self.T0 = 0
        self.ALPHA = 0.5
        self.RHO = 0.5
        self.NPI = 0.75
        self.MAX_DAYS = -1
        self.BOOSTER_DAY = 400

        """
        Incorporate demography. This is done by drawing, from a corresponding rate/probability
        vector, the number of children and a boolean represeting if the person will die of
        non-COVID causes.

        Sources: 
        https://www.cdc.gov/nchs/products/databriefs/db442.htm
        https://www.cdc.gov/nchs/products/databriefs/db456.htm
        """


        fertility = [13.9/(1000 / (3/18)), 61.5/1000, (93.0 + 97.6)/(2*1000), (53.7 + 12.0)/(2*1000), 0, 0, 0, 0, 0]
        self.FERTILITY_V = (ctypes.c_float*9)(*fertility)
        mortality = [14.3/100000, 88.9/100000, 180.8/100000, 287.9/100000, 531.0/100000, 1117.1/100000, 2151.3/100000, 5119.4/100000, 15743.3/100000]
        self.MORTALITY_V = (ctypes.c_float*9)(*mortality)


"""
Passing population array to ABM
C-type corresponding to numpy 2-dimensional array (matrix)
https://asiffer.github.io/posts/numpy/

IMPORTANT: note that this is COLUMN-MAJOR order, Fortran style,
which saves us a loop on the c++ side when adding to igraph

"""
ND_POINTER_2 = np.ctypeslib.ndpointer(dtype=ctypes.c_double,
                                      ndim=2,
                                      flags="F")


"""
Return object
"""
TRAJ_SIZE = 100000
class Trajectory (ctypes.Structure):
    _fields_ = [
        ('S_array', ctypes.c_int*TRAJ_SIZE),
        ('E_array', ctypes.c_int*TRAJ_SIZE),
        ('Ic_array', ctypes.c_int*TRAJ_SIZE),
        ('Cc_array', ctypes.c_int*TRAJ_SIZE),
        ('Isc_array', ctypes.c_int*TRAJ_SIZE),
        ('Csc_array', ctypes.c_int*TRAJ_SIZE),
        ('R_array', ctypes.c_int*TRAJ_SIZE),
        ('D_array', ctypes.c_int*TRAJ_SIZE),
        ('V1_array', ctypes.c_int*TRAJ_SIZE),
        ('V2_array', ctypes.c_int*TRAJ_SIZE),
        ('VW_array', ctypes.c_int*TRAJ_SIZE),
        ('VBoost_array', ctypes.c_int*TRAJ_SIZE),
        ('n_edges_array', ctypes.c_int*TRAJ_SIZE),
        ('counter', ctypes.c_int)
    ]

_BICS_ABM.BICS_ABM.argtypes = [ND_POINTER_2, ctypes.c_size_t, ctypes.c_size_t, ctypes.POINTER(Trajectory), ctypes.POINTER(Params), ctypes.c_bool]
_BICS_ABM.BICS_ABM.restype = None # Trajectory

# _BICS_ABM.init_params.argtypes = ()
# _BICS_ABM.init_params.restype = Params



class BICS_ABM:

    def __init__(self, n_hh = 1000, wave = 6,
            vax_rules = [VaccineRule(general=True, hesitancy = 0.5)], silent = False,
            pop = None, POLYMOD = True, **kwargs):

        kwargs = {k.upper():v for k,v in kwargs.items()}

        self._params = Params()  # _BICS_ABM.init_params()

        # Handle Beta first
        if "BETA0" in kwargs and "BETA1" in kwargs:
            if "BETA" in kwargs or "BETA_VEC" in kwargs:
                raise ValueError("Either BETA, BETA_VEC, or BETA1 and BETA2 must be provided")

            v = [max(0, kwargs["BETA0"] * (1+kwargs["BETA1"]*np.cos(np.pi/182.5*t))) for t in range(365)]
            v = (ctypes.c_float * 365)(*v)
            setattr(self._params, "BETA_VEC", v)

            del kwargs["BETA0"]
            del kwargs["BETA1"]

        if "BETA" in kwargs:
            raise ValueError("BETA parameter is deprecated; instead use BETA1 with BETA2=0")

        if "BETA_VEC" in kwargs:
            if len(kwargs["BETA_VEC"]) != 365:
                raise ValueError("Daily Beta must be 365 element list of float")
            v = (ctypes.c_float * 365)(*kwargs["BETA_VEC"])
            setattr(self._params, "BETA_VEC", v)

            del kwargs["BETA_VEC"]

        for k, v in kwargs.items():
            if k not in self._params.__dir__():

                raise ValueError("Invalid parameter " + k + " passed to BICS_ABM")

            else:
                if k == "MU_VEC":
                    if len(v) != 9:
                        raise ValueError("Case fatality must be 9 element list")
                    v = (ctypes.c_float * 9)(* v)
                    setattr(self._params, k, v)

                elif k == "IMPORT_CASES_VEC":
                    if len(v) != 365:
                        raise ValueError("Daily import cases must be 365 element list of int")
                    v = (ctypes.c_int * 365)(* v)
                    setattr(self._params, k, v)

                elif k == "CONTACT_MULT_VEC":
                    if len(v) != 365:
                        raise ValueError("Daily contact multipliers must be 365 element list of int")
                    v = (ctypes.c_float * 365)(* v)
                    setattr(self._params, k, v)

                else:
                    setattr(self._params, k, v)

        self.seed = self._params.SEED

        if POLYMOD:
            self._POLYMOD = load_POLYMOD()
            self._POLYMOD = create_vax_priority(self._POLYMOD, vax_rules, seed=self.seed)
            _, self._POLYMOD = pop_to_np(self._POLYMOD)

        if pop is None:
            self._pop = BICS_global.loc[BICS_global.wave == self._params.WAVE,:].copy(deep=True)
            self._pop = create_vax_priority(self._pop, vax_rules, seed=self.seed)
            self._colnames, self._pop = pop_to_np(self._pop)
            self._pop = create_pop(
                    colnames = self._colnames, 
                    pop = self._pop, 
                    n_hh = self._params.N_HH, 
                    wave = self._params.WAVE,
                    seed=self.seed, 
                    polymod = self._POLYMOD)

        else:
            self._pop = pop

        self._pop = self._pop.astype(ctypes.c_double)


        self._trajectory = Trajectory()
        # self._instance = _BICS_ABM.BICS_ABM(self._pop, *self._pop.shape, self._params, silent)
        ctypes._reset_cache()
        # _BICS_ABM.BICS_ABM(self._pop, *self._pop.shape, self._trajectory, self._params, silent)

        # Trying to add threading
        t = Thread(target = _BICS_ABM.BICS_ABM, args = [self._pop, *self._pop.shape, self._trajectory, self._params, silent])
        t.daemon = True
        t.start()
        while t.is_alive():
            t.join(0.1)


        self.counter = self._trajectory.counter
        self.S = self._trajectory.S_array[:self.counter]
        self.E = self._trajectory.E_array[:self.counter]
        self.Ic = self._trajectory.Ic_array[:self.counter]
        self.Cc = self._trajectory.Cc_array[:self.counter]
        self.Isc = self._trajectory.Isc_array[:self.counter]
        self.Csc = self._trajectory.Csc_array[:self.counter]
        self.R = self._trajectory.R_array[:self.counter]
        self.D = self._trajectory.D_array[:self.counter]
        self.V1 = self._trajectory.V1_array[:self.counter]
        self.V2 = self._trajectory.V2_array[:self.counter]
        self.VW = self._trajectory.VW_array[:self.counter]
        self.VBoost = self._trajectory.VBoost_array[:self.counter]
        self.n_edges= self._trajectory.n_edges_array[:self.counter]

        del self._trajectory

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
    result = BICS_ABM(
        BETA0 = 0.1,
        BETA1 = 0.5,
        IMPORT_CASES_VEC = [1 if i%7 == 0 else 0 for i in range(365)],
        CONTACT_MULT_VEC = [1+np.cos(2*i/365*np.pi) for i in range(365)],
        MAX_DAYS = 365,
        silent = False,
        vax_rules=[VaccineRule("age>80")]
    )   
    """
    result = BICS_ABM(
        N_HH = 1000,
        SEED = 4949,
        RHO = 0.5,
        ALPHA = 0.25,
        BETA0 =0.5,
        BETA1 = 1,
        T0 = 60,
        ISOLATION_MULTIPLIER = 1, #0.5,
        N_VAX_DAILY = 1500,
        T_REINFECTION = 24*180,
        IMPORT_CASES_VEC = [1 if i%7 == 0 else 0 for i in range(365)],
        vax_rules = [VaccineRule(general=True, hesitancy=.5)],
        VEBoost = 1,

#         vax_rules = [
#             VaccineRule("age > 80"),
#             VaccineRule("age > 70"),
#             VaccineRule("age > 60"),
#             VaccineRule("age > 50", hesitancy = 0.5),
#             VaccineRule("age > 40", hesitancy = 0.5),
#             VaccineRule("age > 30", hesitancy = 0.5),
#             VaccineRule(general = True, hesitancy = 0.5),
#         ],
        silent = False,
        BOOSTER_DAY = 90,
        MAX_DAYS = 1*365)
    """
    """
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
        """

