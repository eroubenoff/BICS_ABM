"""
This script produces a .csv for use by the cpp_version.

Output is a csv file where each line is a node.
"""
# from load_data import sim_pop, lucid_data
import load_data
import csv

if __name__ == "__main__":
    n_hh = 100
    pop = load_data.sim_pop(n_hh, load_data.lucid_data['wave4'])
    popl = list()

    for k, v in pop.G.nodes(data=True):
        v["id"] = k
        popl.append(v)

    keys = popl[0].keys()

    with open('cpp_version/pop.csv', 'w', newline='') as output_file:
        dict_writer = csv.DictWriter(output_file, keys)
        dict_writer.writeheader()
        dict_writer.writerows(popl)