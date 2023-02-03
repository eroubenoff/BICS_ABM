from BICS_ABM import BICS_ABM, VaccineRule
import gc
import psutil
import sys
import numpy as np

for i in range(10):
    print("Iteration %d: %0.3f MB" % (
        i, psutil.Process().memory_info().rss / 1e6))
    # x = 
    x = BICS_ABM(silent=True)

    print("Iteration %d: %0.3f MB" % (
        i, psutil.Process().memory_info().rss / 1e6))
    del x
    # gc.collect()
    print("Iteration %d: %0.3f MB" % (
        i, psutil.Process().memory_info().rss / 1e6))
