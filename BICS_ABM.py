import os
import ctypes
import numpy
from numpy.ctypeslib import ndpointer

path = os.getcwd()
path = os.path.join(path, "build", "libBICS_ABM_lib.dylib")
_BICS_ABM = ctypes.CDLL(path)


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
            ('POP_SEED', ctypes.c_int),
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
            ('VAX_RULES_COLS', ctypes.c_char*1000),
            ('VAX_RULES_VALS', ctypes.c_char*1000),
            ('VAX_CONDS_N', ctypes.c_int*100),
            ('VAX_RULES_N', ctypes.c_int)
            
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
        ('counter', ctypes.c_int)
    ]

_BICS_ABM.BICS_ABM.argtypes = [Params]
_BICS_ABM.BICS_ABM.restype = Trajectory

_BICS_ABM.init_params.argtypes = () 
_BICS_ABM.init_params.restype = Params 



class BICS_ABM:

    def __init__(self, vax_rules = None, **kwargs):
        self._params = _BICS_ABM.init_params()
        for k, v in kwargs.items():
            if k not in self._params.__dir__():

                raise ValueError("Invalid parameter " + k + " passed to BICS_ABM")

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
        self.I = self._instance.Ic_array[:counter]
        self.I = self._instance.Isc_array[:counter]
        self.R = self._instance.R_array[:counter]
        self.D = self._instance.D_array[:counter]
        self.V1 = self._instance.V1_array[:counter]
        self.V2 = self._instance.V2_array[:counter]

