from load_data import sim_pop, lucid_data
import networkx as nx

pop = sim_pop(10, lucid_data['wave4'])
nx.draw_circular(pop.G)


def test_load():
    print(pop.node_ids)
    print(pop.edges)
