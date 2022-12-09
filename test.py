from BICS_ABM import BICS_ABM, VaccineRule
import gc
import psutil
import sys

for i in range(10):
    print("Iteration %d: %0.3f MB" % (
        i, psutil.Process().memory_info().rss / 1e6))
    # x = 
    BICS_ABM()

    # print(sum([sys.getsizeof(getattr(x,y) ) for y in dir(x)]) / 1e6)
    # del x
    gc.collect()
