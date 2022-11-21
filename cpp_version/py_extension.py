import os
import ctypes
import numpy
from numpy.ctypeslib import ndpointer

path = os.getcwd()
path = os.path.join(path, "build", "libBICS_ABM_lib.dylib")
_BICS_ABM = ctypes.CDLL(path)


# Return object
class Trajectory (ctypes.Structure):
    _fields_ = [
        ('S_array', ctypes.c_int*5000),
        ('E_array', ctypes.c_int*5000),
        ('I_array', ctypes.c_int*5000),
        ('R_array', ctypes.c_int*5000),
        ('D_array', ctypes.c_int*5000),
        ('V1_array', ctypes.c_int*5000),
        ('V2_array', ctypes.c_int*5000),
        ('counter', ctypes.c_int)
    ]

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
            ('POP_SEED', ctypes.c_int),
            ('N_VAX_DAILY', ctypes.c_int),
            ('VE1', ctypes.c_float),
            ('VE2', ctypes.c_float),
            ('ISOLATION_MULTIPLIER', ctypes.c_float),
            ('VAX_RULES_COLS', ctypes.c_char*1000),
            ('VAX_RULES_VALS', ctypes.c_char*1000),
            ('VAX_CONDS_N', ctypes.c_int*100),
            ('VAX_RULES_N', ctypes.c_int)
            
    ]

_BICS_ABM.BICS_ABM.argtypes = [Params]
_BICS_ABM.BICS_ABM.restype = Trajectory

_BICS_ABM.init_params.argtypes = () 
_BICS_ABM.init_params.restype = Params 



class BICS_ABM:

    def __init__(self, vax_rules = None, **kwargs):
        self._params = _BICS_ABM.init_params()
        for k, v in kwargs.items():
            if k not in ['N_HH','WAVE', 'GAMMA_MIN', 'GAMMA_MAX', 'SIGMA_MIN', 'SIGMA_MAX', \
                    'BETA', 'MU_VEC', 'INDEX_CASES', 'SEED', 'POP_SEED', 'N_VAX_DAILY',\
                    'VE1', 'VE2']:
                raise ValueError("Invalid parameter passed to BICS_ABM")

            else:
                setattr(self._params, k, v)

        """
        Assume that vax_rules is in the form of:
        [
            [("age", ">85")],
            [("age", "[75,85)")],
            [("age", "[65,75)"), ("hesitancy", "0.4")]
        ]

        """
        vax_rules_cols = ""
        vax_rules_vals = ""
        vax_rules_n = 0      # Number of rules
        vax_conds_n = [0] * 100 # Number of conditions per rule
        if vax_rules is not None:
            for rule in vax_rules:
                vax_rules_n += 1
                for criteria in rule:
                    vax_conds_n[vax_rules_n - 1] += 1
                    vax_rules_cols += criteria[0] 
                    vax_rules_cols += ";"
                    vax_rules_vals += criteria[1]
                    vax_rules_vals += ";"

            print(vax_rules_n, vax_conds_n)

            self._params.VAX_RULES_COLS = vax_rules_cols.encode("utf-8")
            self._params.VAX_RULES_VALS = vax_rules_vals.encode("utf-8")
            self._params.VAX_RULES_N = ctypes.c_int(vax_rules_n)
            for i in range(100):
                self._params.VAX_CONDS_N[i] = ctypes.c_int(vax_conds_n[i])


        self._instance = _BICS_ABM.BICS_ABM(self._params)

        counter = self._instance.counter
        self.S = self._instance.S_array[:counter]
        self.E = self._instance.E_array[:counter]
        self.I = self._instance.I_array[:counter]
        self.R = self._instance.R_array[:counter]
        self.D = self._instance.D_array[:counter]
        self.V1 = self._instance.V1_array[:counter]
        self.V2 = self._instance.V2_array[:counter]


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

beta_range = np.arange(0, .5, 0.05)
deaths = list() 

for beta in beta_range:
    tmp_list = list()
    for j in range(10):
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

