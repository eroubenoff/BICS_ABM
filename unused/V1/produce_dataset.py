"""
This script produces a .csv for use by the cpp_version.

Output is a csv file where each line is a node.
"""
# from load_data import sim_pop, lucid_data
import pdb

import load_data
import csv
import igraph as ig

if __name__ == "__main__":
    n_hh = 1000
    pop = load_data.sim_pop(n_hh, 4)
    popl = list()

    # Need to turn age bins from strings into labels for cpp

    agedict = {}
    agedict['[0,18)'] = 0
    agedict['[18,25)'] = 1
    agedict['[25,35)'] = 2
    agedict['[35,45)'] = 3
    agedict['[45,55)'] = 4
    agedict['[55,65)'] = 5
    agedict['[65,75)'] = 6
    agedict['[75,85)'] = 7
    agedict['>85'] = 8

    pop.G.vs['age'] = [agedict[x] for x in pop.G.vs['age']]

    pop = pop.G.get_vertex_dataframe()
    pop.to_csv("cpp_version/pop.csv")


    for k, v in load_data.lucid_data.items():
        v.to_csv("cpp_version/lucid/" + k + ".csv")
