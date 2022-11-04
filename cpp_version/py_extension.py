import os
import ctypes

path = os.getcwd()
path = os.path.join(path, "build", "libBICS_ABM.so")
BICS_ABM = ctypes.CDLL(path)

mu = (ctypes.c_float* 9)()
mu[0] = 0.0001
mu[1] = 0.0001
mu[2] = 0.0001
mu[3] = 0.0001
mu[4] = 0.0001
mu[5] = 0.0001
mu[6] = 0.0001
mu[7] = 0.0001
mu[8] = 0.0001

BICS_ABM.BICS_ABM_py()
