import pstats, cProfile

import pyximport
pyximport.install()

import Population_cy

import igraph as ig
g = ig.Graph.Full(1000)

cProfile.runctx("Population_cy.extract_nodes_from_edgelist_cy(g.es)",
                globals(), locals(), "Profile.prof")

s = pstats.Stats("Profile.prof")
s.strip_dirs().sort_stats("time").print_stats()